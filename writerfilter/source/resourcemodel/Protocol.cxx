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

#ifdef DEBUG_PROTOCOL
#include <stdio.h>
#include <rtl/ustrbuf.hxx>
#include <resourcemodel/Protocol.hxx>
#include <resourcemodel/WW8ResourceModel.hxx>
#include <resourcemodel/QNameToString.hxx>
namespace writerfilter
{

/*
  StreamProtocol
*/

StreamProtocol::StreamProtocol(Stream * pStream,
                               TagLogger::Pointer_t pTagLogger)
  : m_pStream(pStream), m_pTagLogger(pTagLogger)
{
}

StreamProtocol::~StreamProtocol()
{
}

void StreamProtocol::startSectionGroup()
{
    m_pTagLogger->element("protocol-startSectionGroup");
    m_pStream->startSectionGroup();
}

void StreamProtocol::endSectionGroup()
{
    m_pTagLogger->element("protocol-endSectionGroup");
    m_pStream->endSectionGroup();
}

void StreamProtocol::startParagraphGroup()
{
    m_pTagLogger->element("protocol-startParagraphGroup");
    m_pStream->startParagraphGroup();
}

void StreamProtocol::endParagraphGroup()
{
    m_pTagLogger->element("protocol-endParagraphGroup");
    m_pStream->endParagraphGroup();
}

void StreamProtocol::startCharacterGroup()
{
    m_pTagLogger->element("protocol-startCharacterGroup");
    m_pStream->startCharacterGroup();
}

void StreamProtocol::endCharacterGroup()
{
    m_pTagLogger->element("protocol-endCharacterGroup");
    m_pStream->endCharacterGroup();
}

void StreamProtocol::text(const sal_uInt8 * data, size_t len)
{
    OUString sText((const sal_Char*) data, len,
                          RTL_TEXTENCODING_MS_1252);
    m_pTagLogger->startElement("protocol-text");
    m_pTagLogger->chars(sText);
    m_pTagLogger->endElement();

    m_pStream->text(data, len);
}

void StreamProtocol::utext(const sal_uInt8 * data, size_t len)
{
    OUString sText;
    OUStringBuffer aBuffer = OUStringBuffer(len);
    aBuffer.append( (const sal_Unicode *) data, len);
    sText = aBuffer.makeStringAndClear();

    m_pTagLogger->startElement("protocol-utext");
    m_pTagLogger->chars(sText);
    m_pTagLogger->endElement();

    m_pStream->utext(data, len);
}

void StreamProtocol::props(writerfilter::Reference<Properties>::Pointer_t ref)
{
    m_pTagLogger->startElement("protocol-props");
    m_pStream->props(ref);
    m_pTagLogger->endElement();
}

void StreamProtocol::table(Id name,
                           writerfilter::Reference<Table>::Pointer_t ref)
{
    m_pTagLogger->startElement("protocol-table");
    m_pTagLogger->attribute("name", (*QNameToString::Instance())(name));
    m_pStream->table(name, ref);
    m_pTagLogger->endElement();
}

void StreamProtocol::substream(Id name,
                               writerfilter::Reference<Stream>::Pointer_t ref)
{
    m_pTagLogger->startElement("protocol-substream");
    m_pTagLogger->attribute("name", (*QNameToString::Instance())(name));

    m_pStream->substream(name, ref);
    m_pTagLogger->endElement();
}

void StreamProtocol::info(const string & rInfo)
{
    m_pStream->info(rInfo);
}

void StreamProtocol::startShape( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape )
{
    m_pTagLogger->element("protocol-startShape");

    m_pStream->startShape(xShape);
}

void StreamProtocol::endShape()
{
    m_pTagLogger->element("protocol-endShape");

    m_pStream->endShape();
}

/*
    PropertiesProtocol
*/

PropertiesProtocol::PropertiesProtocol(Properties * pProperties,
                                       TagLogger::Pointer_t pTagLogger)
: m_pProperties(pProperties), m_pTagLogger(pTagLogger)
{
}

PropertiesProtocol::~PropertiesProtocol()
{
}

void PropertiesProtocol::attribute(Id name, Value & val)
{
    m_pTagLogger->startElement("protocol-attribute");
    m_pTagLogger->attribute("name", (*QNameToString::Instance())(name));
    m_pTagLogger->attribute("value", val.toString());
    m_pProperties->attribute(name, val);
    m_pTagLogger->endElement();
}

void PropertiesProtocol::sprm(Sprm & _sprm)
{
    m_pTagLogger->startElement("protocol-sprm");
    static char sBuffer[256];
    snprintf(sBuffer, sizeof(sBuffer), "%04" SAL_PRIxUINT32, _sprm.getId());
    m_pTagLogger->attribute("id", sBuffer);
    m_pTagLogger->attribute("name", _sprm.getName());
    m_pTagLogger->chars(_sprm.toString());
    m_pProperties->sprm(_sprm);
    m_pTagLogger->endElement();
}

/*
  TableProtocol
 */

TableProtocol::TableProtocol(Table * pTable, TagLogger::Pointer_t pTagLogger)
: m_pTable(pTable), m_pTagLogger(pTagLogger)
{
}

TableProtocol::~TableProtocol()
{
}

void TableProtocol::entry(int pos,
                          writerfilter::Reference<Properties>::Pointer_t ref)
{
    m_pTagLogger->startElement("protocol-entry");
    m_pTagLogger->attribute("pos", pos);
    m_pTable->entry(pos, ref);
    m_pTagLogger->endElement();
}

}
#endif // DEBUG_PROTOCOL

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
