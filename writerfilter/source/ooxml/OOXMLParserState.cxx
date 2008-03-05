/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OOXMLParserState.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:05:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include <stdio.h>
#include <iostream>
#include "OOXMLParserState.hxx"

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
        logger("DEBUG", "<newCharacterProperty>");
        logger("DEBUG", pProperty->toString());
        logger("DEBUG", "</newCharacterProperty>");
#endif

        mpCharacterProps->add(pProperty);
    }

}

void OOXMLParserState::resolveCharacterProperties(Stream & rStream)
{
    if (mpCharacterProps.get() != NULL)
    {
#ifdef DEBUG_PROPERTIES
        logger("DEBUG", "<resolveCharacterProperties>");
        logger("DEBUG", mpCharacterProps->toString());
        logger("DEBUG", "</resolveCharacterProperties>");
#endif
        rStream.props(mpCharacterProps);
        mpCharacterProps.reset(new OOXMLPropertySetImpl());
    }
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

string OOXMLParserState::toString() const
{
    string sResult = "(";

    if (isInSectionGroup())
        sResult += "s";
    else
        sResult += "-";

    if (isInParagraphGroup())
        sResult += "p";
    else
        sResult += "-";

    if (isInCharacterGroup())
        sResult += "c";
    else
        sResult += "-";

    if (isForwardEvents())
        sResult += "f";
    else
        sResult += "-";

    sResult += ", \"";
    sResult += OUStringToOString(getXNoteId(),
                                 RTL_TEXTENCODING_ASCII_US).getStr();

    sResult += "\")";

    return sResult;
}

}}
