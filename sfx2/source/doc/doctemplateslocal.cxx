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


#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <comphelper/attributelist.hxx>

#include "doctemplateslocal.hxx"

using namespace ::com::sun::star;

// -----------------------------------
uno::Sequence< beans::StringPair > DocTemplLocaleHelper::ReadGroupLocalizationSequence( const uno::Reference< io::XInputStream >& xInStream, const uno::Reference< uno::XComponentContext > xContext )
    throw( uno::Exception )
{
    ::rtl::OUString aStringID = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "groupuinames.xml" ) );
    return ReadLocalizationSequence_Impl( xInStream, aStringID, xContext );
}

// -----------------------------------
void SAL_CALL DocTemplLocaleHelper::WriteGroupLocalizationSequence( const uno::Reference< io::XOutputStream >& xOutStream, const uno::Sequence< beans::StringPair >& aSequence, const uno::Reference< uno::XComponentContext > xContext )
    throw( uno::Exception )
{
    if ( !xOutStream.is() )
        throw uno::RuntimeException();

    uno::Reference< xml::sax::XWriter > xWriterHandler(
        xml::sax::Writer::create(xContext) );

    xWriterHandler->setOutputStream( xOutStream );

    ::rtl::OUString aGroupListElement( RTL_CONSTASCII_USTRINGPARAM( "groupuinames:template-group-list" ) );
    ::rtl::OUString aGroupElement( RTL_CONSTASCII_USTRINGPARAM( "groupuinames:template-group" ) );
    ::rtl::OUString aNameAttr( RTL_CONSTASCII_USTRINGPARAM( "groupuinames:name" ) );
    ::rtl::OUString aUINameAttr( RTL_CONSTASCII_USTRINGPARAM( "groupuinames:default-ui-name" ) );
    ::rtl::OUString aCDATAString( RTL_CONSTASCII_USTRINGPARAM ( "CDATA" ) );
    ::rtl::OUString aWhiteSpace( RTL_CONSTASCII_USTRINGPARAM ( " " ) );

    // write the namespace
    ::comphelper::AttributeList* pRootAttrList = new ::comphelper::AttributeList;
    uno::Reference< xml::sax::XAttributeList > xRootAttrList( pRootAttrList );
    pRootAttrList->AddAttribute(
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "xmlns" ) ),
        aCDATAString,
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "http://openoffice.org/2006/groupuinames" ) ) );

    xWriterHandler->startDocument();
    xWriterHandler->startElement( aGroupListElement, xRootAttrList );

    for ( sal_Int32 nInd = 0; nInd < aSequence.getLength(); nInd++ )
    {
        ::comphelper::AttributeList *pAttrList = new ::comphelper::AttributeList;
        uno::Reference< xml::sax::XAttributeList > xAttrList( pAttrList );
        pAttrList->AddAttribute( aNameAttr, aCDATAString, aSequence[nInd].First );
        pAttrList->AddAttribute( aUINameAttr, aCDATAString, aSequence[nInd].Second );

        xWriterHandler->startElement( aGroupElement, xAttrList );
        xWriterHandler->ignorableWhitespace( aWhiteSpace );
        xWriterHandler->endElement( aGroupElement );
    }

    xWriterHandler->ignorableWhitespace( aWhiteSpace );
    xWriterHandler->endElement( aGroupListElement );
    xWriterHandler->endDocument();
}

// ==================================================================================

// -----------------------------------
uno::Sequence< beans::StringPair > SAL_CALL DocTemplLocaleHelper::ReadLocalizationSequence_Impl( const uno::Reference< io::XInputStream >& xInStream, const ::rtl::OUString& aStringID, const uno::Reference< uno::XComponentContext > xContext )
    throw( uno::Exception )
{
    if ( !xContext.is() || !xInStream.is() )
        throw uno::RuntimeException();

    uno::Sequence< beans::StringPair > aResult;

    uno::Reference< xml::sax::XParser > xParser = xml::sax::Parser::create( xContext );

    DocTemplLocaleHelper* pHelper = new DocTemplLocaleHelper();
    uno::Reference< xml::sax::XDocumentHandler > xHelper( static_cast< xml::sax::XDocumentHandler* >( pHelper ) );
    xml::sax::InputSource aParserInput;
    aParserInput.aInputStream = xInStream;
    aParserInput.sSystemId = aStringID;
    xParser->setDocumentHandler( xHelper );
    xParser->parseStream( aParserInput );
    xParser->setDocumentHandler( uno::Reference < xml::sax::XDocumentHandler > () );

    return pHelper->GetParsingResult();
}

// -----------------------------------
DocTemplLocaleHelper::DocTemplLocaleHelper()
: m_aGroupListElement( RTL_CONSTASCII_USTRINGPARAM( "groupuinames:template-group-list" ) )
, m_aGroupElement( RTL_CONSTASCII_USTRINGPARAM( "groupuinames:template-group" ) )
, m_aNameAttr( RTL_CONSTASCII_USTRINGPARAM( "groupuinames:name" ) )
, m_aUINameAttr( RTL_CONSTASCII_USTRINGPARAM( "groupuinames:default-ui-name" ) )
{
}

// -----------------------------------
DocTemplLocaleHelper::~DocTemplLocaleHelper()
{
}

// -----------------------------------
uno::Sequence< beans::StringPair > DocTemplLocaleHelper::GetParsingResult()
{
    if ( m_aElementsSeq.getLength() )
        throw uno::RuntimeException(); // the parsing has still not finished!

    return m_aResultSeq;
}

// -----------------------------------
void SAL_CALL DocTemplLocaleHelper::startDocument()
        throw(xml::sax::SAXException, uno::RuntimeException)
{
}

// -----------------------------------
void SAL_CALL DocTemplLocaleHelper::endDocument()
        throw(xml::sax::SAXException, uno::RuntimeException)
{
}

// -----------------------------------
void SAL_CALL DocTemplLocaleHelper::startElement( const ::rtl::OUString& aName, const uno::Reference< xml::sax::XAttributeList >& xAttribs )
        throw( xml::sax::SAXException, uno::RuntimeException )
{
    if ( aName == m_aGroupListElement )
    {
        sal_Int32 nNewLength = m_aElementsSeq.getLength() + 1;

        if ( nNewLength != 1 )
            throw xml::sax::SAXException(); // TODO: this element must be the first level element

        m_aElementsSeq.realloc( nNewLength );
        m_aElementsSeq[nNewLength-1] = aName;

        return; // nothing to do
    }
    else if ( aName == m_aGroupElement )
    {
        sal_Int32 nNewLength = m_aElementsSeq.getLength() + 1;
        if ( nNewLength != 2 )
            throw xml::sax::SAXException(); // TODO: this element must be the second level element

        m_aElementsSeq.realloc( nNewLength );
        m_aElementsSeq[nNewLength-1] = aName;

        sal_Int32 nNewEntryNum = m_aResultSeq.getLength() + 1;
        m_aResultSeq.realloc( nNewEntryNum );

        ::rtl::OUString aNameValue = xAttribs->getValueByName( m_aNameAttr );
        if ( aNameValue.isEmpty() )
            throw xml::sax::SAXException(); // TODO: the ID value must present

        ::rtl::OUString aUINameValue = xAttribs->getValueByName( m_aUINameAttr );
        if ( aUINameValue.isEmpty() )
            throw xml::sax::SAXException(); // TODO: the ID value must present

        m_aResultSeq[nNewEntryNum-1].First = aNameValue;
        m_aResultSeq[nNewEntryNum-1].Second = aUINameValue;
    }
    else
    {
        // accept future extensions
        sal_Int32 nNewLength = m_aElementsSeq.getLength() + 1;

        if ( !nNewLength )
            throw xml::sax::SAXException(); // TODO: the extension element must not be the first level element

        m_aElementsSeq.realloc( nNewLength );
        m_aElementsSeq[nNewLength-1] = aName;
    }
}

// -----------------------------------
void SAL_CALL DocTemplLocaleHelper::endElement( const ::rtl::OUString& aName )
    throw( xml::sax::SAXException, uno::RuntimeException )
{
    sal_Int32 nLength = m_aElementsSeq.getLength();
    if ( nLength <= 0 )
        throw xml::sax::SAXException(); // TODO: no other end elements expected!

    if ( !m_aElementsSeq[nLength-1].equals( aName ) )
        throw xml::sax::SAXException(); // TODO: unexpected element ended

    m_aElementsSeq.realloc( nLength - 1 );
}

// -----------------------------------
void SAL_CALL DocTemplLocaleHelper::characters( const ::rtl::OUString& /*aChars*/ )
        throw(xml::sax::SAXException, uno::RuntimeException)
{
}

// -----------------------------------
void SAL_CALL DocTemplLocaleHelper::ignorableWhitespace( const ::rtl::OUString& /*aWhitespaces*/ )
        throw(xml::sax::SAXException, uno::RuntimeException)
{
}

// -----------------------------------
void SAL_CALL DocTemplLocaleHelper::processingInstruction( const ::rtl::OUString& /*aTarget*/, const ::rtl::OUString& /*aData*/ )
        throw(xml::sax::SAXException, uno::RuntimeException)
{
}

// -----------------------------------
void SAL_CALL DocTemplLocaleHelper::setDocumentLocator( const uno::Reference< xml::sax::XLocator >& /*xLocator*/ )
        throw(xml::sax::SAXException, uno::RuntimeException)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
