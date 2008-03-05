/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlExportDocumentHandler.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:02:45 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "xmlExportDocumentHandler.hxx"
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/chart2/data/XDatabaseDataProvider.hpp>
#include <com/sun/star/reflection/XProxyFactory.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <comphelper/sequence.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/documentconstants.hxx>
#include <xmloff/attrlist.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlement.hxx>
#include <xmloff/xmluconv.hxx>
#include <svtools/saveopt.hxx>
#include <rtl/ustrbuf.hxx>
#include <connectivity/dbtools.hxx>
#include <rtl/ustrbuf.hxx>

namespace rptxml
{
using namespace ::com::sun::star;
using namespace ::xmloff::token;

void lcl_exportPrettyPrinting(const uno::Reference< xml::sax::XDocumentHandler >& _xDelegatee)
{
    SvtSaveOptions aSaveOpt;
    if ( aSaveOpt.IsPrettyPrinting() )
    {
        static const ::rtl::OUString s_sWhitespaces(RTL_CONSTASCII_USTRINGPARAM(" "));
        _xDelegatee->ignorableWhitespace(s_sWhitespaces);
    }
}

::rtl::OUString lcl_createAttribute(const xmloff::token::XMLTokenEnum& _eNamespace,const xmloff::token::XMLTokenEnum& _eAttribute)
{
    ::rtl::OUStringBuffer sQName;
    // ...if it's in our map, make the prefix
    sQName.append ( xmloff::token::GetXMLToken(_eNamespace) );
    sQName.append ( sal_Unicode(':') );
    sQName.append ( xmloff::token::GetXMLToken(_eAttribute) );
    return sQName.makeStringAndClear();
}

ExportDocumentHandler::ExportDocumentHandler(uno::Reference< uno::XComponentContext > const & context) :
    m_xContext(context)
    ,m_nCurrentCellIndex(0)
    ,m_bTableRowsStarted(false)
    ,m_bFirstRowExported(false)
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
::rtl::OUString SAL_CALL ExportDocumentHandler::getImplementationName(  ) throw(uno::RuntimeException)
{
    return getImplementationName_Static();
}

//------------------------------------------------------------------------
sal_Bool SAL_CALL ExportDocumentHandler::supportsService( const ::rtl::OUString& ServiceName ) throw(uno::RuntimeException)
{
    return ::comphelper::existsValue(ServiceName,getSupportedServiceNames_static());
}

//------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL ExportDocumentHandler::getSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aSupported;
    if ( m_xServiceInfo.is() )
        aSupported = m_xServiceInfo->getSupportedServiceNames();
    return ::comphelper::concatSequences(getSupportedServiceNames_static(),aSupported);
}

//------------------------------------------------------------------------
::rtl::OUString ExportDocumentHandler::getImplementationName_Static(  ) throw(uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.report.ExportDocumentHandler"));
}

//------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > ExportDocumentHandler::getSupportedServiceNames_static(  ) throw(uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aSupported(1);
    aSupported[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.report.ExportDocumentHandler"));
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

void SAL_CALL ExportDocumentHandler::startElement(const ::rtl::OUString & _sName, const uno::Reference< xml::sax::XAttributeList > & xAttribs) throw (uno::RuntimeException, xml::sax::SAXException)
{

    bool bExport = true;
    if ( _sName.equalsAscii("office:chart") )
    {
        SvXMLAttributeList* pList = new SvXMLAttributeList();
        uno::Reference< xml::sax::XAttributeList > xNewAttribs = pList;
        ::rtl::OUStringBuffer sValue;
        static SvXMLEnumMapEntry aXML_CommnadTypeEnumMap[] =
        {
            { XML_TABLE, sdb::CommandType::TABLE },
            { XML_QUERY, sdb::CommandType::QUERY },
            // { XML_COMMAND, CommandType::COMMAND }, // default
            { XML_TOKEN_INVALID, 0 }
        };
        if ( SvXMLUnitConverter::convertEnum( sValue, static_cast<sal_uInt16>(m_xDatabaseDataProvider->getCommandType()),aXML_CommnadTypeEnumMap ) )
        {
            pList->AddAttribute(lcl_createAttribute(XML_NP_RPT,XML_COMMAND_TYPE),sValue.makeStringAndClear());
        }
        ::rtl::OUString sComamnd = m_xDatabaseDataProvider->getCommand();
        if ( sComamnd.getLength() )
            pList->AddAttribute(lcl_createAttribute(XML_NP_RPT,XML_COMMAND),sComamnd);

        ::rtl::OUString sFilter( m_xDatabaseDataProvider->getFilter() );
        if ( sFilter.getLength() )
            pList->AddAttribute(lcl_createAttribute(XML_NP_RPT,XML_FILTER),sFilter);

        sal_Bool bEscapeProcessing( m_xDatabaseDataProvider->getEscapeProcessing() );
        if ( !bEscapeProcessing )
            pList->AddAttribute(lcl_createAttribute(XML_NP_RPT,XML_ESCAPE_PROCESSING),::xmloff::token::GetXMLToken( XML_FALSE ));

        pList->AddAttribute(lcl_createAttribute(XML_NP_OFFICE,XML_MIMETYPE),MIMETYPE_OASIS_OPENDOCUMENT_CHART);

        m_xDelegatee->startElement(lcl_createAttribute(XML_NP_OFFICE,XML_REPORT),xNewAttribs);

        const uno::Sequence< ::rtl::OUString > aDetailFields = m_xDatabaseDataProvider->getDetailFields();
        if ( aDetailFields.getLength() )
        {
            lcl_exportPrettyPrinting(m_xDelegatee);
            m_xDelegatee->startElement(lcl_createAttribute(XML_NP_RPT,XML_MASTER_DETAIL_FIELDS),NULL);
            const uno::Sequence< ::rtl::OUString > aMasterFields = m_xDatabaseDataProvider->getMasterFields();
            OSL_ENSURE(aDetailFields.getLength() == aMasterFields.getLength(),"not equal length for master and detail fields!");
            const ::rtl::OUString sDetailToken = lcl_createAttribute(XML_NP_RPT, XML_DETAIL);
            const ::rtl::OUString sMasterToken = lcl_createAttribute(XML_NP_RPT, XML_MASTER);
            const ::rtl::OUString sElementToken = lcl_createAttribute(XML_NP_RPT,XML_MASTER_DETAIL_FIELD);
            const ::rtl::OUString* pDetailFieldsIter = aDetailFields.getConstArray();
            const ::rtl::OUString* pIter = aMasterFields.getConstArray();
            const ::rtl::OUString* pEnd   = pIter + aMasterFields.getLength();
            for(;pIter != pEnd;++pIter,++pDetailFieldsIter)
            {
                pList = new SvXMLAttributeList();
                xNewAttribs = pList;
                pList->AddAttribute( sMasterToken , *pIter );
                if ( pDetailFieldsIter->getLength() )
                    pList->AddAttribute( sDetailToken , *pDetailFieldsIter );
                lcl_exportPrettyPrinting(m_xDelegatee);
                m_xDelegatee->startElement(sElementToken,xNewAttribs);
                lcl_exportPrettyPrinting(m_xDelegatee);
                m_xDelegatee->endElement(sElementToken);
            }
            lcl_exportPrettyPrinting(m_xDelegatee);
            m_xDelegatee->endElement(lcl_createAttribute(XML_NP_RPT,XML_MASTER_DETAIL_FIELDS));
        }
        bExport = false;
    }
    else if ( _sName.equalsAscii("table:table") )
    {
        m_xDelegatee->startElement(lcl_createAttribute(XML_NP_RPT,XML_DETAIL),NULL);
        lcl_exportPrettyPrinting(m_xDelegatee);
    }
    else if ( _sName.equalsAscii("table:table-rows") )
        m_bTableRowsStarted = true;
    else if ( m_bTableRowsStarted && m_bFirstRowExported && (_sName.equalsAscii("table:table-row") || _sName.equalsAscii("table:table-cell")) )
        bExport = false;
    else if ( m_bTableRowsStarted && _sName.equalsAscii("text:p") )
    {
        if ( !m_bFirstRowExported )
        {
            m_xDelegatee->startElement(_sName,xAttribs);
            SvXMLAttributeList* pList = new SvXMLAttributeList();
            uno::Reference< xml::sax::XAttributeList > xNewAttribs = pList;

            ::rtl::OUString sFormula;
            if( m_nCurrentCellIndex < m_aColumns.getLength() )
                sFormula = m_aColumns[m_nCurrentCellIndex];
            pList->AddAttribute(lcl_createAttribute(XML_NP_RPT,XML_FORMULA),sFormula);

            m_xDelegatee->startElement(lcl_createAttribute(XML_NP_RPT,XML_FORMATTED_TEXT),xNewAttribs);
            m_xDelegatee->startElement(lcl_createAttribute(XML_NP_RPT,XML_REPORT_ELEMENT),NULL);
            m_xDelegatee->startElement(lcl_createAttribute(XML_NP_RPT,XML_REPORT_COMPONENT),NULL);

            ++m_nCurrentCellIndex;
        }
        bExport = false;
    }
    if ( bExport )
        m_xDelegatee->startElement(_sName,xAttribs);
}
// -----------------------------------------------------------------------------
void SAL_CALL ExportDocumentHandler::endElement(const ::rtl::OUString & _sName) throw (uno::RuntimeException, xml::sax::SAXException)
{
    bool bExport = true;
    ::rtl::OUString sNewName = _sName;
    if ( _sName.equalsAscii("office:chart") )
    {
        sNewName = lcl_createAttribute(XML_NP_OFFICE,XML_REPORT);
    }
    else if ( _sName.equalsAscii("table:table") )
    {
        m_xDelegatee->endElement(_sName);
        lcl_exportPrettyPrinting(m_xDelegatee);
        sNewName = lcl_createAttribute(XML_NP_RPT,XML_DETAIL);
    }
    else if ( _sName.equalsAscii("table:table-rows") )
        m_bTableRowsStarted = false;
    else if ( m_bTableRowsStarted && m_bFirstRowExported && (_sName.equalsAscii("table:table-row") || _sName.equalsAscii("table:table-cell")) )
        bExport = false;
    else if ( m_bTableRowsStarted && _sName.equalsAscii("table:table-row") )
        m_bFirstRowExported = true;
    else if ( m_bTableRowsStarted && _sName.equalsAscii("text:p") )
    {
        bExport = !m_bFirstRowExported;
        if ( bExport )
        {
            m_xDelegatee->endElement(lcl_createAttribute(XML_NP_RPT,XML_REPORT_COMPONENT));
            m_xDelegatee->endElement(lcl_createAttribute(XML_NP_RPT,XML_REPORT_ELEMENT));
            m_xDelegatee->endElement(lcl_createAttribute(XML_NP_RPT,XML_FORMATTED_TEXT));
        }
    }

    if ( bExport )
        m_xDelegatee->endElement(sNewName);
}

void SAL_CALL ExportDocumentHandler::characters(const ::rtl::OUString & aChars) throw (uno::RuntimeException, xml::sax::SAXException)
{
    if ( !(m_bTableRowsStarted && m_bFirstRowExported) )
        m_xDelegatee->characters(aChars);
}

void SAL_CALL ExportDocumentHandler::ignorableWhitespace(const ::rtl::OUString & aWhitespaces) throw (uno::RuntimeException, xml::sax::SAXException)
{
    m_xDelegatee->ignorableWhitespace(aWhitespaces);
}

void SAL_CALL ExportDocumentHandler::processingInstruction(const ::rtl::OUString & aTarget, const ::rtl::OUString & aData) throw (uno::RuntimeException, xml::sax::SAXException)
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
    m_xDelegatee = aArgs.getUnpackedValueOrDefault(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DocumentHandler")),m_xDelegatee);
    m_xModel = aArgs.getUnpackedValueOrDefault(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Model")),m_xModel);

    OSL_ENSURE(m_xDelegatee.is(),"No document handler avialable!");
    if ( !m_xDelegatee.is() || !m_xModel.is() )
        throw uno::Exception();

    m_xDatabaseDataProvider.set(m_xModel->getDataProvider(),uno::UNO_QUERY);
    if ( !m_xDatabaseDataProvider.is() )
        throw uno::Exception();

    uno::Reference< reflection::XProxyFactory > xProxyFactory( m_xContext->getServiceManager()->createInstanceWithContext(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.reflection.ProxyFactory")),m_xContext),
        uno::UNO_QUERY);
    m_xProxy = xProxyFactory->createProxy(m_xDelegatee.get());
    ::comphelper::query_aggregation(m_xProxy,m_xDelegatee);
    m_xTypeProvider.set(m_xDelegatee,uno::UNO_QUERY);
    m_xServiceInfo.set(m_xDelegatee,uno::UNO_QUERY);

    // set ourself as delegator
    m_xProxy->setDelegator( *this );

    const ::rtl::OUString sCommand = m_xDatabaseDataProvider->getCommand();
    if ( sCommand.getLength() )
        m_aColumns = ::dbtools::getFieldNamesByCommandDescriptor(m_xDatabaseDataProvider->getActiveConnection()
                    ,m_xDatabaseDataProvider->getCommandType()
                    ,sCommand);
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
} // namespace rptxml
// -----------------------------------------------------------------------------
