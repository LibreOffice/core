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

#include "sal/config.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Intrinsic.h>

#include "jni.h"

// Workaround for problematic IBM JDK 1.6.0 on ppc
#ifndef _JNI_IMPORT_OR_EXPORT_
#define _JNI_IMPORT_OR_EXPORT_
#endif

#include "jawt_md.h"
#include "jawt.h"

#include "sal/types.h"

#define SYSTEM_XWINDOW 6

SAL_DLLPUBLIC_EXPORT jlong JNICALL Java_com_sun_star_comp_beans_LocalOfficeWindow_getNativeWindow
  (JNIEnv * env, jobject obj_this);

/*****************************************************************************/
/*
 * Class:     com_sun_star_comp_beans_LocalOfficeWindow
 * Method:    getNativeWindowSystemType
 * Signature: ()I
 */
SAL_DLLPUBLIC_EXPORT jint JNICALL Java_com_sun_star_beans_LocalOfficeWindow_getNativeWindowSystemType
  (JNIEnv * env, jobject obj_this)
{
    (void) env; /* avoid warning about unused parameter */
    (void) obj_this; /* avoid warning about unused parameter */
    return (SYSTEM_XWINDOW);
}


/*****************************************************************************/
/*
 * Class:     com_sun_star_beans_LocalOfficeWindow
 * Method:    getNativeWindow
 * Signature: ()J
 */
SAL_DLLPUBLIC_EXPORT jlong JNICALL Java_com_sun_star_beans_LocalOfficeWindow_getNativeWindow
  (JNIEnv * env, jobject obj_this)
{
    (void) env; /* avoid warning about unused parameter */
    (void) obj_this; /* avoid warning about unused parameter */
    return Java_com_sun_star_comp_beans_LocalOfficeWindow_getNativeWindow(env, obj_this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
