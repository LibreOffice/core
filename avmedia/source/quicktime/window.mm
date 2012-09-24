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

namespace avmedia { namespace quicktime {

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

// ---------------
// - Window -
// ---------------

// ------------------------------------------------------------------------------

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

    OSL_TRACE ("Window::Window");
}

// ------------------------------------------------------------------------------

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

bool Window::create( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& )
{
    return true;
}

// XPlayerWindow
// ------------------------------------------------------------------------------

void SAL_CALL Window::update(  )
    throw (uno::RuntimeException)
{
    ;
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Window::setZoomLevel( media::ZoomLevel )
    throw (uno::RuntimeException)
{
        return false;
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

// XWindow
// ------------------------------------------------------------------------------

void SAL_CALL Window::setPosSize( sal_Int32 , sal_Int32 , sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags )
    throw (uno::RuntimeException)
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

// ------------------------------------------------------------------------------

awt::Rectangle SAL_CALL Window::getPosSize()
    throw (uno::RuntimeException)
{
    awt::Rectangle aRet;

    NSRect aRect = [mpMovieView frame];
    aRet.X = aRet.Y = 0;
    aRet.Width = aRect.size.width;
    aRet.Height = aRect.size.height;

    return aRet;
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::setVisible( sal_Bool )
    throw (uno::RuntimeException)
{
    OSL_TRACE ("Window::setVisible");

}

// ------------------------------------------------------------------------------

void SAL_CALL Window::setEnable( sal_Bool )
    throw (uno::RuntimeException)
{
    ;
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::setFocus(  )
    throw (uno::RuntimeException)
{
    OSL_TRACE ("Window::setFocus");
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

// XComponent
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

// XServiceInfo
// ------------------------------------------------------------------------------

::rtl::OUString SAL_CALL Window::getImplementationName(  )
    throw (uno::RuntimeException)
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( AVMEDIA_QUICKTIME_WINDOW_IMPLEMENTATIONNAME ) );
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Window::supportsService( const ::rtl::OUString& ServiceName )
    throw (uno::RuntimeException)
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( AVMEDIA_QUICKTIME_WINDOW_SERVICENAME ) );
}

// ------------------------------------------------------------------------------

uno::Sequence< ::rtl::OUString > SAL_CALL Window::getSupportedServiceNames(  )
    throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aRet(1);
    aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( AVMEDIA_QUICKTIME_WINDOW_SERVICENAME ) );

    return aRet;
}

} // namespace quicktime
} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
