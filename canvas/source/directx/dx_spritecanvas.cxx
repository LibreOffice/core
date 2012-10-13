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


#include <ctype.h> // don't ask. msdev breaks otherwise...
#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>
#include <tools/diagnose_ex.h>

#include <canvas/canvastools.hxx>

#include <osl/mutex.hxx>

#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>

#include <toolkit/helper/vclunohelper.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <comphelper/servicedecl.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/numeric/ftools.hxx>

#include "dx_winstuff.hxx"
#include "dx_spritecanvas.hxx"

#if DIRECTX_VERSION < 0x0900
# define CANVAS_TECH "DX5"
#else
# define CANVAS_TECH "DX9"
#endif

#define SPRITECANVAS_SERVICE_NAME        "com.sun.star.rendering.SpriteCanvas."      CANVAS_TECH
#define SPRITECANVAS_IMPLEMENTATION_NAME "com.sun.star.comp.rendering.SpriteCanvas." CANVAS_TECH


using namespace ::com::sun::star;

namespace dxcanvas
{
    SpriteCanvas::SpriteCanvas( const uno::Sequence< uno::Any >&                aArguments,
                                const uno::Reference< uno::XComponentContext >& rxContext ) :
        maArguments(aArguments),
        mxComponentContext( rxContext )
    {
    }

    void SpriteCanvas::initialize()
    {
        // #i64742# Only call initialize when not in probe mode
        if( maArguments.getLength() == 0 )
            return;

        VERBOSE_TRACE( "SpriteCanvas::initialize called" );

        /* aArguments:
           0: ptr to creating instance (Window or VirtualDevice)
           1: SystemEnvData as a streamed Any (or empty for VirtualDevice)
           2: current bounds of creating instance
           3: bool, denoting always on top state for Window (always false for VirtualDevice)
           4: XWindow for creating Window (or empty for VirtualDevice)
           5: SystemGraphicsData as a streamed Any
         */
        ENSURE_ARG_OR_THROW( maArguments.getLength() >= 5 &&
                             maArguments[4].getValueTypeClass() == uno::TypeClass_INTERFACE,
                             "VCLSpriteCanvas::initialize: wrong number of arguments, or wrong types" );

        uno::Reference< awt::XWindow > xParentWindow;
        maArguments[4] >>= xParentWindow;
        Window* pParentWindow = VCLUnoHelper::GetWindow(xParentWindow);
        if( !pParentWindow )
            throw lang::NoSupportException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                     "Parent window not VCL window, or canvas out-of-process!")),
                NULL);

        awt::Rectangle aRect;
        maArguments[2] >>= aRect;

        sal_Bool bIsFullscreen( sal_False );
        maArguments[3] >>= bIsFullscreen;

        // setup helper
        maDeviceHelper.init( *pParentWindow,
                             *this,
                             aRect,
                             bIsFullscreen );
        maCanvasHelper.init( *this,
                             maRedrawManager,
                             maDeviceHelper.getRenderModule(),
                             maDeviceHelper.getSurfaceProxy(),
                             maDeviceHelper.getBackBuffer(),
                             ::basegfx::B2ISize() );
        maArguments.realloc(0);
    }

    void SpriteCanvas::disposeThis()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        mxComponentContext.clear();

        // forward to parent
        SpriteCanvasBaseT::disposeThis();
    }

    ::sal_Bool SAL_CALL SpriteCanvas::showBuffer( ::sal_Bool bUpdateAll ) throw (uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // avoid repaints on hidden window (hidden: not mapped to
        // screen). Return failure, since the screen really has _not_
        // been updated (caller should try again later)
        return !mbIsVisible ? false : SpriteCanvasBaseT::showBuffer( bUpdateAll );
    }

    ::sal_Bool SAL_CALL SpriteCanvas::switchBuffer( ::sal_Bool bUpdateAll ) throw (uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // avoid repaints on hidden window (hidden: not mapped to
        // screen). Return failure, since the screen really has _not_
        // been updated (caller should try again later)
        return !mbIsVisible ? false : SpriteCanvasBaseT::switchBuffer( bUpdateAll );
    }

    sal_Bool SAL_CALL SpriteCanvas::updateScreen( sal_Bool bUpdateAll ) throw (uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // avoid repaints on hidden window (hidden: not mapped to
        // screen). Return failure, since the screen really has _not_
        // been updated (caller should try again later)
        return !mbIsVisible ? false : maCanvasHelper.updateScreen(
            ::basegfx::unotools::b2IRectangleFromAwtRectangle(maBounds),
            bUpdateAll,
            mbSurfaceDirty );
    }

    ::rtl::OUString SAL_CALL SpriteCanvas::getServiceName(  ) throw (uno::RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SPRITECANVAS_SERVICE_NAME ) );
    }

    const IDXRenderModuleSharedPtr& SpriteCanvas::getRenderModule() const
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        return maDeviceHelper.getRenderModule();
    }

    const DXSurfaceBitmapSharedPtr& SpriteCanvas::getBackBuffer() const
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        return maDeviceHelper.getBackBuffer();
    }

    IBitmapSharedPtr SpriteCanvas::getBitmap() const
    {
        return maDeviceHelper.getBackBuffer();
    }

    static uno::Reference<uno::XInterface> initCanvas( SpriteCanvas* pCanvas )
    {
        uno::Reference<uno::XInterface> xRet(static_cast<cppu::OWeakObject*>(pCanvas));
        pCanvas->initialize();
        return xRet;
    }

    namespace sdecl = comphelper::service_decl;
    sdecl::class_<SpriteCanvas, sdecl::with_args<true> > serviceImpl(&initCanvas);
    const sdecl::ServiceDecl dxSpriteCanvasDecl(
        serviceImpl,
        SPRITECANVAS_IMPLEMENTATION_NAME,
        SPRITECANVAS_SERVICE_NAME );
}

// The C shared lib entry points
COMPHELPER_SERVICEDECL_EXPORTS1(directx9canvas, dxcanvas::dxSpriteCanvasDecl)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
