//ItKindaWorks - Creative Commons 2016
//github.com/ItKindaWorks
//
//Requires PubSubClient found here: https://github.com/knolleary/pubsubclient
//
//ESP8266 Simple MQTT light controller


#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>



//EDIT THESE LINES TO MATCH YOUR SETUP
#define MQTT_SERVER "YOUR.MQTT.SERVER.IP"
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

//LED on ESP8266 GPIO2
const int lightPin = 12;

//topic to subscribe to for the light
char* lightTopic = "/house/light1";

//topic to publish to confirm that the light has been turned on for the python script to log
char* lightConfirmTopic = "/house/light1confirm";


WiFiClient wifiClient;
PubSubClient client(MQTT_SERVER, 1883, callback, wifiClient);

void setup() {
	//initialize the light as an output and set to LOW (off)
	pinMode(lightPin, OUTPUT);
	digitalWrite(lightPin, LOW);

	//start the serial line for debugging
	Serial.begin(115200);
	delay(100);


	//start wifi subsystem
	WiFi.begin(ssid, password);

	//attempt to connect to the WIFI network and then connect to the MQTT server
	reconnect();

	//wait a bit before starting the main loop
    	delay(2000);
}

//OTA

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop(){
        ArduinoOTA.handle();
	//reconnect if connection is lost
	if (!client.connected() && WiFi.status() == 3) {reconnect();}

	//maintain MQTT connection
	client.loop();

	//MUST delay to allow ESP8266 WIFI functions to run
	delay(10); 
}


//MQTT callback
void callback(char* topic, byte* payload, unsigned int length) {

	//convert topic to string to make it easier to work with
	String topicStr = topic; 

	//Print out some debugging info
	Serial.println("Callback update.");
	Serial.print("Topic: ");
	Serial.println(topicStr);

	//turn the light on if the payload is '1' and publish to the confirmation topic so the python script can log it
	if(payload[0] == '1'){
		digitalWrite(lightPin, HIGH);
		client.publish(lightConfirmTopic, "On");
	}

	//turn the light off if the payload is '0' and publish to the confirmation topic so the python script can log it
	else if (payload[0] == '0'){
		digitalWrite(lightPin, LOW);
		client.publish(lightConfirmTopic, "Off");
	}

}








//networking functions

void reconnect() {

	//attempt to connect to the wifi if connection is lost
	if(WiFi.status() != WL_CONNECTED){
		//debug printing
		Serial.print("Connecting to ");
		Serial.println(ssid);

		//loop while we wait for connection
		while (WiFi.status() != WL_CONNECTED) {
			delay(500);
			Serial.print(".");
		}

		//print out some more debug once connected
		Serial.println("");
		Serial.println("WiFi connected");  
		Serial.println("IP address: ");
		Serial.println(WiFi.localIP());
	}

	//make sure we are connected to WIFI before attemping to reconnect to MQTT
	if(WiFi.status() == WL_CONNECTED){
	// Loop until we're reconnected to the MQTT server
		while (!client.connected()) {
			Serial.print("Attempting MQTT connection...");

			// Generate client name based on MAC address and last 8 bits of microsecond counter
			String clientName;
			clientName += "esp8266-";
			uint8_t mac[6];
			WiFi.macAddress(mac);
			clientName += macToStr(mac);

			//if connected, subscribe to the topic(s) we want to be notified about
			if (client.connect((char*) clientName.c_str())) {
				Serial.print("\tMTQQ Connected");
				client.subscribe(lightTopic);
			}

			//otherwise print failed for debugging
			else{Serial.println("\tFailed."); abort();}
		}
	}
}

//generate unique name from MAC addr
String macToStr(const uint8_t* mac){

  String result;

  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);

    if (i < 5){
      result += ':';
    }
  }

  return result;
}








