#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xmu/WinUtil.h>
#include <X11/extensions/scrnsaver.h>


#define INACTIVE 180000 	// 3 minutes

Bool xerror = False;

static char*  application;
static time_t active_start, active_stop;
static Bool   clock_on = False;


char* getFocusedWindowClass( Display* d, Window w ); 


void writeActivityToFile(char* fname)
{
	FILE *f = fopen(fname, "a");
	fprintf(f, "%s %ld\n", application, active_stop - active_start);
	fclose(f);
}


void onUserActivity() 
{
	if (!clock_on) {
		clock_on = True;
		active_start = time(NULL);
	}
}


void onUserInactivity(char* fname) 
{
	active_stop = time(NULL);
	writeActivityToFile(fname);
}


char* onActiveWindowChange(Display* d, Window w, char* fname)
{
	active_stop = time(NULL);
	writeActivityToFile(fname);
	return getFocusedWindowClass(d, w);

}



int main( int argc, char *argv[] ) 
{		
	if ( argc < 2 ) {
		printf("Need a db\n");
		return -1;
	}
		
	Display 		*display;
	Window   		 window;
	XEvent   		 event;

	display = 		 XOpenDisplay(NULL);
	XScreenSaverInfo *info = XScreenSaverAllocInfo();
	application = 		 getFocusedWindowClass(display, window);
	clock_on = 		 True;
	active_start = 		 time(NULL);
	
	// start event loop
	while (1) {
		XScreenSaverQueryInfo(display, DefaultRootWindow(display), info);
		
		// if user focuses on another application
		if (strcmp(application, getFocusedWindowClass(display, window)) != 0) {
			application = onActiveWindowChange(display, window, argv[1]);
			active_start = time(NULL);
		}
		if (info->idle >= INACTIVE && clock_on == True) { /// if idle for >= INACTIVE milliseconds
			onUserInactivity(argv[1]);
			clock_on = False;
		} else if (info->idle < INACTIVE && clock_on == False) {
			onUserActivity();
		}
	}
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

	// return window class
	class = XAllocClassHint();
	stat = XGetClassHint(d, w, class);
	
	// I dunno why, but this ONLY happens w spotify :/
	if (class->res_name == 0) 
		return "spotify";
	return class->res_name;
}
