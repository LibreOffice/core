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

#include <sal/config.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Intrinsic.h>

// to deal with gcc specific attribute "externally_visible" used in
// /usr/lib/jvm/java-11-openjdk-armhf/include/linux/jni_md.h:35
// via /usr/lib/jvm/java-11-openjdk-armhf/include/jni.h
#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-attributes"
#endif
#include <jni.h>

#include <jawt_md.h>
#include <jawt.h>
#if defined __clang__
#pragma clang diagnostic pop
#endif

#include <sal/types.h>

#define SYSTEM_XWINDOW 6


/* type must be something like java/lang/RuntimeException
 */
static void ThrowException(JNIEnv * env, char const * type, char const * msg) {
    jclass c;
    (*env)->ExceptionClear(env);
    c = (*env)->FindClass(env, type);
    if (c == NULL) {
        (*env)->ExceptionClear(env);
        (*env)->FatalError(
            env, "JNI FindClass failed");
    }
    if ((*env)->ThrowNew(env, c, msg) != 0) {
        (*env)->ExceptionClear(env);
        (*env)->FatalError(env, "JNI ThrowNew failed");
    }
}

/*****************************************************************************/
/*
 * Class:     com_sun_star_comp_beans_LocalOfficeWindow
 * Method:    getNativeWindowSystemType
 * Signature: ()I
 */
SAL_DLLPUBLIC_EXPORT jint JNICALL Java_com_sun_star_comp_beans_LocalOfficeWindow_getNativeWindowSystemType
  (JNIEnv * env, jobject obj_this)
{
    (void) env; /* avoid warning about unused parameter */
    (void) obj_this; /* avoid warning about unused parameter */
    return SYSTEM_XWINDOW;
}


/*****************************************************************************/
/*
 * Class:     com_sun_star_beans_LocalOfficeWindow
 * Method:    getNativeWindow
 * Signature: ()J
 */
SAL_DLLPUBLIC_EXPORT jlong JNICALL Java_com_sun_star_comp_beans_LocalOfficeWindow_getNativeWindow
  (JNIEnv * env, jobject obj_this)
{
    jboolean result;
    jint lock;

    JAWT awt;
    JAWT_DrawingSurface* ds;
    JAWT_DrawingSurfaceInfo* dsi;
    JAWT_X11DrawingSurfaceInfo* dsi_x11;

    Drawable drawable;

    /* Get the AWT */
    awt.version = JAWT_VERSION_1_3;
    result = JAWT_GetAWT(env, &awt);
    if (!result)
        ThrowException(env, "java/lang/RuntimeException", "JAWT_GetAWT failed");

    /* Get the drawing surface */
    ds = awt.GetDrawingSurface(env, obj_this);
    if (ds == NULL)
        return 0;

    /* Lock the drawing surface */
    lock = ds->Lock(ds);
    if ( (lock & JAWT_LOCK_ERROR) != 0)
        ThrowException(env, "java/lang/RuntimeException",
                       "Could not get AWT drawing surface.");

    /* Get the drawing surface info */
    dsi = ds->GetDrawingSurfaceInfo(ds);

    /* Get the platform-specific drawing info */
    dsi_x11 = (JAWT_X11DrawingSurfaceInfo*)dsi->platformInfo;

    drawable = dsi_x11->drawable;

    /* Free the drawing surface info */
    ds->FreeDrawingSurfaceInfo(dsi);
    /* Unlock the drawing surface */
    ds->Unlock(ds);
    /* Free the drawing surface */
    awt.FreeDrawingSurface(ds);

    return (jlong)drawable;
}











/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
