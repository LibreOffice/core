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



#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#include "jawt.h"

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include "jawt_md.h"
#if defined _MSC_VER
#pragma warning(pop)
#endif

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
    (void) env; // unused
    (void) obj_this; // unused
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
    LONG_PTR hFuncPtr;

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
        hFuncPtr = SetWindowLongPtr( hWnd, GWLP_WNDPROC, (ULONG_PTR)OpenOfficeWndProc );
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

#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4152) /* function/data pointer conversion: */
#endif
    return CallWindowProc(GetProp(hWnd, OLD_PROC_KEY),
                          hWnd, uMsg, wParam, lParam);
#if defined _MSC_VER
#pragma warning(pop)
#endif
}










