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

#include <cppuhelper/compbase2.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/media/XPlayerWindow.hpp>
#include <com/sun/star/media/ZoomLevel.hpp>

#include <libgltf.h>

#include <vcl/opengl/OpenGLContext.hxx>
#include <vcl/syschild.hxx>

namespace avmedia { namespace ogl {

class OGLWindow : public ::cppu::WeakImplHelper2 < com::sun::star::media::XPlayerWindow,
                                                   com::sun::star::lang::XServiceInfo >
{
public:
    OGLWindow( libgltf::glTFHandle& rHandle, const rtl::Reference<OpenGLContext> & rContext, vcl::Window& rEventHandlerParent );
    virtual ~OGLWindow();

    virtual void SAL_CALL update() throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL setZoomLevel( com::sun::star::media::ZoomLevel ZoomLevel ) throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual com::sun::star::media::ZoomLevel SAL_CALL getZoomLevel() throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setPointerType( sal_Int32 SystemPointerType ) throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual OUString SAL_CALL getImplementationName() throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName ) throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL dispose() throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addEventListener( const com::sun::star::uno::Reference< com::sun::star::lang::XEventListener >& xListener ) throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeEventListener( const com::sun::star::uno::Reference< com::sun::star::lang::XEventListener >& aListener ) throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL setPosSize( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight, sal_Int16 nFlags ) throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual com::sun::star::awt::Rectangle SAL_CALL getPosSize() throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setVisible( sal_Bool Visible ) throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setEnable( sal_Bool Enable ) throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setFocus() throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addWindowListener( const com::sun::star::uno::Reference< com::sun::star::awt::XWindowListener >& xListener ) throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeWindowListener( const com::sun::star::uno::Reference< com::sun::star::awt::XWindowListener >& xListener ) throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addFocusListener( const com::sun::star::uno::Reference< com::sun::star::awt::XFocusListener >& xListener ) throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeFocusListener( const com::sun::star::uno::Reference< com::sun::star::awt::XFocusListener >& xListener ) throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addKeyListener( const com::sun::star::uno::Reference< com::sun::star::awt::XKeyListener >& xListener ) throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeKeyListener( const com::sun::star::uno::Reference< com::sun::star::awt::XKeyListener >& xListener ) throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addMouseListener( const com::sun::star::uno::Reference< com::sun::star::awt::XMouseListener >& xListener ) throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeMouseListener( const com::sun::star::uno::Reference< com::sun::star::awt::XMouseListener >& xListener ) throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addMouseMotionListener( const com::sun::star::uno::Reference< com::sun::star::awt::XMouseMotionListener >& xListener ) throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeMouseMotionListener( const com::sun::star::uno::Reference< com::sun::star::awt::XMouseMotionListener >& xListener ) throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addPaintListener( const com::sun::star::uno::Reference< com::sun::star::awt::XPaintListener >& xListener ) throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removePaintListener( const com::sun::star::uno::Reference< com::sun::star::awt::XPaintListener >& xListener ) throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

private:
    DECL_LINK( FocusGrabber, VclWindowEvent* );
    DECL_LINK( CameraHandler, VclWindowEvent* );

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
