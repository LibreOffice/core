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

// include PropertyMap.hxx with this define
// to create the maps
#ifndef _PROPERTYMAP_HXX_
#define XML_SCH_CREATE_GLOBAL_MAPS
#include "PropertyMap.hxx"
#undef XML_SCH_CREATE_GLOBAL_MAPS
#endif

#include <sax/tools/converter.hxx>

#include "SchXMLTools.hxx"
#include "XMLChartPropertySetMapper.hxx"
#include "XMLErrorIndicatorPropertyHdl.hxx"
#include "XMLErrorBarStylePropertyHdl.hxx"
#include "XMLTextOrientationHdl.hxx"
#include "XMLSymbolTypePropertyHdl.hxx"
#include "XMLAxisPositionPropertyHdl.hxx"

#include <xmloff/EnumPropertyHdl.hxx>
#include <xmloff/XMLConstantsPropertyHandler.hxx>
#include <xmloff/attrlist.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/shapeimport.hxx>
#include <xmloff/NamedBoolPropertyHdl.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmltoken.hxx>

#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/chart/ChartAxisMarks.hpp>
#include <com/sun/star/chart/ChartDataCaption.hpp>
#include <com/sun/star/chart/ChartSymbolType.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart/ChartAxisPosition.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/data/XRangeXMLConversion.hpp>

// header for any2enum
#include <comphelper/extract.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/math.hxx>

#define SCH_XML_SETFLAG( status, flag )     (status)|= (flag)
#define SCH_XML_UNSETFLAG( status, flag )   (status) = ((status) | (flag)) - (flag)

using namespace com::sun::star;
using namespace ::xmloff::token;

// the following class implementations are in this file:
//
// * XMLChartPropHdlFactory
// * XMLChartPropertySetMapper
// * XMLChartExportPropertyMapper
// * XMLChartImportPropertyMapper
// * SchXMLStyleExport

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
                pHdl = new XMLEnumPropertyHdl( aXMLChartAxisLabelPositionEnumMap,
                                               ::getCppuType((const chart::ChartAxisLabelPosition*)0) );
                break;

            case XML_SCH_TYPE_TICK_MARK_POSITION:
                pHdl = new XMLEnumPropertyHdl( aXMLChartAxisMarkPositionEnumMap,
                                               ::getCppuType((const chart::ChartAxisMarkPosition*)0) );
                break;

            case XML_SCH_TYPE_AXIS_ARRANGEMENT:
                pHdl = new XMLEnumPropertyHdl( aXMLChartAxisArrangementEnumMap,
                                               ::getCppuType((const chart::ChartAxisArrangeOrderType*)0) );
                break;

            case XML_SCH_TYPE_ERROR_BAR_STYLE:
                // here we have a constant rather than an enum
                pHdl = new XMLErrorBarStylePropertyHdl( aXMLChartErrorBarStyleEnumMap,
                                               ::getCppuType((const sal_Int32*)0) );
                break;

            case XML_SCH_TYPE_ERROR_INDICATOR_LOWER:
                pHdl = new XMLErrorIndicatorPropertyHdl( sal_False );
                break;
            case XML_SCH_TYPE_ERROR_INDICATOR_UPPER:
                pHdl = new XMLErrorIndicatorPropertyHdl( sal_True );
                break;

            case XML_SCH_TYPE_SOLID_TYPE:
                // here we have a constant rather than an enum
                pHdl = new XMLEnumPropertyHdl( aXMLChartSolidTypeEnumMap,
                                               ::getCppuType((const sal_Int32*)0) );
                break;
            case XML_SCH_TYPE_LABEL_PLACEMENT_TYPE:
                // here we have a constant rather than an enum
                pHdl = new XMLEnumPropertyHdl( aXMLChartDataLabelPlacementEnumMap,
                                                ::getCppuType((const sal_Int32*)0) );
                break;
            case XML_SCH_TYPE_DATAROWSOURCE:
                pHdl = new XMLEnumPropertyHdl( aXMLChartDataRowSourceTypeEnumMap,
                                               ::getCppuType((const chart::ChartDataRowSource*)0) );
                break;
            case XML_SCH_TYPE_TEXT_ORIENTATION:
                pHdl = new XMLTextOrientationHdl();
                break;

            case XML_SCH_TYPE_INTERPOLATION:
                pHdl = new XMLEnumPropertyHdl( aXMLChartInterpolationTypeEnumMap,
                                               ::getCppuType((const sal_Int32*)0) );
                break;
            case XML_SCH_TYPE_SYMBOL_TYPE:
                pHdl = new XMLSymbolTypePropertyHdl( false );
                break;

            case XML_SCH_TYPE_NAMED_SYMBOL:
                pHdl = new XMLSymbolTypePropertyHdl( true );
                break;

            case XML_SCH_TYPE_MISSING_VALUE_TREATMENT:
                pHdl = new XMLEnumPropertyHdl( aXMLChartMissingValueTreatmentEnumMap,
                                               ::getCppuType((const sal_Int32*)0) );
                break;
        }
        if( pHdl )
            PutHdlCache( nType, pHdl );
    }

    return pHdl;
}

XMLChartPropertySetMapper::XMLChartPropertySetMapper() :
        XMLPropertySetMapper( aXMLChartPropMap, new XMLChartPropHdlFactory )
{
}

XMLChartPropertySetMapper::~XMLChartPropertySetMapper()
{
}

XMLChartExportPropertyMapper::XMLChartExportPropertyMapper( const UniReference< XMLPropertySetMapper >& rMapper,
                                                            SvXMLExport& rExport) :
        SvXMLExportPropertyMapper( rMapper ),
        msTrue( GetXMLToken( XML_TRUE )),
        msFalse( GetXMLToken( XML_FALSE )),
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
    uno::Reference< beans::XPropertySet > rPropSet ) const
{
    OUString aAutoPropName;
    bool bCheckAuto = false;

    // filter properties
    for( std::vector< XMLPropertyState >::iterator property = rProperties.begin();
         property != rProperties.end();
         ++property )
    {
        // find properties with context
        // to prevent writing this property set mnIndex member to -1
        switch( getPropertySetMapper()->GetEntryContextId( property->mnIndex ))
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
            // elemet-item symbol-image is used now
            case XML_SCH_CONTEXT_SPECIAL_SYMBOL_IMAGE_NAME:
                property->mnIndex = -1;
                break;

            case XML_SCH_CONTEXT_STOCK_WITH_VOLUME:
            case XML_SCH_CONTEXT_LINES_USED:
                // note this avoids export of the properties in OASIS format,
                // but also for the OOo XML Flat format (used by binfilter),
                // because there, the transformation to OOo is done after the
                // complete export of the chart in OASIS format.
                if( mrExport.getExportFlags() & EXPORT_OASIS )
                    property->mnIndex = -1;
                break;
        }

        if( bCheckAuto )
        {
            if( rPropSet.is())
            {
                try
                {
                    sal_Bool bAuto = false;
                    uno::Any aAny = rPropSet->getPropertyValue( aAutoPropName );
                    aAny >>= bAuto;
                    if( bAuto )
                        property->mnIndex = -1;
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
    const XMLPropertyState& rProperty, sal_uInt16 nFlags,
    const ::std::vector< XMLPropertyState > *pProperties,
    sal_uInt32 nIdx ) const
{
    switch( getPropertySetMapper()->GetEntryContextId( rProperty.mnIndex ))
    {
        case XML_SCH_CONTEXT_SPECIAL_SYMBOL_IMAGE:
            {
                OUString aURLStr;
                rProperty.maValue >>= aURLStr;

                // export as XLink reference into the package
                // if embedding is off
                OUString sTempURL( mrExport.AddEmbeddedGraphicObject( aURLStr ));
                if( !sTempURL.isEmpty() )
                {
                    mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, sTempURL );
                    mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE,
                                              XML_SIMPLE );
                    mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_ACTUATE,
                                              XML_ONLOAD );
                }

                {
                    sal_uInt32 nPropIndex = rProperty.mnIndex;
                    // this is the element that has to live until the next statement
                    SvXMLElementExport aElem( mrExport,
                                              getPropertySetMapper()->GetEntryNameSpace( nPropIndex ),
                                              getPropertySetMapper()->GetEntryXMLName( nPropIndex ),
                                              sal_True, sal_True );

                    // export as Base64 embedded graphic
                    // if embedding is on
                    if( !aURLStr.isEmpty())
                        mrExport.AddEmbeddedGraphicObjectAsBase64( aURLStr );
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
                                              sal_True, sal_True );

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
    if( xConversion.is())
        aResult = xConversion->convertRangeToXML( rRange );
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
        OUString sValue;

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
                    double fVal = (double)(nValue) / 100.0;
                    ::sax::Converter::convertDouble( sValueBuffer, fVal );
                }
                break;
            case XML_SCH_CONTEXT_SPECIAL_DATA_LABEL_NUMBER:
                {
                    rProperty.maValue >>= nValue;
                    if((( nValue & chart::ChartDataCaption::VALUE ) == chart::ChartDataCaption::VALUE ))
                    {
                        if( ( nValue & chart::ChartDataCaption::PERCENT ) == chart::ChartDataCaption::PERCENT )
                        {
                            const SvtSaveOptions::ODFDefaultVersion nCurrentVersion( SvtSaveOptions().GetODFDefaultVersion() );
                            if( nCurrentVersion < SvtSaveOptions::ODFVER_012 )
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
                    OUString aServiceName;
                    rProperty.maValue >>= aServiceName;
                    if      (aServiceName == OUString("com.sun.star.chart2.LinearRegressionCurve"))
                        sValueBuffer.append( GetXMLToken( XML_LINEAR ));
                    else if (aServiceName == OUString("com.sun.star.chart2.LogarithmicRegressionCurve"))
                        sValueBuffer.append( GetXMLToken( XML_LOGARITHMIC ));
                    else if (aServiceName == OUString("com.sun.star.chart2.ExponentialRegressionCurve"))
                        sValueBuffer.append( GetXMLToken( XML_EXPONENTIAL ));
                    else if (aServiceName == OUString("com.sun.star.chart2.PotentialRegressionCurve"))
                        sValueBuffer.append( GetXMLToken( XML_POWER ));
                    else if (aServiceName == OUString("com.sun.star.chart2.PolynomialRegressionCurve"))
                        sValueBuffer.append( GetXMLToken( XML_POLYNOMIAL ));
                    else if (aServiceName == OUString("com.sun.star.chart2.MovingAverageRegressionCurve"))
                        sValueBuffer.append( GetXMLToken( XML_MOVING_AVERAGE ));
                }
                break;

            default:
                bHandled = false;
                break;
        }

        if( !sValueBuffer.isEmpty())
        {
            sValue = sValueBuffer.makeStringAndClear();
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

void XMLChartExportPropertyMapper::setChartDoc( uno::Reference< chart2::XChartDocument > xChartDoc )
{
    mxChartDoc = xChartDoc;
}

XMLChartImportPropertyMapper::XMLChartImportPropertyMapper( const UniReference< XMLPropertySetMapper >& rMapper,
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
                ::sax::Converter::convertBool( bValue, rValue );
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
                ::sax::Converter::convertBool( bValue, rValue );
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
                    nValue = (sal_Int32)( fVal * 100.0 );
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
                ::sax::Converter::convertBool( bValue, rValue );
                if( bValue )
                    SCH_XML_SETFLAG( nValue, chart::ChartDataCaption::TEXT );
                else
                    SCH_XML_UNSETFLAG( nValue, chart::ChartDataCaption::TEXT );
                rProperty.maValue <<= nValue;
                break;
            case XML_SCH_CONTEXT_SPECIAL_DATA_LABEL_SYMBOL:
                rProperty.maValue >>= nValue;
                ::sax::Converter::convertBool( bValue, rValue );
                if( bValue )
                    SCH_XML_SETFLAG( nValue, chart::ChartDataCaption::SYMBOL );
                else
                    SCH_XML_UNSETFLAG( nValue, chart::ChartDataCaption::SYMBOL );
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
                rProperty.maValue <<= mrImport.ResolveGraphicObjectURL( rValue, sal_False );
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
