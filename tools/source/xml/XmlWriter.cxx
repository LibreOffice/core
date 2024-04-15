/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/stream.hxx>
#include <tools/XmlWriter.hxx>

#include <libxml/xmlwriter.h>

namespace tools
{
namespace
{
int funcWriteCallback(void* pContext, const char* sBuffer, int nLen)
{
    SvStream* pStream = static_cast<SvStream*>(pContext);
    return static_cast<int>(pStream->WriteBytes(sBuffer, nLen));
}

int funcCloseCallback(void* pContext)
{
    SvStream* pStream = static_cast<SvStream*>(pContext);
    pStream->Flush();
    return 0; // 0 or -1 in case of error
}

} // end anonymous namespace

struct XmlWriterImpl
{
    XmlWriterImpl(SvStream* pStream)
        : mpStream(pStream)
        , mpWriter(nullptr)
        , mbWriteXmlHeader(true)
    {
    }

    SvStream* mpStream;
    xmlTextWriterPtr mpWriter;
    bool mbWriteXmlHeader;
};

XmlWriter::XmlWriter(SvStream* pStream)
    : mpImpl(std::make_unique<XmlWriterImpl>(pStream))
{
}

XmlWriter::~XmlWriter()
{
    if (mpImpl && mpImpl->mpWriter != nullptr)
        endDocument();
}

bool XmlWriter::startDocument(sal_Int32 nIndent, bool bWriteXmlHeader)
{
    mpImpl->mbWriteXmlHeader = bWriteXmlHeader;
    xmlCharEncodingHandlerPtr pEncodingHandler = xmlGetCharEncodingHandler(XML_CHAR_ENCODING_UTF8);
    xmlOutputBufferPtr xmlOutBuffer = xmlOutputBufferCreateIO(funcWriteCallback, funcCloseCallback,
                                                              mpImpl->mpStream, pEncodingHandler);
    mpImpl->mpWriter = xmlNewTextWriter(xmlOutBuffer);
    if (mpImpl->mpWriter == nullptr)
        return false;
    xmlTextWriterSetIndent(mpImpl->mpWriter, nIndent);
    if (mpImpl->mbWriteXmlHeader)
        (void)xmlTextWriterStartDocument(mpImpl->mpWriter, nullptr, "UTF-8", nullptr);
    return true;
}

void XmlWriter::endDocument()
{
    if (mpImpl->mbWriteXmlHeader)
        (void)xmlTextWriterEndDocument(mpImpl->mpWriter);
    xmlFreeTextWriter(mpImpl->mpWriter);
    mpImpl->mpWriter = nullptr;
}

void XmlWriter::startElement(const OString& sPrefix, const OString& sName,
                             const OString& sNamespaceUri)
{
    xmlChar* xmlName = BAD_CAST(sName.getStr());
    xmlChar* xmlPrefix = nullptr;
    xmlChar* xmlNamespaceUri = nullptr;
    if (!sPrefix.isEmpty())
        xmlPrefix = BAD_CAST(sPrefix.getStr());
    if (!sNamespaceUri.isEmpty())
        xmlNamespaceUri = BAD_CAST(sNamespaceUri.getStr());

    (void)xmlTextWriterStartElementNS(mpImpl->mpWriter, xmlPrefix, xmlName, xmlNamespaceUri);
}

void XmlWriter::startElement(const char* pName)
{
    xmlChar* xmlName = BAD_CAST(pName);
    (void)xmlTextWriterStartElement(mpImpl->mpWriter, xmlName);
}

void XmlWriter::endElement() { (void)xmlTextWriterEndElement(mpImpl->mpWriter); }

void XmlWriter::attributeBase64(const char* pName, std::vector<sal_uInt8> const& rValueInBytes)
{
    std::vector<char> aSignedBytes(rValueInBytes.begin(), rValueInBytes.end());
    attributeBase64(pName, aSignedBytes);
}

void XmlWriter::attributeBase64(const char* pName, std::vector<char> const& rValueInBytes)
{
    xmlChar* xmlName = BAD_CAST(pName);
    (void)xmlTextWriterStartAttribute(mpImpl->mpWriter, xmlName);
    (void)xmlTextWriterWriteBase64(mpImpl->mpWriter, rValueInBytes.data(), 0, rValueInBytes.size());
    (void)xmlTextWriterEndAttribute(mpImpl->mpWriter);
}

void XmlWriter::attribute(const char* name, const OString& value)
{
    xmlChar* xmlName = BAD_CAST(name);
    xmlChar* xmlValue = BAD_CAST(value.getStr());
    (void)xmlTextWriterWriteAttribute(mpImpl->mpWriter, xmlName, xmlValue);
}

void XmlWriter::attribute(const char* name, std::u16string_view value)
{
    attribute(name, OUStringToOString(value, RTL_TEXTENCODING_UTF8));
}

void XmlWriter::attribute(const char* name, const sal_Int64 aNumber)
{
    attribute(name, OString::number(aNumber));
}

void XmlWriter::attributeDouble(const char* name, const double aNumber)
{
    attribute(name, OString::number(aNumber));
}

void XmlWriter::content(const OString& sValue)
{
    xmlChar* xmlValue = BAD_CAST(sValue.getStr());
    (void)xmlTextWriterWriteString(mpImpl->mpWriter, xmlValue);
}

void XmlWriter::content(std::u16string_view sValue)
{
    content(OUStringToOString(sValue, RTL_TEXTENCODING_UTF8));
}

void XmlWriter::element(const char* sName)
{
    startElement(sName);
    endElement();
}

} // end tools namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
