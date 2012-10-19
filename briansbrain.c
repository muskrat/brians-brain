/* Brian's Brain
 * Cellular Automata: a Discrete View of the World, pp. 110
 *
 * There are three cell states: firing, refractory, and ready. The rules
 * bear a vague resemblance to how neurons in the brain behave - 
 * 
 * 1. A cell fires only if it is in the ready state and exactly two of
 * its moore neighbours are firing.
 *
 * 2. Every iteration, a firing cell changes to the refractory state and
 * and a refractory cell reverts to the ready state.
 *
 * (c) I K Stead, 20-09-2012
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <SDL/SDL.h>

#define MAX_X       300
#define MAX_Y       300
#define WAIT        100000  /* Microseconds to wait between iterations */
#define CELLSIZE    2      /* Width/height of cells in pixels */
#define LINE        1       /* Pixel width of line between cells */
#define DENSITY     7       /* Starting density of firing cells */
#define DEPTH       32      /* Colour depth */

enum {READY, REFRACTORY, FIRING};   /* Automaton cell states */

/* Generate a random integer between 0 and 1 */
int randint()
{
    int r = rand() % 2;
    return r;
}

/* Choose 0 or 1 at random with weighting */
int weighted_randint(int true_weight)
{
    int choice = rand() % 10;  /* Take last digit of random int */
    
    if (choice > true_weight)
        return 1;
    else
        return 0;
}

/* Count the number of firing neighbours of a given (ready) cell */
int count_neighbours(int world[MAX_X][MAX_Y], int x_pos, int y_pos)
{
    int x, y, cx, cy, cell;
    int count = 0;
    /* Iterate through neighbouring cells */
    for (y = -1; y < 2; y++) {
        for (x = -1; x < 2; x++) {
            cx = x_pos + x;
            cy = y_pos + y;
            /* Check that current x and y indices aren't out of bounds */
            if ( (0 <= cx && cx < MAX_X) && (0 <= cy && cy < MAX_Y)) {
                cell = world[x_pos + x][y_pos + y];
                if (cell == FIRING)
                    count ++;
            }
        }
    }
    return count;
}

/* Advance game one step: apply rules to all cells in source */
void apply_rules(int world[MAX_X][MAX_Y])
{
    int x, y, cell, neighbours;
    /* Create a temporary copy of world to iterate through. This is so
     * the state of the world isn't changing as it's being checked.
     */
    int temp[MAX_X][MAX_Y];
    memcpy(temp, world, sizeof(temp));

    for (y = 0; y < MAX_X; y++) {
        for (x = 0; x < MAX_Y; x++){
            cell = temp[x][y];
            
            /* Apply first rule */
            if (cell == READY) {
                neighbours = count_neighbours(temp, x, y);
                if (neighbours == 2)
                    world[x][y] = FIRING;
            }
            /* Apply second rule */
            else if (cell == FIRING)
                world[x][y] = REFRACTORY;
            else
                world[x][y] = READY;
        }
    }
}
/* Randomise world state. Set  */
void populate(int world[MAX_X][MAX_Y], int rand)
{
    int x, y, r;
    for (y = 0; y < MAX_X; y++) {
        for (x = 0; x < MAX_Y; x++){
            if (rand == 1) {
                r = weighted_randint(DENSITY);
                if (r == 1)
                    world[x][y] = FIRING;
            }
            else
                world[x][y] = READY;
        }
    }
}
/* Print world array to terminal, followed by a newline. */
void print_world(int world[MAX_X][MAX_Y])
{
    int x, y;
    char dc;
    /* Print out array values */
    for (y = 0; y < MAX_X; y++) {
        for (x = 0; x < MAX_Y; x++){
            if (world[x][y] == FIRING)
                dc = '@';
            else if (world[x][y] == REFRACTORY)
                dc = '*';
            else 
                dc = '`';
            printf("%c ", dc); 
        }
        printf("\n");
    }
    printf("\n");
}

/* Draw a rectangle of specified size to the screen for each cell
 * in given world array.
 */
void draw_world_rects(SDL_Surface *surface, int world[MAX_X][MAX_Y], int cellsize, int linew)
{
    /* Map colour white to the display */
    Uint32 blue = SDL_MapRGB(surface->format, 0x0, 0x0, 0xff);
    Uint32 green = SDL_MapRGB(surface->format, 0x0, 0xff, 0x0);
    Uint32 red = SDL_MapRGB(surface->format, 0xff, 0x0, 0x0);

    int x, y;
    for (y = 0; y < MAX_Y; y++) {
        for (x = 0; x < MAX_X; x++) {
            /* Cell rectangle; Multiply array indices to get top left,
             * subtract linewidth from dimensions for line between 
             * cells.
             */
            SDL_Rect cell = {x*cellsize, y*cellsize, cellsize-linew, cellsize-linew};
            if (world[x][y] == FIRING) 
                SDL_FillRect(surface, &cell, green);
            else if (world[x][y] == REFRACTORY)
                SDL_FillRect(surface, &cell, blue);
        }
    }
}

int main()
{
    /* Seed random number generator. Seeded with system time so a
     * different random sequence is produced each time
     */
    srand(time(0));
    
    int world[MAX_X][MAX_Y];
    populate(world, 1); /* Initialise world array with random values */

    /* Set up SDL */
    SDL_Surface *screen;
    SDL_Event event;
    int user_exit;
    
    SDL_Init(SDL_INIT_VIDEO);
    
    /* Initialise screen with size dependent on cell size. TODO: Either
     * remove pixel drawing function or add code to allow easy selection
     * between rect and pixel drawing methods.
     */
    screen = SDL_SetVideoMode(MAX_X*CELLSIZE, MAX_Y*CELLSIZE, DEPTH, SDL_SWSURFACE);

    /* Main loop */
    while (!user_exit) {
        SDL_FillRect(screen, NULL, 0); /* Blank out screen */
        apply_rules(world);
        draw_world_rects(screen, world, CELLSIZE, LINE);
        SDL_Flip(screen);   /* Updates SDL window */
        
        /* Check for user input, quit if required */
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    user_exit = 1;
                    break;
                case SDL_KEYDOWN:
                    user_exit = 1;
                    break;
            }
        }
        usleep(WAIT);
    }
    SDL_Quit();
    return EXIT_SUCCESS;
}
