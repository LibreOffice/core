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
    meZoomLevel( css::media::ZoomLevel_NOT_AVAILABLE ),
    mnPointerType( css::awt::SystemPointer::ARROW )
{
    ::osl::MutexGuard aGuard( ImplGetOwnStaticMutex() );
}

Window::~Window()
{
}

// XPlayerWindow


void SAL_CALL Window::update(  )
    throw (css::uno::RuntimeException, std::exception)
{
}

sal_Bool SAL_CALL Window::setZoomLevel( css::media::ZoomLevel eZoomLevel )
    throw (css::uno::RuntimeException, std::exception)
{
        bool bRet = false;

        if( css::media::ZoomLevel_NOT_AVAILABLE != meZoomLevel &&
            css::media::ZoomLevel_NOT_AVAILABLE != eZoomLevel )
        {
            if( eZoomLevel != meZoomLevel )
            {
                meZoomLevel = eZoomLevel;
            }

            bRet = true;
        }

        return bRet;
}

css::media::ZoomLevel SAL_CALL Window::getZoomLevel(  )
    throw (css::uno::RuntimeException, std::exception)
{
    return meZoomLevel;
}

void SAL_CALL Window::setPointerType( sal_Int32 nPointerType )
    throw (css::uno::RuntimeException, std::exception)
{
    mnPointerType = nPointerType;
}

// XWindow


void SAL_CALL Window::setPosSize( sal_Int32 /*X*/, sal_Int32 /*Y*/, sal_Int32 /*Width*/, sal_Int32 /*Height*/, sal_Int16 /*Flags*/ )
    throw (css::uno::RuntimeException, std::exception)
{
}

css::awt::Rectangle SAL_CALL Window::getPosSize()
    throw (css::uno::RuntimeException, std::exception)
{
    css::awt::Rectangle aRet;

    aRet.X = aRet.Y = 0;
    aRet.Width = 320;
    aRet.Height = 240;

    return aRet;
}

void SAL_CALL Window::setVisible( sal_Bool /*bVisible*/ )
    throw (css::uno::RuntimeException, std::exception)
{
}

void SAL_CALL Window::setEnable( sal_Bool /*bEnable*/ )
    throw (css::uno::RuntimeException, std::exception)
{
}

void SAL_CALL Window::setFocus(  )
    throw (css::uno::RuntimeException, std::exception)
{
}

void SAL_CALL Window::addWindowListener( const css::uno::Reference< css::awt::XWindowListener >& )
    throw (css::uno::RuntimeException, std::exception)
{
}

void SAL_CALL Window::removeWindowListener( const css::uno::Reference< css::awt::XWindowListener >& )
    throw (css::uno::RuntimeException, std::exception)
{
}

void SAL_CALL Window::addFocusListener( const css::uno::Reference< css::awt::XFocusListener >& )
    throw (css::uno::RuntimeException, std::exception)
{
}

void SAL_CALL Window::removeFocusListener( const css::uno::Reference< css::awt::XFocusListener >& )
    throw (css::uno::RuntimeException, std::exception)
{
}

void SAL_CALL Window::addKeyListener( const css::uno::Reference< css::awt::XKeyListener >& )
    throw (css::uno::RuntimeException, std::exception)
{
}

void SAL_CALL Window::removeKeyListener( const css::uno::Reference< css::awt::XKeyListener >& )
    throw (css::uno::RuntimeException, std::exception)
{
}

void SAL_CALL Window::addMouseListener( const css::uno::Reference< css::awt::XMouseListener >& )
    throw (css::uno::RuntimeException, std::exception)
{
}

void SAL_CALL Window::removeMouseListener( const css::uno::Reference< css::awt::XMouseListener >& )
    throw (css::uno::RuntimeException, std::exception)
{
}

void SAL_CALL Window::addMouseMotionListener( const css::uno::Reference< css::awt::XMouseMotionListener >& )
    throw (css::uno::RuntimeException, std::exception)
{
}

void SAL_CALL Window::removeMouseMotionListener( const css::uno::Reference< css::awt::XMouseMotionListener >& )
    throw (css::uno::RuntimeException, std::exception)
{
}

void SAL_CALL Window::addPaintListener( const css::uno::Reference< css::awt::XPaintListener >& )
    throw (css::uno::RuntimeException, std::exception)
{
}

void SAL_CALL Window::removePaintListener( const css::uno::Reference< css::awt::XPaintListener >& )
    throw (css::uno::RuntimeException, std::exception)
{
}

// XComponent


void SAL_CALL Window::dispose(  )
    throw (css::uno::RuntimeException, std::exception)
{
}

void SAL_CALL Window::addEventListener( const css::uno::Reference< css::lang::XEventListener >& )
    throw (css::uno::RuntimeException, std::exception)
{
}

void SAL_CALL Window::removeEventListener( const css::uno::Reference< css::lang::XEventListener >& )
    throw (css::uno::RuntimeException, std::exception)
{
}

// XServiceInfo


OUString SAL_CALL Window::getImplementationName(  )
    throw (css::uno::RuntimeException, std::exception)
{
    return OUString( AVMEDIA_GST_WINDOW_IMPLEMENTATIONNAME );
}

sal_Bool SAL_CALL Window::supportsService( const OUString& ServiceName )
    throw (css::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > SAL_CALL Window::getSupportedServiceNames(  )
    throw (css::uno::RuntimeException, std::exception)
{
    css::uno::Sequence<OUString> aRet { AVMEDIA_GST_WINDOW_SERVICENAME };

    return aRet;
}

} // namespace gstreamer
} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
