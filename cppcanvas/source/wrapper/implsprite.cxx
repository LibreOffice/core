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


#include <com/sun/star/rendering/XSprite.hpp>
#include <com/sun/star/rendering/XAnimatedSprite.hpp>

#include <basegfx/utils/canvastools.hxx>
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
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
