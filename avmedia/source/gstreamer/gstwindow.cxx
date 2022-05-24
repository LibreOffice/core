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

#include <cppuhelper/supportsservice.hxx>

#include "gstplayer.hxx"
#include "gstwindow.hxx"

constexpr OUStringLiteral AVMEDIA_GST_WINDOW_IMPLEMENTATIONNAME = u"com.sun.star.comp.avmedia.Window_GStreamer";
constexpr OUStringLiteral AVMEDIA_GST_WINDOW_SERVICENAME = u"com.sun.star.media.Window_GStreamer";

using namespace ::com::sun::star;

namespace avmedia::gstreamer {

// Window


Window::Window(Player* pPlayer) :
    meZoomLevel( media::ZoomLevel_NOT_AVAILABLE )
    , m_pPlayer(pPlayer)
{
    m_aPosSize.X = 0;
    m_aPosSize.Y = 0;
    m_aPosSize.Width = 320;
    m_aPosSize.Height = 200;
}

Window::~Window()
{
}

// XPlayerWindow


void SAL_CALL Window::update(  )
{
}

sal_Bool SAL_CALL Window::setZoomLevel( media::ZoomLevel eZoomLevel )
{
        bool bRet = false;

        if( meZoomLevel != media::ZoomLevel_NOT_AVAILABLE &&
            eZoomLevel != media::ZoomLevel_NOT_AVAILABLE )
        {
            if( eZoomLevel != meZoomLevel )
            {
                meZoomLevel = eZoomLevel;
            }

            bRet = true;
        }

        return bRet;
}

media::ZoomLevel SAL_CALL Window::getZoomLevel(  )
{
    return meZoomLevel;
}

void SAL_CALL Window::setPointerType( sal_Int32 /*nPointerType*/ )
{
}

// XWindow


void SAL_CALL Window::setPosSize( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight, sal_Int16 /*Flags*/ )
{
    m_aPosSize.X = nX;
    m_aPosSize.Y = nY;
    m_aPosSize.Width = nWidth;
    m_aPosSize.Height = nHeight;
    if (m_pPlayer)
        m_pPlayer->setPosSize(nX, nY, nWidth, nHeight);
}

awt::Rectangle SAL_CALL Window::getPosSize()
{
    return m_aPosSize;
}

void SAL_CALL Window::setVisible( sal_Bool /*bVisible*/ )
{
}

void SAL_CALL Window::setEnable( sal_Bool /*bEnable*/ )
{
}

void SAL_CALL Window::setFocus(  )
{
}

void SAL_CALL Window::addWindowListener( const uno::Reference< awt::XWindowListener >& )
{
}

void SAL_CALL Window::removeWindowListener( const uno::Reference< awt::XWindowListener >& )
{
}

void SAL_CALL Window::addFocusListener( const uno::Reference< awt::XFocusListener >& )
{
}

void SAL_CALL Window::removeFocusListener( const uno::Reference< awt::XFocusListener >& )
{
}

void SAL_CALL Window::addKeyListener( const uno::Reference< awt::XKeyListener >& )
{
}

void SAL_CALL Window::removeKeyListener( const uno::Reference< awt::XKeyListener >& )
{
}

void SAL_CALL Window::addMouseListener( const uno::Reference< awt::XMouseListener >& )
{
}

void SAL_CALL Window::removeMouseListener( const uno::Reference< awt::XMouseListener >& )
{
}

void SAL_CALL Window::addMouseMotionListener( const uno::Reference< awt::XMouseMotionListener >& )
{
}

void SAL_CALL Window::removeMouseMotionListener( const uno::Reference< awt::XMouseMotionListener >& )
{
}

void SAL_CALL Window::addPaintListener( const uno::Reference< awt::XPaintListener >& )
{
}

void SAL_CALL Window::removePaintListener( const uno::Reference< awt::XPaintListener >& )
{
}

// XComponent


void SAL_CALL Window::dispose(  )
{
}

void SAL_CALL Window::addEventListener( const uno::Reference< lang::XEventListener >& )
{
}

void SAL_CALL Window::removeEventListener( const uno::Reference< lang::XEventListener >& )
{
}

// XServiceInfo


OUString SAL_CALL Window::getImplementationName(  )
{
    return AVMEDIA_GST_WINDOW_IMPLEMENTATIONNAME;
}

sal_Bool SAL_CALL Window::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL Window::getSupportedServiceNames(  )
{
    return { AVMEDIA_GST_WINDOW_SERVICENAME };
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
