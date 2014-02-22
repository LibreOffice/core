/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <tchar.h>
#include "helppopupwindow.hxx"
#include <osl/diagnose.h>


//


using osl::Mutex;


//


namespace /* private */
{

    const LPCTSTR CURRENT_INSTANCE = TEXT("CurrInst");

};

#define HELPPOPUPWND_CLASS_NAME TEXT("hlppopupwnd###")

const sal_Int32 MAX_CHARS_PER_LINE = 55;

const sal_Int32 SHADOW_WIDTH  = 6;
const sal_Int32 SHADOW_HEIGHT = 6;
const sal_Int32 SHADOW_OFFSET = 6;
const sal_Int32 YOFFSET       = 20;

const DWORD OUTER_FRAME_COLOR     = 0; 
const sal_Int32 OUTER_FRAME_WIDTH = 1; 


const DWORD INNER_FRAME_COLOR     = 0xC8D0D4;
const sal_Int32 INNER_FRAME_WIDTH = 1; 





osl::Mutex CHelpPopupWindow::s_Mutex;
ATOM CHelpPopupWindow::s_ClassAtom = 0;
sal_Int32 CHelpPopupWindow::s_RegisterWndClassCount = 0;


//


CHelpPopupWindow::CHelpPopupWindow(
    HINSTANCE hInstance,
    HWND hwndParent ) :
    m_hMargins( 0 ),
    m_vMargins( 0 ),
    m_avCharWidth( 0 ),
    m_avCharHeight( 0 ),
    m_hwnd( NULL ),
    m_hwndParent( hwndParent ),
    m_hInstance( hInstance ),
    m_hBitmapShadow( NULL ),
    m_hBrushShadow( NULL )
{
    m_bWndClassRegistered = RegisterWindowClass( ) ? sal_True : sal_False;

    
    WORD aPattern[] = { 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55 };

    m_hBitmapShadow = CreateBitmap( 8, 8, 1, 1, aPattern );
    m_hBrushShadow  = CreatePatternBrush( m_hBitmapShadow );
}


//


CHelpPopupWindow::~CHelpPopupWindow( )
{
    
    
    
    
    if ( m_bWndClassRegistered )
        UnregisterWindowClass( );

    DeleteObject( m_hBitmapShadow );
    DeleteObject( m_hBrushShadow );
}


//


void SAL_CALL CHelpPopupWindow::setText( const OUString& aHelpText )
{
    m_HelpText = aHelpText;
}


//


void SAL_CALL CHelpPopupWindow::show( sal_Int32 x, sal_Int32 y )
{
    OSL_ENSURE( NULL == m_hwnd, "method should not be called twice in sequence" );

    
    
    
    

    m_hwnd = CreateWindowEx(
        0,
        HELPPOPUPWND_CLASS_NAME,
        NULL,
        WS_POPUP,
        0,
        0,
        0,
        0,
        m_hwndParent,
        NULL,
        m_hInstance,
        (LPVOID)this );

    OSL_ENSURE( m_hwnd, "creating help popup window failed" );

    sal_Int32 cx_new;
    sal_Int32 cy_new;

    adjustWindowSize( &cx_new, &cy_new );
    adjustWindowPos( x, y, cx_new, cy_new );

    UpdateWindow( m_hwnd );
    ShowWindow( m_hwnd, SW_SHOW );
}


//


HWND SAL_CALL CHelpPopupWindow::setParent( HWND hwndNewParent )
{
    HWND oldParent = m_hwndParent;

    m_hwndParent = hwndNewParent;

    return oldParent;
}






void SAL_CALL CHelpPopupWindow::calcWindowRect( LPRECT lprect )
{
    OSL_ASSERT( m_hwnd && lprect );

    SetRect( lprect, 0, 0, MAX_CHARS_PER_LINE * m_avCharWidth, 0 );

    HDC hdc = GetDC( m_hwnd );

    
    HGDIOBJ oldFont = SelectObject(
        hdc, GetStockObject( DEFAULT_GUI_FONT ) );

    UINT nFormat = DT_WORDBREAK | DT_CALCRECT | DT_EXTERNALLEADING | DT_LEFT;

    if ( m_HelpText.getLength( ) <= MAX_CHARS_PER_LINE )
        nFormat |= DT_SINGLELINE;

    DrawText(
      hdc,
      reinterpret_cast<LPCTSTR>(m_HelpText.getStr( )),
      m_HelpText.getLength( ),
      lprect,
      nFormat );

    
    

    lprect->bottom +=
        m_vMargins +
        SHADOW_HEIGHT +
        OUTER_FRAME_WIDTH * 2 +
        INNER_FRAME_WIDTH * 2;

    lprect->right +=
        SHADOW_WIDTH +
        2 * m_avCharWidth +
        OUTER_FRAME_WIDTH * 2 +
        INNER_FRAME_WIDTH * 2;

    SelectObject( hdc, oldFont );

    ReleaseDC( m_hwnd, hdc );
}


//


void SAL_CALL CHelpPopupWindow::adjustWindowSize( sal_Int32* cx_new, sal_Int32* cy_new )
{
    OSL_ASSERT( cx_new && cy_new );

    RECT rect;
    calcWindowRect( &rect );

    
    SetWindowPos(
        m_hwnd,
        NULL,
        0,
        0,
        rect.right,
        rect.bottom,
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );

    *cx_new = rect.right;
    *cy_new = rect.bottom;
}


//


void SAL_CALL CHelpPopupWindow::adjustWindowPos(
    sal_Int32 x, sal_Int32 y, sal_Int32 cx, sal_Int32 cy )
{
    int   popX;
    int   popY;
    int   popWidth;
    int   popHeight;

    OSL_ASSERT( m_hwnd );

    HDC hdc = GetDC( m_hwnd );

    
    popWidth  = cx;
    popHeight = cy;
    popX      = x - ( popWidth / 2 );
    popY      = y - YOFFSET;

    int xScreen = GetDeviceCaps( hdc, HORZRES );
    int yScreen = GetDeviceCaps( hdc, VERTRES );

    if (popX < 0)
        popX = 0;

    if (popY < 0)
        popY = 0;

    if ((popX + popWidth) > xScreen)
        popX = xScreen - popWidth;

    if ((popY + popHeight) > yScreen)
        popY = yScreen - popHeight;

    SetWindowPos(
        m_hwnd,
        NULL,
        popX,
        popY,
        0,
        0,
        SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE );

    ReleaseDC( m_hwnd, hdc );
}


//


void SAL_CALL CHelpPopupWindow::onPaint( HWND hWnd, HDC hdc )
{
    RECT        rc;
    RECT        rect;
    HGDIOBJ     hpen, hpenOld;
    HGDIOBJ     hbrOld;
    COLORREF    oldBkColor;
    COLORREF    oldTextColor;
    HGDIOBJ     oldFont;
    HGDIOBJ     oldBrush;
    HGDIOBJ     hBrush;

    GetClientRect( hWnd, &rc );

    

    hBrush   = CreateSolidBrush( GetSysColor( COLOR_INFOBK ) );
    oldBrush = SelectObject( hdc, hBrush );

    hpen    = CreatePen( PS_SOLID, 0, OUTER_FRAME_COLOR );
    hpenOld = SelectObject( hdc, hpen );

    Rectangle(  hdc,
                rc.left   + OUTER_FRAME_WIDTH,
                rc.top    + OUTER_FRAME_WIDTH,
                rc.right  - SHADOW_WIDTH,
                rc.bottom - SHADOW_HEIGHT);

    SelectObject( hdc, oldBrush );
    SelectObject( hdc, hpenOld );

    DeleteObject( hBrush );
    DeleteObject( hpen );

    

    hBrush   = CreateSolidBrush( GetSysColor( COLOR_INFOBK ) );
    oldBrush = SelectObject( hdc, hBrush );

    hpen    = CreatePen( PS_SOLID, 0, INNER_FRAME_COLOR );
    hpenOld = SelectObject( hdc, hpen );

    Rectangle(  hdc,
                rc.left   + OUTER_FRAME_WIDTH + 1,
                rc.top    + OUTER_FRAME_WIDTH + 1,
                rc.right  - SHADOW_WIDTH  - OUTER_FRAME_WIDTH,
                rc.bottom - SHADOW_HEIGHT - OUTER_FRAME_WIDTH );

    SelectObject( hdc, oldBrush );
    SelectObject( hdc, hpenOld );

    DeleteObject( hBrush );
    DeleteObject( hpen );

    

    rect.left   = rc.left   + OUTER_FRAME_WIDTH + INNER_FRAME_WIDTH + 1 + m_hMargins;
    rect.top    = rc.top    + OUTER_FRAME_WIDTH + INNER_FRAME_WIDTH + 1 + m_vMargins / 2;
    rect.right  = rc.right  - SHADOW_WIDTH      - OUTER_FRAME_WIDTH - INNER_FRAME_WIDTH - m_hMargins;
    rect.bottom = rc.bottom - SHADOW_HEIGHT     - OUTER_FRAME_WIDTH - INNER_FRAME_WIDTH - m_vMargins / 2;

    oldBkColor   = SetBkColor( hdc, GetSysColor( COLOR_INFOBK ) );
    oldTextColor = SetTextColor( hdc, COLOR_INFOTEXT );

    oldFont = SelectObject( hdc, GetStockObject( DEFAULT_GUI_FONT ) );

    UINT nFormat = DT_WORDBREAK | DT_EXTERNALLEADING | DT_LEFT;

    if ( m_HelpText.getLength( ) <= MAX_CHARS_PER_LINE )
        nFormat |= DT_SINGLELINE;

    DrawText(
        hdc,
        (LPWSTR)m_HelpText.getStr( ),
        m_HelpText.getLength( ),
        &rect,
        nFormat );

    SelectObject( hdc, oldFont );
    SetTextColor( hdc, oldTextColor );
    SetBkColor( hdc, oldBkColor );

    
    

    oldBkColor   = SetBkColor( hdc, RGB( 0, 0, 0 ) );
    oldTextColor = SetTextColor( hdc, RGB( 255, 255, 255 ) );

    

    UnrealizeObject( m_hBrushShadow );
    hbrOld = SelectObject( hdc, m_hBrushShadow );

    

    PatBlt(hdc,
           rc.left + SHADOW_OFFSET,
           rc.bottom - SHADOW_HEIGHT,
           rc.right - SHADOW_OFFSET - SHADOW_WIDTH,
           SHADOW_HEIGHT,
           0xA000C9);

    

    PatBlt(hdc,
           rc.right - SHADOW_WIDTH,
           rc.top + SHADOW_OFFSET,
           SHADOW_WIDTH,
           rc.bottom - SHADOW_OFFSET,
           0xA000C9);

    SelectObject(hdc, hbrOld);
    SetTextColor( hdc, oldTextColor );
    SetBkColor( hdc, oldBkColor );
}


//


void SAL_CALL CHelpPopupWindow::onNcDestroy()
{
    m_hwnd = NULL;
}


//


void SAL_CALL CHelpPopupWindow::onCreate( HWND hwnd )
{
    m_hwnd = hwnd;

    HDC hdc = GetDC( m_hwnd );

    HGDIOBJ oldFont = SelectObject(
        hdc, GetStockObject( DEFAULT_GUI_FONT ) );

    TEXTMETRIC tm;
    GetTextMetrics( hdc, &tm );

    m_avCharWidth  = tm.tmAveCharWidth;
    m_avCharHeight = tm.tmHeight;

    if ( 0 == m_hMargins )
        m_hMargins = m_avCharWidth;

    if ( 0 == m_vMargins )
        m_vMargins = m_avCharHeight;

    SelectObject( hdc, oldFont );

    ReleaseDC( m_hwnd, hdc );
}


//


LRESULT CALLBACK CHelpPopupWindow::WndProc(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    LRESULT lResult = 0;

    switch ( uMsg )
    {
        case WM_CREATE:
            {
                LPCREATESTRUCT lpcs =
                    reinterpret_cast< LPCREATESTRUCT >( lParam );

                OSL_ASSERT( lpcs->lpCreateParams );

                CHelpPopupWindow* pImpl = reinterpret_cast< CHelpPopupWindow* >(
                    lpcs->lpCreateParams );

                
                SetProp( hWnd, CURRENT_INSTANCE, pImpl );

                pImpl->onCreate( hWnd );

                
                SetCapture( hWnd );
             }
        break;

        case WM_PAINT:
            {
                CHelpPopupWindow* pImpl = reinterpret_cast< CHelpPopupWindow* >(
                GetProp( hWnd, CURRENT_INSTANCE ) );

                OSL_ASSERT( pImpl );

                PAINTSTRUCT ps;

                BeginPaint(hWnd, &ps);
                pImpl->onPaint( hWnd, ps.hdc );
                EndPaint(hWnd, &ps);
            }
         break;

         case WM_NCDESTROY:
            {
                
                CHelpPopupWindow* pImpl = reinterpret_cast< CHelpPopupWindow* >(
                    RemoveProp( hWnd, CURRENT_INSTANCE ) );

                OSL_ASSERT( pImpl );

                pImpl->onNcDestroy();
            }
         break;

         case WM_LBUTTONDOWN:
         case WM_KEYDOWN:
         case WM_SYSKEYDOWN:
         case WM_MBUTTONDOWN:
         case WM_RBUTTONDOWN:
             ReleaseCapture();
             DestroyWindow(hWnd);
         break;

         default:
             return DefWindowProc(hWnd, uMsg, wParam, lParam);
       }

    return lResult;
}


//


ATOM SAL_CALL CHelpPopupWindow::RegisterWindowClass( )
{
    osl::MutexGuard aGuard( s_Mutex );

    if ( 0 == s_ClassAtom )
    {
        
        WNDCLASSEX wndClsEx;

        ZeroMemory(&wndClsEx, sizeof(wndClsEx));

        wndClsEx.cbSize        = sizeof(wndClsEx);
        wndClsEx.lpfnWndProc   = CHelpPopupWindow::WndProc;
        wndClsEx.hInstance     = m_hInstance;
        wndClsEx.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wndClsEx.hbrBackground = (HBRUSH)GetStockObject( NULL_BRUSH );
        wndClsEx.lpszClassName = HELPPOPUPWND_CLASS_NAME;

        
        
        
        
        
        s_ClassAtom = RegisterClassEx( &wndClsEx );
        OSL_ASSERT(s_ClassAtom);
    }

    
    
    
    if (0 != s_ClassAtom)
        s_RegisterWndClassCount++;

    return s_ClassAtom;
}


//


void SAL_CALL CHelpPopupWindow::UnregisterWindowClass( )
{
    osl::MutexGuard aGuard( s_Mutex );

    OSL_ASSERT( ( (0 != s_ClassAtom) && (s_RegisterWndClassCount > 0)) ||
                ( (0 == s_ClassAtom) && (0 == s_RegisterWndClassCount) ) );

    
    
    
    if ( 0 != s_ClassAtom )
    {
        s_RegisterWndClassCount--;
        OSL_ASSERT( s_RegisterWndClassCount >= 0 );
    }

    if ( 0 == s_RegisterWndClassCount )
    {
        if ( !UnregisterClass(
                 (LPCTSTR)(DWORD_PTR)MAKELONG( s_ClassAtom, 0 ), m_hInstance ) )
        {
            OSL_FAIL( "unregister window class failed" );
        }

        s_ClassAtom = 0;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
