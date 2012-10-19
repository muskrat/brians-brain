all:
	clang `sdl-config --cflags --libs` -o brian briansbrain.c
