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

#include "gstwindow.hxx"
#include "gstplayer.hxx"

#define AVMEDIA_GST_WINDOW_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.Window_GStreamer"
#define AVMEDIA_GST_WINDOW_SERVICENAME "com.sun.star.media.Window_GStreamer"

using namespace ::com::sun::star;

namespace avmedia { namespace gstreamer {

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

// Window


Window::Window() :
    meZoomLevel( media::ZoomLevel_NOT_AVAILABLE ),
    mnPointerType( awt::SystemPointer::ARROW )
{
    ::osl::MutexGuard aGuard( ImplGetOwnStaticMutex() );
}

Window::~Window()
{
}

// XPlayerWindow


void SAL_CALL Window::update(  )
    throw (uno::RuntimeException, std::exception)
{
}

sal_Bool SAL_CALL Window::setZoomLevel( media::ZoomLevel eZoomLevel )
    throw (uno::RuntimeException, std::exception)
{
        bool bRet = false;

        if( media::ZoomLevel_NOT_AVAILABLE != meZoomLevel &&
            media::ZoomLevel_NOT_AVAILABLE != eZoomLevel )
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
    throw (uno::RuntimeException, std::exception)
{
    return meZoomLevel;
}

void SAL_CALL Window::setPointerType( sal_Int32 nPointerType )
    throw (uno::RuntimeException, std::exception)
{
    mnPointerType = nPointerType;
}

// XWindow


void SAL_CALL Window::setPosSize( sal_Int32 /*X*/, sal_Int32 /*Y*/, sal_Int32 /*Width*/, sal_Int32 /*Height*/, sal_Int16 /*Flags*/ )
    throw (uno::RuntimeException, std::exception)
{
}

awt::Rectangle SAL_CALL Window::getPosSize()
    throw (uno::RuntimeException, std::exception)
{
    awt::Rectangle aRet;

    aRet.X = aRet.Y = 0;
    aRet.Width = 320;
    aRet.Height = 240;

    return aRet;
}

void SAL_CALL Window::setVisible( sal_Bool /*bVisible*/ )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL Window::setEnable( sal_Bool /*bEnable*/ )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL Window::setFocus(  )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL Window::addWindowListener( const uno::Reference< awt::XWindowListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL Window::removeWindowListener( const uno::Reference< awt::XWindowListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL Window::addFocusListener( const uno::Reference< awt::XFocusListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL Window::removeFocusListener( const uno::Reference< awt::XFocusListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL Window::addKeyListener( const uno::Reference< awt::XKeyListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL Window::removeKeyListener( const uno::Reference< awt::XKeyListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL Window::addMouseListener( const uno::Reference< awt::XMouseListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL Window::removeMouseListener( const uno::Reference< awt::XMouseListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL Window::addMouseMotionListener( const uno::Reference< awt::XMouseMotionListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL Window::removeMouseMotionListener( const uno::Reference< awt::XMouseMotionListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL Window::addPaintListener( const uno::Reference< awt::XPaintListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL Window::removePaintListener( const uno::Reference< awt::XPaintListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

// XComponent


void SAL_CALL Window::dispose(  )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL Window::addEventListener( const uno::Reference< lang::XEventListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL Window::removeEventListener( const uno::Reference< lang::XEventListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

// XServiceInfo


OUString SAL_CALL Window::getImplementationName(  )
    throw (uno::RuntimeException, std::exception)
{
    return OUString( AVMEDIA_GST_WINDOW_IMPLEMENTATIONNAME );
}

sal_Bool SAL_CALL Window::supportsService( const OUString& ServiceName )
    throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL Window::getSupportedServiceNames(  )
    throw (uno::RuntimeException, std::exception)
{
    return { AVMEDIA_GST_WINDOW_SERVICENAME };
}

} // namespace gstreamer
} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
