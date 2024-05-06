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
#include <DataSeries.hxx>
#include <FormattedString.hxx>
#include <Legend.hxx>
#include <Axis.hxx>
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

static ItemInfoPackage& getItemInfoPackageChart()
{
    class ItemInfoPackageChart : public ItemInfoPackage
    {
        typedef std::array<ItemInfoStatic, SCHATTR_END - SCHATTR_START + 1> ItemInfoArrayChart;
        ItemInfoArrayChart maItemInfos {{
            // m_nWhich, m_pItem, m_nSlotID, m_nItemInfoFlags
            { SCHATTR_DATADESCR_SHOW_NUMBER, new SfxBoolItem(SCHATTR_DATADESCR_SHOW_NUMBER), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_DATADESCR_SHOW_PERCENTAGE, new SfxBoolItem(SCHATTR_DATADESCR_SHOW_PERCENTAGE), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_DATADESCR_SHOW_CATEGORY, new SfxBoolItem(SCHATTR_DATADESCR_SHOW_CATEGORY), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_DATADESCR_SHOW_SYMBOL, new SfxBoolItem(SCHATTR_DATADESCR_SHOW_SYMBOL), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_DATADESCR_WRAP_TEXT, new SfxBoolItem(SCHATTR_DATADESCR_WRAP_TEXT), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_DATADESCR_SEPARATOR, new SfxStringItem(SCHATTR_DATADESCR_SEPARATOR,u" "_ustr), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_DATADESCR_PLACEMENT, new SfxInt32Item(SCHATTR_DATADESCR_PLACEMENT,0), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_DATADESCR_AVAILABLE_PLACEMENTS, new SfxIntegerListItem(SCHATTR_DATADESCR_AVAILABLE_PLACEMENTS, std::vector < sal_Int32 >() ), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_DATADESCR_NO_PERCENTVALUE, new SfxBoolItem(SCHATTR_DATADESCR_NO_PERCENTVALUE), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_DATADESCR_CUSTOM_LEADER_LINES, new SfxBoolItem(SCHATTR_DATADESCR_CUSTOM_LEADER_LINES, true), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_PERCENT_NUMBERFORMAT_VALUE, new SfxUInt32Item(SCHATTR_PERCENT_NUMBERFORMAT_VALUE, 0), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_PERCENT_NUMBERFORMAT_SOURCE, new SfxBoolItem(SCHATTR_PERCENT_NUMBERFORMAT_SOURCE), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_DATADESCR_SHOW_DATA_SERIES_NAME, new SfxBoolItem(SCHATTR_DATADESCR_SHOW_DATA_SERIES_NAME), 0, SFX_ITEMINFOFLAG_NONE },

            //legend
            { SCHATTR_LEGEND_POS, new SfxInt32Item(SCHATTR_LEGEND_POS, sal_Int32(css::chart2::LegendPosition_LINE_END) ), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_LEGEND_SHOW, new SfxBoolItem(SCHATTR_LEGEND_SHOW, true), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_LEGEND_NO_OVERLAY, new SfxBoolItem(SCHATTR_LEGEND_NO_OVERLAY, true), 0, SFX_ITEMINFOFLAG_NONE },

            //text
            { SCHATTR_TEXT_DEGREES, new SdrAngleItem(SCHATTR_TEXT_DEGREES, 0_deg100), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_TEXT_STACKED, new SfxBoolItem(SCHATTR_TEXT_STACKED,false), 0, SFX_ITEMINFOFLAG_NONE },

            //statistic
            { SCHATTR_STAT_AVERAGE, new SfxBoolItem (SCHATTR_STAT_AVERAGE), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_STAT_KIND_ERROR, new SvxChartKindErrorItem (SvxChartKindError::NONE, SCHATTR_STAT_KIND_ERROR), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_STAT_PERCENT, new SvxDoubleItem (0.0, SCHATTR_STAT_PERCENT), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_STAT_BIGERROR, new SvxDoubleItem (0.0, SCHATTR_STAT_BIGERROR), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_STAT_CONSTPLUS, new SvxDoubleItem (0.0, SCHATTR_STAT_CONSTPLUS), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_STAT_CONSTMINUS, new SvxDoubleItem (0.0, SCHATTR_STAT_CONSTMINUS), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_STAT_INDICATE, new SvxChartIndicateItem (SvxChartIndicate::NONE, SCHATTR_STAT_INDICATE), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_STAT_RANGE_POS, new SfxStringItem (SCHATTR_STAT_RANGE_POS, OUString()), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_STAT_RANGE_NEG, new SfxStringItem (SCHATTR_STAT_RANGE_NEG, OUString()), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_STAT_ERRORBAR_TYPE, new SfxBoolItem(SCHATTR_STAT_ERRORBAR_TYPE, true), 0, SFX_ITEMINFOFLAG_NONE },

            { SCHATTR_STYLE_DEEP, new SfxBoolItem (SCHATTR_STYLE_DEEP, false), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_STYLE_3D, new SfxBoolItem (SCHATTR_STYLE_3D, false), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_STYLE_VERTICAL, new SfxBoolItem (SCHATTR_STYLE_VERTICAL, false), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_STYLE_BASETYPE, new SfxInt32Item(SCHATTR_STYLE_BASETYPE, 0), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_STYLE_LINES, new SfxBoolItem (SCHATTR_STYLE_LINES, false), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_STYLE_PERCENT, new SfxBoolItem (SCHATTR_STYLE_PERCENT, false), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_STYLE_STACKED, new SfxBoolItem (SCHATTR_STYLE_STACKED, false), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_STYLE_SPLINES, new SfxInt32Item (SCHATTR_STYLE_SPLINES, 0), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_STYLE_SYMBOL, new SfxInt32Item (SCHATTR_STYLE_SYMBOL, 0), SID_ATTR_SYMBOLTYPE, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_STYLE_SHAPE, new SfxInt32Item (SCHATTR_STYLE_SHAPE, 0), 0, SFX_ITEMINFOFLAG_NONE },

            { SCHATTR_AXIS, new SfxInt32Item(SCHATTR_AXIS,2), 0, SFX_ITEMINFOFLAG_NONE },

            //axis scale
            { SCHATTR_AXISTYPE, new SfxInt32Item(SCHATTR_AXISTYPE, CHART_AXIS_REALNUMBER), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_AXIS_REVERSE, new SfxBoolItem(SCHATTR_AXIS_REVERSE,false), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_AXIS_AUTO_MIN, new SfxBoolItem(SCHATTR_AXIS_AUTO_MIN), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_AXIS_MIN, new SvxDoubleItem(0.0, SCHATTR_AXIS_MIN), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_AXIS_AUTO_MAX, new SfxBoolItem(SCHATTR_AXIS_AUTO_MAX), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_AXIS_MAX, new SvxDoubleItem(0.0, SCHATTR_AXIS_MAX), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_AXIS_AUTO_STEP_MAIN, new SfxBoolItem(SCHATTR_AXIS_AUTO_STEP_MAIN), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_AXIS_STEP_MAIN, new SvxDoubleItem(0.0, SCHATTR_AXIS_STEP_MAIN), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_AXIS_MAIN_TIME_UNIT, new SfxInt32Item(SCHATTR_AXIS_MAIN_TIME_UNIT,2), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_AXIS_AUTO_STEP_HELP, new SfxBoolItem(SCHATTR_AXIS_AUTO_STEP_HELP), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_AXIS_STEP_HELP, new SfxInt32Item(SCHATTR_AXIS_STEP_HELP,0), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_AXIS_HELP_TIME_UNIT, new SfxInt32Item(SCHATTR_AXIS_HELP_TIME_UNIT,2), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_AXIS_AUTO_TIME_RESOLUTION, new SfxBoolItem(SCHATTR_AXIS_AUTO_TIME_RESOLUTION), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_AXIS_TIME_RESOLUTION, new SfxInt32Item(SCHATTR_AXIS_TIME_RESOLUTION,2), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_AXIS_LOGARITHM, new SfxBoolItem(SCHATTR_AXIS_LOGARITHM), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_AXIS_AUTO_DATEAXIS, new SfxBoolItem(SCHATTR_AXIS_AUTO_DATEAXIS), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_AXIS_ALLOW_DATEAXIS, new SfxBoolItem(SCHATTR_AXIS_ALLOW_DATEAXIS), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_AXIS_AUTO_ORIGIN, new SfxBoolItem(SCHATTR_AXIS_AUTO_ORIGIN), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_AXIS_ORIGIN, new SvxDoubleItem(0.0, SCHATTR_AXIS_ORIGIN), 0, SFX_ITEMINFOFLAG_NONE },

            //axis position
            { SCHATTR_AXIS_TICKS, new SfxInt32Item(SCHATTR_AXIS_TICKS,CHAXIS_MARK_OUTER), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_AXIS_HELPTICKS, new SfxInt32Item(SCHATTR_AXIS_HELPTICKS,0), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_AXIS_POSITION, new SfxInt32Item(SCHATTR_AXIS_POSITION,0), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_AXIS_POSITION_VALUE, new SvxDoubleItem(0.0, SCHATTR_AXIS_POSITION_VALUE), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_AXIS_CROSSING_MAIN_AXIS_NUMBERFORMAT, new SfxUInt32Item(SCHATTR_AXIS_CROSSING_MAIN_AXIS_NUMBERFORMAT,0), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_AXIS_SHIFTED_CATEGORY_POSITION, new SfxBoolItem(SCHATTR_AXIS_SHIFTED_CATEGORY_POSITION,false), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_AXIS_LABEL_POSITION, new SfxInt32Item(SCHATTR_AXIS_LABEL_POSITION,0), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_AXIS_MARK_POSITION, new SfxInt32Item(SCHATTR_AXIS_MARK_POSITION,0), 0, SFX_ITEMINFOFLAG_NONE },

            //axis label
            { SCHATTR_AXIS_SHOWDESCR, new SfxBoolItem(SCHATTR_AXIS_SHOWDESCR,false), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_AXIS_LABEL_ORDER, new SvxChartTextOrderItem(SvxChartTextOrder::SideBySide, SCHATTR_AXIS_LABEL_ORDER), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_AXIS_LABEL_OVERLAP, new SfxBoolItem(SCHATTR_AXIS_LABEL_OVERLAP,false), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_AXIS_LABEL_BREAK, new SfxBoolItem(SCHATTR_AXIS_LABEL_BREAK, false ), 0, SFX_ITEMINFOFLAG_NONE },

            { SCHATTR_SYMBOL_BRUSH, new SvxBrushItem(SCHATTR_SYMBOL_BRUSH), SID_ATTR_BRUSH, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_STOCK_VOLUME, new SfxBoolItem(SCHATTR_STOCK_VOLUME,false), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_STOCK_UPDOWN, new SfxBoolItem(SCHATTR_STOCK_UPDOWN,false), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_SYMBOL_SIZE, new SvxSizeItem(SCHATTR_SYMBOL_SIZE,Size(0,0)), SID_ATTR_SYMBOLSIZE, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_HIDE_DATA_POINT_LEGEND_ENTRY, new SfxBoolItem(SCHATTR_HIDE_DATA_POINT_LEGEND_ENTRY, false), 0, SFX_ITEMINFOFLAG_NONE },

            // new for New Chart
            { SCHATTR_BAR_OVERLAP, new SfxInt32Item(SCHATTR_BAR_OVERLAP,0), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_BAR_GAPWIDTH, new SfxInt32Item(SCHATTR_BAR_GAPWIDTH,0), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_BAR_CONNECT, new SfxBoolItem(SCHATTR_BAR_CONNECT, false), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_NUM_OF_LINES_FOR_BAR, new SfxInt32Item( SCHATTR_NUM_OF_LINES_FOR_BAR, 0 ), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_SPLINE_ORDER, new SfxInt32Item( SCHATTR_SPLINE_ORDER, 3 ), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_SPLINE_RESOLUTION, new SfxInt32Item( SCHATTR_SPLINE_RESOLUTION, 20 ), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_GROUP_BARS_PER_AXIS, new SfxBoolItem(SCHATTR_GROUP_BARS_PER_AXIS, false), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_STARTING_ANGLE, new SdrAngleItem( SCHATTR_STARTING_ANGLE, 9000_deg100 ), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_CLOCKWISE, new SfxBoolItem( SCHATTR_CLOCKWISE, false ), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_MISSING_VALUE_TREATMENT, new SfxInt32Item(SCHATTR_MISSING_VALUE_TREATMENT, 0), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_AVAILABLE_MISSING_VALUE_TREATMENTS, new SfxIntegerListItem(SCHATTR_AVAILABLE_MISSING_VALUE_TREATMENTS, std::vector < sal_Int32 >() ), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_INCLUDE_HIDDEN_CELLS, new SfxBoolItem(SCHATTR_INCLUDE_HIDDEN_CELLS, true), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_HIDE_LEGEND_ENTRY, new SfxBoolItem(SCHATTR_HIDE_LEGEND_ENTRY, false), 0, SFX_ITEMINFOFLAG_NONE },

            { SCHATTR_AXIS_FOR_ALL_SERIES, new SfxInt32Item(SCHATTR_AXIS_FOR_ALL_SERIES, 0), 0, SFX_ITEMINFOFLAG_NONE },

            { SCHATTR_REGRESSION_TYPE, new SvxChartRegressItem  (SvxChartRegress::NONE, SCHATTR_REGRESSION_TYPE), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_REGRESSION_SHOW_EQUATION, new SfxBoolItem(SCHATTR_REGRESSION_SHOW_EQUATION, false), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_REGRESSION_SHOW_COEFF, new SfxBoolItem(SCHATTR_REGRESSION_SHOW_COEFF, false), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_REGRESSION_DEGREE, new SfxInt32Item(SCHATTR_REGRESSION_DEGREE, 2), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_REGRESSION_PERIOD, new SfxInt32Item(SCHATTR_REGRESSION_PERIOD, 2), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_REGRESSION_EXTRAPOLATE_FORWARD, new SvxDoubleItem(0.0, SCHATTR_REGRESSION_EXTRAPOLATE_FORWARD), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_REGRESSION_EXTRAPOLATE_BACKWARD, new SvxDoubleItem(0.0, SCHATTR_REGRESSION_EXTRAPOLATE_BACKWARD), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_REGRESSION_SET_INTERCEPT, new SfxBoolItem(SCHATTR_REGRESSION_SET_INTERCEPT, false), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_REGRESSION_INTERCEPT_VALUE, new SvxDoubleItem(0.0, SCHATTR_REGRESSION_INTERCEPT_VALUE), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_REGRESSION_CURVE_NAME, new SfxStringItem(SCHATTR_REGRESSION_CURVE_NAME, OUString()), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_REGRESSION_XNAME, new SfxStringItem(SCHATTR_REGRESSION_XNAME, u"x"_ustr), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_REGRESSION_YNAME, new SfxStringItem(SCHATTR_REGRESSION_YNAME, u"f(x)"_ustr), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_REGRESSION_MOVING_TYPE, new SfxInt32Item(SCHATTR_REGRESSION_MOVING_TYPE, css::chart2::MovingAverageType::Prior), 0, SFX_ITEMINFOFLAG_NONE },

            { SCHATTR_DATA_TABLE_HORIZONTAL_BORDER, new SfxBoolItem(SCHATTR_DATA_TABLE_HORIZONTAL_BORDER, false), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_DATA_TABLE_VERTICAL_BORDER, new SfxBoolItem(SCHATTR_DATA_TABLE_VERTICAL_BORDER, false), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_DATA_TABLE_OUTLINE, new SfxBoolItem(SCHATTR_DATA_TABLE_OUTLINE, false), 0, SFX_ITEMINFOFLAG_NONE },
            { SCHATTR_DATA_TABLE_KEYS, new SfxBoolItem(SCHATTR_DATA_TABLE_KEYS, false), 0, SFX_ITEMINFOFLAG_NONE }
        }};

        virtual const ItemInfoStatic& getItemInfoStatic(size_t nIndex) const override { return maItemInfos[nIndex]; }

    public:
        virtual size_t size() const override { return maItemInfos.size(); }
        virtual const ItemInfo& getItemInfo(size_t nIndex, SfxItemPool& /*rPool*/) override { return maItemInfos[nIndex]; }
    };

    static std::unique_ptr<ItemInfoPackageChart> g_aItemInfoPackageChart;
    if (!g_aItemInfoPackageChart)
        g_aItemInfoPackageChart.reset(new ItemInfoPackageChart);
    return *g_aItemInfoPackageChart;
}

ChartItemPool::ChartItemPool()
: SfxItemPool(u"ChartItemPool"_ustr)
{
    registerItemInfoPackage(getItemInfoPackageChart());
}

ChartItemPool::ChartItemPool(const ChartItemPool& rPool)
: SfxItemPool(rPool)
{
}

ChartItemPool::~ChartItemPool()
{
    sendShutdownHint();
}

rtl::Reference<SfxItemPool> ChartItemPool::Clone() const
{
    return new ChartItemPool(*this);
}

MapUnit ChartItemPool::GetMetric(sal_uInt16 /* nWhich */) const
{
    return MapUnit::Map100thMM;
}

rtl::Reference<SfxItemPool> ChartItemPool::CreateChartItemPool()
{
    // There are various default values which want to call
    // OutputDevice::GetDefaultFont. Unfortunately, when processing
    // UNO methods which may get called from out of process, this
    // happens on a thread that does not take the SolarMutex, which
    // causes trouble in ImplFontCache.
    // Trying to take the SolarMutex when initialising these default
    // leads to ABBA deadlocks.
    // So rather just trigger the initialisation of these things here.
    StaticDataSeriesDefaults();
    StaticAxisDefaults();
    StaticLegendDefaults();
    StaticFormattedStringDefaults();

    return new ChartItemPool();
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
