#include "raylib.h"
#include "raymath.h"

#define G 600
#define PLAYER_JUMP_SPD 350.0f
#define PLAYER_HOR_SPD 300.0f

typedef struct Player {
    Vector2 position;
    float speed;
    bool canJump;
    bool died;
    float deadTime;
} Player;

typedef struct EnvItem {
    Rectangle rect;
    int blocking;
    Color color;
    bool canKill;
} EnvItem;

//----------------------------------------------------------------------------------
// Module functions declaration
//----------------------------------------------------------------------------------
void UpdatePlayer(Player *player, EnvItem *envItems, int envItemsLength, float delta, Sound jumpSound);
void UpdateCameraCenter(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "CyberDash - 0.1");

    Player player = { 0 };
    player.position = (Vector2){ 300, 0 };
    player.speed = 0;
    player.canJump = false;
    player.died = false;

    EnvItem envItems[] = {
    {{ 0, 0, 1000, 400 }, 0, LIGHTGRAY, false }, // Skybox
    {{ 0, 0, 600, 200 }, 1, GRAY, false }, // Spawn
    {{ 600, 0, 50, 200 }, 1, RED, true }, // Plateforme Tueur 

    {{ 700, 0, 100, 10 }, 1, GRAY, false },  
    {{ 850, -30, 100, 10 }, 1, GRAY, false }, 
    {{ 1000, 0, 100, 10 }, 1, RED, true },    
    {{ 1150, -30, 100, 10 }, 1, GRAY, false },

    {{ 1300, 100, 100, 10 }, 1, GRAY, false },  
    {{ 1400, 150, 100, 10 }, 1, GRAY, false },  
    
    {{ 1600, 50, 50, 200 }, 1, RED, true },    

    {{ 1700, 100, 100, 10 }, 1, GRAY, false }, 
    {{ 1800, 200, 100, 10 }, 1, GRAY, false },  

    {{ 1900, 100, 100, 10 }, 1, RED, true },    
    {{ 2000, 100, 100, 10 }, 1, RED, true },    

    {{ 2100, 200, 100, 10 }, 1, GRAY, false },   
    {{ 2200, 100, 100, 10 }, 1, GRAY, false },  

    {{ 2300, 0, 100, 10 }, 1, GRAY, false },  
    {{ 2400, -100, 100, 10 }, 1, GRAY, false },  

    {{ 2600, 0, 50, 200 }, 1, RED, true },   

    {{ 2900, 0, 100, 10 }, 1, GRAY, false },  

    // Dernier niveau
    {{ 3000, 0, 20, 10 }, 1, GRAY, false },  

    {{ 3100, 0, 20, 10 }, 1, GRAY, false },  

    {{ 3200, 0, 20, 10 }, 1, GRAY, false }, 

    {{ 3300, 0, 20, 10 }, 1, RED, true },

    {{ 3400, 0, 20, 10 }, 1, GRAY, false },

    {{ 3500, 0, 20, 10 }, 1, RED, true },

    // Win
    {{ 3600, 0, 10, 10 }, 1, BLUE, false },

    // Limite 
    {{ -500, 250, 10000, 20 }, 1, LIGHTGRAY, true }
};




    int envItemsLength = sizeof(envItems)/sizeof(envItems[0]);

    Camera2D camera = { 0 };
    camera.target = player.position;
    camera.offset = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    // Store pointers to the multiple update camera functions
    void (*cameraUpdaters[])(Camera2D*, Player*, EnvItem*, int, float, int, int) = {
        UpdateCameraCenter,
    };

    int cameraOption = 0;
    int cameraUpdatersLength = sizeof(cameraUpdaters)/sizeof(cameraUpdaters[0]);

    char *cameraDescriptions[] = {
        "Follow player center",
        "Follow player center, but clamp to map edges",
        "Follow player center; smoothed",
        "Follow player center horizontally; update player center vertically after landing",
        "Player push camera on getting too close to screen edge"
    };

    unsigned int framesCounter = 0;

    // Initialisation de l'audio 
    InitAudioDevice();              

    Music Intro = LoadMusicStream("Sound/Intro.mp3");
    Sound jumpSound = LoadSound("Sound/Jump.mp3");
    Sound Death = LoadSound("Sound/Death.mp3");

    SetMusicVolume(Intro, 0.5);

    PlayMusicStream(Intro);

    // FPS
    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())
    {
        // Update
        //----------------------------------------------------------------------------------
        framesCounter++;

        // Background Music
        UpdateMusicStream(Intro);

        float deltaTime = GetFrameTime();

        UpdatePlayer(&player, envItems, envItemsLength, deltaTime, jumpSound);

        camera.zoom += ((float)GetMouseWheelMove()*0.05f);

        if (camera.zoom > 3.0f) camera.zoom = 3.0f;
        else if (camera.zoom < 0.25f) camera.zoom = 0.25f;

        if (IsKeyPressed(KEY_R))
        {
             player.position = (Vector2){ 300, 0 };
        }

        // Update de la camera
        cameraUpdaters[cameraOption](&camera, &player, envItems, envItemsLength, deltaTime, screenWidth, screenHeight);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(LIGHTGRAY);

            BeginMode2D(camera);

                for (int i = 0; i < envItemsLength; i++) DrawRectangleRec(envItems[i].rect, envItems[i].color);

                Rectangle playerRect = { player.position.x - 20, player.position.y - 40, 40.0f, 40.0f };

                if (player.died) {
                    DrawRectangleRec(playerRect, RED);
                } else {
                    DrawRectangleRec(playerRect, GREEN);
                }
                             
                DrawCircleV(player.position, 5.0f, GOLD);

            EndMode2D();

            DrawText("Controls:", 20, 20, 10, BLACK);
            DrawText("- AD/ Arrow Left/ Arrow Right to move", 40, 40, 10, DARKGRAY);
            DrawText("- Space/W/Arrow Up to jump", 40, 60, 10, DARKGRAY);
            DrawText("- Mouse Wheel to Zoom in-out, R to reset", 40, 80, 10, DARKGRAY);
            DrawText("- Press R to reset", 40, 100, 10, DARKGRAY);


        EndDrawing();

        if (player.died == true) {
            PlaySound(Death);
            player.speed = 0;
            player.canJump = false;
            player.died = false;
            player.position = (Vector2){ 300, 0 };
        }

        //----------------------------------------------------------------------------------
    }

    UnloadSound(jumpSound);

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

void UpdatePlayer(Player *player, EnvItem *envItems, int envItemsLength, float delta, Sound jumpSound)
{


    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) player->position.x -= PLAYER_HOR_SPD * delta;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) player->position.x += PLAYER_HOR_SPD * delta;
    if ((IsKeyDown(KEY_SPACE) && player->canJump) || (IsKeyDown(KEY_UP) && player->canJump) || (IsKeyDown(KEY_W) && player->canJump)) {
        PlaySound(jumpSound);
        player->speed = -PLAYER_JUMP_SPD;
        player->canJump = false;
    }

    bool hitObstacle = false;

    for (int i = 0; i < envItemsLength; i++) {
        EnvItem *ei = envItems + i;
        Vector2 *p = &(player->position);

        // Si (canKill == true)
        if (ei->canKill &&
            ei->rect.x <= p->x &&
            ei->rect.x + ei->rect.width >= p->x &&
            ei->rect.y <= p->y &&
            ei->rect.y + ei->rect.height >= p->y) {
            player->died = true;  
            return;
        }

        if (ei->blocking &&
            ei->rect.x <= p->x &&
            ei->rect.x + ei->rect.width >= p->x &&
            ei->rect.y >= p->y &&
            ei->rect.y <= p->y + player->speed * delta) {
            hitObstacle = true;
            player->speed = 0.0f;
            p->y = ei->rect.y;
            break;
        }
    }

    if (!hitObstacle) {
        player->position.y += player->speed * delta;
        player->speed += G * delta;
        player->canJump = false;
    } else {
        player->canJump = true;
    }
}

void UpdateCameraCenter(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height)
{
    camera->offset = (Vector2){ width/2.0f, height/2.0f };
    camera->target = player->position;
}
