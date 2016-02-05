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

#include <com/sun/star/awt/SystemPointer.hpp>
#include <com/sun/star/awt/PosSize.hpp>

#include "window.hxx"
#include "player.hxx"

using namespace ::com::sun::star;

SAL_WNODEPRECATED_DECLARATIONS_PUSH //TODO: 10.9

namespace avmedia { namespace quicktime {

static ::osl::Mutex& ImplGetOwnStaticMutex()
{
    static ::osl::Mutex* pMutex = nullptr;

    if( pMutex == nullptr )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );

        if( pMutex == nullptr )
        {
            static ::osl::Mutex aMutex;
            pMutex = &aMutex;
        }
    }

    return *pMutex;
}

avmedia::quicktime::Window::Window( const css::uno::Reference< css::lang::XMultiServiceFactory >& i_rxMgr, Player& i_rPlayer, NSView* i_pParentView ) :
    mxMgr( i_rxMgr ),
    maListeners( maMutex ),
    meZoomLevel( css::media::ZoomLevel_NOT_AVAILABLE ),
    mrPlayer( i_rPlayer ),
    mnPointerType( css::awt::SystemPointer::ARROW ),
    mpParentView( i_pParentView ),
    mpMovieView( nil )
{

    ::osl::MutexGuard aGuard( ImplGetOwnStaticMutex() );

    if( mpParentView ) // sanity check
    {

        NSRect aViewRect = [mpParentView frame];
        aViewRect.origin.x = aViewRect.origin.y = 0;
        mpMovieView = [[QTMovieView alloc] initWithFrame: aViewRect];
        [mpMovieView setMovie: mrPlayer.getMovie() ];
        [mpMovieView setControllerVisible: NO];
        [mpMovieView setPreservesAspectRatio: YES];
        [mpMovieView setAutoresizingMask: NSViewWidthSizable | NSViewHeightSizable];
        [mpParentView addSubview: mpMovieView];
        [mpParentView setAutoresizesSubviews: YES];
    }

    SAL_INFO ( "avmedia.quicktime", "Window::Window" );
}

avmedia::quicktime::Window::~Window()
{
    if( mpMovieView )
    {
        [mpMovieView removeFromSuperview];
        [mpMovieView setMovie:nil];
        [mpMovieView release];
        mpMovieView = nil;
    }
}

// XPlayerWindow

void SAL_CALL avmedia::quicktime::Window::update(  )
    throw (css::uno::RuntimeException)
{
    ;
}

sal_Bool SAL_CALL avmedia::quicktime::Window::setZoomLevel( css::media::ZoomLevel )
    throw (css::uno::RuntimeException)
{
        return false;
}

media::ZoomLevel SAL_CALL avmedia::quicktime::Window::getZoomLevel(  )
    throw (css::uno::RuntimeException)
{
    return meZoomLevel;
}

void SAL_CALL avmedia::quicktime::Window::setPointerType( sal_Int32 nPointerType )
    throw (css::uno::RuntimeException)
{
    mnPointerType = nPointerType;
}

// XWindow

void SAL_CALL avmedia::quicktime::Window::setPosSize( sal_Int32 , sal_Int32 , sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags )
    throw (css::uno::RuntimeException)
{
    if( mpParentView && mpMovieView )
    {
        NSRect aRect = [mpMovieView frame];
        if( (Flags & css::awt::PosSize::WIDTH) )
            aRect.size.width = Width;
        if( (Flags & css::awt::PosSize::HEIGHT) )
            aRect.size.height = Height;
    }
}

css::awt::Rectangle SAL_CALL avmedia::quicktime::Window::getPosSize()
    throw (css::uno::RuntimeException)
{
    css::awt::Rectangle aRet;

    NSRect aRect = [mpMovieView frame];
    aRet.X = aRet.Y = 0;
    aRet.Width = aRect.size.width;
    aRet.Height = aRect.size.height;

    return aRet;
}

void SAL_CALL avmedia::quicktime::Window::setVisible( sal_Bool )
    throw (css::uno::RuntimeException)
{
    SAL_INFO ( "avmedia.quicktime", "Window::setVisible" );

}

void SAL_CALL avmedia::quicktime::Window::setEnable( sal_Bool )
    throw (css::uno::RuntimeException)
{
    ;
}

void SAL_CALL avmedia::quicktime::Window::setFocus(  )
    throw (css::uno::RuntimeException)
{
    SAL_INFO ( "avmedia.quicktime", "Window::setFocus" );
}

void SAL_CALL avmedia::quicktime::Window::addWindowListener( const css::uno::Reference< css::awt::XWindowListener >& xListener )
    throw (css::uno::RuntimeException)
{
    maListeners.addInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL avmedia::quicktime::Window::removeWindowListener( const css::uno::Reference< css::awt::XWindowListener >& xListener )
    throw (css::uno::RuntimeException)
{
    maListeners.removeInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL avmedia::quicktime::Window::addFocusListener( const css::uno::Reference< css::awt::XFocusListener >& xListener )
    throw (css::uno::RuntimeException)
{
    maListeners.addInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL avmedia::quicktime::Window::removeFocusListener( const css::uno::Reference< css::awt::XFocusListener >& xListener )
    throw (css::uno::RuntimeException)
{
    maListeners.removeInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL avmedia::quicktime::Window::addKeyListener( const css::uno::Reference< css::awt::XKeyListener >& xListener )
    throw (css::uno::RuntimeException)
{
    maListeners.addInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL avmedia::quicktime::Window::removeKeyListener( const css::uno::Reference< css::awt::XKeyListener >& xListener )
    throw (css::uno::RuntimeException)
{
    maListeners.removeInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL avmedia::quicktime::Window::addMouseListener( const css::uno::Reference< css::awt::XMouseListener >& xListener )
    throw (css::uno::RuntimeException)
{
    maListeners.addInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL avmedia::quicktime::Window::removeMouseListener( const css::uno::Reference< css::awt::XMouseListener >& xListener )
    throw (css::uno::RuntimeException)
{
    maListeners.removeInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL avmedia::quicktime::Window::addMouseMotionListener( const css::uno::Reference< css::awt::XMouseMotionListener >& xListener )
    throw (css::uno::RuntimeException)
{
    maListeners.addInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL avmedia::quicktime::Window::removeMouseMotionListener( const css::uno::Reference< css::awt::XMouseMotionListener >& xListener )
    throw (css::uno::RuntimeException)
{
    maListeners.removeInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL avmedia::quicktime::Window::addPaintListener( const css::uno::Reference< css::awt::XPaintListener >& xListener )
    throw (css::uno::RuntimeException)
{
    maListeners.addInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL avmedia::quicktime::Window::removePaintListener( const css::uno::Reference< css::awt::XPaintListener >& xListener )
    throw (css::uno::RuntimeException)
{
    maListeners.removeInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

// XComponent

void SAL_CALL avmedia::quicktime::Window::dispose(  )
    throw (css::uno::RuntimeException)
{
}

void SAL_CALL avmedia::quicktime::Window::addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener )
    throw (css::uno::RuntimeException)
{
    maListeners.addInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL avmedia::quicktime::Window::removeEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener )
    throw (css::uno::RuntimeException)
{
    maListeners.removeInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

// XServiceInfo

OUString SAL_CALL avmedia::quicktime::Window::getImplementationName(  )
    throw (css::uno::RuntimeException)
{
    return OUString( AVMEDIA_QUICKTIME_WINDOW_IMPLEMENTATIONNAME );
}

sal_Bool SAL_CALL avmedia::quicktime::Window::supportsService( const OUString& ServiceName )
    throw (css::uno::RuntimeException)
{
    return ( ServiceName == AVMEDIA_QUICKTIME_WINDOW_SERVICENAME );
}

css::uno::Sequence< OUString > SAL_CALL avmedia::quicktime::Window::getSupportedServiceNames(  )
    throw (css::uno::RuntimeException)
{
    css::uno::Sequence<OUString> aRet { AVMEDIA_QUICKTIME_WINDOW_SERVICENAME };

    return aRet;
}

} // namespace quicktime
} // namespace avmedia

SAL_WNODEPRECATED_DECLARATIONS_POP

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
