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

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/canvastools.hxx>
#include <vcl/metric.hxx>
#include <vcl/virdev.hxx>

#include <canvas/canvastools.hxx>

#include "cairo_canvasfont.hxx"
#include "cairo_canvashelper.hxx"
#include "cairo_textlayout.hxx"

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
        // TODO
        return uno::Sequence< rendering::FontInfo >();
    }

    static bool
    setupFontTransform( ::OutputDevice&                 rOutDev,
                        ::Point&                        o_rPoint,
                        vcl::Font&                      io_rVCLFont,
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

        // query font metric _before_ tampering with width and height
        if( !::rtl::math::approxEqual(aScale.getX(), aScale.getY()) )
        {
            // retrieve true font width
            const sal_Int32 nFontWidth( rOutDev.GetFontMetric( io_rVCLFont ).GetWidth() );

            const sal_Int32 nScaledFontWidth( ::basegfx::fround(nFontWidth * aScale.getX()) );

            if( !nScaledFontWidth )
            {
                // scale is smaller than one pixel - disable text
                // output altogether
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

        // TODO(F2): Missing functionality in VCL: shearing
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
                                      const_cast<rendering::XCanvas*>(pOwner), // only for refcount
                                      2,
                                      eColorType == IGNORE_COLOR ? 0 : 3 );

        int nTransparency(0);

        // TODO(P2): Don't change clipping all the time, maintain current clip
        // state and change only when update is necessary
        ::canvas::tools::clipOutDev(viewState, renderState, rOutDev);

        if( eColorType != IGNORE_COLOR )
        {
            Color aColor( COL_WHITE );

            if( renderState.DeviceColor.getLength() > 2 )
            {
                aColor = vcl::unotools::stdColorSpaceSequenceToColor( renderState.DeviceColor );
            }

            // extract alpha, and make color opaque
            // afterwards. Otherwise, OutputDevice won't draw anything
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

    class DeviceSettingsGuard
    {
    private:
        VclPtr<OutputDevice> mpVirtualDevice;
        cairo_t *mpCairo;
        bool mbMappingWasEnabled;
    public:
        DeviceSettingsGuard(OutputDevice *pVirtualDevice, cairo_t *pCairo)
            : mpVirtualDevice(pVirtualDevice)
            , mpCairo(pCairo)
            , mbMappingWasEnabled(mpVirtualDevice->IsMapModeEnabled())
        {
            cairo_save(mpCairo);
            mpVirtualDevice->Push();
            mpVirtualDevice->EnableMapMode(false);
        }

        ~DeviceSettingsGuard()
        {
            mpVirtualDevice->EnableMapMode(mbMappingWasEnabled);
            mpVirtualDevice->Pop();
            cairo_restore(mpCairo);
        }
    };

    bool setupTextOutput( OutputDevice&                                     rOutDev,
                          const rendering::XCanvas*                         pOwner,
                          ::Point&                                          o_rOutPos,
                          const rendering::ViewState&                       viewState,
                          const rendering::RenderState&                     renderState,
                          const uno::Reference< rendering::XCanvasFont >&   xFont   )
    {
        setupOutDevState( rOutDev, pOwner, viewState, renderState, TEXT_COLOR );

        CanvasFont* pFont = dynamic_cast< CanvasFont* >( xFont.get() );

        ENSURE_ARG_OR_THROW( pFont,
                         "CanvasHelper::setupTextOutput(): Font not compatible with this canvas" );

        vcl::Font aVCLFont = pFont->getVCLFont();

        Color aColor( COL_BLACK );

        if( renderState.DeviceColor.getLength() > 2 )
        {
            aColor = vcl::unotools::stdColorSpaceSequenceToColor(renderState.DeviceColor );
        }

        // setup font color
        aVCLFont.SetColor( aColor );
        aVCLFont.SetFillColor( aColor );

        // no need to replicate this for mp2ndOutDev, we're modifying only aVCLFont here.
        if( !setupFontTransform( rOutDev, o_rOutPos, aVCLFont, viewState, renderState ) )
            return false;

        rOutDev.SetFont( aVCLFont );

        return true;
    }

    //set the clip of the rOutDev to the cairo surface
    void CanvasHelper::clip_cairo_from_dev(::OutputDevice& rOutDev)
    {
        vcl::Region aRegion(rOutDev.GetClipRegion());
        if (!aRegion.IsEmpty() && !aRegion.IsNull())
        {
            doPolyPolygonImplementation(aRegion.GetAsB2DPolyPolygon(), Clip, mpCairo.get(),
                                        nullptr, mpSurfaceProvider, rendering::FillRule_EVEN_ODD);
        }
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
            DeviceSettingsGuard aGuard(mpVirtualDevice.get(), mpCairo.get());

#if defined CAIRO_HAS_WIN32_SURFACE
            // FIXME: Some kind of work-araound...
            cairo_rectangle (mpCairo.get(), 0, 0, 0, 0);
            cairo_fill(mpCairo.get());
#endif
            ::Point aOutpos;
            if( !setupTextOutput( *mpVirtualDevice.get(), pOwner, aOutpos, viewState, renderState, xFont ) )
                return uno::Reference< rendering::XCachedPrimitive >(nullptr); // no output necessary

                // change text direction and layout mode
            ComplexTextLayoutMode nLayoutMode(TEXT_LAYOUT_DEFAULT);
            switch( textDirection )
            {
                case rendering::TextDirection::WEAK_LEFT_TO_RIGHT:
                    // FALLTHROUGH intended
                case rendering::TextDirection::STRONG_LEFT_TO_RIGHT:
                    nLayoutMode |= TEXT_LAYOUT_BIDI_STRONG;
                    nLayoutMode |= TEXT_LAYOUT_TEXTORIGIN_LEFT;
                    break;

                case rendering::TextDirection::WEAK_RIGHT_TO_LEFT:
                    nLayoutMode |= TEXT_LAYOUT_BIDI_RTL;
                    SAL_FALLTHROUGH;
                case rendering::TextDirection::STRONG_RIGHT_TO_LEFT:
                    nLayoutMode |= TEXT_LAYOUT_BIDI_RTL | TEXT_LAYOUT_BIDI_STRONG;
                    nLayoutMode |= TEXT_LAYOUT_TEXTORIGIN_RIGHT;
                    break;
            }

            // TODO(F2): alpha
            mpVirtualDevice->SetLayoutMode( nLayoutMode );

            clip_cairo_from_dev(*mpVirtualDevice);

            OSL_TRACE(":cairocanvas::CanvasHelper::drawText(O,t,f,v,r,d): %s", OUStringToOString( text.Text.copy( text.StartPosition, text.Length ),
                                                                                                         RTL_TEXTENCODING_UTF8 ).getStr());

            rtl::Reference< TextLayout > pTextLayout( new TextLayout(text, textDirection, 0, CanvasFont::Reference(dynamic_cast< CanvasFont* >( xFont.get() )), mpSurfaceProvider) );
            pTextLayout->draw(mpCairo, *mpVirtualDevice, aOutpos, viewState, renderState);
        }

        return uno::Reference< rendering::XCachedPrimitive >(nullptr);
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
                DeviceSettingsGuard aGuard(mpVirtualDevice.get(), mpCairo.get());

#if defined CAIRO_HAS_WIN32_SURFACE
                // FIXME: Some kind of work-araound...
                cairo_rectangle(mpCairo.get(), 0, 0, 0, 0);
                cairo_fill(mpCairo.get());
#endif
                // TODO(T3): Race condition. We're taking the font
                // from xLayoutedText, and then calling draw() at it,
                // without exclusive access. Move setupTextOutput(),
                // e.g. to impltools?

                ::Point aOutpos;
                if( !setupTextOutput( *mpVirtualDevice, pOwner, aOutpos, viewState, renderState, xLayoutedText->getFont() ) )
                    return uno::Reference< rendering::XCachedPrimitive >(nullptr); // no output necessary

                clip_cairo_from_dev(*mpVirtualDevice);

                // TODO(F2): What about the offset scalings?
                pTextLayout->draw(mpCairo, *mpVirtualDevice, aOutpos, viewState, renderState);
            }
        }
        else
        {
            ENSURE_ARG_OR_THROW( false,
                             "CanvasHelper::drawTextLayout(): TextLayout not compatible with this canvas" );
        }

        return uno::Reference< rendering::XCachedPrimitive >(nullptr);
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
