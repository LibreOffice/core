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

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <rtl/math.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/canvastools.hxx>

#include <cairo.h>

#include "cairo_spritehelper.hxx"

using namespace ::cairo;
using namespace ::com::sun::star;

namespace cairocanvas
{
    SpriteHelper::SpriteHelper() :
        mpSpriteCanvas(),
        mpBufferSurface(),
        mbTextureDirty(true)
    {}

    void SpriteHelper::init( const geometry::RealSize2D& rSpriteSize,
                             const SpriteCanvasRef&      rSpriteCanvas )
    {
        ENSURE_OR_THROW( rSpriteCanvas,
                          "SpriteHelper::init(): Invalid device, sprite canvas or surface" );

        mpSpriteCanvas     = rSpriteCanvas;
        mbTextureDirty     = true;

        // also init base class
        CanvasCustomSpriteHelper::init( rSpriteSize, rSpriteCanvas );
    }

    void SpriteHelper::setSurface( const SurfaceSharedPtr& pBufferSurface )
    {
        mpBufferSurface = pBufferSurface;
    }

    void SpriteHelper::disposing()
    {
        mpBufferSurface.reset();
        mpSpriteCanvas.clear();

        // forward to parent
        CanvasCustomSpriteHelper::disposing();
    }

    void SpriteHelper::redraw( const CairoSharedPtr&      pCairo,
                               const ::basegfx::B2DPoint& rPos,
                               bool&                      /*io_bSurfacesDirty*/,
                               bool                       /*bBufferedUpdate*/ ) const
    {
#ifdef CAIRO_CANVAS_PERF_TRACE
        struct timespec aTimer;
        mxDevice->startPerfTrace( &aTimer );
#endif

        const double fAlpha( getAlpha() );
        ::basegfx::B2DHomMatrix aTransform( getTransformation() );

        if( !isActive() || ::basegfx::fTools::equalZero( fAlpha ) )
            return;

        SAL_INFO( "canvas.cairo", "CanvasCustomSprite::redraw called");
        if( !pCairo )
            return;

        basegfx::B2DVector aSize = getSizePixel();
        ::Size  aOutputSize( vcl::unotools::sizeFromB2DSize( aSize ) );
        ::Point aOutPos( vcl::unotools::pointFromB2DPoint( rPos ) );
        cairo_save( pCairo.get() );

        ::basegfx::B2DHomMatrix aSizeTransform, aMoveTransform;
        aSizeTransform.scale( aOutputSize.Width(), aOutputSize.Height() );
        aMoveTransform.translate( aOutPos.X(), aOutPos.Y() );
        aTransform = aMoveTransform * aTransform * aSizeTransform;

        cairo_matrix_t aResultMatrix;
        cairo_matrix_init( &aResultMatrix,
                           aTransform.get( 0, 0 ), aTransform.get( 1, 0 ), aTransform.get( 0, 1 ),
                           aTransform.get( 1, 1 ), aTransform.get( 0, 2 ), aTransform.get( 1, 2 ) );

        if( getClip().is() )
        {
            const uno::Reference<rendering::XPolyPolygon2D>& rClip( getClip() );

            ::basegfx::B2DPolyPolygon aClipPoly(
                ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(
                    rClip ));

            doPolyPolygonImplementation( aClipPoly, Clip, pCairo.get(),
                                         nullptr, SurfaceProviderRef(mpSpriteCanvas),
                                         rClip->getFillRule() );
        }

        SAL_INFO( "canvas.cairo","aSize " << aSize.getX() << " x " << aSize.getY());
        //cairo_rectangle( pCairo.get(), aOutPos.X(), aOutPos.Y(), floor( aSize.getX() ), floor( aSize.getY() ) );
        //cairo_clip( pCairo.get() );
        cairo_set_matrix( pCairo.get(), &aResultMatrix );

        if( isContentFullyOpaque() )
            cairo_set_operator( pCairo.get(), CAIRO_OPERATOR_SOURCE );
        cairo_set_source_surface( pCairo.get(),
                                  mpBufferSurface->getCairoSurface().get(),
                                  aOutPos.X(), aOutPos.Y() );

        if( ::rtl::math::approxEqual( fAlpha, 1.0 ) )
            cairo_paint( pCairo.get() );
        else
            cairo_paint_with_alpha( pCairo.get(), fAlpha );

        cairo_restore( pCairo.get() );

#ifdef CAIRO_CANVAS_PERF_TRACE
        mxDevice->stopPerfTrace( &aTimer, "sprite redraw" );
#endif
    }

    ::basegfx::B2DPolyPolygon SpriteHelper::polyPolygonFromXPolyPolygon2D( uno::Reference< rendering::XPolyPolygon2D >& xPoly ) const
    {
        return ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(xPoly);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
