#include <genesis.h>
#include <resources.h>
#include <string.h>

#define ANIM_RUN	0
#define ANIM_JUMP	1

// General stuff
const char msg_start[22] = "Press START to Begin!";
const char msg_reset[22] = "Press START to Reset!";
bool game_on = FALSE;

const int scrollspeed = 2;

Sprite* player;
const int player_x = 32;
fix16 player_y = FIX16(112);

fix16 player_vel_y = FIX16(0); // FIX16 data type is used to give movement like a float, but isn't irrational

int player_height = 16;
const int floor_height = 128;
fix16 gravity = FIX16(0.2);
bool jumping = FALSE;

// Obstacle stuff
Sprite* obstacle;
int obstacle_x = 320;
int obstacle_vel_x = 0;

//scoring
int score = 0;
char label_score[6] = "SCORE\0";
char str_score[3] = "0";
bool score_added = FALSE;



void showText(char s[]){
	VDP_drawText(s, 20 - strlen(s)/2 ,10);
}

void clearText(){
	VDP_clearText(0,10,32);
}

void startGame(){
	if(game_on == FALSE){
		game_on = TRUE;
		clearText();
        VDP_drawText(label_score,10,1);
        score = 0;
        updateScoreDisplay();
        obstacle_x = 320;
	}
}

void endGame(){
	if(game_on == TRUE){
		showText(msg_reset);
		game_on = FALSE;
	}
}

void myJoyHandler( u16 joy, u16 changed, u16 state)
{
	if (joy == JOY_1)
	{
		/*Start game if START is pressed*/
		if (state & BUTTON_START)
		{
			if(game_on == FALSE){
				startGame();
			}
		}
		if (state & BUTTON_C)
        {
            if(jumping == FALSE){
                jumping = TRUE;
                player_vel_y = FIX16(-4);
                SPR_setAnim(player,ANIM_JUMP);
            }
        }
	}
}

void updateScoreDisplay(){
	sprintf(str_score,"%d",score);
	VDP_clearText(1,2,3);
	VDP_drawText(str_score,10,2);
}

int main()
{
	//Input
	JOY_init();
	JOY_setEventHandler( &myJoyHandler );

    VDP_setPaletteColor(0,RGB24_TO_VDPCOLOR(0x6dc2ca)); // Set background color to blue
    VDP_setPalette(PAL2, runner.palette->data); // player palette

    VDP_setPlanSize(32,32); //We’re setting the size of the planes in tiles. Valid combinations are 32x32, 32x64, 64x64, or 32x128
    VDP_setScrollingMode(HSCROLL_PLANE,VSCROLL_PLANE);
    VDP_loadTileSet(floor.tileset,1,DMA);
    VDP_loadTileSet(wall.tileset,2,DMA);
    VDP_loadTileSet(light.tileset,3,DMA);
    VDP_setPalette(PAL1, light.palette->data);
    VDP_fillTileMapRect(BG_B, TILE_ATTR_FULL(PAL1,0,FALSE,FALSE,1),0,16,32,1); //adds a row of floor tiles at tile-position (1:16) that covers the plane from left to right
    VDP_fillTileMapRect(BG_B, TILE_ATTR_FULL(PAL1,0,FALSE,TRUE,2),0,17,32,14); //adds wall-tiles below it that fill out the bottom part of the screen

    VDP_loadTileSet(light.tileset,3,DMA);
    VDP_fillTileMapRectInc(BG_B,TILE_ATTR_FULL(PAL1,0,FALSE,FALSE,3),15,13,2,3);

    SPR_init(0,0,0);
    player = SPR_addSprite(&runner,player_x,fix16ToInt(player_y),TILE_ATTR(PAL2,0,FALSE,FALSE));
    SPR_setAnim(player,ANIM_RUN);

    obstacle = SPR_addSprite(&rock,obstacle_x,128,TILE_ATTR(PAL2,0,FALSE,FALSE));

    SPR_update();
	showText(msg_start);

    int offset = 0; // scrolling keep track variable
	while(1)
	{
		if(game_on == TRUE){
            VDP_setHorizontalScroll(BG_B, offset -= scrollspeed);
            if(offset <= -256) offset = 0;

            //Apply velocity
            player_y = fix16Add(player_y, player_vel_y);

            //Move the obstacle
            obstacle_vel_x = -scrollspeed;
            obstacle_x = obstacle_x + obstacle_vel_x;
            if(obstacle_x < -8) obstacle_x = 320;
            SPR_setPosition(obstacle,obstacle_x,120);

            SPR_setPosition(player,player_x,fix16ToInt(player_y));
            
            //Apply gravity
            if(jumping == TRUE) player_vel_y = fix16Add(player_vel_y, gravity);
            //Check if player is on floor
            if(jumping == TRUE && fix16ToInt(player_y)+player_height >= (floor_height)){
                jumping = FALSE;
                player_vel_y = FIX16(0);
                player_y = intToFix16(floor_height-player_height);
                SPR_setAnim(player,ANIM_RUN);
                score_added = FALSE;
            }

            if(player_x < obstacle_x + 8 && player_x + 8 > obstacle_x){
                if(jumping == FALSE){
                    endGame();
                } else {
                    if(score_added == FALSE){
                        score++;
                        updateScoreDisplay();
                        score_added = TRUE;
                    }
                }
            }

            SPR_update();
		}

		SYS_doVBlankProcess();
	}

	return 0;
}
