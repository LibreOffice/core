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


#include <com/sun/star/rendering/XSprite.hpp>
#include <com/sun/star/rendering/XAnimatedSprite.hpp>

#include <basegfx/tools/canvastools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <canvas/canvastools.hxx>

#include "implsprite.hxx"


using namespace ::com::sun::star;

namespace cppcanvas
{
    namespace internal
    {

        ImplSprite::ImplSprite( const uno::Reference< rendering::XSpriteCanvas >&       rParentCanvas,
                                const uno::Reference< rendering::XSprite >&             rSprite,
                                const ImplSpriteCanvas::TransformationArbiterSharedPtr& rTransformArbiter ) :
            mxGraphicDevice(),
            mxSprite( rSprite ),
            mxAnimatedSprite(),
            mpTransformArbiter( rTransformArbiter )
        {
            // Avoiding ternary operator in initializer list (Solaris
            // compiler bug, when function call and temporary is
            // involved)
            if( rParentCanvas.is() )
                mxGraphicDevice = rParentCanvas->getDevice();

            OSL_ENSURE( rParentCanvas.is() , "ImplSprite::ImplSprite(): Invalid canvas");
            OSL_ENSURE( mxGraphicDevice.is(), "ImplSprite::ImplSprite(): Invalid graphic device");
            OSL_ENSURE( mxSprite.is(), "ImplSprite::ImplSprite(): Invalid sprite");
            OSL_ENSURE( mpTransformArbiter.get(), "ImplSprite::ImplSprite(): Invalid transformation arbiter");
        }

        ImplSprite::~ImplSprite()
        {
            // hide the sprite on the canvas. If we don't hide the
            // sprite, it will stay on the canvas forever, since the
            // canvas naturally keeps a list of visible sprites
            // (otherwise, it wouldn't be able to paint them
            // autonomously)
            if( mxSprite.is() )
                mxSprite->hide();
        }

        void ImplSprite::setAlpha( const double& rAlpha )
        {
            OSL_ENSURE( mxSprite.is(), "ImplSprite::setAlpha(): Invalid sprite");

            if( mxSprite.is() )
                mxSprite->setAlpha( rAlpha );
        }

        void ImplSprite::movePixel( const ::basegfx::B2DPoint& rNewPos )
        {
            OSL_ENSURE( mxSprite.is(), "ImplSprite::movePixel(): Invalid sprite");

            if( mxSprite.is() )
            {
                rendering::ViewState    aViewState;
                rendering::RenderState  aRenderState;

                ::canvas::tools::initViewState( aViewState );
                ::canvas::tools::initRenderState( aRenderState );

                mxSprite->move( ::basegfx::unotools::point2DFromB2DPoint( rNewPos ),
                                aViewState,
                                aRenderState );
            }
        }

        void ImplSprite::move( const ::basegfx::B2DPoint& rNewPos )
        {
            OSL_ENSURE( mxSprite.is(), "ImplSprite::move(): Invalid sprite");

            if( mxSprite.is() )
            {
                rendering::ViewState    aViewState;
                rendering::RenderState  aRenderState;

                ::canvas::tools::initViewState( aViewState );
                ::canvas::tools::initRenderState( aRenderState );

                ::canvas::tools::setViewStateTransform( aViewState,
                                                        mpTransformArbiter->getTransformation() );

                mxSprite->move( ::basegfx::unotools::point2DFromB2DPoint( rNewPos ),
                                aViewState,
                                aRenderState );
            }
        }

        void ImplSprite::transform( const ::basegfx::B2DHomMatrix& rMatrix )
        {
            OSL_ENSURE( mxSprite.is(), "ImplSprite::transform(): Invalid sprite");

            if( mxSprite.is() )
            {
                geometry::AffineMatrix2D aMatrix;

                mxSprite->transform( ::basegfx::unotools::affineMatrixFromHomMatrix( aMatrix,
                                                                                     rMatrix ) );
            }
        }

        void ImplSprite::setClipPixel( const ::basegfx::B2DPolyPolygon& rClipPoly )
        {
            OSL_ENSURE( mxGraphicDevice.is(), "ImplSprite::setClip(): Invalid canvas");
            OSL_ENSURE( mxSprite.is(), "ImplSprite::transform(): Invalid sprite");

            if( mxSprite.is() && mxGraphicDevice.is() )
                mxSprite->clip( ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon( mxGraphicDevice,
                                                                                         rClipPoly ) );
        }

        void ImplSprite::setClip( const ::basegfx::B2DPolyPolygon& rClipPoly )
        {
            OSL_ENSURE( mxGraphicDevice.is(), "ImplSprite::setClip(): Invalid canvas");
            OSL_ENSURE( mxSprite.is(), "ImplSprite::transform(): Invalid sprite");

            if( mxSprite.is() && mxGraphicDevice.is() )
            {
                ::basegfx::B2DPolyPolygon   aTransformedClipPoly( rClipPoly );

                // extract linear part of canvas view transformation (linear means:
                // without translational components)
                ::basegfx::B2DHomMatrix     aViewTransform( mpTransformArbiter->getTransformation() );
                aViewTransform.set( 0, 2, 0.0 );
                aViewTransform.set( 1, 2, 0.0 );

                // transform polygon from view to device coordinate space
                aTransformedClipPoly.transform( aViewTransform );

                mxSprite->clip( ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon( mxGraphicDevice,
                                                                                     aTransformedClipPoly ) );
            }
        }

        void ImplSprite::setClip()
        {
            OSL_ENSURE( mxGraphicDevice.is(), "ImplSprite::setClip(): Invalid canvas");
            OSL_ENSURE( mxSprite.is(), "ImplSprite::setClip(): Invalid sprite");

            if( mxSprite.is() && mxGraphicDevice.is() )
                mxSprite->clip( uno::Reference< rendering::XPolyPolygon2D >() );
        }

        void ImplSprite::show()
        {
            OSL_ENSURE( mxSprite.is(), "ImplSprite::show(): Invalid sprite");

            if( mxSprite.is() )
                mxSprite->show();
        }

        void ImplSprite::hide()
        {
            OSL_ENSURE( mxSprite.is(), "ImplSprite::hide(): Invalid sprite");

            if( mxSprite.is() )
                mxSprite->hide();
        }

        void ImplSprite::setPriority( double fPriority )
        {
            OSL_ENSURE( mxSprite.is(), "ImplSprite::setPriority(): Invalid sprite");

            if( mxSprite.is() )
                mxSprite->setPriority(fPriority);
        }

        uno::Reference< rendering::XSprite > ImplSprite::getUNOSprite() const
        {
            return mxSprite;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
