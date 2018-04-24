#include <math.h>
#include "LedControlMS.h"

 //pin 4 is connected to the DataIn
// pin 3 is connected to the CLK
 //pin 2 is connected to LOAD

#define NBR_MTX 1 //number of matrices attached
LedControl lc=LedControl(4,3,2, NBR_MTX);//

// Logic for changing screen
bool dotsMap[8*NBR_MTX][8];
int score = 0;
int currentX = 0;
const float Pi = 3.14159;
int inputDirectionPin = 3;

int dotCount = 10;
int dots[][3] = {
  {0,0,1000},
  {0,1,1000},
  {0,2,1000},
  {0,3,1000},
  {0,4,1000},
  {0,5,1000},
  {0,6,1000},
  {0,7,1000},
  {1,1,1000},
  {1,2,1000}
};

unsigned long _lastTickTimeMs; // Initialized in setup
void setup()
{
  randomSeed(analogRead(0));
  Serial.begin(9600);
  
  for (int i=0; i< NBR_MTX; i++)
  {
    lc.shutdown(i,false);
    lc.setIntensity(i,0);
    lc.clearDisplay(i);
    
    delay(100);
  }

  // Start out flipping the map points for each dot
  for(int i = 0; i < dotCount; i++){
    int dotX = dots[i][0];
    int dotY = dots[i][1];
    dotsMap[dotX][dotY] = true;
  }
  _lastTickTimeMs = millis();
}



double straightDownDir = 0;

int tickRateMs = 40; // 40ms
void loop()
{
  double minInput = 250;
  double maxInput = 400;

  // Following mid values was found by callibration (placing on a flat surface)
  // double midX = 333;
  // double midY = 322;
  
  // Read input
  double rawX = analogRead(A5);
  double rawY = analogRead(A4);

  Serial.println(String(rawX) + " x " + String(rawY));
  
  double x = ((double)map(rawX, minInput, maxInput, -1000, 1000))/(double)1000;
  double y = ((double)map(rawY, minInput, maxInput, -1000, 1000))/(double)1000;
  
  double dir = atan2(y,x);
  straightDownDir = - dir + PI * 0.5;
  
  // LOGIC
  unsigned long loopStartMs = millis();
  unsigned long dif = loopStartMs - _lastTickTimeMs;
  while(dif > tickRateMs)
  {
    updateLogic();
    _lastTickTimeMs += tickRateMs;
    dif = dif = loopStartMs - _lastTickTimeMs;
    Serial.print(".");
  }

  // DRAW 
  drawMap(dotsMap);
  Serial.println("!");
}

double randomDouble(){
  return ((double)random(1000)) / ((double)1000);
}

void updateLogic()
{
  
    // Start out flipping the map points for each dot
    for(int i = 0; i < dotCount; i++){
      int dotX = dots[i][0];
      int dotY = dots[i][1];
      int dotSpeed = dots[i][2];

      int degreesSize = 2;
      double degreesToTest[] = {straightDownDir, straightDownDir + randomDouble()-0.5, straightDownDir - randomDouble()-0.5};
      bool hasMoved = false;
      for(int degreesIndex = 0; degreesIndex < degreesSize && hasMoved == false; degreesIndex++){
        for(double speedIndexToTest = 0; speedIndexToTest < 1 && hasMoved == false; speedIndexToTest++)
        {
          double moveDistanceToTest = ((double)dotSpeed / 1000.0) / (1.0 + speedIndexToTest/5.0);
          double dirToTest = degreesToTest[degreesIndex];
          int targetX = dotX + (int)round(cos(dirToTest) * moveDistanceToTest);
          int targetY = dotY + (int)round(sin(dirToTest) * moveDistanceToTest);
          
          hasMoved = tryMove(dotX, dotY, targetX, targetY);
          if(hasMoved){
             dots[i][0] = targetX;
             dots[i][1] = targetY;
          }
        }
      }
      
      double acceleration = 1.15;
      if(hasMoved){
        // Increase speed
        dots[i][2] = dotSpeed * acceleration;
      }else{
        dots[i][2] = max(dotSpeed / acceleration, 1000);
      }
   }

}

bool tryMove(int fromX, int fromY, int toX, int toY){
    if(checkAvailable(toX, toY))
    {
      dotsMap[fromX][fromY] = false;
      dotsMap[toX][toY] = true;
      return true;
    }else{
      return false;
    }
}
bool checkAvailable(int x, int y){
  if(x < 0 || x > 8*NBR_MTX-1)
    return false;
  if(y < 0 || y > 7)
    return false;
   return !dotsMap[x][y];
}

// DRAWING 2D BOOL ARRAY TO SCREEN
byte lastDrawnRows[8*NBR_MTX];
void drawMap(bool dotMapToDraw[][8]){
  
  for(int matrix = 0; matrix < NBR_MTX; matrix++){
    for(int i = 0; i < 8; i++){
      int rowIndex = matrix * 8 + i;
      byte rowAsByte = boolArrayToByte(dotMapToDraw[rowIndex]);
      if(lastDrawnRows[rowIndex] != rowAsByte){
        lc.setColumn(matrix,7-i, rowAsByte);
        lastDrawnRows[rowIndex] = rowAsByte; // Remember last byt drawn on this row to save power when nothing changed on row
      }
    }
  }
  
}

byte boolArrayToByte(bool boolArray[8])
{
  byte result = 0; 

  for(int i = 0; i < 8; i++)
  {
    if(boolArray[i])
    {
      result = result | (1 << i);
    }
  }
  return result;
}
