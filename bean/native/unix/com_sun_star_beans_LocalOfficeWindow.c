/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: com_sun_star_beans_LocalOfficeWindow.c,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2007-10-04 08:13:09 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Intrinsic.h>

#include "jni.h"
#include "jawt_md.h"
#include "jawt.h"

/*#include "../inc/com_sun_star_comp_beans_LocalOfficeWindow.h"*/

#if defined assert
#undef assert
#endif

#define assert(X) if (!X) { (*env)->ThrowNew(env, (*env)->FindClass(env, "java/lang/RuntimeException"), "assertion failed"); return;}


#define SYSTEM_WIN32   1
#define SYSTEM_WIN16   2
#define SYSTEM_JAVA    3
#define SYSTEM_OS2     4
#define SYSTEM_MAC     5
#define SYSTEM_XWINDOW 6

JNIEXPORT jlong JNICALL Java_com_sun_star_comp_beans_LocalOfficeWindow_getNativeWindow
  (JNIEnv * env, jobject obj_this);

/*****************************************************************************/
/*
 * Class:     com_sun_star_comp_beans_LocalOfficeWindow
 * Method:    getNativeWindowSystemType
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_sun_star_beans_LocalOfficeWindow_getNativeWindowSystemType
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
JNIEXPORT jlong JNICALL Java_com_sun_star_beans_LocalOfficeWindow_getNativeWindow
  (JNIEnv * env, jobject obj_this)
{
    (void) env; /* avoid warning about unused parameter */
    (void) obj_this; /* avoid warning about unused parameter */
    return Java_com_sun_star_comp_beans_LocalOfficeWindow_getNativeWindow(env, obj_this);
}











