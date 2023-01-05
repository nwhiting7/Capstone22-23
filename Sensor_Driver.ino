//--------------------------------------------------------------------------------------------------------
//SPS30 Initialization------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
#include "sps30.h"
// Defines for SPS30 functionality.
#define SP30_COMMS I2C_COMMS
#define DEBUG 0
// function prototypes (sometimes the pre-processor does not create prototypes themself on ESPxx)
void serialTrigger(char *mess);
void ErrtoMess(char *mess, uint8_t r);
void Errorloop(char *mess, uint8_t r);
void GetDeviceInfo();
bool read_all();
// create constructor
SPS30 sps30;
//--------------------------------------------------------------------------------------------------------
//End of SPS30 Initialization-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------------
//SCD30 Initialization------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
#include <Adafruit_SCD30.h>
Adafruit_SCD30  scd30;
//--------------------------------------------------------------------------------------------------------
//End of SCD30 Initialization-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------------
//IR Motion Initialization------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
const int motionPin = 23;
int motionState = 0;
//--------------------------------------------------------------------------------------------------------
//End of IR Motion Initialization-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------



void setup() {

  Serial.begin(115200);

  //Connect to SPS30---------------------------------------------------------------------------------------
  Serial.println(F("Trying to connect to SPS30"));

  // set driver debug level
  sps30.EnableDebugging(DEBUG);

  // Begin communication channel;
  if (!sps30.begin(SP30_COMMS))
    Errorloop((char *)"could not initialize communication channel for SPS30.", 0);

  // check for SPS30 connection
  if (!sps30.probe()) Errorloop((char *)"could not probe / connect with SPS30.", 0);
  else Serial.println(F("Detected SPS30."));

  // reset SPS30 connection
  if (!sps30.reset()) Errorloop((char *)"could not reset.", 0);

  // start measurement
  if (sps30.start()) Serial.println(F("SPS32 started"));
  else Errorloop((char *)"Could NOT start measurement", 0);

  if (SP30_COMMS == I2C_COMMS) {
    if (sps30.I2C_expect() == 4)
      Serial.println(F(" !!! Due to I2C buffersize only the SPS30 MASS concentration is available !!! \n"));
  }
  //Done Connecting to SPS30---------------------------------------------------------------------------------------

  //Connect to SCD30-----------------------------------------------------------------------------------------------
  Serial.println("Connecting to SCD30!");

  // Try to initialize!
  if (!scd30.begin()) {
    Serial.println("Failed to connect to SCD30 chip");
    while (1) { delay(10); }
  }
  scd30.setAltitudeOffset(0);
  Serial.println("SCD30 Found!");
  Serial.print("Measurement Interval: ");
  Serial.print(scd30.getMeasurementInterval());
  Serial.println(" seconds");
  //Done Connecting to SCD30---------------------------------------------------------------------------------------
}

void loop() {
  if (scd30.dataReady()){
    Serial.println("Data available!");

    if (!scd30.read()){ Serial.println("Error reading SCD30 data"); return; }

    Serial.print("Temperature: ");
    Serial.print(scd30.temperature);
    Serial.println(" degrees C");
    
    Serial.print("Relative Humidity: ");
    Serial.print(scd30.relative_humidity);
    Serial.println(" %");
    
    Serial.print("Altitude: ");
    Serial.print(scd30.getAltitudeOffset());
    Serial.println("");
    Serial.print("CO2: ");
    Serial.print(scd30.CO2, 3);
    Serial.println(" ppm");
    Serial.println("");

    if(!read_all()){ Serial.println("Error reading SPS30 data"); return; }
    Serial.println("");

    motionState = digitalRead(motionPin);
    Serial.print("Motion[Yes(1)/No(0)]:");
    Serial.println(motionState);   
    Serial.println("");  

  }
}

/**
 * @brief : read and display device info
 */
void GetDeviceInfo() {
  char buf[32];
  uint8_t ret;
  SPS30_version v;

  //try to read serial number
  ret = sps30.GetSerialNumber(buf, 32);
  if (ret == SPS30_ERR_OK) {
    Serial.print(F("Serial number : "));
    if (strlen(buf) > 0) Serial.println(buf);
    else Serial.println(F("not available"));
  } else
    ErrtoMess((char *)"could not get serial number", ret);

  // try to get product name
  ret = sps30.GetProductName(buf, 32);
  if (ret == SPS30_ERR_OK) {
    Serial.print(F("Product name  : "));

    if (strlen(buf) > 0) Serial.println(buf);
    else Serial.println(F("not available"));
  } else
    ErrtoMess((char *)"could not get product name.", ret);

  // try to get version info
  ret = sps30.GetVersion(&v);
  if (ret != SPS30_ERR_OK) {
    Serial.println(F("Can not read version info"));
    return;
  }

  Serial.print(F("Firmware level: "));
  Serial.print(v.major);
  Serial.print(".");
  Serial.println(v.minor);

  if (SP30_COMMS != I2C_COMMS) {
    Serial.print(F("Hardware level: "));
    Serial.println(v.HW_version);

    Serial.print(F("SHDLC protocol: "));
    Serial.print(v.SHDLC_major);
    Serial.print(".");
    Serial.println(v.SHDLC_minor);
  }

  Serial.print(F("Library level : "));
  Serial.print(v.DRV_major);
  Serial.print(".");
  Serial.println(v.DRV_minor);
}

/**
 * @brief : read and display all values
 */
bool read_all() {
  static bool header = true;
  uint8_t ret, error_cnt = 0;
  struct sps_values val;

  // loop to get data
  do {

    ret = sps30.GetValues(&val);

    // data might not have been ready
    if (ret == SPS30_ERR_DATALENGTH) {

      if (error_cnt++ > 3) {
        ErrtoMess((char *)"Error during reading values: ", ret);
        return (false);
      }
      delay(1000);
    }

    // if other error
    else if (ret != SPS30_ERR_OK) {
      //ErrtoMess((char *)"Error during reading values: ", ret);
      return (false);
    }

  } while (ret != SPS30_ERR_OK);

  Serial.println(F("PM2.5[μg/m3]"));
  Serial.print(val.MassPM2);
  Serial.print(F("\n"));

  return (true);
}

/**
 *  @brief : continued loop after fatal error
 *  @param mess : message to display
 *  @param r : error code
 *
 *  if r is zero, it will only display the message
 */
void Errorloop(char *mess, uint8_t r) {
  if (r) ErrtoMess(mess, r);
  else Serial.println(mess);
  Serial.println(F("Program on hold"));
  for (;;) delay(100000);
}

/**
 *  @brief : display error message
 *  @param mess : message to display
 *  @param r : error code
 *
 */
void ErrtoMess(char *mess, uint8_t r) {
  char buf[80];

  Serial.print(mess);

  sps30.GetErrDescription(r, buf, 80);
  Serial.println(buf);
}
