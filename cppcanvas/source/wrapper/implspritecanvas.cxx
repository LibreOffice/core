/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: implspritecanvas.cxx,v $
 * $Revision: 1.10 $
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
