/*************************************************************************
 *
 *  $RCSfile: PropertyMaps.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:07:02 $
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

#include "XMLChartPropertySetMapper.hxx"

#ifndef _XMLOFF_PROPMAPPINGTYPES_HXX
#include "maptype.hxx"
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLTYPES_HXX
#include "xmltypes.hxx"
#endif
#ifndef _XMLOFF_XMLEMENT_HXX
#include "xmlement.hxx"
#endif
#ifndef _XMLOFF_ENUMPROPERTYHANDLER_HXX
#include "EnumPropertyHdl.hxx"
#endif
#ifndef _XMLOFF_ATTRLIST_HXX
#include "attrlist.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
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
#ifndef _COM_SUN_STAR_CHART_CHARTAXISMARKS_HPP_
#include <com/sun/star/chart/ChartAxisMarks.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTDATACAPTION_HPP_
#include <com/sun/star/chart/ChartDataCaption.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTSOLIDTYPE_HPP_
#include <com/sun/star/chart/ChartSolidType.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTSYMBOLTYPE_HPP_
#include <com/sun/star/chart/ChartSymbolType.hpp>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
using namespace com::sun::star;

#define XML_SCH_TYPE_AXIS_ARRANGEMENT       ( XML_SCH_TYPES_START + 0 )
#define XML_SCH_TYPE_ERROR_CATEGORY         ( XML_SCH_TYPES_START + 1 )
#define XML_SCH_TYPE_REGRESSION_TYPE        ( XML_SCH_TYPES_START + 2 )
#define XML_SCH_TYPE_SOLID_TYPE             ( XML_SCH_TYPES_START + 3 )

#define MAP_ENTRY( a, ns, nm, t ) { a, XML_NAMESPACE_##ns, sXML_##nm, t }
#define MAP_CONTEXT( a, ns, nm, t, c ) { a, XML_NAMESPACE_##ns, sXML_##nm, t, c }
#define MAP_SPECIAL( a, ns, nm, t, c ) { a, XML_NAMESPACE_##ns, sXML_##nm, t | MID_FLAG_SPECIAL_ITEM, XML_SCH_SPECIAL_##c }
#define MAP_ENTRY_END { 0,0,0,0 }

#define XML_SCH_SPECIAL_TICKS_MAJ_INNER 1
#define XML_SCH_SPECIAL_TICKS_MAJ_OUTER 2
#define XML_SCH_SPECIAL_TICKS_MIN_INNER 3
#define XML_SCH_SPECIAL_TICKS_MIN_OUTER 4
#define XML_SCH_SPECIAL_ERROR_UPPER_INDICATOR 5
#define XML_SCH_SPECIAL_ERROR_LOWER_INDICATOR 6
#define XML_SCH_SPECIAL_TEXT_ROTATION 7
#define XML_SCH_SPECIAL_DATA_LABEL_NUMBER 8
#define XML_SCH_SPECIAL_DATA_LABEL_TEXT 9
#define XML_SCH_SPECIAL_DATA_LABEL_SYMBOL 10

#define XML_SCH_USER_SYMBOL 1001

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
    MAP_CONTEXT( "SymbolType", CHART, symbol, XML_TYPE_NUMBER, XML_SCH_USER_SYMBOL ),
    MAP_ENTRY( "Vertical", CHART, vertical, XML_TYPE_BOOL ),
    MAP_ENTRY( "NumberOfLines", CHART, lines_used, XML_TYPE_NUMBER ),
    MAP_ENTRY( "StackedBarsConnected", CHART, connect_bars, XML_TYPE_BOOL ),

    // axis properties
    MAP_ENTRY( "DisplayLabels", CHART, display_label, XML_TYPE_BOOL ),
    MAP_SPECIAL( "Marks", CHART, tick_marks_major_inner, XML_TYPE_NUMBER, TICKS_MAJ_INNER ),            // convert one constant
    MAP_SPECIAL( "Marks", CHART, tick_marks_major_outer, XML_TYPE_NUMBER, TICKS_MAJ_OUTER ),            // to two bools
    MAP_SPECIAL( "HelpMarks", CHART, tick_marks_minor_inner, XML_TYPE_NUMBER, TICKS_MIN_INNER ),        // see above
    MAP_SPECIAL( "HelpMarks", CHART, tick_marks_minor_outer, XML_TYPE_NUMBER, TICKS_MIN_OUTER ),
    MAP_ENTRY( "Logarithmic", CHART, logarithmic, XML_TYPE_BOOL ),
    MAP_ENTRY( "Max", CHART, maximum, XML_TYPE_DOUBLE ),
    MAP_ENTRY( "Min", CHART, minimum, XML_TYPE_DOUBLE ),
    MAP_ENTRY( "Origin", CHART, origin, XML_TYPE_DOUBLE ),
    MAP_ENTRY( "Step", CHART, interval_major, XML_TYPE_DOUBLE ),
    MAP_ENTRY( "StepHelp", CHART, interval_minor, XML_TYPE_DOUBLE ),
    MAP_ENTRY( "GapWidth", CHART, gap_width, XML_TYPE_NUMBER ),
    MAP_ENTRY( "Overlap", CHART, overlap, XML_TYPE_NUMBER ),
    MAP_ENTRY( "TextBreak", TEXT, line_break, XML_TYPE_BOOL ),
    MAP_ENTRY( "ArrangeOrder", CHART, label_arrangement, XML_SCH_TYPE_AXIS_ARRANGEMENT ),
//  MAP_ENTRY( NUMBERFORMAT )  separate style - use HelperClass

    // statistical properties
    MAP_ENTRY( "MeanValue", CHART,  mean_value, XML_TYPE_BOOL ),
    MAP_ENTRY( "ErrorMargin", CHART, error_margin, XML_TYPE_DOUBLE ),
    MAP_ENTRY( "ConstantErrorLow", CHART, error_lower_limit, XML_TYPE_DOUBLE ),
    MAP_ENTRY( "ConstantErrorHigh", CHART, error_upper_limit, XML_TYPE_DOUBLE ),
    MAP_SPECIAL( "ErrorIndicator", CHART, error_upper_indicator, XML_TYPE_NUMBER, ERROR_UPPER_INDICATOR ),  // convert one constant
    MAP_SPECIAL( "ErrorIndicator", CHART, error_lower_indicator, XML_TYPE_NUMBER, ERROR_LOWER_INDICATOR ),  // to two bools
//      MAP_ENTRY( "ErrorCategory", CHART,          SCHATTR_STAT_KIND_ERROR,        &::getCppuType((const chart::ChartErrorCategory*)0), 0, 0 },\
//      MAP_ENTRY( "PercentageError", CHART,                SCHATTR_STAT_PERCENT,           &::getCppuType((const double*)0),   0, 0 },\
//      MAP_ENTRY( "RegressionCurves", CHART,           SCHATTR_STAT_REGRESSTYPE,       &::getCppuType((const chart::ChartRegressionCurveType*)0), 0, 0 }

    // series/data-point properties
    MAP_SPECIAL( "DataCaption", CHART, data_label_number, XML_TYPE_NUMBER, DATA_LABEL_NUMBER ), // convert one constant
    MAP_SPECIAL( "DataCaption", CHART, data_label_text, XML_TYPE_NUMBER, DATA_LABEL_TEXT ),     // to 'tristate' and two bools
    MAP_SPECIAL( "DataCaption", CHART, data_label_symbol, XML_TYPE_NUMBER, DATA_LABEL_SYMBOL ),
    MAP_ENTRY( "SegmentOffset", CHART, pie_offset, XML_TYPE_NUMBER ),

    // text properties for titles
    MAP_SPECIAL( "TextRotation", TEXT, rotation_angle, XML_TYPE_NUMBER, TEXT_ROTATION ),    // convert 1/100th degrees to degrees

    // misc properties

    // draw properties
    // ...

    // stroke attributes
//      { "LineStyle",      XML_NAMESPACE_DRAW, sXML_stroke,                XML_SD_TYPE_STROKE, 0 },
//      { "LineDash",       XML_NAMESPACE_SVG,  sXML_stroke_dasharray,      XML_SD_TYPE_DASHARRAY, 0 },
//      { "LineWidth",      XML_NAMESPACE_SVG,  sXML_stroke_width,          XML_TYPE_MEASURE, 0 },
//      { "LineColor",      XML_NAMESPACE_SVG,  sXML_stroke_color,          XML_TYPE_COLOR, 0 },
//      { "LineStartWidth", XML_NAMESPACE_DRAW, sXML_marker_start_width,    XML_TYPE_MEASURE, 0 },
//      { "LineStartCenter",XML_NAMESPACE_DRAW, sXML_marker_start_center,   XML_TYPE_BOOL, 0 },
//      { "LineEndWidth",   XML_NAMESPACE_DRAW, sXML_marker_end_width,      XML_TYPE_MEASURE, 0 },
//      { "LineEndCenter",  XML_NAMESPACE_DRAW, sXML_marker_end_center,     XML_TYPE_BOOL, 0 },
//      { "LineJoint",      XML_NAMESPACE_SVG,  sXML_stroke_linejoin,       XML_SD_TYPE_LINEJOIN, 0 },

//      // fill attributes
//      { "FillStyle",      XML_NAMESPACE_DRAW, sXML_fill,                  XML_SD_TYPE_FILLSTYLE, 0 },
//      { "FillColor",      XML_NAMESPACE_DRAW, sXML_fill_color,            XML_TYPE_COLOR, 0 },

    MAP_ENTRY_END
};

// ----------------------------------------
// maps for enums to XML attributes
// ----------------------------------------

SvXMLEnumMapEntry aXMLChartAxisArrangementEnumMap[] =
{
    { sXML_side_by_side,    chart::ChartAxisArrangeOrderType_SIDE_BY_SIDE },
    { sXML_stagger_even,    chart::ChartAxisArrangeOrderType_STAGGER_EVEN },
    { sXML_stagger_odd,     chart::ChartAxisArrangeOrderType_STAGGER_ODD }
};

SvXMLEnumMapEntry aXMLChartErrorCategoryEnumMap[] =
{
    { sXML_none,                chart::ChartErrorCategory_NONE },
    { sXML_variance,            chart::ChartErrorCategory_VARIANCE },
    { sXML_standard_deviation,  chart::ChartErrorCategory_STANDARD_DEVIATION },
    { sXML_percentage,          chart::ChartErrorCategory_PERCENT },
    { sXML_error_margin,        chart::ChartErrorCategory_ERROR_MARGIN },
    { sXML_constant,            chart::ChartErrorCategory_CONSTANT_VALUE }
};

SvXMLEnumMapEntry aXMLChartRegressionCurveTypeEnumMap[] =
{
    { sXML_none,        chart::ChartRegressionCurveType_NONE },
    { sXML_linear,      chart::ChartRegressionCurveType_LINEAR },
    { sXML_logarithmic, chart::ChartRegressionCurveType_LOGARITHM },
    { sXML_exponential, chart::ChartRegressionCurveType_EXPONENTIAL },
    { sXML_polynomial,  chart::ChartRegressionCurveType_POLYNOMIAL },
    { sXML_power,       chart::ChartRegressionCurveType_POWER }
};

SvXMLEnumMapEntry aXMLChartSolidTypeEnumMap[] =
{
    { sXML_cuboid,      chart::ChartSolidType::RECTANGULAR_SOLID },
    { sXML_cylinder,    chart::ChartSolidType::CYLINDER },
    { sXML_cone,        chart::ChartSolidType::CONE },
    { sXML_pyramid,     chart::ChartSolidType::PYRAMID },
};

// ----------------------------------------

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
            case XML_SCH_TYPE_AXIS_ARRANGEMENT:
                pHdl = new XMLEnumPropertyHdl( aXMLChartAxisArrangementEnumMap,
                                               ::getCppuType((const chart::ChartAxisArrangeOrderType*)0) );
                break;

            case XML_SCH_TYPE_ERROR_CATEGORY:
                pHdl = new XMLEnumPropertyHdl( aXMLChartErrorCategoryEnumMap,
                                               ::getCppuType((const chart::ChartErrorCategory*)0) );
                break;

            case XML_SCH_TYPE_REGRESSION_TYPE:
                pHdl = new XMLEnumPropertyHdl( aXMLChartRegressionCurveTypeEnumMap,
                                               ::getCppuType((const chart::ChartRegressionCurveType*)0) );
                break;

            case XML_SCH_TYPE_SOLID_TYPE:
                // here we have a constant rather than an enum
                pHdl = new XMLEnumPropertyHdl( aXMLChartSolidTypeEnumMap, ::getCppuType((const sal_Int32*)0) );
                break;
        }
        if( pHdl )
            PutHdlCache( nType, pHdl );
    }

    return pHdl;
}

// ----------------------------------------

XMLChartPropertySetMapper::XMLChartPropertySetMapper() :
        XMLPropertySetMapper( aXMLChartPropMap, new XMLChartPropHdlFactory )
{
}

XMLChartPropertySetMapper::~XMLChartPropertySetMapper()
{
}

void XMLChartPropertySetMapper::ContextFilter(
    std::vector< XMLPropertyState >& rProperties,
    uno::Reference< beans::XPropertySet > rPropSet ) const
{
    // filter properties
    for( std::vector< XMLPropertyState >::iterator property = rProperties.begin();
         property != rProperties.end();
         property++ )
    {
        // find properties with context
        // to prevent writing this property set mnIndex member to -1
        switch( GetEntryContextId( property->mnIndex ))
        {
            case XML_SCH_USER_SYMBOL:
                {
                    sal_Int32 nIndex = chart::ChartSymbolType::AUTO;
                    property->maValue >>= nIndex;
                    if( nIndex == chart::ChartSymbolType::AUTO )
                        property->mnIndex = -1;
                }
                break;
        }
    }
}

// ----------------------------------------

XMLChartExportPropertyMapper::XMLChartExportPropertyMapper( const UniReference< XMLPropertySetMapper >& rMapper ) :
        SvXMLExportPropertyMapper( rMapper ),
        msCDATA( rtl::OUString::createFromAscii( sXML_CDATA )),
        msTrue( rtl::OUString::createFromAscii( sXML_true )),
        msFalse( rtl::OUString::createFromAscii( sXML_false ))
{
}

XMLChartExportPropertyMapper::~XMLChartExportPropertyMapper()
{
}

void XMLChartExportPropertyMapper::handleElementItem(
    const uno::Reference< xml::sax::XDocumentHandler > & rHandler,
    const XMLPropertyState& rProperty, const SvXMLUnitConverter& rUnitConverter,
    const SvXMLNamespaceMap& rNamespaceMap, sal_uInt16 nFlags ) const
{
    // call parent
    SvXMLExportPropertyMapper::handleElementItem( rHandler, rProperty, rUnitConverter, rNamespaceMap, nFlags );
}

void XMLChartExportPropertyMapper::handleSpecialItem(
    SvXMLAttributeList& rAttrList, const XMLPropertyState& rProperty,
    const SvXMLUnitConverter& rUnitConverter, const SvXMLNamespaceMap& rNamespaceMap ) const
{
    sal_Int32 nContextId = maPropMapper->GetEntryContextId( rProperty.mnIndex );

    if( nContextId )
    {
        rtl::OUString sAttrName = maPropMapper->GetEntryXMLName( rProperty.mnIndex );
        sal_Int32 nNameSpace = XML_NAMESPACE_CHART;
        rtl::OUStringBuffer sValueBuffer;
        rtl::OUString sValue;

        sal_Int32 nValue = 0;
        sal_Bool bValue = sal_False;

        switch( nContextId )
        {
            case XML_SCH_SPECIAL_TICKS_MAJ_INNER:
            case XML_SCH_SPECIAL_TICKS_MIN_INNER:
                rProperty.maValue >>= nValue;
                bValue = (( nValue & chart::ChartAxisMarks::INNER ) == chart::ChartAxisMarks::INNER );
                SvXMLUnitConverter::convertBool( sValueBuffer, bValue );
                break;
            case XML_SCH_SPECIAL_TICKS_MAJ_OUTER:
            case XML_SCH_SPECIAL_TICKS_MIN_OUTER:
                rProperty.maValue >>= nValue;
                bValue = (( nValue & chart::ChartAxisMarks::OUTER ) == chart::ChartAxisMarks::OUTER );
                SvXMLUnitConverter::convertBool( sValueBuffer, bValue );
                break;
            case XML_SCH_SPECIAL_ERROR_UPPER_INDICATOR:
                {
                    chart::ChartErrorIndicatorType eType;
                    rProperty.maValue >>= eType;
                    bValue = ( eType == chart::ChartErrorIndicatorType_TOP_AND_BOTTOM ||
                               eType == chart::ChartErrorIndicatorType_UPPER );
                    SvXMLUnitConverter::convertBool( sValueBuffer, bValue );
                }
                break;
            case XML_SCH_SPECIAL_ERROR_LOWER_INDICATOR:
                {
                    chart::ChartErrorIndicatorType eType;
                    rProperty.maValue >>= eType;
                    bValue = ( eType == chart::ChartErrorIndicatorType_TOP_AND_BOTTOM ||
                               eType == chart::ChartErrorIndicatorType_LOWER );
                    SvXMLUnitConverter::convertBool( sValueBuffer, bValue );
                }
                break;
            case XML_SCH_SPECIAL_TEXT_ROTATION:
                {
                    // convert from 100th degrees to degrees (double)
                    rProperty.maValue >>= nValue;
                    double fVal = (double)(nValue) / 100.0;
                    SvXMLUnitConverter::convertNumber( sValueBuffer, fVal );
                }
                break;
            case XML_SCH_SPECIAL_DATA_LABEL_NUMBER:
                {
                    rProperty.maValue >>= nValue;
                    if((( nValue & chart::ChartDataCaption::VALUE ) == chart::ChartDataCaption::VALUE ))
                        sValueBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM( sXML_value ));
                    else if(( nValue & chart::ChartDataCaption::PERCENT ) == chart::ChartDataCaption::PERCENT )
                        sValueBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM( sXML_percentage ));
                    else
                        sValueBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM( sXML_none ));
                }
                break;
            case XML_SCH_SPECIAL_DATA_LABEL_TEXT:
                rProperty.maValue >>= nValue;
                bValue = (( nValue & chart::ChartDataCaption::TEXT ) == chart::ChartDataCaption::TEXT );
                SvXMLUnitConverter::convertBool( sValueBuffer, bValue );
                break;
            case XML_SCH_SPECIAL_DATA_LABEL_SYMBOL:
                rProperty.maValue >>= nValue;
                bValue = (( nValue & chart::ChartDataCaption::SYMBOL ) == chart::ChartDataCaption::SYMBOL );
                SvXMLUnitConverter::convertBool( sValueBuffer, bValue );
                break;
        }
        sValue = sValueBuffer.makeStringAndClear();
        sAttrName = rNamespaceMap.GetQNameByKey( nNameSpace, sAttrName );
        rAttrList.AddAttribute( sAttrName, msCDATA, sValue );
    }
    else
    {
        // call parent
        SvXMLExportPropertyMapper::handleSpecialItem( rAttrList, rProperty, rUnitConverter, rNamespaceMap );
    }
}

// ----------------------------------------

XMLChartImportPropertyMapper::XMLChartImportPropertyMapper( const UniReference< XMLPropertySetMapper >& rMapper ) :
        SvXMLImportPropertyMapper( rMapper )
{
}

XMLChartImportPropertyMapper::~XMLChartImportPropertyMapper()
{
}

sal_Bool XMLChartImportPropertyMapper::handleSpecialItem(
    XMLPropertyState& rProperty,
    ::std::vector< XMLPropertyState >& rProperties,
    const ::rtl::OUString& rValue,
    const SvXMLUnitConverter& rUnitConverter,
    const SvXMLNamespaceMap& rNamespaceMap ) const
{
    return sal_False;
}

void XMLChartImportPropertyMapper::finished( ::std::vector< XMLPropertyState >& rProperties ) const
{
}

