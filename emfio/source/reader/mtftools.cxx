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

#include <mtftools.hxx>

#include <cassert>
#include <memory>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <vcl/metric.hxx>
#include <vcl/graphictools.hxx>
#include <vcl/BitmapTools.hxx>
#include <vcl/metaact.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/svapp.hxx>
#include <tools/fract.hxx>
#include <tools/stream.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/tencinfo.h>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <vcl/virdev.hxx>
#include <o3tl/safeint.hxx>
#include <officecfg/Setup.hxx>
#include <officecfg/Office/Linguistic.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/wincodepage.hxx>
#include <tools/helpers.hxx>
#include <vcl/bitmapaccess.hxx>

#if OSL_DEBUG_LEVEL > 1
#define EMFP_DEBUG(x) x
#else
#define EMFP_DEBUG(x)
#endif

namespace emfio
{
    SvStream& operator >> (SvStream& rInStream, XForm& rXForm)
    {
        if (sizeof(float) != 4)
        {
            OSL_FAIL("EmfReader::sizeof( float ) != 4");
            rXForm = XForm();
        }
        else
        {
            rInStream.ReadFloat(rXForm.eM11);
            rInStream.ReadFloat(rXForm.eM12);
            rInStream.ReadFloat(rXForm.eM21);
            rInStream.ReadFloat(rXForm.eM22);
            rInStream.ReadFloat(rXForm.eDx);
            rInStream.ReadFloat(rXForm.eDy);
        }
        return rInStream;
    }

    void WinMtfClipPath::intersectClipRect( const tools::Rectangle& rRect )
    {
        maClip.intersectRange(
            vcl::unotools::b2DRectangleFromRectangle(rRect));
    }

    void WinMtfClipPath::excludeClipRect( const tools::Rectangle& rRect )
    {
        maClip.subtractRange(
            vcl::unotools::b2DRectangleFromRectangle(rRect));
    }

    void WinMtfClipPath::setClipPath( const tools::PolyPolygon& rPolyPolygon, sal_Int32 nClippingMode )
    {
        const basegfx::B2DPolyPolygon& rB2DPoly=rPolyPolygon.getB2DPolyPolygon();
        switch ( nClippingMode )
        {
            case RGN_OR :
                maClip.unionPolyPolygon(rB2DPoly);
                break;
            case RGN_XOR :
                maClip.xorPolyPolygon(rB2DPoly);
                break;
            case RGN_DIFF :
                maClip.subtractPolyPolygon(rB2DPoly);
                break;
            case RGN_AND :
                maClip.intersectPolyPolygon(rB2DPoly);
                break;
            case RGN_COPY :
                maClip = basegfx::utils::B2DClipState(rB2DPoly);
                break;
        }
    }

    void WinMtfClipPath::moveClipRegion( const Size& rSize )
    {
        basegfx::B2DHomMatrix aTranslate;
        aTranslate.translate(rSize.Width(), rSize.Height());
        maClip.transform(aTranslate);
    }

    void WinMtfClipPath::setDefaultClipPath()
    {
        // Empty clip region - everything visible
        maClip = basegfx::utils::B2DClipState();
    }

    basegfx::B2DPolyPolygon const & WinMtfClipPath::getClipPath() const
    {
        return maClip.getClipPoly();
    }

    void WinMtfPathObj::AddPoint( const Point& rPoint )
    {
        if ( bClosed )
            Insert( tools::Polygon() );
        tools::Polygon& rPoly = static_cast<tools::PolyPolygon&>(*this)[ Count() - 1 ];
        rPoly.Insert( rPoly.GetSize(), rPoint );
        bClosed = false;
    }

    void WinMtfPathObj::AddPolyLine( const tools::Polygon& rPolyLine )
    {
        if ( bClosed )
            Insert( tools::Polygon() );
        tools::Polygon& rPoly = static_cast<tools::PolyPolygon&>(*this)[ Count() - 1 ];
        rPoly.Insert( rPoly.GetSize(), rPolyLine );
        bClosed = false;
    }

    void WinMtfPathObj::AddPolygon( const tools::Polygon& rPoly )
    {
        Insert( rPoly );
        bClosed = true;
    }

    void WinMtfPathObj::AddPolyPolygon( const tools::PolyPolygon& rPolyPoly )
    {
        sal_uInt16 i, nCount = rPolyPoly.Count();
        for ( i = 0; i < nCount; i++ )
            Insert( rPolyPoly[ i ] );
        bClosed = true;
    }

    void WinMtfPathObj::ClosePath()
    {
        if ( Count() )
        {
            tools::Polygon& rPoly = static_cast<tools::PolyPolygon&>(*this)[ Count() - 1 ];
            if ( rPoly.GetSize() > 2 )
            {
                Point aFirst( rPoly[ 0 ] );
                if ( aFirst != rPoly[ rPoly.GetSize() - 1 ] )
                    rPoly.Insert( rPoly.GetSize(), aFirst );
            }
        }
        bClosed = true;
    }

    namespace {

    OUString getLODefaultLanguage()
    {
        if (utl::ConfigManager::IsFuzzing())
            return OUString("en-US");
        OUString result(officecfg::Office::Linguistic::General::DefaultLocale::get());
        if (result.isEmpty())
            result = officecfg::Setup::L10N::ooSetupSystemLocale::get();
        return result;
    }

    }

    WinMtfFontStyle::WinMtfFontStyle( LOGFONTW const & rFont )
    {
        rtl_TextEncoding eCharSet;
        if ((rFont.alfFaceName == "Symbol")
         || (rFont.alfFaceName == "MT Extra"))
            eCharSet = RTL_TEXTENCODING_SYMBOL;
        else if ((rFont.lfCharSet == DEFAULT_CHARSET) || (rFont.lfCharSet == OEM_CHARSET))
            eCharSet = utl_getWinTextEncodingFromLangStr(getLODefaultLanguage().toUtf8().getStr(),
                                                         rFont.lfCharSet == OEM_CHARSET);
        else
            eCharSet = rtl_getTextEncodingFromWindowsCharset( rFont.lfCharSet );
        if ( eCharSet == RTL_TEXTENCODING_DONTKNOW )
            eCharSet = RTL_TEXTENCODING_MS_1252;
        aFont.SetCharSet( eCharSet );
        aFont.SetFamilyName( rFont.alfFaceName );
        FontFamily eFamily;
        switch ( rFont.lfPitchAndFamily & 0xf0 )
        {
            case FF_ROMAN:
                eFamily = FAMILY_ROMAN;
            break;

            case FF_SWISS:
                eFamily = FAMILY_SWISS;
            break;

            case FF_MODERN:
                eFamily = FAMILY_MODERN;
            break;

            case FF_SCRIPT:
                eFamily = FAMILY_SCRIPT;
            break;

            case FF_DECORATIVE:
                 eFamily = FAMILY_DECORATIVE;
            break;

            default:
                eFamily = FAMILY_DONTKNOW;
            break;
        }
        aFont.SetFamily( eFamily );

        FontPitch ePitch;
        switch ( rFont.lfPitchAndFamily & 0x0f )
        {
            case FIXED_PITCH:
                ePitch = PITCH_FIXED;
            break;

            case DEFAULT_PITCH:
            case VARIABLE_PITCH:
            default:
                ePitch = PITCH_VARIABLE;
            break;
        }
        aFont.SetPitch( ePitch );

        FontWeight eWeight;
        if (rFont.lfWeight == 0) // default weight SHOULD be used
            eWeight = WEIGHT_DONTKNOW;
        else if (rFont.lfWeight <= FW_THIN)
            eWeight = WEIGHT_THIN;
        else if( rFont.lfWeight <= FW_ULTRALIGHT )
            eWeight = WEIGHT_ULTRALIGHT;
        else if( rFont.lfWeight <= FW_LIGHT )
            eWeight = WEIGHT_LIGHT;
        else if( rFont.lfWeight <  FW_MEDIUM )
            eWeight = WEIGHT_NORMAL;
        else if( rFont.lfWeight == FW_MEDIUM )
            eWeight = WEIGHT_MEDIUM;
        else if( rFont.lfWeight <= FW_SEMIBOLD )
            eWeight = WEIGHT_SEMIBOLD;
        else if( rFont.lfWeight <= FW_BOLD )
            eWeight = WEIGHT_BOLD;
        else if( rFont.lfWeight <= FW_ULTRABOLD )
            eWeight = WEIGHT_ULTRABOLD;
        else
            eWeight = WEIGHT_BLACK;
        aFont.SetWeight( eWeight );

        if( rFont.lfItalic )
            aFont.SetItalic( ITALIC_NORMAL );

        if( rFont.lfUnderline )
            aFont.SetUnderline( LINESTYLE_SINGLE );

        if( rFont.lfStrikeOut )
            aFont.SetStrikeout( STRIKEOUT_SINGLE );

        aFont.SetOrientation( static_cast<short>(rFont.lfEscapement) );

        Size aFontSize( Size( rFont.lfWidth, rFont.lfHeight ) );
        if ( rFont.lfHeight > 0 )
        {
            // #i117968# VirtualDevice is not thread safe, but filter is used in multithreading
            SolarMutexGuard aGuard;
            ScopedVclPtrInstance< VirtualDevice > pVDev;
            // converting the cell height into a font height
            aFont.SetFontSize( aFontSize );
            pVDev->SetFont( aFont );
            FontMetric aMetric( pVDev->GetFontMetric() );
            long nHeight = aMetric.GetAscent() + aMetric.GetDescent();
            if (nHeight)
            {
                double fHeight = (static_cast<double>(aFontSize.Height()) * rFont.lfHeight ) / nHeight;
                aFontSize.setHeight( static_cast<sal_Int32>( fHeight + 0.5 ) );
            }
        }

        // Convert height to positive
        aFontSize.setHeight( std::abs(aFontSize.Height()) );

        aFont.SetFontSize(aFontSize);
    };

    Color MtfTools::ReadColor()
    {
        sal_uInt32 nColor;

        mpInputStream->ReadUInt32( nColor );
        return Color( static_cast<sal_uInt8>(nColor), static_cast<sal_uInt8>( nColor >> 8 ), static_cast<sal_uInt8>( nColor >> 16 ) );
    };

    Point MtfTools::ImplScale(const Point& rPoint) // Hack to set varying defaults for incompletely defined files.
    {
        if (!mbIsMapDevSet)
            return Point(rPoint.X() * UNDOCUMENTED_WIN_RCL_RELATION - mrclFrame.Left(),
                         rPoint.Y() * UNDOCUMENTED_WIN_RCL_RELATION - mrclFrame.Top());
        else
            return rPoint;
    }

    Point MtfTools::ImplMap( const Point& rPt )
    {
        if ( mnWinExtX && mnWinExtY )
        {
            double fX = rPt.X();
            double fY = rPt.Y();

            double fX2 = fX * maXForm.eM11 + fY * maXForm.eM21 + maXForm.eDx;
            double fY2 = fX * maXForm.eM12 + fY * maXForm.eM22 + maXForm.eDy;

            if ( mnGfxMode == GM_COMPATIBLE )
            {
                switch( mnMapMode )
                {
                    case MM_LOENGLISH :
                    {
                        fX2 -= mnWinOrgX;
                        fY2  = mnWinOrgY-fY2;
                        fX2 *= HUNDREDTH_MILLIMETERS_PER_MILLIINCH*10;
                        fY2 *= HUNDREDTH_MILLIMETERS_PER_MILLIINCH*10;
                        fX2 += mnDevOrgX;
                        fY2 += mnDevOrgY;
                    }
                    break;
                    case MM_HIENGLISH :
                    {
                        fX2 -= mnWinOrgX;
                        fY2  = mnWinOrgY-fY2;
                        fX2 *= HUNDREDTH_MILLIMETERS_PER_MILLIINCH;
                        fY2 *= HUNDREDTH_MILLIMETERS_PER_MILLIINCH;
                        fX2 += mnDevOrgX;
                        fY2 += mnDevOrgY;
                    }
                    break;
                    case MM_TWIPS:
                    {
                        fX2 -= mnWinOrgX;
                        fY2  = mnWinOrgY-fY2;
                        fX2 *= HUNDREDTH_MILLIMETERS_PER_MILLIINCH/MILLIINCH_PER_TWIPS;
                        fY2 *= HUNDREDTH_MILLIMETERS_PER_MILLIINCH/MILLIINCH_PER_TWIPS;
                        fX2 += mnDevOrgX;
                        fY2 += mnDevOrgY;
                    }
                    break;
                    case MM_LOMETRIC :
                    {
                        fX2 -= mnWinOrgX;
                        fY2  = mnWinOrgY-fY2;
                        fX2 *= 10;
                        fY2 *= 10;
                        fX2 += mnDevOrgX;
                        fY2 += mnDevOrgY;
                    }
                    break;
                    case MM_HIMETRIC : // in hundredth of a millimeter
                    {
                        fX2 -= mnWinOrgX;
                        fY2  = mnWinOrgY-fY2;
                        fX2 += mnDevOrgX;
                        fY2 += mnDevOrgY;
                    }
                    break;
                    default :
                    {
                        if (mnPixX == 0 || mnPixY == 0)
                        {
                            SAL_WARN("emfio", "invalid scaling factor");
                            return Point();
                        }
                        else
                        {
                            fX2 -= mnWinOrgX;
                            fY2 -= mnWinOrgY;
                            fX2 /= mnWinExtX;
                            fY2 /= mnWinExtY;
                            fX2 *= mnDevWidth;
                            fY2 *= mnDevHeight;
                            fX2 += mnDevOrgX;
                            fY2 += mnDevOrgY;   // fX2, fY2 now in device units
                            fX2 *= static_cast<double>(mnMillX) * 100.0 / static_cast<double>(mnPixX);
                            fY2 *= static_cast<double>(mnMillY) * 100.0 / static_cast<double>(mnPixY);
                        }
                    }
                    break;
                }
                fX2 -= mrclFrame.Left();
                fY2 -= mrclFrame.Top();
            }
            return Point(basegfx::fround(fX2), basegfx::fround(fY2));
        }
        else
            return Point();
    };

    Size MtfTools::ImplMap(const Size& rSz, bool bDoWorldTransform)
    {
        if ( mnWinExtX && mnWinExtY )
        {
            // #i121382# apply the whole WorldTransform, else a rotation will be misinterpreted
            double fWidth, fHeight;
            if (bDoWorldTransform)
            {
                fWidth = rSz.Width() * maXForm.eM11 + rSz.Height() * maXForm.eM21;
                fHeight = rSz.Width() * maXForm.eM12 + rSz.Height() * maXForm.eM22;
            }
            else
            {
                //take the scale, but not the rotation
                basegfx::B2DHomMatrix aMatrix(maXForm.eM11, maXForm.eM12, 0,
                                              maXForm.eM21, maXForm.eM22, 0);
                basegfx::B2DTuple aScale, aTranslate;
                double fRotate, fShearX;
                if (!aMatrix.decompose(aScale, aTranslate, fRotate, fShearX))
                {
                    aScale.setX(1.0);
                    aScale.setY(1.0);
                }
                fWidth = rSz.Width() * aScale.getX();
                fHeight = rSz.Height() * aScale.getY();
            }

            if ( mnGfxMode == GM_COMPATIBLE )
            {
                switch( mnMapMode )
                {
                    case MM_LOENGLISH :
                    {
                        fWidth *= HUNDREDTH_MILLIMETERS_PER_MILLIINCH*10;
                        fHeight*=-HUNDREDTH_MILLIMETERS_PER_MILLIINCH*10;
                    }
                    break;
                    case MM_HIENGLISH :
                    {
                        fWidth *= HUNDREDTH_MILLIMETERS_PER_MILLIINCH;
                        fHeight*=-HUNDREDTH_MILLIMETERS_PER_MILLIINCH;
                    }
                    break;
                    case MM_LOMETRIC :
                    {
                        fWidth *= 10;
                        fHeight*=-10;
                    }
                    break;
                    case MM_HIMETRIC : // in hundredth of millimeters
                    {
                        fHeight *= -1;
                    }
                    break;
                    case MM_TWIPS:
                    {
                        fWidth *= HUNDREDTH_MILLIMETERS_PER_MILLIINCH/MILLIINCH_PER_TWIPS;
                        fHeight*=-HUNDREDTH_MILLIMETERS_PER_MILLIINCH/MILLIINCH_PER_TWIPS;
                    }
                    break;
                    default :
                    {
                        if (mnPixX == 0 || mnPixY == 0)
                        {
                            SAL_WARN("emfio", "invalid scaling factor");
                            return Size();
                        }
                        else
                        {
                            fWidth /= mnWinExtX;
                            fHeight /= mnWinExtY;
                            fWidth *= mnDevWidth;
                            fHeight *= mnDevHeight;
                            fWidth *= static_cast<double>(mnMillX) * 100.0 / static_cast<double>(mnPixX);
                            fHeight *= static_cast<double>(mnMillY) * 100.0 / static_cast<double>(mnPixY);
                        }
                    }
                    break;
                }
            }
            return Size(basegfx::fround(fWidth), basegfx::fround(fHeight));
        }
        else
            return Size();
    }

    tools::Rectangle MtfTools::ImplMap( const tools::Rectangle& rRect )
    {
        tools::Rectangle aRect;
        aRect.SetPos(ImplMap(rRect.TopLeft()));
        aRect.SaturatingSetSize(ImplMap(rRect.GetSize()));
        return aRect;
    }

    void MtfTools::ImplMap( vcl::Font& rFont )
    {
        // !!! HACK: we now always set the width to zero because the OS width is interpreted differently;
        // must later be made portable in SV (KA 1996-02-08)
        Size  aFontSize = ImplMap (rFont.GetFontSize(), false);

        const auto nHeight = aFontSize.Height();
        if (nHeight < 0)
            aFontSize.setHeight( o3tl::saturating_toggle_sign(nHeight) );

        rFont.SetFontSize( aFontSize );

        sal_Int32 nResult;
        const bool bFail = o3tl::checked_multiply(mnWinExtX, mnWinExtY, nResult);
        if (!bFail && nResult < 0)
            rFont.SetOrientation( 3600 - rFont.GetOrientation() );
    }

    tools::Polygon& MtfTools::ImplMap( tools::Polygon& rPolygon )
    {
        sal_uInt16 nPoints = rPolygon.GetSize();
        for ( sal_uInt16 i = 0; i < nPoints; i++ )
        {
            rPolygon[ i ] = ImplMap( rPolygon[ i ] );
        }
        return rPolygon;
    }

    void MtfTools::ImplScale( tools::Polygon& rPolygon )
    {
        sal_uInt16 nPoints = rPolygon.GetSize();
        for ( sal_uInt16 i = 0; i < nPoints; i++ )
        {
            rPolygon[ i ] = ImplScale( rPolygon[ i ] );
        }
    }

    tools::PolyPolygon& MtfTools::ImplScale( tools::PolyPolygon& rPolyPolygon )
    {
        sal_uInt16 nPolys = rPolyPolygon.Count();
        for (sal_uInt16 i = 0; i < nPolys; ++i)
        {
            ImplScale(rPolyPolygon[i]);
        }
        return rPolyPolygon;
    }

    tools::PolyPolygon& MtfTools::ImplMap( tools::PolyPolygon& rPolyPolygon )
    {
        sal_uInt16 nPolys = rPolyPolygon.Count();
        for ( sal_uInt16 i = 0; i < nPolys; ImplMap( rPolyPolygon[ i++ ] ) ) ;
        return rPolyPolygon;
    }

    void MtfTools::SelectObject( sal_Int32 nIndex )
    {
        if ( nIndex & ENHMETA_STOCK_OBJECT )
        {
            sal_uInt16 nStockId = static_cast<sal_uInt8>(nIndex);
            switch( nStockId )
            {
                case WHITE_BRUSH :
                {
                    maFillStyle = WinMtfFillStyle( COL_WHITE );
                    mbFillStyleSelected = true;
                }
                break;
                case LTGRAY_BRUSH :
                {
                    maFillStyle = WinMtfFillStyle( COL_LIGHTGRAY );
                    mbFillStyleSelected = true;
                }
                break;
                case GRAY_BRUSH :
                case DKGRAY_BRUSH :
                {
                    maFillStyle = WinMtfFillStyle( COL_GRAY );
                    mbFillStyleSelected = true;
                }
                break;
                case BLACK_BRUSH :
                {
                    maFillStyle = WinMtfFillStyle( COL_BLACK );
                    mbFillStyleSelected = true;
                }
                break;
                case NULL_BRUSH :
                {
                   maFillStyle = WinMtfFillStyle( COL_TRANSPARENT, true );
                   mbFillStyleSelected = true;
                }
                break;
                case WHITE_PEN :
                {
                    maLineStyle = WinMtfLineStyle( COL_WHITE );
                }
                break;
                case BLACK_PEN :
                {
                    maLineStyle = WinMtfLineStyle( COL_BLACK );
                }
                break;
                case NULL_PEN :
                {
                    maLineStyle = WinMtfLineStyle( COL_TRANSPARENT, true );
                }
                break;
                default:
                break;
            }
        }
        else
        {
            nIndex &= 0xffff;       // safety check: don't allow index to be > 65535

            GDIObj *pGDIObj = nullptr;

            if ( static_cast<sal_uInt32>(nIndex) < mvGDIObj.size() )
                pGDIObj = mvGDIObj[ nIndex ].get();

            if ( pGDIObj )
            {
                if (const auto pen = dynamic_cast<WinMtfLineStyle*>(pGDIObj))
                    maLineStyle = *pen;
                else if (const auto brush = dynamic_cast<WinMtfFillStyle*>(
                             pGDIObj))
                {
                    maFillStyle = *brush;
                    mbFillStyleSelected = true;
                }
                else if (const auto font = dynamic_cast<WinMtfFontStyle*>(
                             pGDIObj))
                {
                    maFont = font->aFont;
                }
            }
        }
    }

    void MtfTools::SetTextLayoutMode( ComplexTextLayoutFlags nTextLayoutMode )
    {
        mnTextLayoutMode = nTextLayoutMode;
    }

    void MtfTools::SetBkMode( BkMode nMode )
    {
        mnBkMode = nMode;
    }

    void MtfTools::SetBkColor( const Color& rColor )
    {
        maBkColor = rColor;
    }

    void MtfTools::SetTextColor( const Color& rColor )
    {
        maTextColor = rColor;
    }

    void MtfTools::SetTextAlign( sal_uInt32 nAlign )
    {
        mnTextAlign = nAlign;
    }

    void MtfTools::ImplResizeObjectArry( sal_uInt32 nNewEntrys )
    {
        mvGDIObj.resize(nNewEntrys);
    }

    void MtfTools::ImplDrawClippedPolyPolygon( const tools::PolyPolygon& rPolyPoly )
    {
        if ( rPolyPoly.Count() )
        {
            ImplSetNonPersistentLineColorTransparenz();
            if ( rPolyPoly.Count() == 1 )
            {
                if ( rPolyPoly.IsRect() )
                    mpGDIMetaFile->AddAction( new MetaRectAction( rPolyPoly.GetBoundRect() ) );
                else
                {
                    tools::Polygon aPoly( rPolyPoly[ 0 ] );
                    sal_uInt16 nCount = aPoly.GetSize();
                    if ( nCount )
                    {
                        if ( aPoly[ nCount - 1 ] != aPoly[ 0 ] )
                        {
                            Point aPoint( aPoly[ 0 ] );
                            aPoly.Insert( nCount, aPoint );
                        }
                        mpGDIMetaFile->AddAction( new MetaPolygonAction( aPoly ) );
                    }
                }
            }
            else
                mpGDIMetaFile->AddAction( new MetaPolyPolygonAction( rPolyPoly ) );
        }
    }

    void MtfTools::CreateObject( std::unique_ptr<GDIObj> pObject )
    {
        if ( pObject )
        {
            const auto pLineStyle = dynamic_cast<WinMtfLineStyle*>(pObject.get());
            const auto pFontStyle = dynamic_cast<WinMtfFontStyle*>(pObject.get());

            if ( pFontStyle )
            {
                if (pFontStyle->aFont.GetFontHeight() == 0)
                    pFontStyle->aFont.SetFontHeight(423);
                ImplMap(pFontStyle->aFont); // defaulting to 12pt
            }
            else if ( pLineStyle )
            {
                Size aSize(pLineStyle->aLineInfo.GetWidth(), 0);
                aSize = ImplMap(aSize);
                pLineStyle->aLineInfo.SetWidth(aSize.Width());
            }
        }
        std::vector<std::unique_ptr<GDIObj>>::size_type nIndex;
        for ( nIndex = 0; nIndex < mvGDIObj.size(); nIndex++ )
        {
            if ( !mvGDIObj[ nIndex ] )
                break;
        }
        if ( nIndex == mvGDIObj.size() )
            ImplResizeObjectArry( mvGDIObj.size() + 16 );

        mvGDIObj[ nIndex ] = std::move(pObject);
    }

    void MtfTools::CreateObjectIndexed( sal_Int32 nIndex, std::unique_ptr<GDIObj> pObject )
    {
        if ( ( nIndex & ENHMETA_STOCK_OBJECT ) == 0 )
        {
            nIndex &= 0xffff;       // safety check: do not allow index to be > 65535
            if ( pObject )
            {
                const auto pLineStyle = dynamic_cast<WinMtfLineStyle*>(pObject.get());
                const auto pFontStyle = dynamic_cast<WinMtfFontStyle*>(pObject.get());
                if ( pFontStyle )
                {
                    if (pFontStyle->aFont.GetFontHeight() == 0)
                        pFontStyle->aFont.SetFontHeight(423);
                    ImplMap(pFontStyle->aFont);
                }
                else if ( pLineStyle )
                {
                    Size aSize(pLineStyle->aLineInfo.GetWidth(), 0);
                    pLineStyle->aLineInfo.SetWidth( ImplMap(aSize).Width() );

                    if ( pLineStyle->aLineInfo.GetStyle() == LineStyle::Dash )
                    {
                        aSize.AdjustWidth(1 );
                        long nDotLen = ImplMap( aSize ).Width();
                        pLineStyle->aLineInfo.SetDistance( nDotLen );
                        pLineStyle->aLineInfo.SetDotLen( nDotLen );
                        pLineStyle->aLineInfo.SetDashLen( nDotLen * 3 );
                    }
                }
            }
            if ( static_cast<sal_uInt32>(nIndex) >= mvGDIObj.size() )
                ImplResizeObjectArry( nIndex + 16 );

            mvGDIObj[ nIndex ] = std::move(pObject);
        }
    }

    void MtfTools::CreateObject()
    {
        CreateObject(std::make_unique<GDIObj>());
    }

    void MtfTools::DeleteObject( sal_Int32 nIndex )
    {
        if ( ( nIndex & ENHMETA_STOCK_OBJECT ) == 0 )
        {
            if ( static_cast<sal_uInt32>(nIndex) < mvGDIObj.size() )
            {
                mvGDIObj[ nIndex ].reset();
            }
        }
    }

    void MtfTools::IntersectClipRect( const tools::Rectangle& rRect )
    {
        if (utl::ConfigManager::IsFuzzing())
            return;
        mbClipNeedsUpdate=true;
        if ((rRect.Left()-rRect.Right()==0) && (rRect.Top()-rRect.Bottom()==0))
        {
            return; // empty rectangles cause trouble
        }
        maClipPath.intersectClipRect( ImplMap( rRect ) );
    }

    void MtfTools::ExcludeClipRect( const tools::Rectangle& rRect )
    {
        if (utl::ConfigManager::IsFuzzing())
            return;
        mbClipNeedsUpdate=true;
        maClipPath.excludeClipRect( ImplMap( rRect ) );
    }

    void MtfTools::MoveClipRegion( const Size& rSize )
    {
        if (utl::ConfigManager::IsFuzzing())
            return;
        mbClipNeedsUpdate=true;
        maClipPath.moveClipRegion( ImplMap( rSize ) );
    }

    void MtfTools::SetClipPath( const tools::PolyPolygon& rPolyPolygon, sal_Int32 nClippingMode, bool bIsMapped )
    {
        if (utl::ConfigManager::IsFuzzing())
            return;
        mbClipNeedsUpdate = true;
        tools::PolyPolygon aPolyPolygon(rPolyPolygon);

        if (!bIsMapped)
        {
            if (!mbIsMapDevSet && (mnMapMode == MM_ISOTROPIC || mnMapMode == MM_ANISOTROPIC))
                aPolyPolygon = ImplScale(aPolyPolygon);
            else
                aPolyPolygon = ImplMap(aPolyPolygon);
        }
        maClipPath.setClipPath(aPolyPolygon, nClippingMode);
    }

    void MtfTools::SetDefaultClipPath()
    {
        mbClipNeedsUpdate = true;
        maClipPath.setDefaultClipPath();
    }

    MtfTools::MtfTools( GDIMetaFile& rGDIMetaFile, SvStream& rStreamWMF)
    :   maPathObj(),
        maClipPath(),
        maLatestLineStyle(),
        maLineStyle(),
        maNopLineStyle(),
        maLatestFillStyle(),
        maFillStyle(),
        maNopFillStyle(),
        maLatestFont(),
        maFont(),
        mnLatestTextAlign(90),
        mnTextAlign(TA_LEFT | TA_TOP | TA_NOUPDATECP),
        maLatestTextColor(),
        maTextColor(),
        maLatestBkColor(0x12345678),
        maBkColor(COL_WHITE),
        mnLatestTextLayoutMode(ComplexTextLayoutFlags::Default),
        mnTextLayoutMode(ComplexTextLayoutFlags::Default),
        mnLatestBkMode(BkMode::NONE),
        mnBkMode(BkMode::OPAQUE),
        meLatestRasterOp(RasterOp::Invert),
        meRasterOp(RasterOp::OverPaint),
        mvGDIObj(),
        maActPos(),
        mnRop(),
        mvSaveStack(),
        mnGfxMode(GM_COMPATIBLE),
        mnMapMode(MM_TEXT),
        maXForm(),
        mnDevOrgX(0),
        mnDevOrgY(0),
        mnDevWidth(1),
        mnDevHeight(1),
        mnWinOrgX(0),
        mnWinOrgY(0),
        mnWinExtX(1),
        mnWinExtY(1),
        mnPixX(100),
        mnPixY(100),
        mnMillX(1),
        mnMillY(1),
        mrclFrame(),
        mrclBounds(),
        mpGDIMetaFile(&rGDIMetaFile),
        mpInputStream(&rStreamWMF),
        mnStartPos(0),
        mnEndPos(0),
        maBmpSaveList(),
        mbNopMode(false),
        mbFillStyleSelected(false),
        mbClipNeedsUpdate(true),
        mbComplexClip(false),
        mbIsMapWinSet(false),
        mbIsMapDevSet(false)
    {
        SvLockBytes *pLB = mpInputStream->GetLockBytes();

        if (pLB)
        {
            pLB->SetSynchronMode();
        }

        mnStartPos = mpInputStream->Tell();
        SetDevOrg(Point());

        mpGDIMetaFile->AddAction( new MetaPushAction( PushFlags::CLIPREGION ) ); // The original clipregion has to be on top
                                                                                 // of the stack so it can always be restored
                                                                                 // this is necessary to be able to support
                                                                                 // SetClipRgn( NULL ) and similar ClipRgn actions (SJ)

        maFont.SetFamilyName( "Arial" );                                         // sj: #i57205#, we do have some scaling problems if using
        maFont.SetCharSet( RTL_TEXTENCODING_MS_1252 );                           // the default font then most times a x11 font is used, we
        maFont.SetFontHeight( 423 );                                      // will prevent this defining a font

        maLatestLineStyle.aLineColor = Color( 0x12, 0x34, 0x56 );
        maLatestFillStyle.aFillColor = Color( 0x12, 0x34, 0x56 );

        mnRop = WMFRasterOp::Black;
        meRasterOp = RasterOp::OverPaint;
        mpGDIMetaFile->AddAction( new MetaRasterOpAction( RasterOp::OverPaint ) );
    }

    MtfTools::~MtfTools() COVERITY_NOEXCEPT_FALSE
    {
        mpGDIMetaFile->AddAction( new MetaPopAction() );
        mpGDIMetaFile->SetPrefMapMode(MapMode(MapUnit::Map100thMM));
        if ( mrclFrame.IsEmpty() )
            mpGDIMetaFile->SetPrefSize( Size( mnDevWidth, mnDevHeight ) );
        else
            mpGDIMetaFile->SetPrefSize( mrclFrame.GetSize() );
    }

    void MtfTools::UpdateClipRegion()
    {
        if (mbClipNeedsUpdate)
        {
            mbClipNeedsUpdate = false;
            mbComplexClip = false;

            mpGDIMetaFile->AddAction( new MetaPopAction() );                    // taking the original clipregion
            mpGDIMetaFile->AddAction( new MetaPushAction( PushFlags::CLIPREGION ) );

            // skip for 'no clipping at all' case
            if( !maClipPath.isEmpty() )
            {
                const basegfx::B2DPolyPolygon& rClipPoly( maClipPath.getClipPath() );

                mbComplexClip = rClipPoly.count() > 1
                    || !basegfx::utils::isRectangle(rClipPoly);

                static bool bEnableComplexClipViaRegion = getenv("SAL_WMF_COMPLEXCLIP_VIA_REGION") != nullptr;

                if (bEnableComplexClipViaRegion)
                {
                    //this makes cases like tdf#45820 work in reasonable time, and I feel in theory should
                    //be just fine. In practice I see the output is different so needs work before its the
                    //default, but for file fuzzing it should be good enough
                    if (mbComplexClip)
                    {
                        mpGDIMetaFile->AddAction(
                            new MetaISectRegionClipRegionAction(
                                vcl::Region(rClipPoly)));
                        mbComplexClip = false;
                    }
                    else
                    {
                        mpGDIMetaFile->AddAction(
                            new MetaISectRectClipRegionAction(
                                vcl::unotools::rectangleFromB2DRectangle(
                                    rClipPoly.getB2DRange())));
                    }
                }
                else
                {
                    //normal case
                    mpGDIMetaFile->AddAction(
                        new MetaISectRectClipRegionAction(
                            vcl::unotools::rectangleFromB2DRectangle(
                                rClipPoly.getB2DRange())));
                }
            }
        }
    }

    void MtfTools::ImplSetNonPersistentLineColorTransparenz()
    {
        WinMtfLineStyle aTransparentLine( COL_TRANSPARENT, true );
        if ( ! ( maLatestLineStyle == aTransparentLine ) )
        {
            maLatestLineStyle = aTransparentLine;
            mpGDIMetaFile->AddAction( new MetaLineColorAction( aTransparentLine.aLineColor, !aTransparentLine.bTransparent ) );
        }
    }

    void MtfTools::UpdateLineStyle()
    {
        if (!( maLatestLineStyle == maLineStyle ) )
        {
            maLatestLineStyle = maLineStyle;
            mpGDIMetaFile->AddAction( new MetaLineColorAction( maLineStyle.aLineColor, !maLineStyle.bTransparent ) );
        }
    }

    void MtfTools::UpdateFillStyle()
    {
        if ( !mbFillStyleSelected )     // SJ: #i57205# taking care of bkcolor if no brush is selected
            maFillStyle = WinMtfFillStyle( maBkColor, mnBkMode == BkMode::Transparent );
        if (!( maLatestFillStyle == maFillStyle ) )
        {
            maLatestFillStyle = maFillStyle;
            if (maFillStyle.aType == WinMtfFillStyleType::Solid)
                mpGDIMetaFile->AddAction( new MetaFillColorAction( maFillStyle.aFillColor, !maFillStyle.bTransparent ) );
        }
    }

    WMFRasterOp MtfTools::SetRasterOp( WMFRasterOp nRasterOp )
    {
        WMFRasterOp nRetROP = mnRop;
        if ( nRasterOp != mnRop )
        {
            mnRop = nRasterOp;

            if ( mbNopMode && ( nRasterOp != WMFRasterOp::Nop ) )
            {   // changing modes from WMFRasterOp::Nop so set pen and brush
                maFillStyle = maNopFillStyle;
                maLineStyle = maNopLineStyle;
                mbNopMode = false;
            }
            switch( nRasterOp )
            {
                case WMFRasterOp::Not:
                    meRasterOp = RasterOp::Invert;
                break;

                case WMFRasterOp::XorPen:
                    meRasterOp = RasterOp::Xor;
                break;

                case WMFRasterOp::Nop:
                {
                    meRasterOp = RasterOp::OverPaint;
                    if( !mbNopMode )
                    {
                        maNopFillStyle = maFillStyle;
                        maNopLineStyle = maLineStyle;
                        maFillStyle = WinMtfFillStyle( COL_TRANSPARENT, true );
                        maLineStyle = WinMtfLineStyle( COL_TRANSPARENT, true );
                        mbNopMode = true;
                    }
                }
                break;

                default:
                    meRasterOp = RasterOp::OverPaint;
                break;
            }
        }
        if ( nRetROP != nRasterOp )
            mpGDIMetaFile->AddAction( new MetaRasterOpAction( meRasterOp ) );
        return nRetROP;
    };

    void MtfTools::StrokeAndFillPath( bool bStroke, bool bFill )
    {
        if ( maPathObj.Count() )
        {
            UpdateClipRegion();
            UpdateLineStyle();
            UpdateFillStyle();
            if ( bFill )
            {
                if ( !bStroke )
                {
                    mpGDIMetaFile->AddAction( new MetaPushAction( PushFlags::LINECOLOR ) );
                    mpGDIMetaFile->AddAction( new MetaLineColorAction( Color(), false ) );
                }
                if ( maPathObj.Count() == 1 )
                    mpGDIMetaFile->AddAction( new MetaPolygonAction( maPathObj.GetObject( 0 ) ) );
                else
                    mpGDIMetaFile->AddAction( new MetaPolyPolygonAction( maPathObj ) );

                if ( !bStroke )
                    mpGDIMetaFile->AddAction( new MetaPopAction() );
            }
            else
            {
                sal_uInt16 i, nCount = maPathObj.Count();
                for ( i = 0; i < nCount; i++ )
                    mpGDIMetaFile->AddAction( new MetaPolyLineAction( maPathObj[ i ], maLineStyle.aLineInfo ) );
            }
            ClearPath();
        }
    }

    void MtfTools::DrawPixel( const Point& rSource, const Color& rColor )
    {
        mpGDIMetaFile->AddAction( new MetaPixelAction( ImplMap( rSource), rColor ) );
    }

    void MtfTools::MoveTo( const Point& rPoint, bool bRecordPath )
    {
        Point aDest( ImplMap( rPoint ) );
        if ( bRecordPath )
        {
            // fdo#57353 create new subpath for subsequent moves
            if ( maPathObj.Count() )
                if ( maPathObj[ maPathObj.Count() - 1 ].GetSize() )
                    maPathObj.Insert( tools::Polygon() );
            maPathObj.AddPoint( aDest );
        }
        maActPos = aDest;
    }

    void MtfTools::LineTo( const Point& rPoint, bool bRecordPath )
    {
        UpdateClipRegion();
        Point aDest( ImplMap( rPoint ) );
        if ( bRecordPath )
            maPathObj.AddPoint( aDest );
        else
        {
            UpdateLineStyle();
            mpGDIMetaFile->AddAction( new MetaLineAction( maActPos, aDest, maLineStyle.aLineInfo ) );
        }
        maActPos = aDest;
    }

    void MtfTools::DrawRect( const tools::Rectangle& rRect, bool bEdge )
    {
        UpdateClipRegion();
        UpdateFillStyle();

        if ( mbComplexClip )
        {
            tools::Polygon aPoly( ImplMap( rRect ) );
            tools::PolyPolygon aPolyPolyRect( aPoly );
            tools::PolyPolygon aDest;
            tools::PolyPolygon(maClipPath.getClipPath()).GetIntersection( aPolyPolyRect, aDest );
            ImplDrawClippedPolyPolygon( aDest );
        }
        else
        {
            if ( bEdge )
            {
                if ( maLineStyle.aLineInfo.GetWidth() || ( maLineStyle.aLineInfo.GetStyle() == LineStyle::Dash ) )
                {
                    ImplSetNonPersistentLineColorTransparenz();
                    mpGDIMetaFile->AddAction( new MetaRectAction( ImplMap( rRect ) ) );
                    UpdateLineStyle();
                    mpGDIMetaFile->AddAction( new MetaPolyLineAction( tools::Polygon( ImplMap( rRect ) ),maLineStyle.aLineInfo ) );
                }
                else
                {
                    UpdateLineStyle();
                    mpGDIMetaFile->AddAction( new MetaRectAction( ImplMap( rRect ) ) );
                }
            }
            else
            {
                ImplSetNonPersistentLineColorTransparenz();
                mpGDIMetaFile->AddAction( new MetaRectAction( ImplMap( rRect ) ) );
            }
        }
    }

    void MtfTools::DrawRoundRect( const tools::Rectangle& rRect, const Size& rSize )
    {
        UpdateClipRegion();
        UpdateLineStyle();
        UpdateFillStyle();
        mpGDIMetaFile->AddAction( new MetaRoundRectAction( ImplMap( rRect ), labs( ImplMap( rSize ).Width() ), labs( ImplMap( rSize ).Height() ) ) );
    }

    void MtfTools::DrawEllipse( const tools::Rectangle& rRect )
    {
        UpdateClipRegion();
        UpdateFillStyle();

        if ( maLineStyle.aLineInfo.GetWidth() || ( maLineStyle.aLineInfo.GetStyle() == LineStyle::Dash ) )
        {
            Point aCenter( ImplMap( rRect.Center() ) );
            Size  aRad( ImplMap( Size( rRect.GetWidth() / 2, rRect.GetHeight() / 2 ) ) );

            ImplSetNonPersistentLineColorTransparenz();
            mpGDIMetaFile->AddAction( new MetaEllipseAction( ImplMap( rRect ) ) );
            UpdateLineStyle();
            mpGDIMetaFile->AddAction( new MetaPolyLineAction( tools::Polygon( aCenter, aRad.Width(), aRad.Height() ), maLineStyle.aLineInfo ) );
        }
        else
        {
            UpdateLineStyle();
            mpGDIMetaFile->AddAction( new MetaEllipseAction( ImplMap( rRect ) ) );
        }
    }

    void MtfTools::DrawArc( const tools::Rectangle& rRect, const Point& rStart, const Point& rEnd, bool bTo )
    {
        UpdateClipRegion();
        UpdateLineStyle();
        UpdateFillStyle();

        tools::Rectangle   aRect( ImplMap( rRect ) );
        Point       aStart( ImplMap( rStart ) );
        Point       aEnd( ImplMap( rEnd ) );

        if ( maLineStyle.aLineInfo.GetWidth() || ( maLineStyle.aLineInfo.GetStyle() == LineStyle::Dash ) )
        {
            if ( aStart == aEnd )
            {   // SJ: #i53768# if start & end is identical, then we have to draw a full ellipse
                Point aCenter( aRect.Center() );
                Size  aRad( aRect.GetWidth() / 2, aRect.GetHeight() / 2 );

                mpGDIMetaFile->AddAction( new MetaPolyLineAction( tools::Polygon( aCenter, aRad.Width(), aRad.Height() ), maLineStyle.aLineInfo ) );
            }
            else
                mpGDIMetaFile->AddAction( new MetaPolyLineAction( tools::Polygon( aRect, aStart, aEnd, PolyStyle::Arc ), maLineStyle.aLineInfo ) );
        }
        else
            mpGDIMetaFile->AddAction( new MetaArcAction( aRect, aStart, aEnd ) );

        if ( bTo )
            maActPos = aEnd;
    }

    void MtfTools::DrawPie( const tools::Rectangle& rRect, const Point& rStart, const Point& rEnd )
    {
        UpdateClipRegion();
        UpdateFillStyle();

        tools::Rectangle   aRect( ImplMap( rRect ) );
        Point       aStart( ImplMap( rStart ) );
        Point       aEnd( ImplMap( rEnd ) );

        if ( maLineStyle.aLineInfo.GetWidth() || ( maLineStyle.aLineInfo.GetStyle() == LineStyle::Dash ) )
        {
            ImplSetNonPersistentLineColorTransparenz();
            mpGDIMetaFile->AddAction( new MetaPieAction( aRect, aStart, aEnd ) );
            UpdateLineStyle();
            mpGDIMetaFile->AddAction( new MetaPolyLineAction( tools::Polygon( aRect, aStart, aEnd, PolyStyle::Pie ), maLineStyle.aLineInfo ) );
        }
        else
        {
            UpdateLineStyle();
            mpGDIMetaFile->AddAction( new MetaPieAction( aRect, aStart, aEnd ) );
        }
    }

    void MtfTools::DrawChord( const tools::Rectangle& rRect, const Point& rStart, const Point& rEnd )
    {
        UpdateClipRegion();
        UpdateFillStyle();

        tools::Rectangle   aRect( ImplMap( rRect ) );
        Point       aStart( ImplMap( rStart ) );
        Point       aEnd( ImplMap( rEnd ) );

        if ( maLineStyle.aLineInfo.GetWidth() || ( maLineStyle.aLineInfo.GetStyle() == LineStyle::Dash ) )
        {
            ImplSetNonPersistentLineColorTransparenz();
            mpGDIMetaFile->AddAction( new MetaChordAction( aRect, aStart, aEnd ) );
            UpdateLineStyle();
            mpGDIMetaFile->AddAction( new MetaPolyLineAction( tools::Polygon( aRect, aStart, aEnd, PolyStyle::Chord ), maLineStyle.aLineInfo ) );
        }
        else
        {
            UpdateLineStyle();
            mpGDIMetaFile->AddAction( new MetaChordAction( aRect, aStart, aEnd ) );
        }
    }

    void MtfTools::DrawPolygon( tools::Polygon rPolygon, bool bRecordPath )
    {
        UpdateClipRegion();
        ImplMap( rPolygon );
        if ( bRecordPath )
            maPathObj.AddPolygon( rPolygon );
        else
        {
            UpdateFillStyle();

            if ( mbComplexClip )
            {
                tools::PolyPolygon aPolyPoly( rPolygon );
                tools::PolyPolygon aDest;
                tools::PolyPolygon(maClipPath.getClipPath()).GetIntersection( aPolyPoly, aDest );
                ImplDrawClippedPolyPolygon( aDest );
            }
            else
            {
                if ( maLineStyle.aLineInfo.GetWidth() || ( maLineStyle.aLineInfo.GetStyle() == LineStyle::Dash ) )
                {
                    sal_uInt16 nCount = rPolygon.GetSize();
                    if ( nCount )
                    {
                        if ( rPolygon[ nCount - 1 ] != rPolygon[ 0 ] )
                        {
                            Point aPoint( rPolygon[ 0 ] );
                            rPolygon.Insert( nCount, aPoint );
                        }
                    }
                    ImplSetNonPersistentLineColorTransparenz();
                    mpGDIMetaFile->AddAction( new MetaPolygonAction( rPolygon ) );
                    UpdateLineStyle();
                    mpGDIMetaFile->AddAction( new MetaPolyLineAction( rPolygon, maLineStyle.aLineInfo ) );
                }
                else
                {
                    UpdateLineStyle();

                    if (maLatestFillStyle.aType != WinMtfFillStyleType::Pattern)
                        mpGDIMetaFile->AddAction( new MetaPolygonAction( rPolygon ) );
                    else {
                        SvtGraphicFill aFill = SvtGraphicFill( tools::PolyPolygon( rPolygon ),
                                                               Color(),
                                                               0.0,
                                                               SvtGraphicFill::fillNonZero,
                                                               SvtGraphicFill::fillTexture,
                                                               SvtGraphicFill::Transform(),
                                                               true,
                                                               SvtGraphicFill::hatchSingle,
                                                               Color(),
                                                               SvtGraphicFill::GradientType::Linear,
                                                               Color(),
                                                               Color(),
                                                               0,
                                                               Graphic (maLatestFillStyle.aBmp) );

                        SvMemoryStream  aMemStm;

                        WriteSvtGraphicFill( aMemStm, aFill );

                        mpGDIMetaFile->AddAction( new MetaCommentAction( "XPATHFILL_SEQ_BEGIN", 0,
                                                                static_cast<const sal_uInt8*>(aMemStm.GetData()),
                                                                aMemStm.TellEnd() ) );
                        mpGDIMetaFile->AddAction( new MetaCommentAction( "XPATHFILL_SEQ_END" ) );
                    }

                }
            }
        }
    }

    void MtfTools::DrawPolyPolygon( tools::PolyPolygon& rPolyPolygon, bool bRecordPath )
    {
        UpdateClipRegion();

        ImplMap( rPolyPolygon );

        if ( bRecordPath )
            maPathObj.AddPolyPolygon( rPolyPolygon );
        else
        {
            UpdateFillStyle();

            if ( mbComplexClip )
            {
                tools::PolyPolygon aDest;
                tools::PolyPolygon(maClipPath.getClipPath()).GetIntersection( rPolyPolygon, aDest );
                ImplDrawClippedPolyPolygon( aDest );
            }
            else
            {
                UpdateLineStyle();
                mpGDIMetaFile->AddAction( new MetaPolyPolygonAction( rPolyPolygon ) );
                if (maLineStyle.aLineInfo.GetWidth() > 0 || maLineStyle.aLineInfo.GetStyle() == LineStyle::Dash)
                {
                    for (sal_uInt16 nPoly = 0; nPoly < rPolyPolygon.Count(); ++nPoly)
                    {
                        mpGDIMetaFile->AddAction(new MetaPolyLineAction(rPolyPolygon[nPoly], maLineStyle.aLineInfo));
                    }
                }
            }
        }
    }

    void MtfTools::DrawPolyLine( tools::Polygon rPolygon, bool bTo, bool bRecordPath )
    {
        UpdateClipRegion();

        sal_uInt16 nPoints = rPolygon.GetSize();
        if (nPoints >= 1)
        {
            ImplMap( rPolygon );
            if ( bTo )
            {
                rPolygon[ 0 ] = maActPos;
                maActPos = rPolygon[ rPolygon.GetSize() - 1 ];
            }
            if ( bRecordPath )
                maPathObj.AddPolyLine( rPolygon );
            else
            {
                UpdateLineStyle();
                mpGDIMetaFile->AddAction( new MetaPolyLineAction( rPolygon, maLineStyle.aLineInfo ) );
            }
        }
    }

    void MtfTools::DrawPolyBezier( tools::Polygon rPolygon, bool bTo, bool bRecordPath )
    {
        sal_uInt16 nPoints = rPolygon.GetSize();
        if ( ( nPoints >= 4 ) && ( ( ( nPoints - 4 ) % 3 ) == 0 ) )
        {
            UpdateClipRegion();

            ImplMap( rPolygon );
            if ( bTo )
            {
                rPolygon[ 0 ] = maActPos;
                maActPos = rPolygon[ nPoints - 1 ];
            }
            sal_uInt16 i;
            for ( i = 0; ( i + 2 ) < nPoints; )
            {
                rPolygon.SetFlags( i++, PolyFlags::Normal );
                rPolygon.SetFlags( i++, PolyFlags::Control );
                rPolygon.SetFlags( i++, PolyFlags::Control );
            }
            if ( bRecordPath )
                maPathObj.AddPolyLine( rPolygon );
            else
            {
                UpdateLineStyle();
                mpGDIMetaFile->AddAction( new MetaPolyLineAction( rPolygon, maLineStyle.aLineInfo ) );
            }
        }
    }

    void MtfTools::DrawText( Point& rPosition, OUString const & rText, long* pDXArry, long* pDYArry, bool bRecordPath, sal_Int32 nGfxMode )
    {
        UpdateClipRegion();
        rPosition = ImplMap( rPosition );
        sal_Int32 nOldGfxMode = GetGfxMode();
        SetGfxMode( GM_COMPATIBLE );

        if (pDXArry)
        {
            sal_Int32 nSumX = 0, nSumY = 0;
            for (sal_Int32 i = 0; i < rText.getLength(); i++ )
            {
                nSumX += pDXArry[i];

                // #i121382# Map DXArray using WorldTransform
                const Size aSizeX(ImplMap(Size(nSumX, 0)));
                const basegfx::B2DVector aVectorX(aSizeX.Width(), aSizeX.Height());
                pDXArry[i] = basegfx::fround(aVectorX.getLength());
                pDXArry[i] *= (nSumX >= 0 ? 1 : -1);

                if (pDYArry)
                {
                    nSumY += pDYArry[i];

                    const Size aSizeY(ImplMap(Size(0, nSumY)));
                    const basegfx::B2DVector aVectorY(aSizeY.Width(), aSizeY.Height());
                    // Reverse Y
                    pDYArry[i] = basegfx::fround(aVectorY.getLength());
                    pDYArry[i] *= (nSumY >= 0 ? -1 : 1);
                }
            }
        }
        if ( mnLatestTextLayoutMode != mnTextLayoutMode )
        {
            mnLatestTextLayoutMode = mnTextLayoutMode;
            mpGDIMetaFile->AddAction( new MetaLayoutModeAction( mnTextLayoutMode ) );
        }
        SetGfxMode( nGfxMode );
        TextAlign eTextAlign;
        if ( ( mnTextAlign & TA_BASELINE) == TA_BASELINE )
            eTextAlign = ALIGN_BASELINE;
        else if( ( mnTextAlign & TA_BOTTOM) == TA_BOTTOM )
            eTextAlign = ALIGN_BOTTOM;
        else
            eTextAlign = ALIGN_TOP;
        bool bChangeFont = false;
        if ( mnLatestTextAlign != mnTextAlign )
        {
            bChangeFont = true;
            mnLatestTextAlign = mnTextAlign;
            mpGDIMetaFile->AddAction( new MetaTextAlignAction( eTextAlign ) );
        }
        if ( maLatestTextColor != maTextColor )
        {
            bChangeFont = true;
            maLatestTextColor = maTextColor;
            mpGDIMetaFile->AddAction( new MetaTextColorAction( maTextColor ) );
        }
        bool bChangeFillColor = false;
        if ( maLatestBkColor != maBkColor )
        {
            bChangeFillColor = true;
            maLatestBkColor = maBkColor;
        }
        if ( mnLatestBkMode != mnBkMode )
        {
            bChangeFillColor = true;
            mnLatestBkMode = mnBkMode;
        }
        if ( bChangeFillColor )
        {
            bChangeFont = true;
            mpGDIMetaFile->AddAction( new MetaTextFillColorAction( maFont.GetFillColor(), !maFont.IsTransparent() ) );
        }
        vcl::Font aTmp( maFont );
        aTmp.SetColor( maTextColor );
        aTmp.SetFillColor( maBkColor );

        if( mnBkMode == BkMode::Transparent )
            aTmp.SetTransparent( true );
        else
            aTmp.SetTransparent( false );

        aTmp.SetAlignment( eTextAlign );

        if ( nGfxMode == GM_ADVANCED )
        {
            // check whether there is a font rotation applied via transformation
            Point aP1( ImplMap( Point() ) );
            Point aP2( ImplMap( Point( 0, 100 ) ) );
            aP2.AdjustX( -(aP1.X()) );
            aP2.AdjustY( -(aP1.Y()) );
            double fX = aP2.X();
            double fY = aP2.Y();
            if ( fX )
            {
                double fOrientation = acos( fX / sqrt( fX * fX + fY * fY ) ) * 57.29577951308;
                if ( fY > 0 )
                    fOrientation = 360 - fOrientation;
                fOrientation += 90;
                fOrientation *= 10;
                fOrientation += aTmp.GetOrientation();
                aTmp.SetOrientation( sal_Int16( fOrientation ) );
            }
        }

        if( mnTextAlign & ( TA_UPDATECP | TA_RIGHT_CENTER ) )
        {
            // #i117968# VirtualDevice is not thread safe, but filter is used in multithreading
            SolarMutexGuard aGuard;
            ScopedVclPtrInstance< VirtualDevice > pVDev;
            sal_Int32 nTextWidth;
            Point aActPosDelta;
            pVDev->SetMapMode( MapMode( MapUnit::Map100thMM ) );
            pVDev->SetFont( maFont );
            const sal_uInt32 nLen = pDXArry ? rText.getLength() : 0;
            if (nLen)
            {
                nTextWidth = pVDev->GetTextWidth( OUString(rText[ nLen - 1 ]) );
                if( nLen > 1 )
                    nTextWidth += pDXArry[ nLen - 2 ];
                // tdf#39894: We should consider the distance to next character cell origin
                aActPosDelta.setX( pDXArry[ nLen - 1 ] );
                if ( pDYArry )
                {
                    aActPosDelta.setY( pDYArry[ nLen - 1 ] );
                }
            }
            else
            {
                nTextWidth = pVDev->GetTextWidth( rText );
                aActPosDelta.setX( nTextWidth );
            }

            if( mnTextAlign & TA_UPDATECP )
                rPosition = maActPos;

            if ( mnTextAlign & TA_RIGHT_CENTER )
            {
                Point aDisplacement( ( ( mnTextAlign & TA_RIGHT_CENTER ) == TA_RIGHT ) ? nTextWidth : nTextWidth >> 1, 0 );
                Point().RotateAround(aDisplacement, maFont.GetOrientation());
                rPosition -= aDisplacement;
            }

            if( mnTextAlign & TA_UPDATECP )
            {
                Point().RotateAround(aActPosDelta, maFont.GetOrientation());
                maActPos = rPosition + aActPosDelta;
            }
        }
        if ( bChangeFont || ( maLatestFont != aTmp ) )
        {
            maLatestFont = aTmp;
            mpGDIMetaFile->AddAction( new MetaFontAction( aTmp ) );
            mpGDIMetaFile->AddAction( new MetaTextAlignAction( aTmp.GetAlignment() ) );
            mpGDIMetaFile->AddAction( new MetaTextColorAction( aTmp.GetColor() ) );
            mpGDIMetaFile->AddAction( new MetaTextFillColorAction( aTmp.GetFillColor(), !aTmp.IsTransparent() ) );
        }
        if ( bRecordPath )
        {
            // TODO
        }
        else
        {
            if ( pDXArry && pDYArry )
            {
                for (sal_Int32 i = 0; i < rText.getLength(); ++i)
                {
                    Point aCharDisplacement( i ? pDXArry[i-1] : 0, i ? pDYArry[i-1] : 0 );
                    Point().RotateAround(aCharDisplacement, maFont.GetOrientation());
                    mpGDIMetaFile->AddAction( new MetaTextArrayAction( rPosition + aCharDisplacement, OUString( rText[i] ), nullptr, 0, 1 ) );
                }
            }
            else
            {
                /* because text without dx array is badly scaled, we
                   will create such an array if necessary */
                long* pDX = pDXArry;
                if (!pDXArry)
                {
                    // #i117968# VirtualDevice is not thread safe, but filter is used in multithreading
                    SolarMutexGuard aGuard;
                    ScopedVclPtrInstance< VirtualDevice > pVDev;
                    pDX = new long[ rText.getLength() ];
                    pVDev->SetMapMode(MapMode(MapUnit::Map100thMM));
                    pVDev->SetFont( maLatestFont );
                    pVDev->GetTextArray( rText, pDX, 0, rText.getLength());
                }
                mpGDIMetaFile->AddAction( new MetaTextArrayAction( rPosition, rText, pDX, 0, rText.getLength() ) );
                if ( !pDXArry )     // this means we have created our own array
                    delete[] pDX;   // which must be deleted
            }
        }
        SetGfxMode( nOldGfxMode );
    }

    void MtfTools::ImplDrawBitmap( const Point& rPos, const Size& rSize, const BitmapEx& rBitmap )
    {
        BitmapEx aBmpEx( rBitmap );
        if ( mbComplexClip )
        {
            vcl::bitmap::DrawAndClipBitmap(rPos, rSize, rBitmap, aBmpEx, maClipPath.getClipPath());
        }

        if ( aBmpEx.IsTransparent() )
            mpGDIMetaFile->AddAction( new MetaBmpExScaleAction( rPos, rSize, aBmpEx ) );
        else
            mpGDIMetaFile->AddAction( new MetaBmpScaleAction( rPos, rSize, aBmpEx.GetBitmap() ) );
    }

    void MtfTools::ResolveBitmapActions( std::vector<std::unique_ptr<BSaveStruct>>& rSaveList )
    {
        UpdateClipRegion();

        size_t nObjects     = rSaveList.size();
        size_t nObjectsLeft = nObjects;

        while ( nObjectsLeft )
        {
            size_t          i;
            size_t          nObjectsOfSameSize = 0;
            size_t          nObjectStartIndex = nObjects - nObjectsLeft;

            BSaveStruct*    pSave = rSaveList[nObjectStartIndex].get();
            tools::Rectangle       aRect( pSave->aOutRect );

            for ( i = nObjectStartIndex; i < nObjects; )
            {
                nObjectsOfSameSize++;
                if ( ++i < nObjects )
                {
                    pSave = rSaveList[i].get();
                    if ( pSave->aOutRect != aRect )
                        break;
                }
            }
            Point   aPos( ImplMap( aRect.TopLeft() ) );
            Size    aSize( ImplMap( aRect.GetSize() ) );

            for ( i = nObjectStartIndex; i < ( nObjectStartIndex + nObjectsOfSameSize ); i++ )
            {
                pSave = rSaveList[i].get();

                sal_uInt32  nWinRop = pSave->nWinRop;
                sal_uInt8   nRasterOperation = static_cast<sal_uInt8>( nWinRop >> 16 );

                sal_uInt32  nUsed =  0;
                if ( ( nRasterOperation & 0xf )  != ( nRasterOperation >> 4 ) )
                    nUsed |= 1;     // pattern is used
                if ( ( nRasterOperation & 0x33 ) != ( ( nRasterOperation & 0xcc ) >> 2 ) )
                    nUsed |= 2;     // source is used
                if ( ( nRasterOperation & 0xaa ) != ( ( nRasterOperation & 0x55 ) << 1 ) )
                    nUsed |= 4;     // destination is used

                if ( (nUsed & 1) && (( nUsed & 2 ) == 0) && nWinRop != PATINVERT )
                {   // patterns aren't well supported yet
                    WMFRasterOp nOldRop = SetRasterOp( WMFRasterOp::NONE );  // in this case nRasterOperation is either 0 or 0xff
                    UpdateFillStyle();
                    DrawRect( aRect, false );
                    SetRasterOp( nOldRop );
                }
                else
                {
                    bool bDrawn = false;

                    if ( i == nObjectStartIndex )   // optimizing, sometimes it is possible to create just one transparent bitmap
                    {
                        if ( nObjectsOfSameSize == 2 )
                        {
                            BSaveStruct* pSave2 = rSaveList[i + 1].get();
                            if ( ( pSave->aBmpEx.GetPrefSize() == pSave2->aBmpEx.GetPrefSize() ) &&
                                 ( pSave->aBmpEx.GetPrefMapMode() == pSave2->aBmpEx.GetPrefMapMode() ) )
                            {
                                // TODO: Strictly speaking, we should
                                // check whether mask is monochrome, and
                                // whether image is black (upper branch)
                                // or white (lower branch). Otherwise, the
                                // effect is not the same as a masked
                                // bitmap.
                                if ( ( nWinRop == SRCPAINT ) && ( pSave2->nWinRop == SRCAND ) )
                                {
                                    Bitmap aMask( pSave->aBmpEx.GetBitmap() ); aMask.Invert();
                                    BitmapEx aBmpEx( pSave2->aBmpEx.GetBitmap(), aMask );
                                    ImplDrawBitmap( aPos, aSize, aBmpEx );
                                    bDrawn = true;
                                    i++;
                                }
                                // #i20085# This is just the other way
                                // around as above. Only difference: mask
                                // is inverted
                                else if ( ( nWinRop == SRCAND ) && ( pSave2->nWinRop == SRCPAINT ) )
                                {
                                    Bitmap aMask( pSave->aBmpEx.GetBitmap() );
                                    BitmapEx aBmpEx( pSave2->aBmpEx.GetBitmap(), aMask );
                                    ImplDrawBitmap( aPos, aSize, aBmpEx );
                                    bDrawn = true;
                                    i++;
                                }
                                // tdf#90539
                                else if ( ( nWinRop == SRCAND ) && ( pSave2->nWinRop == SRCINVERT ) )
                                {
                                    Bitmap aMask( pSave->aBmpEx.GetBitmap() );
                                    BitmapEx aBmpEx( pSave2->aBmpEx.GetBitmap(), aMask );
                                    ImplDrawBitmap( aPos, aSize, aBmpEx );
                                    bDrawn = true;
                                    i++;
                                }
                            }
                        }
                    }

                    if ( !bDrawn )
                    {
                        Push();
                        WMFRasterOp nOldRop = SetRasterOp( WMFRasterOp::CopyPen );
                        Bitmap      aBitmap( pSave->aBmpEx.GetBitmap() );
                        sal_uInt32  nOperation = ( nRasterOperation & 0xf );
                        switch( nOperation )
                        {
                            case 0x1 :
                            case 0xe :
                            {
                                if(pSave->aBmpEx.IsAlpha())
                                {
                                    ImplDrawBitmap( aPos, aSize, pSave->aBmpEx );
                                }
                                else
                                {
                                    SetRasterOp( WMFRasterOp::XorPen );
                                    ImplDrawBitmap( aPos, aSize, BitmapEx(aBitmap) );
                                    SetRasterOp( WMFRasterOp::CopyPen );
                                    Bitmap  aMask( aBitmap );
                                    aMask.Invert();
                                    BitmapEx aBmpEx( aBitmap, aMask );
                                    ImplDrawBitmap( aPos, aSize, aBmpEx );
                                    if ( nOperation == 0x1 )
                                    {
                                        SetRasterOp( WMFRasterOp::Not );
                                        DrawRect( aRect, false );
                                    }
                                }
                            }
                            break;
                            case 0x7 :
                            case 0x8 :
                            {
                                Bitmap  aMask( aBitmap );
                                if ( ( nUsed & 1 ) && ( nRasterOperation & 0xb0 ) == 0xb0 )     // pattern used
                                {
                                    aBitmap.Convert( BmpConversion::N24Bit );
                                    aBitmap.Erase( maFillStyle.aFillColor );
                                }
                                BitmapEx aBmpEx( aBitmap, aMask );
                                ImplDrawBitmap( aPos, aSize, aBmpEx );
                                if ( nOperation == 0x7 )
                                {
                                    SetRasterOp( WMFRasterOp::Not );
                                    DrawRect( aRect, false );
                                }
                            }
                            break;

                            case 0x4 :
                            case 0xb :
                            {
                                SetRasterOp( WMFRasterOp::Not );
                                DrawRect( aRect, false );
                                SetRasterOp( WMFRasterOp::CopyPen );
                                Bitmap  aMask( aBitmap );
                                aBitmap.Invert();
                                BitmapEx aBmpEx( aBitmap, aMask );
                                ImplDrawBitmap( aPos, aSize, aBmpEx );
                                SetRasterOp( WMFRasterOp::XorPen );
                                ImplDrawBitmap( aPos, aSize, BitmapEx(aBitmap) );
                                if ( nOperation == 0xb )
                                {
                                    SetRasterOp( WMFRasterOp::Not );
                                    DrawRect( aRect, false );
                                }
                            }
                            break;

                            case 0x2 :
                            case 0xd :
                            {
                                Bitmap  aMask( aBitmap );
                                aMask.Invert();
                                BitmapEx aBmpEx( aBitmap, aMask );
                                ImplDrawBitmap( aPos, aSize, aBmpEx );
                                SetRasterOp( WMFRasterOp::XorPen );
                                ImplDrawBitmap( aPos, aSize, BitmapEx(aBitmap) );
                                if ( nOperation == 0xd )
                                {
                                    SetRasterOp( WMFRasterOp::Not );
                                    DrawRect( aRect, false );
                                }
                            }
                            break;
                            case 0x6 :
                            case 0x9 :
                            {
                                SetRasterOp( WMFRasterOp::XorPen );
                                ImplDrawBitmap( aPos, aSize, BitmapEx(aBitmap) );
                                if ( nOperation == 0x9 )
                                {
                                    SetRasterOp( WMFRasterOp::Not );
                                    DrawRect( aRect, false );
                                }
                            }
                            break;

                            case 0x0 :  // WHITENESS
                            case 0xf :  // BLACKNESS
                            {                                                   // in this case nRasterOperation is either 0 or 0xff
                                maFillStyle = WinMtfFillStyle( Color( nRasterOperation, nRasterOperation, nRasterOperation ) );
                                UpdateFillStyle();
                                DrawRect( aRect, false );
                            }
                            break;

                            case 0x3 :  // only source is used
                            case 0xc :
                            {
                                if ( nRasterOperation == 0x33 )
                                    aBitmap.Invert();
                                ImplDrawBitmap( aPos, aSize, BitmapEx(aBitmap) );
                            }
                            break;

                            case 0x5 :  // only destination is used
                            {
                                SetRasterOp( WMFRasterOp::Not );
                                DrawRect( aRect, false );
                            }
                            break;

                            case 0xa :  // no operation
                            break;
                        }
                        SetRasterOp( nOldRop );
                        Pop();
                    }
                }
            }
            nObjectsLeft -= nObjectsOfSameSize;
        }

        rSaveList.clear();
    }

    void MtfTools::SetDevOrg( const Point& rPoint )
    {
        mnDevOrgX = rPoint.X();
        mnDevOrgY = rPoint.Y();
    }

    void MtfTools::SetDevOrgOffset( sal_Int32 nXAdd, sal_Int32 nYAdd )
    {
        mnDevOrgX += nXAdd;
        mnDevOrgY += nYAdd;
    }

    void MtfTools::SetDevExt( const Size& rSize ,bool regular)
    {
        if ( rSize.Width() && rSize.Height() )
        {
            switch( mnMapMode )
            {
                case MM_ISOTROPIC :
                case MM_ANISOTROPIC :
                {
                    mnDevWidth = rSize.Width();
                    mnDevHeight = rSize.Height();
                }
            }
            if (regular)
            {
                mbIsMapDevSet=true;
            }
        }
    }

    void MtfTools::ScaleDevExt(double fX, double fY)
    {
        mnDevWidth = basegfx::fround(mnDevWidth * fX);
        mnDevHeight = basegfx::fround(mnDevHeight * fY);
    }

    void MtfTools::SetWinOrg( const Point& rPoint , bool bIsEMF)
    {
        mnWinOrgX = rPoint.X();
        mnWinOrgY = rPoint.Y();
        if (bIsEMF)
        {
            SetDevByWin();
        }
        mbIsMapWinSet=true;
    }

    void MtfTools::SetWinOrgOffset( sal_Int32 nXAdd, sal_Int32 nYAdd )
    {
        mnWinOrgX += nXAdd;
        mnWinOrgY += nYAdd;
    }

    void MtfTools::SetDevByWin() //mnWinExt...-stuff has to be assigned before.
    {
        if (!mbIsMapDevSet)
        {
            if ( mnMapMode == MM_ISOTROPIC ) //TODO: WHAT ABOUT ANISOTROPIC???
            {
                sal_Int32 nX, nY;
                if (o3tl::checked_add(mnWinExtX, mnWinOrgX, nX) || o3tl::checked_sub(mnWinExtY, mnWinOrgY, nY))
                    return;
                Size aSize(nX >> MS_FIXPOINT_BITCOUNT_28_4, -(nY >> MS_FIXPOINT_BITCOUNT_28_4));
                SetDevExt(aSize, false);
            }
        }
    }

    void MtfTools::SetWinExt(const Size& rSize, bool bIsEMF)
    {
        if (rSize.Width() && rSize.Height())
        {
            switch( mnMapMode )
            {
                case MM_ISOTROPIC :
                case MM_ANISOTROPIC :
                {
                    mnWinExtX = rSize.Width();
                    mnWinExtY = rSize.Height();
                    if (bIsEMF)
                    {
                        SetDevByWin();
                    }
                    mbIsMapWinSet = true;
                }
            }
        }
    }

    void MtfTools::ScaleWinExt(double fX, double fY)
    {
        mnWinExtX = basegfx::fround(mnWinExtX * fX);
        mnWinExtY = basegfx::fround(mnWinExtY * fY);
    }

    void MtfTools::SetrclBounds( const tools::Rectangle& rRect )
    {
        mrclBounds = rRect;
    }

    void MtfTools::SetrclFrame( const tools::Rectangle& rRect )
    {
        mrclFrame = rRect;
    }

    void MtfTools::SetRefPix( const Size& rSize )
    {
        mnPixX = rSize.Width();
        mnPixY = rSize.Height();
    }

    void MtfTools::SetRefMill( const Size& rSize )
    {
        mnMillX = rSize.Width();
        mnMillY = rSize.Height();
    }

    void MtfTools::SetMapMode( sal_uInt32 nMapMode )
    {
        mnMapMode = nMapMode;
        if ( nMapMode == MM_TEXT && !mbIsMapWinSet )
        {
            mnWinExtX = mnDevWidth;
            mnWinExtY = mnDevHeight;
        }
        else if ( mnMapMode == MM_HIMETRIC )
        {
            sal_Int32 nWinExtX, nWinExtY;
            if (o3tl::checked_multiply<sal_Int32>(mnMillX, 100, nWinExtX) ||
                o3tl::checked_multiply<sal_Int32>(mnMillY, 100, nWinExtY))
            {
                return;
            }
            mnWinExtX = nWinExtX;
            mnWinExtY = nWinExtY;
        }
    }

    void MtfTools::SetWorldTransform( const XForm& rXForm )
    {
        maXForm.eM11 = rXForm.eM11;
        maXForm.eM12 = rXForm.eM12;
        maXForm.eM21 = rXForm.eM21;
        maXForm.eM22 = rXForm.eM22;
        maXForm.eDx = rXForm.eDx;
        maXForm.eDy = rXForm.eDy;
    }

    void MtfTools::ModifyWorldTransform( const XForm& rXForm, sal_uInt32 nMode )
    {
        switch( nMode )
        {
            case MWT_IDENTITY :
            {
                maXForm.eM11 = maXForm.eM22 = 1.0f;
                maXForm.eM12 = maXForm.eM21 = maXForm.eDx = maXForm.eDy = 0.0f;
                break;
            }

            case MWT_RIGHTMULTIPLY :
            case MWT_LEFTMULTIPLY :
            {
                const XForm* pLeft;
                const XForm* pRight;

                if ( nMode == MWT_LEFTMULTIPLY )
                {
                    pLeft = &rXForm;
                    pRight = &maXForm;
                }
                else
                {
                    pLeft = &maXForm;
                    pRight = &rXForm;
                }

                float aF[3][3];
                float bF[3][3];
                float cF[3][3];

                aF[0][0] = pLeft->eM11;
                aF[0][1] = pLeft->eM12;
                aF[0][2] = 0;
                aF[1][0] = pLeft->eM21;
                aF[1][1] = pLeft->eM22;
                aF[1][2] = 0;
                aF[2][0] = pLeft->eDx;
                aF[2][1] = pLeft->eDy;
                aF[2][2] = 1;

                bF[0][0] = pRight->eM11;
                bF[0][1] = pRight->eM12;
                bF[0][2] = 0;
                bF[1][0] = pRight->eM21;
                bF[1][1] = pRight->eM22;
                bF[1][2] = 0;
                bF[2][0] = pRight->eDx;
                bF[2][1] = pRight->eDy;
                bF[2][2] = 1;

                int i, j, k;
                for ( i = 0; i < 3; i++ )
                {
                  for ( j = 0; j < 3; j++ )
                  {
                     cF[i][j] = 0;
                     for ( k = 0; k < 3; k++ )
                        cF[i][j] += aF[i][k] * bF[k][j];
                  }
                }
                maXForm.eM11 = cF[0][0];
                maXForm.eM12 = cF[0][1];
                maXForm.eM21 = cF[1][0];
                maXForm.eM22 = cF[1][1];
                maXForm.eDx = cF[2][0];
                maXForm.eDy = cF[2][1];
                break;
            }
            case MWT_SET:
            {
                SetWorldTransform(rXForm);
                break;
            }
        }
    }

    void MtfTools::Push()                       // !! to be able to access the original ClipRegion it
    {                                               // is not allowed to use the MetaPushAction()
        UpdateClipRegion();                         // (the original clip region is on top of the stack) (SJ)
        std::shared_ptr<SaveStruct> pSave( new SaveStruct );

        pSave->aLineStyle = maLineStyle;
        pSave->aFillStyle = maFillStyle;

        pSave->aFont = maFont;
        pSave->aTextColor = maTextColor;
        pSave->nTextAlign = mnTextAlign;
        pSave->nTextLayoutMode = mnTextLayoutMode;
        pSave->nMapMode = mnMapMode;
        pSave->nGfxMode = mnGfxMode;
        pSave->nBkMode = mnBkMode;
        pSave->aBkColor = maBkColor;
        pSave->bFillStyleSelected = mbFillStyleSelected;

        pSave->aActPos = maActPos;
        pSave->aXForm = maXForm;
        pSave->eRasterOp = meRasterOp;

        pSave->nWinOrgX = mnWinOrgX;
        pSave->nWinOrgY = mnWinOrgY;
        pSave->nWinExtX = mnWinExtX;
        pSave->nWinExtY = mnWinExtY;
        pSave->nDevOrgX = mnDevOrgX;
        pSave->nDevOrgY = mnDevOrgY;
        pSave->nDevWidth = mnDevWidth;
        pSave->nDevHeight = mnDevHeight;

        pSave->maPathObj = maPathObj;
        pSave->maClipPath = maClipPath;

        mvSaveStack.push_back( pSave );
    }

    void MtfTools::Pop()
    {
        // Get the latest data from the stack
        if( !mvSaveStack.empty() )
        {
            // Backup the current data on the stack
            std::shared_ptr<SaveStruct> pSave( mvSaveStack.back() );

            maLineStyle = pSave->aLineStyle;
            maFillStyle = pSave->aFillStyle;

            maFont = pSave->aFont;
            maTextColor = pSave->aTextColor;
            mnTextAlign = pSave->nTextAlign;
            mnTextLayoutMode = pSave->nTextLayoutMode;
            mnBkMode = pSave->nBkMode;
            mnGfxMode = pSave->nGfxMode;
            mnMapMode = pSave->nMapMode;
            maBkColor = pSave->aBkColor;
            mbFillStyleSelected = pSave->bFillStyleSelected;

            maActPos = pSave->aActPos;
            maXForm = pSave->aXForm;
            meRasterOp = pSave->eRasterOp;

            mnWinOrgX = pSave->nWinOrgX;
            mnWinOrgY = pSave->nWinOrgY;
            mnWinExtX = pSave->nWinExtX;
            mnWinExtY = pSave->nWinExtY;
            mnDevOrgX = pSave->nDevOrgX;
            mnDevOrgY = pSave->nDevOrgY;
            mnDevWidth = pSave->nDevWidth;
            mnDevHeight = pSave->nDevHeight;

            maPathObj = pSave->maPathObj;
            if ( ! ( maClipPath == pSave->maClipPath ) )
            {
                maClipPath = pSave->maClipPath;
                mbClipNeedsUpdate = true;
            }
            if ( meLatestRasterOp != meRasterOp )
            {
                mpGDIMetaFile->AddAction( new MetaRasterOpAction( meRasterOp ) );
                meLatestRasterOp = meRasterOp;
            }
            mvSaveStack.pop_back();
        }
    }

    void MtfTools::AddFromGDIMetaFile( GDIMetaFile& rGDIMetaFile )
    {
       rGDIMetaFile.Play( *mpGDIMetaFile );
    }

    void MtfTools::PassEMFPlusHeaderInfo()
    {
        EMFP_DEBUG(printf ("\t\t\tadd EMF_PLUS header info\n"));

        SvMemoryStream mem;
        sal_Int32 nLeft, nRight, nTop, nBottom;

        nLeft = mrclFrame.Left();
        nTop = mrclFrame.Top();
        nRight = mrclFrame.Right();
        nBottom = mrclFrame.Bottom();

        // emf header info
        mem.WriteInt32( nLeft ).WriteInt32( nTop ).WriteInt32( nRight ).WriteInt32( nBottom );
        mem.WriteInt32( mnPixX ).WriteInt32( mnPixY ).WriteInt32( mnMillX ).WriteInt32( mnMillY );

        float one, zero;

        one = 1;
        zero = 0;

        // add transformation matrix to be used in vcl's metaact.cxx for
        // rotate and scale operations
        mem.WriteFloat( one ).WriteFloat( zero ).WriteFloat( zero ).WriteFloat( one ).WriteFloat( zero ).WriteFloat( zero );

        // need to flush the stream, otherwise GetEndOfData will return 0
        // on windows where the function parameters are probably resolved in reverse order
        mem.Flush();

        mpGDIMetaFile->AddAction( new MetaCommentAction( "EMF_PLUS_HEADER_INFO", 0, static_cast<const sal_uInt8*>(mem.GetData()), mem.GetEndOfData() ) );
        mpGDIMetaFile->UseCanvas( true );
    }

    void MtfTools::PassEMFPlus( void const * pBuffer, sal_uInt32 nLength )
    {
        EMFP_DEBUG(printf ("\t\t\tadd EMF_PLUS comment length %04x\n",(unsigned int) nLength));
        mpGDIMetaFile->AddAction( new MetaCommentAction( "EMF_PLUS", 0, static_cast<const sal_uInt8*>(pBuffer), nLength ) );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
