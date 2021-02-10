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

#include <sal/config.h>
#include <sal/log.hxx>

#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <osl/mutex.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/sysdata.hxx>
#include <vcl/skia/SkiaHelper.hxx>
#include <cppuhelper/supportsservice.hxx>

#include "cairo_canvas.hxx"

using namespace ::cairo;
using namespace ::com::sun::star;

namespace cairocanvas
{
    Canvas::Canvas( const uno::Sequence< uno::Any >&                aArguments,
                    const uno::Reference< uno::XComponentContext >& /*rxContext*/ ) :
        maArguments(aArguments)
    {
    }

    void Canvas::initialize()
    {
        // #i64742# Only perform initialization when not in probe mode
        if( !maArguments.hasElements() )
            return;

        assert( !SkiaHelper::isVCLSkiaEnabled() );

        /* maArguments:
           0: ptr to creating instance (Window or VirtualDevice)
           1: current bounds of creating instance
           2: bool, denoting always on top state for Window (always false for VirtualDevice)
           3: XWindow for creating Window (or empty for VirtualDevice)
           4: SystemGraphicsData as a streamed Any
         */
        SAL_INFO("canvas.cairo","Canvas created " <<  this);

        ENSURE_ARG_OR_THROW( maArguments.getLength() >= 5 &&
                             maArguments[0].getValueTypeClass() == uno::TypeClass_HYPER &&
                             maArguments[4].getValueTypeClass() == uno::TypeClass_SEQUENCE,
                             "Canvas::initialize: wrong number of arguments, or wrong types" );

        // We expect a single Any here, containing a pointer to a valid
        // VCL output device, on which to output (mostly needed for text)
        sal_Int64 nPtr = 0;
        maArguments[0] >>= nPtr;
        OutputDevice* pOutDev = reinterpret_cast<OutputDevice*>(nPtr);

        ENSURE_ARG_OR_THROW( pOutDev != nullptr,
                             "Canvas::initialize: invalid OutDev pointer" );

        awt::Rectangle aBounds;
        maArguments[1] >>= aBounds;

        uno::Sequence<sal_Int8> aSeq;
        maArguments[4] >>= aSeq;

        const SystemGraphicsData* pSysData=reinterpret_cast<const SystemGraphicsData*>(aSeq.getConstArray());
        if( !pSysData || !pSysData->nSize )
            throw lang::NoSupportException( "Passed SystemGraphicsData invalid!" );

        bool bHasCairo = pOutDev->SupportsCairo();
        ENSURE_ARG_OR_THROW(bHasCairo, "SpriteCanvas::SpriteCanvas: No Cairo capability");

        // setup helper
        maDeviceHelper.init( *this, *pOutDev );

        maCanvasHelper.init( basegfx::B2ISize(aBounds.Width, aBounds.Height), *this, this );

        // forward surface to render on to canvashelper
        maCanvasHelper.setSurface( maDeviceHelper.getSurface(), false );

        maArguments.realloc(0);
    }

    Canvas::~Canvas()
    {
        SAL_INFO("canvas.cairo", "CairoCanvas destroyed" );
    }

    void Canvas::disposeThis()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // forward to parent
        CanvasBaseT::disposeThis();
    }

    OUString SAL_CALL Canvas::getServiceName(  )
    {
        return "com.sun.star.rendering.Canvas.Cairo";
    }

    //  XServiceInfo
    sal_Bool Canvas::supportsService(const OUString& sServiceName)
    {
        return cppu::supportsService(this, sServiceName);

    }
    OUString Canvas::getImplementationName()
    {
        return "com.sun.star.comp.rendering.Canvas.Cairo";
    }
    css::uno::Sequence< OUString > Canvas::getSupportedServiceNames()
    {
        return { getServiceName() };
    }

    bool Canvas::repaint( const SurfaceSharedPtr&       pSurface,
                          const rendering::ViewState&   viewState,
                          const rendering::RenderState& renderState )
    {
        return maCanvasHelper.repaint( pSurface, viewState, renderState );
    }

    SurfaceSharedPtr Canvas::getSurface()
    {
        return maDeviceHelper.getSurface();
    }

    SurfaceSharedPtr Canvas::createSurface( const ::basegfx::B2ISize& rSize, int aContent )
    {
        return maDeviceHelper.createSurface( rSize, aContent );
    }

    SurfaceSharedPtr Canvas::createSurface( ::Bitmap& rBitmap )
    {
        SurfaceSharedPtr pSurface;

        BitmapSystemData aData;
        if( rBitmap.GetSystemData( aData ) ) {
            const Size& rSize = rBitmap.GetSizePixel();

            pSurface = maDeviceHelper.createSurface( aData, rSize );
        }

        return pSurface;
    }

    SurfaceSharedPtr Canvas::changeSurface()
    {
        // non-modifiable surface here
        return SurfaceSharedPtr();
    }

    OutputDevice* Canvas::getOutputDevice()
    {
        return maDeviceHelper.getOutputDevice();
    }
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_rendering_Canvas_Cairo_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& args)
{
    rtl::Reference<cairocanvas::Canvas> p = new cairocanvas::Canvas(args, context);
    p->acquire();
    try {
        p->initialize();
    } catch (css::uno::Exception&) {
        p->dispose();
        p->release();
        throw;
    }
    return static_cast<cppu::OWeakObject*>(p.get());
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
