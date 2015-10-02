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

Window::Window( const uno::Reference< lang::XMultiServiceFactory >& i_rxMgr, Player& i_rPlayer, NSView* i_pParentView ) :
    mxMgr( i_rxMgr ),
    maListeners( maMutex ),
    meZoomLevel( media::ZoomLevel_NOT_AVAILABLE ),
    mrPlayer( i_rPlayer ),
    mnPointerType( awt::SystemPointer::ARROW ),
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

Window::~Window()
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

void SAL_CALL Window::update(  )
{
    ;
}

sal_Bool SAL_CALL Window::setZoomLevel( media::ZoomLevel )
{
        return false;
}

media::ZoomLevel SAL_CALL Window::getZoomLevel(  )
{
    return meZoomLevel;
}

void SAL_CALL Window::setPointerType( sal_Int32 nPointerType )
{
    mnPointerType = nPointerType;
}

// XWindow

void SAL_CALL Window::setPosSize( sal_Int32 , sal_Int32 , sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags )
{
    if( mpParentView && mpMovieView )
    {
        NSRect aRect = [mpMovieView frame];
        if( (Flags & awt::PosSize::WIDTH) )
            aRect.size.width = Width;
        if( (Flags & awt::PosSize::HEIGHT) )
            aRect.size.height = Height;
    }
}

awt::Rectangle SAL_CALL Window::getPosSize()
{
    awt::Rectangle aRet;

    NSRect aRect = [mpMovieView frame];
    aRet.X = aRet.Y = 0;
    aRet.Width = aRect.size.width;
    aRet.Height = aRect.size.height;

    return aRet;
}

void SAL_CALL Window::setVisible( sal_Bool )
{
    SAL_INFO ( "avmedia.quicktime", "Window::setVisible" );

}

void SAL_CALL Window::setEnable( sal_Bool )
{
    ;
}

void SAL_CALL Window::setFocus(  )
{
    SAL_INFO ( "avmedia.quicktime", "Window::setFocus" );
}

void SAL_CALL Window::addWindowListener( const uno::Reference< awt::XWindowListener >& xListener )
{
    maListeners.addInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL Window::removeWindowListener( const uno::Reference< awt::XWindowListener >& xListener )
{
    maListeners.removeInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL Window::addFocusListener( const uno::Reference< awt::XFocusListener >& xListener )
{
    maListeners.addInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL Window::removeFocusListener( const uno::Reference< awt::XFocusListener >& xListener )
{
    maListeners.removeInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL Window::addKeyListener( const uno::Reference< awt::XKeyListener >& xListener )
{
    maListeners.addInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL Window::removeKeyListener( const uno::Reference< awt::XKeyListener >& xListener )
{
    maListeners.removeInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL Window::addMouseListener( const uno::Reference< awt::XMouseListener >& xListener )
{
    maListeners.addInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL Window::removeMouseListener( const uno::Reference< awt::XMouseListener >& xListener )
{
    maListeners.removeInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL Window::addMouseMotionListener( const uno::Reference< awt::XMouseMotionListener >& xListener )
{
    maListeners.addInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL Window::removeMouseMotionListener( const uno::Reference< awt::XMouseMotionListener >& xListener )
{
    maListeners.removeInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL Window::addPaintListener( const uno::Reference< awt::XPaintListener >& xListener )
{
    maListeners.addInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL Window::removePaintListener( const uno::Reference< awt::XPaintListener >& xListener )
{
    maListeners.removeInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

// XComponent

void SAL_CALL Window::dispose(  )
{
}

void SAL_CALL Window::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
{
    maListeners.addInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void SAL_CALL Window::removeEventListener( const uno::Reference< lang::XEventListener >& xListener )
{
    maListeners.removeInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

// XServiceInfo

::rtl::OUString SAL_CALL Window::getImplementationName(  )
{
    return ::rtl::OUString( AVMEDIA_QUICKTIME_WINDOW_IMPLEMENTATIONNAME );
}

sal_Bool SAL_CALL Window::supportsService( const ::rtl::OUString& ServiceName )
{
    return ( ServiceName == AVMEDIA_QUICKTIME_WINDOW_SERVICENAME );
}

uno::Sequence< ::rtl::OUString > SAL_CALL Window::getSupportedServiceNames(  )
{
    return { AVMEDIA_QUICKTIME_WINDOW_SERVICENAME };
}

} // namespace quicktime
} // namespace avmedia

SAL_WNODEPRECATED_DECLARATIONS_POP

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
