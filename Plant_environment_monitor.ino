#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

// WiFi配置
const char* ssid = "TP-LINK_5569";
const char* password = "68686868";

// DHT传感器配置
#define DHTPIN 16     
#define DHTTYPE DHT11     
DHT dht(DHTPIN, DHTTYPE);

// 雨滴和光敏传感器配置
const int analogPin = A4;      
const int digitalPin = 17;     
const int ledPinRain = 13;     

const int photocellPin = A5;   
const int ledPinLight = 14;    

int rainAnalogState = 0;       
boolean rainDigitalState = 0;  
int lightAnalogState = 0;      

// 风扇配置
const int motorIn1 = 15;  
const int motorIn2 = 26;   

#define rank1 150
#define rank2 200
#define rank3 250

int fanSpeedState = 0;  

AsyncWebServer server(80);

// 函数声明
void updateFanSpeed();
void clockwise(int Speed);

String readDHTTemperature() {
  float t = dht.readTemperature();
  if (isnan(t)) {    
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
    if (t > 30) {
      fanSpeedState = 1; // 自动打开风扇，设为最低档
      updateFanSpeed();
    }
    Serial.println(t);
    return String(t);
  }
}

String readDHTHumidity() {
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
    Serial.println(h);
    return String(h);
  }
}

String readRainAnalog() {
  rainAnalogState = analogRead(analogPin);

  if (rainAnalogState < 1000) {
    return "Heavy Rain";
  } else if (rainAnalogState < 3000) {
    return "Drizzle";
  } else {
    return "No Rain";
  }
}

String readRainDigital() {
  rainDigitalState = digitalRead(digitalPin);
  return rainDigitalState == 1 ? "No" : "Yes";
}

String readLightAnalog() {
  lightAnalogState = analogRead(photocellPin);
  return lightAnalogState > 1500 ? "Low Light" : "Strong Light";
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
    .button {
      padding: 15px;
      font-size: 2.0rem;
      margin: 10px;
      cursor: pointer;
      border-radius: 5px;
    }
  </style>
</head>
<body>
  <h2>ESP32 Plant environment monitoring</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="dht-labels">Temperature</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p >
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Humidity</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">&percnt;</sup>
  </p >
  <p>
    <i class="fas fa-cloud-rain" style="color:#007BFF;"></i> 
    <span class="dht-labels">Rainfall</span>
    <span id="rainAnalog">%RAINANALOG%</span>
  </p >
  <p>
    <i class="fas fa-sun" style="color:#FF8C00;"></i> 
    <span class="dht-labels">Light Level</span>
    <span id="lightAnalog">%LIGHTANALOG%</span>
  </p >
  <p>
    <button class="button" onclick="fetch('/speed?value=1')">Speed 1</button>
    <button class="button" onclick="fetch('/speed?value=2')">Speed 2</button>
    <button class="button" onclick="fetch('/speed?value=3')">Speed 3</button>
  </p >
  <p>Current Fan Speed: <span id="fanSpeed">%FANSPEED%</span></p >
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("rainAnalog").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/rainAnalog", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("rainDigital").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/rainDigital", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("lightAnalog").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/lightAnalog", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("fanSpeed").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/fanSpeed", true);
  xhttp.send();
}, 10000 ) ;

</script>
</html>)rawliteral";

String processor(const String& var){
  if(var == "TEMPERATURE"){
    return readDHTTemperature();
  }
  else if(var == "HUMIDITY"){
    return readDHTHumidity();
  }
  else if(var == "RAINANALOG"){
    return readRainAnalog();
  }
  else if(var == "RAINDIGITAL"){
    return readRainDigital();
  }
  else if(var == "LIGHTANALOG"){
    return readLightAnalog();
  }
  else if(var == "FANSPEED"){
    return String(fanSpeedState);
  }
  return String();
}

void setup(){
  Serial.begin(115200);

  pinMode(ledPinRain, OUTPUT);     
  pinMode(digitalPin, INPUT);      
  pinMode(ledPinLight, OUTPUT);    

  pinMode(motorIn1, OUTPUT);
  pinMode(motorIn2, OUTPUT);

  analogSetWidth(10);              
  dht.begin();
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTTemperature().c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTHumidity().c_str());
  });
  server.on("/rainAnalog", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readRainAnalog().c_str());
  });
  server.on("/rainDigital", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readRainDigital().c_str());
  });
  server.on("/lightAnalog", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readLightAnalog().c_str());
  });
  server.on("/fanSpeed", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(fanSpeedState).c_str());
  });
  server.on("/speed", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("value")) {
      String speedValue = request->getParam("value")->value();
      fanSpeedState = speedValue.toInt();
      updateFanSpeed();
      request->send(200, "text/plain", "Speed set to " + speedValue);
    } else {
      request->send(400, "text/plain", "Bad Request");
    }
  });

  server.begin();
  Serial.println("Server started");
}

void loop(){
  // 雨滴传感器部分
  rainAnalogState = analogRead(analogPin);    
  rainDigitalState = digitalRead(digitalPin); 
  if(rainDigitalState == HIGH) 
  {
    digitalWrite(ledPinRain, LOW); 
  }
  else 
  {
    digitalWrite(ledPinRain, HIGH); 
  }

  // 光敏传感器部分
  lightAnalogState = analogRead(photocellPin);  
  if(lightAnalogState >= 400) 
  {
    digitalWrite(ledPinLight, HIGH);  
  }
  else 
  {
    digitalWrite(ledPinLight, LOW);   
  }

  delay(1000); 
}

void updateFanSpeed() {
  switch(fanSpeedState) {
    case 1:
      clockwise(rank1);
      break;
    case 2:
      clockwise(rank2);
      break;
    case 3:
      clockwise(rank3);
      break;
    default:
      clockwise(0);
  }
}

void clockwise(int Speed) {
  dacWrite(motorIn1, 0);
  dacWrite(motorIn2, Speed);
}