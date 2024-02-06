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

#include <cppuhelper/supportsservice.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <utility>
#include <vcl/bitmapex.hxx>
#include <vcl/BitmapTools.hxx>

#include <cairo.h>

#include "cairo_canvasbitmap.hxx"

using namespace ::cairo;
using namespace ::com::sun::star;

namespace cairocanvas
{
    CanvasBitmap::CanvasBitmap( const ::basegfx::B2ISize&  rSize,
                                SurfaceProviderRef         rSurfaceProvider,
                                rendering::XGraphicDevice* pDevice,
                                bool                       bHasAlpha ) :
        mpSurfaceProvider(std::move( rSurfaceProvider )),
        maSize(rSize),
        mbHasAlpha(bHasAlpha)
    {
        ENSURE_OR_THROW( mpSurfaceProvider.is(),
                          "CanvasBitmap::CanvasBitmap(): Invalid surface or device" );

        SAL_INFO(
            "canvas.cairo",
            "bitmap size: " << rSize.getWidth() << "x" << rSize.getHeight());

        mpBufferSurface = mpSurfaceProvider->createSurface( rSize, bHasAlpha ? CAIRO_CONTENT_COLOR_ALPHA : CAIRO_CONTENT_COLOR );
        mpBufferCairo = mpBufferSurface->getCairo();

        maCanvasHelper.init( rSize, *mpSurfaceProvider, pDevice );
        maCanvasHelper.setSurface( mpBufferSurface, bHasAlpha );

        // clear bitmap to 100% transparent
        maCanvasHelper.clear();
    }

    void CanvasBitmap::disposeThis()
    {
        mpSurfaceProvider.clear();

        mpBufferCairo.reset();
        mpBufferSurface.reset();

        // forward to parent
        CanvasBitmap_Base::disposeThis();
    }

    SurfaceSharedPtr CanvasBitmap::getSurface()
    {
        return mpBufferSurface;
    }

    SurfaceSharedPtr CanvasBitmap::createSurface( const ::basegfx::B2ISize& rSize, int aContent )
    {
        return mpSurfaceProvider->createSurface(rSize,aContent);
    }

    SurfaceSharedPtr CanvasBitmap::createSurface( ::Bitmap& rBitmap )
    {
        return mpSurfaceProvider->createSurface(rBitmap);
    }

    SurfaceSharedPtr CanvasBitmap::changeSurface()
    {
        // non-modifiable surface here
        return SurfaceSharedPtr();
    }

    OutputDevice* CanvasBitmap::getOutputDevice()
    {
        return mpSurfaceProvider->getOutputDevice();
    }

    bool CanvasBitmap::repaint( const SurfaceSharedPtr&       pSurface,
                                const rendering::ViewState&   viewState,
                                const rendering::RenderState& renderState )
    {
        return maCanvasHelper.repaint( pSurface, viewState, renderState );
    }

    uno::Any SAL_CALL CanvasBitmap::getFastPropertyValue( sal_Int32 nHandle )
    {
        uno::Any aRV( sal_Int32(0) );
        // 0 ... get BitmapEx
        // 1 ... get Pixbuf with bitmap RGB content
        // 2 ... return nothing (empty Any)
        switch( nHandle )
        {
            case 0:
            {
                aRV <<= reinterpret_cast<sal_Int64>( nullptr );
                if ( !mbHasAlpha )
                    break;

                BitmapEx* pBitmapEx = vcl::bitmap::CreateFromCairoSurface(
                                          ::Size( maSize.getWidth(), maSize.getHeight() ),
                                          getSurface()->getCairoSurface().get());
                if (pBitmapEx)
                    aRV <<= reinterpret_cast<sal_Int64>( pBitmapEx );

                break;
            }
            case 1:
            {
                aRV = getOutputDevice()->GetNativeSurfaceHandle(mpBufferSurface, maSize);
                break;
            }
            case 2:
            {
                // Always return nothing - for the RGB surface support.
                // Alpha code paths go via the above case 0.
                aRV = uno::Any();
                break;
            }
        }

        return aRV;
    }

    OUString SAL_CALL CanvasBitmap::getImplementationName(  )
    {
        return u"CairoCanvas.CanvasBitmap"_ustr;
    }

    sal_Bool SAL_CALL CanvasBitmap::supportsService( const OUString& ServiceName )
    {
        return cppu::supportsService( this, ServiceName );
    }

    uno::Sequence< OUString > SAL_CALL CanvasBitmap::getSupportedServiceNames(  )
    {
        return { u"com.sun.star.rendering.CanvasBitmap"_ustr };
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
