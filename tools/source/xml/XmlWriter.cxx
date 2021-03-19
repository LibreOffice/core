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
    xmlOutputBufferPtr xmlOutBuffer
        = xmlOutputBufferCreateIO(funcWriteCallback, funcCloseCallback, mpImpl->mpStream, nullptr);
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
    xmlChar* xmlName = xmlCharStrdup(sName.getStr());
    xmlChar* xmlPrefix = nullptr;
    xmlChar* xmlNamespaceUri = nullptr;
    if (!sPrefix.isEmpty())
        xmlPrefix = xmlCharStrdup(sPrefix.getStr());
    if (!sNamespaceUri.isEmpty())
        xmlNamespaceUri = xmlCharStrdup(sNamespaceUri.getStr());

    (void)xmlTextWriterStartElementNS(mpImpl->mpWriter, xmlPrefix, xmlName, xmlNamespaceUri);

    xmlFree(xmlName);
    if (!sPrefix.isEmpty())
        xmlFree(xmlPrefix);
    if (!sNamespaceUri.isEmpty())
        xmlFree(xmlNamespaceUri);
}

void XmlWriter::startElement(const OString& sName)
{
    xmlChar* xmlName = xmlCharStrdup(sName.getStr());
    (void)xmlTextWriterStartElement(mpImpl->mpWriter, xmlName);
    xmlFree(xmlName);
}

void XmlWriter::endElement() { (void)xmlTextWriterEndElement(mpImpl->mpWriter); }

void XmlWriter::attributeBase64(const OString& rsName, std::vector<sal_uInt8> const& rValueInBytes)
{
    std::vector<char> aSignedBytes(rValueInBytes.begin(), rValueInBytes.end());
    attributeBase64(rsName, aSignedBytes);
}

void XmlWriter::attributeBase64(const OString& rsName, std::vector<char> const& rValueInBytes)
{
    xmlChar* xmlName = xmlCharStrdup(rsName.getStr());
    (void)xmlTextWriterStartAttribute(mpImpl->mpWriter, xmlName);
    (void)xmlTextWriterWriteBase64(mpImpl->mpWriter, rValueInBytes.data(), 0, rValueInBytes.size());
    (void)xmlTextWriterEndAttribute(mpImpl->mpWriter);
    xmlFree(xmlName);
}

void XmlWriter::attribute(const OString& name, const OString& value)
{
    xmlChar* xmlName = xmlCharStrdup(name.getStr());
    xmlChar* xmlValue = xmlCharStrdup(value.getStr());
    (void)xmlTextWriterWriteAttribute(mpImpl->mpWriter, xmlName, xmlValue);
    xmlFree(xmlValue);
    xmlFree(xmlName);
}

void XmlWriter::attribute(const OString& name, std::u16string_view value)
{
    attribute(name, OUStringToOString(value, RTL_TEXTENCODING_UTF8).getStr());
}

void XmlWriter::attribute(const OString& name, const sal_Int32 aNumber)
{
    attribute(name, OUString::number(aNumber));
}

void XmlWriter::attributeDouble(const OString& name, const double aNumber)
{
    attribute(name, OUString::number(aNumber));
}

void XmlWriter::content(const OString& sValue)
{
    xmlChar* xmlValue = xmlCharStrdup(sValue.getStr());
    (void)xmlTextWriterWriteString(mpImpl->mpWriter, xmlValue);
    xmlFree(xmlValue);
}

void XmlWriter::content(std::u16string_view sValue)
{
    content(OUStringToOString(sValue, RTL_TEXTENCODING_UTF8));
}

void XmlWriter::element(const OString& sName)
{
    startElement(sName);
    endElement();
}

} // end tools namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
