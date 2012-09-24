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

#if defined _MSC_VER
#pragma warning(push, 1)
#pragma warning(disable: 4917)
#endif
#include <objbase.h>
#include <strmif.h>
#include <control.h>
#define STRSAFE_NO_DEPRECATE
#include <dshow.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <com/sun/star/awt/SystemPointer.hpp>

#include "window.hxx"
#include "player.hxx"

#define AVMEDIA_WIN_WINDOW_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.Window_DirectX"
#define AVMEDIA_WIN_WINDOW_SERVICENAME "com.sun.star.media.Window_DirectX"

using namespace ::com::sun::star;

namespace avmedia { namespace win {

// -----------
// - statics -
// -----------

static ::osl::Mutex& ImplGetOwnStaticMutex()
{
    static ::osl::Mutex* pMutex = NULL;

    if( pMutex == NULL )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );

        if( pMutex == NULL )
        {
            static ::osl::Mutex aMutex;
            pMutex = &aMutex;
        }
    }

    return *pMutex;
}

// -----------
// - WndProc -
// -----------

LRESULT CALLBACK MediaPlayerWndProc( HWND hWnd,UINT nMsg, WPARAM nPar1, LPARAM nPar2 )
{
    Window* pWindow = (Window*) ::GetWindowLong( hWnd, 0 );
    bool    bProcessed = true;

    if( pWindow )
    {
        switch( nMsg )
        {
            case( WM_SETCURSOR ):
                pWindow->updatePointer();
            break;

            case( WM_GRAPHNOTIFY ):
                pWindow->processGraphEvent();
            break;

            case( WM_MOUSEMOVE ):
            case( WM_LBUTTONDOWN ):
            case( WM_MBUTTONDOWN ):
            case( WM_RBUTTONDOWN ):
            case( WM_LBUTTONUP ):
            case( WM_MBUTTONUP ):
            case( WM_RBUTTONUP ):
            {
                awt::MouseEvent aUNOEvt;
                POINT           aWinPoint;

                if( !::GetCursorPos( &aWinPoint ) || !::ScreenToClient( hWnd, &aWinPoint ) )
                {
                    aWinPoint.x = GET_X_LPARAM( nPar2 );
                    aWinPoint.y = GET_Y_LPARAM( nPar2 );
                }
                aUNOEvt.Modifiers = 0;
                aUNOEvt.Buttons = 0;
                aUNOEvt.X = aWinPoint.x;
                aUNOEvt.Y = aWinPoint.y;
                aUNOEvt.PopupTrigger = false;

                // Modifiers
                if( nPar1 & MK_SHIFT )
                    aUNOEvt.Modifiers |= awt::KeyModifier::SHIFT;

                if( nPar1 & MK_CONTROL )
                    aUNOEvt.Modifiers |= awt::KeyModifier::MOD1;

                // Buttons
                if( WM_LBUTTONDOWN == nMsg || WM_LBUTTONUP == nMsg )
                    aUNOEvt.Buttons |= awt::MouseButton::LEFT;

                if( WM_MBUTTONDOWN == nMsg || WM_MBUTTONUP == nMsg )
                    aUNOEvt.Buttons |= awt::MouseButton::MIDDLE;

                if( WM_RBUTTONDOWN == nMsg || WM_RBUTTONUP == nMsg )
                    aUNOEvt.Buttons |= awt::MouseButton::RIGHT;

                // event type
                if( WM_LBUTTONDOWN == nMsg ||
                    WM_MBUTTONDOWN == nMsg ||
                    WM_RBUTTONDOWN == nMsg )
                {
                    aUNOEvt.ClickCount = 1;
                    pWindow->fireMousePressedEvent( aUNOEvt );
                }
                else if( WM_LBUTTONUP == nMsg ||
                         WM_MBUTTONUP == nMsg ||
                         WM_RBUTTONUP == nMsg )
                {
                    aUNOEvt.ClickCount = 1;
                    pWindow->fireMouseReleasedEvent( aUNOEvt );
                }
                else if( WM_MOUSEMOVE == nMsg )
                {
                    aUNOEvt.ClickCount = 0;
                    pWindow->fireMouseMovedEvent( aUNOEvt );
                    pWindow->updatePointer();
                }
            }
            break;

            case( WM_SETFOCUS ):
            {
                const awt::FocusEvent aUNOEvt;
                pWindow->fireSetFocusEvent( aUNOEvt );
            }
            break;

            default:
                bProcessed = false;
            break;
        }
    }
    else
        bProcessed = false;

    return( bProcessed ? 0 : DefWindowProc( hWnd, nMsg, nPar1, nPar2 ) );
}

// ---------------
// - Window -
// ---------------

WNDCLASS* lcl_getWndClass()
{
    static WNDCLASS* s_pWndClass = NULL;
    if ( !s_pWndClass )
    {
        s_pWndClass = new WNDCLASS;

        memset( s_pWndClass, 0, sizeof( *s_pWndClass ) );
        s_pWndClass->hInstance = GetModuleHandle( NULL );
        s_pWndClass->cbWndExtra = sizeof( DWORD );
        s_pWndClass->lpfnWndProc = MediaPlayerWndProc;
        s_pWndClass->lpszClassName = "com_sun_star_media_PlayerWnd";
        s_pWndClass->hbrBackground = (HBRUSH) ::GetStockObject( BLACK_BRUSH );
        s_pWndClass->hCursor = ::LoadCursor( NULL, IDC_ARROW );

        ::RegisterClass( s_pWndClass );
    }
    return s_pWndClass;
}

// ------------------------------------------------------------------------------

Window::Window( const uno::Reference< lang::XMultiServiceFactory >& rxMgr, Player& rPlayer ) :
    mxMgr( rxMgr ),
    mrPlayer( rPlayer ),
    meZoomLevel( media::ZoomLevel_NOT_AVAILABLE ),
    mnParentWnd( 0 ),
    mnFrameWnd( 0 ),
    maListeners( maMutex ),
    mnPointerType( awt::SystemPointer::ARROW )
{
    ::osl::MutexGuard aGuard( ImplGetOwnStaticMutex() );

    lcl_getWndClass();
}

// ------------------------------------------------------------------------------

Window::~Window()
{
    if( mnFrameWnd )
        ::DestroyWindow( (HWND) mnFrameWnd );
}

// ------------------------------------------------------------------------------

void Window::ImplLayoutVideoWindow()
{
    if( media::ZoomLevel_NOT_AVAILABLE != meZoomLevel )
    {
        awt::Size           aPrefSize( mrPlayer.getPreferredPlayerWindowSize() );
        awt::Rectangle      aRect = getPosSize();
        int                 nW = aRect.Width, nH = aRect.Height;
        int                 nVideoW = nW, nVideoH = nH;
        int                 nX = 0, nY = 0, nWidth = 0, nHeight = 0;
        bool                bDone = false, bZoom = false;

        if( media::ZoomLevel_ORIGINAL == meZoomLevel )
        {
            bZoom = true;
        }
        else if( media::ZoomLevel_ZOOM_1_TO_4 == meZoomLevel )
        {
            aPrefSize.Width >>= 2;
            aPrefSize.Height >>= 2;
            bZoom = true;
        }
        else if( media::ZoomLevel_ZOOM_1_TO_2 == meZoomLevel )
        {
            aPrefSize.Width >>= 1;
            aPrefSize.Height >>= 1;
            bZoom = true;
        }
        else if( media::ZoomLevel_ZOOM_2_TO_1 == meZoomLevel )
        {
            aPrefSize.Width <<= 1;
            aPrefSize.Height <<= 1;
            bZoom = true;
        }
        else if( media::ZoomLevel_ZOOM_4_TO_1 == meZoomLevel )
        {
            aPrefSize.Width <<= 2;
            aPrefSize.Height <<= 2;
            bZoom = true;
        }
        else if( media::ZoomLevel_FIT_TO_WINDOW == meZoomLevel )
        {
            nWidth = nVideoW;
            nHeight = nVideoH;
            bDone = true;
        }

        if( bZoom )
        {
            if( ( aPrefSize.Width <= nVideoW ) && ( aPrefSize.Height <= nVideoH ) )
            {
                nX = ( nVideoW - aPrefSize.Width ) >> 1;
                nY = ( nVideoH - aPrefSize.Height ) >> 1;
                nWidth = aPrefSize.Width;
                nHeight = aPrefSize.Height;
                bDone = true;
            }
        }

        if( !bDone )
        {
            if( aPrefSize.Width > 0 && aPrefSize.Height > 0 && nVideoW > 0 && nVideoH > 0 )
            {
                double fPrefWH = (double) aPrefSize.Width / aPrefSize.Height;

                if( fPrefWH < ( (double) nVideoW / nVideoH ) )
                    nVideoW = (int)( nVideoH * fPrefWH );
                else
                    nVideoH = (int)( nVideoW / fPrefWH );

                nX = ( nW - nVideoW ) >> 1;
                nY = ( nH - nVideoH ) >> 1;
                nWidth = nVideoW;
                nHeight = nVideoH;
            }
            else
                nX = nY = nWidth = nHeight = 0;
        }

        IVideoWindow* pVideoWindow = const_cast< IVideoWindow* >( mrPlayer.getVideoWindow() );

        if( pVideoWindow )
            pVideoWindow->SetWindowPosition( nX, nY, nWidth, nHeight );
    }
}

// ------------------------------------------------------------------------------

bool Window::create( const uno::Sequence< uno::Any >& rArguments )
{
    IVideoWindow* pVideoWindow = const_cast< IVideoWindow* >( mrPlayer.getVideoWindow() );
    WNDCLASS* mpWndClass = lcl_getWndClass();

    if( !mnFrameWnd && pVideoWindow && mpWndClass )
    {
        awt::Rectangle  aRect;
        sal_IntPtr       nWnd;

        rArguments[ 0 ] >>= nWnd;
        rArguments[ 1 ] >>= aRect;

        mnParentWnd = static_cast<int>(nWnd);

        mnFrameWnd = (int) ::CreateWindow( mpWndClass->lpszClassName, NULL,
                                           WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                                           aRect.X, aRect.Y, aRect.Width, aRect.Height,
                                           (HWND) mnParentWnd, NULL, mpWndClass->hInstance, 0 );

        // if the last CreateWindow failed...
        if( mnFrameWnd == 0 )
        {
            // try again and this time assume that mnParent is indeed a dc
            mnParentWnd = reinterpret_cast<int>(::WindowFromDC( (HDC)mnParentWnd ));
            mnFrameWnd = (int) ::CreateWindow( mpWndClass->lpszClassName, NULL,
                                           WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                                           aRect.X, aRect.Y, aRect.Width, aRect.Height,
                                           (HWND)mnParentWnd , NULL, mpWndClass->hInstance, 0 );
        }

        if( mnFrameWnd )
        {
            ::SetWindowLong( (HWND) mnFrameWnd, 0, (DWORD) this );

                        pVideoWindow->put_Owner( (OAHWND) mnFrameWnd );
                        pVideoWindow->put_MessageDrain( (OAHWND) mnFrameWnd );
                        pVideoWindow->put_WindowStyle( WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN );

                        mrPlayer.setNotifyWnd( mnFrameWnd );

                        meZoomLevel = media::ZoomLevel_FIT_TO_WINDOW;
                        ImplLayoutVideoWindow();
        }
    }

    return( mnFrameWnd != 0 );
}

// ------------------------------------------------------------------------------

void Window::processGraphEvent()
{
    mrPlayer.processEvent();
}

// ------------------------------------------------------------------------------

void Window::updatePointer()
{
    char* pCursorName;

    switch( mnPointerType )
    {
        case( awt::SystemPointer::CROSS ): pCursorName = IDC_CROSS; break;
        case( awt::SystemPointer::MOVE ): pCursorName = IDC_SIZEALL; break;
        case( awt::SystemPointer::WAIT ): pCursorName = IDC_WAIT; break;

        default:
            pCursorName = IDC_ARROW;
        break;
    }

    ::SetCursor( ::LoadCursor( NULL, pCursorName ) );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::update(  )
    throw (uno::RuntimeException)
{
    ::RedrawWindow( (HWND) mnFrameWnd, NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE  );
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Window::setZoomLevel( media::ZoomLevel eZoomLevel )
    throw (uno::RuntimeException)
{
        boolean bRet = false;

        if( media::ZoomLevel_NOT_AVAILABLE != meZoomLevel &&
            media::ZoomLevel_NOT_AVAILABLE != eZoomLevel )
        {
            if( eZoomLevel != meZoomLevel )
            {
                meZoomLevel = eZoomLevel;
                ImplLayoutVideoWindow();
            }

            bRet = true;
        }

        return bRet;
}

// ------------------------------------------------------------------------------

media::ZoomLevel SAL_CALL Window::getZoomLevel(  )
    throw (uno::RuntimeException)
{
    return meZoomLevel;
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::setPointerType( sal_Int32 nPointerType )
    throw (uno::RuntimeException)
{
    mnPointerType = nPointerType;
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 )
    throw (uno::RuntimeException)
{
    if( mnFrameWnd )
    {
        ::SetWindowPos( (HWND) mnFrameWnd, HWND_TOP, X, Y, Width, Height, 0 );
        ImplLayoutVideoWindow();
    }
}

// ------------------------------------------------------------------------------

awt::Rectangle SAL_CALL Window::getPosSize()
    throw (uno::RuntimeException)
{
    awt::Rectangle aRet;

    if( mnFrameWnd )
    {
        ::RECT  aWndRect;

        if( ::GetClientRect( (HWND) mnFrameWnd, &aWndRect ) )
        {
            aRet.X = aWndRect.left;
            aRet.Y = aWndRect.top;
            aRet.Width = aWndRect.right - aWndRect.left + 1;
            aRet.Height = aWndRect.bottom - aWndRect.top + 1;
        }
    }

    return aRet;
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::setVisible( sal_Bool bVisible )
    throw (uno::RuntimeException)
{
    if( mnFrameWnd )
    {
        IVideoWindow* pVideoWindow = const_cast< IVideoWindow* >( mrPlayer.getVideoWindow() );

        if( pVideoWindow )
            pVideoWindow->put_Visible( bVisible ? OATRUE : OAFALSE );

        ::ShowWindow( (HWND) mnFrameWnd, bVisible ? SW_SHOW : SW_HIDE );
    }
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::setEnable( sal_Bool bEnable )
    throw (uno::RuntimeException)
{
    if( mnFrameWnd )
        ::EnableWindow( (HWND) mnFrameWnd, bEnable );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::setFocus(  )
    throw (uno::RuntimeException)
{
    if( mnFrameWnd )
        ::SetFocus( (HWND) mnFrameWnd );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::addWindowListener( const uno::Reference< awt::XWindowListener >& xListener )
    throw (uno::RuntimeException)
{
    maListeners.addInterface( getCppuType( &xListener ), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::removeWindowListener( const uno::Reference< awt::XWindowListener >& xListener )
    throw (uno::RuntimeException)
{
    maListeners.removeInterface( getCppuType( &xListener ), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::addFocusListener( const uno::Reference< awt::XFocusListener >& xListener )
    throw (uno::RuntimeException)
{
    maListeners.addInterface( getCppuType( &xListener ), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::removeFocusListener( const uno::Reference< awt::XFocusListener >& xListener )
    throw (uno::RuntimeException)
{
    maListeners.removeInterface( getCppuType( &xListener ), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::addKeyListener( const uno::Reference< awt::XKeyListener >& xListener )
    throw (uno::RuntimeException)
{
    maListeners.addInterface( getCppuType( &xListener ), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::removeKeyListener( const uno::Reference< awt::XKeyListener >& xListener )
    throw (uno::RuntimeException)
{
    maListeners.removeInterface( getCppuType( &xListener ), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::addMouseListener( const uno::Reference< awt::XMouseListener >& xListener )
    throw (uno::RuntimeException)
{
    maListeners.addInterface( getCppuType( &xListener ), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::removeMouseListener( const uno::Reference< awt::XMouseListener >& xListener )
    throw (uno::RuntimeException)
{
    maListeners.removeInterface( getCppuType( &xListener ), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::addMouseMotionListener( const uno::Reference< awt::XMouseMotionListener >& xListener )
    throw (uno::RuntimeException)
{
    maListeners.addInterface( getCppuType( &xListener ), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::removeMouseMotionListener( const uno::Reference< awt::XMouseMotionListener >& xListener )
    throw (uno::RuntimeException)
{
    maListeners.removeInterface( getCppuType( &xListener ), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::addPaintListener( const uno::Reference< awt::XPaintListener >& xListener )
    throw (uno::RuntimeException)
{
    maListeners.addInterface( getCppuType( &xListener ), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::removePaintListener( const uno::Reference< awt::XPaintListener >& xListener )
    throw (uno::RuntimeException)
{
    maListeners.removeInterface( getCppuType( &xListener ), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::dispose(  )
    throw (uno::RuntimeException)
{
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
    throw (uno::RuntimeException)
{
    maListeners.addInterface( getCppuType( &xListener ), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::removeEventListener( const uno::Reference< lang::XEventListener >& xListener )
    throw (uno::RuntimeException)
{
    maListeners.removeInterface( getCppuType( &xListener ), xListener );
}

// ------------------------------------------------------------------------------

void Window::fireMousePressedEvent( const ::com::sun::star::awt::MouseEvent& rEvt )
{
    ::cppu::OInterfaceContainerHelper* pContainer = maListeners.getContainer( getCppuType( (uno::Reference< awt::XMouseListener >*) 0 ) );

    if( pContainer )
    {
        ::cppu::OInterfaceIteratorHelper aIter( *pContainer );

        while( aIter.hasMoreElements() )
            uno::Reference< awt::XMouseListener >( aIter.next(), uno::UNO_QUERY )->mousePressed( rEvt );
    }
}

// -----------------------------------------------------------------------------

void Window::fireMouseReleasedEvent( const ::com::sun::star::awt::MouseEvent& rEvt )
{
    ::cppu::OInterfaceContainerHelper* pContainer = maListeners.getContainer( getCppuType( (uno::Reference< awt::XMouseListener >*) 0 ) );

    if( pContainer )
    {
        ::cppu::OInterfaceIteratorHelper aIter( *pContainer );

        while( aIter.hasMoreElements() )
            uno::Reference< awt::XMouseListener >( aIter.next(), uno::UNO_QUERY )->mouseReleased( rEvt );
    }
}

// -----------------------------------------------------------------------------

void Window::fireMouseMovedEvent( const ::com::sun::star::awt::MouseEvent& rEvt )
{
    ::cppu::OInterfaceContainerHelper* pContainer = maListeners.getContainer( getCppuType( (uno::Reference< awt::XMouseMotionListener >*) 0 ) );

    if( pContainer )
    {
        ::cppu::OInterfaceIteratorHelper aIter( *pContainer );

        while( aIter.hasMoreElements() )
            uno::Reference< awt::XMouseMotionListener >( aIter.next(), uno::UNO_QUERY )->mouseMoved( rEvt );
    }
}

// -----------------------------------------------------------------------------

void Window::fireSetFocusEvent( const ::com::sun::star::awt::FocusEvent& rEvt )
{
    ::cppu::OInterfaceContainerHelper* pContainer = maListeners.getContainer( getCppuType( (uno::Reference< awt::XFocusListener >*) 0 ) );

    if( pContainer )
    {
        ::cppu::OInterfaceIteratorHelper aIter( *pContainer );

        while( aIter.hasMoreElements() )
            uno::Reference< awt::XFocusListener >( aIter.next(), uno::UNO_QUERY )->focusGained( rEvt );
    }
}

// ------------------------------------------------------------------------------

OUString SAL_CALL Window::getImplementationName(  )
    throw (uno::RuntimeException)
{
    return OUString( AVMEDIA_WIN_WINDOW_IMPLEMENTATIONNAME );
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Window::supportsService( const OUString& ServiceName )
    throw (uno::RuntimeException)
{
    return ServiceName == AVMEDIA_WIN_WINDOW_SERVICENAME;
}

// ------------------------------------------------------------------------------

uno::Sequence< OUString > SAL_CALL Window::getSupportedServiceNames(  )
    throw (uno::RuntimeException)
{
    uno::Sequence< OUString > aRet(1);
    aRet[0] = AVMEDIA_WIN_WINDOW_SERVICENAME ;

    return aRet;
}

} // namespace win
} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
