// ESP8266 Messung Temperatur im Kühlfach und im Gefrierfach
// Senden der Daten mittels mqqt
// Darstellung der Daten in Dash-Board ESPDash
// OTA integriert 

#include <Arduino.h>
#if defined(ESP8266)
  /* ESP8266 Dependencies */
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
  #include <ESPAsyncWebSrv.h>
#elif defined(ESP32)
  /* ESP32 Dependencies */
  #include <WiFi.h>
  #include <AsyncTCP.h>
  #include <ESPAsyncWebSrv.h>
#endif
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <ESPDash.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Variables to store temperature values
String tempDev0 = "";
String tempDev1 = "";
// Number of temperature devices found
int numberOfDevices;

// MQTT
const char* mqttServer = "ip-address-mqtt-broker";
const int mqttPort = 1883;
const char* mqttUser = "mqtt-user";
const char* mqttPassword = "mqtt-password";
const char* mqttHostname = "hostname";

WiFiClient espClient;
PubSubClient client(espClient);

//DS 18B20
DeviceAddress tempDeviceAddress; 

// Data wire is connected to GPIO 4
#define ONE_WIRE_BUS 4

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature sensors(&oneWire);

// Set your Static IP address
IPAddress local_IP(xxx, xxx, xxx, xxx);
// Set your Gateway IP address
IPAddress gateway(xxx, xxx, xxx, xxx);
IPAddress subnet(xxx, xxx, xxx, xxx);
IPAddress primaryDNS(xxx, xxx, xxx, xxx);   //optional
IPAddress secondaryDNS(xxx, xxx, xxx, xxx); //optional
/* Your WiFi Credentials */
const char* Hostname = "hostname";
const char* ssid = "wlan-ssid";
const char* password = "wlan-password";
const char* OTAHostname ="OTA-Hostname";
const char* OTAPassword = "OTA-password";


/* Start Webserver */
AsyncWebServer server(80);

/* Attach ESP-DASH to AsyncWebServer */
ESPDash dashboard(&server); 

Card tempDev1Dash(&dashboard, TEMPERATURE_CARD, "Temperatur Kühlfach", "°C");
Card tempDev0Dash(&dashboard, TEMPERATURE_CARD, "Temperatur Gefrierfach", "°C");

void setup() {
  Serial.begin(115200);
  server.begin();
  sensors.begin();

  // Grab a count of devices on the wire
  numberOfDevices = sensors.getDeviceCount();
  client.setServer(mqttServer, mqttPort);
  /* Connect WiFi */
  WiFi.begin(ssid, password);
  WiFi.hostname(Hostname);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) 
  {
      Serial.printf("WiFi Connection Failed!\n");
      return;
  }
  ArduinoOTA.setHostname(OTAHostname);
  ArduinoOTA.setPassword(OTAPassword);
  ArduinoOTA.setPort(80);
  ArduinoOTA.begin();
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() 
{
  ArduinoOTA.handle();
  client.connect(mqttHostname, mqttUser, mqttPassword);
  sensors.requestTemperatures(); 
    for(int i=0;i<numberOfDevices; i++){
    // Search the wire for address
    if(sensors.getAddress(tempDeviceAddress, i)){
      float tempC = sensors.getTempC(tempDeviceAddress);
      if (i<=0){
        tempDev0 = tempC;
        //Serial.println("tempDev0 =");
        //Serial.print(tempDev0);
        client.publish("esp-kuehlschrank/temperatur/Gefrierfach",  String(tempDev0).c_str());
        }
      else{
        tempDev1 = tempC;
        //Serial.println("tempDev1 =");
        //Serial.print(tempDev1);
        client.publish("esp-kuehlschrank/temperatur/Kuehlfach",  String(tempDev1).c_str());
      }
    }
  }
  /* Update Card Values */
  tempDev1Dash.update(tempDev1, "°C");
  tempDev0Dash.update(tempDev0, "°C");
  /* Send Updates to our Dashboard (realtime) */
  dashboard.sendUpdates();
  delay(5000);
}
