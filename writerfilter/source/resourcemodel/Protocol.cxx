/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: WW8ResourceModel.hxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <rtl/ustrbuf.hxx>
#include <resourcemodel/Protocol.hxx>
#include <resourcemodel/WW8ResourceModel.hxx>
#include <resourcemodel/QNameToString.hxx>
namespace writerfilter
{

StreamProtocol::StreamProtocol(Stream::Pointer_t pStream,
                               TagLogger::Pointer_t pTagLogger)
  : m_pStream(pStream), m_pTagLogger(pTagLogger)
{
    m_pTagLogger->startElement("protocol:stream");
}

StreamProtocol::~StreamProtocol()
{
    m_pTagLogger->endElement("protocol:stream");
}

void StreamProtocol::startSectionGroup()
{
    m_pTagLogger->element("protocol:startSectionGroup");
    m_pStream->startSectionGroup();
}

void StreamProtocol::endSectionGroup()
{
    m_pTagLogger->element("protocol:endSectionGroup");
    m_pStream->endSectionGroup();
}

void StreamProtocol::startParagraphGroup()
{
    m_pTagLogger->element("protocol:startParagraphGroup");
    m_pStream->startParagraphGroup();
}

void StreamProtocol::endParagraphGroup()
{
    m_pTagLogger->element("protocol:endParagraphGroup");
    m_pStream->endParagraphGroup();
}

void StreamProtocol::startCharacterGroup()
{
    m_pTagLogger->element("protocol:startCharacterGroup");
    m_pStream->startCharacterGroup();
}

void StreamProtocol::endCharacterGroup()
{
    m_pTagLogger->element("protocol:endCharacterGroup");
    m_pStream->endCharacterGroup();
}

void StreamProtocol::text(const sal_uInt8 * data, size_t len)
{
    ::rtl::OUString sText((const sal_Char*) data, len,
                          RTL_TEXTENCODING_MS_1252);
    m_pTagLogger->startElement("protocol:text");
    m_pTagLogger->chars(sText);
    m_pTagLogger->endElement("protocol:text");

    m_pStream->text(data, len);
}

void StreamProtocol::utext(const sal_uInt8 * data, size_t len)
{
    ::rtl::OUString sText;
    ::rtl::OUStringBuffer aBuffer = ::rtl::OUStringBuffer(len);
    aBuffer.append( (const sal_Unicode *) data, len);
    sText = aBuffer.makeStringAndClear();

    m_pTagLogger->startElement("protocol:text");
    m_pTagLogger->chars(sText);
    m_pTagLogger->endElement("protocol:text");

    m_pStream->text(data, len);
}

void StreamProtocol::props(writerfilter::Reference<Properties>::Pointer_t ref)
{
    m_pTagLogger->startElement("protocol:props");
    m_pStream->props(ref);
    m_pTagLogger->endElement("protocol:props");
}

void StreamProtocol::table(Id name,
                           writerfilter::Reference<Table>::Pointer_t ref)
{
    m_pTagLogger->startElement("protocol:table");
    m_pTagLogger->attribute("name", (*QNameToString::Instance())(name));
    m_pStream->table(name, ref);
    m_pTagLogger->endElement("protocol:table");
}

void StreamProtocol::substream(Id name,
                               writerfilter::Reference<Stream>::Pointer_t ref)
{
    m_pTagLogger->startElement("protocol:substream");
    m_pTagLogger->attribute("name", (*QNameToString::Instance())(name));

    m_pStream->substream(name, ref);
    m_pTagLogger->endElement("protocol:substream");
}

}
