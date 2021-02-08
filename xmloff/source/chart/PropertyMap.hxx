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
#ifndef INCLUDED_XMLOFF_SOURCE_CHART_PROPERTYMAP_HXX
#define INCLUDED_XMLOFF_SOURCE_CHART_PROPERTYMAP_HXX

#include <xmloff/maptype.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltypes.hxx>
#include <xmloff/contextid.hxx>
#include <xmloff/xmlement.hxx>
#include <com/sun/star/chart/ChartAxisArrangeOrderType.hpp>
#include <com/sun/star/chart/ChartAxisLabelPosition.hpp>
#include <com/sun/star/chart/ChartAxisMarkPosition.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>
#include <com/sun/star/chart/ChartSolidType.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart/DataLabelPlacement.hpp>
#include <com/sun/star/chart/MissingValueTreatment.hpp>

// custom types
#define XML_SCH_TYPE_AXIS_ARRANGEMENT       ( XML_SCH_TYPES_START + 0 )
#define XML_SCH_TYPE_ERROR_BAR_STYLE        ( XML_SCH_TYPES_START + 1 )
// free
#define XML_SCH_TYPE_SOLID_TYPE             ( XML_SCH_TYPES_START + 3 )
#define XML_SCH_TYPE_ERROR_INDICATOR_UPPER  ( XML_SCH_TYPES_START + 4 )
#define XML_SCH_TYPE_ERROR_INDICATOR_LOWER  ( XML_SCH_TYPES_START + 5 )
#define XML_SCH_TYPE_DATAROWSOURCE          ( XML_SCH_TYPES_START + 6 )
#define XML_SCH_TYPE_TEXT_ORIENTATION       ( XML_SCH_TYPES_START + 7 )
#define XML_SCH_TYPE_INTERPOLATION          ( XML_SCH_TYPES_START + 8 )
#define XML_SCH_TYPE_SYMBOL_TYPE            ( XML_SCH_TYPES_START + 9 )
#define XML_SCH_TYPE_NAMED_SYMBOL           ( XML_SCH_TYPES_START + 10 )
#define XML_SCH_TYPE_LABEL_PLACEMENT_TYPE   ( XML_SCH_TYPES_START + 11 )
#define XML_SCH_TYPE_MISSING_VALUE_TREATMENT    ( XML_SCH_TYPES_START + 12 )
#define XML_SCH_TYPE_AXIS_POSITION          ( XML_SCH_TYPES_START + 13 )
#define XML_SCH_TYPE_AXIS_POSITION_VALUE    ( XML_SCH_TYPES_START + 14 )
#define XML_SCH_TYPE_AXIS_LABEL_POSITION    ( XML_SCH_TYPES_START + 15 )
#define XML_SCH_TYPE_TICK_MARK_POSITION     ( XML_SCH_TYPES_START + 16 )
#define XML_SCH_TYPE_LABEL_BORDER_STYLE     ( XML_SCH_TYPES_START + 17 )
#define XML_SCH_TYPE_LABEL_BORDER_OPACITY   ( XML_SCH_TYPES_START + 18 )
#define XML_SCH_TYPE_LABEL_FILL_STYLE       ( XML_SCH_TYPES_START + 19 )

// context ids
#define XML_SCH_CONTEXT_USER_SYMBOL                 ( XML_SCH_CTF_START + 0 )
#define XML_SCH_CONTEXT_MIN                         ( XML_SCH_CTF_START + 1 )
#define XML_SCH_CONTEXT_MAX                         ( XML_SCH_CTF_START + 2 )
#define XML_SCH_CONTEXT_STEP_MAIN                   ( XML_SCH_CTF_START + 3 )
#define XML_SCH_CONTEXT_STEP_HELP_COUNT             ( XML_SCH_CTF_START + 4 )
#define XML_SCH_CONTEXT_ORIGIN                      ( XML_SCH_CTF_START + 5 )
#define XML_SCH_CONTEXT_LOGARITHMIC                 ( XML_SCH_CTF_START + 6 )
#define XML_SCH_CONTEXT_STOCK_WITH_VOLUME           ( XML_SCH_CTF_START + 7 )
#define XML_SCH_CONTEXT_LINES_USED                  ( XML_SCH_CTF_START + 8 )

#define XML_SCH_CONTEXT_SPECIAL_TICKS_MAJ_INNER     ( XML_SCH_CTF_START + 10 )
#define XML_SCH_CONTEXT_SPECIAL_TICKS_MAJ_OUTER     ( XML_SCH_CTF_START + 11 )
#define XML_SCH_CONTEXT_SPECIAL_TICKS_MIN_INNER     ( XML_SCH_CTF_START + 12 )
#define XML_SCH_CONTEXT_SPECIAL_TICKS_MIN_OUTER     ( XML_SCH_CTF_START + 13 )
#define XML_SCH_CONTEXT_SPECIAL_TEXT_ROTATION       ( XML_SCH_CTF_START + 14 )
#define XML_SCH_CONTEXT_SPECIAL_DATA_LABEL_NUMBER   ( XML_SCH_CTF_START + 15 )
#define XML_SCH_CONTEXT_SPECIAL_DATA_LABEL_TEXT     ( XML_SCH_CTF_START + 16 )
#define XML_SCH_CONTEXT_SPECIAL_DATA_LABEL_SYMBOL   ( XML_SCH_CTF_START + 17 )
#define XML_SCH_CONTEXT_SPECIAL_NUMBER_FORMAT       ( XML_SCH_CTF_START + 18 )
#define XML_SCH_CONTEXT_SPECIAL_DATA_ROW_SOURCE     ( XML_SCH_CTF_START + 19 )
#define XML_SCH_CONTEXT_SPECIAL_SYMBOL_WIDTH        ( XML_SCH_CTF_START + 20 )
#define XML_SCH_CONTEXT_SPECIAL_SYMBOL_HEIGHT       ( XML_SCH_CTF_START + 21 )
#define XML_SCH_CONTEXT_SPECIAL_SYMBOL_IMAGE_NAME   ( XML_SCH_CTF_START + 22 )
#define XML_SCH_CONTEXT_SPECIAL_SYMBOL_IMAGE        ( XML_SCH_CTF_START + 23 )
#define XML_SCH_CONTEXT_SPECIAL_LABEL_SEPARATOR     ( XML_SCH_CTF_START + 24 )
#define XML_SCH_CONTEXT_SPECIAL_ERRORBAR_RANGE      ( XML_SCH_CTF_START + 25 )
#define XML_SCH_CONTEXT_SPECIAL_REGRESSION_TYPE     ( XML_SCH_CTF_START + 26 )
#define XML_SCH_CONTEXT_SPECIAL_DATA_LABEL_SERIES   ( XML_SCH_CTF_START + 27 )

#define MAP_FULL( ApiName, NameSpace, XMLTokenName, XMLType, ContextId, EarliestODFVersionForExport ) { ApiName, sizeof(ApiName)-1, XML_NAMESPACE_##NameSpace, xmloff::token::XMLTokenName, XMLType|XML_TYPE_PROP_CHART, ContextId, EarliestODFVersionForExport, false }
#define MAP_ENTRY( a, ns, nm, t )            { a, sizeof(a)-1, XML_NAMESPACE_##ns, xmloff::token::nm, t|XML_TYPE_PROP_CHART, 0, SvtSaveOptions::ODFSVER_010, false }
#define MAP_ENTRY_ODF12( a, ns, nm, t )      { a, sizeof(a)-1, XML_NAMESPACE_##ns, xmloff::token::nm, t|XML_TYPE_PROP_CHART, 0, SvtSaveOptions::ODFSVER_012, false }
#define MAP_ENTRY_ODF13( a, ns, nm, t )      { a, sizeof(a)-1, ns, xmloff::token::nm, t|XML_TYPE_PROP_CHART, 0, SvtSaveOptions::ODFSVER_013, false }
#define MAP_ENTRY_ODF_EXT( a, ns, nm, t )    { a, sizeof(a)-1, XML_NAMESPACE_##ns, xmloff::token::nm, t|XML_TYPE_PROP_CHART, 0, SvtSaveOptions::ODFSVER_FUTURE_EXTENDED, false }
#define MAP_ENTRY_ODF_EXT_IMPORT( a, ns, nm, t )    { a, sizeof(a)-1, XML_NAMESPACE_##ns, xmloff::token::nm, t|XML_TYPE_PROP_CHART, 0, SvtSaveOptions::ODFSVER_FUTURE_EXTENDED, true }
#define MAP_CONTEXT( a, ns, nm, t, c )       { a, sizeof(a)-1, XML_NAMESPACE_##ns, xmloff::token::nm, t|XML_TYPE_PROP_CHART, c, SvtSaveOptions::ODFSVER_010, false }
#define MAP_SPECIAL( a, ns, nm, t, c )       { a, sizeof(a)-1, XML_NAMESPACE_##ns, xmloff::token::nm, t|XML_TYPE_PROP_CHART | MID_FLAG_SPECIAL_ITEM, c, SvtSaveOptions::ODFSVER_010, false }
#define MAP_SPECIAL_ODF12( a, ns, nm, t, c ) { a, sizeof(a)-1, XML_NAMESPACE_##ns, xmloff::token::nm, t|XML_TYPE_PROP_CHART | MID_FLAG_SPECIAL_ITEM, c, SvtSaveOptions::ODFSVER_012, false }
#define MAP_SPECIAL_ODF13( a, ns, nm, t, c ) { a, sizeof(a)-1, XML_NAMESPACE_##ns, xmloff::token::nm, t|XML_TYPE_PROP_CHART | MID_FLAG_SPECIAL_ITEM, c, SvtSaveOptions::ODFSVER_013, false }
#define MAP_ENTRY_END { nullptr,0,0,xmloff::token::XML_TOKEN_INVALID,0,0,SvtSaveOptions::ODFSVER_010, false }

// PropertyMap for Chart properties drawing- and
// textproperties are added later using the chaining
// mechanism

// only create maps once!
// this define is set in PropertyMaps.cxx

#ifdef XML_SCH_CREATE_GLOBAL_MAPS

const XMLPropertyMapEntry aXMLChartPropMap[] =
{
    // chart subtypes
    MAP_ENTRY( "UpDown", CHART, XML_JAPANESE_CANDLE_STICK, XML_TYPE_BOOL ), // formerly XML_STOCK_UPDOWN_BARS
    MAP_CONTEXT( "Volume", CHART, XML_STOCK_WITH_VOLUME, XML_TYPE_BOOL, XML_SCH_CONTEXT_STOCK_WITH_VOLUME ),
    MAP_ENTRY( "Dim3D", CHART, XML_THREE_DIMENSIONAL, XML_TYPE_BOOL ),
    MAP_ENTRY( "Deep", CHART, XML_DEEP, XML_TYPE_BOOL ),
    MAP_ENTRY( "Lines", CHART, XML_LINES, XML_TYPE_BOOL ),
    MAP_ENTRY( "Percent", CHART, XML_PERCENTAGE, XML_TYPE_BOOL ),
    MAP_ENTRY( "SolidType", CHART, XML_SOLID_TYPE, XML_SCH_TYPE_SOLID_TYPE ),
    // ODF 1.3 OFFICE-3662 added values
    MAP_ENTRY( "SplineType", CHART, XML_INTERPOLATION, XML_SCH_TYPE_INTERPOLATION ),
    MAP_ENTRY( "Stacked", CHART, XML_STACKED, XML_TYPE_BOOL ),
    // type: "none", "automatic", "named-symbol" or "image"
    MAP_ENTRY( "SymbolType", CHART, XML_SYMBOL_TYPE, XML_SCH_TYPE_SYMBOL_TYPE | MID_FLAG_MULTI_PROPERTY ),
    // if type=="named-symbol" => name of symbol (square, diamond, ...)
    MAP_ENTRY( "SymbolType", CHART, XML_SYMBOL_NAME, XML_SCH_TYPE_NAMED_SYMBOL | MID_FLAG_MULTI_PROPERTY ),
    // if type=="image" => an xlink:href element with a linked (package) URI
    MAP_SPECIAL( "SymbolBitmap", CHART, XML_SYMBOL_IMAGE, XML_TYPE_STRING | MID_FLAG_ELEMENT_ITEM, XML_SCH_CONTEXT_SPECIAL_SYMBOL_IMAGE ),
    MAP_SPECIAL( "SymbolSize", CHART, XML_SYMBOL_WIDTH, XML_TYPE_MEASURE | MID_FLAG_MERGE_PROPERTY, XML_SCH_CONTEXT_SPECIAL_SYMBOL_WIDTH ),
    MAP_SPECIAL( "SymbolSize", CHART, XML_SYMBOL_HEIGHT, XML_TYPE_MEASURE | MID_FLAG_MERGE_PROPERTY, XML_SCH_CONTEXT_SPECIAL_SYMBOL_HEIGHT ),
    MAP_ENTRY( "Vertical", CHART, XML_VERTICAL, XML_TYPE_BOOL ),
    // #i32368# property should no longer be used as XML-property (in OASIS
    // format), but is still ex-/imported for compatibility with the OOo file format
    MAP_CONTEXT( "NumberOfLines", CHART, XML_LINES_USED, XML_TYPE_NUMBER, XML_SCH_CONTEXT_LINES_USED  ),
    MAP_ENTRY( "StackedBarsConnected", CHART, XML_CONNECT_BARS, XML_TYPE_BOOL ),

    MAP_ENTRY_ODF12( "GroupBarsPerAxis", CHART, XML_GROUP_BARS_PER_AXIS, XML_TYPE_BOOL ),
    MAP_ENTRY_ODF12( "IncludeHiddenCells", CHART, XML_INCLUDE_HIDDEN_CELLS, XML_TYPE_BOOL ),
    MAP_ENTRY_ODF12( "AutomaticPosition", CHART, XML_AUTOMATIC_POSITION, XML_TYPE_BOOL ),
    MAP_ENTRY_ODF12( "AutomaticSize", CHART, XML_AUTOMATIC_SIZE, XML_TYPE_BOOL ),
    MAP_ENTRY_ODF12( "StartingAngle", CHART, XML_ANGLE_OFFSET, XML_TYPE_NUMBER ),
    MAP_ENTRY_ODF12( "MissingValueTreatment", CHART, XML_TREAT_EMPTY_CELLS, XML_SCH_TYPE_MISSING_VALUE_TREATMENT ),
    // #72304 Chart data table flags
    MAP_ENTRY_ODF_EXT( "DataTableHBorder", LO_EXT, XML_DATA_TABLE_SHOW_HORZ_BORDER, XML_TYPE_BOOL ),
    MAP_ENTRY_ODF_EXT( "DataTableVBorder", LO_EXT, XML_DATA_TABLE_SHOW_VERT_BORDER, XML_TYPE_BOOL ),
    MAP_ENTRY_ODF_EXT( "DataTableOutline", LO_EXT, XML_DATA_TABLE_SHOW_OUTLINE, XML_TYPE_BOOL ),
    MAP_ENTRY_ODF_EXT_IMPORT( "DataTableHBorder", CHART, XML_DATA_TABLE_SHOW_HORZ_BORDER, XML_TYPE_BOOL ),
    MAP_ENTRY_ODF_EXT_IMPORT( "DataTableVBorder", CHART, XML_DATA_TABLE_SHOW_VERT_BORDER, XML_TYPE_BOOL ),
    MAP_ENTRY_ODF_EXT_IMPORT( "DataTableOutline", CHART, XML_DATA_TABLE_SHOW_OUTLINE, XML_TYPE_BOOL ),
    // Chart display units flags
    MAP_ENTRY_ODF_EXT( "DisplayUnits", LO_EXT, XML_CHART_DUNITS_DISPLAYUNITS, XML_TYPE_BOOL ),
    MAP_ENTRY_ODF_EXT( "BuiltInUnit", LO_EXT, XML_CHART_DUNITS_BUILTINUNIT, XML_TYPE_STRING ),
    MAP_ENTRY_ODF_EXT( "ExternalData", LO_EXT, XML_EXTERNALDATA, XML_TYPE_STRING),

    MAP_ENTRY_ODF_EXT( "LabelBorderColor", LO_EXT, XML_LABEL_STROKE_COLOR, XML_TYPE_COLOR ),
    MAP_ENTRY_ODF_EXT( "LabelBorderStyle", LO_EXT, XML_LABEL_STROKE, XML_SCH_TYPE_LABEL_BORDER_STYLE ),
    MAP_ENTRY_ODF_EXT( "LabelBorderTransparency", LO_EXT, XML_LABEL_STROKE_OPACITY, XML_SCH_TYPE_LABEL_BORDER_OPACITY ),
    MAP_ENTRY_ODF_EXT( "LabelBorderWidth", LO_EXT, XML_LABEL_STROKE_WIDTH, XML_TYPE_MEASURE ),

    MAP_ENTRY_ODF_EXT( "LabelFillColor", LO_EXT, XML_LABEL_FILL_COLOR, XML_TYPE_COLOR ),
    MAP_ENTRY_ODF_EXT( "LabelFillStyle", LO_EXT, XML_LABEL_FILL, XML_SCH_TYPE_LABEL_FILL_STYLE ),
    MAP_ENTRY_ODF_EXT( "LabelFillBackground", LO_EXT, XML_FILL_HATCH_SOLID, XML_TYPE_BOOL ),
    MAP_ENTRY_ODF_EXT( "LabelFillHatchName", LO_EXT, XML_FILL_HATCH_NAME, XML_TYPE_STYLENAME),

    MAP_ENTRY( "ScaleText", CHART, XML_SCALE_TEXT, XML_TYPE_BOOL ),

    // spline settings
    MAP_ENTRY( "SplineOrder", CHART, XML_SPLINE_ORDER, XML_TYPE_NUMBER ),
    MAP_ENTRY( "SplineResolution", CHART, XML_SPLINE_RESOLUTION, XML_TYPE_NUMBER ),

    // plot-area properties
    MAP_ENTRY( "DataRowSource", CHART, XML_SERIES_SOURCE, XML_SCH_TYPE_DATAROWSOURCE ),
    MAP_ENTRY_ODF12( "SortByXValues", CHART, XML_SORT_BY_X_VALUES, XML_TYPE_BOOL ),
    MAP_ENTRY_ODF12( "RightAngledAxes", CHART, XML_RIGHT_ANGLED_AXES, XML_TYPE_BOOL ),

    // axis properties
    MAP_ENTRY( "DisplayLabels", CHART, XML_DISPLAY_LABEL, XML_TYPE_BOOL ),
    MAP_SPECIAL( "Marks", CHART, XML_TICK_MARKS_MAJOR_INNER, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, XML_SCH_CONTEXT_SPECIAL_TICKS_MAJ_INNER ),          // convert one constant
    MAP_SPECIAL( "Marks", CHART, XML_TICK_MARKS_MAJOR_OUTER, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, XML_SCH_CONTEXT_SPECIAL_TICKS_MAJ_OUTER ),          // to two bools
    MAP_SPECIAL( "HelpMarks", CHART, XML_TICK_MARKS_MINOR_INNER, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, XML_SCH_CONTEXT_SPECIAL_TICKS_MIN_INNER ),      // see above
    MAP_SPECIAL( "HelpMarks", CHART, XML_TICK_MARKS_MINOR_OUTER, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, XML_SCH_CONTEXT_SPECIAL_TICKS_MIN_OUTER ),
    MAP_CONTEXT( "Logarithmic", CHART, XML_LOGARITHMIC, XML_TYPE_BOOL, XML_SCH_CONTEXT_LOGARITHMIC ),
    MAP_CONTEXT( "Min", CHART, XML_MINIMUM, XML_TYPE_DOUBLE, XML_SCH_CONTEXT_MIN ),
    MAP_CONTEXT( "Max", CHART, XML_MAXIMUM, XML_TYPE_DOUBLE, XML_SCH_CONTEXT_MAX ),
    MAP_CONTEXT( "Origin", CHART, XML_ORIGIN, XML_TYPE_DOUBLE, XML_SCH_CONTEXT_ORIGIN ),
    MAP_CONTEXT( "StepMain", CHART, XML_INTERVAL_MAJOR, XML_TYPE_DOUBLE, XML_SCH_CONTEXT_STEP_MAIN ),
    MAP_CONTEXT( "StepHelpCount", CHART, XML_INTERVAL_MINOR_DIVISOR, XML_TYPE_NUMBER, XML_SCH_CONTEXT_STEP_HELP_COUNT ),
    MAP_ENTRY( "GapWidth", CHART, XML_GAP_WIDTH, XML_TYPE_NUMBER ),
    MAP_ENTRY( "Overlap", CHART, XML_OVERLAP, XML_TYPE_NUMBER ),
    MAP_ENTRY( "TextCanOverlap", CHART, XML_TEXT_OVERLAP, XML_TYPE_BOOL ),
    MAP_ENTRY_ODF12( "ReverseDirection", CHART, XML_REVERSE_DIRECTION, XML_TYPE_BOOL ),
    MAP_ENTRY( "TextBreak", TEXT, XML_LINE_BREAK, XML_TYPE_BOOL ),
    MAP_ENTRY_ODF_EXT( "TryStaggeringFirst", LO_EXT, XML_TRY_STAGGERING_FIRST, XML_TYPE_BOOL ),
    MAP_ENTRY( "ArrangeOrder", CHART, XML_LABEL_ARRANGEMENT, XML_SCH_TYPE_AXIS_ARRANGEMENT ),
    MAP_SPECIAL( "NumberFormat", STYLE, XML_DATA_STYLE_NAME, XML_TYPE_NUMBER, XML_SCH_CONTEXT_SPECIAL_NUMBER_FORMAT ),
    MAP_ENTRY( "LinkNumberFormatToSource", CHART, XML_LINK_DATA_STYLE_TO_SOURCE, XML_TYPE_BOOL ),
    MAP_ENTRY( "Visible", CHART, XML_VISIBLE, XML_TYPE_BOOL ),
    MAP_ENTRY_ODF_EXT( "MajorOrigin", LO_EXT, XML_MAJOR_ORIGIN, XML_TYPE_DOUBLE ),

    MAP_FULL( "CrossoverPosition", CHART, XML_AXIS_POSITION, XML_SCH_TYPE_AXIS_POSITION|MID_FLAG_MERGE_ATTRIBUTE|MID_FLAG_MULTI_PROPERTY, 0, SvtSaveOptions::ODFSVER_012 ),
    MAP_FULL( "CrossoverValue", CHART, XML_AXIS_POSITION, XML_SCH_TYPE_AXIS_POSITION_VALUE|MID_FLAG_MERGE_ATTRIBUTE|MID_FLAG_MULTI_PROPERTY, 0, SvtSaveOptions::ODFSVER_012 ),
    MAP_FULL( "LabelPosition", CHART, XML_AXIS_LABEL_POSITION, XML_SCH_TYPE_AXIS_LABEL_POSITION, 0, SvtSaveOptions::ODFSVER_012 ),
    MAP_FULL( "MarkPosition", CHART, XML_TICK_MARK_POSITION, XML_SCH_TYPE_TICK_MARK_POSITION, 0, SvtSaveOptions::ODFSVER_012 ),

    // statistical properties

    MAP_ENTRY( "MeanValue", CHART, XML_MEAN_VALUE, XML_TYPE_BOOL ),
    MAP_ENTRY( "ErrorMargin", CHART, XML_ERROR_MARGIN, XML_TYPE_DOUBLE ),
    MAP_ENTRY( "PositiveError", CHART, XML_ERROR_LOWER_LIMIT, XML_TYPE_DOUBLE),
    MAP_ENTRY( "NegativeError", CHART, XML_ERROR_UPPER_LIMIT, XML_TYPE_DOUBLE),
    MAP_ENTRY( "ShowPositiveError", CHART, XML_ERROR_UPPER_INDICATOR, XML_TYPE_BOOL),
    MAP_ENTRY( "ShowNegativeError", CHART, XML_ERROR_LOWER_INDICATOR, XML_TYPE_BOOL),
    MAP_ENTRY( "ErrorBarStyle", CHART, XML_ERROR_CATEGORY, XML_SCH_TYPE_ERROR_BAR_STYLE ),
    MAP_ENTRY( "PercentageError", CHART, XML_ERROR_PERCENTAGE, XML_TYPE_DOUBLE ),

    // regression curve properties
    MAP_SPECIAL( "RegressionType", CHART, XML_REGRESSION_TYPE, XML_TYPE_STRING, XML_SCH_CONTEXT_SPECIAL_REGRESSION_TYPE ),

    // ODF 1.3 OFFICE-3958
    MAP_ENTRY_ODF13( "CurveName", XML_NAMESPACE_LO_EXT, XML_REGRESSION_CURVE_NAME, XML_TYPE_STRING ),
    MAP_ENTRY_ODF13( "CurveName", XML_NAMESPACE_CHART, XML_REGRESSION_CURVE_NAME, XML_TYPE_STRING ),
    MAP_ENTRY_ODF13( "PolynomialDegree", XML_NAMESPACE_LO_EXT, XML_REGRESSION_MAX_DEGREE, XML_TYPE_NUMBER ),
    MAP_ENTRY_ODF13( "PolynomialDegree", XML_NAMESPACE_CHART, XML_REGRESSION_MAX_DEGREE, XML_TYPE_NUMBER ),
    MAP_ENTRY_ODF13( "ForceIntercept", XML_NAMESPACE_LO_EXT, XML_REGRESSION_FORCE_INTERCEPT, XML_TYPE_BOOL ),
    MAP_ENTRY_ODF13( "ForceIntercept", XML_NAMESPACE_CHART, XML_REGRESSION_FORCE_INTERCEPT, XML_TYPE_BOOL ),
    MAP_ENTRY_ODF13( "InterceptValue", XML_NAMESPACE_LO_EXT, XML_REGRESSION_INTERCEPT_VALUE, XML_TYPE_DOUBLE ),
    MAP_ENTRY_ODF13( "InterceptValue", XML_NAMESPACE_CHART, XML_REGRESSION_INTERCEPT_VALUE, XML_TYPE_DOUBLE ),

    // ODF 1.3 OFFICE-3959
    MAP_ENTRY_ODF13( "MovingAveragePeriod", XML_NAMESPACE_LO_EXT, XML_REGRESSION_PERIOD, XML_TYPE_NUMBER ),
    MAP_ENTRY_ODF13( "MovingAveragePeriod", XML_NAMESPACE_CHART, XML_REGRESSION_PERIOD, XML_TYPE_NUMBER ),
    MAP_ENTRY_ODF13( "MovingAverageType", XML_NAMESPACE_LO_EXT, XML_REGRESSION_MOVING_TYPE, XML_TYPE_STRING ),
    MAP_ENTRY_ODF13( "MovingAverageType", XML_NAMESPACE_CHART, XML_REGRESSION_MOVING_TYPE, XML_TYPE_STRING ),

    MAP_ENTRY_ODF_EXT( "ExtrapolateForward", LO_EXT, XML_REGRESSION_EXTRAPOLATE_FORWARD, XML_TYPE_DOUBLE ),
    MAP_ENTRY_ODF_EXT( "ExtrapolateBackward", LO_EXT, XML_REGRESSION_EXTRAPOLATE_BACKWARD, XML_TYPE_DOUBLE ),
    MAP_ENTRY_ODF_EXT_IMPORT( "ExtrapolateForward", CHART, XML_REGRESSION_EXTRAPOLATE_FORWARD, XML_TYPE_DOUBLE ),
    MAP_ENTRY_ODF_EXT_IMPORT( "ExtrapolateBackward", CHART, XML_REGRESSION_EXTRAPOLATE_BACKWARD, XML_TYPE_DOUBLE ),

    MAP_ENTRY_ODF_EXT( "XName", LO_EXT, XML_REGRESSION_X_NAME, XML_TYPE_STRING ),
    MAP_ENTRY_ODF_EXT( "YName", LO_EXT, XML_REGRESSION_Y_NAME, XML_TYPE_STRING ),

    MAP_SPECIAL_ODF12( "ErrorBarRangePositive", CHART, XML_ERROR_UPPER_RANGE, XML_TYPE_STRING, XML_SCH_CONTEXT_SPECIAL_ERRORBAR_RANGE ), // export only
    MAP_SPECIAL_ODF12( "ErrorBarRangeNegative", CHART, XML_ERROR_LOWER_RANGE, XML_TYPE_STRING, XML_SCH_CONTEXT_SPECIAL_ERRORBAR_RANGE ), // export only

    // errorbars properties (chart2)
    MAP_ENTRY_ODF_EXT( "Weight", LO_EXT, XML_ERROR_STANDARD_WEIGHT, XML_TYPE_DOUBLE),
    MAP_ENTRY_ODF_EXT_IMPORT( "Weight", CHART, XML_ERROR_STANDARD_WEIGHT, XML_TYPE_DOUBLE),

    // series/data-point properties
    MAP_SPECIAL( "DataCaption", CHART, XML_DATA_LABEL_NUMBER, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, XML_SCH_CONTEXT_SPECIAL_DATA_LABEL_NUMBER ),   // convert one constant
    MAP_SPECIAL( "DataCaption", CHART, XML_DATA_LABEL_TEXT, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, XML_SCH_CONTEXT_SPECIAL_DATA_LABEL_TEXT ),       // to 'tristate' and two bools
    MAP_SPECIAL( "DataCaption", CHART, XML_DATA_LABEL_SYMBOL, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, XML_SCH_CONTEXT_SPECIAL_DATA_LABEL_SYMBOL ),
    MAP_SPECIAL_ODF13( "DataCaption", CHART, XML_DATA_LABEL_SERIES, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, XML_SCH_CONTEXT_SPECIAL_DATA_LABEL_SERIES ),
    MAP_SPECIAL_ODF12( "LabelSeparator", CHART, XML_LABEL_SEPARATOR, XML_TYPE_STRING | MID_FLAG_ELEMENT_ITEM, XML_SCH_CONTEXT_SPECIAL_LABEL_SEPARATOR ),
    MAP_ENTRY_ODF12( "LabelPlacement", CHART, XML_LABEL_POSITION, XML_SCH_TYPE_LABEL_PLACEMENT_TYPE ),
    MAP_ENTRY( "SegmentOffset", CHART, XML_PIE_OFFSET, XML_TYPE_NUMBER ),
    MAP_SPECIAL_ODF12( "PercentageNumberFormat", STYLE, XML_PERCENTAGE_DATA_STYLE_NAME, XML_TYPE_NUMBER, XML_SCH_CONTEXT_SPECIAL_NUMBER_FORMAT ),
    MAP_ENTRY_ODF_EXT( "ShowCustomLeaderLines", LO_EXT, XML_CUSTOM_LEADERLINES, XML_TYPE_BOOL ),

    // text properties for titles
    MAP_SPECIAL( "TextRotation", STYLE, XML_ROTATION_ANGLE, XML_TYPE_NUMBER, XML_SCH_CONTEXT_SPECIAL_TEXT_ROTATION ),   // convert 1/100th degrees to degrees
    MAP_ENTRY( "StackedText", STYLE, XML_DIRECTION, XML_SCH_TYPE_TEXT_ORIENTATION ),

    // for compatibility to pre 6.0beta documents
//     MAP_SPECIAL( "SymbolBitmap", CHART, XML_SYMBOL_IMAGE_NAME, XML_TYPE_STRING, XML_SCH_CONTEXT_SPECIAL_SYMBOL_IMAGE_NAME ),

    MAP_ENTRY( "ChartUserDefinedAttributes", TEXT, XML_XMLNS, XML_TYPE_ATTRIBUTE_CONTAINER | MID_FLAG_SPECIAL_ITEM ),

    MAP_ENTRY_END
};

// maps for enums to XML attributes

const SvXMLEnumMapEntry<css::chart::ChartAxisLabelPosition> aXMLChartAxisLabelPositionEnumMap[] =
{
    { ::xmloff::token::XML_NEAR_AXIS,               css::chart::ChartAxisLabelPosition_NEAR_AXIS },
    { ::xmloff::token::XML_NEAR_AXIS_OTHER_SIDE,    css::chart::ChartAxisLabelPosition_NEAR_AXIS_OTHER_SIDE },
    { ::xmloff::token::XML_OUTSIDE_START,           css::chart::ChartAxisLabelPosition_OUTSIDE_START },
    { ::xmloff::token::XML_OUTSIDE_END,             css::chart::ChartAxisLabelPosition_OUTSIDE_END },
    { ::xmloff::token::XML_OUTSIDE_MINIMUM,         css::chart::ChartAxisLabelPosition_OUTSIDE_START },//#i114142#
    { ::xmloff::token::XML_OUTSIDE_MAXIMUM,         css::chart::ChartAxisLabelPosition_OUTSIDE_END },//#i114142#
    { ::xmloff::token::XML_TOKEN_INVALID, css::chart::ChartAxisLabelPosition(0) }
};

const SvXMLEnumMapEntry<css::chart::ChartAxisMarkPosition> aXMLChartAxisMarkPositionEnumMap[] =
{
    { ::xmloff::token::XML_AT_LABELS,           css::chart::ChartAxisMarkPosition_AT_LABELS },
    { ::xmloff::token::XML_AT_AXIS,             css::chart::ChartAxisMarkPosition_AT_AXIS },
    { ::xmloff::token::XML_AT_LABELS_AND_AXIS,  css::chart::ChartAxisMarkPosition_AT_LABELS_AND_AXIS },
    { ::xmloff::token::XML_TOKEN_INVALID, css::chart::ChartAxisMarkPosition(0) }
};

const SvXMLEnumMapEntry<css::chart::ChartAxisArrangeOrderType> aXMLChartAxisArrangementEnumMap[] =
{
    { ::xmloff::token::XML_SIDE_BY_SIDE,  css::chart::ChartAxisArrangeOrderType_SIDE_BY_SIDE },
    { ::xmloff::token::XML_STAGGER_EVEN,  css::chart::ChartAxisArrangeOrderType_STAGGER_EVEN },
    { ::xmloff::token::XML_STAGGER_ODD,   css::chart::ChartAxisArrangeOrderType_STAGGER_ODD },
    { ::xmloff::token::XML_TOKEN_INVALID, css::chart::ChartAxisArrangeOrderType(0) }
};

const SvXMLEnumMapEntry<sal_Int32> aXMLChartErrorBarStyleEnumMap[] =
{
    { ::xmloff::token::XML_NONE,                css::chart::ErrorBarStyle::NONE },
    { ::xmloff::token::XML_VARIANCE,            css::chart::ErrorBarStyle::VARIANCE },
    { ::xmloff::token::XML_STANDARD_DEVIATION,  css::chart::ErrorBarStyle::STANDARD_DEVIATION },
    { ::xmloff::token::XML_CONSTANT,            css::chart::ErrorBarStyle::ABSOLUTE },
    { ::xmloff::token::XML_PERCENTAGE,          css::chart::ErrorBarStyle::RELATIVE },
    { ::xmloff::token::XML_ERROR_MARGIN,        css::chart::ErrorBarStyle::ERROR_MARGIN },
    { ::xmloff::token::XML_STANDARD_ERROR,      css::chart::ErrorBarStyle::STANDARD_ERROR },
    { ::xmloff::token::XML_CELL_RANGE,          css::chart::ErrorBarStyle::FROM_DATA },
    { ::xmloff::token::XML_TOKEN_INVALID, 0 }
};

const SvXMLEnumMapEntry<sal_Int32> aXMLChartSolidTypeEnumMap[] =
{
    { ::xmloff::token::XML_CUBOID,      css::chart::ChartSolidType::RECTANGULAR_SOLID },
    { ::xmloff::token::XML_CYLINDER,    css::chart::ChartSolidType::CYLINDER },
    { ::xmloff::token::XML_CONE,        css::chart::ChartSolidType::CONE },
    { ::xmloff::token::XML_PYRAMID,     css::chart::ChartSolidType::PYRAMID },
    { ::xmloff::token::XML_TOKEN_INVALID, 0 }
};

const SvXMLEnumMapEntry<css::chart::ChartDataRowSource> aXMLChartDataRowSourceTypeEnumMap[] =
{
    { ::xmloff::token::XML_COLUMNS,     css::chart::ChartDataRowSource_COLUMNS },
    { ::xmloff::token::XML_ROWS,        css::chart::ChartDataRowSource_ROWS },
    { ::xmloff::token::XML_TOKEN_INVALID, css::chart::ChartDataRowSource(0) }
};

const SvXMLEnumMapEntry<sal_Int32> g_XMLChartInterpolationTypeEnumMap_ODF12[] =
{
    // this is neither an enum nor a constants group, but just a
    // documented long property
    { ::xmloff::token::XML_NONE,               0 },
    { ::xmloff::token::XML_CUBIC_SPLINE,       1 },
    { ::xmloff::token::XML_B_SPLINE,           2 },
    { ::xmloff::token::XML_TOKEN_INVALID,      0 }
};

const SvXMLEnumMapEntry<sal_Int32> g_XMLChartInterpolationTypeEnumMap[] =
{
    // this is neither an enum nor a constants group, but just a
    // documented long property
    { ::xmloff::token::XML_NONE,               0 },
    { ::xmloff::token::XML_CUBIC_SPLINE,       1 },
    { ::xmloff::token::XML_B_SPLINE,           2 },
    { ::xmloff::token::XML_STEP_START,         3 },
    { ::xmloff::token::XML_STEP_END,           4 },
    { ::xmloff::token::XML_STEP_CENTER_X,      5 },
    { ::xmloff::token::XML_STEP_CENTER_Y,      6 },
    // the GNM values should only be used for reading Gnumeric ods files
    // they should never be used for writing ods file
    { ::xmloff::token::XML_GNM_STEP_START,     3 },
    { ::xmloff::token::XML_GNM_STEP_END,       4 },
    { ::xmloff::token::XML_GNM_STEP_CENTER_X,  5 },
    { ::xmloff::token::XML_GNM_STEP_CENTER_Y,  6 },
    { ::xmloff::token::XML_TOKEN_INVALID,      0 }
};

const SvXMLEnumMapEntry<sal_Int32> aXMLChartDataLabelPlacementEnumMap[] =
{
    { ::xmloff::token::XML_AVOID_OVERLAP,   css::chart::DataLabelPlacement::AVOID_OVERLAP },
    { ::xmloff::token::XML_CENTER,          css::chart::DataLabelPlacement::CENTER },
    { ::xmloff::token::XML_TOP,             css::chart::DataLabelPlacement::TOP },
    { ::xmloff::token::XML_TOP_LEFT,        css::chart::DataLabelPlacement::TOP_LEFT },
    { ::xmloff::token::XML_LEFT,            css::chart::DataLabelPlacement::LEFT },
    { ::xmloff::token::XML_BOTTOM_LEFT,     css::chart::DataLabelPlacement::BOTTOM_LEFT },
    { ::xmloff::token::XML_BOTTOM,          css::chart::DataLabelPlacement::BOTTOM },
    { ::xmloff::token::XML_BOTTOM_RIGHT,    css::chart::DataLabelPlacement::BOTTOM_RIGHT },
    { ::xmloff::token::XML_RIGHT,           css::chart::DataLabelPlacement::RIGHT },
    { ::xmloff::token::XML_TOP_RIGHT,       css::chart::DataLabelPlacement::TOP_RIGHT },
    { ::xmloff::token::XML_INSIDE,          css::chart::DataLabelPlacement::INSIDE },
    { ::xmloff::token::XML_OUTSIDE,         css::chart::DataLabelPlacement::OUTSIDE },
    { ::xmloff::token::XML_NEAR_ORIGIN,     css::chart::DataLabelPlacement::NEAR_ORIGIN },
    { ::xmloff::token::XML_TOKEN_INVALID, 0 }
};

const SvXMLEnumMapEntry<sal_Int32> aXMLChartMissingValueTreatmentEnumMap[] =
{
    { ::xmloff::token::XML_LEAVE_GAP,    css::chart::MissingValueTreatment::LEAVE_GAP },
    { ::xmloff::token::XML_USE_ZERO,     css::chart::MissingValueTreatment::USE_ZERO },
    { ::xmloff::token::XML_IGNORE,       css::chart::MissingValueTreatment::CONTINUE },
    { ::xmloff::token::XML_TOKEN_INVALID,0 },
};

#endif  // XML_SCH_CREATE_GLOBAL_MAPS

#endif  // _PROPERTYMAP_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
