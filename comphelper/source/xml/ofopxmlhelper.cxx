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

#include <rtl/ref.hxx>
#include <sal/log.hxx>

#include <com/sun/star/xml/sax/FastParser.hpp>
#include <com/sun/star/xml/sax/XFastParser.hpp>
#include <com/sun/star/xml/sax/XFastDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XFastContextHandler.hpp>
#include <com/sun/star/xml/sax/XFastAttributeList.hpp>

#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <vector>

using namespace ::com::sun::star;

namespace comphelper
{
namespace
{
enum class Format
{
    RelationInfo,
    ContentType
};

// this helper class is designed to allow to parse ContentType- and Relationship-related information from OfficeOpenXML format
class OFOPXMLHelper_Impl : public cppu::WeakImplHelper<css::xml::sax::XFastDocumentHandler>
{
    Format const m_eFormat; // which format to parse

    css::uno::Sequence<css::uno::Sequence<css::beans::StringPair>> m_aResultSeq;
    std::vector<OUString> m_aElementsSeq; // stack of elements being parsed

public:
    css::uno::Sequence<css::uno::Sequence<css::beans::StringPair>> const& GetParsingResult() const;

    explicit OFOPXMLHelper_Impl(Format eFormat); // must not be created directly

    // XFastDocumentHandler
    virtual void SAL_CALL startDocument() override;
    virtual void SAL_CALL endDocument() override;
    virtual void SAL_CALL processingInstruction(const OUString& aTarget,
                                                const OUString& aData) override;
    virtual void SAL_CALL
    setDocumentLocator(const uno::Reference<css::xml::sax::XLocator>& xLocator) override;

    // XFastContextHandler
    virtual uno::Reference<css::xml::sax::XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 Element,
        const uno::Reference<css::xml::sax::XFastAttributeList>& Attribs) override;

    virtual uno::Reference<css::xml::sax::XFastContextHandler> SAL_CALL createUnknownChildContext(
        const OUString& Namespace, const OUString& Name,
        const uno::Reference<css::xml::sax::XFastAttributeList>& Attribs) override;

    virtual void SAL_CALL
    startFastElement(sal_Int32 Element,
                     const uno::Reference<css::xml::sax::XFastAttributeList>& Attribs) override;

    virtual void SAL_CALL
    startUnknownElement(const OUString& Namespace, const OUString& Name,
                        const uno::Reference<css::xml::sax::XFastAttributeList>& Attribs) override;

    virtual void SAL_CALL endFastElement(sal_Int32 Element) override;

    virtual void SAL_CALL endUnknownElement(const OUString& Namespace,
                                            const OUString& Name) override;

    virtual void SAL_CALL characters(const OUString& aChars) override;
};
}

namespace OFOPXMLHelper
{
/// @throws css::uno::Exception
static uno::Sequence<uno::Sequence<beans::StringPair>>
ReadSequence_Impl(const uno::Reference<io::XInputStream>& xInStream, const OUString& aStringID,
                  Format eFormat, const uno::Reference<uno::XComponentContext>& xContext);

uno::Sequence<uno::Sequence<beans::StringPair>>
ReadRelationsInfoSequence(const uno::Reference<io::XInputStream>& xInStream,
                          std::u16string_view aStreamName,
                          const uno::Reference<uno::XComponentContext>& rContext)
{
    OUString aStringID = OUString::Concat("_rels/") + aStreamName;
    return ReadSequence_Impl(xInStream, aStringID, Format::RelationInfo, rContext);
}

uno::Sequence<uno::Sequence<beans::StringPair>>
ReadContentTypeSequence(const uno::Reference<io::XInputStream>& xInStream,
                        const uno::Reference<uno::XComponentContext>& rContext)
{
    return ReadSequence_Impl(xInStream, u"[Content_Types].xml"_ustr, Format::ContentType, rContext);
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

    // An Override names one part, and a Default covers the parts with that extension that have no
    // Override. Matching part names and extensions is done ignoring case.
    const OUString aPartName = rFilename.startsWith("/") ? rFilename : "/" + rFilename;
    const auto itOverride = std::find_if(rOverrides.begin(), rOverrides.end(),
                                         [&aPartName](const beans::StringPair& rPair) {
                                             return rPair.First.equalsIgnoreAsciiCase(aPartName);
                                         });
    if (itOverride != rOverrides.end())
        return itOverride->Second;

    // The extension is what comes after its last dot
    const sal_Int32 nDotOffset = aPartName.lastIndexOf('.');
    if (nDotOffset < aPartName.lastIndexOf('/'))
        return OUString();
    const OUString aExtension = aPartName.copy(nDotOffset + 1);
    const auto itDefault = std::find_if(rDefaults.begin(), rDefaults.end(),
                                        [&aExtension](const beans::StringPair& rPair) {
                                            return rPair.First.equalsIgnoreAsciiCase(aExtension);
                                        });
    if (itDefault != rDefaults.end())
        return itDefault->Second;

    return OUString();
}

void WriteRelationsInfoSequence(const uno::Reference<io::XOutputStream>& xOutStream,
                                const uno::Sequence<uno::Sequence<beans::StringPair>>& aSequence,
                                const uno::Reference<uno::XComponentContext>& rContext)
{
    if (!xOutStream.is())
        throw uno::RuntimeException(u"Invalid output stream"_ustr);

    uno::Reference<css::xml::sax::XWriter> xWriter = css::xml::sax::Writer::create(rContext);

    xWriter->setOutputStream(xOutStream);

    OUString aRelListElement(u"Relationships"_ustr);
    OUString aRelElement(u"Relationship"_ustr);
    OUString aWhiteSpace(u" "_ustr);

    // write the namespace
    rtl::Reference<AttributeList> pRootAttrList = new AttributeList;
    pRootAttrList->AddAttribute(
        u"xmlns"_ustr, u"http://schemas.openxmlformats.org/package/2006/relationships"_ustr);

    xWriter->startDocument();
    xWriter->startElement(aRelListElement, pRootAttrList);

    for (const auto& i : aSequence)
    {
        rtl::Reference<AttributeList> pAttrList = new AttributeList;
        for (const beans::StringPair& pair : i)
        {
            if (!(pair.First == "Id" || pair.First == "Type" || pair.First == "TargetMode"
                  || pair.First == "Target"))
            {
                // TODO/LATER: should the extensions be allowed?
                throw lang::IllegalArgumentException();
            }
            pAttrList->AddAttribute(pair.First, pair.Second);
        }

        xWriter->startElement(aRelElement, pAttrList);
        xWriter->ignorableWhitespace(aWhiteSpace);
        xWriter->endElement(aRelElement);
    }

    xWriter->ignorableWhitespace(aWhiteSpace);
    xWriter->endElement(aRelListElement);
    xWriter->endDocument();
}

void WriteContentSequence(const uno::Reference<io::XOutputStream>& xOutStream,
                          const uno::Sequence<beans::StringPair>& aDefaultsSequence,
                          const uno::Sequence<beans::StringPair>& aOverridesSequence,
                          const uno::Reference<uno::XComponentContext>& rContext)
{
    if (!xOutStream.is())
        throw uno::RuntimeException(u"Invalid output stream"_ustr);

    uno::Reference<css::xml::sax::XWriter> xWriter = css::xml::sax::Writer::create(rContext);

    xWriter->setOutputStream(xOutStream);

    static constexpr OUString aTypesElement(u"Types"_ustr);
    static constexpr OUString aDefaultElement(u"Default"_ustr);
    static constexpr OUString aOverrideElement(u"Override"_ustr);
    static constexpr OUString aContentTypeAttr(u"ContentType"_ustr);
    static constexpr OUString aWhiteSpace(u" "_ustr);

    // write the namespace
    rtl::Reference<AttributeList> pRootAttrList = new AttributeList;
    pRootAttrList->AddAttribute(
        u"xmlns"_ustr, u"http://schemas.openxmlformats.org/package/2006/content-types"_ustr);

    xWriter->startDocument();
    xWriter->startElement(aTypesElement, pRootAttrList);

    for (const beans::StringPair& pair : aDefaultsSequence)
    {
        rtl::Reference<AttributeList> pAttrList = new AttributeList;
        pAttrList->AddAttribute(u"Extension"_ustr, pair.First);
        pAttrList->AddAttribute(aContentTypeAttr, pair.Second);

        xWriter->startElement(aDefaultElement, pAttrList);
        xWriter->ignorableWhitespace(aWhiteSpace);
        xWriter->endElement(aDefaultElement);
    }

    for (const beans::StringPair& pair : aOverridesSequence)
    {
        rtl::Reference<AttributeList> pAttrList = new AttributeList;
        pAttrList->AddAttribute(u"PartName"_ustr, pair.First);
        pAttrList->AddAttribute(aContentTypeAttr, pair.Second);

        xWriter->startElement(aOverrideElement, pAttrList);
        xWriter->ignorableWhitespace(aWhiteSpace);
        xWriter->endElement(aOverrideElement);
    }

    xWriter->ignorableWhitespace(aWhiteSpace);
    xWriter->endElement(aTypesElement);
    xWriter->endDocument();
}

uno::Sequence<uno::Sequence<beans::StringPair>>
ReadSequence_Impl(const uno::Reference<io::XInputStream>& xInStream, const OUString& aStringID,
                  Format eFormat, const uno::Reference<uno::XComponentContext>& rContext)
{
    if (!rContext.is() || !xInStream.is())
        throw uno::RuntimeException(u"Invalid input stream or context"_ustr);

    uno::Reference<css::xml::sax::XFastParser> xParser
        = css::xml::sax::FastParser::create(rContext);

    rtl::Reference<OFOPXMLHelper_Impl> pHelper = new OFOPXMLHelper_Impl(eFormat);
    css::xml::sax::InputSource aParserInput;
    aParserInput.aInputStream = xInStream;
    aParserInput.sSystemId = aStringID;
    xParser->setFastDocumentHandler(pHelper);
    xParser->parseStream(aParserInput);
    xParser->setFastDocumentHandler(uno::Reference<css::xml::sax::XFastDocumentHandler>());

    return pHelper->GetParsingResult();
}

} // namespace OFOPXMLHelper

// Relations info related strings

constexpr OUString g_aIDAttr(u"Id"_ustr);
constexpr OUString g_aTypeAttr(u"Type"_ustr);
constexpr OUString g_aTargetModeAttr(u"TargetMode"_ustr);
constexpr OUString g_aTargetAttr(u"Target"_ustr);

// ContentType related strings
constexpr OUString g_aContentTypeAttr(u"ContentType"_ustr);

OFOPXMLHelper_Impl::OFOPXMLHelper_Impl(Format eFormat)
    : m_eFormat(eFormat)
{
}

uno::Sequence<uno::Sequence<beans::StringPair>> const& OFOPXMLHelper_Impl::GetParsingResult() const
{
    if (!m_aElementsSeq.empty())
        throw uno::RuntimeException(); // the parsing has still not finished!

    return m_aResultSeq;
}

void SAL_CALL OFOPXMLHelper_Impl::startDocument() {}

void SAL_CALL OFOPXMLHelper_Impl::endDocument() {}

static OUString getAttributeValue(const uno::Reference<css::xml::sax::XFastAttributeList>& xAttribs,
                                  std::u16string_view rName)
{
    for (const auto& rAttr : xAttribs->getUnknownAttributes())
    {
        if (rAttr.Name == rName)
            return rAttr.Value;
    }

    return {};
}

void SAL_CALL OFOPXMLHelper_Impl::startUnknownElement(
    const OUString& rNamespace, const OUString& rName,
    const uno::Reference<css::xml::sax::XFastAttributeList>& xAttribs)
{
    SAL_INFO("comphelper", "startUnknownElement: namespace=" << rNamespace << ", name=" << rName);

    const OUString rExpectedNamespace
        = m_eFormat == Format::RelationInfo
              ? u"http://schemas.openxmlformats.org/package/2006/relationships"_ustr
              : u"http://schemas.openxmlformats.org/package/2006/content-types"_ustr;

    if (rNamespace != rExpectedNamespace)
        throw css::xml::sax::SAXException();

    const sal_Int32 nColonPos = rName.lastIndexOf(':');
    const OUString aLocalName = nColonPos >= 0 ? rName.copy(nColonPos + 1) : rName;

    if (m_eFormat == Format::RelationInfo)
    {
        if (aLocalName == u"Relationships"_ustr)
        {
            sal_Int32 nNewLength = m_aElementsSeq.size() + 1;

            if (nNewLength != 1)
                throw css::xml::sax::
                    SAXException(); // TODO: this element must be the first level element

            m_aElementsSeq.push_back(aLocalName);

            return; // nothing to do
        }
        else if (aLocalName == u"Relationship"_ustr)
        {
            sal_Int32 nNewLength = m_aElementsSeq.size() + 1;
            if (nNewLength != 2)
                throw css::xml::sax::
                    SAXException(); // TODO: this element must be the second level element

            m_aElementsSeq.push_back(aLocalName);

            sal_Int32 nNewEntryNum = m_aResultSeq.getLength() + 1;
            m_aResultSeq.realloc(nNewEntryNum);
            auto pResultSeq = m_aResultSeq.getArray();
            sal_Int32 nAttrNum = 0;
            pResultSeq[nNewEntryNum - 1].realloc(
                4); // the maximal expected number of arguments is 4
            auto pAttrs = pResultSeq[nNewEntryNum - 1].getArray();

            OUString aIDValue = getAttributeValue(xAttribs, g_aIDAttr);
            if (aIDValue.isEmpty())
                throw css::xml::sax::SAXException(); // TODO: the ID value must present

            OUString aTypeValue = getAttributeValue(xAttribs, g_aTypeAttr);
            OUString aTargetValue = getAttributeValue(xAttribs, g_aTargetAttr);
            OUString aTargetModeValue = getAttributeValue(xAttribs, g_aTargetModeAttr);

            pAttrs[++nAttrNum - 1].First = g_aIDAttr;
            pAttrs[nAttrNum - 1].Second = aIDValue;

            if (!aTypeValue.isEmpty())
            {
                pAttrs[++nAttrNum - 1].First = g_aTypeAttr;
                pAttrs[nAttrNum - 1].Second = aTypeValue;
            }

            if (!aTargetValue.isEmpty())
            {
                pAttrs[++nAttrNum - 1].First = g_aTargetAttr;
                pAttrs[nAttrNum - 1].Second = aTargetValue;
            }

            if (!aTargetModeValue.isEmpty())
            {
                pAttrs[++nAttrNum - 1].First = g_aTargetModeAttr;
                pAttrs[nAttrNum - 1].Second = aTargetModeValue;
            }

            pResultSeq[nNewEntryNum - 1].realloc(nAttrNum);
        }
        else
            throw css::xml::sax::SAXException(); // TODO: no other elements expected!
    }
    else if (m_eFormat == Format::ContentType)
    {
        if (aLocalName == u"Types"_ustr)
        {
            sal_Int32 nNewLength = m_aElementsSeq.size() + 1;

            if (nNewLength != 1)
                throw css::xml::sax::
                    SAXException(); // TODO: this element must be the first level element

            m_aElementsSeq.push_back(aLocalName);

            if (!m_aResultSeq.hasElements())
                m_aResultSeq.realloc(2);

            return; // nothing to do
        }
        else if (aLocalName == u"Default"_ustr)
        {
            sal_Int32 nNewLength = m_aElementsSeq.size() + 1;
            if (nNewLength != 2)
                throw css::xml::sax::
                    SAXException(); // TODO: this element must be the second level element

            m_aElementsSeq.push_back(aLocalName);

            if (!m_aResultSeq.hasElements())
                m_aResultSeq.realloc(2);

            if (m_aResultSeq.getLength() != 2)
                throw uno::RuntimeException(
                    u"m_aResultSeq already has elements and is not reallocated to 2."_ustr);

            auto pResultSeq = m_aResultSeq.getArray();

            const OUString aExtensionValue = getAttributeValue(xAttribs, u"Extension");
            if (aExtensionValue.isEmpty())
                throw css::xml::sax::SAXException(); // TODO: the Extension value must present

            const OUString aContentTypeValue = getAttributeValue(xAttribs, g_aContentTypeAttr);
            if (aContentTypeValue.isEmpty())
                throw css::xml::sax::SAXException(); // TODO: the ContentType value must present

            const sal_Int32 nNewResultLen = m_aResultSeq[0].getLength() + 1;
            pResultSeq[0].realloc(nNewResultLen);
            auto pSeq = pResultSeq[0].getArray();

            pSeq[nNewResultLen - 1].First = aExtensionValue;
            pSeq[nNewResultLen - 1].Second = aContentTypeValue;
        }
        else if (aLocalName == u"Override"_ustr)
        {
            sal_Int32 nNewLength = m_aElementsSeq.size() + 1;
            if (nNewLength != 2)
                throw css::xml::sax::
                    SAXException(); // TODO: this element must be the second level element

            m_aElementsSeq.push_back(aLocalName);

            if (!m_aResultSeq.hasElements())
                m_aResultSeq.realloc(2);

            if (m_aResultSeq.getLength() != 2)
                throw uno::RuntimeException(
                    u"m_aResultSeq already has elements and is not reallocated to 2."_ustr);

            auto pResultSeq = m_aResultSeq.getArray();

            OUString aPartNameValue = getAttributeValue(xAttribs, u"PartName");
            if (aPartNameValue.isEmpty())
                throw css::xml::sax::SAXException(); // TODO: the PartName value must present

            OUString aContentTypeValue = getAttributeValue(xAttribs, g_aContentTypeAttr);
            if (aContentTypeValue.isEmpty())
                throw css::xml::sax::SAXException(); // TODO: the ContentType value must present

            sal_Int32 nNewResultLen = m_aResultSeq[1].getLength() + 1;
            pResultSeq[1].realloc(nNewResultLen);
            auto pSeq = pResultSeq[1].getArray();

            pSeq[nNewResultLen - 1].First = aPartNameValue;
            pSeq[nNewResultLen - 1].Second = aContentTypeValue;
        }
        else
            throw css::xml::sax::SAXException(); // TODO: no other elements expected!
    }
    else
        throw css::xml::sax::SAXException(); // TODO: no other elements expected!
}

void SAL_CALL OFOPXMLHelper_Impl::endUnknownElement(const OUString& rNamespace,
                                                    const OUString& rName)
{
    const OUString rExpectedNamespace
        = m_eFormat == Format::RelationInfo
              ? u"http://schemas.openxmlformats.org/package/2006/relationships"_ustr
              : u"http://schemas.openxmlformats.org/package/2006/content-types"_ustr;

    if (rNamespace != rExpectedNamespace)
        throw css::xml::sax::SAXException();

    // Remove the namespace prefix, if present.
    const sal_Int32 nColonPos = rName.lastIndexOf(':');
    const OUString aLocalName = nColonPos >= 0 ? rName.copy(nColonPos + 1) : rName;

    sal_Int32 nLength = m_aElementsSeq.size();

    if (nLength <= 0)
        throw css::xml::sax::SAXException();

    if (m_aElementsSeq[nLength - 1] != aLocalName)
        throw css::xml::sax::SAXException();

    SAL_INFO("comphelper", "endUnknownElement: localName=" << aLocalName << ", stackName="
                                                           << m_aElementsSeq[nLength - 1]);

    m_aElementsSeq.resize(nLength - 1);
}

void SAL_CALL OFOPXMLHelper_Impl::characters(const OUString& /*aChars*/) {}

void SAL_CALL OFOPXMLHelper_Impl::processingInstruction(const OUString& /*aTarget*/,
                                                        const OUString& /*aData*/)
{
}

void SAL_CALL
OFOPXMLHelper_Impl::setDocumentLocator(const uno::Reference<css::xml::sax::XLocator>& /*xLocator*/)
{
}

uno::Reference<css::xml::sax::XFastContextHandler> SAL_CALL
OFOPXMLHelper_Impl::createFastChildContext(sal_Int32,
                                           const uno::Reference<css::xml::sax::XFastAttributeList>&)
{
    return this;
}

uno::Reference<css::xml::sax::XFastContextHandler>
    SAL_CALL OFOPXMLHelper_Impl::createUnknownChildContext(
        const OUString&, const OUString&, const uno::Reference<css::xml::sax::XFastAttributeList>&)
{
    return this;
}

void SAL_CALL OFOPXMLHelper_Impl::startFastElement(
    sal_Int32, const uno::Reference<css::xml::sax::XFastAttributeList>&)
{
}

void SAL_CALL OFOPXMLHelper_Impl::endFastElement(sal_Int32) {}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
