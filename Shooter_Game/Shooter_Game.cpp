#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <chrono>
using namespace std;

#include <stdio.h>
#include <Windows.h>

int screenWidth = 120;
int screenHeight = 40;
int mapWidth = 16;
int mapHeight = 16;

float playerX = 14.7f;
float playerY = 5.09f;
float playerAngle = 0.0f;
float fov = 3.14159f / 4.0f;
float depth = 16.0f;
float speed = 5.0f;

int main()
{
    wchar_t* screen = new wchar_t[screenWidth * screenHeight];
    HANDLE consoleHandle = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(consoleHandle);
    DWORD bytesWritten = 0;

    wstring map;
    map += L"########........";
    map += L"#...............";
    map += L"#.......########";
    map += L"#..............#";
    map += L"#......#.......#";
    map += L"#......##......#";
    map += L"#..............#";
    map += L"###............#";
    map += L"##.............#";
    map += L"#............###";
    map += L"#...##.........#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#......###..####";
    map += L"#..............#";
    map += L"################";

    auto timePoint1 = chrono::system_clock::now();
    auto timePoint2 = chrono::system_clock::now();

    while (1)
    {
        timePoint2 = chrono::system_clock::now();
        chrono::duration<float> elapsedTime = timePoint2 - timePoint1;
        timePoint1 = timePoint2;
        float deltaTime = elapsedTime.count();

        if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
            playerAngle -= (speed * 0.75f) * deltaTime;

        if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
            playerAngle += (speed * 0.75f) * deltaTime;

        if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
        {
            playerX += sinf(playerAngle) * speed * deltaTime;
            playerY += cosf(playerAngle) * speed * deltaTime;
            if (map.c_str()[(int)playerX * mapWidth + (int)playerY] == '#')
            {
                playerX -= sinf(playerAngle) * speed * deltaTime;
                playerY -= cosf(playerAngle) * speed * deltaTime;
            }
        }

        if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
        {
            playerX -= sinf(playerAngle) * speed * deltaTime;
            playerY -= cosf(playerAngle) * speed * deltaTime;
            if (map.c_str()[(int)playerX * mapWidth + (int)playerY] == '#')
            {
                playerX += sinf(playerAngle) * speed * deltaTime;
                playerY += cosf(playerAngle) * speed * deltaTime;
            }
        }

        for (int x = 0; x < screenWidth; x++)
        {
            float rayAngle = (playerAngle - fov / 2.0f) + ((float)x / (float)screenWidth) * fov;
            float stepSize = 0.1f;
            float distanceToWall = 0.0f;

            bool hitWall = false;
            bool boundary = false;

            float eyeX = sinf(rayAngle);
            float eyeY = cosf(rayAngle);

            while (!hitWall && distanceToWall < depth)
            {
                distanceToWall += stepSize;
                int testX = (int)(playerX + eyeX * distanceToWall);
                int testY = (int)(playerY + eyeY * distanceToWall);

                if (testX < 0 || testX >= mapWidth || testY < 0 || testY >= mapHeight)
                {
                    hitWall = true;
                    distanceToWall = depth;
                }
                else
                {
                    if (map.c_str()[testX * mapWidth + testY] == '#')
                    {
                        hitWall = true;
                        vector<pair<float, float>> p;
                        for (int tx = 0; tx < 2; tx++)
                            for (int ty = 0; ty < 2; ty++)
                            {
                                float vy = (float)testY + ty - playerY;
                                float vx = (float)testX + tx - playerX;
                                float d = sqrt(vx * vx + vy * vy);
                                float dot = (eyeX * vx / d) + (eyeY * vy / d);
                                p.push_back(make_pair(d, dot));
                            }

                        sort(p.begin(), p.end(), [](const pair<float, float>& left, const pair<float, float>& right) {return left.first < right.first; });
                        float boundaryThreshold = 0.01;
                        if (acos(p.at(0).second) < boundaryThreshold) boundary = true;
                        if (acos(p.at(1).second) < boundaryThreshold) boundary = true;
                        if (acos(p.at(2).second) < boundaryThreshold) boundary = true;
                    }
                }
            }
            int ceiling = (float)(screenHeight / 2.0) - screenHeight / ((float)distanceToWall);
            int floor = screenHeight - ceiling;

            short shade = ' ';
            if (distanceToWall <= depth / 4.0f)            shade = 0x2588;
            else if (distanceToWall < depth / 3.0f)        shade = 0x2593;
            else if (distanceToWall < depth / 2.0f)        shade = 0x2592;
            else if (distanceToWall < depth)                shade = 0x2591;
            else                                            shade = ' ';

            if (boundary)        shade = ' ';

            for (int y = 0; y < screenHeight; y++)
            {
                if (y <= ceiling)
                    screen[y * screenWidth + x] = ' ';
                else if (y > ceiling && y <= floor)
                    screen[y * screenWidth + x] = shade;
                else
                {
                    float b = 1.0f - (((float)y - screenHeight / 2.0f) / ((float)screenHeight / 2.0f));
                    if (b < 0.25)        shade = '#';
                    else if (b < 0.5)    shade = 'x';
                    else if (b < 0.75)   shade = '.';
                    else if (b < 0.9)    shade = '-';
                    else                shade = ' ';
                    screen[y * screenWidth + x] = shade;
                }
            }
        }
        swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f ", playerX, playerY, playerAngle, 1.0f / deltaTime);

        for (int nx = 0; nx < mapWidth; nx++)
            for (int ny = 0; ny < mapWidth; ny++)
            {
                screen[(ny + 1) * screenWidth + nx] = map[ny * mapWidth + nx];
            }
        screen[((int)playerX + 1) * screenWidth + (int)playerY] = 'P';
        screen[screenWidth * screenHeight - 1] = '\0';
        WriteConsoleOutputCharacter(consoleHandle, screen, screenWidth * screenHeight, { 0,0 }, &bytesWritten);
    }

    return 0;
}
