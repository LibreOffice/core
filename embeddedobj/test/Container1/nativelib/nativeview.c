/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: nativeview.c,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 18:47:33 $
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
 *************************************************************************/

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
#define SYSTEM_OS2     4
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
    JAWT_Win32DrawingSurfaceInfo* dsi_win ;
    jlong                         drawable;

#if 0
    LONG                          hFuncPtr;
#endif

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

#if 0
    /* Register own window procedure
       Do it one times only! Otherwhise
       multiple instances will be registered
       and calls on such construct produce
       a stack overflow.
     */

    if (GetProp( (HWND)drawable, OLD_PROC_KEY )==0)
    {
        hFuncPtr = SetWindowLong( (HWND)drawable, GWL_WNDPROC, (DWORD)NativeViewWndProc );
        SetProp( (HWND)drawable, OLD_PROC_KEY, (HANDLE)hFuncPtr );
    }
#endif

    return drawable;
}

#if 0
/*****************************************************************************
 *
 * Class      : -
 * Method     : NativeViewWndProc
 * Signature  : -
 * Description: registered window handler to intercept window messages between
 *              java and office process
 */
static LRESULT APIENTRY NativeViewWndProc(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HANDLE hFuncPtr;

    /* resize new created child window to fill out the java window complete */
    if (uMsg==WM_PARENTNOTIFY)
    {
        if (wParam == WM_CREATE)
        {
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
    }
    /* handle normal resize events */
    else if(uMsg==WM_SIZE)
    {
        WORD newHeight = HIWORD(lParam);
        WORD newWidth  = LOWORD(lParam);
        HWND hChild    = GetWindow(hWnd, GW_CHILD);

        if (hChild != NULL)
            SetWindowPos(hChild, NULL, 0, 0, newWidth, newHeight, SWP_NOZORDER);
    }

    /* forward request to original handler which is intercepted by this window procedure */
    hFuncPtr = GetProp(hWnd, OLD_PROC_KEY);
    MY_ASSERT(hFuncPtr,"lost original window proc handler");
    return CallWindowProc( hFuncPtr, hWnd, uMsg, wParam, lParam);
}
#endif

