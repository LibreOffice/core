/*************************************************************************
 *
 *  $RCSfile: wincomp.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:49 $
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

#ifndef _SV_WINCOMP_HXX
#define _SV_WINCOMP_HXX

#ifndef _STRING_H
#include <string.h>
#endif

#ifndef _SV_SV_H
#include <sv.h>
#endif

// ----------
// - Strict -
// ----------

// Anpassungen fuer TypeChecking

inline HPEN SelectPen( HDC hDC, HPEN hPen )
{
    return (HPEN)SelectObject( hDC, (HGDIOBJ)hPen );
}

inline void DeletePen( HPEN hPen )
{
    DeleteObject( (HGDIOBJ)hPen );
}

inline HPEN GetStockPen( int nObject )
{
    return (HPEN)GetStockObject( nObject );
}

inline HBRUSH SelectBrush( HDC hDC, HBRUSH hBrush )
{
    return (HBRUSH)SelectObject( hDC, (HGDIOBJ)hBrush );
}

inline void DeleteBrush( HBRUSH hBrush )
{
    DeleteObject( (HGDIOBJ)hBrush );
}

inline HBRUSH GetStockBrush( int nObject )
{
    return (HBRUSH)GetStockObject( nObject );
}

inline HFONT SelectFont( HDC hDC, HFONT hFont )
{
    return (HFONT)SelectObject( hDC, (HGDIOBJ)hFont );
}

inline void DeleteFont( HFONT hFont )
{
    DeleteObject( (HGDIOBJ)hFont );
}

inline HFONT GetStockFont( int nObject )
{
    return (HFONT)GetStockObject( nObject );
}

inline HBITMAP SelectBitmap( HDC hDC, HBITMAP hBitmap )
{
    return (HBITMAP)SelectObject( hDC, (HGDIOBJ)hBitmap );
}

inline void DeleteBitmap( HBITMAP hBitmap )
{
    DeleteObject( (HGDIOBJ)hBitmap );
}

inline void DeleteRegion( HRGN hRegion )
{
    DeleteObject( (HGDIOBJ)hRegion );
}

inline HPALETTE GetStockPalette( int nObject )
{
    return (HPALETTE)GetStockObject( nObject );
}

inline void DeletePalette( HPALETTE hPalette )
{
    DeleteObject( (HGDIOBJ)hPalette );
}

inline void SetWindowStyle( HWND hWnd, DWORD nStyle )
{
    SetWindowLong( hWnd, GWL_STYLE, nStyle );
}

inline DWORD GetWindowStyle( HWND hWnd )
{
    return GetWindowLong( hWnd, GWL_STYLE );
}

inline void SetWindowExStyle( HWND hWnd, DWORD nStyle )
{
    SetWindowLong( hWnd, GWL_EXSTYLE, nStyle );
}

inline DWORD GetWindowExStyle( HWND hWnd )
{
    return GetWindowLong( hWnd, GWL_EXSTYLE );
}

inline WIN_BOOL IsMinimized( HWND hWnd )
{
    return IsIconic( hWnd );
}

inline WIN_BOOL IsMaximized( HWND hWnd )
{
    return IsZoomed( hWnd );
}

inline void SetWindowFont( HWND hWnd, HFONT hFont, WIN_BOOL bRedraw )
{
    SendMessage( hWnd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM((UINT)bRedraw,0) );
}

inline HFONT GetWindowFont( HWND hWnd )
{
    return (HFONT)(UINT)SendMessage( hWnd, WM_GETFONT, 0, 0 );
}

// ---------------------
// - Windows/Window NT -
// ---------------------

// Anpassung fuer Unterschiede zwischen 3.x und NT

inline void SetClassCursor( HWND hWnd, HCURSOR hCursor )
{
#ifndef WNT
    SetClassWord( hWnd, GCW_HCURSOR, (WORD)hCursor );
#else
    SetClassLong( hWnd, GCL_HCURSOR, (DWORD)hCursor );
#endif
}

inline HCURSOR GetClassCursor( HWND hWnd )
{
#ifndef WNT
    return (HCURSOR)GetClassWord( hWnd, GCW_HCURSOR );
#else
    return (HCURSOR)GetClassLong( hWnd, GCL_HCURSOR );
#endif
}

inline void SetClassIcon( HWND hWnd, HICON hIcon )
{
#ifndef WNT
    SetClassWord( hWnd, GCW_HICON, (WORD)hIcon );
#else
    SetClassLong( hWnd, GCL_HICON, (DWORD)hIcon );
#endif
}

inline HICON GetClassIcon( HWND hWnd )
{
#ifndef WNT
    return (HICON)GetClassWord( hWnd, GCW_HICON );
#else
    return (HICON)GetClassLong( hWnd, GCL_HICON );
#endif
}

inline HBRUSH SetClassBrush( HWND hWnd, HBRUSH hBrush )
{
#ifndef WNT
    return (HBRUSH)SetClassWord( hWnd, GCW_HBRBACKGROUND, (WORD)hBrush );
#else
    return (HBRUSH)SetClassLong( hWnd, GCL_HBRBACKGROUND, (DWORD)hBrush );
#endif
}

inline HBRUSH GetClassBrush( HWND hWnd )
{
#ifndef WNT
    return (HBRUSH)GetClassWord( hWnd, GCW_HBRBACKGROUND );
#else
    return (HBRUSH)GetClassLong( hWnd, GCL_HBRBACKGROUND );
#endif
}

inline HINSTANCE GetWindowInstance( HWND hWnd )
{
#ifndef WNT
    return (HINSTANCE)GetWindowWord( hWnd, GWW_HINSTANCE );
#else
    return (HINSTANCE)GetWindowLong( hWnd, GWL_HINSTANCE );
#endif
}

#ifndef WNT
inline UINT CharLowerBuff( LPSTR lpStr, UINT nLen )
{
    return AnsiLowerBuff( lpStr, nLen );
}
#endif

#ifndef WNT
inline UINT CharUpperBuff( LPSTR lpStr, UINT nLen )
{
    return AnsiUpperBuff( lpStr, nLen );
}
#endif

#ifndef WNT
inline void OemToChar( LPCSTR lpStr1, LPSTR lpStr2 )
{
    OemToAnsi( lpStr1, lpStr2 );
}
#endif


// -----------------------------------
// - Unterschiede zwischen 16/32-Bit -
// -----------------------------------

#ifdef WIN
#define SVWINAPI    WINAPI
#else
#define SVWINAPI    APIENTRY
#endif

#ifdef WIN
#define NEARDATA    _near
#else
#define NEARDATA
#endif

// Zum kopieren von mehr als 64 KB
#ifdef WIN
inline void lmemcpy( void* pDst, const void* pSrc, ULONG nSize )
{
    hmemcpy( pDst, pSrc, nSize );
}
#else
inline void lmemcpy( void* pDst, const void* pSrc, ULONG nSize )
{
    memcpy( pDst, pSrc, nSize );
}
#endif

#ifdef WNT
typedef LONG    WinWeight;
#else
typedef int     WinWeight;
#endif


// ----------------------------------------------------
// - Steuerungen fuer Versionen und Laufzeit-Abfragen -
// ----------------------------------------------------

#if defined( WNT )
#define W95_VERSION         400
#else
#define W95_VERSION         395
#endif

// Wenn eine 32-Bit SV Version die nur unter W95 laeuft gebildet werden soll,
// muss nur dieses Define W40ONLY definiert werden
#if ( WINVER >= 0x0400 )
#define W40ONLY
#endif

// Wenn wir sowieso erst ab W95 laufen, brauchen wir auch keine
// Laufzeit-Abfragen
#ifdef W40ONLY
#define W40IF
#define W40NIF
#define W40ELSE

#else

// Nur ein 32-Bit-SDK definiert WINVER >= 0x0400 und somit brauchen wir
// diese W40-Abfragen auch nur hier. Die Abfragen, die sowohl fuer 3.1
// als auch fuer NT gelten sind als normale if-Abfragen kodiert
#ifdef WIN
#define W40NIF
#else
#define W40IF               if ( aSalShlData.mbW40 )
#define W40NIF              if ( !aSalShlData.mbW40 )
#define W40ELSE             else
#endif

#endif

/****************************

Beispiel fuer Klammerung:

#if ( WINVER >= 0x0400 )
    W40IF
    {
    ... W40-Code
    }
    W40ELSE
#endif
#ifndef W40ONLY
    {
    ... Normaler 3.1 und NT 3.5(1)-Code
    }
#endif

*****************************/


// ------------------------
// - ZMouse Erweiterungen -
// ------------------------

#if defined( WNT )

#ifdef UNICODE
#define MSH_MOUSEWHEEL L"MSWHEEL_ROLLMSG"
#else
#define MSH_MOUSEWHEEL "MSWHEEL_ROLLMSG"
#endif

// Default value for rolling one notch
#ifndef WHEEL_DELTA
#define WHEEL_DELTA      120
#endif

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL                   0x020A
#endif

#ifndef  WHEEL_PAGESCROLL
// signifies to scroll a page, also defined in winuser.h in the NT4.0 SDK
#define WHEEL_PAGESCROLL  (UINT_MAX)
#endif

#ifdef UNICODE
#define MOUSEZ_CLASSNAME  L"MouseZ"           // wheel window class
#define MOUSEZ_TITLE      L"Magellan MSWHEEL" // wheel window title
#else
#define MOUSEZ_CLASSNAME  "MouseZ"            // wheel window class
#define MOUSEZ_TITLE      "Magellan MSWHEEL"  // wheel window title
#endif

#define MSH_WHEELMODULE_CLASS (MOUSEZ_CLASSNAME)
#define MSH_WHEELMODULE_TITLE (MOUSEZ_TITLE)

#ifdef UNICODE
#define MSH_SCROLL_LINES L"MSH_SCROLL_LINES_MSG"
#else
#define MSH_SCROLL_LINES "MSH_SCROLL_LINES_MSG"
#endif

#ifndef SPI_GETWHEELSCROLLLINES
#define SPI_GETWHEELSCROLLLINES   104
#endif
#ifndef SPI_SETWHEELSCROLLLINES
#define SPI_SETWHEELSCROLLLINES   105
#endif

#endif


// -----------------------------
// - SystemAgent Erweiterungen -
// -----------------------------

#if ( WINVER >= 0x0400 )
#define ENABLE_AGENT            1
#define DISABLE_AGENT           2
#define GET_AGENT_STATUS        3

typedef int (SVWINAPI* SysAgt_Enable_PROC)( int );
#endif

// ---------------------
// - 5.0-Erweiterungen -
// ---------------------

#ifndef COLOR_HOTLIGHT
#define COLOR_HOTLIGHT                  26
#endif
#ifndef COLOR_GRADIENTACTIVECAPTION
#define COLOR_GRADIENTACTIVECAPTION     27
#endif
#ifndef COLOR_GRADIENTINACTIVECAPTION
#define COLOR_GRADIENTINACTIVECAPTION   28
#endif

#endif // _SV_WINCOMP_HXX
