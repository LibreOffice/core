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


#include <tools/diagnose_ex.h>
#include <slidebitmap.hxx>
#include <sal/log.hxx>

#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XBitmap.hpp>
#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include <canvas/canvastools.hxx>
#include <basegfx/utils/canvastools.hxx>


using namespace ::com::sun::star;

namespace slideshow
{
    namespace internal
    {

        SlideBitmap::SlideBitmap( const ::cppcanvas::BitmapSharedPtr& rBitmap ) :
            maOutputPos(),
            maClipPoly(),
            mxBitmap()
        {
            if( rBitmap )
                mxBitmap = rBitmap->getUNOBitmap();

            ENSURE_OR_THROW( mxBitmap.is(), "SlideBitmap::SlideBitmap(): Invalid bitmap" );
        }

        bool SlideBitmap::draw( const ::cppcanvas::CanvasSharedPtr& rCanvas ) const
        {
            ENSURE_OR_RETURN_FALSE( rCanvas && rCanvas->getUNOCanvas().is(),
                               "SlideBitmap::draw(): Invalid canvas" );

            // selectively only copy the transformation from current viewstate,
            // don't want no clipping here.
            rendering::ViewState aViewState;
            aViewState.AffineTransform = rCanvas->getViewState().AffineTransform;

            rendering::RenderState aRenderState;
            ::canvas::tools::initRenderState( aRenderState );

            const basegfx::B2DHomMatrix aTranslation(basegfx::utils::createTranslateB2DHomMatrix(maOutputPos));
            ::canvas::tools::setRenderStateTransform( aRenderState, aTranslation );

            try
            {
                if( maClipPoly.count() )
                {
                    // TODO(P1): Buffer the clip polygon
                    aRenderState.Clip =
                        ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                            rCanvas->getUNOCanvas()->getDevice(),
                            maClipPoly );
                }

                rCanvas->getUNOCanvas()->drawBitmap( mxBitmap,
                                                     aViewState,
                                                     aRenderState );
            }
            catch( uno::Exception& )
            {
                SAL_WARN( "slideshow", exceptionToString( cppu::getCaughtException() ) );
                return false;
            }

            return true;
        }

        ::basegfx::B2ISize SlideBitmap::getSize() const
        {
            return ::basegfx::unotools::b2ISizeFromIntegerSize2D( mxBitmap->getSize() );
        }

        void SlideBitmap::move( const ::basegfx::B2DPoint& rNewPos )
        {
            maOutputPos = rNewPos;
        }

        void SlideBitmap::clip( const ::basegfx::B2DPolyPolygon& rClipPoly )
        {
            maClipPoly = rClipPoly;
        }

        const css::uno::Reference< css::rendering::XBitmap >&  SlideBitmap::getXBitmap()
        {
        return mxBitmap;
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
