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
#ifndef INCLUDED_CHART2_SOURCE_INC_CHARTVIEW_CHARTSFXITEMIDS_HXX
#define INCLUDED_CHART2_SOURCE_INC_CHARTVIEW_CHARTSFXITEMIDS_HXX

// SCHATTR

// can't this be changed to 0?
#define SCHATTR_START                       1

#define SCHATTR_DATADESCR_START             SCHATTR_START
#define SCHATTR_DATADESCR_SHOW_NUMBER       TypedWhichId<SfxBoolItem>(SCHATTR_DATADESCR_START)
#define SCHATTR_DATADESCR_SHOW_PERCENTAGE   TypedWhichId<SfxBoolItem>(SCHATTR_DATADESCR_START + 1)
#define SCHATTR_DATADESCR_SHOW_CATEGORY     TypedWhichId<SfxBoolItem>(SCHATTR_DATADESCR_START + 2)
#define SCHATTR_DATADESCR_SHOW_SYMBOL       TypedWhichId<SfxBoolItem>(SCHATTR_DATADESCR_START + 3)
#define SCHATTR_DATADESCR_WRAP_TEXT         TypedWhichId<SfxBoolItem>(SCHATTR_DATADESCR_START + 4)
#define SCHATTR_DATADESCR_SEPARATOR         TypedWhichId<SfxStringItem>(SCHATTR_DATADESCR_START + 5)
#define SCHATTR_DATADESCR_PLACEMENT         TypedWhichId<SfxInt32Item>(SCHATTR_DATADESCR_START + 6)
#define SCHATTR_DATADESCR_AVAILABLE_PLACEMENTS   TypedWhichId<SfxIntegerListItem>(SCHATTR_DATADESCR_START + 7)
#define SCHATTR_DATADESCR_NO_PERCENTVALUE   TypedWhichId<SfxBoolItem>(SCHATTR_DATADESCR_START + 8) //percentage values should not be offered
#define SCHATTR_PERCENT_NUMBERFORMAT_VALUE  TypedWhichId<SfxUInt32Item>(SCHATTR_DATADESCR_START + 9)
#define SCHATTR_PERCENT_NUMBERFORMAT_SOURCE TypedWhichId<SfxBoolItem>(SCHATTR_DATADESCR_START + 10)
#define SCHATTR_DATADESCR_END               SCHATTR_PERCENT_NUMBERFORMAT_SOURCE

//legend
#define SCHATTR_LEGEND_START                (SCHATTR_DATADESCR_END + 1)
#define SCHATTR_LEGEND_POS                  TypedWhichId<SfxInt32Item>(SCHATTR_LEGEND_START)
#define SCHATTR_LEGEND_SHOW                 TypedWhichId<SfxBoolItem>(SCHATTR_LEGEND_START + 1)
#define SCHATTR_LEGEND_END                  SCHATTR_LEGEND_SHOW

//text
#define SCHATTR_TEXT_START                  (SCHATTR_LEGEND_END + 1)
#define SCHATTR_TEXT_DEGREES                TypedWhichId<SfxInt32Item>(SCHATTR_TEXT_START)
#define SCHATTR_TEXT_STACKED                TypedWhichId<SfxBoolItem>(SCHATTR_TEXT_START + 1)
#define SCHATTR_TEXT_END                    SCHATTR_TEXT_STACKED

// statistic
#define SCHATTR_STAT_START                  (SCHATTR_TEXT_END + 1)
#define SCHATTR_STAT_AVERAGE                TypedWhichId<SfxBoolItem>(SCHATTR_STAT_START)
#define SCHATTR_STAT_KIND_ERROR             TypedWhichId<SvxChartKindErrorItem>(SCHATTR_STAT_START + 1)
#define SCHATTR_STAT_PERCENT                TypedWhichId<SvxDoubleItem>(SCHATTR_STAT_START + 2)
#define SCHATTR_STAT_BIGERROR               TypedWhichId<SvxDoubleItem>(SCHATTR_STAT_START + 3)
#define SCHATTR_STAT_CONSTPLUS              TypedWhichId<SvxDoubleItem>(SCHATTR_STAT_START + 4)
#define SCHATTR_STAT_CONSTMINUS             TypedWhichId<SvxDoubleItem>(SCHATTR_STAT_START + 5)
#define SCHATTR_STAT_INDICATE               TypedWhichId<SvxChartIndicateItem>(SCHATTR_STAT_START + 6)
#define SCHATTR_STAT_RANGE_POS              TypedWhichId<SfxStringItem>(SCHATTR_STAT_START + 7)
#define SCHATTR_STAT_RANGE_NEG              TypedWhichId<SfxStringItem>(SCHATTR_STAT_START + 8)
#define SCHATTR_STAT_ERRORBAR_TYPE          TypedWhichId<SfxBoolItem>(SCHATTR_STAT_START + 9)
#define SCHATTR_STAT_END                    SCHATTR_STAT_ERRORBAR_TYPE

// these attributes are for replacement of enum eChartStyle

#define SCHATTR_STYLE_START             ( SCHATTR_STAT_END +1 )

// for whole chart
#define SCHATTR_STYLE_DEEP              TypedWhichId<SfxBoolItem>( SCHATTR_STYLE_START     )
#define SCHATTR_STYLE_3D                TypedWhichId<SfxBoolItem>( SCHATTR_STYLE_START + 1 )
#define SCHATTR_STYLE_VERTICAL          TypedWhichId<SfxBoolItem>( SCHATTR_STYLE_START + 2 )

// also for series
#define SCHATTR_STYLE_BASETYPE          TypedWhichId<SfxInt32Item>( SCHATTR_STYLE_START + 3 )// Line,Area,...,Pie
#define SCHATTR_STYLE_LINES             TypedWhichId<SfxBoolItem>( SCHATTR_STYLE_START + 4 )// draw line
#define SCHATTR_STYLE_PERCENT           TypedWhichId<SfxBoolItem>( SCHATTR_STYLE_START + 5 )
#define SCHATTR_STYLE_STACKED           TypedWhichId<SfxBoolItem>( SCHATTR_STYLE_START + 6 )
#define SCHATTR_STYLE_SPLINES           TypedWhichId<SfxInt32Item>( SCHATTR_STYLE_START + 7 )

// also for data point
#define SCHATTR_STYLE_SYMBOL            TypedWhichId<SfxInt32Item>( SCHATTR_STYLE_START + 8 )
#define SCHATTR_STYLE_SHAPE             TypedWhichId<SfxInt32Item>( SCHATTR_STYLE_START + 9 )
#define SCHATTR_STYLE_END               ( SCHATTR_STYLE_SHAPE )

#define SCHATTR_AXIS                    TypedWhichId<SfxInt32Item>(SCHATTR_STYLE_END + 1)// see chtmodel.hxx defines CHART_AXIS_PRIMARY_X, etc.
//Re-mapped:
#define SCHATTR_AXIS_START              (SCHATTR_AXIS + 1)
//axis scale
#define SCHATTR_AXISTYPE                TypedWhichId<SfxInt32Item>(SCHATTR_AXIS_START)
#define SCHATTR_AXIS_REVERSE            TypedWhichId<SfxBoolItem>(SCHATTR_AXIS_START + 1)
#define SCHATTR_AXIS_AUTO_MIN           TypedWhichId<SfxBoolItem>(SCHATTR_AXIS_START + 2)
#define SCHATTR_AXIS_MIN                TypedWhichId<SvxDoubleItem>(SCHATTR_AXIS_START + 3)
#define SCHATTR_AXIS_AUTO_MAX           TypedWhichId<SfxBoolItem>(SCHATTR_AXIS_START + 4)
#define SCHATTR_AXIS_MAX                TypedWhichId<SvxDoubleItem>(SCHATTR_AXIS_START + 5)
#define SCHATTR_AXIS_AUTO_STEP_MAIN     TypedWhichId<SfxBoolItem>(SCHATTR_AXIS_START + 6)
#define SCHATTR_AXIS_STEP_MAIN          TypedWhichId<SvxDoubleItem>(SCHATTR_AXIS_START + 7)
#define SCHATTR_AXIS_MAIN_TIME_UNIT     TypedWhichId<SfxInt32Item>(SCHATTR_AXIS_START + 8)
#define SCHATTR_AXIS_AUTO_STEP_HELP     TypedWhichId<SfxBoolItem>(SCHATTR_AXIS_START + 9)
#define SCHATTR_AXIS_STEP_HELP          TypedWhichId<SfxInt32Item>(SCHATTR_AXIS_START + 10)
#define SCHATTR_AXIS_HELP_TIME_UNIT     TypedWhichId<SfxInt32Item>(SCHATTR_AXIS_START + 11)
#define SCHATTR_AXIS_AUTO_TIME_RESOLUTION   TypedWhichId<SfxBoolItem>(SCHATTR_AXIS_START + 12)
#define SCHATTR_AXIS_TIME_RESOLUTION    TypedWhichId<SfxInt32Item>(SCHATTR_AXIS_START + 13)
#define SCHATTR_AXIS_LOGARITHM          TypedWhichId<SfxBoolItem>(SCHATTR_AXIS_START + 14)
#define SCHATTR_AXIS_AUTO_DATEAXIS      TypedWhichId<SfxBoolItem>(SCHATTR_AXIS_START + 15)
#define SCHATTR_AXIS_ALLOW_DATEAXIS     TypedWhichId<SfxBoolItem>(SCHATTR_AXIS_START + 16)
#define SCHATTR_AXIS_AUTO_ORIGIN        TypedWhichId<SfxBoolItem>(SCHATTR_AXIS_START + 17)
#define SCHATTR_AXIS_ORIGIN             TypedWhichId<SvxDoubleItem>(SCHATTR_AXIS_START + 18)
//axis position
#define SCHATTR_AXIS_POSITION_START     (SCHATTR_AXIS_ORIGIN +1)
#define SCHATTR_AXIS_TICKS              TypedWhichId<SfxInt32Item>(SCHATTR_AXIS_POSITION_START)
#define SCHATTR_AXIS_HELPTICKS          TypedWhichId<SfxInt32Item>(SCHATTR_AXIS_POSITION_START + 1)
#define SCHATTR_AXIS_POSITION           TypedWhichId<SfxInt32Item>(SCHATTR_AXIS_POSITION_START + 2)
#define SCHATTR_AXIS_POSITION_VALUE     TypedWhichId<SvxDoubleItem>(SCHATTR_AXIS_POSITION_START + 3)
#define SCHATTR_AXIS_CROSSING_MAIN_AXIS_NUMBERFORMAT    TypedWhichId<SfxUInt32Item>(SCHATTR_AXIS_POSITION_START + 4)
#define SCHATTR_AXIS_LABEL_POSITION     TypedWhichId<SfxInt32Item>(SCHATTR_AXIS_POSITION_START + 5)
#define SCHATTR_AXIS_MARK_POSITION      TypedWhichId<SfxInt32Item>(SCHATTR_AXIS_POSITION_START + 6)
//axis label
#define SCHATTR_AXIS_LABEL_START        (SCHATTR_AXIS_MARK_POSITION +1)
#define SCHATTR_AXIS_SHOWDESCR          TypedWhichId<SfxBoolItem>(SCHATTR_AXIS_LABEL_START)
#define SCHATTR_AXIS_LABEL_ORDER        TypedWhichId<SvxChartTextOrderItem>(SCHATTR_AXIS_LABEL_START + 1)
#define SCHATTR_AXIS_LABEL_OVERLAP      TypedWhichId<SfxBoolItem>(SCHATTR_AXIS_LABEL_START + 2)
#define SCHATTR_AXIS_LABEL_BREAK        TypedWhichId<SfxBoolItem>(SCHATTR_AXIS_LABEL_START + 3)
#define SCHATTR_AXIS_LABEL_END          SCHATTR_AXIS_LABEL_BREAK

#define SCHATTR_AXIS_END                SCHATTR_AXIS_LABEL_END

#define SCHATTR_SYMBOL_BRUSH            TypedWhichId<SvxBrushItem>(SCHATTR_AXIS_END + 1)
#define SCHATTR_STOCK_VOLUME            TypedWhichId<SfxBoolItem>(SCHATTR_AXIS_END + 2)
#define SCHATTR_STOCK_UPDOWN            TypedWhichId<SfxBoolItem>(SCHATTR_AXIS_END + 3)
#define SCHATTR_SYMBOL_SIZE             TypedWhichId<SvxSizeItem>(SCHATTR_AXIS_END + 4)

// non persistent items (binary format)
#define SCHATTR_CHARTTYPE_START         (SCHATTR_SYMBOL_SIZE + 1)

// new from New Chart
#define SCHATTR_BAR_OVERLAP             TypedWhichId<SfxInt32Item>(SCHATTR_CHARTTYPE_START )
#define SCHATTR_BAR_GAPWIDTH            TypedWhichId<SfxInt32Item>(SCHATTR_CHARTTYPE_START + 1)
#define SCHATTR_BAR_CONNECT             TypedWhichId<SfxBoolItem>(SCHATTR_CHARTTYPE_START + 2)
#define SCHATTR_NUM_OF_LINES_FOR_BAR    TypedWhichId<SfxInt32Item>(SCHATTR_CHARTTYPE_START + 3)
#define SCHATTR_SPLINE_ORDER            TypedWhichId<SfxInt32Item>(SCHATTR_CHARTTYPE_START + 4)
#define SCHATTR_SPLINE_RESOLUTION       TypedWhichId<SfxInt32Item>(SCHATTR_CHARTTYPE_START + 5)
#define SCHATTR_GROUP_BARS_PER_AXIS     TypedWhichId<SfxBoolItem>(SCHATTR_CHARTTYPE_START + 6)
#define SCHATTR_STARTING_ANGLE          TypedWhichId<SfxInt32Item>(SCHATTR_CHARTTYPE_START + 7)
#define SCHATTR_CLOCKWISE               TypedWhichId<SfxBoolItem>(SCHATTR_CHARTTYPE_START + 8)
#define SCHATTR_MISSING_VALUE_TREATMENT     TypedWhichId<SfxInt32Item>(SCHATTR_CHARTTYPE_START + 9)
#define SCHATTR_AVAILABLE_MISSING_VALUE_TREATMENTS TypedWhichId<SfxIntegerListItem>(SCHATTR_CHARTTYPE_START + 10)
#define SCHATTR_INCLUDE_HIDDEN_CELLS    TypedWhichId<SfxBoolItem>(SCHATTR_CHARTTYPE_START + 11)

#define SCHATTR_CHARTTYPE_END           SCHATTR_INCLUDE_HIDDEN_CELLS

// items for transporting information to dialogs
#define SCHATTR_MISC_START              (SCHATTR_CHARTTYPE_END + 1)
#define SCHATTR_AXIS_FOR_ALL_SERIES     TypedWhichId<SfxInt32Item>(SCHATTR_MISC_START)
#define SCHATTR_MISC_END                 SCHATTR_AXIS_FOR_ALL_SERIES

// regression curve
#define SCHATTR_REGRESSION_START                (SCHATTR_MISC_END + 1)
#define SCHATTR_REGRESSION_TYPE                 TypedWhichId<SvxChartRegressItem>(SCHATTR_REGRESSION_START)
#define SCHATTR_REGRESSION_SHOW_EQUATION        TypedWhichId<SfxBoolItem>(SCHATTR_REGRESSION_START + 1)
#define SCHATTR_REGRESSION_SHOW_COEFF           TypedWhichId<SfxBoolItem>(SCHATTR_REGRESSION_START + 2)
#define SCHATTR_REGRESSION_DEGREE               TypedWhichId<SfxInt32Item>(SCHATTR_REGRESSION_START + 3)
#define SCHATTR_REGRESSION_PERIOD               TypedWhichId<SfxInt32Item>(SCHATTR_REGRESSION_START + 4)
#define SCHATTR_REGRESSION_EXTRAPOLATE_FORWARD  TypedWhichId<SvxDoubleItem>(SCHATTR_REGRESSION_START + 5)
#define SCHATTR_REGRESSION_EXTRAPOLATE_BACKWARD TypedWhichId<SvxDoubleItem>(SCHATTR_REGRESSION_START + 6)
#define SCHATTR_REGRESSION_SET_INTERCEPT        TypedWhichId<SfxBoolItem>(SCHATTR_REGRESSION_START + 7)
#define SCHATTR_REGRESSION_INTERCEPT_VALUE      TypedWhichId<SvxDoubleItem>(SCHATTR_REGRESSION_START + 8)
#define SCHATTR_REGRESSION_CURVE_NAME           TypedWhichId<SfxStringItem>(SCHATTR_REGRESSION_START + 9)
#define SCHATTR_REGRESSION_XNAME                TypedWhichId<SfxStringItem>(SCHATTR_REGRESSION_START + 10)
#define SCHATTR_REGRESSION_YNAME                TypedWhichId<SfxStringItem>(SCHATTR_REGRESSION_START + 11)
#define SCHATTR_REGRESSION_END                   SCHATTR_REGRESSION_YNAME

#define SCHATTR_END                     SCHATTR_REGRESSION_END

// values for Items

// values for SCHATTR_AXIS_TICKS and SCHATTR_AXIS_HELPTICKS items
#define CHAXIS_MARK_BOTH   3
#define CHAXIS_MARK_OUTER  2
#define CHAXIS_MARK_INNER  1
#define CHAXIS_MARK_NONE   0

// values for SCHATTR_AXISTYPE items
#define CHART_AXIS_REALNUMBER   0
#define CHART_AXIS_PERCENT      1
#define CHART_AXIS_CATEGORY     2
#define CHART_AXIS_SERIES       3
#define CHART_AXIS_DATE         4

// values for SCHATTR_STYLE_SHAPE items
#define CHART_SHAPE3D_IGNORE  -2 //internal! (GetChartShapeStyle()!)
#define CHART_SHAPE3D_ANY     -1 //undefined type (GetChartShapeStyle()!)
#define CHART_SHAPE3D_SQUARE   0
#define CHART_SHAPE3D_CYLINDER 1
#define CHART_SHAPE3D_CONE     2
#define CHART_SHAPE3D_PYRAMID  3 //reserved
#define CHART_SHAPE3D_HANOI    4

// values for SCHATTR_AXIS items
#define CHART_AXIS_PRIMARY_X    1
#define CHART_AXIS_PRIMARY_Y    2
#define CHART_AXIS_PRIMARY_Z    3
#define CHART_AXIS_SECONDARY_Y  4
#define CHART_AXIS_SECONDARY_X  5

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
