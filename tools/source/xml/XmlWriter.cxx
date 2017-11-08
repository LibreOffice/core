/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/XmlWriter.hxx>
#include <o3tl/make_unique.hxx>

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
    {
    }

    SvStream* mpStream;
    xmlTextWriterPtr mpWriter;
};

XmlWriter::XmlWriter(SvStream* pStream)
    : mpImpl(o3tl::make_unique<XmlWriterImpl>(pStream))
{
}

XmlWriter::~XmlWriter()
{
    if (mpImpl && mpImpl->mpWriter != nullptr)
        endDocument();
}

bool XmlWriter::startDocument()
{
    xmlOutputBufferPtr xmlOutBuffer
        = xmlOutputBufferCreateIO(funcWriteCallback, funcCloseCallback, mpImpl->mpStream, nullptr);
    mpImpl->mpWriter = xmlNewTextWriter(xmlOutBuffer);
    if (mpImpl->mpWriter == nullptr)
        return false;
    xmlTextWriterSetIndent(mpImpl->mpWriter, 1);
    xmlTextWriterStartDocument(mpImpl->mpWriter, nullptr, "UTF-8", nullptr);
    return true;
}

void XmlWriter::endDocument()
{
    xmlTextWriterEndDocument(mpImpl->mpWriter);
    xmlFreeTextWriter(mpImpl->mpWriter);
    mpImpl->mpWriter = nullptr;
}

void XmlWriter::startElement(const OString& sName)
{
    xmlChar* xmlName = xmlCharStrdup(sName.getStr());
    xmlTextWriterStartElement(mpImpl->mpWriter, xmlName);
    xmlFree(xmlName);
}

void XmlWriter::endElement() { xmlTextWriterEndElement(mpImpl->mpWriter); }

void XmlWriter::attribute(const OString& name, const OString& value)
{
    xmlChar* xmlName = xmlCharStrdup(name.getStr());
    xmlChar* xmlValue = xmlCharStrdup(value.getStr());
    xmlTextWriterWriteAttribute(mpImpl->mpWriter, xmlName, xmlValue);
    xmlFree(xmlValue);
    xmlFree(xmlName);
}

void XmlWriter::attribute(const OString& name, const OUString& value)
{
    attribute(name, OUStringToOString(value, RTL_TEXTENCODING_UTF8).getStr());
}

void XmlWriter::attribute(const OString& name, const sal_Int32 aNumber)
{
    attribute(name, OUString::number(aNumber));
}

void XmlWriter::content(const OString& sValue)
{
    xmlChar* xmlValue = xmlCharStrdup(sValue.getStr());
    xmlTextWriterWriteString(mpImpl->mpWriter, xmlValue);
    xmlFree(xmlValue);
}

void XmlWriter::content(const OUString& sValue)
{
    content(OUStringToOString(sValue, RTL_TEXTENCODING_UTF8));
}

} // end tools namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
