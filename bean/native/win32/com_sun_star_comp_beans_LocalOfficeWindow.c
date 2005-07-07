/*************************************************************************
 *
 *  $RCSfile: com_sun_star_comp_beans_LocalOfficeWindow.c,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2005-07-07 13:17:46 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <windows.h>

#include "jawt.h"
#include "jawt_md.h"

#define SYSTEM_WIN32   1
#define SYSTEM_WIN16   2
#define SYSTEM_JAVA    3
#define SYSTEM_OS2     4
#define SYSTEM_MAC     5
#define SYSTEM_XWINDOW 6

#define OLD_PROC_KEY "oldwindowproc"

static LRESULT APIENTRY OpenOfficeWndProc( HWND , UINT , WPARAM , LPARAM );



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
JNIEXPORT jint JNICALL Java_com_sun_star_comp_beans_LocalOfficeWindow_getNativeWindowSystemType
  (JNIEnv * env, jobject obj_this)
{
    return (SYSTEM_WIN32);
}


/*****************************************************************************/
/*
 * Class:     com_sun_star_comp_beans_LocalOfficeWindow
 * Method:    getNativeWindow
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_sun_star_comp_beans_LocalOfficeWindow_getNativeWindow
  (JNIEnv * env, jobject obj_this)
{
    jboolean result;
    jint lock;

    JAWT awt;
    JAWT_DrawingSurface* ds;
    JAWT_DrawingSurfaceInfo* dsi;
    JAWT_Win32DrawingSurfaceInfo* dsi_win;
    HDC hdc;
    HWND hWnd;
    LONG hFuncPtr;

    /* Get the AWT */
    awt.version = JAWT_VERSION_1_3;
    result = JAWT_GetAWT(env, &awt);
    if (result == JNI_FALSE)
        ThrowException(env, "java/lang/RuntimeException", "JAWT_GetAWT failed");

                                /* Get the drawing surface */
    if ((ds = awt.GetDrawingSurface(env, obj_this)) == NULL)
        return 0L;

    /* Lock the drawing surface */
    lock = ds->Lock(ds);
    if ( (lock & JAWT_LOCK_ERROR) != 0)
        ThrowException(env, "java/lang/RuntimeException",
                       "Could not get AWT drawing surface.");

    /* Get the drawing surface info */
    dsi = ds->GetDrawingSurfaceInfo(ds);

    /* Get the platform-specific drawing info */
    dsi_win = (JAWT_Win32DrawingSurfaceInfo*)dsi->platformInfo;

    hdc = dsi_win->hdc;

    hWnd = dsi_win->hwnd;

    /* Free the drawing surface info */
    ds->FreeDrawingSurfaceInfo(dsi);
    /* Unlock the drawing surface */
    ds->Unlock(ds);
    /* Free the drawing surface */
    awt.FreeDrawingSurface(ds);

    /* Register own window procedure
       Do it one times only! Otherwhise
       multiple instances will be registered
       and calls on such construct produce
       a stack overflow.
     */
    if (GetProp( hWnd, OLD_PROC_KEY )==0)
    {
        hFuncPtr = SetWindowLong( hWnd, GWL_WNDPROC, (DWORD)OpenOfficeWndProc );
        SetProp( hWnd, OLD_PROC_KEY, (HANDLE)hFuncPtr );
    }

    return ((jlong)hWnd);
}


static LRESULT APIENTRY OpenOfficeWndProc(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_PARENTNOTIFY: {
            if (wParam == WM_CREATE) {
                RECT rect;
                HWND hChild = (HWND) lParam;

                GetClientRect(hWnd, &rect);

                SetWindowPos(hChild,
                             NULL,
                             rect.left,
                             rect.top,
                             rect.right - rect.left,
                             rect.bottom - rect.top,
                             SWP_NOZORDER);
            }
            break;
        }
        case WM_SIZE: {
            WORD newHeight = HIWORD(lParam);
            WORD newWidth = LOWORD(lParam);
            HWND hChild = GetWindow(hWnd, GW_CHILD);

            if (hChild != NULL) {
                SetWindowPos(hChild, NULL, 0, 0, newWidth, newHeight, SWP_NOZORDER);
            }
            break;
        }
    }

    return CallWindowProc(GetProp(hWnd, OLD_PROC_KEY),
                          hWnd, uMsg, wParam, lParam);
}










