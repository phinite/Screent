#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xmu/WinUtil.h>


Bool xerror = False;

char* getFocusedWindowClass(Display* d, Window w);
char* onActiveWindowChange(Display* d, Window w, FILE* f);
void onUserActivity();
void onUserInactivity(FILE* f);
void writeActivityToFile(FILE* f);

static char* application;
static time_t	active_start, active_stop, 
		inactive_start;

static Bool clock_on = False;


int main( int argc, char *argv[] ) 
{		
	if ( argc < 2 ) {
		printf("Need a db\n");
		return -1;
	}
	FILE *fpt = fopen(argv[1], "a");

	

	Display *display;
	Window window;
	XEvent event;
	

	display = XOpenDisplay(NULL);
	application = getFocusedWindowClass(display, window);
	inactive_start = active_start = time(NULL);
	
	
	// start event loop
	while (1) {
		XNextEvent(display, &event);

		if (event.type == KeyPress || 
		    event.type == KeyRelease ||
		    event.type == MotionNotify ) 
		{
			// if user focuses on another application
			if (application != getFocusedWindowClass(display, window)) {
				application = onActiveWindowChange(display, window, fpt);
				active_start = time(NULL);
			}
			onUserActivity();

		}
		//
		if (time(NULL)-inactive_start >= 30) {
			onUserInactivity(fpt);
			printf("INACTIVE!");
		}
	}
	/*
	window = GetFocusedWindowClass(display);
	printf("%s\n", GetWindowClass(display, window));
	*/
}



char* getFocusedWindowClass( Display* d, Window w ) 
{
	Status stat;
	XClassHint* class;
	int revt;

	// Get focused window
	XGetInputFocus(d, &w, &revt);
	if (xerror) {
		printf("Fail\n");
		exit(-1);
	}

	// return Class name
	class = XAllocClassHint();
	stat = XGetClassHint(d, w, class);
	
	// I dunno why, but this ONLY happens w spotify :/
	if (class->res_name == 0) 
		return "spotify";
	return class->res_name;
}



void onUserActivity() 
{
	if (!clock_on) {
		clock_on = True;
		active_start = time(NULL);
	}
	inactive_start = time(NULL);
}


void onUserInactivity(FILE* f) 
{
	clock_on = False;
	active_stop = time(NULL);
	writeActivityToFile(f);
}

char* onActiveWindowChange(Display* d, Window w, FILE* f)
{
	writeActivityToFile(f);
	return getFocusedWindowClass(d, w);

}

void writeActivityToFile(FILE* f)
{
	printf("Writing!\n");
	// char* data = (char*)malloc(sizeof(char)*50);
	fprintf(f, "%s %s", application, (char) active_stop - active_start);
}

