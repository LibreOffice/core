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

#include <memory>

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <com/sun/star/rendering/FontRequest.hpp>
#include <com/sun/star/rendering/PanoseProportion.hpp>
#include <com/sun/star/rendering/XCanvasFont.hpp>
#include <comphelper/scopeguard.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <tools/color.hxx>
#include <tools/diagnose_ex.h>
#include <tools/poly.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/metric.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/virdev.hxx>

#include <canvas/canvastools.hxx>

#include "dx_bitmap.hxx"
#include "dx_canvasfont.hxx"
#include "dx_impltools.hxx"
#include "dx_textlayout_drawhelper.hxx"

using namespace ::com::sun::star;


namespace dxcanvas
{
    TextLayoutDrawHelper::TextLayoutDrawHelper(
        const uno::Reference< rendering::XGraphicDevice >& xGraphicDevice ) :
            mxGraphicDevice(xGraphicDevice)
    {
    }

    TextLayoutDrawHelper::~TextLayoutDrawHelper()
    {
    }

    void TextLayoutDrawHelper::drawText(
        const GraphicsSharedPtr&               rGraphics,
        const css::rendering::ViewState&       rViewState,
        const css::rendering::RenderState&     rRenderState,
        const ::basegfx::B2ISize&              rOutputOffset,
        const css::rendering::StringContext&   rText,
        const css::uno::Sequence< double >&    rLogicalAdvancements,
        const css::uno::Reference<
            css::rendering::XCanvasFont >&     rCanvasFont,
        const css::geometry::Matrix2D&         rFontMatrix,
        bool                                   bAlphaSurface,
        bool bIsRTL)
    {
        HDC hdc = rGraphics->GetHDC();

        // issue an ReleaseHDC() when leaving the scope
        const ::comphelper::ScopeGuard aGuard(
            [&rGraphics, &hdc]() mutable { rGraphics->ReleaseHDC(hdc); } );

        SystemGraphicsData aSystemGraphicsData;
        aSystemGraphicsData.nSize = sizeof(SystemGraphicsData);
        aSystemGraphicsData.hDC = reinterpret_cast< ::HDC >(hdc);
        ScopedVclPtrInstance<VirtualDevice> xVirtualDevice(&aSystemGraphicsData, Size(1, 1), DeviceFormat::DEFAULT);

        // disable font antialiasing - GDI does not handle alpha
        // surfaces properly.
        if( bAlphaSurface )
            xVirtualDevice->SetAntialiasing(AntialiasingFlags::DisableText);

        if(rText.Length)
        {
            bool test = mxGraphicDevice.is();
            ENSURE_OR_THROW( test,
                              "TextLayoutDrawHelper::drawText(): Invalid GraphicDevice" );

            // set text color. Make sure to remove transparence part first.
            Color aColor( COL_WHITE );

            if( rRenderState.DeviceColor.getLength() > 2 )
                aColor = vcl::unotools::doubleSequenceToColor(
                    rRenderState.DeviceColor,
                    mxGraphicDevice->getDeviceColorSpace());
            aColor.SetTransparency(0);
            xVirtualDevice->SetTextColor(aColor);

            // create the font
            const css::rendering::FontRequest& rFontRequest = rCanvasFont->getFontRequest();
            vcl::Font aFont(
                rFontRequest.FontDescription.FamilyName,
                rFontRequest.FontDescription.StyleName,
                Size( 0, ::basegfx::fround(rFontRequest.CellSize)));

            aFont.SetAlignment( ALIGN_BASELINE );
            aFont.SetCharSet( (rFontRequest.FontDescription.IsSymbolFont==css::util::TriState_YES) ? RTL_TEXTENCODING_SYMBOL : RTL_TEXTENCODING_UNICODE );
            aFont.SetVertical( rFontRequest.FontDescription.IsVertical==css::util::TriState_YES );
            aFont.SetWeight( static_cast<FontWeight>(rFontRequest.FontDescription.FontDescription.Weight) );
            aFont.SetItalic( (rFontRequest.FontDescription.FontDescription.Letterform<=8) ? ITALIC_NONE : ITALIC_NORMAL );
            aFont.SetPitch(
                    rFontRequest.FontDescription.FontDescription.Proportion == rendering::PanoseProportion::MONO_SPACED
                    ? PITCH_FIXED : PITCH_VARIABLE);

            aFont.SetLanguage(LanguageTag::convertToLanguageType(rFontRequest.Locale));

            // setup font color
            aFont.SetColor( aColor );
            aFont.SetFillColor( aColor );

            CanvasFont::ImplRef pFont(tools::canvasFontFromXFont(rCanvasFont));
            if (pFont.is() && pFont->getEmphasisMark())
                aFont.SetEmphasisMark(FontEmphasisMark(pFont->getEmphasisMark()));

            // adjust to stretched font
            if(!::rtl::math::approxEqual(rFontMatrix.m00, rFontMatrix.m11))
            {
                const Size aSize = xVirtualDevice->GetFontMetric( aFont ).GetFontSize();
                const double fDividend( rFontMatrix.m10 + rFontMatrix.m11 );
                double fStretch = rFontMatrix.m00 + rFontMatrix.m01;

                if( !::basegfx::fTools::equalZero( fDividend) )
                    fStretch /= fDividend;

                const sal_Int32 nNewWidth = ::basegfx::fround( aSize.Width() * fStretch );

                aFont.SetAverageFontWidth( nNewWidth );
            }

            // set font
            xVirtualDevice->SetFont(aFont);

            // create world transformation matrix
            ::basegfx::B2DHomMatrix aWorldTransform;
            ::canvas::tools::mergeViewAndRenderTransform(aWorldTransform, rViewState, rRenderState);

            if(!rOutputOffset.equalZero())
            {
                aWorldTransform.translate(rOutputOffset.getX(), rOutputOffset.getY());
            }

            // set ViewState clipping
            if(rViewState.Clip.is())
            {
                ::basegfx::B2DPolyPolygon aClipPoly(dxcanvas::tools::polyPolygonFromXPolyPolygon2D(rViewState.Clip));
                ::basegfx::B2DHomMatrix aMatrix;
                ::basegfx::unotools::homMatrixFromAffineMatrix(aMatrix, rViewState.AffineTransform );

                if(!rOutputOffset.equalZero())
                {
                    aMatrix.translate(rOutputOffset.getX(), rOutputOffset.getY());
                }

                aClipPoly.transform(aMatrix);
                const vcl::Region& rClipRegion = vcl::Region(::tools::PolyPolygon(aClipPoly));
                xVirtualDevice->IntersectClipRegion(rClipRegion);
            }

            if(rRenderState.Clip.is())
            {
                ::basegfx::B2DPolyPolygon aClipPoly(dxcanvas::tools::polyPolygonFromXPolyPolygon2D(rRenderState.Clip));
                aClipPoly.transform(aWorldTransform);
                const vcl::Region& rClipRegion = vcl::Region(::tools::PolyPolygon(aClipPoly));
                xVirtualDevice->IntersectClipRegion(rClipRegion);
            }

            // set world transform
            XFORM aXForm;
            aXForm.eM11 = static_cast<FLOAT>(aWorldTransform.get(0, 0));
            aXForm.eM12 = static_cast<FLOAT>(aWorldTransform.get(1, 0));
            aXForm.eM21 = static_cast<FLOAT>(aWorldTransform.get(0, 1));
            aXForm.eM22 = static_cast<FLOAT>(aWorldTransform.get(1, 1));
            aXForm.eDx = static_cast<FLOAT>(aWorldTransform.get(0, 2));
            aXForm.eDy = static_cast<FLOAT>(aWorldTransform.get(1, 2));

            // TODO(F3): This is NOT supported on 95/98/ME!
            SetGraphicsMode(hdc, GM_ADVANCED);
            SetTextAlign(hdc, TA_BASELINE);
            SetWorldTransform(hdc, &aXForm);

            // use a empty StartPosition for text rendering
            const Point aEmptyPoint(0, 0);

            // create the String
            const OUString aText(rText.Text);

            if( rLogicalAdvancements.getLength() )
            {
                // create the DXArray
                const sal_Int32 nLen( rLogicalAdvancements.getLength() );
                std::unique_ptr<sal_Int32[]> pDXArray( new sal_Int32[nLen] );
                for( sal_Int32 i=0; i<nLen; ++i )
                    pDXArray[i] = basegfx::fround( rLogicalAdvancements[i] );

                // draw the String
                xVirtualDevice->DrawTextArray( aEmptyPoint,
                                              aText,
                                              pDXArray.get(),
                                              rText.StartPosition,
                                              rText.Length,
                                              bIsRTL ? SalLayoutFlags::BiDiRtl : SalLayoutFlags::NONE);
            }
            else
            {
                // draw the String
                xVirtualDevice->DrawText( aEmptyPoint,
                                         aText,
                                         rText.StartPosition,
                                         rText.Length );
            }
        }
    }

    geometry::RealRectangle2D TextLayoutDrawHelper::queryTextBounds( const rendering::StringContext&                    rText,
                                                                     const uno::Sequence< double >&                     rLogicalAdvancements,
                                                                     const uno::Reference< rendering::XCanvasFont >&    rCanvasFont,
                                                                     const geometry::Matrix2D&                          rFontMatrix )
    {
        if(!(rText.Length))
            return geometry::RealRectangle2D();

        // TODO(F1): Fetching default screen DC here, will yield wrong
        // metrics when e.g. formatting for a printer!
        SystemGraphicsData aSystemGraphicsData;
        aSystemGraphicsData.nSize = sizeof(SystemGraphicsData);
        aSystemGraphicsData.hDC = reinterpret_cast< ::HDC >(GetDC( nullptr ));
        ScopedVclPtrInstance<VirtualDevice> xVirtualDevice(&aSystemGraphicsData, Size(1, 1), DeviceFormat::DEFAULT);

        // create the font
        const css::rendering::FontRequest& rFontRequest = rCanvasFont->getFontRequest();
        vcl::Font aFont(
            rFontRequest.FontDescription.FamilyName,
            rFontRequest.FontDescription.StyleName,
            Size( 0, ::basegfx::fround(rFontRequest.CellSize)));

        aFont.SetAlignment( ALIGN_BASELINE );
        aFont.SetCharSet( (rFontRequest.FontDescription.IsSymbolFont==css::util::TriState_YES) ? RTL_TEXTENCODING_SYMBOL : RTL_TEXTENCODING_UNICODE );
        aFont.SetVertical( rFontRequest.FontDescription.IsVertical==css::util::TriState_YES );
        aFont.SetWeight( static_cast<FontWeight>(rFontRequest.FontDescription.FontDescription.Weight) );
        aFont.SetItalic( (rFontRequest.FontDescription.FontDescription.Letterform<=8) ? ITALIC_NONE : ITALIC_NORMAL );
        aFont.SetPitch(
                rFontRequest.FontDescription.FontDescription.Proportion == rendering::PanoseProportion::MONO_SPACED
                ? PITCH_FIXED : PITCH_VARIABLE);

        // adjust to stretched font
        if(!::rtl::math::approxEqual(rFontMatrix.m00, rFontMatrix.m11))
        {
            const Size aSize = xVirtualDevice->GetFontMetric( aFont ).GetFontSize();
            const double fDividend( rFontMatrix.m10 + rFontMatrix.m11 );
            double fStretch = rFontMatrix.m00 + rFontMatrix.m01;

            if( !::basegfx::fTools::equalZero( fDividend) )
                fStretch /= fDividend;

            const sal_Int32 nNewWidth = ::basegfx::fround( aSize.Width() * fStretch );

            aFont.SetAverageFontWidth( nNewWidth );
        }

        CanvasFont::ImplRef pFont(tools::canvasFontFromXFont(rCanvasFont));
        if (pFont.is() && pFont->getEmphasisMark())
            aFont.SetEmphasisMark(FontEmphasisMark(pFont->getEmphasisMark()));

        // set font
        xVirtualDevice->SetFont(aFont);

        // need metrics for Y offset, the XCanvas always renders
        // relative to baseline
        const ::FontMetric& aMetric( xVirtualDevice->GetFontMetric() );

        const sal_Int32 nAboveBaseline( -aMetric.GetInternalLeading() - aMetric.GetAscent() );
        const sal_Int32 nBelowBaseline( aMetric.GetDescent() );

        if( rLogicalAdvancements.getLength() )
        {
            return geometry::RealRectangle2D( 0, nAboveBaseline,
                                              rLogicalAdvancements[ rLogicalAdvancements.getLength()-1 ],
                                              nBelowBaseline );
        }
        else
        {
            return geometry::RealRectangle2D( 0, nAboveBaseline,
                                              xVirtualDevice->GetTextWidth(
                                                  rText.Text,
                                                  ::canvas::tools::numeric_cast<sal_uInt16>(rText.StartPosition),
                                                  ::canvas::tools::numeric_cast<sal_uInt16>(rText.Length) ),
                                              nBelowBaseline );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
