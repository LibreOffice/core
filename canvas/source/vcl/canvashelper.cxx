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

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <com/sun/star/drawing/LineCap.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/PathCapType.hpp>
#include <com/sun/star/rendering/PathJoinType.hpp>
#include <com/sun/star/rendering/TextDirection.hpp>
#include <comphelper/sequence.hxx>
#include <rtl/math.hxx>
#include <tools/diagnose_ex.h>
#include <tools/poly.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/BitmapReadAccess.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/BitmapAlphaClampFilter.hxx>
#include <vcl/skia/SkiaHelper.hxx>

#include <canvas/canvastools.hxx>

#include "canvasbitmap.hxx"
#include "canvasfont.hxx"
#include "canvashelper.hxx"
#include "impltools.hxx"
#include "textlayout.hxx"


using namespace ::com::sun::star;

namespace vclcanvas
{
    namespace
    {
        basegfx::B2DLineJoin b2DJoineFromJoin( sal_Int8 nJoinType )
        {
            switch( nJoinType )
            {
                case rendering::PathJoinType::NONE:
                    return basegfx::B2DLineJoin::NONE;

                case rendering::PathJoinType::MITER:
                    return basegfx::B2DLineJoin::Miter;

                case rendering::PathJoinType::ROUND:
                    return basegfx::B2DLineJoin::Round;

                case rendering::PathJoinType::BEVEL:
                    return basegfx::B2DLineJoin::Bevel;

                default:
                    ENSURE_OR_THROW( false,
                                      "b2DJoineFromJoin(): Unexpected join type" );
            }

            return basegfx::B2DLineJoin::NONE;
        }

        drawing::LineCap unoCapeFromCap( sal_Int8 nCapType)
        {
            switch ( nCapType)
            {
                case rendering::PathCapType::BUTT:
                    return drawing::LineCap_BUTT;

                case rendering::PathCapType::ROUND:
                    return drawing::LineCap_ROUND;

                case rendering::PathCapType::SQUARE:
                    return drawing::LineCap_SQUARE;

                default:
                    ENSURE_OR_THROW( false,
                                      "unoCapeFromCap(): Unexpected cap type" );
            }
            return drawing::LineCap_BUTT;
        }
    }

    CanvasHelper::CanvasHelper() :
        mpDevice(),
        mbHaveAlpha( false )
    {
    }

    void CanvasHelper::disposing()
    {
        mpDevice = nullptr;
        mpProtectedOutDevProvider.reset();
        mpOutDevProvider.reset();
        mp2ndOutDevProvider.reset();
    }

    void CanvasHelper::init( rendering::XGraphicDevice&     rDevice,
                             const OutDevProviderSharedPtr& rOutDev,
                             bool                           bProtect,
                             bool                           bHaveAlpha )
    {
        // cast away const, need to change refcount (as this is
        // ~invisible to client code, still logically const)
        mpDevice    = &rDevice;
        mbHaveAlpha = bHaveAlpha;

        setOutDev( rOutDev, bProtect );
    }

    void CanvasHelper::setOutDev( const OutDevProviderSharedPtr& rOutDev,
                                  bool                           bProtect )
    {
        if( bProtect )
            mpProtectedOutDevProvider = rOutDev;
        else
            mpProtectedOutDevProvider.reset();

        mpOutDevProvider = rOutDev;
    }

    void CanvasHelper::setBackgroundOutDev( const OutDevProviderSharedPtr& rOutDev )
    {
        mp2ndOutDevProvider = rOutDev;
        mp2ndOutDevProvider->getOutDev().EnableMapMode( false );
        mp2ndOutDevProvider->getOutDev().SetAntialiasing( AntialiasingFlags::Enable );
    }

    void CanvasHelper::clear()
    {
        // are we disposed?
        if( !mpOutDevProvider )
            return;

        OutputDevice& rOutDev( mpOutDevProvider->getOutDev() );
        tools::OutDevStateKeeper aStateKeeper( mpProtectedOutDevProvider );

        rOutDev.EnableMapMode( false );
        rOutDev.SetAntialiasing( AntialiasingFlags::Enable );
        rOutDev.SetLineColor( COL_WHITE );
        rOutDev.SetFillColor( COL_WHITE );
        rOutDev.SetClipRegion();
        rOutDev.DrawRect( ::tools::Rectangle( Point(),
                                     rOutDev.GetOutputSizePixel()) );

        if( !mp2ndOutDevProvider )
            return;

        OutputDevice& rOutDev2( mp2ndOutDevProvider->getOutDev() );

        rOutDev2.SetDrawMode( DrawModeFlags::Default );
        rOutDev2.EnableMapMode( false );
        rOutDev2.SetAntialiasing( AntialiasingFlags::Enable );
        rOutDev2.SetLineColor( COL_WHITE );
        rOutDev2.SetFillColor( COL_WHITE );
        rOutDev2.SetClipRegion();
        rOutDev2.DrawRect( ::tools::Rectangle( Point(),
                                      rOutDev2.GetOutputSizePixel()) );
        rOutDev2.SetDrawMode( DrawModeFlags::BlackLine | DrawModeFlags::BlackFill | DrawModeFlags::BlackText |
                              DrawModeFlags::BlackGradient | DrawModeFlags::BlackBitmap );
    }

    void CanvasHelper::drawLine( const rendering::XCanvas*      ,
                                 const geometry::RealPoint2D&   aStartRealPoint2D,
                                 const geometry::RealPoint2D&   aEndRealPoint2D,
                                 const rendering::ViewState&    viewState,
                                 const rendering::RenderState&  renderState )
    {
        // are we disposed?
        if( !mpOutDevProvider )
            return;

        // nope, render
        tools::OutDevStateKeeper aStateKeeper( mpProtectedOutDevProvider );
        setupOutDevState( viewState, renderState, LINE_COLOR );

        const Point aStartPoint( tools::mapRealPoint2D( aStartRealPoint2D,
                                                        viewState, renderState ) );
        const Point aEndPoint( tools::mapRealPoint2D( aEndRealPoint2D,
                                                      viewState, renderState ) );
        // TODO(F2): alpha
        mpOutDevProvider->getOutDev().DrawLine( aStartPoint, aEndPoint );

        if( mp2ndOutDevProvider )
            mp2ndOutDevProvider->getOutDev().DrawLine( aStartPoint, aEndPoint );
    }

    void CanvasHelper::drawBezier( const rendering::XCanvas*            ,
                                   const geometry::RealBezierSegment2D& aBezierSegment,
                                   const geometry::RealPoint2D&         _aEndPoint,
                                   const rendering::ViewState&          viewState,
                                   const rendering::RenderState&        renderState )
    {
        if( !mpOutDevProvider )
            return;

        tools::OutDevStateKeeper aStateKeeper( mpProtectedOutDevProvider );
        setupOutDevState( viewState, renderState, LINE_COLOR );

        const Point& rStartPoint( tools::mapRealPoint2D( geometry::RealPoint2D(aBezierSegment.Px,
                                                                               aBezierSegment.Py),
                                                        viewState, renderState ) );
        const Point& rCtrlPoint1( tools::mapRealPoint2D( geometry::RealPoint2D(aBezierSegment.C1x,
                                                                               aBezierSegment.C1y),
                                                        viewState, renderState ) );
        const Point& rCtrlPoint2( tools::mapRealPoint2D( geometry::RealPoint2D(aBezierSegment.C2x,
                                                                               aBezierSegment.C2y),
                                                         viewState, renderState ) );
        const Point& rEndPoint( tools::mapRealPoint2D( _aEndPoint,
                                                       viewState, renderState ) );

        ::tools::Polygon aPoly(4);
        aPoly.SetPoint( rStartPoint, 0 );
        aPoly.SetFlags( 0, PolyFlags::Normal );
        aPoly.SetPoint( rCtrlPoint1, 1 );
        aPoly.SetFlags( 1, PolyFlags::Control );
        aPoly.SetPoint( rCtrlPoint2, 2 );
        aPoly.SetFlags( 2, PolyFlags::Control );
        aPoly.SetPoint( rEndPoint, 3 );
        aPoly.SetFlags( 3, PolyFlags::Normal );

        // TODO(F2): alpha
        mpOutDevProvider->getOutDev().DrawPolygon( aPoly );
        if( mp2ndOutDevProvider )
            mp2ndOutDevProvider->getOutDev().DrawPolygon( aPoly );
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::drawPolyPolygon( const rendering::XCanvas*                          ,
                                                                                 const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon,
                                                                                 const rendering::ViewState&                        viewState,
                                                                                 const rendering::RenderState&                      renderState )
    {
        ENSURE_ARG_OR_THROW( xPolyPolygon.is(),
                         "polygon is NULL");

        if( mpOutDevProvider )
        {
            tools::OutDevStateKeeper aStateKeeper( mpProtectedOutDevProvider );
            setupOutDevState( viewState, renderState, LINE_COLOR );

            const ::basegfx::B2DPolyPolygon& rPolyPoly(
                ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(xPolyPolygon) );
            const ::tools::PolyPolygon aPolyPoly( tools::mapPolyPolygon( rPolyPoly, viewState, renderState ) );

            if( rPolyPoly.isClosed() )
            {
                mpOutDevProvider->getOutDev().DrawPolyPolygon( aPolyPoly );

                if( mp2ndOutDevProvider )
                    mp2ndOutDevProvider->getOutDev().DrawPolyPolygon( aPolyPoly );
            }
            else
            {
                // mixed open/closed state. Cannot render open polygon
                // via DrawPolyPolygon(), since that implicitly
                // closed every polygon. OTOH, no need to distinguish
                // further and render closed polygons via
                // DrawPolygon(), and open ones via DrawPolyLine():
                // closed polygons will simply already contain the
                // closing segment.
                sal_uInt16 nSize( aPolyPoly.Count() );

                for( sal_uInt16 i=0; i<nSize; ++i )
                {
                    mpOutDevProvider->getOutDev().DrawPolyLine( aPolyPoly[i] );

                    if( mp2ndOutDevProvider )
                        mp2ndOutDevProvider->getOutDev().DrawPolyLine( aPolyPoly[i] );
                }
            }
        }

        // TODO(P1): Provide caching here.
        return uno::Reference< rendering::XCachedPrimitive >(nullptr);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::strokePolyPolygon( const rendering::XCanvas*                            ,
                                                                                   const uno::Reference< rendering::XPolyPolygon2D >&   xPolyPolygon,
                                                                                   const rendering::ViewState&                          viewState,
                                                                                   const rendering::RenderState&                        renderState,
                                                                                   const rendering::StrokeAttributes&                   strokeAttributes )
    {
        ENSURE_ARG_OR_THROW( xPolyPolygon.is(),
                         "polygon is NULL");

        if( mpOutDevProvider )
        {
            tools::OutDevStateKeeper aStateKeeper( mpProtectedOutDevProvider );

            ::basegfx::B2DHomMatrix aMatrix;
            ::canvas::tools::mergeViewAndRenderTransform(aMatrix, viewState, renderState);

            ::basegfx::B2DPolyPolygon aPolyPoly(
                ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(xPolyPolygon) );

            std::vector<double> aDashArray;
            if( strokeAttributes.DashArray.hasElements() )
                aDashArray = ::comphelper::sequenceToContainer< std::vector<double> >(strokeAttributes.DashArray);

            // First try to draw directly using VCL.
            bool directFailed = false;
            setupOutDevState( viewState, renderState, LINE_COLOR );
            for( sal_uInt32 i=0; i<aPolyPoly.count(); ++i )
            {
                if( !mpOutDevProvider->getOutDev().DrawPolyLineDirect( aMatrix, aPolyPoly.getB2DPolygon(i),
                    strokeAttributes.StrokeWidth, 0, !aDashArray.empty() ? &aDashArray : nullptr,
                    b2DJoineFromJoin(strokeAttributes.JoinType), unoCapeFromCap(strokeAttributes.StartCapType)))
                {
                    directFailed = true;
                    break;
                }
            }
            if(!directFailed)
                return uno::Reference< rendering::XCachedPrimitive >(nullptr);

            // Do it all manually.

            // apply dashing, if any
            if( strokeAttributes.DashArray.hasElements() )
            {
                ::basegfx::B2DPolyPolygon aDashedPolyPoly;

                for( sal_uInt32 i=0; i<aPolyPoly.count(); ++i )
                {
                    // AW: new interface; You may also get gaps in the same run now
                    basegfx::utils::applyLineDashing(aPolyPoly.getB2DPolygon(i), aDashArray, &aDashedPolyPoly);
                    //aDashedPolyPoly.append(
                    //    ::basegfx::utils::applyLineDashing( aPolyPoly.getB2DPolygon(i),
                    //                                        aDashArray ) );
                }

                aPolyPoly = aDashedPolyPoly;
            }

            ::basegfx::B2DSize aLinePixelSize(strokeAttributes.StrokeWidth,
                                              strokeAttributes.StrokeWidth);
            aLinePixelSize *= aMatrix;
            ::basegfx::B2DPolyPolygon aStrokedPolyPoly;
            if( aLinePixelSize.getLength() < 1.42 )
            {
                // line width < 1.0 in device pixel, thus, output as a
                // simple hairline poly-polygon
                setupOutDevState( viewState, renderState, LINE_COLOR );

                aStrokedPolyPoly = aPolyPoly;
            }
            else
            {
                // render as a 'thick' line
                setupOutDevState( viewState, renderState, FILL_COLOR );

                for( sal_uInt32 i=0; i<aPolyPoly.count(); ++i )
                {
                    double fMiterMinimumAngle;
                    if (strokeAttributes.MiterLimit <= 1.0)
                    {
                        fMiterMinimumAngle = M_PI_2;
                    }
                    else
                    {
                        fMiterMinimumAngle = 2.0 * asin(1.0/strokeAttributes.MiterLimit);
                    }

                    // TODO(F2): Also use Cap settings from
                    // StrokeAttributes, the
                    // createAreaGeometryForLineStartEnd() method does not
                    // seem to fit very well here

                    // AW: New interface, will create bezier polygons now
                    aStrokedPolyPoly.append(basegfx::utils::createAreaGeometry(
                        aPolyPoly.getB2DPolygon(i),
                        strokeAttributes.StrokeWidth*0.5,
                        b2DJoineFromJoin(strokeAttributes.JoinType),
                        unoCapeFromCap(strokeAttributes.StartCapType),
                        basegfx::deg2rad(12.5) /* default fMaxAllowedAngle*/ ,
                        0.4 /* default fMaxPartOfEdge*/ ,
                        fMiterMinimumAngle
                        ));
                    //aStrokedPolyPoly.append(
                    //    ::basegfx::utils::createAreaGeometryForPolygon( aPolyPoly.getB2DPolygon(i),
                    //                                                    strokeAttributes.StrokeWidth*0.5,
                    //                                                    b2DJoineFromJoin(strokeAttributes.JoinType) ) );
                }
            }

            // transform only _now_, all the StrokeAttributes are in
            // user coordinates.
            aStrokedPolyPoly.transform( aMatrix );

            // TODO(F2): When using alpha here, must handle that via
            // temporary surface or somesuch.

            // Note: the generated stroke poly-polygon is NOT free of
            // self-intersections. Therefore, if we would render it
            // via OutDev::DrawPolyPolygon(), on/off fill would
            // generate off areas on those self-intersections.
            for( sal_uInt32 i=0; i<aStrokedPolyPoly.count(); ++i )
            {
                const basegfx::B2DPolygon& polygon = aStrokedPolyPoly.getB2DPolygon( i );
                if( polygon.isClosed()) {
                    mpOutDevProvider->getOutDev().DrawPolygon( polygon );
                    if( mp2ndOutDevProvider )
                        mp2ndOutDevProvider->getOutDev().DrawPolygon( polygon );
                } else {
                    mpOutDevProvider->getOutDev().DrawPolyLine( polygon );
                    if( mp2ndOutDevProvider )
                        mp2ndOutDevProvider->getOutDev().DrawPolyLine( polygon );
                }
            }
        }

        // TODO(P1): Provide caching here.
        return uno::Reference< rendering::XCachedPrimitive >(nullptr);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::strokeTexturedPolyPolygon( const rendering::XCanvas*                            ,
                                                                                           const uno::Reference< rendering::XPolyPolygon2D >&   ,
                                                                                           const rendering::ViewState&                          ,
                                                                                           const rendering::RenderState&                        ,
                                                                                           const uno::Sequence< rendering::Texture >&           ,
                                                                                           const rendering::StrokeAttributes&                    )
    {
        return uno::Reference< rendering::XCachedPrimitive >(nullptr);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::strokeTextureMappedPolyPolygon( const rendering::XCanvas*                           ,
                                                                                                const uno::Reference< rendering::XPolyPolygon2D >&  ,
                                                                                                const rendering::ViewState&                         ,
                                                                                                const rendering::RenderState&                       ,
                                                                                                const uno::Sequence< rendering::Texture >&          ,
                                                                                                const uno::Reference< geometry::XMapping2D >&       ,
                                                                                                const rendering::StrokeAttributes&                   )
    {
        return uno::Reference< rendering::XCachedPrimitive >(nullptr);
    }

    uno::Reference< rendering::XPolyPolygon2D >   CanvasHelper::queryStrokeShapes( const rendering::XCanvas*                            ,
                                                                                   const uno::Reference< rendering::XPolyPolygon2D >&   ,
                                                                                   const rendering::ViewState&                          ,
                                                                                   const rendering::RenderState&                        ,
                                                                                   const rendering::StrokeAttributes&                    )
    {
        return uno::Reference< rendering::XPolyPolygon2D >(nullptr);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::fillPolyPolygon( const rendering::XCanvas*                          ,
                                                                                 const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon,
                                                                                 const rendering::ViewState&                        viewState,
                                                                                 const rendering::RenderState&                      renderState )
    {
        ENSURE_ARG_OR_THROW( xPolyPolygon.is(),
                         "polygon is NULL");

        if( mpOutDevProvider )
        {
            tools::OutDevStateKeeper aStateKeeper( mpProtectedOutDevProvider );

            const int nAlpha( setupOutDevState( viewState, renderState, FILL_COLOR ) );
            ::basegfx::B2DPolyPolygon aB2DPolyPoly(
                ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(xPolyPolygon));
            aB2DPolyPoly.setClosed(true); // ensure closed poly, otherwise VCL does not fill
            const ::tools::PolyPolygon aPolyPoly( tools::mapPolyPolygon(
                                             aB2DPolyPoly,
                                             viewState, renderState ) );
            const bool bSourceAlpha( renderState.CompositeOperation == rendering::CompositeOperation::SOURCE );
            if( nAlpha == 255 || bSourceAlpha )
            {
                mpOutDevProvider->getOutDev().DrawPolyPolygon( aPolyPoly );
            }
            else
            {
                const int nTransPercent( ((255 - nAlpha) * 100 + 128) / 255 );  // normal rounding, no truncation here
                mpOutDevProvider->getOutDev().DrawTransparent( aPolyPoly, static_cast<sal_uInt16>(nTransPercent) );
            }

            if( mp2ndOutDevProvider )
            {
                // HACK. Normally, CanvasHelper does not care about
                // actually what mp2ndOutDev is...  well, here we do &
                // assume a 1bpp target - everything beyond 97%
                // transparency is fully transparent
                if( nAlpha > 2 )
                {
                    mp2ndOutDevProvider->getOutDev().SetFillColor( COL_BLACK );
                    mp2ndOutDevProvider->getOutDev().DrawPolyPolygon( aPolyPoly );
                }
            }
        }

        // TODO(P1): Provide caching here.
        return uno::Reference< rendering::XCachedPrimitive >(nullptr);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::fillTextureMappedPolyPolygon( const rendering::XCanvas*                             ,
                                                                                              const uno::Reference< rendering::XPolyPolygon2D >&    ,
                                                                                              const rendering::ViewState&                           ,
                                                                                              const rendering::RenderState&                         ,
                                                                                              const uno::Sequence< rendering::Texture >&            ,
                                                                                              const uno::Reference< geometry::XMapping2D >&              )
    {
        return uno::Reference< rendering::XCachedPrimitive >(nullptr);
    }

    uno::Reference< rendering::XCanvasFont > CanvasHelper::createFont( const rendering::XCanvas*                        ,
                                                                       const rendering::FontRequest&                    fontRequest,
                                                                       const uno::Sequence< beans::PropertyValue >&     extraFontProperties,
                                                                       const geometry::Matrix2D&                        fontMatrix )
    {
        if( mpOutDevProvider && mpDevice )
        {
            // TODO(F2): font properties and font matrix
            return uno::Reference< rendering::XCanvasFont >(
                    new CanvasFont(fontRequest, extraFontProperties, fontMatrix,
                                   *mpDevice, mpOutDevProvider) );
        }

        return uno::Reference< rendering::XCanvasFont >();
    }

    uno::Sequence< rendering::FontInfo > CanvasHelper::queryAvailableFonts( const rendering::XCanvas*                       ,
                                                                            const rendering::FontInfo&                      ,
                                                                            const uno::Sequence< beans::PropertyValue >&     )
    {
        // TODO(F2)
        return uno::Sequence< rendering::FontInfo >();
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::drawText( const rendering::XCanvas*                         ,
                                                                          const rendering::StringContext&                   text,
                                                                          const uno::Reference< rendering::XCanvasFont >&   xFont,
                                                                          const rendering::ViewState&                       viewState,
                                                                          const rendering::RenderState&                     renderState,
                                                                          sal_Int8                                          textDirection )
    {
        ENSURE_ARG_OR_THROW( xFont.is(),
                         "font is NULL");

        if( mpOutDevProvider )
        {
            tools::OutDevStateKeeper aStateKeeper( mpProtectedOutDevProvider );

            ::Point aOutpos;
            if( !setupTextOutput( aOutpos, viewState, renderState, xFont ) )
                return uno::Reference< rendering::XCachedPrimitive >(nullptr); // no output necessary

            // change text direction and layout mode
            vcl::text::ComplexTextLayoutFlags nLayoutMode(vcl::text::ComplexTextLayoutFlags::Default);
            switch( textDirection )
            {
                case rendering::TextDirection::WEAK_LEFT_TO_RIGHT:
                case rendering::TextDirection::STRONG_LEFT_TO_RIGHT:
                    nLayoutMode |= vcl::text::ComplexTextLayoutFlags::BiDiStrong;
                    nLayoutMode |= vcl::text::ComplexTextLayoutFlags::TextOriginLeft;
                    break;

                case rendering::TextDirection::WEAK_RIGHT_TO_LEFT:
                    nLayoutMode |= vcl::text::ComplexTextLayoutFlags::BiDiRtl;
                    [[fallthrough]];
                case rendering::TextDirection::STRONG_RIGHT_TO_LEFT:
                    nLayoutMode |= vcl::text::ComplexTextLayoutFlags::BiDiRtl | vcl::text::ComplexTextLayoutFlags::BiDiStrong;
                    nLayoutMode |= vcl::text::ComplexTextLayoutFlags::TextOriginRight;
                    break;
            }

            // TODO(F2): alpha
            mpOutDevProvider->getOutDev().SetLayoutMode( nLayoutMode );
            mpOutDevProvider->getOutDev().DrawText( aOutpos,
                                            text.Text,
                                            ::canvas::tools::numeric_cast<sal_uInt16>(text.StartPosition),
                                            ::canvas::tools::numeric_cast<sal_uInt16>(text.Length) );

            if( mp2ndOutDevProvider )
            {
                mp2ndOutDevProvider->getOutDev().SetLayoutMode( nLayoutMode );
                mp2ndOutDevProvider->getOutDev().DrawText( aOutpos,
                                                   text.Text,
                                                   ::canvas::tools::numeric_cast<sal_uInt16>(text.StartPosition),
                                                   ::canvas::tools::numeric_cast<sal_uInt16>(text.Length) );
            }
        }

        return uno::Reference< rendering::XCachedPrimitive >(nullptr);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::drawTextLayout( const rendering::XCanvas*                       ,
                                                                                const uno::Reference< rendering::XTextLayout >& xLayoutedText,
                                                                                const rendering::ViewState&                     viewState,
                                                                                const rendering::RenderState&                   renderState )
    {
        ENSURE_ARG_OR_THROW( xLayoutedText.is(),
                         "layout is NULL");

        TextLayout* pTextLayout = dynamic_cast< TextLayout* >( xLayoutedText.get() );

        if( pTextLayout )
        {
            if( mpOutDevProvider )
            {
                tools::OutDevStateKeeper aStateKeeper( mpProtectedOutDevProvider );

                // TODO(T3): Race condition. We're taking the font
                // from xLayoutedText, and then calling draw() at it,
                // without exclusive access. Move setupTextOutput(),
                // e.g. to impltools?

                ::Point aOutpos;
                if( !setupTextOutput( aOutpos, viewState, renderState, xLayoutedText->getFont() ) )
                    return uno::Reference< rendering::XCachedPrimitive >(nullptr); // no output necessary

                // TODO(F2): What about the offset scalings?
                // TODO(F2): alpha
                pTextLayout->draw( mpOutDevProvider->getOutDev(), aOutpos, viewState, renderState );

                if( mp2ndOutDevProvider )
                    pTextLayout->draw( mp2ndOutDevProvider->getOutDev(), aOutpos, viewState, renderState );
            }
        }
        else
        {
            ENSURE_ARG_OR_THROW( false,
                                 "TextLayout not compatible with this canvas" );
        }

        return uno::Reference< rendering::XCachedPrimitive >(nullptr);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::implDrawBitmap( const rendering::XCanvas*                   pCanvas,
                                                                                const uno::Reference< rendering::XBitmap >& xBitmap,
                                                                                const rendering::ViewState&                 viewState,
                                                                                const rendering::RenderState&               renderState,
                                                                                bool                                        bModulateColors )
    {
        ENSURE_ARG_OR_THROW( xBitmap.is(),
                             "bitmap is NULL");

        ::canvas::tools::verifyInput( renderState,
                                      __func__,
                                      mpDevice,
                                      4,
                                      bModulateColors ? 3 : 0 );

        if( mpOutDevProvider )
        {
            tools::OutDevStateKeeper aStateKeeper( mpProtectedOutDevProvider );
            setupOutDevState( viewState, renderState, IGNORE_COLOR );

            ::basegfx::B2DHomMatrix aMatrix;
            ::canvas::tools::mergeViewAndRenderTransform(aMatrix, viewState, renderState);

            ::basegfx::B2DPoint aOutputPos( 0.0, 0.0 );
            aOutputPos *= aMatrix;

            BitmapEx aBmpEx( tools::bitmapExFromXBitmap(xBitmap) );

            // TODO(F2): Implement modulation again for other color
            // channels (currently, works only for alpha). Note: this
            // is already implemented in transformBitmap()
            if( bModulateColors &&
                renderState.DeviceColor.getLength() > 3 )
            {
                // optimize away the case where alpha modulation value
                // is 1.0 - we then simply switch off modulation at all
                bModulateColors = !::rtl::math::approxEqual(
                    renderState.DeviceColor[3], 1.0);
            }

            // check whether we can render bitmap as-is: must not
            // modulate colors, matrix must either be the identity
            // transform (that's clear), _or_ contain only
            // translational components.
            if( !bModulateColors &&
                (aMatrix.isIdentity() ||
                 (::basegfx::fTools::equalZero( aMatrix.get(0,1) ) &&
                  ::basegfx::fTools::equalZero( aMatrix.get(1,0) ) &&
                  ::rtl::math::approxEqual(aMatrix.get(0,0), 1.0) &&
                  ::rtl::math::approxEqual(aMatrix.get(1,1), 1.0)) ) )
            {
                // optimized case: identity matrix, or only
                // translational components.
                mpOutDevProvider->getOutDev().DrawBitmapEx( vcl::unotools::pointFromB2DPoint( aOutputPos ),
                                                    aBmpEx );

                if( mp2ndOutDevProvider )
                {
                    // HACK. Normally, CanvasHelper does not care about
                    // actually what mp2ndOutDev is...  well, here we do &
                    // assume a 1bpp target - everything beyond 97%
                    // transparency is fully transparent
                    if( aBmpEx.IsAlpha() && !SkiaHelper::isVCLSkiaEnabled())
                    {
                        BitmapFilter::Filter(aBmpEx, BitmapAlphaClampFilter(253));
                    }

                    mp2ndOutDevProvider->getOutDev().DrawBitmapEx( vcl::unotools::pointFromB2DPoint( aOutputPos ),
                                                           aBmpEx );
                }

                // Returning a cache object is not useful, the XBitmap
                // itself serves this purpose
                return uno::Reference< rendering::XCachedPrimitive >(nullptr);
            }
            else if( mpOutDevProvider->getOutDev().HasFastDrawTransformedBitmap())
            {
                ::basegfx::B2DHomMatrix aSizeTransform;
                aSizeTransform.scale( aBmpEx.GetSizePixel().Width(), aBmpEx.GetSizePixel().Height() );
                aMatrix = aMatrix * aSizeTransform;
                const double fAlpha = bModulateColors ? renderState.DeviceColor[3] : 1.0;

                mpOutDevProvider->getOutDev().DrawTransformedBitmapEx( aMatrix, aBmpEx, fAlpha );
                if( mp2ndOutDevProvider )
                {
                    // HACK. Normally, CanvasHelper does not care about
                    // actually what mp2ndOutDev is...  well, here we do &
                    // assume a 1bpp target - everything beyond 97%
                    // transparency is fully transparent
                    if( aBmpEx.IsAlpha() && !SkiaHelper::isVCLSkiaEnabled())
                    {
                        BitmapFilter::Filter(aBmpEx, BitmapAlphaClampFilter(253));
                    }

                    mp2ndOutDevProvider->getOutDev().DrawTransformedBitmapEx( aMatrix, aBmpEx );
                }
                return uno::Reference< rendering::XCachedPrimitive >(nullptr);
            }
            else
            {
                // Matrix contains non-trivial transformation (or
                // color modulation is requested), decompose to check
                // whether GraphicObject suffices
                ::basegfx::B2DVector aScale;
                double               nRotate;
                double               nShearX;
                aMatrix.decompose( aScale, aOutputPos, nRotate, nShearX );

                GraphicAttr             aGrfAttr;
                GraphicObjectSharedPtr  pGrfObj;

                ::Size aBmpSize( aBmpEx.GetSizePixel() );

                // setup alpha modulation
                if( bModulateColors )
                {
                    const double nAlphaModulation( renderState.DeviceColor[3] );

                    // TODO(F1): Note that the GraphicManager has a
                    // subtle difference in how it calculates the
                    // resulting alpha value: it's using the inverse
                    // alpha values (i.e. 'transparency'), and
                    // calculates transOrig + transModulate, instead
                    // of transOrig + transModulate -
                    // transOrig*transModulate (which would be
                    // equivalent to the origAlpha*modulateAlpha the
                    // DX canvas performs)
                    aGrfAttr.SetAlpha(
                        static_cast< sal_uInt8 >(
                            ::basegfx::fround( 255.0 * nAlphaModulation ) ) );
                }

                if( ::basegfx::fTools::equalZero( nShearX ) )
                {
                    // no shear, GraphicObject is enough (the
                    // GraphicObject only supports scaling, rotation
                    // and translation)

                    // #i75339# don't apply mirror flags, having
                    // negative size values is enough to make
                    // GraphicObject flip the bitmap

                    // The angle has to be mapped from radian to tenths of
                    // degrees with the orientation reversed: [0,2Pi) ->
                    // (3600,0].  Note that the original angle may have
                    // values outside the [0,2Pi) interval.
                    const double nAngleInTenthOfDegrees (3600.0 - basegfx::rad2deg<10>(nRotate));
                    aGrfAttr.SetRotation( Degree10(::basegfx::fround(nAngleInTenthOfDegrees)) );

                    pGrfObj = std::make_shared<GraphicObject>( aBmpEx );
                }
                else
                {
                    // modify output position, to account for the fact
                    // that transformBitmap() always normalizes its output
                    // bitmap into the smallest enclosing box.
                    ::basegfx::B2DRectangle aDestRect;
                    ::canvas::tools::calcTransformedRectBounds( aDestRect,
                                                                ::basegfx::B2DRectangle(0,
                                                                                        0,
                                                                                        aBmpSize.Width(),
                                                                                        aBmpSize.Height()),
                                                                aMatrix );

                    aOutputPos.setX( aDestRect.getMinX() );
                    aOutputPos.setY( aDestRect.getMinY() );

                    // complex transformation, use generic affine bitmap
                    // transformation
                    aBmpEx = tools::transformBitmap( aBmpEx,
                                                     aMatrix );

                    pGrfObj = std::make_shared<GraphicObject>( aBmpEx );

                    // clear scale values, generated bitmap already
                    // contains scaling
                    aScale.setX( 1.0 ); aScale.setY( 1.0 );

                    // update bitmap size, bitmap has changed above.
                    aBmpSize = aBmpEx.GetSizePixel();
                }

                // output GraphicObject
                const ::Point aPt( vcl::unotools::pointFromB2DPoint( aOutputPos ) );
                const ::Size  aSz( ::basegfx::fround( aScale.getX() * aBmpSize.Width() ),
                                   ::basegfx::fround( aScale.getY() * aBmpSize.Height() ) );

                pGrfObj->Draw(mpOutDevProvider->getOutDev(),
                              aPt,
                              aSz,
                              &aGrfAttr);

                if( mp2ndOutDevProvider )
                    pGrfObj->Draw(mp2ndOutDevProvider->getOutDev(),
                                  aPt,
                                  aSz,
                                  &aGrfAttr);

                // created GraphicObject, which possibly cached
                // display bitmap - return cache object, to retain
                // that information.
                return uno::Reference< rendering::XCachedPrimitive >(
                    new CachedBitmap( pGrfObj,
                                      aPt,
                                      aSz,
                                      aGrfAttr,
                                      viewState,
                                      renderState,
                                      // cast away const, need to
                                      // change refcount (as this is
                                      // ~invisible to client code,
                                      // still logically const)
                                      const_cast< rendering::XCanvas* >(pCanvas)) );
            }
        }

        // Nothing rendered
        return uno::Reference< rendering::XCachedPrimitive >(nullptr);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::drawBitmap( const rendering::XCanvas*                   pCanvas,
                                                                            const uno::Reference< rendering::XBitmap >& xBitmap,
                                                                            const rendering::ViewState&                 viewState,
                                                                            const rendering::RenderState&               renderState )
    {
        return implDrawBitmap( pCanvas,
                               xBitmap,
                               viewState,
                               renderState,
                               false );
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::drawBitmapModulated( const rendering::XCanvas*                      pCanvas,
                                                                                     const uno::Reference< rendering::XBitmap >&    xBitmap,
                                                                                     const rendering::ViewState&                    viewState,
                                                                                     const rendering::RenderState&                  renderState )
    {
        return implDrawBitmap( pCanvas,
                               xBitmap,
                               viewState,
                               renderState,
                               true );
    }

    geometry::IntegerSize2D CanvasHelper::getSize()
    {
        if( !mpOutDevProvider )
            return geometry::IntegerSize2D(); // we're disposed

        return vcl::unotools::integerSize2DFromSize( mpOutDevProvider->getOutDev().GetOutputSizePixel() );
    }

    uno::Reference< rendering::XBitmap > CanvasHelper::getScaledBitmap( const geometry::RealSize2D& newSize,
                                                                        bool                        beFast )
    {
        if( !mpOutDevProvider || !mpDevice )
            return uno::Reference< rendering::XBitmap >(); // we're disposed

        OutputDevice& rOutDev( mpOutDevProvider->getOutDev() );

        tools::OutDevStateKeeper aStateKeeper( mpProtectedOutDevProvider );
        rOutDev.EnableMapMode( false );
        rOutDev.SetAntialiasing( AntialiasingFlags::Enable );

        // TODO(F2): Support alpha vdev canvas here
        const Point aEmptyPoint(0,0);
        const Size  aBmpSize( rOutDev.GetOutputSizePixel() );

        BitmapEx aBitmap( rOutDev.GetBitmapEx(aEmptyPoint, aBmpSize) );

        aBitmap.Scale( vcl::unotools::sizeFromRealSize2D(newSize),
                       beFast ? BmpScaleFlag::Default : BmpScaleFlag::BestQuality );

        return uno::Reference< rendering::XBitmap >(
            new CanvasBitmap( aBitmap, *mpDevice, mpOutDevProvider ) );
    }

    uno::Sequence< sal_Int8 > CanvasHelper::getData( rendering::IntegerBitmapLayout&     rLayout,
                                                     const geometry::IntegerRectangle2D& rect )
    {
        if( !mpOutDevProvider )
            return uno::Sequence< sal_Int8 >(); // we're disposed

        rLayout = getMemoryLayout();

        // TODO(F2): Support alpha canvas here
        const ::tools::Rectangle aRect( vcl::unotools::rectangleFromIntegerRectangle2D(rect) );

        OutputDevice& rOutDev( mpOutDevProvider->getOutDev() );

        tools::OutDevStateKeeper aStateKeeper( mpProtectedOutDevProvider );
        rOutDev.EnableMapMode( false );
        rOutDev.SetAntialiasing( AntialiasingFlags::Enable );

        Bitmap aBitmap( rOutDev.GetBitmapEx(aRect.TopLeft(),
                                          aRect.GetSize()).GetBitmap() );

        Bitmap::ScopedReadAccess pReadAccess( aBitmap );

        ENSURE_OR_THROW( pReadAccess.get() != nullptr,
                         "Could not acquire read access to OutDev bitmap" );

        const sal_Int32 nWidth( rect.X2 - rect.X1 );
        const sal_Int32 nHeight( rect.Y2 - rect.Y1 );

        rLayout.ScanLines = nHeight;
        rLayout.ScanLineBytes = nWidth*4;
        rLayout.ScanLineStride = rLayout.ScanLineBytes;

        uno::Sequence< sal_Int8 > aRes( 4*nWidth*nHeight );
        sal_Int8* pRes = aRes.getArray();

        int nCurrPos(0);
        for( int y=0; y<nHeight; ++y )
        {
            for( int x=0; x<nWidth; ++x )
            {
                pRes[ nCurrPos++ ] = pReadAccess->GetColor( y, x ).GetRed();
                pRes[ nCurrPos++ ] = pReadAccess->GetColor( y, x ).GetGreen();
                pRes[ nCurrPos++ ] = pReadAccess->GetColor( y, x ).GetBlue();
                pRes[ nCurrPos++ ] = -1;
            }
        }

        return aRes;
    }

    uno::Sequence< sal_Int8 > CanvasHelper::getPixel( rendering::IntegerBitmapLayout& rLayout,
                                                      const geometry::IntegerPoint2D& pos )
    {
        if( !mpOutDevProvider )
            return uno::Sequence< sal_Int8 >(); // we're disposed

        rLayout = getMemoryLayout();
        rLayout.ScanLines = 1;
        rLayout.ScanLineBytes = 4;
        rLayout.ScanLineStride = rLayout.ScanLineBytes;

        OutputDevice& rOutDev( mpOutDevProvider->getOutDev() );

        tools::OutDevStateKeeper aStateKeeper( mpProtectedOutDevProvider );
        rOutDev.EnableMapMode( false );
        rOutDev.SetAntialiasing( AntialiasingFlags::Enable );

        const Size aBmpSize( rOutDev.GetOutputSizePixel() );

        ENSURE_ARG_OR_THROW( pos.X >= 0 && pos.X < aBmpSize.Width(),
                             "X coordinate out of bounds" );
        ENSURE_ARG_OR_THROW( pos.Y >= 0 && pos.Y < aBmpSize.Height(),
                             "Y coordinate out of bounds" );

        // TODO(F2): Support alpha canvas here
        return ::canvas::tools::colorToStdIntSequence(
            rOutDev.GetPixel(
                vcl::unotools::pointFromIntegerPoint2D( pos )));
    }

    rendering::IntegerBitmapLayout CanvasHelper::getMemoryLayout()
    {
        if( !mpOutDevProvider )
            return rendering::IntegerBitmapLayout(); // we're disposed

        rendering::IntegerBitmapLayout aBitmapLayout( ::canvas::tools::getStdMemoryLayout(getSize()) );
        if ( !mbHaveAlpha )
            aBitmapLayout.ColorSpace = canvas::tools::getStdColorSpaceWithoutAlpha();

        return aBitmapLayout;
    }

    int CanvasHelper::setupOutDevState( const rendering::ViewState&     viewState,
                                        const rendering::RenderState&   renderState,
                                        ColorType                       eColorType ) const
    {
        ENSURE_OR_THROW( mpOutDevProvider,
                         "outdev null. Are we disposed?" );

        ::canvas::tools::verifyInput( renderState,
                                      __func__,
                                      mpDevice,
                                      2,
                                      eColorType == IGNORE_COLOR ? 0 : 3 );

        OutputDevice& rOutDev( mpOutDevProvider->getOutDev() );
        OutputDevice* p2ndOutDev = nullptr;

        rOutDev.EnableMapMode( false );
        rOutDev.SetAntialiasing( AntialiasingFlags::Enable );

        if( mp2ndOutDevProvider )
            p2ndOutDev = &mp2ndOutDevProvider->getOutDev();

        int nAlpha(255);

        // TODO(P2): Don't change clipping all the time, maintain current clip
        // state and change only when update is necessary
        ::canvas::tools::clipOutDev(viewState, renderState, rOutDev, p2ndOutDev);

        Color aColor( COL_WHITE );

        if( renderState.DeviceColor.getLength() > 2 )
        {
            aColor = vcl::unotools::stdColorSpaceSequenceToColor(
                renderState.DeviceColor );
        }

        // extract alpha, and make color opaque
        // afterwards. Otherwise, OutputDevice won't draw anything
        nAlpha = aColor.GetAlpha();
        aColor.SetAlpha(255);

        if( eColorType != IGNORE_COLOR )
        {
            switch( eColorType )
            {
                case LINE_COLOR:
                    rOutDev.SetLineColor( aColor );
                    rOutDev.SetFillColor();

                    if( p2ndOutDev )
                    {
                        p2ndOutDev->SetLineColor( aColor );
                        p2ndOutDev->SetFillColor();
                    }
                    break;

                case FILL_COLOR:
                    rOutDev.SetFillColor( aColor );
                    rOutDev.SetLineColor();

                    if( p2ndOutDev )
                    {
                        p2ndOutDev->SetFillColor( aColor );
                        p2ndOutDev->SetLineColor();
                    }
                    break;

                case TEXT_COLOR:
                    rOutDev.SetTextColor( aColor );

                    if( p2ndOutDev )
                        p2ndOutDev->SetTextColor( aColor );
                    break;

                default:
                    ENSURE_OR_THROW( false,
                                     "Unexpected color type");
                    break;
            }
        }

        return nAlpha;
    }

    bool CanvasHelper::setupTextOutput( ::Point&                                        o_rOutPos,
                                        const rendering::ViewState&                     viewState,
                                        const rendering::RenderState&                   renderState,
                                        const uno::Reference< rendering::XCanvasFont >& xFont   ) const
    {
        ENSURE_OR_THROW( mpOutDevProvider,
                         "outdev null. Are we disposed?" );

        OutputDevice& rOutDev( mpOutDevProvider->getOutDev() );

        setupOutDevState( viewState, renderState, TEXT_COLOR );

        CanvasFont* pFont = dynamic_cast< CanvasFont* >( xFont.get() );

        ENSURE_ARG_OR_THROW( pFont,
                             "Font not compatible with this canvas" );

        vcl::Font aVCLFont = pFont->getVCLFont();

        Color aColor( COL_BLACK );

        if( renderState.DeviceColor.getLength() > 2 )
        {
            aColor = vcl::unotools::stdColorSpaceSequenceToColor(
                renderState.DeviceColor );
        }

        // setup font color
        aVCLFont.SetColor( aColor );
        aVCLFont.SetFillColor( aColor );

        // no need to replicate this for mp2ndOutDev, we're modifying only aVCLFont here.
        if( !tools::setupFontTransform( o_rOutPos, aVCLFont, viewState, renderState, rOutDev ) )
            return false;

        rOutDev.SetFont( aVCLFont );

        if( mp2ndOutDevProvider )
            mp2ndOutDevProvider->getOutDev().SetFont( aVCLFont );

        return true;
    }

    bool CanvasHelper::repaint( const GraphicObjectSharedPtr&   rGrf,
                                const rendering::ViewState&     viewState,
                                const rendering::RenderState&   renderState,
                                const ::Point&                  rPt,
                                const ::Size&                   rSz,
                                const GraphicAttr&              rAttr ) const
    {
        ENSURE_OR_RETURN_FALSE( rGrf,
                          "Invalid Graphic" );

        if( !mpOutDevProvider )
            return false; // disposed
        else
        {
            tools::OutDevStateKeeper aStateKeeper( mpProtectedOutDevProvider );
            setupOutDevState( viewState, renderState, IGNORE_COLOR );

            if (!rGrf->Draw(mpOutDevProvider->getOutDev(), rPt, rSz, &rAttr))
                return false;

            // #i80779# Redraw also into mask outdev
            if (mp2ndOutDevProvider)
                return rGrf->Draw(mp2ndOutDevProvider->getOutDev(), rPt, rSz, &rAttr);

            return true;
        }
    }

    void CanvasHelper::flush() const
    {
        if (mpOutDevProvider)
            mpOutDevProvider->getOutDev().Flush();

        if  (mp2ndOutDevProvider)
            mp2ndOutDevProvider->getOutDev().Flush();
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
