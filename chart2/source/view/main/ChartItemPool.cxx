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

#include "ChartItemPool.hxx"
#include "macros.hxx"
#include "chartview/ChartSfxItemIds.hxx"
#include <svx/chrtitem.hxx>
#include <svl/intitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/sizeitem.hxx>
#include <svl/stritem.hxx>
#include <svl/rectitem.hxx>
#include <svl/ilstitem.hxx>
#include <editeng/editids.hrc>
#include <svx/svxids.hrc>
#include <vector>

#include <com/sun/star/chart2/LegendPosition.hpp>

namespace chart
{

ChartItemPool::ChartItemPool():
        SfxItemPool( "ChartItemPool" , SCHATTR_START, SCHATTR_END, NULL, NULL )
{
    /**************************************************************************
    * PoolDefaults
    **************************************************************************/
    ppPoolDefaults = new SfxPoolItem*[SCHATTR_END - SCHATTR_START + 1];

    ppPoolDefaults[SCHATTR_DATADESCR_SHOW_NUMBER    - SCHATTR_START] = new SfxBoolItem(SCHATTR_DATADESCR_SHOW_NUMBER);
    ppPoolDefaults[SCHATTR_DATADESCR_SHOW_PERCENTAGE- SCHATTR_START] = new SfxBoolItem(SCHATTR_DATADESCR_SHOW_PERCENTAGE);
    ppPoolDefaults[SCHATTR_DATADESCR_SHOW_CATEGORY  - SCHATTR_START] = new SfxBoolItem(SCHATTR_DATADESCR_SHOW_CATEGORY);
    ppPoolDefaults[SCHATTR_DATADESCR_SHOW_SYMBOL    - SCHATTR_START] = new SfxBoolItem(SCHATTR_DATADESCR_SHOW_SYMBOL);
    ppPoolDefaults[SCHATTR_DATADESCR_SEPARATOR      - SCHATTR_START] = new SfxStringItem(SCHATTR_DATADESCR_SEPARATOR," ");
    ppPoolDefaults[SCHATTR_DATADESCR_PLACEMENT      - SCHATTR_START] = new SfxInt32Item(SCHATTR_DATADESCR_PLACEMENT,0);
    ppPoolDefaults[SCHATTR_DATADESCR_AVAILABLE_PLACEMENTS - SCHATTR_START] = new SfxIntegerListItem(SCHATTR_DATADESCR_AVAILABLE_PLACEMENTS, ::std::vector < sal_Int32 >() );
    ppPoolDefaults[SCHATTR_DATADESCR_NO_PERCENTVALUE    - SCHATTR_START] = new SfxBoolItem(SCHATTR_DATADESCR_NO_PERCENTVALUE);
    ppPoolDefaults[SCHATTR_PERCENT_NUMBERFORMAT_VALUE  - SCHATTR_START] = new SfxUInt32Item(SCHATTR_PERCENT_NUMBERFORMAT_VALUE, 0);
    ppPoolDefaults[SCHATTR_PERCENT_NUMBERFORMAT_SOURCE - SCHATTR_START] = new SfxBoolItem(SCHATTR_PERCENT_NUMBERFORMAT_SOURCE);

    //legend
    ppPoolDefaults[SCHATTR_LEGEND_POS               - SCHATTR_START] = new SfxInt32Item(SCHATTR_LEGEND_POS, ::com::sun::star::chart2::LegendPosition_LINE_END );
    ppPoolDefaults[SCHATTR_LEGEND_SHOW              - SCHATTR_START] = new SfxBoolItem(SCHATTR_LEGEND_SHOW, sal_True);

    //text
    ppPoolDefaults[SCHATTR_TEXT_DEGREES             - SCHATTR_START] = new SfxInt32Item(SCHATTR_TEXT_DEGREES, 0);
    ppPoolDefaults[SCHATTR_TEXT_STACKED             - SCHATTR_START] = new SfxBoolItem(SCHATTR_TEXT_STACKED,sal_False);

    //statistic
    ppPoolDefaults[SCHATTR_STAT_AVERAGE             - SCHATTR_START] = new SfxBoolItem (SCHATTR_STAT_AVERAGE);
    ppPoolDefaults[SCHATTR_STAT_KIND_ERROR          - SCHATTR_START] = new SvxChartKindErrorItem (CHERROR_NONE, SCHATTR_STAT_KIND_ERROR);
    ppPoolDefaults[SCHATTR_STAT_PERCENT             - SCHATTR_START] = new SvxDoubleItem (0.0, SCHATTR_STAT_PERCENT);
    ppPoolDefaults[SCHATTR_STAT_BIGERROR            - SCHATTR_START] = new SvxDoubleItem (0.0, SCHATTR_STAT_BIGERROR);
    ppPoolDefaults[SCHATTR_STAT_CONSTPLUS           - SCHATTR_START] = new SvxDoubleItem (0.0, SCHATTR_STAT_CONSTPLUS);
    ppPoolDefaults[SCHATTR_STAT_CONSTMINUS          - SCHATTR_START] = new SvxDoubleItem (0.0, SCHATTR_STAT_CONSTMINUS);
    ppPoolDefaults[SCHATTR_STAT_INDICATE            - SCHATTR_START] = new SvxChartIndicateItem (CHINDICATE_NONE, SCHATTR_STAT_INDICATE);
    ppPoolDefaults[SCHATTR_STAT_RANGE_POS           - SCHATTR_START] = new SfxStringItem (SCHATTR_STAT_RANGE_POS, OUString());
    ppPoolDefaults[SCHATTR_STAT_RANGE_NEG           - SCHATTR_START] = new SfxStringItem (SCHATTR_STAT_RANGE_NEG, OUString());
    ppPoolDefaults[SCHATTR_STAT_ERRORBAR_TYPE       - SCHATTR_START] = new SfxBoolItem(SCHATTR_STAT_ERRORBAR_TYPE, true);

    ppPoolDefaults[SCHATTR_STYLE_DEEP     - SCHATTR_START] = new SfxBoolItem (SCHATTR_STYLE_DEEP, 0);
    ppPoolDefaults[SCHATTR_STYLE_3D       - SCHATTR_START] = new SfxBoolItem (SCHATTR_STYLE_3D, 0);
    ppPoolDefaults[SCHATTR_STYLE_VERTICAL - SCHATTR_START] = new SfxBoolItem (SCHATTR_STYLE_VERTICAL, 0);
    ppPoolDefaults[SCHATTR_STYLE_BASETYPE - SCHATTR_START] = new SfxInt32Item(SCHATTR_STYLE_BASETYPE, 0);
    ppPoolDefaults[SCHATTR_STYLE_LINES    - SCHATTR_START] = new SfxBoolItem (SCHATTR_STYLE_LINES, 0);
    ppPoolDefaults[SCHATTR_STYLE_PERCENT  - SCHATTR_START] = new SfxBoolItem (SCHATTR_STYLE_PERCENT, 0);
    ppPoolDefaults[SCHATTR_STYLE_STACKED  - SCHATTR_START] = new SfxBoolItem (SCHATTR_STYLE_STACKED, 0);
    ppPoolDefaults[SCHATTR_STYLE_SPLINES  - SCHATTR_START] = new SfxInt32Item (SCHATTR_STYLE_SPLINES, 0); //Bug: was Bool! test ->Fileformat (touches only 5's)
    ppPoolDefaults[SCHATTR_STYLE_SYMBOL   - SCHATTR_START] = new SfxInt32Item (SCHATTR_STYLE_SYMBOL, 0);
    ppPoolDefaults[SCHATTR_STYLE_SHAPE    - SCHATTR_START] = new SfxInt32Item (SCHATTR_STYLE_SHAPE, 0);

    ppPoolDefaults[SCHATTR_AXIS             - SCHATTR_START] = new SfxInt32Item(SCHATTR_AXIS,2); //2 = Y-Axis!!!

    //axis scale
    ppPoolDefaults[SCHATTR_AXISTYPE             - SCHATTR_START] = new SfxInt32Item(SCHATTR_AXISTYPE, CHART_AXIS_REALNUMBER);
    ppPoolDefaults[SCHATTR_AXIS_REVERSE         - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_REVERSE,0);
    ppPoolDefaults[SCHATTR_AXIS_AUTO_MIN        - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_AUTO_MIN);
    ppPoolDefaults[SCHATTR_AXIS_MIN             - SCHATTR_START] = new SvxDoubleItem(0.0, SCHATTR_AXIS_MIN);
    ppPoolDefaults[SCHATTR_AXIS_AUTO_MAX        - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_AUTO_MAX);
    ppPoolDefaults[SCHATTR_AXIS_MAX             - SCHATTR_START] = new SvxDoubleItem(0.0, SCHATTR_AXIS_MAX);
    ppPoolDefaults[SCHATTR_AXIS_AUTO_STEP_MAIN  - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_AUTO_STEP_MAIN);
    ppPoolDefaults[SCHATTR_AXIS_STEP_MAIN       - SCHATTR_START] = new SvxDoubleItem(0.0, SCHATTR_AXIS_STEP_MAIN);
    ppPoolDefaults[SCHATTR_AXIS_MAIN_TIME_UNIT  - SCHATTR_START] = new SfxInt32Item(SCHATTR_AXIS_MAIN_TIME_UNIT,2);
    ppPoolDefaults[SCHATTR_AXIS_AUTO_STEP_HELP  - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_AUTO_STEP_HELP);
    ppPoolDefaults[SCHATTR_AXIS_STEP_HELP       - SCHATTR_START] = new SfxInt32Item(SCHATTR_AXIS_STEP_HELP,0);
    ppPoolDefaults[SCHATTR_AXIS_HELP_TIME_UNIT  - SCHATTR_START] = new SfxInt32Item(SCHATTR_AXIS_HELP_TIME_UNIT,2);
    ppPoolDefaults[SCHATTR_AXIS_AUTO_TIME_RESOLUTION    - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_AUTO_TIME_RESOLUTION);
    ppPoolDefaults[SCHATTR_AXIS_TIME_RESOLUTION - SCHATTR_START] = new SfxInt32Item(SCHATTR_AXIS_TIME_RESOLUTION,2);
    ppPoolDefaults[SCHATTR_AXIS_LOGARITHM       - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_LOGARITHM);
    ppPoolDefaults[SCHATTR_AXIS_AUTO_DATEAXIS       - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_AUTO_DATEAXIS);
    ppPoolDefaults[SCHATTR_AXIS_ALLOW_DATEAXIS      - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_ALLOW_DATEAXIS);
    ppPoolDefaults[SCHATTR_AXIS_AUTO_ORIGIN     - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_AUTO_ORIGIN);
    ppPoolDefaults[SCHATTR_AXIS_ORIGIN          - SCHATTR_START] = new SvxDoubleItem(0.0, SCHATTR_AXIS_ORIGIN);

    //axis position
    ppPoolDefaults[SCHATTR_AXIS_TICKS           - SCHATTR_START] = new SfxInt32Item(SCHATTR_AXIS_TICKS,CHAXIS_MARK_OUTER);
    ppPoolDefaults[SCHATTR_AXIS_HELPTICKS       - SCHATTR_START] = new SfxInt32Item(SCHATTR_AXIS_HELPTICKS,0);
    ppPoolDefaults[SCHATTR_AXIS_POSITION        - SCHATTR_START] = new SfxInt32Item(SCHATTR_AXIS_POSITION,0);
    ppPoolDefaults[SCHATTR_AXIS_POSITION_VALUE  - SCHATTR_START] = new SvxDoubleItem(0.0, SCHATTR_AXIS_POSITION_VALUE);
    ppPoolDefaults[SCHATTR_AXIS_CROSSING_MAIN_AXIS_NUMBERFORMAT - SCHATTR_START] = new SfxUInt32Item(SCHATTR_AXIS_CROSSING_MAIN_AXIS_NUMBERFORMAT,0);
    ppPoolDefaults[SCHATTR_AXIS_LABEL_POSITION  - SCHATTR_START] = new SfxInt32Item(SCHATTR_AXIS_LABEL_POSITION,0);
    ppPoolDefaults[SCHATTR_AXIS_MARK_POSITION   - SCHATTR_START] = new SfxInt32Item(SCHATTR_AXIS_MARK_POSITION,0);

    //axis label
    ppPoolDefaults[SCHATTR_AXIS_SHOWDESCR       - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_SHOWDESCR,0);
    ppPoolDefaults[SCHATTR_AXIS_LABEL_ORDER     - SCHATTR_START] = new SvxChartTextOrderItem(CHTXTORDER_SIDEBYSIDE, SCHATTR_AXIS_LABEL_ORDER);
    ppPoolDefaults[SCHATTR_AXIS_LABEL_OVERLAP   - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_LABEL_OVERLAP,sal_False);
    ppPoolDefaults[SCHATTR_AXIS_LABEL_BREAK     - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_LABEL_BREAK, sal_False );

    //--
    ppPoolDefaults[SCHATTR_SYMBOL_BRUSH         - SCHATTR_START] = new SvxBrushItem(SCHATTR_SYMBOL_BRUSH);
    ppPoolDefaults[SCHATTR_STOCK_VOLUME         - SCHATTR_START] = new SfxBoolItem(SCHATTR_STOCK_VOLUME,0);
    ppPoolDefaults[SCHATTR_STOCK_UPDOWN         - SCHATTR_START] = new SfxBoolItem(SCHATTR_STOCK_UPDOWN,0);
    ppPoolDefaults[SCHATTR_SYMBOL_SIZE          - SCHATTR_START] = new SvxSizeItem(SCHATTR_SYMBOL_SIZE,Size(0,0));

    // new for New Chart
    ppPoolDefaults[SCHATTR_BAR_OVERLAP          - SCHATTR_START] = new SfxInt32Item(SCHATTR_BAR_OVERLAP,0);
    ppPoolDefaults[SCHATTR_BAR_GAPWIDTH         - SCHATTR_START] = new SfxInt32Item(SCHATTR_BAR_GAPWIDTH,0);
    ppPoolDefaults[SCHATTR_BAR_CONNECT          - SCHATTR_START] = new SfxBoolItem(SCHATTR_BAR_CONNECT, sal_False);
    ppPoolDefaults[SCHATTR_NUM_OF_LINES_FOR_BAR - SCHATTR_START] = new SfxInt32Item( SCHATTR_NUM_OF_LINES_FOR_BAR, 0 );
    ppPoolDefaults[SCHATTR_SPLINE_ORDER         - SCHATTR_START] = new SfxInt32Item( SCHATTR_SPLINE_ORDER, 3 );
    ppPoolDefaults[SCHATTR_SPLINE_RESOLUTION    - SCHATTR_START] = new SfxInt32Item( SCHATTR_SPLINE_RESOLUTION, 20 );
    ppPoolDefaults[SCHATTR_DIAGRAM_STYLE        - SCHATTR_START] = new SvxChartStyleItem( CHSTYLE_2D_COLUMN, SCHATTR_DIAGRAM_STYLE );
    ppPoolDefaults[SCHATTR_GROUP_BARS_PER_AXIS  - SCHATTR_START] = new SfxBoolItem(SCHATTR_GROUP_BARS_PER_AXIS, sal_False);
    ppPoolDefaults[SCHATTR_STARTING_ANGLE       - SCHATTR_START] = new SfxInt32Item( SCHATTR_STARTING_ANGLE, 90 );
    ppPoolDefaults[SCHATTR_CLOCKWISE            - SCHATTR_START] = new SfxBoolItem( SCHATTR_CLOCKWISE, sal_False );

    ppPoolDefaults[SCHATTR_MISSING_VALUE_TREATMENT    - SCHATTR_START] = new SfxInt32Item(SCHATTR_MISSING_VALUE_TREATMENT, 0);
    ppPoolDefaults[SCHATTR_AVAILABLE_MISSING_VALUE_TREATMENTS - SCHATTR_START] = new SfxIntegerListItem(SCHATTR_AVAILABLE_MISSING_VALUE_TREATMENTS, ::std::vector < sal_Int32 >() );
    ppPoolDefaults[SCHATTR_INCLUDE_HIDDEN_CELLS - SCHATTR_START] = new SfxBoolItem(SCHATTR_INCLUDE_HIDDEN_CELLS, sal_True);

    ppPoolDefaults[SCHATTR_AXIS_FOR_ALL_SERIES  - SCHATTR_START] = new SfxInt32Item(SCHATTR_AXIS_FOR_ALL_SERIES, 0);

    ppPoolDefaults[SCHATTR_REGRESSION_TYPE                  - SCHATTR_START] = new SvxChartRegressItem  (CHREGRESS_NONE, SCHATTR_REGRESSION_TYPE);
    ppPoolDefaults[SCHATTR_REGRESSION_SHOW_EQUATION         - SCHATTR_START] = new SfxBoolItem(SCHATTR_REGRESSION_SHOW_EQUATION, 0);
    ppPoolDefaults[SCHATTR_REGRESSION_SHOW_COEFF            - SCHATTR_START] = new SfxBoolItem(SCHATTR_REGRESSION_SHOW_COEFF, 0);
    ppPoolDefaults[SCHATTR_REGRESSION_DEGREE                - SCHATTR_START] = new SfxInt32Item(SCHATTR_REGRESSION_DEGREE, 2);
    ppPoolDefaults[SCHATTR_REGRESSION_PERIOD                - SCHATTR_START] = new SfxInt32Item(SCHATTR_REGRESSION_PERIOD, 2);
    ppPoolDefaults[SCHATTR_REGRESSION_EXTRAPOLATE_FORWARD   - SCHATTR_START] = new SvxDoubleItem(0.0, SCHATTR_REGRESSION_EXTRAPOLATE_FORWARD);
    ppPoolDefaults[SCHATTR_REGRESSION_EXTRAPOLATE_BACKWARD  - SCHATTR_START] = new SvxDoubleItem(0.0, SCHATTR_REGRESSION_EXTRAPOLATE_BACKWARD);
    ppPoolDefaults[SCHATTR_REGRESSION_SET_INTERCEPT         - SCHATTR_START] = new SfxBoolItem(SCHATTR_REGRESSION_SET_INTERCEPT, sal_False);
    ppPoolDefaults[SCHATTR_REGRESSION_INTERCEPT_VALUE       - SCHATTR_START] = new SvxDoubleItem( 0.0, SCHATTR_REGRESSION_INTERCEPT_VALUE);

    /**************************************************************************
    * ItemInfos
    **************************************************************************/
    pItemInfos = new SfxItemInfo[SCHATTR_END - SCHATTR_START + 1];

    const sal_uInt16 nMax = SCHATTR_END - SCHATTR_START + 1;
    for( sal_uInt16 i = 0; i < nMax; i++ )
    {
        pItemInfos[i]._nSID = 0;
        pItemInfos[i]._nFlags = SFX_ITEM_POOLABLE;
    }

    // slot ids differing from which ids
    pItemInfos[SCHATTR_SYMBOL_BRUSH - SCHATTR_START]._nSID = SID_ATTR_BRUSH;
    pItemInfos[SCHATTR_STYLE_SYMBOL - SCHATTR_START]._nSID = SID_ATTR_SYMBOLTYPE;
    pItemInfos[SCHATTR_SYMBOL_SIZE - SCHATTR_START]._nSID  = SID_ATTR_SYMBOLSIZE;

    SetDefaults(ppPoolDefaults);
    SetItemInfos(pItemInfos);
}

ChartItemPool::ChartItemPool(const ChartItemPool& rPool):
    SfxItemPool(rPool), ppPoolDefaults(0), pItemInfos(0)
{
}

ChartItemPool::~ChartItemPool()
{
    Delete();

    delete[] pItemInfos;

    const sal_uInt16 nMax = SCHATTR_END - SCHATTR_START + 1;
    for( sal_uInt16 i=0; i<nMax; ++i )
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

SfxMapUnit ChartItemPool::GetMetric(sal_uInt16 /* nWhich */) const
{
    return SFX_MAPUNIT_100TH_MM;
}

SfxItemPool* ChartItemPool::CreateChartItemPool()
{
    return new ChartItemPool();
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
