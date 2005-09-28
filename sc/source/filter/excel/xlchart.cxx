/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xlchart.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-28 11:51:29 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_XLCHART_HXX
#include "xlchart.hxx"
#endif

#ifndef _COM_SUN_STAR_DRAWING_LINEDASH_HPP_
#include <com/sun/star/drawing/LineDash.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINESTYLE_HPP_
#include <com/sun/star/drawing/LineStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FILLSTYLE_HPP_
#include <com/sun/star/drawing/FillStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_BITMAPMODE_HPP_
#include <com/sun/star/drawing/BitmapMode.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_GRADIENT_HPP_
#include <com/sun/star/awt/Gradient.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XBITMAP_HPP_
#include <com/sun/star/awt/XBitmap.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTSYMBOLTYPE_HPP_
#include <com/sun/star/chart/ChartSymbolType.hpp>
#endif

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
#ifndef SVX_XFILLIT0_HXX
#include <svx/xfillit0.hxx>
#endif
#ifndef _SVX_XFLGRIT_HXX
#include <svx/xflgrit.hxx>
#endif
#ifndef _SVX_XBTMPIT_HXX
#include <svx/xbtmpit.hxx>
#endif
#ifndef _SVX_UNOMID_HXX
#include <svx/unomid.hxx>
#endif

#ifndef SC_XLTOOLS_HXX
#include "xltools.hxx"
#endif
#ifndef SC_XLSTYLE_HXX
#include "xlstyle.hxx"
#endif

using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::awt::XBitmap;

typedef ::com::sun::star::awt::Size             ApiSize;
typedef ::com::sun::star::awt::Gradient         ApiGradient;
typedef ::com::sun::star::drawing::LineStyle    ApiLineStyle;
typedef ::com::sun::star::drawing::LineDash     ApiLineDash;
typedef ::com::sun::star::drawing::FillStyle    ApiFillStyle;
typedef ::com::sun::star::drawing::BitmapMode   ApiBitmapMode;

// Common =====================================================================

XclChPoint::XclChPoint() :
    mnSeriesIdx( EXC_CHSERIES_INVALID ),
    mnPointIdx( EXC_CHDATAFORMAT_ALLPOINTS )
{
}

XclChPoint::XclChPoint( sal_uInt16 nSeriesIdx, sal_uInt16 nPointIdx ) :
    mnSeriesIdx( nSeriesIdx ),
    mnPointIdx( nPointIdx )
{
}

bool operator==( const XclChPoint& rL, const XclChPoint& rR )
{
    return (rL.mnSeriesIdx == rR.mnSeriesIdx) && (rL.mnPointIdx == rR.mnPointIdx);
}

bool operator<( const XclChPoint& rL, const XclChPoint& rR )
{
    return (rL.mnSeriesIdx < rR.mnSeriesIdx) ||
        ((rL.mnSeriesIdx == rR.mnSeriesIdx) && (rL.mnPointIdx < rR.mnPointIdx));
}

// Formatting =================================================================

XclChPos::XclChPos() :
    mnX( 0 ),
    mnY( 0 ),
    mnWidth( 0 ),
    mnHeight( 0 )
{
}

// ----------------------------------------------------------------------------

XclChLineFormat::XclChLineFormat() :
    mnPattern( EXC_CHLINEFORMAT_SOLID ),
    mnWeight( EXC_CHLINEFORMAT_SINGLE ),
    mnFlags( EXC_CHLINEFORMAT_AUTO ),
    mnColorIdx( 0 )
{
}

// ----------------------------------------------------------------------------

XclChAreaFormat::XclChAreaFormat() :
    mnPattern( EXC_CHAREAFORMAT_NONE ),
    mnFlags( EXC_CHAREAFORMAT_AUTO ),
    mnForeColorIdx( 0 ),
    mnBackColorIdx( 0 )
{
}

// ----------------------------------------------------------------------------

XclChPicFormat::XclChPicFormat() :
    mnBmpMode( EXC_CHPICFORMAT_STACK ),
    mnFormat( 0 ),
    mnFlags( 0 ),
    mfScale( 0.5 )
{
}

// ----------------------------------------------------------------------------

XclChMarkerFormat::XclChMarkerFormat() :
    mnMarkerSize( EXC_CHMARKERFORMAT_SINGLESIZE ),
    mnMarkerType( EXC_CHMARKERFORMAT_NOSYMBOL ),
    mnFlags( EXC_CHMARKERFORMAT_AUTO ),
    mnLineColorIdx( 0 ),
    mnFillColorIdx( 0 )
{
};

// ----------------------------------------------------------------------------

XclCh3dDataFormat::XclCh3dDataFormat() :
    mnBase( EXC_CH3DDATAFORMAT_RECT ),
    mnTop( EXC_CH3DDATAFORMAT_STRAIGHT )
{
}

// ----------------------------------------------------------------------------

XclChFrame::XclChFrame() :
    mnFormat( EXC_CHFRAME_STANDARD ),
    mnFlags( 0 )
{
}

// Text =======================================================================

XclChObjectLink::XclChObjectLink() :
    mnTarget( EXC_CHOBJLINK_NONE )
{
}

// ----------------------------------------------------------------------------

XclChText::XclChText() :
    mnHAlign( EXC_CHTEXT_ALIGN_CENTER ),
    mnVAlign( EXC_CHTEXT_ALIGN_CENTER ),
    mnBackMode( EXC_CHTEXT_TRANSPARENT ),
    mnFlags( 0 ),
    mnTextColorIdx( 0 ),
    mnPlacement( EXC_CHTEXT_POS_DEFAULT ),
    mnRotation( 0 )
{
}

// Linked source data =========================================================

XclChSourceLink::XclChSourceLink() :
    mnDestType( EXC_CHSRCLINK_TITLE ),
    mnLinkType( EXC_CHSRCLINK_DIRECTLY ),
    mnFlags( 0 ),
    mnNumFmtIdx( 0 )
{
}

// ----------------------------------------------------------------------------

XclChDataFormat::XclChDataFormat() :
    mnFormatIdx( EXC_CHDATAFORMAT_DEFAULT ),
    mnFlags( 0 )
{
}

// ----------------------------------------------------------------------------

XclChSeries::XclChSeries() :
    mnCategType( EXC_CHSERIES_NUMERIC ),
    mnValueType( EXC_CHSERIES_NUMERIC ),
    mnBubbleType( EXC_CHSERIES_NUMERIC ),
    mnCategCount( 0 ),
    mnValueCount( 0 ),
    mnBubbleCount( 0 )
{
}

// Chart structure ============================================================

XclChType::XclChType() :
    mnOverlap( 0 ),
    mnGap( 50 ),
    mnRotation( 0 ),
    mnPieHole( 0 ),
    mnBubbleSize( 100 ),
    mnBubbleType( EXC_CHSCATTER_AREA ),
    mnFlags( 0 )
{
}

// ----------------------------------------------------------------------------

XclChChart3d::XclChChart3d() :
    mnRotation( 0 ),
    mnElevation( 0 ),
    mnDist( 0 ),
    mnHeight( 0 ),
    mnDepth( 0 ),
    mnGap( 0 ),
    mnFlags( 0 )
{
}

// ----------------------------------------------------------------------------

XclChLegend::XclChLegend() :
    mnDockMode( EXC_CHLEGEND_RIGHT ),
    mnSpacing( EXC_CHLEGEND_MEDIUM ),
    mnFlags( 0 )
{
}

// ----------------------------------------------------------------------------

XclChChartFormat::XclChChartFormat() :
    mnFlags( 0 ),
    mnGroupIdx( EXC_CHSERGROUP_NONE )
{
}

// ----------------------------------------------------------------------------

XclChProperties::XclChProperties() :
    mnFlags( 0 ),
    mnEmptyMode( EXC_CHPROPS_EMPTY_SKIP )
{
}

// Axes =======================================================================

XclChLabelRange::XclChLabelRange() :
    mnCross( 1 ),
    mnLabelFreq( 1 ),
    mnTickFreq( 1 ),
    mnFlags( 0 )
{
}

// ----------------------------------------------------------------------------

XclChValueRange::XclChValueRange() :
    mfMin( 0.0 ),
    mfMax( 0.0 ),
    mfMajorStep( 0.0 ),
    mfMinorStep( 0.0 ),
    mfCross( 0.0 ),
    mnFlags( 0 )
{
}

// ----------------------------------------------------------------------------

XclChTick::XclChTick() :
    mnMajor( 0 ),
    mnMinor( 0 ),
    mnLabelPos( EXC_CHTICK_NOLABEL ),
    mnBackMode( EXC_CHTICK_TRANSPARENT ),
    mnFlags( 0 ),
    mnTextColorIdx( 0 ),
    mnRotation( 0 )
{
}

// ----------------------------------------------------------------------------

XclChAxis::XclChAxis() :
    mnType( EXC_CHAXIS_X )
{
}

// ----------------------------------------------------------------------------

XclChAxesSet::XclChAxesSet() :
    mnAxesSetId( EXC_CHAXESSET_PRIMARY )
{
}

// Property helpers ===========================================================

// Property names -------------------------------------------------------------

/** Property names for line style without dash. MUST be sorted alphabetically. */
const sal_Char* sppLineNamesNoDash[] =
{
    "LineColor",
    "LineStyle",
    "LineTransparence",
    "LineWidth"
};

const sal_Int32 PROPINDEX_LINECOLOR_NODASH  = 0;
const sal_Int32 PROPINDEX_LINESTYLE_NODASH  = 1;
const sal_Int32 PROPINDEX_LINETRANSP_NODASH = 2;
const sal_Int32 PROPINDEX_LINEWIDTH_NODASH  = 3;

/** Property names for line style with dash. MUST be sorted alphabetically. */
const sal_Char* sppLineNamesDash[] =
{
    "LineColor",
    "LineDash",
    "LineStyle",
    "LineTransparence",
    "LineWidth"
};

const sal_Int32 PROPINDEX_LINECOLOR_DASH    = 0;
const sal_Int32 PROPINDEX_LINEDASH_DASH     = 1;
const sal_Int32 PROPINDEX_LINESTYLE_DASH    = 2;
const sal_Int32 PROPINDEX_LINETRANSP_DASH   = 3;
const sal_Int32 PROPINDEX_LINEWIDTH_DASH    = 4;

/** Property names for solid area style. MUST be sorted alphabetically. */
const sal_Char* sppAreaNamesSolid[] =
{
    "FillColor",
    "FillStyle"
};

const sal_Int32 PROPINDEX_FILLCOLOR_SOLID   = 0;
const sal_Int32 PROPINDEX_FILLSTYLE_SOLID   = 1;

/** Property names for gradient area style. MUST be sorted alphabetically. */
const sal_Char* sppAreaNamesGradient[] =
{
    "FillGradient",
    "FillGradientStepCount",
    "FillStyle"
};

const sal_Int32 PROPINDEX_FILLGRADIENT_GRADIENT     = 0;
const sal_Int32 PROPINDEX_FILLGRADIENTST_GRADIENT   = 1;
const sal_Int32 PROPINDEX_FILLSTYLE_GRADIENT        = 2;

/** Property names for bitmap area style. MUST be sorted alphabetically. */
const sal_Char* sppAreaNamesBitmap[] =
{
    "FillBitmap",
    "FillBitmapMode",
    "FillStyle"
};

const sal_Int32 PROPINDEX_FILLBITMAP_BITMAP         = 0;
const sal_Int32 PROPINDEX_FILLBITMAPMODE_BITMAP     = 1;
const sal_Int32 PROPINDEX_FILLSTYLE_BITMAP          = 2;

/** Property names for font settings. MUST be sorted alphabetically. */
const sal_Char* sppFontNames[] =
{
    "CharContoured",
    "CharCrossedOut",
    "CharFontName",
    "CharHeight",
    "CharPosture",
    "CharShadowed",
    "CharUnderline",
    "CharWeight"
};

const sal_Int32 PROPINDEX_CHARCOUNTOURED    = 0;
const sal_Int32 PROPINDEX_CHARCROSSEDOUT    = 1;
const sal_Int32 PROPINDEX_CHARFONTNAME      = 2;
const sal_Int32 PROPINDEX_CHARHEIGHT        = 3;
const sal_Int32 PROPINDEX_CHARPOSTURE       = 4;
const sal_Int32 PROPINDEX_CHARSHADOWED      = 5;
const sal_Int32 PROPINDEX_CHARUNDERLINE     = 6;
const sal_Int32 PROPINDEX_CHARWEIGHT        = 7;

/** Property names for marker settings. MUST be sorted alphabetically. */
const sal_Char* sppMarkerNames[] =
{
    "SymbolSize",
    "SymbolType"
};

const sal_Int32 PROPINDEX_SYMBOLSIZE    = 0;
const sal_Int32 PROPINDEX_SYMBOLTYPE    = 1;

// ----------------------------------------------------------------------------

XclChPropSetHelper::XclChPropSetHelper() :
    maLineHlpNoDash(    sppLineNamesNoDash,     STATIC_TABLE_SIZE( sppLineNamesNoDash ) ),
    maLineHlpDash(      sppLineNamesDash,       STATIC_TABLE_SIZE( sppLineNamesDash ) ),
    maAreaHlpSolid(     sppAreaNamesSolid,      STATIC_TABLE_SIZE( sppAreaNamesSolid ) ),
    maAreaHlpGradient(  sppAreaNamesGradient,   STATIC_TABLE_SIZE( sppAreaNamesGradient ) ),
    maAreaHlpBitmap(    sppAreaNamesBitmap,     STATIC_TABLE_SIZE( sppAreaNamesBitmap ) ),
    maFontHlp(          sppFontNames,           STATIC_TABLE_SIZE( sppFontNames ) ),
    maMarkerHlp(        sppMarkerNames,         STATIC_TABLE_SIZE( sppMarkerNames ) )
{
}

void XclChPropSetHelper::WriteToPropertySet( ScfPropertySet& rPropSet, const XclChLineFormat& rLineFmt )
{
    namespace cssdraw = ::com::sun::star::drawing;

    // line width
    sal_Int32 nApiWidth = 0;
    switch( rLineFmt.mnWeight )
    {
        case EXC_CHLINEFORMAT_SINGLE:   nApiWidth = 35;     break;
        case EXC_CHLINEFORMAT_DOUBLE:   nApiWidth = 70;     break;
        case EXC_CHLINEFORMAT_TRIPLE:   nApiWidth = 105;    break;
    }

    // line style
    ApiLineStyle eApiStyle = cssdraw::LineStyle_NONE;
    sal_Int16 nApiTrans = 0;
    sal_Int32 nDotLen = ::std::min< sal_Int32 >( rLineFmt.mnWeight + 105, 210 );
    ApiLineDash aApiDash( cssdraw::DashStyle_RECT, 0, nDotLen, 0, 4 * nDotLen, nDotLen );

    switch( rLineFmt.mnPattern )
    {
        case EXC_CHLINEFORMAT_SOLID:
            eApiStyle = cssdraw::LineStyle_SOLID;
        break;
        case EXC_CHLINEFORMAT_DARKTRANS:
            eApiStyle = cssdraw::LineStyle_SOLID; nApiTrans = 25;
        break;
        case EXC_CHLINEFORMAT_MEDTRANS:
            eApiStyle = cssdraw::LineStyle_SOLID; nApiTrans = 50;
        break;
        case EXC_CHLINEFORMAT_LIGHTTRANS:
            eApiStyle = cssdraw::LineStyle_SOLID; nApiTrans = 75;
        break;
        case EXC_CHLINEFORMAT_DASH:
            eApiStyle = cssdraw::LineStyle_DASH; aApiDash.Dashes = 1;
        break;
        case EXC_CHLINEFORMAT_DOT:
            eApiStyle = cssdraw::LineStyle_DASH; aApiDash.Dots = 1;
        break;
        case EXC_CHLINEFORMAT_DASHDOT:
            eApiStyle = cssdraw::LineStyle_DASH; aApiDash.Dashes = aApiDash.Dots = 1;
        break;
        case EXC_CHLINEFORMAT_DASHDOTDOT:
            eApiStyle = cssdraw::LineStyle_DASH; aApiDash.Dashes = 1; aApiDash.Dots = 2;
        break;
    }

    // color
    sal_Int32 nApiColor = static_cast< sal_Int32 >( rLineFmt.maColor.GetColor() );

    // set the properties
    if( eApiStyle == cssdraw::LineStyle_DASH )
    {
        maLineHlpDash.GetAny( PROPINDEX_LINESTYLE_DASH )  <<= eApiStyle;
        maLineHlpDash.GetAny( PROPINDEX_LINEWIDTH_DASH )  <<= nApiWidth;
        maLineHlpDash.GetAny( PROPINDEX_LINECOLOR_DASH )  <<= nApiColor;
        maLineHlpDash.GetAny( PROPINDEX_LINETRANSP_DASH ) <<= nApiTrans;
        maLineHlpDash.GetAny( PROPINDEX_LINEDASH_DASH )   <<= aApiDash;
        maLineHlpDash.WriteToPropertySet( rPropSet );
    }
    else
    {
        maLineHlpNoDash.GetAny( PROPINDEX_LINESTYLE_NODASH )  <<= eApiStyle;
        maLineHlpNoDash.GetAny( PROPINDEX_LINEWIDTH_NODASH )  <<= nApiWidth;
        maLineHlpNoDash.GetAny( PROPINDEX_LINECOLOR_NODASH )  <<= nApiColor;
        maLineHlpNoDash.GetAny( PROPINDEX_LINETRANSP_NODASH ) <<= nApiTrans;
        maLineHlpNoDash.WriteToPropertySet( rPropSet );
    }
}

void XclChPropSetHelper::WriteToPropertySet( ScfPropertySet& rPropSet, const XclChAreaFormat& rAreaFmt )
{
    namespace cssdraw = ::com::sun::star::drawing;

    ApiFillStyle eApiStyle = cssdraw::FillStyle_NONE;
    sal_Int32 nApiColor = 0;
    if( rAreaFmt.mnPattern != EXC_CHAREAFORMAT_NONE )
    {
        eApiStyle = cssdraw::FillStyle_SOLID;
        const Color& rColor = (rAreaFmt.mnPattern == EXC_CHAREAFORMAT_SOLID) ?
            rAreaFmt.maForeColor : rAreaFmt.maBackColor;
        nApiColor = static_cast< sal_Int32 >( rColor.GetColor() );
    }

    maAreaHlpSolid.GetAny( PROPINDEX_FILLSTYLE_SOLID ) <<= eApiStyle;
    maAreaHlpSolid.GetAny( PROPINDEX_FILLCOLOR_SOLID ) <<= nApiColor;
    maAreaHlpSolid.WriteToPropertySet( rPropSet );
}

void XclChPropSetHelper::WriteToPropertySet( ScfPropertySet& rPropSet,
        const SfxItemSet& rItemSet, const XclChPicFormat& rPicFmt )
{
    XFillStyle eFillStyle = XFILL_NONE;
    if( const XFillStyleItem* pStyleItem = static_cast< const XFillStyleItem* >( rItemSet.GetItem( XATTR_FILLSTYLE, FALSE ) ) )
        eFillStyle = pStyleItem->GetValue();

    switch( eFillStyle )
    {
        case XFILL_GRADIENT:
            if( const XFillGradientItem* pGradItem = static_cast< const XFillGradientItem* >( rItemSet.GetItem( XATTR_FILLGRADIENT, FALSE ) ) )
            {
                ApiGradient aApiGradient;
                Any aAny;
                if( pGradItem->QueryValue( aAny, MID_FILLGRADIENT ) && (aAny >>= aApiGradient) )
                {
                    aApiGradient.StepCount = 256;

                    maAreaHlpGradient.GetAny( PROPINDEX_FILLSTYLE_GRADIENT )      <<= ::com::sun::star::drawing::FillStyle_GRADIENT;
                    maAreaHlpGradient.GetAny( PROPINDEX_FILLGRADIENT_GRADIENT )   <<= aApiGradient;
                    maAreaHlpGradient.GetAny( PROPINDEX_FILLGRADIENTST_GRADIENT ) <<= aApiGradient.StepCount;
                    maAreaHlpGradient.WriteToPropertySet( rPropSet );
                }
            }
        break;
        case XFILL_BITMAP:
            if( const XFillBitmapItem* pBmpItem = static_cast< const XFillBitmapItem* >( rItemSet.GetItem( XATTR_FILLBITMAP, FALSE ) ) )
            {
                Reference< XBitmap > xBitmap;
                Any aAny;
                if( pBmpItem->QueryValue( aAny, MID_BITMAP ) && (aAny >>= xBitmap) )
                {
                    namespace cssd = ::com::sun::star::drawing;
                    ApiBitmapMode eApiBmpMode = (rPicFmt.mnBmpMode == EXC_CHPICFORMAT_STRETCH) ?
                        cssd::BitmapMode_STRETCH : cssd::BitmapMode_REPEAT;

                    maAreaHlpBitmap.GetAny( PROPINDEX_FILLSTYLE_BITMAP )      <<= ::com::sun::star::drawing::FillStyle_BITMAP;
                    maAreaHlpBitmap.GetAny( PROPINDEX_FILLBITMAP_BITMAP )     <<= xBitmap;
                    maAreaHlpBitmap.GetAny( PROPINDEX_FILLBITMAPMODE_BITMAP ) <<= eApiBmpMode;
                    maAreaHlpBitmap.WriteToPropertySet( rPropSet );
                }
            }
        break;
    }
}

void XclChPropSetHelper::WriteToPropertySet( ScfPropertySet& rPropSet, const XclFontData& rFontData )
{
    maFontHlp.GetAny( PROPINDEX_CHARFONTNAME )    <<= OUString( rFontData.maName );
    maFontHlp.GetAny( PROPINDEX_CHARHEIGHT )      <<= rFontData.GetApiHeight();
    maFontHlp.GetAny( PROPINDEX_CHARPOSTURE )     <<= rFontData.GetApiPosture();
    maFontHlp.GetAny( PROPINDEX_CHARUNDERLINE )   <<= rFontData.GetApiUnderline();
    maFontHlp.GetAny( PROPINDEX_CHARWEIGHT )      <<= rFontData.GetApiWeight();

    ::comphelper::setBOOL( maFontHlp.GetAny( PROPINDEX_CHARCOUNTOURED ), rFontData.mbOutline );
    ::comphelper::setBOOL( maFontHlp.GetAny( PROPINDEX_CHARCROSSEDOUT ), rFontData.mbStrikeout );
    ::comphelper::setBOOL( maFontHlp.GetAny( PROPINDEX_CHARSHADOWED ),   rFontData.mbShadow );

    maFontHlp.WriteToPropertySet( rPropSet );
}

void XclChPropSetHelper::WriteToPropertySet( ScfPropertySet& rPropSet, const XclChMarkerFormat& rMarkerFmt )
{
    using namespace ::com::sun::star::chart::ChartSymbolType;
    sal_Int32 nApiType = SYMBOL1;
    switch( rMarkerFmt.mnMarkerType )
    {
        case EXC_CHMARKERFORMAT_NOSYMBOL:   nApiType = NONE;    break;
        case EXC_CHMARKERFORMAT_SQUARE:     nApiType = SYMBOL0; break;  // square
        case EXC_CHMARKERFORMAT_DIAMOND:    nApiType = SYMBOL1; break;  // diamond
        case EXC_CHMARKERFORMAT_TRIANGLE:   nApiType = SYMBOL3; break;  // triangle up
        case EXC_CHMARKERFORMAT_CROSS:      nApiType = SYMBOL6; break;  // bow tie
        case EXC_CHMARKERFORMAT_STAR:       nApiType = SYMBOL7; break;  // rotated bow tie
        case EXC_CHMARKERFORMAT_DOWJ:       nApiType = AUTO;    break;
        case EXC_CHMARKERFORMAT_STDDEV:     nApiType = SYMBOL2; break;  // triangle down
        case EXC_CHMARKERFORMAT_CIRCLE:     nApiType = SYMBOL4; break;  // triangle right
        case EXC_CHMARKERFORMAT_PLUS:       nApiType = SYMBOL5; break;  // triangle left
    }
    sal_Int32 nApiSize = XclTools::GetHmmFromTwips( rMarkerFmt.mnMarkerSize );
    ApiSize aApiSize( nApiSize, nApiSize );

    maMarkerHlp.GetAny( PROPINDEX_SYMBOLSIZE )  <<= aApiSize;
    maMarkerHlp.GetAny( PROPINDEX_SYMBOLTYPE )  <<= nApiType;
    maMarkerHlp.WriteToPropertySet( rPropSet );
}

// ============================================================================

