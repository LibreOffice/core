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


namespace avmedia { namespace macavf {

// ---------------
// - Window -
// ---------------

Window::Window( const uno::Reference< lang::XMultiServiceFactory >& i_rxMgr, Player& i_rPlayer, NSView* i_pParentView )
:   mxMgr( i_rxMgr )
,   maListeners( maMutex )
,   meZoomLevel( media::ZoomLevel_NOT_AVAILABLE )
,   mrPlayer( i_rPlayer )
,   mnPointerType( awt::SystemPointer::ARROW )
,   mpView( i_pParentView )
,   mpPlayerLayer( nullptr )
{
    OSL_TRACE ("Constructing an avmedia::macavf::Window");
    if( !mpView ) // sanity check
        return;

    // check the media asset for video content
    AVPlayer* pAVPlayer = mrPlayer.getAVPlayer();
    AVAsset* pMovie = [[pAVPlayer currentItem] asset];
    const int nVideoCount = [pMovie tracksWithMediaType:AVMediaTypeVideo].count;
    const int nAudioCount = [pMovie tracksWithMediaType:AVMediaTypeAudio].count;
    OSL_TRACE( "Found %d video and %d audio tracks.", nVideoCount, nAudioCount );
    (void)nAudioCount;
    if( nVideoCount <= 0 )
        return;

    // setup the AVPlayerLayer
    [pAVPlayer retain];
    [pAVPlayer pause];
    mpPlayerLayer = [AVPlayerLayer playerLayerWithPlayer:pAVPlayer];
    [mpPlayerLayer retain];
    NSRect viewFrame = [mpView frame];
    [mpPlayerLayer setFrame:CGRectMake(viewFrame.origin.x, viewFrame.origin.y, viewFrame.size.width, viewFrame.size.height)];
    [mpPlayerLayer setHidden:YES];
    [mpPlayerLayer setVideoGravity:AVLayerVideoGravityResizeAspectFill];
    [mpPlayerLayer addObserver:getObserver() forKeyPath:@"readyForDisplay" options:0 context:this];

    // setup the target view
    [mpView setWantsLayer:YES];
    [mpView.layer addSublayer:mpPlayerLayer];
}

// ------------------------------------------------------------------------------

Window::~Window()
{
    [mpPlayerLayer removeObserver:getObserver() forKeyPath:@"readyForDisplay"];
    [mpPlayerLayer release];
}

// ------------------------------------------------------------------------------

bool Window::handleObservation( NSString* pKeyPath )
{
    OSL_TRACE( "AVPlayer::handleObservation key=\"%s\"", [pKeyPath UTF8String]);
    const BOOL bReadyForDisplay = [mpPlayerLayer isReadyForDisplay];
    [mpPlayerLayer setHidden:!bReadyForDisplay];
    return true;
}

// XPlayerWindow
// ------------------------------------------------------------------------------

void SAL_CALL Window::update()
    throw (uno::RuntimeException)
{}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Window::setZoomLevel( media::ZoomLevel /* eZoomLevel */ )
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

void SAL_CALL Window::setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 /* Flags */ )
    throw (uno::RuntimeException)
{
    OSL_TRACE( "AVWindow::setPosSize( %dx%d%+d%+d)", (int)Width,(int)Height,(int)X,(int)Y);//######
    if( !mpView )
        return;
    NSRect aRect = [mpView frame];
    // NOTE: if( (Flags & awt::PosSize::WIDTH) )
    aRect.size.width = Width;
    // NOTE: if( (Flags & awt::PosSize::HEIGHT) )
    aRect.size.height = Height;

    [mpView setFrameSize: aRect.size];
    NSRect viewFrame = [mpView frame];
    [mpPlayerLayer setFrame:CGRectMake(viewFrame.origin.x, viewFrame.origin.y, viewFrame.size.width, viewFrame.size.height)];
}

// ------------------------------------------------------------------------------

awt::Rectangle SAL_CALL Window::getPosSize()
    throw (uno::RuntimeException)
{
    awt::Rectangle aRet;

    NSRect aRect = [mpView frame];
    aRet.X = aRet.Y = 0;
    aRet.Width = aRect.size.width;
    aRet.Height = aRect.size.height;

    return aRet;
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::setVisible( sal_Bool bVisible )
    throw (uno::RuntimeException)
{
    OSL_TRACE ("Window::setVisible(%d)", bVisible);
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::setEnable( sal_Bool bEnable )
    throw (uno::RuntimeException)
{
    OSL_TRACE ("Window::setEnable(%d)", bEnable);
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::setFocus()
    throw (uno::RuntimeException)
{
    OSL_TRACE ("Window::setFocus");
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::addWindowListener( const uno::Reference< awt::XWindowListener >& xListener )
    throw (uno::RuntimeException)
{
    maListeners.addInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::removeWindowListener( const uno::Reference< awt::XWindowListener >& xListener )
    throw (uno::RuntimeException)
{
    maListeners.removeInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::addFocusListener( const uno::Reference< awt::XFocusListener >& xListener )
    throw (uno::RuntimeException)
{
    maListeners.addInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::removeFocusListener( const uno::Reference< awt::XFocusListener >& xListener )
    throw (uno::RuntimeException)
{
    maListeners.removeInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::addKeyListener( const uno::Reference< awt::XKeyListener >& xListener )
    throw (uno::RuntimeException)
{
    maListeners.addInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::removeKeyListener( const uno::Reference< awt::XKeyListener >& xListener )
    throw (uno::RuntimeException)
{
    maListeners.removeInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::addMouseListener( const uno::Reference< awt::XMouseListener >& xListener )
    throw (uno::RuntimeException)
{
    maListeners.addInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::removeMouseListener( const uno::Reference< awt::XMouseListener >& xListener )
    throw (uno::RuntimeException)
{
    maListeners.removeInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::addMouseMotionListener( const uno::Reference< awt::XMouseMotionListener >& xListener )
    throw (uno::RuntimeException)
{
    maListeners.addInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::removeMouseMotionListener( const uno::Reference< awt::XMouseMotionListener >& xListener )
    throw (uno::RuntimeException)
{
    maListeners.removeInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::addPaintListener( const uno::Reference< awt::XPaintListener >& xListener )
    throw (uno::RuntimeException)
{
    maListeners.addInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::removePaintListener( const uno::Reference< awt::XPaintListener >& xListener )
    throw (uno::RuntimeException)
{
    maListeners.removeInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
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
    maListeners.addInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

// ------------------------------------------------------------------------------

void SAL_CALL Window::removeEventListener( const uno::Reference< lang::XEventListener >& xListener )
    throw (uno::RuntimeException)
{
    maListeners.removeInterface( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

// XServiceInfo
// ------------------------------------------------------------------------------

::rtl::OUString SAL_CALL Window::getImplementationName(  )
    throw (uno::RuntimeException)
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( AVMEDIA_MACAVF_WINDOW_IMPLEMENTATIONNAME ) );
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Window::supportsService( const ::rtl::OUString& ServiceName )
    throw (uno::RuntimeException)
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( AVMEDIA_MACAVF_WINDOW_SERVICENAME ) );
}

// ------------------------------------------------------------------------------

uno::Sequence< ::rtl::OUString > SAL_CALL Window::getSupportedServiceNames(  )
    throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aRet { AVMEDIA_MACAVF_WINDOW_SERVICENAME };
    return aRet;
}

} // namespace macavf
} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
