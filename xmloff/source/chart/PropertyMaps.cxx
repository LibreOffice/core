/*************************************************************************
 *
 *  $RCSfile: PropertyMaps.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: bm $ $Date: 2001-01-11 17:00:38 $
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
#ifndef _XMLOFF_CONTEXTID_HXX_
#include "contextid.hxx"
#endif
#ifndef _XMLOFF_XMLEMENT_HXX
#include "xmlement.hxx"
#endif
#ifndef _XMLOFF_ENUMPROPERTYHANDLER_HXX
#include "EnumPropertyHdl.hxx"
#endif
#ifndef _XMLOFF_XMLCONSTANTSPROPERTYHANDLER_HXX
#include "XMLConstantsPropertyHandler.hxx"
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
#ifndef _XMLOFF_SHAPEIMPORT_HXX_
#include "shapeimport.hxx"
#endif
#ifndef _XMLOFF_NAMEDBOOLPROPERTYHANDLER_HXX
#include "NamedBoolPropertyHdl.hxx"
#endif

#ifndef _XMLERRORINDICATORPROPERTYHDL_HXX_
#include "XMLErrorIndicatorPropertyHdl.hxx"
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
#ifndef _COM_SUN_STAR_DRAWING_LINESTYLE_HPP_
#include <com/sun/star/drawing/LineStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FILLSTYLE_HPP_
#include <com/sun/star/drawing/FillStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINEJOINT_HPP_
#include <com/sun/star/drawing/LineJoint.hpp>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#define SCH_XML_SETFLAG( status, flag )     (status)|= (flag)
#define SCH_XML_UNSETFLAG( status, flag )   (status) = ((status) | (flag)) - (flag)

using namespace com::sun::star;

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

#define MAP_ENTRY( a, ns, nm, t ) { a, XML_NAMESPACE_##ns, sXML_##nm, t }
#define MAP_CONTEXT( a, ns, nm, t, c ) { a, XML_NAMESPACE_##ns, sXML_##nm, t, XML_SCH_CONTEXT_##c }
#define MAP_SPECIAL( a, ns, nm, t, c ) { a, XML_NAMESPACE_##ns, sXML_##nm, t | MID_FLAG_SPECIAL_ITEM, XML_SCH_CONTEXT_SPECIAL_##c }
#define MAP_ENTRY_END { 0,0,0,0 }

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
//  MAP_ENTRY( NUMBERFORMAT )  separate style - use HelperClass

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
    { sXML_automatic,       chart::ChartAxisArrangeOrderType_AUTO },
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

// enum maps copied from draw
SvXMLEnumMapEntry aXMLChartLineStyleEnumMap[] =
{
    { sXML_none, drawing::LineStyle_NONE },
    { sXML_solid, drawing::LineStyle_SOLID },
    { sXML_dash, drawing::LineStyle_DASH },
    { NULL, 0 }
};

SvXMLEnumMapEntry aXMLChartLineJointEnumMap[] =
{
    { sXML_none,    drawing::LineJoint_NONE },
    { sXML_miter,   drawing::LineJoint_MITER },
    { sXML_round,   drawing::LineJoint_ROUND },
    { sXML_bevel,   drawing::LineJoint_BEVEL },
    { sXML_middle,  drawing::LineJoint_MIDDLE },
    { NULL, 0 }
};

SvXMLEnumMapEntry aXMLChartFillStyleEnumMap[] =
{
    { sXML_none,    drawing::FillStyle_NONE },
    { sXML_solid,   drawing::FillStyle_SOLID },
    { sXML_bitmap,  drawing::FillStyle_BITMAP },
    { sXML_gradient,drawing::FillStyle_GRADIENT },
    { sXML_hatch,   drawing::FillStyle_HATCH },
    { NULL, 0 }
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

            case XML_SCH_TYPE_ERROR_INDICATOR_LOWER:
                pHdl = new XMLErrorIndicatorPropertyHdl( sal_False );
                break;
            case XML_SCH_TYPE_ERROR_INDICATOR_UPPER:
                pHdl = new XMLErrorIndicatorPropertyHdl( sal_True );
                break;

            case XML_SCH_TYPE_SOLID_TYPE:
                // here we have a constant rather than an enum
                pHdl = new XMLConstantsPropertyHandler( aXMLChartSolidTypeEnumMap, sXML_cuboid );
                break;

//              case XML_SCH_TYPE_FILLSTYLE:
//                  pHdl = new XMLEnumPropertyHdl( aXMLChartFillStyleEnumMap, ::getCppuType((const drawing::FillStyle*)0) );
//                  break;

//              case XML_SCH_TYPE_STROKE:
//                  pHdl = new XMLEnumPropertyHdl( aXMLChartLineStyleEnumMap, ::getCppuType((const drawing::LineStyle*)0) );
//                  break;

//              case XML_SCH_TYPE_LINEJOIN:
//                  pHdl = new XMLEnumPropertyHdl( aXMLChartLineJointEnumMap, ::getCppuType((const drawing::LineJoint*)0) );
//                  break;

//              case XML_SCH_TYPE_TEXT_CROSSEDOUT:
//              {
//                  const rtl::OUString aTrueStr( rtl::OUString::createFromAscii( sXML_crossedout_single ));
//                  const rtl::OUString aFalseStr( rtl::OUString::createFromAscii( sXML_none ));
//                  pHdl = new XMLNamedBoolPropertyHdl( aTrueStr, aFalseStr );
//                  break;
//              }
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

void XMLChartExportPropertyMapper::ContextFilter(
    std::vector< XMLPropertyState >& rProperties,
    uno::Reference< beans::XPropertySet > rPropSet ) const
{
    ::rtl::OUString aAutoPropName;
    sal_Bool bCheckAuto = sal_False;

    // filter properties
    for( std::vector< XMLPropertyState >::iterator property = rProperties.begin();
         property != rProperties.end();
         property++ )
    {
        // find properties with context
        // to prevent writing this property set mnIndex member to -1
        switch( getPropertySetMapper()->GetEntryContextId( property->mnIndex ))
        {
            case XML_SCH_CONTEXT_USER_SYMBOL:
                {
                    sal_Int32 nIndex = chart::ChartSymbolType::AUTO;
                    property->maValue >>= nIndex;
                    if( nIndex == chart::ChartSymbolType::AUTO )
                        property->mnIndex = -1;
                }
                break;

            // if Auto... is set the corresponding properties mustn't be exported
            case XML_SCH_CONTEXT_MIN:
                bCheckAuto = sal_True;
                aAutoPropName = ::rtl::OUString::createFromAscii( "AutoMin" );
                break;
            case XML_SCH_CONTEXT_MAX:
                bCheckAuto = sal_True;
                aAutoPropName = ::rtl::OUString::createFromAscii( "AutoMax" );
                break;
            case XML_SCH_CONTEXT_STEP_MAIN:
                bCheckAuto = sal_True;
                aAutoPropName = ::rtl::OUString::createFromAscii( "AutoStepMain" );
                break;
            case XML_SCH_CONTEXT_STEP_HELP:
                bCheckAuto = sal_True;
                aAutoPropName = ::rtl::OUString::createFromAscii( "AutoStepHelp" );
                break;
            case XML_SCH_CONTEXT_ORIGIN:
                bCheckAuto = sal_True;
                aAutoPropName = ::rtl::OUString::createFromAscii( "AutoOrigin" );
                break;
        }

        if( bCheckAuto )
        {
            if( rPropSet.is())
            {
                try
                {
                    sal_Bool bAuto;
                    uno::Any aAny = rPropSet->getPropertyValue( aAutoPropName );
                    aAny >>= bAuto;
                    if( bAuto )
                        property->mnIndex = -1;
                }
                catch( beans::UnknownPropertyException )
                {
                }
            }
            bCheckAuto = sal_False;
        }
    }

    SvXMLExportPropertyMapper::ContextFilter(rProperties, rPropSet);
}

void XMLChartExportPropertyMapper::handleElementItem(
    const uno::Reference< xml::sax::XDocumentHandler > & rHandler,
    const XMLPropertyState& rProperty, const SvXMLUnitConverter& rUnitConverter,
    const SvXMLNamespaceMap& rNamespaceMap, sal_uInt16 nFlags,
    const ::std::vector< XMLPropertyState > *pProperties,
    sal_uInt32 nIdx ) const
{
    // call parent
    SvXMLExportPropertyMapper::handleElementItem( rHandler, rProperty,
            rUnitConverter, rNamespaceMap, nFlags, pProperties, nIdx );
}

void XMLChartExportPropertyMapper::handleSpecialItem(
    SvXMLAttributeList& rAttrList, const XMLPropertyState& rProperty,
    const SvXMLUnitConverter& rUnitConverter,
    const SvXMLNamespaceMap& rNamespaceMap,
    const ::std::vector< XMLPropertyState > *pProperties,
    sal_uInt32 nIdx ) const
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
            case XML_SCH_CONTEXT_SPECIAL_TICKS_MAJ_INNER:
            case XML_SCH_CONTEXT_SPECIAL_TICKS_MIN_INNER:
                rProperty.maValue >>= nValue;
                bValue = (( nValue & chart::ChartAxisMarks::INNER ) == chart::ChartAxisMarks::INNER );
                SvXMLUnitConverter::convertBool( sValueBuffer, bValue );
                break;
            case XML_SCH_CONTEXT_SPECIAL_TICKS_MAJ_OUTER:
            case XML_SCH_CONTEXT_SPECIAL_TICKS_MIN_OUTER:
                rProperty.maValue >>= nValue;
                bValue = (( nValue & chart::ChartAxisMarks::OUTER ) == chart::ChartAxisMarks::OUTER );
                SvXMLUnitConverter::convertBool( sValueBuffer, bValue );
                break;
            case XML_SCH_CONTEXT_SPECIAL_TEXT_ROTATION:
                {
                    // convert from 100th degrees to degrees (double)
                    rProperty.maValue >>= nValue;
                    double fVal = (double)(nValue) / 100.0;
                    SvXMLUnitConverter::convertNumber( sValueBuffer, fVal );
                }
                break;
            case XML_SCH_CONTEXT_SPECIAL_DATA_LABEL_NUMBER:
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
            case XML_SCH_CONTEXT_SPECIAL_DATA_LABEL_TEXT:
                rProperty.maValue >>= nValue;
                bValue = (( nValue & chart::ChartDataCaption::TEXT ) == chart::ChartDataCaption::TEXT );
                SvXMLUnitConverter::convertBool( sValueBuffer, bValue );
                break;
            case XML_SCH_CONTEXT_SPECIAL_DATA_LABEL_SYMBOL:
                rProperty.maValue >>= nValue;
                bValue = (( nValue & chart::ChartDataCaption::SYMBOL ) == chart::ChartDataCaption::SYMBOL );
                SvXMLUnitConverter::convertBool( sValueBuffer, bValue );
                break;
        }
        if( sValueBuffer.getLength())
        {
            sValue = sValueBuffer.makeStringAndClear();
            sAttrName = rNamespaceMap.GetQNameByKey( nNameSpace, sAttrName );
            rAttrList.AddAttribute( sAttrName, msCDATA, sValue );
        }
    }
    else
    {
        // call parent
        SvXMLExportPropertyMapper::handleSpecialItem( rAttrList, rProperty, rUnitConverter, rNamespaceMap, pProperties, nIdx );
    }
}

// ----------------------------------------

XMLChartImportPropertyMapper::XMLChartImportPropertyMapper( const UniReference< XMLPropertySetMapper >& rMapper ) :
        SvXMLImportPropertyMapper( rMapper )
{
    // chain shape mapper for drawing properties

    // give an empty model. It is only used for numbering rules that don't exist in chart
    uno::Reference< frame::XModel > xEmptyModel;
    ChainImportMapper( XMLShapeImportHelper::CreateShapePropMapper( xEmptyModel ));
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
    sal_Int32 nContextId = maPropMapper->GetEntryContextId( rProperty.mnIndex );
    sal_Bool bRet = (nContextId != 0);

    if( nContextId )
    {
        sal_Int32 nValue = 0;
        sal_Bool bValue = sal_False;

        switch( nContextId )
        {
            case XML_SCH_CONTEXT_SPECIAL_TICKS_MAJ_INNER:
            case XML_SCH_CONTEXT_SPECIAL_TICKS_MIN_INNER:
                SvXMLUnitConverter::convertBool( bValue, rValue );
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
                SvXMLUnitConverter::convertBool( bValue, rValue );
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
                    SvXMLUnitConverter::convertNumber( fVal, rValue );
                    nValue = (sal_Int32)( fVal * 100.0 );
                    rProperty.maValue <<= nValue;
                }
                break;
            case XML_SCH_CONTEXT_SPECIAL_DATA_LABEL_NUMBER:
                {
                    // modify old value
                    rProperty.maValue >>= nValue;
                    if( rValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_none )))
                        SCH_XML_UNSETFLAG( nValue, chart::ChartDataCaption::VALUE | chart::ChartDataCaption::PERCENT );
                    else if( rValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_value )))
                        SCH_XML_SETFLAG( nValue, chart::ChartDataCaption::VALUE );
                    else // must be sXML_percentage
                        SCH_XML_SETFLAG( nValue, chart::ChartDataCaption::PERCENT );
                    rProperty.maValue <<= nValue;
                }
                break;
            case XML_SCH_CONTEXT_SPECIAL_DATA_LABEL_TEXT:
                rProperty.maValue >>= nValue;
                SvXMLUnitConverter::convertBool( bValue, rValue );
                if( bValue )
                    SCH_XML_SETFLAG( nValue, chart::ChartDataCaption::TEXT );
                else
                    SCH_XML_UNSETFLAG( nValue, chart::ChartDataCaption::TEXT );
                rProperty.maValue <<= nValue;
                break;
            case XML_SCH_CONTEXT_SPECIAL_DATA_LABEL_SYMBOL:
                rProperty.maValue >>= nValue;
                SvXMLUnitConverter::convertBool( bValue, rValue );
                if( bValue )
                    SCH_XML_SETFLAG( nValue, chart::ChartDataCaption::SYMBOL );
                else
                    SCH_XML_UNSETFLAG( nValue, chart::ChartDataCaption::SYMBOL );
                rProperty.maValue <<= nValue;
                break;
            default:
                bRet = sal_False;
                break;
        }
    }
    else
    {
        // call parent
        bRet = SvXMLImportPropertyMapper::handleSpecialItem( rProperty, rProperties, rValue, rUnitConverter, rNamespaceMap );
    }

    return bRet;
}

void XMLChartImportPropertyMapper::finished( ::std::vector< XMLPropertyState >& rProperties, sal_Int32 nStartIndex, sal_Int32 nEndIndex ) const
{
}
