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

        void AnimatedSprite::resize( const ::basegfx::B2DSize& rSpriteSizePixel )
        {
            // Enlarge or reduce the sprite size, if necessary.  This
            // method employs a strategy similar to container, when
            // allocating memory: size is doubled or halved every time
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

            if( !bNeedResize )
                return;

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
            if (mbSpriteVisible)
            {
                mpSprite->show();
                mpSprite->setAlpha( mnAlpha );

                if( maPosPixel )
                    mpSprite->movePixel( *maPosPixel );

                if( maClip )
                    mpSprite->setClip( *maClip );
            }
        }

        void AnimatedSprite::setPixelOffset( const ::basegfx::B2DSize& rPixelOffset )
        {
            maContentPixelOffset = rPixelOffset;
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
