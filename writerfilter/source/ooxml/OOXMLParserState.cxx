/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

const ::rtl::OUString & OOXMLParserState::getTarget() const
{
    return mpDocument->getTarget();
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

void OOXMLParserState::setCharacterProperties
(OOXMLPropertySet::Pointer_t pProps)
{
    if (mpCharacterProps.get() == NULL)
        mpCharacterProps = pProps;
    else
        mpCharacterProps->add(pProps);
}

void OOXMLParserState::setCellProperties
(OOXMLPropertySet::Pointer_t pProps)
{
    if (mCellProps.size() > 0)
    {
        OOXMLPropertySet::Pointer_t & rCellProps = mCellProps.top();
        
        if (rCellProps.get() == NULL)
            rCellProps = pProps;
        else 
            rCellProps->add(pProps);
    }
}
    
void OOXMLParserState::setRowProperties
(OOXMLPropertySet::Pointer_t pProps)
{
    if (mRowProps.size() > 0)
    {
        OOXMLPropertySet::Pointer_t & rRowProps = mRowProps.top();
        
        if (rRowProps.get() == NULL)
            rRowProps = pProps;
        else 
            rRowProps->add(pProps);
    }
}

void OOXMLParserState::resolveCellProperties(Stream & rStream)
{
    if (mCellProps.size() > 0)
    {
        OOXMLPropertySet::Pointer_t & rCellProps = mCellProps.top();
        
        if (rCellProps.get() != NULL)
        {
            rStream.props(rCellProps);
            rCellProps.reset(new OOXMLPropertySetImpl());
        }
    }
}

void OOXMLParserState::resolveRowProperties(Stream & rStream)
{
    if (mRowProps.size() > 0)
    {
        OOXMLPropertySet::Pointer_t & rRowProps = mRowProps.top();
        
        if (rRowProps.get() != NULL)
        {
            rStream.props(rRowProps);
            rRowProps.reset(new OOXMLPropertySetImpl());
        }
    }
}

void OOXMLParserState::resolveTableProperties(Stream & rStream)
{
    if (mTableProps.size() > 0)
    {
        OOXMLPropertySet::Pointer_t & rTableProps = mTableProps.top();
        
        if (rTableProps.get() != NULL)
        {
            rStream.props(rTableProps);
            rTableProps.reset(new OOXMLPropertySetImpl());
        }
    }
}

void OOXMLParserState::setTableProperties
(OOXMLPropertySet::Pointer_t pProps)
{
    if (mTableProps.size() > 0)
    {
        OOXMLPropertySet::Pointer_t & rTableProps = mTableProps.top();
        if (rTableProps.get() == NULL) 
            rTableProps = pProps;
        else
            rTableProps->add(pProps);
    }
}

void OOXMLParserState::startTable()
{
    OOXMLPropertySet::Pointer_t pCellProps;
    OOXMLPropertySet::Pointer_t pRowProps;
    OOXMLPropertySet::Pointer_t pTableProps;
    
    mCellProps.push(pCellProps);
    mRowProps.push(pRowProps);
    mTableProps.push(pTableProps);
}
    
void OOXMLParserState::endTable()
{
    mCellProps.pop();
    mRowProps.pop();
    mTableProps.pop();
}

void OOXMLParserState::incContextCount()
{
    mnContexts++;
}

#ifdef DEBUG
unsigned int OOXMLParserState::getContextCount() const
{
    return mnContexts;
}

string OOXMLParserState::toString() const
{
    return toTag()->toString();
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
#endif

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
