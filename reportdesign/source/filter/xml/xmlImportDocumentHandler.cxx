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

#include "xmlImportDocumentHandler.hxx"
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/chart2/data/DatabaseDataProvider.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart/XComplexDescriptionAccess.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/reflection/ProxyFactory.hpp>
#include <comphelper/sequence.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/documentconstants.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <xmloff/attrlist.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlement.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmlnmspe.hxx>

#include "xmlHelper.hxx"
#include "xmlEnums.hxx"

#include <connectivity/dbtools.hxx>

namespace rptxml
{
using namespace ::com::sun::star;
using namespace ::xmloff::token;

OUString lcl_createAttribute(const xmloff::token::XMLTokenEnum& _eNamespace,const xmloff::token::XMLTokenEnum& _eAttribute);

ImportDocumentHandler::ImportDocumentHandler(uno::Reference< uno::XComponentContext > const & context)
    :m_bImportedChart( false )
    ,m_xContext(context)
{
}
// -----------------------------------------------------------------------------
ImportDocumentHandler::~ImportDocumentHandler()
{
    if ( m_xProxy.is() )
    {
        m_xProxy->setDelegator( NULL );
        m_xProxy.clear();
    }
}
IMPLEMENT_GET_IMPLEMENTATION_ID(ImportDocumentHandler)
IMPLEMENT_FORWARD_REFCOUNT( ImportDocumentHandler, ImportDocumentHandler_BASE )
//------------------------------------------------------------------------
OUString SAL_CALL ImportDocumentHandler::getImplementationName(  ) throw(uno::RuntimeException)
{
    return getImplementationName_Static();
}

//------------------------------------------------------------------------
sal_Bool SAL_CALL ImportDocumentHandler::supportsService( const OUString& ServiceName ) throw(uno::RuntimeException)
{
    return ::comphelper::existsValue(ServiceName,getSupportedServiceNames_static());
}

//------------------------------------------------------------------------
uno::Sequence< OUString > SAL_CALL ImportDocumentHandler::getSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSupported;
    if ( m_xServiceInfo.is() )
        aSupported = m_xServiceInfo->getSupportedServiceNames();
    return ::comphelper::concatSequences(getSupportedServiceNames_static(),aSupported);
}

//------------------------------------------------------------------------
OUString ImportDocumentHandler::getImplementationName_Static(  ) throw(uno::RuntimeException)
{
    return OUString("com.sun.star.comp.report.ImportDocumentHandler");
}

//------------------------------------------------------------------------
uno::Sequence< OUString > ImportDocumentHandler::getSupportedServiceNames_static(  ) throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSupported(1);
    aSupported[0] = OUString("com.sun.star.report.ImportDocumentHandler");
    return aSupported;
}

//------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL ImportDocumentHandler::create( const uno::Reference< uno::XComponentContext >& _rxContext )
{
    return *(new ImportDocumentHandler( _rxContext ));
}
// xml::sax::XDocumentHandler:
void SAL_CALL ImportDocumentHandler::startDocument() throw (uno::RuntimeException, xml::sax::SAXException)
{
    m_xDelegatee->startDocument();
}

void SAL_CALL ImportDocumentHandler::endDocument() throw (uno::RuntimeException, xml::sax::SAXException)
{
    m_xDelegatee->endDocument();
    uno::Reference< chart2::data::XDataReceiver > xReceiver(m_xModel,uno::UNO_QUERY_THROW);
    if ( xReceiver.is() && m_bImportedChart )
    {
        // this fills the chart again
        ::comphelper::NamedValueCollection aArgs;
        aArgs.put( "CellRangeRepresentation", OUString("all") );
        aArgs.put( "FirstCellAsLabel", uno::makeAny( sal_True ) );
        aArgs.put( "DataRowSource", uno::makeAny( chart::ChartDataRowSource_COLUMNS ) );

        sal_Bool bHasCategories = sal_False;

        uno::Reference< chart2::data::XDataSource > xDataSource(m_xModel, uno::UNO_QUERY);
        if( xDataSource.is())
        {
            uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aSequences(xDataSource->getDataSequences());
            const sal_Int32 nCount( aSequences.getLength());
            for( sal_Int32 nIdx=0; nIdx<nCount; ++nIdx )
            {
                if( aSequences[nIdx].is() )
                {
                    uno::Reference< beans::XPropertySet > xSeqProp( aSequences[nIdx]->getValues(), uno::UNO_QUERY );
                    OUString aRole;
                    if  (   xSeqProp.is()
                        &&  ( xSeqProp->getPropertyValue( "Role" ) >>= aRole )
                        &&  aRole == "categories"
                        )
                    {
                        bHasCategories = sal_True;
                        break;
                    }
                }
            }
        }
        aArgs.put( "HasCategories", uno::makeAny( bHasCategories ) );

        uno::Reference< chart::XComplexDescriptionAccess > xDataProvider(m_xModel->getDataProvider(),uno::UNO_QUERY);
        if ( xDataProvider.is() )
        {
            const uno::Sequence< OUString > aColumnNames = xDataProvider->getColumnDescriptions();
            aArgs.put( "ColumnDescriptions", uno::makeAny( aColumnNames ) );
        }

        xReceiver->attachDataProvider( m_xDatabaseDataProvider.get() );
        xReceiver->setArguments( aArgs.getPropertyValues() );
    }
}

void SAL_CALL ImportDocumentHandler::startElement(const OUString & _sName, const uno::Reference< xml::sax::XAttributeList > & _xAttrList) throw (uno::RuntimeException, xml::sax::SAXException)
{
    uno::Reference< xml::sax::XAttributeList > xNewAttribs = _xAttrList;
    bool bExport = true;
    if ( _sName == "office:report" )
    {
        const sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
        static const OUString s_sTRUE = ::xmloff::token::GetXMLToken(XML_TRUE);
        try
        {
            for(sal_Int16 i = 0; i < nLength; ++i)
            {
                OUString sLocalName;
                const OUString sAttrName = _xAttrList->getNameByIndex( i );
                const sal_Int32 nColonPos = sAttrName.indexOf( sal_Unicode(':') );
                if( -1L == nColonPos )
                    sLocalName = sAttrName;
                else
                    sLocalName = sAttrName.copy( nColonPos + 1L );
                const OUString sValue = _xAttrList->getValueByIndex( i );

                switch( m_pReportElemTokenMap->Get( XML_NAMESPACE_REPORT, sLocalName ) )
                {
                    case XML_TOK_COMMAND_TYPE:
                        {
                            sal_uInt16 nRet = static_cast<sal_uInt16>(sdb::CommandType::COMMAND);
                            const SvXMLEnumMapEntry* aXML_EnumMap = OXMLHelper::GetCommandTypeOptions();
                            SvXMLUnitConverter::convertEnum( nRet, sValue, aXML_EnumMap );
                            m_xDatabaseDataProvider->setCommandType(nRet);
                        }
                        break;
                    case XML_TOK_COMMAND:
                        m_xDatabaseDataProvider->setCommand(sValue);
                        break;
                    case XML_TOK_FILTER:
                        m_xDatabaseDataProvider->setFilter(sValue);
                        break;
                    case XML_TOK_ESCAPE_PROCESSING:
                        m_xDatabaseDataProvider->setEscapeProcessing(sValue == s_sTRUE);
                        break;
                    default:
                        break;
                }
            }
        }
        catch(uno::Exception&)
        {
        }
        m_xDelegatee->startElement(lcl_createAttribute(XML_NP_OFFICE,XML_CHART),NULL);
        bExport = false;
        m_bImportedChart = true;
    }
    else if ( _sName == "rpt:master-detail-field" )
    {
        const sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<SvXMLTokenMap> pMasterElemTokenMap( OXMLHelper::GetSubDocumentElemTokenMap());
        SAL_WNODEPRECATED_DECLARATIONS_POP
        try
        {
            OUString sMasterField,sDetailField;
            for(sal_Int16 i = 0; i < nLength; ++i)
            {
                OUString sLocalName;
                const OUString sAttrName = _xAttrList->getNameByIndex( i );
                const sal_Int32 nColonPos = sAttrName.indexOf( sal_Unicode(':') );
                if( -1L == nColonPos )
                    sLocalName = sAttrName;
                else
                    sLocalName = sAttrName.copy( nColonPos + 1L );
                const OUString sValue = _xAttrList->getValueByIndex( i );

                switch( pMasterElemTokenMap->Get( XML_NAMESPACE_REPORT, sLocalName ) )
                {
                    case XML_TOK_MASTER:
                        sMasterField = sValue;
                        break;
                    case XML_TOK_SUB_DETAIL:
                        sDetailField = sValue;
                        break;
                }
            }
            if ( sDetailField.isEmpty() )
                sDetailField = sMasterField;
            m_aMasterFields.push_back(sMasterField);
            m_aDetailFields.push_back(sDetailField);
        }
        catch(uno::Exception&)
        {
            OSL_FAIL("Exception catched while filling the report definition props");
        }
        bExport = false;
    }
    else if ( _sName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("rpt:detail"))
        ||    _sName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("rpt:formatted-text"))
        ||    _sName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("rpt:master-detail-fields"))
        ||    _sName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("rpt:report-component"))
        ||    _sName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("rpt:report-element")))
        bExport = false;
    else if ( _sName == "chart:plot-area" )
    {
        sal_Bool bHasCategories = sal_True;
        const sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<SvXMLTokenMap> pMasterElemTokenMap( OXMLHelper::GetSubDocumentElemTokenMap());
        SAL_WNODEPRECATED_DECLARATIONS_POP
        for(sal_Int16 i = 0; i < nLength; ++i)
        {
            OUString sLocalName;
            const OUString sAttrName = _xAttrList->getNameByIndex( i );
            const sal_Int32 nColonPos = sAttrName.indexOf( sal_Unicode(':') );
            if( -1L == nColonPos )
                sLocalName = sAttrName;
            else
                sLocalName = sAttrName.copy( nColonPos + 1L );
            if ( sLocalName == "data-source-has-labels" )
            {
                const OUString sValue = _xAttrList->getValueByIndex( i );
                bHasCategories = sValue.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("both"));
                break;
            }
        }
        beans::PropertyValue* pArgIter = m_aArguments.getArray();
        beans::PropertyValue* pArgEnd  = pArgIter + m_aArguments.getLength();
        for(;pArgIter != pArgEnd;++pArgIter)
        {
            if ( pArgIter->Name == "HasCategories" )
            {
                pArgIter->Value <<= bHasCategories;
                break;
            }
        }

        SvXMLAttributeList* pList = new SvXMLAttributeList();
        xNewAttribs = pList;
        pList->AppendAttributeList(_xAttrList);
        pList->AddAttribute(OUString("table:cell-range-address"),OUString("local-table.$A$1:.$Z$65536"));

    }

    if ( bExport )
        m_xDelegatee->startElement(_sName,xNewAttribs);
}

void SAL_CALL ImportDocumentHandler::endElement(const OUString & _sName) throw (uno::RuntimeException, xml::sax::SAXException)
{
    bool bExport = true;
    OUString sNewName = _sName;
    if ( _sName == "office:report" )
    {
        sNewName = lcl_createAttribute(XML_NP_OFFICE,XML_CHART);
    }
    else if ( _sName == "rpt:master-detail-fields" )
    {
        if ( !m_aMasterFields.empty() )
            m_xDatabaseDataProvider->setMasterFields(uno::Sequence< OUString>(&*m_aMasterFields.begin(),m_aMasterFields.size()));
        if ( !m_aDetailFields.empty() )
            m_xDatabaseDataProvider->setDetailFields(uno::Sequence< OUString>(&*m_aDetailFields.begin(),m_aDetailFields.size()));
        bExport = false;
    }
    else if ( _sName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("rpt:detail"))
        ||    _sName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("rpt:formatted-text"))
        ||    _sName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("rpt:master-detail-field"))
        ||    _sName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("rpt:report-component"))
        ||    _sName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("rpt:report-element")))
        bExport = false;

    if ( bExport )
        m_xDelegatee->endElement(sNewName);
}

void SAL_CALL ImportDocumentHandler::characters(const OUString & aChars) throw (uno::RuntimeException, xml::sax::SAXException)
{
    m_xDelegatee->characters(aChars);
}

void SAL_CALL ImportDocumentHandler::ignorableWhitespace(const OUString & aWhitespaces) throw (uno::RuntimeException, xml::sax::SAXException)
{
    m_xDelegatee->ignorableWhitespace(aWhitespaces);
}

void SAL_CALL ImportDocumentHandler::processingInstruction(const OUString & aTarget, const OUString & aData) throw (uno::RuntimeException, xml::sax::SAXException)
{
    m_xDelegatee->processingInstruction(aTarget,aData);
}

void SAL_CALL ImportDocumentHandler::setDocumentLocator(const uno::Reference< xml::sax::XLocator > & xLocator) throw (uno::RuntimeException, xml::sax::SAXException)
{
    m_xDelegatee->setDocumentLocator(xLocator);
}
void SAL_CALL ImportDocumentHandler::initialize( const uno::Sequence< uno::Any >& _aArguments ) throw (uno::Exception, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    comphelper::SequenceAsHashMap aArgs(_aArguments);
    m_xDelegatee = aArgs.getUnpackedValueOrDefault("DocumentHandler",m_xDelegatee);
    m_xModel = aArgs.getUnpackedValueOrDefault("Model",m_xModel);

    OSL_ENSURE(m_xDelegatee.is(),"No document handler avialable!");
    if ( !m_xDelegatee.is() || !m_xModel.is() )
        throw uno::Exception();

    m_xDatabaseDataProvider.set(m_xModel->getDataProvider(),uno::UNO_QUERY);
    if ( !m_xDatabaseDataProvider.is() )
    {
        const static OUString s_sDatabaseDataProvider("com.sun.star.chart2.data.DatabaseDataProvider");
        m_xDatabaseDataProvider.set(m_xContext->getServiceManager()->createInstanceWithContext(s_sDatabaseDataProvider
            ,m_xContext),uno::UNO_QUERY);
        if ( !m_xDatabaseDataProvider.is() )
            throw uno::Exception();

        m_xDatabaseDataProvider->setRowLimit(10);

        uno::Reference< chart2::data::XDataReceiver > xReceiver(m_xModel,uno::UNO_QUERY_THROW);
        xReceiver->attachDataProvider(m_xDatabaseDataProvider.get());
    }

    m_aArguments = m_xDatabaseDataProvider->detectArguments(NULL);

    uno::Reference< reflection::XProxyFactory > xProxyFactory = reflection::ProxyFactory::create( m_xContext );
    m_xProxy = xProxyFactory->createProxy(m_xDelegatee.get());
    ::comphelper::query_aggregation(m_xProxy,m_xDelegatee);
    m_xTypeProvider.set(m_xDelegatee,uno::UNO_QUERY);
    m_xServiceInfo.set(m_xDelegatee,uno::UNO_QUERY);

    // set ourself as delegator
    m_xProxy->setDelegator( *this );

    m_pReportElemTokenMap.reset(OXMLHelper::GetReportElemTokenMap());
}
// --------------------------------------------------------------------------------
uno::Any SAL_CALL ImportDocumentHandler::queryInterface( const uno::Type& _rType ) throw (uno::RuntimeException)
{
    uno::Any aReturn = ImportDocumentHandler_BASE::queryInterface(_rType);
    return aReturn.hasValue() ? aReturn : (m_xProxy.is() ? m_xProxy->queryAggregation(_rType) : aReturn);
}
// --------------------------------------------------------------------------------
uno::Sequence< uno::Type > SAL_CALL ImportDocumentHandler::getTypes(  ) throw (uno::RuntimeException)
{
    if ( m_xTypeProvider.is() )
        return ::comphelper::concatSequences(
            ImportDocumentHandler_BASE::getTypes(),
            m_xTypeProvider->getTypes()
        );
    return ImportDocumentHandler_BASE::getTypes();
}

// -----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
