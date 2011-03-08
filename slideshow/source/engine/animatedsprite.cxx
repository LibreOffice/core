/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_slideshow.hxx"

// must be first
#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>

#include <animatedsprite.hxx>

#include <cppcanvas/canvas.hxx>
#include <canvas/canvastools.hxx>

#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>


using namespace ::com::sun::star;

namespace slideshow
{
    namespace internal
    {
        AnimatedSprite::AnimatedSprite( const ViewLayerSharedPtr&   rViewLayer,
                                        const ::basegfx::B2DSize&   rSpriteSizePixel,
                                        double                      nSpritePrio ) :
            mpViewLayer( rViewLayer ),
            mpSprite(),
            maEffectiveSpriteSizePixel( rSpriteSizePixel ),
            maContentPixelOffset(),
            mnSpritePrio(nSpritePrio),
            mnAlpha(0.0),
            maPosPixel(),
            maClip(),
            maTransform(),
            mbSpriteVisible( false )
        {
            ENSURE_OR_THROW( mpViewLayer, "AnimatedSprite::AnimatedSprite(): Invalid view layer" );

            // Add half a pixel tolerance to sprite size, since we later on compare
            // against it in resize(). And view transformations will almost never yield
            // the same data bits when transforming to device coordinates
            maEffectiveSpriteSizePixel += ::basegfx::B2DSize(0.5, 0.5);

            mpSprite = mpViewLayer->createSprite( maEffectiveSpriteSizePixel,
                                                  mnSpritePrio );

            ENSURE_OR_THROW( mpSprite, "AnimatedSprite::AnimatedSprite(): Could not create sprite" );
        }

        ::cppcanvas::CanvasSharedPtr AnimatedSprite::getContentCanvas() const
        {
            ENSURE_OR_THROW( mpViewLayer->getCanvas(), "AnimatedSprite::getContentCanvas(): No view layer canvas" );

            const ::cppcanvas::CanvasSharedPtr pContentCanvas( mpSprite->getContentCanvas() );
            pContentCanvas->clear();

            // extract linear part of canvas view transformation
            // (linear means: without translational components). The
            // only translation that is imposed at the view transform
            // is the local content pixel offset.
            //
            // We can apply that directly here, no need to call
            // aLinearTransform.translate(), since, as said above, the
            // last column of aLinearTransform is assumed [0 0 1]
            ::basegfx::B2DHomMatrix aLinearTransform( mpViewLayer->getTransformation() );
            aLinearTransform.set( 0, 2, maContentPixelOffset.getX() );
            aLinearTransform.set( 1, 2, maContentPixelOffset.getY() );

            // apply linear part of canvas view transformation to sprite canvas
            pContentCanvas->setTransformation( aLinearTransform );

            return pContentCanvas;
        }

        bool AnimatedSprite::resize( const ::basegfx::B2DSize& rSpriteSizePixel )
        {
            // Enlarge or reduce the sprite size, if necessary.  This
            // method employs a strategy similar to container, when
            // allocating memory: size is doubled or halved everytime
            // the limit is reached. This makes for amortized constant
            // time in runtime complexity. Note that we take exact
            // powers of two here, since several HW-accelerated canvas
            // implementations are limited to such sprite sizes
            // (otherwise, those implementations would internally
            // round up, too, wasting precious mem).
            ::basegfx::B2DSize  aNewSize( maEffectiveSpriteSizePixel );
            bool                bNeedResize( false );

            if( rSpriteSizePixel.getX() > maEffectiveSpriteSizePixel.getX() ||
                rSpriteSizePixel.getX() < 0.5*maEffectiveSpriteSizePixel.getX() )
            {
                // enlarge or shrink width
                aNewSize.setX( ::canvas::tools::nextPow2( ::basegfx::fround(rSpriteSizePixel.getX()) ) );
                bNeedResize = true;
            }

            if( rSpriteSizePixel.getY() > maEffectiveSpriteSizePixel.getY() ||
                rSpriteSizePixel.getY() < 0.5*maEffectiveSpriteSizePixel.getY() )
            {
                // enlarge or shrink height, by doubling it
                aNewSize.setY( ::canvas::tools::nextPow2( ::basegfx::fround(rSpriteSizePixel.getY()) ) );
                bNeedResize = true;
            }

            if( bNeedResize )
            {
                // as the old sprite might have already been altered
                // (and therefore been placed in the update list of
                // the spritecanvas for this frame), must hide it
                // here, to ensure it's not visible on screen any
                // longer.
                mpSprite->hide();

                maEffectiveSpriteSizePixel = aNewSize;
                mpSprite = mpViewLayer->createSprite( maEffectiveSpriteSizePixel,
                                                      mnSpritePrio );

                ENSURE_OR_THROW( mpSprite,
                                  "AnimatedSprite::resize(): Could not create new sprite" );

                // set attributes similar to previous sprite
                if( mpSprite && mbSpriteVisible )
                {
                    mpSprite->show();
                    mpSprite->setAlpha( mnAlpha );

                    if( maPosPixel )
                        mpSprite->movePixel( *maPosPixel );

                    if( maClip )
                        mpSprite->setClip( *maClip );
                }
            }

            return mpSprite;
        }

        void AnimatedSprite::setPixelOffset( const ::basegfx::B2DSize& rPixelOffset )
        {
            maContentPixelOffset = rPixelOffset;
        }

        ::basegfx::B2DSize AnimatedSprite::getPixelOffset() const
        {
            return maContentPixelOffset;
        }

        void AnimatedSprite::movePixel( const ::basegfx::B2DPoint& rNewPos )
        {
            maPosPixel.reset( rNewPos );
            mpSprite->movePixel( rNewPos );
        }

        void AnimatedSprite::setAlpha( double nAlpha )
        {
            mnAlpha = nAlpha;
            mpSprite->setAlpha( nAlpha );
        }

        void AnimatedSprite::clip( const ::basegfx::B2DPolyPolygon& rClip )
        {
            maClip.reset( rClip );
            mpSprite->setClipPixel( rClip );
        }

        void AnimatedSprite::clip()
        {
            maClip.reset();
            mpSprite->setClip();
        }

        void AnimatedSprite::transform( const ::basegfx::B2DHomMatrix& rTransform )
        {
            maTransform.reset( rTransform );
            mpSprite->transform( rTransform );
        }

        void AnimatedSprite::setPriority( double nPrio )
        {
            mpSprite->setPriority( nPrio );
        }

        void AnimatedSprite::hide()
        {
            mpSprite->hide();
            mbSpriteVisible = false;
        }

        void AnimatedSprite::show()
        {
            mbSpriteVisible = true;
            mpSprite->show();
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
