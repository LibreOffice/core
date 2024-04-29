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
#include <rtl/ref.hxx>

#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <vector>

#define RELATIONINFO_FORMAT 0
#define CONTENTTYPE_FORMAT  1
#define FORMAT_MAX_ID CONTENTTYPE_FORMAT

using namespace ::com::sun::star;

namespace comphelper {

namespace {

// this helper class is designed to allow to parse ContentType- and Relationship-related information from OfficeOpenXML format
class OFOPXMLHelper_Impl
    : public cppu::WeakImplHelper< css::xml::sax::XDocumentHandler >
{
    sal_uInt16 const m_nFormat; // which format to parse

    css::uno::Sequence< css::uno::Sequence< css::beans::StringPair > > m_aResultSeq;
    std::vector< OUString > m_aElementsSeq; // stack of elements being parsed


public:
    css::uno::Sequence< css::uno::Sequence< css::beans::StringPair > > const & GetParsingResult() const;

    explicit OFOPXMLHelper_Impl( sal_uInt16 nFormat ); // must not be created directly

    // XDocumentHandler
    virtual void SAL_CALL startDocument() override;
    virtual void SAL_CALL endDocument() override;
    virtual void SAL_CALL startElement( const OUString& aName, const css::uno::Reference< css::xml::sax::XAttributeList >& xAttribs ) override;
    virtual void SAL_CALL endElement( const OUString& aName ) override;
    virtual void SAL_CALL characters( const OUString& aChars ) override;
    virtual void SAL_CALL ignorableWhitespace( const OUString& aWhitespaces ) override;
    virtual void SAL_CALL processingInstruction( const OUString& aTarget, const OUString& aData ) override;
    virtual void SAL_CALL setDocumentLocator( const css::uno::Reference< css::xml::sax::XLocator >& xLocator ) override;
};

}

namespace OFOPXMLHelper {

/// @throws css::uno::Exception
static uno::Sequence<uno::Sequence< beans::StringPair>> ReadSequence_Impl(
    const uno::Reference<io::XInputStream>& xInStream,
    const OUString& aStringID, sal_uInt16 nFormat,
    const uno::Reference<uno::XComponentContext>& xContext);

uno::Sequence< uno::Sequence< beans::StringPair > > ReadRelationsInfoSequence(
        const uno::Reference< io::XInputStream >& xInStream,
        std::u16string_view aStreamName,
        const uno::Reference< uno::XComponentContext >& rContext )
{
    OUString aStringID = OUString::Concat("_rels/") + aStreamName;
    return ReadSequence_Impl( xInStream, aStringID, RELATIONINFO_FORMAT, rContext );
}


uno::Sequence< uno::Sequence< beans::StringPair > > ReadContentTypeSequence(
        const uno::Reference< io::XInputStream >& xInStream,
        const uno::Reference< uno::XComponentContext >& rContext )
{
    return ReadSequence_Impl( xInStream, u"[Content_Types].xml"_ustr, CONTENTTYPE_FORMAT, rContext );
}

OUString GetContentTypeByName(
                const css::uno::Sequence<css::uno::Sequence<css::beans::StringPair>>& rContentTypes,
                const OUString& rFilename)
{
    if (rContentTypes.getLength() < 2)
    {
        return OUString();
    }

    const uno::Sequence<beans::StringPair>& rDefaults = rContentTypes[0];
    const uno::Sequence<beans::StringPair>& rOverrides = rContentTypes[1];

    // Find the extension and use it to get the type.
    const sal_Int32 nDotOffset = rFilename.lastIndexOf('.');
    const OUString aExt = (nDotOffset >= 0 ? rFilename.copy(nDotOffset + 1) : rFilename); // Skip the dot.

    const std::vector<OUString> aNames = { aExt, "/" + rFilename };
    for (const OUString& aName : aNames)
    {
        const auto it1 = std::find_if(rOverrides.begin(), rOverrides.end(), [&aName](const beans::StringPair& rPair)
                                                                              { return rPair.First == aName; });
        if (it1 != rOverrides.end())
            return it1->Second;

        const auto it2 = std::find_if(rDefaults.begin(), rDefaults.end(), [&aName](const beans::StringPair& rPair)
                                                                            { return rPair.First == aName; });
        if (it2 != rDefaults.end())
            return it2->Second;
    }

    return OUString();
}

void WriteRelationsInfoSequence(
        const uno::Reference< io::XOutputStream >& xOutStream,
        const uno::Sequence< uno::Sequence< beans::StringPair > >& aSequence,
        const uno::Reference< uno::XComponentContext >& rContext )
{
    if ( !xOutStream.is() )
        throw uno::RuntimeException();

    uno::Reference< css::xml::sax::XWriter > xWriter = css::xml::sax::Writer::create(rContext);

    xWriter->setOutputStream( xOutStream );

    OUString aRelListElement( u"Relationships"_ustr );
    OUString aRelElement( u"Relationship"_ustr );
    OUString aWhiteSpace( u" "_ustr );

    // write the namespace
    rtl::Reference<AttributeList> pRootAttrList = new AttributeList;
    pRootAttrList->AddAttribute(
        u"xmlns"_ustr,
        u"http://schemas.openxmlformats.org/package/2006/relationships"_ustr );

    xWriter->startDocument();
    xWriter->startElement( aRelListElement, pRootAttrList );

    for ( const auto & i : aSequence )
    {
        rtl::Reference<AttributeList> pAttrList = new AttributeList;
        for( const beans::StringPair & pair : i )
        {
            if ( !(pair.First == "Id"
                  || pair.First == "Type"
                  || pair.First == "TargetMode"
                  || pair.First == "Target") )
            {
                // TODO/LATER: should the extensions be allowed?
                throw lang::IllegalArgumentException();
            }
            pAttrList->AddAttribute( pair.First, pair.Second );
        }

        xWriter->startElement( aRelElement, pAttrList );
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
{
    if ( !xOutStream.is() )
        throw uno::RuntimeException();

    uno::Reference< css::xml::sax::XWriter > xWriter = css::xml::sax::Writer::create(rContext);

    xWriter->setOutputStream( xOutStream );

    static constexpr OUString aTypesElement(u"Types"_ustr);
    static constexpr OUString aDefaultElement(u"Default"_ustr);
    static constexpr OUString aOverrideElement(u"Override"_ustr);
    static constexpr OUString aContentTypeAttr(u"ContentType"_ustr);
    static constexpr OUString aWhiteSpace(u" "_ustr);

    // write the namespace
    rtl::Reference<AttributeList> pRootAttrList = new AttributeList;
    pRootAttrList->AddAttribute(
        u"xmlns"_ustr,
        u"http://schemas.openxmlformats.org/package/2006/content-types"_ustr );

    xWriter->startDocument();
    xWriter->startElement( aTypesElement, pRootAttrList );

    for ( const beans::StringPair & pair : aDefaultsSequence )
    {
        rtl::Reference<AttributeList> pAttrList = new AttributeList;
        pAttrList->AddAttribute( u"Extension"_ustr, pair.First );
        pAttrList->AddAttribute( aContentTypeAttr, pair.Second );

        xWriter->startElement( aDefaultElement, pAttrList  );
        xWriter->ignorableWhitespace( aWhiteSpace );
        xWriter->endElement( aDefaultElement );
    }

    for ( const beans::StringPair & pair : aOverridesSequence )
    {
        rtl::Reference<AttributeList> pAttrList = new AttributeList;
        pAttrList->AddAttribute( u"PartName"_ustr, pair.First );
        pAttrList->AddAttribute( aContentTypeAttr, pair.Second );

        xWriter->startElement( aOverrideElement, pAttrList );
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
{
    if ( !rContext.is() || !xInStream.is() || nFormat > FORMAT_MAX_ID )
        throw uno::RuntimeException();

    uno::Reference< css::xml::sax::XParser > xParser = css::xml::sax::Parser::create( rContext );

    rtl::Reference<OFOPXMLHelper_Impl> pHelper = new OFOPXMLHelper_Impl( nFormat );
    css::xml::sax::InputSource aParserInput;
    aParserInput.aInputStream = xInStream;
    aParserInput.sSystemId = aStringID;
    xParser->setDocumentHandler( pHelper );
    xParser->parseStream( aParserInput );
    xParser->setDocumentHandler( uno::Reference < css::xml::sax::XDocumentHandler > () );

    return pHelper->GetParsingResult();
}

} // namespace OFOPXMLHelper

// Relations info related strings
constexpr OUStringLiteral g_aRelListElement(u"Relationships");
constexpr OUStringLiteral g_aRelElement( u"Relationship" );
constexpr OUString g_aIDAttr( u"Id"_ustr );
constexpr OUString g_aTypeAttr( u"Type"_ustr );
constexpr OUString g_aTargetModeAttr( u"TargetMode"_ustr );
constexpr OUString g_aTargetAttr( u"Target"_ustr );

// ContentType related strings
constexpr OUStringLiteral g_aTypesElement( u"Types" );
constexpr OUStringLiteral g_aDefaultElement( u"Default" );
constexpr OUStringLiteral g_aOverrideElement( u"Override" );
constexpr OUStringLiteral g_aExtensionAttr( u"Extension" );
constexpr OUStringLiteral g_aPartNameAttr( u"PartName" );
constexpr OUString g_aContentTypeAttr( u"ContentType"_ustr );

OFOPXMLHelper_Impl::OFOPXMLHelper_Impl( sal_uInt16 nFormat )
: m_nFormat( nFormat )
{
}

uno::Sequence< uno::Sequence< beans::StringPair > > const & OFOPXMLHelper_Impl::GetParsingResult() const
{
    if ( !m_aElementsSeq.empty() )
        throw uno::RuntimeException(); // the parsing has still not finished!

    return m_aResultSeq;
}


void SAL_CALL OFOPXMLHelper_Impl::startDocument()
{
}


void SAL_CALL OFOPXMLHelper_Impl::endDocument()
{
}


void SAL_CALL OFOPXMLHelper_Impl::startElement( const OUString& aName, const uno::Reference< css::xml::sax::XAttributeList >& xAttribs )
{
    if ( m_nFormat == RELATIONINFO_FORMAT )
    {
        if ( aName == g_aRelListElement )
        {
            sal_Int32 nNewLength = m_aElementsSeq.size() + 1;

            if ( nNewLength != 1 )
                throw css::xml::sax::SAXException(); // TODO: this element must be the first level element

            m_aElementsSeq.push_back( aName );

            return; // nothing to do
        }
        else if ( aName == g_aRelElement )
        {
            sal_Int32 nNewLength = m_aElementsSeq.size() + 1;
            if ( nNewLength != 2 )
                throw css::xml::sax::SAXException(); // TODO: this element must be the second level element

            m_aElementsSeq.push_back( aName );

            sal_Int32 nNewEntryNum = m_aResultSeq.getLength() + 1;
            m_aResultSeq.realloc( nNewEntryNum );
            auto pResultSeq = m_aResultSeq.getArray();
            sal_Int32 nAttrNum = 0;
            pResultSeq[nNewEntryNum-1].realloc( 4 ); // the maximal expected number of arguments is 4
            auto pAttrs = pResultSeq[nNewEntryNum-1].getArray();

            OUString aIDValue = xAttribs->getValueByName( g_aIDAttr );
            if ( aIDValue.isEmpty() )
                throw css::xml::sax::SAXException(); // TODO: the ID value must present

            OUString aTypeValue = xAttribs->getValueByName( g_aTypeAttr );
            OUString aTargetValue = xAttribs->getValueByName( g_aTargetAttr );
            OUString aTargetModeValue = xAttribs->getValueByName( g_aTargetModeAttr );

            pAttrs[++nAttrNum - 1].First = g_aIDAttr;
            pAttrs[nAttrNum - 1].Second = aIDValue;

            if ( !aTypeValue.isEmpty() )
            {
                pAttrs[++nAttrNum - 1].First = g_aTypeAttr;
                pAttrs[nAttrNum - 1].Second = aTypeValue;
            }

            if ( !aTargetValue.isEmpty() )
            {
                pAttrs[++nAttrNum - 1].First = g_aTargetAttr;
                pAttrs[nAttrNum - 1].Second = aTargetValue;
            }

            if ( !aTargetModeValue.isEmpty() )
            {
                pAttrs[++nAttrNum - 1].First = g_aTargetModeAttr;
                pAttrs[nAttrNum - 1].Second = aTargetModeValue;
            }

            pResultSeq[nNewEntryNum-1].realloc( nAttrNum );
        }
        else
            throw css::xml::sax::SAXException(); // TODO: no other elements expected!
    }
    else if ( m_nFormat == CONTENTTYPE_FORMAT )
    {
        if ( aName == g_aTypesElement )
        {
            sal_Int32 nNewLength = m_aElementsSeq.size() + 1;

            if ( nNewLength != 1 )
                throw css::xml::sax::SAXException(); // TODO: this element must be the first level element

            m_aElementsSeq.push_back( aName );

            if ( !m_aResultSeq.hasElements() )
                m_aResultSeq.realloc( 2 );

            return; // nothing to do
        }
        else if ( aName == g_aDefaultElement )
        {
            sal_Int32 nNewLength = m_aElementsSeq.size() + 1;
            if ( nNewLength != 2 )
                throw css::xml::sax::SAXException(); // TODO: this element must be the second level element

            m_aElementsSeq.push_back( aName );

            if ( !m_aResultSeq.hasElements() )
                m_aResultSeq.realloc( 2 );

            if ( m_aResultSeq.getLength() != 2 )
                throw uno::RuntimeException();

            auto pResultSeq = m_aResultSeq.getArray();

            const OUString aExtensionValue = xAttribs->getValueByName( g_aExtensionAttr );
            if ( aExtensionValue.isEmpty() )
                throw css::xml::sax::SAXException(); // TODO: the Extension value must present

            const OUString aContentTypeValue = xAttribs->getValueByName( g_aContentTypeAttr );
            if ( aContentTypeValue.isEmpty() )
                throw css::xml::sax::SAXException(); // TODO: the ContentType value must present

            const sal_Int32 nNewResultLen = m_aResultSeq[0].getLength() + 1;
            pResultSeq[0].realloc( nNewResultLen );
            auto pSeq = pResultSeq[0].getArray();

            pSeq[nNewResultLen-1].First = aExtensionValue;
            pSeq[nNewResultLen-1].Second = aContentTypeValue;
        }
        else if ( aName == g_aOverrideElement )
        {
            sal_Int32 nNewLength = m_aElementsSeq.size() + 1;
            if ( nNewLength != 2 )
                throw css::xml::sax::SAXException(); // TODO: this element must be the second level element

            m_aElementsSeq.push_back( aName );

            if ( !m_aResultSeq.hasElements() )
                m_aResultSeq.realloc( 2 );

            if ( m_aResultSeq.getLength() != 2 )
                throw uno::RuntimeException();

            auto pResultSeq = m_aResultSeq.getArray();

            OUString aPartNameValue = xAttribs->getValueByName( g_aPartNameAttr );
            if ( aPartNameValue.isEmpty() )
                throw css::xml::sax::SAXException(); // TODO: the PartName value must present

            OUString aContentTypeValue = xAttribs->getValueByName( g_aContentTypeAttr );
            if ( aContentTypeValue.isEmpty() )
                throw css::xml::sax::SAXException(); // TODO: the ContentType value must present

            sal_Int32 nNewResultLen = m_aResultSeq[1].getLength() + 1;
            pResultSeq[1].realloc( nNewResultLen );
            auto pSeq = pResultSeq[1].getArray();

            pSeq[nNewResultLen-1].First = aPartNameValue;
            pSeq[nNewResultLen-1].Second = aContentTypeValue;
        }
        else
            throw css::xml::sax::SAXException(); // TODO: no other elements expected!
    }
    else
        throw css::xml::sax::SAXException(); // TODO: no other elements expected!
}


void SAL_CALL OFOPXMLHelper_Impl::endElement( const OUString& aName )
{
    if ( m_nFormat == RELATIONINFO_FORMAT || m_nFormat == CONTENTTYPE_FORMAT )
    {
        sal_Int32 nLength = m_aElementsSeq.size();
        if ( nLength <= 0 )
            throw css::xml::sax::SAXException(); // TODO: no other end elements expected!

        if ( m_aElementsSeq[nLength-1] != aName )
            throw css::xml::sax::SAXException(); // TODO: unexpected element ended

        m_aElementsSeq.resize( nLength - 1 );
    }
}


void SAL_CALL OFOPXMLHelper_Impl::characters( const OUString& /*aChars*/ )
{
}


void SAL_CALL OFOPXMLHelper_Impl::ignorableWhitespace( const OUString& /*aWhitespaces*/ )
{
}


void SAL_CALL OFOPXMLHelper_Impl::processingInstruction( const OUString& /*aTarget*/, const OUString& /*aData*/ )
{
}


void SAL_CALL OFOPXMLHelper_Impl::setDocumentLocator( const uno::Reference< css::xml::sax::XLocator >& /*xLocator*/ )
{
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
