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

#include <com/sun/star/rendering/TexturingMode.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/RepaintResult.hpp>
#include <com/sun/star/rendering/PathCapType.hpp>
#include <com/sun/star/rendering/PathJoinType.hpp>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include <comphelper/sequence.hxx>
#include <canvas/canvastools.hxx>

#include "dx_spritecanvas.hxx"
#include "dx_impltools.hxx"
#include "dx_vcltools.hxx"
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
                    ENSURE_OR_THROW( false,
                                      "gdiCapFromCap(): Unexpected cap type" );
            }

            return Gdiplus::LineCapFlat;
        }

        Gdiplus::LineJoin gdiJoinFromJoin( sal_Int8 nJoinType )
        {
            switch( nJoinType )
            {
                case rendering::PathJoinType::NONE:
                    SAL_WARN( "canvas.directx", "gdiJoinFromJoin(): Join NONE not possible, mapping to MITER" );
                    
                case rendering::PathJoinType::MITER:
                    return Gdiplus::LineJoinMiter;

                case rendering::PathJoinType::ROUND:
                    return Gdiplus::LineJoinRound;

                case rendering::PathJoinType::BEVEL:
                    return Gdiplus::LineJoinBevel;

                default:
                    ENSURE_OR_THROW( false,
                                      "gdiJoinFromJoin(): Unexpected join type" );
            }

            return Gdiplus::LineJoinMiter;
        }
    }

    CanvasHelper::CanvasHelper() :
        mpGdiPlusUser( GDIPlusUser::createInstance() ),
        mpDevice( NULL ),
        mpGraphicsProvider(),
        maOutputOffset()
    {
    }

    void CanvasHelper::disposing()
    {
        mpGraphicsProvider.reset();
        mpDevice = NULL;
        mpGdiPlusUser.reset();
    }

    void CanvasHelper::setDevice( rendering::XGraphicDevice& rDevice )
    {
        mpDevice = &rDevice;
    }

    void CanvasHelper::setTarget( const GraphicsProviderSharedPtr& rTarget )
    {
        ENSURE_OR_THROW( rTarget,
                          "CanvasHelper::setTarget(): Invalid target" );
        ENSURE_OR_THROW( !mpGraphicsProvider.get(),
                          "CanvasHelper::setTarget(): target set, old target would be overwritten" );

        mpGraphicsProvider = rTarget;
    }

    void CanvasHelper::setTarget( const GraphicsProviderSharedPtr& rTarget,
                                  const ::basegfx::B2ISize&        rOutputOffset )
    {
        ENSURE_OR_THROW( rTarget,
                         "CanvasHelper::setTarget(): invalid target" );
        ENSURE_OR_THROW( !mpGraphicsProvider.get(),
                         "CanvasHelper::setTarget(): target set, old target would be overwritten" );

        mpGraphicsProvider = rTarget;
        maOutputOffset = rOutputOffset;
    }

    void CanvasHelper::clear()
    {
        if( needOutput() )
        {
            GraphicsSharedPtr pGraphics( mpGraphicsProvider->getGraphics() );
            Gdiplus::Color aClearColor = Gdiplus::Color((Gdiplus::ARGB)Gdiplus::Color::White);

            ENSURE_OR_THROW(
                Gdiplus::Ok == pGraphics->SetCompositingMode(
                    Gdiplus::CompositingModeSourceCopy ), 
                "CanvasHelper::clear(): GDI+ SetCompositingMode call failed" );
            ENSURE_OR_THROW(
                Gdiplus::Ok == pGraphics->Clear( aClearColor ),
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
            GraphicsSharedPtr pGraphics( mpGraphicsProvider->getGraphics() );

            setupGraphicsState( pGraphics, viewState, renderState );

            Gdiplus::SolidBrush aBrush(
                Gdiplus::Color(
                    tools::sequenceToArgb(renderState.DeviceColor)) );

            
            Gdiplus::Matrix aMatrix;
            pGraphics->GetTransform(&aMatrix);
            aMatrix.Invert();
            Gdiplus::PointF vector(1, 1);
            aMatrix.TransformVectors(&vector);

            
            
            ENSURE_OR_THROW(
                Gdiplus::Ok == pGraphics->FillEllipse( &aBrush,
                                                       
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
            GraphicsSharedPtr pGraphics( mpGraphicsProvider->getGraphics() );

            setupGraphicsState( pGraphics, viewState, renderState );

            Gdiplus::Pen aPen(
                Gdiplus::Color(
                    tools::sequenceToArgb(renderState.DeviceColor)),
                Gdiplus::REAL(0.0) );

            
            
            
            
            
            const Gdiplus::PixelOffsetMode aOldMode(
                pGraphics->GetPixelOffsetMode() );
            pGraphics->SetPixelOffsetMode( Gdiplus::PixelOffsetModeNone );

            Gdiplus::Status hr = pGraphics->DrawLine( &aPen,
                                                      Gdiplus::REAL(aStartPoint.X), 
                                                      Gdiplus::REAL(aStartPoint.Y),
                                                      Gdiplus::REAL(aEndPoint.X),
                                                      Gdiplus::REAL(aEndPoint.Y) );
            pGraphics->SetPixelOffsetMode( aOldMode );

            ENSURE_OR_THROW(
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
            GraphicsSharedPtr pGraphics( mpGraphicsProvider->getGraphics() );

            setupGraphicsState( pGraphics, viewState, renderState );

            Gdiplus::Pen aPen(
                Gdiplus::Color(
                    tools::sequenceToArgb(renderState.DeviceColor)),
                Gdiplus::REAL(0.0) );

            
            
            
            
            
            const Gdiplus::PixelOffsetMode aOldMode(
                pGraphics->GetPixelOffsetMode() );
            pGraphics->SetPixelOffsetMode( Gdiplus::PixelOffsetModeNone );

            Gdiplus::Status hr = pGraphics->DrawBezier( &aPen,
                                                        Gdiplus::REAL(aBezierSegment.Px), 
                                                        Gdiplus::REAL(aBezierSegment.Py),
                                                        Gdiplus::REAL(aBezierSegment.C1x),
                                                        Gdiplus::REAL(aBezierSegment.C1y),
                                                        Gdiplus::REAL(aEndPoint.X),
                                                        Gdiplus::REAL(aEndPoint.Y),
                                                        Gdiplus::REAL(aBezierSegment.C2x),
                                                        Gdiplus::REAL(aBezierSegment.C2y) );

            pGraphics->SetPixelOffsetMode( aOldMode );

            ENSURE_OR_THROW(
                Gdiplus::Ok == hr,
                "CanvasHelper::drawBezier(): GDI+ call failed" );
        }
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::drawPolyPolygon( const rendering::XCanvas*                          /*pCanvas*/,
                                                                                 const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon,
                                                                                 const rendering::ViewState&                        viewState,
                                                                                 const rendering::RenderState&                      renderState )
    {
        ENSURE_OR_THROW( xPolyPolygon.is(),
                          "CanvasHelper::drawPolyPolygon: polygon is NULL");

        if( needOutput() )
        {
            GraphicsSharedPtr pGraphics( mpGraphicsProvider->getGraphics() );

            setupGraphicsState( pGraphics, viewState, renderState );

            Gdiplus::Pen aPen(
                Gdiplus::Color(
                    tools::sequenceToArgb(renderState.DeviceColor)),
                Gdiplus::REAL(0.0) );

            
            
            
            
            
            const Gdiplus::PixelOffsetMode aOldMode(
                pGraphics->GetPixelOffsetMode() );
            pGraphics->SetPixelOffsetMode( Gdiplus::PixelOffsetModeNone );

            GraphicsPathSharedPtr pPath( tools::graphicsPathFromXPolyPolygon2D( xPolyPolygon ) );

            
            Gdiplus::Status hr = pGraphics->DrawPath( &aPen, pPath.get() );

            pGraphics->SetPixelOffsetMode( aOldMode );

            ENSURE_OR_THROW(
                Gdiplus::Ok == hr,
                "CanvasHelper::drawPolyPolygon(): GDI+ call failed" );
        }

        
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::strokePolyPolygon( const rendering::XCanvas*                            /*pCanvas*/,
                                                                                   const uno::Reference< rendering::XPolyPolygon2D >&   xPolyPolygon,
                                                                                   const rendering::ViewState&                          viewState,
                                                                                   const rendering::RenderState&                        renderState,
                                                                                   const rendering::StrokeAttributes&                   strokeAttributes )
    {
        ENSURE_OR_THROW( xPolyPolygon.is(),
                          "CanvasHelper::drawPolyPolygon: polygon is NULL");

        if( needOutput() )
        {
            GraphicsSharedPtr pGraphics( mpGraphicsProvider->getGraphics() );

            setupGraphicsState( pGraphics, viewState, renderState );


            
            

            Gdiplus::Pen aPen(
                Gdiplus::Color(
                    tools::sequenceToArgb(renderState.DeviceColor)),
                static_cast< Gdiplus::REAL >(strokeAttributes.StrokeWidth) );

            
            
            
            
            
            const Gdiplus::PixelOffsetMode aOldMode(
                pGraphics->GetPixelOffsetMode() );
            pGraphics->SetPixelOffsetMode( Gdiplus::PixelOffsetModeNone );

            const bool bIsMiter(rendering::PathJoinType::MITER == strokeAttributes.JoinType);
            const bool bIsNone(rendering::PathJoinType::NONE == strokeAttributes.JoinType);

            if(bIsMiter)
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
            if(!bIsNone)
                aPen.SetLineJoin( gdiJoinFromJoin(strokeAttributes.JoinType) );

            GraphicsPathSharedPtr pPath( tools::graphicsPathFromXPolyPolygon2D( xPolyPolygon, bIsNone ) );

            
            Gdiplus::Status hr = pGraphics->DrawPath( &aPen, pPath.get() );

            pGraphics->SetPixelOffsetMode( aOldMode );

            ENSURE_OR_THROW(
                Gdiplus::Ok == hr,
                "CanvasHelper::strokePolyPolygon(): GDI+ call failed" );
        }

        
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::strokeTexturedPolyPolygon( const rendering::XCanvas*                            /*pCanvas*/,
                                                                                           const uno::Reference< rendering::XPolyPolygon2D >&   /*xPolyPolygon*/,
                                                                                           const rendering::ViewState&                          /*viewState*/,
                                                                                           const rendering::RenderState&                        /*renderState*/,
                                                                                           const uno::Sequence< rendering::Texture >&           /*textures*/,
                                                                                           const rendering::StrokeAttributes&                   /*strokeAttributes*/ )
    {
        
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
        
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XPolyPolygon2D >   CanvasHelper::queryStrokeShapes( const rendering::XCanvas*                            /*pCanvas*/,
                                                                                   const uno::Reference< rendering::XPolyPolygon2D >&   /*xPolyPolygon*/,
                                                                                   const rendering::ViewState&                          /*viewState*/,
                                                                                   const rendering::RenderState&                        /*renderState*/,
                                                                                   const rendering::StrokeAttributes&                   /*strokeAttributes*/ )
    {
        
        return uno::Reference< rendering::XPolyPolygon2D >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::fillPolyPolygon( const rendering::XCanvas*                          /*pCanvas*/,
                                                                                 const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon,
                                                                                 const rendering::ViewState&                        viewState,
                                                                                 const rendering::RenderState&                      renderState )
    {
        ENSURE_OR_THROW( xPolyPolygon.is(),
                          "CanvasHelper::fillPolyPolygon: polygon is NULL");

        if( needOutput() )
        {
            GraphicsSharedPtr pGraphics( mpGraphicsProvider->getGraphics() );

            setupGraphicsState( pGraphics, viewState, renderState );

            Gdiplus::SolidBrush aBrush(
                tools::sequenceToArgb(renderState.DeviceColor));

            GraphicsPathSharedPtr pPath( tools::graphicsPathFromXPolyPolygon2D( xPolyPolygon ) );

            
            ENSURE_OR_THROW( Gdiplus::Ok == pGraphics->FillPath( &aBrush, pPath.get() ),
                             "CanvasHelper::fillPolyPolygon(): GDI+ call failed  " );
        }

        
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::fillTextureMappedPolyPolygon( const rendering::XCanvas*                             /*pCanvas*/,
                                                                                              const uno::Reference< rendering::XPolyPolygon2D >&    /*xPolyPolygon*/,
                                                                                              const rendering::ViewState&                           /*viewState*/,
                                                                                              const rendering::RenderState&                         /*renderState*/,
                                                                                              const uno::Sequence< rendering::Texture >&            /*textures*/,
                                                                                              const uno::Reference< geometry::XMapping2D >&         /*xMapping*/ )
    {
        
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
        
        return uno::Sequence< rendering::FontInfo >();
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::drawText( const rendering::XCanvas*                         /*pCanvas*/,
                                                                          const rendering::StringContext&                   text,
                                                                          const uno::Reference< rendering::XCanvasFont >&   xFont,
                                                                          const rendering::ViewState&                       viewState,
                                                                          const rendering::RenderState&                     renderState,
                                                                          sal_Int8                                          /*textDirection*/ )
    {
        ENSURE_OR_THROW( xFont.is(),
                          "CanvasHelper::drawText: font is NULL");

        if( needOutput() )
        {
            GraphicsSharedPtr pGraphics( mpGraphicsProvider->getGraphics() );

            setupGraphicsState( pGraphics, viewState, renderState );

            Gdiplus::SolidBrush aBrush(
                Gdiplus::Color(
                    tools::sequenceToArgb(renderState.DeviceColor)));

            CanvasFont::ImplRef pFont(
                tools::canvasFontFromXFont(xFont) );

            
            
            Gdiplus::PointF aPoint( 0.0,
                                    static_cast<Gdiplus::REAL>(-(pFont->getFont()->GetSize()*
                                                                 pFont->getCellAscent() /
                                                                 pFont->getEmHeight())) );

            
            
            
            
            

            
            
            ENSURE_OR_THROW(
                Gdiplus::Ok == pGraphics->DrawString( reinterpret_cast<LPCWSTR>(
                                                          text.Text.copy( text.StartPosition,
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
        ENSURE_OR_THROW( xLayoutetText.is(),
                          "CanvasHelper::drawTextLayout: layout is NULL");

        if( needOutput() )
        {
            TextLayout* pTextLayout =
                dynamic_cast< TextLayout* >( xLayoutetText.get() );

            ENSURE_OR_THROW( pTextLayout,
                                "CanvasHelper::drawTextLayout(): TextLayout not compatible with this canvas" );

            pTextLayout->draw( mpGraphicsProvider->getGraphics(),
                               viewState,
                               renderState,
                               maOutputOffset,
                               mpDevice,
                               false );
        }

        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::drawBitmap( const rendering::XCanvas*                   /*pCanvas*/,
                                                                            const uno::Reference< rendering::XBitmap >& xBitmap,
                                                                            const rendering::ViewState&                 viewState,
                                                                            const rendering::RenderState&               renderState )
    {
        ENSURE_OR_THROW( xBitmap.is(),
                          "CanvasHelper::drawBitmap: bitmap is NULL");

        if( needOutput() )
        {
            
            
            
            
            BitmapSharedPtr pGdiBitmap;
            BitmapProvider* pBitmap = dynamic_cast< BitmapProvider* >(xBitmap.get());
            if( pBitmap )
            {
                IBitmapSharedPtr pDXBitmap( pBitmap->getBitmap() );
                if( pDXBitmap )
                    pGdiBitmap = pDXBitmap->getBitmap();
            }

            GraphicsSharedPtr pGraphics( mpGraphicsProvider->getGraphics() );
            setupGraphicsState( pGraphics, viewState, renderState );

            if( pGdiBitmap )
                tools::drawGdiPlusBitmap(pGraphics,pGdiBitmap);
            else
                tools::drawVCLBitmapFromXBitmap(pGraphics,
                                                xBitmap);
        }

        
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::drawBitmapModulated( const rendering::XCanvas*                      pCanvas,
                                                                                     const uno::Reference< rendering::XBitmap >&    xBitmap,
                                                                                     const rendering::ViewState&                    viewState,
                                                                                     const rendering::RenderState&                  renderState )
    {
        ENSURE_OR_THROW( xBitmap.is(),
                          "CanvasHelper::drawBitmap: bitmap is NULL");

        
        if( renderState.DeviceColor.getLength() < 3 )
            return drawBitmap( pCanvas, xBitmap, viewState, renderState );

        if( needOutput() )
        {
            GraphicsSharedPtr pGraphics( mpGraphicsProvider->getGraphics() );

            setupGraphicsState( pGraphics, viewState, renderState );

            BitmapSharedPtr pBitmap( tools::bitmapFromXBitmap( xBitmap ) );
            Gdiplus::Rect aRect( 0, 0,
                                 pBitmap->GetWidth(),
                                 pBitmap->GetHeight() );

            
            
            const rendering::ARGBColor& rARGBColor(
                mpDevice->getDeviceColorSpace()->convertToARGB(renderState.DeviceColor)[0]);

            Gdiplus::ImageAttributes aImgAttr;
            tools::setModulateImageAttributes( aImgAttr,
                                               rARGBColor.Red,
                                               rARGBColor.Green,
                                               rARGBColor.Blue,
                                               rARGBColor.Alpha );

            ENSURE_OR_THROW(
                Gdiplus::Ok == pGraphics->DrawImage( pBitmap.get(),
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

        
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XGraphicDevice > CanvasHelper::getDevice()
    {
        return uno::Reference< rendering::XGraphicDevice >(mpDevice);
    }

    
    

    Gdiplus::CompositingMode CanvasHelper::calcCompositingMode( sal_Int8 nMode )
    {
        Gdiplus::CompositingMode aRet( Gdiplus::CompositingModeSourceOver );

        switch( nMode )
        {
            case rendering::CompositeOperation::OVER:
                
            case rendering::CompositeOperation::CLEAR:
                aRet = Gdiplus::CompositingModeSourceOver;
                break;

            case rendering::CompositeOperation::SOURCE:
                aRet = Gdiplus::CompositingModeSourceCopy;
                break;

            case rendering::CompositeOperation::DESTINATION:
                
            case rendering::CompositeOperation::UNDER:
                
            case rendering::CompositeOperation::INSIDE:
                
            case rendering::CompositeOperation::INSIDE_REVERSE:
                
            case rendering::CompositeOperation::OUTSIDE:
                
            case rendering::CompositeOperation::OUTSIDE_REVERSE:
                
            case rendering::CompositeOperation::ATOP:
                
            case rendering::CompositeOperation::ATOP_REVERSE:
                
            case rendering::CompositeOperation::XOR:
                
            case rendering::CompositeOperation::ADD:
                
            case rendering::CompositeOperation::SATURATE:
                
                aRet = Gdiplus::CompositingModeSourceOver;
                break;

            default:
                ENSURE_OR_THROW( false, "CanvasHelper::calcCompositingMode: unexpected mode" );
                break;
        }

        return aRet;
    }

    void CanvasHelper::setupGraphicsState( GraphicsSharedPtr&            rGraphics,
                                           const rendering::ViewState&   viewState,
                                           const rendering::RenderState& renderState )
    {
        ENSURE_OR_THROW( needOutput(),
                          "CanvasHelper::setupGraphicsState: primary graphics invalid" );
        ENSURE_OR_THROW( mpDevice,
                          "CanvasHelper::setupGraphicsState: reference device invalid" );

        
        ::basegfx::B2DHomMatrix aTransform;
        ::canvas::tools::getViewStateTransform(aTransform, viewState);

        
        if( !maOutputOffset.equalZero() )
        {
            const basegfx::B2DHomMatrix aOutputOffset(basegfx::tools::createTranslateB2DHomMatrix(
                maOutputOffset.getX(), maOutputOffset.getY()));
            aTransform = aOutputOffset * aTransform;
        }

        Gdiplus::Matrix aMatrix;
        tools::gdiPlusMatrixFromB2DHomMatrix( aMatrix, aTransform );

        ENSURE_OR_THROW(
            Gdiplus::Ok == rGraphics->SetTransform( &aMatrix ),
            "CanvasHelper::setupGraphicsState(): Failed to set GDI+ transformation" );

        
        ENSURE_OR_THROW(
            Gdiplus::Ok == rGraphics->ResetClip(),
            "CanvasHelper::setupGraphicsState(): Failed to reset GDI+ clip" );

        if( viewState.Clip.is() )
        {
            GraphicsPathSharedPtr aClipPath( tools::graphicsPathFromXPolyPolygon2D( viewState.Clip ) );

            
            
            
            ENSURE_OR_THROW(
                Gdiplus::Ok == rGraphics->SetClip( aClipPath.get(),
                                                   Gdiplus::CombineModeIntersect ),
                "CanvasHelper::setupGraphicsState(): Cannot set GDI+ clip" );
        }

        
        
        ::canvas::tools::mergeViewAndRenderTransform(aTransform,
                                                     viewState,
                                                     renderState);

        
        if( !maOutputOffset.equalZero() )
        {
            const basegfx::B2DHomMatrix aOutputOffset(basegfx::tools::createTranslateB2DHomMatrix(
                maOutputOffset.getX(), maOutputOffset.getY()));
            aTransform = aOutputOffset * aTransform;
        }

        tools::gdiPlusMatrixFromB2DHomMatrix( aMatrix, aTransform );

        ENSURE_OR_THROW(
            Gdiplus::Ok == rGraphics->SetTransform( &aMatrix ),
            "CanvasHelper::setupGraphicsState(): Cannot set GDI+ transformation" );

        if( renderState.Clip.is() )
        {
            GraphicsPathSharedPtr aClipPath( tools::graphicsPathFromXPolyPolygon2D( renderState.Clip ) );

            
            
            
            ENSURE_OR_THROW(
                Gdiplus::Ok == rGraphics->SetClip( aClipPath.get(),
                                                   Gdiplus::CombineModeIntersect ),
                "CanvasHelper::setupGraphicsState(): Cannot set GDI+ clip" );
        }

        
        const Gdiplus::CompositingMode eCompositing( calcCompositingMode( renderState.CompositeOperation ) );
        ENSURE_OR_THROW(
            Gdiplus::Ok == rGraphics->SetCompositingMode( eCompositing ),
            "CanvasHelper::setupGraphicsState(): Cannot set GDI* compositing mode)" );
    }

    void CanvasHelper::flush() const
    {
        if( needOutput() )
            mpGraphicsProvider->getGraphics()->Flush( Gdiplus::FlushIntentionSync );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
