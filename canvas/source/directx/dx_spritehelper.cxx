/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dx_spritehelper.cxx,v $
 * $Revision: 1.3 $
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

#include <ctype.h> // don't ask. msdev breaks otherwise...
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

#include "dx_canvascustomsprite.hxx"
#include "dx_spritehelper.hxx"
#include "dx_impltools.hxx"
#include "dx_surfacegraphics.hxx"

#include <memory>

using namespace ::com::sun::star;

namespace dxcanvas
{
    SpriteHelper::SpriteHelper() :
        mpSpriteCanvas(),
        mpBitmap(),
        mbTextureDirty( true ),
        mbShowSpriteBounds( false )
    {
    }

    void SpriteHelper::init( const geometry::RealSize2D&        rSpriteSize,
                             const SpriteCanvasRef&             rSpriteCanvas,
                             const IDXRenderModuleSharedPtr&    rRenderModule,
                             const DXBitmapSharedPtr            rBitmap,
                             bool                               bShowSpriteBounds )
    {
        ENSURE_AND_THROW( rSpriteCanvas.get() &&
                          rRenderModule &&
                          rBitmap,
                          "SpriteHelper::init(): Invalid device, sprite canvas or surface" );

        mpSpriteCanvas     = rSpriteCanvas;
        mpBitmap           = rBitmap;
        mbTextureDirty     = true;
        mbShowSpriteBounds = bShowSpriteBounds;

        // also init base class
        CanvasCustomSpriteHelper::init( rSpriteSize,
                                        rSpriteCanvas.get() );
    }

    void SpriteHelper::disposing()
    {
        mpBitmap.reset();
        mpSpriteCanvas.clear();

        // forward to parent
        CanvasCustomSpriteHelper::disposing();
    }

    ::basegfx::B2DPolyPolygon SpriteHelper::polyPolygonFromXPolyPolygon2D( uno::Reference< rendering::XPolyPolygon2D >& xPoly ) const
    {
        return tools::polyPolygonFromXPolyPolygon2D( xPoly );
    }

    bool SpriteHelper::needRedraw() const
    {
        if( !mpBitmap ||
            !mpSpriteCanvas.get() )
        {
            return false; // we're disposed, no redraw necessary
        }

        if( !isActive() ||
            ::basegfx::fTools::equalZero( getAlpha() ) )
        {
            return false; // sprite is invisible
        }

        return true;
    }

    void SpriteHelper::redraw( bool& io_bSurfaceDirty ) const
    {
        if( !mpBitmap ||
            !mpSpriteCanvas.get() )
        {
            return; // we're disposed
        }

        const ::basegfx::B2DPoint& rPos( getPosPixel() );
        const double               fAlpha( getAlpha() );

        if( isActive() &&
            !::basegfx::fTools::equalZero( fAlpha ) )
        {

            // TODO(Q2): For the time being, Device does not take a target
            // surface, but always unconditionally renders to the
            // background buffer.

            // log output pos in device pixel
            VERBOSE_TRACE( "SpriteHelper::redraw(): output pos is (%f, %f)",
                           rPos.getX(),
                           rPos.getY() );

            const double                                       fAlpha( getAlpha() );
            const ::basegfx::B2DVector&                        rSize( getSizePixel() );
            const ::basegfx::B2DHomMatrix&                     rTransform( getTransformation() );
            const uno::Reference< rendering::XPolyPolygon2D >& xClip( getClip() );

            mbTextureDirty   = false;
            io_bSurfaceDirty = false; // state taken, and processed.

            ::basegfx::B2DPolyPolygon   aClipPath; // empty for no clip
            bool                        bIsClipRectangular( false ); // false, if no
                                                                    // clip, or clip
                                                                    // is complex

            // setup and apply clip (if any)
            // =================================

            if( xClip.is() )
            {
                aClipPath = tools::polyPolygonFromXPolyPolygon2D( xClip );

                const sal_Int32 nNumClipPolygons( aClipPath.count() );
                if( nNumClipPolygons )
                {
                    // TODO(P2): hold rectangle attribute directly
                    // at the XPolyPolygon2D

                    // check whether the clip is rectangular
                    if( nNumClipPolygons == 1 )
                        if( ::basegfx::tools::isRectangle( aClipPath.getB2DPolygon( 0 ) ) )
                            bIsClipRectangular = true;
                }
            }

            const ::basegfx::B2DRectangle aSourceRect( 0.0,
                                                       0.0,
                                                       rSize.getX(),
                                                       rSize.getY() );

            // draw simple rectangular area if no clip is set.
            if( !aClipPath.count() )
            {
                mpBitmap->draw(fAlpha,rPos,rTransform);
            }
            else if( bIsClipRectangular )
            {
                // apply a simple rect clip
                // ========================

                ::basegfx::B2DRectangle aClipBounds(
                    ::basegfx::tools::getRange( aClipPath ) );
                aClipBounds.intersect( aSourceRect );

                mpBitmap->draw(fAlpha,rPos,aClipBounds,rTransform);
            }
            else
            {
                // apply clip the hard way
                // =======================

                mpBitmap->draw(fAlpha,rPos,aClipPath,rTransform);
            }

            if( mbShowSpriteBounds )
            {
                if( aClipPath.count() )
                {
                    // TODO(F2): Re-enable debug output
                }
            }
        }
    }
}
