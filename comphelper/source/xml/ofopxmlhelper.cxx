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
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <comphelper/ofopxmlhelper.hxx>
#include <comphelper/attributelist.hxx>

#define RELATIONINFO_FORMAT 0
#define CONTENTTYPE_FORMAT  1
#define FORMAT_MAX_ID CONTENTTYPE_FORMAT

using namespace ::com::sun::star;

namespace comphelper {

// -----------------------------------
uno::Sequence< uno::Sequence< beans::StringPair > > SAL_CALL OFOPXMLHelper::ReadRelationsInfoSequence( const uno::Reference< io::XInputStream >& xInStream, const ::rtl::OUString aStreamName, const uno::Reference< uno::XComponentContext > xContext )
    throw( uno::Exception )
{
    ::rtl::OUString aStringID = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "_rels/" ) );
    aStringID += aStreamName;
    return ReadSequence_Impl( xInStream, aStringID, RELATIONINFO_FORMAT, xContext );
}

// -----------------------------------
uno::Sequence< uno::Sequence< beans::StringPair > > SAL_CALL OFOPXMLHelper::ReadContentTypeSequence( const uno::Reference< io::XInputStream >& xInStream, const uno::Reference< uno::XComponentContext > xContext )
    throw( uno::Exception )
{
    ::rtl::OUString aStringID = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "[Content_Types].xml" ) );
    return ReadSequence_Impl( xInStream, aStringID, CONTENTTYPE_FORMAT, xContext );
}

// -----------------------------------
void SAL_CALL OFOPXMLHelper::WriteRelationsInfoSequence( const uno::Reference< io::XOutputStream >& xOutStream, const uno::Sequence< uno::Sequence< beans::StringPair > >& aSequence, const uno::Reference< uno::XComponentContext > xContext )
    throw( uno::Exception )
{
    if ( !xOutStream.is() )
        throw uno::RuntimeException();

    uno::Reference< xml::sax::XWriter > xWriter = xml::sax::Writer::create(xContext);

    xWriter->setOutputStream( xOutStream );

    ::rtl::OUString aRelListElement( RTL_CONSTASCII_USTRINGPARAM( "Relationships" ) );
    ::rtl::OUString aRelElement( RTL_CONSTASCII_USTRINGPARAM( "Relationship" ) );
    ::rtl::OUString aIDAttr( RTL_CONSTASCII_USTRINGPARAM( "Id" ) );
    ::rtl::OUString aTypeAttr( RTL_CONSTASCII_USTRINGPARAM( "Type" ) );
    ::rtl::OUString aTargetModeAttr( RTL_CONSTASCII_USTRINGPARAM( "TargetMode" ) );
    ::rtl::OUString aTargetAttr( RTL_CONSTASCII_USTRINGPARAM( "Target" ) );
    ::rtl::OUString aCDATAString( RTL_CONSTASCII_USTRINGPARAM ( "CDATA" ) );
    ::rtl::OUString aWhiteSpace( RTL_CONSTASCII_USTRINGPARAM ( " " ) );

    // write the namespace
    AttributeList* pRootAttrList = new AttributeList;
    uno::Reference< xml::sax::XAttributeList > xRootAttrList( pRootAttrList );
    pRootAttrList->AddAttribute(
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "xmlns" ) ),
        aCDATAString,
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "http://schemas.openxmlformats.org/package/2006/relationships" ) ) );

    xWriter->startDocument();
    xWriter->startElement( aRelListElement, xRootAttrList );

    for ( sal_Int32 nInd = 0; nInd < aSequence.getLength(); nInd++ )
    {
        AttributeList *pAttrList = new AttributeList;
        uno::Reference< xml::sax::XAttributeList > xAttrList( pAttrList );
        for( sal_Int32 nSecInd = 0; nSecInd < aSequence[nInd].getLength(); nSecInd++ )
        {
            if ( aSequence[nInd][nSecInd].First.equals( aIDAttr )
              || aSequence[nInd][nSecInd].First.equals( aTypeAttr )
              || aSequence[nInd][nSecInd].First.equals( aTargetModeAttr )
              || aSequence[nInd][nSecInd].First.equals( aTargetAttr ) )
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

// -----------------------------------
void SAL_CALL OFOPXMLHelper::WriteContentSequence( const uno::Reference< io::XOutputStream >& xOutStream, const uno::Sequence< beans::StringPair >& aDefaultsSequence, const uno::Sequence< beans::StringPair >& aOverridesSequence, const uno::Reference< uno::XComponentContext > xContext )
    throw( uno::Exception )
{
    if ( !xOutStream.is() )
        throw uno::RuntimeException();

    uno::Reference< xml::sax::XWriter > xWriter = xml::sax::Writer::create(xContext);

    xWriter->setOutputStream( xOutStream );

    ::rtl::OUString aTypesElement( RTL_CONSTASCII_USTRINGPARAM( "Types" ) );
    ::rtl::OUString aDefaultElement( RTL_CONSTASCII_USTRINGPARAM( "Default" ) );
    ::rtl::OUString aOverrideElement( RTL_CONSTASCII_USTRINGPARAM( "Override" ) );
    ::rtl::OUString aExtensionAttr( RTL_CONSTASCII_USTRINGPARAM( "Extension" ) );
    ::rtl::OUString aPartNameAttr( RTL_CONSTASCII_USTRINGPARAM( "PartName" ) );
    ::rtl::OUString aContentTypeAttr( RTL_CONSTASCII_USTRINGPARAM( "ContentType" ) );
    ::rtl::OUString aCDATAString( RTL_CONSTASCII_USTRINGPARAM ( "CDATA" ) );
    ::rtl::OUString aWhiteSpace( RTL_CONSTASCII_USTRINGPARAM ( " " ) );

    // write the namespace
    AttributeList* pRootAttrList = new AttributeList;
    uno::Reference< xml::sax::XAttributeList > xRootAttrList( pRootAttrList );
    pRootAttrList->AddAttribute(
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "xmlns" ) ),
        aCDATAString,
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "http://schemas.openxmlformats.org/package/2006/content-types" ) ) );

    xWriter->startDocument();
    xWriter->startElement( aTypesElement, xRootAttrList );

    for ( sal_Int32 nInd = 0; nInd < aDefaultsSequence.getLength(); nInd++ )
    {
        AttributeList *pAttrList = new AttributeList;
        uno::Reference< xml::sax::XAttributeList > xAttrList( pAttrList );
        pAttrList->AddAttribute( aExtensionAttr, aCDATAString, aDefaultsSequence[nInd].First );
        pAttrList->AddAttribute( aContentTypeAttr, aCDATAString, aDefaultsSequence[nInd].Second );

        xWriter->startElement( aDefaultElement, xAttrList );
        xWriter->ignorableWhitespace( aWhiteSpace );
        xWriter->endElement( aDefaultElement );
    }

    for ( sal_Int32 nInd = 0; nInd < aOverridesSequence.getLength(); nInd++ )
    {
        AttributeList *pAttrList = new AttributeList;
        uno::Reference< xml::sax::XAttributeList > xAttrList( pAttrList );
        pAttrList->AddAttribute( aPartNameAttr, aCDATAString, aOverridesSequence[nInd].First );
        pAttrList->AddAttribute( aContentTypeAttr, aCDATAString, aOverridesSequence[nInd].Second );

        xWriter->startElement( aOverrideElement, xAttrList );
        xWriter->ignorableWhitespace( aWhiteSpace );
        xWriter->endElement( aOverrideElement );
    }

    xWriter->ignorableWhitespace( aWhiteSpace );
    xWriter->endElement( aTypesElement );
    xWriter->endDocument();

}

// ==================================================================================

// -----------------------------------
uno::Sequence< uno::Sequence< beans::StringPair > > SAL_CALL OFOPXMLHelper::ReadSequence_Impl( const uno::Reference< io::XInputStream >& xInStream, const ::rtl::OUString& aStringID, sal_uInt16 nFormat, const uno::Reference< uno::XComponentContext > xContext )
    throw( uno::Exception )
{
    if ( !xContext.is() || !xInStream.is() || nFormat > FORMAT_MAX_ID )
        throw uno::RuntimeException();

    uno::Sequence< uno::Sequence< beans::StringPair > > aResult;

    uno::Reference< xml::sax::XParser > xParser = xml::sax::Parser::create( xContext );

    OFOPXMLHelper* pHelper = new OFOPXMLHelper( nFormat );
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
OFOPXMLHelper::OFOPXMLHelper( sal_uInt16 nFormat )
: m_nFormat( nFormat )
, m_aRelListElement( RTL_CONSTASCII_USTRINGPARAM( "Relationships" ) )
, m_aRelElement( RTL_CONSTASCII_USTRINGPARAM( "Relationship" ) )
, m_aIDAttr( RTL_CONSTASCII_USTRINGPARAM( "Id" ) )
, m_aTypeAttr( RTL_CONSTASCII_USTRINGPARAM( "Type" ) )
, m_aTargetModeAttr( RTL_CONSTASCII_USTRINGPARAM( "TargetMode" ) )
, m_aTargetAttr( RTL_CONSTASCII_USTRINGPARAM( "Target" ) )
, m_aTypesElement( RTL_CONSTASCII_USTRINGPARAM( "Types" ) )
, m_aDefaultElement( RTL_CONSTASCII_USTRINGPARAM( "Default" ) )
, m_aOverrideElement( RTL_CONSTASCII_USTRINGPARAM( "Override" ) )
, m_aExtensionAttr( RTL_CONSTASCII_USTRINGPARAM( "Extension" ) )
, m_aPartNameAttr( RTL_CONSTASCII_USTRINGPARAM( "PartName" ) )
, m_aContentTypeAttr( RTL_CONSTASCII_USTRINGPARAM( "ContentType" ) )
{
}

// -----------------------------------
OFOPXMLHelper::~OFOPXMLHelper()
{
}

// -----------------------------------
uno::Sequence< uno::Sequence< beans::StringPair > > OFOPXMLHelper::GetParsingResult()
{
    if ( m_aElementsSeq.getLength() )
        throw uno::RuntimeException(); // the parsing has still not finished!

    return m_aResultSeq;
}

// -----------------------------------
void SAL_CALL OFOPXMLHelper::startDocument()
        throw(xml::sax::SAXException, uno::RuntimeException)
{
}

// -----------------------------------
void SAL_CALL OFOPXMLHelper::endDocument()
        throw(xml::sax::SAXException, uno::RuntimeException)
{
}

// -----------------------------------
void SAL_CALL OFOPXMLHelper::startElement( const ::rtl::OUString& aName, const uno::Reference< xml::sax::XAttributeList >& xAttribs )
        throw( xml::sax::SAXException, uno::RuntimeException )
{
    if ( m_nFormat == RELATIONINFO_FORMAT )
    {
        if ( aName == m_aRelListElement )
        {
            sal_Int32 nNewLength = m_aElementsSeq.getLength() + 1;

            if ( nNewLength != 1 )
                throw xml::sax::SAXException(); // TODO: this element must be the first level element

            m_aElementsSeq.realloc( nNewLength );
            m_aElementsSeq[nNewLength-1] = aName;

            return; // nothing to do
        }
        else if ( aName == m_aRelElement )
        {
            sal_Int32 nNewLength = m_aElementsSeq.getLength() + 1;
            if ( nNewLength != 2 )
                throw xml::sax::SAXException(); // TODO: this element must be the second level element

            m_aElementsSeq.realloc( nNewLength );
            m_aElementsSeq[nNewLength-1] = aName;

            sal_Int32 nNewEntryNum = m_aResultSeq.getLength() + 1;
            m_aResultSeq.realloc( nNewEntryNum );
            sal_Int32 nAttrNum = 0;
            m_aResultSeq[nNewEntryNum-1].realloc( 4 ); // the maximal expected number of arguments is 4

            ::rtl::OUString aIDValue = xAttribs->getValueByName( m_aIDAttr );
            if ( aIDValue.isEmpty() )
                throw xml::sax::SAXException(); // TODO: the ID value must present

            ::rtl::OUString aTypeValue = xAttribs->getValueByName( m_aTypeAttr );
            ::rtl::OUString aTargetValue = xAttribs->getValueByName( m_aTargetAttr );
            ::rtl::OUString aTargetModeValue = xAttribs->getValueByName( m_aTargetModeAttr );

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
            throw xml::sax::SAXException(); // TODO: no other elements expected!
    }
    else if ( m_nFormat == CONTENTTYPE_FORMAT )
    {
        if ( aName == m_aTypesElement )
        {
            sal_Int32 nNewLength = m_aElementsSeq.getLength() + 1;

            if ( nNewLength != 1 )
                throw xml::sax::SAXException(); // TODO: this element must be the first level element

            m_aElementsSeq.realloc( nNewLength );
            m_aElementsSeq[nNewLength-1] = aName;

            if ( !m_aResultSeq.getLength() )
                m_aResultSeq.realloc( 2 );

            return; // nothing to do
        }
        else if ( aName == m_aDefaultElement )
        {
            sal_Int32 nNewLength = m_aElementsSeq.getLength() + 1;
            if ( nNewLength != 2 )
                throw xml::sax::SAXException(); // TODO: this element must be the second level element

            m_aElementsSeq.realloc( nNewLength );
            m_aElementsSeq[nNewLength-1] = aName;

            if ( !m_aResultSeq.getLength() )
                m_aResultSeq.realloc( 2 );

            if ( m_aResultSeq.getLength() != 2 )
                throw uno::RuntimeException();

            ::rtl::OUString aExtensionValue = xAttribs->getValueByName( m_aExtensionAttr );
            if ( aExtensionValue.isEmpty() )
                throw xml::sax::SAXException(); // TODO: the Extension value must present

            ::rtl::OUString aContentTypeValue = xAttribs->getValueByName( m_aContentTypeAttr );
            if ( aContentTypeValue.isEmpty() )
                throw xml::sax::SAXException(); // TODO: the ContentType value must present

            sal_Int32 nNewResultLen = m_aResultSeq[0].getLength() + 1;
            m_aResultSeq[0].realloc( nNewResultLen );

            m_aResultSeq[0][nNewResultLen-1].First = aExtensionValue;
            m_aResultSeq[0][nNewResultLen-1].Second = aContentTypeValue;
        }
        else if ( aName == m_aOverrideElement )
        {
            sal_Int32 nNewLength = m_aElementsSeq.getLength() + 1;
            if ( nNewLength != 2 )
                throw xml::sax::SAXException(); // TODO: this element must be the second level element

            m_aElementsSeq.realloc( nNewLength );
            m_aElementsSeq[nNewLength-1] = aName;

            if ( !m_aResultSeq.getLength() )
                m_aResultSeq.realloc( 2 );

            if ( m_aResultSeq.getLength() != 2 )
                throw uno::RuntimeException();

            ::rtl::OUString aPartNameValue = xAttribs->getValueByName( m_aPartNameAttr );
            if ( aPartNameValue.isEmpty() )
                throw xml::sax::SAXException(); // TODO: the PartName value must present

            ::rtl::OUString aContentTypeValue = xAttribs->getValueByName( m_aContentTypeAttr );
            if ( aContentTypeValue.isEmpty() )
                throw xml::sax::SAXException(); // TODO: the ContentType value must present

            sal_Int32 nNewResultLen = m_aResultSeq[1].getLength() + 1;
            m_aResultSeq[1].realloc( nNewResultLen );

            m_aResultSeq[1][nNewResultLen-1].First = aPartNameValue;
            m_aResultSeq[1][nNewResultLen-1].Second = aContentTypeValue;
        }
        else
            throw xml::sax::SAXException(); // TODO: no other elements expected!
    }
    else
        throw xml::sax::SAXException(); // TODO: no other elements expected!
}

// -----------------------------------
void SAL_CALL OFOPXMLHelper::endElement( const ::rtl::OUString& aName )
    throw( xml::sax::SAXException, uno::RuntimeException )
{
    if ( m_nFormat == RELATIONINFO_FORMAT || m_nFormat == CONTENTTYPE_FORMAT )
    {
        sal_Int32 nLength = m_aElementsSeq.getLength();
        if ( nLength <= 0 )
            throw xml::sax::SAXException(); // TODO: no other end elements expected!

        if ( !m_aElementsSeq[nLength-1].equals( aName ) )
            throw xml::sax::SAXException(); // TODO: unexpected element ended

        m_aElementsSeq.realloc( nLength - 1 );
    }
}

// -----------------------------------
void SAL_CALL OFOPXMLHelper::characters( const ::rtl::OUString& /*aChars*/ )
        throw(xml::sax::SAXException, uno::RuntimeException)
{
}

// -----------------------------------
void SAL_CALL OFOPXMLHelper::ignorableWhitespace( const ::rtl::OUString& /*aWhitespaces*/ )
        throw(xml::sax::SAXException, uno::RuntimeException)
{
}

// -----------------------------------
void SAL_CALL OFOPXMLHelper::processingInstruction( const ::rtl::OUString& /*aTarget*/, const ::rtl::OUString& /*aData*/ )
        throw(xml::sax::SAXException, uno::RuntimeException)
{
}

// -----------------------------------
void SAL_CALL OFOPXMLHelper::setDocumentLocator( const uno::Reference< xml::sax::XLocator >& /*xLocator*/ )
        throw(xml::sax::SAXException, uno::RuntimeException)
{
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
