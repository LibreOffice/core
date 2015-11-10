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

#include <libxml/xmlstring.h>
#include <tools/stream.hxx>
#include <test/xmlwriter.hxx>

namespace
{

int lclWriteCallback(void* pContext, const char* sBuffer, int nLen)
{
    SvStream* pStream = static_cast<SvStream*>(pContext);
    return (int) pStream->Write(sBuffer, nLen);
}

int lclCloseCallback(void* pContext)
{
    SvStream* pStream = static_cast<SvStream*>(pContext);
    pStream->Flush();
    return 0; // 0 or -1 in case of error
}

} // anonymous namespace

XmlWriter::XmlWriter(SvStream* pStream) :
    mpStream(pStream),
    mpWriter(nullptr)
{}

XmlWriter::~XmlWriter()
{}

void XmlWriter::startDocument()
{
    if (mpWriter == nullptr && mpStream != nullptr)
    {
        xmlOutputBufferPtr xmlOutBuffer = xmlOutputBufferCreateIO(lclWriteCallback, lclCloseCallback, mpStream, nullptr);
        mpWriter = xmlNewTextWriter(xmlOutBuffer);
        xmlTextWriterSetIndent(mpWriter, 1);
        xmlTextWriterStartDocument(mpWriter, nullptr, "UTF-8", nullptr);
    }
}

void XmlWriter::endDocument()
{
    xmlTextWriterEndDocument(mpWriter);
    xmlFreeTextWriter(mpWriter);
    mpWriter = nullptr;
}

void XmlWriter::element(const OString& name)
{
    startElement(name);
    endElement();
}

void XmlWriter::startElement(const OString& name)
{
    xmlChar* xmlName = xmlCharStrdup(name.getStr());
    xmlTextWriterStartElement(mpWriter, xmlName);
    xmlFree(xmlName);
}

void XmlWriter::attribute(const OString& name, const OString & value)
{
    xmlChar* xmlName = xmlCharStrdup(name.getStr());
    xmlChar* xmlValue = xmlCharStrdup(value.getStr());
    xmlTextWriterWriteAttribute(mpWriter, xmlName, xmlValue);
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

void XmlWriter::content(const OUString& aValue)
{
    content(OUStringToOString(aValue, RTL_TEXTENCODING_UTF8));
}

void XmlWriter::content(const OString& aValue)
{
    xmlChar* xmlValue = xmlCharStrdup(aValue.getStr());
    xmlTextWriterWriteString(mpWriter, xmlValue);
    xmlFree(xmlValue);
}

void XmlWriter::endElement()
{
    xmlTextWriterEndElement(mpWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
