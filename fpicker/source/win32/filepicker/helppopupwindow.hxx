/*************************************************************************
 *
 *  $RCSfile: helppopupwindow.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2003-10-06 15:56:58 $
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

#ifndef _HELPPOPUPWINDOW_HXX_
#define _HELPPOPUPWINDOW_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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
        HWND hwndParent = NULL,
        sal_Int32 minWidth = 0,
        sal_Int32 hMargins = 0,
        sal_Int32 vMargins = 0 );

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
    void SAL_CALL onNcDestroy( HWND );
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
