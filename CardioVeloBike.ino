#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

BLEServer *pServer = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
int upPin = 18;
int dwnPin = 19;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      for(int i = 0; i < 4; i++){    //on connect shifting gear to the lowest (first) gear
        delay(100);
        digitalWrite(dwnPin, LOW);   // GND for 100ms on dwnPin
        delay(100);
        digitalWrite(dwnPin, HIGH);
        }
    }
    };

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        
        int rxTaskBase = rxValue[1];    //  base64 char to ASCII 
        if(rxTaskBase == 81 ){          //   -> 1 -> shifting down
          digitalWrite(dwnPin, LOW);
          delay(100);
          digitalWrite(dwnPin, HIGH);
          }else if(rxTaskBase == 103){  // -> 2  -> shifting up
          digitalWrite(upPin, LOW);
          delay(100);
          digitalWrite(upPin, HIGH);
        }     
      }
    }
};

void setup() {
  Serial.begin(115200);
  pinMode(upPin, OUTPUT);
  pinMode(dwnPin, OUTPUT);
  digitalWrite(dwnPin, HIGH); // 3.3V default at ext pins
  digitalWrite(upPin, HIGH);
  
  BLEDevice::init("cardioVelo Bike");   // custom BLE Device name
  
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  
  BLEService *pService = pServer->createService(BLEUUID((uint16_t)0x1816));     //  "bike speed and cadence" 16-bit assigned
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(  
                                         BLEUUID((uint16_t)0x2A65),             // "cycling power feature" 16-bit assigned
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();
  pServer->getAdvertising()->start();
  
}

void loop() {
  if (!deviceConnected && oldDeviceConnected) {
        delay(600);
        pServer->getAdvertising()->start(); 
        oldDeviceConnected = deviceConnected;
    }
  if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = deviceConnected;
    }
  
}
