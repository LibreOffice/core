/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cppcanvas.hxx"
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <com/sun/star/rendering/InterpolationMode.hpp>

#include <implspritecanvas.hxx>
#include <implcustomsprite.hxx>


using namespace ::com::sun::star;

namespace cppcanvas
{
    namespace internal
    {
        ImplSpriteCanvas::TransformationArbiter::TransformationArbiter() :
            maTransformation()
        {
        }

        void ImplSpriteCanvas::TransformationArbiter::setTransformation( const ::basegfx::B2DHomMatrix& rViewTransform )
        {
            maTransformation = rViewTransform;
        }

        ::basegfx::B2DHomMatrix ImplSpriteCanvas::TransformationArbiter::getTransformation() const
        {
            return maTransformation;
        }


        ImplSpriteCanvas::ImplSpriteCanvas( const uno::Reference< rendering::XSpriteCanvas >& rCanvas ) :
            ImplCanvas( uno::Reference< rendering::XCanvas >(rCanvas,
                                                             uno::UNO_QUERY) ),
            ImplBitmapCanvas( uno::Reference< rendering::XBitmapCanvas >(rCanvas,
                                                                         uno::UNO_QUERY) ),
            mxSpriteCanvas( rCanvas ),
            mpTransformArbiter( new TransformationArbiter() )
        {
            OSL_ENSURE( mxSpriteCanvas.is(), "ImplSpriteCanvas::ImplSpriteCanvas(): Invalid canvas" );
        }

        ImplSpriteCanvas::ImplSpriteCanvas(const ImplSpriteCanvas& rOrig) :
            Canvas(),
            BitmapCanvas(),
            SpriteCanvas(),
            ImplCanvas( rOrig ),
            ImplBitmapCanvas( rOrig ),
            mxSpriteCanvas( rOrig.getUNOSpriteCanvas() ),
            mpTransformArbiter( new TransformationArbiter() )
        {
            OSL_ENSURE( mxSpriteCanvas.is(), "ImplSpriteCanvas::ImplSpriteCanvas( const ImplSpriteCanvas& ): Invalid canvas" );

            mpTransformArbiter->setTransformation( getTransformation() );
        }

        ImplSpriteCanvas::~ImplSpriteCanvas()
        {
        }

        void ImplSpriteCanvas::setTransformation( const ::basegfx::B2DHomMatrix& rMatrix )
        {
            mpTransformArbiter->setTransformation( rMatrix );

            ImplCanvas::setTransformation( rMatrix );
        }

        bool ImplSpriteCanvas::updateScreen( bool bUpdateAll ) const
        {
            OSL_ENSURE( mxSpriteCanvas.is(), "ImplSpriteCanvas::updateScreen(): Invalid canvas" );

            if( !mxSpriteCanvas.is() )
                return false;

            return mxSpriteCanvas->updateScreen( bUpdateAll );
        }

        CustomSpriteSharedPtr ImplSpriteCanvas::createCustomSprite( const ::basegfx::B2DSize& rSize ) const
        {
            OSL_ENSURE( mxSpriteCanvas.is(), "ImplSpriteCanvas::createCustomSprite(): Invalid canvas" );

            if( !mxSpriteCanvas.is() )
                return CustomSpriteSharedPtr();

            return CustomSpriteSharedPtr(
                new ImplCustomSprite( mxSpriteCanvas,
                                      mxSpriteCanvas->createCustomSprite( ::basegfx::unotools::size2DFromB2DSize(rSize) ),
                                      mpTransformArbiter ) );
        }

        SpriteSharedPtr ImplSpriteCanvas::createClonedSprite( const SpriteSharedPtr& rSprite ) const
        {
            OSL_ENSURE( mxSpriteCanvas.is(), "ImplSpriteCanvas::createCustomSprite(): Invalid canvas" );
            OSL_ENSURE( rSprite.get() != NULL && rSprite->getUNOSprite().is(),
                        "ImplSpriteCanvas::createCustomSprite(): Invalid sprite" );

            if( !mxSpriteCanvas.is() ||
                rSprite.get() == NULL ||
                !rSprite->getUNOSprite().is() )
            {
                return SpriteSharedPtr();
            }

            return SpriteSharedPtr(
                new ImplSprite( mxSpriteCanvas,
                                mxSpriteCanvas->createClonedSprite( rSprite->getUNOSprite() ),
                                mpTransformArbiter ) );
        }

        SpriteSharedPtr ImplSpriteCanvas::createSpriteFromBitmaps( const uno::Sequence< uno::Reference< rendering::XBitmap > >& rAnimationBitmaps,
                                                                   sal_Int8                                                     nInterpolationMode )
        {
            return SpriteSharedPtr( new internal::ImplSprite( mxSpriteCanvas,
                                                              mxSpriteCanvas->createSpriteFromBitmaps( rAnimationBitmaps,
                                                                                                       nInterpolationMode ),
                                                              mpTransformArbiter ) );
        }

        CanvasSharedPtr ImplSpriteCanvas::clone() const
        {
            return SpriteCanvasSharedPtr( new ImplSpriteCanvas( *this ) );
        }

        uno::Reference< rendering::XSpriteCanvas > ImplSpriteCanvas::getUNOSpriteCanvas() const
        {
            return mxSpriteCanvas;
        }

    }
}
