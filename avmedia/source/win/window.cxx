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
#include <cppuhelper/supportsservice.hxx>

#include "window.hxx"
#include "player.hxx"

#define AVMEDIA_WIN_WINDOW_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.Window_DirectX"
#define AVMEDIA_WIN_WINDOW_SERVICENAME "com.sun.star.media.Window_DirectX"

using namespace ::com::sun::star;

namespace avmedia { namespace win {

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

LRESULT CALLBACK MediaPlayerWndProc( HWND hWnd,UINT nMsg, WPARAM nPar1, LPARAM nPar2 )
{
    Window* pWindow = (Window*) ::GetWindowLongPtr( hWnd, 0 );
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
                css::awt::MouseEvent aUNOEvt;
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
                    aUNOEvt.Modifiers |= css::awt::KeyModifier::SHIFT;

                if( nPar1 & MK_CONTROL )
                    aUNOEvt.Modifiers |= css::awt::KeyModifier::MOD1;

                // Buttons
                if( WM_LBUTTONDOWN == nMsg || WM_LBUTTONUP == nMsg )
                    aUNOEvt.Buttons |= css::awt::MouseButton::LEFT;

                if( WM_MBUTTONDOWN == nMsg || WM_MBUTTONUP == nMsg )
                    aUNOEvt.Buttons |= css::awt::MouseButton::MIDDLE;

                if( WM_RBUTTONDOWN == nMsg || WM_RBUTTONUP == nMsg )
                    aUNOEvt.Buttons |= css::awt::MouseButton::RIGHT;

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
                const css::awt::FocusEvent aUNOEvt;
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

Window::Window( const css::uno::Reference< css::lang::XMultiServiceFactory >& rxMgr, Player& rPlayer ) :
    mxMgr( rxMgr ),
    maListeners( maMutex ),
    meZoomLevel( css::media::ZoomLevel_NOT_AVAILABLE ),
    mrPlayer( rPlayer ),
    mnFrameWnd( 0 ),
    mnParentWnd( 0 ),
    mnPointerType( css::awt::SystemPointer::ARROW )
{
    ::osl::MutexGuard aGuard( ImplGetOwnStaticMutex() );

    lcl_getWndClass();
}

Window::~Window()
{
    if( mnFrameWnd )
        ::DestroyWindow( mnFrameWnd );
}

void Window::ImplLayoutVideoWindow()
{
    if( css::media::ZoomLevel_NOT_AVAILABLE != meZoomLevel )
    {
        css::awt::Size           aPrefSize( mrPlayer.getPreferredPlayerWindowSize() );
        css::awt::Rectangle      aRect = getPosSize();
        int                 nW = aRect.Width, nH = aRect.Height;
        int                 nVideoW = nW, nVideoH = nH;
        int                 nX = 0, nY = 0, nWidth = 0, nHeight = 0;
        bool                bDone = false, bZoom = false;

        if( css::media::ZoomLevel_ORIGINAL == meZoomLevel )
        {
            bZoom = true;
        }
        else if( css::media::ZoomLevel_ZOOM_1_TO_4 == meZoomLevel )
        {
            aPrefSize.Width >>= 2;
            aPrefSize.Height >>= 2;
            bZoom = true;
        }
        else if( css::media::ZoomLevel_ZOOM_1_TO_2 == meZoomLevel )
        {
            aPrefSize.Width >>= 1;
            aPrefSize.Height >>= 1;
            bZoom = true;
        }
        else if( css::media::ZoomLevel_ZOOM_2_TO_1 == meZoomLevel )
        {
            aPrefSize.Width <<= 1;
            aPrefSize.Height <<= 1;
            bZoom = true;
        }
        else if( css::media::ZoomLevel_ZOOM_4_TO_1 == meZoomLevel )
        {
            aPrefSize.Width <<= 2;
            aPrefSize.Height <<= 2;
            bZoom = true;
        }
        else if( css::media::ZoomLevel_FIT_TO_WINDOW == meZoomLevel )
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

bool Window::create( const css::uno::Sequence< css::uno::Any >& rArguments )
{
    IVideoWindow* pVideoWindow = const_cast< IVideoWindow* >( mrPlayer.getVideoWindow() );
    WNDCLASS* mpWndClass = lcl_getWndClass();

    if( !mnFrameWnd && pVideoWindow && mpWndClass )
    {
        css::awt::Rectangle  aRect;
        sal_IntPtr       nWnd;

        rArguments[ 0 ] >>= nWnd;
        rArguments[ 1 ] >>= aRect;

        mnParentWnd = reinterpret_cast<HWND>(nWnd);

        mnFrameWnd = ::CreateWindow( mpWndClass->lpszClassName, NULL,
                                           WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                                           aRect.X, aRect.Y, aRect.Width, aRect.Height,
                                           mnParentWnd, NULL, mpWndClass->hInstance, 0 );

        if( mnFrameWnd )
        {
            ::SetWindowLongPtr( mnFrameWnd, 0, (LONG_PTR) this );

                        pVideoWindow->put_Owner( (OAHWND) mnFrameWnd );
                        pVideoWindow->put_MessageDrain( (OAHWND) mnFrameWnd );
                        pVideoWindow->put_WindowStyle( WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN );

                        mrPlayer.setNotifyWnd( mnFrameWnd );

                        meZoomLevel = css::media::ZoomLevel_FIT_TO_WINDOW;
                        ImplLayoutVideoWindow();
        }
    }

    return( mnFrameWnd != 0 );
}

void Window::processGraphEvent()
{
    mrPlayer.processEvent();
}

void Window::updatePointer()
{
    char* pCursorName;

    switch( mnPointerType )
    {
        case( css::awt::SystemPointer::CROSS ): pCursorName = IDC_CROSS; break;
        case( css::awt::SystemPointer::MOVE ): pCursorName = IDC_SIZEALL; break;
        case( css::awt::SystemPointer::WAIT ): pCursorName = IDC_WAIT; break;

        default:
            pCursorName = IDC_ARROW;
        break;
    }

    ::SetCursor( ::LoadCursor( NULL, pCursorName ) );
}

void SAL_CALL Window::update(  )
    throw (css::uno::RuntimeException)
{
    ::RedrawWindow( (HWND) mnFrameWnd, NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE  );
}

sal_Bool SAL_CALL Window::setZoomLevel( css::media::ZoomLevel eZoomLevel )
    throw (css::uno::RuntimeException)
{
        boolean bRet = false;

        if( css::media::ZoomLevel_NOT_AVAILABLE != meZoomLevel &&
            css::media::ZoomLevel_NOT_AVAILABLE != eZoomLevel )
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

media::ZoomLevel SAL_CALL Window::getZoomLevel(  )
    throw (css::uno::RuntimeException)
{
    return meZoomLevel;
}

void SAL_CALL Window::setPointerType( sal_Int32 nPointerType )
    throw (css::uno::RuntimeException)
{
    mnPointerType = nPointerType;
}

void SAL_CALL Window::setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 )
    throw (css::uno::RuntimeException)
{
    if( mnFrameWnd )
    {
        ::SetWindowPos( mnFrameWnd, HWND_TOP, X, Y, Width, Height, 0 );
        ImplLayoutVideoWindow();
    }
}

css::awt::Rectangle SAL_CALL Window::getPosSize()
    throw (css::uno::RuntimeException)
{
    css::awt::Rectangle aRet;

    if( mnFrameWnd )
    {
        ::RECT  aWndRect;

        if( ::GetClientRect( mnFrameWnd, &aWndRect ) )
        {
            aRet.X = aWndRect.left;
            aRet.Y = aWndRect.top;
            aRet.Width = aWndRect.right - aWndRect.left + 1;
            aRet.Height = aWndRect.bottom - aWndRect.top + 1;
        }
    }

    return aRet;
}

void SAL_CALL Window::setVisible( sal_Bool bVisible )
    throw (css::uno::RuntimeException)
{
    if( mnFrameWnd )
    {
        IVideoWindow* pVideoWindow = const_cast< IVideoWindow* >( mrPlayer.getVideoWindow() );

        if( pVideoWindow )
            pVideoWindow->put_Visible( bVisible ? OATRUE : OAFALSE );

        ::ShowWindow( mnFrameWnd, bVisible ? SW_SHOW : SW_HIDE );
    }
}

void SAL_CALL Window::setEnable( sal_Bool bEnable )
    throw (css::uno::RuntimeException)
{
    if( mnFrameWnd )
        ::EnableWindow( mnFrameWnd, bEnable );
}

void SAL_CALL Window::setFocus(  )
    throw (css::uno::RuntimeException)
{
    if( mnFrameWnd )
        ::SetFocus( mnFrameWnd );
}

void SAL_CALL Window::addWindowListener( const css::uno::Reference< css::awt::XWindowListener >& xListener )
    throw (css::uno::RuntimeException)
{
    maListeners.addInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL Window::removeWindowListener( const css::uno::Reference< css::awt::XWindowListener >& xListener )
    throw (css::uno::RuntimeException)
{
    maListeners.removeInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL Window::addFocusListener( const css::uno::Reference< css::awt::XFocusListener >& xListener )
    throw (css::uno::RuntimeException)
{
    maListeners.addInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL Window::removeFocusListener( const css::uno::Reference< css::awt::XFocusListener >& xListener )
    throw (css::uno::RuntimeException)
{
    maListeners.removeInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL Window::addKeyListener( const css::uno::Reference< css::awt::XKeyListener >& xListener )
    throw (css::uno::RuntimeException)
{
    maListeners.addInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL Window::removeKeyListener( const css::uno::Reference< css::awt::XKeyListener >& xListener )
    throw (css::uno::RuntimeException)
{
    maListeners.removeInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL Window::addMouseListener( const css::uno::Reference< css::awt::XMouseListener >& xListener )
    throw (css::uno::RuntimeException)
{
    maListeners.addInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL Window::removeMouseListener( const css::uno::Reference< css::awt::XMouseListener >& xListener )
    throw (css::uno::RuntimeException)
{
    maListeners.removeInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL Window::addMouseMotionListener( const css::uno::Reference< css::awt::XMouseMotionListener >& xListener )
    throw (css::uno::RuntimeException)
{
    maListeners.addInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL Window::removeMouseMotionListener( const css::uno::Reference< css::awt::XMouseMotionListener >& xListener )
    throw (css::uno::RuntimeException)
{
    maListeners.removeInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL Window::addPaintListener( const css::uno::Reference< css::awt::XPaintListener >& xListener )
    throw (css::uno::RuntimeException)
{
    maListeners.addInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL Window::removePaintListener( const css::uno::Reference< css::awt::XPaintListener >& xListener )
    throw (css::uno::RuntimeException)
{
    maListeners.removeInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL Window::dispose(  )
    throw (css::uno::RuntimeException)
{
}

void SAL_CALL Window::addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener )
    throw (css::uno::RuntimeException)
{
    maListeners.addInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL Window::removeEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener )
    throw (css::uno::RuntimeException)
{
    maListeners.removeInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void Window::fireMousePressedEvent( const css::css::awt::MouseEvent& rEvt )
{
    ::cppu::OInterfaceContainerHelper* pContainer = maListeners.getContainer( cppu::UnoType<css::awt::XMouseListener>::get());

    if( pContainer )
    {
        ::cppu::OInterfaceIteratorHelper aIter( *pContainer );

        while( aIter.hasMoreElements() )
            css::uno::Reference< css::awt::XMouseListener >( aIter.next(), css::uno::UNO_QUERY )->mousePressed( rEvt );
    }
}

void Window::fireMouseReleasedEvent( const css::css::awt::MouseEvent& rEvt )
{
    ::cppu::OInterfaceContainerHelper* pContainer = maListeners.getContainer( cppu::UnoType<css::awt::XMouseListener>::get());

    if( pContainer )
    {
        ::cppu::OInterfaceIteratorHelper aIter( *pContainer );

        while( aIter.hasMoreElements() )
            css::uno::Reference< css::awt::XMouseListener >( aIter.next(), css::uno::UNO_QUERY )->mouseReleased( rEvt );
    }
}

void Window::fireMouseMovedEvent( const css::css::awt::MouseEvent& rEvt )
{
    ::cppu::OInterfaceContainerHelper* pContainer = maListeners.getContainer( cppu::UnoType<css::awt::XMouseMotionListener>::get());

    if( pContainer )
    {
        ::cppu::OInterfaceIteratorHelper aIter( *pContainer );

        while( aIter.hasMoreElements() )
            css::uno::Reference< css::awt::XMouseMotionListener >( aIter.next(), css::uno::UNO_QUERY )->mouseMoved( rEvt );
    }
}

void Window::fireSetFocusEvent( const css::css::awt::FocusEvent& rEvt )
{
    ::cppu::OInterfaceContainerHelper* pContainer = maListeners.getContainer( cppu::UnoType<css::awt::XFocusListener>::get());

    if( pContainer )
    {
        ::cppu::OInterfaceIteratorHelper aIter( *pContainer );

        while( aIter.hasMoreElements() )
            css::uno::Reference< css::awt::XFocusListener >( aIter.next(), css::uno::UNO_QUERY )->focusGained( rEvt );
    }
}

OUString SAL_CALL Window::getImplementationName(  )
    throw (css::uno::RuntimeException)
{
    return OUString( AVMEDIA_WIN_WINDOW_IMPLEMENTATIONNAME );
}

sal_Bool SAL_CALL Window::supportsService( const OUString& ServiceName )
    throw (css::uno::RuntimeException)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > SAL_CALL Window::getSupportedServiceNames(  )
    throw (css::uno::RuntimeException)
{
    css::uno::Sequence<OUString> aRet { AVMEDIA_WIN_WINDOW_SERVICENAME };

    return aRet;
}

} // namespace win
} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
