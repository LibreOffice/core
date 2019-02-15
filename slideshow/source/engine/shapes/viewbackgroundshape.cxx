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


// must be first
#include <tools/diagnose_ex.h>
#include <sal/log.hxx>

#include "viewbackgroundshape.hxx"
#include <tools.hxx>

#include <rtl/math.hxx>

#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include <com/sun/star/rendering/XCanvas.hpp>

#include <canvas/canvastools.hxx>
#include <cppcanvas/vclfactory.hxx>
#include <cppcanvas/basegfxfactory.hxx>
#include <cppcanvas/renderer.hxx>
#include <cppcanvas/bitmap.hxx>

using namespace ::com::sun::star;


namespace slideshow
{
    namespace internal
    {

        bool ViewBackgroundShape::prefetch( const ::cppcanvas::CanvasSharedPtr& rDestinationCanvas,
                                            const GDIMetaFileSharedPtr&         rMtf ) const
        {
            SAL_INFO( "slideshow", "::presentation::internal::ViewBackgroundShape::prefetch()" );
            ENSURE_OR_RETURN_FALSE( rMtf,
                               "ViewBackgroundShape::prefetch(): no valid metafile!" );

            const ::basegfx::B2DHomMatrix& rCanvasTransform(
                mpViewLayer->getTransformation() );

            if( !mxBitmap.is() ||
                rMtf != mpLastMtf ||
                rCanvasTransform != maLastTransformation )
            {
                // buffered bitmap is invalid, re-create

                // determine transformed page bounds
                ::basegfx::B2DRectangle aTmpRect;
                ::canvas::tools::calcTransformedRectBounds( aTmpRect,
                                                            maBounds,
                                                            rCanvasTransform );

                // determine pixel size of bitmap (choose it one pixel
                // larger, as polygon rendering takes one pixel more
                // to the right and to the bottom)
                const ::basegfx::B2ISize aBmpSizePixel(
                    ::basegfx::fround( aTmpRect.getRange().getX() + 1),
                    ::basegfx::fround( aTmpRect.getRange().getY() + 1) );

                // create a bitmap of appropriate size
                ::cppcanvas::BitmapSharedPtr pBitmap(
                    ::cppcanvas::BaseGfxFactory::createBitmap(
                        rDestinationCanvas,
                        aBmpSizePixel ) );

                ENSURE_OR_THROW( pBitmap,
                                  "ViewBackgroundShape::prefetch(): Cannot create background bitmap" );

                ::cppcanvas::BitmapCanvasSharedPtr pBitmapCanvas( pBitmap->getBitmapCanvas() );

                ENSURE_OR_THROW( pBitmapCanvas,
                                  "ViewBackgroundShape::prefetch(): Cannot create background bitmap canvas" );

                // clear bitmap
                initSlideBackground( pBitmapCanvas,
                                     aBmpSizePixel );

                // apply linear part of destination canvas transformation (linear means in this context:
                // transformation without any translational components)
                ::basegfx::B2DHomMatrix aLinearTransform( rCanvasTransform );
                aLinearTransform.set( 0, 2, 0.0 );
                aLinearTransform.set( 1, 2, 0.0 );
                pBitmapCanvas->setTransformation( aLinearTransform );

                const basegfx::B2DHomMatrix aShapeTransform(basegfx::utils::createScaleTranslateB2DHomMatrix(
                    maBounds.getWidth(), maBounds.getHeight(),
                    maBounds.getMinX(), maBounds.getMinY()));

                ::cppcanvas::RendererSharedPtr pRenderer(
                    ::cppcanvas::VCLFactory::createRenderer(
                        pBitmapCanvas,
                        *rMtf,
                        ::cppcanvas::Renderer::Parameters() ) );

                ENSURE_OR_RETURN_FALSE( pRenderer,
                                   "ViewBackgroundShape::prefetch(): Could not create Renderer" );

                pRenderer->setTransformation( aShapeTransform );
                pRenderer->draw();

                mxBitmap = pBitmap->getUNOBitmap();
            }

            mpLastMtf            = rMtf;
            maLastTransformation = rCanvasTransform;

            return mxBitmap.is();
        }

        ViewBackgroundShape::ViewBackgroundShape( const ViewLayerSharedPtr&         rViewLayer,
                                                  const ::basegfx::B2DRectangle&    rShapeBounds ) :
            mpViewLayer( rViewLayer ),
            mxBitmap(),
            mpLastMtf(),
            maLastTransformation(),
            maBounds( rShapeBounds )
        {
            ENSURE_OR_THROW( mpViewLayer, "ViewBackgroundShape::ViewBackgroundShape(): Invalid View" );
            ENSURE_OR_THROW( mpViewLayer->getCanvas(), "ViewBackgroundShape::ViewBackgroundShape(): Invalid ViewLayer canvas" );
        }

        const ViewLayerSharedPtr& ViewBackgroundShape::getViewLayer() const
        {
            return mpViewLayer;
        }

        bool ViewBackgroundShape::render( const GDIMetaFileSharedPtr& rMtf ) const
        {
            SAL_INFO( "slideshow", "::presentation::internal::ViewBackgroundShape::draw()" );

            const ::cppcanvas::CanvasSharedPtr& rDestinationCanvas( mpViewLayer->getCanvas() );

            if( !prefetch( rDestinationCanvas, rMtf ) )
                return false;

            ENSURE_OR_RETURN_FALSE( mxBitmap.is(),
                               "ViewBackgroundShape::draw(): Invalid background bitmap" );

            ::basegfx::B2DHomMatrix aTransform( mpViewLayer->getTransformation() );

            // invert the linear part of the view transformation
            // (i.e. the view transformation without translational
            // components), to be able to leave the canvas
            // transformation intact (would otherwise destroy possible
            // clippings, as the clip polygon is relative to the view
            // coordinate system).
            aTransform.set(0,2, 0.0 );
            aTransform.set(1,2, 0.0 );
            aTransform.invert();

            rendering::RenderState aRenderState;
            ::canvas::tools::initRenderState( aRenderState );

            ::canvas::tools::setRenderStateTransform( aRenderState, aTransform );

            try
            {
                rDestinationCanvas->getUNOCanvas()->drawBitmap( mxBitmap,
                                                                rDestinationCanvas->getViewState(),
                                                                aRenderState );
            }
            catch( uno::Exception& )
            {
                SAL_WARN( "slideshow", exceptionToString( cppu::getCaughtException() ) );
                return false;
            }

            return true;
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
