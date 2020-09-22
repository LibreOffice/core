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
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>

#include <comphelper/attributelist.hxx>

#include "doctemplateslocal.hxx"

using namespace ::com::sun::star;

namespace
{

// Relations info related strings
constexpr OUStringLiteral g_sGroupListElement(u"groupuinames:template-group-list");
constexpr OUStringLiteral g_sGroupElement(u"groupuinames:template-group");
constexpr OUStringLiteral g_sNameAttr(u"groupuinames:name");
constexpr OUStringLiteral g_sUINameAttr(u"groupuinames:default-ui-name");

}

std::vector< beans::StringPair > DocTemplLocaleHelper::ReadGroupLocalizationSequence( const uno::Reference< io::XInputStream >& xInStream, const uno::Reference< uno::XComponentContext >& xContext )
{
    return ReadLocalizationSequence_Impl( xInStream, "groupuinames.xml", xContext );
}


void DocTemplLocaleHelper::WriteGroupLocalizationSequence( const uno::Reference< io::XOutputStream >& xOutStream, const std::vector< beans::StringPair >& aSequence, const uno::Reference< uno::XComponentContext >& xContext )
{
    if ( !xOutStream.is() )
        throw uno::RuntimeException();

    uno::Reference< xml::sax::XWriter > xWriterHandler(
        xml::sax::Writer::create(xContext) );

    xWriterHandler->setOutputStream( xOutStream );

    const OUString aCDATAString( "CDATA" );
    const OUString aWhiteSpace( " " );

    // write the namespace
    ::comphelper::AttributeList* pRootAttrList = new ::comphelper::AttributeList;
    uno::Reference< xml::sax::XAttributeList > xRootAttrList( pRootAttrList );
    pRootAttrList->AddAttribute(
        "xmlns:groupuinames",
        aCDATAString,
        "http://openoffice.org/2006/groupuinames" );

    xWriterHandler->startDocument();
    xWriterHandler->startElement( g_sGroupListElement, xRootAttrList );

    for (const auto & i : aSequence)
    {
        ::comphelper::AttributeList *pAttrList = new ::comphelper::AttributeList;
        uno::Reference< xml::sax::XAttributeList > xAttrList( pAttrList );
        pAttrList->AddAttribute( g_sNameAttr, aCDATAString, i.First );
        pAttrList->AddAttribute( g_sUINameAttr, aCDATAString, i.Second );

        xWriterHandler->startElement( g_sGroupElement, xAttrList );
        xWriterHandler->ignorableWhitespace( aWhiteSpace );
        xWriterHandler->endElement( g_sGroupElement );
    }

    xWriterHandler->ignorableWhitespace( aWhiteSpace );
    xWriterHandler->endElement( g_sGroupListElement );
    xWriterHandler->endDocument();
}


std::vector< beans::StringPair > DocTemplLocaleHelper::ReadLocalizationSequence_Impl( const uno::Reference< io::XInputStream >& xInStream, const OUString& aStringID, const uno::Reference< uno::XComponentContext >& xContext )
{
    if ( !xContext.is() || !xInStream.is() )
        throw uno::RuntimeException();

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


DocTemplLocaleHelper::DocTemplLocaleHelper()
{
}


DocTemplLocaleHelper::~DocTemplLocaleHelper()
{
}


std::vector< beans::StringPair > const & DocTemplLocaleHelper::GetParsingResult() const
{
    if ( !m_aElementsSeq.empty() )
        throw uno::RuntimeException(); // the parsing has still not finished!

    return m_aResultSeq;
}


void SAL_CALL DocTemplLocaleHelper::startDocument()
{
}


void SAL_CALL DocTemplLocaleHelper::endDocument()
{
}


void SAL_CALL DocTemplLocaleHelper::startElement( const OUString& aName, const uno::Reference< xml::sax::XAttributeList >& xAttribs )
{
    if ( aName == g_sGroupListElement )
    {
        if ( !m_aElementsSeq.empty() )
            throw xml::sax::SAXException(); // TODO: this element must be the first level element

        m_aElementsSeq.push_back( aName );

        return; // nothing to do
    }
    else if ( aName == g_sGroupElement )
    {
        if ( m_aElementsSeq.size() != 1 )
            throw xml::sax::SAXException(); // TODO: this element must be the second level element

        m_aElementsSeq.push_back( aName );

        const auto nNewEntryNum = m_aResultSeq.size();
        m_aResultSeq.resize( nNewEntryNum+1 );

        const OUString aNameValue = xAttribs->getValueByName( g_sNameAttr );
        if ( aNameValue.isEmpty() )
            throw xml::sax::SAXException(); // TODO: the ID value must present

        const OUString aUINameValue = xAttribs->getValueByName( g_sUINameAttr );
        if ( aUINameValue.isEmpty() )
            throw xml::sax::SAXException(); // TODO: the ID value must present

        m_aResultSeq[nNewEntryNum].First = aNameValue;
        m_aResultSeq[nNewEntryNum].Second = aUINameValue;
    }
    else
    {
        // accept future extensions
        if ( m_aElementsSeq.empty() )
            throw xml::sax::SAXException(); // TODO: the extension element must not be the first level element

        m_aElementsSeq.push_back( aName );
    }
}


void SAL_CALL DocTemplLocaleHelper::endElement( const OUString& aName )
{
    if ( m_aElementsSeq.empty() )
        throw xml::sax::SAXException(); // TODO: no other end elements expected!

    if ( m_aElementsSeq.back() != aName )
        throw xml::sax::SAXException(); // TODO: unexpected element ended

    m_aElementsSeq.pop_back();
}


void SAL_CALL DocTemplLocaleHelper::characters( const OUString& /*aChars*/ )
{
}


void SAL_CALL DocTemplLocaleHelper::ignorableWhitespace( const OUString& /*aWhitespaces*/ )
{
}


void SAL_CALL DocTemplLocaleHelper::processingInstruction( const OUString& /*aTarget*/, const OUString& /*aData*/ )
{
}


void SAL_CALL DocTemplLocaleHelper::setDocumentLocator( const uno::Reference< xml::sax::XLocator >& /*xLocator*/ )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
