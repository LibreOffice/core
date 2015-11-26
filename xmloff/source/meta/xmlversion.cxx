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
#include <unotools/streamwrap.hxx>
#include <xmlversion.hxx>
#include <xmloff/xmlmetae.hxx>

#include <xmloff/xmltoken.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <cppuhelper/supportsservice.hxx>

using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

const char XMLN_VERSIONSLIST[] = "VersionList.xml";

XMLVersionListExport::XMLVersionListExport(
    const css::uno::Reference< css::uno::XComponentContext >& rContext,
    const css::uno::Sequence < css::util::RevisionTag >& rVersions,
    const OUString &rFileName,
    Reference< XDocumentHandler > &rHandler )
:   SvXMLExport( rContext, "", rFileName, util::MeasureUnit::CM, rHandler ),
    maVersions( rVersions )
{
    _GetNamespaceMap().AddAtIndex( XML_NAMESPACE_DC_IDX, xmloff::token::GetXMLToken(xmloff::token::XML_NP_DC),
                                   xmloff::token::GetXMLToken(xmloff::token::XML_N_DC), XML_NAMESPACE_DC );
    _GetNamespaceMap().AddAtIndex( XML_NAMESPACE_FRAMEWORK_IDX, xmloff::token::GetXMLToken(xmloff::token::XML_NP_VERSIONS_LIST),
                                   xmloff::token::GetXMLToken(xmloff::token::XML_N_VERSIONS_LIST), XML_NAMESPACE_FRAMEWORK );
}

sal_uInt32 XMLVersionListExport::exportDoc( enum ::xmloff::token::XMLTokenEnum )
{
    GetDocHandler()->startDocument();

    addChaffWhenEncryptedStorage();

    sal_uInt16 nPos = SvXMLNamespaceMap::GetIndexByKey( XML_NAMESPACE_DC );

    AddAttribute( XML_NAMESPACE_NONE, _GetNamespaceMap().GetAttrNameByIndex( nPos ),
                             _GetNamespaceMap().GetNameByIndex ( nPos ) );

    nPos = SvXMLNamespaceMap::GetIndexByKey( XML_NAMESPACE_FRAMEWORK );
    AddAttribute( XML_NAMESPACE_NONE, _GetNamespaceMap().GetAttrNameByIndex( nPos ),
                             _GetNamespaceMap().GetNameByIndex ( nPos ) );

    {
        // the following object will write all collected attributes in its dtor
        SvXMLElementExport aRoot( *this, XML_NAMESPACE_FRAMEWORK, xmloff::token::XML_VERSION_LIST, true, true );

        for ( sal_Int32 n=0; n<maVersions.getLength(); n++ )
        {
            const util::RevisionTag& rInfo = maVersions[n];
            AddAttribute( XML_NAMESPACE_FRAMEWORK,
                          xmloff::token::XML_TITLE,
                          OUString( rInfo.Identifier ) );
            AddAttribute( XML_NAMESPACE_FRAMEWORK,
                          xmloff::token::XML_COMMENT,
                          OUString( rInfo.Comment ) );
            AddAttribute( XML_NAMESPACE_FRAMEWORK,
                          xmloff::token::XML_CREATOR,
                          OUString( rInfo.Author ) );

            OUString aDateStr =
                SvXMLMetaExport::GetISODateTimeString( rInfo.TimeStamp );

            AddAttribute( XML_NAMESPACE_DC, xmloff::token::XML_DATE_TIME, aDateStr );

            // the following object will write all collected attributes in its dtor
            SvXMLElementExport aEntry( *this, XML_NAMESPACE_FRAMEWORK, xmloff::token::XML_VERSION_ENTRY, true, true );
        }
    }
    GetDocHandler()->endDocument();
    return 0;
}

XMLVersionListImport::XMLVersionListImport(
    const css::uno::Reference< css::uno::XComponentContext >& rContext,
    css::uno::Sequence < css::util::RevisionTag >& rVersions )
:   SvXMLImport(rContext, ""),
    maVersions( rVersions )
{
    GetNamespaceMap().AddAtIndex( XML_NAMESPACE_FRAMEWORK_IDX, xmloff::token::GetXMLToken(xmloff::token::XML_NP_VERSIONS_LIST),
                                  xmloff::token::GetXMLToken(xmloff::token::XML_N_VERSIONS_LIST), XML_NAMESPACE_FRAMEWORK );
}

XMLVersionListImport::~XMLVersionListImport() throw()
{}

SvXMLImportContext *XMLVersionListImport::CreateContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

    if ( XML_NAMESPACE_FRAMEWORK == nPrefix &&
        rLocalName == xmloff::token::GetXMLToken(xmloff::token::XML_VERSION_LIST) )
    {
        pContext = new XMLVersionListContext( *this, nPrefix, rLocalName, xAttrList );
    }
    else
    {
        pContext = SvXMLImport::CreateContext( nPrefix, rLocalName, xAttrList );
    }

    return pContext;
}

XMLVersionListContext::XMLVersionListContext( XMLVersionListImport& rImport,
                                        sal_uInt16 nPrefix,
                                        const OUString& rLocalName,
                                        const Reference< XAttributeList > & )
    : SvXMLImportContext( rImport, nPrefix, rLocalName )
    , rLocalRef( rImport )
{
}

XMLVersionListContext::~XMLVersionListContext()
{}

SvXMLImportContext *XMLVersionListContext::CreateChildContext( sal_uInt16 nPrefix,
                                        const OUString& rLocalName,
                                        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

    if ( nPrefix == XML_NAMESPACE_FRAMEWORK &&
         rLocalName == xmloff::token::GetXMLToken(xmloff::token::XML_VERSION_ENTRY) )
    {
        pContext = new XMLVersionContext( rLocalRef, nPrefix, rLocalName, xAttrList );
    }
    else
    {
        pContext = new SvXMLImportContext( rLocalRef, nPrefix, rLocalName );
    }

    return pContext;
}

XMLVersionContext::XMLVersionContext( XMLVersionListImport& rImport,
                                        sal_uInt16 nPref,
                                        const OUString& rLocalName,
                                        const Reference< XAttributeList > & xAttrList )
    : SvXMLImportContext( rImport, nPref, rLocalName )
    , rLocalRef( rImport )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;

    if ( !nAttrCount )
        return;

    util::RevisionTag aInfo;
    for ( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        OUString        aLocalName;
        const OUString& rAttrName   = xAttrList->getNameByIndex( i );
        sal_uInt16      nPrefix     = rImport.GetNamespaceMap().GetKeyByAttrName( rAttrName, &aLocalName );

        if ( XML_NAMESPACE_FRAMEWORK == nPrefix )
        {
            if ( aLocalName == xmloff::token::GetXMLToken(xmloff::token::XML_TITLE) )
            {
                const OUString& rAttrValue = xAttrList->getValueByIndex( i );
                aInfo.Identifier = rAttrValue;
            }
            else if ( aLocalName == xmloff::token::GetXMLToken(xmloff::token::XML_COMMENT) )
            {
                const OUString& rAttrValue = xAttrList->getValueByIndex( i );
                aInfo.Comment = rAttrValue;
            }
            else if ( aLocalName == xmloff::token::GetXMLToken(xmloff::token::XML_CREATOR) )
            {
                const OUString& rAttrValue = xAttrList->getValueByIndex( i );
                aInfo.Author = rAttrValue;
            }
        }
        else if ( ( XML_NAMESPACE_DC == nPrefix ) &&
                  ( aLocalName == xmloff::token::GetXMLToken(xmloff::token::XML_DATE_TIME) ) )
        {
            const OUString& rAttrValue = xAttrList->getValueByIndex( i );
            util::DateTime aTime;
            if ( ParseISODateTimeString( rAttrValue, aTime ) )
                aInfo.TimeStamp = aTime;
        }
    }

    uno::Sequence < util::RevisionTag >& aList = rLocalRef.GetList();
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
    sal_Int32 nPos = rString.indexOf( (sal_Unicode) 'T' );
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
    throw (css::io::IOException, css::uno::Exception, css::uno::RuntimeException, std::exception)
{
    // no storage, no version list!
    if ( xRoot.is() )
    {
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

            Reference< io::XActiveDataSource > xSrc( xWriter, uno::UNO_QUERY );
            xSrc->setOutputStream(xOut);

            Reference< XDocumentHandler > xHandler( xWriter, uno::UNO_QUERY );

            Reference< XMLVersionListExport > xExp( new XMLVersionListExport( xContext, rVersions, sVerName, xHandler ) );

            xExp->exportDoc( ::xmloff::token::XML_VERSION );

            xVerStream.clear(); // use refcounting for now to dispose
        }
        catch( uno::Exception& )
        {
            // TODO: error handling
        }
    }
}

uno::Sequence< util::RevisionTag > SAL_CALL XMLVersionListPersistence::load( const uno::Reference< embed::XStorage >& xRoot )
        throw (css::container::NoSuchElementException, css::io::IOException, css::uno::Exception, css::uno::RuntimeException, std::exception)
{
    css::uno::Sequence < css::util::RevisionTag > aVersions;

    const OUString sDocName( XMLN_VERSIONSLIST  );
    uno::Reference< container::XNameAccess > xRootNames( xRoot, uno::UNO_QUERY );

    try {
        if ( xRootNames.is() && xRootNames->hasByName( sDocName ) && xRoot->isStreamElement( sDocName ) )
        {
            Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();

            InputSource aParserInput;

            uno::Reference< beans::XPropertySet > xProps( xRoot, uno::UNO_QUERY );
            OSL_ENSURE( xProps.is(), "Storage must implement XPropertySet!\n" );
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
                        "The stream was successfully opened for reading, the input part must be accessible!\n" );
            if ( !aParserInput.aInputStream.is() )
                throw uno::RuntimeException();

            // get filter
            Reference< XDocumentHandler > xFilter = new XMLVersionListImport( xContext, aVersions );

            // connect parser and filter
            Reference< XParser > xParser = xml::sax::Parser::create(xContext);
            xParser->setDocumentHandler( xFilter );

            // parse
            try
            {
                xParser->parseStream( aParserInput );
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
    throw (css::uno::RuntimeException, std::exception)
{
    return OUString("XMLVersionListPersistence");
}

sal_Bool XMLVersionListPersistence::supportsService(
    OUString const & ServiceName)
    throw (css::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString>
XMLVersionListPersistence::getSupportedServiceNames()
    throw (css::uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<OUString>{
        "com.sun.star.document.DocumentRevisionListPersistence"};
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
XMLVersionListPersistence_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new XMLVersionListPersistence());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
