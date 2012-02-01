/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Intrinsic.h>

#include "jawt.h"
#include "jawt_md.h"
#include "nativeview.h"

#define MY_ASSERT(X,S) if (!X) { fprintf(stderr,S); return 0L;}

#define SYSTEM_WIN32   1
#define SYSTEM_WIN16   2
#define SYSTEM_JAVA    3
#define SYSTEM_OS2     4
#define SYSTEM_MAC     5
#define SYSTEM_XWINDOW 6

/*****************************************************************************/
/*
 * Class:     NativeView
 * Method:    getNativeWindowSystemType
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_NativeView_getNativeWindowSystemType
  (JNIEnv * env, jobject obj_this)
{
    return (SYSTEM_XWINDOW);
}

/*****************************************************************************/
/*
 * Class:     NativeView
 * Method:    getNativeWindow
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_NativeView_getNativeWindow
  (JNIEnv * env, jobject obj_this)
{
    jboolean                    result  ;
    jint                        lock    ;
    JAWT                        awt     ;
    JAWT_DrawingSurface*        ds      ;
    JAWT_DrawingSurfaceInfo*    dsi     ;
    JAWT_X11DrawingSurfaceInfo* dsi_x11 ;
    Drawable                    drawable;
    Display*                    display ;

    /* Get the AWT */
    awt.version = JAWT_VERSION_1_3;
    result      = JAWT_GetAWT(env, &awt);
    MY_ASSERT(result != JNI_FALSE,"wrong jawt version");

    /* Get the drawing surface */
    if ((ds = awt.GetDrawingSurface(env, obj_this)) == NULL)
        return 0L;

    /* Lock the drawing surface */
    lock = ds->Lock(ds);
    MY_ASSERT((lock & JAWT_LOCK_ERROR)==0,"can't lock the drawing surface");

    /* Get the drawing surface info */
    dsi = ds->GetDrawingSurfaceInfo(ds);

    /* Get the platform-specific drawing info */
    dsi_x11  = (JAWT_X11DrawingSurfaceInfo*)dsi->platformInfo;
    drawable = dsi_x11->drawable;
    display  = dsi_x11->display;

    /* Free the drawing surface info */
    ds->FreeDrawingSurfaceInfo(dsi);
    /* Unlock the drawing surface */
    ds->Unlock(ds);
    /* Free the drawing surface */
    awt.FreeDrawingSurface(ds);

    return ((jlong)drawable);
}
