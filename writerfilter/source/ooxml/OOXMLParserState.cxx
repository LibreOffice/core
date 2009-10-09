/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OOXMLParserState.cxx,v $
 * $Revision: 1.6 $
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

#include <stdio.h>
#include <iostream>
#include "OOXMLParserState.hxx"
#include "ooxmlLoggers.hxx"

namespace writerfilter {
namespace ooxml
{
/*
  class OOXMLParserState
*/

OOXMLParserState::OOXMLParserState() :
    mbInSectionGroup(false),
    mbInParagraphGroup(false),
    mbInCharacterGroup(false),
    mbLastParagraphInSection(false),
    mbForwardEvents(true),
    mnContexts(0),
    mnHandle(0),
    mpDocument(NULL)
{
}

OOXMLParserState::~OOXMLParserState()
{
}

void OOXMLParserState::setLastParagraphInSection(bool bLastParagraphInSection)
{
    mbLastParagraphInSection = bLastParagraphInSection;
}

bool OOXMLParserState::isLastParagraphInSection() const
{
    return mbLastParagraphInSection;
}

bool OOXMLParserState::isInSectionGroup() const
{
    return mbInSectionGroup;
}

void OOXMLParserState::setInSectionGroup(bool bInSectionGroup)
{
    mbInSectionGroup = bInSectionGroup;
}

bool OOXMLParserState::isInParagraphGroup() const
{
    return mbInParagraphGroup;
}

void OOXMLParserState::setInParagraphGroup(bool bInParagraphGroup)
{
    mbInParagraphGroup = bInParagraphGroup;
}

bool OOXMLParserState::isInCharacterGroup() const
{
    return mbInCharacterGroup;
}

void OOXMLParserState::setInCharacterGroup(bool bInCharacterGroup)
{
    mbInCharacterGroup = bInCharacterGroup;
}

void OOXMLParserState::setForwardEvents(bool bForwardEvents)
{
    mbForwardEvents = bForwardEvents;
}

bool OOXMLParserState::isForwardEvents() const
{
    return mbForwardEvents;
}

void OOXMLParserState::incContextCount()
{
    mnContexts++;
}

const string OOXMLParserState::getHandle() const
{
    char sBuffer[256];

    snprintf(sBuffer, sizeof(sBuffer), "%d", mnHandle);

    return sBuffer;
}

void OOXMLParserState::setHandle()
{
    mnHandle = mnContexts;
}

unsigned int OOXMLParserState::getContextCount() const
{
    return mnContexts;
}

void OOXMLParserState::setDocument(OOXMLDocument * pDocument)
{
    mpDocument = pDocument;
}

OOXMLDocument * OOXMLParserState::getDocument() const
{
    return mpDocument;
}

void OOXMLParserState::setXNoteId(const rtl::OUString & rId)
{
    mpDocument->setXNoteId(rId);
}

const rtl::OUString & OOXMLParserState::getXNoteId() const
{
    return mpDocument->getXNoteId();
}

void OOXMLParserState::setXNoteType(const Id & rId)
{
    mpDocument->setXNoteType(rId);
}

const Id & OOXMLParserState::getXNoteType() const
{
    return mpDocument->getXNoteType();
}

const ::rtl::OUString & OOXMLParserState::getTarget() const
{
    return mpDocument->getTarget();
}

void OOXMLParserState::newCharacterProperty(const Id & rId,
                                            OOXMLValue::Pointer_t pVal)
{
    if (rId != 0x0)
    {
        if (mpCharacterProps.get() == NULL)
            mpCharacterProps =
                OOXMLPropertySet::Pointer_t(new OOXMLPropertySetImpl());

        OOXMLPropertyImpl::Pointer_t pProperty
            (new OOXMLPropertyImpl(rId, pVal, OOXMLPropertyImpl::ATTRIBUTE));

#ifdef DEBUG_PROPERTIES
        debug_logger->startElement("<newCharacterProperty");
        debug_logger->chars(pProperty->toString());
        debug_logger->endElement("newCharacterProperty");
#endif

        mpCharacterProps->add(pProperty);
    }

}

void OOXMLParserState::resolveCharacterProperties(Stream & rStream)
{
    if (mpCharacterProps.get() != NULL)
    {
#ifdef DEBUG_PROPERTIES
        debug_logger->startElement("resolveCharacterProperties");
#endif

        rStream.props(mpCharacterProps);
        mpCharacterProps.reset(new OOXMLPropertySetImpl());

#ifdef DEBUG_PROPERTIES
        debug_logger->endElement("resolveCharacterProperties");
#endif
    }
}

OOXMLPropertySet::Pointer_t OOXMLParserState::getCharacterProperties() const
{
    return mpCharacterProps;
}

void OOXMLParserState::setCharacterProperties
(OOXMLPropertySet::Pointer_t pProps)
{
    if (mpCharacterProps.get() == NULL)
        mpCharacterProps = pProps;
    else
        mpCharacterProps->add(pProps);
}

void OOXMLParserState::resolveTableProperties(Stream & rStream)
{
    if (mpTableProps.get() != NULL)
    {
        rStream.props(mpTableProps);
        mpTableProps.reset(new OOXMLPropertySetImpl());
    }
}

void OOXMLParserState::setTableProperties
(OOXMLPropertySet::Pointer_t pProps)
{
    if (mpTableProps.get() == NULL)
        mpTableProps = pProps;
    else
        mpTableProps->add(pProps);
}

XMLTag::Pointer_t OOXMLParserState::toTag() const
{
    XMLTag::Pointer_t pTag(new XMLTag("parserstate"));

    string sTmp;

    if (isInSectionGroup())
        sTmp += "s";
    else
        sTmp += "-";

    if (isInParagraphGroup())
        sTmp += "p";
    else
        sTmp += "-";

    if (isInCharacterGroup())
        sTmp += "c";
    else
        sTmp += "-";

    if (isForwardEvents())
        sTmp += "f";
    else
        sTmp += "-";

    pTag->addAttr("state", sTmp);
    pTag->addAttr("XNoteId",
                  OUStringToOString(getXNoteId(),
                                    RTL_TEXTENCODING_ASCII_US).getStr());
    if (mpCharacterProps != OOXMLPropertySet::Pointer_t())
        pTag->chars(mpCharacterProps->toString());

    return pTag;
 }

string OOXMLParserState::toString() const
{
    return toTag()->toString();
}

}}
