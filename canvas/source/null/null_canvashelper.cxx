/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: null_canvashelper.cxx,v $
 * $Revision: 1.6 $
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

#include <rtl/logfile.hxx>
#include <rtl/math.hxx>

#include <com/sun/star/rendering/IntegerBitmapFormat.hpp>
#include <com/sun/star/rendering/Endianness.hpp>
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
        ENSURE_AND_THROW( xLayoutetText.is(),
                          "CanvasHelper::drawTextLayout: layout is NULL");

        TextLayout* pTextLayout =
            dynamic_cast< TextLayout* >( xLayoutetText.get() );

        ENSURE_AND_THROW( pTextLayout,
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

    uno::Reference< rendering::XBitmapPalette > CanvasHelper::getPalette()
    {
        // TODO(F1): Palette bitmaps NYI
        return uno::Reference< rendering::XBitmapPalette >();
    }

    rendering::IntegerBitmapLayout CanvasHelper::getMemoryLayout()
    {
        // TODO(F1): finish memory layout initialization
        rendering::IntegerBitmapLayout aLayout;

        const geometry::IntegerSize2D& rBmpSize( getSize() );

        aLayout.ScanLines = rBmpSize.Width;
        aLayout.ScanLineBytes = rBmpSize.Height * 4;
        aLayout.ScanLineStride = aLayout.ScanLineBytes;
        aLayout.PlaneStride = 0;
        aLayout.ColorSpace.set( mpDevice );
        aLayout.NumComponents = 4;
        aLayout.ComponentMasks.realloc(4);
        aLayout.ComponentMasks[0] = 0x00FF0000;
        aLayout.ComponentMasks[1] = 0x0000FF00;
        aLayout.ComponentMasks[2] = 0x000000FF;
        aLayout.ComponentMasks[3] = 0xFF000000;
        aLayout.Palette.clear();
        aLayout.Endianness = rendering::Endianness::LITTLE;
        aLayout.Format = rendering::IntegerBitmapFormat::CHUNKY_32BIT;
        aLayout.IsMsbFirst = sal_False;

        return aLayout;
    }

    void CanvasHelper::flush() const
    {
    }

    bool CanvasHelper::hasAlpha() const
    {
        return mbHaveAlpha;
    }

}
