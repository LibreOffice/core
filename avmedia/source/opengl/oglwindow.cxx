/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "oglwindow.hxx"
#include <cppuhelper/supportsservice.hxx>

using namespace com::sun::star;

namespace avmedia { namespace ogl {

OGLWindow::OGLWindow( OGLPlayer& rPlayer )
    : m_rPlayer( rPlayer )
    , meZoomLevel( media::ZoomLevel_ORIGINAL )
{
    (void) m_rPlayer;
}

OGLWindow::~OGLWindow()
{
}

void SAL_CALL OGLWindow::update() throw (css::uno::RuntimeException, std::exception)
{
}

sal_Bool SAL_CALL OGLWindow::setZoomLevel( css::media::ZoomLevel eZoomLevel ) throw (css::uno::RuntimeException, std::exception)
{
    sal_Bool bRet = false;

    if( media::ZoomLevel_NOT_AVAILABLE != meZoomLevel &&
        media::ZoomLevel_NOT_AVAILABLE != eZoomLevel )
    {
        if( eZoomLevel != meZoomLevel )
        {
            meZoomLevel = eZoomLevel;
        }
        bRet = true;
    }
    // TODO: set zoom level, not just store this value
    return bRet;
}

css::media::ZoomLevel SAL_CALL OGLWindow::getZoomLevel() throw (css::uno::RuntimeException, std::exception)
{
    return meZoomLevel;
}

void SAL_CALL OGLWindow::setPointerType( sal_Int32 ) throw (css::uno::RuntimeException, std::exception)
{
}

OUString SAL_CALL OGLWindow::getImplementationName() throw (css::uno::RuntimeException, std::exception)
{
    return OUString("com.sun.star.comp.avmedia.Window_OpenGL");
}

sal_Bool SAL_CALL OGLWindow::supportsService( const OUString& rServiceName ) throw (css::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL OGLWindow::getSupportedServiceNames() throw (css::uno::RuntimeException, std::exception)
{
    uno::Sequence< OUString > aRet(1);
    aRet[0] = OUString("com.sun.star.media.Window_OpenGL");
    return aRet;
}

void SAL_CALL OGLWindow::dispose() throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL OGLWindow::addEventListener( const uno::Reference< lang::XEventListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL OGLWindow::removeEventListener( const uno::Reference< lang::XEventListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL OGLWindow::setPosSize( sal_Int32 /*X*/, sal_Int32 /*Y*/, sal_Int32 /*Width*/, sal_Int32 /*Height*/, sal_Int16 /* Flags */ )
    throw (uno::RuntimeException, std::exception)
{
    // TODO: store size
}

awt::Rectangle SAL_CALL OGLWindow::getPosSize()
    throw (uno::RuntimeException, std::exception)
{
    //  TODO: get size
    return awt::Rectangle();
}

void SAL_CALL OGLWindow::setVisible( sal_Bool )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL OGLWindow::setEnable( sal_Bool )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL OGLWindow::setFocus()
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL OGLWindow::addWindowListener( const uno::Reference< awt::XWindowListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL OGLWindow::removeWindowListener( const uno::Reference< awt::XWindowListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL OGLWindow::addFocusListener( const uno::Reference< awt::XFocusListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL OGLWindow::removeFocusListener( const uno::Reference< awt::XFocusListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL OGLWindow::addKeyListener( const uno::Reference< awt::XKeyListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL OGLWindow::removeKeyListener( const uno::Reference< awt::XKeyListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL OGLWindow::addMouseListener( const uno::Reference< awt::XMouseListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL OGLWindow::removeMouseListener( const uno::Reference< awt::XMouseListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL OGLWindow::addMouseMotionListener( const uno::Reference< awt::XMouseMotionListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL OGLWindow::removeMouseMotionListener( const uno::Reference< awt::XMouseMotionListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL OGLWindow::addPaintListener( const uno::Reference< awt::XPaintListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL OGLWindow::removePaintListener( const uno::Reference< awt::XPaintListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

} // namespace ogl
} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
