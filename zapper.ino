#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

// Network name for Wifi Access Point
#define WIFI_SSID "Nerds Rule"

// Milliseconds to keep relay open
#define ZAP_TIME 1000

// Milliseconds to keep relay open
#define ZAP_INTERVAL 60000

// Pins connected to relay signal pins
#define RELAY_PIN_1 16
#define RELAY_PIN_2 5

// Misc Setup
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;
ESP8266WebServer server(80);

// Misc variables for keeping track of things
unsigned long lastControlZapStartTime;
unsigned long lastZapIntervalStartTime;
unsigned long lastIntervalClickTime;
boolean zapping1 = false;
boolean zapping2 = false;
int zapIntervalCounter = 1;

// Webserver response to http://192.168.0.1/
// Displays a simple page for user interaction
void handleRoot() {
  Serial.println("Enter handleRoot");
  // Super simple website for user interaction
  // Show a button that hits a URL
  String content = "<html><body style=\"background-color:black;\"><div style=\"width:100%;height:100%;display:flex;align-items:center;justify-content:center;\">"
                   "<button class=\"button\" style=\"height:400px;width:400px;background-color:red;color:white;font-size:4em;border-radius:50%;\" onclick=\"send()\">Zap</button>"
                   "<script>function send(){ var xhttp = new XMLHttpRequest();"
                   "xhttp.open(\"GET\", \"zap\", true);"
                   "xhttp.send();}</script>"
                   "</body></html>";
  server.send(200, "text/html", content);
}

// Webserver response to http://192.168.0.1/zap
// Turns on the zapper
void handleControlZap() {
  Serial.println("Enter handleRoot");
  Serial.println("starting zap");
  // Set the time for zapper auto-off
  lastControlZapStartTime = millis();
  // Activate relay
  zapping1 = true;
  setRelayState(RELAY_PIN_1, zapping1);
  // Redirect the user to the default landing page
  server.sendHeader("Location", "/");
  server.sendHeader("Cache-Control", "no-cache");
  server.send(301);
}

// Run constantly to turn of the control zapper when the time has expired
void checkControlTimer() {
  // If currently zapping
  if (zapping1) {
    // Check if they've had enough
    if (millis() - lastControlZapStartTime > ZAP_TIME) {
      // Disable relay
      zapping1 = false;
      setRelayState(RELAY_PIN_1, zapping1);
      Serial.println("ending controlled zap");
    }
  }
}

// Run constantly to control the interval zapper
void checkIntervalTimer() {
  //if not currently zapping
  if (!zapping2) {
    //check when next zap should happen
    long zapIntervalWait = ZAP_INTERVAL - ( 500 * zapIntervalCounter );
    //if it's time for a zap
    if (millis() - lastZapIntervalStartTime > zapIntervalWait) {
      zapping2 = true;
      setRelayState(RELAY_PIN_2, zapping2);
      Serial.println("start interval zap");
      Serial.print("counter: ");
      Serial.println(zapIntervalCounter);
    }
  }
  //if currently zapping
  else {
    //check when zap should stop
    if (millis() - lastZapIntervalStartTime > ZAP_INTERVAL) {
      lastZapIntervalStartTime = millis();
      //reset the counter after 5
      zapIntervalCounter %= 5;
      zapIntervalCounter++;
      zapping2 = false;
      setRelayState(RELAY_PIN_2, zapping2);
      Serial.println("stop interval zap");
    }
  }
}

// Hide the confusing fact that you have to set the LED_BUILTING pin to LOW to turn on the zapper
// Ain't noone got time for that
void setRelayState(int relay, bool zap) {
  digitalWrite(relay, !zap);
}

// Initial Arduino setup
void setup() {
  Serial.begin(115200);
  Serial.println("Booting Sketch...");

  // Initialize the relay pins as an output
  pinMode(RELAY_PIN_1, OUTPUT);
  pinMode(RELAY_PIN_2, OUTPUT);
  // Initialize the relays to the predetermined start state
  setRelayState(RELAY_PIN_1, zapping1);
  setRelayState(RELAY_PIN_2, zapping2);

  // Set up the Wifi to be an Access Point
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  // Start the access point with the predetermined SSID
  WiFi.softAP(WIFI_SSID);

  Serial.println("Starting captive webserver.");

  // If DNSServer is started with "*" for domain name, it will reply with
  // this device's IP to all DNS request. This makes it a 'captive portal'.
  // Most smartphones will automatically direct the user to this page when connected.
  dnsServer.start(DNS_PORT, "*", apIP);

  // This is the default endpoint returned when the user is directed to our IP
  server.on("/", HTTP_GET, handleRoot);
  // This is the endpoint that activates the zapper
  server.on("/zap", HTTP_GET, handleControlZap);
  // Reply to any unknown requests to our default page
  server.onNotFound(handleRoot);
  server.begin();

}

// Arduino main loop
void loop() {
  // Handle any waiting DNS requests
  dnsServer.processNextRequest();
  // Handle any URL requests
  server.handleClient();
  // Check whether zapper needs to be turned off
  checkControlTimer();
  // Check whether anything needs to be done with the interval timer
  checkIntervalTimer();
}
