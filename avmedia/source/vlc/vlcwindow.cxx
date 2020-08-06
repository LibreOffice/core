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

#include <iostream>
#include <cppuhelper/supportsservice.hxx>
#include "vlcwindow.hxx"
#include "vlcplayer.hxx"

using namespace ::com::sun::star;

namespace avmedia::vlc {

namespace
{
    const OUStringLiteral AVMEDIA_VLC_WINDOW_IMPLEMENTATIONNAME = "com.sun.star.comp.avmedia.Window_VLC";
    const OUStringLiteral AVMEDIA_VLC_WINDOW_SERVICENAME = "com.sun.star.media.Window_VLC";
}

VLCWindow::VLCWindow( VLCPlayer& player, const intptr_t prevWinID )
    : mPlayer( player )
    , mPrevWinID( prevWinID )
    , meZoomLevel( media::ZoomLevel_ORIGINAL )
{
}

VLCWindow::~VLCWindow()
{
    if ( mPrevWinID != 0 )
        mPlayer.setWindowID( mPrevWinID );
}

void SAL_CALL VLCWindow::update()
{
}

sal_Bool SAL_CALL VLCWindow::setZoomLevel( css::media::ZoomLevel eZoomLevel )
{
    bool bRet = false;

    if( media::ZoomLevel_NOT_AVAILABLE != meZoomLevel &&
        media::ZoomLevel_NOT_AVAILABLE != eZoomLevel )
    {
        if( eZoomLevel != meZoomLevel )
        {
            meZoomLevel = eZoomLevel;
        }

        switch ( eZoomLevel )
        {
        case media::ZoomLevel_ORIGINAL:
        case media::ZoomLevel_FIT_TO_WINDOW_FIXED_ASPECT:
            mPlayer.setVideoSize( mSize.Width, mSize.Height );
            break;
        case media::ZoomLevel_ZOOM_1_TO_2:
            mPlayer.setVideoSize( mSize.Width / 2, mSize.Height / 2 );
            break;
        case media::ZoomLevel_ZOOM_2_TO_1:
            mPlayer.setVideoSize( mSize.Width * 2, mSize.Height * 2 );
            break;
        default:
            break;
        }

        bRet = true;
    }

    return bRet;
}

css::media::ZoomLevel SAL_CALL VLCWindow::getZoomLevel()
{
    return meZoomLevel;
}

void SAL_CALL VLCWindow::setPointerType( ::sal_Int32 )
{
}

OUString SAL_CALL VLCWindow::getImplementationName()
{
    return AVMEDIA_VLC_WINDOW_IMPLEMENTATIONNAME;
}

sal_Bool SAL_CALL VLCWindow::supportsService( const OUString& serviceName )
{
    return cppu::supportsService(this, serviceName);
}

uno::Sequence< OUString > SAL_CALL VLCWindow::getSupportedServiceNames()
{
    return { AVMEDIA_VLC_WINDOW_SERVICENAME };
}

void SAL_CALL VLCWindow::dispose()
{
}

void SAL_CALL VLCWindow::addEventListener( const uno::Reference< lang::XEventListener >& )
{
}

void SAL_CALL VLCWindow::removeEventListener( const uno::Reference< lang::XEventListener >& )
{
}

void SAL_CALL VLCWindow::setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 /* Flags */ )
{
    mSize.X = X;
    mSize.Y = Y;
    mSize.Width = Width;
    mSize.Height = Height;
}

awt::Rectangle SAL_CALL VLCWindow::getPosSize()
{
    return mSize;
}

void SAL_CALL VLCWindow::setVisible( sal_Bool )
{
}

void SAL_CALL VLCWindow::setEnable( sal_Bool )
{
}

void SAL_CALL VLCWindow::setFocus()
{
}

void SAL_CALL VLCWindow::addWindowListener( const uno::Reference< awt::XWindowListener >& )
{
}

void SAL_CALL VLCWindow::removeWindowListener( const uno::Reference< awt::XWindowListener >& )
{
}

void SAL_CALL VLCWindow::addFocusListener( const uno::Reference< awt::XFocusListener >& )
{
}

void SAL_CALL VLCWindow::removeFocusListener( const uno::Reference< awt::XFocusListener >& )
{
}

void SAL_CALL VLCWindow::addKeyListener( const uno::Reference< awt::XKeyListener >& )
{
}

void SAL_CALL VLCWindow::removeKeyListener( const uno::Reference< awt::XKeyListener >& )
{
}

void SAL_CALL VLCWindow::addMouseListener( const uno::Reference< awt::XMouseListener >& )
{
}

void SAL_CALL VLCWindow::removeMouseListener( const uno::Reference< awt::XMouseListener >& )
{
}

void SAL_CALL VLCWindow::addMouseMotionListener( const uno::Reference< awt::XMouseMotionListener >& )
{
}

void SAL_CALL VLCWindow::removeMouseMotionListener( const uno::Reference< awt::XMouseMotionListener >& )
{
}

void SAL_CALL VLCWindow::addPaintListener( const uno::Reference< awt::XPaintListener >& )
{
}

void SAL_CALL VLCWindow::removePaintListener( const uno::Reference< awt::XPaintListener >& )
{
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
