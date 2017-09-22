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

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygoncutandtouch.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygontriangulator.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <rtl/math.hxx>
#include <tools/diagnose_ex.h>

#include <canvas/canvastools.hxx>

#include "dx_canvascustomsprite.hxx"
#include "dx_impltools.hxx"
#include "dx_spritehelper.hxx"

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

    void SpriteHelper::init( const geometry::RealSize2D&     rSpriteSize,
                             const SpriteCanvasRef&          rSpriteCanvas,
                             const IDXRenderModuleSharedPtr& rRenderModule,
                             const DXSurfaceBitmapSharedPtr& rBitmap,
                             bool                            bShowSpriteBounds )
    {
        ENSURE_OR_THROW( rSpriteCanvas.get() &&
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
            SAL_INFO("canvas.directx", "SpriteHelper::redraw(): output pos is (" <<
                     rPos.getX() << "," << rPos.getY() << ")" );

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
                        if( ::basegfx::utils::isRectangle( aClipPath.getB2DPolygon( 0 ) ) )
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
                    ::basegfx::utils::getRange( aClipPath ) );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
