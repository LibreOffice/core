/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "winmtf.hxx"
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <vcl/metaact.hxx>
#include <vcl/graphictools.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/metric.hxx>
#include <vcl/svapp.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/tencinfo.h>

// ------------------------------------------------------------------------

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

void WinMtfClipPath::setClipPath( const PolyPolygon& rPolyPolygon, sal_Int32 nClippingMode )
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

basegfx::B2DPolyPolygon WinMtfClipPath::getClipPath() const
{
    return maClip.getClipPoly();
}

// ------------------------------------------------------------------------

void WinMtfPathObj::AddPoint( const Point& rPoint )
{
    if ( bClosed )
        Insert( Polygon(), POLYPOLY_APPEND );
    Polygon& rPoly = ((PolyPolygon&)*this)[ Count() - 1 ];
    rPoly.Insert( rPoly.GetSize(), rPoint, POLY_NORMAL );
    bClosed = sal_False;
}

void WinMtfPathObj::AddPolyLine( const Polygon& rPolyLine )
{
    if ( bClosed )
        Insert( Polygon(), POLYPOLY_APPEND );
    Polygon& rPoly = ((PolyPolygon&)*this)[ Count() - 1 ];
    rPoly.Insert( rPoly.GetSize(), rPolyLine );
    bClosed = sal_False;
}

void WinMtfPathObj::AddPolygon( const Polygon& rPoly )
{
    Insert( rPoly, POLYPOLY_APPEND );
    bClosed = sal_True;
}

void WinMtfPathObj::AddPolyPolygon( const PolyPolygon& rPolyPoly )
{
    sal_uInt16 i, nCount = rPolyPoly.Count();
    for ( i = 0; i < nCount; i++ )
        Insert( rPolyPoly[ i ], POLYPOLY_APPEND );
    bClosed = sal_True;
}

void WinMtfPathObj::ClosePath()
{
    if ( Count() )
    {
        Polygon& rPoly = ((PolyPolygon&)*this)[ Count() - 1 ];
        if ( rPoly.GetSize() > 2 )
        {
            Point aFirst( rPoly[ 0 ] );
            if ( aFirst != rPoly[ rPoly.GetSize() - 1 ] )
                rPoly.Insert( rPoly.GetSize(), aFirst, POLY_NORMAL );
        }
    }
    bClosed = sal_True;
}

// ------------------------------------------------------------------------

WinMtfFontStyle::WinMtfFontStyle( LOGFONTW& rFont )
{
    CharSet eCharSet;
    if ( ( rFont.lfCharSet == OEM_CHARSET ) || ( rFont.lfCharSet == DEFAULT_CHARSET ) )
        eCharSet = RTL_TEXTENCODING_MS_1252;
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

    Size  aFontSize( Size( rFont.lfWidth, rFont.lfHeight ) );
    if ( rFont.lfHeight > 0 )
    {
        // converting the cell height into a font height
        VirtualDevice aVDev;
        aFont.SetSize( aFontSize );
        aVDev.SetFont( aFont );
        FontMetric aMetric( aVDev.GetFontMetric() );
        long nHeight = aMetric.GetAscent() + aMetric.GetDescent();
        if ( nHeight )
        {
            double fHeight = ((double)aFontSize.Height() * rFont.lfHeight ) / nHeight;
            aFontSize.Height() = (sal_Int32)( fHeight + 0.5 );
        }
    }
    else if ( aFontSize.Height() < 0 )
        aFontSize.Height() *= -1;

    if ( !rFont.lfWidth )
    {
        VirtualDevice aVDev;
        aFont.SetSize( aFontSize );
        aVDev.SetFont( aFont );
        FontMetric aMetric( aVDev.GetFontMetric() );
        aFontSize.Width() = aMetric.GetWidth();
    }

    aFont.SetSize( aFontSize );
};

// ------------------------------------------------------------------------

#ifdef WIN_MTF_ASSERT
void WinMtfAssertHandler( const sal_Char* pAction, sal_uInt32 nFlags )
{
    static sal_Bool     bOnlyOnce;
    static sal_Int32    nAssertCount;

    if ( nFlags & WIN_MTF_ASSERT_INIT )
        nAssertCount = 0;
    if ( nFlags & WIN_MTF_ASSERT_ONCE )
       bOnlyOnce = sal_True;
    if ( nFlags & WIN_MTF_ASSERT_MIFE )
    {
        if ( ( nAssertCount == 0 ) || ( bOnlyOnce == sal_False ) )
        {
            rtl::OStringBuffer aText(RTL_CONSTASCII_STRINGPARAM(
                "WMF/EMF Import: "));
            if (pAction)
                aText.append(pAction);
            aText.append(RTL_CONSTASCII_STRINGPARAM(
                " needs to be implemented"));
            DBG_ASSERT( 0, aText.getStr() );
        }
        nAssertCount++;
    }
}
#endif

// ------------------------------------------------------------------------

WinMtf::WinMtf( WinMtfOutput* pWinMtfOutput, SvStream& rStreamWMF, FilterConfigItem* pConfigItem ) :
    pOut                ( pWinMtfOutput ),
    pWMF                ( &rStreamWMF ),
    pFilterConfigItem   ( pConfigItem )
{
#ifdef WIN_MTF_ASSERT
    // we want to assert not implemented features, but we do this
    // only once, so that nobody is handicaped by getting too much assertions
    // I hope this will bring more testdocuments, without support of these
    // testdocuments the implementation of missing features won't be possible. (SJ)
    WinMtfAssertHandler( NULL, WIN_MTF_ASSERT_INIT | WIN_MTF_ASSERT_ONCE );
#endif

    SvLockBytes *pLB = pWMF->GetLockBytes();
    if ( pLB )
        pLB->SetSynchronMode( sal_True );

    nStartPos = pWMF->Tell();

    pOut->SetDevOrg( Point() );
    if ( pFilterConfigItem )
    {
        xStatusIndicator = pFilterConfigItem->GetStatusIndicator();
        if ( xStatusIndicator.is() )
        {
            rtl::OUString aMsg;
            xStatusIndicator->start( aMsg, 100 );
        }
    }
}

// ------------------------------------------------------------------------

WinMtf::~WinMtf()
{
    delete pOut;

    if ( xStatusIndicator.is() )
        xStatusIndicator->end();
}

// ------------------------------------------------------------------------

void WinMtf::Callback( sal_uInt16 nPercent )
{
    if ( xStatusIndicator.is() )
        xStatusIndicator->setValue( nPercent );
}

// ------------------------------------------------------------------------

Color WinMtf::ReadColor()
{
    sal_uInt32 nColor;
    *pWMF >> nColor;
    return Color( (sal_uInt8)nColor, (sal_uInt8)( nColor >> 8 ), (sal_uInt8)( nColor >> 16 ) );
};

//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------

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
                case MM_TEXT:
                    fX2 -= mnWinOrgX;
                    fY2 -= mnWinOrgY;
                    if( mnDevWidth != 1 || mnDevHeight != 1 ) {
                        fX2 *= 2540.0/mnUnitsPerInch;
                        fY2 *= 2540.0/mnUnitsPerInch;
                    }
                    fX2 += mnDevOrgX;
                    fY2 += mnDevOrgY;
                    fX2 *= (double)mnMillX * 100.0 / (double)mnPixX;
                    fY2 *= (double)mnMillY * 100.0 / (double)mnPixY;

                    break;
                case MM_LOENGLISH :
                {
                    fX2 -= mnWinOrgX;
                    fY2  = mnWinOrgY-fY2;
                    fX2 *= 25.40;
                    fY2 *= 25.40;
                    fX2 += mnDevOrgX;
                    fY2 += mnDevOrgY;
                }
                break;
                case MM_HIENGLISH :
                {
                    fX2 -= mnWinOrgX;
                    fY2  = mnWinOrgY-fY2;
                    fX2 *= 2.540;
                    fY2 *= 2.540;
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
                case MM_HIMETRIC :
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

// ------------------------------------------------------------------------

Size WinMtfOutput::ImplMap( const Size& rSz )
{
    if ( mnWinExtX && mnWinExtY )
    {
        double fWidth = rSz.Width() * maXForm.eM11;
        double fHeight = rSz.Height() * maXForm.eM22;

        if ( mnGfxMode == GM_COMPATIBLE )
        {
            switch( mnMapMode )
            {
                case MM_TEXT:
                if( mnDevWidth != 1 && mnDevHeight != 1 ) {
                    fWidth *= 2540.0/mnUnitsPerInch;
                    fHeight*= 2540.0/mnUnitsPerInch;
                } else {
                    fWidth *= (double)mnMillX * 100 / (double)mnPixX;
                    fHeight *= (double)mnMillY * 100 / (double)mnPixY;
                }
                break;
                case MM_LOENGLISH :
                {
                    fWidth *= 25.40;
                    fHeight*=-25.40;
                }
                break;
                case MM_HIENGLISH :
                {
                    fWidth *= 2.540;
                    fHeight*=-2.540;
                }
                break;
                case MM_LOMETRIC :
                {
                    fWidth *= 10;
                    fHeight*=-10;
                }
                break;
                case MM_HIMETRIC :
                {
                    fHeight *= -1;
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

//-----------------------------------------------------------------------------------

Rectangle WinMtfOutput::ImplMap( const Rectangle& rRect )
{
    return Rectangle( ImplMap( rRect.TopLeft() ), ImplMap( rRect.GetSize() ) );
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::ImplMap( Font& rFont )
{
    // !!! HACK: Wir setzen die Breite jetzt immer auf Null,
    // da OS die Breite unterschiedlich interpretieren;
    // muss spaeter in SV portabel gemacht werden ( KA 08.02.96 )
    Size  aFontSize = ImplMap ( rFont.GetSize() );

    if( aFontSize.Height() < 0 )
        aFontSize.Height() *= -1;

    rFont.SetSize( aFontSize );

    if( ( mnWinExtX * mnWinExtY ) < 0 )
        rFont.SetOrientation( 3600 - rFont.GetOrientation() );
}

//-----------------------------------------------------------------------------------

Polygon& WinMtfOutput::ImplMap( Polygon& rPolygon )
{
    sal_uInt16 nPoints = rPolygon.GetSize();
    for ( sal_uInt16 i = 0; i < nPoints; i++ )
    {
        rPolygon[ i ] = ImplMap( rPolygon[ i ] );
    }
    return rPolygon;
}

//-----------------------------------------------------------------------------------

PolyPolygon& WinMtfOutput::ImplMap( PolyPolygon& rPolyPolygon )
{
    sal_uInt16 nPolys = rPolyPolygon.Count();
    for ( sal_uInt16 i = 0; i < nPolys; ImplMap( rPolyPolygon[ i++ ] ) ) ;
    return rPolyPolygon;
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::SelectObject( sal_Int32 nIndex )
{
    GDIObj* pGDIObj = NULL;

    if ( nIndex & ENHMETA_STOCK_OBJECT )
        pGDIObj = new GDIObj();
    else
    {
        nIndex &= 0xffff;       // zur Sicherheit: mehr als 65535 nicht zulassen

        if ( (sal_uInt32)nIndex < vGDIObj.size() )
            pGDIObj = vGDIObj[ nIndex ];
    }

    if( pGDIObj == NULL )
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
                pGDIObj->Set( GDI_BRUSH, new WinMtfFillStyle( Color( COL_TRANSPARENT ), sal_True ) );
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
                pGDIObj->Set( GDI_PEN, new WinMtfLineStyle( Color( COL_TRANSPARENT ), sal_True ) );
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
                maLineStyle = (WinMtfLineStyle*)pGDIObj->pStyle;
            break;
            case GDI_BRUSH :
            {
                maFillStyle = (WinMtfFillStyle*)pGDIObj->pStyle;
                mbFillStyleSelected = sal_True;
            }
            break;
            case GDI_FONT :
                maFont = ((WinMtfFontStyle*)pGDIObj->pStyle)->aFont;
            break;
            default:
            break;  //  -Wall many options not handled.
        }
    }
    if ( nIndex & ENHMETA_STOCK_OBJECT )
        delete pGDIObj;
}

//-----------------------------------------------------------------------------------

const Font& WinMtfOutput::GetFont() const
{
    return maFont;
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::SetTextLayoutMode( const sal_uInt32 nTextLayoutMode )
{
    mnTextLayoutMode = nTextLayoutMode;
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::SetBkMode( sal_uInt32 nMode )
{
    mnBkMode = nMode;
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::SetBkColor( const Color& rColor )
{
    maBkColor = rColor;
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::SetTextColor( const Color& rColor )
{
    maTextColor = rColor;
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::SetTextAlign( sal_uInt32 nAlign )
{
    mnTextAlign = nAlign;
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::ImplResizeObjectArry( sal_uInt32 nNewEntrys )
{
    sal_uInt32 i = vGDIObj.size();
    vGDIObj.resize( nNewEntrys );
    for ( ; i < nNewEntrys ; i++ )
        vGDIObj[ i ] = NULL;
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::ImplDrawClippedPolyPolygon( const PolyPolygon& rPolyPoly )
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
                Polygon aPoly( rPolyPoly[ 0 ] );
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


//-----------------------------------------------------------------------------------

void WinMtfOutput::CreateObject( GDIObjectType eType, void* pStyle )
{
    if ( pStyle )
    {
        if ( eType == GDI_FONT )
        {
            ImplMap( ((WinMtfFontStyle*)pStyle)->aFont );
            if (!((WinMtfFontStyle*)pStyle)->aFont.GetHeight() )
                ((WinMtfFontStyle*)pStyle)->aFont.SetHeight( 423 );     // defaulting to 12pt
        }
        else if ( eType == GDI_PEN )
        {
            Size aSize( ((WinMtfLineStyle*)pStyle)->aLineInfo.GetWidth(), 0 );
            ((WinMtfLineStyle*)pStyle)->aLineInfo.SetWidth( ImplMap( aSize ).Width() );
            if ( ((WinMtfLineStyle*)pStyle)->aLineInfo.GetStyle() == LINE_DASH )
            {
                aSize.Width() += 1;
                long nDotLen = ImplMap( aSize ).Width();
                ((WinMtfLineStyle*)pStyle)->aLineInfo.SetDistance( nDotLen );
                ((WinMtfLineStyle*)pStyle)->aLineInfo.SetDotLen( nDotLen );
                ((WinMtfLineStyle*)pStyle)->aLineInfo.SetDashLen( nDotLen * 4 );
            }
        }
    }
    sal_uInt32 nIndex;
    for ( nIndex = 0; nIndex < vGDIObj.size(); nIndex++ )
    {
        if ( vGDIObj[ nIndex ] == NULL )
            break;
    }
    if ( nIndex == vGDIObj.size() )
        ImplResizeObjectArry( vGDIObj.size() + 16 );

    vGDIObj[ nIndex ] = new GDIObj( eType, pStyle );
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::CreateObject( sal_Int32 nIndex, GDIObjectType eType, void* pStyle )
{
    if ( ( nIndex & ENHMETA_STOCK_OBJECT ) == 0 )
    {
        nIndex &= 0xffff;       // zur Sicherheit: mehr als 65535 nicht zulassen
        if ( pStyle )
        {
            if ( eType == GDI_FONT )
                ImplMap( ((WinMtfFontStyle*)pStyle)->aFont );
            else if ( eType == GDI_PEN )
            {
                Size aSize( ((WinMtfLineStyle*)pStyle)->aLineInfo.GetWidth(), 0 );
                ((WinMtfLineStyle*)pStyle)->aLineInfo.SetWidth( ImplMap( aSize ).Width() );
                if ( ((WinMtfLineStyle*)pStyle)->aLineInfo.GetStyle() == LINE_DASH )
                {
                    aSize.Width() += 1;
                    long nDotLen = ImplMap( aSize ).Width();
                    ((WinMtfLineStyle*)pStyle)->aLineInfo.SetDistance( nDotLen );
                    ((WinMtfLineStyle*)pStyle)->aLineInfo.SetDotLen( nDotLen );
                    ((WinMtfLineStyle*)pStyle)->aLineInfo.SetDashLen( nDotLen * 4 );
                }
            }
        }
        if ( (sal_uInt32)nIndex >= vGDIObj.size() )
            ImplResizeObjectArry( nIndex + 16 );

        if ( vGDIObj[ nIndex ] != NULL )
            delete vGDIObj[ nIndex ];

        vGDIObj[ nIndex ] = new GDIObj( eType, pStyle );
    }
    else
    {
        switch ( eType )
        {
            case GDI_PEN :
                delete (WinMtfLineStyle*)pStyle;
            break;
            case GDI_BRUSH :
                delete (WinMtfFillStyle*)pStyle;
            break;
            case GDI_FONT :
                delete (WinMtfFontStyle*)pStyle;
            break;

            default:
                OSL_FAIL( "unsupported style not deleted" );
                break;
        }
    }
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::DeleteObject( sal_Int32 nIndex )
{
    if ( ( nIndex & ENHMETA_STOCK_OBJECT ) == 0 )
    {
        if ( (sal_uInt32)nIndex < vGDIObj.size() )
        {
            delete vGDIObj[ nIndex ];
            vGDIObj[ nIndex ] = NULL;
        }
    }
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::IntersectClipRect( const Rectangle& rRect )
{
    mbClipNeedsUpdate=true;
    aClipPath.intersectClipRect( ImplMap( rRect ) );
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::ExcludeClipRect( const Rectangle& rRect )
{
    mbClipNeedsUpdate=true;
    aClipPath.excludeClipRect( ImplMap( rRect ) );
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::MoveClipRegion( const Size& rSize )
{
    mbClipNeedsUpdate=true;
    aClipPath.moveClipRegion( ImplMap( rSize ) );
}

void WinMtfOutput::SetClipPath( const PolyPolygon& rPolyPolygon, sal_Int32 nClippingMode, sal_Bool bIsMapped )
{
    mbClipNeedsUpdate=true;
    if ( bIsMapped )
        aClipPath.setClipPath( rPolyPolygon, nClippingMode );
    else
    {
        PolyPolygon aPP( rPolyPolygon );
        aClipPath.setClipPath( ImplMap( aPP ), nClippingMode );
    }
}

//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------

WinMtfOutput::WinMtfOutput( GDIMetaFile& rGDIMetaFile ) :
    mnLatestTextAlign   ( 0 ),
    mnTextAlign         ( TA_LEFT | TA_TOP | TA_NOUPDATECP ),
    maLatestBkColor     ( 0x12345678 ),
    maBkColor           ( COL_WHITE ),
    mnLatestTextLayoutMode( TEXT_LAYOUT_DEFAULT ),
    mnTextLayoutMode    ( TEXT_LAYOUT_DEFAULT ),
    mnLatestBkMode      ( 0 ),
    mnBkMode            ( OPAQUE ),
    meLatestRasterOp    ( ROP_INVERT ),
    meRasterOp          ( ROP_OVERPAINT ),
    maActPos            ( Point() ),
    mbNopMode           ( sal_False ),
    mbFillStyleSelected ( sal_False ),
    mbClipNeedsUpdate   ( true ),
    mbComplexClip       ( false ),
    mnGfxMode           ( GM_COMPATIBLE ),
    mnMapMode           ( MM_TEXT ),
    mnUnitsPerInch ( 96 ),
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
    mpGDIMetaFile->AddAction( new MetaPushAction( PUSH_CLIPREGION ) );      // The original clipregion has to be on top
                                                                            // of the stack so it can always be restored
                                                                            // this is necessary to be able to support
                                                                            // SetClipRgn( NULL ) and similar ClipRgn actions (SJ)

    maFont.SetName( String( RTL_CONSTASCII_USTRINGPARAM( "Arial" )) );  // sj: #i57205#, we do have some scaling problems if using
    maFont.SetCharSet( RTL_TEXTENCODING_MS_1252 );                      // the default font then most times a x11 font is used, we
    maFont.SetHeight( 423 );                                                // will prevent this defining a font

    maLatestLineStyle.aLineColor = Color( 0x12, 0x34, 0x56 );
    maLatestFillStyle.aFillColor = Color( 0x12, 0x34, 0x56 );

    mnRop = R2_BLACK + 1;
    SetRasterOp( R2_BLACK );
};

//-----------------------------------------------------------------------------------

WinMtfOutput::~WinMtfOutput()
{
    mpGDIMetaFile->AddAction( new MetaPopAction() );
    mpGDIMetaFile->SetPrefMapMode( MAP_100TH_MM );
    if ( mrclFrame.IsEmpty() )
        mpGDIMetaFile->SetPrefSize( Size( mnDevWidth, mnDevHeight ) );
    else
        mpGDIMetaFile->SetPrefSize( mrclFrame.GetSize() );

    for ( sal_uInt32 i = 0; i < vGDIObj.size(); i++ )
        delete vGDIObj[ i ];
};

//-----------------------------------------------------------------------------------

void WinMtfOutput::UpdateClipRegion()
{
    if ( mbClipNeedsUpdate )
    {
        mbClipNeedsUpdate = false;
        mbComplexClip = false;

        mpGDIMetaFile->AddAction( new MetaPopAction() );                    // taking the orignal clipregion
        mpGDIMetaFile->AddAction( new MetaPushAction( PUSH_CLIPREGION ) );  //

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

//-----------------------------------------------------------------------------------

void WinMtfOutput::ImplSetNonPersistentLineColorTransparenz()
{
    Color aColor(  COL_TRANSPARENT);
    WinMtfLineStyle aTransparentLine( aColor, sal_True );
    if ( ! ( maLatestLineStyle == aTransparentLine ) )
    {
        maLatestLineStyle = aTransparentLine;
        mpGDIMetaFile->AddAction( new MetaLineColorAction( aTransparentLine.aLineColor, !aTransparentLine.bTransparent ) );
    }
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::UpdateLineStyle()
{
    if (!( maLatestLineStyle == maLineStyle ) )
    {
        maLatestLineStyle = maLineStyle;
        mpGDIMetaFile->AddAction( new MetaLineColorAction( maLineStyle.aLineColor, !maLineStyle.bTransparent ) );
    }
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::UpdateFillStyle()
{
    if ( !mbFillStyleSelected )     // SJ: #i57205# taking care of bkcolor if no brush is selected
        maFillStyle = WinMtfFillStyle( maBkColor, mnBkMode == TRANSPARENT );
    if (!( maLatestFillStyle == maFillStyle ) )
    {
        maLatestFillStyle = maFillStyle;
        if (maFillStyle.aType == FillStyleSolid)
            mpGDIMetaFile->AddAction( new MetaFillColorAction( maFillStyle.aFillColor, !maFillStyle.bTransparent ) );
    }
}

//-----------------------------------------------------------------------------------

sal_uInt32 WinMtfOutput::SetRasterOp( sal_uInt32 nRasterOp )
{
    sal_uInt32 nRetROP = mnRop;
    if ( nRasterOp != mnRop )
    {
        mnRop = nRasterOp;
        static WinMtfFillStyle aNopFillStyle;
        static WinMtfLineStyle aNopLineStyle;

        if ( mbNopMode && ( nRasterOp != R2_NOP ) )
        {   // beim uebergang von R2_NOP auf anderen Modus
            // gesetzten Pen und Brush aktivieren
            maFillStyle = aNopFillStyle;
            maLineStyle = aNopLineStyle;
            mbNopMode = sal_False;
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
                if( mbNopMode == sal_False )
                {
                    aNopFillStyle = maFillStyle;
                    aNopLineStyle = maLineStyle;
                    maFillStyle = WinMtfFillStyle( Color( COL_TRANSPARENT ), sal_True );
                    maLineStyle = WinMtfLineStyle( Color( COL_TRANSPARENT ), sal_True );
                    mbNopMode = sal_True;
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

//-----------------------------------------------------------------------------------

void WinMtfOutput::StrokeAndFillPath( sal_Bool bStroke, sal_Bool bFill )
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
                mpGDIMetaFile->AddAction( new MetaPushAction( PUSH_LINECOLOR ) );
                mpGDIMetaFile->AddAction( new MetaLineColorAction( Color(), sal_False ) );
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

//-----------------------------------------------------------------------------------

void WinMtfOutput::DrawPixel( const Point& rSource, const Color& rColor )
{
    mpGDIMetaFile->AddAction( new MetaPixelAction( ImplMap( rSource), rColor ) );
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::MoveTo( const Point& rPoint, sal_Bool bRecordPath )
{
    Point aDest( ImplMap( rPoint ) );
    if ( bRecordPath )
        aPathObj.AddPoint( aDest );
    maActPos = aDest;
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::LineTo( const Point& rPoint, sal_Bool bRecordPath )
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

//-----------------------------------------------------------------------------------

void WinMtfOutput::DrawRect( const Rectangle& rRect, sal_Bool bEdge )
{
    UpdateClipRegion();
    UpdateFillStyle();

    if ( mbComplexClip )
    {
        Polygon aPoly( ImplMap( rRect ) );
        PolyPolygon aPolyPolyRect( aPoly );
        PolyPolygon aDest;
        PolyPolygon(aClipPath.getClipPath()).GetIntersection( aPolyPolyRect, aDest );
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
                mpGDIMetaFile->AddAction( new MetaPolyLineAction( Polygon( ImplMap( rRect ) ),maLineStyle.aLineInfo ) );
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

//-----------------------------------------------------------------------------------

void WinMtfOutput::DrawRoundRect( const Rectangle& rRect, const Size& rSize )
{
    UpdateClipRegion();
    UpdateLineStyle();
    UpdateFillStyle();
    mpGDIMetaFile->AddAction( new MetaRoundRectAction( ImplMap( rRect ), labs( ImplMap( rSize ).Width() ), labs( ImplMap( rSize ).Height() ) ) );
}

//-----------------------------------------------------------------------------------

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
        mpGDIMetaFile->AddAction( new MetaPolyLineAction( Polygon( aCenter, aRad.Width(), aRad.Height() ), maLineStyle.aLineInfo ) );
    }
    else
    {
        UpdateLineStyle();
        mpGDIMetaFile->AddAction( new MetaEllipseAction( ImplMap( rRect ) ) );
    }
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::DrawArc( const Rectangle& rRect, const Point& rStart, const Point& rEnd, sal_Bool bTo )
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

            mpGDIMetaFile->AddAction( new MetaPolyLineAction( Polygon( aCenter, aRad.Width(), aRad.Height() ), maLineStyle.aLineInfo ) );
        }
        else
            mpGDIMetaFile->AddAction( new MetaPolyLineAction( Polygon( aRect, aStart, aEnd, POLY_ARC ), maLineStyle.aLineInfo ) );
    }
    else
        mpGDIMetaFile->AddAction( new MetaArcAction( aRect, aStart, aEnd ) );

    if ( bTo )
        maActPos = aEnd;
}

//-----------------------------------------------------------------------------------

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
        mpGDIMetaFile->AddAction( new MetaPolyLineAction( Polygon( aRect, aStart, aEnd, POLY_PIE ), maLineStyle.aLineInfo ) );
    }
    else
    {
        UpdateLineStyle();
        mpGDIMetaFile->AddAction( new MetaPieAction( aRect, aStart, aEnd ) );
    }
}

//-----------------------------------------------------------------------------------

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
        mpGDIMetaFile->AddAction( new MetaPolyLineAction( Polygon( aRect, aStart, aEnd, POLY_CHORD ), maLineStyle.aLineInfo ) );
    }
    else
    {
        UpdateLineStyle();
        mpGDIMetaFile->AddAction( new MetaChordAction( aRect, aStart, aEnd ) );
    }
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::DrawPolygon( Polygon& rPolygon, sal_Bool bRecordPath )
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
            PolyPolygon aPolyPoly( rPolygon );
            PolyPolygon aDest;
            PolyPolygon(aClipPath.getClipPath()).GetIntersection( aPolyPoly, aDest );
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
                    SvtGraphicFill aFill = SvtGraphicFill( PolyPolygon( rPolygon ),
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

                    aMemStm << aFill;

                    mpGDIMetaFile->AddAction( new MetaCommentAction( "XPATHFILL_SEQ_BEGIN", 0,
                                                            static_cast<const sal_uInt8*>(aMemStm.GetData()),
                                                            aMemStm.Seek( STREAM_SEEK_TO_END ) ) );
                    mpGDIMetaFile->AddAction( new MetaCommentAction( "XPATHFILL_SEQ_END" ) );
                }

            }
        }
    }
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::DrawPolyPolygon( PolyPolygon& rPolyPolygon, sal_Bool bRecordPath )
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
            PolyPolygon aDest;
            PolyPolygon(aClipPath.getClipPath()).GetIntersection( rPolyPolygon, aDest );
            ImplDrawClippedPolyPolygon( aDest );
        }
        else
        {
            UpdateLineStyle();
            mpGDIMetaFile->AddAction( new MetaPolyPolygonAction( rPolyPolygon ) );
        }
    }
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::DrawPolyLine( Polygon& rPolygon, sal_Bool bTo, sal_Bool bRecordPath )
{
    UpdateClipRegion();

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

//-----------------------------------------------------------------------------------

void WinMtfOutput::DrawPolyBezier( Polygon& rPolygon, sal_Bool bTo, sal_Bool bRecordPath )
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

//-----------------------------------------------------------------------------------

void WinMtfOutput::DrawText( Point& rPosition, String& rText, sal_Int32* pDXArry, sal_Bool bRecordPath, sal_Int32 nGfxMode )
{
    UpdateClipRegion();

    VirtualDevice* pVDev = NULL;

    rPosition = ImplMap( rPosition );

    sal_Int32 nOldGfxMode = GetGfxMode();
    SetGfxMode( GM_COMPATIBLE );
    if ( pDXArry )
    {
        sal_Int32 i, nSum, nLen = rText.Len();

        for( i = 0, nSum = 0; i < nLen; i++ )
        {
            if ( i ) {
                pDXArry[ i - 1 ] = ImplMap( Size( nSum, 0 ) ).Width();
            }
            nSum += pDXArry[ i ];
        }
    }
    if ( mnLatestTextLayoutMode != mnTextLayoutMode )
    {
        mnLatestTextLayoutMode = mnTextLayoutMode;
        mpGDIMetaFile->AddAction( new MetaLayoutModeAction( mnTextLayoutMode ) );
    }
    SetGfxMode( nGfxMode );
    sal_Bool bChangeFont = sal_False;
    if ( mnLatestTextAlign != mnTextAlign )
    {
        bChangeFont = sal_True;
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
        bChangeFont = sal_True;
        maLatestTextColor = maTextColor;
        mpGDIMetaFile->AddAction( new MetaTextColorAction( maTextColor ) );
    }
    sal_Bool bChangeFillColor = sal_False;
    if ( maLatestBkColor != maBkColor )
    {
        bChangeFillColor = sal_True;
        maLatestBkColor = maBkColor;
    }
    if ( mnLatestBkMode != mnBkMode )
    {
        bChangeFillColor = sal_True;
        mnLatestBkMode = mnBkMode;
    }
    if ( bChangeFillColor )
    {
        bChangeFont = sal_True;
        mpGDIMetaFile->AddAction( new MetaTextFillColorAction( maFont.GetFillColor(), !maFont.IsTransparent() ) );
    }
    Font aTmp( maFont );
    aTmp.SetColor( maTextColor );
    aTmp.SetFillColor( maBkColor );

    if( mnBkMode == TRANSPARENT )
        aTmp.SetTransparent( sal_True );
    else
        aTmp.SetTransparent( sal_False );

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
        if ( !pVDev )
            pVDev = new VirtualDevice;
        sal_Int32 nTextWidth;
        pVDev->SetMapMode( MapMode( MAP_100TH_MM ) );
        pVDev->SetFont( maFont );
        if( pDXArry )
        {
            sal_uInt32 nLen = rText.Len();
            nTextWidth = pVDev->GetTextWidth( rtl::OUString(rText.GetChar( (sal_uInt16)( nLen - 1 ) )) );
            if( nLen > 1 )
                nTextWidth += pDXArry[ nLen - 2 ];
        }
        else
            nTextWidth = pVDev->GetTextWidth( rText );

        if( mnTextAlign & TA_UPDATECP )
            rPosition = maActPos;

        if ( mnTextAlign & TA_RIGHT_CENTER )
        {
            double fLenght = ( ( mnTextAlign & TA_RIGHT_CENTER ) == TA_RIGHT ) ? nTextWidth : nTextWidth >> 1;
            rPosition.X() -= (sal_Int32)( fLenght * cos( maFont.GetOrientation() * F_PI1800 ) );
            rPosition.Y() -= (sal_Int32)(-( fLenght * sin( maFont.GetOrientation() * F_PI1800 ) ) );
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
        // ToDo
    }
    else
    {
        /* because text without dx array is badly scaled, we
           will create such an array if necessary */
        sal_Int32* pDX = pDXArry;
        if ( !pDXArry )
        {
            SolarMutexGuard aGuard;

            pDX = new sal_Int32[ rText.Len() ];
            if ( !pVDev )
                pVDev = new VirtualDevice;
            pVDev->SetMapMode( MAP_100TH_MM );
            pVDev->SetFont( maLatestFont );
            pVDev->GetTextArray( rText, pDX, 0, STRING_LEN );
        }
        mpGDIMetaFile->AddAction( new MetaTextArrayAction( rPosition, rText, pDX, 0, STRING_LEN ) );
        if ( !pDXArry )     // this means we have created our own array
            delete[] pDX;   // which must be deleted
    }
    SetGfxMode( nOldGfxMode );
    delete pVDev;
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::ImplDrawBitmap( const Point& rPos, const Size& rSize, const BitmapEx rBitmap )
{
    BitmapEx aBmpEx( rBitmap );
    if ( mbComplexClip )
    {
        VirtualDevice aVDev;
        MapMode aMapMode( MAP_100TH_MM );
        aMapMode.SetOrigin( Point( -rPos.X(), -rPos.Y() ) );
        const Size aOutputSizePixel( aVDev.LogicToPixel( rSize, aMapMode ) );
        const Size aSizePixel( rBitmap.GetSizePixel() );
        if ( aOutputSizePixel.Width() && aOutputSizePixel.Height() )
        {
            aMapMode.SetScaleX( Fraction( aSizePixel.Width(), aOutputSizePixel.Width() ) );
            aMapMode.SetScaleY( Fraction( aSizePixel.Height(), aOutputSizePixel.Height() ) );
        }
        aVDev.SetMapMode( aMapMode );
        aVDev.SetOutputSizePixel( aSizePixel );
        aVDev.SetFillColor( Color( COL_BLACK ) );
        const PolyPolygon aClip( aClipPath.getClipPath() );
        aVDev.DrawPolyPolygon( aClip );
        const Point aEmptyPoint;

        // #i50672# Extract whole VDev content (to match size of rBitmap)
        aVDev.EnableMapMode( sal_False );
        Bitmap aMask( aVDev.GetBitmap( aEmptyPoint, aSizePixel ).CreateMask( Color( COL_WHITE ) ) );

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

//-----------------------------------------------------------------------------------

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
                DrawRect( aRect, sal_False );
                SetRasterOp( nOldRop );
            }
            else
            {
                sal_Bool bDrawn = sal_False;

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
                                bDrawn = sal_True;
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
                                bDrawn = sal_True;
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
                                DrawRect( aRect, sal_False );
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
                                DrawRect( aRect, sal_False );
                            }
                        }
                        break;

                        case 0x4 :
                        case 0xb :
                        {
                            SetRasterOp( R2_NOT );
                            DrawRect( aRect, sal_False );
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
                                DrawRect( aRect, sal_False );
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
                                DrawRect( aRect, sal_False );
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
                                DrawRect( aRect, sal_False );
                            }
                        }
                        break;

                        case 0x0 :  // WHITENESS
                        case 0xf :  // BLACKNESS
                        {                                                   // in this case nRasterOperation is either 0 or 0xff
                            maFillStyle = WinMtfFillStyle( Color( nRasterOperation, nRasterOperation, nRasterOperation ) );
                            UpdateFillStyle();
                            DrawRect( aRect, sal_False );
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
                            DrawRect( aRect, sal_False );
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

//-----------------------------------------------------------------------------------

void WinMtfOutput::SetDevOrg( const Point& rPoint )
{
    mnDevOrgX = rPoint.X();
    mnDevOrgY = rPoint.Y();
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::SetDevOrgOffset( sal_Int32 nXAdd, sal_Int32 nYAdd )
{
    mnDevOrgX += nXAdd;
    mnDevOrgY += nYAdd;
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::SetDevExt( const Size& rSize )
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
    }
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::ScaleDevExt( double fX, double fY )
{
    mnDevWidth = FRound( mnDevWidth * fX );
    mnDevHeight = FRound( mnDevHeight * fY );
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::SetWinOrg( const Point& rPoint )
{
    mnWinOrgX = rPoint.X();
    mnWinOrgY = rPoint.Y();
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::SetWinOrgOffset( sal_Int32 nXAdd, sal_Int32 nYAdd )
{
    mnWinOrgX += nXAdd;
    mnWinOrgY += nYAdd;
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::SetWinExt( const Size& rSize )
{

    if( rSize.Width() && rSize.Height() )
    {
        switch( mnMapMode )
        {
            case MM_ISOTROPIC :
            case MM_ANISOTROPIC :
            {
                mnWinExtX = rSize.Width();
                mnWinExtY = rSize.Height();
            }
        }
    }
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::ScaleWinExt( double fX, double fY )
{
    mnWinExtX = FRound( mnWinExtX * fX );
    mnWinExtY = FRound( mnWinExtY * fY );
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::SetrclBounds( const Rectangle& rRect )
{
    mrclBounds = rRect;
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::SetrclFrame( const Rectangle& rRect )
{
    mrclFrame = rRect;
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::SetRefPix( const Size& rSize )
{
    mnPixX = rSize.Width();
    mnPixY = rSize.Height();
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::SetRefMill( const Size& rSize )
{
    mnMillX = rSize.Width();
    mnMillY = rSize.Height();
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::SetMapMode( sal_uInt32 nMapMode )
{
    mnMapMode = nMapMode;
    if ( nMapMode == MM_TEXT )
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

//-----------------------------------------------------------------------------------

void WinMtfOutput::SetUnitsPerInch( sal_uInt16 nUnitsPerInch )
{
    if( nUnitsPerInch != 0 )
    mnUnitsPerInch = nUnitsPerInch;
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::SetWorldTransform( const XForm& rXForm )
{
    maXForm.eM11 = rXForm.eM11;
    maXForm.eM12 = rXForm.eM12;
    maXForm.eM21 = rXForm.eM21;
    maXForm.eM22 = rXForm.eM22;
    maXForm.eDx = rXForm.eDx;
    maXForm.eDy = rXForm.eDy;
}

//-----------------------------------------------------------------------------------

void WinMtfOutput::ModifyWorldTransform( const XForm& rXForm, sal_uInt32 nMode )
{
    switch( nMode )
    {
        case MWT_IDENTITY :
        {
            maXForm.eM11 = maXForm.eM12 = maXForm.eM21 = maXForm.eM22 = 1.0f;
            maXForm.eDx = maXForm.eDy = 0.0f;
        }
        break;

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
        }
        break;
    }
 }

//-----------------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------------

void WinMtfOutput::Pop()
{
    // Die aktuellen Daten vom Stack holen
    if( !vSaveStack.empty() )
    {
        // Die aktuelle Daten auf dem Stack sichern
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
   rGDIMetaFile.Play( *mpGDIMetaFile, 0xFFFFFFFF );
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
    mem << nLeft << nTop << nRight << nBottom;
    mem << mnPixX << mnPixY << mnMillX << mnMillY;

    float one, zero;

    one = 1;
    zero = 0;

    // add transformation matrix to be used in vcl's metaact.cxx for
    // rotate and scale operations
    mem << one << zero << zero << one << zero << zero;

    // need to flush the stream, otherwise GetEndOfData will return 0
    // on windows where the function parameters are probably resolved in reverse order
    mem.Flush();

    mpGDIMetaFile->AddAction( new MetaCommentAction( "EMF_PLUS_HEADER_INFO", 0, (const sal_uInt8*) mem.GetData(), mem.GetEndOfData() ) );
    mpGDIMetaFile->UseCanvas( sal_True );
}

void WinMtfOutput::PassEMFPlus( void* pBuffer, sal_uInt32 nLength )
{
    EMFP_DEBUG(printf ("\t\t\tadd EMF_PLUS comment length %04x\n",(unsigned int) nLength));
    mpGDIMetaFile->AddAction( new MetaCommentAction( "EMF_PLUS", 0, static_cast<const sal_uInt8*>(pBuffer), nLength ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
