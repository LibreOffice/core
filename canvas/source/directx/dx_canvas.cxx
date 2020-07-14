// /* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <sal/config.h>

#include <memory>

#include <sal/log.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <com/sun/star/awt/XSystemDependentWindowPeer.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <osl/mutex.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/sysdata.hxx>
#include <vcl/opengl/OpenGLWrapper.hxx>
#include <vcl/skia/SkiaHelper.hxx>

#include <canvas/canvastools.hxx>

#include "dx_canvas.hxx"
#include "dx_graphicsprovider.hxx"
#include "dx_winstuff.hxx"

using namespace ::com::sun::star;

namespace dxcanvas
{
    namespace {

    /// Actual canonical implementation of the GraphicsProvider interface
    class GraphicsProviderImpl : public GraphicsProvider
    {
        GraphicsSharedPtr mpGraphics;
    public:
        explicit GraphicsProviderImpl( Gdiplus::Graphics* pGraphics ) : mpGraphics( pGraphics ) {}
        virtual GraphicsSharedPtr getGraphics() override { return mpGraphics; }
    };

    }

    Canvas::Canvas( const uno::Sequence< uno::Any >&                aArguments,
                    const uno::Reference< uno::XComponentContext >& rxContext ) :
        maArguments(aArguments),
        mxComponentContext( rxContext )
    {
    }

    void Canvas::initialize()
    {
        // #i64742# Only perform initialization when not in probe mode
        if( maArguments.getLength() == 0 )
            return;

        // tdf#93870 - force VCL canvas in OpenGL mode for now.
        assert( !OpenGLWrapper::isVCLOpenGLEnabled() );
        assert( !SkiaHelper::isVCLSkiaEnabled() );

        SAL_INFO("canvas.directx", "Canvas::initialize called" );

        // At index 1, we expect a HWND handle here, containing a
        // pointer to a valid window, on which to output
        // At index 2, we expect the current window bound rect
        ENSURE_ARG_OR_THROW( maArguments.getLength() >= 5 &&
                             maArguments[4].getValueTypeClass() == uno::TypeClass_SEQUENCE,
                             "Canvas::initialize: wrong number of arguments, or wrong types" );

        uno::Sequence<sal_Int8> aSeq;
        maArguments[4] >>= aSeq;

        const SystemGraphicsData* pSysData=reinterpret_cast<const SystemGraphicsData*>(aSeq.getConstArray());
        if( !pSysData || !pSysData->hDC )
            throw lang::NoSupportException("Passed SystemGraphicsData or HDC invalid!");

        sal_Int64 nPtr = 0;
        maArguments[0] >>= nPtr;
        OutputDevice* pOutDev = reinterpret_cast<OutputDevice*>(nPtr);
        ENSURE_ARG_OR_THROW( pOutDev != nullptr,"Canvas::initialize: invalid OutDev pointer" );

        // setup helper
        maDeviceHelper.init( pSysData->hDC, pOutDev, *this );
        maCanvasHelper.setDevice( *this );
        maCanvasHelper.setTarget(
            std::make_shared<GraphicsProviderImpl>(
                    Gdiplus::Graphics::FromHDC(pSysData->hDC)));

        maArguments.realloc(0);
    }

    void Canvas::disposeThis()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        mxComponentContext.clear();

        // forward to parent
        CanvasBaseT::disposeThis();
    }

    OUString SAL_CALL Canvas::getServiceName(  )
    {
        return "com.sun.star.rendering.Canvas.GDI+";
    }

    // XServiceInfo
    css::uno::Sequence<OUString> Canvas::getSupportedServiceNames(  )
    {
        return { "com.sun.star.rendering.Canvas.GDI+" };
    }
    OUString Canvas::getImplementationName(  )
    {
        return "com.sun.star.comp.rendering.Canvas.GDI+";
    }
    sal_Bool Canvas::supportsService( const OUString& sServiceName )
    {
        return cppu::supportsService(this, sServiceName);
    }

    BitmapCanvas::BitmapCanvas( const uno::Sequence< uno::Any >&                aArguments,
                                const uno::Reference< uno::XComponentContext >& rxContext ) :
        maArguments(aArguments),
        mxComponentContext( rxContext ),
        mpTarget()
    {
    }

    void BitmapCanvas::initialize()
    {
        // #i64742# Only perform initialization when not in probe mode
        if( maArguments.getLength() == 0 )
            return;

        SAL_INFO("canvas.directx", "BitmapCanvas::initialize called" );

        // At index 1, we expect a HWND handle here, containing a
        // pointer to a valid window, on which to output
        // At index 2, we expect the current window bound rect
        ENSURE_ARG_OR_THROW( maArguments.getLength() >= 5 &&
                             maArguments[4].getValueTypeClass() == uno::TypeClass_SEQUENCE,
                             "Canvas::initialize: wrong number of arguments, or wrong types" );

        uno::Sequence<sal_Int8> aSeq;
        maArguments[4] >>= aSeq;

        const SystemGraphicsData* pSysData=reinterpret_cast<const SystemGraphicsData*>(aSeq.getConstArray());
        if( !pSysData || !pSysData->hDC )
            throw lang::NoSupportException( "Passed SystemGraphicsData or HDC invalid!");

        sal_Int64 nPtr = 0;
        maArguments[0] >>= nPtr;
        OutputDevice* pOutDev = reinterpret_cast<OutputDevice*>(nPtr);
        ENSURE_ARG_OR_THROW( pOutDev != nullptr,"Canvas::initialize: invalid OutDev pointer" );

        // setup helper
        maDeviceHelper.init( pSysData->hDC, pOutDev, *this );
        maCanvasHelper.setDevice( *this );

        // check whether we can actually provide a BitmapCanvas
        // here. for this, check whether the HDC has a bitmap
        // selected.
        HBITMAP hBmp;
        hBmp=static_cast<HBITMAP>(GetCurrentObject(pSysData->hDC, OBJ_BITMAP));
        if( !hBmp || GetObjectType(pSysData->hDC) != OBJ_MEMDC )
        {
            throw lang::NoSupportException( "Passed HDC is no mem DC/has no bitmap selected!");
        }

        mpTarget = std::make_shared<DXBitmap>(
                            BitmapSharedPtr(
                                Gdiplus::Bitmap::FromHBITMAP(
                                    hBmp, nullptr) ),
                            false );

        maCanvasHelper.setTarget( mpTarget );

        maArguments.realloc(0);
    }

    void BitmapCanvas::disposeThis()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        mpTarget.reset();
        mxComponentContext.clear();

        // forward to parent
        BitmapCanvasBaseT::disposeThis();
    }

    OUString SAL_CALL BitmapCanvas::getServiceName(  )
    {
        return "com.sun.star.rendering.BitmapCanvas.GDI+";
    }

    // XServiceInfo
    css::uno::Sequence<OUString> BitmapCanvas::getSupportedServiceNames(  )
    {
        return { "com.sun.star.rendering.BitmapCanvas.GDI+" };
    }
    OUString BitmapCanvas::getImplementationName(  )
    {
        return "com.sun.star.comp.rendering.BitmapCanvas.GDI+";
    }
    sal_Bool BitmapCanvas::supportsService( const OUString& sServiceName )
    {
        return cppu::supportsService(this, sServiceName);
    }

    IBitmapSharedPtr BitmapCanvas::getBitmap() const
    {
        return mpTarget;
    }

    extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
    canvas_gdiplus_Canvas_get_implementation(
        css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& args)
    {
        rtl::Reference<Canvas> xCanvas(new Canvas(args, context));
        xCanvas->initialize();
        xCanvas->acquire();
        return static_cast<cppu::OWeakObject*>(xCanvas.get());
    }

    extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
    canvas_gdiplus_BitmapCanvas_get_implementation(
        css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& args)
    {
        rtl::Reference<BitmapCanvas> xCanvas(new BitmapCanvas(args, context));
        xCanvas->initialize();
        xCanvas->acquire();
        return static_cast<cppu::OWeakObject*>(xCanvas.get());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
