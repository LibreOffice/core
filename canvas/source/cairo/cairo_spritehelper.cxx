/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cairo_spritehelper.cxx,v $
 * $Revision: 1.8 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_canvas.hxx"

#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>

#include <rtl/logfile.hxx>
#include <rtl/math.hxx>

#include <canvas/canvastools.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygonrasterconverter.hxx>
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
        mbTextureDirty( true ),
    mpBufferSurface( NULL )
    {
    }

    void SpriteHelper::init( const geometry::RealSize2D&    rSpriteSize,
                             const SpriteCanvasRef&         rSpriteCanvas)
    {
        ENSURE_AND_THROW( rSpriteCanvas.get(),
                          "SpriteHelper::init(): Invalid device, sprite canvas or surface" );

        mpSpriteCanvas     = rSpriteCanvas;
        mbTextureDirty     = true;

        // also init base class
        CanvasCustomSpriteHelper::init( rSpriteSize,
                                        rSpriteCanvas.get() );
    }

    void SpriteHelper::setSurface( Surface* pBufferSurface )
    {
        if( mpBufferSurface )
            mpBufferSurface->Unref();

        mpBufferSurface = pBufferSurface;
        mpBufferSurface->Ref();
    }

    void SpriteHelper::disposing()
    {
        mpSpriteCanvas.clear();

        // forward to parent
        CanvasCustomSpriteHelper::disposing();

        if( mpBufferSurface ) {
            mpBufferSurface->Unref();
            mpBufferSurface = NULL;
        }
    }

    void SpriteHelper::redraw( Cairo*                     pCairo,
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

    if( isActive() && !::basegfx::fTools::equalZero( fAlpha ) ) {
        OSL_TRACE ("CanvasCustomSprite::redraw called");
        if( pCairo ) {
        basegfx::B2DVector aSize = getSizePixel();
        cairo_save( pCairo );

        double fX, fY;

        fX = rPos.getX();
        fY = rPos.getY();

         if( !aTransform.isIdentity() ) {
             cairo_matrix_t aMatrix, aInverseMatrix;
             cairo_matrix_init( &aMatrix,
                        aTransform.get( 0, 0 ), aTransform.get( 1, 0 ), aTransform.get( 0, 1 ),
                        aTransform.get( 1, 1 ), aTransform.get( 0, 2 ), aTransform.get( 1, 2 ) );

             aMatrix.x0 = basegfx::fround( aMatrix.x0 );
             aMatrix.y0 = basegfx::fround( aMatrix.y0 );

            cairo_matrix_init( &aInverseMatrix, aMatrix.xx, aMatrix.yx, aMatrix.xy, aMatrix.yy, aMatrix.x0, aMatrix.y0 );
            cairo_matrix_invert( &aInverseMatrix );
            cairo_matrix_transform_distance( &aInverseMatrix, &fX, &fY );

             cairo_set_matrix( pCairo, &aMatrix );
         }

         fX = basegfx::fround( fX );
         fY = basegfx::fround( fY );

        cairo_matrix_t aOrigMatrix;
        cairo_get_matrix( pCairo, &aOrigMatrix );
        cairo_translate( pCairo, fX, fY );

                if( getClip().is() )
                {
                    ::basegfx::B2DPolyPolygon aClipPoly(
                        ::canvas::tools::polyPolygonFromXPolyPolygon2D(
                            getClip() ));

            //cairo_reset_clip( pCairo );
            doPolyPolygonImplementation( aClipPoly, Clip, pCairo );
        }

        OSL_TRACE ("aSize %f x %f position: %f,%f", aSize.getX(), aSize.getY(), fX, fY );
        cairo_rectangle( pCairo, 0, 0, floor( aSize.getX() ), floor( aSize.getY() ) );
        cairo_clip( pCairo );
        cairo_set_matrix( pCairo, &aOrigMatrix );

        if( isContentFullyOpaque() )
             cairo_set_operator( pCairo, CAIRO_OPERATOR_SOURCE );
        cairo_set_source_surface( pCairo, mpBufferSurface->mpSurface, fX, fY );
                if( ::rtl::math::approxEqual( fAlpha, 1.0 ) )
            cairo_paint( pCairo );
        else
            cairo_paint_with_alpha( pCairo, fAlpha );

        cairo_restore( pCairo );
        }
    }

    #ifdef CAIRO_CANVAS_PERF_TRACE
    mxDevice->stopPerfTrace( &aTimer, "sprite redraw" );
        #endif
    }

    ::basegfx::B2DPolyPolygon SpriteHelper::polyPolygonFromXPolyPolygon2D( uno::Reference< rendering::XPolyPolygon2D >& xPoly ) const
    {
        return ::canvas::tools::polyPolygonFromXPolyPolygon2D( xPoly );
    }
}
