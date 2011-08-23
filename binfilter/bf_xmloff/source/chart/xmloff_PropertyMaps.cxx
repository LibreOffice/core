/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// include PropertyMap.hxx with this define
// to create the maps
#ifndef _PROPERTYMAP_HXX_
#define XML_SCH_CREATE_GLOBAL_MAPS
#include "PropertyMap.hxx"
#undef XML_SCH_CREATE_GLOBAL_MAPS
#endif

#include "XMLChartPropertySetMapper.hxx"

#include "EnumPropertyHdl.hxx"
#include "nmspmap.hxx"
#include "xmluconv.hxx"
#include "xmlexp.hxx"

#include "XMLErrorIndicatorPropertyHdl.hxx"
#include "XMLTextOrientationHdl.hxx"

#include <com/sun/star/chart/ChartAxisMarks.hpp>
#include <com/sun/star/chart/ChartDataCaption.hpp>

// header for any2enum
namespace binfilter {

#define SCH_XML_SETFLAG( status, flag )   	(status)|= (flag)
#define SCH_XML_UNSETFLAG( status, flag )   (status) = ((status) | (flag)) - (flag)

using namespace ::com::sun::star;
using namespace ::binfilter::xmloff::token;

// the following class implementations are in this file:
//
// * XMLChartPropHdlFactory
// * XMLChartPropertySetMapper
// * XMLChartExportPropertyMapper
// * XMLChartImportPropertyMapper
// * SchXMLStyleExport

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
                pHdl = new XMLEnumPropertyHdl( aXMLChartSolidTypeEnumMap,
                                               ::getCppuType((const sal_Int32*)0) );
                break;
            case XML_SCH_TYPE_DATAROWSOURCE:
                pHdl = new XMLEnumPropertyHdl( aXMLChartDataRowSourceTypeEnumMap,
                                               ::getCppuType((const chart::ChartDataRowSource*)0) );
                break;
            case XML_SCH_TYPE_TEXT_ORIENTATION:
                pHdl = new XMLTextOrientationHdl();
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

// ----------------------------------------

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
    std::vector< XMLPropertyState >& rProperties,
    uno::Reference< beans::XPropertySet > rPropSet ) const
{
    ::rtl::OUString aAutoPropName;
    sal_Bool bCheckAuto = sal_False;

    // filter properties
    for( ::std::vector< XMLPropertyState >::iterator property = rProperties.begin();
         property != rProperties.end();
         property++ )
    {
        // find properties with context
        // to prevent writing this property set mnIndex member to -1
        switch( getPropertySetMapper()->GetEntryContextId( property->mnIndex ))
        {
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

            // the following property is deprecated
            // elemet-item symbol-image is used now
            case XML_SCH_CONTEXT_SPECIAL_SYMBOL_IMAGE_NAME:
                property->mnIndex = -1;
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
    SvXMLExport& rExport,
    const XMLPropertyState& rProperty, sal_uInt16 nFlags,
    const ::std::vector< XMLPropertyState > *pProperties,
    sal_uInt32 nIdx ) const
{
    switch( getPropertySetMapper()->GetEntryContextId( rProperty.mnIndex ))
    {
        case XML_SCH_CONTEXT_SPECIAL_SYMBOL_IMAGE:
            {
                ::rtl::OUString aURLStr;
                rProperty.maValue >>= aURLStr;

                // export as XLink reference into the package
                // if embedding is off
                ::rtl::OUString sTempURL( mrExport.AddEmbeddedGraphicObject( aURLStr ));
                if( sTempURL.getLength() )
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
                    if( aURLStr.getLength())
                        mrExport.AddEmbeddedGraphicObjectAsBase64( aURLStr );
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

void XMLChartExportPropertyMapper::handleSpecialItem(
    SvXMLAttributeList& rAttrList, const XMLPropertyState& rProperty,
    const SvXMLUnitConverter& rUnitConverter,
    const SvXMLNamespaceMap& rNamespaceMap,
    const ::std::vector< XMLPropertyState > *pProperties,
    sal_uInt32 nIdx ) const
{
    sal_Bool bHandled = sal_False;

    sal_Int32 nContextId = maPropMapper->GetEntryContextId( rProperty.mnIndex );

    if( nContextId )
    {
        bHandled = sal_True;

        ::rtl::OUString sAttrName = maPropMapper->GetEntryXMLName( rProperty.mnIndex );
        sal_uInt16 nNameSpace = maPropMapper->GetEntryNameSpace( rProperty.mnIndex );
        ::rtl::OUStringBuffer sValueBuffer;
        ::rtl::OUString sValue;

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
                    SvXMLUnitConverter::convertDouble( sValueBuffer, fVal );
                }
                break;
            case XML_SCH_CONTEXT_SPECIAL_DATA_LABEL_NUMBER:
                {
                    rProperty.maValue >>= nValue;
                    if((( nValue & chart::ChartDataCaption::VALUE ) == chart::ChartDataCaption::VALUE ))
                        sValueBuffer.append( GetXMLToken( XML_VALUE ));
                    else if(( nValue & chart::ChartDataCaption::PERCENT ) == chart::ChartDataCaption::PERCENT )
                        sValueBuffer.append( GetXMLToken( XML_PERCENTAGE ));
                    else
                        sValueBuffer.append( GetXMLToken( XML_NONE ));
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

            case XML_SCH_CONTEXT_SPECIAL_SYMBOL_WIDTH:
            case XML_SCH_CONTEXT_SPECIAL_SYMBOL_HEIGHT:
                {
                    awt::Size aSize;
                    rProperty.maValue >>= aSize;
                    rUnitConverter.convertMeasure( sValueBuffer,
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
            default:
                bHandled = sal_False;
                break;
        }

        if( sValueBuffer.getLength())
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

// ----------------------------------------

XMLChartImportPropertyMapper::XMLChartImportPropertyMapper( const UniReference< XMLPropertySetMapper >& rMapper,
                                                            const SvXMLImport& rImport ) :
        SvXMLImportPropertyMapper( rMapper, const_cast< SvXMLImport & >( rImport )),
        mrImport( const_cast< SvXMLImport & > ( rImport ))
{
    // chain shape mapper for drawing properties

    // give an empty model. It is only used for numbering rules that don't exist in chart
    uno::Reference< frame::XModel > xEmptyModel;
    ChainImportMapper( XMLShapeImportHelper::CreateShapePropMapper( xEmptyModel, mrImport ));

    // do not chain text properties: on import this is done by shape mapper
    // to import old documents
//  	ChainImportMapper( XMLTextImportHelper::CreateParaExtPropMapper());
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
    static const ::rtl::OUString sPackageProtocol( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.Package:" ) );
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
                    SvXMLUnitConverter::convertDouble( fVal, rValue );
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
                    else if( IsXMLToken( rValue, XML_VALUE ) )
                        SCH_XML_SETFLAG( nValue, chart::ChartDataCaption::VALUE );
                    else // must be XML_PERCENTAGE
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
            case XML_SCH_CONTEXT_SPECIAL_SYMBOL_WIDTH:
            case XML_SCH_CONTEXT_SPECIAL_SYMBOL_HEIGHT:
                {
                    awt::Size aSize;
                    rProperty.maValue >>= aSize;
                    rUnitConverter.convertMeasure( nContextId == XML_SCH_CONTEXT_SPECIAL_SYMBOL_WIDTH
                                                   ? aSize.Width
                                                   : aSize.Height,
                                                   rValue );
                    rProperty.maValue <<= aSize;
                }
                break;

            // deprecated from 6.0 beta on
            case XML_SCH_CONTEXT_SPECIAL_SYMBOL_IMAGE_NAME:
                rProperty.maValue <<= mrImport.ResolveGraphicObjectURL( rValue, sal_False );
                break;

            default:
                bRet = sal_False;
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

void XMLChartImportPropertyMapper::finished( ::std::vector< XMLPropertyState >& rProperties, sal_Int32 nStartIndex, sal_Int32 nEndIndex ) const
{
}
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
