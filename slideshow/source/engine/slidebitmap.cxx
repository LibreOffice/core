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
#include <slidebitmap.hxx>

#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XBitmap.hpp>
#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include <canvas/canvastools.hxx>
#include <basegfx/tools/canvastools.hxx>


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

            const basegfx::B2DHomMatrix aTranslation(basegfx::tools::createTranslateB2DHomMatrix(maOutputPos));
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
                OSL_FAIL( rtl::OUStringToOString(
                                comphelper::anyToString( cppu::getCaughtException() ),
                                RTL_TEXTENCODING_UTF8 ).getStr() );

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

        ::com::sun::star::uno::Reference<
                ::com::sun::star::rendering::XBitmap >    SlideBitmap::getXBitmap()
        {
        return mxBitmap;
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
