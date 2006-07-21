/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xlchart.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 12:15:42 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#ifndef SC_XLCHART_HXX
#include "xlchart.hxx"
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_GRADIENT_HPP_
#include <com/sun/star/awt/Gradient.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XBITMAP_HPP_
#include <com/sun/star/awt/XBitmap.hpp>
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

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

#ifndef SC_XLCONST_HXX
#include "xlconst.hxx"
#endif
#ifndef SC_XLTOOLS_HXX
#include "xltools.hxx"
#endif

using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::awt::XBitmap;

typedef ::com::sun::star::awt::Gradient ApiGradient;

// Common =====================================================================

XclChRectangle::XclChRectangle() :
    mnX( 0 ),
    mnY( 0 ),
    mnWidth( 0 ),
    mnHeight( 0 )
{
}

// ----------------------------------------------------------------------------

XclChDataPointPos::XclChDataPointPos() :
    mnSeriesIdx( EXC_CHSERIES_INVALID ),
    mnPointIdx( EXC_CHDATAFORMAT_ALLPOINTS )
{
}

bool operator==( const XclChDataPointPos& rL, const XclChDataPointPos& rR )
{
    return (rL.mnSeriesIdx == rR.mnSeriesIdx) && (rL.mnPointIdx == rR.mnPointIdx);
}

bool operator<( const XclChDataPointPos& rL, const XclChDataPointPos& rR )
{
    return (rL.mnSeriesIdx < rR.mnSeriesIdx) ||
        ((rL.mnSeriesIdx == rR.mnSeriesIdx) && (rL.mnPointIdx < rR.mnPointIdx));
}

// Formatting =================================================================

XclChFramePos::XclChFramePos() :
    mnObjType( EXC_CHFRAMEPOS_ANY ),
    mnSizeMode( EXC_CHFRAMEPOS_AUTOSIZE )
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

XclChSerTrendLine::XclChSerTrendLine() :
    mfForecastFor( 0.0 ),
    mfForecastBack( 0.0 ),
    mnLineType( EXC_CHSERTREND_POLYNOMIAL ),
    mnOrder( 1 ),
    mnShowEquation( 0 ),
    mnShowRSquared( 0 )
{
    ::rtl::math::setNan( &mfIntercept );
}

// ----------------------------------------------------------------------------

XclChSerErrorBar::XclChSerErrorBar() :
    mfValue( 0.0 ),
    mnValueCount( 1 ),
    mnBarType( EXC_CHSERERR_NONE ),
    mnSourceType( EXC_CHSERERR_FIXED ),
    mnLineEnd( EXC_CHSERERR_END_TSHAPE )
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

XclChChartGroup::XclChChartGroup() :
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

// Static helper functions ====================================================

bool XclChartHelper::IsLineChartType( sal_uInt16 nTypeId )
{
    return
        (nTypeId == EXC_ID_CHLINE) ||
        (nTypeId == EXC_ID_CHAREA) ||
        (nTypeId == EXC_ID_CHSTOCK);
}

bool XclChartHelper::IsBarChartType( sal_uInt16 nTypeId )
{
    return
        (nTypeId == EXC_ID_CHBAR) ||
        (nTypeId == EXC_ID_CHCOLUMN);
}

bool XclChartHelper::IsRadarChartType( sal_uInt16 nTypeId )
{
    return
        (nTypeId == EXC_ID_CHRADARLINE) ||
        (nTypeId == EXC_ID_CHRADARAREA);
}

bool XclChartHelper::IsPieChartType( sal_uInt16 nTypeId )
{
    return
        (nTypeId == EXC_ID_CHPIE) ||
        (nTypeId == EXC_ID_CHDONUT) ||
        (nTypeId == EXC_ID_CHPIEEXT);
}

bool XclChartHelper::IsScatterChartType( sal_uInt16 nTypeId )
{
    return
        (nTypeId == EXC_ID_CHSCATTER) ||
        (nTypeId == EXC_ID_CHBUBBLES);
}

bool XclChartHelper::HasPolarCoordSystem( sal_uInt16 nTypeId )
{
    return IsRadarChartType( nTypeId ) || IsPieChartType( nTypeId );
}

bool XclChartHelper::HasLinearSeries( sal_uInt16 nTypeId )
{
    return
        (nTypeId == EXC_ID_CHLINE) ||
        (nTypeId == EXC_ID_CHSTOCK) ||
        (nTypeId == EXC_ID_CHSCATTER) ||
        (nTypeId == EXC_ID_CHRADARLINE);
}

bool XclChartHelper::HasCategoryAxis( sal_uInt16 nTypeId )
{
    return !IsScatterChartType( nTypeId );
}

bool XclChartHelper::HasSwappedAxesSet( sal_uInt16 nTypeId )
{
    return nTypeId == EXC_ID_CHBAR;
}

// Property helpers ===========================================================

XclChObjectTable::XclChObjectTable( Reference< XMultiServiceFactory > xFactory,
        const OUString& rServiceName, const OUString& rObjNameBase ) :
    mxFactory( xFactory ),
    maServiceName( rServiceName ),
    maObjNameBase( rObjNameBase ),
    mnIndex( 0 )
{
}

OUString XclChObjectTable::InsertObject( const Any& rObj )
{
    OUString aObjName;
    bool bInserted = false;

    // create object table
    if( !mxContainer.is() )
        mxContainer.set( ScfApiHelper::CreateInstance( mxFactory, maServiceName ), UNO_QUERY );

    if( mxContainer.is() )
    {
        // create new unused identifier
        do
        {
            aObjName = maObjNameBase + OUString::valueOf( ++mnIndex );
        }
        while( mxContainer->hasByName( aObjName ) );

        // insert object
        try
        {
            mxContainer->insertByName( aObjName, rObj );
            bInserted = true;
        }
        catch( Exception& )
        {
        }
    }
    DBG_ASSERT( bInserted, "XclChObjectTable::InsertObject - cannot insert object" );
    return bInserted ? aObjName : OUString();
}

// Property names -------------------------------------------------------------

namespace {

/** Property names for line style. */
const sal_Char* const sppcLineNames[] =
    { "LineStyle", "LineWidth", "LineColor", "LineTransparence", "LineDashName", 0 };

/** Property names for solid area style. */
const sal_Char* const sppcAreaNames[] = { "FillStyle", "FillColor", 0 };
/** Property names for gradient area style. */
const sal_Char* const sppcGradientNames[] = {  "FillStyle", "FillGradientName", 0 };
/** Property names for bitmap area style. */
const sal_Char* const sppcBitmapNames[] = { "FillStyle", "FillBitmapName", "FillBitmapMode", 0 };

/** Property names for marker settings. */
const sal_Char* const sppcMarkerNames[] = { "SymbolType", "SymbolSize", 0 };

} // namespace

// ----------------------------------------------------------------------------

XclChPropSetHelper::XclChPropSetHelper() :
    maLineHlp( sppcLineNames ),
    maAreaHlp( sppcAreaNames ),
    maGradientHlp( sppcGradientNames ),
    maBitmapHlp( sppcBitmapNames ),
    maMarkerHlp( sppcMarkerNames )
{
}

void XclChPropSetHelper::WriteLineProperties(
        ScfPropertySet& rPropSet, XclChObjectTable& rDashTable,
        const XclChLineFormat& rLineFmt, XclChPropertyMode ePropMode )
{
    namespace cssd = ::com::sun::star::drawing;

    // line width
    sal_Int32 nApiWidth = 0;
    switch( rLineFmt.mnWeight )
    {
        case EXC_CHLINEFORMAT_SINGLE:   nApiWidth = 35;     break;
        case EXC_CHLINEFORMAT_DOUBLE:   nApiWidth = 70;     break;
        case EXC_CHLINEFORMAT_TRIPLE:   nApiWidth = 105;    break;
    }

    // line style
    cssd::LineStyle eApiStyle = cssd::LineStyle_NONE;
    sal_Int16 nApiTrans = 0;
    sal_Int32 nDotLen = ::std::min< sal_Int32 >( rLineFmt.mnWeight + 105, 210 );
    cssd::LineDash aApiDash( cssd::DashStyle_RECT, 0, nDotLen, 0, 4 * nDotLen, nDotLen );

    switch( rLineFmt.mnPattern )
    {
        case EXC_CHLINEFORMAT_SOLID:
            eApiStyle = cssd::LineStyle_SOLID;
        break;
        case EXC_CHLINEFORMAT_DARKTRANS:
            eApiStyle = cssd::LineStyle_SOLID; nApiTrans = 25;
        break;
        case EXC_CHLINEFORMAT_MEDTRANS:
            eApiStyle = cssd::LineStyle_SOLID; nApiTrans = 50;
        break;
        case EXC_CHLINEFORMAT_LIGHTTRANS:
            eApiStyle = cssd::LineStyle_SOLID; nApiTrans = 75;
        break;
        case EXC_CHLINEFORMAT_DASH:
            eApiStyle = cssd::LineStyle_DASH; aApiDash.Dashes = 1;
        break;
        case EXC_CHLINEFORMAT_DOT:
            eApiStyle = cssd::LineStyle_DASH; aApiDash.Dots = 1;
        break;
        case EXC_CHLINEFORMAT_DASHDOT:
            eApiStyle = cssd::LineStyle_DASH; aApiDash.Dashes = aApiDash.Dots = 1;
        break;
        case EXC_CHLINEFORMAT_DASHDOTDOT:
            eApiStyle = cssd::LineStyle_DASH; aApiDash.Dashes = 1; aApiDash.Dots = 2;
        break;
    }

    // color
    sal_Int32 nApiColor = ScfApiHelper::ConvertToApiColor( rLineFmt.maColor );

    // try to insert the dash style and receive its name
    Any aDashNameAny;
    if( eApiStyle == cssd::LineStyle_DASH )
    {
        OUString aDashName = rDashTable.InsertObject( ::com::sun::star::uno::makeAny( aApiDash ) );
        if( aDashName.getLength() )
            aDashNameAny <<= aDashName;
    }

    // write the properties
    maLineHlp.InitializeWrite();
    maLineHlp << eApiStyle << nApiWidth << nApiColor << nApiTrans << aDashNameAny;
    maLineHlp.WriteToPropertySet( rPropSet );
}

void XclChPropSetHelper::WriteAreaProperties( ScfPropertySet& rPropSet,
        const XclChAreaFormat& rAreaFmt, XclChPropertyMode ePropMode )
{
    namespace cssd = ::com::sun::star::drawing;
    if( rAreaFmt.mnPattern == EXC_CHAREAFORMAT_NONE )
    {
        rPropSet.SetProperty( EXC_CHPROP_FILLSTYLE, cssd::FillStyle_NONE );
    }
    else
    {
        // fill color
        bool bSolid = rAreaFmt.mnPattern == EXC_CHAREAFORMAT_SOLID;
        const Color& rColor = bSolid ? rAreaFmt.maForeColor : rAreaFmt.maBackColor;

        // write the properties
        maAreaHlp.InitializeWrite();
        maAreaHlp << cssd::FillStyle_SOLID << rColor;
        maAreaHlp.WriteToPropertySet( rPropSet );
    }
}

void XclChPropSetHelper::WriteEscherProperties( ScfPropertySet& rPropSet,
        XclChObjectTable& rGradientTable, XclChObjectTable& rBitmapTable,
        const SfxItemSet& rItemSet, const XclChPicFormat& rPicFmt )
{
    if( const XFillStyleItem* pStyleItem = static_cast< const XFillStyleItem* >( rItemSet.GetItem( XATTR_FILLSTYLE, FALSE ) ) )
    {
        switch( pStyleItem->GetValue() )
        {
            case XFILL_GRADIENT:
                if( const XFillGradientItem* pGradItem = static_cast< const XFillGradientItem* >( rItemSet.GetItem( XATTR_FILLGRADIENT, FALSE ) ) )
                {
                    Any aGradient;
                    if( pGradItem->QueryValue( aGradient, MID_FILLGRADIENT ) )
                    {
                        OUString aGradName = rGradientTable.InsertObject( aGradient );
                        if( aGradName.getLength() )
                        {
                            namespace cssd = ::com::sun::star::drawing;
                            maGradientHlp.InitializeWrite();
                            maGradientHlp << cssd::FillStyle_GRADIENT << aGradName;
                            maGradientHlp.WriteToPropertySet( rPropSet );
                        }
                    }
                }
            break;
            case XFILL_BITMAP:
                if( const XFillBitmapItem* pBmpItem = static_cast< const XFillBitmapItem* >( rItemSet.GetItem( XATTR_FILLBITMAP, FALSE ) ) )
                {
                    Any aBitmap;
                    if( pBmpItem->QueryValue( aBitmap, MID_GRAFURL ) )
                    {
                        OUString aBmpName = rBitmapTable.InsertObject( aBitmap );
                        if( aBmpName.getLength() )
                        {
                            namespace cssd = ::com::sun::star::drawing;
                            cssd::BitmapMode eApiBmpMode = (rPicFmt.mnBmpMode == EXC_CHPICFORMAT_STRETCH) ?
                                cssd::BitmapMode_STRETCH : cssd::BitmapMode_REPEAT;
                            maBitmapHlp.InitializeWrite();
                            maBitmapHlp << cssd::FillStyle_BITMAP << aBmpName << eApiBmpMode;
                            maBitmapHlp.WriteToPropertySet( rPropSet );
                        }
                    }
                }
            break;
            default:
                DBG_ERRORFILE( "XclChPropSetHelper::WriteEscherProperties - unknown fill mode" );
        }
    }
}

void XclChPropSetHelper::WriteMarkerProperties(
        ScfPropertySet& rPropSet, const XclChMarkerFormat& rMarkerFmt )
{
    using namespace ::com::sun::star::chart::ChartSymbolType;
    namespace cssa = ::com::sun::star::awt;
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
    cssa::Size aApiSize( nApiSize, nApiSize );

    maMarkerHlp.InitializeWrite();
    maMarkerHlp << nApiType << aApiSize;
    maMarkerHlp.WriteToPropertySet( rPropSet );
}

// ============================================================================

