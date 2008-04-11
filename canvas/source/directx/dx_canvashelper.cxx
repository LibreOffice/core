/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dx_canvashelper.cxx,v $
 * $Revision: 1.4 $
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

#include "dx_spritecanvas.hxx"
#include "dx_impltools.hxx"
#include "dx_canvasfont.hxx"
#include "dx_textlayout.hxx"
#include "dx_canvashelper.hxx"

#include <algorithm>


using namespace ::com::sun::star;

namespace dxcanvas
{
    namespace
    {
        Gdiplus::LineCap gdiCapFromCap( sal_Int8 nCapType )
        {
            switch( nCapType )
            {
                case rendering::PathCapType::BUTT:
                    return Gdiplus::LineCapFlat;

                case rendering::PathCapType::ROUND:
                    return Gdiplus::LineCapRound;

                case rendering::PathCapType::SQUARE:
                    return Gdiplus::LineCapSquare;

                default:
                    ENSURE_AND_THROW( false,
                                      "gdiCapFromCap(): Unexpected cap type" );
            }

            return Gdiplus::LineCapFlat;
        }

        Gdiplus::LineJoin gdiJoinFromJoin( sal_Int8 nJoinType )
        {
            switch( nJoinType )
            {
                case rendering::PathJoinType::NONE:
                    OSL_ENSURE( false,
                                "gdiJoinFromJoin(): Join NONE not possible, mapping to MITER" );
                    // FALLTHROUGH intended
                case rendering::PathJoinType::MITER:
                    return Gdiplus::LineJoinMiter;

                case rendering::PathJoinType::ROUND:
                    return Gdiplus::LineJoinRound;

                case rendering::PathJoinType::BEVEL:
                    return Gdiplus::LineJoinBevel;

                default:
                    ENSURE_AND_THROW( false,
                                      "gdiJoinFromJoin(): Unexpected join type" );
            }

            return Gdiplus::LineJoinMiter;
        }
    }

    CanvasHelper::CanvasHelper() :
        mpDevice( NULL ),
        mpTarget(),
        mpGdiPlusUser( GDIPlusUser::createInstance() ),
        maOutputOffset()
    {
    }

    void CanvasHelper::disposing()
    {
        mpGdiPlusUser.reset();
        mpDevice = NULL;
        mpTarget.reset();
    }

    void CanvasHelper::setDevice( SpriteCanvas& rDevice )
    {
        mpDevice = &rDevice;
    }

    void CanvasHelper::setTarget( const DXBitmapSharedPtr& rTarget )
    {
        ENSURE_AND_THROW( rTarget,
                          "CanvasHelper::setTarget(): Invalid target" );
        ENSURE_AND_THROW( !mpTarget.get(),
                          "CanvasHelper::setTarget(): target set, old target would be overwritten" );

        mpTarget = rTarget;
    }

    void CanvasHelper::setTarget( const DXBitmapSharedPtr&      rTarget,
                                   const ::basegfx::B2ISize&    rOutputOffset )
    {
        ENSURE_AND_THROW( rTarget,
                          "CanvasHelper::setTarget(): invalid target" );
        ENSURE_AND_THROW( !mpTarget.get(),
                          "CanvasHelper::setTarget(): target set, old target would be overwritten" );

        mpTarget = rTarget;
        maOutputOffset = rOutputOffset;
    }

    void CanvasHelper::clear()
    {
        if( needOutput() )
        {
            SurfaceGraphicsSharedPtr aGraphics( mpTarget->getGraphics() );

            Gdiplus::Color aClearColor = hasAlpha() ?
                Gdiplus::Color( 0,255,255,255 ) : Gdiplus::Color((Gdiplus::ARGB)Gdiplus::Color::White);

            ENSURE_AND_THROW(
                Gdiplus::Ok == (*aGraphics.get())->SetCompositingMode(
                    Gdiplus::CompositingModeSourceCopy ), // force set, don't blend
                "CanvasHelper::clear(): GDI+ SetCompositingMode call failed" );
            ENSURE_AND_THROW(
                Gdiplus::Ok == (*aGraphics)->Clear( aClearColor ),
                "CanvasHelper::clear(): GDI+ Clear call failed" );
        }
    }

    void CanvasHelper::drawPoint( const rendering::XCanvas*     /*pCanvas*/,
                                  const geometry::RealPoint2D&  aPoint,
                                  const rendering::ViewState&   viewState,
                                  const rendering::RenderState& renderState )
    {
        if( needOutput() )
        {
            SurfaceGraphicsSharedPtr aGraphics( mpTarget->getGraphics() );

            setupGraphicsState( aGraphics, viewState, renderState );

            Gdiplus::SolidBrush aBrush(
                Gdiplus::Color(
                    tools::sequenceToArgb(renderState.DeviceColor)) );

            // determine size of one-by-one device pixel ellipse
            Gdiplus::Matrix aMatrix;
            (*aGraphics)->GetTransform(&aMatrix);
            aMatrix.Invert();
            Gdiplus::PointF vector(1, 1);
            aMatrix.TransformVectors(&vector);

            // paint a one-by-one circle, with the given point
            // in the middle (rounded to float)
            ENSURE_AND_THROW(
                Gdiplus::Ok == (*aGraphics)->FillEllipse( &aBrush,
                                                          // disambiguate call
                                                          Gdiplus::REAL(aPoint.X),
                                                          Gdiplus::REAL(aPoint.Y),
                                                          Gdiplus::REAL(vector.X),
                                                          Gdiplus::REAL(vector.Y) ),
                "CanvasHelper::drawPoint(): GDI+ call failed" );
        }
    }

    void CanvasHelper::drawLine( const rendering::XCanvas*      /*pCanvas*/,
                                 const geometry::RealPoint2D&   aStartPoint,
                                 const geometry::RealPoint2D&   aEndPoint,
                                 const rendering::ViewState&    viewState,
                                 const rendering::RenderState&  renderState )
    {
        if( needOutput() )
        {
            SurfaceGraphicsSharedPtr aGraphics( mpTarget->getGraphics() );

            setupGraphicsState( aGraphics, viewState, renderState );

            Gdiplus::Pen aPen(
                Gdiplus::Color(
                    tools::sequenceToArgb(renderState.DeviceColor)),
                Gdiplus::REAL(0.0) );

            // #122683# Switched precedence of pixel offset
            // mode. Seemingly, polygon stroking needs
            // PixelOffsetModeNone to achieve visually pleasing
            // results, whereas all other operations (e.g. polygon
            // fills, bitmaps) look better with PixelOffsetModeHalf.
            const Gdiplus::PixelOffsetMode aOldMode(
                (*aGraphics)->GetPixelOffsetMode() );
            (*aGraphics)->SetPixelOffsetMode( Gdiplus::PixelOffsetModeNone );

            Gdiplus::Status hr = (*aGraphics)->DrawLine( &aPen,
                                                         Gdiplus::REAL(aStartPoint.X), // disambiguate call
                                                         Gdiplus::REAL(aStartPoint.Y),
                                                         Gdiplus::REAL(aEndPoint.X),
                                                         Gdiplus::REAL(aEndPoint.Y) );
            (*aGraphics)->SetPixelOffsetMode( aOldMode );

            ENSURE_AND_THROW(
                Gdiplus::Ok == hr,
                "CanvasHelper::drawLine(): GDI+ call failed" );
        }
    }

    void CanvasHelper::drawBezier( const rendering::XCanvas*            /*pCanvas*/,
                                   const geometry::RealBezierSegment2D& aBezierSegment,
                                   const geometry::RealPoint2D&         aEndPoint,
                                   const rendering::ViewState&          viewState,
                                   const rendering::RenderState&        renderState )
    {
        if( needOutput() )
        {
            SurfaceGraphicsSharedPtr aGraphics( mpTarget->getGraphics() );

            setupGraphicsState( aGraphics, viewState, renderState );

            Gdiplus::Pen aPen(
                Gdiplus::Color(
                    tools::sequenceToArgb(renderState.DeviceColor)),
                Gdiplus::REAL(0.0) );

            // #122683# Switched precedence of pixel offset
            // mode. Seemingly, polygon stroking needs
            // PixelOffsetModeNone to achieve visually pleasing
            // results, whereas all other operations (e.g. polygon
            // fills, bitmaps) look better with PixelOffsetModeHalf.
            const Gdiplus::PixelOffsetMode aOldMode(
                (*aGraphics)->GetPixelOffsetMode() );
            (*aGraphics)->SetPixelOffsetMode( Gdiplus::PixelOffsetModeNone );

            Gdiplus::Status hr = (*aGraphics)->DrawBezier( &aPen,
                                                           Gdiplus::REAL(aBezierSegment.Px), // disambiguate call
                                                           Gdiplus::REAL(aBezierSegment.Py),
                                                           Gdiplus::REAL(aBezierSegment.C1x),
                                                           Gdiplus::REAL(aBezierSegment.C1y),
                                                           Gdiplus::REAL(aEndPoint.X),
                                                           Gdiplus::REAL(aEndPoint.Y),
                                                           Gdiplus::REAL(aBezierSegment.C2x),
                                                           Gdiplus::REAL(aBezierSegment.C2y) );

            (*aGraphics)->SetPixelOffsetMode( aOldMode );

            ENSURE_AND_THROW(
                Gdiplus::Ok == hr,
                "CanvasHelper::drawBezier(): GDI+ call failed" );
        }
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::drawPolyPolygon( const rendering::XCanvas*                          /*pCanvas*/,
                                                                                 const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon,
                                                                                 const rendering::ViewState&                        viewState,
                                                                                 const rendering::RenderState&                      renderState )
    {
        ENSURE_AND_THROW( xPolyPolygon.is(),
                          "CanvasHelper::drawPolyPolygon: polygon is NULL");

        if( needOutput() )
        {
            SurfaceGraphicsSharedPtr aGraphics( mpTarget->getGraphics() );

            setupGraphicsState( aGraphics, viewState, renderState );

            Gdiplus::Pen aPen(
                Gdiplus::Color(
                    tools::sequenceToArgb(renderState.DeviceColor)),
                Gdiplus::REAL(0.0) );

            // #122683# Switched precedence of pixel offset
            // mode. Seemingly, polygon stroking needs
            // PixelOffsetModeNone to achieve visually pleasing
            // results, whereas all other operations (e.g. polygon
            // fills, bitmaps) look better with PixelOffsetModeHalf.
            const Gdiplus::PixelOffsetMode aOldMode(
                (*aGraphics)->GetPixelOffsetMode() );
            (*aGraphics)->SetPixelOffsetMode( Gdiplus::PixelOffsetModeNone );

            GraphicsPathSharedPtr pPath( tools::graphicsPathFromXPolyPolygon2D( xPolyPolygon ) );

            // TODO(E1): Return value
            Gdiplus::Status hr = (*aGraphics)->DrawPath( &aPen, pPath.get() );

            (*aGraphics)->SetPixelOffsetMode( aOldMode );

            ENSURE_AND_THROW(
                Gdiplus::Ok == hr,
                "CanvasHelper::drawPolyPolygon(): GDI+ call failed" );
        }

        // TODO(P1): Provide caching here.
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::strokePolyPolygon( const rendering::XCanvas*                            /*pCanvas*/,
                                                                                   const uno::Reference< rendering::XPolyPolygon2D >&   xPolyPolygon,
                                                                                   const rendering::ViewState&                          viewState,
                                                                                   const rendering::RenderState&                        renderState,
                                                                                   const rendering::StrokeAttributes&                   strokeAttributes )
    {
        ENSURE_AND_THROW( xPolyPolygon.is(),
                          "CanvasHelper::drawPolyPolygon: polygon is NULL");

        if( needOutput() )
        {
            SurfaceGraphicsSharedPtr aGraphics( mpTarget->getGraphics() );

            setupGraphicsState( aGraphics, viewState, renderState );


            // Setup stroke pen
            // ----------------

            Gdiplus::Pen aPen(
                Gdiplus::Color(
                    tools::sequenceToArgb(renderState.DeviceColor)),
                static_cast< Gdiplus::REAL >(strokeAttributes.StrokeWidth) );

            // #122683# Switched precedence of pixel offset
            // mode. Seemingly, polygon stroking needs
            // PixelOffsetModeNone to achieve visually pleasing
            // results, whereas all other operations (e.g. polygon
            // fills, bitmaps) look better with PixelOffsetModeHalf.
            const Gdiplus::PixelOffsetMode aOldMode(
                (*aGraphics)->GetPixelOffsetMode() );
            (*aGraphics)->SetPixelOffsetMode( Gdiplus::PixelOffsetModeNone );

            aPen.SetMiterLimit( static_cast< Gdiplus::REAL >(strokeAttributes.MiterLimit) );

            const ::std::vector< Gdiplus::REAL >& rDashArray(
                ::comphelper::sequenceToContainer< ::std::vector< Gdiplus::REAL > >(
                    strokeAttributes.DashArray ) );
            if( !rDashArray.empty() )
            {
                aPen.SetDashPattern( &rDashArray[0],
                                     rDashArray.size() );
            }
            aPen.SetLineCap( gdiCapFromCap(strokeAttributes.StartCapType),
                             gdiCapFromCap(strokeAttributes.EndCapType),
                             Gdiplus::DashCapFlat );
            aPen.SetLineJoin( gdiJoinFromJoin(strokeAttributes.JoinType) );

            GraphicsPathSharedPtr pPath( tools::graphicsPathFromXPolyPolygon2D( xPolyPolygon ) );

            // TODO(E1): Return value
            Gdiplus::Status hr = (*aGraphics)->DrawPath( &aPen, pPath.get() );

            (*aGraphics)->SetPixelOffsetMode( aOldMode );

            ENSURE_AND_THROW(
                Gdiplus::Ok == hr,
                "CanvasHelper::strokePolyPolygon(): GDI+ call failed" );
        }

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
                                                                                 const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon,
                                                                                 const rendering::ViewState&                        viewState,
                                                                                 const rendering::RenderState&                      renderState )
    {
        ENSURE_AND_THROW( xPolyPolygon.is(),
                          "CanvasHelper::fillPolyPolygon: polygon is NULL");

        if( needOutput() )
        {
            SurfaceGraphicsSharedPtr aGraphics( mpTarget->getGraphics() );

            setupGraphicsState( aGraphics, viewState, renderState );

            Gdiplus::SolidBrush aBrush(
                tools::sequenceToArgb(renderState.DeviceColor));

            GraphicsPathSharedPtr pPath( tools::graphicsPathFromXPolyPolygon2D( xPolyPolygon ) );

            // TODO(F1): FillRule
            ENSURE_AND_THROW( Gdiplus::Ok == (*aGraphics)->FillPath( &aBrush, pPath.get() ),
                              "CanvasHelper::fillPolyPolygon(): GDI+ call failed  " );
        }

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
        if( needOutput() )
        {
            return uno::Reference< rendering::XCanvasFont >(
                    new CanvasFont(fontRequest, extraFontProperties, fontMatrix ) );
        }

        return uno::Reference< rendering::XCanvasFont >();
    }

    uno::Sequence< rendering::FontInfo > CanvasHelper::queryAvailableFonts( const rendering::XCanvas*                       /*pCanvas*/,
                                                                            const rendering::FontInfo&                      /*aFilter*/,
                                                                            const uno::Sequence< beans::PropertyValue >&    /*aFontProperties*/ )
    {
        // TODO
        return uno::Sequence< rendering::FontInfo >();
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::drawText( const rendering::XCanvas*                         /*pCanvas*/,
                                                                          const rendering::StringContext&                   text,
                                                                          const uno::Reference< rendering::XCanvasFont >&   xFont,
                                                                          const rendering::ViewState&                       viewState,
                                                                          const rendering::RenderState&                     renderState,
                                                                          sal_Int8                                          /*textDirection*/ )
    {
        ENSURE_AND_THROW( xFont.is(),
                          "CanvasHelper::drawText: font is NULL");

        if( needOutput() )
        {
            SurfaceGraphicsSharedPtr aGraphics( mpTarget->getGraphics() );

            setupGraphicsState( aGraphics, viewState, renderState );

            Gdiplus::SolidBrush aBrush(
                Gdiplus::Color(
                    tools::sequenceToArgb(renderState.DeviceColor)));

            CanvasFont::ImplRef pFont(
                tools::canvasFontFromXFont(xFont) );

            // Move glyphs up, such that output happens at the font
            // baseline.
            Gdiplus::PointF aPoint( 0.0,
                                    static_cast<Gdiplus::REAL>(-(pFont->getFont()->GetSize()*
                                                                 pFont->getCellAscent() /
                                                                 pFont->getEmHeight())) );

            // TODO(F1): According to
            // http://support.microsoft.com/default.aspx?scid=kb;EN-US;Q307208,
            // we might have to revert to GDI and ExTextOut here,
            // since GDI+ takes the scalability a little bit too
            // far...

            // TODO(F2): Proper layout (BiDi, CTL)! IMHO must use
            // DrawDriverString here, and perform layouting myself...
            ENSURE_AND_THROW(
                Gdiplus::Ok == (*aGraphics)->DrawString( reinterpret_cast<LPCWSTR>(text.Text.copy( text.StartPosition,
                                                                         text.Length ).getStr()),
                                                         text.Length,
                                                         pFont->getFont().get(),
                                                         aPoint,
                                                         &aBrush ),
                "CanvasHelper::drawText(): GDI+ call failed" );
        }

        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::drawTextLayout( const rendering::XCanvas*                       /*pCanvas*/,
                                                                                const uno::Reference< rendering::XTextLayout >& xLayoutetText,
                                                                                const rendering::ViewState&                     viewState,
                                                                                const rendering::RenderState&                   renderState )
    {
        ENSURE_AND_THROW( xLayoutetText.is(),
                          "CanvasHelper::drawTextLayout: layout is NULL");

        if( needOutput() )
        {
            TextLayout* pTextLayout =
                dynamic_cast< TextLayout* >( xLayoutetText.get() );

            ENSURE_AND_THROW( pTextLayout,
                                "CanvasHelper::drawTextLayout(): TextLayout not compatible with this canvas" );

            pTextLayout->draw( mpTarget,
                               viewState,
                               renderState,
                               maOutputOffset,
                               mpDevice );
        }

        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::drawBitmap( const rendering::XCanvas*                   /*pCanvas*/,
                                                                            const uno::Reference< rendering::XBitmap >& xBitmap,
                                                                            const rendering::ViewState&                 viewState,
                                                                            const rendering::RenderState&               renderState )
    {
        ENSURE_AND_THROW( xBitmap.is(),
                          "CanvasHelper::drawBitmap: bitmap is NULL");

        if( needOutput() )
        {
            SurfaceGraphicsSharedPtr aGraphics( mpTarget->getGraphics() );
            setupGraphicsState( aGraphics, viewState, renderState );
            Gdiplus::PointF aPoint;
            tools::drawXBitmap( aGraphics, xBitmap );
        }

        // TODO(P1): Provide caching here.
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::drawBitmapModulated( const rendering::XCanvas*                      pCanvas,
                                                                                     const uno::Reference< rendering::XBitmap >&    xBitmap,
                                                                                     const rendering::ViewState&                    viewState,
                                                                                     const rendering::RenderState&                  renderState )
    {
        ENSURE_AND_THROW( xBitmap.is(),
                          "CanvasHelper::drawBitmap: bitmap is NULL");

        // no color set -> this is equivalent to a plain drawBitmap(), then
        if( renderState.DeviceColor.getLength() < 3 )
            return drawBitmap( pCanvas, xBitmap, viewState, renderState );

        if( needOutput() )
        {
            SurfaceGraphicsSharedPtr aGraphics( mpTarget->getGraphics() );

            setupGraphicsState( aGraphics, viewState, renderState );

            BitmapSharedPtr pBitmap( tools::bitmapFromXBitmap( xBitmap ) );
            Gdiplus::Rect aRect( 0, 0,
                                 pBitmap->GetWidth(),
                                 pBitmap->GetHeight() );

            // Setup an ImageAttributes with an alpha-modulating
            // color matrix.
            double nRed;
            double nGreen;
            double nBlue;
            double nAlpha;
            ::canvas::tools::getDeviceColor( nRed, nGreen, nBlue, nAlpha,
                                             renderState );

            Gdiplus::ImageAttributes aImgAttr;
            tools::setModulateImageAttributes( aImgAttr,
                                               nRed, nGreen, nBlue, nAlpha );

            ENSURE_AND_THROW(
                Gdiplus::Ok == (*aGraphics)->DrawImage( pBitmap.get(),
                                                        aRect,
                                                        0, 0,
                                                        pBitmap->GetWidth(),
                                                        pBitmap->GetHeight(),
                                                        Gdiplus::UnitPixel,
                                                        &aImgAttr,
                                                        NULL,
                                                        NULL ),
                "CanvasHelper::drawBitmapModulated(): GDI+ call failed" );
        }

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
        if( !mpTarget )
            geometry::IntegerSize2D(1, 1); // we're disposed

        return ::basegfx::unotools::integerSize2DFromB2ISize(mpTarget->getSize());
    }

    uno::Reference< rendering::XBitmap > CanvasHelper::getScaledBitmap( const geometry::RealSize2D& /*newSize*/,
                                                                        sal_Bool                    /*beFast*/ )
    {
        // TODO(F1):
        return uno::Reference< rendering::XBitmap >();
    }

    uno::Sequence< sal_Int8 > CanvasHelper::getData( rendering::IntegerBitmapLayout&     bitmapLayout,
                                                     const geometry::IntegerRectangle2D& rect )
    {
        RTL_LOGFILE_CONTEXT( aLog, "::dxcanvas::CanvasHelper::getData()" );

        ENSURE_AND_THROW( mpTarget,
                          "::dxcanvas::CanvasHelper::getData(): disposed" );

        if( !mpTarget )
            return uno::Sequence< sal_Int8 >();

        return mpTarget->getData(bitmapLayout,rect);
    }

    void CanvasHelper::setData( const uno::Sequence< sal_Int8 >&        data,
                                const rendering::IntegerBitmapLayout&   bitmapLayout,
                                const geometry::IntegerRectangle2D&     rect )
    {
        RTL_LOGFILE_CONTEXT( aLog, "::dxcanvas::CanvasHelper::setData()" );

        ENSURE_AND_THROW( mpTarget,
                          "::dxcanvas::CanvasHelper::setData(): disposed" );

        if( !mpTarget )
            return;

        mpTarget->setData(data,bitmapLayout,rect);
    }

    void CanvasHelper::setPixel( const uno::Sequence< sal_Int8 >&       color,
                                 const rendering::IntegerBitmapLayout&  bitmapLayout,
                                 const geometry::IntegerPoint2D&        pos )
    {
        RTL_LOGFILE_CONTEXT( aLog, "::dxcanvas::CanvasHelper::setPixel()" );

        ENSURE_AND_THROW( mpTarget,
                          "::dxcanvas::CanvasHelper::setPixel(): disposed" );

        if( !mpTarget )
            return;

        mpTarget->setPixel(color,bitmapLayout,pos);
    }

    uno::Sequence< sal_Int8 > CanvasHelper::getPixel( rendering::IntegerBitmapLayout&   bitmapLayout,
                                                      const geometry::IntegerPoint2D&   pos )
    {
        RTL_LOGFILE_CONTEXT( aLog, "::dxcanvas::CanvasHelper::getPixel()" );

        ENSURE_AND_THROW( mpTarget,
                          "::dxcanvas::CanvasHelper::getPixel(): disposed" );

        if( !mpTarget )
            return uno::Sequence< sal_Int8 >();

        return mpTarget->getPixel(bitmapLayout,pos);
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


    // private helper
    // --------------------------------------------------

    Gdiplus::CompositingMode CanvasHelper::calcCompositingMode( sal_Int8 nMode )
    {
        Gdiplus::CompositingMode aRet( Gdiplus::CompositingModeSourceOver );

        switch( nMode )
        {
            case rendering::CompositeOperation::OVER:
                // FALLTHROUGH intended
            case rendering::CompositeOperation::CLEAR:
                aRet = Gdiplus::CompositingModeSourceOver;
                break;

            case rendering::CompositeOperation::SOURCE:
                aRet = Gdiplus::CompositingModeSourceCopy;
                break;

            case rendering::CompositeOperation::DESTINATION:
                // FALLTHROUGH intended
            case rendering::CompositeOperation::UNDER:
                // FALLTHROUGH intended
            case rendering::CompositeOperation::INSIDE:
                // FALLTHROUGH intended
            case rendering::CompositeOperation::INSIDE_REVERSE:
                // FALLTHROUGH intended
            case rendering::CompositeOperation::OUTSIDE:
                // FALLTHROUGH intended
            case rendering::CompositeOperation::OUTSIDE_REVERSE:
                // FALLTHROUGH intended
            case rendering::CompositeOperation::ATOP:
                // FALLTHROUGH intended
            case rendering::CompositeOperation::ATOP_REVERSE:
                // FALLTHROUGH intended
            case rendering::CompositeOperation::XOR:
                // FALLTHROUGH intended
            case rendering::CompositeOperation::ADD:
                // FALLTHROUGH intended
            case rendering::CompositeOperation::SATURATE:
                // TODO(F2): Problem, because GDI+ only knows about two compositing modes
                aRet = Gdiplus::CompositingModeSourceOver;
                break;

            default:
                ENSURE_AND_THROW( false, "CanvasHelper::calcCompositingMode: unexpected mode" );
                break;
        }

        return aRet;
    }

    void CanvasHelper::setupGraphicsState( SurfaceGraphicsSharedPtr&        rGraphics,
                                           const rendering::ViewState&      viewState,
                                           const rendering::RenderState&    renderState )
    {
        ENSURE_AND_THROW( needOutput(),
                          "CanvasHelper::setupGraphicsState: primary graphics invalid" );
        ENSURE_AND_THROW( mpDevice,
                          "CanvasHelper::setupGraphicsState: reference device invalid" );

        // setup view transform first. Clipping e.g. depends on it
        ::basegfx::B2DHomMatrix aTransform;
        ::canvas::tools::getViewStateTransform(aTransform, viewState);

        // add output offset
        if( !maOutputOffset.equalZero() )
        {
            ::basegfx::B2DHomMatrix aOutputOffset;
            aOutputOffset.translate( maOutputOffset.getX(),
                                     maOutputOffset.getY() );

            aTransform = aOutputOffset * aTransform;
        }

        Gdiplus::Matrix aMatrix;
        tools::gdiPlusMatrixFromB2DHomMatrix( aMatrix, aTransform );

        ENSURE_AND_THROW(
            Gdiplus::Ok == (*rGraphics)->SetTransform( &aMatrix ),
            "CanvasHelper::setupGraphicsState(): Failed to set GDI+ transformation" );

        // setup view and render state clipping
        ENSURE_AND_THROW(
            Gdiplus::Ok == (*rGraphics)->ResetClip(),
            "CanvasHelper::setupGraphicsState(): Failed to reset GDI+ clip" );

        if( viewState.Clip.is() )
        {
            GraphicsPathSharedPtr aClipPath( tools::graphicsPathFromXPolyPolygon2D( viewState.Clip ) );

            // TODO(P3): Cache clip. SetClip( GraphicsPath ) performs abyssmally on GDI+.
            // Try SetClip( Rect ) or similar for simple clip paths (need some support in
            // LinePolyPolygon, then)
            ENSURE_AND_THROW(
                Gdiplus::Ok == (*rGraphics)->SetClip( aClipPath.get(),
                                                      Gdiplus::CombineModeIntersect ),
                "CanvasHelper::setupGraphicsState(): Cannot set GDI+ clip" );
        }

        // setup overall transform only now. View clip above was relative to
        // view transform
        ::canvas::tools::mergeViewAndRenderTransform(aTransform,
                                                     viewState,
                                                     renderState);

        // add output offset
        if( !maOutputOffset.equalZero() )
        {
            ::basegfx::B2DHomMatrix aOutputOffset;
            aOutputOffset.translate( maOutputOffset.getX(),
                                     maOutputOffset.getY() );

            aTransform = aOutputOffset * aTransform;
        }

        tools::gdiPlusMatrixFromB2DHomMatrix( aMatrix, aTransform );

        ENSURE_AND_THROW(
            Gdiplus::Ok == (*rGraphics)->SetTransform( &aMatrix ),
            "CanvasHelper::setupGraphicsState(): Cannot set GDI+ transformation" );

        if( renderState.Clip.is() )
        {
            GraphicsPathSharedPtr aClipPath( tools::graphicsPathFromXPolyPolygon2D( renderState.Clip ) );

            // TODO(P3): Cache clip. SetClip( GraphicsPath ) performs abyssmally on GDI+.
            // Try SetClip( Rect ) or similar for simple clip paths (need some support in
            // LinePolyPolygon, then)
            ENSURE_AND_THROW(
                Gdiplus::Ok == (*rGraphics)->SetClip( aClipPath.get(),
                                                      Gdiplus::CombineModeIntersect ),
                "CanvasHelper::setupGraphicsState(): Cannot set GDI+ clip" );
        }

        // setup compositing
        const Gdiplus::CompositingMode eCompositing( calcCompositingMode( renderState.CompositeOperation ) );
        ENSURE_AND_THROW(
            Gdiplus::Ok == (*rGraphics)->SetCompositingMode( eCompositing ),
            "CanvasHelper::setupGraphicsState(): Cannot set GDI* compositing mode)" );
    }

    void CanvasHelper::flush() const
    {
        if( needOutput() )
        {
            (*mpTarget->getGraphics())->Flush( Gdiplus::FlushIntentionSync );
        }
    }

    bool CanvasHelper::hasAlpha() const
    {
        return mpTarget ? mpTarget->hasAlpha() : false;
    }
}
