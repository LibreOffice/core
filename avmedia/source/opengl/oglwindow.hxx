/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_AVMEDIA_SOURCE_OPENGL_OGLWINDOW_HXX
#define INCLUDED_AVMEDIA_SOURCE_OPENGL_OGLWINDOW_HXX

#include "oglplayer.hxx"

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/media/XPlayerWindow.hpp>
#include <com/sun/star/media/ZoomLevel.hpp>

#include <libgltf.h>

#include <vcl/opengl/OpenGLContext.hxx>
#include <vcl/syschild.hxx>

namespace avmedia { namespace ogl {

class OGLWindow : public ::cppu::WeakImplHelper< css::media::XPlayerWindow, css::lang::XServiceInfo >
{
public:
    OGLWindow( libgltf::glTFHandle& rHandle, const rtl::Reference<OpenGLContext> & rContext, vcl::Window& rEventHandlerParent );
    virtual ~OGLWindow() override;

    virtual void SAL_CALL update() override;
    virtual sal_Bool SAL_CALL setZoomLevel( css::media::ZoomLevel ZoomLevel ) override;
    virtual css::media::ZoomLevel SAL_CALL getZoomLevel() override;
    virtual void SAL_CALL setPointerType( sal_Int32 SystemPointerType ) override;

    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    virtual void SAL_CALL dispose() final override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

    virtual void SAL_CALL setPosSize( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight, sal_Int16 nFlags ) override;
    virtual css::awt::Rectangle SAL_CALL getPosSize() override;
    virtual void SAL_CALL setVisible( sal_Bool Visible ) override;
    virtual void SAL_CALL setEnable( sal_Bool Enable ) override;
    virtual void SAL_CALL setFocus() override;
    virtual void SAL_CALL addWindowListener( const css::uno::Reference< css::awt::XWindowListener >& xListener ) override;
    virtual void SAL_CALL removeWindowListener( const css::uno::Reference< css::awt::XWindowListener >& xListener ) override;
    virtual void SAL_CALL addFocusListener( const css::uno::Reference< css::awt::XFocusListener >& xListener ) override;
    virtual void SAL_CALL removeFocusListener( const css::uno::Reference< css::awt::XFocusListener >& xListener ) override;
    virtual void SAL_CALL addKeyListener( const css::uno::Reference< css::awt::XKeyListener >& xListener ) override;
    virtual void SAL_CALL removeKeyListener( const css::uno::Reference< css::awt::XKeyListener >& xListener ) override;
    virtual void SAL_CALL addMouseListener( const css::uno::Reference< css::awt::XMouseListener >& xListener ) override;
    virtual void SAL_CALL removeMouseListener( const css::uno::Reference< css::awt::XMouseListener >& xListener ) override;
    virtual void SAL_CALL addMouseMotionListener( const css::uno::Reference< css::awt::XMouseMotionListener >& xListener ) override;
    virtual void SAL_CALL removeMouseMotionListener( const css::uno::Reference< css::awt::XMouseMotionListener >& xListener ) override;
    virtual void SAL_CALL addPaintListener( const css::uno::Reference< css::awt::XPaintListener >& xListener ) override;
    virtual void SAL_CALL removePaintListener( const css::uno::Reference< css::awt::XPaintListener >& xListener ) override;

private:
    DECL_LINK( FocusGrabber, VclWindowEvent&, void );
    DECL_LINK( CameraHandler, VclWindowEvent&, void );

    libgltf::glTFHandle& m_rHandle;
    rtl::Reference<OpenGLContext> m_xContext;
    vcl::Window& m_rEventHandler;

    bool m_bVisible;
    Point m_aLastMousePos;
    bool m_bIsOrbitMode;
};

} // namespace ogl
} // namespace avmedia

#endif // INCLUDED_AVMEDIA_SOURCE_OPENGL_OGLWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
