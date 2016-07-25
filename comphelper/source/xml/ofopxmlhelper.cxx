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


#include <comphelper/ofopxmlhelper.hxx>
#include <comphelper/attributelist.hxx>

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <vector>

#define RELATIONINFO_FORMAT 0
#define CONTENTTYPE_FORMAT  1
#define FORMAT_MAX_ID CONTENTTYPE_FORMAT

using namespace ::com::sun::star;

namespace comphelper {

// this helper class is designed to allow to parse ContentType- and Relationship-related information from OfficeOpenXML format
class OFOPXMLHelper_Impl
    : public cppu::WeakImplHelper< css::xml::sax::XDocumentHandler >
{
    sal_uInt16 m_nFormat; // which format to parse

    // Relations info related strings
    OUString m_aRelListElement;
    OUString m_aRelElement;
    OUString m_aIDAttr;
    OUString m_aTypeAttr;
    OUString m_aTargetModeAttr;
    OUString m_aTargetAttr;

    // ContentType related strings
    OUString m_aTypesElement;
    OUString m_aDefaultElement;
    OUString m_aOverrideElement;
    OUString m_aExtensionAttr;
    OUString m_aPartNameAttr;
    OUString m_aContentTypeAttr;

    css::uno::Sequence< css::uno::Sequence< css::beans::StringPair > > m_aResultSeq;
    std::vector< OUString > m_aElementsSeq; // stack of elements being parsed


public:
    css::uno::Sequence< css::uno::Sequence< css::beans::StringPair > > GetParsingResult();

    explicit OFOPXMLHelper_Impl( sal_uInt16 nFormat ); // must not be created directly
    virtual ~OFOPXMLHelper_Impl();

    // XDocumentHandler
    virtual void SAL_CALL startDocument() throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endDocument() throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL startElement( const OUString& aName, const css::uno::Reference< css::xml::sax::XAttributeList >& xAttribs ) throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endElement( const OUString& aName ) throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL characters( const OUString& aChars ) throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL ignorableWhitespace( const OUString& aWhitespaces ) throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL processingInstruction( const OUString& aTarget, const OUString& aData ) throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDocumentLocator( const css::uno::Reference< css::xml::sax::XLocator >& xLocator ) throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
};


namespace OFOPXMLHelper {

static uno::Sequence<uno::Sequence< beans::StringPair>> ReadSequence_Impl(
    const uno::Reference<io::XInputStream>& xInStream,
    const OUString& aStringID, sal_uInt16 nFormat,
    const uno::Reference<uno::XComponentContext>& xContext)
throw (uno::Exception);

uno::Sequence< uno::Sequence< beans::StringPair > > ReadRelationsInfoSequence(
        const uno::Reference< io::XInputStream >& xInStream,
        const OUString & aStreamName,
        const uno::Reference< uno::XComponentContext >& rContext )
    throw( uno::Exception )
{
    OUString aStringID = "_rels/" + aStreamName;
    return ReadSequence_Impl( xInStream, aStringID, RELATIONINFO_FORMAT, rContext );
}


uno::Sequence< uno::Sequence< beans::StringPair > > ReadContentTypeSequence(
        const uno::Reference< io::XInputStream >& xInStream,
        const uno::Reference< uno::XComponentContext >& rContext )
    throw( uno::Exception )
{
    OUString aStringID = "[Content_Types].xml";
    return ReadSequence_Impl( xInStream, aStringID, CONTENTTYPE_FORMAT, rContext );
}


void WriteRelationsInfoSequence(
        const uno::Reference< io::XOutputStream >& xOutStream,
        const uno::Sequence< uno::Sequence< beans::StringPair > >& aSequence,
        const uno::Reference< uno::XComponentContext >& rContext )
    throw( uno::Exception )
{
    if ( !xOutStream.is() )
        throw uno::RuntimeException();

    uno::Reference< css::xml::sax::XWriter > xWriter = css::xml::sax::Writer::create(rContext);

    xWriter->setOutputStream( xOutStream );

    OUString aRelListElement( "Relationships" );
    OUString aRelElement( "Relationship" );
    OUString aCDATAString( "CDATA" );
    OUString aWhiteSpace( " " );

    // write the namespace
    AttributeList* pRootAttrList = new AttributeList;
    uno::Reference< css::xml::sax::XAttributeList > xRootAttrList( pRootAttrList );
    pRootAttrList->AddAttribute(
        "xmlns",
        aCDATAString,
        "http://schemas.openxmlformats.org/package/2006/relationships" );

    xWriter->startDocument();
    xWriter->startElement( aRelListElement, xRootAttrList );

    for ( sal_Int32 nInd = 0; nInd < aSequence.getLength(); nInd++ )
    {
        AttributeList *pAttrList = new AttributeList;
        uno::Reference< css::xml::sax::XAttributeList > xAttrList( pAttrList );
        for( sal_Int32 nSecInd = 0; nSecInd < aSequence[nInd].getLength(); nSecInd++ )
        {
            if ( aSequence[nInd][nSecInd].First == "Id"
              || aSequence[nInd][nSecInd].First == "Type"
              || aSequence[nInd][nSecInd].First == "TargetMode"
              || aSequence[nInd][nSecInd].First == "Target" )
            {
                pAttrList->AddAttribute( aSequence[nInd][nSecInd].First, aCDATAString, aSequence[nInd][nSecInd].Second );
            }
            else
            {
                // TODO/LATER: should the extensions be allowed?
                throw lang::IllegalArgumentException();
            }
        }

        xWriter->startElement( aRelElement, xAttrList );
        xWriter->ignorableWhitespace( aWhiteSpace );
        xWriter->endElement( aRelElement );
    }

    xWriter->ignorableWhitespace( aWhiteSpace );
    xWriter->endElement( aRelListElement );
    xWriter->endDocument();
}


void WriteContentSequence(
        const uno::Reference< io::XOutputStream >& xOutStream,
        const uno::Sequence< beans::StringPair >& aDefaultsSequence,
        const uno::Sequence< beans::StringPair >& aOverridesSequence,
        const uno::Reference< uno::XComponentContext >& rContext )
    throw( uno::Exception )
{
    if ( !xOutStream.is() )
        throw uno::RuntimeException();

    uno::Reference< css::xml::sax::XWriter > xWriter = css::xml::sax::Writer::create(rContext);

    xWriter->setOutputStream( xOutStream );

    OUString aTypesElement( "Types" );
    OUString aDefaultElement( "Default" );
    OUString aOverrideElement( "Override" );
    OUString aContentTypeAttr( "ContentType" );
    OUString aCDATAString( "CDATA" );
    OUString aWhiteSpace( " " );

    // write the namespace
    AttributeList* pRootAttrList = new AttributeList;
    uno::Reference< css::xml::sax::XAttributeList > xRootAttrList( pRootAttrList );
    pRootAttrList->AddAttribute(
        "xmlns",
        aCDATAString,
        "http://schemas.openxmlformats.org/package/2006/content-types" );

    xWriter->startDocument();
    xWriter->startElement( aTypesElement, xRootAttrList );

    for ( sal_Int32 nInd = 0; nInd < aDefaultsSequence.getLength(); nInd++ )
    {
        AttributeList *pAttrList = new AttributeList;
        uno::Reference< css::xml::sax::XAttributeList > xAttrList( pAttrList );
        pAttrList->AddAttribute( "Extension", aCDATAString, aDefaultsSequence[nInd].First );
        pAttrList->AddAttribute( aContentTypeAttr, aCDATAString, aDefaultsSequence[nInd].Second );

        xWriter->startElement( aDefaultElement, xAttrList );
        xWriter->ignorableWhitespace( aWhiteSpace );
        xWriter->endElement( aDefaultElement );
    }

    for ( sal_Int32 nInd = 0; nInd < aOverridesSequence.getLength(); nInd++ )
    {
        AttributeList *pAttrList = new AttributeList;
        uno::Reference< css::xml::sax::XAttributeList > xAttrList( pAttrList );
        pAttrList->AddAttribute( "PartName", aCDATAString, aOverridesSequence[nInd].First );
        pAttrList->AddAttribute( aContentTypeAttr, aCDATAString, aOverridesSequence[nInd].Second );

        xWriter->startElement( aOverrideElement, xAttrList );
        xWriter->ignorableWhitespace( aWhiteSpace );
        xWriter->endElement( aOverrideElement );
    }

    xWriter->ignorableWhitespace( aWhiteSpace );
    xWriter->endElement( aTypesElement );
    xWriter->endDocument();

}

uno::Sequence< uno::Sequence< beans::StringPair > > ReadSequence_Impl(
        const uno::Reference< io::XInputStream >& xInStream,
        const OUString& aStringID, sal_uInt16 nFormat,
        const uno::Reference< uno::XComponentContext >& rContext )
    throw( uno::Exception )
{
    if ( !rContext.is() || !xInStream.is() || nFormat > FORMAT_MAX_ID )
        throw uno::RuntimeException();

    uno::Reference< css::xml::sax::XParser > xParser = css::xml::sax::Parser::create( rContext );

    OFOPXMLHelper_Impl *const pHelper = new OFOPXMLHelper_Impl( nFormat );
    uno::Reference< css::xml::sax::XDocumentHandler > xHelper( static_cast< css::xml::sax::XDocumentHandler* >( pHelper ) );
    css::xml::sax::InputSource aParserInput;
    aParserInput.aInputStream = xInStream;
    aParserInput.sSystemId = aStringID;
    xParser->setDocumentHandler( xHelper );
    xParser->parseStream( aParserInput );
    xParser->setDocumentHandler( uno::Reference < css::xml::sax::XDocumentHandler > () );

    return pHelper->GetParsingResult();
}

} // namespace OFOPXMLHelper

OFOPXMLHelper_Impl::OFOPXMLHelper_Impl( sal_uInt16 nFormat )
: m_nFormat( nFormat )
, m_aRelListElement( "Relationships" )
, m_aRelElement( "Relationship" )
, m_aIDAttr( "Id" )
, m_aTypeAttr( "Type" )
, m_aTargetModeAttr( "TargetMode" )
, m_aTargetAttr( "Target" )
, m_aTypesElement( "Types" )
, m_aDefaultElement( "Default" )
, m_aOverrideElement( "Override" )
, m_aExtensionAttr( "Extension" )
, m_aPartNameAttr( "PartName" )
, m_aContentTypeAttr( "ContentType" )
{
}


OFOPXMLHelper_Impl::~OFOPXMLHelper_Impl()
{
}


uno::Sequence< uno::Sequence< beans::StringPair > > OFOPXMLHelper_Impl::GetParsingResult()
{
    if ( m_aElementsSeq.size() )
        throw uno::RuntimeException(); // the parsing has still not finished!

    return m_aResultSeq;
}


void SAL_CALL OFOPXMLHelper_Impl::startDocument()
        throw(css::xml::sax::SAXException, uno::RuntimeException, std::exception)
{
}


void SAL_CALL OFOPXMLHelper_Impl::endDocument()
        throw(css::xml::sax::SAXException, uno::RuntimeException, std::exception)
{
}


void SAL_CALL OFOPXMLHelper_Impl::startElement( const OUString& aName, const uno::Reference< css::xml::sax::XAttributeList >& xAttribs )
        throw( css::xml::sax::SAXException, uno::RuntimeException, std::exception )
{
    if ( m_nFormat == RELATIONINFO_FORMAT )
    {
        if ( aName == m_aRelListElement )
        {
            sal_Int32 nNewLength = m_aElementsSeq.size() + 1;

            if ( nNewLength != 1 )
                throw css::xml::sax::SAXException(); // TODO: this element must be the first level element

            m_aElementsSeq.push_back( aName );

            return; // nothing to do
        }
        else if ( aName == m_aRelElement )
        {
            sal_Int32 nNewLength = m_aElementsSeq.size() + 1;
            if ( nNewLength != 2 )
                throw css::xml::sax::SAXException(); // TODO: this element must be the second level element

            m_aElementsSeq.push_back( aName );

            sal_Int32 nNewEntryNum = m_aResultSeq.getLength() + 1;
            m_aResultSeq.realloc( nNewEntryNum );
            sal_Int32 nAttrNum = 0;
            m_aResultSeq[nNewEntryNum-1].realloc( 4 ); // the maximal expected number of arguments is 4

            OUString aIDValue = xAttribs->getValueByName( m_aIDAttr );
            if ( aIDValue.isEmpty() )
                throw css::xml::sax::SAXException(); // TODO: the ID value must present

            OUString aTypeValue = xAttribs->getValueByName( m_aTypeAttr );
            OUString aTargetValue = xAttribs->getValueByName( m_aTargetAttr );
            OUString aTargetModeValue = xAttribs->getValueByName( m_aTargetModeAttr );

            m_aResultSeq[nNewEntryNum-1][++nAttrNum - 1].First = m_aIDAttr;
            m_aResultSeq[nNewEntryNum-1][nAttrNum - 1].Second = aIDValue;

            if ( !aTypeValue.isEmpty() )
            {
                m_aResultSeq[nNewEntryNum-1][++nAttrNum - 1].First = m_aTypeAttr;
                m_aResultSeq[nNewEntryNum-1][nAttrNum - 1].Second = aTypeValue;
            }

            if ( !aTargetValue.isEmpty() )
            {
                m_aResultSeq[nNewEntryNum-1][++nAttrNum - 1].First = m_aTargetAttr;
                m_aResultSeq[nNewEntryNum-1][nAttrNum - 1].Second = aTargetValue;
            }

            if ( !aTargetModeValue.isEmpty() )
            {
                m_aResultSeq[nNewEntryNum-1][++nAttrNum - 1].First = m_aTargetModeAttr;
                m_aResultSeq[nNewEntryNum-1][nAttrNum - 1].Second = aTargetModeValue;
            }

            m_aResultSeq[nNewEntryNum-1].realloc( nAttrNum );
        }
        else
            throw css::xml::sax::SAXException(); // TODO: no other elements expected!
    }
    else if ( m_nFormat == CONTENTTYPE_FORMAT )
    {
        if ( aName == m_aTypesElement )
        {
            sal_Int32 nNewLength = m_aElementsSeq.size() + 1;

            if ( nNewLength != 1 )
                throw css::xml::sax::SAXException(); // TODO: this element must be the first level element

            m_aElementsSeq.push_back( aName );

            if ( !m_aResultSeq.getLength() )
                m_aResultSeq.realloc( 2 );

            return; // nothing to do
        }
        else if ( aName == m_aDefaultElement )
        {
            sal_Int32 nNewLength = m_aElementsSeq.size() + 1;
            if ( nNewLength != 2 )
                throw css::xml::sax::SAXException(); // TODO: this element must be the second level element

            m_aElementsSeq.push_back( aName );

            if ( !m_aResultSeq.getLength() )
                m_aResultSeq.realloc( 2 );

            if ( m_aResultSeq.getLength() != 2 )
                throw uno::RuntimeException();

            OUString aExtensionValue = xAttribs->getValueByName( m_aExtensionAttr );
            if ( aExtensionValue.isEmpty() )
                throw css::xml::sax::SAXException(); // TODO: the Extension value must present

            OUString aContentTypeValue = xAttribs->getValueByName( m_aContentTypeAttr );
            if ( aContentTypeValue.isEmpty() )
                throw css::xml::sax::SAXException(); // TODO: the ContentType value must present

            sal_Int32 nNewResultLen = m_aResultSeq[0].getLength() + 1;
            m_aResultSeq[0].realloc( nNewResultLen );

            m_aResultSeq[0][nNewResultLen-1].First = aExtensionValue;
            m_aResultSeq[0][nNewResultLen-1].Second = aContentTypeValue;
        }
        else if ( aName == m_aOverrideElement )
        {
            sal_Int32 nNewLength = m_aElementsSeq.size() + 1;
            if ( nNewLength != 2 )
                throw css::xml::sax::SAXException(); // TODO: this element must be the second level element

            m_aElementsSeq.push_back( aName );

            if ( !m_aResultSeq.getLength() )
                m_aResultSeq.realloc( 2 );

            if ( m_aResultSeq.getLength() != 2 )
                throw uno::RuntimeException();

            OUString aPartNameValue = xAttribs->getValueByName( m_aPartNameAttr );
            if ( aPartNameValue.isEmpty() )
                throw css::xml::sax::SAXException(); // TODO: the PartName value must present

            OUString aContentTypeValue = xAttribs->getValueByName( m_aContentTypeAttr );
            if ( aContentTypeValue.isEmpty() )
                throw css::xml::sax::SAXException(); // TODO: the ContentType value must present

            sal_Int32 nNewResultLen = m_aResultSeq[1].getLength() + 1;
            m_aResultSeq[1].realloc( nNewResultLen );

            m_aResultSeq[1][nNewResultLen-1].First = aPartNameValue;
            m_aResultSeq[1][nNewResultLen-1].Second = aContentTypeValue;
        }
        else
            throw css::xml::sax::SAXException(); // TODO: no other elements expected!
    }
    else
        throw css::xml::sax::SAXException(); // TODO: no other elements expected!
}


void SAL_CALL OFOPXMLHelper_Impl::endElement( const OUString& aName )
    throw( css::xml::sax::SAXException, uno::RuntimeException, std::exception )
{
    if ( m_nFormat == RELATIONINFO_FORMAT || m_nFormat == CONTENTTYPE_FORMAT )
    {
        sal_Int32 nLength = m_aElementsSeq.size();
        if ( nLength <= 0 )
            throw css::xml::sax::SAXException(); // TODO: no other end elements expected!

        if ( !m_aElementsSeq[nLength-1].equals( aName ) )
            throw css::xml::sax::SAXException(); // TODO: unexpected element ended

        m_aElementsSeq.resize( nLength - 1 );
    }
}


void SAL_CALL OFOPXMLHelper_Impl::characters( const OUString& /*aChars*/ )
        throw(css::xml::sax::SAXException, uno::RuntimeException, std::exception)
{
}


void SAL_CALL OFOPXMLHelper_Impl::ignorableWhitespace( const OUString& /*aWhitespaces*/ )
        throw(css::xml::sax::SAXException, uno::RuntimeException, std::exception)
{
}


void SAL_CALL OFOPXMLHelper_Impl::processingInstruction( const OUString& /*aTarget*/, const OUString& /*aData*/ )
        throw(css::xml::sax::SAXException, uno::RuntimeException, std::exception)
{
}


void SAL_CALL OFOPXMLHelper_Impl::setDocumentLocator( const uno::Reference< css::xml::sax::XLocator >& /*xLocator*/ )
        throw(css::xml::sax::SAXException, uno::RuntimeException, std::exception)
{
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
