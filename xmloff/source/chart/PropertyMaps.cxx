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

#include "PropertyMap.hxx"

#include <sax/tools/converter.hxx>

#include "SchXMLTools.hxx"
#include <XMLChartPropertySetMapper.hxx>
#include "XMLErrorIndicatorPropertyHdl.hxx"
#include "XMLErrorBarStylePropertyHdl.hxx"
#include "XMLTextOrientationHdl.hxx"
#include "XMLSymbolTypePropertyHdl.hxx"
#include "XMLAxisPositionPropertyHdl.hxx"
#include <propimp0.hxx>

#include <xmloff/EnumPropertyHdl.hxx>
#include <xmloff/attrlist.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/shapeimport.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/prhdlfac.hxx>

#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/chart/ChartAxisMarks.hpp>
#include <com/sun/star/chart/ChartDataCaption.hpp>
#include <com/sun/star/chart2/MovingAverageType.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/data/XRangeXMLConversion.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>

#include <rtl/ustrbuf.hxx>

#define SCH_XML_SETFLAG( status, flag )     (status)|= (flag)
#define SCH_XML_UNSETFLAG( status, flag )   (status) = ((status) | (flag)) - (flag)

using namespace com::sun::star;
using namespace ::xmloff::token;
using namespace css::chart2;


#define MAP_FULL( ApiName, NameSpace, XMLTokenName, XMLType, ContextId, EarliestODFVersionForExport ) { ApiName, XML_NAMESPACE_##NameSpace, xmloff::token::XMLTokenName, XMLType|XML_TYPE_PROP_CHART, ContextId, EarliestODFVersionForExport, false }
#define MAP_ENTRY( a, ns, nm, t )            { a, XML_NAMESPACE_##ns, xmloff::token::nm, t|XML_TYPE_PROP_CHART, 0, SvtSaveOptions::ODFSVER_010, false }
#define MAP_ENTRY_ODF12( a, ns, nm, t )      { a, XML_NAMESPACE_##ns, xmloff::token::nm, t|XML_TYPE_PROP_CHART, 0, SvtSaveOptions::ODFSVER_012, false }
#define MAP_ENTRY_ODF13( a, ns, nm, t )      { a, ns, xmloff::token::nm, t|XML_TYPE_PROP_CHART, 0, SvtSaveOptions::ODFSVER_013, false }
#define MAP_ENTRY_ODF_EXT( a, ns, nm, t )    { a, XML_NAMESPACE_##ns, xmloff::token::nm, t|XML_TYPE_PROP_CHART, 0, SvtSaveOptions::ODFSVER_FUTURE_EXTENDED, false }
#define MAP_ENTRY_ODF_EXT_IMPORT( a, ns, nm, t ) { a, XML_NAMESPACE_##ns, xmloff::token::nm, t|XML_TYPE_PROP_CHART, 0, SvtSaveOptions::ODFSVER_FUTURE_EXTENDED, true }
#define MAP_CONTEXT( a, ns, nm, t, c )       { a, XML_NAMESPACE_##ns, xmloff::token::nm, t|XML_TYPE_PROP_CHART, c, SvtSaveOptions::ODFSVER_010, false }
#define MAP_SPECIAL( a, ns, nm, t, c )       { a, XML_NAMESPACE_##ns, xmloff::token::nm, t|XML_TYPE_PROP_CHART | MID_FLAG_SPECIAL_ITEM, c, SvtSaveOptions::ODFSVER_010, false }
#define MAP_SPECIAL_ODF12( a, ns, nm, t, c ) { a, XML_NAMESPACE_##ns, xmloff::token::nm, t|XML_TYPE_PROP_CHART | MID_FLAG_SPECIAL_ITEM, c, SvtSaveOptions::ODFSVER_012, false }
#define MAP_SPECIAL_ODF13( a, ns, nm, t, c ) { a, XML_NAMESPACE_##ns, xmloff::token::nm, t|XML_TYPE_PROP_CHART | MID_FLAG_SPECIAL_ITEM, c, SvtSaveOptions::ODFSVER_013, false }
#define MAP_ENTRY_END { nullptr,0,xmloff::token::XML_TOKEN_INVALID,0,0,SvtSaveOptions::ODFSVER_010, false }

// PropertyMap for Chart properties drawing- and
// textproperties are added later using the chaining
// mechanism

constexpr OUStringLiteral ARRANGE_ORDER = u"ArrangeOrder";
constexpr OUStringLiteral AUTOMATIC_POSITION = u"AutomaticPosition";
constexpr OUStringLiteral AUTOMATIC_SIZE = u"AutomaticSize";
constexpr OUStringLiteral BUILT_IN_UNIT = u"BuiltInUnit";
constexpr OUStringLiteral CHART_USER_DEFINED_ATTRIBUTES = u"ChartUserDefinedAttributes";
constexpr OUStringLiteral CROSSOVER_POSITION = u"CrossoverPosition";
constexpr OUStringLiteral CROSSOVER_VALUE = u"CrossoverValue";
constexpr OUStringLiteral CURVE_NAME = u"CurveName";
constexpr OUStringLiteral DATA_CAPTION = u"DataCaption";
constexpr OUStringLiteral DATA_ROW_SOURCE = u"DataRowSource";
constexpr OUStringLiteral DATA_TABLE_HBORDER = u"DataTableHBorder";
constexpr OUStringLiteral DATA_TABLE_OUTLINE = u"DataTableOutline";
constexpr OUStringLiteral DATA_TABLE_VBORDER = u"DataTableVBorder";
constexpr OUStringLiteral DEEP = u"Deep";
constexpr OUStringLiteral DIM3D = u"Dim3D";
constexpr OUStringLiteral DISPLAY_LABELS = u"DisplayLabels";
constexpr OUStringLiteral DISPLAY_UNITS = u"DisplayUnits";
constexpr OUStringLiteral ERROR_BAR_RANGE_NEGATIVE = u"ErrorBarRangeNegative";
constexpr OUStringLiteral ERROR_BAR_STYLE = u"ErrorBarStyle";
constexpr OUStringLiteral ERROR_MARGIN = u"ErrorMargin";
constexpr OUStringLiteral EXTERNAL_DATA = u"ExternalData";
constexpr OUStringLiteral EXTRAPOLATE_BACKWARD = u"ExtrapolateBackward";
constexpr OUStringLiteral EXTRAPOLATE_FORWARD = u"ExtrapolateForward";
constexpr OUStringLiteral FORCE_INTERCEPT = u"ForceIntercept";
constexpr OUStringLiteral GAP_WIDTH = u"GapWidth";
constexpr OUStringLiteral GROUP_BARS_PER_AXIS = u"GroupBarsPerAxis";
constexpr OUStringLiteral HELP_MARKS = u"HelpMarks";
constexpr OUStringLiteral INCLUDE_HIDDEN_CELLS = u"IncludeHiddenCells";
constexpr OUStringLiteral INTERCEPT_VALUE = u"InterceptValue";
constexpr OUStringLiteral LABEL_BORDER_COLOR = u"LabelBorderColor";
constexpr OUStringLiteral LABEL_BORDER_STYLE = u"LabelBorderStyle";
constexpr OUStringLiteral LABEL_BORDER_TRANSPARENCY = u"LabelBorderTransparency";
constexpr OUStringLiteral LABEL_BORDER_WIDTH = u"LabelBorderWidth";
constexpr OUStringLiteral LABEL_FILL_BACKGROUND = u"LabelFillBackground";
constexpr OUStringLiteral LABEL_FILL_COLOR = u"LabelFillColor";
constexpr OUStringLiteral LABEL_FILL_HATCH_NAME = u"LabelFillHatchName";
constexpr OUStringLiteral LABEL_FILL_STYLE = u"LabelFillStyle";
constexpr OUStringLiteral LABEL_PLACEMENT = u"LabelPlacement";
constexpr OUStringLiteral LABEL_POSITION = u"LabelPosition";
constexpr OUStringLiteral LABEL_SEPARATOR = u"LabelSeparator";
constexpr OUStringLiteral LINES = u"Lines";
constexpr OUStringLiteral LINK_NUMBER_FORMAT_TO_SOURCE = u"LinkNumberFormatToSource";
constexpr OUStringLiteral LOGARITHMIC = u"Logarithmic";
constexpr OUStringLiteral MAJOR_ORIGIN = u"MajorOrigin";
constexpr OUStringLiteral MARKS = u"Marks";
constexpr OUStringLiteral MARK_POSITION = u"MarkPosition";
constexpr OUStringLiteral MAX = u"Max";
constexpr OUStringLiteral MEAN_VALUE = u"MeanValue";
constexpr OUStringLiteral MIN = u"Min";
constexpr OUStringLiteral MISSING_VALUE_TREATMENT = u"MissingValueTreatment";
constexpr OUStringLiteral MOVING_AVERAGE_PERIOD = u"MovingAveragePeriod";
constexpr OUStringLiteral MOVING_AVERAGE_TYPE = u"MovingAverageType";
constexpr OUStringLiteral NEGATIVE_ERROR = u"NegativeError";
constexpr OUStringLiteral NUMBER_FORMAT = u"NumberFormat";
constexpr OUStringLiteral NUMBER_OF_LINES = u"NumberOfLines";
constexpr OUStringLiteral ORIGIN = u"Origin";
constexpr OUStringLiteral OVERLAP = u"Overlap";
constexpr OUStringLiteral PERCENT = u"Percent";
constexpr OUStringLiteral PERCENTAGE_ERROR = u"PercentageError";
constexpr OUStringLiteral PERCENTAGE_NUMBER_FORMAT = u"PercentageNumberFormat";
constexpr OUStringLiteral POLYNOMIAL_DEGREE = u"PolynomialDegree";
constexpr OUStringLiteral POSITIVE_ERROR = u"PositiveError";
constexpr OUStringLiteral REGRESSION_TYPE = u"RegressionType";
constexpr OUStringLiteral REVERSE_DIRECTION = u"ReverseDirection";
constexpr OUStringLiteral RIGHT_ANGLED_AXES = u"RightAngledAxes";
constexpr OUStringLiteral SCALE_TEXT = u"ScaleText";
constexpr OUStringLiteral SEGMENT_OFFSET = u"SegmentOffset";
constexpr OUStringLiteral SHOW_CUSTOM_LEADER_LINES = u"ShowCustomLeaderLines";
constexpr OUStringLiteral SHOW_NEGATIVE_ERROR = u"ShowNegativeError";
constexpr OUStringLiteral SHOW_POSITIVE_ERROR = u"ShowPositiveError";
constexpr OUStringLiteral SOLID_TYPE = u"SolidType";
constexpr OUStringLiteral SORT_BY_XVALUES = u"SortByXValues";
constexpr OUStringLiteral SPLINE_ORDER = u"SplineOrder";
constexpr OUStringLiteral SPLINE_RESOLUTION = u"SplineResolution";
constexpr OUStringLiteral SPLINE_TYPE = u"SplineType";
constexpr OUStringLiteral STACKED = u"Stacked";
constexpr OUStringLiteral STACKED_BARS_CONNECTED = u"StackedBarsConnected";
constexpr OUStringLiteral STACKED_TEXT = u"StackedText";
constexpr OUStringLiteral STARTING_ANGLE = u"StartingAngle";
constexpr OUStringLiteral STEP_HELP_COUNT = u"StepHelpCount";
constexpr OUStringLiteral STEP_MAIN = u"StepMain";
constexpr OUStringLiteral SYMBOL_BITMAP = u"SymbolBitmap";
constexpr OUStringLiteral SYMBOL_SIZE = u"SymbolSize";
constexpr OUStringLiteral SYMBOL_TYPE = u"SymbolType";
constexpr OUStringLiteral TEXT_BREAK = u"TextBreak";
constexpr OUStringLiteral TEXT_CAN_OVERLAP = u"TextCanOverlap";
constexpr OUStringLiteral TEXT_ROTATION = u"TextRotation";
constexpr OUStringLiteral TRY_STAGGERING_FIRST = u"TryStaggeringFirst";
constexpr OUStringLiteral UPDOWN = u"UpDown";
constexpr OUStringLiteral VERTICAL = u"Vertical";
constexpr OUStringLiteral VISIBLE = u"Visible";
constexpr OUStringLiteral VOLUME = u"Volume";
constexpr OUStringLiteral WEIGHT = u"Weight";
constexpr OUStringLiteral X_NAME = u"XName";
constexpr OUStringLiteral Y_NAME = u"YName";

const XMLPropertyMapEntry aXMLChartPropMap[] =
{
    // chart subtypes
    MAP_ENTRY( UPDOWN, CHART, XML_JAPANESE_CANDLE_STICK, XML_TYPE_BOOL ), // formerly XML_STOCK_UPDOWN_BARS
    MAP_CONTEXT( VOLUME, CHART, XML_STOCK_WITH_VOLUME, XML_TYPE_BOOL, XML_SCH_CONTEXT_STOCK_WITH_VOLUME ),
    MAP_ENTRY( DIM3D, CHART, XML_THREE_DIMENSIONAL, XML_TYPE_BOOL ),
    MAP_ENTRY( DEEP, CHART, XML_DEEP, XML_TYPE_BOOL ),
    MAP_ENTRY( LINES, CHART, XML_LINES, XML_TYPE_BOOL ),
    MAP_ENTRY( PERCENT, CHART, XML_PERCENTAGE, XML_TYPE_BOOL ),
    MAP_ENTRY( SOLID_TYPE, CHART, XML_SOLID_TYPE, XML_SCH_TYPE_SOLID_TYPE ),
    // ODF 1.3 OFFICE-3662 added values
    MAP_ENTRY( SPLINE_TYPE, CHART, XML_INTERPOLATION, XML_SCH_TYPE_INTERPOLATION ),
    MAP_ENTRY( STACKED, CHART, XML_STACKED, XML_TYPE_BOOL ),
    // type: "none", "automatic", "named-symbol" or "image"
    MAP_ENTRY( SYMBOL_TYPE, CHART, XML_SYMBOL_TYPE, XML_SCH_TYPE_SYMBOL_TYPE | MID_FLAG_MULTI_PROPERTY ),
    // if type=="named-symbol" => name of symbol (square, diamond, ...)
    MAP_ENTRY( SYMBOL_TYPE, CHART, XML_SYMBOL_NAME, XML_SCH_TYPE_NAMED_SYMBOL | MID_FLAG_MULTI_PROPERTY ),
    // if type=="image" => an xlink:href element with a linked (package) URI
    MAP_SPECIAL( SYMBOL_BITMAP, CHART, XML_SYMBOL_IMAGE, XML_TYPE_STRING | MID_FLAG_ELEMENT_ITEM, XML_SCH_CONTEXT_SPECIAL_SYMBOL_IMAGE ),
    MAP_SPECIAL( SYMBOL_SIZE, CHART, XML_SYMBOL_WIDTH, XML_TYPE_MEASURE | MID_FLAG_MERGE_PROPERTY, XML_SCH_CONTEXT_SPECIAL_SYMBOL_WIDTH ),
    MAP_SPECIAL( SYMBOL_SIZE, CHART, XML_SYMBOL_HEIGHT, XML_TYPE_MEASURE | MID_FLAG_MERGE_PROPERTY, XML_SCH_CONTEXT_SPECIAL_SYMBOL_HEIGHT ),
    MAP_ENTRY( VERTICAL, CHART, XML_VERTICAL, XML_TYPE_BOOL ),
    // #i32368# property should no longer be used as XML-property (in OASIS
    // format), but is still ex-/imported for compatibility with the OOo file format
    MAP_CONTEXT( NUMBER_OF_LINES, CHART, XML_LINES_USED, XML_TYPE_NUMBER, XML_SCH_CONTEXT_LINES_USED  ),
    MAP_ENTRY( STACKED_BARS_CONNECTED, CHART, XML_CONNECT_BARS, XML_TYPE_BOOL ),

    MAP_ENTRY_ODF12( GROUP_BARS_PER_AXIS, CHART, XML_GROUP_BARS_PER_AXIS, XML_TYPE_BOOL ),
    MAP_ENTRY_ODF12( INCLUDE_HIDDEN_CELLS, CHART, XML_INCLUDE_HIDDEN_CELLS, XML_TYPE_BOOL ),
    MAP_ENTRY_ODF12( AUTOMATIC_POSITION, CHART, XML_AUTOMATIC_POSITION, XML_TYPE_BOOL ),
    MAP_ENTRY_ODF12( AUTOMATIC_SIZE, CHART, XML_AUTOMATIC_SIZE, XML_TYPE_BOOL ),
    MAP_ENTRY_ODF12( STARTING_ANGLE, CHART, XML_ANGLE_OFFSET, XML_TYPE_NUMBER ),
    MAP_ENTRY_ODF12( MISSING_VALUE_TREATMENT, CHART, XML_TREAT_EMPTY_CELLS, XML_SCH_TYPE_MISSING_VALUE_TREATMENT ),
    // #72304 Chart data table flags
    MAP_ENTRY_ODF_EXT( DATA_TABLE_HBORDER, LO_EXT, XML_DATA_TABLE_SHOW_HORZ_BORDER, XML_TYPE_BOOL ),
    MAP_ENTRY_ODF_EXT( DATA_TABLE_VBORDER, LO_EXT, XML_DATA_TABLE_SHOW_VERT_BORDER, XML_TYPE_BOOL ),
    MAP_ENTRY_ODF_EXT( DATA_TABLE_OUTLINE, LO_EXT, XML_DATA_TABLE_SHOW_OUTLINE, XML_TYPE_BOOL ),
    MAP_ENTRY_ODF_EXT_IMPORT( DATA_TABLE_HBORDER, CHART, XML_DATA_TABLE_SHOW_HORZ_BORDER, XML_TYPE_BOOL ),
    MAP_ENTRY_ODF_EXT_IMPORT( DATA_TABLE_VBORDER, CHART, XML_DATA_TABLE_SHOW_VERT_BORDER, XML_TYPE_BOOL ),
    MAP_ENTRY_ODF_EXT_IMPORT( DATA_TABLE_OUTLINE, CHART, XML_DATA_TABLE_SHOW_OUTLINE, XML_TYPE_BOOL ),
    // Chart display units flags
    MAP_ENTRY_ODF_EXT( DISPLAY_UNITS, LO_EXT, XML_CHART_DUNITS_DISPLAYUNITS, XML_TYPE_BOOL ),
    MAP_ENTRY_ODF_EXT( BUILT_IN_UNIT, LO_EXT, XML_CHART_DUNITS_BUILTINUNIT, XML_TYPE_STRING ),
    MAP_ENTRY_ODF_EXT( EXTERNAL_DATA, LO_EXT, XML_EXTERNALDATA, XML_TYPE_STRING),

    MAP_ENTRY_ODF_EXT( LABEL_BORDER_COLOR, LO_EXT, XML_LABEL_STROKE_COLOR, XML_TYPE_COLOR ),
    MAP_ENTRY_ODF_EXT( LABEL_BORDER_STYLE, LO_EXT, XML_LABEL_STROKE, XML_SCH_TYPE_LABEL_BORDER_STYLE ),
    MAP_ENTRY_ODF_EXT( LABEL_BORDER_TRANSPARENCY, LO_EXT, XML_LABEL_STROKE_OPACITY, XML_SCH_TYPE_LABEL_BORDER_OPACITY ),
    MAP_ENTRY_ODF_EXT( LABEL_BORDER_WIDTH, LO_EXT, XML_LABEL_STROKE_WIDTH, XML_TYPE_MEASURE ),

    MAP_ENTRY_ODF_EXT( LABEL_FILL_COLOR, LO_EXT, XML_LABEL_FILL_COLOR, XML_TYPE_COLOR ),
    MAP_ENTRY_ODF_EXT( LABEL_FILL_STYLE, LO_EXT, XML_LABEL_FILL, XML_SCH_TYPE_LABEL_FILL_STYLE ),
    MAP_ENTRY_ODF_EXT( LABEL_FILL_BACKGROUND, LO_EXT, XML_FILL_HATCH_SOLID, XML_TYPE_BOOL ),
    MAP_ENTRY_ODF_EXT( LABEL_FILL_HATCH_NAME, LO_EXT, XML_FILL_HATCH_NAME, XML_TYPE_STYLENAME),

    MAP_ENTRY( SCALE_TEXT, CHART, XML_SCALE_TEXT, XML_TYPE_BOOL ),

    // spline settings
    MAP_ENTRY( SPLINE_ORDER, CHART, XML_SPLINE_ORDER, XML_TYPE_NUMBER ),
    MAP_ENTRY( SPLINE_RESOLUTION, CHART, XML_SPLINE_RESOLUTION, XML_TYPE_NUMBER ),

    // plot-area properties
    MAP_ENTRY( DATA_ROW_SOURCE, CHART, XML_SERIES_SOURCE, XML_SCH_TYPE_DATAROWSOURCE ),
    MAP_ENTRY_ODF12( SORT_BY_XVALUES, CHART, XML_SORT_BY_X_VALUES, XML_TYPE_BOOL ),
    MAP_ENTRY_ODF12( RIGHT_ANGLED_AXES, CHART, XML_RIGHT_ANGLED_AXES, XML_TYPE_BOOL ),

    // axis properties
    MAP_ENTRY( DISPLAY_LABELS, CHART, XML_DISPLAY_LABEL, XML_TYPE_BOOL ),
    MAP_SPECIAL( MARKS, CHART, XML_TICK_MARKS_MAJOR_INNER, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, XML_SCH_CONTEXT_SPECIAL_TICKS_MAJ_INNER ),          // convert one constant
    MAP_SPECIAL( MARKS, CHART, XML_TICK_MARKS_MAJOR_OUTER, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, XML_SCH_CONTEXT_SPECIAL_TICKS_MAJ_OUTER ),          // to two bools
    MAP_SPECIAL( HELP_MARKS, CHART, XML_TICK_MARKS_MINOR_INNER, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, XML_SCH_CONTEXT_SPECIAL_TICKS_MIN_INNER ),      // see above
    MAP_SPECIAL( HELP_MARKS, CHART, XML_TICK_MARKS_MINOR_OUTER, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, XML_SCH_CONTEXT_SPECIAL_TICKS_MIN_OUTER ),
    MAP_CONTEXT( LOGARITHMIC, CHART, XML_LOGARITHMIC, XML_TYPE_BOOL, XML_SCH_CONTEXT_LOGARITHMIC ),
    MAP_CONTEXT( MIN, CHART, XML_MINIMUM, XML_TYPE_DOUBLE, XML_SCH_CONTEXT_MIN ),
    MAP_CONTEXT( MAX, CHART, XML_MAXIMUM, XML_TYPE_DOUBLE, XML_SCH_CONTEXT_MAX ),
    MAP_CONTEXT( ORIGIN, CHART, XML_ORIGIN, XML_TYPE_DOUBLE, XML_SCH_CONTEXT_ORIGIN ),
    MAP_CONTEXT( STEP_MAIN, CHART, XML_INTERVAL_MAJOR, XML_TYPE_DOUBLE, XML_SCH_CONTEXT_STEP_MAIN ),
    MAP_CONTEXT( STEP_HELP_COUNT, CHART, XML_INTERVAL_MINOR_DIVISOR, XML_TYPE_NUMBER, XML_SCH_CONTEXT_STEP_HELP_COUNT ),
    MAP_ENTRY( GAP_WIDTH, CHART, XML_GAP_WIDTH, XML_TYPE_NUMBER ),
    MAP_ENTRY( OVERLAP, CHART, XML_OVERLAP, XML_TYPE_NUMBER ),
    MAP_ENTRY( TEXT_CAN_OVERLAP, CHART, XML_TEXT_OVERLAP, XML_TYPE_BOOL ),
    MAP_ENTRY_ODF12( REVERSE_DIRECTION, CHART, XML_REVERSE_DIRECTION, XML_TYPE_BOOL ),
    MAP_ENTRY( TEXT_BREAK, TEXT, XML_LINE_BREAK, XML_TYPE_BOOL ),
    MAP_ENTRY_ODF_EXT( TRY_STAGGERING_FIRST, LO_EXT, XML_TRY_STAGGERING_FIRST, XML_TYPE_BOOL ),
    MAP_ENTRY( ARRANGE_ORDER, CHART, XML_LABEL_ARRANGEMENT, XML_SCH_TYPE_AXIS_ARRANGEMENT ),
    MAP_SPECIAL( NUMBER_FORMAT, STYLE, XML_DATA_STYLE_NAME, XML_TYPE_NUMBER, XML_SCH_CONTEXT_SPECIAL_NUMBER_FORMAT ),
    MAP_ENTRY( LINK_NUMBER_FORMAT_TO_SOURCE, CHART, XML_LINK_DATA_STYLE_TO_SOURCE, XML_TYPE_BOOL ),
    MAP_ENTRY( VISIBLE, CHART, XML_VISIBLE, XML_TYPE_BOOL ),
    MAP_ENTRY_ODF_EXT( MAJOR_ORIGIN, LO_EXT, XML_MAJOR_ORIGIN, XML_TYPE_DOUBLE ),

    MAP_FULL( CROSSOVER_POSITION, CHART, XML_AXIS_POSITION, XML_SCH_TYPE_AXIS_POSITION|MID_FLAG_MERGE_ATTRIBUTE|MID_FLAG_MULTI_PROPERTY, 0, SvtSaveOptions::ODFSVER_012 ),
    MAP_FULL( CROSSOVER_VALUE, CHART, XML_AXIS_POSITION, XML_SCH_TYPE_AXIS_POSITION_VALUE|MID_FLAG_MERGE_ATTRIBUTE|MID_FLAG_MULTI_PROPERTY, 0, SvtSaveOptions::ODFSVER_012 ),
    MAP_FULL( LABEL_POSITION, CHART, XML_AXIS_LABEL_POSITION, XML_SCH_TYPE_AXIS_LABEL_POSITION, 0, SvtSaveOptions::ODFSVER_012 ),
    MAP_FULL( MARK_POSITION, CHART, XML_TICK_MARK_POSITION, XML_SCH_TYPE_TICK_MARK_POSITION, 0, SvtSaveOptions::ODFSVER_012 ),

    // statistical properties

    MAP_ENTRY( MEAN_VALUE, CHART, XML_MEAN_VALUE, XML_TYPE_BOOL ),
    MAP_ENTRY( ERROR_MARGIN, CHART, XML_ERROR_MARGIN, XML_TYPE_DOUBLE ),
    MAP_ENTRY( POSITIVE_ERROR, CHART, XML_ERROR_LOWER_LIMIT, XML_TYPE_DOUBLE),
    MAP_ENTRY( NEGATIVE_ERROR, CHART, XML_ERROR_UPPER_LIMIT, XML_TYPE_DOUBLE),
    MAP_ENTRY( SHOW_POSITIVE_ERROR, CHART, XML_ERROR_UPPER_INDICATOR, XML_TYPE_BOOL),
    MAP_ENTRY( SHOW_NEGATIVE_ERROR, CHART, XML_ERROR_LOWER_INDICATOR, XML_TYPE_BOOL),
    MAP_ENTRY( ERROR_BAR_STYLE, CHART, XML_ERROR_CATEGORY, XML_SCH_TYPE_ERROR_BAR_STYLE ),
    MAP_ENTRY( PERCENTAGE_ERROR, CHART, XML_ERROR_PERCENTAGE, XML_TYPE_DOUBLE ),

    // regression curve properties
    MAP_SPECIAL( REGRESSION_TYPE, CHART, XML_REGRESSION_TYPE, XML_TYPE_STRING, XML_SCH_CONTEXT_SPECIAL_REGRESSION_TYPE ),
    MAP_SPECIAL_ODF13( MOVING_AVERAGE_TYPE, LO_EXT, XML_REGRESSION_MOVING_TYPE, XML_TYPE_STRING, XML_SCH_CONTEXT_SPECIAL_MOVING_AVERAGE_TYPE ),
    MAP_SPECIAL_ODF13( MOVING_AVERAGE_TYPE, CHART, XML_REGRESSION_MOVING_TYPE, XML_TYPE_STRING, XML_SCH_CONTEXT_SPECIAL_MOVING_AVERAGE_TYPE ),

    // ODF 1.3 OFFICE-3958
    MAP_ENTRY_ODF13( CURVE_NAME, XML_NAMESPACE_LO_EXT, XML_REGRESSION_CURVE_NAME, XML_TYPE_STRING ),
    MAP_ENTRY_ODF13( CURVE_NAME, XML_NAMESPACE_CHART, XML_REGRESSION_CURVE_NAME, XML_TYPE_STRING ),
    MAP_ENTRY_ODF13( POLYNOMIAL_DEGREE, XML_NAMESPACE_LO_EXT, XML_REGRESSION_MAX_DEGREE, XML_TYPE_NUMBER ),
    MAP_ENTRY_ODF13( POLYNOMIAL_DEGREE, XML_NAMESPACE_CHART, XML_REGRESSION_MAX_DEGREE, XML_TYPE_NUMBER ),
    MAP_ENTRY_ODF13( FORCE_INTERCEPT, XML_NAMESPACE_LO_EXT, XML_REGRESSION_FORCE_INTERCEPT, XML_TYPE_BOOL ),
    MAP_ENTRY_ODF13( FORCE_INTERCEPT, XML_NAMESPACE_CHART, XML_REGRESSION_FORCE_INTERCEPT, XML_TYPE_BOOL ),
    MAP_ENTRY_ODF13( INTERCEPT_VALUE, XML_NAMESPACE_LO_EXT, XML_REGRESSION_INTERCEPT_VALUE, XML_TYPE_DOUBLE ),
    MAP_ENTRY_ODF13( INTERCEPT_VALUE, XML_NAMESPACE_CHART, XML_REGRESSION_INTERCEPT_VALUE, XML_TYPE_DOUBLE ),

    // ODF 1.3 OFFICE-3959
    MAP_ENTRY_ODF13( MOVING_AVERAGE_PERIOD, XML_NAMESPACE_LO_EXT, XML_REGRESSION_PERIOD, XML_TYPE_NUMBER ),
    MAP_ENTRY_ODF13( MOVING_AVERAGE_PERIOD, XML_NAMESPACE_CHART, XML_REGRESSION_PERIOD, XML_TYPE_NUMBER ),

    MAP_ENTRY_ODF_EXT( EXTRAPOLATE_FORWARD, LO_EXT, XML_REGRESSION_EXTRAPOLATE_FORWARD, XML_TYPE_DOUBLE ),
    MAP_ENTRY_ODF_EXT( EXTRAPOLATE_BACKWARD, LO_EXT, XML_REGRESSION_EXTRAPOLATE_BACKWARD, XML_TYPE_DOUBLE ),
    MAP_ENTRY_ODF_EXT_IMPORT( EXTRAPOLATE_FORWARD, CHART, XML_REGRESSION_EXTRAPOLATE_FORWARD, XML_TYPE_DOUBLE ),
    MAP_ENTRY_ODF_EXT_IMPORT( EXTRAPOLATE_BACKWARD, CHART, XML_REGRESSION_EXTRAPOLATE_BACKWARD, XML_TYPE_DOUBLE ),

    MAP_ENTRY_ODF_EXT( X_NAME, LO_EXT, XML_REGRESSION_X_NAME, XML_TYPE_STRING ),
    MAP_ENTRY_ODF_EXT( Y_NAME, LO_EXT, XML_REGRESSION_Y_NAME, XML_TYPE_STRING ),

    MAP_SPECIAL_ODF12( ERROR_BAR_RANGE_NEGATIVE, CHART, XML_ERROR_UPPER_RANGE, XML_TYPE_STRING, XML_SCH_CONTEXT_SPECIAL_ERRORBAR_RANGE ), // export only
    MAP_SPECIAL_ODF12( ERROR_BAR_RANGE_NEGATIVE, CHART, XML_ERROR_LOWER_RANGE, XML_TYPE_STRING, XML_SCH_CONTEXT_SPECIAL_ERRORBAR_RANGE ), // export only

    // errorbars properties (chart2)
    MAP_ENTRY_ODF_EXT( WEIGHT, LO_EXT, XML_ERROR_STANDARD_WEIGHT, XML_TYPE_DOUBLE),
    MAP_ENTRY_ODF_EXT_IMPORT( WEIGHT, CHART, XML_ERROR_STANDARD_WEIGHT, XML_TYPE_DOUBLE),

    // series/data-point properties
    MAP_SPECIAL( DATA_CAPTION, CHART, XML_DATA_LABEL_NUMBER, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, XML_SCH_CONTEXT_SPECIAL_DATA_LABEL_NUMBER ),   // convert one constant
    MAP_SPECIAL( DATA_CAPTION, CHART, XML_DATA_LABEL_TEXT, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, XML_SCH_CONTEXT_SPECIAL_DATA_LABEL_TEXT ),       // to 'tristate' and two bools
    MAP_SPECIAL( DATA_CAPTION, CHART, XML_DATA_LABEL_SYMBOL, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, XML_SCH_CONTEXT_SPECIAL_DATA_LABEL_SYMBOL ),
    MAP_SPECIAL_ODF13( DATA_CAPTION, CHART, XML_DATA_LABEL_SERIES, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, XML_SCH_CONTEXT_SPECIAL_DATA_LABEL_SERIES ),
    MAP_SPECIAL_ODF12( LABEL_SEPARATOR, CHART, XML_LABEL_SEPARATOR, XML_TYPE_STRING | MID_FLAG_ELEMENT_ITEM, XML_SCH_CONTEXT_SPECIAL_LABEL_SEPARATOR ),
    MAP_ENTRY_ODF12( LABEL_PLACEMENT, CHART, XML_LABEL_POSITION, XML_SCH_TYPE_LABEL_PLACEMENT_TYPE ),
    MAP_ENTRY( SEGMENT_OFFSET, CHART, XML_PIE_OFFSET, XML_TYPE_NUMBER ),
    MAP_SPECIAL_ODF12( PERCENTAGE_NUMBER_FORMAT, STYLE, XML_PERCENTAGE_DATA_STYLE_NAME, XML_TYPE_NUMBER, XML_SCH_CONTEXT_SPECIAL_NUMBER_FORMAT ),
    MAP_ENTRY_ODF_EXT( SHOW_CUSTOM_LEADER_LINES, LO_EXT, XML_CUSTOM_LEADERLINES, XML_TYPE_BOOL ),

    // text properties for titles
    MAP_SPECIAL( TEXT_ROTATION, STYLE, XML_ROTATION_ANGLE, XML_TYPE_NUMBER, XML_SCH_CONTEXT_SPECIAL_TEXT_ROTATION ),   // convert 1/100th degrees to degrees
    MAP_ENTRY( STACKED_TEXT, STYLE, XML_DIRECTION, XML_SCH_TYPE_TEXT_ORIENTATION ),

    // for compatibility to pre 6.0beta documents
//     MAP_SPECIAL( "SymbolBitmap", CHART, XML_SYMBOL_IMAGE_NAME, XML_TYPE_STRING, XML_SCH_CONTEXT_SPECIAL_SYMBOL_IMAGE_NAME ),

    MAP_ENTRY( CHART_USER_DEFINED_ATTRIBUTES, TEXT, XML_XMLNS, XML_TYPE_ATTRIBUTE_CONTAINER | MID_FLAG_SPECIAL_ITEM ),

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

namespace {

SvXMLEnumMapEntry<drawing::LineStyle> const aLineStyleMap[] =
{
    { XML_NONE,     drawing::LineStyle_NONE },
    { XML_SOLID,    drawing::LineStyle_SOLID },
    { XML_DASH,     drawing::LineStyle_DASH },
    { XML_TOKEN_INVALID, drawing::LineStyle(0) }
};

SvXMLEnumMapEntry<drawing::FillStyle> const aFillStyleMap[] =
{
    { XML_NONE,     drawing::FillStyle_NONE },
    { XML_SOLID,    drawing::FillStyle_SOLID },
    { XML_HATCH,    drawing::FillStyle_HATCH }
};

}

// the following class implementations are in this file:

// * XMLChartPropHdlFactory
// * XMLChartPropertySetMapper
// * XMLChartExportPropertyMapper
// * XMLChartImportPropertyMapper
// * SchXMLStyleExport

XMLChartPropHdlFactory::XMLChartPropHdlFactory(SvXMLExport const*const pExport)
    : m_pExport(pExport)
{
}

XMLChartPropHdlFactory::~XMLChartPropHdlFactory()
{
}

const XMLPropertyHandler* XMLChartPropHdlFactory::GetPropertyHandler( sal_Int32 nType ) const
{
    const XMLPropertyHandler* pHdl = XMLPropertyHandlerFactory::GetPropertyHandler( nType );
    if( ! pHdl )
    {
        switch( nType )
        {
            case XML_SCH_TYPE_AXIS_POSITION:
                pHdl = new XMLAxisPositionPropertyHdl( false );
                break;
            case XML_SCH_TYPE_AXIS_POSITION_VALUE:
                pHdl = new XMLAxisPositionPropertyHdl( true );
                break;

            case XML_SCH_TYPE_AXIS_LABEL_POSITION:
                pHdl = new XMLEnumPropertyHdl( aXMLChartAxisLabelPositionEnumMap);
                break;

            case XML_SCH_TYPE_TICK_MARK_POSITION:
                pHdl = new XMLEnumPropertyHdl( aXMLChartAxisMarkPositionEnumMap);
                break;

            case XML_SCH_TYPE_AXIS_ARRANGEMENT:
                pHdl = new XMLEnumPropertyHdl( aXMLChartAxisArrangementEnumMap);
                break;

            case XML_SCH_TYPE_ERROR_BAR_STYLE:
                // here we have a constant rather than an enum
                pHdl = new XMLErrorBarStylePropertyHdl( aXMLChartErrorBarStyleEnumMap );
                break;

            case XML_SCH_TYPE_ERROR_INDICATOR_LOWER:
                pHdl = new XMLErrorIndicatorPropertyHdl( false );
                break;
            case XML_SCH_TYPE_ERROR_INDICATOR_UPPER:
                pHdl = new XMLErrorIndicatorPropertyHdl( true );
                break;

            case XML_SCH_TYPE_SOLID_TYPE:
                // here we have a constant rather than an enum
                pHdl = new XMLEnumPropertyHdl( aXMLChartSolidTypeEnumMap );
                break;
            case XML_SCH_TYPE_LABEL_PLACEMENT_TYPE:
                // here we have a constant rather than an enum
                pHdl = new XMLEnumPropertyHdl( aXMLChartDataLabelPlacementEnumMap );
                break;
            case XML_SCH_TYPE_DATAROWSOURCE:
                pHdl = new XMLEnumPropertyHdl( aXMLChartDataRowSourceTypeEnumMap);
                break;
            case XML_SCH_TYPE_TEXT_ORIENTATION:
                pHdl = new XMLTextOrientationHdl;
                break;

            case XML_SCH_TYPE_INTERPOLATION:
                if (m_pExport && m_pExport->getSaneDefaultVersion() < SvtSaveOptions::ODFSVER_013)
                {
                    pHdl = new XMLEnumPropertyHdl(g_XMLChartInterpolationTypeEnumMap_ODF12);
                }
                else // ODF 1.3 OFFICE-3662
                {
                    pHdl = new XMLEnumPropertyHdl(g_XMLChartInterpolationTypeEnumMap);
                }
                break;
            case XML_SCH_TYPE_SYMBOL_TYPE:
                pHdl = new XMLSymbolTypePropertyHdl( false );
                break;

            case XML_SCH_TYPE_NAMED_SYMBOL:
                pHdl = new XMLSymbolTypePropertyHdl( true );
                break;

            case XML_SCH_TYPE_MISSING_VALUE_TREATMENT:
                pHdl = new XMLEnumPropertyHdl( aXMLChartMissingValueTreatmentEnumMap );
                break;
            case XML_SCH_TYPE_LABEL_BORDER_STYLE:
                pHdl = new XMLEnumPropertyHdl( aLineStyleMap );
            break;
            case XML_SCH_TYPE_LABEL_BORDER_OPACITY:
                pHdl = new XMLOpacityPropertyHdl(nullptr);
            break;
            case XML_SCH_TYPE_LABEL_FILL_STYLE:
                pHdl = new XMLEnumPropertyHdl( aFillStyleMap );
            break;
            default:
                ;
        }
        if( pHdl )
            PutHdlCache( nType, pHdl );
    }

    return pHdl;
}

XMLChartPropertySetMapper::XMLChartPropertySetMapper(SvXMLExport const*const pExport)
    : XMLPropertySetMapper(aXMLChartPropMap, new XMLChartPropHdlFactory(pExport), pExport != nullptr)
{
}

XMLChartPropertySetMapper::~XMLChartPropertySetMapper()
{
}

XMLChartExportPropertyMapper::XMLChartExportPropertyMapper( const rtl::Reference< XMLPropertySetMapper >& rMapper,
                                                            SvXMLExport& rExport) :
        SvXMLExportPropertyMapper( rMapper ),
        mrExport( rExport )
{
    // chain draw properties
    ChainExportMapper( XMLShapeExport::CreateShapePropMapper( rExport ));

    // chain text properties
    ChainExportMapper( XMLTextParagraphExport::CreateParaExtPropMapper( rExport ));
}

XMLChartExportPropertyMapper::~XMLChartExportPropertyMapper()
{
}

void XMLChartExportPropertyMapper::ContextFilter(
    bool bEnableFoFontFamily,
    std::vector< XMLPropertyState >& rProperties,
    const uno::Reference< beans::XPropertySet >& rPropSet ) const
{
    OUString aAutoPropName;
    bool bCheckAuto = false;

    // filter properties
    for( auto& rProperty : rProperties )
    {
        // find properties with context
        // to prevent writing this property set mnIndex member to -1
        switch( getPropertySetMapper()->GetEntryContextId( rProperty.mnIndex ))
        {
            // if Auto... is set the corresponding properties mustn't be exported
            case XML_SCH_CONTEXT_MIN:
                bCheckAuto = true;
                aAutoPropName = "AutoMin";
                break;
            case XML_SCH_CONTEXT_MAX:
                bCheckAuto = true;
                aAutoPropName = "AutoMax";
                break;
            case XML_SCH_CONTEXT_STEP_MAIN:
                bCheckAuto = true;
                aAutoPropName = "AutoStepMain";
                break;
            case XML_SCH_CONTEXT_STEP_HELP_COUNT:
                bCheckAuto = true;
                aAutoPropName = "AutoStepHelp";
                break;

            case XML_SCH_CONTEXT_ORIGIN:
                bCheckAuto = true;
                aAutoPropName = "AutoOrigin";
                break;

            // the following property is deprecated
            // element-item symbol-image is used now
            case XML_SCH_CONTEXT_SPECIAL_SYMBOL_IMAGE_NAME:
                rProperty.mnIndex = -1;
                break;

            case XML_SCH_CONTEXT_STOCK_WITH_VOLUME:
            case XML_SCH_CONTEXT_LINES_USED:
                // note this avoids export of the properties in OASIS format,
                // but also for the OOo XML Flat format (used by binfilter),
                // because there, the transformation to OOo is done after the
                // complete export of the chart in OASIS format.
                if( mrExport.getExportFlags() & SvXMLExportFlags::OASIS )
                    rProperty.mnIndex = -1;
                break;
        }

        if( bCheckAuto )
        {
            if( rPropSet.is())
            {
                try
                {
                    bool bAuto = false;
                    uno::Any aAny = rPropSet->getPropertyValue( aAutoPropName );
                    aAny >>= bAuto;
                    if( bAuto )
                        rProperty.mnIndex = -1;
                }
                catch(const beans::UnknownPropertyException&)
                {
                }
            }
            bCheckAuto = false;
        }
    }

    SvXMLExportPropertyMapper::ContextFilter(bEnableFoFontFamily, rProperties, rPropSet);
}

void XMLChartExportPropertyMapper::handleElementItem(
    SvXMLExport& rExport,
    const XMLPropertyState& rProperty, SvXmlExportFlags nFlags,
    const ::std::vector< XMLPropertyState > *pProperties,
    sal_uInt32 nIdx ) const
{
    switch( getPropertySetMapper()->GetEntryContextId( rProperty.mnIndex ))
    {
        case XML_SCH_CONTEXT_SPECIAL_SYMBOL_IMAGE:
            {
                uno::Reference<graphic::XGraphic> xGraphic;
                rProperty.maValue >>= xGraphic;

                OUString sInternalURL;
                // export as XLink reference into the package
                // if embedding is off
                if (xGraphic.is())
                {
                    OUString aOutMimeType;
                    sInternalURL = mrExport.AddEmbeddedXGraphic(xGraphic, aOutMimeType);
                }
                if (!sInternalURL.isEmpty())
                {
                    mrExport.AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, sInternalURL);
                }

                {
                    sal_uInt32 nPropIndex = rProperty.mnIndex;
                    // this is the element that has to live until the next statement
                    SvXMLElementExport aElem( mrExport,
                                              getPropertySetMapper()->GetEntryNameSpace( nPropIndex ),
                                              getPropertySetMapper()->GetEntryXMLName( nPropIndex ),
                                              true, true );

                    // export as Base64 embedded graphic
                    // if embedding is on
                    if (xGraphic.is())
                        mrExport.AddEmbeddedXGraphicAsBase64(xGraphic);
                }
            }
            break;

        case XML_SCH_CONTEXT_SPECIAL_LABEL_SEPARATOR:
            {
                OUString aSeparator;
                rProperty.maValue >>= aSeparator;

                if( !aSeparator.isEmpty() )
                {
                    sal_uInt32 nPropIndex = rProperty.mnIndex;
                    SvXMLElementExport aElem( mrExport,
                                              getPropertySetMapper()->GetEntryNameSpace( nPropIndex ),
                                              getPropertySetMapper()->GetEntryXMLName( nPropIndex ),
                                              true, true );

                    SchXMLTools::exportText( mrExport, aSeparator, true );
                }
            }
            break;

        default:
            // call parent
            SvXMLExportPropertyMapper::handleElementItem( rExport, rProperty,
                                                          nFlags, pProperties, nIdx );
            break;
    }
}

namespace {

OUString convertRange( const OUString & rRange, const uno::Reference< chart2::XChartDocument > & xDoc )
{
    OUString aResult = rRange;
    if( !xDoc.is() )
        return aResult;
    uno::Reference< chart2::data::XRangeXMLConversion > xConversion(
        xDoc->getDataProvider(), uno::UNO_QUERY );
    try
    {
        if( xConversion.is())
            aResult = xConversion->convertRangeToXML( rRange );
    }
    catch (css::lang::IllegalArgumentException&)
    {
    }

    return aResult;
}

}

void XMLChartExportPropertyMapper::handleSpecialItem(
    SvXMLAttributeList& rAttrList, const XMLPropertyState& rProperty,
    const SvXMLUnitConverter& rUnitConverter,
    const SvXMLNamespaceMap& rNamespaceMap,
    const ::std::vector< XMLPropertyState > *pProperties,
    sal_uInt32 nIdx ) const
{
    bool bHandled = false;

    sal_Int32 nContextId = getPropertySetMapper()->GetEntryContextId( rProperty.mnIndex );

    if( nContextId )
    {
        bHandled = true;

        OUString sAttrName = getPropertySetMapper()->GetEntryXMLName( rProperty.mnIndex );
        sal_uInt16 nNameSpace = getPropertySetMapper()->GetEntryNameSpace( rProperty.mnIndex );
        OUStringBuffer sValueBuffer;

        sal_Int32 nValue = 0;
        bool bValue = false;

        switch( nContextId )
        {
            case XML_SCH_CONTEXT_SPECIAL_TICKS_MAJ_INNER:
            case XML_SCH_CONTEXT_SPECIAL_TICKS_MIN_INNER:
                rProperty.maValue >>= nValue;
                bValue = (( nValue & chart::ChartAxisMarks::INNER ) == chart::ChartAxisMarks::INNER );
                ::sax::Converter::convertBool( sValueBuffer, bValue );
                break;
            case XML_SCH_CONTEXT_SPECIAL_TICKS_MAJ_OUTER:
            case XML_SCH_CONTEXT_SPECIAL_TICKS_MIN_OUTER:
                rProperty.maValue >>= nValue;
                bValue = (( nValue & chart::ChartAxisMarks::OUTER ) == chart::ChartAxisMarks::OUTER );
                ::sax::Converter::convertBool( sValueBuffer, bValue );
                break;
            case XML_SCH_CONTEXT_SPECIAL_TEXT_ROTATION:
                {
                    // convert from 100th degrees to degrees (double)
                    rProperty.maValue >>= nValue;
                    double fVal = static_cast<double>(nValue) / 100.0;
                    ::sax::Converter::convertDouble( sValueBuffer, fVal );
                }
                break;
            case XML_SCH_CONTEXT_SPECIAL_DATA_LABEL_NUMBER:
                {
                    rProperty.maValue >>= nValue;
                    if( ( nValue & chart::ChartDataCaption::VALUE ) == chart::ChartDataCaption::VALUE )
                    {
                        if( ( nValue & chart::ChartDataCaption::PERCENT ) == chart::ChartDataCaption::PERCENT )
                        {
                            const SvtSaveOptions::ODFSaneDefaultVersion nCurrentVersion(
                                    mrExport.getSaneDefaultVersion());
                            if (nCurrentVersion < SvtSaveOptions::ODFSVER_012)
                                sValueBuffer.append( GetXMLToken( XML_PERCENTAGE ));
                            else
                                sValueBuffer.append( GetXMLToken( XML_VALUE_AND_PERCENTAGE ));
                        }
                        else
                            sValueBuffer.append( GetXMLToken( XML_VALUE ));
                    }
                    else if(( nValue & chart::ChartDataCaption::PERCENT ) == chart::ChartDataCaption::PERCENT )
                        sValueBuffer.append( GetXMLToken( XML_PERCENTAGE ));
                    else
                        sValueBuffer.append( GetXMLToken( XML_NONE ));
                }
                break;
            case XML_SCH_CONTEXT_SPECIAL_DATA_LABEL_TEXT:
                rProperty.maValue >>= nValue;
                bValue = (( nValue & chart::ChartDataCaption::TEXT ) == chart::ChartDataCaption::TEXT );
                ::sax::Converter::convertBool( sValueBuffer, bValue );
                break;
            case XML_SCH_CONTEXT_SPECIAL_DATA_LABEL_SYMBOL:
                rProperty.maValue >>= nValue;
                bValue = (( nValue & chart::ChartDataCaption::SYMBOL ) == chart::ChartDataCaption::SYMBOL );
                ::sax::Converter::convertBool( sValueBuffer, bValue );
                break;
            case XML_SCH_CONTEXT_SPECIAL_DATA_LABEL_SERIES:
                rProperty.maValue >>= nValue;
                bValue = (( nValue & chart::ChartDataCaption::DATA_SERIES ) == chart::ChartDataCaption::DATA_SERIES );
                ::sax::Converter::convertBool( sValueBuffer, bValue );
                break;
            case XML_SCH_CONTEXT_SPECIAL_SYMBOL_WIDTH:
            case XML_SCH_CONTEXT_SPECIAL_SYMBOL_HEIGHT:
                {
                    awt::Size aSize;
                    rProperty.maValue >>= aSize;
                    rUnitConverter.convertMeasureToXML( sValueBuffer,
                                                   nContextId == XML_SCH_CONTEXT_SPECIAL_SYMBOL_WIDTH
                                                   ? aSize.Width
                                                   : aSize.Height );
                }
                break;

            case XML_SCH_CONTEXT_SPECIAL_NUMBER_FORMAT:
                {
                    // just for import
                    break;
                }

            case XML_SCH_CONTEXT_SPECIAL_ERRORBAR_RANGE:
                {
                    OUString aRangeStr;
                    rProperty.maValue >>= aRangeStr;
                    sValueBuffer.append(convertRange(aRangeStr, mxChartDoc));
                }
                break;
            case XML_SCH_CONTEXT_SPECIAL_REGRESSION_TYPE:
                {
                    const SvtSaveOptions::ODFSaneDefaultVersion nCurrentVersion(
                            mrExport.getSaneDefaultVersion());

                    OUString aServiceName;
                    rProperty.maValue >>= aServiceName;
                    if      (aServiceName == "com.sun.star.chart2.LinearRegressionCurve")
                        sValueBuffer.append( GetXMLToken( XML_LINEAR ));
                    else if (aServiceName == "com.sun.star.chart2.LogarithmicRegressionCurve")
                        sValueBuffer.append( GetXMLToken( XML_LOGARITHMIC ));
                    else if (aServiceName == "com.sun.star.chart2.ExponentialRegressionCurve")
                        sValueBuffer.append( GetXMLToken( XML_EXPONENTIAL ));
                    else if (aServiceName == "com.sun.star.chart2.PotentialRegressionCurve")
                        sValueBuffer.append( GetXMLToken( XML_POWER ));
                    else if (nCurrentVersion >= SvtSaveOptions::ODFSVER_013 && aServiceName == "com.sun.star.chart2.PolynomialRegressionCurve")
                    {   // ODF 1.3 OFFICE-3958
                        sValueBuffer.append( GetXMLToken( XML_POLYNOMIAL ));
                    }
                    else if (nCurrentVersion >= SvtSaveOptions::ODFSVER_013 && aServiceName == "com.sun.star.chart2.MovingAverageRegressionCurve")
                    {   // ODF 1.3 OFFICE-3959
                        sValueBuffer.append( GetXMLToken( XML_MOVING_AVERAGE ));
                    }
                }
                break;

            case XML_SCH_CONTEXT_SPECIAL_MOVING_AVERAGE_TYPE:
                {
                    rProperty.maValue >>= nValue;
                    if (nValue == MovingAverageType::Prior)
                        sValueBuffer.append( GetXMLToken( XML_PRIOR ));
                    else if (nValue == MovingAverageType::Central)
                        sValueBuffer.append( GetXMLToken( XML_CENTRAL ));
                    else if (nValue == MovingAverageType::AveragedAbscissa)
                        sValueBuffer.append( GetXMLToken( XML_AVERAGED_ABSCISSA ));
                    else // default
                        sValueBuffer.append( GetXMLToken( XML_PRIOR ));
                }
                break;

            default:
                bHandled = false;
                break;
        }

        if( !sValueBuffer.isEmpty())
        {
            OUString sValue = sValueBuffer.makeStringAndClear();
            sAttrName = rNamespaceMap.GetQNameByKey( nNameSpace, sAttrName );
            rAttrList.AddAttribute( sAttrName, sValue );
        }
    }

    if( !bHandled )
    {
        // call parent
        SvXMLExportPropertyMapper::handleSpecialItem( rAttrList, rProperty, rUnitConverter, rNamespaceMap, pProperties, nIdx );
    }
}

void XMLChartExportPropertyMapper::setChartDoc( const uno::Reference< chart2::XChartDocument >& xChartDoc )
{
    mxChartDoc = xChartDoc;
}

XMLChartImportPropertyMapper::XMLChartImportPropertyMapper( const rtl::Reference< XMLPropertySetMapper >& rMapper,
                                                            const SvXMLImport& _rImport ) :
        SvXMLImportPropertyMapper( rMapper, const_cast< SvXMLImport & >( _rImport )),
        mrImport( const_cast< SvXMLImport & > ( _rImport ))
{
    // chain shape mapper for drawing properties

    // give an empty model. It is only used for numbering rules that don't exist in chart
    uno::Reference< frame::XModel > xEmptyModel;
    ChainImportMapper( XMLShapeImportHelper::CreateShapePropMapper( xEmptyModel, mrImport ));

    //#i14365# save and load writing-mode for chart elements
    //The property TextWritingMode is mapped wrongly in the underlying draw mapper, but for draw it is necessary
    //We remove that property here only for chart thus the chart can use the correct mapping from the writer paragraph settings (attribute 'writing-mode' <-> property 'WritingMode')
    sal_Int32 nUnwantedWrongEntry = maPropMapper->FindEntryIndex( "TextWritingMode", XML_NAMESPACE_STYLE, GetXMLToken(XML_WRITING_MODE) );
    maPropMapper->RemoveEntry(nUnwantedWrongEntry);

    // do not chain text properties: on import this is done by shape mapper
    // to import old documents
}

XMLChartImportPropertyMapper::~XMLChartImportPropertyMapper()
{
}

bool XMLChartImportPropertyMapper::handleSpecialItem(
    XMLPropertyState& rProperty,
    ::std::vector< XMLPropertyState >& rProperties,
    const OUString& rValue,
    const SvXMLUnitConverter& rUnitConverter,
    const SvXMLNamespaceMap& rNamespaceMap ) const
{
    sal_Int32 nContextId = maPropMapper->GetEntryContextId( rProperty.mnIndex );
    bool bRet = (nContextId != 0);

    if( nContextId )
    {
        sal_Int32 nValue = 0;
        bool bValue = false;

        switch( nContextId )
        {
            case XML_SCH_CONTEXT_SPECIAL_TICKS_MAJ_INNER:
            case XML_SCH_CONTEXT_SPECIAL_TICKS_MIN_INNER:
                (void)::sax::Converter::convertBool( bValue, rValue );
                // modify old value
                rProperty.maValue >>= nValue;
                if( bValue )
                    SCH_XML_SETFLAG( nValue, chart::ChartAxisMarks::INNER );
                else
                    SCH_XML_UNSETFLAG( nValue, chart::ChartAxisMarks::INNER );
                rProperty.maValue <<= nValue;
                break;
            case XML_SCH_CONTEXT_SPECIAL_TICKS_MAJ_OUTER:
            case XML_SCH_CONTEXT_SPECIAL_TICKS_MIN_OUTER:
                (void)::sax::Converter::convertBool( bValue, rValue );
                // modify old value
                rProperty.maValue >>= nValue;
                if( bValue )
                    SCH_XML_SETFLAG( nValue, chart::ChartAxisMarks::OUTER );
                else
                    SCH_XML_UNSETFLAG( nValue, chart::ChartAxisMarks::OUTER );
                rProperty.maValue <<= nValue;
                break;
            case XML_SCH_CONTEXT_SPECIAL_TEXT_ROTATION:
                {
                    // convert from degrees (double) to 100th degrees (integer)
                    double fVal;
                    ::sax::Converter::convertDouble( fVal, rValue );
                    nValue = static_cast<sal_Int32>( fVal * 100.0 );
                    rProperty.maValue <<= nValue;
                }
                break;
            case XML_SCH_CONTEXT_SPECIAL_DATA_LABEL_NUMBER:
                {
                    // modify old value
                    rProperty.maValue >>= nValue;
                    if( IsXMLToken( rValue, XML_NONE ))
                        SCH_XML_UNSETFLAG( nValue, chart::ChartDataCaption::VALUE | chart::ChartDataCaption::PERCENT );
                    else if( IsXMLToken( rValue, XML_VALUE_AND_PERCENTAGE ) )
                        SCH_XML_SETFLAG( nValue, chart::ChartDataCaption::VALUE | chart::ChartDataCaption::PERCENT );
                    else if( IsXMLToken( rValue, XML_VALUE ) )
                        SCH_XML_SETFLAG( nValue, chart::ChartDataCaption::VALUE );
                    else // must be XML_PERCENTAGE
                        SCH_XML_SETFLAG( nValue, chart::ChartDataCaption::PERCENT );
                    rProperty.maValue <<= nValue;
                }
                break;
            case XML_SCH_CONTEXT_SPECIAL_DATA_LABEL_TEXT:
                rProperty.maValue >>= nValue;
                (void)::sax::Converter::convertBool( bValue, rValue );
                if( bValue )
                    SCH_XML_SETFLAG( nValue, chart::ChartDataCaption::TEXT );
                else
                    SCH_XML_UNSETFLAG( nValue, chart::ChartDataCaption::TEXT );
                rProperty.maValue <<= nValue;
                break;
            case XML_SCH_CONTEXT_SPECIAL_DATA_LABEL_SYMBOL:
                rProperty.maValue >>= nValue;
                (void)::sax::Converter::convertBool( bValue, rValue );
                if( bValue )
                    SCH_XML_SETFLAG( nValue, chart::ChartDataCaption::SYMBOL );
                else
                    SCH_XML_UNSETFLAG( nValue, chart::ChartDataCaption::SYMBOL );
                rProperty.maValue <<= nValue;
                break;
            case XML_SCH_CONTEXT_SPECIAL_DATA_LABEL_SERIES:
                rProperty.maValue >>= nValue;
                (void)::sax::Converter::convertBool( bValue, rValue );
                if( bValue )
                    SCH_XML_SETFLAG( nValue, chart::ChartDataCaption::DATA_SERIES );
                else
                    SCH_XML_UNSETFLAG( nValue, chart::ChartDataCaption::DATA_SERIES );
                rProperty.maValue <<= nValue;
                break;
            case XML_SCH_CONTEXT_SPECIAL_SYMBOL_WIDTH:
            case XML_SCH_CONTEXT_SPECIAL_SYMBOL_HEIGHT:
                {
                    awt::Size aSize;
                    rProperty.maValue >>= aSize;
                    rUnitConverter.convertMeasureToCore(
                        (nContextId == XML_SCH_CONTEXT_SPECIAL_SYMBOL_WIDTH)
                                                   ? aSize.Width
                                                   : aSize.Height,
                                                   rValue );
                    rProperty.maValue <<= aSize;
                }
                break;

            case XML_SCH_CONTEXT_SPECIAL_ERRORBAR_RANGE:
                {
                    rProperty.maValue <<= rValue;
                }
                break;

            // deprecated from 6.0 beta on
            case XML_SCH_CONTEXT_SPECIAL_SYMBOL_IMAGE_NAME:
                rProperty.maValue <<= mrImport.loadGraphicByURL(rValue);
                break;

            case XML_SCH_CONTEXT_SPECIAL_REGRESSION_TYPE:
            {
                if      (IsXMLToken( rValue, XML_LINEAR ))
                    rProperty.maValue <<= OUString("com.sun.star.chart2.LinearRegressionCurve");
                else if (IsXMLToken( rValue, XML_LOGARITHMIC))
                    rProperty.maValue <<= OUString("com.sun.star.chart2.LogarithmicRegressionCurve");
                else if (IsXMLToken( rValue, XML_EXPONENTIAL))
                    rProperty.maValue <<= OUString("com.sun.star.chart2.ExponentialRegressionCurve");
                else if (IsXMLToken( rValue, XML_POWER))
                    rProperty.maValue <<= OUString("com.sun.star.chart2.PotentialRegressionCurve");
                else if (IsXMLToken( rValue, XML_POLYNOMIAL))
                    rProperty.maValue <<= OUString("com.sun.star.chart2.PolynomialRegressionCurve");
                else if (IsXMLToken( rValue, XML_MOVING_AVERAGE))
                    rProperty.maValue <<= OUString("com.sun.star.chart2.MovingAverageRegressionCurve");
            }
            break;

            case XML_SCH_CONTEXT_SPECIAL_MOVING_AVERAGE_TYPE:
            {
                if (IsXMLToken( rValue, XML_PRIOR ))
                    rProperty.maValue <<= MovingAverageType::Prior;
                else if (IsXMLToken( rValue, XML_CENTRAL))
                    rProperty.maValue <<= MovingAverageType::Central;
                else if (IsXMLToken( rValue, XML_AVERAGED_ABSCISSA))
                    rProperty.maValue <<= MovingAverageType::AveragedAbscissa;
                else // default
                    rProperty.maValue <<= MovingAverageType::Prior;
            }
            break;

            default:
                bRet = false;
                break;
        }
    }

    // if we didn't handle it, the parent should
    if( !bRet )
    {
        // call parent
        bRet = SvXMLImportPropertyMapper::handleSpecialItem( rProperty, rProperties, rValue, rUnitConverter, rNamespaceMap );
    }

    return bRet;
}

void XMLChartImportPropertyMapper::finished( ::std::vector< XMLPropertyState >& /*rProperties*/, sal_Int32 /*nStartIndex*/, sal_Int32 /*nEndIndex*/ ) const
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
