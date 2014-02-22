/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */



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

            
            
            
            
            //
            
            
            
            ::basegfx::B2DHomMatrix aLinearTransform( mpViewLayer->getTransformation() );
            aLinearTransform.set( 0, 2, maContentPixelOffset.getX() );
            aLinearTransform.set( 1, 2, maContentPixelOffset.getY() );

            
            pContentCanvas->setTransformation( aLinearTransform );

            return pContentCanvas;
        }

        bool AnimatedSprite::resize( const ::basegfx::B2DSize& rSpriteSizePixel )
        {
            
            
            
            
            
            
            
            
            
            ::basegfx::B2DSize  aNewSize( maEffectiveSpriteSizePixel );
            bool                bNeedResize( false );

            if( rSpriteSizePixel.getX() > maEffectiveSpriteSizePixel.getX() ||
                rSpriteSizePixel.getX() < 0.5*maEffectiveSpriteSizePixel.getX() )
            {
                
                aNewSize.setX( ::canvas::tools::nextPow2( ::basegfx::fround(rSpriteSizePixel.getX()) ) );
                bNeedResize = true;
            }

            if( rSpriteSizePixel.getY() > maEffectiveSpriteSizePixel.getY() ||
                rSpriteSizePixel.getY() < 0.5*maEffectiveSpriteSizePixel.getY() )
            {
                
                aNewSize.setY( ::canvas::tools::nextPow2( ::basegfx::fround(rSpriteSizePixel.getY()) ) );
                bNeedResize = true;
            }

            if( bNeedResize )
            {
                
                
                
                
                
                mpSprite->hide();

                maEffectiveSpriteSizePixel = aNewSize;
                mpSprite = mpViewLayer->createSprite( maEffectiveSpriteSizePixel,
                                                      mnSpritePrio );

                ENSURE_OR_THROW( mpSprite,
                                  "AnimatedSprite::resize(): Could not create new sprite" );

                
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

            return static_cast< bool >(mpSprite);
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
