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

#include <objbase.h>
#include <dshow.h>

#include <com/sun/star/awt/SystemPointer.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <avmedia/mediaitem.hxx>
#include <avmedia/mediawindow.hxx>

#include "window.hxx"
#include "player.hxx"

constexpr OUStringLiteral AVMEDIA_WIN_WINDOW_IMPLEMENTATIONNAME = u"com.sun.star.comp.avmedia.Window_DirectX";
constexpr OUString AVMEDIA_WIN_WINDOW_SERVICENAME = u"com.sun.star.media.Window_DirectX"_ustr;

using namespace ::com::sun::star;

namespace avmedia::win {

static LRESULT CALLBACK MediaPlayerWndProc( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    Window* pWindow = reinterpret_cast<Window*>(GetWindowLongPtrW( hWnd, 0 ));
    bool    bProcessed = true;

    if( pWindow )
    {
        switch( nMsg )
        {
            HANDLE_MSG(hWnd, WM_CLOSE, pWindow->getPlayer().OnClose);
            HANDLE_MSG(hWnd, WM_PAINT, pWindow->getPlayer().OnPaint);
            HANDLE_MSG(hWnd, WM_SIZE, pWindow->getPlayer().OnSize);

            case WM_SETCURSOR:
                pWindow->updatePointer();
            break;

            case WM_MOUSEMOVE:
            case WM_LBUTTONDOWN:
            case WM_MBUTTONDOWN:
            case WM_RBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_MBUTTONUP:
            case WM_RBUTTONUP:
                PostMessage(pWindow->getParentWnd(), nMsg, wParam, lParam);
            break;

            case WM_SETFOCUS:
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

    return( bProcessed ? 0 : DefWindowProcW( hWnd, nMsg, wParam, lParam ) );
}

static WNDCLASSW* lcl_getWndClass()
{
    WNDCLASSW* s_pWndClass = new WNDCLASSW;

    memset( s_pWndClass, 0, sizeof( *s_pWndClass ) );
    s_pWndClass->hInstance = GetModuleHandleW( nullptr );
    s_pWndClass->cbWndExtra = sizeof( DWORD_PTR );
    s_pWndClass->lpfnWndProc = MediaPlayerWndProc;
    s_pWndClass->lpszClassName = L"com_sun_star_media_PlayerWnd";
    s_pWndClass->hbrBackground = static_cast<HBRUSH>(::GetStockObject( BLACK_BRUSH ));
    s_pWndClass->hCursor = ::LoadCursor( nullptr, IDC_ARROW );

    RegisterClassW( s_pWndClass );

    return s_pWndClass;
}

Window::Window( Player& rPlayer ) :
    meZoomLevel( media::ZoomLevel_NOT_AVAILABLE ),
    mrPlayer( rPlayer ),
    mnFrameWnd( nullptr ),
    mnParentWnd( nullptr ),
    mnPointerType( awt::SystemPointer::ARROW )
{
}

Window::~Window()
{
    if( mnFrameWnd )
        ::DestroyWindow( mnFrameWnd );
}

const css::awt::Rectangle Window::getParentPosSize() const
{
    awt::Rectangle aRet;

    if (mnParentWnd)
    {
        ::RECT  aWndRect;

        if (::GetClientRect(mnParentWnd, &aWndRect))
        {
            aRet.X = aWndRect.left;
            aRet.Y = aWndRect.top;
            aRet.Width = aWndRect.right - aWndRect.left + 1;
            aRet.Height = aWndRect.bottom - aWndRect.top + 1;
        }
    }

    return aRet;
}

void Window::ImplLayoutVideoWindow()
{
    if( media::ZoomLevel_NOT_AVAILABLE != meZoomLevel )
    {
        awt::Size           aPrefSize( mrPlayer.getPreferredPlayerWindowSize() );
        awt::Rectangle      aRect = getParentPosSize();
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
                double fPrefWH = static_cast<double>(aPrefSize.Width) / aPrefSize.Height;

                if( fPrefWH < ( static_cast<double>(nVideoW) / nVideoH ) )
                    nVideoW = static_cast<int>( nVideoH * fPrefWH );
                else
                    nVideoH = static_cast<int>( nVideoW / fPrefWH );

                nX = ( nW - nVideoW ) >> 1;
                nY = ( nH - nVideoH ) >> 1;
                nWidth = nVideoW;
                nHeight = nVideoH;
            }
            else
                nX = nY = nWidth = nHeight = 0;
        }

        if (mnFrameWnd && mrPlayer.GetVideoWidth() && mrPlayer.GetVideoHeight() )
            SetWindowPos( mnFrameWnd, HWND_TOP, nX, nY, nWidth, nHeight, 0 );
    }
}

bool Window::create( const uno::Sequence< uno::Any >& rArguments )
{
    static WNDCLASSW* mpWndClass = lcl_getWndClass();

    if( !mnFrameWnd && mpWndClass )
    {
        awt::Rectangle  aRect;
        sal_IntPtr       nWnd;

        rArguments[ 0 ] >>= nWnd;
        rArguments[ 1 ] >>= aRect;

        mnParentWnd = reinterpret_cast<HWND>(nWnd);

        mnFrameWnd = CreateWindowW( mpWndClass->lpszClassName, nullptr,
            WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
            aRect.X, aRect.Y, aRect.Width, aRect.Height,
            mnParentWnd, nullptr, mpWndClass->hInstance, nullptr );

        if( mnFrameWnd )
        {
            SetWindowLongPtrW( mnFrameWnd, 0, reinterpret_cast<LONG_PTR>(this) );
            mrPlayer.setNotifyWnd(mnFrameWnd);
            if (SUCCEEDED(mrPlayer.InitializeWindow(false)))
            {
                sal_IntPtr pIntPtr = 0;
                if ((rArguments.getLength() >= 4) && (rArguments[3] >>= pIntPtr) && pIntPtr)
                {
                    auto pItem = reinterpret_cast<const avmedia::MediaItem*>(pIntPtr);
                    if (pItem->getState() == avmedia::MediaState::Play)
                    {
                        mrPlayer.setAutoPlayBack(true);
                    }
                }
                meZoomLevel = media::ZoomLevel_FIT_TO_WINDOW;
                ImplLayoutVideoWindow();
            }
            else
            {
                ::avmedia::MediaWindow::executeFormatErrorBox(nullptr);
            }
        }
    }

    return( mnFrameWnd != nullptr );
}


void Window::updatePointer()
{
    LPCTSTR pCursorName;

    switch( mnPointerType )
    {
        case awt::SystemPointer::CROSS: pCursorName = IDC_CROSS; break;
        case awt::SystemPointer::MOVE: pCursorName = IDC_SIZEALL; break;
        case awt::SystemPointer::WAIT: pCursorName = IDC_WAIT; break;

        default:
            pCursorName = IDC_ARROW;
        break;
    }

    SetCursor( LoadCursor( nullptr, pCursorName ) );
}

void SAL_CALL Window::update(  )
{
    ::RedrawWindow( mnFrameWnd, nullptr, nullptr, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE  );
}

sal_Bool SAL_CALL Window::setZoomLevel( media::ZoomLevel eZoomLevel )
{
        bool bRet = false;

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

media::ZoomLevel SAL_CALL Window::getZoomLevel(  )
{
    return meZoomLevel;
}

void SAL_CALL Window::setPointerType( sal_Int32 nPointerType )
{
    mnPointerType = nPointerType;
}

void SAL_CALL Window::setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 )
{
    if( mnFrameWnd )
    {
        ::SetWindowPos( mnFrameWnd, HWND_TOP, X, Y, Width, Height, 0 );
        ImplLayoutVideoWindow();
    }
}

awt::Rectangle SAL_CALL Window::getPosSize()
{
    awt::Rectangle aRet;

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
{
    if( mnFrameWnd && mrPlayer.GetVideoWidth() && mrPlayer.GetVideoHeight() )
        ::ShowWindow( mnFrameWnd, bVisible ? SW_SHOW : SW_HIDE );
}

void SAL_CALL Window::setEnable( sal_Bool bEnable )
{
    if( mnFrameWnd )
        ::EnableWindow( mnFrameWnd, bEnable );
}

void SAL_CALL Window::setFocus(  )
{
    if( mnFrameWnd )
        ::SetFocus( mnFrameWnd );
}

void SAL_CALL Window::addWindowListener( const uno::Reference< awt::XWindowListener >& xListener )
{
    std::unique_lock g(maMutex);
    maWindowListeners.addInterface( g, xListener );
}

void SAL_CALL Window::removeWindowListener( const uno::Reference< awt::XWindowListener >& xListener )
{
    std::unique_lock g(maMutex);
    maWindowListeners.removeInterface( g, xListener );
}

void SAL_CALL Window::addFocusListener( const uno::Reference< awt::XFocusListener >& xListener )
{
    std::unique_lock g(maMutex);
    maFocusListeners.addInterface( g, xListener );
}

void SAL_CALL Window::removeFocusListener( const uno::Reference< awt::XFocusListener >& xListener )
{
    std::unique_lock g(maMutex);
    maFocusListeners.removeInterface( g, xListener );
}

void SAL_CALL Window::addKeyListener( const uno::Reference< awt::XKeyListener >& xListener )
{
    std::unique_lock g(maMutex);
    maKeyListeners.addInterface( g, xListener );
}

void SAL_CALL Window::removeKeyListener( const uno::Reference< awt::XKeyListener >& xListener )
{
    std::unique_lock g(maMutex);
    maKeyListeners.removeInterface( g, xListener );
}

void SAL_CALL Window::addMouseListener( const uno::Reference< awt::XMouseListener >& xListener )
{
    std::unique_lock g(maMutex);
    maMouseListeners.addInterface( g, xListener );
}

void SAL_CALL Window::removeMouseListener( const uno::Reference< awt::XMouseListener >& xListener )
{
    std::unique_lock g(maMutex);
    maMouseListeners.removeInterface( g, xListener );
}

void SAL_CALL Window::addMouseMotionListener( const uno::Reference< awt::XMouseMotionListener >& xListener )
{
    std::unique_lock g(maMutex);
    maMouseMotionListeners.addInterface( g, xListener );
}

void SAL_CALL Window::removeMouseMotionListener( const uno::Reference< awt::XMouseMotionListener >& xListener )
{
    std::unique_lock g(maMutex);
    maMouseMotionListeners.removeInterface( g, xListener );
}

void SAL_CALL Window::addPaintListener( const uno::Reference< awt::XPaintListener >& xListener )
{
    std::unique_lock g(maMutex);
    maPaintListeners.addInterface( g, xListener );
}

void SAL_CALL Window::removePaintListener( const uno::Reference< awt::XPaintListener >& xListener )
{
    std::unique_lock g(maMutex);
    maPaintListeners.removeInterface( g, xListener );
}

void SAL_CALL Window::dispose(  )
{
}

void SAL_CALL Window::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
{
    std::unique_lock g(maMutex);
    maEventListeners.addInterface( g, xListener );
}

void SAL_CALL Window::removeEventListener( const uno::Reference< lang::XEventListener >& xListener )
{
    std::unique_lock g(maMutex);
    maEventListeners.removeInterface( g, xListener );
}

void Window::fireMousePressedEvent( const css::awt::MouseEvent& rEvt )
{
    std::unique_lock g(maMutex);
    maMouseListeners.notifyEach(g, &awt::XMouseListener::mousePressed, rEvt);
}

void Window::fireMouseReleasedEvent( const css::awt::MouseEvent& rEvt )
{
    std::unique_lock g(maMutex);
    maMouseListeners.notifyEach(g, &awt::XMouseListener::mouseReleased, rEvt);
}

void Window::fireMouseMovedEvent( const css::awt::MouseEvent& rEvt )
{
    std::unique_lock g(maMutex);
    maMouseMotionListeners.notifyEach(g, &awt::XMouseMotionListener::mouseMoved, rEvt);
}

void Window::fireSetFocusEvent( const css::awt::FocusEvent& rEvt )
{
    std::unique_lock g(maMutex);
    maFocusListeners.notifyEach(g, &awt::XFocusListener::focusGained, rEvt);
}

OUString SAL_CALL Window::getImplementationName(  )
{
    return AVMEDIA_WIN_WINDOW_IMPLEMENTATIONNAME;
}

sal_Bool SAL_CALL Window::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL Window::getSupportedServiceNames(  )
{
    return { AVMEDIA_WIN_WINDOW_SERVICENAME };
}

} // namespace avmedia::win


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
