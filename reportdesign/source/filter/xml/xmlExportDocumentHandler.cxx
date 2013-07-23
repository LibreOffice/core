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
#include <comphelper/sequence.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/documentconstants.hxx>
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

void lcl_exportPrettyPrinting(const uno::Reference< xml::sax::XDocumentHandler >& _xDelegatee)
{
    SvtSaveOptions aSaveOpt;
    if ( aSaveOpt.IsPrettyPrinting() )
    {
        static const OUString s_sWhitespaces(" ");
        _xDelegatee->ignorableWhitespace(s_sWhitespaces);
    }
}

OUString lcl_createAttribute(const xmloff::token::XMLTokenEnum& _eNamespace,const xmloff::token::XMLTokenEnum& _eAttribute)
{
    OUStringBuffer sQName;
    // ...if it's in our map, make the prefix
    sQName.append ( xmloff::token::GetXMLToken(_eNamespace) );
    sQName.append ( sal_Unicode(':') );
    sQName.append ( xmloff::token::GetXMLToken(_eAttribute) );
    return sQName.makeStringAndClear();
}

void lcl_correctCellAddress(const OUString & _sName, const uno::Reference< xml::sax::XAttributeList > & xAttribs)
{
    SvXMLAttributeList* pList = SvXMLAttributeList::getImplementation(xAttribs);
    OUString sCellAddress = pList->getValueByName(_sName);
    const sal_Int32 nPos = sCellAddress.lastIndexOf('$');
    if ( nPos != -1 )
    {
        sCellAddress = sCellAddress.copy(0,nPos);
        sCellAddress += OUString("$65535");
        pList->RemoveAttribute(_sName);
        pList->AddAttribute(_sName,sCellAddress);
    }
}

ExportDocumentHandler::ExportDocumentHandler(uno::Reference< uno::XComponentContext > const & context) :
    m_xContext(context)
    ,m_nCurrentCellIndex(0)
    ,m_nColumnCount(0)
    ,m_bTableRowsStarted(false)
    ,m_bFirstRowExported(false)
    ,m_bExportChar(false)
    ,m_bCountColumnHeader(false)
{
}
// -----------------------------------------------------------------------------
ExportDocumentHandler::~ExportDocumentHandler()
{
    if ( m_xProxy.is() )
    {
        m_xProxy->setDelegator( NULL );
        m_xProxy.clear();
    }
}
IMPLEMENT_GET_IMPLEMENTATION_ID(ExportDocumentHandler)
IMPLEMENT_FORWARD_REFCOUNT( ExportDocumentHandler, ExportDocumentHandler_BASE )
//------------------------------------------------------------------------
OUString SAL_CALL ExportDocumentHandler::getImplementationName(  ) throw(uno::RuntimeException)
{
    return getImplementationName_Static();
}

//------------------------------------------------------------------------
sal_Bool SAL_CALL ExportDocumentHandler::supportsService( const OUString& ServiceName ) throw(uno::RuntimeException)
{
    return ::comphelper::existsValue(ServiceName,getSupportedServiceNames_static());
}

//------------------------------------------------------------------------
uno::Sequence< OUString > SAL_CALL ExportDocumentHandler::getSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSupported;
    if ( m_xServiceInfo.is() )
        aSupported = m_xServiceInfo->getSupportedServiceNames();
    return ::comphelper::concatSequences(getSupportedServiceNames_static(),aSupported);
}

//------------------------------------------------------------------------
OUString ExportDocumentHandler::getImplementationName_Static(  ) throw(uno::RuntimeException)
{
    return OUString("com.sun.star.comp.report.ExportDocumentHandler");
}

//------------------------------------------------------------------------
uno::Sequence< OUString > ExportDocumentHandler::getSupportedServiceNames_static(  ) throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSupported(1);
    aSupported[0] = OUString("com.sun.star.report.ExportDocumentHandler");
    return aSupported;
}

//------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL ExportDocumentHandler::create( const uno::Reference< uno::XComponentContext >& _rxContext )
{
    return *(new ExportDocumentHandler( _rxContext ));
}
// xml::sax::XDocumentHandler:
void SAL_CALL ExportDocumentHandler::startDocument() throw (uno::RuntimeException, xml::sax::SAXException)
{
    m_xDelegatee->startDocument();
}

void SAL_CALL ExportDocumentHandler::endDocument() throw (uno::RuntimeException, xml::sax::SAXException)
{
    m_xDelegatee->endDocument();
}

void SAL_CALL ExportDocumentHandler::startElement(const OUString & _sName, const uno::Reference< xml::sax::XAttributeList > & xAttribs) throw (uno::RuntimeException, xml::sax::SAXException)
{
    bool bExport = true;
    if ( _sName == "office:chart" )
    {
        SvXMLAttributeList* pList = new SvXMLAttributeList();
        uno::Reference< xml::sax::XAttributeList > xNewAttribs = pList;
        OUStringBuffer sValue;
        static SvXMLEnumMapEntry aXML_CommnadTypeEnumMap[] =
        {
            { XML_TABLE, sdb::CommandType::TABLE },
            { XML_QUERY, sdb::CommandType::QUERY },
            { XML_TOKEN_INVALID, 0 }
        };
        if ( SvXMLUnitConverter::convertEnum( sValue, static_cast<sal_uInt16>(m_xDatabaseDataProvider->getCommandType()),aXML_CommnadTypeEnumMap ) )
        {
            pList->AddAttribute(lcl_createAttribute(XML_NP_RPT,XML_COMMAND_TYPE),sValue.makeStringAndClear());
        }
        const OUString sComamnd = m_xDatabaseDataProvider->getCommand();
        if ( !sComamnd.isEmpty() )
            pList->AddAttribute(lcl_createAttribute(XML_NP_RPT,XML_COMMAND),sComamnd);

        const OUString sFilter( m_xDatabaseDataProvider->getFilter() );
        if ( !sFilter.isEmpty() )
            pList->AddAttribute(lcl_createAttribute(XML_NP_RPT,XML_FILTER),sFilter);

        const sal_Bool bEscapeProcessing( m_xDatabaseDataProvider->getEscapeProcessing() );
        if ( !bEscapeProcessing )
            pList->AddAttribute(lcl_createAttribute(XML_NP_RPT,XML_ESCAPE_PROCESSING),::xmloff::token::GetXMLToken( XML_FALSE ));

        pList->AddAttribute(lcl_createAttribute(XML_NP_OFFICE,XML_MIMETYPE),MIMETYPE_OASIS_OPENDOCUMENT_CHART);

        m_xDelegatee->startElement(lcl_createAttribute(XML_NP_OFFICE,XML_REPORT),xNewAttribs);

        const OUString sTableCalc = lcl_createAttribute(XML_NP_TABLE,XML_CALCULATION_SETTINGS);
        m_xDelegatee->startElement(sTableCalc,NULL);
        pList = new SvXMLAttributeList();
        uno::Reference< xml::sax::XAttributeList > xNullAttr = pList;
        pList->AddAttribute(lcl_createAttribute(XML_NP_TABLE,XML_DATE_VALUE),OUString("1899-12-30"));

        const OUString sNullDate = lcl_createAttribute(XML_NP_TABLE,XML_NULL_DATE);
        m_xDelegatee->startElement(sNullDate,xNullAttr);
        m_xDelegatee->endElement(sNullDate);
        m_xDelegatee->endElement(sTableCalc);
        bExport = false;
    }
    else if ( _sName == "table:table" )
    {
        m_xDelegatee->startElement(lcl_createAttribute(XML_NP_RPT,XML_DETAIL),NULL);
        lcl_exportPrettyPrinting(m_xDelegatee);
    }
    else if ( _sName == "table:table-header-rows" )
    {
        m_bCountColumnHeader = true;
    }
    else if ( m_bCountColumnHeader && _sName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("table:table-cell")) )
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
    else if ( m_bTableRowsStarted && m_bFirstRowExported && (_sName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("table:table-row")) || _sName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("table:table-cell"))) )
        bExport = false;
    else if ( _sName == "chart:plot-area" )
    {
        SvXMLAttributeList* pList = SvXMLAttributeList::getImplementation(xAttribs);
        pList->RemoveAttribute(OUString("table:cell-range-address"));
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
    else if ( m_bTableRowsStarted && !m_bFirstRowExported && _sName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("table:table-cell")) )
    {
        SvXMLAttributeList* pList = SvXMLAttributeList::getImplementation(xAttribs);
        static OUString s_sValue(lcl_createAttribute(XML_NP_OFFICE,XML_VALUE));
        pList->RemoveAttribute(s_sValue);
    }
    else if ( m_bTableRowsStarted && _sName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("text:p")) )
    {
        bExport = false;
    }
    if ( bExport )
        m_xDelegatee->startElement(_sName,xAttribs);
}
// -----------------------------------------------------------------------------
void SAL_CALL ExportDocumentHandler::endElement(const OUString & _sName) throw (uno::RuntimeException, xml::sax::SAXException)
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
    else if ( m_bTableRowsStarted && m_bFirstRowExported && (_sName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("table:table-row")) || _sName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("table:table-cell"))) )
        bExport = false;
    else if ( m_bTableRowsStarted && _sName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("table:table-row")) )
        m_bFirstRowExported = true;
    else if ( m_bTableRowsStarted && _sName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("text:p")) )
    {
        bExport = !m_bFirstRowExported;
    }

    if ( bExport )
        m_xDelegatee->endElement(sNewName);
}

void SAL_CALL ExportDocumentHandler::characters(const OUString & aChars) throw (uno::RuntimeException, xml::sax::SAXException)
{
    if ( !(m_bTableRowsStarted || m_bFirstRowExported) )
    {
        m_xDelegatee->characters(aChars);
    }
    else if ( m_bExportChar )
    {
        static const OUString s_sZero("0");
        m_xDelegatee->characters(s_sZero);
    }
}

void SAL_CALL ExportDocumentHandler::ignorableWhitespace(const OUString & aWhitespaces) throw (uno::RuntimeException, xml::sax::SAXException)
{
    m_xDelegatee->ignorableWhitespace(aWhitespaces);
}

void SAL_CALL ExportDocumentHandler::processingInstruction(const OUString & aTarget, const OUString & aData) throw (uno::RuntimeException, xml::sax::SAXException)
{
    m_xDelegatee->processingInstruction(aTarget,aData);
}

void SAL_CALL ExportDocumentHandler::setDocumentLocator(const uno::Reference< xml::sax::XLocator > & xLocator) throw (uno::RuntimeException, xml::sax::SAXException)
{
    m_xDelegatee->setDocumentLocator(xLocator);
}
void SAL_CALL ExportDocumentHandler::initialize( const uno::Sequence< uno::Any >& _aArguments ) throw (uno::Exception, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    comphelper::SequenceAsHashMap aArgs(_aArguments);
    m_xDelegatee = aArgs.getUnpackedValueOrDefault("DocumentHandler",m_xDelegatee);
    m_xModel = aArgs.getUnpackedValueOrDefault("Model",m_xModel);

    OSL_ENSURE(m_xDelegatee.is(),"No document handler avialable!");
    if ( !m_xDelegatee.is() || !m_xModel.is() )
        throw uno::Exception();

    m_xDatabaseDataProvider.set(m_xModel->getDataProvider(),uno::UNO_QUERY);
    if ( !m_xDatabaseDataProvider.is() || !m_xDatabaseDataProvider->getActiveConnection().is() )
        throw uno::Exception();

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
    if ( xDataProvider.is() )
    {
        m_aColumns.realloc(1);
        uno::Sequence< OUString > aColumnNames = xDataProvider->getColumnDescriptions();
        for(sal_Int32 i = 0 ; i < aColumnNames.getLength();++i)
        {
            if ( !aColumnNames[i].isEmpty() )
            {
                sal_Int32 nCount = m_aColumns.getLength();
                m_aColumns.realloc(nCount+1);
                m_aColumns[nCount] = aColumnNames[i];
            }
        }
    }
}
// --------------------------------------------------------------------------------
uno::Any SAL_CALL ExportDocumentHandler::queryInterface( const uno::Type& _rType ) throw (uno::RuntimeException)
{
    uno::Any aReturn = ExportDocumentHandler_BASE::queryInterface(_rType);
    return aReturn.hasValue() ? aReturn : (m_xProxy.is() ? m_xProxy->queryAggregation(_rType) : aReturn);
}
// --------------------------------------------------------------------------------
uno::Sequence< uno::Type > SAL_CALL ExportDocumentHandler::getTypes(  ) throw (uno::RuntimeException)
{
    if ( m_xTypeProvider.is() )
        return ::comphelper::concatSequences(
            ExportDocumentHandler_BASE::getTypes(),
            m_xTypeProvider->getTypes()
        );
    return ExportDocumentHandler_BASE::getTypes();
}
// -----------------------------------------------------------------------------
void ExportDocumentHandler::exportTableRows()
{
    const OUString sRow( lcl_createAttribute(XML_NP_TABLE, XML_TABLE_ROW) );
    m_xDelegatee->startElement(sRow,NULL);

    const OUString sValueType( lcl_createAttribute(XML_NP_OFFICE, XML_VALUE_TYPE) );

    const static OUString s_sFieldPrefix("field:[");
    const static OUString s_sFieldPostfix("]");
    const OUString sCell( lcl_createAttribute(XML_NP_TABLE, XML_TABLE_CELL) );
    const OUString sP( lcl_createAttribute(XML_NP_TEXT, XML_P) );
    const OUString sFtext(lcl_createAttribute(XML_NP_RPT,XML_FORMATTED_TEXT) );
    const OUString sRElement(lcl_createAttribute(XML_NP_RPT,XML_REPORT_ELEMENT) );
    const OUString sRComponent( lcl_createAttribute(XML_NP_RPT,XML_REPORT_COMPONENT) ) ;
    const OUString sFormulaAttrib( lcl_createAttribute(XML_NP_RPT,XML_FORMULA) );
    const static OUString s_sString("string");
    const static OUString s_sFloat("float");

    SvXMLAttributeList* pCellAtt = new SvXMLAttributeList();
    uno::Reference< xml::sax::XAttributeList > xCellAtt = pCellAtt;
    pCellAtt->AddAttribute(sValueType,s_sString);

    bool bRemoveString = true;
    OUString sFormula;
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
            m_xDelegatee->startElement(sP,NULL);
            m_xDelegatee->endElement(sP);
            m_xDelegatee->endElement(sCell);
        }
    }
    for(sal_Int32 i = 0; i < nCount ; ++i)
    {
        sFormula = s_sFieldPrefix;
        sFormula += m_aColumns[i];
        sFormula += s_sFieldPostfix;
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
        m_xDelegatee->startElement(sP,NULL);
        m_xDelegatee->startElement(sFtext,xAttribs);
        m_xDelegatee->startElement(sRElement,NULL);
        m_xDelegatee->startElement(sRComponent,NULL);

        m_xDelegatee->endElement(sRComponent);
        m_xDelegatee->endElement(sRElement);
        m_xDelegatee->endElement(sFtext);
        m_xDelegatee->endElement(sP);
        m_xDelegatee->endElement(sCell);
    }

    m_xDelegatee->endElement(sRow);
}
// -----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
