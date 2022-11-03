#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <Arduino_JSON.h>
char Ssid[40];
char Password[40];

void loadConfig(){ // Load data SSID & Password WiFi .json from SPIFFS for WiFiSmartConfig
  Serial.println("Mounting FS...");
  if (SPIFFS.begin()) {
    Serial.println("Mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("Reading config file");
      File configFile = SPIFFS.open("/config.json");
      if (configFile) {
        Serial.println("Opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(Ssid, json["Ssid"]);
          strcpy(Password, json["Password"]);
 
        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  
  WiFi.begin(Ssid, Password);
  Serial.println("Load Config...");
  delay(2000);
}

void setspiffs(){
  if (WiFi.status() != WL_CONNECTED) { // if WiFi is not connected
    //Init WiFi as Station, start SmartConfig
    WiFi.mode(WIFI_AP_STA);
    WiFi.beginSmartConfig();

    //Wait for SmartConfig packet from mobile
    Serial.println("Waiting for SmartConfig.");
    while (!WiFi.smartConfigDone()) {
      delay(500);
      Serial.print(".");
    }

    Serial.println("");
    Serial.println("SmartConfig received.");
    
    //Wait for WiFi to connect to AP
    Serial.println("Waiting for WiFi");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    
    Serial.println("WiFi Connected.");
   
    String Ssid = WiFi.SSID();
    String Password =  WiFi.psk();
  
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("SSID: ");
    Serial.println(Ssid);
    Serial.print("PSS: ");
    Serial.println(Password);

    //--------- Write and Read SSID & PSK ---------
    if(!SPIFFS.begin(true)){
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
    }

    //--------- Write to file
    File fileToWrite = SPIFFS.open("/config.json", FILE_WRITE);

    if(!fileToWrite){
      Serial.println("There was an error opening the file for writing");
      return;
    }

    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& doc = jsonBuffer.createObject();
  
    doc["Ssid"] = Ssid;
    doc["Password"] = Password;

    doc.prettyPrintTo(Serial);
    doc.printTo(fileToWrite);
    Serial.println(" ");

    fileToWrite.close();
  } else {
    Serial.println("WiFi Connected.");
    digitalWrite(2, HIGH);
  }

  //---------- Read file
  File fileToRead = SPIFFS.open("/config.json");

  if(!fileToRead){
    Serial.println("Failed to open file for reading");
    return;
  }
  Serial.println("File Content:");

  while(fileToRead.available()){
    Serial.write(fileToRead.read());
  }
    
  fileToRead.close();
}

void listAllFiles(){ // Show all files in SPIFFS
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while(file){
    Serial.println("");
    Serial.print("FILE: ");
    Serial.println(file.name());
    file = root.openNextFile();
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);

  loadConfig();
  setspiffs();
  listAllFiles();
}

void loop() {
  // put your main code here, to run repeatedly:

}
