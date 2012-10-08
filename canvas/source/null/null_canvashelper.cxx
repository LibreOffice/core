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


#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>

#include <rtl/logfile.hxx>
#include <rtl/math.hxx>

#include <com/sun/star/util/Endianness.hpp>
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

#include "null_spritecanvas.hxx"
#include "null_canvasfont.hxx"
#include "null_textlayout.hxx"
#include "null_canvashelper.hxx"

#include <algorithm>


using namespace ::com::sun::star;

namespace nullcanvas
{
    CanvasHelper::CanvasHelper() :
        mpDevice( NULL ),
        maSize(),
        mbHaveAlpha()
    {
    }

    void CanvasHelper::disposing()
    {
        mpDevice = NULL;
    }

    void CanvasHelper::init( const ::basegfx::B2ISize& rSize,
                             SpriteCanvas&             rDevice,
                             bool                      bHasAlpha )
    {
        mpDevice = &rDevice;
        maSize = rSize;
        mbHaveAlpha = bHasAlpha;
    }

    void CanvasHelper::clear()
    {
    }

    void CanvasHelper::drawPoint( const rendering::XCanvas*     /*pCanvas*/,
                                  const geometry::RealPoint2D&  /*aPoint*/,
                                  const rendering::ViewState&   /*viewState*/,
                                  const rendering::RenderState& /*renderState*/ )
    {
    }

    void CanvasHelper::drawLine( const rendering::XCanvas*      /*pCanvas*/,
                                 const geometry::RealPoint2D&   /*aPoint1*/,
                                 const geometry::RealPoint2D&   /*aPoint2*/,
                                 const rendering::ViewState&    /*viewState*/,
                                 const rendering::RenderState&  /*renderState*/ )
    {
    }

    void CanvasHelper::drawBezier( const rendering::XCanvas*            /*pCanvas*/,
                                   const geometry::RealBezierSegment2D& /*aBezierSegment*/,
                                   const geometry::RealPoint2D&         /*aEndPoint*/,
                                   const rendering::ViewState&          /*viewState*/,
                                   const rendering::RenderState&        /*renderState*/ )
    {
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::drawPolyPolygon( const rendering::XCanvas*                          /*pCanvas*/,
                                                                                 const uno::Reference< rendering::XPolyPolygon2D >& /*xPolyPolygon*/,
                                                                                 const rendering::ViewState&                        /*viewState*/,
                                                                                 const rendering::RenderState&                      /*renderState*/ )
    {
        // TODO(P1): Provide caching here.
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::strokePolyPolygon( const rendering::XCanvas*                            /*pCanvas*/,
                                                                                   const uno::Reference< rendering::XPolyPolygon2D >&   /*xPolyPolygon*/,
                                                                                   const rendering::ViewState&                          /*viewState*/,
                                                                                   const rendering::RenderState&                        /*renderState*/,
                                                                                   const rendering::StrokeAttributes&                   /*strokeAttributes*/ )
    {
        // TODO(P1): Provide caching here.
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::strokeTexturedPolyPolygon( const rendering::XCanvas*                            /*pCanvas*/,
                                                                                           const uno::Reference< rendering::XPolyPolygon2D >&   /*xPolyPolygon*/,
                                                                                           const rendering::ViewState&                          /*viewState*/,
                                                                                           const rendering::RenderState&                        /*renderState*/,
                                                                                           const uno::Sequence< rendering::Texture >&           /*textures*/,
                                                                                           const rendering::StrokeAttributes&                   /*strokeAttributes*/ )
    {
        // TODO
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::strokeTextureMappedPolyPolygon( const rendering::XCanvas*                           /*pCanvas*/,
                                                                                                const uno::Reference< rendering::XPolyPolygon2D >&  /*xPolyPolygon*/,
                                                                                                const rendering::ViewState&                         /*viewState*/,
                                                                                                const rendering::RenderState&                       /*renderState*/,
                                                                                                const uno::Sequence< rendering::Texture >&          /*textures*/,
                                                                                                const uno::Reference< geometry::XMapping2D >&       /*xMapping*/,
                                                                                                const rendering::StrokeAttributes&                  /*strokeAttributes*/ )
    {
        // TODO
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XPolyPolygon2D >   CanvasHelper::queryStrokeShapes( const rendering::XCanvas*                            /*pCanvas*/,
                                                                                   const uno::Reference< rendering::XPolyPolygon2D >&   /*xPolyPolygon*/,
                                                                                   const rendering::ViewState&                          /*viewState*/,
                                                                                   const rendering::RenderState&                        /*renderState*/,
                                                                                   const rendering::StrokeAttributes&                   /*strokeAttributes*/ )
    {
        // TODO
        return uno::Reference< rendering::XPolyPolygon2D >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::fillPolyPolygon( const rendering::XCanvas*                          /*pCanvas*/,
                                                                                 const uno::Reference< rendering::XPolyPolygon2D >& /*xPolyPolygon*/,
                                                                                 const rendering::ViewState&                        /*viewState*/,
                                                                                 const rendering::RenderState&                      /*renderState*/ )
    {
        // TODO(P1): Provide caching here.
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::fillTexturedPolyPolygon( const rendering::XCanvas*                          /*pCanvas*/,
                                                                                         const uno::Reference< rendering::XPolyPolygon2D >& /*xPolyPolygon*/,
                                                                                         const rendering::ViewState&                        /*viewState*/,
                                                                                         const rendering::RenderState&                      /*renderState*/,
                                                                                         const uno::Sequence< rendering::Texture >&         /*textures*/ )
    {
        // TODO(P1): Provide caching here.
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::fillTextureMappedPolyPolygon( const rendering::XCanvas*                             /*pCanvas*/,
                                                                                              const uno::Reference< rendering::XPolyPolygon2D >&    /*xPolyPolygon*/,
                                                                                              const rendering::ViewState&                           /*viewState*/,
                                                                                              const rendering::RenderState&                         /*renderState*/,
                                                                                              const uno::Sequence< rendering::Texture >&            /*textures*/,
                                                                                              const uno::Reference< geometry::XMapping2D >&         /*xMapping*/ )
    {
        // TODO
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCanvasFont > CanvasHelper::createFont( const rendering::XCanvas*                    /*pCanvas*/,
                                                                       const rendering::FontRequest&                fontRequest,
                                                                       const uno::Sequence< beans::PropertyValue >& extraFontProperties,
                                                                       const geometry::Matrix2D&                    fontMatrix )
    {
        return uno::Reference< rendering::XCanvasFont >(
            new CanvasFont(fontRequest, extraFontProperties, fontMatrix ) );
    }

    uno::Sequence< rendering::FontInfo > CanvasHelper::queryAvailableFonts( const rendering::XCanvas*                       /*pCanvas*/,
                                                                            const rendering::FontInfo&                      /*aFilter*/,
                                                                            const uno::Sequence< beans::PropertyValue >&    /*aFontProperties*/ )
    {
        // TODO
        return uno::Sequence< rendering::FontInfo >();
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::drawText( const rendering::XCanvas*                         /*pCanvas*/,
                                                                          const rendering::StringContext&                   /*text*/,
                                                                          const uno::Reference< rendering::XCanvasFont >&   /*xFont*/,
                                                                          const rendering::ViewState&                       /*viewState*/,
                                                                          const rendering::RenderState&                     /*renderState*/,
                                                                          sal_Int8                                          /*textDirection*/ )
    {
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::drawTextLayout( const rendering::XCanvas*                       /*pCanvas*/,
                                                                                const uno::Reference< rendering::XTextLayout >& xLayoutetText,
                                                                                const rendering::ViewState&                     viewState,
                                                                                const rendering::RenderState&                   renderState )
    {
        ENSURE_OR_THROW( xLayoutetText.is(),
                          "CanvasHelper::drawTextLayout: layout is NULL");

        TextLayout* pTextLayout =
            dynamic_cast< TextLayout* >( xLayoutetText.get() );

        ENSURE_OR_THROW( pTextLayout,
                          "CanvasHelper::drawTextLayout(): TextLayout not compatible with this canvas" );

        pTextLayout->draw( viewState,
                           renderState,
                           mpDevice );

        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::drawBitmap( const rendering::XCanvas*                   /*pCanvas*/,
                                                                            const uno::Reference< rendering::XBitmap >& /*xBitmap*/,
                                                                            const rendering::ViewState&                 /*viewState*/,
                                                                            const rendering::RenderState&               /*renderState*/ )
    {
        // TODO(P1): Provide caching here.
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::drawBitmapModulated( const rendering::XCanvas*                      /*pCanvas*/,
                                                                                     const uno::Reference< rendering::XBitmap >&    /*xBitmap*/,
                                                                                     const rendering::ViewState&                    /*viewState*/,
                                                                                     const rendering::RenderState&                  /*renderState*/ )
    {
        // TODO(P1): Provide caching here.
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XGraphicDevice > CanvasHelper::getDevice()
    {
        return uno::Reference< rendering::XGraphicDevice >(mpDevice);
    }

    void CanvasHelper::copyRect( const rendering::XCanvas*                          /*pCanvas*/,
                                 const uno::Reference< rendering::XBitmapCanvas >&  /*sourceCanvas*/,
                                 const geometry::RealRectangle2D&                   /*sourceRect*/,
                                 const rendering::ViewState&                        /*sourceViewState*/,
                                 const rendering::RenderState&                      /*sourceRenderState*/,
                                 const geometry::RealRectangle2D&                   /*destRect*/,
                                 const rendering::ViewState&                        /*destViewState*/,
                                 const rendering::RenderState&                      /*destRenderState*/ )
    {
        // TODO(F2): copyRect NYI
    }

    geometry::IntegerSize2D CanvasHelper::getSize()
    {
        if( !mpDevice )
            geometry::IntegerSize2D(1, 1); // we're disposed

        return ::basegfx::unotools::integerSize2DFromB2ISize( maSize );
    }

    uno::Reference< rendering::XBitmap > CanvasHelper::getScaledBitmap( const geometry::RealSize2D& /*newSize*/,
                                                                        sal_Bool                    /*beFast*/ )
    {
        // TODO(F1):
        return uno::Reference< rendering::XBitmap >();
    }

    uno::Sequence< sal_Int8 > CanvasHelper::getData( rendering::IntegerBitmapLayout&     /*bitmapLayout*/,
                                                     const geometry::IntegerRectangle2D& /*rect*/ )
    {
        // TODO
        return uno::Sequence< sal_Int8 >();
    }

    void CanvasHelper::setData( const uno::Sequence< sal_Int8 >&        /*data*/,
                                const rendering::IntegerBitmapLayout&   /*bitmapLayout*/,
                                const geometry::IntegerRectangle2D&     /*rect*/ )
    {
    }

    void CanvasHelper::setPixel( const uno::Sequence< sal_Int8 >&       /*color*/,
                                 const rendering::IntegerBitmapLayout&  /*bitmapLayout*/,
                                 const geometry::IntegerPoint2D&        /*pos*/ )
    {
    }

    uno::Sequence< sal_Int8 > CanvasHelper::getPixel( rendering::IntegerBitmapLayout&   /*bitmapLayout*/,
                                                      const geometry::IntegerPoint2D&   /*pos*/ )
    {
        return uno::Sequence< sal_Int8 >();
    }

    rendering::IntegerBitmapLayout CanvasHelper::getMemoryLayout()
    {
        return ::canvas::tools::getStdMemoryLayout(getSize());
    }

    bool CanvasHelper::hasAlpha() const
    {
        return mbHaveAlpha;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
