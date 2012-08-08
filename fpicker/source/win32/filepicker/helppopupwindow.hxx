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

#ifndef _HELPPOPUPWINDOW_HXX_
#define _HELPPOPUPWINDOW_HXX_

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
