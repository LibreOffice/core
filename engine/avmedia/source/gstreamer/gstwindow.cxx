/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <cppuhelper/supportsservice.hxx>

#include "gstwindow.hxx"

constexpr OUString AVMEDIA_GST_WINDOW_IMPLEMENTATIONNAME = u"com.sun.star.comp.avmedia.Window_GStreamer"_ustr;
constexpr OUString AVMEDIA_GST_WINDOW_SERVICENAME = u"com.sun.star.media.Window_GStreamer"_ustr;

using namespace ::com::sun::star;

namespace avmedia::gstreamer {

// Window


Window::Window() :
    meZoomLevel( media::ZoomLevel_NOT_AVAILABLE )
{
}

Window::~Window()
{
}

// XPlayerWindow


void Window::update(  )
{
}

bool Window::setZoomLevel( media::ZoomLevel eZoomLevel )
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

media::ZoomLevel Window::getZoomLevel(  )
{
    return meZoomLevel;
}

void Window::setPointerType( sal_Int32 /*nPointerType*/ )
{
}

// XWindow


void Window::setPosSize( sal_Int32 /*X*/, sal_Int32 /*Y*/, sal_Int32 /*Width*/, sal_Int32 /*Height*/, sal_Int16 /*Flags*/ )
{
}

awt::Rectangle Window::getPosSize()
{
    awt::Rectangle aRet;

    aRet.X = aRet.Y = 0;
    aRet.Width = 320;
    aRet.Height = 240;

    return aRet;
}

void Window::setVisible( bool /*bVisible*/ )
{
}

void Window::setEnable( bool /*bEnable*/ )
{
}

void Window::setFocus(  )
{
}

void Window::addWindowListener( const uno::Reference< awt::XWindowListener >& )
{
}

void Window::removeWindowListener( const uno::Reference< awt::XWindowListener >& )
{
}

void Window::addFocusListener( const uno::Reference< awt::XFocusListener >& )
{
}

void Window::removeFocusListener( const uno::Reference< awt::XFocusListener >& )
{
}

void Window::addKeyListener( const uno::Reference< awt::XKeyListener >& )
{
}

void Window::removeKeyListener( const uno::Reference< awt::XKeyListener >& )
{
}

void Window::addMouseListener( const uno::Reference< awt::XMouseListener >& )
{
}

void Window::removeMouseListener( const uno::Reference< awt::XMouseListener >& )
{
}

void Window::addMouseMotionListener( const uno::Reference< awt::XMouseMotionListener >& )
{
}

void Window::removeMouseMotionListener( const uno::Reference< awt::XMouseMotionListener >& )
{
}

void Window::addPaintListener( const uno::Reference< awt::XPaintListener >& )
{
}

void Window::removePaintListener( const uno::Reference< awt::XPaintListener >& )
{
}

// XComponent


void Window::dispose(  )
{
}

void Window::addEventListener( const uno::Reference< lang::XEventListener >& )
{
}

void Window::removeEventListener( const uno::Reference< lang::XEventListener >& )
{
}

// XServiceInfo


OUString Window::getImplementationName(  )
{
    return AVMEDIA_GST_WINDOW_IMPLEMENTATIONNAME;
}

bool Window::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > Window::getSupportedServiceNames(  )
{
    return { AVMEDIA_GST_WINDOW_SERVICENAME };
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
