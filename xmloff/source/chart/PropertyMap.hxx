/*************************************************************************
 *
 *  $RCSfile: PropertyMap.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: bm $ $Date: 2001-03-30 13:07:12 $
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

// custom types
#define XML_SCH_TYPE_AXIS_ARRANGEMENT       ( XML_SCH_TYPES_START + 0 )
#define XML_SCH_TYPE_ERROR_CATEGORY         ( XML_SCH_TYPES_START + 1 )
#define XML_SCH_TYPE_REGRESSION_TYPE        ( XML_SCH_TYPES_START + 2 )
#define XML_SCH_TYPE_SOLID_TYPE             ( XML_SCH_TYPES_START + 3 )
#define XML_SCH_TYPE_ERROR_INDICATOR_UPPER  ( XML_SCH_TYPES_START + 4 )
#define XML_SCH_TYPE_ERROR_INDICATOR_LOWER  ( XML_SCH_TYPES_START + 5 )

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

#define MAP_ENTRY( a, ns, nm, t ) { a, XML_NAMESPACE_##ns, sXML_##nm, t }
#define MAP_CONTEXT( a, ns, nm, t, c ) { a, XML_NAMESPACE_##ns, sXML_##nm, t, XML_SCH_CONTEXT_##c }
#define MAP_SPECIAL( a, ns, nm, t, c ) { a, XML_NAMESPACE_##ns, sXML_##nm, t | MID_FLAG_SPECIAL_ITEM, XML_SCH_CONTEXT_SPECIAL_##c }
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
    MAP_ENTRY( "UpDown", CHART, stock_updown_bars, XML_TYPE_BOOL ),
    MAP_ENTRY( "Volume", CHART, stock_with_volume, XML_TYPE_BOOL ),
    MAP_ENTRY( "Dim3D", CHART, three_dimensional, XML_TYPE_BOOL ),
    MAP_ENTRY( "Deep", CHART, deep, XML_TYPE_BOOL ),
    MAP_ENTRY( "Lines", CHART, lines, XML_TYPE_BOOL ),
    MAP_ENTRY( "Percent", CHART, percentage, XML_TYPE_BOOL ),
    MAP_ENTRY( "SolidType", CHART, solid_type, XML_SCH_TYPE_SOLID_TYPE ),
    MAP_ENTRY( "SplineType", CHART, splines, XML_TYPE_NUMBER ),
    MAP_ENTRY( "Stacked", CHART, stacked, XML_TYPE_BOOL ),
    MAP_CONTEXT( "SymbolType", CHART, symbol, XML_TYPE_NUMBER, USER_SYMBOL ),
    MAP_ENTRY( "Vertical", CHART, vertical, XML_TYPE_BOOL ),
    MAP_ENTRY( "NumberOfLines", CHART, lines_used, XML_TYPE_NUMBER ),
    MAP_ENTRY( "StackedBarsConnected", CHART, connect_bars, XML_TYPE_BOOL ),

    // axis properties
    MAP_ENTRY( "DisplayLabels", CHART, display_label, XML_TYPE_BOOL ),
    MAP_SPECIAL( "Marks", CHART, tick_marks_major_inner, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, TICKS_MAJ_INNER ),          // convert one constant
    MAP_SPECIAL( "Marks", CHART, tick_marks_major_outer, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, TICKS_MAJ_OUTER ),          // to two bools
    MAP_SPECIAL( "HelpMarks", CHART, tick_marks_minor_inner, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, TICKS_MIN_INNER ),      // see above
    MAP_SPECIAL( "HelpMarks", CHART, tick_marks_minor_outer, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, TICKS_MIN_OUTER ),
    MAP_ENTRY( "Logarithmic", CHART, logarithmic, XML_TYPE_BOOL ),
    MAP_CONTEXT( "Min", CHART, minimum, XML_TYPE_DOUBLE, MIN ),
    MAP_CONTEXT( "Max", CHART, maximum, XML_TYPE_DOUBLE, MAX ),
    MAP_CONTEXT( "Origin", CHART, origin, XML_TYPE_DOUBLE, ORIGIN ),
    MAP_CONTEXT( "StepMain", CHART, interval_major, XML_TYPE_DOUBLE, STEP_MAIN ),
    MAP_CONTEXT( "StepHelp", CHART, interval_minor, XML_TYPE_DOUBLE, STEP_HELP ),
    MAP_ENTRY( "GapWidth", CHART, gap_width, XML_TYPE_NUMBER ),
    MAP_ENTRY( "Overlap", CHART, overlap, XML_TYPE_NUMBER ),
    MAP_ENTRY( "TextBreak", TEXT, line_break, XML_TYPE_BOOL ),
    MAP_ENTRY( "ArrangeOrder", CHART, label_arrangement, XML_SCH_TYPE_AXIS_ARRANGEMENT ),
    MAP_SPECIAL( "NumberFormat", STYLE, data_style_name, XML_TYPE_NUMBER, NUMBER_FORMAT ),
    MAP_ENTRY( "Visible", CHART, visible, XML_TYPE_BOOL ),

    // statistical properties
    MAP_ENTRY( "MeanValue", CHART,  mean_value, XML_TYPE_BOOL ),
    MAP_ENTRY( "ErrorMargin", CHART, error_margin, XML_TYPE_DOUBLE ),
    MAP_ENTRY( "ConstantErrorLow", CHART, error_lower_limit, XML_TYPE_DOUBLE ),
    MAP_ENTRY( "ConstantErrorHigh", CHART, error_upper_limit, XML_TYPE_DOUBLE ),
    MAP_ENTRY( "ErrorIndicator", CHART, error_upper_indicator, XML_SCH_TYPE_ERROR_INDICATOR_UPPER | MID_FLAG_MERGE_PROPERTY ),  // convert one constant
    MAP_ENTRY( "ErrorIndicator", CHART, error_lower_indicator, XML_SCH_TYPE_ERROR_INDICATOR_LOWER | MID_FLAG_MERGE_PROPERTY ),  // to two bools
      MAP_ENTRY( "ErrorCategory", CHART, error_category, XML_SCH_TYPE_ERROR_CATEGORY ),
      MAP_ENTRY( "PercentageError", CHART, error_percentage, XML_TYPE_DOUBLE ),
      MAP_ENTRY( "RegressionCurves", CHART, regression_type, XML_SCH_TYPE_REGRESSION_TYPE ),

    // series/data-point properties
    MAP_SPECIAL( "DataCaption", CHART, data_label_number, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, DATA_LABEL_NUMBER ),   // convert one constant
    MAP_SPECIAL( "DataCaption", CHART, data_label_text, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, DATA_LABEL_TEXT ),       // to 'tristate' and two bools
    MAP_SPECIAL( "DataCaption", CHART, data_label_symbol, XML_TYPE_NUMBER | MID_FLAG_MERGE_PROPERTY, DATA_LABEL_SYMBOL ),
    MAP_ENTRY( "SegmentOffset", CHART, pie_offset, XML_TYPE_NUMBER ),

    // text properties for titles
    MAP_SPECIAL( "TextRotation", TEXT, rotation_angle, XML_TYPE_NUMBER, TEXT_ROTATION ),    // convert 1/100th degrees to degrees

    MAP_ENTRY_END
};

// ----------------------------------------
// maps for enums to XML attributes
// ----------------------------------------

SvXMLEnumMapEntry aXMLChartAxisArrangementEnumMap[] =
{
    { sXML_automatic,       ::com::sun::star::chart::ChartAxisArrangeOrderType_AUTO },
    { sXML_side_by_side,    ::com::sun::star::chart::ChartAxisArrangeOrderType_SIDE_BY_SIDE },
    { sXML_stagger_even,    ::com::sun::star::chart::ChartAxisArrangeOrderType_STAGGER_EVEN },
    { sXML_stagger_odd,     ::com::sun::star::chart::ChartAxisArrangeOrderType_STAGGER_ODD }
};

SvXMLEnumMapEntry aXMLChartErrorCategoryEnumMap[] =
{
    { sXML_none,                ::com::sun::star::chart::ChartErrorCategory_NONE },
    { sXML_variance,            ::com::sun::star::chart::ChartErrorCategory_VARIANCE },
    { sXML_standard_deviation,  ::com::sun::star::chart::ChartErrorCategory_STANDARD_DEVIATION },
    { sXML_percentage,          ::com::sun::star::chart::ChartErrorCategory_PERCENT },
    { sXML_error_margin,        ::com::sun::star::chart::ChartErrorCategory_ERROR_MARGIN },
    { sXML_constant,            ::com::sun::star::chart::ChartErrorCategory_CONSTANT_VALUE }
};

SvXMLEnumMapEntry aXMLChartRegressionCurveTypeEnumMap[] =
{
    { sXML_none,        ::com::sun::star::chart::ChartRegressionCurveType_NONE },
    { sXML_linear,      ::com::sun::star::chart::ChartRegressionCurveType_LINEAR },
    { sXML_logarithmic, ::com::sun::star::chart::ChartRegressionCurveType_LOGARITHM },
    { sXML_exponential, ::com::sun::star::chart::ChartRegressionCurveType_EXPONENTIAL },
    { sXML_polynomial,  ::com::sun::star::chart::ChartRegressionCurveType_POLYNOMIAL },
    { sXML_power,       ::com::sun::star::chart::ChartRegressionCurveType_POWER }
};

SvXMLEnumMapEntry aXMLChartSolidTypeEnumMap[] =
{
    { sXML_cuboid,      ::com::sun::star::chart::ChartSolidType::RECTANGULAR_SOLID },
    { sXML_cylinder,    ::com::sun::star::chart::ChartSolidType::CYLINDER },
    { sXML_cone,        ::com::sun::star::chart::ChartSolidType::CONE },
    { sXML_pyramid,     ::com::sun::star::chart::ChartSolidType::PYRAMID },
};

#endif  // XML_SCH_CREATE_GLOBAL_MAPS

#endif  // _PROPERTYMAP_HXX_
