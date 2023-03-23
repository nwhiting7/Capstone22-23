#include "BLEDevice.h"

/* Specify the Service UUID of Server */
static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
/* Specify the Characteristic UUID of Server */
static BLEUUID    charUUID_PM25("beb5483e-36e1-4688-b7f5-ea07361b26a8");
static BLEUUID    charUUID_CO2("beb5483e-36e1-4688-n9w7-ea07361b26c9");
static BLEUUID    charUUID_TempF("beb5483e-36e1-7497-n9w7-ea07361b26c9");
static BLEUUID    charUUID_Humidity("beb5483e-74e9-1234-n9w7-ea07361b26c9");
static BLEUUID    charUUID_Motion("ndw5493f-36e1-7499-n9w7-ea07361b26c9");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pCharacteristic_PM25;
static BLERemoteCharacteristic* pCharacteristic_CO2;
static BLERemoteCharacteristic* pCharacteristic_TempF;
static BLERemoteCharacteristic* pCharacteristic_Humidity;
static BLERemoteCharacteristic* pCharacteristic_Motion;
static BLEAdvertisedDevice* myDevice;

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
double read_all();
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
std:: string pm25_send;
std:: string temp_send;
std:: string co2_send;
std:: string hum_send;
std:: string mot_send;

static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,
                            uint8_t* pData, size_t length, bool isNotify)
{
  Serial.print("Notify callback for characteristic ");
  Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
  Serial.print(" of data length ");
  Serial.println(length);
  Serial.print("data: ");
  Serial.println((char*)pData);
}

class MyClientCallback : public BLEClientCallbacks
{
  void onConnect(BLEClient* pclient)
  {
    
  }

  void onDisconnect(BLEClient* pclient)
  {
    connected = false;
    Serial.println("onDisconnect");
  }
};

/* Start connection to the BLE Server */
bool connectToServer()
{
  Serial.print("Forming a connection to ");
  Serial.println(myDevice->getAddress().toString().c_str());
    
  BLEClient*  pClient  = BLEDevice::createClient();
  Serial.println(" - Created client");

  pClient->setClientCallbacks(new MyClientCallback());

    /* Connect to the remote BLE Server */
  pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
  Serial.println(" - Connected to server");

    /* Obtain a reference to the service we are after in the remote BLE server */
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr)
  {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our service");


  /* Obtain a reference to the characteristic in the service of the remote BLE server */
  pCharacteristic_PM25 = pRemoteService->getCharacteristic(charUUID_PM25);
  if (pCharacteristic_PM25 == nullptr)
  {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charUUID_PM25.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our characteristic");

  /* Read the value of the characteristic */
  /* Initial value is 'Hello, World!' */
  if(pCharacteristic_PM25->canRead())
  {
    std::string value = pCharacteristic_PM25->readValue();
    Serial.print("The characteristic value was: ");
    Serial.println(value.c_str());
  }

  if(pCharacteristic_PM25->canNotify())
  {
    pCharacteristic_PM25->registerForNotify(notifyCallback);

  }

  pCharacteristic_CO2 = pRemoteService->getCharacteristic(charUUID_CO2);
  if (pCharacteristic_CO2 == nullptr)
  {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charUUID_CO2.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our characteristic");

  /* Read the value of the characteristic */
  /* Initial value is 'Hello, World!' */
  if(pCharacteristic_CO2->canRead())
  {
    std::string value = pCharacteristic_CO2->readValue();
    Serial.print("The characteristic value was: ");
    Serial.println(value.c_str());
  }

  if(pCharacteristic_CO2->canNotify())
  {
    pCharacteristic_CO2->registerForNotify(notifyCallback);

  }

  pCharacteristic_TempF = pRemoteService->getCharacteristic(charUUID_TempF);
  if (pCharacteristic_TempF == nullptr)
  {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charUUID_TempF.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our characteristic");

  /* Read the value of the characteristic */
  /* Initial value is 'Hello, World!' */
  if(pCharacteristic_TempF->canRead())
  {
    std::string value = pCharacteristic_TempF->readValue();
    Serial.print("The characteristic value was: ");
    Serial.println(value.c_str());
  }

  if(pCharacteristic_TempF->canNotify())
  {
    pCharacteristic_TempF->registerForNotify(notifyCallback);

  }

  pCharacteristic_Humidity = pRemoteService->getCharacteristic(charUUID_Humidity);
  if (pCharacteristic_Humidity == nullptr)
  {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charUUID_Humidity.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our characteristic");

  /* Read the value of the characteristic */
  /* Initial value is 'Hello, World!' */
  if(pCharacteristic_Humidity->canRead())
  {
    std::string value = pCharacteristic_Humidity->readValue();
    Serial.print("The characteristic value was: ");
    Serial.println(value.c_str());
  }

  if(pCharacteristic_Humidity->canNotify())
  {
    pCharacteristic_Humidity->registerForNotify(notifyCallback);

  }

  pCharacteristic_Motion = pRemoteService->getCharacteristic(charUUID_Motion);
  if (pCharacteristic_Motion == nullptr)
  {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charUUID_Motion.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our characteristic");

  /* Read the value of the characteristic */
  /* Initial value is 'Hello, World!' */
  if(pCharacteristic_Motion->canRead())
  {
    std::string value = pCharacteristic_Motion->readValue();
    Serial.print("The characteristic value was: ");
    Serial.println(value.c_str());
  }

  if(pCharacteristic_Motion->canNotify())
  {
    pCharacteristic_Motion->registerForNotify(notifyCallback);

  }

    connected = true;
    return true;
}
/* Scan for BLE servers and find the first one that advertises the service we are looking for. */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks
{
 /* Called for each advertising BLE server. */
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    /* We have found a device, let us now see if it contains the service we are looking for. */
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID))
    {
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    }
  }
};

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
double read_all() {
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

  return val.MassPM2;
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


void setup()
{
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("ESP32-BLE-Client");

  /* Retrieve a Scanner and set the callback we want to use to be informed when we
     have detected a new device.  Specify that we want active scanning and start the
     scan to run for 5 seconds. */
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);

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
  scd30.forceRecalibrationWithReference(400);
  Serial.println("SCD30 Found!");
  Serial.print("Measurement Interval: ");
  Serial.print(scd30.getMeasurementInterval());
  Serial.println(" seconds");
  //Done Connecting to SCD30---------------------------------------------------------------------------------------
}


void loop()
{

  /* If the flag "doConnect" is true, then we have scanned for and found the desired
     BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
     connected we set the connected flag to be true. */
  if (doConnect == true)
  {
    if (connectToServer())
    {
      Serial.println("We are now connected to the BLE Server.");
    } 
    else
    {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }

  /* If we are connected to a peer BLE Server, update the characteristic each time we are reached
     with the current time since boot */
  if (connected)
  {
    if (scd30.dataReady()){
    Serial.println("Data available!");

    if (!scd30.read()){ Serial.println("Error reading SCD30 data"); return; }

    Serial.print("Temperature: ");
    Serial.print(scd30.temperature);
    Serial.println(" degrees C");
    temp_send = std::to_string(scd30.temperature);
    
    Serial.print("Relative Humidity: ");
    Serial.print(scd30.relative_humidity);
    Serial.println(" %");
    hum_send = std::to_string(scd30.relative_humidity);
    
    Serial.print("Altitude: ");
    Serial.print(scd30.getAltitudeOffset());
    Serial.println("");
    Serial.print("CO2: ");
    Serial.print(scd30.CO2, 3);
    co2_send = std::to_string(scd30.CO2);

    Serial.println(" ppm");
    Serial.println("");
  }

    pm25_send = std::to_string(read_all());

    motionState = digitalRead(motionPin);
    Serial.print("Motion[Yes(1)/No(0)]:");
    Serial.println(motionState);   
    Serial.println("");  
    mot_send = std::to_string(motionState);

    /* Set the characteristic's value to be the array of bytes that is actually a string */
    pCharacteristic_TempF->writeValue(temp_send.c_str(), true);
    pCharacteristic_CO2->writeValue(co2_send.c_str(), true);
    pCharacteristic_PM25->writeValue(pm25_send.c_str(), true);
    pCharacteristic_Humidity->writeValue(hum_send.c_str(), true);
    pCharacteristic_Motion->writeValue(mot_send.c_str(), true);
    /* You can see this value updated in the Server's Characteristic */
  }
  else if(doScan)
  {
    BLEDevice::getScan()->start(0);  // this is just example to start scan after disconnect, most likely there is better way to do it in arduino
    Serial.println("Error.");
  }
  
  delay(1000); /* Delay a second between loops */
}
