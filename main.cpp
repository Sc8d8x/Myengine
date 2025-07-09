#include <stdio.h>
#include <iostream>
#include <Windows.h>
#include <chrono>
#include <vector>
using namespace std;
double pi = 3.14159;

int DisplayHeight = 40;
int DisplayWidth = 120;

float CamX = 8.0f;
float CamY = 8.0f;
float CamA = 0.0f;

double Fov = pi / 4.0;
double Depth = 16.0f;

int MapWidth = 21;
int MapHeight = 8;


const int MiniMapWidth = 20;
const int MiniMapHeight = 10;

void DrawMiniMap(wchar_t* display, const wstring& map, int mapWidth, int mapHeight) {
  
    int offsetX = DisplayWidth - MiniMapWidth - 2;
    int offsetY = 1;

    
    for (int x = 0; x < MiniMapWidth + 2; x++) {
        display[offsetY * DisplayWidth + offsetX + x] = L'─';
        display[(offsetY + MiniMapHeight + 1) * DisplayWidth + offsetX + x] = L'─';
    }
    for (int y = 0; y < MiniMapHeight + 2; y++) {
        display[(offsetY + y) * DisplayWidth + offsetX] = L'│';
        display[(offsetY + y) * DisplayWidth + offsetX + MiniMapWidth + 1] = L'│';
    }
    
    display[offsetY * DisplayWidth + offsetX] = L'┌';
    display[offsetY * DisplayWidth + offsetX + MiniMapWidth + 1] = L'┐';
    display[(offsetY + MiniMapHeight + 1) * DisplayWidth + offsetX] = L'└';
    display[(offsetY + MiniMapHeight + 1) * DisplayWidth + offsetX + MiniMapWidth + 1] = L'┘';

    
    float scaleX = (float)MiniMapWidth / mapWidth;
    float scaleY = (float)MiniMapHeight / mapHeight;

   
    for (int y = 0; y < mapHeight; y++) {
        for (int x = 0; x < mapWidth; x++) {
            int mx = offsetX + 1 + (int)(x * scaleX);
            int my = offsetY + 1 + (int)(y * scaleY);

            if (mx < offsetX + 1  ||
                my < offsetY + 1 || my >= offsetY + 1 + MiniMapHeight)
                continue;

            if (map[y * mapWidth + x] == L'#') {
                display[my * DisplayWidth + mx] = L'█';
            }
            else {
                display[my * DisplayWidth + mx] = L'·';
            }
        }
    }

  
    int playerX = offsetX + 1 + (int)(CamX * scaleX);
    int playerY = offsetY + 1 + (int)(CamY * scaleY);

    if (playerX >= offsetX + 1 && playerX < offsetX + 1 + MiniMapWidth &&
        playerY >= offsetY + 1 && playerY < offsetY + 1 + MiniMapHeight) {
       
        wchar_t dirChar;
        if (CamA >= -pi / 4 && CamA < pi / 4) dirChar = L'→';
        else if (CamA >= pi / 4 && CamA < 3 * pi / 4) dirChar = L'↑';
        else if (CamA >= -3 * pi / 4 && CamA < -pi / 4) dirChar = L'↓';
        else dirChar = L'←';

        display[playerY * DisplayWidth + playerX] = dirChar;
    }
}

int main() {
    wchar_t* display = new wchar_t[DisplayHeight * DisplayWidth];
    HANDLE AConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(AConsole);
    DWORD WriteBytes = 0;

    std::wstring map = L"#####################"
        L"#...................#"
        L"#...................#"
        L"#...................#"
        L"#...................#"
        L"#...................#"
        L"#...................#"
        L"#####################";

    auto q1 = std::chrono::system_clock::now();
    auto q2 = std::chrono::system_clock::now();

    while (1) {
        q2 = chrono::system_clock::now();
        chrono::duration<float> TimeElapse = q2 - q1;
        q1 = q2;
        float TimeElapseA = TimeElapse.count();

        if (GetAsyncKeyState((unsigned short)'A') & 0x8000)CamA -= (1.0f) * TimeElapseA;
        if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
            CamA += (1.0f) * TimeElapseA;
        if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
            CamX += sinf(CamA) * 5.0f * TimeElapseA;
            CamY += cosf(CamA) * 5.0f * TimeElapseA;
        }
        if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {
            CamX -= sinf(CamA) * 5.0f * TimeElapseA;
            CamY -= cosf(CamA) * 5.0f * TimeElapseA;
        }

        
        if (CamX < 0) CamX = 0;
        if (CamX >= MapWidth) CamX = MapWidth - 0.1f;
        if (CamY < 0) CamY = 0;
        if (CamY >= MapHeight) CamY = MapHeight - 0.1f;

        
        for (int i = 0; i < DisplayHeight * DisplayWidth; i++) {
            display[i] = ' ';
        }

        for (int x = 0; x < DisplayWidth; x++) {
            float RayAngle = (CamA - Fov / 2.0f) + ((float)x / (float)DisplayWidth) * Fov;

            float WallDistance = 0;
            bool WallCollision = false;

            float ViewX = sinf(RayAngle);
            float ViewY = cosf(RayAngle);

            while (!WallCollision && WallDistance < Depth) {
                WallDistance += 0.1f;
                int TestX = (int)(CamX + ViewX * WallDistance);
                int TestY = (int)(CamY + ViewY * WallDistance);

                if (TestX < 0 || TestY < 0 || TestY >= MapHeight) {
                    WallCollision = true;
                    WallDistance = Depth;
                }
                else {
                    if (map[TestY * MapWidth + TestX] == '#') {
                        WallCollision = true;
                    }
                }
            }

            int Ceiling = (float)(DisplayHeight / 2.0) - DisplayHeight / ((float)WallDistance);
            int Floor = DisplayHeight - Ceiling;
            wchar_t Shader = ' ';

            if (WallDistance <= Depth / 4.0f)       Shader = 0x2588;
            else if (WallDistance < Depth / 3.0f)   Shader = 0x2593;
            else if (WallDistance < Depth / 2.0f)    Shader = 0x2592;
            else if (WallDistance < Depth)          Shader = 0x2591;
            else                                    Shader = ' ';

            for (int y = 0; y < DisplayHeight; y++) {
                if (y < Ceiling)
                    display[y * DisplayWidth + x] = ' ';
                else if (y > Ceiling && y <= Floor)
                    display[y * DisplayWidth + x] = Shader;
                else
                    display[y * DisplayWidth + x] = ' ';
            }
        }

        
        DrawMiniMap(display, map, MapWidth, MapHeight);

        swprintf(&display[0], DisplayWidth, L"X:%.1f Y:%.1f A:%.1f", CamX, CamY, CamA);

        display[DisplayWidth * DisplayHeight - 1] = '\0';
        WriteConsoleOutputCharacterW(AConsole, display, DisplayWidth * DisplayHeight, { 0,0 }, &WriteBytes);
    }
    return 0;
}
