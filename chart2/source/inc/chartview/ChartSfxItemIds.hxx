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
#pragma once

#include <svl/typedwhich.hxx>

class SvxSizeItem;
class SfxIntegerListItem;
class SfxBoolItem;
class SfxStringItem;
class SfxInt32Item;
class SfxUInt32Item;
class SvxChartIndicateItem;
class SvxDoubleItem;
class SvxBrushItem;
class SvxChartKindErrorItem;
class SvxChartTextOrderItem;
class SvxChartRegressItem;
class SdrAngleItem;
class SvxChartColorPaletteItem;

// SCHATTR

// can't this be changed to 0?
inline constexpr sal_uInt16 SCHATTR_START = 1;

inline constexpr sal_uInt16                       SCHATTR_DATADESCR_START                (SCHATTR_START);
inline constexpr TypedWhichId<SfxBoolItem>        SCHATTR_DATADESCR_SHOW_NUMBER          (SCHATTR_DATADESCR_START);
inline constexpr TypedWhichId<SfxBoolItem>        SCHATTR_DATADESCR_SHOW_PERCENTAGE      (SCHATTR_DATADESCR_START + 1);
inline constexpr TypedWhichId<SfxBoolItem>        SCHATTR_DATADESCR_SHOW_CATEGORY        (SCHATTR_DATADESCR_START + 2);
inline constexpr TypedWhichId<SfxBoolItem>        SCHATTR_DATADESCR_SHOW_SYMBOL          (SCHATTR_DATADESCR_START + 3);
inline constexpr TypedWhichId<SfxBoolItem>        SCHATTR_DATADESCR_WRAP_TEXT            (SCHATTR_DATADESCR_START + 4);
inline constexpr TypedWhichId<SfxStringItem>      SCHATTR_DATADESCR_SEPARATOR            (SCHATTR_DATADESCR_START + 5);
inline constexpr TypedWhichId<SfxInt32Item>       SCHATTR_DATADESCR_PLACEMENT            (SCHATTR_DATADESCR_START + 6);
inline constexpr TypedWhichId<SfxIntegerListItem> SCHATTR_DATADESCR_AVAILABLE_PLACEMENTS (SCHATTR_DATADESCR_START + 7);
inline constexpr TypedWhichId<SfxBoolItem>        SCHATTR_DATADESCR_NO_PERCENTVALUE      (SCHATTR_DATADESCR_START + 8); //percentage values should not be offered
inline constexpr TypedWhichId<SfxBoolItem>        SCHATTR_DATADESCR_CUSTOM_LEADER_LINES  (SCHATTR_DATADESCR_START + 9);
inline constexpr TypedWhichId<SfxUInt32Item>      SCHATTR_PERCENT_NUMBERFORMAT_VALUE     (SCHATTR_DATADESCR_START + 10);
inline constexpr TypedWhichId<SfxBoolItem>        SCHATTR_PERCENT_NUMBERFORMAT_SOURCE    (SCHATTR_DATADESCR_START + 11);
inline constexpr TypedWhichId<SfxBoolItem>        SCHATTR_DATADESCR_SHOW_DATA_SERIES_NAME (SCHATTR_DATADESCR_START + 12);
inline constexpr sal_uInt16                       SCHATTR_DATADESCR_END                  (SCHATTR_DATADESCR_SHOW_DATA_SERIES_NAME);

//legend
inline constexpr sal_uInt16                 SCHATTR_LEGEND_START      (SCHATTR_DATADESCR_END + 1);
inline constexpr TypedWhichId<SfxInt32Item> SCHATTR_LEGEND_POS        (SCHATTR_LEGEND_START);
inline constexpr TypedWhichId<SfxBoolItem>  SCHATTR_LEGEND_SHOW       (SCHATTR_LEGEND_START + 1);
inline constexpr TypedWhichId<SfxBoolItem>  SCHATTR_LEGEND_NO_OVERLAY (SCHATTR_LEGEND_START + 2);
inline constexpr sal_uInt16                 SCHATTR_LEGEND_END        (SCHATTR_LEGEND_NO_OVERLAY);

//text
inline constexpr sal_uInt16                 SCHATTR_TEXT_START   (SCHATTR_LEGEND_END + 1);
inline constexpr TypedWhichId<SdrAngleItem> SCHATTR_TEXT_DEGREES (SCHATTR_TEXT_START);
inline constexpr TypedWhichId<SfxBoolItem>  SCHATTR_TEXT_STACKED (SCHATTR_TEXT_START + 1);
inline constexpr sal_uInt16                 SCHATTR_TEXT_END     (SCHATTR_TEXT_STACKED);

// statistic
inline constexpr sal_uInt16                          SCHATTR_STAT_START         (SCHATTR_TEXT_END + 1);
inline constexpr TypedWhichId<SfxBoolItem>           SCHATTR_STAT_AVERAGE       (SCHATTR_STAT_START);
inline constexpr TypedWhichId<SvxChartKindErrorItem> SCHATTR_STAT_KIND_ERROR    (SCHATTR_STAT_START + 1);
inline constexpr TypedWhichId<SvxDoubleItem>         SCHATTR_STAT_PERCENT       (SCHATTR_STAT_START + 2);
inline constexpr TypedWhichId<SvxDoubleItem>         SCHATTR_STAT_BIGERROR      (SCHATTR_STAT_START + 3);
inline constexpr TypedWhichId<SvxDoubleItem>         SCHATTR_STAT_CONSTPLUS     (SCHATTR_STAT_START + 4);
inline constexpr TypedWhichId<SvxDoubleItem>         SCHATTR_STAT_CONSTMINUS    (SCHATTR_STAT_START + 5);
inline constexpr TypedWhichId<SvxChartIndicateItem>  SCHATTR_STAT_INDICATE      (SCHATTR_STAT_START + 6);
inline constexpr TypedWhichId<SfxStringItem>         SCHATTR_STAT_RANGE_POS     (SCHATTR_STAT_START + 7);
inline constexpr TypedWhichId<SfxStringItem>         SCHATTR_STAT_RANGE_NEG     (SCHATTR_STAT_START + 8);
inline constexpr TypedWhichId<SfxBoolItem>           SCHATTR_STAT_ERRORBAR_TYPE (SCHATTR_STAT_START + 9);
inline constexpr sal_uInt16                          SCHATTR_STAT_END           (SCHATTR_STAT_ERRORBAR_TYPE);

// these attributes are for replacement of enum eChartStyle

inline constexpr sal_uInt16                 SCHATTR_STYLE_START    (SCHATTR_STAT_END    + 1);

// for whole chart
inline constexpr TypedWhichId<SfxBoolItem>  SCHATTR_STYLE_DEEP     (SCHATTR_STYLE_START    );
inline constexpr TypedWhichId<SfxBoolItem>  SCHATTR_STYLE_3D       (SCHATTR_STYLE_START + 1);
inline constexpr TypedWhichId<SfxBoolItem>  SCHATTR_STYLE_VERTICAL (SCHATTR_STYLE_START + 2);

// also for series
inline constexpr TypedWhichId<SfxInt32Item> SCHATTR_STYLE_BASETYPE (SCHATTR_STYLE_START + 3);// Line,Area,...,Pie
inline constexpr TypedWhichId<SfxBoolItem>  SCHATTR_STYLE_LINES    (SCHATTR_STYLE_START + 4);// draw line
inline constexpr TypedWhichId<SfxBoolItem>  SCHATTR_STYLE_PERCENT  (SCHATTR_STYLE_START + 5);
inline constexpr TypedWhichId<SfxBoolItem>  SCHATTR_STYLE_STACKED  (SCHATTR_STYLE_START + 6);
inline constexpr TypedWhichId<SfxInt32Item> SCHATTR_STYLE_SPLINES  (SCHATTR_STYLE_START + 7);

// also for data point
inline constexpr TypedWhichId<SfxInt32Item> SCHATTR_STYLE_SYMBOL   (SCHATTR_STYLE_START + 8);
inline constexpr TypedWhichId<SfxInt32Item> SCHATTR_STYLE_SHAPE    (SCHATTR_STYLE_START + 9);
inline constexpr sal_uInt16                 SCHATTR_STYLE_END      (SCHATTR_STYLE_SHAPE    );

inline constexpr TypedWhichId<SfxInt32Item>          SCHATTR_AXIS                                 (SCHATTR_STYLE_END + 1); // see chtmodel.hxx defines CHART_AXIS_PRIMARY_X, etc.
//Re-mapped:
inline constexpr sal_uInt16                          SCHATTR_AXIS_START                           (SCHATTR_AXIS + 1);
//axis scale
inline constexpr TypedWhichId<SfxInt32Item>          SCHATTR_AXISTYPE                             (SCHATTR_AXIS_START);
inline constexpr TypedWhichId<SfxBoolItem>           SCHATTR_AXIS_REVERSE                         (SCHATTR_AXIS_START + 1);
inline constexpr TypedWhichId<SfxBoolItem>           SCHATTR_AXIS_AUTO_MIN                        (SCHATTR_AXIS_START + 2);
inline constexpr TypedWhichId<SvxDoubleItem>         SCHATTR_AXIS_MIN                             (SCHATTR_AXIS_START + 3);
inline constexpr TypedWhichId<SfxBoolItem>           SCHATTR_AXIS_AUTO_MAX                        (SCHATTR_AXIS_START + 4);
inline constexpr TypedWhichId<SvxDoubleItem>         SCHATTR_AXIS_MAX                             (SCHATTR_AXIS_START + 5);
inline constexpr TypedWhichId<SfxBoolItem>           SCHATTR_AXIS_AUTO_STEP_MAIN                  (SCHATTR_AXIS_START + 6);
inline constexpr TypedWhichId<SvxDoubleItem>         SCHATTR_AXIS_STEP_MAIN                       (SCHATTR_AXIS_START + 7);
inline constexpr TypedWhichId<SfxInt32Item>          SCHATTR_AXIS_MAIN_TIME_UNIT                  (SCHATTR_AXIS_START + 8);
inline constexpr TypedWhichId<SfxBoolItem>           SCHATTR_AXIS_AUTO_STEP_HELP                  (SCHATTR_AXIS_START + 9);
inline constexpr TypedWhichId<SfxInt32Item>          SCHATTR_AXIS_STEP_HELP                       (SCHATTR_AXIS_START + 10);
inline constexpr TypedWhichId<SfxInt32Item>          SCHATTR_AXIS_HELP_TIME_UNIT                  (SCHATTR_AXIS_START + 11);
inline constexpr TypedWhichId<SfxBoolItem>           SCHATTR_AXIS_AUTO_TIME_RESOLUTION            (SCHATTR_AXIS_START + 12);
inline constexpr TypedWhichId<SfxInt32Item>          SCHATTR_AXIS_TIME_RESOLUTION                 (SCHATTR_AXIS_START + 13);
inline constexpr TypedWhichId<SfxBoolItem>           SCHATTR_AXIS_LOGARITHM                       (SCHATTR_AXIS_START + 14);
inline constexpr TypedWhichId<SfxBoolItem>           SCHATTR_AXIS_AUTO_DATEAXIS                   (SCHATTR_AXIS_START + 15);
inline constexpr TypedWhichId<SfxBoolItem>           SCHATTR_AXIS_ALLOW_DATEAXIS                  (SCHATTR_AXIS_START + 16);
inline constexpr TypedWhichId<SfxBoolItem>           SCHATTR_AXIS_AUTO_ORIGIN                     (SCHATTR_AXIS_START + 17);
inline constexpr TypedWhichId<SvxDoubleItem>         SCHATTR_AXIS_ORIGIN                          (SCHATTR_AXIS_START + 18);
//axis position
inline constexpr sal_uInt16                          SCHATTR_AXIS_POSITION_START                  (SCHATTR_AXIS_ORIGIN + 1);
inline constexpr TypedWhichId<SfxInt32Item>          SCHATTR_AXIS_TICKS                           (SCHATTR_AXIS_POSITION_START);
inline constexpr TypedWhichId<SfxInt32Item>          SCHATTR_AXIS_HELPTICKS                       (SCHATTR_AXIS_POSITION_START + 1);
inline constexpr TypedWhichId<SfxInt32Item>          SCHATTR_AXIS_POSITION                        (SCHATTR_AXIS_POSITION_START + 2);
inline constexpr TypedWhichId<SvxDoubleItem>         SCHATTR_AXIS_POSITION_VALUE                  (SCHATTR_AXIS_POSITION_START + 3);
inline constexpr TypedWhichId<SfxUInt32Item>         SCHATTR_AXIS_CROSSING_MAIN_AXIS_NUMBERFORMAT (SCHATTR_AXIS_POSITION_START + 4);
inline constexpr TypedWhichId<SfxBoolItem>           SCHATTR_AXIS_SHIFTED_CATEGORY_POSITION       (SCHATTR_AXIS_POSITION_START + 5);
inline constexpr TypedWhichId<SfxInt32Item>          SCHATTR_AXIS_LABEL_POSITION                  (SCHATTR_AXIS_POSITION_START + 6);
inline constexpr TypedWhichId<SfxInt32Item>          SCHATTR_AXIS_MARK_POSITION                   (SCHATTR_AXIS_POSITION_START + 7);
//axis label
inline constexpr sal_uInt16                          SCHATTR_AXIS_LABEL_START                     (SCHATTR_AXIS_MARK_POSITION + 1);
inline constexpr TypedWhichId<SfxBoolItem>           SCHATTR_AXIS_SHOWDESCR                       (SCHATTR_AXIS_LABEL_START);
inline constexpr TypedWhichId<SvxChartTextOrderItem> SCHATTR_AXIS_LABEL_ORDER                     (SCHATTR_AXIS_LABEL_START + 1);
inline constexpr TypedWhichId<SfxBoolItem>           SCHATTR_AXIS_LABEL_OVERLAP                   (SCHATTR_AXIS_LABEL_START + 2);
inline constexpr TypedWhichId<SfxBoolItem>           SCHATTR_AXIS_LABEL_BREAK                     (SCHATTR_AXIS_LABEL_START + 3);
inline constexpr sal_uInt16                          SCHATTR_AXIS_LABEL_END                       (SCHATTR_AXIS_LABEL_BREAK);

inline constexpr sal_uInt16                          SCHATTR_AXIS_END                             (SCHATTR_AXIS_LABEL_END);

inline constexpr TypedWhichId<SvxBrushItem> SCHATTR_SYMBOL_BRUSH                 (SCHATTR_AXIS_END + 1);
inline constexpr TypedWhichId<SfxBoolItem>  SCHATTR_STOCK_VOLUME                 (SCHATTR_AXIS_END + 2);
inline constexpr TypedWhichId<SfxBoolItem>  SCHATTR_STOCK_UPDOWN                 (SCHATTR_AXIS_END + 3);
inline constexpr TypedWhichId<SvxSizeItem>  SCHATTR_SYMBOL_SIZE                  (SCHATTR_AXIS_END + 4);
inline constexpr TypedWhichId<SfxBoolItem>  SCHATTR_HIDE_DATA_POINT_LEGEND_ENTRY (SCHATTR_AXIS_END + 5);

// non persistent items (binary format)
inline constexpr sal_uInt16                       SCHATTR_CHARTTYPE_START                    (SCHATTR_HIDE_DATA_POINT_LEGEND_ENTRY + 1);

// new from New Chart
inline constexpr TypedWhichId<SfxInt32Item>       SCHATTR_BAR_OVERLAP                        (SCHATTR_CHARTTYPE_START );
inline constexpr TypedWhichId<SfxInt32Item>       SCHATTR_BAR_GAPWIDTH                       (SCHATTR_CHARTTYPE_START + 1);
inline constexpr TypedWhichId<SfxBoolItem>        SCHATTR_BAR_CONNECT                        (SCHATTR_CHARTTYPE_START + 2);
inline constexpr TypedWhichId<SfxInt32Item>       SCHATTR_NUM_OF_LINES_FOR_BAR               (SCHATTR_CHARTTYPE_START + 3);
inline constexpr TypedWhichId<SfxInt32Item>       SCHATTR_SPLINE_ORDER                       (SCHATTR_CHARTTYPE_START + 4);
inline constexpr TypedWhichId<SfxInt32Item>       SCHATTR_SPLINE_RESOLUTION                  (SCHATTR_CHARTTYPE_START + 5);
inline constexpr TypedWhichId<SfxBoolItem>        SCHATTR_GROUP_BARS_PER_AXIS                (SCHATTR_CHARTTYPE_START + 6);
inline constexpr TypedWhichId<SdrAngleItem>       SCHATTR_STARTING_ANGLE                     (SCHATTR_CHARTTYPE_START + 7);
inline constexpr TypedWhichId<SfxBoolItem>        SCHATTR_CLOCKWISE                          (SCHATTR_CHARTTYPE_START + 8);
inline constexpr TypedWhichId<SfxInt32Item>       SCHATTR_MISSING_VALUE_TREATMENT            (SCHATTR_CHARTTYPE_START + 9);
inline constexpr TypedWhichId<SfxIntegerListItem> SCHATTR_AVAILABLE_MISSING_VALUE_TREATMENTS (SCHATTR_CHARTTYPE_START + 10);
inline constexpr TypedWhichId<SfxBoolItem>        SCHATTR_INCLUDE_HIDDEN_CELLS               (SCHATTR_CHARTTYPE_START + 11);
inline constexpr TypedWhichId<SfxBoolItem>        SCHATTR_HIDE_LEGEND_ENTRY                  (SCHATTR_CHARTTYPE_START + 12);

inline constexpr sal_uInt16                       SCHATTR_CHARTTYPE_END                      (SCHATTR_HIDE_LEGEND_ENTRY);

// items for transporting information to dialogs
inline constexpr sal_uInt16                 SCHATTR_MISC_START          (SCHATTR_CHARTTYPE_END + 1);
inline constexpr TypedWhichId<SfxInt32Item> SCHATTR_AXIS_FOR_ALL_SERIES (SCHATTR_MISC_START);
inline constexpr sal_uInt16                 SCHATTR_MISC_END            (SCHATTR_AXIS_FOR_ALL_SERIES);

// regression curve
inline constexpr sal_uInt16                        SCHATTR_REGRESSION_START                (SCHATTR_MISC_END + 1);
inline constexpr TypedWhichId<SvxChartRegressItem> SCHATTR_REGRESSION_TYPE                 (SCHATTR_REGRESSION_START);
inline constexpr TypedWhichId<SfxBoolItem>         SCHATTR_REGRESSION_SHOW_EQUATION        (SCHATTR_REGRESSION_START + 1);
inline constexpr TypedWhichId<SfxBoolItem>         SCHATTR_REGRESSION_SHOW_COEFF           (SCHATTR_REGRESSION_START + 2);
inline constexpr TypedWhichId<SfxInt32Item>        SCHATTR_REGRESSION_DEGREE               (SCHATTR_REGRESSION_START + 3);
inline constexpr TypedWhichId<SfxInt32Item>        SCHATTR_REGRESSION_PERIOD               (SCHATTR_REGRESSION_START + 4);
inline constexpr TypedWhichId<SvxDoubleItem>       SCHATTR_REGRESSION_EXTRAPOLATE_FORWARD  (SCHATTR_REGRESSION_START + 5);
inline constexpr TypedWhichId<SvxDoubleItem>       SCHATTR_REGRESSION_EXTRAPOLATE_BACKWARD (SCHATTR_REGRESSION_START + 6);
inline constexpr TypedWhichId<SfxBoolItem>         SCHATTR_REGRESSION_SET_INTERCEPT        (SCHATTR_REGRESSION_START + 7);
inline constexpr TypedWhichId<SvxDoubleItem>       SCHATTR_REGRESSION_INTERCEPT_VALUE      (SCHATTR_REGRESSION_START + 8);
inline constexpr TypedWhichId<SfxStringItem>       SCHATTR_REGRESSION_CURVE_NAME           (SCHATTR_REGRESSION_START + 9);
inline constexpr TypedWhichId<SfxStringItem>       SCHATTR_REGRESSION_XNAME                (SCHATTR_REGRESSION_START + 10);
inline constexpr TypedWhichId<SfxStringItem>       SCHATTR_REGRESSION_YNAME                (SCHATTR_REGRESSION_START + 11);
inline constexpr TypedWhichId<SfxInt32Item>        SCHATTR_REGRESSION_MOVING_TYPE          (SCHATTR_REGRESSION_START + 12);
inline constexpr sal_uInt16                        SCHATTR_REGRESSION_END                  (SCHATTR_REGRESSION_MOVING_TYPE);

inline constexpr sal_uInt16                        SCHATTR_DATA_TABLE_START                (SCHATTR_REGRESSION_END + 1);
inline constexpr TypedWhichId<SfxBoolItem>         SCHATTR_DATA_TABLE_HORIZONTAL_BORDER    (SCHATTR_DATA_TABLE_START + 0);
inline constexpr TypedWhichId<SfxBoolItem>         SCHATTR_DATA_TABLE_VERTICAL_BORDER      (SCHATTR_DATA_TABLE_START + 1);
inline constexpr TypedWhichId<SfxBoolItem>         SCHATTR_DATA_TABLE_OUTLINE              (SCHATTR_DATA_TABLE_START + 2);
inline constexpr TypedWhichId<SfxBoolItem>         SCHATTR_DATA_TABLE_KEYS                 (SCHATTR_DATA_TABLE_START + 3);
inline constexpr sal_uInt16                        SCHATTR_DATA_TABLE_END                  (SCHATTR_DATA_TABLE_KEYS);

inline constexpr sal_uInt16                             SCHATTR_COLOR_PALETTE_START        (SCHATTR_DATA_TABLE_END + 1);
inline constexpr TypedWhichId<SvxChartColorPaletteItem> SCHATTR_COLOR_PALETTE              (SCHATTR_COLOR_PALETTE_START);
inline constexpr sal_uInt16                             SCHATTR_COLOR_PALETTE_END          (SCHATTR_COLOR_PALETTE);

inline constexpr sal_uInt16 SCHATTR_END (SCHATTR_COLOR_PALETTE_END);

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
