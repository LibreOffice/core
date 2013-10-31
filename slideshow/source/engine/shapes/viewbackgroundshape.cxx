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
#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>

#include "viewbackgroundshape.hxx"
#include "tools.hxx"

#include <rtl/math.hxx>

#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>

#include <canvas/verbosetrace.hxx>
#include <canvas/canvastools.hxx>

using namespace ::com::sun::star;


namespace slideshow
{
    namespace internal
    {

        bool ViewBackgroundShape::prefetch( const css::uno::Reference< css::rendering::XCanvas >& rDestinationCanvas,
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
                // to the right and to the bottom),
                // create a non-transparent bitmap of that size
                uno::Reference< rendering::XIntegerBitmap > pBitmap(
                    rDestinationCanvas->getDevice()->createCompatibleBitmap(
                        ::basegfx::fround( aTmpRect.getRange().getX() + 1),
                        ::basegfx::fround( aTmpRect.getRange().getY() + 1) ),
                    uno::UNO_QUERY_THROW);

                ENSURE_OR_THROW( pBitmap.is(),
                                 "ViewBackgroundShape::prefetch(): Cannot create background bitmap" );

                uno::Reference< rendering::XCanvas > pBitmapCanvas(
                    pBitmap, uno::UNO_QUERY_THROW);

                ENSURE_OR_THROW( pBitmapCanvas.is(),
                                 "ViewBackgroundShape::prefetch(): Cannot create background bitmap canvas" );

                // clear bitmap
                pBitmapCanvas->fill(
                    ::basegfx::BColor(1.0,1.0,1.0).colorToDoubleSequence(
                        pBitmapCanvas->getDevice()) );

                // apply linear part of destination canvas transformation (linear means in this context:
                // transformation without any translational components)
                ::basegfx::B2DHomMatrix aLinearTransform( rCanvasTransform );
                aLinearTransform.set( 0, 2, 0.0 );
                aLinearTransform.set( 1, 2, 0.0 );
                // TODO-NYI
                //pBitmapCanvas->setTransformation( aLinearTransform );

                const basegfx::B2DHomMatrix aShapeTransform(basegfx::tools::createScaleTranslateB2DHomMatrix(
                    maBounds.getWidth(), maBounds.getHeight(),
                    maBounds.getMinX(), maBounds.getMinY()));

# if 0
                // TODO-NYI
                ::cppcanvas::RendererSharedPtr pRenderer(
                    ::cppcanvas::VCLFactory::getInstance().createRenderer(
                        pBitmapCanvas,
                        *rMtf.get(),
                        ::cppcanvas::Renderer::Parameters() ) );

                ENSURE_OR_RETURN_FALSE( pRenderer,
                                        "ViewBackgroundShape::prefetch(): Could not create Renderer" );

                pRenderer->setTransformation( aShapeTransform );
                pRenderer->draw();
# endif
                mxBitmap = pBitmap;
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
            ENSURE_OR_THROW( mpViewLayer,
                             "ViewBackgroundShape::ViewBackgroundShape(): Invalid View" );
            ENSURE_OR_THROW( mpViewLayer->getCanvas().is(),
                             "ViewBackgroundShape::ViewBackgroundShape(): Invalid ViewLayer canvas" );
        }

        ViewLayerSharedPtr ViewBackgroundShape::getViewLayer() const
        {
            return mpViewLayer;
        }

        bool ViewBackgroundShape::render( const GDIMetaFileSharedPtr& rMtf ) const
        {
            SAL_INFO( "slideshow", "::presentation::internal::ViewBackgroundShape::draw()" );

            const css::uno::Reference< css::rendering::XCanvas >& rDestinationCanvas( mpViewLayer->getCanvas() );

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
#if 0
                // TODO-NYI
                rDestinationCanvas->drawBitmap( mxBitmap,
                                                rDestinationCanvas->getViewState(),
                                                aRenderState );
#endif
            }
            catch( uno::Exception& )
            {
                OSL_FAIL( OUStringToOString(
                                comphelper::anyToString( cppu::getCaughtException() ),
                                RTL_TEXTENCODING_UTF8 ).getStr() );

                return false;
            }

            return true;
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
