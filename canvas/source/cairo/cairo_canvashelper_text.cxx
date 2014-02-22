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
#include <canvas/canvastools.hxx>
#include <tools/diagnose_ex.h>

#include <vcl/virdev.hxx>
#include <vcl/metric.hxx>
#include <vcl/canvastools.hxx>

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/tools/canvastools.hxx>

#include "cairo_canvasfont.hxx"
#include "cairo_textlayout.hxx"
#include "cairo_canvashelper.hxx"

using namespace ::cairo;
using namespace ::com::sun::star;

namespace cairocanvas
{
    enum ColorType
    {
        LINE_COLOR, FILL_COLOR, TEXT_COLOR, IGNORE_COLOR
    };

    uno::Reference< rendering::XCanvasFont > CanvasHelper::createFont( const rendering::XCanvas*                    ,
                                                                       const rendering::FontRequest&                fontRequest,
                                                                       const uno::Sequence< beans::PropertyValue >& extraFontProperties,
                                                                       const geometry::Matrix2D&                    fontMatrix )
    {
        return uno::Reference< rendering::XCanvasFont >( new CanvasFont( fontRequest, extraFontProperties, fontMatrix, mpSurfaceProvider ));
    }

    uno::Sequence< rendering::FontInfo > CanvasHelper::queryAvailableFonts( const rendering::XCanvas*                       ,
                                                                            const rendering::FontInfo&                      /*aFilter*/,
                                                                            const uno::Sequence< beans::PropertyValue >&    /*aFontProperties*/ )
    {
        
        return uno::Sequence< rendering::FontInfo >();
    }

    static bool
    setupFontTransform( ::OutputDevice&                 rOutDev,
                        ::Point&                        o_rPoint,
                        ::Font&                         io_rVCLFont,
                        const rendering::ViewState&     rViewState,
                        const rendering::RenderState&   rRenderState )
    {
        ::basegfx::B2DHomMatrix aMatrix;

        ::canvas::tools::mergeViewAndRenderTransform(aMatrix,
                                                     rViewState,
                                                     rRenderState);

        ::basegfx::B2DTuple aScale;
        ::basegfx::B2DTuple aTranslate;
        double nRotate, nShearX;

        aMatrix.decompose( aScale, aTranslate, nRotate, nShearX );

        
        if( !::rtl::math::approxEqual(aScale.getX(), aScale.getY()) )
        {
            
            const sal_Int32 nFontWidth( rOutDev.GetFontMetric( io_rVCLFont ).GetWidth() );

            const sal_Int32 nScaledFontWidth( ::basegfx::fround(nFontWidth * aScale.getX()) );

            if( !nScaledFontWidth )
            {
                
                
                return false;
            }

            io_rVCLFont.SetWidth( nScaledFontWidth );
        }

        if( !::rtl::math::approxEqual(aScale.getY(), 1.0) )
        {
            const sal_Int32 nFontHeight( io_rVCLFont.GetHeight() );
            io_rVCLFont.SetHeight( ::basegfx::fround(nFontHeight * aScale.getY()) );
        }

        io_rVCLFont.SetOrientation( static_cast< short >( ::basegfx::fround(-fmod(nRotate, 2*M_PI)*(1800.0/M_PI)) ) );

        
        o_rPoint.X() = ::basegfx::fround(aTranslate.getX());
        o_rPoint.Y() = ::basegfx::fround(aTranslate.getY());

        return true;
    }

    static int
    setupOutDevState( OutputDevice&                 rOutDev,
                      const rendering::XCanvas*     pOwner,
                      const rendering::ViewState&   viewState,
                      const rendering::RenderState& renderState,
                      ColorType                     eColorType )
    {
        ::canvas::tools::verifyInput( renderState,
                                      BOOST_CURRENT_FUNCTION,
                                      const_cast<rendering::XCanvas*>(pOwner), 
                                      2,
                                      eColorType == IGNORE_COLOR ? 0 : 3 );

        int nTransparency(0);

        
        

        
        

        Region aClipRegion;

        if( viewState.Clip.is() )
        {
            ::basegfx::B2DPolyPolygon aClipPoly(
                ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(
                    viewState.Clip) );

            if( aClipPoly.count() )
            {
                
                ::basegfx::B2DHomMatrix aMatrix;
                aClipPoly.transform(
                    ::basegfx::unotools::homMatrixFromAffineMatrix( aMatrix,
                                                                    viewState.AffineTransform ) );

                aClipRegion = Region::GetRegionFromPolyPolygon( ::PolyPolygon( aClipPoly ) );
            }
        }

        if( renderState.Clip.is() )
        {
            ::basegfx::B2DPolyPolygon aClipPoly(
                ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(
                    renderState.Clip) );

            ::basegfx::B2DHomMatrix aMatrix;
            aClipPoly.transform(
                ::canvas::tools::mergeViewAndRenderTransform( aMatrix,
                                                              viewState,
                                                              renderState ) );

            if( aClipPoly.count() )
            {
                
                Region aRegion = Region::GetRegionFromPolyPolygon( ::PolyPolygon( aClipPoly ) );

                if( aClipRegion.IsEmpty() )
                    aClipRegion = aRegion;
                else
                    aClipRegion.Intersect( aRegion );
            }
            else
            {
                
                aClipRegion.SetEmpty();
            }
        }

        
        
        
        
        
        
        if( aClipRegion.IsEmpty() )
        {
            rOutDev.SetClipRegion();
        }
        else
        {
            rOutDev.SetClipRegion( aClipRegion );
        }

        if( eColorType != IGNORE_COLOR )
        {
            Color aColor( COL_WHITE );

            if( renderState.DeviceColor.getLength() > 2 )
            {
                aColor = ::vcl::unotools::stdColorSpaceSequenceToColor( renderState.DeviceColor );
            }

            
            
            nTransparency = aColor.GetTransparency();
            aColor.SetTransparency(0);

            switch( eColorType )
            {
                case LINE_COLOR:
                    rOutDev.SetLineColor( aColor );
                    rOutDev.SetFillColor();

                    break;

                case FILL_COLOR:
                    rOutDev.SetFillColor( aColor );
                    rOutDev.SetLineColor();

                    break;

                case TEXT_COLOR:
                    rOutDev.SetTextColor( aColor );

                    break;

                default:
                    ENSURE_OR_THROW( false,
                                      "CanvasHelper::setupOutDevState(): Unexpected color type");
                    break;
            }
        }

        return nTransparency;
    }

    bool setupTextOutput( OutputDevice&                                     rOutDev,
                          const rendering::XCanvas*                         pOwner,
                          ::Point&                                          o_rOutPos,
                          const rendering::ViewState&                       viewState,
                          const rendering::RenderState&                     renderState,
                          const uno::Reference< rendering::XCanvasFont >&   xFont   )
    {
        setupOutDevState( rOutDev, pOwner, viewState, renderState, TEXT_COLOR );

        ::Font aVCLFont;

        CanvasFont* pFont = dynamic_cast< CanvasFont* >( xFont.get() );

        ENSURE_ARG_OR_THROW( pFont,
                         "CanvasHelper::setupTextOutput(): Font not compatible with this canvas" );

        aVCLFont = pFont->getVCLFont();

        Color aColor( COL_BLACK );

        if( renderState.DeviceColor.getLength() > 2 )
        {
            aColor = ::vcl::unotools::stdColorSpaceSequenceToColor(renderState.DeviceColor );
        }

        
        aVCLFont.SetColor( aColor );
        aVCLFont.SetFillColor( aColor );

        
        if( !setupFontTransform( rOutDev, o_rOutPos, aVCLFont, viewState, renderState ) )
            return false;

        rOutDev.SetFont( aVCLFont );


        return true;
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::drawText( const rendering::XCanvas*                         pOwner,
                                                                          const rendering::StringContext&                   text,
                                                                          const uno::Reference< rendering::XCanvasFont >&   xFont,
                                                                          const rendering::ViewState&                       viewState,
                                                                          const rendering::RenderState&                     renderState,
                                                                          sal_Int8                                          textDirection )
    {
#ifdef CAIRO_CANVAS_PERF_TRACE
        struct timespec aTimer;
        mxDevice->startPerfTrace( &aTimer );
#endif

        ENSURE_ARG_OR_THROW( xFont.is(),
                         "CanvasHelper::drawText(): font is NULL");

        if( !mpVirtualDevice )
            mpVirtualDevice = mpSurface->createVirtualDevice();

        if( mpVirtualDevice )
        {
#if defined CAIRO_HAS_WIN32_SURFACE
            
            cairo_rectangle (mpSurface->getCairo().get(), 0, 0, 0, 0);
            cairo_fill(mpSurface->getCairo().get());
#endif
            ::Point aOutpos;
            if( !setupTextOutput( *mpVirtualDevice, pOwner, aOutpos, viewState, renderState, xFont ) )
                return uno::Reference< rendering::XCachedPrimitive >(NULL); 

                
            sal_uLong nLayoutMode(0);
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

            
            mpVirtualDevice->SetLayoutMode( nLayoutMode );

            OSL_TRACE(":cairocanvas::CanvasHelper::drawText(O,t,f,v,r,d): %s", OUStringToOString( text.Text.copy( text.StartPosition, text.Length ),
                                                                                                         RTL_TEXTENCODING_UTF8 ).getStr());

            TextLayout* pTextLayout = new TextLayout(text, textDirection, 0, CanvasFont::Reference(dynamic_cast< CanvasFont* >( xFont.get() )), mpSurfaceProvider);
            pTextLayout->draw( mpSurface, *mpVirtualDevice, aOutpos, viewState, renderState );
        }

        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::drawTextLayout( const rendering::XCanvas*                       pOwner,
                                                                                const uno::Reference< rendering::XTextLayout >& xLayoutedText,
                                                                                const rendering::ViewState&                     viewState,
                                                                                const rendering::RenderState&                   renderState )
    {
        ENSURE_ARG_OR_THROW( xLayoutedText.is(),
                         "CanvasHelper::drawTextLayout(): layout is NULL");

        TextLayout* pTextLayout = dynamic_cast< TextLayout* >( xLayoutedText.get() );

        if( pTextLayout )
        {
            if( !mpVirtualDevice )
                mpVirtualDevice = mpSurface->createVirtualDevice();

            if( mpVirtualDevice )
            {
#if defined CAIRO_HAS_WIN32_SURFACE
                
                cairo_rectangle( mpSurface->getCairo().get(), 0, 0, 0, 0);
                cairo_fill(mpSurface->getCairo().get());
#endif
                
                
                
                

                ::Point aOutpos;
                if( !setupTextOutput( *mpVirtualDevice, pOwner, aOutpos, viewState, renderState, xLayoutedText->getFont() ) )
                    return uno::Reference< rendering::XCachedPrimitive >(NULL); 

                
                pTextLayout->draw( mpSurface, *mpVirtualDevice, aOutpos, viewState, renderState );
            }
        }
        else
        {
            ENSURE_ARG_OR_THROW( false,
                             "CanvasHelper::drawTextLayout(): TextLayout not compatible with this canvas" );
        }

        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
