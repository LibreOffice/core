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

#include "winmtf.hxx"
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <vcl/metaact.hxx>
#include <vcl/graphictools.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/metric.hxx>
#include <vcl/svapp.hxx>
#include <tools/fract.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/tencinfo.h>
#include <vcl/virdev.hxx>

#if OSL_DEBUG_LEVEL > 1
#define EMFP_DEBUG(x) x
#else
#define EMFP_DEBUG(x)
#endif

void WinMtfClipPath::intersectClipRect( const Rectangle& rRect )
{
    maClip.intersectRange(
        vcl::unotools::b2DRectangleFromRectangle(rRect));
}

void WinMtfClipPath::excludeClipRect( const Rectangle& rRect )
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
            maClip = basegfx::tools::B2DClipState(rB2DPoly);
            break;
    }
}

void WinMtfClipPath::moveClipRegion( const Size& rSize )
{
    // what a weird concept. emulate, don't want this in B2DClipState
    // API
    basegfx::B2DPolyPolygon aCurrClip=maClip.getClipPoly();
    basegfx::B2DHomMatrix aTranslate;
    aTranslate.translate(rSize.Width(), rSize.Height());

    aCurrClip.transform(aTranslate);
    maClip = basegfx::tools::B2DClipState( aCurrClip );
}

void WinMtfClipPath::setDefaultClipPath()
{
    // Empty clip region - everything visible
    maClip = basegfx::tools::B2DClipState();
}

basegfx::B2DPolyPolygon WinMtfClipPath::getClipPath() const
{
    return maClip.getClipPoly();
}

void WinMtfPathObj::AddPoint( const Point& rPoint )
{
    if ( bClosed )
        Insert( tools::Polygon() );
    tools::Polygon& rPoly = ((tools::PolyPolygon&)*this)[ Count() - 1 ];
    rPoly.Insert( rPoly.GetSize(), rPoint );
    bClosed = false;
}

void WinMtfPathObj::AddPolyLine( const tools::Polygon& rPolyLine )
{
    if ( bClosed )
        Insert( tools::Polygon() );
    tools::Polygon& rPoly = ((tools::PolyPolygon&)*this)[ Count() - 1 ];
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
        tools::Polygon& rPoly = ((tools::PolyPolygon&)*this)[ Count() - 1 ];
        if ( rPoly.GetSize() > 2 )
        {
            Point aFirst( rPoly[ 0 ] );
            if ( aFirst != rPoly[ rPoly.GetSize() - 1 ] )
                rPoly.Insert( rPoly.GetSize(), aFirst );
        }
    }
    bClosed = true;
}

WinMtfFontStyle::WinMtfFontStyle( LOGFONTW& rFont )
{
    rtl_TextEncoding eCharSet;
    if ( ( rFont.lfCharSet == OEM_CHARSET ) || ( rFont.lfCharSet == DEFAULT_CHARSET ) )
        eCharSet = osl_getThreadTextEncoding();
    else
        eCharSet = rtl_getTextEncodingFromWindowsCharset( rFont.lfCharSet );
    if ( eCharSet == RTL_TEXTENCODING_DONTKNOW )
        eCharSet = RTL_TEXTENCODING_MS_1252;
    aFont.SetCharSet( eCharSet );
    aFont.SetName( rFont.alfFaceName );
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
    if( rFont.lfWeight <= FW_THIN )
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
        aFont.SetUnderline( UNDERLINE_SINGLE );

    if( rFont.lfStrikeOut )
        aFont.SetStrikeout( STRIKEOUT_SINGLE );

    if ( rFont.lfOrientation )
        aFont.SetOrientation( (short)rFont.lfOrientation );
    else
        aFont.SetOrientation( (short)rFont.lfEscapement );

    Size aFontSize( Size( rFont.lfWidth, rFont.lfHeight ) );
    if ( rFont.lfHeight > 0 )
    {
        // #i117968# VirtualDevice is not thread safe, but filter is used in multithreading
        SolarMutexGuard aGuard;
        ScopedVclPtrInstance< VirtualDevice > pVDev;
        // converting the cell height into a font height
        aFont.SetSize( aFontSize );
        pVDev->SetFont( aFont );
        FontMetric aMetric( pVDev->GetFontMetric() );
        long nHeight = aMetric.GetAscent() + aMetric.GetDescent();
        if (nHeight)
        {
            double fHeight = ((double)aFontSize.Height() * rFont.lfHeight ) / nHeight;
            aFontSize.Height() = (sal_Int32)( fHeight + 0.5 );
        }
    }

    // Convert height to positive
    aFontSize.Height() = std::abs(aFontSize.Height());

    aFont.SetSize(aFontSize);
};

WinMtf::WinMtf( WinMtfOutput* pWinMtfOutput, SvStream& rStreamWMF, FilterConfigItem* pConfigItem )
    : pOut( pWinMtfOutput )
    , pWMF( &rStreamWMF )
    , nEndPos( 0 )
    , pFilterConfigItem( pConfigItem )
{
    SvLockBytes *pLB = pWMF->GetLockBytes();
    if ( pLB )
        pLB->SetSynchronMode();

    nStartPos = pWMF->Tell();

    pOut->SetDevOrg( Point() );
    if ( pFilterConfigItem )
    {
        xStatusIndicator = pFilterConfigItem->GetStatusIndicator();
        if ( xStatusIndicator.is() )
        {
            OUString aMsg;
            xStatusIndicator->start( aMsg, 100 );
        }
    }
}

WinMtf::~WinMtf()
{
    delete pOut;

    if ( xStatusIndicator.is() )
        xStatusIndicator->end();
}

void WinMtf::Callback( sal_uInt16 nPercent )
{
    if ( xStatusIndicator.is() )
        xStatusIndicator->setValue( nPercent );
}

Color WinMtf::ReadColor()
{
    sal_uInt32 nColor;
    pWMF->ReadUInt32( nColor );
    return Color( (sal_uInt8)nColor, (sal_uInt8)( nColor >> 8 ), (sal_uInt8)( nColor >> 16 ) );
};

Point WinMtfOutput::ImplScale(const Point& rPoint) // Hack to set varying defaults for incompletely defined files.
{
    if (!mbIsMapDevSet)
        return Point(rPoint.X() * UNDOCUMENTED_WIN_RCL_RELATION - mrclFrame.Left(),
                     rPoint.Y() * UNDOCUMENTED_WIN_RCL_RELATION - mrclFrame.Top());
    else
        return rPoint;
}

Point WinMtfOutput::ImplMap( const Point& rPt )
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
                    fX2 -= mnWinOrgX;
                    fY2 -= mnWinOrgY;
                    fX2 /= mnWinExtX;
                    fY2 /= mnWinExtY;
                    fX2 *= mnDevWidth;
                    fY2 *= mnDevHeight;
                    fX2 += mnDevOrgX;
                    fY2 += mnDevOrgY;   // fX2, fY2 now in device units
                    fX2 *= (double)mnMillX * 100.0 / (double)mnPixX;
                    fY2 *= (double)mnMillY * 100.0 / (double)mnPixY;
                }
                break;
            }
            fX2 -= mrclFrame.Left();
            fY2 -= mrclFrame.Top();
        }
        return Point( FRound( fX2 ), FRound( fY2 ) );
    }
    else
        return Point();
};

Size WinMtfOutput::ImplMap(const Size& rSz, bool bDoWorldTransform)
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
                    fWidth /= mnWinExtX;
                    fHeight /= mnWinExtY;
                    fWidth *= mnDevWidth;
                    fHeight *= mnDevHeight;
                    fWidth *= (double)mnMillX * 100 / (double)mnPixX;
                    fHeight *= (double)mnMillY * 100 / (double)mnPixY;
                }
                break;
            }
        }
        return Size( FRound( fWidth ), FRound( fHeight ) );
    }
    else
        return Size();
}

Rectangle WinMtfOutput::ImplMap( const Rectangle& rRect )
{
    return Rectangle( ImplMap( rRect.TopLeft() ), ImplMap( rRect.GetSize() ) );
}

void WinMtfOutput::ImplMap( vcl::Font& rFont )
{
    // !!! HACK: we now always set the width to zero because the OS width is interpreted differently;
    // must later be made portable in SV (KA 1996-02-08)
    Size  aFontSize = ImplMap (rFont.GetSize(), false);

    if( aFontSize.Height() < 0 )
        aFontSize.Height() *= -1;

    rFont.SetSize( aFontSize );

    if( ( mnWinExtX * mnWinExtY ) < 0 )
        rFont.SetOrientation( 3600 - rFont.GetOrientation() );
}

tools::Polygon& WinMtfOutput::ImplMap( tools::Polygon& rPolygon )
{
    sal_uInt16 nPoints = rPolygon.GetSize();
    for ( sal_uInt16 i = 0; i < nPoints; i++ )
    {
        rPolygon[ i ] = ImplMap( rPolygon[ i ] );
    }
    return rPolygon;
}

tools::Polygon& WinMtfOutput::ImplScale( tools::Polygon& rPolygon )
{
    sal_uInt16 nPoints = rPolygon.GetSize();
    for ( sal_uInt16 i = 0; i < nPoints; i++ )
    {
        rPolygon[ i ] = ImplScale( rPolygon[ i ] );
    }
    return rPolygon;
}

tools::PolyPolygon& WinMtfOutput::ImplScale( tools::PolyPolygon& rPolyPolygon )
{
    sal_uInt16 nPolys = rPolyPolygon.Count();
    for (sal_uInt16 i = 0; i < nPolys; ++i)
    {
        ImplScale(rPolyPolygon[i]);
    }
    return rPolyPolygon;
}

tools::PolyPolygon& WinMtfOutput::ImplMap( tools::PolyPolygon& rPolyPolygon )
{
    sal_uInt16 nPolys = rPolyPolygon.Count();
    for ( sal_uInt16 i = 0; i < nPolys; ImplMap( rPolyPolygon[ i++ ] ) ) ;
    return rPolyPolygon;
}

void WinMtfOutput::SelectObject( sal_Int32 nIndex )
{
    GDIObj* pGDIObj = nullptr;

    if ( nIndex & ENHMETA_STOCK_OBJECT )
        pGDIObj = new GDIObj();
    else
    {
        nIndex &= 0xffff;       // safety check: don't allow index to be > 65535

        if ( (sal_uInt32)nIndex < vGDIObj.size() )
            pGDIObj = vGDIObj[ nIndex ];
    }

    if( pGDIObj == nullptr )
        return;

    if ( nIndex & ENHMETA_STOCK_OBJECT )
    {
        sal_uInt16 nStockId = (sal_uInt8)nIndex;
        switch( nStockId )
        {
            case WHITE_BRUSH :
            {
                pGDIObj->Set( GDI_BRUSH, new WinMtfFillStyle( Color( COL_WHITE ) ) );
            }
            break;
            case LTGRAY_BRUSH :
            {
                pGDIObj->Set( GDI_BRUSH, new WinMtfFillStyle( Color( COL_LIGHTGRAY ) ) );
            }
            break;
            case GRAY_BRUSH :
            case DKGRAY_BRUSH :
            {
                pGDIObj->Set( GDI_BRUSH, new WinMtfFillStyle( Color( COL_GRAY ) ) );
            }
            break;
            case BLACK_BRUSH :
            {
                pGDIObj->Set( GDI_BRUSH, new WinMtfFillStyle( Color( COL_BLACK ) ) );
            }
            break;
            case NULL_BRUSH :
            {
                pGDIObj->Set( GDI_BRUSH, new WinMtfFillStyle( Color( COL_TRANSPARENT ), true ) );
            }
            break;
            case WHITE_PEN :
            {
                pGDIObj->Set( GDI_PEN, new WinMtfLineStyle( Color( COL_WHITE ) ) );
            }
            break;
            case BLACK_PEN :
            {
                pGDIObj->Set( GDI_PEN, new WinMtfLineStyle( Color( COL_BLACK ) ) );
            }
            break;
            case NULL_PEN :
            {
                pGDIObj->Set( GDI_PEN, new WinMtfLineStyle( Color( COL_TRANSPARENT ), true ) );
            }
            break;
            default:
            break;
        }
    }
    if ( pGDIObj->pStyle )
    {
        switch( pGDIObj->eType )
        {
            case GDI_PEN :
                maLineStyle = static_cast<WinMtfLineStyle*>(pGDIObj->pStyle);
            break;
            case GDI_BRUSH :
            {
                maFillStyle = static_cast<WinMtfFillStyle*>(pGDIObj->pStyle);
                mbFillStyleSelected = true;
            }
            break;
            case GDI_FONT :
                maFont = static_cast<WinMtfFontStyle*>(pGDIObj->pStyle)->aFont;
            break;
            default:
            break;  //  -Wall many options not handled.
        }
    }
    if ( nIndex & ENHMETA_STOCK_OBJECT )
        delete pGDIObj;
}


void WinMtfOutput::SetTextLayoutMode( ComplexTextLayoutMode nTextLayoutMode )
{
    mnTextLayoutMode = nTextLayoutMode;
}

void WinMtfOutput::SetBkMode( BkMode nMode )
{
    mnBkMode = nMode;
}

void WinMtfOutput::SetBkColor( const Color& rColor )
{
    maBkColor = rColor;
}

void WinMtfOutput::SetTextColor( const Color& rColor )
{
    maTextColor = rColor;
}

void WinMtfOutput::SetTextAlign( sal_uInt32 nAlign )
{
    mnTextAlign = nAlign;
}

void WinMtfOutput::ImplResizeObjectArry( sal_uInt32 nNewEntrys )
{
    vGDIObj.resize(nNewEntrys, nullptr);
}

void WinMtfOutput::ImplDrawClippedPolyPolygon( const tools::PolyPolygon& rPolyPoly )
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

void WinMtfOutput::CreateObject( GDIObjectType eType, void* pStyle )
{
    if ( pStyle )
    {
        if ( eType == GDI_FONT )
        {
            WinMtfFontStyle* pFontStyle = static_cast<WinMtfFontStyle*>(pStyle);
            if (pFontStyle->aFont.GetHeight() == 0)
                pFontStyle->aFont.SetHeight(423);
            ImplMap(pFontStyle->aFont); // defaulting to 12pt
        }
        else if ( eType == GDI_PEN )
        {
            WinMtfLineStyle* pLineStyle = static_cast<WinMtfLineStyle*>(pStyle);
            Size aSize(pLineStyle->aLineInfo.GetWidth(), 0);
            aSize = ImplMap(aSize);
            pLineStyle->aLineInfo.SetWidth(aSize.Width());
        }
    }
    sal_uInt32 nIndex;
    for ( nIndex = 0; nIndex < vGDIObj.size(); nIndex++ )
    {
        if ( vGDIObj[ nIndex ] == nullptr )
            break;
    }
    if ( nIndex == vGDIObj.size() )
        ImplResizeObjectArry( vGDIObj.size() + 16 );

    vGDIObj[ nIndex ] = new GDIObj( eType, pStyle );
}

void WinMtfOutput::CreateObject( sal_Int32 nIndex, GDIObjectType eType, void* pStyle )
{
    if ( ( nIndex & ENHMETA_STOCK_OBJECT ) == 0 )
    {
        nIndex &= 0xffff;       // safety check: do not allow index to be > 65535
        if ( pStyle )
        {
            if ( eType == GDI_FONT )
            {
                WinMtfFontStyle* pFontStyle = static_cast<WinMtfFontStyle*>(pStyle);
                if (pFontStyle->aFont.GetHeight() == 0)
                    pFontStyle->aFont.SetHeight(423);
                ImplMap(pFontStyle->aFont);
            }
            else if ( eType == GDI_PEN )
            {
                WinMtfLineStyle* pLineStyle = static_cast<WinMtfLineStyle*>(pStyle);
                Size aSize(pLineStyle->aLineInfo.GetWidth(), 0);
                aSize = ImplMap(aSize);
                pLineStyle->aLineInfo.SetWidth(aSize.Width());
            }
        }
        if ( (sal_uInt32)nIndex >= vGDIObj.size() )
            ImplResizeObjectArry( nIndex + 16 );

        if ( vGDIObj[ nIndex ] != nullptr )
            delete vGDIObj[ nIndex ];

        vGDIObj[ nIndex ] = new GDIObj( eType, pStyle );
    }
    else
    {
        switch ( eType )
        {
            case GDI_PEN :
                delete static_cast<WinMtfLineStyle*>(pStyle);
            break;
            case GDI_BRUSH :
                delete static_cast<WinMtfFillStyle*>(pStyle);
            break;
            case GDI_FONT :
                delete static_cast<WinMtfFontStyle*>(pStyle);
            break;

            default:
                OSL_FAIL( "unsupported style not deleted" );
                break;
        }
    }
}

void WinMtfOutput::DeleteObject( sal_Int32 nIndex )
{
    if ( ( nIndex & ENHMETA_STOCK_OBJECT ) == 0 )
    {
        if ( (sal_uInt32)nIndex < vGDIObj.size() )
        {
            delete vGDIObj[ nIndex ];
            vGDIObj[ nIndex ] = nullptr;
        }
    }
}

void WinMtfOutput::IntersectClipRect( const Rectangle& rRect )
{
    mbClipNeedsUpdate=true;
    if ((rRect.Left()-rRect.Right()==0) && (rRect.Top()-rRect.Bottom()==0))
    {
        return; // empty rectangles cause trouble
    }
    aClipPath.intersectClipRect( ImplMap( rRect ) );
}

void WinMtfOutput::ExcludeClipRect( const Rectangle& rRect )
{
    mbClipNeedsUpdate=true;
    aClipPath.excludeClipRect( ImplMap( rRect ) );
}

void WinMtfOutput::MoveClipRegion( const Size& rSize )
{
    mbClipNeedsUpdate=true;
    aClipPath.moveClipRegion( ImplMap( rSize ) );
}

void WinMtfOutput::SetClipPath( const tools::PolyPolygon& rPolyPolygon, sal_Int32 nClippingMode, bool bIsMapped )
{
    mbClipNeedsUpdate = true;
    tools::PolyPolygon aPolyPolygon(rPolyPolygon);

    if (!bIsMapped)
    {
        if (!mbIsMapDevSet && (mnMapMode == MM_ISOTROPIC || mnMapMode == MM_ANISOTROPIC))
            aPolyPolygon = ImplScale(aPolyPolygon);
        else
            aPolyPolygon = ImplMap(aPolyPolygon);
    }
    aClipPath.setClipPath(aPolyPolygon, nClippingMode);
}

void WinMtfOutput::SetDefaultClipPath()
{
    mbClipNeedsUpdate = true;
    aClipPath.setDefaultClipPath();
}

WinMtfOutput::WinMtfOutput( GDIMetaFile& rGDIMetaFile ) :
    mnLatestTextAlign   ( 0 ),
    mnTextAlign         ( TA_LEFT | TA_TOP | TA_NOUPDATECP ),
    maLatestBkColor     ( 0x12345678 ),
    maBkColor           ( COL_WHITE ),
    mnLatestTextLayoutMode( TEXT_LAYOUT_DEFAULT ),
    mnTextLayoutMode    ( TEXT_LAYOUT_DEFAULT ),
    mnLatestBkMode      ( BkMode::NONE ),
    mnBkMode            ( BkMode::OPAQUE ),
    meLatestRasterOp    ( ROP_INVERT ),
    meRasterOp          ( ROP_OVERPAINT ),
    maActPos            ( Point() ),
    mbNopMode           ( false ),
    mbFillStyleSelected ( false ),
    mbClipNeedsUpdate   ( true ),
    mbComplexClip       ( false ),
    mnGfxMode           ( GM_COMPATIBLE ),
    mnMapMode           ( MM_TEXT ),
    mnDevOrgX           ( 0 ),
    mnDevOrgY           ( 0 ),
    mnDevWidth          ( 1 ),
    mnDevHeight         ( 1 ),
    mnWinOrgX           ( 0 ),
    mnWinOrgY           ( 0 ),
    mnWinExtX           ( 1 ),
    mnWinExtY           ( 1 ),
    mnPixX              ( 100 ),
    mnPixY              ( 100 ),
    mnMillX             ( 1 ),
    mnMillY             ( 1 ),
    mpGDIMetaFile       ( &rGDIMetaFile )
{
    mbIsMapWinSet = false;
    mbIsMapDevSet = false;
    mpGDIMetaFile->AddAction( new MetaPushAction( PushFlags::CLIPREGION ) );      // The original clipregion has to be on top
                                                                            // of the stack so it can always be restored
                                                                            // this is necessary to be able to support
                                                                            // SetClipRgn( NULL ) and similar ClipRgn actions (SJ)

    maFont.SetName( "Arial" );                                              // sj: #i57205#, we do have some scaling problems if using
    maFont.SetCharSet( RTL_TEXTENCODING_MS_1252 );                          // the default font then most times a x11 font is used, we
    maFont.SetHeight( 423 );                                                // will prevent this defining a font

    maLatestLineStyle.aLineColor = Color( 0x12, 0x34, 0x56 );
    maLatestFillStyle.aFillColor = Color( 0x12, 0x34, 0x56 );

    mnRop = R2_BLACK + 1;
    SetRasterOp( R2_BLACK );
}

WinMtfOutput::~WinMtfOutput()
{
    mpGDIMetaFile->AddAction( new MetaPopAction() );
    mpGDIMetaFile->SetPrefMapMode( MAP_100TH_MM );
    if ( mrclFrame.IsEmpty() )
        mpGDIMetaFile->SetPrefSize( Size( mnDevWidth, mnDevHeight ) );
    else
        mpGDIMetaFile->SetPrefSize( mrclFrame.GetSize() );

    for ( size_t i = 0; i < vGDIObj.size(); i++ )
        delete vGDIObj[ i ];
}

void WinMtfOutput::UpdateClipRegion()
{
    if ( mbClipNeedsUpdate )
    {
        mbClipNeedsUpdate = false;
        mbComplexClip = false;

        mpGDIMetaFile->AddAction( new MetaPopAction() );                    // taking the original clipregion
        mpGDIMetaFile->AddAction( new MetaPushAction( PushFlags::CLIPREGION ) );

        // skip for 'no clipping at all' case
        if( !aClipPath.isEmpty() )
        {
            const basegfx::B2DPolyPolygon& rClipPoly( aClipPath.getClipPath() );
            mpGDIMetaFile->AddAction(
                new MetaISectRectClipRegionAction(
                    vcl::unotools::rectangleFromB2DRectangle(
                        rClipPoly.getB2DRange())));

            mbComplexClip = rClipPoly.count() > 1
                || !basegfx::tools::isRectangle(rClipPoly);
        }
    }
}

void WinMtfOutput::ImplSetNonPersistentLineColorTransparenz()
{
    Color aColor(  COL_TRANSPARENT);
    WinMtfLineStyle aTransparentLine( aColor, true );
    if ( ! ( maLatestLineStyle == aTransparentLine ) )
    {
        maLatestLineStyle = aTransparentLine;
        mpGDIMetaFile->AddAction( new MetaLineColorAction( aTransparentLine.aLineColor, !aTransparentLine.bTransparent ) );
    }
}

void WinMtfOutput::UpdateLineStyle()
{
    if (!( maLatestLineStyle == maLineStyle ) )
    {
        maLatestLineStyle = maLineStyle;
        mpGDIMetaFile->AddAction( new MetaLineColorAction( maLineStyle.aLineColor, !maLineStyle.bTransparent ) );
    }
}

void WinMtfOutput::UpdateFillStyle()
{
    if ( !mbFillStyleSelected )     // SJ: #i57205# taking care of bkcolor if no brush is selected
        maFillStyle = WinMtfFillStyle( maBkColor, mnBkMode == BkMode::Transparent );
    if (!( maLatestFillStyle == maFillStyle ) )
    {
        maLatestFillStyle = maFillStyle;
        if (maFillStyle.aType == FillStyleSolid)
            mpGDIMetaFile->AddAction( new MetaFillColorAction( maFillStyle.aFillColor, !maFillStyle.bTransparent ) );
    }
}

sal_uInt32 WinMtfOutput::SetRasterOp( sal_uInt32 nRasterOp )
{
    sal_uInt32 nRetROP = mnRop;
    if ( nRasterOp != mnRop )
    {
        mnRop = nRasterOp;

        if ( mbNopMode && ( nRasterOp != R2_NOP ) )
        {   // changing modes from R2_NOP so set pen and brush
            maFillStyle = m_NopFillStyle;
            maLineStyle = m_NopLineStyle;
            mbNopMode = false;
        }
        switch( nRasterOp )
        {
            case R2_NOT:
                meRasterOp = ROP_INVERT;
            break;

            case R2_XORPEN:
                meRasterOp = ROP_XOR;
            break;

            case R2_NOP:
            {
                meRasterOp = ROP_OVERPAINT;
                if( !mbNopMode )
                {
                    m_NopFillStyle = maFillStyle;
                    m_NopLineStyle = maLineStyle;
                    maFillStyle = WinMtfFillStyle( Color( COL_TRANSPARENT ), true );
                    maLineStyle = WinMtfLineStyle( Color( COL_TRANSPARENT ), true );
                    mbNopMode = true;
                }
            }
            break;

            default:
                meRasterOp = ROP_OVERPAINT;
            break;
        }
    }
    if ( nRetROP != nRasterOp )
        mpGDIMetaFile->AddAction( new MetaRasterOpAction( meRasterOp ) );
    return nRetROP;
};

void WinMtfOutput::StrokeAndFillPath( bool bStroke, bool bFill )
{
    if ( aPathObj.Count() )
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
            if ( aPathObj.Count() == 1 )
                mpGDIMetaFile->AddAction( new MetaPolygonAction( aPathObj.GetObject( 0 ) ) );
            else
                mpGDIMetaFile->AddAction( new MetaPolyPolygonAction( aPathObj ) );

            if ( !bStroke )
                mpGDIMetaFile->AddAction( new MetaPopAction() );
        }
        else
        {
            sal_uInt16 i, nCount = aPathObj.Count();
            for ( i = 0; i < nCount; i++ )
                mpGDIMetaFile->AddAction( new MetaPolyLineAction( aPathObj[ i ], maLineStyle.aLineInfo ) );
        }
        ClearPath();
    }
}

void WinMtfOutput::DrawPixel( const Point& rSource, const Color& rColor )
{
    mpGDIMetaFile->AddAction( new MetaPixelAction( ImplMap( rSource), rColor ) );
}

void WinMtfOutput::MoveTo( const Point& rPoint, bool bRecordPath )
{
    Point aDest( ImplMap( rPoint ) );
    if ( bRecordPath )
    {
        // fdo#57353 create new subpath for subsequent moves
        if ( aPathObj.Count() )
            if ( aPathObj[ aPathObj.Count() - 1 ].GetSize() )
                aPathObj.Insert( tools::Polygon() );
        aPathObj.AddPoint( aDest );
    }
    maActPos = aDest;
}

void WinMtfOutput::LineTo( const Point& rPoint, bool bRecordPath )
{
    UpdateClipRegion();
    Point aDest( ImplMap( rPoint ) );
    if ( bRecordPath )
        aPathObj.AddPoint( aDest );
    else
    {
        UpdateLineStyle();
        mpGDIMetaFile->AddAction( new MetaLineAction( maActPos, aDest, maLineStyle.aLineInfo ) );
    }
    maActPos = aDest;
}

void WinMtfOutput::DrawRect( const Rectangle& rRect, bool bEdge )
{
    UpdateClipRegion();
    UpdateFillStyle();

    if ( mbComplexClip )
    {
        tools::Polygon aPoly( ImplMap( rRect ) );
        tools::PolyPolygon aPolyPolyRect( aPoly );
        tools::PolyPolygon aDest;
        tools::PolyPolygon(aClipPath.getClipPath()).GetIntersection( aPolyPolyRect, aDest );
        ImplDrawClippedPolyPolygon( aDest );
    }
    else
    {
        if ( bEdge )
        {
            if ( maLineStyle.aLineInfo.GetWidth() || ( maLineStyle.aLineInfo.GetStyle() == LINE_DASH ) )
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

void WinMtfOutput::DrawRoundRect( const Rectangle& rRect, const Size& rSize )
{
    UpdateClipRegion();
    UpdateLineStyle();
    UpdateFillStyle();
    mpGDIMetaFile->AddAction( new MetaRoundRectAction( ImplMap( rRect ), labs( ImplMap( rSize ).Width() ), labs( ImplMap( rSize ).Height() ) ) );
}

void WinMtfOutput::DrawEllipse( const Rectangle& rRect )
{
    UpdateClipRegion();
    UpdateFillStyle();

    if ( maLineStyle.aLineInfo.GetWidth() || ( maLineStyle.aLineInfo.GetStyle() == LINE_DASH ) )
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

void WinMtfOutput::DrawArc( const Rectangle& rRect, const Point& rStart, const Point& rEnd, bool bTo )
{
    UpdateClipRegion();
    UpdateLineStyle();
    UpdateFillStyle();

    Rectangle   aRect( ImplMap( rRect ) );
    Point       aStart( ImplMap( rStart ) );
    Point       aEnd( ImplMap( rEnd ) );

    if ( maLineStyle.aLineInfo.GetWidth() || ( maLineStyle.aLineInfo.GetStyle() == LINE_DASH ) )
    {
        if ( aStart == aEnd )
        {   // SJ: #i53768# if start & end is identical, then we have to draw a full ellipse
            Point aCenter( aRect.Center() );
            Size  aRad( aRect.GetWidth() / 2, aRect.GetHeight() / 2 );

            mpGDIMetaFile->AddAction( new MetaPolyLineAction( tools::Polygon( aCenter, aRad.Width(), aRad.Height() ), maLineStyle.aLineInfo ) );
        }
        else
            mpGDIMetaFile->AddAction( new MetaPolyLineAction( tools::Polygon( aRect, aStart, aEnd, POLY_ARC ), maLineStyle.aLineInfo ) );
    }
    else
        mpGDIMetaFile->AddAction( new MetaArcAction( aRect, aStart, aEnd ) );

    if ( bTo )
        maActPos = aEnd;
}

void WinMtfOutput::DrawPie( const Rectangle& rRect, const Point& rStart, const Point& rEnd )
{
    UpdateClipRegion();
    UpdateFillStyle();

    Rectangle   aRect( ImplMap( rRect ) );
    Point       aStart( ImplMap( rStart ) );
    Point       aEnd( ImplMap( rEnd ) );

    if ( maLineStyle.aLineInfo.GetWidth() || ( maLineStyle.aLineInfo.GetStyle() == LINE_DASH ) )
    {
        ImplSetNonPersistentLineColorTransparenz();
        mpGDIMetaFile->AddAction( new MetaPieAction( aRect, aStart, aEnd ) );
        UpdateLineStyle();
        mpGDIMetaFile->AddAction( new MetaPolyLineAction( tools::Polygon( aRect, aStart, aEnd, POLY_PIE ), maLineStyle.aLineInfo ) );
    }
    else
    {
        UpdateLineStyle();
        mpGDIMetaFile->AddAction( new MetaPieAction( aRect, aStart, aEnd ) );
    }
}

void WinMtfOutput::DrawChord( const Rectangle& rRect, const Point& rStart, const Point& rEnd )
{
    UpdateClipRegion();
    UpdateFillStyle();

    Rectangle   aRect( ImplMap( rRect ) );
    Point       aStart( ImplMap( rStart ) );
    Point       aEnd( ImplMap( rEnd ) );

    if ( maLineStyle.aLineInfo.GetWidth() || ( maLineStyle.aLineInfo.GetStyle() == LINE_DASH ) )
    {
        ImplSetNonPersistentLineColorTransparenz();
        mpGDIMetaFile->AddAction( new MetaChordAction( aRect, aStart, aEnd ) );
        UpdateLineStyle();
        mpGDIMetaFile->AddAction( new MetaPolyLineAction( tools::Polygon( aRect, aStart, aEnd, POLY_CHORD ), maLineStyle.aLineInfo ) );
    }
    else
    {
        UpdateLineStyle();
        mpGDIMetaFile->AddAction( new MetaChordAction( aRect, aStart, aEnd ) );
    }
}

void WinMtfOutput::DrawPolygon( tools::Polygon& rPolygon, bool bRecordPath )
{
    UpdateClipRegion();
    ImplMap( rPolygon );
    if ( bRecordPath )
        aPathObj.AddPolygon( rPolygon );
    else
    {
        UpdateFillStyle();

        if ( mbComplexClip )
        {
            tools::PolyPolygon aPolyPoly( rPolygon );
            tools::PolyPolygon aDest;
            tools::PolyPolygon(aClipPath.getClipPath()).GetIntersection( aPolyPoly, aDest );
            ImplDrawClippedPolyPolygon( aDest );
        }
        else
        {
            if ( maLineStyle.aLineInfo.GetWidth() || ( maLineStyle.aLineInfo.GetStyle() == LINE_DASH ) )
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

                if (maLatestFillStyle.aType != FillStylePattern)
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
                                                           SvtGraphicFill::gradientLinear,
                                                           Color(),
                                                           Color(),
                                                           0,
                                                           Graphic (maLatestFillStyle.aBmp) );

                    SvMemoryStream  aMemStm;

                    WriteSvtGraphicFill( aMemStm, aFill );

                    mpGDIMetaFile->AddAction( new MetaCommentAction( "XPATHFILL_SEQ_BEGIN", 0,
                                                            static_cast<const sal_uInt8*>(aMemStm.GetData()),
                                                            aMemStm.Seek( STREAM_SEEK_TO_END ) ) );
                    mpGDIMetaFile->AddAction( new MetaCommentAction( "XPATHFILL_SEQ_END" ) );
                }

            }
        }
    }
}

void WinMtfOutput::DrawPolyPolygon( tools::PolyPolygon& rPolyPolygon, bool bRecordPath )
{
    UpdateClipRegion();

    ImplMap( rPolyPolygon );

    if ( bRecordPath )
        aPathObj.AddPolyPolygon( rPolyPolygon );
    else
    {
        UpdateFillStyle();

        if ( mbComplexClip )
        {
            tools::PolyPolygon aDest;
            tools::PolyPolygon(aClipPath.getClipPath()).GetIntersection( rPolyPolygon, aDest );
            ImplDrawClippedPolyPolygon( aDest );
        }
        else
        {
            UpdateLineStyle();
            mpGDIMetaFile->AddAction( new MetaPolyPolygonAction( rPolyPolygon ) );
            if (maLineStyle.aLineInfo.GetWidth() > 0 || maLineStyle.aLineInfo.GetStyle() == LINE_DASH)
            {
                for (sal_uInt16 nPoly = 0; nPoly < rPolyPolygon.Count(); ++nPoly)
                {
                    mpGDIMetaFile->AddAction(new MetaPolyLineAction(rPolyPolygon[nPoly], maLineStyle.aLineInfo));
                }
            }
        }
    }
}

void WinMtfOutput::DrawPolyLine( tools::Polygon& rPolygon, bool bTo, bool bRecordPath )
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
            aPathObj.AddPolyLine( rPolygon );
        else
        {
            UpdateLineStyle();
            mpGDIMetaFile->AddAction( new MetaPolyLineAction( rPolygon, maLineStyle.aLineInfo ) );
        }
    }
}

void WinMtfOutput::DrawPolyBezier( tools::Polygon& rPolygon, bool bTo, bool bRecordPath )
{
    UpdateClipRegion();

    sal_uInt16 nPoints = rPolygon.GetSize();
    if ( ( nPoints >= 4 ) && ( ( ( nPoints - 4 ) % 3 ) == 0 ) )
    {
        ImplMap( rPolygon );
        if ( bTo )
        {
            rPolygon[ 0 ] = maActPos;
            maActPos = rPolygon[ nPoints - 1 ];
        }
        sal_uInt16 i;
        for ( i = 0; ( i + 2 ) < nPoints; )
        {
            rPolygon.SetFlags( i++, POLY_NORMAL );
            rPolygon.SetFlags( i++, POLY_CONTROL );
            rPolygon.SetFlags( i++, POLY_CONTROL );
        }
        if ( bRecordPath )
            aPathObj.AddPolyLine( rPolygon );
        else
        {
            UpdateLineStyle();
            mpGDIMetaFile->AddAction( new MetaPolyLineAction( rPolygon, maLineStyle.aLineInfo ) );
        }
    }
}

void WinMtfOutput::DrawText( Point& rPosition, OUString& rText, long* pDXArry, bool bRecordPath, sal_Int32 nGfxMode )
{
    UpdateClipRegion();
    rPosition = ImplMap( rPosition );
    sal_Int32 nOldGfxMode = GetGfxMode();
    SetGfxMode( GM_COMPATIBLE );

    if (pDXArry)
    {
        sal_Int32 i;
        sal_Int32 nSum = 0;
        sal_Int32 nLen = rText.getLength();

        for (i = 0; i < nLen; i++ )
        {
            if (i > 0)
            {
                // #i121382# Map DXArray using WorldTransform
                const Size aSize(ImplMap(Size(nSum, 0)));
                const basegfx::B2DVector aVector(aSize.Width(), aSize.Height());
                pDXArry[i - 1] = basegfx::fround(aVector.getLength());
            }
            nSum += pDXArry[i];
        }
    }
    if ( mnLatestTextLayoutMode != mnTextLayoutMode )
    {
        mnLatestTextLayoutMode = mnTextLayoutMode;
        mpGDIMetaFile->AddAction( new MetaLayoutModeAction( mnTextLayoutMode ) );
    }
    SetGfxMode( nGfxMode );
    bool bChangeFont = false;
    if ( mnLatestTextAlign != mnTextAlign )
    {
        bChangeFont = true;
        mnLatestTextAlign = mnTextAlign;
        TextAlign eTextAlign;
        if ( ( mnTextAlign & TA_BASELINE) == TA_BASELINE )
            eTextAlign = ALIGN_BASELINE;
        else if( ( mnTextAlign & TA_BOTTOM) == TA_BOTTOM )
            eTextAlign = ALIGN_BOTTOM;
        else
            eTextAlign = ALIGN_TOP;
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

    if ( ( mnTextAlign & TA_BASELINE) == TA_BASELINE )
        aTmp.SetAlign( ALIGN_BASELINE );
    else if( ( mnTextAlign & TA_BOTTOM) == TA_BOTTOM )
        aTmp.SetAlign( ALIGN_BOTTOM );
    else
        aTmp.SetAlign( ALIGN_TOP );

    if ( nGfxMode == GM_ADVANCED )
    {
        // check whether there is a font rotation applied via transformation
        Point aP1( ImplMap( Point() ) );
        Point aP2( ImplMap( Point( 0, 100 ) ) );
        aP2.X() -= aP1.X();
        aP2.Y() -= aP1.Y();
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
        pVDev->SetMapMode( MapMode( MAP_100TH_MM ) );
        pVDev->SetFont( maFont );
        if( pDXArry )
        {
            sal_uInt32 nLen = rText.getLength();
            nTextWidth = pVDev->GetTextWidth( OUString(rText[ nLen - 1 ]) );
            if( nLen > 1 )
                nTextWidth += pDXArry[ nLen - 2 ];
        }
        else
            nTextWidth = pVDev->GetTextWidth( rText );

        if( mnTextAlign & TA_UPDATECP )
            rPosition = maActPos;

        if ( mnTextAlign & TA_RIGHT_CENTER )
        {
            double fLength = ( ( mnTextAlign & TA_RIGHT_CENTER ) == TA_RIGHT ) ? nTextWidth : nTextWidth >> 1;
            rPosition.X() -= (sal_Int32)( fLength * cos( maFont.GetOrientation() * F_PI1800 ) );
            rPosition.Y() -= (sal_Int32)(-( fLength * sin( maFont.GetOrientation() * F_PI1800 ) ) );
        }

        if( mnTextAlign & TA_UPDATECP )
            maActPos.X() = rPosition.X() + nTextWidth;
    }
    if ( bChangeFont || ( maLatestFont != aTmp ) )
    {
        maLatestFont = aTmp;
        mpGDIMetaFile->AddAction( new MetaFontAction( aTmp ) );
        mpGDIMetaFile->AddAction( new MetaTextAlignAction( aTmp.GetAlign() ) );
        mpGDIMetaFile->AddAction( new MetaTextColorAction( aTmp.GetColor() ) );
        mpGDIMetaFile->AddAction( new MetaTextFillColorAction( aTmp.GetFillColor(), !aTmp.IsTransparent() ) );
    }
    if ( bRecordPath )
    {
        // TODO
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
            pVDev->SetMapMode( MAP_100TH_MM );
            pVDev->SetFont( maLatestFont );
            pVDev->GetTextArray( rText, pDX, 0, rText.getLength());
        }
        mpGDIMetaFile->AddAction( new MetaTextArrayAction( rPosition, rText, pDX, 0, rText.getLength() ) );
        if ( !pDXArry )     // this means we have created our own array
            delete[] pDX;   // which must be deleted
    }
    SetGfxMode( nOldGfxMode );
}

void WinMtfOutput::ImplDrawBitmap( const Point& rPos, const Size& rSize, const BitmapEx& rBitmap )
{
    BitmapEx aBmpEx( rBitmap );
    if ( mbComplexClip )
    {
        VclPtrInstance< VirtualDevice > pVDev;
        MapMode aMapMode( MAP_100TH_MM );
        aMapMode.SetOrigin( Point( -rPos.X(), -rPos.Y() ) );
        const Size aOutputSizePixel( pVDev->LogicToPixel( rSize, aMapMode ) );
        const Size aSizePixel( rBitmap.GetSizePixel() );
        if ( aOutputSizePixel.Width() && aOutputSizePixel.Height() )
        {
            aMapMode.SetScaleX( Fraction( aSizePixel.Width(), aOutputSizePixel.Width() ) );
            aMapMode.SetScaleY( Fraction( aSizePixel.Height(), aOutputSizePixel.Height() ) );
        }
        pVDev->SetMapMode( aMapMode );
        pVDev->SetOutputSizePixel( aSizePixel );
        pVDev->SetFillColor( Color( COL_BLACK ) );
        const tools::PolyPolygon aClip( aClipPath.getClipPath() );
        pVDev->DrawPolyPolygon( aClip );
        const Point aEmptyPoint;

        // #i50672# Extract whole VDev content (to match size of rBitmap)
        pVDev->EnableMapMode( false );
        Bitmap aMask( pVDev->GetBitmap( aEmptyPoint, aSizePixel ).CreateMask( Color( COL_WHITE ) ) );

        if ( aBmpEx.IsTransparent() )
        {
            if ( rBitmap.GetTransparentColor() == Color( COL_WHITE ) )
                aMask.CombineSimple( rBitmap.GetMask(), BMP_COMBINE_OR );
            else
                aMask.CombineSimple( rBitmap.GetMask(), BMP_COMBINE_AND );
            aBmpEx = BitmapEx( rBitmap.GetBitmap(), aMask );
        }
        else
            aBmpEx = BitmapEx( rBitmap.GetBitmap(), aMask );
    }
    if ( aBmpEx.IsTransparent() )
        mpGDIMetaFile->AddAction( new MetaBmpExScaleAction( rPos, rSize, aBmpEx ) );
    else
        mpGDIMetaFile->AddAction( new MetaBmpScaleAction( rPos, rSize, aBmpEx.GetBitmap() ) );
}

void WinMtfOutput::ResolveBitmapActions( BSaveStructList_impl& rSaveList )
{
    UpdateClipRegion();

    size_t nObjects     = rSaveList.size();
    size_t nObjectsLeft = nObjects;

    while ( nObjectsLeft )
    {
        size_t          i;
        size_t          nObjectsOfSameSize = 0;
        size_t          nObjectStartIndex = nObjects - nObjectsLeft;

        BSaveStruct*    pSave = rSaveList[ nObjectStartIndex ];
        Rectangle       aRect( pSave->aOutRect );

        for ( i = nObjectStartIndex; i < nObjects; )
        {
            nObjectsOfSameSize++;
            if ( ++i < nObjects )
            {
                pSave = rSaveList[ i ];
                if ( pSave->aOutRect != aRect )
                    break;
            }
        }
        Point   aPos( ImplMap( aRect.TopLeft() ) );
        Size    aSize( ImplMap( aRect.GetSize() ) );

        for ( i = nObjectStartIndex; i < ( nObjectStartIndex + nObjectsOfSameSize ); i++ )
        {
            pSave = rSaveList[ i ];

            sal_uInt32  nWinRop = pSave->nWinRop;
            sal_uInt8   nRasterOperation = (sal_uInt8)( nWinRop >> 16 );

            sal_uInt32  nUsed =  0;
            if ( ( nRasterOperation & 0xf )  != ( nRasterOperation >> 4 ) )
                nUsed |= 1;     // pattern is used
            if ( ( nRasterOperation & 0x33 ) != ( ( nRasterOperation & 0xcc ) >> 2 ) )
                nUsed |= 2;     // source is used
            if ( ( nRasterOperation & 0xaa ) != ( ( nRasterOperation & 0x55 ) << 1 ) )
                nUsed |= 4;     // destination is used

            if ( (nUsed & 1) && (( nUsed & 2 ) == 0) && nWinRop != PATINVERT )
            {   // patterns aren't well supported yet
                sal_uInt32 nOldRop = SetRasterOp( ROP_OVERPAINT );  // in this case nRasterOperation is either 0 or 0xff
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
                        BSaveStruct* pSave2 = rSaveList[ i + 1 ];
                        if ( ( pSave->aBmp.GetPrefSize() == pSave2->aBmp.GetPrefSize() ) &&
                             ( pSave->aBmp.GetPrefMapMode() == pSave2->aBmp.GetPrefMapMode() ) )
                        {
                            // TODO: Strictly speaking, we should
                            // check whether mask is monochrome, and
                            // whether image is black (upper branch)
                            // or white (lower branch). Otherwise, the
                            // effect is not the same as a masked
                            // bitmap.
                            if ( ( nWinRop == SRCPAINT ) && ( pSave2->nWinRop == SRCAND ) )
                            {
                                Bitmap aMask( pSave->aBmp ); aMask.Invert();
                                BitmapEx aBmpEx( pSave2->aBmp, aMask );
                                ImplDrawBitmap( aPos, aSize, aBmpEx );
                                bDrawn = true;
                                i++;
                            }
                            // #i20085# This is just the other way
                            // around as above. Only difference: mask
                            // is inverted
                            else if ( ( nWinRop == SRCAND ) && ( pSave2->nWinRop == SRCPAINT ) )
                            {
                                Bitmap aMask( pSave->aBmp );
                                BitmapEx aBmpEx( pSave2->aBmp, aMask );
                                ImplDrawBitmap( aPos, aSize, aBmpEx );
                                bDrawn = true;
                                i++;
                            }
                            // tdf#90539
                            else if ( ( nWinRop == SRCAND ) && ( pSave2->nWinRop == SRCINVERT ) )
                            {
                                Bitmap aMask( pSave->aBmp );
                                BitmapEx aBmpEx( pSave2->aBmp, aMask );
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
                    sal_uInt32  nOldRop = SetRasterOp( R2_COPYPEN );
                    Bitmap      aBitmap( pSave->aBmp );
                    sal_uInt32  nOperation = ( nRasterOperation & 0xf );
                    switch( nOperation )
                    {
                        case 0x1 :
                        case 0xe :
                        {
                            SetRasterOp( R2_XORPEN );
                            ImplDrawBitmap( aPos, aSize, aBitmap );
                            SetRasterOp( R2_COPYPEN );
                            Bitmap  aMask( aBitmap );
                            aMask.Invert();
                            BitmapEx aBmpEx( aBitmap, aMask );
                            ImplDrawBitmap( aPos, aSize, aBmpEx );
                            if ( nOperation == 0x1 )
                            {
                                SetRasterOp( R2_NOT );
                                DrawRect( aRect, false );
                            }
                        }
                        break;
                        case 0x7 :
                        case 0x8 :
                        {
                            Bitmap  aMask( aBitmap );
                            if ( ( nUsed & 1 ) && ( nRasterOperation & 0xb0 ) == 0xb0 )     // pattern used
                            {
                                aBitmap.Convert( BMP_CONVERSION_24BIT );
                                aBitmap.Erase( maFillStyle.aFillColor );
                            }
                            BitmapEx aBmpEx( aBitmap, aMask );
                            ImplDrawBitmap( aPos, aSize, aBmpEx );
                            if ( nOperation == 0x7 )
                            {
                                SetRasterOp( R2_NOT );
                                DrawRect( aRect, false );
                            }
                        }
                        break;

                        case 0x4 :
                        case 0xb :
                        {
                            SetRasterOp( R2_NOT );
                            DrawRect( aRect, false );
                            SetRasterOp( R2_COPYPEN );
                            Bitmap  aMask( aBitmap );
                            aBitmap.Invert();
                            BitmapEx aBmpEx( aBitmap, aMask );
                            ImplDrawBitmap( aPos, aSize, aBmpEx );
                            SetRasterOp( R2_XORPEN );
                            ImplDrawBitmap( aPos, aSize, aBitmap );
                            if ( nOperation == 0xb )
                            {
                                SetRasterOp( R2_NOT );
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
                            SetRasterOp( R2_XORPEN );
                            ImplDrawBitmap( aPos, aSize, aBitmap );
                            if ( nOperation == 0xd )
                            {
                                SetRasterOp( R2_NOT );
                                DrawRect( aRect, false );
                            }
                        }
                        break;
                        case 0x6 :
                        case 0x9 :
                        {
                            SetRasterOp( R2_XORPEN );
                            ImplDrawBitmap( aPos, aSize, aBitmap );
                            if ( nOperation == 0x9 )
                            {
                                SetRasterOp( R2_NOT );
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
                            ImplDrawBitmap( aPos, aSize, aBitmap );
                        }
                        break;

                        case 0x5 :  // only destination is used
                        {
                            SetRasterOp( R2_NOT );
                            DrawRect( aRect, false );
                        }
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

    for( size_t i = 0, n = rSaveList.size(); i < n; ++i )
        delete rSaveList[ i ];
    rSaveList.clear();
}

void WinMtfOutput::SetDevOrg( const Point& rPoint )
{
    mnDevOrgX = rPoint.X();
    mnDevOrgY = rPoint.Y();
}

void WinMtfOutput::SetDevOrgOffset( sal_Int32 nXAdd, sal_Int32 nYAdd )
{
    mnDevOrgX += nXAdd;
    mnDevOrgY += nYAdd;
}

void WinMtfOutput::SetDevExt( const Size& rSize ,bool regular)
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

void WinMtfOutput::ScaleDevExt( double fX, double fY )
{
    mnDevWidth = FRound( mnDevWidth * fX );
    mnDevHeight = FRound( mnDevHeight * fY );
}

void WinMtfOutput::SetWinOrg( const Point& rPoint , bool bIsEMF)
{
    mnWinOrgX = rPoint.X();
    mnWinOrgY = rPoint.Y();
    if (bIsEMF)
    {
        SetDevByWin();
    }
    mbIsMapWinSet=true;
}

void WinMtfOutput::SetWinOrgOffset( sal_Int32 nXAdd, sal_Int32 nYAdd )
{
    mnWinOrgX += nXAdd;
    mnWinOrgY += nYAdd;
}

void WinMtfOutput::SetDevByWin() //mnWinExt...-stuff has to be assigned before.
{
    if (!mbIsMapDevSet)
    {
        if ( mnMapMode == MM_ISOTROPIC ) //TODO: WHAT ABOUT ANISOTROPIC???
        {
            Size aSize( (mnWinExtX + mnWinOrgX) >> MS_FIXPOINT_BITCOUNT_28_4,
                      -((mnWinExtY - mnWinOrgY) >> MS_FIXPOINT_BITCOUNT_28_4));

            SetDevExt(aSize, false);
        }
    }
}

void WinMtfOutput::SetWinExt(const Size& rSize, bool bIsEMF)
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

void WinMtfOutput::ScaleWinExt( double fX, double fY )
{
    mnWinExtX = FRound( mnWinExtX * fX );
    mnWinExtY = FRound( mnWinExtY * fY );
}

void WinMtfOutput::SetrclBounds( const Rectangle& rRect )
{
    mrclBounds = rRect;
}

void WinMtfOutput::SetrclFrame( const Rectangle& rRect )
{
    mrclFrame = rRect;
}

void WinMtfOutput::SetRefPix( const Size& rSize )
{
    mnPixX = rSize.Width();
    mnPixY = rSize.Height();
}

void WinMtfOutput::SetRefMill( const Size& rSize )
{
    mnMillX = rSize.Width();
    mnMillY = rSize.Height();
}

void WinMtfOutput::SetMapMode( sal_uInt32 nMapMode )
{
    mnMapMode = nMapMode;
    if ( nMapMode == MM_TEXT && !mbIsMapWinSet )
    {
        mnWinExtX = mnDevWidth;
        mnWinExtY = mnDevHeight;
    }
    else if ( mnMapMode == MM_HIMETRIC )
    {
        mnWinExtX = mnMillX * 100;
        mnWinExtY = mnMillY * 100;
    }
}

void WinMtfOutput::SetWorldTransform( const XForm& rXForm )
{
    maXForm.eM11 = rXForm.eM11;
    maXForm.eM12 = rXForm.eM12;
    maXForm.eM21 = rXForm.eM21;
    maXForm.eM22 = rXForm.eM22;
    maXForm.eDx = rXForm.eDx;
    maXForm.eDy = rXForm.eDy;
}

void WinMtfOutput::ModifyWorldTransform( const XForm& rXForm, sal_uInt32 nMode )
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

void WinMtfOutput::Push()                       // !! to be able to access the original ClipRegion it
{                                               // is not allowed to use the MetaPushAction()
    UpdateClipRegion();                         // (the original clip region is on top of the stack) (SJ)
    SaveStructPtr pSave( new SaveStruct );

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

    pSave->aPathObj = aPathObj;
    pSave->aClipPath = aClipPath;

    vSaveStack.push_back( pSave );
}

void WinMtfOutput::Pop()
{
    // Get the latest data from the stack
    if( !vSaveStack.empty() )
    {
        // Backup the current data on the stack
        SaveStructPtr pSave( vSaveStack.back() );

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

        aPathObj = pSave->aPathObj;
        if ( ! ( aClipPath == pSave->aClipPath ) )
        {
            aClipPath = pSave->aClipPath;
            mbClipNeedsUpdate = true;
        }
        if ( meLatestRasterOp != meRasterOp )
            mpGDIMetaFile->AddAction( new MetaRasterOpAction( meRasterOp ) );
        vSaveStack.pop_back();
    }
}

void WinMtfOutput::AddFromGDIMetaFile( GDIMetaFile& rGDIMetaFile )
{
   rGDIMetaFile.Play( *mpGDIMetaFile );
}

void WinMtfOutput::PassEMFPlusHeaderInfo()
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

void WinMtfOutput::PassEMFPlus( void* pBuffer, sal_uInt32 nLength )
{
    EMFP_DEBUG(printf ("\t\t\tadd EMF_PLUS comment length %04x\n",(unsigned int) nLength));
    mpGDIMetaFile->AddAction( new MetaCommentAction( "EMF_PLUS", 0, static_cast<const sal_uInt8*>(pBuffer), nLength ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
