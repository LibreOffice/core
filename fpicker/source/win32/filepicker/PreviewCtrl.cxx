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
#include "PreviewCtrl.hxx"
#include <osl/diagnose.h>

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <ocidl.h>
#include <olectl.h>

#define PREVIEWWND_CLASS_NAME TEXT("PreviewWnd###")

#define HIMETRIC_INCH 2540


#define HORZ_BODER_SPACE    6


#define VERT_BORDER_SPACE   6





CFilePreview* CFilePreview::s_FilePreviewInst = NULL;
CFilePreview::FILEPREVIEW_SINGLETON_DESTROYER_T CFilePreview::s_SingletonDestroyer;





namespace 
{
    class CPreviewException
    {
        
        
    };

    
    //
    

    inline
    sal_Int32 SubDiv( sal_Int32 nNumber, sal_Int32 nMinuend, sal_Int32 nDenominator )
    {
        return ( static_cast<sal_Int32>( ( nNumber - nMinuend ) / nDenominator ) );
    }

    
    
    

    inline
    sal_Int32 Himetric2Pixel( HDC hDC, sal_Int32 hmSize, sal_Int32 nIndex )
    {
        return MulDiv( hmSize, GetDeviceCaps( hDC, nIndex), HIMETRIC_INCH );
    }

    
    //
    

    inline
    sal_uInt32 _getWidthRect( const RECT& aRect )
    {
        return ( aRect.right - aRect.left );
    }

    
    //
    

    inline
    sal_uInt32 _getHeightRect( const RECT& aRect )
    {
        return ( aRect.bottom - aRect.top );
    }

    
    
    
    

    inline
    POINT _calcULCorner( HWND hwnd, const CDimension& aPicSize )
    {
        RECT rect;
        GetClientRect( hwnd, &rect );

        sal_Int32 nWidthWnd  = _getWidthRect( rect );
        sal_Int32 nHeightWnd = _getHeightRect( rect );

        POINT ulCorner;
        ulCorner.x = SubDiv( nWidthWnd,  aPicSize.m_cx, 2 );
        ulCorner.y = SubDiv( nHeightWnd, aPicSize.m_cy, 2 );

        return ulCorner;
    }

    
    
    
    
    

    inline
    sal_Bool _pictureSizeFitsWindowSize( HWND hwnd, const CDimension& aPicSize )
    {
        RECT rect;
        GetClientRect( hwnd, &rect );

        sal_Int32 nWidthWnd  = _getWidthRect( rect );
        sal_Int32 nHeightWnd = _getHeightRect( rect );

        return ( ( ( nWidthWnd  - HORZ_BODER_SPACE )  >= aPicSize.m_cx ) &&
                 ( ( nHeightWnd - VERT_BORDER_SPACE ) >= aPicSize.m_cy ) );
    }

    
    
    
    
    

    inline
    CDimension _scalePictureSize( HWND hwnd, const CDimension& aPicSize )
    {
        CDimension scaledPicSize = aPicSize;

        if ( !_pictureSizeFitsWindowSize( hwnd, aPicSize ) )
        {
            RECT rect;
            GetClientRect( hwnd, &rect );

            
            
            sal_Int32 nHeightWnd = _getHeightRect( rect ) - VERT_BORDER_SPACE;
            sal_Int32 nWidthWnd  = nHeightWnd;

            if ( aPicSize.m_cx >= aPicSize.m_cy )
            {
                scaledPicSize.m_cx = nWidthWnd;
                scaledPicSize.m_cy =
                    static_cast< sal_Int32 >(
                        aPicSize.m_cy * nWidthWnd / aPicSize.m_cx );
            }
            else
            {
                scaledPicSize.m_cx =
                    static_cast< sal_Int32 >(
                        aPicSize.m_cx * nHeightWnd / aPicSize.m_cy );
                scaledPicSize.m_cy = nHeightWnd;
            }
        }

        return scaledPicSize;
    }

} 






CFilePreview* CFilePreview::createInstance(
    HWND aParent,
    POINT ulCorner,
    const CDimension& aSize,
    HINSTANCE hInstance,
    sal_Bool bShow,
    sal_Bool bEnabled )
{
    if ( !s_FilePreviewInst )
    {
        try
        {
            s_FilePreviewInst = new CFilePreview(
                aParent, ulCorner, aSize, hInstance, bShow, bEnabled );
            s_SingletonDestroyer.reset( s_FilePreviewInst );
        }
        catch( CPreviewException& )
        {
            OSL_ASSERT( !s_FilePreviewInst );
            OSL_FAIL( "Creation of the preview window failed" );
        }
        catch( CAutoOleInit::COleInitException& )
        {
            OSL_ASSERT( !s_FilePreviewInst );
            OSL_FAIL( "OleInitalize failed" );
        }
    }

    return s_FilePreviewInst;
}


//


CFilePreview::CFilePreview(
    HWND aParent,
    POINT ulCorner,
    const CDimension& aSize,
    HINSTANCE hInstance,
    sal_Bool bShow,
    sal_Bool bEnabled ) :
    m_hInstance( hInstance ),
    m_bEnabled( bEnabled )
{
    
    WNDCLASSEX wndClsEx;
    ZeroMemory(&wndClsEx, sizeof(wndClsEx));

    wndClsEx.cbSize        = sizeof(wndClsEx);
    wndClsEx.style         = CS_HREDRAW | CS_VREDRAW;
    wndClsEx.lpfnWndProc   = CFilePreview::WndProc;
    wndClsEx.hInstance     = m_hInstance;
    wndClsEx.hbrBackground = (HBRUSH)( COLOR_INACTIVEBORDER + 1 );
    wndClsEx.lpszClassName = PREVIEWWND_CLASS_NAME;

    
    
    
    
    
    m_atomPrevWndClass = RegisterClassEx(&wndClsEx);
    if ( !m_atomPrevWndClass )
        throw CPreviewException( );

    
    sal_uInt32 dwStyle = bShow ? (WS_CHILD | WS_VISIBLE) : WS_CHILD;
    m_hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        PREVIEWWND_CLASS_NAME,
        TEXT(""),
        dwStyle,
        ulCorner.x,
        ulCorner.y,
        aSize.m_cx,
        aSize.m_cy,
        aParent,
        (HMENU)100, 
                    
        m_hInstance,
        0 );
    if (!IsWindow(m_hwnd))
        throw CPreviewException( );
}


//


CFilePreview::~CFilePreview( )
{
    
    sal_Bool bRet = UnregisterClass(
        (LPCTSTR)(DWORD_PTR)MAKELONG( m_atomPrevWndClass, 0 ),
        m_hInstance );
    OSL_POSTCOND( bRet, "Unregister preview window class failed" );
}





sal_Bool SAL_CALL CFilePreview::setSize( const CDimension& aSize )
{
    OSL_PRECOND( IsWindow( m_hwnd ), "Preview window not initialized" );

    
    return SetWindowPos(
        m_hwnd,
        NULL,
        0,
        0,
        aSize.m_cx,
        aSize.m_cy,
        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
}





sal_Bool SAL_CALL CFilePreview::getSize( CDimension& theSize ) const
{
    OSL_PRECOND( IsWindow( m_hwnd ), "Preview window not initialized" );

    RECT rect;
    sal_Bool bRet = GetWindowRect( m_hwnd, &rect );

    theSize.m_cx = _getWidthRect( rect );
    theSize.m_cy = _getHeightRect( rect );

    return bRet;
}







sal_Bool SAL_CALL CFilePreview::setPos( POINT ulCorner )
{
    OSL_PRECOND( IsWindow( m_hwnd ), "Preview window not initialized" );

    
    return SetWindowPos(
        m_hwnd,
        NULL,
        ulCorner.x,
        ulCorner.y,
        0,
        0,
        SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
}







sal_Bool SAL_CALL CFilePreview::getPos( POINT& ulCorner ) const
{
    OSL_PRECOND( IsWindow( m_hwnd ), "Preview window not initialized" );

    POINT pt = { 0, 0 };
    RECT rect;

    sal_Bool bRet = GetWindowRect( m_hwnd, &rect );

    ulCorner.x = rect.left;
    ulCorner.y = rect.top;

    ScreenToClient( m_hwnd, &ulCorner );

    return bRet;
}


//


void SAL_CALL CFilePreview::enable( sal_Bool bEnable )
{
    m_bEnabled = bEnable;

    
    InvalidateRect( m_hwnd, NULL, sal_True );
    UpdateWindow( m_hwnd );
}










sal_Bool SAL_CALL CFilePreview::show( sal_Bool bShow )
{
    OSL_PRECOND( IsWindow( m_hwnd ), "Preview window not initialized" );

    sal_Int32 showState = bShow ? SW_SHOW : SW_HIDE;
    return ShowWindow( m_hwnd, showState );
}








sal_Bool SAL_CALL CFilePreview::update( const OUString& aFileName )
{
    OSL_PRECOND( IsWindow( m_hwnd ), "Preview window not initialized" );

    try
    {
        if ( m_bEnabled )
        {
            if ( m_IPicture )
                m_IPicture.Release( );

            loadFile( aFileName );

            
            InvalidateRect( m_hwnd, NULL, sal_True );
            UpdateWindow( m_hwnd );
        }
    }
    catch( _com_error& )
    {
    }

    return sal_True;
}


//


void SAL_CALL CFilePreview::onPaint( HWND hWnd, HDC hDC )
{
    OSL_PRECOND( IsWindow( m_hwnd ), "Preview window not initialized" );

    try
    {
        if ( m_bEnabled )
        {
            
            long cxPicHIMETRIC;
            long cyPicHIMETRIC;

            m_IPicture->get_Width( &cxPicHIMETRIC );
            m_IPicture->get_Height( &cyPicHIMETRIC );

            
            int cxPicPIXEL = Himetric2Pixel( hDC, cxPicHIMETRIC, LOGPIXELSX );
            int cyPicPIXEL = Himetric2Pixel( hDC, cyPicHIMETRIC, LOGPIXELSY );

            
            RECT rcPrevWnd;
            GetClientRect(hWnd, &rcPrevWnd);

            CDimension scaledPicSize = _scalePictureSize(
                hWnd, CDimension( cxPicPIXEL, cyPicPIXEL ) );

            
            
            POINT ulCorner = _calcULCorner( hWnd, scaledPicSize );

            
            HRESULT hr = m_IPicture->Render(
                hDC,
                ulCorner.x,
                ulCorner.y,
                scaledPicSize.m_cx,
                scaledPicSize.m_cy,
                0,
                cyPicHIMETRIC,
                cxPicHIMETRIC,
                -cyPicHIMETRIC,
                &rcPrevWnd );
        } 
    }
    catch( _com_error& )
    {
    }
}


//


sal_Bool CFilePreview::loadFile( const OUString& aFileName )
{
    HANDLE      hFile   = 0;
    HGLOBAL     hGlobal = 0;
    LPVOID      pData   = NULL;
    IStreamPtr  pIStream;
    HRESULT     hr = E_FAIL;
    sal_Bool    bRet;
    sal_uInt32  nBytesRead;
    sal_uInt32  fszExtra;
    sal_uInt32  fsize;

    hFile = CreateFile(
        aFileName.getStr( ),
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL );
    if ( INVALID_HANDLE_VALUE == hFile )
        goto CLEANUP_AND_EXIT;

    fszExtra = 0;
    fsize = GetFileSize( hFile, &fszExtra );

    
    if ( -1 == fsize || 0 == fsize || fszExtra )
        goto CLEANUP_AND_EXIT;

    hGlobal = GlobalAlloc( GMEM_MOVEABLE, fsize );
    if ( !hGlobal )
        goto CLEANUP_AND_EXIT;

    pData = GlobalLock( hGlobal );
    if ( !pData )
        goto CLEANUP_AND_EXIT;

    bRet = ReadFile(
        hFile, pData, fsize, &nBytesRead, NULL );

    if ( !bRet )
        goto CLEANUP_AND_EXIT;

    hr = CreateStreamOnHGlobal(
        hGlobal, sal_False, &pIStream );

    if ( SUCCEEDED( hr ) )
    {
        hr = OleLoadPicture(
            pIStream, fsize, sal_False,
            __uuidof( IPicture ), (LPVOID*)&m_IPicture );
    }

CLEANUP_AND_EXIT:
    if ( hFile )
        CloseHandle( hFile );

    if ( pData )
        GlobalUnlock( hGlobal );

    if ( hGlobal )
        GlobalFree( hGlobal );

    return ( SUCCEEDED( hr ) );
}


//


LRESULT CALLBACK CFilePreview::WndProc(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    LRESULT lResult = 0;

    switch( uMsg )
    {
    case WM_PAINT:
    {
        OSL_PRECOND( s_FilePreviewInst, "Static member not initialized" );

        HDC         hDC;
        PAINTSTRUCT ps;

        hDC = BeginPaint( hWnd, &ps );
        s_FilePreviewInst->onPaint( hWnd, hDC );
        EndPaint( hWnd, &ps );
    }
    break;

    
        
        
        
        
        
    case WM_NCCREATE:
        lResult = sal_True;
        break;

    default:
        return DefWindowProc( hWnd, uMsg, wParam, lParam );
    }

    return lResult;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
