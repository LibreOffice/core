/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: helppopupwindow.hxx,v $
 * $Revision: 1.7 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _HELPPOPUPWINDOW_HXX_
#define _HELPPOPUPWINDOW_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <osl/mutex.hxx>

#define WIN32_LEAN_AND_MEAN
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

//---------------------------------------------
// declaration
//---------------------------------------------

/*
    A simple popup window similary to the one the
    windows help (using WinHelp) creates when called
    with the option HELP_CONTEXTPOPUP.

    The interface is very simple but necessary for our
    needs.
    The window automaticaly calculates the necessary
    dimensions of the window and a appropriate show
    position based on the position the client provides.
    When the user click any mouse button or hits any key
    the window closes itself and disappears.
*/

class CHelpPopupWindow
{
public:

    /*
        The client may set some parameter of the window.
        When the client omits to set one or more values
        a default value will be taken.
        The values are in pixel.
    */
    CHelpPopupWindow(
        HINSTANCE hInstance,
        HWND hwndParent );

    /*
        dtor
    */
    ~CHelpPopupWindow( );

    /*
        The client may set the text the window is showing
        on next activation.
    */
    void SAL_CALL setText( const rtl::OUString& aHelpText );

    /*
        Shows the window with the text that was last set.
        The posistion is the prefered position. The window
        may itself show at a slightly different position
        if it fits not at the prefered position.
    */
    void SAL_CALL show( sal_Int32 x, sal_Int32 y );

    HWND SAL_CALL setParent( HWND hwndNewParent );

private:
    void SAL_CALL onPaint( HWND, HDC );
    void SAL_CALL onNcDestroy();
    void SAL_CALL onCreate( HWND );

    POINT SAL_CALL calcUpperLeftCorner( );
    void  SAL_CALL calcWindowRect( LPRECT lprect );

    void SAL_CALL adjustWindowSize( sal_Int32*, sal_Int32* );
    void SAL_CALL adjustWindowPos( sal_Int32 x, sal_Int32 y, sal_Int32 cx, sal_Int32 cy );

    ATOM SAL_CALL RegisterWindowClass( );
    void SAL_CALL UnregisterWindowClass( );

    static LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

private:
    sal_Int32       m_hMargins;
    sal_Int32       m_vMargins;
    sal_Int32       m_avCharWidth;
    sal_Int32       m_avCharHeight;
    HWND            m_hwnd;
    HWND            m_hwndParent;
    HINSTANCE       m_hInstance;
    sal_Bool        m_bWndClassRegistered;
    ::rtl::OUString m_HelpText;
    HBITMAP         m_hBitmapShadow;
    HBRUSH          m_hBrushShadow;

    // the window class has to be registered only
    // once per process, so multiple instance of this class
    // share the registered window class
    static ATOM       s_ClassAtom;
    static osl::Mutex s_Mutex;
    static sal_Int32  s_RegisterWndClassCount;

// prevent copy and assignment
private:
    CHelpPopupWindow( const CHelpPopupWindow& );
    CHelpPopupWindow& operator=( const CHelpPopupWindow& );
};

#endif
