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
#include "precompiled_canvas.hxx"

#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>

#include <rtl/logfile.hxx>
#include <rtl/math.hxx>

#include <com/sun/star/rendering/TexturingMode.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/RepaintResult.hpp>
#include <com/sun/star/rendering/PathCapType.hpp>
#include <com/sun/star/rendering/PathJoinType.hpp>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/tools/canvastools.hxx>

#include <comphelper/sequence.hxx>
#include <canvas/canvastools.hxx>

#include "dx_spritecanvas.hxx"
#include "dx_impltools.hxx"
#include "dx_canvasfont.hxx"
#include "dx_textlayout.hxx"
#include "dx_bitmapcanvashelper.hxx"

#include <algorithm>


using namespace ::com::sun::star;

namespace dxcanvas
{
    BitmapCanvasHelper::BitmapCanvasHelper() :
        mpTarget()
    {}

    void BitmapCanvasHelper::disposing()
    {
        mpTarget.reset();
        CanvasHelper::disposing();
    }

    void BitmapCanvasHelper::setTarget( const IBitmapSharedPtr& rTarget )
    {
        ENSURE_OR_THROW( rTarget,
                          "BitmapCanvasHelper::setTarget(): Invalid target" );
        ENSURE_OR_THROW( !mpTarget.get(),
                          "BitmapCanvasHelper::setTarget(): target set, old target would be overwritten" );

        mpTarget = rTarget;
        CanvasHelper::setTarget(rTarget);
    }

    void BitmapCanvasHelper::setTarget( const IBitmapSharedPtr&   rTarget,
                                        const ::basegfx::B2ISize& rOutputOffset )
    {
        ENSURE_OR_THROW( rTarget,
                         "BitmapCanvasHelper::setTarget(): invalid target" );
        ENSURE_OR_THROW( !mpTarget.get(),
                         "BitmapCanvasHelper::setTarget(): target set, old target would be overwritten" );

        mpTarget = rTarget;
        CanvasHelper::setTarget(rTarget,rOutputOffset);
    }

    void BitmapCanvasHelper::clear()
    {
        if( needOutput() )
        {
            GraphicsSharedPtr pGraphics( mpTarget->getGraphics() );

            Gdiplus::Color aClearColor = hasAlpha() ?
                Gdiplus::Color( 0,255,255,255 ) : Gdiplus::Color((Gdiplus::ARGB)Gdiplus::Color::White);

            ENSURE_OR_THROW(
                Gdiplus::Ok == pGraphics->SetCompositingMode(
                    Gdiplus::CompositingModeSourceCopy ), // force set, don't blend
                "BitmapCanvasHelper::clear(): GDI+ SetCompositingMode call failed" );
            ENSURE_OR_THROW(
                Gdiplus::Ok == pGraphics->Clear( aClearColor ),
                "BitmapCanvasHelper::clear(): GDI+ Clear call failed" );
        }
    }

    uno::Reference< rendering::XCachedPrimitive > BitmapCanvasHelper::drawTextLayout( const rendering::XCanvas*                       /*pCanvas*/,
                                                                                      const uno::Reference< rendering::XTextLayout >& xLayoutetText,
                                                                                      const rendering::ViewState&                     viewState,
                                                                                      const rendering::RenderState&                   renderState )
    {
        ENSURE_OR_THROW( xLayoutetText.is(),
                          "BitmapCanvasHelper::drawTextLayout: layout is NULL");

        if( needOutput() )
        {
            TextLayout* pTextLayout =
                dynamic_cast< TextLayout* >( xLayoutetText.get() );

            ENSURE_OR_THROW( pTextLayout,
                                "BitmapCanvasHelper::drawTextLayout(): TextLayout not compatible with this canvas" );

            pTextLayout->draw( mpTarget->getGraphics(),
                               viewState,
                               renderState,
                               maOutputOffset,
                               mpDevice,
                               mpTarget->hasAlpha() );
        }

        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    void BitmapCanvasHelper::copyRect( const rendering::XCanvas*                            /*pCanvas*/,
                                       const uno::Reference< rendering::XBitmapCanvas >&    /*sourceCanvas*/,
                                       const geometry::RealRectangle2D&                     /*sourceRect*/,
                                       const rendering::ViewState&                          /*sourceViewState*/,
                                       const rendering::RenderState&                        /*sourceRenderState*/,
                                       const geometry::RealRectangle2D&                     /*destRect*/,
                                       const rendering::ViewState&                          /*destViewState*/,
                                       const rendering::RenderState&                        /*destRenderState*/ )
    {
        // TODO(F2): copyRect NYI
    }

    geometry::IntegerSize2D BitmapCanvasHelper::getSize()
    {
        if( !mpTarget )
            return geometry::IntegerSize2D(1, 1);

        return basegfx::unotools::integerSize2DFromB2ISize(mpTarget->getSize());
    }

    uno::Reference< rendering::XBitmap > BitmapCanvasHelper::getScaledBitmap( const geometry::RealSize2D& /*newSize*/,
                                                                              sal_Bool                    /*beFast*/ )
    {
        // TODO(F1):
        return uno::Reference< rendering::XBitmap >();
    }

    uno::Sequence< sal_Int8 > BitmapCanvasHelper::getData( rendering::IntegerBitmapLayout&     bitmapLayout,
                                                           const geometry::IntegerRectangle2D& rect )
    {
        RTL_LOGFILE_CONTEXT( aLog, "::dxcanvas::BitmapCanvasHelper::getData()" );

        ENSURE_OR_THROW( mpTarget,
                          "::dxcanvas::BitmapCanvasHelper::getData(): disposed" );

        if( !mpTarget )
            return uno::Sequence< sal_Int8 >();

        bitmapLayout = getMemoryLayout();
        return mpTarget->getData(bitmapLayout,rect);
    }

    void BitmapCanvasHelper::setData( const uno::Sequence< sal_Int8 >&      data,
                                      const rendering::IntegerBitmapLayout& bitmapLayout,
                                      const geometry::IntegerRectangle2D&   rect )
    {
        RTL_LOGFILE_CONTEXT( aLog, "::dxcanvas::BitmapCanvasHelper::setData()" );

        ENSURE_OR_THROW( mpTarget,
                          "::dxcanvas::BitmapCanvasHelper::setData(): disposed" );

        if( !mpTarget )
            return;

        mpTarget->setData(data,bitmapLayout,rect);
    }

    void BitmapCanvasHelper::setPixel( const uno::Sequence< sal_Int8 >&       color,
                                       const rendering::IntegerBitmapLayout&  bitmapLayout,
                                       const geometry::IntegerPoint2D&        pos )
    {
        RTL_LOGFILE_CONTEXT( aLog, "::dxcanvas::BitmapCanvasHelper::setPixel()" );

        ENSURE_OR_THROW( mpTarget,
                          "::dxcanvas::BitmapCanvasHelper::setPixel(): disposed" );

        if( !mpTarget )
            return;

        mpTarget->setPixel(color,bitmapLayout,pos);
    }

    uno::Sequence< sal_Int8 > BitmapCanvasHelper::getPixel( rendering::IntegerBitmapLayout&   bitmapLayout,
                                                            const geometry::IntegerPoint2D&   pos )
    {
        RTL_LOGFILE_CONTEXT( aLog, "::dxcanvas::BitmapCanvasHelper::getPixel()" );

        ENSURE_OR_THROW( mpTarget,
                          "::dxcanvas::BitmapCanvasHelper::getPixel(): disposed" );

        if( !mpTarget )
            return uno::Sequence< sal_Int8 >();

        bitmapLayout = getMemoryLayout();
        return mpTarget->getPixel(bitmapLayout,pos);
    }

    uno::Reference< rendering::XBitmapPalette > BitmapCanvasHelper::getPalette()
    {
        // TODO(F1): Palette bitmaps NYI
        return uno::Reference< rendering::XBitmapPalette >();
    }

    rendering::IntegerBitmapLayout BitmapCanvasHelper::getMemoryLayout()
    {
        if( !mpTarget )
            return rendering::IntegerBitmapLayout(); // we're disposed

        return ::canvas::tools::getStdMemoryLayout(getSize());
    }
    bool BitmapCanvasHelper::hasAlpha() const
    {
        return mpTarget ? mpTarget->hasAlpha() : false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
