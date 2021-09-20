#include <FS.h>          // this needs to be first, or it all crashes and burns...
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <BlynkSimpleEsp8266.h>
#include <WiFiManager.h>
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson

#ifdef ESP32
  #include <SPIFFS.h>
#endif
WiFiManager wm;





const char* ssid = "Dragon";
const char* pass = "";
const char* auth = "";
char api_token[33] = "YOUR_API_TOKEN";
#define PIN 14

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void setupSpiffs(){
  //clean FS, for testing
   //SPIFFS.format();

  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          //strcpy(mqtt_server, json["mqtt_server"]);
          //strcpy(mqtt_port, json["mqtt_port"]);
          strcpy(api_token, json["api_token"]);
          Serial.print("api-tokenfromjson");
          Serial.println(api_token);

          // if(json["ip"]) {
          //   Serial.println("setting custom ip from config");
          //   strcpy(static_ip, json["ip"]);
          //   strcpy(static_gw, json["gateway"]);
          //   strcpy(static_sn, json["subnet"]);
          //   Serial.println(static_ip);
          // } else {
          //   Serial.println("no custom ip in config");
          // }

        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read
}

//  The overall fire brightness
//  (this can affect both color levels and power consumption)
int brightness = 255;

struct RGB {
  byte r;
  byte g;
  byte b;
  byte w;
};

//  The flame color array (the first is the default):
RGB flameColors[] = {
  { 226, 121, 35,50},  // Orange flame
  { 158, 8, 148,0},   // Purple flame 
  { 74, 150, 12,0},   // Green flame
  { 226, 35, 15,15}    // Red flame
  };

//  Number of flame colors
int NUMBER_OF_COLORS = sizeof(flameColors) / sizeof(RGB);

//  Tracks the current color
int currentColorIndex = 3;

//  The button pin
//const int buttonPin = 12;

//  Variable for reading the pushbutton status
int buttonState = 0;

int ligth = 0; //switch oncanal w

//  Tracking if it's ok to shift colors or not
bool okToChangeColors = false;

int previousligth = 0;

//char auth[] = "3rIKC-brneIuH_3a3g6LhMdE0ULWt4Hp";
//http://cloud.blynk.cc:8080/3rIKC-brneIuH_3a3g6LhMdE0ULWt4Hp/pin/v3/2



Adafruit_NeoPixel strip = Adafruit_NeoPixel(16, PIN, NEO_GRBW + NEO_KHZ800);






void setup() {

  

   

  WiFi.mode(WIFI_STA);
  Serial.begin(115200);
  delay(1000);

   setupSpiffs();
  
   //set config save notify callback
  wm.setSaveConfigCallback(saveConfigCallback);
  WiFiManagerParameter custom_api_token("api", "api token", "", 33);
  wm.addParameter(&custom_api_token);

//WiFi.disconnect();

  Serial.println("connection");
  pass = "devdragon12345";
  Serial.print("password:");
  Serial.println(pass);
  Serial.println("\n");
  
  if(!wm.autoConnect(ssid,pass))
    Serial.println ("erreur de connexion");
 else 
      Serial.println("connexion ok");
       Serial.print("serial ");
      const char* tokentest = custom_api_token.getValue();
 Serial.print("size:");
  Serial.println(strlen(tokentest));
 Serial.println(tokentest);
 if(strlen(tokentest) > 30){
  
  Serial.println("strcpy");
  strcpy(api_token,tokentest);
 }
 
 
      Serial.print("serial2 ");
 Serial.println(api_token);
  
 
    if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    //json["mqtt_server"] = mqtt_server;
    //json["mqtt_port"]   = mqtt_port;
    json["api_token"]   = api_token;

    // json["ip"]          = WiFi.localIP().toString();
    // json["gateway"]     = WiFi.gatewayIP().toString();
    // json["subnet"]      = WiFi.subnetMask().toString();

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.prettyPrintTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
    shouldSaveConfig = false;
  }
  
  
  
  strip.begin();
 
  strip.setBrightness(brightness);
  strip.show(); // Initialize all pixels to 'off'


  Serial.println("ici");
  Serial.println(api_token);
  Serial.println("ici2");
  ;
 

 
  Blynk.begin(api_token, ssid, pass);
  
}

void loop() {
  //  Get the current colors based on the color index:
 strip.clear();
  strip.show();
  Blynk.run();
  if(ligth > 0){
    if(previousligth != ligth){
    uint32_t white = strip.Color(0, 0, 0, ligth);
    strip.fill(white,0,16);
    strip.show();
    previousligth = ligth;
  }
  }
  
}
BLYNK_WRITE(V1)
{
  ligth = param.asInt();
}

BLYNK_WRITE(V3)
{
  currentColorIndex = param.asInt();
  Serial.println("update");
  
  
}
    
BLYNK_WRITE(V2)
{
  brightness = param.asInt();
}
BLYNK_WRITE(V4)
{
  
  flame(param.asInt());
  
} 

void flame(const byte boucle) {
  

  strip.setBrightness(brightness);
  RGB currentColor = flameColors[currentColorIndex];

  // read the state of the pushbutton value:
  
  Serial.println(currentColorIndex);

  //  Button is being pressed and it's OK to change colors:
  if (buttonState == HIGH) {
    if(okToChangeColors){
      okToChangeColors = false;
      
      //  Shift to another color flame:
      if(currentColorIndex < (NUMBER_OF_COLORS - 1)){ 
        currentColorIndex++;  
      } else {
        currentColorIndex = 0;
      }
    }
  }
  else {
    okToChangeColors = true;
  }
  Serial.println("flame is on");
for (int y=0; y<boucle; y++){
  //  Flicker, based on our initial RGB values
  for(int i=0; i<strip.numPixels(); i++) {
    int flicker = random(0,55);
    int r1 = currentColor.r-flicker;
    int g1 = currentColor.g-flicker;
    int b1 = currentColor.b-flicker;
    int w1 = 0;
    if(g1<0) g1=0;
    if(r1<0) r1=0;
    if(b1<0) b1=0;
    if(w1<0) w1=0;
    strip.setPixelColor(i,r1,g1, b1,ligth);
  }
  strip.show();

  //  Adjust the delay here, if you'd like.  Right now, it randomizes the 
  //  color switch delay to give a sense of realism
  delay(random(50,100));
}

 Serial.println("flameoff");
  strip.clear();
  strip.show();
}
