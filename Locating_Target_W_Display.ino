/*  This program uses load cells to locate the location of impact on a target.
 *  Authors: Josue Perez, Christopher Goodell, Patrick , Seth 
 *  Class: EENG 
 *  Assignment: Final Project
 */
 
//Libraries 
#include "HX711.h"
#include "Nextion.h"

//Function Declarations
void sensorPlot(void);
void impactDetect(void);
void impactProcess(void);
void impactConvert(float impactA, float impactB, float impactC);

//Display Component Declarations (page id, component id, component name)
//Pages
NexPage page1 = NexPage(1, 0, "page1");
//Buttons
NexButton b0 = NexButton(1,3,"b0");
//Text boxes
NexText t0 = NexText(1, 4, "t0");
NexText t2 = NexText(1,5,"t2");

char buffer[100] = {0};

//Register display objects to touch event list
NexTouch *nex_listen_list[] = 
{
  &page1,
  &b0,
  &t0,
  &t2,
  NULL
};

//Display components pop callback function declarations
void b0PopCallback(void *ptr);
void t0PopCallback(void *ptr);
void t2PopCallback(void *ptr);

//Loadcells data Pin Definitions
const int dataPin1 = 2, dataPin2 = 4, dataPin3 = 6;

//Loadcells Clock Pin Definitions
const int clockPin1 = 3, clockPin2 = 5, clockPin3 = 7;

//Sensor Declarations
HX711 sensor1, sensor2, sensor3;

//Load threshold
const int threshold = 200000;

//Sensor offsets. These are added to match the numbers observed from the test data and conditions.
long offSet1 = 126206;
long offSet2 = 132816;
long offSet3 = 29905;

//Impact detected indicator
bool impactDetected = false;

//Start indicator
bool Start = false;

//Arrays to store values above threshold
const int arraySize = 80;
long sensor1Array[arraySize] = {0};
long sensor2Array[arraySize] = {0};
long sensor3Array[arraySize] = {0};

//Counter to fill array sequentially
int arrayCount = 0;

//value of the detected impact
int sensor1Impact = 0;
int sensor2Impact = 0;
int sensor3Impact = 0;

//Sensor Baseline values
float baseA = 87555.0;
float baseB = 149612.0;
float baseC = 96102.0;

void setup() 
{
  //Initialize Serial only without display connected
  //Serial.begin(115200);
  //Serial.print("Initializing");
  
  //initialize Display
  nexInit();

  //Register pop event callback functions
  b0.attachPop(b0PopCallback, &b0);
  t0.attachPop(t0PopCallback, &t0);
  t2.attachPop(t2PopCallback, &t2);

  //Initialize Sensors
  sensor1.begin(dataPin1, clockPin1);
  sensor2.begin(dataPin2, clockPin2);
  sensor3.begin(dataPin3, clockPin3);

  while(!sensor1.is_ready() && !sensor2.is_ready() && !sensor3.is_ready())
  { }

  page1.show();

}

void loop() 
{
  impactDetect();
  impactProcess();
  //sensorPlot();
  nexLoop(nex_listen_list);
}

///////////////////////////////////////////////////////////////////////
//Function Definitions

//Function to plot the sensor input in the serial plotter
void sensorPlot(void)
{
  if (sensor1.is_ready() && sensor2.is_ready() && sensor3.is_ready()) {
    //read and print sensor 1
    Serial.print("Sensor 1: ");
    Serial.print(sensor1.read());
    Serial.print(",");
    
    //read and print sensor 2
    Serial.print("Sensor 2: ");
    Serial.print(sensor2.read());
    Serial.print(",");
    
    //read and print sensor 3
    Serial.print("Sensor 3: ");
    Serial.println(sensor3.read());
  }
  return;
}

//Detect impact exceeding threshold
void impactDetect(void)
{
  long sensorValue1;
  long sensorValue2;
  long sensorValue3;
  if (sensor1.is_ready() && sensor2.is_ready() && sensor3.is_ready() && !impactDetected && Start) 
  {
    //Read Sensors
    sensorValue1 = sensor1.read()+offSet1;
    sensorValue2 = sensor2.read()+offSet2;
    sensorValue3 = sensor3.read()+offSet3;

/*  Debugging serial code
    Serial.print("Sensor 1: ");
    Serial.print(sensorValue1);
    Serial.print(" Sensor 2: ");
    Serial.print(sensorValue2);
    Serial.print(" Sensor 3: ");
    Serial.println(sensorValue3);
*/
    if(sensorValue1 > threshold || sensorValue2 > threshold || sensorValue3 > threshold)
    {
      while(sensorValue1 > threshold || sensorValue2 > threshold || sensorValue3 > threshold)
      {
        sensor1Array[arrayCount] = sensorValue1;
        Serial.print("Sensor 1 threshold exceeded: ");
        Serial.println(sensorValue1);
    
        sensor2Array[arrayCount] = sensorValue2;
        Serial.print("Sensor 2 threshold exceeded: ");
        Serial.println(sensorValue2);
    
        sensor3Array[arrayCount] = sensorValue3;
        Serial.print("Sensor 3 threshold exceeded: ");
        Serial.println(sensorValue3);
        arrayCount++;
         
        if(arrayCount >= arraySize)
        {
          impactDetected = true;
          arrayCount = 0;
          Serial.println("Impact Detected!");
          return;
        }
    
    sensorValue1 = sensor1.read()+offSet1;
    sensorValue2 = sensor2.read()+offSet2;
    sensorValue3 = sensor3.read()+offSet3;
      }

      impactDetected = true;
      arrayCount = 0;
      Serial.println("Impact Detected!");
    }
  }
  return;
}

//Process data to find impact value
void impactProcess(void)
{
  if(impactDetected){
    sensor1Impact = sensor1Array[0];
    sensor2Impact = sensor2Array[0];
    sensor3Impact = sensor3Array[0];
    for(int n = 0; n < arraySize; n++){
      if(sensor1Impact < sensor1Array[n]){
        sensor1Impact = sensor1Array[n];
      }
      if(sensor2Impact < sensor2Array[n]){
        sensor2Impact = sensor2Array[n];
      }
      if(sensor3Impact < sensor3Array[n]){
        sensor3Impact = sensor3Array[n];
      }
    }
    
    Serial.print("Sensor1: ");
    Serial.print(sensor1Impact);
    Serial.print(", ");
    Serial.print("Sensor2: ");
    Serial.print(sensor2Impact);
    Serial.print(", ");
    Serial.print("Sensor3: ");
    Serial.println(sensor3Impact);
    impactDetected = false;
    Start = !Start;

    memset(sensor1Array, 0, sizeof(sensor1Array));
    memset(sensor2Array, 0, sizeof(sensor2Array));
    memset(sensor3Array, 0, sizeof(sensor3Array));
    
    impactConvert(float(sensor1Impact), float(sensor2Impact), float(sensor3Impact));
  }
  return;
}

//Convert impact value to location
void impactConvert(float impactA, float impactB, float impactC)
{
  //remove baseline from readings
  float trueA = impactA - baseA;
  float trueB = impactB - baseB;
  float trueC = impactC - baseC;
  
  //find total of the sensor values
  float total = trueA + trueB + trueC;
  
  //find ratio of the reading to the total
  float ratioA = trueA / total;
  float ratioB = trueB / total;
  float ratioC = trueC / total;

  //print ratios
  Serial.print("ratioA: ");
  Serial.println(ratioA);
  Serial.print("ratioB: ");
  Serial.println(ratioB);
  Serial.print("ratioC: ");
  Serial.println(ratioC);
  
  //compare ratio to database and print approximate location
   if ( (abs(  0.579 -ratioA) + abs( 0.412 -ratioB) + abs( 0.008 -ratioC))<  0.05  )  {Serial.println("X=  -4  , Y=  14  ");}
   else if ( (abs( 0.537 -ratioA) + abs( 0.451 -ratioB) + abs( 0.012 -ratioC))<  0.05  )  {Serial.println("X=  -2  , Y=  14  ");}
   else if ( (abs( 0.503 -ratioA) + abs( 0.493 -ratioB) + abs( 0.004 -ratioC))<  0.05  )  {Serial.println("X=  0 , Y=  14  ");}
   else if ( (abs( 0.467 -ratioA) + abs( 0.532 -ratioB) + abs( 0.002 -ratioC))<  0.05  )  {Serial.println("X=  2 , Y=  14  ");}
   else if ( (abs( 0.428 -ratioA) + abs( 0.572 -ratioB) + abs( 0.000 -ratioC))<  0.05  )  {Serial.println("X=  4 , Y=  14  ");}
   else if ( (abs( 0.634 -ratioA) + abs( 0.308 -ratioB) + abs( 0.058 -ratioC))<  0.05  )  {Serial.println("X=  -8  , Y=  12  ");}
   else if ( (abs( 0.599 -ratioA) + abs( 0.353 -ratioB) + abs( 0.048 -ratioC))<  0.05  )  {Serial.println("X=  -6  , Y=  12  ");}
   else if ( (abs( 0.562 -ratioA) + abs( 0.390 -ratioB) + abs( 0.048 -ratioC))<  0.05  )  {Serial.println("X=  -4  , Y=  12  ");}
   else if ( (abs( 0.521 -ratioA) + abs( 0.435 -ratioB) + abs( 0.044 -ratioC))<  0.05  )  {Serial.println("X=  -2  , Y=  12  ");}
   else if ( (abs( 0.478 -ratioA) + abs( 0.471 -ratioB) + abs( 0.051 -ratioC))<  0.05  )  {Serial.println("X=  0 , Y=  12  ");}
   else if ( (abs( 0.434 -ratioA) + abs( 0.517 -ratioB) + abs( 0.049 -ratioC))<  0.05  )  {Serial.println("X=  2 , Y=  12  ");}
   else if ( (abs( 0.400 -ratioA) + abs( 0.553 -ratioB) + abs( 0.046 -ratioC))<  0.05  )  {Serial.println("X=  4 , Y=  12  ");}
   else if ( (abs( 0.360 -ratioA) + abs( 0.592 -ratioB) + abs( 0.048 -ratioC))<  0.05  )  {Serial.println("X=  6 , Y=  12  ");}
   else if ( (abs( 0.323 -ratioA) + abs( 0.631 -ratioB) + abs( 0.045 -ratioC))<  0.05  )  {Serial.println("X=  8 , Y=  12  ");}
   else if ( (abs( 0.650 -ratioA) + abs( 0.245 -ratioB) + abs( 0.104 -ratioC))<  0.05  )  {Serial.println("X=  -10 , Y=  10  ");}
   else if ( (abs( 0.607 -ratioA) + abs( 0.291 -ratioB) + abs( 0.102 -ratioC))<  0.05  )  {Serial.println("X=  -8  , Y=  10  ");}
   else if ( (abs( 0.571 -ratioA) + abs( 0.332 -ratioB) + abs( 0.097 -ratioC))<  0.05  )  {Serial.println("X=  -6  , Y=  10  ");}
   else if ( (abs( 0.534 -ratioA) + abs( 0.376 -ratioB) + abs( 0.090 -ratioC))<  0.05  )  {Serial.println("X=  -4  , Y=  10  ");}
   else if ( (abs( 0.491 -ratioA) + abs( 0.411 -ratioB) + abs( 0.098 -ratioC))<  0.05  )  {Serial.println("X=  -2  , Y=  10  ");}
   else if ( (abs( 0.452 -ratioA) + abs( 0.454 -ratioB) + abs( 0.094 -ratioC))<  0.05  )  {Serial.println("X=  0 , Y=  10  ");}
   else if ( (abs( 0.414 -ratioA) + abs( 0.495 -ratioB) + abs( 0.091 -ratioC))<  0.05  )  {Serial.println("X=  2 , Y=  10  ");}
   else if ( (abs( 0.375 -ratioA) + abs( 0.531 -ratioB) + abs( 0.094 -ratioC))<  0.05  )  {Serial.println("X=  4 , Y=  10  ");}
   else if ( (abs( 0.339 -ratioA) + abs( 0.570 -ratioB) + abs( 0.091 -ratioC))<  0.05  )  {Serial.println("X=  6 , Y=  10  ");}
   else if ( (abs( 0.299 -ratioA) + abs( 0.615 -ratioB) + abs( 0.086 -ratioC))<  0.05  )  {Serial.println("X=  8 , Y=  10  ");}
   else if ( (abs( 0.258 -ratioA) + abs( 0.654 -ratioB) + abs( 0.087 -ratioC))<  0.05  )  {Serial.println("X=  10  , Y=  10  ");}
   else if ( (abs( 0.662 -ratioA) + abs( 0.189 -ratioB) + abs( 0.149 -ratioC))<  0.05  )  {Serial.println("X=  -12 , Y=  8 ");}
   else if ( (abs( 0.625 -ratioA) + abs( 0.226 -ratioB) + abs( 0.149 -ratioC))<  0.05  )  {Serial.println("X=  -10 , Y=  8 ");}
   else if ( (abs( 0.589 -ratioA) + abs( 0.265 -ratioB) + abs( 0.145 -ratioC))<  0.05  )  {Serial.println("X=  -8  , Y=  8 ");}
   else if ( (abs( 0.546 -ratioA) + abs( 0.308 -ratioB) + abs( 0.146 -ratioC))<  0.05  )  {Serial.println("X=  -6  , Y=  8 ");}
   else if ( (abs( 0.510 -ratioA) + abs( 0.349 -ratioB) + abs( 0.141 -ratioC))<  0.05  )  {Serial.println("X=  -4  , Y=  8 ");}
   else if ( (abs( 0.470 -ratioA) + abs( 0.391 -ratioB) + abs( 0.139 -ratioC))<  0.05  )  {Serial.println("X=  -2  , Y=  8 ");}
   else if ( (abs( 0.430 -ratioA) + abs( 0.433 -ratioB) + abs( 0.137 -ratioC))<  0.05  )  {Serial.println("X=  0 , Y=  8 ");}
   else if ( (abs( 0.392 -ratioA) + abs( 0.474 -ratioB) + abs( 0.134 -ratioC))<  0.05  )  {Serial.println("X=  2 , Y=  8 ");}
   else if ( (abs( 0.350 -ratioA) + abs( 0.512 -ratioB) + abs( 0.138 -ratioC))<  0.05  )  {Serial.println("X=  4 , Y=  8 ");}
   else if ( (abs( 0.312 -ratioA) + abs( 0.552 -ratioB) + abs( 0.136 -ratioC))<  0.05  )  {Serial.println("X=  6 , Y=  8 ");}
   else if ( (abs( 0.273 -ratioA) + abs( 0.592 -ratioB) + abs( 0.134 -ratioC))<  0.05  )  {Serial.println("X=  8 , Y=  8 ");}
   else if ( (abs( 0.226 -ratioA) + abs( 0.626 -ratioB) + abs( 0.148 -ratioC))<  0.05  )  {Serial.println("X=  10  , Y=  8 ");}
   else if ( (abs( 0.187 -ratioA) + abs( 0.674 -ratioB) + abs( 0.139 -ratioC))<  0.05  )  {Serial.println("X=  12  , Y=  8 ");}
   else if ( (abs( 0.641 -ratioA) + abs( 0.164 -ratioB) + abs( 0.195 -ratioC))<  0.05  )  {Serial.println("X=  -12 , Y=  6 ");}
   else if ( (abs( 0.598 -ratioA) + abs( 0.206 -ratioB) + abs( 0.196 -ratioC))<  0.05  )  {Serial.println("X=  -10 , Y=  6 ");}
   else if ( (abs( 0.563 -ratioA) + abs( 0.244 -ratioB) + abs( 0.193 -ratioC))<  0.05  )  {Serial.println("X=  -8  , Y=  6 ");}
   else if ( (abs( 0.526 -ratioA) + abs( 0.283 -ratioB) + abs( 0.191 -ratioC))<  0.05  )  {Serial.println("X=  -6  , Y=  6 ");}
   else if ( (abs( 0.482 -ratioA) + abs( 0.326 -ratioB) + abs( 0.192 -ratioC))<  0.05  )  {Serial.println("X=  -4  , Y=  6 ");}
   else if ( (abs( 0.444 -ratioA) + abs( 0.368 -ratioB) + abs( 0.188 -ratioC))<  0.05  )  {Serial.println("X=  -2  , Y=  6 ");}
   else if ( (abs( 0.401 -ratioA) + abs( 0.410 -ratioB) + abs( 0.189 -ratioC))<  0.05  )  {Serial.println("X=  0 , Y=  6 ");}
   else if ( (abs( 0.363 -ratioA) + abs( 0.449 -ratioB) + abs( 0.188 -ratioC))<  0.05  )  {Serial.println("X=  2 , Y=  6 ");}
   else if ( (abs( 0.327 -ratioA) + abs( 0.487 -ratioB) + abs( 0.186 -ratioC))<  0.05  )  {Serial.println("X=  4 , Y=  6 ");}
   else if ( (abs( 0.284 -ratioA) + abs( 0.531 -ratioB) + abs( 0.185 -ratioC))<  0.05  )  {Serial.println("X=  6 , Y=  6 ");}
   else if ( (abs( 0.244 -ratioA) + abs( 0.569 -ratioB) + abs( 0.187 -ratioC))<  0.05  )  {Serial.println("X=  8 , Y=  6 ");}
   else if ( (abs( 0.205 -ratioA) + abs( 0.609 -ratioB) + abs( 0.186 -ratioC))<  0.05  )  {Serial.println("X=  10  , Y=  6 ");}
   else if ( (abs( 0.169 -ratioA) + abs( 0.651 -ratioB) + abs( 0.180 -ratioC))<  0.05  )  {Serial.println("X=  12  , Y=  6 ");}
   else if ( (abs( 0.653 -ratioA) + abs( 0.098 -ratioB) + abs( 0.249 -ratioC))<  0.05  )  {Serial.println("X=  -14 , Y=  4 ");}
   else if ( (abs( 0.617 -ratioA) + abs( 0.140 -ratioB) + abs( 0.243 -ratioC))<  0.05  )  {Serial.println("X=  -12 , Y=  4 ");}
   else if ( (abs( 0.576 -ratioA) + abs( 0.183 -ratioB) + abs( 0.241 -ratioC))<  0.05  )  {Serial.println("X=  -10 , Y=  4 ");}
   else if ( (abs( 0.536 -ratioA) + abs( 0.225 -ratioB) + abs( 0.239 -ratioC))<  0.05  )  {Serial.println("X=  -8  , Y=  4 ");}
   else if ( (abs( 0.499 -ratioA) + abs( 0.265 -ratioB) + abs( 0.236 -ratioC))<  0.05  )  {Serial.println("X=  -6  , Y=  4 ");}
   else if ( (abs( 0.458 -ratioA) + abs( 0.306 -ratioB) + abs( 0.236 -ratioC))<  0.05  )  {Serial.println("X=  -4  , Y=  4 ");}
   else if ( (abs( 0.417 -ratioA) + abs( 0.345 -ratioB) + abs( 0.238 -ratioC))<  0.05  )  {Serial.println("X=  -2  , Y=  4 ");}
   else if ( (abs( 0.377 -ratioA) + abs( 0.389 -ratioB) + abs( 0.235 -ratioC))<  0.05  )  {Serial.println("X=  0 , Y=  4 ");}
   else if ( (abs( 0.340 -ratioA) + abs( 0.427 -ratioB) + abs( 0.232 -ratioC))<  0.05  )  {Serial.println("X=  2 , Y=  4 ");}
   else if ( (abs( 0.299 -ratioA) + abs( 0.469 -ratioB) + abs( 0.232 -ratioC))<  0.05  )  {Serial.println("X=  4 , Y=  4 ");}
   else if ( (abs( 0.260 -ratioA) + abs( 0.506 -ratioB) + abs( 0.234 -ratioC))<  0.05  )  {Serial.println("X=  6 , Y=  4 ");}
   else if ( (abs( 0.221 -ratioA) + abs( 0.548 -ratioB) + abs( 0.231 -ratioC))<  0.05  )  {Serial.println("X=  8 , Y=  4 ");}
   else if ( (abs( 0.185 -ratioA) + abs( 0.591 -ratioB) + abs( 0.225 -ratioC))<  0.05  )  {Serial.println("X=  10  , Y=  4 ");}
   else if ( (abs( 0.145 -ratioA) + abs( 0.629 -ratioB) + abs( 0.225 -ratioC))<  0.05  )  {Serial.println("X=  12  , Y=  4 ");}
   else if ( (abs( 0.107 -ratioA) + abs( 0.667 -ratioB) + abs( 0.226 -ratioC))<  0.05  )  {Serial.println("X=  14  , Y=  4 ");}
   else if ( (abs( 0.251 -ratioA) + abs( 0.423 -ratioB) + abs( 0.326 -ratioC))<  0.05  )  {Serial.println("X=  4 , Y=  0 ");}
   else if ( (abs( 0.276 -ratioA) + abs( 0.446 -ratioB) + abs( 0.278 -ratioC))<  0.05  )  {Serial.println("X=  4 , Y=  2 ");}
   else if ( (abs( 0.214 -ratioA) + abs( 0.465 -ratioB) + abs( 0.321 -ratioC))<  0.05  )  {Serial.println("X=  6 , Y=  0 ");}
   else if ( (abs( 0.292 -ratioA) + abs( 0.382 -ratioB) + abs( 0.327 -ratioC))<  0.05  )  {Serial.println("X=  2 , Y=  0 ");}
   else if ( (abs( 0.230 -ratioA) + abs( 0.401 -ratioB) + abs( 0.368 -ratioC))<  0.05  )  {Serial.println("X=  4 , Y=  -2  ");}
   else if ( (abs( 0.239 -ratioA) + abs( 0.483 -ratioB) + abs( 0.277 -ratioC))<  0.05  )  {Serial.println("X=  6 , Y=  2 ");}
   else if ( (abs( 0.314 -ratioA) + abs( 0.406 -ratioB) + abs( 0.280 -ratioC))<  0.05  )  {Serial.println("X=  2 , Y=  2 ");}
   else if ( (abs( 0.268 -ratioA) + abs( 0.363 -ratioB) + abs( 0.369 -ratioC))<  0.05  )  {Serial.println("X=  2 , Y=  -2  ");}
   else if ( (abs( 0.189 -ratioA) + abs( 0.443 -ratioB) + abs( 0.368 -ratioC))<  0.05  )  {Serial.println("X=  6 , Y=  -2  ");}
   else if ( (abs( 0.177 -ratioA) + abs( 0.503 -ratioB) + abs( 0.320 -ratioC))<  0.05  )  {Serial.println("X=  8 , Y=  0 ");}
   else if ( (abs( 0.330 -ratioA) + abs( 0.344 -ratioB) + abs( 0.326 -ratioC))<  0.05  )  {Serial.println("X=  0 , Y=  0 ");}
   else if ( (abs( 0.353 -ratioA) + abs( 0.366 -ratioB) + abs( 0.281 -ratioC))<  0.05  )  {Serial.println("X=  0 , Y=  2 ");}
   else if ( (abs( 0.308 -ratioA) + abs( 0.323 -ratioB) + abs( 0.369 -ratioC))<  0.05  )  {Serial.println("X=  0 , Y=  -2  ");}
   else if ( (abs( 0.198 -ratioA) + abs( 0.527 -ratioB) + abs( 0.275 -ratioC))<  0.05  )  {Serial.println("X=  8 , Y=  2 ");}
   else if ( (abs( 0.150 -ratioA) + abs( 0.483 -ratioB) + abs( 0.367 -ratioC))<  0.05  )  {Serial.println("X=  8 , Y=  -2  ");}
   else if ( (abs( 0.136 -ratioA) + abs( 0.545 -ratioB) + abs( 0.320 -ratioC))<  0.05  )  {Serial.println("X=  10  , Y=  0 ");}
   else if ( (abs( 0.370 -ratioA) + abs( 0.301 -ratioB) + abs( 0.329 -ratioC))<  0.05  )  {Serial.println("X=  -2  , Y=  0 ");}
   else if ( (abs( 0.393 -ratioA) + abs( 0.322 -ratioB) + abs( 0.285 -ratioC))<  0.05  )  {Serial.println("X=  -2  , Y=  2 ");}
   else if ( (abs( 0.158 -ratioA) + abs( 0.566 -ratioB) + abs( 0.276 -ratioC))<  0.05  )  {Serial.println("X=  10  , Y=  2 ");}
   else if ( (abs( 0.114 -ratioA) + abs( 0.521 -ratioB) + abs( 0.365 -ratioC))<  0.05  )  {Serial.println("X=  10  , Y=  -2  ");}
   else if ( (abs( 0.351 -ratioA) + abs( 0.280 -ratioB) + abs( 0.370 -ratioC))<  0.05  )  {Serial.println("X=  -2  , Y=  -2  ");}
   else if ( (abs( 0.097 -ratioA) + abs( 0.585 -ratioB) + abs( 0.318 -ratioC))<  0.05  )  {Serial.println("X=  12  , Y=  0 ");}
   else if ( (abs( 0.409 -ratioA) + abs( 0.259 -ratioB) + abs( 0.331 -ratioC))<  0.05  )  {Serial.println("X=  -4  , Y=  0 ");}
   else if ( (abs( 0.433 -ratioA) + abs( 0.283 -ratioB) + abs( 0.283 -ratioC))<  0.05  )  {Serial.println("X=  -4  , Y=  2 ");}
   else if ( (abs( 0.382 -ratioA) + abs( 0.244 -ratioB) + abs( 0.374 -ratioC))<  0.05  )  {Serial.println("X=  -4  , Y=  -2  ");}
   else if ( (abs( 0.120 -ratioA) + abs( 0.608 -ratioB) + abs( 0.272 -ratioC))<  0.05  )  {Serial.println("X=  12  , Y=  2 ");}
   else if ( (abs( 0.071 -ratioA) + abs( 0.564 -ratioB) + abs( 0.365 -ratioC))<  0.05  )  {Serial.println("X=  12  , Y=  -2  ");}
   else if ( (abs( 0.058 -ratioA) + abs( 0.625 -ratioB) + abs( 0.318 -ratioC))<  0.05  )  {Serial.println("X=  14  , Y=  0 ");}
   else if ( (abs( 0.450 -ratioA) + abs( 0.218 -ratioB) + abs( 0.331 -ratioC))<  0.05  )  {Serial.println("X=  -6  , Y=  0 ");}
   else if ( (abs( 0.470 -ratioA) + abs( 0.241 -ratioB) + abs( 0.289 -ratioC))<  0.05  )  {Serial.println("X=  -6  , Y=  2 ");}
   else if ( (abs( 0.081 -ratioA) + abs( 0.645 -ratioB) + abs( 0.274 -ratioC))<  0.05  )  {Serial.println("X=  14  , Y=  2 ");}
   else if ( (abs( 0.034 -ratioA) + abs( 0.605 -ratioB) + abs( 0.361 -ratioC))<  0.05  )  {Serial.println("X=  14  , Y=  -2  ");}
   else if ( (abs( 0.426 -ratioA) + abs( 0.201 -ratioB) + abs( 0.373 -ratioC))<  0.05  )  {Serial.println("X=  -6  , Y=  -2  ");}
   else if ( (abs( 0.489 -ratioA) + abs( 0.179 -ratioB) + abs( 0.332 -ratioC))<  0.05  )  {Serial.println("X=  -8  , Y=  0 ");}
   else if ( (abs( 0.510 -ratioA) + abs( 0.200 -ratioB) + abs( 0.289 -ratioC))<  0.05  )  {Serial.println("X=  -8  , Y=  2 ");}
   else if ( (abs( 0.463 -ratioA) + abs( 0.159 -ratioB) + abs( 0.378 -ratioC))<  0.05  )  {Serial.println("X=  -8  , Y=  -2  ");}
   else if ( (abs( 0.532 -ratioA) + abs( 0.133 -ratioB) + abs( 0.335 -ratioC))<  0.05  )  {Serial.println("X=  -10 , Y=  0 ");}
   else if ( (abs( 0.552 -ratioA) + abs( 0.159 -ratioB) + abs( 0.289 -ratioC))<  0.05  )  {Serial.println("X=  -10 , Y=  2 ");}
   else if ( (abs( 0.506 -ratioA) + abs( 0.114 -ratioB) + abs( 0.381 -ratioC))<  0.05  )  {Serial.println("X=  -10 , Y=  -2  ");}
   else if ( (abs( 0.570 -ratioA) + abs( 0.094 -ratioB) + abs( 0.335 -ratioC))<  0.05  )  {Serial.println("X=  -12 , Y=  0 ");}
   else if ( (abs( 0.594 -ratioA) + abs( 0.119 -ratioB) + abs( 0.287 -ratioC))<  0.05  )  {Serial.println("X=  -12 , Y=  2 ");}
   else if ( (abs( 0.541 -ratioA) + abs( 0.076 -ratioB) + abs( 0.384 -ratioC))<  0.05  )  {Serial.println("X=  -12 , Y=  -2  ");}
   else if ( (abs( 0.611 -ratioA) + abs( 0.054 -ratioB) + abs( 0.335 -ratioC))<  0.05  )  {Serial.println("X=  -14 , Y=  0 ");}
   else if ( (abs( 0.633 -ratioA) + abs( 0.075 -ratioB) + abs( 0.292 -ratioC))<  0.05  )  {Serial.println("X=  -14 , Y=  2 ");}
   else if ( (abs( 0.587 -ratioA) + abs( 0.030 -ratioB) + abs( 0.383 -ratioC))<  0.05  )  {Serial.println("X=  -14 , Y=  -2  ");}
   else if ( (abs( 0.521 -ratioA) + abs( 0.045 -ratioB) + abs( 0.434 -ratioC))<  0.05  )  {Serial.println("X=  -12 , Y=  -4  ");}
   else if ( (abs( 0.478 -ratioA) + abs( 0.093 -ratioB) + abs( 0.428 -ratioC))<  0.05  )  {Serial.println("X=  -10 , Y=  -4  ");}
   else if ( (abs( 0.436 -ratioA) + abs( 0.135 -ratioB) + abs( 0.429 -ratioC))<  0.05  )  {Serial.println("X=  -8  , Y=  -4  ");}
   else if ( (abs( 0.400 -ratioA) + abs( 0.179 -ratioB) + abs( 0.421 -ratioC))<  0.05  )  {Serial.println("X=  -6  , Y=  -4  ");}
   else if ( (abs( 0.362 -ratioA) + abs( 0.219 -ratioB) + abs( 0.419 -ratioC))<  0.05  )  {Serial.println("X=  -4  , Y=  -4  ");}
   else if ( (abs( 0.321 -ratioA) + abs( 0.259 -ratioB) + abs( 0.420 -ratioC))<  0.05  )  {Serial.println("X=  -2  , Y=  -4  ");}
   else if ( (abs( 0.280 -ratioA) + abs( 0.301 -ratioB) + abs( 0.419 -ratioC))<  0.05  )  {Serial.println("X=  0 , Y=  -4  ");}
   else if ( (abs( 0.242 -ratioA) + abs( 0.340 -ratioB) + abs( 0.418 -ratioC))<  0.05  )  {Serial.println("X=  2 , Y=  -4  ");}
   else if ( (abs( 0.205 -ratioA) + abs( 0.377 -ratioB) + abs( 0.418 -ratioC))<  0.05  )  {Serial.println("X=  4 , Y=  -4  ");}
   else if ( (abs( 0.165 -ratioA) + abs( 0.419 -ratioB) + abs( 0.416 -ratioC))<  0.05  )  {Serial.println("X=  6 , Y=  -4  ");}
   else if ( (abs( 0.125 -ratioA) + abs( 0.458 -ratioB) + abs( 0.417 -ratioC))<  0.05  )  {Serial.println("X=  8 , Y=  -4  ");}
   else if ( (abs( 0.087 -ratioA) + abs( 0.496 -ratioB) + abs( 0.417 -ratioC))<  0.05  )  {Serial.println("X=  10  , Y=  -4  ");}
   else if ( (abs( 0.048 -ratioA) + abs( 0.539 -ratioB) + abs( 0.413 -ratioC))<  0.05  )  {Serial.println("X=  12  , Y=  -4  ");}
   else if ( (abs( 0.496 -ratioA) + abs( 0.024 -ratioB) + abs( 0.480 -ratioC))<  0.05  )  {Serial.println("X=  -12 , Y=  -6  ");}
   else if ( (abs( 0.455 -ratioA) + abs( 0.066 -ratioB) + abs( 0.478 -ratioC))<  0.05  )  {Serial.println("X=  -10 , Y=  -6  ");}
   else if ( (abs( 0.414 -ratioA) + abs( 0.110 -ratioB) + abs( 0.476 -ratioC))<  0.05  )  {Serial.println("X=  -8  , Y=  -6  ");}
   else if ( (abs( 0.373 -ratioA) + abs( 0.153 -ratioB) + abs( 0.474 -ratioC))<  0.05  )  {Serial.println("X=  -6  , Y=  -6  ");}
   else if ( (abs( 0.336 -ratioA) + abs( 0.195 -ratioB) + abs( 0.470 -ratioC))<  0.05  )  {Serial.println("X=  -4  , Y=  -6  ");}
   else if ( (abs( 0.295 -ratioA) + abs( 0.235 -ratioB) + abs( 0.469 -ratioC))<  0.05  )  {Serial.println("X=  -2  , Y=  -6  ");}
   else if ( (abs( 0.253 -ratioA) + abs( 0.281 -ratioB) + abs( 0.466 -ratioC))<  0.05  )  {Serial.println("X=  0 , Y=  -6  ");}
   else if ( (abs( 0.219 -ratioA) + abs( 0.318 -ratioB) + abs( 0.463 -ratioC))<  0.05  )  {Serial.println("X=  2 , Y=  -6  ");}
   else if ( (abs( 0.183 -ratioA) + abs( 0.355 -ratioB) + abs( 0.461 -ratioC))<  0.05  )  {Serial.println("X=  4 , Y=  -6  ");}
   else if ( (abs( 0.142 -ratioA) + abs( 0.398 -ratioB) + abs( 0.461 -ratioC))<  0.05  )  {Serial.println("X=  6 , Y=  -6  ");}
   else if ( (abs( 0.103 -ratioA) + abs( 0.437 -ratioB) + abs( 0.461 -ratioC))<  0.05  )  {Serial.println("X=  8 , Y=  -6  ");}
   else if ( (abs( 0.061 -ratioA) + abs( 0.481 -ratioB) + abs( 0.458 -ratioC))<  0.05  )  {Serial.println("X=  10  , Y=  -6  ");}
   else if ( (abs( 0.022 -ratioA) + abs( 0.520 -ratioB) + abs( 0.458 -ratioC))<  0.05  )  {Serial.println("X=  12  , Y=  -6  ");}
   else if ( (abs( 0.475 -ratioA) + abs( 0.000 -ratioB) + abs( 0.525 -ratioC))<  0.05  )  {Serial.println("X=  -12 , Y=  -8  ");}
   else if ( (abs( 0.434 -ratioA) + abs( 0.046 -ratioB) + abs( 0.520 -ratioC))<  0.05  )  {Serial.println("X=  -10 , Y=  -8  ");}
   else if ( (abs( 0.392 -ratioA) + abs( 0.090 -ratioB) + abs( 0.518 -ratioC))<  0.05  )  {Serial.println("X=  -8  , Y=  -8  ");}
   else if ( (abs( 0.350 -ratioA) + abs( 0.130 -ratioB) + abs( 0.520 -ratioC))<  0.05  )  {Serial.println("X=  -6  , Y=  -8  ");}
   else if ( (abs( 0.311 -ratioA) + abs( 0.170 -ratioB) + abs( 0.519 -ratioC))<  0.05  )  {Serial.println("X=  -4  , Y=  -8  ");}
   else if ( (abs( 0.267 -ratioA) + abs( 0.214 -ratioB) + abs( 0.519 -ratioC))<  0.05  )  {Serial.println("X=  -2  , Y=  -8  ");}
   else if ( (abs( 0.233 -ratioA) + abs( 0.256 -ratioB) + abs( 0.511 -ratioC))<  0.05  )  {Serial.println("X=  0 , Y=  -8  ");}
   else if ( (abs( 0.194 -ratioA) + abs( 0.301 -ratioB) + abs( 0.506 -ratioC))<  0.05  )  {Serial.println("X=  2 , Y=  -8  ");}
   else if ( (abs( 0.157 -ratioA) + abs( 0.335 -ratioB) + abs( 0.508 -ratioC))<  0.05  )  {Serial.println("X=  4 , Y=  -8  ");}
   else if ( (abs( 0.114 -ratioA) + abs( 0.377 -ratioB) + abs( 0.509 -ratioC))<  0.05  )  {Serial.println("X=  6 , Y=  -8  ");}
   else if ( (abs( 0.076 -ratioA) + abs( 0.417 -ratioB) + abs( 0.507 -ratioC))<  0.05  )  {Serial.println("X=  8 , Y=  -8  ");}
   else if ( (abs( 0.040 -ratioA) + abs( 0.456 -ratioB) + abs( 0.504 -ratioC))<  0.05  )  {Serial.println("X=  10  , Y=  -8  ");}
   else if ( (abs( 0.000 -ratioA) + abs( 0.498 -ratioB) + abs( 0.502 -ratioC))<  0.05  )  {Serial.println("X=  12  , Y=  -8  ");}
   else if ( (abs( 0.408 -ratioA) + abs( 0.022 -ratioB) + abs( 0.570 -ratioC))<  0.05  )  {Serial.println("X=  -10 , Y=  -10 ");}
   else if ( (abs( 0.365 -ratioA) + abs( 0.066 -ratioB) + abs( 0.569 -ratioC))<  0.05  )  {Serial.println("X=  -8  , Y=  -10 ");}
   else if ( (abs( 0.328 -ratioA) + abs( 0.102 -ratioB) + abs( 0.570 -ratioC))<  0.05  )  {Serial.println("X=  -6  , Y=  -10 ");}
   else if ( (abs( 0.286 -ratioA) + abs( 0.147 -ratioB) + abs( 0.567 -ratioC))<  0.05  )  {Serial.println("X=  -4  , Y=  -10 ");}
   else if ( (abs( 0.219 -ratioA) + abs( 0.198 -ratioB) + abs( 0.583 -ratioC))<  0.05  )  {Serial.println("X=  -2  , Y=  -10 ");}
   else if ( (abs( 0.208 -ratioA) + abs( 0.235 -ratioB) + abs( 0.557 -ratioC))<  0.05  )  {Serial.println("X=  0 , Y=  -10 ");}
   else if ( (abs( 0.170 -ratioA) + abs( 0.276 -ratioB) + abs( 0.554 -ratioC))<  0.05  )  {Serial.println("X=  2 , Y=  -10 ");}
   else if ( (abs( 0.132 -ratioA) + abs( 0.317 -ratioB) + abs( 0.551 -ratioC))<  0.05  )  {Serial.println("X=  4 , Y=  -10 ");}
   else if ( (abs( 0.093 -ratioA) + abs( 0.356 -ratioB) + abs( 0.551 -ratioC))<  0.05  )  {Serial.println("X=  6 , Y=  -10 ");}
   else if ( (abs( 0.058 -ratioA) + abs( 0.394 -ratioB) + abs( 0.548 -ratioC))<  0.05  )  {Serial.println("X=  8 , Y=  -10 ");}
   else if ( (abs( 0.014 -ratioA) + abs( 0.439 -ratioB) + abs( 0.546 -ratioC))<  0.05  )  {Serial.println("X=  10  , Y=  -10 ");}
   else if ( (abs( 0.344 -ratioA) + abs( 0.042 -ratioB) + abs( 0.613 -ratioC))<  0.05  )  {Serial.println("X=  -8  , Y=  -12 ");}
   else if ( (abs( 0.308 -ratioA) + abs( 0.085 -ratioB) + abs( 0.607 -ratioC))<  0.05  )  {Serial.println("X=  -6  , Y=  -12 ");}
   else if ( (abs( 0.265 -ratioA) + abs( 0.129 -ratioB) + abs( 0.606 -ratioC))<  0.05  )  {Serial.println("X=  -4  , Y=  -12 ");}
   else if ( (abs( 0.226 -ratioA) + abs( 0.170 -ratioB) + abs( 0.604 -ratioC))<  0.05  )  {Serial.println("X=  -2  , Y=  -12 ");}
   else if ( (abs( 0.181 -ratioA) + abs( 0.213 -ratioB) + abs( 0.606 -ratioC))<  0.05  )  {Serial.println("X=  0 , Y=  -12 ");}
   else if ( (abs( 0.149 -ratioA) + abs( 0.249 -ratioB) + abs( 0.601 -ratioC))<  0.05  )  {Serial.println("X=  2 , Y=  -12 ");}
   else if ( (abs( 0.108 -ratioA) + abs( 0.294 -ratioB) + abs( 0.599 -ratioC))<  0.05  )  {Serial.println("X=  4 , Y=  -12 ");}
   else if ( (abs( 0.070 -ratioA) + abs( 0.336 -ratioB) + abs( 0.595 -ratioC))<  0.05  )  {Serial.println("X=  6 , Y=  -12 ");}
   else if ( (abs( 0.029 -ratioA) + abs( 0.376 -ratioB) + abs( 0.594 -ratioC))<  0.05  )  {Serial.println("X=  8 , Y=  -12 ");}
   else if ( (abs( 0.241 -ratioA) + abs( 0.106 -ratioB) + abs( 0.653 -ratioC))<  0.05  )  {Serial.println("X=  -4  , Y=  -14 ");}
   else if ( (abs( 0.200 -ratioA) + abs( 0.147 -ratioB) + abs( 0.653 -ratioC))<  0.05  )  {Serial.println("X=  -2  , Y=  -14 ");}
   else if ( (abs( 0.159 -ratioA) + abs( 0.190 -ratioB) + abs( 0.651 -ratioC))<  0.05  )  {Serial.println("X=  0 , Y=  -14 ");}
   else if ( (abs( 0.120 -ratioA) + abs( 0.232 -ratioB) + abs( 0.648 -ratioC))<  0.05  )  {Serial.println("X=  2 , Y=  -14 ");}
   else if ( (abs( 0.083 -ratioA) + abs( 0.272 -ratioB) + abs( 0.645 -ratioC))<  0.05  )  {Serial.println("X=  4 , Y=  -14 ");}
   else{Serial.println("Error: Beyond Database!");}
   //delay(1000);

   
  return;  
}

//Display callback Function definitions
void b0PopCallback(void *ptr)
{
  Start = !Start;
  return;
}
void t0PopCallback(void *ptr)
{
  
  return;
}
void t2PopCallback(void *ptr)
{
  
  return;
}
