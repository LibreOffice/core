/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <tools/poly.hxx>

#include <vcl/metric.hxx>
#include <vcl/virdev.hxx>
#include <vcl/metric.hxx>
#include <vcl/canvastools.hxx>
#include <tools/diagnose_ex.h>

#include <boost/scoped_array.hpp>
#include <boost/bind.hpp>
#include <com/sun/star/rendering/FontRequest.hpp>
#include <com/sun/star/rendering/XCanvasFont.hpp>
#include <comphelper/sequence.hxx>
#include <comphelper/scopeguard.hxx>
#include <tools/color.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <canvas/canvastools.hxx>
#include <canvas/debug.hxx>
#include "dx_impltools.hxx"
#include <vcl/sysdata.hxx>
#include <i18npool/mslangid.hxx>
#include "dx_textlayout_drawhelper.hxx"
#include "dx_bitmap.hxx"
#include "dx_canvasfont.hxx"

class ::com::sun::star::rendering::XCanvasFont;

using namespace ::com::sun::star;


//////////////////////////////////////////////////////////////////////////////

namespace dxcanvas
{
    class DXBitmap;
    TextLayoutDrawHelper::TextLayoutDrawHelper(
        const uno::Reference< rendering::XGraphicDevice >& xGraphicDevice ) :
            mxGraphicDevice(xGraphicDevice)
    {
    }

    TextLayoutDrawHelper::~TextLayoutDrawHelper()
    {
    }

    void TextLayoutDrawHelper::drawText(
        const GraphicsSharedPtr&                            rGraphics,
        const ::com::sun::star::rendering::ViewState&       rViewState,
        const ::com::sun::star::rendering::RenderState&     rRenderState,
        const ::basegfx::B2ISize&                           rOutputOffset,
        const ::com::sun::star::rendering::StringContext&   rText,
        const ::com::sun::star::uno::Sequence< double >&    rLogicalAdvancements,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::rendering::XCanvasFont >&     rCanvasFont,
        const ::com::sun::star::geometry::Matrix2D&         rFontMatrix,
        bool                                                bAlphaSurface )
    {
        HDC hdc = rGraphics->GetHDC();

        // issue an ReleaseHDC() when leaving the scope
        const ::comphelper::ScopeGuard aGuard(
            boost::bind( &Gdiplus::Graphics::ReleaseHDC,
                         rGraphics.get(),
                         hdc ));

        SystemGraphicsData aSystemGraphicsData;
        aSystemGraphicsData.nSize = sizeof(SystemGraphicsData);
        aSystemGraphicsData.hDC = reinterpret_cast< ::HDC >(hdc);
        VirtualDevice aVirtualDevice(&aSystemGraphicsData, 0);

        // disable font antialiasing - GDI does not handle alpha
        // surfaces properly.
        if( bAlphaSurface )
            aVirtualDevice.SetAntialiasing(ANTIALIASING_DISABLE_TEXT);

        if(rText.Length)
        {
            sal_Bool test = mxGraphicDevice.is();
            ENSURE_OR_THROW( test,
                              "TextLayoutDrawHelper::drawText(): Invalid GraphicDevice" );

            // set text color. Make sure to remove transparence part first.
            Color aColor( COL_WHITE );

            if( rRenderState.DeviceColor.getLength() > 2 )
                aColor = ::vcl::unotools::doubleSequenceToColor(
                    rRenderState.DeviceColor,
                    mxGraphicDevice->getDeviceColorSpace());
            aColor.SetTransparency(0);
            aVirtualDevice.SetTextColor(aColor);

            // create the font
            const ::com::sun::star::rendering::FontRequest& rFontRequest = rCanvasFont->getFontRequest();
            Font aFont(
                rFontRequest.FontDescription.FamilyName,
                rFontRequest.FontDescription.StyleName,
                Size( 0, ::basegfx::fround(rFontRequest.CellSize)));

            aFont.SetAlign( ALIGN_BASELINE );
            aFont.SetCharSet( (rFontRequest.FontDescription.IsSymbolFont==com::sun::star::util::TriState_YES) ? RTL_TEXTENCODING_SYMBOL : RTL_TEXTENCODING_UNICODE );
            aFont.SetVertical( (rFontRequest.FontDescription.IsVertical==com::sun::star::util::TriState_YES) ? sal_True : sal_False );
            aFont.SetWeight( static_cast<FontWeight>(rFontRequest.FontDescription.FontDescription.Weight) );
            aFont.SetItalic( (rFontRequest.FontDescription.FontDescription.Letterform<=8) ? ITALIC_NONE : ITALIC_NORMAL );

            aFont.SetLanguage(MsLangId::convertLocaleToLanguage(rFontRequest.Locale));

            // setup font color
            aFont.SetColor( aColor );
            aFont.SetFillColor( aColor );

            // adjust to stretched font
            if(!::rtl::math::approxEqual(rFontMatrix.m00, rFontMatrix.m11))
            {
                const Size aSize = aVirtualDevice.GetFontMetric( aFont ).GetSize();
                const double fDividend( rFontMatrix.m10 + rFontMatrix.m11 );
                double fStretch = (rFontMatrix.m00 + rFontMatrix.m01);

                if( !::basegfx::fTools::equalZero( fDividend) )
                    fStretch /= fDividend;

                const sal_Int32 nNewWidth = ::basegfx::fround( aSize.Width() * fStretch );

                aFont.SetWidth( nNewWidth );
            }

            // set font
            aVirtualDevice.SetFont(aFont);

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
                const Region& rClipRegion = Region(PolyPolygon(aClipPoly));
                aVirtualDevice.IntersectClipRegion(rClipRegion);
            }

            if(rRenderState.Clip.is())
            {
                ::basegfx::B2DPolyPolygon aClipPoly(dxcanvas::tools::polyPolygonFromXPolyPolygon2D(rRenderState.Clip));
                aClipPoly.transform(aWorldTransform);
                const Region& rClipRegion = Region(PolyPolygon(aClipPoly));
                aVirtualDevice.IntersectClipRegion(rClipRegion);
            }

            // set world transform
            XFORM aXForm;
            aXForm.eM11 = (FLOAT)aWorldTransform.get(0, 0);
            aXForm.eM12 = (FLOAT)aWorldTransform.get(1, 0);
            aXForm.eM21 = (FLOAT)aWorldTransform.get(0, 1);
            aXForm.eM22 = (FLOAT)aWorldTransform.get(1, 1);
            aXForm.eDx = (FLOAT)aWorldTransform.get(0, 2);
            aXForm.eDy = (FLOAT)aWorldTransform.get(1, 2);

            // TODO(F3): This is NOT supported on 95/98/ME!
            SetGraphicsMode(hdc, GM_ADVANCED);
            SetTextAlign(hdc, TA_BASELINE);
            SetWorldTransform(hdc, &aXForm);

            // use a empty StartPosition for text rendering
            const Point aEmptyPoint(0, 0);

            // create the String
            const String aText(rText.Text.getStr());

            if( rLogicalAdvancements.getLength() )
            {
                // create the DXArray
                const sal_Int32 nLen( rLogicalAdvancements.getLength() );
                ::boost::scoped_array<sal_Int32> pDXArray( new sal_Int32[nLen] );
                for( sal_Int32 i=0; i<nLen; ++i )
                    pDXArray[i] = basegfx::fround( rLogicalAdvancements[i] );

                // draw the String
                aVirtualDevice.DrawTextArray( aEmptyPoint,
                                              aText,
                                              pDXArray.get(),
                                              (xub_StrLen)rText.StartPosition,
                                              (xub_StrLen)rText.Length );
            }
            else
            {
                // draw the String
                aVirtualDevice.DrawText( aEmptyPoint,
                                         aText,
                                         (xub_StrLen)rText.StartPosition,
                                         (xub_StrLen)rText.Length );
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
        aSystemGraphicsData.hDC = reinterpret_cast< ::HDC >(GetDC( NULL ));
        VirtualDevice aVirtualDevice(&aSystemGraphicsData, 0);

        // create the font
        const ::com::sun::star::rendering::FontRequest& rFontRequest = rCanvasFont->getFontRequest();
        Font aFont(
            rFontRequest.FontDescription.FamilyName,
            rFontRequest.FontDescription.StyleName,
            Size( 0, ::basegfx::fround(rFontRequest.CellSize)));

        aFont.SetAlign( ALIGN_BASELINE );
        aFont.SetCharSet( (rFontRequest.FontDescription.IsSymbolFont==com::sun::star::util::TriState_YES) ? RTL_TEXTENCODING_SYMBOL : RTL_TEXTENCODING_UNICODE );
        aFont.SetVertical( (rFontRequest.FontDescription.IsVertical==com::sun::star::util::TriState_YES) ? sal_True : sal_False );
        aFont.SetWeight( static_cast<FontWeight>(rFontRequest.FontDescription.FontDescription.Weight) );
        aFont.SetItalic( (rFontRequest.FontDescription.FontDescription.Letterform<=8) ? ITALIC_NONE : ITALIC_NORMAL );

        // adjust to stretched font
        if(!::rtl::math::approxEqual(rFontMatrix.m00, rFontMatrix.m11))
        {
            const Size aSize = aVirtualDevice.GetFontMetric( aFont ).GetSize();
            const double fDividend( rFontMatrix.m10 + rFontMatrix.m11 );
            double fStretch = (rFontMatrix.m00 + rFontMatrix.m01);

            if( !::basegfx::fTools::equalZero( fDividend) )
                fStretch /= fDividend;

            const sal_Int32 nNewWidth = ::basegfx::fround( aSize.Width() * fStretch );

            aFont.SetWidth( nNewWidth );
        }

        // set font
        aVirtualDevice.SetFont(aFont);

        // need metrics for Y offset, the XCanvas always renders
        // relative to baseline
        const ::FontMetric& aMetric( aVirtualDevice.GetFontMetric() );

        const sal_Int32 nAboveBaseline( -aMetric.GetIntLeading() - aMetric.GetAscent() );
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
                                              aVirtualDevice.GetTextWidth(
                                                  rText.Text,
                                                  ::canvas::tools::numeric_cast<sal_uInt16>(rText.StartPosition),
                                                  ::canvas::tools::numeric_cast<sal_uInt16>(rText.Length) ),
                                              nBelowBaseline );
        }
    }
}


// eof
