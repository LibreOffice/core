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

#include <sal/config.h>

#include <algorithm>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/PathCapType.hpp>
#include <com/sun/star/rendering/PathJoinType.hpp>
#include <com/sun/star/rendering/RepaintResult.hpp>
#include <com/sun/star/rendering/TexturingMode.hpp>
#include <comphelper/sequence.hxx>
#include <rtl/math.hxx>
#include <tools/diagnose_ex.h>

#include <canvas/canvastools.hxx>

#include "dx_bitmapcanvashelper.hxx"
#include "dx_canvasfont.hxx"
#include "dx_impltools.hxx"
#include "dx_spritecanvas.hxx"
#include "dx_textlayout.hxx"


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

        return uno::Reference< rendering::XCachedPrimitive >(nullptr);
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
                                                                              bool                        /*beFast*/ )
    {
        // TODO(F1):
        return uno::Reference< rendering::XBitmap >();
    }

    uno::Sequence< sal_Int8 > BitmapCanvasHelper::getData( rendering::IntegerBitmapLayout&     bitmapLayout,
                                                           const geometry::IntegerRectangle2D& rect )
    {
        SAL_INFO( "canvas.directx", "::dxcanvas::BitmapCanvasHelper::getData()" );

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
        SAL_INFO( "canvas.directx", "::dxcanvas::BitmapCanvasHelper::setData()" );

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
        SAL_INFO( "canvas.directx", "::dxcanvas::BitmapCanvasHelper::setPixel()" );

        ENSURE_OR_THROW( mpTarget,
                          "::dxcanvas::BitmapCanvasHelper::setPixel(): disposed" );

        if( !mpTarget )
            return;

        mpTarget->setPixel(color,bitmapLayout,pos);
    }

    uno::Sequence< sal_Int8 > BitmapCanvasHelper::getPixel( rendering::IntegerBitmapLayout&   bitmapLayout,
                                                            const geometry::IntegerPoint2D&   pos )
    {
        SAL_INFO( "canvas.directx", "::dxcanvas::BitmapCanvasHelper::getPixel()" );

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
        return mpTarget && mpTarget->hasAlpha();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
