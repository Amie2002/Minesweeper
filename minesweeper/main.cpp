//
//  main.cpp
//  RaylibExample
//
//  Created by Aditi on 06/08/24.
//

// main.cpp

#include <raylib.h>
#include <raymath.h>
#include <iostream>

#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 600

#define cols 12
#define rows 12

const int TILE_WIDTH = SCREEN_WIDTH/cols;
const int TILE_HEIGHT = SCREEN_HEIGHT/rows;

typedef struct {
    int x;
    int y;
    bool isMine;
    bool isRevealed;
    bool isFlagged;
    int nearbyMineCount;
} sTile;

sTile grid[cols][rows];
int revealedTileCount;
int minesPresentCount;

float timeGameStarted;
float timeGameEnded;

const char* labelGameWin="YOU WIN";
const char* labelGameLose="GAME OVER";
const char* labelEnter="PRESS ENTER FOR MAIN MENU";

bool isSoundEnabled = true;
bool isMusicEnabled = true;

#define MAX_TEXTURES 1
typedef enum {
    TEXTURE_FLAG=0
} texture_asset;

Texture2D textures[MAX_TEXTURES];

#define MAX_SOUNDS 3
typedef enum {
    SOUND_ONE=0,
    SOUND_TWO,
    SOUND_THREE
} sound_asset;

Sound sounds[MAX_SOUNDS];

#define MAX_MUSIC 1
typedef enum {
    MUSIC_INTRO=0
} music_asset;

Music music[MAX_MUSIC];

typedef enum {
    STATE_MAIN_MENU=0,
    STATE_OPTIONS_MENU,
    STATE_PLAYING,
    STATE_LOSE,
    STATE_WIN
} game_states;

game_states gameState;

void GameStartup();
void GameUpdate();
void GameShutdown();
void GameRender();
void GameReset();
void ReserTiles();
void RenderTiles();
void RenderTile(sTile);
int CountNearbyMines(int, int);
bool IsTileIndexValid(int, int);
void RevealTile(int, int);
void FlagTile(int, int);
void RevealTileFrom(int, int);
void GamePlaySound(int sound);

void GamePlaySound(int sound){
    if (isSoundEnabled) {
        PlaySound(sounds[sound]);
    }
}

bool IsTileIndexValid(int col, int row){
    return col>=0 && col<cols && row>=0 && row<rows;
}

void RevealTileFrom(int col, int row){
    for (int colOffset=-1; colOffset<=1; colOffset++) {
        for (int rowOffset=-1; rowOffset<=1; rowOffset++) {
            if (colOffset==0 && rowOffset==0) {
                continue;
            }
            if (!IsTileIndexValid(col+colOffset, row+rowOffset)) {
                continue;
            }
            RevealTile(col+colOffset, row+rowOffset);
        }
    }
}

void FlagTile(int col, int row){
    if (grid[col][row].isFlagged) {
        return;
    }
    if (!grid[col][row].isRevealed) {
        grid[col][row].isFlagged = !grid[col][row].isFlagged;
        GamePlaySound(SOUND_THREE);
    }
}

void RevealTile(int col, int row){
    if (grid[col][row].isFlagged || grid[col][row].isRevealed) {
        return;
    }
    grid[col][row].isRevealed=true;
    
    if (grid[col][row].isMine) {
        //game is over
        gameState=STATE_LOSE;
        timeGameEnded=(float)GetTime();
        GamePlaySound(SOUND_TWO);
        
    }else{
        if (grid[col][row].nearbyMineCount == 0) {
            RevealTileFrom(col, row);
        }
        
        revealedTileCount++;
        
        if (revealedTileCount>=rows * cols - minesPresentCount) {
            //win
            gameState=STATE_WIN;
            timeGameEnded=(float)GetTime();
        }
    }
}

int CountNearbyMines(int col, int row){
    int count =0;
    for (int colOffset=-1; colOffset<=1; colOffset++) {
        for (int rowOffset=-1; rowOffset<=1; rowOffset++) {
            if (colOffset==0 && rowOffset==0) {
                continue;
            }
            
            if (grid[col+colOffset][row+rowOffset].isMine) {
                count++;
            }
        }
    }
    return count;
}

void ResetTiles(){
    for (int i=0;i<cols;i++){
        for (int j=0;j<rows;j++){
            grid[i][j]=(sTile){
                .x=i,
                .y=j,
                .isMine=false,
                .isRevealed=false,
                .isFlagged=false,
                .nearbyMineCount=-1
            };
        }
    }
    
    minesPresentCount=(int)(rows*cols*0.2f);
    int minesToPlace=minesPresentCount;
    while(minesToPlace>0){
        int col=GetRandomValue(0, cols);
        int row=GetRandomValue(0, rows);
        
        if (!grid[col][row].isMine){
            grid[col][row].isMine=true;
            minesToPlace--;
        }
    }
    
    for (int i=0; i<cols; i++) {
        for (int j=0; j<rows; j++) {
            if (!grid[i][j].isMine) {
                grid[i][j].nearbyMineCount=CountNearbyMines(i,j);
            }
        }
    }
    
}

void RenderTile(sTile tile){
    if (tile.isRevealed) {
        if (tile.isMine){
            DrawRectangle(tile.x * TILE_WIDTH, tile.y * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT, RED);
        }
        else{
            DrawRectangle(tile.x * TILE_WIDTH, tile.y * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT, DARKGRAY);
            if (tile.nearbyMineCount>0) {
                DrawText(TextFormat("%d",tile.nearbyMineCount), tile.x * TILE_WIDTH +13, tile.y * TILE_HEIGHT+4, TILE_HEIGHT-8, GREEN);
            }
        }
    } else if (tile.isFlagged){
        Rectangle source={0,0,(float)textures[TEXTURE_FLAG].width,(float)textures[TEXTURE_FLAG].height};
        Rectangle dest={(float)(tile.x * TILE_WIDTH),(float)(tile.y * TILE_HEIGHT),(float)TILE_WIDTH,(float)TILE_HEIGHT};
        Vector2 origin={0,0};
        DrawTexturePro(textures[TEXTURE_FLAG], source, dest, origin, 0.0f, WHITE);
    }

    DrawRectangleLines(tile.x * TILE_WIDTH, tile.y * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT, BLACK);
}

void RenderTiles(){
    for (int i=0;i<cols;i++){
        for (int j=0;j<rows;j++){
            RenderTile(grid[i][j]);
        }
    }
}

void GameStartup(){
    InitAudioDevice();
    
    // load flag
    std::string filepath = "/Users/aditi/Documents/my-c++_codes/Minesweeper/minesweeper/assets/redflag.png";
    Image image1 = LoadImage(filepath.c_str());
    textures[TEXTURE_FLAG] = LoadTextureFromImage(image1);
    UnloadImage(image1);
    
    //load sounds for action
    std::string soundonefilepath = "/Users/aditi/Documents/my-c++_codes/Minesweeper/minesweeper/assets/click.wav";
    std::string soundtwofilepath = "/Users/aditi/Documents/my-c++_codes/Minesweeper/minesweeper/assets/explosion.wav";
    std::string soundthreefilepath = "/Users/aditi/Documents/my-c++_codes/Minesweeper/minesweeper/assets/pickupCoin.wav";
    sounds[SOUND_ONE] = LoadSound(soundonefilepath.c_str());
    sounds[SOUND_TWO] = LoadSound(soundtwofilepath.c_str());
    sounds[SOUND_THREE] = LoadSound(soundthreefilepath.c_str());
    
    //load music stream
    std::string musicfilepath = "/Users/aditi/Documents/my-c++_codes/Minesweeper/minesweeper/assets/8-bit-game-158815.mp3";
    music[MUSIC_INTRO] = LoadMusicStream(musicfilepath.c_str());
    PlayMusicStream(music[MUSIC_INTRO]);
    
    //game state
    gameState=STATE_MAIN_MENU;
    
}

void GameUpdate(){
    //music player
    UpdateMusicStream(music[MUSIC_INTRO]);
    
    //game stage cases
    switch (gameState) {
            //actions for main menu
        case STATE_MAIN_MENU:
            if (IsKeyPressed(KEY_N)) {
                GamePlaySound(SOUND_TWO);
                GameReset();
            } else if (IsKeyPressed(KEY_O)){
                gameState=STATE_OPTIONS_MENU;
                GamePlaySound(SOUND_TWO);
            }
            break;
            //actions for game
        case STATE_PLAYING:
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 mousePos=GetMousePosition();
                int col=(int)(mousePos.x/TILE_WIDTH);
                int row=(int)(mousePos.y/TILE_HEIGHT);
                
                if (IsTileIndexValid(col, row)) {
                    RevealTile(col, row);
                    GamePlaySound(SOUND_ONE);
                }
            } else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)){
                Vector2 mousePos=GetMousePosition();
                int col=(int)(mousePos.x/TILE_WIDTH);
                int row=(int)(mousePos.y/TILE_HEIGHT);
                
                //remove flag
                if (grid[col][row].isFlagged) {
                    grid[col][row].isFlagged =false;
                }
                //add flag
                else if (IsTileIndexValid(col, row)) {
                    FlagTile(col, row);
                }
            }
            break;
            //action for options menu
        case STATE_OPTIONS_MENU:
            if (IsKeyPressed(KEY_ENTER)) {
                gameState=STATE_MAIN_MENU;
                GamePlaySound(SOUND_TWO);
            }
            if (IsKeyPressed(KEY_S)) {
                isSoundEnabled= !isSoundEnabled;
                PlaySound(sounds[SOUND_ONE]);
            }
            if (IsKeyPressed(KEY_M)) {
                isMusicEnabled=!isMusicEnabled;
                PlaySound(sounds[SOUND_ONE]);
                if (isMusicEnabled) {
                    StopMusicStream(music[MUSIC_INTRO]);
                    PlayMusicStream(music[MUSIC_INTRO]);
                }
                else{
                    StopMusicStream(music[MUSIC_INTRO]);
                }
            }
            break;
            //action for win
        case STATE_WIN:
            if (IsKeyPressed(KEY_ENTER)) {
                GamePlaySound(SOUND_TWO);
                gameState=STATE_MAIN_MENU;
            }
            break;
            //action for lose
        case STATE_LOSE:
            if (IsKeyPressed(KEY_ENTER)) {
                GamePlaySound(SOUND_TWO);
                gameState=STATE_MAIN_MENU;
            }
            break;
    }
    
}

void GameRender(){
    int seconds=0;
    
    switch (gameState) {
        case STATE_MAIN_MENU:
            //rendering for main menu
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, DARKPURPLE);
            DrawText("MINESWEEPER", 20, 20, 40, WHITE);
            DrawText("[N]ew Game", 120, 220, 20, WHITE);
            DrawText("[O]ptions", 120, 250, 20, WHITE);
            DrawText("ESC to QUIT", 120, 280, 20, WHITE);
            DrawText("Copyright 2024 @amie", 120, 500, 20, WHITE);
            break;
            
        case STATE_PLAYING:
            //rendering for game
            RenderTiles();
            break;
        case STATE_OPTIONS_MENU:
            //rendering for option menu
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, DARKPURPLE);
            DrawText("MINESWEEPER :: OPTIONS", 20, 20, 40, WHITE);
            
            DrawText("[S]ound ", 120, 220, 20, WHITE);
            if (isSoundEnabled) {
                DrawText("ON", 280, 220, 20, GREEN);
                DrawText(" / ", 310, 220, 20, WHITE);
                DrawText("OFF", 350, 220, 20, WHITE);
            }
            else {
                DrawText("ON", 280, 220, 20, WHITE);
                DrawText(" / ", 310, 220, 20, WHITE);
                DrawText("OFF", 350, 220, 20, GREEN);
            }

            DrawText("[M]usic", 120, 250, 20, WHITE);
            if (isMusicEnabled) {
                DrawText("ON", 280, 250, 20, GREEN);
                DrawText(" / ", 310, 250, 20, WHITE);
                DrawText("OFF", 350, 250, 20, WHITE);
            }
            else {
                DrawText("ON", 280, 250, 20, WHITE);
                DrawText(" / ", 310, 250, 20, WHITE);
                DrawText("OFF", 350, 250, 20, GREEN);
            }
            DrawText(labelEnter, 120, 400, 20, WHITE);
            
            break;
        case STATE_WIN:
            //rendering for win
            RenderTiles();
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(WHITE, 0.8f));
            DrawText(labelGameWin, (SCREEN_WIDTH/2 - MeasureText(labelGameWin,60)/2)+3, (SCREEN_HEIGHT/2 -10)+3, 60, BLACK);
            DrawText(labelGameWin, SCREEN_WIDTH/2 - MeasureText(labelGameWin,60)/2, SCREEN_HEIGHT/2 -10, 60, RED);
            DrawText(labelEnter, SCREEN_WIDTH/2 - MeasureText(labelEnter,34)/2, (int)(SCREEN_HEIGHT*0.75f)-10, 34, BLACK);
            seconds = (int)(timeGameEnded - timeGameStarted) % 60;
            DrawText(TextFormat("TIME PLAYED: %d s", seconds), 20, 40, 34, DARKGRAY);
            break;
        case STATE_LOSE:
            //rendering for lose
            RenderTiles();
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(WHITE, 0.8f));
            DrawText(labelGameLose, (SCREEN_WIDTH/2 - MeasureText(labelGameLose,60)/2)+3, (SCREEN_HEIGHT/2 -10)+3, 60, BLACK);
            DrawText(labelGameLose, SCREEN_WIDTH/2 - MeasureText(labelGameLose,60)/2, SCREEN_HEIGHT/2 -10, 60, RED);
            DrawText(labelEnter, SCREEN_WIDTH/2 - MeasureText(labelEnter,34)/2, (int)(SCREEN_HEIGHT*0.75f)-10, 34, BLACK);
            seconds = (int)(timeGameEnded - timeGameStarted) % 60;
            DrawText(TextFormat("TIME PLAYED: %d s", seconds), 20, 40, 34, DARKGRAY);
            break;
    }
    

}

void GameShutdown(){
    //unload texture
    for (int i=0; i<MAX_TEXTURES; i++) {
        UnloadTexture(textures[i]);
    }
    
    //unload sound and music
    for (int i=0; i<MAX_SOUNDS; i++) {
        UnloadSound(sounds[i]);
    }
    StopMusicStream(music[MUSIC_INTRO]);
    UnloadMusicStream(music[MUSIC_INTRO]);
    
    CloseAudioDevice();
}

void GameReset(){
    ResetTiles();
    gameState=STATE_PLAYING;
    revealedTileCount=0;
    timeGameStarted=(float)GetTime();
}

int main() {
    // Initialize the window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Raylib :: Minesweeper");

    // Set the target frame rate
    SetTargetFPS(60);
    
    GameStartup();

    // Main game loop
    while (!WindowShouldClose()) {
        // Update the game state
        GameUpdate();
    
        BeginDrawing();
        ClearBackground(LIGHTGRAY);
        
        GameRender();
        
        EndDrawing();
    }
    
    GameShutdown();

    // De-initialize the window
    CloseWindow();

    return 0;
}
