/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <com/sun/star/awt/SystemPointer.hdl>

#include "window.hxx"
#include "player.hxx"

using namespace ::com::sun::star;

namespace avmedia { namespace xine {

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

Window::Window( Player& rPlayer ) :
    mrPlayer( rPlayer ),
    maListeners( maMutex ),
    meZoomLevel( media::ZoomLevel_NOT_AVAILABLE ),
    mnPointerType( awt::SystemPointer::ARROW )
{
    ::osl::MutexGuard aGuard( ImplGetOwnStaticMutex() );
}

// ------------------------------------------------------------------------------

Window::~Window()
{
}

// ------------------------------------------------------------------------------

void Window::implLayoutVideoWindow()
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

        /*
        IVideoWindow* pVideoWindow = const_cast< IVideoWindow* >( mrPlayer.getVideoWindow() );

        if( pVideoWindow )
            pVideoWindow->SetWindowPosition( nX, nY, nWidth, nHeight );
        */
    }
}

// ------------------------------------------------------------------------------

bool Window::create( const uno::Sequence< uno::Any >& /*rArguments*/ )
{
    bool bRet = false;

    return bRet;
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::update(  )
    throw (uno::RuntimeException)
{
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Window::setZoomLevel( media::ZoomLevel eZoomLevel )
    throw (uno::RuntimeException)
{
    bool bRet = false;

    if( media::ZoomLevel_NOT_AVAILABLE != meZoomLevel &&
        media::ZoomLevel_NOT_AVAILABLE != eZoomLevel )
    {
        if( eZoomLevel != meZoomLevel )
        {
            meZoomLevel = eZoomLevel;
            implLayoutVideoWindow();
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

void SAL_CALL Window::setPosSize( sal_Int32 /*X*/, sal_Int32 /*Y*/, sal_Int32 /*Width*/, sal_Int32 /*Height*/, sal_Int16 /*Flags*/ )
    throw (uno::RuntimeException)
{
    implLayoutVideoWindow();
}

// ------------------------------------------------------------------------------

awt::Rectangle SAL_CALL Window::getPosSize()
    throw (uno::RuntimeException)
{
    awt::Rectangle aRet;

    return aRet;
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::setVisible( sal_Bool /* bVisible */ )
    throw (uno::RuntimeException)
{
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::setEnable( sal_Bool /* bEnable */ )
    throw (uno::RuntimeException)
{
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::setFocus(  )
    throw (uno::RuntimeException)
{
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

::rtl::OUString SAL_CALL Window::getImplementationName(  )
    throw (uno::RuntimeException)
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( AVMEDIA_XINE_WINDOW_IMPLEMENTATIONNAME ) );
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Window::supportsService( const ::rtl::OUString& ServiceName )
    throw (uno::RuntimeException)
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( AVMEDIA_XINE_WINDOW_SERVICENAME ) );
}

// ------------------------------------------------------------------------------

uno::Sequence< ::rtl::OUString > SAL_CALL Window::getSupportedServiceNames(  )
    throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aRet(1);
    aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( AVMEDIA_XINE_WINDOW_SERVICENAME ) );

    return aRet;
}

} // namespace xine
} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
