/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xlchart.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:28:32 $
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

#include "xlchart.hxx"

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/chart2/LegendPosition.hpp>
#include <com/sun/star/chart2/LegendExpansion.hpp>
#include <com/sun/star/chart2/Symbol.hpp>
#include <com/sun/star/chart/DataLabelPlacement.hpp>

#include <rtl/math.hxx>
#include <svtools/itemset.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xfltrit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/unomid.hxx>
#include <svx/escherex.hxx>

#include "global.hxx"
#include "xlconst.hxx"
#include "xlstyle.hxx"
#include "xltools.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::chart2::XChartDocument;

// Common =====================================================================

XclChRectangle::XclChRectangle() :
    mnX( 0 ),
    mnY( 0 ),
    mnWidth( 0 ),
    mnHeight( 0 )
{
}

// ----------------------------------------------------------------------------

XclChDataPointPos::XclChDataPointPos( sal_uInt16 nSeriesIdx, sal_uInt16 nPointIdx ) :
    mnSeriesIdx( nSeriesIdx ),
    mnPointIdx( nPointIdx )
{
}

bool operator<( const XclChDataPointPos& rL, const XclChDataPointPos& rR )
{
    return (rL.mnSeriesIdx < rR.mnSeriesIdx) ||
        ((rL.mnSeriesIdx == rR.mnSeriesIdx) && (rL.mnPointIdx < rR.mnPointIdx));
}

// Frame formatting ===========================================================

XclChFramePos::XclChFramePos() :
    mnObjType( EXC_CHFRAMEPOS_ANY ),
    mnSizeMode( EXC_CHFRAMEPOS_AUTOSIZE )
{
}

// ----------------------------------------------------------------------------

XclChLineFormat::XclChLineFormat() :
    maColor( COL_BLACK ),
    mnPattern( EXC_CHLINEFORMAT_SOLID ),
    mnWeight( EXC_CHLINEFORMAT_SINGLE ),
    mnFlags( EXC_CHLINEFORMAT_AUTO )
{
}

// ----------------------------------------------------------------------------

XclChAreaFormat::XclChAreaFormat() :
    maPattColor( COL_WHITE ),
    maBackColor( COL_BLACK ),
    mnPattern( EXC_PATT_SOLID ),
    mnFlags( EXC_CHAREAFORMAT_AUTO )
{
}

// ----------------------------------------------------------------------------

XclChEscherFormat::XclChEscherFormat()
{
}

XclChEscherFormat::~XclChEscherFormat()
{
}

// ----------------------------------------------------------------------------

XclChPicFormat::XclChPicFormat() :
    mnBmpMode( EXC_CHPICFORMAT_NONE ),
    mnFormat( EXC_CHPICFORMAT_DEFAULT ),
    mnFlags( EXC_CHPICFORMAT_DEFAULTFLAGS ),
    mfScale( 0.5 )
{
}

// ----------------------------------------------------------------------------

XclChFrame::XclChFrame() :
    mnFormat( EXC_CHFRAME_STANDARD ),
    mnFlags( EXC_CHFRAME_AUTOSIZE | EXC_CHFRAME_AUTOPOS )
{
}

// Source links ===============================================================

XclChSourceLink::XclChSourceLink() :
    mnDestType( EXC_CHSRCLINK_TITLE ),
    mnLinkType( EXC_CHSRCLINK_DEFAULT ),
    mnFlags( 0 ),
    mnNumFmtIdx( 0 )
{
}

// Text =======================================================================

XclChObjectLink::XclChObjectLink() :
    mnTarget( EXC_CHOBJLINK_NONE )
{
}

// ----------------------------------------------------------------------------

XclChText::XclChText() :
    maTextColor( COL_BLACK ),
    mnHAlign( EXC_CHTEXT_ALIGN_CENTER ),
    mnVAlign( EXC_CHTEXT_ALIGN_CENTER ),
    mnBackMode( EXC_CHTEXT_TRANSPARENT ),
    mnFlags( EXC_CHTEXT_AUTOCOLOR | EXC_CHTEXT_AUTOFILL ),
    mnPlacement( EXC_CHTEXT_POS_DEFAULT ),
    mnRotation( EXC_ROT_NONE )
{
}

// Data series ================================================================

XclChMarkerFormat::XclChMarkerFormat() :
    maLineColor( COL_BLACK ),
    maFillColor( COL_WHITE ),
    mnMarkerSize( EXC_CHMARKERFORMAT_SINGLESIZE ),
    mnMarkerType( EXC_CHMARKERFORMAT_NOSYMBOL ),
    mnFlags( EXC_CHMARKERFORMAT_AUTO )
{
};

// ----------------------------------------------------------------------------

XclCh3dDataFormat::XclCh3dDataFormat() :
    mnBase( EXC_CH3DDATAFORMAT_RECT ),
    mnTop( EXC_CH3DDATAFORMAT_STRAIGHT )
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
    /*  Set all bits in mfIntercept to 1 (that is -1.#NAN) to indicate that
        there is no interception point. Cannot use ::rtl::math::setNan() here
        cause it misses the sign bit. */
    sal_math_Double* pDouble = reinterpret_cast< sal_math_Double* >( &mfIntercept );
    pDouble->w32_parts.msw = pDouble->w32_parts.lsw = 0xFFFFFFFF;
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

// Chart type groups ==========================================================

XclChType::XclChType() :
    mnOverlap( 0 ),
    mnGap( 150 ),
    mnRotation( 0 ),
    mnPieHole( 0 ),
    mnBubbleSize( 100 ),
    mnBubbleType( EXC_CHSCATTER_AREA ),
    mnFlags( 0 )
{
}

// ----------------------------------------------------------------------------

XclChChart3d::XclChChart3d() :
    mnRotation( 20 ),
    mnElevation( 15 ),
    mnEyeDist( 30 ),
    mnRelHeight( 100 ),
    mnRelDepth( 100 ),
    mnDepthGap( 150 ),
    mnFlags( EXC_CHCHART3D_AUTOHEIGHT )
{
}

// ----------------------------------------------------------------------------

XclChLegend::XclChLegend() :
    mnDockMode( EXC_CHLEGEND_RIGHT ),
    mnSpacing( EXC_CHLEGEND_MEDIUM ),
    mnFlags( EXC_CHLEGEND_DOCKED | EXC_CHLEGEND_AUTOSERIES |
        EXC_CHLEGEND_AUTOPOSX | EXC_CHLEGEND_AUTOPOSY | EXC_CHLEGEND_STACKED )
{
}

// ----------------------------------------------------------------------------

XclChTypeGroup::XclChTypeGroup() :
    mnFlags( 0 ),
    mnGroupIdx( EXC_CHSERGROUP_NONE )
{
}

// ----------------------------------------------------------------------------

XclChProperties::XclChProperties() :
    mnFlags( EXC_CHPROPS_MANSERIES ),
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
    mnFlags( EXC_CHVALUERANGE_AUTOMIN | EXC_CHVALUERANGE_AUTOMAX |
        EXC_CHVALUERANGE_AUTOMAJOR | EXC_CHVALUERANGE_AUTOMINOR | EXC_CHVALUERANGE_AUTOCROSS | EXC_CHVALUERANGE_BIT8 )
{
}

// ----------------------------------------------------------------------------

XclChTick::XclChTick() :
    maTextColor( COL_BLACK ),
    mnMajor( EXC_CHTICK_INSIDE | EXC_CHTICK_OUTSIDE ),
    mnMinor( 0 ),
    mnLabelPos( EXC_CHTICK_NEXT ),
    mnBackMode( EXC_CHTICK_TRANSPARENT ),
    mnFlags( EXC_CHTICK_AUTOCOLOR | EXC_CHTICK_AUTOROT ),
    mnRotation( EXC_ROT_NONE )
{
}

// ----------------------------------------------------------------------------

XclChAxis::XclChAxis() :
    mnType( EXC_CHAXIS_NONE )
{
}

sal_Int32 XclChAxis::GetApiAxisDimension() const
{
    sal_Int32 nApiAxisDim = EXC_CHART_AXIS_NONE;
    switch( mnType )
    {
        case EXC_CHAXIS_X:  nApiAxisDim = EXC_CHART_AXIS_X; break;
        case EXC_CHAXIS_Y:  nApiAxisDim = EXC_CHART_AXIS_Y; break;
        case EXC_CHAXIS_Z:  nApiAxisDim = EXC_CHART_AXIS_Z; break;
    }
    return nApiAxisDim;
}

// ----------------------------------------------------------------------------

XclChAxesSet::XclChAxesSet() :
    mnAxesSetId( EXC_CHAXESSET_PRIMARY )
{
}

sal_Int32 XclChAxesSet::GetApiAxesSetIndex() const
{
    sal_Int32 nApiAxesSetIdx = EXC_CHART_AXESSET_NONE;
    switch( mnAxesSetId )
    {
        case EXC_CHAXESSET_PRIMARY:     nApiAxesSetIdx = EXC_CHART_AXESSET_PRIMARY;     break;
        case EXC_CHAXESSET_SECONDARY:   nApiAxesSetIdx = EXC_CHART_AXESSET_SECONDARY;   break;
    }
    return nApiAxesSetIdx;
}

// Static helper functions ====================================================

sal_uInt16 XclChartHelper::GetSeriesLineAutoColorIdx( sal_uInt16 nFormatIdx )
{
    static const sal_uInt16 spnLineColors[] =
    {
        32, 33, 34, 35, 36, 37, 38, 39,
        40, 41, 42, 43, 44, 45, 46, 47,
        48, 49, 50, 51, 52, 53, 54, 55,
        56, 57, 58, 59, 60, 61, 62,  8,
         9, 10, 11, 12, 13, 14, 15, 16,
        17, 18, 19, 20, 21, 22, 23, 24,
        25, 26, 27, 28, 29, 30, 31, 63
    };
    return spnLineColors[ nFormatIdx % STATIC_TABLE_SIZE( spnLineColors ) ];
}

sal_uInt16 XclChartHelper::GetSeriesFillAutoColorIdx( sal_uInt16 nFormatIdx )
{
    static const sal_uInt16 spnFillColors[] =
    {
        24, 25, 26, 27, 28, 29, 30, 31,
        32, 33, 34, 35, 36, 37, 38, 39,
        40, 41, 42, 43, 44, 45, 46, 47,
        48, 49, 50, 51, 52, 53, 54, 55,
        56, 57, 58, 59, 60, 61, 62, 63,
         8,  9, 10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22, 23
    };
    return spnFillColors[ nFormatIdx % STATIC_TABLE_SIZE( spnFillColors ) ];
}

sal_uInt8 XclChartHelper::GetSeriesFillAutoTransp( sal_uInt16 nFormatIdx )
{
    static const sal_uInt8 spnTrans[] = { 0x00, 0x40, 0x20, 0x60, 0x70 };
    return spnTrans[ (nFormatIdx / 56) % STATIC_TABLE_SIZE( spnTrans ) ];
}

sal_uInt16 XclChartHelper::GetAutoMarkerType( sal_uInt16 nFormatIdx )
{
    static const sal_uInt16 spnSymbols[] = {
        EXC_CHMARKERFORMAT_DIAMOND, EXC_CHMARKERFORMAT_SQUARE, EXC_CHMARKERFORMAT_TRIANGLE,
        EXC_CHMARKERFORMAT_CROSS, EXC_CHMARKERFORMAT_STAR, EXC_CHMARKERFORMAT_CIRCLE,
        EXC_CHMARKERFORMAT_PLUS, EXC_CHMARKERFORMAT_DOWJ, EXC_CHMARKERFORMAT_STDDEV };
    return spnSymbols[ nFormatIdx % STATIC_TABLE_SIZE( spnSymbols ) ];
}

bool XclChartHelper::HasMarkerFillColor( sal_uInt16 nMarkerType )
{
    static const bool spbFilled[] = {
        false, true, true, true, false, false, false, false, true, false };
    return (nMarkerType < STATIC_TABLE_SIZE( spbFilled )) && spbFilled[ nMarkerType ];
}

// Chart formatting info provider =============================================

namespace {

static const XclChFormatInfo spFmtInfos[] =
{
    // object type                  property mode                auto line color         auto line weight         auto pattern color      missing frame type         create delete isframe
    { EXC_CHOBJTYPE_BACKGROUND,     EXC_CHPROPMODE_COMMON,       EXC_COLOR_CHWINDOWTEXT, EXC_CHLINEFORMAT_HAIR,   EXC_COLOR_CHWINDOWBACK, EXC_CHFRAMETYPE_INVISIBLE, true,  true,  true  },
    { EXC_CHOBJTYPE_PLOTFRAME,      EXC_CHPROPMODE_COMMON,       EXC_COLOR_CHWINDOWTEXT, EXC_CHLINEFORMAT_HAIR,   EXC_COLOR_CHWINDOWBACK, EXC_CHFRAMETYPE_INVISIBLE, true,  true,  true  },
    { EXC_CHOBJTYPE_WALL3D,         EXC_CHPROPMODE_COMMON,       EXC_COLOR_CHWINDOWTEXT, EXC_CHLINEFORMAT_HAIR,   EXC_COLOR_CHWINDOWBACK, EXC_CHFRAMETYPE_AUTO,      true,  false, true  },
    { EXC_CHOBJTYPE_FLOOR3D,        EXC_CHPROPMODE_COMMON,       EXC_COLOR_CHWINDOWTEXT, EXC_CHLINEFORMAT_HAIR,   23,                     EXC_CHFRAMETYPE_AUTO,      true,  false, true  },
    { EXC_CHOBJTYPE_TEXT,           EXC_CHPROPMODE_COMMON,       EXC_COLOR_CHWINDOWTEXT, EXC_CHLINEFORMAT_HAIR,   EXC_COLOR_CHWINDOWBACK, EXC_CHFRAMETYPE_INVISIBLE, false, true,  true  },
    { EXC_CHOBJTYPE_LEGEND,         EXC_CHPROPMODE_COMMON,       EXC_COLOR_CHWINDOWTEXT, EXC_CHLINEFORMAT_HAIR,   EXC_COLOR_CHWINDOWBACK, EXC_CHFRAMETYPE_AUTO,      true,  true,  true  },
    { EXC_CHOBJTYPE_LINEARSERIES,   EXC_CHPROPMODE_LINEARSERIES, EXC_COLOR_CHWINDOWTEXT, EXC_CHLINEFORMAT_SINGLE, EXC_COLOR_CHWINDOWBACK, EXC_CHFRAMETYPE_AUTO,      false, false, false },
    { EXC_CHOBJTYPE_FILLEDSERIES,   EXC_CHPROPMODE_FILLEDSERIES, EXC_COLOR_CHBORDERAUTO, EXC_CHLINEFORMAT_SINGLE, EXC_COLOR_CHWINDOWBACK, EXC_CHFRAMETYPE_AUTO,      false, false, true  },
    { EXC_CHOBJTYPE_AXISLINE,       EXC_CHPROPMODE_COMMON,       EXC_COLOR_CHWINDOWTEXT, EXC_CHLINEFORMAT_HAIR,   EXC_COLOR_CHWINDOWBACK, EXC_CHFRAMETYPE_AUTO,      false, false, false },
    { EXC_CHOBJTYPE_GRIDLINE,       EXC_CHPROPMODE_COMMON,       EXC_COLOR_CHWINDOWTEXT, EXC_CHLINEFORMAT_HAIR,   EXC_COLOR_CHWINDOWBACK, EXC_CHFRAMETYPE_INVISIBLE, false, true,  false  },
    { EXC_CHOBJTYPE_TRENDLINE,      EXC_CHPROPMODE_COMMON,       EXC_COLOR_CHWINDOWTEXT, EXC_CHLINEFORMAT_DOUBLE, EXC_COLOR_CHWINDOWBACK, EXC_CHFRAMETYPE_INVISIBLE, false, false, false },
    { EXC_CHOBJTYPE_ERRORBAR,       EXC_CHPROPMODE_COMMON,       EXC_COLOR_CHWINDOWTEXT, EXC_CHLINEFORMAT_SINGLE, EXC_COLOR_CHWINDOWBACK, EXC_CHFRAMETYPE_INVISIBLE, false, false, false },
    { EXC_CHOBJTYPE_CONNECTLINE,    EXC_CHPROPMODE_COMMON,       EXC_COLOR_CHWINDOWTEXT, EXC_CHLINEFORMAT_HAIR,   EXC_COLOR_CHWINDOWBACK, EXC_CHFRAMETYPE_INVISIBLE, false, false, false },
    { EXC_CHOBJTYPE_HILOLINE,       EXC_CHPROPMODE_LINEARSERIES, EXC_COLOR_CHWINDOWTEXT, EXC_CHLINEFORMAT_HAIR,   EXC_COLOR_CHWINDOWBACK, EXC_CHFRAMETYPE_INVISIBLE, false, false, false },
    { EXC_CHOBJTYPE_WHITEDROPBAR,   EXC_CHPROPMODE_COMMON,       EXC_COLOR_CHWINDOWTEXT, EXC_CHLINEFORMAT_HAIR,   EXC_COLOR_CHWINDOWBACK, EXC_CHFRAMETYPE_INVISIBLE, false, false, true  },
    { EXC_CHOBJTYPE_BLACKDROPBAR,   EXC_CHPROPMODE_COMMON,       EXC_COLOR_CHWINDOWTEXT, EXC_CHLINEFORMAT_HAIR,   EXC_COLOR_CHWINDOWTEXT, EXC_CHFRAMETYPE_INVISIBLE, false, false, true  }
};

}

// ----------------------------------------------------------------------------

XclChFormatInfoProvider::XclChFormatInfoProvider()
{
    const XclChFormatInfo* pEnd = STATIC_TABLE_END( spFmtInfos );
    for( const XclChFormatInfo* pIt = spFmtInfos; pIt != pEnd; ++pIt )
        maInfoMap[ pIt->meObjType ] = pIt;
}

const XclChFormatInfo& XclChFormatInfoProvider::GetFormatInfo( XclChObjectType eObjType ) const
{
    XclFmtInfoMap::const_iterator aIt = maInfoMap.find( eObjType );
    DBG_ASSERT( aIt != maInfoMap.end(), "XclChFormatInfoProvider::GetFormatInfo - unknown object type" );
    return (aIt == maInfoMap.end()) ? *spFmtInfos : *aIt->second;
}

// Chart type info provider ===================================================

namespace {

// chart type service names
const sal_Char SERVICE_CHART2_AREA[]    = "com.sun.star.chart2.AreaChartType";
const sal_Char SERVICE_CHART2_CANDLE[]  = "com.sun.star.chart2.CandleStickChartType";
const sal_Char SERVICE_CHART2_COLUMN[]  = "com.sun.star.chart2.ColumnChartType";
const sal_Char SERVICE_CHART2_LINE[]    = "com.sun.star.chart2.LineChartType";
const sal_Char SERVICE_CHART2_NET[]     = "com.sun.star.chart2.NetChartType";
const sal_Char SERVICE_CHART2_PIE[]     = "com.sun.star.chart2.PieChartType";
const sal_Char SERVICE_CHART2_SCATTER[] = "com.sun.star.chart2.ScatterChartType";
const sal_Char SERVICE_CHART2_SURFACE[] = "com.sun.star.chart2.ColumnChartType";    // Todo

namespace csscd = ::com::sun::star::chart::DataLabelPlacement;

static const XclChTypeInfo spTypeInfos[] =
{
    // chart type             chart type category      record id           service                 varied point color     def label combi       3d     3dwall polar  area2d area3d 1stvis xcateg swap   stack  revers betw
    { EXC_CHTYPEID_BAR,       EXC_CHTYPECATEG_BAR,     EXC_ID_CHBAR,       SERVICE_CHART2_COLUMN,  EXC_CHVARPOINT_SINGLE, csscd::OUTSIDE,       true,  true,  true,  false, true,  true,  false, true,  false, true,  false, true  },
    { EXC_CHTYPEID_HORBAR,    EXC_CHTYPECATEG_BAR,     EXC_ID_CHBAR,       SERVICE_CHART2_COLUMN,  EXC_CHVARPOINT_SINGLE, csscd::OUTSIDE,       false, true,  true,  false, true,  true,  false, true,  true,  true,  false, true  },
    { EXC_CHTYPEID_LINE,      EXC_CHTYPECATEG_LINE,    EXC_ID_CHLINE,      SERVICE_CHART2_LINE,    EXC_CHVARPOINT_SINGLE, csscd::RIGHT,         true,  true,  true,  false, false, true,  false, true,  false, true,  false, true  },
    { EXC_CHTYPEID_AREA,      EXC_CHTYPECATEG_LINE,    EXC_ID_CHAREA,      SERVICE_CHART2_AREA,    EXC_CHVARPOINT_NONE,   csscd::CENTER,        true,  true,  true,  false, true,  true,  false, true,  false, true,  true,  false },
    { EXC_CHTYPEID_STOCK,     EXC_CHTYPECATEG_LINE,    EXC_ID_CHLINE,      SERVICE_CHART2_CANDLE,  EXC_CHVARPOINT_NONE,   csscd::RIGHT,         true,  false, false, false, false, false, false, true,  false, true,  false, true  },
    { EXC_CHTYPEID_RADARLINE, EXC_CHTYPECATEG_RADAR,   EXC_ID_CHRADARLINE, SERVICE_CHART2_NET,     EXC_CHVARPOINT_SINGLE, csscd::TOP,           false, false, false, true,  false, true,  false, true,  false, false, false, false },
    { EXC_CHTYPEID_RADARAREA, EXC_CHTYPECATEG_RADAR,   EXC_ID_CHRADARAREA, SERVICE_CHART2_NET,     EXC_CHVARPOINT_NONE,   csscd::TOP,           false, false, false, true,  true,  true,  false, true,  false, false, false, false },
    { EXC_CHTYPEID_PIE,       EXC_CHTYPECATEG_PIE,     EXC_ID_CHPIE,       SERVICE_CHART2_PIE,     EXC_CHVARPOINT_MULTI,  csscd::AVOID_OVERLAP, false, true,  false, true,  true,  true,  true,  true,  false, false, false, false },
    { EXC_CHTYPEID_DONUT,     EXC_CHTYPECATEG_PIE,     EXC_ID_CHPIE,       SERVICE_CHART2_PIE,     EXC_CHVARPOINT_MULTI,  csscd::AVOID_OVERLAP, false, true,  false, true,  true,  true,  false, true,  false, false, true,  false },
    { EXC_CHTYPEID_PIEEXT,    EXC_CHTYPECATEG_PIE,     EXC_ID_CHPIEEXT,    SERVICE_CHART2_PIE,     EXC_CHVARPOINT_MULTI,  csscd::AVOID_OVERLAP, false, false, false, true,  true,  true,  true,  true,  false, false, false, false },
    { EXC_CHTYPEID_SCATTER,   EXC_CHTYPECATEG_SCATTER, EXC_ID_CHSCATTER,   SERVICE_CHART2_SCATTER, EXC_CHVARPOINT_SINGLE, csscd::RIGHT,         true,  false, false, false, false, true,  false, false, false, false, false, false },
    { EXC_CHTYPEID_BUBBLES,   EXC_CHTYPECATEG_SCATTER, EXC_ID_CHSCATTER,   SERVICE_CHART2_SCATTER, EXC_CHVARPOINT_SINGLE, csscd::RIGHT,         false, false, false, false, true,  true,  false, false, false, false, false, false },
    { EXC_CHTYPEID_SURFACE,   EXC_CHTYPECATEG_SURFACE, EXC_ID_CHSURFACE,   SERVICE_CHART2_SURFACE, EXC_CHVARPOINT_NONE,   csscd::RIGHT,         false, true,  true,  false, true,  true,  false, true,  false, false, false, false },
    { EXC_CHTYPEID_UNKNOWN,   EXC_CHTYPECATEG_BAR,     EXC_ID_CHBAR,       SERVICE_CHART2_COLUMN,  EXC_CHVARPOINT_SINGLE, csscd::OUTSIDE,       true,  true,  true,  false, true,  true,  false, true,  false, true,  false, true  }
};

} // namespace

XclChExtTypeInfo::XclChExtTypeInfo( const XclChTypeInfo& rTypeInfo ) :
    XclChTypeInfo( rTypeInfo ),
    mb3dChart( false ),
    mbSpline( false )
{
}

void XclChExtTypeInfo::Set( const XclChTypeInfo& rTypeInfo, bool b3dChart, bool bSpline )
{
    static_cast< XclChTypeInfo& >( *this ) = rTypeInfo;
    mb3dChart = mbSupports3d && b3dChart;
    mbSpline = bSpline;
}

// ----------------------------------------------------------------------------

XclChTypeInfoProvider::XclChTypeInfoProvider()
{
    const XclChTypeInfo* pEnd = STATIC_TABLE_END( spTypeInfos );
    for( const XclChTypeInfo* pIt = spTypeInfos; pIt != pEnd; ++pIt )
        maInfoMap[ pIt->meTypeId ] = pIt;
}

const XclChTypeInfo& XclChTypeInfoProvider::GetTypeInfo( XclChTypeId eTypeId ) const
{
    XclChTypeInfoMap::const_iterator aIt = maInfoMap.find( eTypeId );
    DBG_ASSERT( aIt != maInfoMap.end(), "XclChTypeInfoProvider::GetTypeInfo - unknown chart type" );
    return (aIt == maInfoMap.end()) ? *maInfoMap.rbegin()->second : *aIt->second;
}

const XclChTypeInfo& XclChTypeInfoProvider::GetTypeInfoFromRecId( sal_uInt16 nRecId ) const
{
    const XclChTypeInfo* pEnd = STATIC_TABLE_END( spTypeInfos );
    for( const XclChTypeInfo* pIt = spTypeInfos; pIt != pEnd; ++pIt )
        if( pIt->mnRecId == nRecId )
            return *pIt;
    DBG_ERRORFILE( "XclChTypeInfoProvider::GetTypeInfoFromRecId - unknown record id" );
    return GetTypeInfo( EXC_CHTYPEID_UNKNOWN );
}

const XclChTypeInfo& XclChTypeInfoProvider::GetTypeInfoFromService( const OUString& rServiceName ) const
{
    const XclChTypeInfo* pEnd = STATIC_TABLE_END( spTypeInfos );
    for( const XclChTypeInfo* pIt = spTypeInfos; pIt != pEnd; ++pIt )
        if( rServiceName.equalsAscii( pIt->mpcServiceName ) )
            return *pIt;
    DBG_ERRORFILE( "XclChTypeInfoProvider::GetTypeInfoFromService - unknown service name" );
    return GetTypeInfo( EXC_CHTYPEID_UNKNOWN );
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

Any XclChObjectTable::GetObject( const OUString& rObjName )
{
    // get object table
    if( !mxContainer.is() )
        mxContainer.set( ScfApiHelper::CreateInstance( mxFactory, maServiceName ), UNO_QUERY );
    DBG_ASSERT( mxContainer.is(), "XclChObjectTable::GetObject - container not found" );

    Any aObj;
    if( mxContainer.is() )
    {
        // get object from container
        try
        {
            aObj = mxContainer->getByName( rObjName );
        }
        catch( Exception& )
        {
            DBG_ERRORFILE( "XclChObjectTable::GetObject - object not found" );
        }
    }
    return aObj;
}

OUString XclChObjectTable::InsertObject( const Any& rObj )
{

    // create object table
    if( !mxContainer.is() )
        mxContainer.set( ScfApiHelper::CreateInstance( mxFactory, maServiceName ), UNO_QUERY );
    DBG_ASSERT( mxContainer.is(), "XclChObjectTable::InsertObject - container not found" );

    OUString aObjName;
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
        }
        catch( Exception& )
        {
            DBG_ERRORFILE( "XclChObjectTable::InsertObject - cannot insert object" );
            aObjName = OUString();
        }
    }
    return aObjName;
}

// Property names -------------------------------------------------------------

namespace {

/** Property names for line style in common objects. */
const sal_Char* const sppcLineNamesCommon[] =
    { "LineStyle", "LineWidth", "LineColor", "LineTransparence", "LineDashName", 0 };
/** Property names for line style in linear series objects. */
const sal_Char* const sppcLineNamesLinear[] =
    { "LineStyle", "LineWidth", "Color", "Transparency", "LineDashName", 0 };
/** Property names for line style in filled series objects. */
const sal_Char* const sppcLineNamesFilled[] =
    { "BorderStyle", "BorderWidth", "BorderColor", "BorderTransparency", "BorderDashName", 0 };

/** Property names for solid area style in common objects. */
const sal_Char* const sppcAreaNamesCommon[] = { "FillStyle", "FillColor", "FillTransparence", 0 };
/** Property names for solid area style in filled series objects. */
const sal_Char* const sppcAreaNamesFilled[] = { "FillStyle", "Color", "Transparency", 0 };
/** Property names for gradient area style in common objects. */
const sal_Char* const sppcGradNamesCommon[] = {  "FillStyle", "FillGradientName", 0 };
/** Property names for gradient area style in filled series objects. */
const sal_Char* const sppcGradNamesFilled[] = {  "FillStyle", "GradientName", 0 };
/** Property names for bitmap area style. */
const sal_Char* const sppcBitmapNames[] = { "FillStyle", "FillBitmapName", "FillBitmapMode", 0 };

/** Property names for text rotation properties. */
const sal_Char* const sppcRotationNames[] = { "TextRotation", "StackCharacters", 0 };
/** Property names for legend properties. */
const sal_Char* const sppcLegendNames[] =
    { "Show", "AnchorPosition", "Expansion", "RelativePosition", 0 };

} // namespace

// ----------------------------------------------------------------------------

XclChPropSetHelper::XclChPropSetHelper() :
    maLineHlpCommon( sppcLineNamesCommon ),
    maLineHlpLinear( sppcLineNamesLinear ),
    maLineHlpFilled( sppcLineNamesFilled ),
    maAreaHlpCommon( sppcAreaNamesCommon ),
    maAreaHlpFilled( sppcAreaNamesFilled ),
    maGradHlpCommon( sppcGradNamesCommon ),
    maGradHlpFilled( sppcGradNamesFilled ),
    maBitmapHlp( sppcBitmapNames ),
    maRotationHlp( sppcRotationNames ),
    maLegendHlp( sppcLegendNames )
{
}

// read properties ------------------------------------------------------------

void XclChPropSetHelper::ReadLineProperties(
        XclChLineFormat& rLineFmt, XclChObjectTable& rDashTable,
        const ScfPropertySet& rPropSet, XclChPropertyMode ePropMode )
{
    namespace cssd = ::com::sun::star::drawing;

    // read properties from property set
    cssd::LineStyle eApiStyle = cssd::LineStyle_NONE;
    sal_Int32 nApiWidth = 0;
    sal_Int16 nApiTrans = 0;
    Any aDashNameAny;

    ScfPropSetHelper& rLineHlp = GetLineHelper( ePropMode );
    rLineHlp.ReadFromPropertySet( rPropSet );
    rLineHlp >> eApiStyle >> nApiWidth >> rLineFmt.maColor >> nApiTrans >> aDashNameAny;

    // clear automatic flag
    ::set_flag( rLineFmt.mnFlags, EXC_CHLINEFORMAT_AUTO, false );

    // line width
    if( nApiWidth <= 0 )        rLineFmt.mnWeight = EXC_CHLINEFORMAT_HAIR;
    else if( nApiWidth <= 35 )  rLineFmt.mnWeight = EXC_CHLINEFORMAT_SINGLE;
    else if( nApiWidth <= 70 )  rLineFmt.mnWeight = EXC_CHLINEFORMAT_DOUBLE;
    else                        rLineFmt.mnWeight = EXC_CHLINEFORMAT_TRIPLE;

    // line style
    switch( eApiStyle )
    {
        case cssd::LineStyle_NONE:
            rLineFmt.mnPattern = EXC_CHLINEFORMAT_NONE;
        break;
        case cssd::LineStyle_SOLID:
        {
            if( nApiTrans < 13 )        rLineFmt.mnPattern = EXC_CHLINEFORMAT_SOLID;
            else if( nApiTrans < 38 )   rLineFmt.mnPattern = EXC_CHLINEFORMAT_DARKTRANS;
            else if( nApiTrans < 63 )   rLineFmt.mnPattern = EXC_CHLINEFORMAT_MEDTRANS;
            else if( nApiTrans < 100 )  rLineFmt.mnPattern = EXC_CHLINEFORMAT_LIGHTTRANS;
            else                        rLineFmt.mnPattern = EXC_CHLINEFORMAT_NONE;
        }
        break;
        case cssd::LineStyle_DASH:
        {
            rLineFmt.mnPattern = EXC_CHLINEFORMAT_SOLID;
            OUString aDashName;
            cssd::LineDash aApiDash;
            if( (aDashNameAny >>= aDashName) && (rDashTable.GetObject( aDashName ) >>= aApiDash) )
            {
                // reorder dashes that are shorter than dots
                if( (aApiDash.Dashes == 0) || (aApiDash.DashLen < aApiDash.DotLen) )
                {
                    ::std::swap( aApiDash.Dashes, aApiDash.Dots );
                    ::std::swap( aApiDash.DashLen, aApiDash.DotLen );
                }
                // ignore dots that are nearly equal to dashes
                if( aApiDash.DotLen * 3 > aApiDash.DashLen * 2 )
                    aApiDash.Dots = 0;

                // convert line dash to predefined Excel dash types
                if( (aApiDash.Dashes == 1) && (aApiDash.Dots >= 1) )
                    // one dash and one or more dots
                    rLineFmt.mnPattern = (aApiDash.Dots == 1) ?
                        EXC_CHLINEFORMAT_DASHDOT : EXC_CHLINEFORMAT_DASHDOTDOT;
                else if( aApiDash.Dashes >= 1 )
                    // one or more dashes and no dots (also: dash-dash-dot)
                    rLineFmt.mnPattern = (aApiDash.DashLen < 250) ?
                        EXC_CHLINEFORMAT_DOT : EXC_CHLINEFORMAT_DASH;
            }
        }
        break;
        default:
            DBG_ERRORFILE( "XclChPropSetHelper::ReadLineProperties - unknown line style" );
            rLineFmt.mnPattern = EXC_CHLINEFORMAT_SOLID;
    }
}

bool XclChPropSetHelper::ReadAreaProperties( XclChAreaFormat& rAreaFmt,
        const ScfPropertySet& rPropSet, XclChPropertyMode ePropMode )
{
    namespace cssd = ::com::sun::star::drawing;

    // read properties from property set
    cssd::FillStyle eApiStyle = cssd::FillStyle_NONE;
    sal_Int16 nTransparency = 0;

    ScfPropSetHelper& rAreaHlp = GetAreaHelper( ePropMode );
    rAreaHlp.ReadFromPropertySet( rPropSet );
    rAreaHlp >> eApiStyle >> rAreaFmt.maPattColor >> nTransparency;

    // clear automatic flag
    ::set_flag( rAreaFmt.mnFlags, EXC_CHAREAFORMAT_AUTO, false );

    // set fill style transparent or solid (set solid for anything but transparent)
    rAreaFmt.mnPattern = (eApiStyle == cssd::FillStyle_NONE) ? EXC_PATT_NONE : EXC_PATT_SOLID;

    // return true to indicate complex fill (gradient, bitmap, solid transparency)
    return (eApiStyle != cssd::FillStyle_NONE) && ((eApiStyle != cssd::FillStyle_SOLID) || (nTransparency > 0));
}

void XclChPropSetHelper::ReadEscherProperties(
        XclChEscherFormat& rEscherFmt, XclChPicFormat& rPicFmt,
        XclChObjectTable& rGradientTable, XclChObjectTable& rBitmapTable,
        const ScfPropertySet& rPropSet, XclChPropertyMode ePropMode )
{
    namespace cssd = ::com::sun::star::drawing;
    namespace cssa = ::com::sun::star::awt;

    // read style and transparency properties from property set
    cssd::FillStyle eApiStyle = cssd::FillStyle_NONE;
    Color aColor;
    sal_Int16 nTransparency = 0;

    ScfPropSetHelper& rAreaHlp = GetAreaHelper( ePropMode );
    rAreaHlp.ReadFromPropertySet( rPropSet );
    rAreaHlp >> eApiStyle >> aColor >> nTransparency;

    switch( eApiStyle )
    {
        case cssd::FillStyle_SOLID:
        {
            DBG_ASSERT( nTransparency > 0, "XclChPropSetHelper::ReadEscherProperties - unexpected solid area without transparency" );
            if( (0 < nTransparency) && (nTransparency <= 100) )
            {
                // convert to Escher properties
                sal_uInt32 nEscherColor = 0x02000000;
                ::insert_value( nEscherColor, aColor.GetBlue(), 16, 8 );
                ::insert_value( nEscherColor, aColor.GetGreen(), 8, 8 );
                ::insert_value( nEscherColor, aColor.GetRed(), 0, 8 );
                sal_uInt32 nEscherOpacity = static_cast< sal_uInt32 >( (100 - nTransparency) * 655.36 );
                rEscherFmt.mxEscherSet.reset( new EscherPropertyContainer );
                rEscherFmt.mxEscherSet->AddOpt( ESCHER_Prop_fillType, ESCHER_FillSolid );
                rEscherFmt.mxEscherSet->AddOpt( ESCHER_Prop_fillColor, nEscherColor );
                rEscherFmt.mxEscherSet->AddOpt( ESCHER_Prop_fillOpacity, nEscherOpacity );
                rEscherFmt.mxEscherSet->AddOpt( ESCHER_Prop_fillBackColor, 0x02FFFFFF );
                rEscherFmt.mxEscherSet->AddOpt( ESCHER_Prop_fillBackOpacity, 0x00010000 );
                rEscherFmt.mxEscherSet->AddOpt( ESCHER_Prop_fNoFillHitTest, 0x001F001C );
            }
        }
        break;
        case cssd::FillStyle_GRADIENT:
        {
            // extract gradient from global gradient table
            OUString aGradientName;
            ScfPropSetHelper& rGradHlp = GetGradientHelper( ePropMode );
            rGradHlp.ReadFromPropertySet( rPropSet );
            rGradHlp >> eApiStyle >> aGradientName;
            cssa::Gradient aGradient;
            if( rGradientTable.GetObject( aGradientName ) >>= aGradient )
            {
                // convert to Escher properties
                rEscherFmt.mxEscherSet.reset( new EscherPropertyContainer );
                rEscherFmt.mxEscherSet->CreateGradientProperties( aGradient );
            }
        }
        break;
        case cssd::FillStyle_HATCH:
        case cssd::FillStyle_BITMAP:
        {
            // extract bitmap URL from global bitmap table
            OUString aBitmapName;
            cssd::BitmapMode eApiBmpMode;
            maBitmapHlp.ReadFromPropertySet( rPropSet );
            maBitmapHlp >> eApiStyle >> aBitmapName >> eApiBmpMode;
            OUString aBitmapUrl;
            if( rBitmapTable.GetObject( aBitmapName ) >>= aBitmapUrl )
            {
                // convert to Escher properties
                rEscherFmt.mxEscherSet.reset( new EscherPropertyContainer );
                rEscherFmt.mxEscherSet->CreateEmbeddedBitmapProperties( aBitmapUrl, eApiBmpMode );
                rPicFmt.mnBmpMode = (eApiBmpMode == cssd::BitmapMode_REPEAT) ?
                    EXC_CHPICFORMAT_STACK : EXC_CHPICFORMAT_STRETCH;
            }
        }
        break;
        default:
            DBG_ERRORFILE( "XclChPropSetHelper::ReadEscherProperties - unknown fill style" );
    }
}

void XclChPropSetHelper::ReadMarkerProperties(
        XclChMarkerFormat& rMarkerFmt, const ScfPropertySet& rPropSet, sal_uInt16 nFormatIdx )
{
    namespace cssc = ::com::sun::star::chart2;
    namespace cssa = ::com::sun::star::awt;
    cssc::Symbol aApiSymbol;
    if( rPropSet.GetProperty( aApiSymbol, EXC_CHPROP_SYMBOL ) )
    {
        // clear automatic flag
        ::set_flag( rMarkerFmt.mnFlags, EXC_CHMARKERFORMAT_AUTO, false );

        // symbol style
        switch( aApiSymbol.Style )
        {
            case cssc::SymbolStyle_NONE:
                rMarkerFmt.mnMarkerType = EXC_CHMARKERFORMAT_NOSYMBOL;
            break;
            case cssc::SymbolStyle_STANDARD:
                switch( aApiSymbol.StandardSymbol )
                {
                    case 0:     rMarkerFmt.mnMarkerType = EXC_CHMARKERFORMAT_SQUARE;    break;  // square
                    case 1:     rMarkerFmt.mnMarkerType = EXC_CHMARKERFORMAT_DIAMOND;   break;  // diamond
                    case 2:     rMarkerFmt.mnMarkerType = EXC_CHMARKERFORMAT_STDDEV;    break;  // arrow down
                    case 3:     rMarkerFmt.mnMarkerType = EXC_CHMARKERFORMAT_TRIANGLE;  break;  // arrow up
                    case 4:     rMarkerFmt.mnMarkerType = EXC_CHMARKERFORMAT_CIRCLE;    break;  // arrow right
                    case 5:     rMarkerFmt.mnMarkerType = EXC_CHMARKERFORMAT_PLUS;      break;  // arrow left
                    case 6:     rMarkerFmt.mnMarkerType = EXC_CHMARKERFORMAT_CROSS;     break;  // bow tie
                    case 7:     rMarkerFmt.mnMarkerType = EXC_CHMARKERFORMAT_STAR;      break;  // sand glass
                    default:    rMarkerFmt.mnMarkerType = XclChartHelper::GetAutoMarkerType( nFormatIdx );
                }
            break;
            default:
                rMarkerFmt.mnMarkerType = XclChartHelper::GetAutoMarkerType( nFormatIdx );
        }
        bool bHasFillColor = XclChartHelper::HasMarkerFillColor( rMarkerFmt.mnMarkerType );
        ::set_flag( rMarkerFmt.mnFlags, EXC_CHMARKERFORMAT_NOFILL, !bHasFillColor );

        // symbol size
        sal_Int32 nApiSize = (aApiSymbol.Size.Width + aApiSymbol.Size.Height + 1) / 2;
        rMarkerFmt.mnMarkerSize = XclTools::GetTwipsFromHmm( nApiSize );

        // symbol colors
        rMarkerFmt.maLineColor = ScfApiHelper::ConvertFromApiColor( aApiSymbol.BorderColor );
        rMarkerFmt.maFillColor = ScfApiHelper::ConvertFromApiColor( aApiSymbol.FillColor );
    }
}

sal_uInt16 XclChPropSetHelper::ReadRotationProperties( const ScfPropertySet& rPropSet )
{
    // chart2 handles rotation as double in the range [0,360)
    double fAngle;
    bool bStacked;
    maRotationHlp.ReadFromPropertySet( rPropSet );
    maRotationHlp >> fAngle >> bStacked;
    return bStacked ? EXC_ROT_STACKED :
        XclTools::GetXclRotation( static_cast< sal_Int32 >( fAngle * 100.0 + 0.5 ) );
}

void XclChPropSetHelper::ReadLegendProperties( XclChLegend& rLegend, const ScfPropertySet& rPropSet )
{
    namespace cssc = ::com::sun::star::chart2;
    namespace cssd = ::com::sun::star::drawing;

    // read the properties
    bool bShow;
    cssc::LegendPosition eApiPos;
    cssc::LegendExpansion eApiExpand;
    Any aRelPosAny;
    maLegendHlp.ReadFromPropertySet( rPropSet );
    maLegendHlp >> bShow >> eApiPos >> eApiExpand >> aRelPosAny;
    DBG_ASSERT( bShow, "XclChPropSetHelper::ReadLegendProperties - legend must be visible" );

    // legend position
    switch( eApiPos )
    {
        case cssc::LegendPosition_LINE_START:   rLegend.mnDockMode = EXC_CHLEGEND_LEFT;     break;
        case cssc::LegendPosition_LINE_END:     rLegend.mnDockMode = EXC_CHLEGEND_RIGHT;    break;
        case cssc::LegendPosition_PAGE_START:   rLegend.mnDockMode = EXC_CHLEGEND_TOP;      break;
        case cssc::LegendPosition_PAGE_END:     rLegend.mnDockMode = EXC_CHLEGEND_BOTTOM;   break;
        default:                                rLegend.mnDockMode = EXC_CHLEGEND_NOTDOCKED;
    }
    // legend expansion
    ::set_flag( rLegend.mnFlags, EXC_CHLEGEND_STACKED, eApiExpand != cssc::LegendExpansion_WIDE );
    // legend position
    if( rLegend.mnDockMode == EXC_CHLEGEND_NOTDOCKED )
    {
        cssc::RelativePosition aRelPos;
        if( aRelPosAny >>= aRelPos )
        {
            rLegend.maRect.mnX = limit_cast< sal_Int32 >( aRelPos.Primary * 4000.0, 0, 4000 );
            rLegend.maRect.mnY = limit_cast< sal_Int32 >( aRelPos.Secondary * 4000.0, 0, 4000 );
        }
        else
            rLegend.mnDockMode = EXC_CHLEGEND_LEFT;
    }
    ::set_flag( rLegend.mnFlags, EXC_CHLEGEND_DOCKED, rLegend.mnDockMode != EXC_CHLEGEND_NOTDOCKED );
}

// write properties -----------------------------------------------------------

void XclChPropSetHelper::WriteLineProperties(
        ScfPropertySet& rPropSet, XclChObjectTable& rDashTable,
        const XclChLineFormat& rLineFmt, XclChPropertyMode ePropMode )
{
    namespace cssd = ::com::sun::star::drawing;

    // line width
    sal_Int32 nApiWidth = 0;    // 0 is the width of a hair line
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

    // line color
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
    ScfPropSetHelper& rLineHlp = GetLineHelper( ePropMode );
    rLineHlp.InitializeWrite();
    rLineHlp << eApiStyle << nApiWidth << nApiColor << nApiTrans << aDashNameAny;
    rLineHlp.WriteToPropertySet( rPropSet );
}

void XclChPropSetHelper::WriteAreaProperties( ScfPropertySet& rPropSet,
        const XclChAreaFormat& rAreaFmt, XclChPropertyMode ePropMode )
{
    namespace cssd = ::com::sun::star::drawing;
    cssd::FillStyle eFillStyle = cssd::FillStyle_NONE;
    Color aColor;
    sal_Int16 nTransparency = 0;

    // fill color
    if( rAreaFmt.mnPattern != EXC_PATT_NONE )
    {
        eFillStyle = cssd::FillStyle_SOLID;
        aColor = XclTools::GetPatternColor( rAreaFmt.maPattColor, rAreaFmt.maBackColor, rAreaFmt.mnPattern );
    }

    // write the properties
    ScfPropSetHelper& rAreaHlp = GetAreaHelper( ePropMode );
    rAreaHlp.InitializeWrite();
    rAreaHlp << eFillStyle << aColor << nTransparency;
    rAreaHlp.WriteToPropertySet( rPropSet );
}

void XclChPropSetHelper::WriteEscherProperties( ScfPropertySet& rPropSet,
        XclChObjectTable& rGradientTable, XclChObjectTable& rBitmapTable,
        const XclChEscherFormat& rEscherFmt, const XclChPicFormat& rPicFmt,
        XclChPropertyMode ePropMode )
{
    if( rEscherFmt.mxItemSet.is() )
    {
        if( const XFillStyleItem* pStyleItem = static_cast< const XFillStyleItem* >( rEscherFmt.mxItemSet->GetItem( XATTR_FILLSTYLE, FALSE ) ) )
        {
            switch( pStyleItem->GetValue() )
            {
                case XFILL_SOLID:
                    // #i84812# Excel 2007 writes Escher properties for solid fill
                    if( const XFillColorItem* pColorItem = static_cast< const XFillColorItem* >( rEscherFmt.mxItemSet->GetItem( XATTR_FILLCOLOR, FALSE ) ) )
                    {
                        namespace cssd = ::com::sun::star::drawing;
                        // get solid transparence too
                        const XFillTransparenceItem* pTranspItem = static_cast< const XFillTransparenceItem* >( rEscherFmt.mxItemSet->GetItem( XATTR_FILLTRANSPARENCE, FALSE ) );
                        sal_uInt16 nTransp = pTranspItem ? pTranspItem->GetValue() : 0;
                        ScfPropSetHelper& rAreaHlp = GetAreaHelper( ePropMode );
                        rAreaHlp.InitializeWrite();
                        rAreaHlp << cssd::FillStyle_SOLID << pColorItem->GetColorValue() << static_cast< sal_Int16 >( nTransp );
                        rAreaHlp.WriteToPropertySet( rPropSet );
                    }
                break;
                case XFILL_GRADIENT:
                    if( const XFillGradientItem* pGradItem = static_cast< const XFillGradientItem* >( rEscherFmt.mxItemSet->GetItem( XATTR_FILLGRADIENT, FALSE ) ) )
                    {
                        Any aGradientAny;
                        if( pGradItem->QueryValue( aGradientAny, MID_FILLGRADIENT ) )
                        {
                            OUString aGradName = rGradientTable.InsertObject( aGradientAny );
                            if( aGradName.getLength() )
                            {
                                namespace cssd = ::com::sun::star::drawing;
                                ScfPropSetHelper& rGradHlp = GetGradientHelper( ePropMode );
                                rGradHlp.InitializeWrite();
                                rGradHlp << cssd::FillStyle_GRADIENT << aGradName;
                                rGradHlp.WriteToPropertySet( rPropSet );
                            }
                        }
                    }
                break;
                case XFILL_BITMAP:
                    if( const XFillBitmapItem* pBmpItem = static_cast< const XFillBitmapItem* >( rEscherFmt.mxItemSet->GetItem( XATTR_FILLBITMAP, FALSE ) ) )
                    {
                        Any aBitmapAny;
                        if( pBmpItem->QueryValue( aBitmapAny, MID_GRAFURL ) )
                        {
                            OUString aBmpName = rBitmapTable.InsertObject( aBitmapAny );
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
}

void XclChPropSetHelper::WriteMarkerProperties(
        ScfPropertySet& rPropSet, const XclChMarkerFormat& rMarkerFmt )
{
    namespace cssc = ::com::sun::star::chart2;
    namespace cssa = ::com::sun::star::awt;

    // symbol style
    cssc::Symbol aApiSymbol;
    aApiSymbol.Style = cssc::SymbolStyle_STANDARD;
    switch( rMarkerFmt.mnMarkerType )
    {
        case EXC_CHMARKERFORMAT_NOSYMBOL:   aApiSymbol.Style = cssc::SymbolStyle_NONE;  break;
        case EXC_CHMARKERFORMAT_SQUARE:     aApiSymbol.StandardSymbol = 0;              break;  // square
        case EXC_CHMARKERFORMAT_DIAMOND:    aApiSymbol.StandardSymbol = 1;              break;  // diamond
        case EXC_CHMARKERFORMAT_TRIANGLE:   aApiSymbol.StandardSymbol = 3;              break;  // arrow up
        case EXC_CHMARKERFORMAT_CROSS:      aApiSymbol.StandardSymbol = 6;              break;  // bow tie
        case EXC_CHMARKERFORMAT_STAR:       aApiSymbol.StandardSymbol = 7;              break;  // sand glass
        case EXC_CHMARKERFORMAT_DOWJ:       aApiSymbol.StandardSymbol = 4;              break;  // arrow right
        case EXC_CHMARKERFORMAT_STDDEV:     aApiSymbol.StandardSymbol = 2;              break;  // arrow down
        case EXC_CHMARKERFORMAT_CIRCLE:     aApiSymbol.StandardSymbol = 4;              break;  // arrow right
        case EXC_CHMARKERFORMAT_PLUS:       aApiSymbol.StandardSymbol = 5;              break;  // arrow left
    }

    // symbol size
    sal_Int32 nApiSize = XclTools::GetHmmFromTwips( rMarkerFmt.mnMarkerSize );
    aApiSymbol.Size = cssa::Size( nApiSize, nApiSize );

    // symbol colors
    aApiSymbol.FillColor = ScfApiHelper::ConvertToApiColor( rMarkerFmt.maFillColor );
    aApiSymbol.BorderColor = ::get_flag( rMarkerFmt.mnFlags, EXC_CHMARKERFORMAT_NOLINE ) ?
        aApiSymbol.FillColor : ScfApiHelper::ConvertToApiColor( rMarkerFmt.maLineColor );

    // set the property
    rPropSet.SetProperty( EXC_CHPROP_SYMBOL, aApiSymbol );
}

void XclChPropSetHelper::WriteRotationProperties(
        ScfPropertySet& rPropSet, sal_uInt16 nRotation )
{
    if( nRotation != EXC_CHART_AUTOROTATION )
    {
        // chart2 handles rotation as double in the range [0,360)
        double fAngle = XclTools::GetScRotation( nRotation, 0 ) / 100.0;
        bool bStacked = nRotation == EXC_ROT_STACKED;
        maRotationHlp.InitializeWrite();
        maRotationHlp << fAngle << bStacked;
        maRotationHlp.WriteToPropertySet( rPropSet );
    }
}

void XclChPropSetHelper::WriteLegendProperties(
        ScfPropertySet& rPropSet, const XclChLegend& rLegend )
{
    namespace cssc = ::com::sun::star::chart2;
    namespace cssd = ::com::sun::star::drawing;

    // legend position
    cssc::LegendPosition eApiPos = cssc::LegendPosition_CUSTOM;
    switch( rLegend.mnDockMode )
    {
        case EXC_CHLEGEND_LEFT:     eApiPos = cssc::LegendPosition_LINE_START;  break;
        case EXC_CHLEGEND_RIGHT:    eApiPos = cssc::LegendPosition_LINE_END;    break;
        case EXC_CHLEGEND_TOP:      eApiPos = cssc::LegendPosition_PAGE_START;  break;
        case EXC_CHLEGEND_BOTTOM:   eApiPos = cssc::LegendPosition_PAGE_END;    break;
    }
    // legend expansion
    cssc::LegendExpansion eApiExpand = ::get_flagvalue(
        rLegend.mnFlags, EXC_CHLEGEND_STACKED, cssc::LegendExpansion_HIGH, cssc::LegendExpansion_WIDE );
    // legend position
    Any aRelPosAny;
    if( eApiPos == cssc::LegendPosition_CUSTOM )
    {
        // #i71697# it is not possible to set the size directly, do some magic here
        double fRatio = ((rLegend.maRect.mnWidth > 0) && (rLegend.maRect.mnHeight > 0)) ?
            (static_cast< double >( rLegend.maRect.mnWidth ) / rLegend.maRect.mnHeight) : 1.0;
        if( fRatio > 1.5 )
            eApiExpand = cssc::LegendExpansion_WIDE;
        else if( fRatio < 0.75 )
            eApiExpand = cssc::LegendExpansion_HIGH;
        else
            eApiExpand = cssc::LegendExpansion_BALANCED;
        // set position
        cssc::RelativePosition aRelPos;
        aRelPos.Primary = rLegend.maRect.mnX / 4000.0;
        aRelPos.Secondary = rLegend.maRect.mnY / 4000.0;
        aRelPos.Anchor = cssd::Alignment_TOP_LEFT;
        aRelPosAny <<= aRelPos;
    }

    // write the properties
    maLegendHlp.InitializeWrite();
    maLegendHlp << true << eApiPos << eApiExpand << aRelPosAny;
    maLegendHlp.WriteToPropertySet( rPropSet );
}

// private --------------------------------------------------------------------

ScfPropSetHelper& XclChPropSetHelper::GetLineHelper( XclChPropertyMode ePropMode )
{
    switch( ePropMode )
    {
        case EXC_CHPROPMODE_COMMON:         return maLineHlpCommon;
        case EXC_CHPROPMODE_LINEARSERIES:   return maLineHlpLinear;
        case EXC_CHPROPMODE_FILLEDSERIES:   return maLineHlpFilled;
        default: DBG_ERRORFILE( "XclChPropSetHelper::GetLineHelper - unknown property mode" );
    }
    return maLineHlpCommon;
}

ScfPropSetHelper& XclChPropSetHelper::GetAreaHelper( XclChPropertyMode ePropMode )
{
    switch( ePropMode )
    {
        case EXC_CHPROPMODE_COMMON:         return maAreaHlpCommon;
        case EXC_CHPROPMODE_FILLEDSERIES:   return maAreaHlpFilled;
        default:    DBG_ERRORFILE( "XclChPropSetHelper::GetAreaHelper - unknown property mode" );
    }
    return maAreaHlpCommon;
}

ScfPropSetHelper& XclChPropSetHelper::GetGradientHelper( XclChPropertyMode ePropMode )
{
    switch( ePropMode )
    {
        case EXC_CHPROPMODE_COMMON:         return maGradHlpCommon;
        case EXC_CHPROPMODE_FILLEDSERIES:   return maGradHlpFilled;
        default:    DBG_ERRORFILE( "XclChPropSetHelper::GetGradientHelper - unknown property mode" );
    }
    return maGradHlpCommon;
}

// ============================================================================

XclChRootData::XclChRootData() :
    mxTypeInfoProv( new XclChTypeInfoProvider ),
    mxFmtInfoProv( new XclChFormatInfoProvider )
{
}

XclChRootData::~XclChRootData()
{
}

Reference< XChartDocument > XclChRootData::GetChartDoc() const
{
    DBG_ASSERT( mxChartDoc.is(), "XclChRootData::GetChartDoc - missing chart document" );
    return mxChartDoc;
}

void XclChRootData::InitConversion( XChartDocRef xChartDoc )
{
    // remember chart document reference
    DBG_ASSERT( xChartDoc.is(), "XclChRootData::InitConversion - missing chart document" );
    mxChartDoc = xChartDoc;

    // create object tables
    Reference< XMultiServiceFactory > xFactory( mxChartDoc, UNO_QUERY );
    mxLineDashTable.reset( new XclChObjectTable(
        xFactory, SERVICE_DRAWING_DASHTABLE, CREATE_OUSTRING( "Excel line dash " ) ) );
    mxGradientTable.reset( new XclChObjectTable(
        xFactory, SERVICE_DRAWING_GRADIENTTABLE, CREATE_OUSTRING( "Excel gradient " ) ) );
    mxBitmapTable.reset( new XclChObjectTable(
        xFactory, SERVICE_DRAWING_BITMAPTABLE, CREATE_OUSTRING( "Excel bitmap " ) ) );
}

void XclChRootData::FinishConversion()
{
    // forget formatting object tables
    mxBitmapTable.reset();
    mxGradientTable.reset();
    mxLineDashTable.reset();
    // forget chart document reference
    mxChartDoc.clear();
}

// ============================================================================

