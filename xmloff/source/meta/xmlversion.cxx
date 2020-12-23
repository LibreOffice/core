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

#include <com/sun/star/embed/ElementModes.hpp>
#include <xmlversion.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlmetae.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>

#include <xmloff/xmltoken.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <cppuhelper/supportsservice.hxx>

using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

const OUStringLiteral XMLN_VERSIONSLIST = u"VersionList.xml";

XMLVersionListExport::XMLVersionListExport(
    const css::uno::Reference< css::uno::XComponentContext >& rContext,
    const css::uno::Sequence < css::util::RevisionTag >& rVersions,
    const OUString &rFileName,
    Reference< XDocumentHandler > const &rHandler )
:   SvXMLExport( rContext, "", rFileName, util::MeasureUnit::CM, rHandler ),
    maVersions( rVersions )
{
    GetNamespaceMap_().AddAtIndex( xmloff::token::GetXMLToken(xmloff::token::XML_NP_DC),
                                   xmloff::token::GetXMLToken(xmloff::token::XML_N_DC), XML_NAMESPACE_DC );
    GetNamespaceMap_().AddAtIndex( xmloff::token::GetXMLToken(xmloff::token::XML_NP_VERSIONS_LIST),
                                   xmloff::token::GetXMLToken(xmloff::token::XML_N_VERSIONS_LIST), XML_NAMESPACE_FRAMEWORK );
}

ErrCode XMLVersionListExport::exportDoc( enum ::xmloff::token::XMLTokenEnum )
{
    GetDocHandler()->startDocument();

    addChaffWhenEncryptedStorage();

    sal_uInt16 nPos = SvXMLNamespaceMap::GetIndexByKey( XML_NAMESPACE_DC );

    AddAttribute( XML_NAMESPACE_NONE, GetNamespaceMap_().GetAttrNameByIndex( nPos ),
                             GetNamespaceMap_().GetNameByIndex ( nPos ) );

    nPos = SvXMLNamespaceMap::GetIndexByKey( XML_NAMESPACE_FRAMEWORK );
    AddAttribute( XML_NAMESPACE_NONE, GetNamespaceMap_().GetAttrNameByIndex( nPos ),
                             GetNamespaceMap_().GetNameByIndex ( nPos ) );

    {
        // the following object will write all collected attributes in its dtor
        SvXMLElementExport aRoot( *this, XML_NAMESPACE_FRAMEWORK, xmloff::token::XML_VERSION_LIST, true, true );

        for ( const util::RevisionTag& rInfo : maVersions )
        {
            AddAttribute( XML_NAMESPACE_FRAMEWORK,
                          xmloff::token::XML_TITLE,
                          rInfo.Identifier );
            AddAttribute( XML_NAMESPACE_FRAMEWORK,
                          xmloff::token::XML_COMMENT,
                          rInfo.Comment );
            AddAttribute( XML_NAMESPACE_FRAMEWORK,
                          xmloff::token::XML_CREATOR,
                          rInfo.Author );

            OUString aDateStr =
                SvXMLMetaExport::GetISODateTimeString( rInfo.TimeStamp );

            AddAttribute( XML_NAMESPACE_DC, xmloff::token::XML_DATE_TIME, aDateStr );

            // the following object will write all collected attributes in its dtor
            SvXMLElementExport aEntry( *this, XML_NAMESPACE_FRAMEWORK, xmloff::token::XML_VERSION_ENTRY, true, true );
        }
    }
    GetDocHandler()->endDocument();
    return ERRCODE_NONE;
}

XMLVersionListImport::XMLVersionListImport(
    const css::uno::Reference< css::uno::XComponentContext >& rContext,
    css::uno::Sequence < css::util::RevisionTag >& rVersions )
:   SvXMLImport(rContext, ""),
    maVersions( rVersions )
{
}

XMLVersionListImport::~XMLVersionListImport() throw()
{}

SvXMLImportContext *XMLVersionListImport::CreateFastContext( sal_Int32 nElement,
        const ::css::uno::Reference< ::css::xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
    SvXMLImportContext *pContext = nullptr;

    if ( nElement == XML_ELEMENT(VERSIONS_LIST, xmloff::token::XML_VERSION_LIST) )
    {
        pContext = new XMLVersionListContext( *this );
    }

    return pContext;
}

XMLVersionListContext::XMLVersionListContext( XMLVersionListImport& rImport)
    : SvXMLImportContext( rImport )
{
}

XMLVersionListContext::~XMLVersionListContext()
{}

css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL
XMLVersionListContext::createFastChildContext(sal_Int32 nElement,
            const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList)
{
    SvXMLImportContext *pContext = nullptr;

    if ( nElement == XML_ELEMENT(FRAMEWORK, xmloff::token::XML_VERSION_ENTRY)
        || nElement == XML_ELEMENT(VERSIONS_LIST, xmloff::token::XML_VERSION_ENTRY) )
    {
        pContext = new XMLVersionContext( GetImport(), xAttrList );
    }

    return pContext;
}

XMLVersionContext::XMLVersionContext( XMLVersionListImport& rImport,
                                        const Reference< XFastAttributeList > & xAttrList )
    : SvXMLImportContext( rImport )
{
    sax_fastparser::FastAttributeList& rAttribList =
        sax_fastparser::castToFastAttributeList( xAttrList );
    if ( rAttribList.getFastAttributeTokens().empty() )
        return;
    util::RevisionTag aInfo;
    for (auto &aIter : rAttribList)
    {
        switch( aIter.getToken() )
        {
        case XML_ELEMENT(FRAMEWORK, xmloff::token::XML_TITLE):
        case XML_ELEMENT(VERSIONS_LIST, xmloff::token::XML_TITLE):
        {
            aInfo.Identifier = aIter.toString();
            break;
        }
        case XML_ELEMENT(FRAMEWORK, xmloff::token::XML_COMMENT):
        case XML_ELEMENT(VERSIONS_LIST, xmloff::token::XML_COMMENT):
        {
            aInfo.Comment = aIter.toString();
            break;
        }
        case XML_ELEMENT(FRAMEWORK, xmloff::token::XML_CREATOR):
        case XML_ELEMENT(VERSIONS_LIST, xmloff::token::XML_CREATOR):
        {
            aInfo.Author = aIter.toString();
            break;
        }
        case XML_ELEMENT(DC, xmloff::token::XML_DATE_TIME):
        {
            util::DateTime aTime;
            if ( ParseISODateTimeString( aIter.toString(), aTime ) )
                aInfo.TimeStamp = aTime;
            break;
        }
        default:
            XMLOFF_WARN_UNKNOWN("xmloff", aIter);
            break;
        }
    }

    uno::Sequence < util::RevisionTag >& aList = rImport.GetList();
    sal_Int32 nLength = aList.getLength();
    aList.realloc( nLength+1 );
    aList[nLength] = aInfo;
}

XMLVersionContext::~XMLVersionContext()
{}

bool XMLVersionContext::ParseISODateTimeString(
                                const OUString& rString,
                                util::DateTime& rDateTime )
{
    bool bSuccess = true;

    OUString aDateStr, aTimeStr;
    sal_Int32 nPos = rString.indexOf( 'T' );
    if ( nPos >= 0 )
    {
        aDateStr = rString.copy( 0, nPos );
        aTimeStr = rString.copy( nPos + 1 );
    }
    else
        aDateStr = rString;         // no separator: only date part

    sal_Int32 nYear  = 0;
    sal_Int32 nMonth = 1;
    sal_Int32 nDay   = 1;
    sal_Int32 nHour  = 0;
    sal_Int32 nMin   = 0;
    sal_Int32 nSec   = 0;

    const sal_Unicode* pStr = aDateStr.getStr();
    sal_Int32 nDateTokens = 1;
    while ( *pStr )
    {
        if ( *pStr == '-' )
            nDateTokens++;
        pStr++;
    }
    if ( nDateTokens > 3 || aDateStr.isEmpty() )
        bSuccess = false;
    else
    {
        sal_Int32 n = 0;
        nYear = aDateStr.getToken( 0, '-', n ).toInt32();
        if ( nYear > 9999 )
            bSuccess = false;
        else if ( nDateTokens >= 2 )
        {
            nMonth = aDateStr.getToken( 0, '-', n ).toInt32();
            if ( nMonth > 12 )
                bSuccess = false;
            else if ( nDateTokens >= 3 )
            {
                nDay = aDateStr.getToken( 0, '-', n ).toInt32();
                if ( nDay > 31 )
                    bSuccess = false;
            }
        }
    }

    if ( bSuccess && !aTimeStr.isEmpty() )         // time is optional
    {
        pStr = aTimeStr.getStr();
        sal_Int32 nTimeTokens = 1;
        while ( *pStr )
        {
            if ( *pStr == ':' )
                nTimeTokens++;
            pStr++;
        }
        if ( nTimeTokens > 3 )
            bSuccess = false;
        else
        {
            sal_Int32 n = 0;
            nHour = aTimeStr.getToken( 0, ':', n ).toInt32();
            if ( nHour > 23 )
                bSuccess = false;
            else if ( nTimeTokens >= 2 )
            {
                nMin = aTimeStr.getToken( 0, ':', n ).toInt32();
                if ( nMin > 59 )
                    bSuccess = false;
                else if ( nTimeTokens >= 3 )
                {
                    nSec = aTimeStr.getToken( 0, ':', n ).toInt32();
                    if ( nSec > 59 )
                        bSuccess = false;
                }
            }
        }
    }

    if ( bSuccess )
    {
        rDateTime.Day = sal::static_int_cast< sal_uInt16 >(nDay);
        rDateTime.Month = sal::static_int_cast< sal_uInt16 >(nMonth);
        rDateTime.Year = sal::static_int_cast< sal_uInt16 >(nYear);
        rDateTime.Hours = sal::static_int_cast< sal_uInt16 >(nHour);
        rDateTime.Minutes = sal::static_int_cast< sal_uInt16 >(nMin);
        rDateTime.Seconds = sal::static_int_cast< sal_uInt16 >(nSec);
    }

    return bSuccess;
}

void SAL_CALL XMLVersionListPersistence::store( const uno::Reference< embed::XStorage >& xRoot, const uno::Sequence< util::RevisionTag >& rVersions )
{
    // no storage, no version list!
    if ( !xRoot.is() )
        return;

    // get the services needed for writing the xml data
    Reference< uno::XComponentContext > xContext =
            comphelper::getProcessComponentContext();

    Reference< XWriter > xWriter = Writer::create(xContext);

    // check whether there's already a sub storage with the version info
    // and delete it
    OUString sVerName( XMLN_VERSIONSLIST  );

    try {
        // open (create) the sub storage with the version info
        uno::Reference< io::XStream > xVerStream = xRoot->openStreamElement(
                                        sVerName,
                                        embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE );
        if ( !xVerStream.is() )
            throw uno::RuntimeException();

        Reference< io::XOutputStream > xOut = xVerStream->getOutputStream();
        if ( !xOut.is() )
            throw uno::RuntimeException(); // the stream was successfully opened for writing already

        xWriter->setOutputStream(xOut);

        rtl::Reference< XMLVersionListExport > xExp( new XMLVersionListExport( xContext, rVersions, sVerName, xWriter ) );

        xExp->exportDoc( ::xmloff::token::XML_VERSION );

        xVerStream.clear(); // use refcounting for now to dispose
    }
    catch( uno::Exception& )
    {
        // TODO: error handling
    }
}

uno::Sequence< util::RevisionTag > SAL_CALL XMLVersionListPersistence::load( const uno::Reference< embed::XStorage >& xRoot )
{
    css::uno::Sequence < css::util::RevisionTag > aVersions;

    const OUString sDocName( XMLN_VERSIONSLIST  );

    try {
        if ( xRoot.is() && xRoot->hasByName( sDocName ) && xRoot->isStreamElement( sDocName ) )
        {
            Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();

            InputSource aParserInput;

            uno::Reference< beans::XPropertySet > xProps( xRoot, uno::UNO_QUERY );
            OSL_ENSURE( xProps.is(), "Storage must implement XPropertySet!" );
            if ( xProps.is() )
            {
                try {
                    xProps->getPropertyValue("URL") >>= aParserInput.sSystemId;
                }
                catch( uno::Exception& )
                {}
            }

            uno::Reference< io::XStream > xDocStream = xRoot->openStreamElement(
                                                            sDocName,
                                                            embed::ElementModes::READ );
            if ( !xDocStream.is() )
                throw uno::RuntimeException();

            aParserInput.aInputStream = xDocStream->getInputStream();
            OSL_ENSURE( aParserInput.aInputStream.is(),
                        "The stream was successfully opened for reading, the input part must be accessible!" );
            if ( !aParserInput.aInputStream.is() )
                throw uno::RuntimeException();

            // get filter
            rtl::Reference< XMLVersionListImport > xImport = new XMLVersionListImport( xContext, aVersions );

            // parse
            try
            {
                xImport->parseStream( aParserInput );
            }
            catch( SAXParseException&  ) {}
            catch( SAXException&  )      {}
            catch( io::IOException& )    {}
        }
    }
    catch( uno::Exception& )
    {
        // TODO: error handling
    }

    return aVersions;
}

OUString XMLVersionListPersistence::getImplementationName()
{
    return "XMLVersionListPersistence";
}

sal_Bool XMLVersionListPersistence::supportsService(
    OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString>
XMLVersionListPersistence::getSupportedServiceNames()
{
    return css::uno::Sequence<OUString>{
        "com.sun.star.document.DocumentRevisionListPersistence"};
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
XMLVersionListPersistence_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new XMLVersionListPersistence);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
