/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <dlfcn.h>
#include <stdlib.h>

typedef int Window;
typedef union  _GdkEvent GdkEvent;
typedef struct _GdkWindow GdkWindow;
typedef struct _GdkDisplay GdkDisplay;
typedef struct _GdkScreen GdkScreen;

typedef enum
{
    GDK_FULLSCREEN_ON_CURRENT_MONITOR,
    GDK_FULLSCREEN_ON_ALL_MONITORS
} GdkFullscreenMode;

int main(int argc, char *argv[])
{
    void *handle;
    void (*gtk_init)(int*, char***);
    GdkWindow* (*gdk_x11_window_foreign_new_for_display)(GdkDisplay*, Window);
    GdkDisplay* (*gdk_display_get_default)(void);
    GdkEvent* (*gdk_event_get)(void);
    void (*gtk_main_do_event)(GdkEvent*);
    void (*gdk_event_free)(GdkEvent*);
    void (*gdk_window_fullscreen)(GdkWindow *);
    void (*gdk_window_set_fullscreen_mode)(GdkWindow *, GdkFullscreenMode);

    GdkEvent *event;
    GdkWindow *window;
    int windowid;

    handle = dlopen("libgtk-3.so.0", RTLD_LAZY);
    if( NULL == handle )
        return -1;

    gtk_init = (void (*) (int*, char***))
        dlsym(handle, "gtk_init");
    gdk_x11_window_foreign_new_for_display = (GdkWindow* (*)(GdkDisplay*, Window))
        dlsym(handle, "gdk_x11_window_foreign_new_for_display");
    gdk_display_get_default = (GdkDisplay* (*)(void))
        dlsym(handle, "gdk_display_get_default");
    gdk_event_get = (GdkEvent* (*)(void))
        dlsym(handle, "gdk_event_get");
    gtk_main_do_event = (void (*)(GdkEvent*))
        dlsym(handle, "gtk_main_do_event");
    gdk_event_free = (void (*)(GdkEvent*))
        dlsym(handle, "gdk_event_free");
    gdk_window_fullscreen = (void (*)(GdkWindow *))
        dlsym(handle, "gdk_window_fullscreen");
    gdk_window_set_fullscreen_mode = (void (*)(GdkWindow *, GdkFullscreenMode))
        dlsym(handle, "gdk_window_set_fullscreen_mode");

    if (!gtk_init ||
        !gdk_x11_window_foreign_new_for_display ||
        !gdk_display_get_default ||
        !gdk_event_get ||
        !gtk_main_do_event ||
        !gdk_event_free ||
        !gdk_window_fullscreen ||
        !gdk_window_set_fullscreen_mode)
    {
        dlclose(handle);
        return -1;
    }

    gtk_init(&argc, &argv);

    windowid = atoi(argv[1]);

    window = gdk_x11_window_foreign_new_for_display(gdk_display_get_default(), windowid);
    if (!window)
    {
        dlclose(handle);
        return -1;
    }

    gdk_window_set_fullscreen_mode(window, GDK_FULLSCREEN_ON_ALL_MONITORS);
    gdk_window_fullscreen(window);

    while ((event = gdk_event_get()) != NULL)
    {
        gtk_main_do_event(event);
        gdk_event_free(event);
    }

    dlclose(handle);
    return 0;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

