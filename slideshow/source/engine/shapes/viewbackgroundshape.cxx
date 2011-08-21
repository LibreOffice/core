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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_slideshow.hxx"

// must be first
#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>

#include "viewbackgroundshape.hxx"
#include "tools.hxx"

#include <rtl/logfile.hxx>
#include <rtl/math.hxx>

#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include <com/sun/star/rendering/XCanvas.hpp>

#include <canvas/verbosetrace.hxx>
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
            RTL_LOGFILE_CONTEXT( aLog, "::presentation::internal::ViewBackgroundShape::prefetch()" );
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
                    ::cppcanvas::BaseGfxFactory::getInstance().createBitmap(
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

                const basegfx::B2DHomMatrix aShapeTransform(basegfx::tools::createScaleTranslateB2DHomMatrix(
                    maBounds.getWidth(), maBounds.getHeight(),
                    maBounds.getMinX(), maBounds.getMinY()));

                ::cppcanvas::RendererSharedPtr pRenderer(
                    ::cppcanvas::VCLFactory::getInstance().createRenderer(
                        pBitmapCanvas,
                        *rMtf.get(),
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

        ViewLayerSharedPtr ViewBackgroundShape::getViewLayer() const
        {
            return mpViewLayer;
        }

        bool ViewBackgroundShape::render( const GDIMetaFileSharedPtr& rMtf ) const
        {
            RTL_LOGFILE_CONTEXT( aLog, "::presentation::internal::ViewBackgroundShape::draw()" );

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
                OSL_FAIL( rtl::OUStringToOString(
                                comphelper::anyToString( cppu::getCaughtException() ),
                                RTL_TEXTENCODING_UTF8 ).getStr() );

                return false;
            }

            return true;
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
