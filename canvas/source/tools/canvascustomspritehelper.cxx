/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: canvascustomspritehelper.cxx,v $
 * $Revision: 1.5 $
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
#include <canvas/canvastools.hxx>

#include <rtl/math.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/numeric/ftools.hxx>

#include <canvas/base/canvascustomspritehelper.hxx>

using namespace ::com::sun::star;


namespace canvas
{
    bool CanvasCustomSpriteHelper::updateClipState( const Sprite::Reference& rSprite )
    {
        if( !mxClipPoly.is() )
        {
            // empty clip polygon -> everything is visible now
            maCurrClipBounds.reset();
            mbIsCurrClipRectangle = true;
        }
        else
        {
            const sal_Int32 nNumClipPolygons( mxClipPoly->getNumberOfPolygons() );

            // clip is not empty - determine actual update area
            ::basegfx::B2DPolyPolygon aClipPath(
                polyPolygonFromXPolyPolygon2D( mxClipPoly ) );

            // apply sprite transformation also to clip!
            aClipPath.transform( maTransform );

            // clip which is about to be set, expressed as a
            // b2drectangle
            const ::basegfx::B2DRectangle& rClipBounds(
                ::basegfx::tools::getRange( aClipPath ) );

            const ::basegfx::B2DRectangle aBounds( 0.0, 0.0,
                                                   maSize.getX(),
                                                   maSize.getY() );

            // rectangular area which is actually covered by the sprite.
            // coordinates are relative to the sprite origin.
            ::basegfx::B2DRectangle aSpriteRectPixel;
            ::canvas::tools::calcTransformedRectBounds( aSpriteRectPixel,
                                                        aBounds,
                                                        maTransform );

            // aClipBoundsA = new clip bound rect, intersected
            // with sprite area
            ::basegfx::B2DRectangle aClipBoundsA(rClipBounds);
            aClipBoundsA.intersect( aSpriteRectPixel );

            if( nNumClipPolygons != 1 )
            {
                // clip cannot be a single rectangle -> cannot
                // optimize update
                mbIsCurrClipRectangle = false;
                maCurrClipBounds = aClipBoundsA;
            }
            else
            {
                // new clip could be a single rectangle - check
                // that now:
                const bool bNewClipIsRect(
                    ::basegfx::tools::isRectangle( aClipPath.getB2DPolygon(0) ) );

                // both new and old clip are truly rectangles
                // - can now take the optimized path
                const bool bUseOptimizedUpdate( bNewClipIsRect &&
                                                mbIsCurrClipRectangle );

                const ::basegfx::B2DRectangle aOldBounds( maCurrClipBounds );

                // store new current clip type
                maCurrClipBounds = aClipBoundsA;
                mbIsCurrClipRectangle = bNewClipIsRect;

                if( mbActive &&
                    bUseOptimizedUpdate  )
                {
                    // aClipBoundsB = maCurrClipBounds, i.e. last
                    // clip, intersected with sprite area
                    typedef ::std::vector< ::basegfx::B2DRectangle > VectorOfRects;
                    VectorOfRects aClipDifferences;

                    // get all rectangles covered by exactly one
                    // of the polygons (aka XOR)
                    ::basegfx::computeSetDifference(aClipDifferences,
                                                    aClipBoundsA,
                                                    aOldBounds);

                    // aClipDifferences now contains the final
                    // update areas, coordinates are still relative
                    // to the sprite origin. before submitting
                    // this area to 'updateSprite()' we need to
                    // translate this area to the final position,
                    // coordinates need to be relative to the
                    // spritecanvas.
                    VectorOfRects::const_iterator       aCurr( aClipDifferences.begin() );
                    const VectorOfRects::const_iterator aEnd( aClipDifferences.end() );
                    while( aCurr != aEnd )
                    {
                        mpSpriteCanvas->updateSprite(
                            rSprite,
                            maPosition,
                            ::basegfx::B2DRectangle(
                                maPosition + aCurr->getMinimum(),
                                maPosition + aCurr->getMaximum() ) );
                        ++aCurr;
                    }

                    // update calls all done
                    return true;
                }
            }
        }

        // caller needs to perform update calls
        return false;
    }

    CanvasCustomSpriteHelper::CanvasCustomSpriteHelper() :
        mpSpriteCanvas(),
        maCurrClipBounds(),
        maPosition(),
        maSize(),
        maTransform(),
        mxClipPoly(),
        mfPriority(0.0),
        mfAlpha(0.0),
        mbActive(false),
        mbIsCurrClipRectangle(true),
        mbIsContentFullyOpaque( false ),
        mbAlphaDirty( true ),
        mbPositionDirty( true ),
        mbTransformDirty( true ),
        mbClipDirty( true ),
        mbPrioDirty( true ),
        mbVisibilityDirty( true )
    {
    }

    void CanvasCustomSpriteHelper::init( const geometry::RealSize2D&        rSpriteSize,
                                         const SpriteSurface::Reference&    rOwningSpriteCanvas )
    {
        ENSURE_AND_THROW( rOwningSpriteCanvas.get(),
                          "CanvasCustomSpriteHelper::init(): Invalid owning sprite canvas" );

        mpSpriteCanvas = rOwningSpriteCanvas;
        maSize.setX( ::std::max( 1.0,
                                 ceil( rSpriteSize.Width ) ) ); // round up to nearest int,
                                                                 // enforce sprite to have at
                                                                 // least (1,1) pixel size
        maSize.setY( ::std::max( 1.0,
                                 ceil( rSpriteSize.Height ) ) );
    }

    void CanvasCustomSpriteHelper::disposing()
    {
        mpSpriteCanvas.clear();
    }

    void CanvasCustomSpriteHelper::clearingContent( const Sprite::Reference& /*rSprite*/ )
    {
        // about to clear content to fully transparent
        mbIsContentFullyOpaque = false;
    }

    void CanvasCustomSpriteHelper::checkDrawBitmap( const Sprite::Reference&                    rSprite,
                                                    const uno::Reference< rendering::XBitmap >& xBitmap,
                                                    const rendering::ViewState&                 viewState,
                                                    const rendering::RenderState&               renderState )
    {
        // check whether bitmap is non-alpha, and whether its
        // transformed size covers the whole sprite.
        if( !xBitmap->hasAlpha() )
        {
            const geometry::IntegerSize2D& rInputSize(
                xBitmap->getSize() );
            const ::basegfx::B2DSize& rOurSize(
                rSprite->getSizePixel() );

            ::basegfx::B2DHomMatrix aTransform;
            if( tools::isInside(
                    ::basegfx::B2DRectangle( 0.0,0.0,
                                             rOurSize.getX(),
                                             rOurSize.getY() ),
                    ::basegfx::B2DRectangle( 0.0,0.0,
                                             rInputSize.Width,
                                             rInputSize.Height ),
                    ::canvas::tools::mergeViewAndRenderTransform(aTransform,
                                                                 viewState,
                                                                 renderState) ) )
            {
                // bitmap is opaque and will fully cover the sprite,
                // set flag appropriately
                mbIsContentFullyOpaque = true;
            }
        }
    }

    void CanvasCustomSpriteHelper::setAlpha( const Sprite::Reference&   rSprite,
                                             double                     alpha )
    {
        if( !mpSpriteCanvas.get() )
            return; // we're disposed

        if( alpha != mfAlpha )
        {
            mfAlpha = alpha;

            if( mbActive )
            {
                mpSpriteCanvas->updateSprite( rSprite,
                                              maPosition,
                                              getUpdateArea() );
            }

            mbAlphaDirty = true;
        }
    }

    void CanvasCustomSpriteHelper::move( const Sprite::Reference&       rSprite,
                                         const geometry::RealPoint2D&   aNewPos,
                                         const rendering::ViewState&    viewState,
                                         const rendering::RenderState&  renderState )
    {
        if( !mpSpriteCanvas.get() )
            return; // we're disposed

        ::basegfx::B2DHomMatrix aTransform;
        ::canvas::tools::mergeViewAndRenderTransform(aTransform,
                                                     viewState,
                                                     renderState);

        // convert position to device pixel
        ::basegfx::B2DPoint aPoint(
            ::basegfx::unotools::b2DPointFromRealPoint2D(aNewPos) );
        aPoint *= aTransform;

        if( aPoint != maPosition )
        {
            const ::basegfx::B2DRectangle&  rBounds( getFullSpriteRect() );

            if( mbActive )
            {
                mpSpriteCanvas->moveSprite( rSprite,
                                            rBounds.getMinimum(),
                                            rBounds.getMinimum() - maPosition + aPoint,
                                            rBounds.getRange() );
            }

            maPosition = aPoint;
            mbPositionDirty = true;
        }
    }

    void CanvasCustomSpriteHelper::transform( const Sprite::Reference&          rSprite,
                                              const geometry::AffineMatrix2D&   aTransformation )
    {
        ::basegfx::B2DHomMatrix aMatrix;
        ::basegfx::unotools::homMatrixFromAffineMatrix(aMatrix,
                                                       aTransformation);

        if( maTransform != aMatrix )
        {
            // retrieve bounds before and after transformation change.
            const ::basegfx::B2DRectangle& rPrevBounds( getUpdateArea() );

            maTransform = aMatrix;

            if( !updateClipState( rSprite ) &&
                mbActive )
            {
                mpSpriteCanvas->updateSprite( rSprite,
                                              maPosition,
                                              rPrevBounds );
                mpSpriteCanvas->updateSprite( rSprite,
                                              maPosition,
                                              getUpdateArea() );
            }

            mbTransformDirty = true;
        }
    }

    void CanvasCustomSpriteHelper::clip( const Sprite::Reference&                           rSprite,
                                         const uno::Reference< rendering::XPolyPolygon2D >& xClip )
    {
        // NULL xClip explicitely allowed here (to clear clipping)

        // retrieve bounds before and after clip change.
        const ::basegfx::B2DRectangle& rPrevBounds( getUpdateArea() );

        mxClipPoly = xClip;

        if( !updateClipState( rSprite ) &&
            mbActive )
        {
            mpSpriteCanvas->updateSprite( rSprite,
                                          maPosition,
                                          rPrevBounds );
            mpSpriteCanvas->updateSprite( rSprite,
                                          maPosition,
                                          getUpdateArea() );
        }

        mbClipDirty = true;
    }

    void CanvasCustomSpriteHelper::setPriority( const Sprite::Reference&    rSprite,
                                                double                      nPriority )
    {
        if( !mpSpriteCanvas.get() )
            return; // we're disposed

        if( nPriority != mfPriority )
        {
            mfPriority = nPriority;

            if( mbActive )
            {
                mpSpriteCanvas->updateSprite( rSprite,
                                              maPosition,
                                              getUpdateArea() );
            }

            mbPrioDirty = true;
        }
    }

    void CanvasCustomSpriteHelper::show( const Sprite::Reference& rSprite )
    {
        if( !mpSpriteCanvas.get() )
            return; // we're disposed

        if( !mbActive )
        {
            mpSpriteCanvas->showSprite( rSprite );
            mbActive = true;

            // TODO(P1): if clip is the NULL clip (nothing visible),
            // also save us the update call.

            if( mfAlpha != 0.0 )
            {
                mpSpriteCanvas->updateSprite( rSprite,
                                              maPosition,
                                              getUpdateArea() );
            }

            mbVisibilityDirty = true;
        }
    }

    void CanvasCustomSpriteHelper::hide( const Sprite::Reference& rSprite )
    {
        if( !mpSpriteCanvas.get() )
            return; // we're disposed

        if( mbActive )
        {
            mpSpriteCanvas->hideSprite( rSprite );
            mbActive = false;

            // TODO(P1): if clip is the NULL clip (nothing visible),
            // also save us the update call.

            if( mfAlpha != 0.0 )
            {
                mpSpriteCanvas->updateSprite( rSprite,
                                              maPosition,
                                              getUpdateArea() );
            }

            mbVisibilityDirty = true;
        }
    }

    // Sprite interface
    bool CanvasCustomSpriteHelper::isAreaUpdateOpaque( const ::basegfx::B2DRange& rUpdateArea ) const
    {
        if( !mbIsCurrClipRectangle ||
            !mbIsContentFullyOpaque ||
            !::rtl::math::approxEqual(mfAlpha, 1.0) )
        {
            // sprite either transparent, or clip rect does not
            // represent exact bounds -> update might not be fully
            // opaque
            return false;
        }
        else
        {
            // make sure sprite rect fully covers update area -
            // although the update area originates from the sprite,
            // it's by no means guaranteed that it's limited to this
            // sprite's update area - after all, other sprites might
            // have been merged, or this sprite is moving.
            return getUpdateArea().isInside( rUpdateArea );
        }
    }

    ::basegfx::B2DPoint CanvasCustomSpriteHelper::getPosPixel() const
    {
        return maPosition;
    }

    ::basegfx::B2DVector CanvasCustomSpriteHelper::getSizePixel() const
    {
        return maSize;
    }

    ::basegfx::B2DRange CanvasCustomSpriteHelper::getUpdateArea( const ::basegfx::B2DRange& rBounds ) const
    {
        // Internal! Only call with locked object mutex!
        ::basegfx::B2DHomMatrix aTransform( maTransform );
        aTransform.translate( maPosition.getX(),
                              maPosition.getY() );

        // transform bounds at origin, as the sprite transformation is
        // formulated that way
        ::basegfx::B2DRectangle aTransformedBounds;
        return ::canvas::tools::calcTransformedRectBounds( aTransformedBounds,
                                                           rBounds,
                                                           aTransform );
    }

    ::basegfx::B2DRange CanvasCustomSpriteHelper::getUpdateArea() const
    {
        // Internal! Only call with locked object mutex!

        // return effective sprite rect, i.e. take active clip into
        // account
        if( maCurrClipBounds.isEmpty() )
            return getUpdateArea( ::basegfx::B2DRectangle( 0.0, 0.0,
                                                           maSize.getX(),
                                                           maSize.getY() ) );
        else
            return ::basegfx::B2DRectangle(
                maPosition + maCurrClipBounds.getMinimum(),
                maPosition + maCurrClipBounds.getMaximum() );
    }

    double CanvasCustomSpriteHelper::getPriority() const
    {
        return mfPriority;
    }

    ::basegfx::B2DRange CanvasCustomSpriteHelper::getFullSpriteRect() const
    {
        // Internal! Only call with locked object mutex!
        return getUpdateArea( ::basegfx::B2DRectangle( 0.0, 0.0,
                                                       maSize.getX(),
                                                       maSize.getY() ) );
    }
}
