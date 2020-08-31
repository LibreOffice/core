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

#include "xmlExportDocumentHandler.hxx"
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/chart2/data/XDatabaseDataProvider.hpp>
#include <com/sun/star/chart/XComplexDescriptionAccess.hpp>
#include <com/sun/star/reflection/ProxyFactory.hpp>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/documentconstants.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <xmloff/attrlist.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlement.hxx>
#include <xmloff/xmluconv.hxx>
#include <unotools/saveopt.hxx>
#include <rtl/ustrbuf.hxx>
#include <connectivity/dbtools.hxx>

namespace rptxml
{
using namespace ::com::sun::star;
using namespace ::xmloff::token;

static void lcl_exportPrettyPrinting(const uno::Reference< xml::sax::XDocumentHandler >& _xDelegatee)
{
    SvtSaveOptions aSaveOpt;
    if ( aSaveOpt.IsPrettyPrinting() )
    {
        _xDelegatee->ignorableWhitespace(" ");
    }
}

OUString lcl_createAttribute(const xmloff::token::XMLTokenEnum& _eNamespace,const xmloff::token::XMLTokenEnum& _eAttribute)
{
    return
    // ...if it's in our map, make the prefix
        xmloff::token::GetXMLToken(_eNamespace) +
        ":" +
        xmloff::token::GetXMLToken(_eAttribute);
}

static void lcl_correctCellAddress(const OUString & _sName, const uno::Reference< xml::sax::XAttributeList > & xAttribs)
{
    SvXMLAttributeList* pList = comphelper::getUnoTunnelImplementation<SvXMLAttributeList>(xAttribs);
    OUString sCellAddress = pList->getValueByName(_sName);
    const sal_Int32 nPos = sCellAddress.lastIndexOf('$');
    if ( nPos != -1 )
    {
        sCellAddress = sCellAddress.copy(0,nPos) + "$65535";
        pList->RemoveAttribute(_sName);
        pList->AddAttribute(_sName,sCellAddress);
    }
}

ExportDocumentHandler::ExportDocumentHandler(uno::Reference< uno::XComponentContext > const & context) :
    m_xContext(context)
    ,m_nColumnCount(0)
    ,m_bTableRowsStarted(false)
    ,m_bFirstRowExported(false)
    ,m_bCountColumnHeader(false)
{
}

ExportDocumentHandler::~ExportDocumentHandler()
{
    if ( m_xProxy.is() )
    {
        m_xProxy->setDelegator( nullptr );
        m_xProxy.clear();
    }
}
IMPLEMENT_GET_IMPLEMENTATION_ID(ExportDocumentHandler)

OUString SAL_CALL ExportDocumentHandler::getImplementationName(  )
{
    return "com.sun.star.comp.report.ExportDocumentHandler";
}

sal_Bool SAL_CALL ExportDocumentHandler::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL ExportDocumentHandler::getSupportedServiceNames(  )
{
    uno::Sequence< OUString > aSupported;
    if ( m_xServiceInfo.is() )
        aSupported = m_xServiceInfo->getSupportedServiceNames();
    return ::comphelper::concatSequences(uno::Sequence< OUString > { "com.sun.star.report.ExportDocumentHandler" },aSupported);
}

// xml::sax::XDocumentHandler:
void SAL_CALL ExportDocumentHandler::startDocument()
{
    m_xDelegatee->startDocument();
}

void SAL_CALL ExportDocumentHandler::endDocument()
{
    m_xDelegatee->endDocument();
}

void SAL_CALL ExportDocumentHandler::startElement(const OUString & _sName, const uno::Reference< xml::sax::XAttributeList > & xAttribs)
{
    bool bExport = true;
    if ( _sName == "office:chart" )
    {
        SvXMLAttributeList* pList = new SvXMLAttributeList();
        uno::Reference< xml::sax::XAttributeList > xNewAttribs = pList;
        OUStringBuffer sValue;
        static const SvXMLEnumMapEntry<sal_uInt16> aXML_CommandTypeEnumMap[] =
        {
            { XML_TABLE, sdb::CommandType::TABLE },
            { XML_QUERY, sdb::CommandType::QUERY },
            { XML_TOKEN_INVALID, 0 }
        };
        if ( SvXMLUnitConverter::convertEnum( sValue, static_cast<sal_uInt16>(m_xDatabaseDataProvider->getCommandType()),aXML_CommandTypeEnumMap ) )
        {
            pList->AddAttribute(lcl_createAttribute(XML_NP_RPT,XML_COMMAND_TYPE),sValue.makeStringAndClear());
        }
        const OUString sCommand = m_xDatabaseDataProvider->getCommand();
        if ( !sCommand.isEmpty() )
            pList->AddAttribute(lcl_createAttribute(XML_NP_RPT,XML_COMMAND),sCommand);

        const OUString sFilter( m_xDatabaseDataProvider->getFilter() );
        if ( !sFilter.isEmpty() )
            pList->AddAttribute(lcl_createAttribute(XML_NP_RPT,XML_FILTER),sFilter);

        const bool bEscapeProcessing( m_xDatabaseDataProvider->getEscapeProcessing() );
        if ( !bEscapeProcessing )
            pList->AddAttribute(lcl_createAttribute(XML_NP_RPT,XML_ESCAPE_PROCESSING),::xmloff::token::GetXMLToken( XML_FALSE ));

        pList->AddAttribute(lcl_createAttribute(XML_NP_OFFICE,XML_MIMETYPE),MIMETYPE_OASIS_OPENDOCUMENT_CHART_ASCII);

        m_xDelegatee->startElement(lcl_createAttribute(XML_NP_OFFICE,XML_REPORT),xNewAttribs);

        const OUString sTableCalc = lcl_createAttribute(XML_NP_TABLE,XML_CALCULATION_SETTINGS);
        m_xDelegatee->startElement(sTableCalc,nullptr);
        pList = new SvXMLAttributeList();
        uno::Reference< xml::sax::XAttributeList > xNullAttr = pList;
        pList->AddAttribute(lcl_createAttribute(XML_NP_TABLE,XML_DATE_VALUE),"1899-12-30");

        const OUString sNullDate = lcl_createAttribute(XML_NP_TABLE,XML_NULL_DATE);
        m_xDelegatee->startElement(sNullDate,xNullAttr);
        m_xDelegatee->endElement(sNullDate);
        m_xDelegatee->endElement(sTableCalc);
        bExport = false;
    }
    else if ( _sName == "table:table" )
    {
        m_xDelegatee->startElement(lcl_createAttribute(XML_NP_RPT,XML_DETAIL),nullptr);
        lcl_exportPrettyPrinting(m_xDelegatee);
    }
    else if ( _sName == "table:table-header-rows" )
    {
        m_bCountColumnHeader = true;
    }
    else if ( m_bCountColumnHeader && _sName == "table:table-cell" )
    {
        ++m_nColumnCount;
    }
    else if ( _sName == "table:table-rows" )
    {
        m_xDelegatee->startElement(_sName,xAttribs);
        exportTableRows();
        bExport = false;
        m_bTableRowsStarted = true;
        m_bFirstRowExported = true;
    }
    else if ( m_bTableRowsStarted && m_bFirstRowExported && (_sName == "table:table-row" || _sName == "table:table-cell") )
        bExport = false;
    else if ( _sName == "chart:plot-area" )
    {
        SvXMLAttributeList* pList = comphelper::getUnoTunnelImplementation<SvXMLAttributeList>(xAttribs);
        pList->RemoveAttribute("table:cell-range-address");
    }
    else if ( _sName == "chart:categories" )
    {
        static OUString s_sCellAddress(lcl_createAttribute(XML_NP_TABLE,XML_CELL_RANGE_ADDRESS));
        lcl_correctCellAddress(s_sCellAddress,xAttribs);
    }
    else if ( _sName == "chart:series" )
    {
        static OUString s_sCellAddress(lcl_createAttribute(XML_NP_CHART,XML_VALUES_CELL_RANGE_ADDRESS));
        lcl_correctCellAddress(s_sCellAddress,xAttribs);
    }
    else if ( m_bTableRowsStarted && !m_bFirstRowExported && _sName == "table:table-cell" )
    {
        SvXMLAttributeList* pList = comphelper::getUnoTunnelImplementation<SvXMLAttributeList>(xAttribs);
        static OUString s_sValue(lcl_createAttribute(XML_NP_OFFICE,XML_VALUE));
        pList->RemoveAttribute(s_sValue);
    }
    else if ( m_bTableRowsStarted && _sName == "text:p" )
    {
        bExport = false;
    }
    if ( bExport )
        m_xDelegatee->startElement(_sName,xAttribs);
}

void SAL_CALL ExportDocumentHandler::endElement(const OUString & _sName)
{
    bool bExport = true;
    OUString sNewName = _sName;
    if ( _sName == "office:chart" )
    {
        sNewName = lcl_createAttribute(XML_NP_OFFICE,XML_REPORT);
    }
    else if ( _sName == "table:table" )
    {
        m_xDelegatee->endElement(_sName);
        lcl_exportPrettyPrinting(m_xDelegatee);
        sNewName = lcl_createAttribute(XML_NP_RPT,XML_DETAIL);
    }
    else if ( _sName == "table:table-header-rows" )
    {
        m_bCountColumnHeader = false;
    }
    else if ( _sName == "table:table-rows" )
        m_bTableRowsStarted = false;
    else if ( m_bTableRowsStarted && m_bFirstRowExported && (_sName == "table:table-row" || _sName == "table:table-cell") )
        bExport = false;
    else if ( m_bTableRowsStarted && _sName == "table:table-row" )
        m_bFirstRowExported = true;
    else if ( m_bTableRowsStarted && _sName == "text:p" )
    {
        bExport = !m_bFirstRowExported;
    }

    if ( bExport )
        m_xDelegatee->endElement(sNewName);
}

void SAL_CALL ExportDocumentHandler::characters(const OUString & aChars)
{
    if ( !(m_bTableRowsStarted || m_bFirstRowExported) )
    {
        m_xDelegatee->characters(aChars);
    }
}

void SAL_CALL ExportDocumentHandler::ignorableWhitespace(const OUString & aWhitespaces)
{
    m_xDelegatee->ignorableWhitespace(aWhitespaces);
}

void SAL_CALL ExportDocumentHandler::processingInstruction(const OUString & aTarget, const OUString & aData)
{
    m_xDelegatee->processingInstruction(aTarget,aData);
}

void SAL_CALL ExportDocumentHandler::setDocumentLocator(const uno::Reference< xml::sax::XLocator > & xLocator)
{
    m_xDelegatee->setDocumentLocator(xLocator);
}
void SAL_CALL ExportDocumentHandler::initialize( const uno::Sequence< uno::Any >& _aArguments )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    comphelper::SequenceAsHashMap aArgs(_aArguments);
    m_xDelegatee = aArgs.getUnpackedValueOrDefault("DocumentHandler",m_xDelegatee);
    m_xModel = aArgs.getUnpackedValueOrDefault("Model",m_xModel);

    OSL_ENSURE(m_xDelegatee.is(),"No document handler available!");
    if ( !m_xDelegatee.is() || !m_xModel.is() )
        throw uno::Exception("no delegatee and no model", nullptr);

    m_xDatabaseDataProvider.set(m_xModel->getDataProvider(),uno::UNO_QUERY_THROW);
    if ( !m_xDatabaseDataProvider->getActiveConnection().is() )
        throw uno::Exception("no active connection", nullptr);

    uno::Reference< reflection::XProxyFactory > xProxyFactory = reflection::ProxyFactory::create( m_xContext );
    m_xProxy = xProxyFactory->createProxy(m_xDelegatee.get());
    ::comphelper::query_aggregation(m_xProxy,m_xDelegatee);
    m_xTypeProvider.set(m_xDelegatee,uno::UNO_QUERY);
    m_xServiceInfo.set(m_xDelegatee,uno::UNO_QUERY);

    // set ourself as delegator
    m_xProxy->setDelegator( *this );
    const OUString sCommand = m_xDatabaseDataProvider->getCommand();
    if ( !sCommand.isEmpty() )
        m_aColumns = ::dbtools::getFieldNamesByCommandDescriptor(m_xDatabaseDataProvider->getActiveConnection()
                    ,m_xDatabaseDataProvider->getCommandType()
                    ,sCommand);

    uno::Reference< chart::XComplexDescriptionAccess > xDataProvider(m_xDatabaseDataProvider,uno::UNO_QUERY);
    if ( !xDataProvider.is() )
        return;

    m_aColumns.realloc(1);
    const uno::Sequence< OUString > aColumnNames = xDataProvider->getColumnDescriptions();
    for(const auto& rColumnName : aColumnNames)
    {
        if ( !rColumnName.isEmpty() )
        {
            sal_Int32 nCount = m_aColumns.getLength();
            m_aColumns.realloc(nCount+1);
            m_aColumns[nCount] = rColumnName;
        }
    }
}

uno::Any SAL_CALL ExportDocumentHandler::queryInterface( const uno::Type& _rType )
{
    uno::Any aReturn = ExportDocumentHandler_BASE::queryInterface(_rType);
    return aReturn.hasValue() ? aReturn : (m_xProxy.is() ? m_xProxy->queryAggregation(_rType) : aReturn);
}

uno::Sequence< uno::Type > SAL_CALL ExportDocumentHandler::getTypes(  )
{
    if ( m_xTypeProvider.is() )
        return ::comphelper::concatSequences(
            ExportDocumentHandler_BASE::getTypes(),
            m_xTypeProvider->getTypes()
        );
    return ExportDocumentHandler_BASE::getTypes();
}

void ExportDocumentHandler::exportTableRows()
{
    const OUString sRow( lcl_createAttribute(XML_NP_TABLE, XML_TABLE_ROW) );
    m_xDelegatee->startElement(sRow,nullptr);

    const OUString sValueType( lcl_createAttribute(XML_NP_OFFICE, XML_VALUE_TYPE) );

    const OUString sCell( lcl_createAttribute(XML_NP_TABLE, XML_TABLE_CELL) );
    const OUString sP( lcl_createAttribute(XML_NP_TEXT, XML_P) );
    const OUString sFtext(lcl_createAttribute(XML_NP_RPT,XML_FORMATTED_TEXT) );
    const OUString sRElement(lcl_createAttribute(XML_NP_RPT,XML_REPORT_ELEMENT) );
    const OUString sRComponent( lcl_createAttribute(XML_NP_RPT,XML_REPORT_COMPONENT) ) ;
    const OUString sFormulaAttrib( lcl_createAttribute(XML_NP_RPT,XML_FORMULA) );
    static const char s_sFloat[] = "float";

    SvXMLAttributeList* pCellAtt = new SvXMLAttributeList();
    uno::Reference< xml::sax::XAttributeList > xCellAtt = pCellAtt;
    pCellAtt->AddAttribute(sValueType, "string");

    bool bRemoveString = true;
    const sal_Int32 nCount = m_aColumns.getLength();
    if ( m_nColumnCount > nCount )
    {
        const sal_Int32 nEmptyCellCount = m_nColumnCount - nCount;
        for(sal_Int32 i = 0; i < nEmptyCellCount ; ++i)
        {
            m_xDelegatee->startElement(sCell,xCellAtt);
            if ( bRemoveString )
            {
                bRemoveString = false;
                pCellAtt->RemoveAttribute(sValueType);
                pCellAtt->AddAttribute(sValueType,s_sFloat);
            }
            m_xDelegatee->startElement(sP,nullptr);
            m_xDelegatee->endElement(sP);
            m_xDelegatee->endElement(sCell);
        }
    }
    for(const auto& rColumn : std::as_const(m_aColumns))
    {
        OUString sFormula = "field:[" + rColumn + "]";
        SvXMLAttributeList* pList = new SvXMLAttributeList();
        uno::Reference< xml::sax::XAttributeList > xAttribs = pList;
        pList->AddAttribute(sFormulaAttrib,sFormula);

        m_xDelegatee->startElement(sCell,xCellAtt);
        if ( bRemoveString )
        {
            bRemoveString = false;
            pCellAtt->RemoveAttribute(sValueType);
            pCellAtt->AddAttribute(sValueType,s_sFloat);
        }
        m_xDelegatee->startElement(sP,nullptr);
        m_xDelegatee->startElement(sFtext,xAttribs);
        m_xDelegatee->startElement(sRElement,nullptr);
        m_xDelegatee->startElement(sRComponent,nullptr);

        m_xDelegatee->endElement(sRComponent);
        m_xDelegatee->endElement(sRElement);
        m_xDelegatee->endElement(sFtext);
        m_xDelegatee->endElement(sP);
        m_xDelegatee->endElement(sCell);
    }

    m_xDelegatee->endElement(sRow);
}

} // namespace rptxml


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
reportdesign_ExportDocumentHandler_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new rptxml::ExportDocumentHandler(context));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
