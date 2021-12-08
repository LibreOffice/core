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

#include <cairo.h>
#include <pixman.h>

#include "cairo_spritehelper.hxx"

using namespace ::cairo;
using namespace ::com::sun::star;

namespace cairocanvas
{
    SpriteHelper::SpriteHelper() :
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
        const ::basegfx::B2DHomMatrix aTransform( getTransformation() );

        if( !isActive() || ::basegfx::fTools::equalZero( fAlpha ) )
            return;

        SAL_INFO( "canvas.cairo", "CanvasCustomSprite::redraw called");
        if( !pCairo )
            return;

        basegfx::B2DVector aSize = getSizePixel();
        cairo_save( pCairo.get() );

        double fX, fY;

        fX = rPos.getX();
        fY = rPos.getY();

        if( !aTransform.isIdentity() )
        {
            cairo_matrix_t aMatrix, aInverseMatrix;
            cairo_matrix_init( &aMatrix,
                               aTransform.get( 0, 0 ), aTransform.get( 1, 0 ), aTransform.get( 0, 1 ),
                               aTransform.get( 1, 1 ), aTransform.get( 0, 2 ), aTransform.get( 1, 2 ) );

            aMatrix.x0 = basegfx::fround( aMatrix.x0 );
            aMatrix.y0 = basegfx::fround( aMatrix.y0 );

            cairo_matrix_init( &aInverseMatrix, aMatrix.xx, aMatrix.yx, aMatrix.xy, aMatrix.yy, aMatrix.x0, aMatrix.y0 );
            cairo_matrix_invert( &aInverseMatrix );
            cairo_matrix_transform_distance( &aInverseMatrix, &fX, &fY );

            cairo_set_matrix( pCairo.get(), &aMatrix );
        }

        fX = basegfx::fround( fX );
        fY = basegfx::fround( fY );

        cairo_matrix_t aOrigMatrix;
        cairo_get_matrix( pCairo.get(), &aOrigMatrix );
        cairo_translate( pCairo.get(), fX, fY );

        if( getClip().is() )
        {
            const uno::Reference<rendering::XPolyPolygon2D>& rClip( getClip() );

            ::basegfx::B2DPolyPolygon aClipPoly(
                ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(
                    rClip ));

            PolyFillMode eFillMode;
            if(rClip->getFillRule() == ::css::rendering::FillRule::FillRule_NON_ZERO)
                eFillMode = PolyFillMode::NON_ZERO_RULE_WINDING;
            else
                eFillMode = PolyFillMode::EVEN_ODD_RULE_ALTERNATE;

            doPolyPolygonImplementation( aClipPoly, Clip, pCairo.get(),
                                         nullptr, SurfaceProviderRef(mpSpriteCanvas),
                                         eFillMode );
        }

        SAL_INFO( "canvas.cairo","aSize " << aSize.getX() << " x " << aSize.getY() << " position: " << fX << "," << fY );
        cairo_rectangle( pCairo.get(), 0, 0, floor( aSize.getX() ), floor( aSize.getY() ) );
        cairo_clip( pCairo.get() );
        cairo_set_matrix( pCairo.get(), &aOrigMatrix );

        cairo_matrix_t aInverseMatrix = aOrigMatrix;
        bool matrixProblem = false;
        // tdf#125949: Cairo internally uses the pixman library, and _cairo_matrix_to_pixman_matrix()
        // checks all matrix components to fix PIXMAN_MAX_INT, which is about 32k. Which means that
        // if our transformation is large, such as an initial step of some zooming animations,
        // the conversion will fail. To make things worse, once something in cairo fails, it's treated
        // as a fatal error, the error status of that cairo_t gets set, and there's no way to reset it
        // besides recreating the whole cairo_t
        // (https://lists.cairographics.org/archives/cairo/2006-September/007892.html).
        // So copy&paste PIXMAN_MAX_INT here, and if our matrix could fail, bail out.
#define PIXMAN_MAX_INT ((pixman_fixed_1 >> 1) - pixman_fixed_e) /* need to ensure deltas also fit */
        if(cairo_matrix_invert(&aInverseMatrix) == CAIRO_STATUS_SUCCESS)
        {
            if(abs(aOrigMatrix.xx) > PIXMAN_MAX_INT || abs(aOrigMatrix.xx) > PIXMAN_MAX_INT
                || abs(aOrigMatrix.xy) > PIXMAN_MAX_INT || abs(aOrigMatrix.yx) > PIXMAN_MAX_INT
                || abs(aOrigMatrix.x0) > PIXMAN_MAX_INT || abs(aOrigMatrix.y0) > PIXMAN_MAX_INT
                || abs(aInverseMatrix.xx) > PIXMAN_MAX_INT || abs(aInverseMatrix.xx) > PIXMAN_MAX_INT
                || abs(aInverseMatrix.xy) > PIXMAN_MAX_INT || abs(aInverseMatrix.yx) > PIXMAN_MAX_INT
                || abs(aInverseMatrix.x0) > PIXMAN_MAX_INT || abs(aInverseMatrix.y0) > PIXMAN_MAX_INT)
                matrixProblem = true;
#undef PIXMAN_MAX_INT
        }
        else
            matrixProblem = true;
        if(matrixProblem)
        {
            SAL_WARN( "canvas.cairo", "matrix would overflow PIXMAN_MAX_INT, avoiding drawing" );
            cairo_restore( pCairo.get() );
            return;
        }

        if( isContentFullyOpaque() )
            cairo_set_operator( pCairo.get(), CAIRO_OPERATOR_SOURCE );
        cairo_set_source_surface( pCairo.get(),
                                  mpBufferSurface->getCairoSurface().get(),
                                  fX, fY );
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
