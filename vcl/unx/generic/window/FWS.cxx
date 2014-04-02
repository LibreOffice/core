/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include "FWS.hxx"

static Atom fwsIconAtom;

static Atom FWS_CLIENT;
static Atom FWS_COMM_WINDOW;
static Atom FWS_PROTOCOLS;
static Atom FWS_STACK_UNDER;
static Atom FWS_PARK_ICONS;
static Atom FWS_PASS_ALL_INPUT;
static Atom FWS_PASSES_INPUT;
static Atom FWS_HANDLES_FOCUS;

static Atom FWS_REGISTER_WINDOW;
static Atom FWS_STATE_CHANGE;
static Atom FWS_UNSEEN_STATE;
static Atom FWS_NORMAL_STATE;
static Atom WM_PROTOCOLS;
static Atom WM_CHANGE_STATE;

static Bool fwsStackUnder;
static Bool fwsParkIcons;
static Bool fwsPassesInput;
static Bool fwsHandlesFocus;

static Window fwsCommWindow;

/*************************************<->***********************************
 *
 * WMSupportsFWS() -
 *
 * Initialize our atoms and determine if the current window manager is
 * providing FWS extension support.
 *
 *************************************<->***********************************/

Bool
WMSupportsFWS (Display *display, int screen)
{
    unsigned int    i;
    Atom            protocol;
    Atom            propType;
    int             propFormat;
    unsigned long   propItems;
    unsigned long   propBytesAfter;
    unsigned char   *propData;
    char            propName[64];

    FWS_CLIENT         = XInternAtom(display, "_SUN_FWS_CLIENT",         False);
    FWS_COMM_WINDOW    = XInternAtom(display, "_SUN_FWS_COMM_WINDOW",    False);
    FWS_PROTOCOLS      = XInternAtom(display, "_SUN_FWS_PROTOCOLS",      False);
    FWS_STACK_UNDER    = XInternAtom(display, "_SUN_FWS_STACK_UNDER",    False);
    FWS_PARK_ICONS     = XInternAtom(display, "_SUN_FWS_PARK_ICONS",     False);
    FWS_PASS_ALL_INPUT = XInternAtom(display, "_SUN_FWS_PASS_ALL_INPUT", False);
    FWS_PASSES_INPUT   = XInternAtom(display, "_SUN_FWS_PASSES_INPUT",   False);
    FWS_HANDLES_FOCUS  = XInternAtom(display, "_SUN_FWS_HANDLES_FOCUS",  False);
    FWS_REGISTER_WINDOW= XInternAtom(display, "_SUN_FWS_REGISTER_WINDOW",False);
    FWS_STATE_CHANGE   = XInternAtom(display, "_SUN_FWS_STATE_CHANGE",   False);
    FWS_UNSEEN_STATE   = XInternAtom(display, "_SUN_FWS_UNSEEN_STATE",   False);
    FWS_NORMAL_STATE   = XInternAtom(display, "_SUN_FWS_NORMAL_STATE",   False);
    WM_PROTOCOLS       = XInternAtom(display, "WM_PROTOCOLS",            False);
    WM_CHANGE_STATE    = XInternAtom(display, "WM_CHANGE_STATE",         False);

    snprintf (propName, sizeof(propName), "_SUN_FWS_NEXT_ICON_%d", screen);
    fwsIconAtom        = XInternAtom(display, propName, False);

    if (XGetWindowProperty (display, DefaultRootWindow (display),
                FWS_COMM_WINDOW, 0, 1,
                False, AnyPropertyType, &propType,
                &propFormat, &propItems,
                &propBytesAfter, &propData) != Success)
        return False;

    if (propFormat != 32 ||
        propItems != 1 ||
        propBytesAfter != 0)
    {
        #if OSL_DEBUG_LEVEL > 1
        fprintf (stderr, "Bad FWS_COMM_WINDOW property on root window.\n");
        #endif
        XFree (propData);
        return False;
    }

    fwsCommWindow = *(Window *) propData;
    #if OSL_DEBUG_LEVEL > 1
    fprintf (stderr, "Using fwsCommWindow = 0x%lx.\n", fwsCommWindow);
    #endif
    XFree (propData);

    if (XGetWindowProperty (display, DefaultRootWindow (display),
            FWS_PROTOCOLS, 0, 10,
            False, AnyPropertyType, &propType,
            &propFormat, &propItems,
            &propBytesAfter, &propData) != Success)
    {
        return False;
    }

    if (propFormat     != 32 ||
        propBytesAfter != 0)
    {
        #if OSL_DEBUG_LEVEL > 1
        fprintf (stderr, "Bad FWS_PROTOCOLS property on root window.\n");
        #endif
        XFree (propData);
        return False;
    }

    for (i = 0; i < propItems; ++i)
    {
        protocol = ((Atom *) propData)[i];
        if (protocol == FWS_STACK_UNDER)
        {
            fwsStackUnder = True;
            #if OSL_DEBUG_LEVEL > 1
            fprintf (stderr, "Using fwsStackUnder.\n");
            #endif
        }
        else
        if (protocol == FWS_PARK_ICONS)
        {
            fwsParkIcons = True;
            #if OSL_DEBUG_LEVEL > 1
            fprintf (stderr, "Using fwsParkIcons.\n");
            #endif
        }
        else
        if (protocol == FWS_PASSES_INPUT)
        {
            fwsPassesInput = True;
            #if OSL_DEBUG_LEVEL > 1
            fprintf (stderr, "Using fwsPassesInput.\n");
            #endif
        }
        else
        if (protocol == FWS_HANDLES_FOCUS)
        {
            fwsHandlesFocus = True;
            #if OSL_DEBUG_LEVEL > 1
            fprintf (stderr, "Using fwsHandlesFocus.\n");
            #endif
        }
    }

    XFree (propData);
    return True;
}

/*************************************<->***********************************
 *
 * newHandler() -
 *
 * Handle X errors (temporarily) to record the occurrence of BadWindow
 * errors without crashing.  Used to detect the FWS_COMM_WINDOW root window
 * property containing an old or obsolete window id.
 *
 *************************************<->***********************************/

extern "C" {

static Bool badWindowFound;
static int (* oldHandler) (Display *, XErrorEvent *);

static int
newHandler (Display *display, XErrorEvent *xerror)
{
    if (xerror->error_code != BadWindow)
        (*oldHandler)(display, xerror);
    else
        badWindowFound = True;

    return 0;
}

}

/*************************************<->***********************************
 *
 * RegisterFwsWindow() -
 *
 * Send a client message to the FWS_COMM_WINDOW indicating the existance
 * of a new FWS client window.  Be careful to avoid BadWindow errors on
 * the XSendEvent in case the FWS_COMM_WINDOW root window property had
 * old/obsolete junk in it.
 *
 *************************************<->***********************************/

Bool
RegisterFwsWindow (Display *display, Window window)
{
    XClientMessageEvent  msg;

    msg.type         = ClientMessage;
    msg.window       = fwsCommWindow;
    msg.message_type = FWS_REGISTER_WINDOW;
    msg.format       = 32;
    msg.data.l[0]    = window;

    XSync (display, False);
    badWindowFound = False;
    oldHandler = XSetErrorHandler (newHandler);

    XSendEvent (display, fwsCommWindow, False, NoEventMask,
            (XEvent *) &msg);
    XSync (display, False);

    XSetErrorHandler (oldHandler);
    #if OSL_DEBUG_LEVEL > 1
    if (badWindowFound)
        fprintf (stderr, "No FWS client window to register with.\n");
    #endif

    return !badWindowFound;
}

/*************************************<->***********************************
 *
 * AddFwsProtocols -
 *
 * Add the FWS protocol atoms to the WMProtocols property for the window.
 *
 *************************************<->***********************************/

void
AddFwsProtocols (Display *display, Window window)
{
    #define MAX_FWS_PROTOS 10

    Atom fwsProtocols[ MAX_FWS_PROTOS ];
    int  nProtos = 0;

    fwsProtocols[ nProtos++ ] = FWS_CLIENT;
    fwsProtocols[ nProtos++ ] = FWS_STACK_UNDER;
    fwsProtocols[ nProtos++ ] = FWS_STATE_CHANGE;
    fwsProtocols[ nProtos++ ] = FWS_PASS_ALL_INPUT;
    XChangeProperty (display, window, WM_PROTOCOLS,
            XA_ATOM, 32, PropModeAppend,
            (unsigned char *) fwsProtocols, nProtos);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
