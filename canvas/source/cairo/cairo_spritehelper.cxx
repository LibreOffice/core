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

#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>
#include <tools/diagnose_ex.h>

#include <rtl/logfile.hxx>
#include <rtl/math.hxx>

#include <canvas/canvastools.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygontriangulator.hxx>
#include <basegfx/polygon/b2dpolygoncutandtouch.hxx>

#include "cairo_canvascustomsprite.hxx"
#include "cairo_spritehelper.hxx"

#include <memory>


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
        ENSURE_OR_THROW( rSpriteCanvas.get(),
                          "SpriteHelper::init(): Invalid device, sprite canvas or surface" );

        mpSpriteCanvas     = rSpriteCanvas;
        mbTextureDirty     = true;

        // also init base class
        CanvasCustomSpriteHelper::init( rSpriteSize,
                                        rSpriteCanvas.get() );
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

        if( isActive() && !::basegfx::fTools::equalZero( fAlpha ) )
        {
            OSL_TRACE ("CanvasCustomSprite::redraw called");
            if( pCairo )
            {
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

                    doPolyPolygonImplementation( aClipPoly, Clip, pCairo.get(),
                                                 NULL, SurfaceProviderRef(mpSpriteCanvas.get()),
                                                 rClip->getFillRule() );
                }

                OSL_TRACE ("aSize %f x %f position: %f,%f", aSize.getX(), aSize.getY(), fX, fY );
                cairo_rectangle( pCairo.get(), 0, 0, floor( aSize.getX() ), floor( aSize.getY() ) );
                cairo_clip( pCairo.get() );
                cairo_set_matrix( pCairo.get(), &aOrigMatrix );

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
            }
        }

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
