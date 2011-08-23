/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _PROPERTYMAP_HXX_
#define _PROPERTYMAP_HXX_

#ifndef _XMLOFF_PROPMAPPINGTYPES_HXX
#include "maptype.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif
#ifndef _XMLOFF_XMLTYPES_HXX
#include "xmltypes.hxx"
#endif
#ifndef _XMLOFF_CONTEXTID_HXX_
#include "contextid.hxx"
#endif
#ifndef _XMLOFF_XMLEMENT_HXX
#include "xmlement.hxx"
#endif

#ifndef _COM_SUN_STAR_CHART_CHARTAXISARRANGEORDERTYPE_HPP_
#include <com/sun/star/chart/ChartAxisArrangeOrderType.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTERRORCATEGORY_HPP_
#include <com/sun/star/chart/ChartErrorCategory.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTERRORINDICATORTYPE_HPP_
#include <com/sun/star/chart/ChartErrorIndicatorType.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTREGRESSIONCURVETYPE_HPP_
#include <com/sun/star/chart/ChartRegressionCurveType.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTSOLIDTYPE_HPP_
#include <com/sun/star/chart/ChartSolidType.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTDATAROWSOURCE_HPP_
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#endif
namespace binfilter {

// custom types
#define XML_SCH_TYPE_AXIS_ARRANGEMENT		( XML_SCH_TYPES_START + 0 )
#define XML_SCH_TYPE_ERROR_CATEGORY			( XML_SCH_TYPES_START + 1 )
#define XML_SCH_TYPE_REGRESSION_TYPE		( XML_SCH_TYPES_START + 2 )
#define XML_SCH_TYPE_SOLID_TYPE				( XML_SCH_TYPES_START + 3 )
#define XML_SCH_TYPE_ERROR_INDICATOR_UPPER	( XML_SCH_TYPES_START + 4 )
#define XML_SCH_TYPE_ERROR_INDICATOR_LOWER	( XML_SCH_TYPES_START + 5 )
#define XML_SCH_TYPE_DATAROWSOURCE          ( XML_SCH_TYPES_START + 6 )
#define XML_SCH_TYPE_TEXT_ORIENTATION       ( XML_SCH_TYPES_START + 7 )

// context ids
#define XML_SCH_CONTEXT_USER_SYMBOL 				( XML_SCH_CTF_START + 0 )
#define XML_SCH_CONTEXT_MIN							( XML_SCH_CTF_START + 1 )
#define XML_SCH_CONTEXT_MAX							( XML_SCH_CTF_START + 2 )
#define XML_SCH_CONTEXT_STEP_MAIN					( XML_SCH_CTF_START + 3 )
#define XML_SCH_CONTEXT_STEP_HELP					( XML_SCH_CTF_START + 4 )
#define XML_SCH_CONTEXT_ORIGIN						( XML_SCH_CTF_START + 5 )
    
#define XML_SCH_CONTEXT_SPECIAL_TICKS_MAJ_INNER		( XML_SCH_CTF_START + 10 )
#define XML_SCH_CONTEXT_SPECIAL_TICKS_MAJ_OUTER		( XML_SCH_CTF_START + 11 )
#define XML_SCH_CONTEXT_SPECIAL_TICKS_MIN_INNER		( XML_SCH_CTF_START + 12 )
#define XML_SCH_CONTEXT_SPECIAL_TICKS_MIN_OUTER		( XML_SCH_CTF_START + 13 )
#define XML_SCH_CONTEXT_SPECIAL_TEXT_ROTATION		( XML_SCH_CTF_START + 14 )
#define XML_SCH_CONTEXT_SPECIAL_DATA_LABEL_NUMBER	( XML_SCH_CTF_START + 15 )
#define XML_SCH_CONTEXT_SPECIAL_DATA_LABEL_TEXT		( XML_SCH_CTF_START + 16 )
#define XML_SCH_CONTEXT_SPECIAL_DATA_LABEL_SYMBOL	( XML_SCH_CTF_START + 17 )
#define XML_SCH_CONTEXT_SPECIAL_NUMBER_FORMAT		( XML_SCH_CTF_START + 18 )
#define XML_SCH_CONTEXT_SPECIAL_DATA_ROW_SOURCE		( XML_SCH_CTF_START + 19 )
#define XML_SCH_CONTEXT_SPECIAL_SYMBOL_WIDTH        ( XML_SCH_CTF_START + 20 )
#define XML_SCH_CONTEXT_SPECIAL_SYMBOL_HEIGHT       ( XML_SCH_CTF_START + 21 )
#define XML_SCH_CONTEXT_SPECIAL_SYMBOL_IMAGE_NAME   ( XML_SCH_CTF_START + 22 )
#define XML_SCH_CONTEXT_SPECIAL_SYMBOL_IMAGE        ( XML_SCH_CTF_START + 23 )

#define MAP_ENTRY( a, ns, nm, t ) { a, sizeof(a)-1, XML_NAMESPACE_##ns, xmloff::token::nm, t }
#define MAP_CONTEXT( a, ns, nm, t, c ) { a, sizeof(a)-1, XML_NAMESPACE_##ns, xmloff::token::nm, t, XML_SCH_CONTEXT_##c }
#define MAP_SPECIAL( a, ns, nm, t, c ) { a, sizeof(a)-1, XML_NAMESPACE_##ns, xmloff::token::nm, t | MID_FLAG_SPECIAL_ITEM, XML_SCH_CONTEXT_SPECIAL_##c }
#define MAP_ENTRY_END { 0,0,0,xmloff::token::XML_TOKEN_INVALID,0 }

// ---------------------------------------------------------
// PropertyMap for Chart properties drawing- and
// textproperties are added later using the chaining
// mechanism
// ---------------------------------------------------------

// only create maps once!
// this define is set in PropertyMaps.cxx

#ifdef XML_SCH_CREATE_GLOBAL_MAPS

const XMLPropertyMapEntry aXMLChartPropMap[] =
{
    // chart subtypes
    MAP_ENTRY( "UpDown", CHART, XML_STOCK_UPDOWN_BARS, XML_TYPE_BOOL ),
    MAP_ENTRY( "Volume", CHART, XML_STOCK_WITH_VOLUME, XML_TYPE_BOOL ),
    MAP_ENTRY( "Dim3D", CHART, XML_THREE_DIMENSIONAL, XML_TYPE_BOOL ),
    MAP_ENTRY( "Deep", CHART, XML_DEEP, XML_TYPE_BOOL ),
    MAP_ENTRY( "Lines", CHART, XML_LINES, XML_TYPE_BOOL ),
    MAP_ENTRY( "Percent", CHART, XML_PERCENTAGE, XML_TYPE_BOOL ),
    MAP_ENTRY( "SolidType", CHART, XML_SOLID_TYPE, XML_SCH_TYPE_SOLID_TYPE ),
    MAP_ENTRY( "SplineType", CHART, XML_SPLINES, XML_TYPE_NUMBER ),
    MAP_ENTRY( "Stacked", CHART, XML_STACKED, XML_TYPE_BOOL ),
    MAP_ENTRY( "SymbolType", CHART, XML_SYMBOL, XML_TYPE_NUMBER ),
    MAP_SPECIAL( "SymbolSize", CHART, XML_SYMBOL_WIDTH, XML_TYPE_MEASURE | MID_FLAG_MERGE_PROPERTY, SYMBOL_WIDTH ),
    MAP_SPECIAL( "SymbolSize", CHART, XML_SYMBOL_HEIGHT, XML_TYPE_MEASURE | MID_FLAG_MERGE_PROPERTY, SYMBOL_HEIGHT ),
    MAP_SPECIAL( "SymbolBitmapURL", STYLE, XML_SYMBOL_IMAGE, XML_TYPE_STRING | MID_FLAG_ELEMENT_ITEM, SYMBOL_IMAGE ),
    MAP_ENTRY( "Vertical", CHART, XML_VERTICAL, XML_TYPE_BOOL ),
    MAP_ENTRY( "NumberOfLines", CHART, XML_LINES_USED, XML_TYPE_NUMBER ),
    MAP_ENTRY( "StackedBarsConnected", CHART, XML_CONNECT_BARS, XML_TYPE_BOOL ),
    // spline settings
    MAP_ENTRY( "SplineOrder", CHART, XML_SPLINE_ORDER, XML_TYPE_NUMBER ),
    MAP_ENTRY( "SplineResolution", CHART, XML_SPLINE_RESOLUTION, XML_TYPE_NUMBER ),

    // plot-area properties
    MAP_ENTRY( "DataRowSource", CHART, XML_SERIES_SOURCE, XML_SCH_TYPE_DATAROWSOURCE ),

    // axis properties
    MAP_ENTRY( "DisplayLabels", CHART, XML_DISPLAY_LABEL, XML_TYPE_BOOL ),
    MAP_SPECIAL( "Marks", CHART, XML_TICK_MARKS_MAJOR_INNER, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, TICKS_MAJ_INNER ),			// convert one constant
    MAP_SPECIAL( "Marks", CHART, XML_TICK_MARKS_MAJOR_OUTER, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, TICKS_MAJ_OUTER ),			// to two bools
    MAP_SPECIAL( "HelpMarks", CHART, XML_TICK_MARKS_MINOR_INNER, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, TICKS_MIN_INNER ),		// see above
    MAP_SPECIAL( "HelpMarks", CHART, XML_TICK_MARKS_MINOR_OUTER, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, TICKS_MIN_OUTER ),
    MAP_ENTRY( "Logarithmic", CHART, XML_LOGARITHMIC, XML_TYPE_BOOL ),
    MAP_CONTEXT( "Min", CHART, XML_MINIMUM, XML_TYPE_DOUBLE, MIN ),
    MAP_CONTEXT( "Max", CHART, XML_MAXIMUM, XML_TYPE_DOUBLE, MAX ),
    MAP_CONTEXT( "Origin", CHART, XML_ORIGIN, XML_TYPE_DOUBLE, ORIGIN ),
    MAP_CONTEXT( "StepMain", CHART, XML_INTERVAL_MAJOR, XML_TYPE_DOUBLE, STEP_MAIN ),
    MAP_CONTEXT( "StepHelp", CHART, XML_INTERVAL_MINOR, XML_TYPE_DOUBLE, STEP_HELP ),
    MAP_ENTRY( "GapWidth", CHART, XML_GAP_WIDTH, XML_TYPE_NUMBER ),
    MAP_ENTRY( "Overlap", CHART, XML_OVERLAP, XML_TYPE_NUMBER ),
    MAP_ENTRY( "TextCanOverlap", CHART, XML_TEXT_OVERLAP, XML_TYPE_BOOL ),
    MAP_ENTRY( "TextBreak", TEXT, XML_LINE_BREAK, XML_TYPE_BOOL ),
    MAP_ENTRY( "ArrangeOrder", CHART, XML_LABEL_ARRANGEMENT, XML_SCH_TYPE_AXIS_ARRANGEMENT ),
    MAP_SPECIAL( "NumberFormat", STYLE, XML_DATA_STYLE_NAME, XML_TYPE_NUMBER, NUMBER_FORMAT ),
    MAP_ENTRY( "LinkNumberFormatToSource", CHART, XML_LINK_DATA_STYLE_TO_SOURCE, XML_TYPE_BOOL ),
    MAP_ENTRY( "Visible", CHART, XML_VISIBLE, XML_TYPE_BOOL ),

    // statistical properties
    MAP_ENTRY( "MeanValue", CHART, XML_MEAN_VALUE, XML_TYPE_BOOL ),
    MAP_ENTRY( "ErrorMargin", CHART, XML_ERROR_MARGIN, XML_TYPE_DOUBLE ),
    MAP_ENTRY( "ConstantErrorLow", CHART, XML_ERROR_LOWER_LIMIT, XML_TYPE_DOUBLE ),
    MAP_ENTRY( "ConstantErrorHigh", CHART, XML_ERROR_UPPER_LIMIT, XML_TYPE_DOUBLE ),
    MAP_ENTRY( "ErrorIndicator", CHART, XML_ERROR_UPPER_INDICATOR, XML_SCH_TYPE_ERROR_INDICATOR_UPPER | MID_FLAG_MERGE_PROPERTY ),	// convert one constant
    MAP_ENTRY( "ErrorIndicator", CHART, XML_ERROR_LOWER_INDICATOR, XML_SCH_TYPE_ERROR_INDICATOR_LOWER | MID_FLAG_MERGE_PROPERTY ),	// to two bools
      MAP_ENTRY( "ErrorCategory", CHART, XML_ERROR_CATEGORY, XML_SCH_TYPE_ERROR_CATEGORY ),
      MAP_ENTRY( "PercentageError", CHART, XML_ERROR_PERCENTAGE, XML_TYPE_DOUBLE ),
      MAP_ENTRY( "RegressionCurves", CHART, XML_REGRESSION_TYPE, XML_SCH_TYPE_REGRESSION_TYPE ),

    // series/data-point properties
    MAP_SPECIAL( "DataCaption", CHART, XML_DATA_LABEL_NUMBER, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, DATA_LABEL_NUMBER ),	// convert one constant
    MAP_SPECIAL( "DataCaption", CHART, XML_DATA_LABEL_TEXT, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, DATA_LABEL_TEXT ),		// to 'tristate' and two bools
    MAP_SPECIAL( "DataCaption", CHART, XML_DATA_LABEL_SYMBOL, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, DATA_LABEL_SYMBOL ),
    MAP_ENTRY( "SegmentOffset", CHART, XML_PIE_OFFSET, XML_TYPE_NUMBER ),

    // text properties for titles
    MAP_SPECIAL( "TextRotation", TEXT, XML_ROTATION_ANGLE, XML_TYPE_NUMBER, TEXT_ROTATION ),	// convert 1/100th degrees to degrees
    MAP_ENTRY( "StackedText", FO, XML_DIRECTION, XML_SCH_TYPE_TEXT_ORIENTATION ),

    // for compatability to pre 6.0beta documents
    MAP_SPECIAL( "SymbolBitmapURL", CHART, XML_SYMBOL_IMAGE_NAME, XML_TYPE_STRING, SYMBOL_IMAGE_NAME ),

    MAP_ENTRY_END
};

// ----------------------------------------
// maps for enums to XML attributes
// ----------------------------------------

SvXMLEnumMapEntry aXMLChartAxisArrangementEnumMap[] =
{
    { ::binfilter::xmloff::token::XML_AUTOMATIC,		::com::sun::star::chart::ChartAxisArrangeOrderType_AUTO },
    { ::binfilter::xmloff::token::XML_SIDE_BY_SIDE,	    ::com::sun::star::chart::ChartAxisArrangeOrderType_SIDE_BY_SIDE },
    { ::binfilter::xmloff::token::XML_STAGGER_EVEN,	    ::com::sun::star::chart::ChartAxisArrangeOrderType_STAGGER_EVEN },
    { ::binfilter::xmloff::token::XML_STAGGER_ODD,		::com::sun::star::chart::ChartAxisArrangeOrderType_STAGGER_ODD },
    { ::binfilter::xmloff::token::XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry aXMLChartErrorCategoryEnumMap[] =
{
    { ::binfilter::xmloff::token::XML_NONE,		    		::com::sun::star::chart::ChartErrorCategory_NONE },
    { ::binfilter::xmloff::token::XML_VARIANCE,		    	::com::sun::star::chart::ChartErrorCategory_VARIANCE },
    { ::binfilter::xmloff::token::XML_STANDARD_DEVIATION,	::com::sun::star::chart::ChartErrorCategory_STANDARD_DEVIATION },
    { ::binfilter::xmloff::token::XML_PERCENTAGE,			::com::sun::star::chart::ChartErrorCategory_PERCENT },
    { ::binfilter::xmloff::token::XML_ERROR_MARGIN,		    ::com::sun::star::chart::ChartErrorCategory_ERROR_MARGIN },
    { ::binfilter::xmloff::token::XML_CONSTANT, 			::com::sun::star::chart::ChartErrorCategory_CONSTANT_VALUE },
    { ::binfilter::xmloff::token::XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry aXMLChartRegressionCurveTypeEnumMap[] =
{
    { ::binfilter::xmloff::token::XML_NONE, 		::com::sun::star::chart::ChartRegressionCurveType_NONE },
    { ::binfilter::xmloff::token::XML_LINEAR,		::com::sun::star::chart::ChartRegressionCurveType_LINEAR },
    { ::binfilter::xmloff::token::XML_LOGARITHMIC,	::com::sun::star::chart::ChartRegressionCurveType_LOGARITHM },
    { ::binfilter::xmloff::token::XML_EXPONENTIAL,	::com::sun::star::chart::ChartRegressionCurveType_EXPONENTIAL },
    { ::binfilter::xmloff::token::XML_POLYNOMIAL,	::com::sun::star::chart::ChartRegressionCurveType_POLYNOMIAL },
    { ::binfilter::xmloff::token::XML_POWER,		::com::sun::star::chart::ChartRegressionCurveType_POWER },
    { ::binfilter::xmloff::token::XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry aXMLChartSolidTypeEnumMap[] =
{
    { ::binfilter::xmloff::token::XML_CUBOID,		::com::sun::star::chart::ChartSolidType::RECTANGULAR_SOLID },
    { ::binfilter::xmloff::token::XML_CYLINDER, 	::com::sun::star::chart::ChartSolidType::CYLINDER },
    { ::binfilter::xmloff::token::XML_CONE,		    ::com::sun::star::chart::ChartSolidType::CONE },
    { ::binfilter::xmloff::token::XML_PYRAMID,		::com::sun::star::chart::ChartSolidType::PYRAMID },
    { ::binfilter::xmloff::token::XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry aXMLChartDataRowSourceTypeEnumMap[] =
{
    { ::binfilter::xmloff::token::XML_COLUMNS,     ::com::sun::star::chart::ChartDataRowSource_COLUMNS },
    { ::binfilter::xmloff::token::XML_ROWS,        ::com::sun::star::chart::ChartDataRowSource_ROWS },
    { ::binfilter::xmloff::token::XML_TOKEN_INVALID, 0 }
};
    
#endif	// XML_SCH_CREATE_GLOBAL_MAPS

}//end of namespace binfilter
#endif	// _PROPERTYMAP_HXX_
