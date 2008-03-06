/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartItemPool.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 17:55:58 $
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
#include "precompiled_chart2.hxx"

#include "ChartItemPool.hxx"
#include "macros.hxx"

#include "chartview/ChartSfxItemIds.hxx"
#ifndef _SVX_CHRTITEM_HXX
#include <svx/chrtitem.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_SIZEITEM_HXX
#include <svx/sizeitem.hxx>
#endif
// header for class SfxStringItem
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif
//SfxIntegerListItem
#include <svtools/ilstitem.hxx>
#define _SVSTDARR_ULONGS
#include <svtools/svstdarr.hxx>

// for Singleton GetChartItemPool() function
// ---------------------------------------
#ifndef INCLUDED_OSL_DOUBLECHECKEDLOCKING_H
#include "rtl/instance.hxx"
#endif
#ifndef INCLUDED_OSL_GETGLOBALMUTEX_HXX
#include "osl/getglobalmutex.hxx"
#endif

namespace {
struct ChartItemPoolInitialization
{
    ::chart::ChartItemPool * operator()()
    {
        static ::chart::ChartItemPool aInstance;
        return &aInstance;
    }
};
}
// ---------------------------------------

namespace chart
{

ChartItemPool * GetChartItemPool()
{
    return rtl_Instance< ChartItemPool, ChartItemPoolInitialization, ::osl::MutexGuard,
        ::osl::GetGlobalMutex >::create(
            ChartItemPoolInitialization(), ::osl::GetGlobalMutex());
}

ChartItemPool::ChartItemPool():
        SfxItemPool( String( RTL_CONSTASCII_USTRINGPARAM( "ChartItemPool" )), SCHATTR_START, SCHATTR_END, NULL, NULL )
{
//     OSL_TRACE( "SCH: CTOR: ChartItemPool" );
    /**************************************************************************
    * PoolDefaults
    **************************************************************************/
    ppPoolDefaults = new SfxPoolItem*[SCHATTR_END - SCHATTR_START + 1];

    ppPoolDefaults[SCHATTR_DATADESCR_SHOW_NUMBER    - SCHATTR_START] = new SfxBoolItem(SCHATTR_DATADESCR_SHOW_NUMBER);
    ppPoolDefaults[SCHATTR_DATADESCR_SHOW_PERCENTAGE- SCHATTR_START] = new SfxBoolItem(SCHATTR_DATADESCR_SHOW_PERCENTAGE);
    ppPoolDefaults[SCHATTR_DATADESCR_SHOW_CATEGORY  - SCHATTR_START] = new SfxBoolItem(SCHATTR_DATADESCR_SHOW_CATEGORY);
    ppPoolDefaults[SCHATTR_DATADESCR_SHOW_SYMBOL    - SCHATTR_START] = new SfxBoolItem(SCHATTR_DATADESCR_SHOW_SYMBOL);
    ppPoolDefaults[SCHATTR_DATADESCR_SEPARATOR      - SCHATTR_START] = new SfxStringItem(SCHATTR_DATADESCR_SEPARATOR,C2U(" "));
    ppPoolDefaults[SCHATTR_DATADESCR_PLACEMENT      - SCHATTR_START] = new SfxInt32Item(SCHATTR_DATADESCR_PLACEMENT,0);
    SvULongs aTmp;
    ppPoolDefaults[SCHATTR_DATADESCR_AVAILABLE_PLACEMENTS - SCHATTR_START] = new SfxIntegerListItem(SCHATTR_DATADESCR_AVAILABLE_PLACEMENTS,aTmp);

    ppPoolDefaults[SCHATTR_LEGEND_POS               - SCHATTR_START] = new SvxChartLegendPosItem( CHLEGEND_RIGHT, SCHATTR_LEGEND_POS );
//  ppPoolDefaults[SCHATTR_TEXT_ORIENT              - SCHATTR_START] = new SvxChartTextOrientItem;
    ppPoolDefaults[SCHATTR_TEXT_STACKED             - SCHATTR_START] = new SfxBoolItem(SCHATTR_TEXT_STACKED,FALSE);
    ppPoolDefaults[SCHATTR_TEXT_ORDER               - SCHATTR_START] = new SvxChartTextOrderItem(CHTXTORDER_SIDEBYSIDE, SCHATTR_TEXT_ORDER);

    ppPoolDefaults[SCHATTR_Y_AXIS_AUTO_MIN          - SCHATTR_START] = new SfxBoolItem(SCHATTR_Y_AXIS_AUTO_MIN);
    ppPoolDefaults[SCHATTR_Y_AXIS_MIN               - SCHATTR_START] = new SvxDoubleItem(0.0, SCHATTR_Y_AXIS_MIN);
    ppPoolDefaults[SCHATTR_Y_AXIS_AUTO_MAX          - SCHATTR_START] = new SfxBoolItem(SCHATTR_Y_AXIS_AUTO_MAX);
    ppPoolDefaults[SCHATTR_Y_AXIS_MAX               - SCHATTR_START] = new SvxDoubleItem(0.0, SCHATTR_Y_AXIS_MAX);
    ppPoolDefaults[SCHATTR_Y_AXIS_AUTO_STEP_MAIN    - SCHATTR_START] = new SfxBoolItem(SCHATTR_Y_AXIS_AUTO_STEP_MAIN);
    ppPoolDefaults[SCHATTR_Y_AXIS_STEP_MAIN         - SCHATTR_START] = new SvxDoubleItem(0.0, SCHATTR_Y_AXIS_STEP_MAIN);
    ppPoolDefaults[SCHATTR_Y_AXIS_AUTO_STEP_HELP    - SCHATTR_START] = new SfxBoolItem(SCHATTR_Y_AXIS_AUTO_STEP_HELP);
    ppPoolDefaults[SCHATTR_Y_AXIS_STEP_HELP         - SCHATTR_START] = new SvxDoubleItem(0.0, SCHATTR_Y_AXIS_STEP_HELP);
    ppPoolDefaults[SCHATTR_Y_AXIS_LOGARITHM         - SCHATTR_START] = new SfxBoolItem(SCHATTR_Y_AXIS_LOGARITHM);
    ppPoolDefaults[SCHATTR_Y_AXIS_AUTO_ORIGIN       - SCHATTR_START] = new SfxBoolItem(SCHATTR_Y_AXIS_AUTO_ORIGIN);
    ppPoolDefaults[SCHATTR_Y_AXIS_ORIGIN            - SCHATTR_START] = new SvxDoubleItem(0.0, SCHATTR_Y_AXIS_ORIGIN);

    ppPoolDefaults[SCHATTR_X_AXIS_AUTO_MIN          - SCHATTR_START] = new SfxBoolItem(SCHATTR_X_AXIS_AUTO_MIN);
    ppPoolDefaults[SCHATTR_X_AXIS_MIN               - SCHATTR_START] = new SvxDoubleItem(0.0, SCHATTR_X_AXIS_MIN);
    ppPoolDefaults[SCHATTR_X_AXIS_AUTO_MAX          - SCHATTR_START] = new SfxBoolItem(SCHATTR_X_AXIS_AUTO_MAX);
    ppPoolDefaults[SCHATTR_X_AXIS_MAX               - SCHATTR_START] = new SvxDoubleItem(0.0, SCHATTR_X_AXIS_MAX);
    ppPoolDefaults[SCHATTR_X_AXIS_AUTO_STEP_MAIN    - SCHATTR_START] = new SfxBoolItem(SCHATTR_X_AXIS_AUTO_STEP_MAIN);
    ppPoolDefaults[SCHATTR_X_AXIS_STEP_MAIN         - SCHATTR_START] = new SvxDoubleItem(0.0, SCHATTR_X_AXIS_STEP_MAIN);
    ppPoolDefaults[SCHATTR_X_AXIS_AUTO_STEP_HELP    - SCHATTR_START] = new SfxBoolItem(SCHATTR_X_AXIS_AUTO_STEP_HELP);
    ppPoolDefaults[SCHATTR_X_AXIS_STEP_HELP         - SCHATTR_START] = new SvxDoubleItem(0.0, SCHATTR_X_AXIS_STEP_HELP);
    ppPoolDefaults[SCHATTR_X_AXIS_LOGARITHM         - SCHATTR_START] = new SfxBoolItem(SCHATTR_X_AXIS_LOGARITHM);
    ppPoolDefaults[SCHATTR_X_AXIS_AUTO_ORIGIN       - SCHATTR_START] = new SfxBoolItem(SCHATTR_X_AXIS_AUTO_ORIGIN);
    ppPoolDefaults[SCHATTR_X_AXIS_ORIGIN            - SCHATTR_START] = new SvxDoubleItem(0.0, SCHATTR_X_AXIS_ORIGIN);

    ppPoolDefaults[SCHATTR_Z_AXIS_AUTO_MIN          - SCHATTR_START] = new SfxBoolItem(SCHATTR_Z_AXIS_AUTO_MIN);
    ppPoolDefaults[SCHATTR_Z_AXIS_MIN               - SCHATTR_START] = new SvxDoubleItem(0.0, SCHATTR_Z_AXIS_MIN);
    ppPoolDefaults[SCHATTR_Z_AXIS_AUTO_MAX          - SCHATTR_START] = new SfxBoolItem(SCHATTR_Z_AXIS_AUTO_MAX);
    ppPoolDefaults[SCHATTR_Z_AXIS_MAX               - SCHATTR_START] = new SvxDoubleItem(0.0, SCHATTR_Z_AXIS_MAX);
    ppPoolDefaults[SCHATTR_Z_AXIS_AUTO_STEP_MAIN    - SCHATTR_START] = new SfxBoolItem(SCHATTR_Z_AXIS_AUTO_STEP_MAIN);
    ppPoolDefaults[SCHATTR_Z_AXIS_STEP_MAIN         - SCHATTR_START] = new SvxDoubleItem(0.0, SCHATTR_Z_AXIS_STEP_MAIN);
    ppPoolDefaults[SCHATTR_Z_AXIS_AUTO_STEP_HELP    - SCHATTR_START] = new SfxBoolItem(SCHATTR_Z_AXIS_AUTO_STEP_HELP);
    ppPoolDefaults[SCHATTR_Z_AXIS_STEP_HELP         - SCHATTR_START] = new SvxDoubleItem(0.0, SCHATTR_Z_AXIS_STEP_HELP);
    ppPoolDefaults[SCHATTR_Z_AXIS_LOGARITHM         - SCHATTR_START] = new SfxBoolItem(SCHATTR_Z_AXIS_LOGARITHM);
    ppPoolDefaults[SCHATTR_Z_AXIS_AUTO_ORIGIN       - SCHATTR_START] = new SfxBoolItem(SCHATTR_Z_AXIS_AUTO_ORIGIN);
    ppPoolDefaults[SCHATTR_Z_AXIS_ORIGIN            - SCHATTR_START] = new SvxDoubleItem(0.0, SCHATTR_Z_AXIS_ORIGIN);

    ppPoolDefaults[SCHATTR_AXISTYPE                 - SCHATTR_START] = new SfxInt32Item(SCHATTR_AXISTYPE, CHART_AXIS_X);
    ppPoolDefaults[SCHATTR_PERCENT_NUMBERFORMAT_VALUE  - SCHATTR_START] = new SfxInt32Item(SCHATTR_PERCENT_NUMBERFORMAT_VALUE, 0);
    ppPoolDefaults[SCHATTR_PERCENT_NUMBERFORMAT_SOURCE - SCHATTR_START] = new SfxBoolItem(SCHATTR_PERCENT_NUMBERFORMAT_SOURCE);

    ppPoolDefaults[SCHATTR_STAT_AVERAGE             - SCHATTR_START] = new SfxBoolItem (SCHATTR_STAT_AVERAGE);
    ppPoolDefaults[SCHATTR_STAT_KIND_ERROR          - SCHATTR_START] = new SvxChartKindErrorItem (CHERROR_NONE, SCHATTR_STAT_KIND_ERROR);
    ppPoolDefaults[SCHATTR_STAT_PERCENT             - SCHATTR_START] = new SvxDoubleItem (0.0, SCHATTR_STAT_PERCENT);
    ppPoolDefaults[SCHATTR_STAT_BIGERROR            - SCHATTR_START] = new SvxDoubleItem (0.0, SCHATTR_STAT_BIGERROR);
    ppPoolDefaults[SCHATTR_STAT_CONSTPLUS           - SCHATTR_START] = new SvxDoubleItem (0.0, SCHATTR_STAT_CONSTPLUS);
    ppPoolDefaults[SCHATTR_STAT_CONSTMINUS          - SCHATTR_START] = new SvxDoubleItem (0.0, SCHATTR_STAT_CONSTMINUS);
    ppPoolDefaults[SCHATTR_STAT_INDICATE            - SCHATTR_START] = new SvxChartIndicateItem (CHINDICATE_NONE, SCHATTR_STAT_INDICATE);
    ppPoolDefaults[SCHATTR_STAT_RANGE_POS           - SCHATTR_START] = new SfxStringItem (SCHATTR_STAT_RANGE_POS, String());
    ppPoolDefaults[SCHATTR_STAT_RANGE_NEG           - SCHATTR_START] = new SfxStringItem (SCHATTR_STAT_RANGE_NEG, String());

    ppPoolDefaults[SCHATTR_TEXT_DEGREES             - SCHATTR_START] = new SfxInt32Item(SCHATTR_TEXT_DEGREES, 0);
    ppPoolDefaults[SCHATTR_TEXT_OVERLAP             - SCHATTR_START] = new SfxBoolItem(SCHATTR_TEXT_OVERLAP,FALSE);

    ppPoolDefaults[SCHATTR_STYLE_DEEP     - SCHATTR_START] = new SfxBoolItem (SCHATTR_STYLE_DEEP, 0);
    ppPoolDefaults[SCHATTR_STYLE_3D       - SCHATTR_START] = new SfxBoolItem (SCHATTR_STYLE_3D, 0);
    ppPoolDefaults[SCHATTR_STYLE_VERTICAL - SCHATTR_START] = new SfxBoolItem (SCHATTR_STYLE_VERTICAL, 0);
    ppPoolDefaults[SCHATTR_STYLE_BASETYPE - SCHATTR_START] = new SfxInt32Item(SCHATTR_STYLE_BASETYPE, 0);
    ppPoolDefaults[SCHATTR_STYLE_LINES    - SCHATTR_START] = new SfxBoolItem (SCHATTR_STYLE_LINES, 0);
    ppPoolDefaults[SCHATTR_STYLE_PERCENT  - SCHATTR_START] = new SfxBoolItem (SCHATTR_STYLE_PERCENT, 0);
    ppPoolDefaults[SCHATTR_STYLE_STACKED  - SCHATTR_START] = new SfxBoolItem (SCHATTR_STYLE_STACKED, 0);
    ppPoolDefaults[SCHATTR_STYLE_SPLINES  - SCHATTR_START] = new SfxInt32Item (SCHATTR_STYLE_SPLINES, 0); //Bug: war Bool! ->Fileformat testen (betrifft nur 5er)
    ppPoolDefaults[SCHATTR_STYLE_SYMBOL   - SCHATTR_START] = new SfxInt32Item (SCHATTR_STYLE_SYMBOL, 0);
    ppPoolDefaults[SCHATTR_STYLE_SHAPE    - SCHATTR_START] = new SfxInt32Item (SCHATTR_STYLE_SHAPE, 0);

    ppPoolDefaults[SCHATTR_AXIS                 - SCHATTR_START] = new SfxInt32Item(SCHATTR_AXIS,2); //2 = Y-Achse!!!

    ppPoolDefaults[SCHATTR_AXIS_AUTO_MIN        - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_AUTO_MIN);
    ppPoolDefaults[SCHATTR_AXIS_MIN             - SCHATTR_START] = new SvxDoubleItem(0.0, SCHATTR_AXIS_MIN);
    ppPoolDefaults[SCHATTR_AXIS_AUTO_MAX        - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_AUTO_MAX);
    ppPoolDefaults[SCHATTR_AXIS_MAX             - SCHATTR_START] = new SvxDoubleItem(0.0, SCHATTR_AXIS_MAX);
    ppPoolDefaults[SCHATTR_AXIS_AUTO_STEP_MAIN  - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_AUTO_STEP_MAIN);
    ppPoolDefaults[SCHATTR_AXIS_STEP_MAIN       - SCHATTR_START] = new SvxDoubleItem(0.0, SCHATTR_AXIS_STEP_MAIN);
    ppPoolDefaults[SCHATTR_AXIS_AUTO_STEP_HELP  - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_AUTO_STEP_HELP);
//  ppPoolDefaults[SCHATTR_AXIS_STEP_HELP       - SCHATTR_START] = new SvxDoubleItem(0.0, SCHATTR_AXIS_STEP_HELP);
    // type changed from double to sal_Int32
    ppPoolDefaults[SCHATTR_AXIS_STEP_HELP       - SCHATTR_START] = new SfxInt32Item(SCHATTR_AXIS_STEP_HELP,0);
    ppPoolDefaults[SCHATTR_AXIS_LOGARITHM       - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_LOGARITHM);
    ppPoolDefaults[SCHATTR_AXIS_AUTO_ORIGIN     - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_AUTO_ORIGIN);
    ppPoolDefaults[SCHATTR_AXIS_ORIGIN          - SCHATTR_START] = new SvxDoubleItem(0.0, SCHATTR_AXIS_ORIGIN);

    ppPoolDefaults[SCHATTR_AXIS_TICKS           - SCHATTR_START] = new SfxInt32Item(SCHATTR_AXIS_TICKS,CHAXIS_MARK_OUTER);
    ppPoolDefaults[SCHATTR_AXIS_NUMFMT          - SCHATTR_START] = new SfxUInt32Item(SCHATTR_AXIS_NUMFMT,0);
    ppPoolDefaults[SCHATTR_AXIS_NUMFMTPERCENT   - SCHATTR_START] = new SfxUInt32Item(SCHATTR_AXIS_NUMFMTPERCENT,11);
    ppPoolDefaults[SCHATTR_AXIS_SHOWAXIS        - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_SHOWAXIS,0);
    ppPoolDefaults[SCHATTR_AXIS_SHOWDESCR       - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_SHOWDESCR,0);
    ppPoolDefaults[SCHATTR_AXIS_SHOWMAINGRID    - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_SHOWMAINGRID,0);
    ppPoolDefaults[SCHATTR_AXIS_SHOWHELPGRID    - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_SHOWHELPGRID,0);
    ppPoolDefaults[SCHATTR_AXIS_TOPDOWN         - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_TOPDOWN,0);
    ppPoolDefaults[SCHATTR_AXIS_HELPTICKS       - SCHATTR_START] = new SfxInt32Item(SCHATTR_AXIS_HELPTICKS,0);
    ppPoolDefaults[SCHATTR_AXIS_REVERSE         - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_REVERSE,0);

    ppPoolDefaults[SCHATTR_SYMBOL_BRUSH         - SCHATTR_START] = new SvxBrushItem(SCHATTR_SYMBOL_BRUSH);
    ppPoolDefaults[SCHATTR_STOCK_VOLUME         - SCHATTR_START] = new SfxBoolItem(SCHATTR_STOCK_VOLUME,0);
    ppPoolDefaults[SCHATTR_STOCK_UPDOWN         - SCHATTR_START] = new SfxBoolItem(SCHATTR_STOCK_UPDOWN,0);
    ppPoolDefaults[SCHATTR_SYMBOL_SIZE          - SCHATTR_START] = new SvxSizeItem(SCHATTR_SYMBOL_SIZE,Size(0,0));

    // new for New Chart
    ppPoolDefaults[SCHATTR_BAR_OVERLAP          - SCHATTR_START] = new SfxInt32Item(SCHATTR_BAR_OVERLAP,0);
    ppPoolDefaults[SCHATTR_BAR_GAPWIDTH         - SCHATTR_START] = new SfxInt32Item(SCHATTR_BAR_GAPWIDTH,0);
    ppPoolDefaults[SCHATTR_BAR_CONNECT          - SCHATTR_START] = new SfxBoolItem(SCHATTR_BAR_CONNECT, FALSE);
    ppPoolDefaults[SCHATTR_NUM_OF_LINES_FOR_BAR - SCHATTR_START] = new SfxInt32Item( SCHATTR_NUM_OF_LINES_FOR_BAR, 0 );
    ppPoolDefaults[SCHATTR_SPLINE_ORDER         - SCHATTR_START] = new SfxInt32Item( SCHATTR_SPLINE_ORDER, 3 );
    ppPoolDefaults[SCHATTR_SPLINE_RESOLUTION    - SCHATTR_START] = new SfxInt32Item( SCHATTR_SPLINE_RESOLUTION, 20 );
    ppPoolDefaults[SCHATTR_DIAGRAM_STYLE        - SCHATTR_START] = new SvxChartStyleItem( CHSTYLE_2D_COLUMN, SCHATTR_DIAGRAM_STYLE );
    ppPoolDefaults[SCHATTR_TEXTBREAK            - SCHATTR_START] = new SfxBoolItem( SCHATTR_TEXTBREAK, FALSE );
    ppPoolDefaults[SCHATTR_GROUP_BARS_PER_AXIS  - SCHATTR_START] = new SfxBoolItem(SCHATTR_GROUP_BARS_PER_AXIS, FALSE);
    ppPoolDefaults[SCHATTR_STARTING_ANGLE       - SCHATTR_START] = new SfxInt32Item( SCHATTR_STARTING_ANGLE, 90 );
    ppPoolDefaults[SCHATTR_CLOCKWISE            - SCHATTR_START] = new SfxBoolItem( SCHATTR_CLOCKWISE, FALSE );

    ppPoolDefaults[SCHATTR_AXIS_FOR_ALL_SERIES  - SCHATTR_START] = new SfxInt32Item(SCHATTR_AXIS_FOR_ALL_SERIES, 0);
    ppPoolDefaults[SCHATTR_REGRESSION_TYPE          - SCHATTR_START] = new SvxChartRegressItem  (CHREGRESS_NONE, SCHATTR_REGRESSION_TYPE);
    ppPoolDefaults[SCHATTR_REGRESSION_SHOW_EQUATION - SCHATTR_START] = new SfxBoolItem(SCHATTR_REGRESSION_SHOW_EQUATION, 0);
    ppPoolDefaults[SCHATTR_REGRESSION_SHOW_COEFF - SCHATTR_START] = new SfxBoolItem(SCHATTR_REGRESSION_SHOW_COEFF, 0);

    /**************************************************************************
    * ItemInfos
    **************************************************************************/
    pItemInfos = new SfxItemInfo[SCHATTR_END - SCHATTR_START + 1];

    USHORT i;
    for( i = SCHATTR_START; i <= SCHATTR_END; i++ )
    {
        pItemInfos[i - SCHATTR_START]._nSID = 0;
        pItemInfos[i - SCHATTR_START]._nFlags = SFX_ITEM_POOLABLE;
    }

    // slot ids differing from which ids
    pItemInfos[SCHATTR_SYMBOL_BRUSH - SCHATTR_START]._nSID = SID_ATTR_BRUSH;
    pItemInfos[SCHATTR_STYLE_SYMBOL - SCHATTR_START]._nSID = SID_ATTR_SYMBOLTYPE;
    pItemInfos[SCHATTR_SYMBOL_SIZE - SCHATTR_START]._nSID  = SID_ATTR_SYMBOLSIZE;

    SetDefaults(ppPoolDefaults);
    SetItemInfos(pItemInfos);
}

ChartItemPool::ChartItemPool(const ChartItemPool& rPool):
    SfxItemPool(rPool)
{
//     OSL_TRACE( "SCH: CTOR: ChartItemPool" );
}

ChartItemPool::~ChartItemPool()
{
//     OSL_TRACE( "SCH: DTOR: ChartItemPool" );
    Delete();

    delete[] pItemInfos;

    const USHORT nMax = SCHATTR_END - SCHATTR_START + 1;
    for( USHORT i=0; i<nMax; ++i )
    {
        SetRefCount(*ppPoolDefaults[i], 0);
        delete ppPoolDefaults[i];
    }

    delete[] ppPoolDefaults;
}

SfxItemPool* ChartItemPool::Clone() const
{
    return new ChartItemPool(*this);
}

SfxMapUnit ChartItemPool::GetMetric(USHORT /* nWhich */) const
{
    return SFX_MAPUNIT_100TH_MM;
}

// static
SfxItemPool* ChartItemPool::CreateChartItemPool()
{
    return new ChartItemPool();
}

} //  namespace chart
