/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: animatedsprite.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:23:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// must be first
#include <canvas/debug.hxx>

#include <animatedsprite.hxx>

#ifndef _CPPCANVAS_CANVAS_HXX
#include <cppcanvas/canvas.hxx>
#endif
#ifndef _CANVAS_CANVASTOOLS_HXX
#include <canvas/canvastools.hxx>
#endif

#ifndef _BGFX_VECTOR_B2DVECTOR_HXX
#include <basegfx/vector/b2dvector.hxx>
#endif
#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif
#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif
#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif


using namespace ::com::sun::star;

namespace presentation
{
    namespace internal
    {
        AnimatedSprite::AnimatedSprite( const ViewLayerSharedPtr&   rViewLayer,
                                        const ::basegfx::B2DSize&   rSpriteSizePixel ) :
            mpViewLayer( rViewLayer ),
            mpSprite(),
            maEffectiveSpriteSizePixel( rSpriteSizePixel ),
            maContentPixelOffset(),
            mnAlpha(0.0),
            maPosPixel(),
            maPos(),
            maClip(),
            maTransform(),
            mbSpriteVisible( false )
        {
            ENSURE_AND_THROW( mpViewLayer.get(), "AnimatedSprite::AnimatedSprite(): Invalid view layer" );

            // Add half a pixel tolerance to sprite size, since we later on compare
            // against it in resize(). And view transformations will almost never yield
            // the same data bits when transforming to device coordinates
            maEffectiveSpriteSizePixel += ::basegfx::B2DSize(0.5, 0.5);

            mpSprite = mpViewLayer->createSprite( maEffectiveSpriteSizePixel );

            ENSURE_AND_THROW( mpSprite.get(), "AnimatedSprite::AnimatedSprite(): Could not create sprite" );
        }

        AnimatedSprite::~AnimatedSprite()
        {
        }

        ::cppcanvas::CanvasSharedPtr AnimatedSprite::getContentCanvas() const
        {
            ENSURE_AND_THROW( mpViewLayer->getCanvas().get(), "AnimatedSprite::getContentCanvas(): No view layer canvas" );

            const ::cppcanvas::CanvasSharedPtr pContentCanvas( mpSprite->getContentCanvas() );

            // extract linear part of canvas view transformation
            // (linear means: without translational components). The
            // only translation that is imposed at the view transform
            // is the local content pixel offset.
            //
            // We can apply that directly here, no need to call
            // aLinearTransform.translate(), since, as said above, the
            // last column of aLinearTransform is assumed [0 0 1]
            ::basegfx::B2DHomMatrix aLinearTransform( mpViewLayer->getCanvas()->getTransformation() );
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
                mpSprite = mpViewLayer->createSprite( maEffectiveSpriteSizePixel );

                ENSURE_AND_THROW( mpSprite.get(),
                                  "AnimatedSprite::resize(): Could not create new sprite" );

                // set attributes similar to previous sprite
                if( mpSprite.get() && mbSpriteVisible )
                {
                    mpSprite->show();
                    mpSprite->setAlpha( mnAlpha );

                    if( maPosPixel.isValid() )
                        mpSprite->movePixel( maPosPixel.getValue() );
                    else if( maPos.isValid() )
                        mpSprite->move( maPos.getValue() );

                    if( maClip.isValid() )
                        mpSprite->setClip( maClip.getValue() );
                }
            }

            return mpSprite.get() != NULL;
        }

        void AnimatedSprite::setPixelOffset( const ::basegfx::B2DSize& rPixelOffset )
        {
            maContentPixelOffset = rPixelOffset;
        }

        ::basegfx::B2DSize AnimatedSprite::getPixelOffset() const
        {
            return maContentPixelOffset;
        }

        void AnimatedSprite::move( const ::basegfx::B2DPoint& rNewPos )
        {
            maPos.setValue( rNewPos );
            mpSprite->move( rNewPos );
        }

        void AnimatedSprite::movePixel( const ::basegfx::B2DPoint& rNewPos )
        {
            maPosPixel.setValue( rNewPos );
            mpSprite->movePixel( rNewPos );
        }

        void AnimatedSprite::setAlpha( double nAlpha )
        {
            mnAlpha = nAlpha;
            mpSprite->setAlpha( nAlpha );
        }

        void AnimatedSprite::clip( const ::basegfx::B2DPolyPolygon& rClip )
        {
            maClip.setValue( rClip );
            mpSprite->setClip( rClip );
        }

        void AnimatedSprite::transform( const ::basegfx::B2DHomMatrix& rTransform )
        {
            maTransform.setValue( rTransform );
            mpSprite->transform( rTransform );
        }

        void AnimatedSprite::setPriority( double )
        {
            // TODO(F3): Missing sprite priority
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
