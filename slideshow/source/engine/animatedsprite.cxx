/*************************************************************************
 *
 *  $RCSfile: animatedsprite.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-21 16:54:58 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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


using namespace ::drafts::com::sun::star;
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
            mnAlpha(0.0),
            maPosPixel(),
            maPos(),
            maClip(),
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

            // extract linear part of canvas view transformation (linear means:
            // without translational components)
            ::basegfx::B2DHomMatrix aLinearTransform( mpViewLayer->getCanvas()->getTransformation() );
            aLinearTransform.set( 0, 2, 0.0 );
            aLinearTransform.set( 1, 2, 0.0 );

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
