/*************************************************************************
 *
 *  $RCSfile: dibpreview.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: tra $ $Date: 2001-07-09 12:57:23 $
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

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _DIBPREVIEW_HXX_
#include "dibpreview.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#include <systools/win32/user9x.h>

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

using ::com::sun::star::uno::Sequence;

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

namespace /* private */
{

    const char* CURRENT_INSTANCE = "CurrInst";

};

//------------------------------------------------------------------------
// defines
//------------------------------------------------------------------------

#define PREVIEWWND_CLASS_NAME L"DIBPreviewWnd###"

// means 3 pixel left and 3 pixel right
#define HORZ_BODER_SPACE    6

// means 3 pixel top and 3 pixel bottom
#define VERT_BORDER_SPACE   6

//---------------------------------------------------
// static member initialization
//---------------------------------------------------

osl::Mutex CDIBPreview::s_Mutex;
ATOM CDIBPreview::s_ClassAtom = 0;
sal_Int32 CDIBPreview::s_RegisterDibPreviewWndCount = 0;

//---------------------------------------------------
//
//---------------------------------------------------

CDIBPreview::CDIBPreview(
    sal_Int32 x,
    sal_Int32 y,
    sal_Int32 cx,
    sal_Int32 cy,
    HWND aParent,
    HINSTANCE hInstance,
    sal_Bool bShow ) :
    m_hwnd( NULL ),
    m_hInstance( hInstance )
{
    if ( RegisterDibPreviewWindowClass( ) )
    {
        // create the preview window in invisible state
        sal_uInt32 dwStyle = bShow ? (WS_CHILD | WS_VISIBLE ) : (WS_CHILD );
        m_hwnd = CreateWindowExW(
            WS_EX_CLIENTEDGE,
            PREVIEWWND_CLASS_NAME,
            L"", dwStyle,
            x, y, cx, cy,
            aParent,
            (HMENU)0x0, // for child windows this will
                        // be used as child window identifier
            m_hInstance,
            (LPVOID)this // pass a pointer to the current
                         // instance of this class
        );

        OSL_ASSERT( IsWindow( m_hwnd ) );
    }
}

//---------------------------------------------------
//
//---------------------------------------------------

CDIBPreview::~CDIBPreview( )
{
    // remember: we don't have to destroy the
    // preview window because it will be destroyed
    // by it's parent window (the FileOpen dialog)
    // but we have to unregister the window class

    UnregisterDibPreviewWindowClass( );
}

//---------------------------------------------------
//
//---------------------------------------------------

void SAL_CALL CDIBPreview::setWidth( sal_Int32 cx_new )
{
    SetWindowPos(
        m_hwnd, NULL, 0, 0, cx_new, getHeight( ),
        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
}

//---------------------------------------------------
//
//---------------------------------------------------

sal_Int32 SAL_CALL CDIBPreview::getWidth( ) const
{
    RECT rect;
    sal_Bool bRet = GetClientRect( m_hwnd, &rect );

    sal_Int32 cx = 0;

    if ( bRet )
        cx = rect.right;

    return cx;
}

//---------------------------------------------------
//
//---------------------------------------------------

void SAL_CALL CDIBPreview::setHeight( sal_Int32 cy_new )
{
    SetWindowPos(
        m_hwnd, NULL, 0, 0, getWidth( ), cy_new,
        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
}

//---------------------------------------------------
//
//---------------------------------------------------

sal_Int32 SAL_CALL CDIBPreview::getHeight( ) const
{
    RECT rect;
    sal_Bool bRet = GetClientRect( m_hwnd, &rect );

    sal_Int32 cy = 0;

    if ( bRet )
        cy = rect.bottom;

    return cy;
}

//---------------------------------------------------
// shows or hides the preview window
//---------------------------------------------------

sal_Bool SAL_CALL CDIBPreview::show( sal_Bool bShow )
{
    sal_Bool bRet = sal_False;

    if ( IsWindow( m_hwnd ) )
    {
        ShowWindow( m_hwnd, bShow ? SW_SHOW : SW_HIDE );
        bRet = sal_True;
    }

    return bRet;
}

//---------------------------------------------------
//
//---------------------------------------------------

sal_Bool SAL_CALL CDIBPreview::isVisible( ) const
{
    return IsWindowVisible( m_hwnd );
}

//---------------------------------------------------
//
//---------------------------------------------------

int SAL_CALL CDIBPreview::getColorDepth( )
{
    HDC hdc = GetDC( m_hwnd );
    int clrRes = 0;

    if ( hdc )
        clrRes = GetDeviceCaps( hdc, COLORRES );

    return clrRes;
}

//---------------------------------------------------
//
//---------------------------------------------------

void SAL_CALL CDIBPreview::setImage( const Sequence< sal_Int8 >& ImageData )
{
    // save the new image data and force a redraw
    m_ImageData = ImageData;
    InvalidateRect( m_hwnd, NULL, TRUE );
    UpdateWindow( m_hwnd );
}
//---------------------------------------------------
//
//---------------------------------------------------

void SAL_CALL CDIBPreview::onPaint( HWND hWnd, HDC hDC )
{
    BITMAPFILEHEADER* pbmfh;
    BITMAPINFO      * pbmi;
    BYTE            * pBits;
    int               cxDib;
    int               cyDib;

    try
    {
        pbmfh = reinterpret_cast< BITMAPFILEHEADER* >( m_ImageData.getArray( ) );

        if ( !IsBadReadPtr( pbmfh, sizeof( BITMAPFILEHEADER ) ) &&
             (pbmfh->bfType == *(WORD*)"BM") )
        {
            pbmi  = reinterpret_cast< BITMAPINFO * >( (pbmfh + 1) );
            pBits = reinterpret_cast< BYTE * >( ((DWORD)pbmfh) + pbmfh->bfOffBits );

            cxDib =      pbmi->bmiHeader.biWidth;
            cyDib = abs (pbmi->bmiHeader.biHeight);

            int oldMode = SetStretchBltMode( hDC, COLORONCOLOR );

            int GDIError = StretchDIBits(
                hDC, 0, 0, getWidth( ), getHeight( ),
                0, 0, cxDib, cyDib, pBits, pbmi,
                DIB_RGB_COLORS, SRCCOPY);

            OSL_ASSERT( GDI_ERROR != GDIError );
        }
    }
    catch(...)
    {
    }
}

//---------------------------------------------------
//
//---------------------------------------------------

LRESULT CALLBACK CDIBPreview::WndProc(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    LRESULT lResult = 0;

    switch( uMsg )
    {

    // we connect a pointer to the current instance
    // with a window instance via SetProp
    case WM_CREATE:
        {
            LPCREATESTRUCT lpcs =
                reinterpret_cast< LPCREATESTRUCT >( lParam );

            OSL_ASSERT( lpcs->lpCreateParams );

            // connect the instance handle to the window
            SetPropA( hWnd, CURRENT_INSTANCE, lpcs->lpCreateParams );
        }
        break;

    // we remove the window property which connects
    // a class instance with a window class
    case WM_NCDESTROY:
        {
            // RemoveProp returns the saved value on success
            CDIBPreview* pImpl = reinterpret_cast< CDIBPreview* >(
                RemovePropA( hWnd, CURRENT_INSTANCE ) );

            OSL_ASSERT( pImpl );
        }
        break;

    case WM_PAINT:
    {
        CDIBPreview* pImpl = reinterpret_cast< CDIBPreview* >(
            GetPropA( hWnd, CURRENT_INSTANCE ) );

        OSL_ASSERT( pImpl );

        HDC         hDC;
        PAINTSTRUCT ps;

        hDC = BeginPaint( hWnd, &ps );
        pImpl->onPaint( hWnd, hDC );
        EndPaint( hWnd, &ps );
    }
    break;

    default:
#pragma message( "####################################" )
#pragma message( " fix this" )
#pragma message( "####################################" )

        return DefWindowProcA( hWnd, uMsg, wParam, lParam );
    }

    return lResult;
}

//---------------------------------------------------
//
//---------------------------------------------------

ATOM SAL_CALL CDIBPreview::RegisterDibPreviewWindowClass( )
{
    osl::MutexGuard aGuard( s_Mutex );

    if ( 0 == s_ClassAtom )
    {
        // register the preview window class
        WNDCLASSEXW wndClsEx;
        ZeroMemory( &wndClsEx, sizeof( WNDCLASSEXW ) );

        wndClsEx.cbSize        = sizeof( WNDCLASSEXW );
        wndClsEx.style         = CS_HREDRAW | CS_VREDRAW;
        wndClsEx.lpfnWndProc   = CDIBPreview::WndProc;
        wndClsEx.hInstance     = m_hInstance;
        wndClsEx.hbrBackground = (HBRUSH)( COLOR_INACTIVEBORDER + 1 );
        wndClsEx.lpszClassName = PREVIEWWND_CLASS_NAME;

        // register the preview window class
        // !!! Win95 -   the window class will be unregistered automaticly
        //               if the dll is unloaded
        //     Win2000 - the window class must be unregistered manually
        //               if the dll is unloaded
        s_ClassAtom = RegisterClassExW( &wndClsEx );
        OSL_ASSERT( s_ClassAtom );
    }

    // increment the register class counter
    // so that we keep track of the number
    // of class registrations
    if ( 0 != s_ClassAtom )
        s_RegisterDibPreviewWndCount++;

    return s_ClassAtom;
}

//---------------------------------------------------
//
//---------------------------------------------------

void SAL_CALL CDIBPreview::UnregisterDibPreviewWindowClass( )
{
    osl::MutexGuard aGuard( s_Mutex );

    OSL_ASSERT( 0 != s_ClassAtom );

    // update the register class counter
    // and unregister the window class if
    // counter drops to zero
    if ( 0 != s_ClassAtom )
    {
        s_RegisterDibPreviewWndCount--;
        OSL_ASSERT( s_RegisterDibPreviewWndCount >= 0 );
    }

    if ( 0 == s_RegisterDibPreviewWndCount )
    {
        UnregisterClass(
            (LPCTSTR)MAKELONG( s_ClassAtom, 0 ), m_hInstance );

        s_ClassAtom = 0;
    }
}