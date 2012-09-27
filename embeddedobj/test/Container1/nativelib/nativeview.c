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

#ifdef WNT

#include <windows.h>
// property name to register own window procedure on hwnd
#define OLD_PROC_KEY "oldwindowproc"
// signature of this window procedure
static LRESULT APIENTRY NativeViewWndProc( HWND , UINT , WPARAM , LPARAM );

#endif

#include "jawt.h"
#include "jawt_md.h"
#include "NativeView.h"

#define MY_ASSERT(X,S) if (!X) { fprintf(stderr,"%s\n",S); return 0L;}

#define SYSTEM_WIN32   1
#define SYSTEM_WIN16   2
#define SYSTEM_JAVA    3
#define SYSTEM_MAC     5
#define SYSTEM_XWINDOW 6

/*****************************************************************************
 *
 * Class      : NativeView
 * Method     : getNativeWindowSystemType
 * Signature  : ()I
 * Description: returns an identifier for the current operating system
 */
JNIEXPORT jint JNICALL Java_embeddedobj_test_NativeView_getNativeWindowSystemType
  (JNIEnv * env, jobject obj_this)
{
    return (SYSTEM_WIN32);
}

/*****************************************************************************
 *
 * Class      : NativeView
 * Method     : getNativeWindow
 * Signature  : ()J
 * Description: returns the native systemw window handle of this object
 */
JNIEXPORT jlong JNICALL Java_embeddedobj_test_NativeView_getNativeWindow
  (JNIEnv * env, jobject obj_this)
{
    jboolean                      result  ;
    jint                          lock    ;
    JAWT                          awt     ;
    JAWT_DrawingSurface*          ds      ;
    JAWT_DrawingSurfaceInfo*      dsi     ;
#ifdef WNT
    JAWT_Win32DrawingSurfaceInfo* dsi_win ;
#else
    // FIXME: Where is dsi_x11 defined?
    // Added below because I'm guessing this test breaks
    //
    // JAWT_X11DrawingSurfaceInfo*dsi_x11 ;
#endif
    jlong                         drawable;

    /* Get the AWT */
    awt.version = JAWT_VERSION_1_3;
    result      = JAWT_GetAWT(env, &awt);
    MY_ASSERT(result!=JNI_FALSE,"wrong jawt version");

    /* Get the drawing surface */
    if ((ds = awt.GetDrawingSurface(env, obj_this)) == NULL)
        return 0L;

    /* Lock the drawing surface */
    lock = ds->Lock(ds);
    MY_ASSERT((lock & JAWT_LOCK_ERROR)==0,"can't lock the drawing surface");

    /* Get the drawing surface info */
    dsi = ds->GetDrawingSurfaceInfo(ds);

    /* Get the platform-specific drawing info */
#ifdef WNT
    dsi_win  = (JAWT_Win32DrawingSurfaceInfo*)dsi->platformInfo;
    drawable = (jlong)dsi_win->hwnd;
#else
    dsi_x11  = (JAWT_X11DrawingSurfaceInfo*)dsi->platformInfo;
    drawable = (jlong)dsi_x11->drawable;
#endif

    /* Free the drawing surface info */
    ds->FreeDrawingSurfaceInfo(dsi);
    /* Unlock the drawing surface */
    ds->Unlock(ds);
    /* Free the drawing surface */
    awt.FreeDrawingSurface(ds);

    return drawable;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
