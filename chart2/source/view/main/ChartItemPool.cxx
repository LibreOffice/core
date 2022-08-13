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
#include <chartview/ChartSfxItemIds.hxx>
#include <svx/chrtitem.hxx>
#include <svx/sdangitm.hxx>
#include <svx/svdpool.hxx>
#include <svx/svx3ditems.hxx>
#include <svl/intitem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/sizeitem.hxx>
#include <svl/stritem.hxx>
#include <svl/ilstitem.hxx>
#include <comphelper/processfactory.hxx>
#include <editeng/editids.hrc>
#include <svx/svxids.hrc>
#include <vector>

#include <com/sun/star/chart2/LegendPosition.hpp>
#include <com/sun/star/chart2/MovingAverageType.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/frame/Desktop.hpp>

namespace chart
{

ChartItemPool::ChartItemPool():
        SfxItemPool( "ChartItemPool" , SCHATTR_START, SCHATTR_END, nullptr, nullptr ),
        pItemInfos(new SfxItemInfo[SCHATTR_END - SCHATTR_START + 1])
{
    /**************************************************************************
    * PoolDefaults
    **************************************************************************/
    std::vector<SfxPoolItem*>* ppPoolDefaults = new std::vector<SfxPoolItem*>(SCHATTR_END - SCHATTR_START + 1);
    std::vector<SfxPoolItem*>& rPoolDefaults = *ppPoolDefaults;
    rPoolDefaults[SCHATTR_DATADESCR_SHOW_NUMBER    - SCHATTR_START] = new SfxBoolItem(SCHATTR_DATADESCR_SHOW_NUMBER);
    rPoolDefaults[SCHATTR_DATADESCR_SHOW_PERCENTAGE- SCHATTR_START] = new SfxBoolItem(SCHATTR_DATADESCR_SHOW_PERCENTAGE);
    rPoolDefaults[SCHATTR_DATADESCR_SHOW_CATEGORY  - SCHATTR_START] = new SfxBoolItem(SCHATTR_DATADESCR_SHOW_CATEGORY);
    rPoolDefaults[SCHATTR_DATADESCR_SHOW_SYMBOL    - SCHATTR_START] = new SfxBoolItem(SCHATTR_DATADESCR_SHOW_SYMBOL);
    rPoolDefaults[SCHATTR_DATADESCR_SHOW_DATA_SERIES_NAME - SCHATTR_START] = new SfxBoolItem(SCHATTR_DATADESCR_SHOW_DATA_SERIES_NAME);
    rPoolDefaults[SCHATTR_DATADESCR_WRAP_TEXT      - SCHATTR_START] = new SfxBoolItem(SCHATTR_DATADESCR_WRAP_TEXT);
    rPoolDefaults[SCHATTR_DATADESCR_SEPARATOR      - SCHATTR_START] = new SfxStringItem(SCHATTR_DATADESCR_SEPARATOR," ");
    rPoolDefaults[SCHATTR_DATADESCR_PLACEMENT      - SCHATTR_START] = new SfxInt32Item(SCHATTR_DATADESCR_PLACEMENT,0);
    rPoolDefaults[SCHATTR_DATADESCR_AVAILABLE_PLACEMENTS - SCHATTR_START] = new SfxIntegerListItem(SCHATTR_DATADESCR_AVAILABLE_PLACEMENTS, std::vector < sal_Int32 >() );
    rPoolDefaults[SCHATTR_DATADESCR_NO_PERCENTVALUE    - SCHATTR_START] = new SfxBoolItem(SCHATTR_DATADESCR_NO_PERCENTVALUE);
    rPoolDefaults[SCHATTR_DATADESCR_CUSTOM_LEADER_LINES - SCHATTR_START] = new SfxBoolItem(SCHATTR_DATADESCR_CUSTOM_LEADER_LINES, true);
    rPoolDefaults[SCHATTR_PERCENT_NUMBERFORMAT_VALUE  - SCHATTR_START] = new SfxUInt32Item(SCHATTR_PERCENT_NUMBERFORMAT_VALUE, 0);
    rPoolDefaults[SCHATTR_PERCENT_NUMBERFORMAT_SOURCE - SCHATTR_START] = new SfxBoolItem(SCHATTR_PERCENT_NUMBERFORMAT_SOURCE);

    //legend
    rPoolDefaults[SCHATTR_LEGEND_POS               - SCHATTR_START] = new SfxInt32Item(SCHATTR_LEGEND_POS, sal_Int32(css::chart2::LegendPosition_LINE_END) );
    rPoolDefaults[SCHATTR_LEGEND_SHOW              - SCHATTR_START] = new SfxBoolItem(SCHATTR_LEGEND_SHOW, true);
    rPoolDefaults[SCHATTR_LEGEND_NO_OVERLAY        - SCHATTR_START] = new SfxBoolItem(SCHATTR_LEGEND_NO_OVERLAY, true);

    //text
    rPoolDefaults[SCHATTR_TEXT_DEGREES             - SCHATTR_START] = new SdrAngleItem(SCHATTR_TEXT_DEGREES, 0_deg100);
    rPoolDefaults[SCHATTR_TEXT_STACKED             - SCHATTR_START] = new SfxBoolItem(SCHATTR_TEXT_STACKED,false);

    //statistic
    rPoolDefaults[SCHATTR_STAT_AVERAGE             - SCHATTR_START] = new SfxBoolItem (SCHATTR_STAT_AVERAGE);
    rPoolDefaults[SCHATTR_STAT_KIND_ERROR          - SCHATTR_START] = new SvxChartKindErrorItem (SvxChartKindError::NONE, SCHATTR_STAT_KIND_ERROR);
    rPoolDefaults[SCHATTR_STAT_PERCENT             - SCHATTR_START] = new SvxDoubleItem (0.0, SCHATTR_STAT_PERCENT);
    rPoolDefaults[SCHATTR_STAT_BIGERROR            - SCHATTR_START] = new SvxDoubleItem (0.0, SCHATTR_STAT_BIGERROR);
    rPoolDefaults[SCHATTR_STAT_CONSTPLUS           - SCHATTR_START] = new SvxDoubleItem (0.0, SCHATTR_STAT_CONSTPLUS);
    rPoolDefaults[SCHATTR_STAT_CONSTMINUS          - SCHATTR_START] = new SvxDoubleItem (0.0, SCHATTR_STAT_CONSTMINUS);
    rPoolDefaults[SCHATTR_STAT_INDICATE            - SCHATTR_START] = new SvxChartIndicateItem (SvxChartIndicate::NONE, SCHATTR_STAT_INDICATE);
    rPoolDefaults[SCHATTR_STAT_RANGE_POS           - SCHATTR_START] = new SfxStringItem (SCHATTR_STAT_RANGE_POS, OUString());
    rPoolDefaults[SCHATTR_STAT_RANGE_NEG           - SCHATTR_START] = new SfxStringItem (SCHATTR_STAT_RANGE_NEG, OUString());
    rPoolDefaults[SCHATTR_STAT_ERRORBAR_TYPE       - SCHATTR_START] = new SfxBoolItem(SCHATTR_STAT_ERRORBAR_TYPE, true);

    rPoolDefaults[SCHATTR_STYLE_DEEP     - SCHATTR_START] = new SfxBoolItem (SCHATTR_STYLE_DEEP, false);
    rPoolDefaults[SCHATTR_STYLE_3D       - SCHATTR_START] = new SfxBoolItem (SCHATTR_STYLE_3D, false);
    rPoolDefaults[SCHATTR_STYLE_VERTICAL - SCHATTR_START] = new SfxBoolItem (SCHATTR_STYLE_VERTICAL, false);
    rPoolDefaults[SCHATTR_STYLE_BASETYPE - SCHATTR_START] = new SfxInt32Item(SCHATTR_STYLE_BASETYPE, 0);
    rPoolDefaults[SCHATTR_STYLE_LINES    - SCHATTR_START] = new SfxBoolItem (SCHATTR_STYLE_LINES, false);
    rPoolDefaults[SCHATTR_STYLE_PERCENT  - SCHATTR_START] = new SfxBoolItem (SCHATTR_STYLE_PERCENT, false);
    rPoolDefaults[SCHATTR_STYLE_STACKED  - SCHATTR_START] = new SfxBoolItem (SCHATTR_STYLE_STACKED, false);
    rPoolDefaults[SCHATTR_STYLE_SPLINES  - SCHATTR_START] = new SfxInt32Item (SCHATTR_STYLE_SPLINES, 0); //Bug: was Bool! test ->Fileformat (touches only 5's)
    rPoolDefaults[SCHATTR_STYLE_SYMBOL   - SCHATTR_START] = new SfxInt32Item (SCHATTR_STYLE_SYMBOL, 0);
    rPoolDefaults[SCHATTR_STYLE_SHAPE    - SCHATTR_START] = new SfxInt32Item (SCHATTR_STYLE_SHAPE, 0);

    rPoolDefaults[SCHATTR_AXIS             - SCHATTR_START] = new SfxInt32Item(SCHATTR_AXIS,2); //2 = Y-Axis!!!

    //axis scale
    rPoolDefaults[SCHATTR_AXISTYPE             - SCHATTR_START] = new SfxInt32Item(SCHATTR_AXISTYPE, CHART_AXIS_REALNUMBER);
    rPoolDefaults[SCHATTR_AXIS_REVERSE         - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_REVERSE,false);
    rPoolDefaults[SCHATTR_AXIS_AUTO_MIN        - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_AUTO_MIN);
    rPoolDefaults[SCHATTR_AXIS_MIN             - SCHATTR_START] = new SvxDoubleItem(0.0, SCHATTR_AXIS_MIN);
    rPoolDefaults[SCHATTR_AXIS_AUTO_MAX        - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_AUTO_MAX);
    rPoolDefaults[SCHATTR_AXIS_MAX             - SCHATTR_START] = new SvxDoubleItem(0.0, SCHATTR_AXIS_MAX);
    rPoolDefaults[SCHATTR_AXIS_AUTO_STEP_MAIN  - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_AUTO_STEP_MAIN);
    rPoolDefaults[SCHATTR_AXIS_STEP_MAIN       - SCHATTR_START] = new SvxDoubleItem(0.0, SCHATTR_AXIS_STEP_MAIN);
    rPoolDefaults[SCHATTR_AXIS_MAIN_TIME_UNIT  - SCHATTR_START] = new SfxInt32Item(SCHATTR_AXIS_MAIN_TIME_UNIT,2);
    rPoolDefaults[SCHATTR_AXIS_AUTO_STEP_HELP  - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_AUTO_STEP_HELP);
    rPoolDefaults[SCHATTR_AXIS_STEP_HELP       - SCHATTR_START] = new SfxInt32Item(SCHATTR_AXIS_STEP_HELP,0);
    rPoolDefaults[SCHATTR_AXIS_HELP_TIME_UNIT  - SCHATTR_START] = new SfxInt32Item(SCHATTR_AXIS_HELP_TIME_UNIT,2);
    rPoolDefaults[SCHATTR_AXIS_AUTO_TIME_RESOLUTION    - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_AUTO_TIME_RESOLUTION);
    rPoolDefaults[SCHATTR_AXIS_TIME_RESOLUTION - SCHATTR_START] = new SfxInt32Item(SCHATTR_AXIS_TIME_RESOLUTION,2);
    rPoolDefaults[SCHATTR_AXIS_LOGARITHM       - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_LOGARITHM);
    rPoolDefaults[SCHATTR_AXIS_AUTO_DATEAXIS       - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_AUTO_DATEAXIS);
    rPoolDefaults[SCHATTR_AXIS_ALLOW_DATEAXIS      - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_ALLOW_DATEAXIS);
    rPoolDefaults[SCHATTR_AXIS_AUTO_ORIGIN     - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_AUTO_ORIGIN);
    rPoolDefaults[SCHATTR_AXIS_ORIGIN          - SCHATTR_START] = new SvxDoubleItem(0.0, SCHATTR_AXIS_ORIGIN);

    //axis position
    rPoolDefaults[SCHATTR_AXIS_TICKS           - SCHATTR_START] = new SfxInt32Item(SCHATTR_AXIS_TICKS,CHAXIS_MARK_OUTER);
    rPoolDefaults[SCHATTR_AXIS_HELPTICKS       - SCHATTR_START] = new SfxInt32Item(SCHATTR_AXIS_HELPTICKS,0);
    rPoolDefaults[SCHATTR_AXIS_POSITION        - SCHATTR_START] = new SfxInt32Item(SCHATTR_AXIS_POSITION,0);
    rPoolDefaults[SCHATTR_AXIS_POSITION_VALUE  - SCHATTR_START] = new SvxDoubleItem(0.0, SCHATTR_AXIS_POSITION_VALUE);
    rPoolDefaults[SCHATTR_AXIS_CROSSING_MAIN_AXIS_NUMBERFORMAT - SCHATTR_START] = new SfxUInt32Item(SCHATTR_AXIS_CROSSING_MAIN_AXIS_NUMBERFORMAT,0);
    rPoolDefaults[SCHATTR_AXIS_SHIFTED_CATEGORY_POSITION - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_SHIFTED_CATEGORY_POSITION,false);
    rPoolDefaults[SCHATTR_AXIS_LABEL_POSITION  - SCHATTR_START] = new SfxInt32Item(SCHATTR_AXIS_LABEL_POSITION,0);
    rPoolDefaults[SCHATTR_AXIS_MARK_POSITION   - SCHATTR_START] = new SfxInt32Item(SCHATTR_AXIS_MARK_POSITION,0);

    //axis label
    rPoolDefaults[SCHATTR_AXIS_SHOWDESCR       - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_SHOWDESCR,false);
    rPoolDefaults[SCHATTR_AXIS_LABEL_ORDER     - SCHATTR_START] = new SvxChartTextOrderItem(SvxChartTextOrder::SideBySide, SCHATTR_AXIS_LABEL_ORDER);
    rPoolDefaults[SCHATTR_AXIS_LABEL_OVERLAP   - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_LABEL_OVERLAP,false);
    rPoolDefaults[SCHATTR_AXIS_LABEL_BREAK     - SCHATTR_START] = new SfxBoolItem(SCHATTR_AXIS_LABEL_BREAK, false );

    rPoolDefaults[SCHATTR_SYMBOL_BRUSH         - SCHATTR_START] = new SvxBrushItem(SCHATTR_SYMBOL_BRUSH);
    rPoolDefaults[SCHATTR_STOCK_VOLUME         - SCHATTR_START] = new SfxBoolItem(SCHATTR_STOCK_VOLUME,false);
    rPoolDefaults[SCHATTR_STOCK_UPDOWN         - SCHATTR_START] = new SfxBoolItem(SCHATTR_STOCK_UPDOWN,false);
    rPoolDefaults[SCHATTR_SYMBOL_SIZE          - SCHATTR_START] = new SvxSizeItem(SCHATTR_SYMBOL_SIZE,Size(0,0));
    rPoolDefaults[SCHATTR_HIDE_DATA_POINT_LEGEND_ENTRY - SCHATTR_START] = new SfxBoolItem(SCHATTR_HIDE_DATA_POINT_LEGEND_ENTRY, false);

    // new for New Chart
    rPoolDefaults[SCHATTR_BAR_OVERLAP          - SCHATTR_START] = new SfxInt32Item(SCHATTR_BAR_OVERLAP,0);
    rPoolDefaults[SCHATTR_BAR_GAPWIDTH         - SCHATTR_START] = new SfxInt32Item(SCHATTR_BAR_GAPWIDTH,0);
    rPoolDefaults[SCHATTR_BAR_CONNECT          - SCHATTR_START] = new SfxBoolItem(SCHATTR_BAR_CONNECT, false);
    rPoolDefaults[SCHATTR_NUM_OF_LINES_FOR_BAR - SCHATTR_START] = new SfxInt32Item( SCHATTR_NUM_OF_LINES_FOR_BAR, 0 );
    rPoolDefaults[SCHATTR_SPLINE_ORDER         - SCHATTR_START] = new SfxInt32Item( SCHATTR_SPLINE_ORDER, 3 );
    rPoolDefaults[SCHATTR_SPLINE_RESOLUTION    - SCHATTR_START] = new SfxInt32Item( SCHATTR_SPLINE_RESOLUTION, 20 );
    rPoolDefaults[SCHATTR_GROUP_BARS_PER_AXIS  - SCHATTR_START] = new SfxBoolItem(SCHATTR_GROUP_BARS_PER_AXIS, false);
    rPoolDefaults[SCHATTR_STARTING_ANGLE       - SCHATTR_START] = new SdrAngleItem( SCHATTR_STARTING_ANGLE, 9000_deg100 );
    rPoolDefaults[SCHATTR_CLOCKWISE            - SCHATTR_START] = new SfxBoolItem( SCHATTR_CLOCKWISE, false );

    rPoolDefaults[SCHATTR_MISSING_VALUE_TREATMENT    - SCHATTR_START] = new SfxInt32Item(SCHATTR_MISSING_VALUE_TREATMENT, 0);
    rPoolDefaults[SCHATTR_AVAILABLE_MISSING_VALUE_TREATMENTS - SCHATTR_START] = new SfxIntegerListItem(SCHATTR_AVAILABLE_MISSING_VALUE_TREATMENTS, std::vector < sal_Int32 >() );
    rPoolDefaults[SCHATTR_INCLUDE_HIDDEN_CELLS - SCHATTR_START] = new SfxBoolItem(SCHATTR_INCLUDE_HIDDEN_CELLS, true);
    rPoolDefaults[SCHATTR_HIDE_LEGEND_ENTRY - SCHATTR_START] = new SfxBoolItem(SCHATTR_HIDE_LEGEND_ENTRY, false);

    rPoolDefaults[SCHATTR_AXIS_FOR_ALL_SERIES  - SCHATTR_START] = new SfxInt32Item(SCHATTR_AXIS_FOR_ALL_SERIES, 0);

    rPoolDefaults[SCHATTR_REGRESSION_TYPE                  - SCHATTR_START] = new SvxChartRegressItem  (SvxChartRegress::NONE, SCHATTR_REGRESSION_TYPE);
    rPoolDefaults[SCHATTR_REGRESSION_SHOW_EQUATION         - SCHATTR_START] = new SfxBoolItem(SCHATTR_REGRESSION_SHOW_EQUATION, false);
    rPoolDefaults[SCHATTR_REGRESSION_SHOW_COEFF            - SCHATTR_START] = new SfxBoolItem(SCHATTR_REGRESSION_SHOW_COEFF, false);
    rPoolDefaults[SCHATTR_REGRESSION_DEGREE                - SCHATTR_START] = new SfxInt32Item(SCHATTR_REGRESSION_DEGREE, 2);
    rPoolDefaults[SCHATTR_REGRESSION_PERIOD                - SCHATTR_START] = new SfxInt32Item(SCHATTR_REGRESSION_PERIOD, 2);
    rPoolDefaults[SCHATTR_REGRESSION_EXTRAPOLATE_FORWARD   - SCHATTR_START] = new SvxDoubleItem(0.0, SCHATTR_REGRESSION_EXTRAPOLATE_FORWARD);
    rPoolDefaults[SCHATTR_REGRESSION_EXTRAPOLATE_BACKWARD  - SCHATTR_START] = new SvxDoubleItem(0.0, SCHATTR_REGRESSION_EXTRAPOLATE_BACKWARD);
    rPoolDefaults[SCHATTR_REGRESSION_SET_INTERCEPT         - SCHATTR_START] = new SfxBoolItem(SCHATTR_REGRESSION_SET_INTERCEPT, false);
    rPoolDefaults[SCHATTR_REGRESSION_INTERCEPT_VALUE       - SCHATTR_START] = new SvxDoubleItem(0.0, SCHATTR_REGRESSION_INTERCEPT_VALUE);
    rPoolDefaults[SCHATTR_REGRESSION_CURVE_NAME            - SCHATTR_START] = new SfxStringItem(SCHATTR_REGRESSION_CURVE_NAME, OUString());
    rPoolDefaults[SCHATTR_REGRESSION_XNAME                 - SCHATTR_START] = new SfxStringItem(SCHATTR_REGRESSION_XNAME, "x");
    rPoolDefaults[SCHATTR_REGRESSION_YNAME                 - SCHATTR_START] = new SfxStringItem(SCHATTR_REGRESSION_YNAME, "f(x)");
    rPoolDefaults[SCHATTR_REGRESSION_MOVING_TYPE           - SCHATTR_START] = new SfxInt32Item(SCHATTR_REGRESSION_MOVING_TYPE, css::chart2::MovingAverageType::Prior);

    rPoolDefaults[SCHATTR_DATA_TABLE_HORIZONTAL_BORDER - SCHATTR_START] = new SfxBoolItem(SCHATTR_DATA_TABLE_HORIZONTAL_BORDER, false);
    rPoolDefaults[SCHATTR_DATA_TABLE_VERTICAL_BORDER   - SCHATTR_START] = new SfxBoolItem(SCHATTR_DATA_TABLE_VERTICAL_BORDER, false);
    rPoolDefaults[SCHATTR_DATA_TABLE_OUTLINE           - SCHATTR_START] = new SfxBoolItem(SCHATTR_DATA_TABLE_OUTLINE, false);
    rPoolDefaults[SCHATTR_DATA_TABLE_KEYS              - SCHATTR_START] = new SfxBoolItem(SCHATTR_DATA_TABLE_KEYS, false);

    /**************************************************************************
    * ItemInfos
    **************************************************************************/
    const sal_uInt16 nMax = SCHATTR_END - SCHATTR_START + 1;
    for( sal_uInt16 i = 0; i < nMax; i++ )
    {
        pItemInfos[i]._nSID = 0;
        pItemInfos[i]._bPoolable = true;
    }

    // slot ids differing from which ids
    pItemInfos[SCHATTR_SYMBOL_BRUSH - SCHATTR_START]._nSID = SID_ATTR_BRUSH;
    pItemInfos[SCHATTR_STYLE_SYMBOL - SCHATTR_START]._nSID = SID_ATTR_SYMBOLTYPE;
    pItemInfos[SCHATTR_SYMBOL_SIZE - SCHATTR_START]._nSID  = SID_ATTR_SYMBOLSIZE;

    SetDefaults(ppPoolDefaults);
    SetItemInfos(pItemInfos.get());
}

ChartItemPool::ChartItemPool(const ChartItemPool& rPool):
    SfxItemPool(rPool)
{
}

ChartItemPool::~ChartItemPool()
{
    Delete();
    // release and delete static pool default items
    ReleaseDefaults(true);
}

rtl::Reference<SfxItemPool> ChartItemPool::Clone() const
{
    return new ChartItemPool(*this);
}

MapUnit ChartItemPool::GetMetric(sal_uInt16 /* nWhich */) const
{
    return MapUnit::Map100thMM;
}

static rtl::Reference<SfxItemPool> g_Pool1, g_Pool2, g_Pool3;

/** If we let the libc runtime clean us up, we trigger a crash */
namespace
{
class TerminateListener : public ::cppu::WeakImplHelper< css::frame::XTerminateListener >
{
    void SAL_CALL queryTermination( const css::lang::EventObject& ) override
    {}
    void SAL_CALL notifyTermination( const css::lang::EventObject& ) override
    {
        g_Pool1.clear();
        g_Pool2.clear();
        g_Pool3.clear();
    }
    virtual void SAL_CALL disposing( const ::css::lang::EventObject& ) override
    {}
};
};

SfxItemPool& ChartItemPool::GetGlobalChartItemPool()
{
    if (!g_Pool1)
    {
        // similar logic to SdrModel's pool, but with our chart pool tagged on the end
        g_Pool1 = new SdrItemPool(nullptr);
        g_Pool2 = EditEngine::CreatePool();
        g_Pool3 = new ChartItemPool();
        g_Pool1->SetSecondaryPool(g_Pool2.get());

        g_Pool1->SetDefaultMetric(MapUnit::Map100thMM);
        g_Pool1->SetPoolDefaultItem(SfxBoolItem(EE_PARA_HYPHENATE, true) );
        g_Pool1->SetPoolDefaultItem(makeSvx3DPercentDiagonalItem (5));

        g_Pool2->SetSecondaryPool(g_Pool3.get());
        g_Pool1->FreezeIdRanges();

        css::uno::Reference< css::frame::XDesktop2 > xDesktop = css::frame::Desktop::create(comphelper::getProcessComponentContext());
        css::uno::Reference< css::frame::XTerminateListener > xListener( new TerminateListener );
        xDesktop->addTerminateListener( xListener );
    }
    return *g_Pool1;
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
