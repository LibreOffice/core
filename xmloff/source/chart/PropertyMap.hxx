/*************************************************************************
 *
 *  $RCSfile: PropertyMap.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: dvo $ $Date: 2001-06-15 10:37:05 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
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

// custom types
#define XML_SCH_TYPE_AXIS_ARRANGEMENT       ( XML_SCH_TYPES_START + 0 )
#define XML_SCH_TYPE_ERROR_CATEGORY         ( XML_SCH_TYPES_START + 1 )
#define XML_SCH_TYPE_REGRESSION_TYPE        ( XML_SCH_TYPES_START + 2 )
#define XML_SCH_TYPE_SOLID_TYPE             ( XML_SCH_TYPES_START + 3 )
#define XML_SCH_TYPE_ERROR_INDICATOR_UPPER  ( XML_SCH_TYPES_START + 4 )
#define XML_SCH_TYPE_ERROR_INDICATOR_LOWER  ( XML_SCH_TYPES_START + 5 )
#define XML_SCH_TYPE_DATAROWSOURCE          ( XML_SCH_TYPES_START + 6 )

// context ids
#define XML_SCH_CONTEXT_USER_SYMBOL                 ( XML_SCH_CTF_START + 0 )
#define XML_SCH_CONTEXT_MIN                         ( XML_SCH_CTF_START + 1 )
#define XML_SCH_CONTEXT_MAX                         ( XML_SCH_CTF_START + 2 )
#define XML_SCH_CONTEXT_STEP_MAIN                   ( XML_SCH_CTF_START + 3 )
#define XML_SCH_CONTEXT_STEP_HELP                   ( XML_SCH_CTF_START + 4 )
#define XML_SCH_CONTEXT_ORIGIN                      ( XML_SCH_CTF_START + 5 )

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

#define MAP_ENTRY( a, ns, nm, t ) { a, XML_NAMESPACE_##ns, ##nm, t }
#define MAP_CONTEXT( a, ns, nm, t, c ) { a, XML_NAMESPACE_##ns, ##nm, t, XML_SCH_CONTEXT_##c }
#define MAP_SPECIAL( a, ns, nm, t, c ) { a, XML_NAMESPACE_##ns, ##nm, t | MID_FLAG_SPECIAL_ITEM, XML_SCH_CONTEXT_SPECIAL_##c }
#define MAP_ENTRY_END { 0,0,0,0 }

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
    MAP_ENTRY( "UpDown", CHART, sXML_stock_updown_bars, XML_TYPE_BOOL ),
    MAP_ENTRY( "Volume", CHART, sXML_stock_with_volume, XML_TYPE_BOOL ),
    MAP_ENTRY( "Dim3D", CHART, sXML_three_dimensional, XML_TYPE_BOOL ),
    MAP_ENTRY( "Deep", CHART, sXML_deep, XML_TYPE_BOOL ),
    MAP_ENTRY( "Lines", CHART, sXML_lines, XML_TYPE_BOOL ),
    MAP_ENTRY( "Percent", CHART, sXML_percentage, XML_TYPE_BOOL ),
    MAP_ENTRY( "SolidType", CHART, sXML_solid_type, XML_SCH_TYPE_SOLID_TYPE ),
    MAP_ENTRY( "SplineType", CHART, sXML_splines, XML_TYPE_NUMBER ),
    MAP_ENTRY( "Stacked", CHART, sXML_stacked, XML_TYPE_BOOL ),
    MAP_ENTRY( "SymbolType", CHART, sXML_symbol, XML_TYPE_NUMBER ),
    MAP_SPECIAL( "SymbolSize", CHART, sXML_symbol_width, XML_TYPE_MEASURE | MID_FLAG_MERGE_PROPERTY, SYMBOL_WIDTH ),
    MAP_SPECIAL( "SymbolSize", CHART, sXML_symbol_height, XML_TYPE_MEASURE | MID_FLAG_MERGE_PROPERTY, SYMBOL_HEIGHT ),
    MAP_SPECIAL( "SymbolBitmapURL", CHART, sXML_symbol_image_name, XML_TYPE_STRING, SYMBOL_IMAGE_NAME ),
    MAP_ENTRY( "Vertical", CHART, sXML_vertical, XML_TYPE_BOOL ),
    MAP_ENTRY( "NumberOfLines", CHART, sXML_lines_used, XML_TYPE_NUMBER ),
    MAP_ENTRY( "StackedBarsConnected", CHART, sXML_connect_bars, XML_TYPE_BOOL ),

    // plot-area properties
    MAP_ENTRY( "DataRowSource", CHART, sXML_series_source, XML_SCH_TYPE_DATAROWSOURCE ),

    // axis properties
    MAP_ENTRY( "DisplayLabels", CHART, sXML_display_label, XML_TYPE_BOOL ),
    MAP_SPECIAL( "Marks", CHART, sXML_tick_marks_major_inner, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, TICKS_MAJ_INNER ),         // convert one constant
    MAP_SPECIAL( "Marks", CHART, sXML_tick_marks_major_outer, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, TICKS_MAJ_OUTER ),         // to two bools
    MAP_SPECIAL( "HelpMarks", CHART, sXML_tick_marks_minor_inner, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, TICKS_MIN_INNER ),     // see above
    MAP_SPECIAL( "HelpMarks", CHART, sXML_tick_marks_minor_outer, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, TICKS_MIN_OUTER ),
    MAP_ENTRY( "Logarithmic", CHART, sXML_logarithmic, XML_TYPE_BOOL ),
    MAP_CONTEXT( "Min", CHART, sXML_minimum, XML_TYPE_DOUBLE, MIN ),
    MAP_CONTEXT( "Max", CHART, sXML_maximum, XML_TYPE_DOUBLE, MAX ),
    MAP_CONTEXT( "Origin", CHART, sXML_origin, XML_TYPE_DOUBLE, ORIGIN ),
    MAP_CONTEXT( "StepMain", CHART, sXML_interval_major, XML_TYPE_DOUBLE, STEP_MAIN ),
    MAP_CONTEXT( "StepHelp", CHART, sXML_interval_minor, XML_TYPE_DOUBLE, STEP_HELP ),
    MAP_ENTRY( "GapWidth", CHART, sXML_gap_width, XML_TYPE_NUMBER ),
    MAP_ENTRY( "Overlap", CHART, sXML_overlap, XML_TYPE_NUMBER ),
    MAP_ENTRY( "TextBreak", TEXT, sXML_line_break, XML_TYPE_BOOL ),
    MAP_ENTRY( "ArrangeOrder", CHART, sXML_label_arrangement, XML_SCH_TYPE_AXIS_ARRANGEMENT ),
    MAP_SPECIAL( "NumberFormat", STYLE, sXML_data_style_name, XML_TYPE_NUMBER, NUMBER_FORMAT ),
    MAP_ENTRY( "LinkNumberFormatToSource", CHART, sXML_link_data_style_to_source, XML_TYPE_BOOL ),
    MAP_ENTRY( "Visible", CHART, sXML_visible, XML_TYPE_BOOL ),

    // statistical properties
    MAP_ENTRY( "MeanValue", CHART, sXML_mean_value, XML_TYPE_BOOL ),
    MAP_ENTRY( "ErrorMargin", CHART, sXML_error_margin, XML_TYPE_DOUBLE ),
    MAP_ENTRY( "ConstantErrorLow", CHART, sXML_error_lower_limit, XML_TYPE_DOUBLE ),
    MAP_ENTRY( "ConstantErrorHigh", CHART, sXML_error_upper_limit, XML_TYPE_DOUBLE ),
    MAP_ENTRY( "ErrorIndicator", CHART, sXML_error_upper_indicator, XML_SCH_TYPE_ERROR_INDICATOR_UPPER | MID_FLAG_MERGE_PROPERTY ), // convert one constant
    MAP_ENTRY( "ErrorIndicator", CHART, sXML_error_lower_indicator, XML_SCH_TYPE_ERROR_INDICATOR_LOWER | MID_FLAG_MERGE_PROPERTY ), // to two bools
      MAP_ENTRY( "ErrorCategory", CHART, sXML_error_category, XML_SCH_TYPE_ERROR_CATEGORY ),
      MAP_ENTRY( "PercentageError", CHART, sXML_error_percentage, XML_TYPE_DOUBLE ),
      MAP_ENTRY( "RegressionCurves", CHART, sXML_regression_type, XML_SCH_TYPE_REGRESSION_TYPE ),

    // series/data-point properties
    MAP_SPECIAL( "DataCaption", CHART, sXML_data_label_number, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, DATA_LABEL_NUMBER ),  // convert one constant
    MAP_SPECIAL( "DataCaption", CHART, sXML_data_label_text, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, DATA_LABEL_TEXT ),      // to 'tristate' and two bools
    MAP_SPECIAL( "DataCaption", CHART, sXML_data_label_symbol, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, DATA_LABEL_SYMBOL ),
    MAP_ENTRY( "SegmentOffset", CHART, sXML_pie_offset, XML_TYPE_NUMBER ),

    // text properties for titles
    MAP_SPECIAL( "TextRotation", TEXT, sXML_rotation_angle, XML_TYPE_NUMBER, TEXT_ROTATION ),   // convert 1/100th degrees to degrees

    MAP_ENTRY_END
};

// ----------------------------------------
// maps for enums to XML attributes
// ----------------------------------------

SvXMLEnumMapEntry aXMLChartAxisArrangementEnumMap[] =
{
    { ::xmloff::token::XML_AUTOMATIC,       ::com::sun::star::chart::ChartAxisArrangeOrderType_AUTO },
    { ::xmloff::token::XML_SIDE_BY_SIDE,        ::com::sun::star::chart::ChartAxisArrangeOrderType_SIDE_BY_SIDE },
    { ::xmloff::token::XML_STAGGER_EVEN,        ::com::sun::star::chart::ChartAxisArrangeOrderType_STAGGER_EVEN },
    { ::xmloff::token::XML_STAGGER_ODD,     ::com::sun::star::chart::ChartAxisArrangeOrderType_STAGGER_ODD },
    { ::xmloff::token::XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry aXMLChartErrorCategoryEnumMap[] =
{
    { ::xmloff::token::XML_NONE,                    ::com::sun::star::chart::ChartErrorCategory_NONE },
    { ::xmloff::token::XML_VARIANCE,                ::com::sun::star::chart::ChartErrorCategory_VARIANCE },
    { ::xmloff::token::XML_STANDARD_DEVIATION,  ::com::sun::star::chart::ChartErrorCategory_STANDARD_DEVIATION },
    { ::xmloff::token::XML_PERCENTAGE,          ::com::sun::star::chart::ChartErrorCategory_PERCENT },
    { ::xmloff::token::XML_ERROR_MARGIN,            ::com::sun::star::chart::ChartErrorCategory_ERROR_MARGIN },
    { ::xmloff::token::XML_CONSTANT,            ::com::sun::star::chart::ChartErrorCategory_CONSTANT_VALUE },
    { ::xmloff::token::XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry aXMLChartRegressionCurveTypeEnumMap[] =
{
    { ::xmloff::token::XML_NONE,        ::com::sun::star::chart::ChartRegressionCurveType_NONE },
    { ::xmloff::token::XML_LINEAR,      ::com::sun::star::chart::ChartRegressionCurveType_LINEAR },
    { ::xmloff::token::XML_LOGARITHMIC, ::com::sun::star::chart::ChartRegressionCurveType_LOGARITHM },
    { ::xmloff::token::XML_EXPONENTIAL, ::com::sun::star::chart::ChartRegressionCurveType_EXPONENTIAL },
    { ::xmloff::token::XML_POLYNOMIAL,  ::com::sun::star::chart::ChartRegressionCurveType_POLYNOMIAL },
    { ::xmloff::token::XML_POWER,       ::com::sun::star::chart::ChartRegressionCurveType_POWER },
    { ::xmloff::token::XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry aXMLChartSolidTypeEnumMap[] =
{
    { ::xmloff::token::XML_CUBOID,      ::com::sun::star::chart::ChartSolidType::RECTANGULAR_SOLID },
    { ::xmloff::token::XML_CYLINDER,    ::com::sun::star::chart::ChartSolidType::CYLINDER },
    { ::xmloff::token::XML_CONE,            ::com::sun::star::chart::ChartSolidType::CONE },
    { ::xmloff::token::XML_PYRAMID,     ::com::sun::star::chart::ChartSolidType::PYRAMID },
    { ::xmloff::token::XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry aXMLChartDataRowSourceTypeEnumMap[] =
{
    { ::xmloff::token::XML_COLUMNS,     ::com::sun::star::chart::ChartDataRowSource_COLUMNS },
    { ::xmloff::token::XML_ROWS,        ::com::sun::star::chart::ChartDataRowSource_ROWS },
    { ::xmloff::token::XML_TOKEN_INVALID, 0 }
};

#endif  // XML_SCH_CREATE_GLOBAL_MAPS

#endif  // _PROPERTYMAP_HXX_
