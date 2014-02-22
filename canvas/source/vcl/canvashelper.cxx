/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>

#include <rtl/math.hxx>

#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/util/Endianness.hpp>
#include <com/sun/star/rendering/TextDirection.hpp>
#include <com/sun/star/rendering/TexturingMode.hpp>
#include <com/sun/star/rendering/PathCapType.hpp>
#include <com/sun/star/rendering/PathJoinType.hpp>
#include <com/sun/star/drawing/LineCap.hpp>

#include <tools/poly.hxx>
#include <vcl/window.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/canvastools.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/numeric/ftools.hxx>

#include <utility>

#include <comphelper/sequence.hxx>
#include <canvas/canvastools.hxx>

#include "textlayout.hxx"
#include "canvashelper.hxx"
#include "canvasbitmap.hxx"
#include "impltools.hxx"
#include "canvasfont.hxx"


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
                    return basegfx::B2DLINEJOIN_NONE;

                case rendering::PathJoinType::MITER:
                    return basegfx::B2DLINEJOIN_MITER;

                case rendering::PathJoinType::ROUND:
                    return basegfx::B2DLINEJOIN_ROUND;

                case rendering::PathJoinType::BEVEL:
                    return basegfx::B2DLINEJOIN_BEVEL;

                default:
                    ENSURE_OR_THROW( false,
                                      "b2DJoineFromJoin(): Unexpected join type" );
            }

            return basegfx::B2DLINEJOIN_NONE;
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
        mpProtectedOutDev(),
        mpOutDev(),
        mp2ndOutDev(),
        mbHaveAlpha( false )
    {
    }

    void CanvasHelper::disposing()
    {
        mpDevice = NULL;
        mpProtectedOutDev.reset();
        mpOutDev.reset();
        mp2ndOutDev.reset();
    }

    void CanvasHelper::init( rendering::XGraphicDevice&     rDevice,
                             const OutDevProviderSharedPtr& rOutDev,
                             bool                           bProtect,
                             bool                           bHaveAlpha )
    {
        
        
        mpDevice    = &rDevice;
        mbHaveAlpha = bHaveAlpha;

        setOutDev( rOutDev, bProtect );
    }

    void CanvasHelper::setOutDev( const OutDevProviderSharedPtr& rOutDev,
                                  bool                           bProtect )
    {
        if( bProtect )
            mpProtectedOutDev = rOutDev;
        else
            mpProtectedOutDev.reset();

        mpOutDev = rOutDev;
    }

    void CanvasHelper::setBackgroundOutDev( const OutDevProviderSharedPtr& rOutDev )
    {
        mp2ndOutDev = rOutDev;
        mp2ndOutDev->getOutDev().EnableMapMode( false );
        mp2ndOutDev->getOutDev().SetAntialiasing( ANTIALIASING_ENABLE_B2DDRAW );
    }

    void CanvasHelper::clear()
    {
        
        if( mpOutDev )
        {
            OutputDevice& rOutDev( mpOutDev->getOutDev() );
            tools::OutDevStateKeeper aStateKeeper( mpProtectedOutDev );

            rOutDev.EnableMapMode( false );
            rOutDev.SetAntialiasing( ANTIALIASING_ENABLE_B2DDRAW );
            rOutDev.SetLineColor( COL_WHITE );
            rOutDev.SetFillColor( COL_WHITE );
            rOutDev.SetClipRegion();
            rOutDev.DrawRect( Rectangle( Point(),
                                         rOutDev.GetOutputSizePixel()) );

            if( mp2ndOutDev )
            {
                OutputDevice& rOutDev2( mp2ndOutDev->getOutDev() );

                rOutDev2.SetDrawMode( DRAWMODE_DEFAULT );
                rOutDev2.EnableMapMode( false );
                rOutDev2.SetAntialiasing( ANTIALIASING_ENABLE_B2DDRAW );
                rOutDev2.SetLineColor( COL_WHITE );
                rOutDev2.SetFillColor( COL_WHITE );
                rOutDev2.SetClipRegion();
                rOutDev2.DrawRect( Rectangle( Point(),
                                              rOutDev2.GetOutputSizePixel()) );
                rOutDev2.SetDrawMode( DRAWMODE_BLACKLINE | DRAWMODE_BLACKFILL | DRAWMODE_BLACKTEXT |
                                      DRAWMODE_BLACKGRADIENT | DRAWMODE_BLACKBITMAP );
            }
        }
    }

    void CanvasHelper::drawPoint( const rendering::XCanvas*     ,
                                  const geometry::RealPoint2D&  aPoint,
                                  const rendering::ViewState&   viewState,
                                  const rendering::RenderState& renderState )
    {
        
        if( mpOutDev )
        {
            
            tools::OutDevStateKeeper aStateKeeper( mpProtectedOutDev );
            setupOutDevState( viewState, renderState, LINE_COLOR );

            const Point aOutPoint( tools::mapRealPoint2D( aPoint,
                                                          viewState, renderState ) );
            
            mpOutDev->getOutDev().DrawPixel( aOutPoint );

            if( mp2ndOutDev )
                mp2ndOutDev->getOutDev().DrawPixel( aOutPoint );
        }
    }

    void CanvasHelper::drawLine( const rendering::XCanvas*      ,
                                 const geometry::RealPoint2D&   aStartRealPoint2D,
                                 const geometry::RealPoint2D&   aEndRealPoint2D,
                                 const rendering::ViewState&    viewState,
                                 const rendering::RenderState&  renderState )
    {
        
        if( mpOutDev )
        {
            
            tools::OutDevStateKeeper aStateKeeper( mpProtectedOutDev );
            setupOutDevState( viewState, renderState, LINE_COLOR );

            const Point aStartPoint( tools::mapRealPoint2D( aStartRealPoint2D,
                                                            viewState, renderState ) );
            const Point aEndPoint( tools::mapRealPoint2D( aEndRealPoint2D,
                                                          viewState, renderState ) );
            
            mpOutDev->getOutDev().DrawLine( aStartPoint, aEndPoint );

            if( mp2ndOutDev )
                mp2ndOutDev->getOutDev().DrawLine( aStartPoint, aEndPoint );
        }
    }

    void CanvasHelper::drawBezier( const rendering::XCanvas*            ,
                                   const geometry::RealBezierSegment2D& aBezierSegment,
                                   const geometry::RealPoint2D&         _aEndPoint,
                                   const rendering::ViewState&          viewState,
                                   const rendering::RenderState&        renderState )
    {
        if( mpOutDev )
        {
            tools::OutDevStateKeeper aStateKeeper( mpProtectedOutDev );
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

            ::Polygon aPoly(4);
            aPoly.SetPoint( rStartPoint, 0 );
            aPoly.SetFlags( 0, POLY_NORMAL );
            aPoly.SetPoint( rCtrlPoint1, 1 );
            aPoly.SetFlags( 1, POLY_CONTROL );
            aPoly.SetPoint( rCtrlPoint2, 2 );
            aPoly.SetFlags( 2, POLY_CONTROL );
            aPoly.SetPoint( rEndPoint, 3 );
            aPoly.SetFlags( 3, POLY_NORMAL );

            
            mpOutDev->getOutDev().DrawPolygon( aPoly );
            if( mp2ndOutDev )
                mp2ndOutDev->getOutDev().DrawPolygon( aPoly );
        }
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::drawPolyPolygon( const rendering::XCanvas*                          ,
                                                                                 const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon,
                                                                                 const rendering::ViewState&                        viewState,
                                                                                 const rendering::RenderState&                      renderState )
    {
        ENSURE_ARG_OR_THROW( xPolyPolygon.is(),
                         "polygon is NULL");

        if( mpOutDev )
        {
            tools::OutDevStateKeeper aStateKeeper( mpProtectedOutDev );
            setupOutDevState( viewState, renderState, LINE_COLOR );

            const ::basegfx::B2DPolyPolygon& rPolyPoly(
                ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(xPolyPolygon) );
            const PolyPolygon aPolyPoly( tools::mapPolyPolygon( rPolyPoly, viewState, renderState ) );

            if( rPolyPoly.isClosed() )
            {
                mpOutDev->getOutDev().DrawPolyPolygon( aPolyPoly );

                if( mp2ndOutDev )
                    mp2ndOutDev->getOutDev().DrawPolyPolygon( aPolyPoly );
            }
            else
            {
                
                
                
                
                
                
                
                sal_uInt16 nSize( aPolyPoly.Count() );

                for( sal_uInt16 i=0; i<nSize; ++i )
                {
                    mpOutDev->getOutDev().DrawPolyLine( aPolyPoly[i] );

                    if( mp2ndOutDev )
                        mp2ndOutDev->getOutDev().DrawPolyLine( aPolyPoly[i] );
                }
            }
        }

        
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::strokePolyPolygon( const rendering::XCanvas*                            ,
                                                                                   const uno::Reference< rendering::XPolyPolygon2D >&   xPolyPolygon,
                                                                                   const rendering::ViewState&                          viewState,
                                                                                   const rendering::RenderState&                        renderState,
                                                                                   const rendering::StrokeAttributes&                   strokeAttributes )
    {
        ENSURE_ARG_OR_THROW( xPolyPolygon.is(),
                         "polygon is NULL");

        if( mpOutDev )
        {
            tools::OutDevStateKeeper aStateKeeper( mpProtectedOutDev );

            ::basegfx::B2DHomMatrix aMatrix;
            ::canvas::tools::mergeViewAndRenderTransform(aMatrix, viewState, renderState);

            ::basegfx::B2DSize aLinePixelSize(strokeAttributes.StrokeWidth,
                                              strokeAttributes.StrokeWidth);
            aLinePixelSize *= aMatrix;

            ::basegfx::B2DPolyPolygon aPolyPoly(
                ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(xPolyPolygon) );

            if( aPolyPoly.areControlPointsUsed() )
            {
                
                aPolyPoly = ::basegfx::tools::adaptiveSubdivideByAngle(aPolyPoly);
            }

            
            if( strokeAttributes.DashArray.getLength() )
            {
                const ::std::vector<double>& aDashArray(
                    ::comphelper::sequenceToContainer< ::std::vector<double> >(strokeAttributes.DashArray) );

                ::basegfx::B2DPolyPolygon aDashedPolyPoly;

                for( sal_uInt32 i=0; i<aPolyPoly.count(); ++i )
                {
                    
                    basegfx::tools::applyLineDashing(aPolyPoly.getB2DPolygon(i), aDashArray, &aDashedPolyPoly);
                    
                    
                    
                }

                aPolyPoly = aDashedPolyPoly;
            }

            ::basegfx::B2DPolyPolygon aStrokedPolyPoly;
            if( aLinePixelSize.getLength() < 1.42 )
            {
                
                
                setupOutDevState( viewState, renderState, LINE_COLOR );

                aStrokedPolyPoly = aPolyPoly;
            }
            else
            {
                
                setupOutDevState( viewState, renderState, FILL_COLOR );

                for( sal_uInt32 i=0; i<aPolyPoly.count(); ++i )
                {
                    
                    

                    
                    
                    
                    

                    
                    aStrokedPolyPoly.append(basegfx::tools::createAreaGeometry(
                        aPolyPoly.getB2DPolygon(i),
                        strokeAttributes.StrokeWidth*0.5,
                        b2DJoineFromJoin(strokeAttributes.JoinType),
                        unoCapeFromCap(strokeAttributes.StartCapType)
                        ));
                    
                    
                    
                    
                }
            }

            
            
            aStrokedPolyPoly.transform( aMatrix );

            const PolyPolygon aVCLPolyPoly( aStrokedPolyPoly );

            
            

            
            
            
            
            sal_uInt16 nSize( aVCLPolyPoly.Count() );

            for( sal_uInt16 i=0; i<nSize; ++i )
            {
                if( aStrokedPolyPoly.getB2DPolygon( i ).isClosed() ) {
                    mpOutDev->getOutDev().DrawPolygon( aVCLPolyPoly[i] );
                    if( mp2ndOutDev )
                        mp2ndOutDev->getOutDev().DrawPolygon( aVCLPolyPoly[i] );
                } else {
                    const sal_uInt16 nPolySize = aVCLPolyPoly[i].GetSize();
                    if( nPolySize ) {
                        Point rPrevPoint = aVCLPolyPoly[i].GetPoint( 0 );
                        Point rPoint;

                        for( sal_uInt16 j=1; j<nPolySize; j++ ) {
                            rPoint = aVCLPolyPoly[i].GetPoint( j );
                            mpOutDev->getOutDev().DrawLine( rPrevPoint, rPoint );
                            if( mp2ndOutDev )
                                mp2ndOutDev->getOutDev().DrawLine( rPrevPoint, rPoint );
                            rPrevPoint = rPoint;
                        }
                    }
                }
            }
        }

        
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::strokeTexturedPolyPolygon( const rendering::XCanvas*                            ,
                                                                                           const uno::Reference< rendering::XPolyPolygon2D >&   ,
                                                                                           const rendering::ViewState&                          ,
                                                                                           const rendering::RenderState&                        ,
                                                                                           const uno::Sequence< rendering::Texture >&           ,
                                                                                           const rendering::StrokeAttributes&                    )
    {
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::strokeTextureMappedPolyPolygon( const rendering::XCanvas*                           ,
                                                                                                const uno::Reference< rendering::XPolyPolygon2D >&  ,
                                                                                                const rendering::ViewState&                         ,
                                                                                                const rendering::RenderState&                       ,
                                                                                                const uno::Sequence< rendering::Texture >&          ,
                                                                                                const uno::Reference< geometry::XMapping2D >&       ,
                                                                                                const rendering::StrokeAttributes&                   )
    {
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XPolyPolygon2D >   CanvasHelper::queryStrokeShapes( const rendering::XCanvas*                            ,
                                                                                   const uno::Reference< rendering::XPolyPolygon2D >&   ,
                                                                                   const rendering::ViewState&                          ,
                                                                                   const rendering::RenderState&                        ,
                                                                                   const rendering::StrokeAttributes&                    )
    {
        return uno::Reference< rendering::XPolyPolygon2D >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::fillPolyPolygon( const rendering::XCanvas*                          ,
                                                                                 const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon,
                                                                                 const rendering::ViewState&                        viewState,
                                                                                 const rendering::RenderState&                      renderState )
    {
        ENSURE_ARG_OR_THROW( xPolyPolygon.is(),
                         "polygon is NULL");

        if( mpOutDev )
        {
            tools::OutDevStateKeeper aStateKeeper( mpProtectedOutDev );

            const int nTransparency( setupOutDevState( viewState, renderState, FILL_COLOR ) );
            ::basegfx::B2DPolyPolygon aB2DPolyPoly(
                ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(xPolyPolygon));
            aB2DPolyPoly.setClosed(true); 
            const PolyPolygon aPolyPoly( tools::mapPolyPolygon(
                                             aB2DPolyPoly,
                                             viewState, renderState ) );
            const bool bSourceAlpha( renderState.CompositeOperation == rendering::CompositeOperation::SOURCE );
            if( !nTransparency || bSourceAlpha )
            {
                mpOutDev->getOutDev().DrawPolyPolygon( aPolyPoly );
            }
            else
            {
                const int nTransPercent( (nTransparency * 100 + 128) / 255 );  
                mpOutDev->getOutDev().DrawTransparent( aPolyPoly, (sal_uInt16)nTransPercent );
            }

            if( mp2ndOutDev )
            {
                
                
                
                
                if( nTransparency < 253 )
                {
                    mp2ndOutDev->getOutDev().SetFillColor( COL_BLACK );
                    mp2ndOutDev->getOutDev().DrawPolyPolygon( aPolyPoly );
                }
            }
        }

        
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::fillTextureMappedPolyPolygon( const rendering::XCanvas*                             ,
                                                                                              const uno::Reference< rendering::XPolyPolygon2D >&    ,
                                                                                              const rendering::ViewState&                           ,
                                                                                              const rendering::RenderState&                         ,
                                                                                              const uno::Sequence< rendering::Texture >&            ,
                                                                                              const uno::Reference< geometry::XMapping2D >&              )
    {
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCanvasFont > CanvasHelper::createFont( const rendering::XCanvas*                        ,
                                                                       const rendering::FontRequest&                    fontRequest,
                                                                       const uno::Sequence< beans::PropertyValue >&     extraFontProperties,
                                                                       const geometry::Matrix2D&                        fontMatrix )
    {
        if( mpOutDev && mpDevice )
        {
            
            return uno::Reference< rendering::XCanvasFont >(
                    new CanvasFont(fontRequest, extraFontProperties, fontMatrix,
                                   *mpDevice, mpOutDev) );
        }

        return uno::Reference< rendering::XCanvasFont >();
    }

    uno::Sequence< rendering::FontInfo > CanvasHelper::queryAvailableFonts( const rendering::XCanvas*                       ,
                                                                            const rendering::FontInfo&                      ,
                                                                            const uno::Sequence< beans::PropertyValue >&     )
    {
        
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

        if( mpOutDev )
        {
            tools::OutDevStateKeeper aStateKeeper( mpProtectedOutDev );

            ::Point aOutpos;
            if( !setupTextOutput( aOutpos, viewState, renderState, xFont ) )
                return uno::Reference< rendering::XCachedPrimitive >(NULL); 

            
            sal_uIntPtr nLayoutMode(0);
            switch( textDirection )
            {
                case rendering::TextDirection::WEAK_LEFT_TO_RIGHT:
                    nLayoutMode |= TEXT_LAYOUT_BIDI_LTR;
                    
                case rendering::TextDirection::STRONG_LEFT_TO_RIGHT:
                    nLayoutMode |= TEXT_LAYOUT_BIDI_LTR | TEXT_LAYOUT_BIDI_STRONG;
                    nLayoutMode |= TEXT_LAYOUT_TEXTORIGIN_LEFT;
                    break;

                case rendering::TextDirection::WEAK_RIGHT_TO_LEFT:
                    nLayoutMode |= TEXT_LAYOUT_BIDI_RTL;
                    
                case rendering::TextDirection::STRONG_RIGHT_TO_LEFT:
                    nLayoutMode |= TEXT_LAYOUT_BIDI_RTL | TEXT_LAYOUT_BIDI_STRONG;
                    nLayoutMode |= TEXT_LAYOUT_TEXTORIGIN_RIGHT;
                    break;
            }

            
            mpOutDev->getOutDev().SetLayoutMode( nLayoutMode );
            mpOutDev->getOutDev().DrawText( aOutpos,
                                            text.Text,
                                            ::canvas::tools::numeric_cast<sal_uInt16>(text.StartPosition),
                                            ::canvas::tools::numeric_cast<sal_uInt16>(text.Length) );

            if( mp2ndOutDev )
            {
                mp2ndOutDev->getOutDev().SetLayoutMode( nLayoutMode );
                mp2ndOutDev->getOutDev().DrawText( aOutpos,
                                                   text.Text,
                                                   ::canvas::tools::numeric_cast<sal_uInt16>(text.StartPosition),
                                                   ::canvas::tools::numeric_cast<sal_uInt16>(text.Length) );
            }
        }

        return uno::Reference< rendering::XCachedPrimitive >(NULL);
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
            if( mpOutDev )
            {
                tools::OutDevStateKeeper aStateKeeper( mpProtectedOutDev );

                
                
                
                

                ::Point aOutpos;
                if( !setupTextOutput( aOutpos, viewState, renderState, xLayoutedText->getFont() ) )
                    return uno::Reference< rendering::XCachedPrimitive >(NULL); 

                
                
                pTextLayout->draw( mpOutDev->getOutDev(), aOutpos, viewState, renderState );

                if( mp2ndOutDev )
                    pTextLayout->draw( mp2ndOutDev->getOutDev(), aOutpos, viewState, renderState );
            }
        }
        else
        {
            ENSURE_ARG_OR_THROW( false,
                                 "TextLayout not compatible with this canvas" );
        }

        return uno::Reference< rendering::XCachedPrimitive >(NULL);
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
                                      BOOST_CURRENT_FUNCTION,
                                      mpDevice,
                                      4,
                                      bModulateColors ? 3 : 0 );

        if( mpOutDev )
        {
            tools::OutDevStateKeeper aStateKeeper( mpProtectedOutDev );
            setupOutDevState( viewState, renderState, IGNORE_COLOR );

            ::basegfx::B2DHomMatrix aMatrix;
            ::canvas::tools::mergeViewAndRenderTransform(aMatrix, viewState, renderState);

            ::basegfx::B2DPoint aOutputPos( 0.0, 0.0 );
            aOutputPos *= aMatrix;

            BitmapEx aBmpEx( tools::bitmapExFromXBitmap(xBitmap) );

            
            
            
            if( bModulateColors &&
                renderState.DeviceColor.getLength() > 3 )
            {
                
                
                bModulateColors = !::rtl::math::approxEqual(
                    renderState.DeviceColor[3], 1.0);
            }

            
            
            
            
            if( !bModulateColors &&
                (aMatrix.isIdentity() ||
                 (::basegfx::fTools::equalZero( aMatrix.get(0,1) ) &&
                  ::basegfx::fTools::equalZero( aMatrix.get(1,0) ) &&
                  ::rtl::math::approxEqual(aMatrix.get(0,0), 1.0) &&
                  ::rtl::math::approxEqual(aMatrix.get(1,1), 1.0)) ) )
            {
                
                
                mpOutDev->getOutDev().DrawBitmapEx( ::vcl::unotools::pointFromB2DPoint( aOutputPos ),
                                                    aBmpEx );

                if( mp2ndOutDev )
                {
                    
                    
                    
                    
                    if( aBmpEx.IsAlpha() )
                    {
                        Bitmap aMask( aBmpEx.GetAlpha().GetBitmap() );
                        aMask.MakeMono( 253 );
                        aBmpEx = BitmapEx( aBmpEx.GetBitmap(), aMask );
                    }
                    else if( aBmpEx.IsTransparent() )
                    {
                        aBmpEx = BitmapEx( aBmpEx.GetBitmap(), aBmpEx.GetMask() );
                    }

                    mp2ndOutDev->getOutDev().DrawBitmapEx( ::vcl::unotools::pointFromB2DPoint( aOutputPos ),
                                                           aBmpEx );
                }

                
                
                return uno::Reference< rendering::XCachedPrimitive >(NULL);
            }
            else
            {
                
                
                
                ::basegfx::B2DVector aScale;
                double               nRotate;
                double               nShearX;
                aMatrix.decompose( aScale, aOutputPos, nRotate, nShearX );

                GraphicAttr             aGrfAttr;
                GraphicObjectSharedPtr  pGrfObj;

                ::Size aBmpSize( aBmpEx.GetSizePixel() );

                
                if( bModulateColors )
                {
                    const double nAlphaModulation( renderState.DeviceColor[3] );

                    
                    
                    
                    
                    
                    
                    
                    
                    
                    aGrfAttr.SetTransparency(
                        static_cast< sal_uInt8 >(
                            ::basegfx::fround( 255.0*( 1.0 - nAlphaModulation ) ) ) );
                }

                if( ::basegfx::fTools::equalZero( nShearX ) )
                {
                    
                    
                    

                    
                    
                    

                    
                    
                    
                    
                    const double nAngleInTenthOfDegrees (3600.0 - nRotate * 3600.0 / (2*M_PI));
                    aGrfAttr.SetRotation( static_cast< sal_uInt16 >(::basegfx::fround(nAngleInTenthOfDegrees)) );

                    pGrfObj.reset( new GraphicObject( aBmpEx ) );
                }
                else
                {
                    
                    
                    
                    ::basegfx::B2DRectangle aDestRect;
                    ::canvas::tools::calcTransformedRectBounds( aDestRect,
                                                                ::basegfx::B2DRectangle(0,
                                                                                        0,
                                                                                        aBmpSize.Width(),
                                                                                        aBmpSize.Height()),
                                                                aMatrix );

                    aOutputPos.setX( aDestRect.getMinX() );
                    aOutputPos.setY( aDestRect.getMinY() );

                    
                    
                    aBmpEx = tools::transformBitmap( aBmpEx,
                                                     aMatrix,
                                                     renderState.DeviceColor,
                                                     tools::MODULATE_NONE );

                    pGrfObj.reset( new GraphicObject( aBmpEx ) );

                    
                    
                    aScale.setX( 1.0 ); aScale.setY( 1.0 );

                    
                    aBmpSize = aBmpEx.GetSizePixel();
                }

                
                const ::Point aPt( ::vcl::unotools::pointFromB2DPoint( aOutputPos ) );
                const ::Size  aSz( ::basegfx::fround( aScale.getX() * aBmpSize.Width() ),
                                   ::basegfx::fround( aScale.getY() * aBmpSize.Height() ) );

                pGrfObj->Draw( &mpOutDev->getOutDev(),
                               aPt,
                               aSz,
                               &aGrfAttr );

                if( mp2ndOutDev )
                    pGrfObj->Draw( &mp2ndOutDev->getOutDev(),
                                   aPt,
                                   aSz,
                                   &aGrfAttr );

                
                
                
                return uno::Reference< rendering::XCachedPrimitive >(
                    new CachedBitmap( pGrfObj,
                                      aPt,
                                      aSz,
                                      aGrfAttr,
                                      viewState,
                                      renderState,
                                      
                                      
                                      
                                      
                                      const_cast< rendering::XCanvas* >(pCanvas)) );
            }
        }

        
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
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

    uno::Reference< rendering::XGraphicDevice > CanvasHelper::getDevice()
    {
        
        
        return uno::Reference< rendering::XGraphicDevice >(mpDevice);
    }

    void CanvasHelper::copyRect( const rendering::XCanvas*                          ,
                                 const uno::Reference< rendering::XBitmapCanvas >&  ,
                                 const geometry::RealRectangle2D&                   ,
                                 const rendering::ViewState&                        ,
                                 const rendering::RenderState&                      ,
                                 const geometry::RealRectangle2D&                   ,
                                 const rendering::ViewState&                        ,
                                 const rendering::RenderState&                       )
    {
        
    }

    geometry::IntegerSize2D CanvasHelper::getSize()
    {
        if( !mpOutDev.get() )
            return geometry::IntegerSize2D(); 

        return ::vcl::unotools::integerSize2DFromSize( mpOutDev->getOutDev().GetOutputSizePixel() );
    }

    uno::Reference< rendering::XBitmap > CanvasHelper::getScaledBitmap( const geometry::RealSize2D& newSize,
                                                                        sal_Bool                    beFast )
    {
        if( !mpOutDev.get() || !mpDevice )
            return uno::Reference< rendering::XBitmap >(); 

        OutputDevice& rOutDev( mpOutDev->getOutDev() );

        tools::OutDevStateKeeper aStateKeeper( mpProtectedOutDev );
        rOutDev.EnableMapMode( false );
        rOutDev.SetAntialiasing( ANTIALIASING_ENABLE_B2DDRAW );

        
        const Point aEmptyPoint(0,0);
        const Size  aBmpSize( rOutDev.GetOutputSizePixel() );

        Bitmap aBitmap( rOutDev.GetBitmap(aEmptyPoint, aBmpSize) );

        aBitmap.Scale( ::vcl::unotools::sizeFromRealSize2D(newSize),
                       beFast ? BMP_SCALE_DEFAULT : BMP_SCALE_BESTQUALITY );

        return uno::Reference< rendering::XBitmap >(
            new CanvasBitmap( aBitmap, *mpDevice, mpOutDev ) );
    }

    uno::Sequence< sal_Int8 > CanvasHelper::getData( rendering::IntegerBitmapLayout&     rLayout,
                                                     const geometry::IntegerRectangle2D& rect )
    {
        if( !mpOutDev.get() )
            return uno::Sequence< sal_Int8 >(); 

        rLayout = getMemoryLayout();

        
        const Rectangle aRect( ::vcl::unotools::rectangleFromIntegerRectangle2D(rect) );

        OutputDevice& rOutDev( mpOutDev->getOutDev() );

        tools::OutDevStateKeeper aStateKeeper( mpProtectedOutDev );
        rOutDev.EnableMapMode( false );
        rOutDev.SetAntialiasing( ANTIALIASING_ENABLE_B2DDRAW );

        Bitmap aBitmap( rOutDev.GetBitmap(aRect.TopLeft(),
                                          aRect.GetSize()) );

        Bitmap::ScopedReadAccess pReadAccess( aBitmap );

        ENSURE_OR_THROW( pReadAccess.get() != NULL,
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

    void CanvasHelper::setData( const uno::Sequence< sal_Int8 >&        data,
                                const rendering::IntegerBitmapLayout&   aLayout,
                                const geometry::IntegerRectangle2D&     rect )
    {
        if( !mpOutDev.get() )
            return; 

        const rendering::IntegerBitmapLayout aRefLayout( getMemoryLayout() );
        ENSURE_ARG_OR_THROW( aRefLayout.PlaneStride != aLayout.PlaneStride ||
                             aRefLayout.ColorSpace  != aLayout.ColorSpace ||
                             aRefLayout.Palette     != aLayout.Palette ||
                             aRefLayout.IsMsbFirst  != aLayout.IsMsbFirst,
                             "Mismatching memory layout" );

        OutputDevice& rOutDev( mpOutDev->getOutDev() );

        tools::OutDevStateKeeper aStateKeeper( mpProtectedOutDev );
        rOutDev.EnableMapMode( false );
        rOutDev.SetAntialiasing( ANTIALIASING_ENABLE_B2DDRAW );

        const Rectangle aRect( ::vcl::unotools::rectangleFromIntegerRectangle2D(rect) );
        const sal_uInt16    nBitCount( ::std::min( (sal_uInt16)24U,
                                               (sal_uInt16)rOutDev.GetBitCount() ) );
        const BitmapPalette* pPalette = NULL;

        if( nBitCount <= 8 )
        {
            

            
            
            const Point aEmptyPoint;
            const Size  aSize(1,1);
            Bitmap aTmpBitmap( rOutDev.GetBitmap( aEmptyPoint,
                                                  aSize ) );

            Bitmap::ScopedReadAccess pReadAccess( aTmpBitmap );

            pPalette = &pReadAccess->GetPalette();
        }

        
        Bitmap aBitmap( aRect.GetSize(), nBitCount, pPalette );

        bool bCopyBack( false ); 
                                 
        {
            Bitmap::ScopedWriteAccess pWriteAccess( aBitmap );

            ENSURE_OR_THROW( pWriteAccess.get() != NULL,
                             "Could not acquire write access to OutDev bitmap" );

            
            const sal_Int32 nWidth( rect.X2 - rect.X1 );
            const sal_Int32 nHeight( rect.Y2 - rect.Y1 );
            int x, y, nCurrPos(0);
            for( y=0; y<nHeight; ++y )
            {
                switch( pWriteAccess->GetScanlineFormat() )
                {
                    case BMP_FORMAT_8BIT_PAL:
                    {
                        Scanline pScan = pWriteAccess->GetScanline( y );

                        for( x=0; x<nWidth; ++x )
                        {
                            *pScan++ = (sal_uInt8)pWriteAccess->GetBestPaletteIndex(
                                BitmapColor( data[ nCurrPos ],
                                             data[ nCurrPos+1 ],
                                             data[ nCurrPos+2 ] ) );

                            nCurrPos += 4;
                        }
                    }
                    break;

                    case BMP_FORMAT_24BIT_TC_BGR:
                    {
                        Scanline pScan = pWriteAccess->GetScanline( y );

                        for( x=0; x<nWidth; ++x )
                        {
                            *pScan++ = data[ nCurrPos+2 ];
                            *pScan++ = data[ nCurrPos+1 ];
                            *pScan++ = data[ nCurrPos   ];

                            nCurrPos += 4;
                        }
                    }
                    break;

                    case BMP_FORMAT_24BIT_TC_RGB:
                    {
                        Scanline pScan = pWriteAccess->GetScanline( y );

                        for( x=0; x<nWidth; ++x )
                        {
                            *pScan++ = data[ nCurrPos   ];
                            *pScan++ = data[ nCurrPos+1 ];
                            *pScan++ = data[ nCurrPos+2 ];

                            nCurrPos += 4;
                        }
                    }
                    break;

                    default:
                    {
                        for( x=0; x<nWidth; ++x )
                        {
                            pWriteAccess->SetPixel( y, x, BitmapColor( data[ nCurrPos   ],
                                                                       data[ nCurrPos+1 ],
                                                                       data[ nCurrPos+2 ] ) );
                            nCurrPos += 4;
                        }
                    }
                    break;
                }
            }

            bCopyBack = true;
        }

        
        
        if( bCopyBack )
        {
            
            rOutDev.DrawBitmap(aRect.TopLeft(), aBitmap);
        }
    }

    void CanvasHelper::setPixel( const uno::Sequence< sal_Int8 >&       color,
                                 const rendering::IntegerBitmapLayout&  rLayout,
                                 const geometry::IntegerPoint2D&        pos )
    {
        if( !mpOutDev.get() )
            return; 

        OutputDevice& rOutDev( mpOutDev->getOutDev() );

        tools::OutDevStateKeeper aStateKeeper( mpProtectedOutDev );
        rOutDev.EnableMapMode( false );
        rOutDev.SetAntialiasing( ANTIALIASING_ENABLE_B2DDRAW );

        const Size aBmpSize( rOutDev.GetOutputSizePixel() );

        ENSURE_ARG_OR_THROW( pos.X >= 0 && pos.X < aBmpSize.Width(),
                             "X coordinate out of bounds" );
        ENSURE_ARG_OR_THROW( pos.Y >= 0 && pos.Y < aBmpSize.Height(),
                             "Y coordinate out of bounds" );
        ENSURE_ARG_OR_THROW( color.getLength() > 3,
                             "not enough color components" );

        const rendering::IntegerBitmapLayout aRefLayout( getMemoryLayout() );
        ENSURE_ARG_OR_THROW( aRefLayout.PlaneStride != rLayout.PlaneStride ||
                             aRefLayout.ColorSpace  != rLayout.ColorSpace ||
                             aRefLayout.Palette     != rLayout.Palette ||
                             aRefLayout.IsMsbFirst  != rLayout.IsMsbFirst,
                             "Mismatching memory layout" );

        
        rOutDev.DrawPixel( ::vcl::unotools::pointFromIntegerPoint2D( pos ),
                           ::canvas::tools::stdIntSequenceToColor( color ));
    }

    uno::Sequence< sal_Int8 > CanvasHelper::getPixel( rendering::IntegerBitmapLayout& rLayout,
                                                      const geometry::IntegerPoint2D& pos )
    {
        if( !mpOutDev.get() )
            return uno::Sequence< sal_Int8 >(); 

        rLayout = getMemoryLayout();
        rLayout.ScanLines = 1;
        rLayout.ScanLineBytes = 4;
        rLayout.ScanLineStride = rLayout.ScanLineBytes;

        OutputDevice& rOutDev( mpOutDev->getOutDev() );

        tools::OutDevStateKeeper aStateKeeper( mpProtectedOutDev );
        rOutDev.EnableMapMode( false );
        rOutDev.SetAntialiasing( ANTIALIASING_ENABLE_B2DDRAW );

        const Size aBmpSize( rOutDev.GetOutputSizePixel() );

        ENSURE_ARG_OR_THROW( pos.X >= 0 && pos.X < aBmpSize.Width(),
                             "X coordinate out of bounds" );
        ENSURE_ARG_OR_THROW( pos.Y >= 0 && pos.Y < aBmpSize.Height(),
                             "Y coordinate out of bounds" );

        
        return ::canvas::tools::colorToStdIntSequence(
            rOutDev.GetPixel(
                ::vcl::unotools::pointFromIntegerPoint2D( pos )));
    }

    rendering::IntegerBitmapLayout CanvasHelper::getMemoryLayout()
    {
        if( !mpOutDev.get() )
            return rendering::IntegerBitmapLayout(); 

        rendering::IntegerBitmapLayout xBitmapLayout( ::canvas::tools::getStdMemoryLayout(getSize()) );
        if ( !mbHaveAlpha )
            xBitmapLayout.ColorSpace = canvas::tools::getStdColorSpaceWithoutAlpha();

        return xBitmapLayout;
    }

    int CanvasHelper::setupOutDevState( const rendering::ViewState&     viewState,
                                        const rendering::RenderState&   renderState,
                                        ColorType                       eColorType ) const
    {
        ENSURE_OR_THROW( mpOutDev.get(),
                         "outdev null. Are we disposed?" );

        ::canvas::tools::verifyInput( renderState,
                                      BOOST_CURRENT_FUNCTION,
                                      mpDevice,
                                      2,
                                      eColorType == IGNORE_COLOR ? 0 : 3 );

        OutputDevice& rOutDev( mpOutDev->getOutDev() );
        OutputDevice* p2ndOutDev = NULL;

        rOutDev.EnableMapMode( false );
        rOutDev.SetAntialiasing( ANTIALIASING_ENABLE_B2DDRAW );

        if( mp2ndOutDev )
            p2ndOutDev = &mp2ndOutDev->getOutDev();

        int nTransparency(0);

        
        

        
        

        Region aClipRegion(true);

        if( viewState.Clip.is() )
        {
            ::basegfx::B2DPolyPolygon aClipPoly(
                ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(viewState.Clip) );

            if( aClipPoly.count() )
            {
                
                ::basegfx::B2DHomMatrix aMatrix;
                aClipPoly.transform(
                    ::basegfx::unotools::homMatrixFromAffineMatrix( aMatrix,
                                                                    viewState.AffineTransform ) );

                aClipRegion = Region::GetRegionFromPolyPolygon( ::PolyPolygon( aClipPoly ) );
            }
            else
            {
                
                aClipRegion.SetEmpty();
            }
        }

        if( renderState.Clip.is() )
        {
            ::basegfx::B2DPolyPolygon aClipPoly(
                ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(renderState.Clip) );

            ::basegfx::B2DHomMatrix aMatrix;
            aClipPoly.transform(
                ::canvas::tools::mergeViewAndRenderTransform( aMatrix,
                                                              viewState,
                                                              renderState ) );

            if( aClipPoly.count() )
            {
                
                Region aRegion = Region::GetRegionFromPolyPolygon( ::PolyPolygon( aClipPoly ) );
                aClipRegion.Intersect( aRegion );
            }
            else
            {
                
                aClipRegion.SetEmpty();
            }
        }

        
        
        
        
        
        
        if( aClipRegion.IsNull() )
        {
            rOutDev.SetClipRegion();

            if( p2ndOutDev )
                p2ndOutDev->SetClipRegion();
        }
        else
        {
            rOutDev.SetClipRegion( aClipRegion );

            if( p2ndOutDev )
                p2ndOutDev->SetClipRegion( aClipRegion );
        }

        Color aColor( COL_WHITE );

        if( renderState.DeviceColor.getLength() > 2 )
        {
            aColor = ::vcl::unotools::stdColorSpaceSequenceToColor(
                renderState.DeviceColor );
        }

        
        
        nTransparency = aColor.GetTransparency();
        aColor.SetTransparency(0);

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

        return nTransparency;
    }

    bool CanvasHelper::setupTextOutput( ::Point&                                        o_rOutPos,
                                        const rendering::ViewState&                     viewState,
                                        const rendering::RenderState&                   renderState,
                                        const uno::Reference< rendering::XCanvasFont >& xFont   ) const
    {
        ENSURE_OR_THROW( mpOutDev.get(),
                         "outdev null. Are we disposed?" );

        setupOutDevState( viewState, renderState, TEXT_COLOR );

        OutputDevice& rOutDev( mpOutDev->getOutDev() );

        ::Font aVCLFont;

        CanvasFont* pFont = dynamic_cast< CanvasFont* >( xFont.get() );

        ENSURE_ARG_OR_THROW( pFont,
                             "Font not compatible with this canvas" );

        aVCLFont = pFont->getVCLFont();

        Color aColor( COL_BLACK );

        if( renderState.DeviceColor.getLength() > 2 )
        {
            aColor = ::vcl::unotools::stdColorSpaceSequenceToColor(
                renderState.DeviceColor );
        }

        
        aVCLFont.SetColor( aColor );
        aVCLFont.SetFillColor( aColor );

        
        if( !tools::setupFontTransform( o_rOutPos, aVCLFont, viewState, renderState, rOutDev ) )
            return false;

        rOutDev.SetFont( aVCLFont );

        if( mp2ndOutDev )
            mp2ndOutDev->getOutDev().SetFont( aVCLFont );

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

        if( !mpOutDev )
            return false; 
        else
        {
            tools::OutDevStateKeeper aStateKeeper( mpProtectedOutDev );
            setupOutDevState( viewState, renderState, IGNORE_COLOR );

            if( !rGrf->Draw( &mpOutDev->getOutDev(), rPt, rSz, &rAttr ) )
                return false;

            
            if( mp2ndOutDev )
                return rGrf->Draw( &mp2ndOutDev->getOutDev(), rPt, rSz, &rAttr );

            return true;
        }
    }

    void CanvasHelper::flush() const
    {
        if( mpOutDev && mpOutDev->getOutDev().GetOutDevType() == OUTDEV_WINDOW )
        {
            
            
            static_cast<Window&>(mpOutDev->getOutDev()).Flush();
        }

        if( mp2ndOutDev && mp2ndOutDev->getOutDev().GetOutDevType() == OUTDEV_WINDOW )
        {
            
            
            static_cast<Window&>(mp2ndOutDev->getOutDev()).Flush();
        }
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
