#include <stdio.h>
#include <time.h>


#include "lua.h"
#include <lauxlib.h>
#include <lualib.h>

#include <SDL2/SDL.h>

#include <emscripten.h>

//#include "functions/lsdl.h"

#pragma region SDL_LIBS

static SDL_Renderer *renderer;
static SDL_Window *window;

static lua_State *lua;

// Created a variable for calling later in emscripten's loop.
static char *func_name;

void SDL_Loop ();

static int CreateWindowAndRenderer (lua_State *L)
{
    if (!renderer && !window)
        SDL_CreateWindowAndRenderer(800, 600, 0, &window, &renderer);

    return 0;
}

static int SetRenderDrawColor (lua_State *L)
{
    if (lua_gettop(L) < 4) {
        return luaL_error(L, "expecting 4 parameters but given less.");
    }

    int r = lua_tointeger(L, 1);
    int g = lua_tointeger(L, 2);
    int b = lua_tointeger(L, 3);
    int a = lua_tointeger(L, 4);

    SDL_SetRenderDrawColor(renderer, r, g, b, a);

    return 0;
}

static int DestroyRenderer (lua_State *L)
{
    SDL_DestroyRenderer (renderer);

    return 0;
}

static int RenderFillRect (lua_State *L)
{
    if (lua_gettop(L) < 4) {
        return luaL_error(L, "expecting 4 parameters but given less.");
    }

    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    int w = lua_tointeger(L, 3);
    int h = lua_tointeger(L, 4);

    SDL_Rect r;

    r.x = x;
    r.y = y;
    r.w = w;
    r.h = h;

    SDL_RenderFillRect(renderer, &r);

    return 0;
}

static int DestroyWindow (lua_State *L)
{
    SDL_DestroyWindow (window);

    return 0;
}

static int RenderPresent (lua_State *L)
{
    SDL_RenderPresent (renderer);

    return 0;
}

static int RenderClear (lua_State *L)
{
    SDL_RenderClear(renderer);

    return 0;
}


static int Init (lua_State *L)
{
    SDL_Init(SDL_INIT_VIDEO);

    return 0;
}

static int Quit (lua_State *L)
{
    SDL_Quit ();

    return 0;
}

#pragma endregion SDL_LIBS

#pragma region SDL_CUSTOM

static int Finish (lua_State *L)
{
    SDL_DestroyRenderer (renderer);
    SDL_DestroyWindow (window);

    SDL_Quit ();

    return 0;
}


static int MainLoop (lua_State *L)
{
    if (lua_gettop(L) <= 0) {
        return luaL_error(L, "expecting at least one parameter.");
    }

    emscripten_cancel_main_loop ();
    emscripten_set_main_loop (SDL_Loop, -1, 0);


    if (func_name) {
        free (func_name);
    }


    const char *lfunc = lua_tostring(L, lua_gettop(L));


    func_name = (char *)malloc(strlen(lfunc) + 1);


    strcpy(func_name, lfunc);

    return 0;
}



void SDL_Loop ()
{
    if (lua) {

        if (func_name) {

            lua_getglobal(lua, func_name);

            lua_pcall(lua, 0, 0, 0);

        }

    }

    SDL_RenderPresent (renderer);
}

#pragma endregion SDL_CUSTOM

static int Randomizer (lua_State *L)
{
    int min = lua_tointeger(L, 1);
    int max = lua_tointeger(L, 2);

    int rnum = min + rand() % (max + 1 - min);

    lua_pushnumber(L, rnum);

    return 1;
}

static int print_async (lua_State* L)
{
	size_t len = 0;
	const char* msg = lua_tolstring(L, 1, &len);

	printf("%s\n", msg);

	return 0;
}

/*
static int l_os_sleep (lua_State *L)
{
	int ms = lua_tonumber(L, 1);


	#ifdef _WIN32
 	Sleep(ms);
  	#else
  	usleep(ms * 1000); 
  	#endif
	
	return 0;
}
*/

int run_lua(const char* script) 
{
    if (!lua) {
        srand (time(NULL));

	    lua = luaL_newstate();

	    luaL_openlibs(lua);

	    //lua_register(lua, "os_sleep", l_os_sleep);
	    lua_register(lua, "print_async", print_async);

	    lua_register(lua, "SDL_CreateWindowAndRenderer", CreateWindowAndRenderer);
	    lua_register(lua, "SDL_SetRenderDrawColor", SetRenderDrawColor);
	    lua_register(lua, "SDL_DestroyRenderer", DestroyRenderer);
	    lua_register(lua, "SDL_RenderFillRect", RenderFillRect);
	    lua_register(lua, "SDL_DestroyWindow", DestroyWindow);
	    lua_register(lua, "SDL_RenderPresent", RenderPresent);
	    lua_register(lua, "SDL_RenderClear", RenderClear);
	    lua_register(lua, "SDL_Init", Init);
	    lua_register(lua, "SDL_Quit", Quit);

        lua_register(lua, "SDL_Finish", Finish);

        lua_register(lua, "SDL_Loop", MainLoop);

        lua_register(lua, "Random", Randomizer);
    }
	

	int res = luaL_dostring(lua, script);

	size_t len = 0;
	const char* value = lua_tolstring(lua, lua_gettop(lua), &len);

	printf("%s\n", value);

	//lua_close(lua);

	return 0;
}