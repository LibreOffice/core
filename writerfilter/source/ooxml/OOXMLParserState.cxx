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
    mpDocument(nullptr),
    inTxbxContent(false),
    savedInParagraphGroup(false),
    savedInCharacterGroup(false),
    savedLastParagraphInSection(false)
{
}

OOXMLParserState::~OOXMLParserState()
{
}

void OOXMLParserState::setLastParagraphInSection(bool bLastParagraphInSection)
{
    mbLastParagraphInSection = bLastParagraphInSection;
}


void OOXMLParserState::setInSectionGroup(bool bInSectionGroup)
{
    mbInSectionGroup = bInSectionGroup;
}


void OOXMLParserState::setInParagraphGroup(bool bInParagraphGroup)
{
    mbInParagraphGroup = bInParagraphGroup;
}


void OOXMLParserState::setInCharacterGroup(bool bInCharacterGroup)
{
    mbInCharacterGroup = bInCharacterGroup;
}

void OOXMLParserState::setForwardEvents(bool bForwardEvents)
{
    mbForwardEvents = bForwardEvents;
}


const std::string OOXMLParserState::getHandle() const
{
    char sBuffer[256];

    snprintf(sBuffer, sizeof(sBuffer), "%u", mnHandle);

    return sBuffer;
}

void OOXMLParserState::setHandle()
{
    mnHandle = mnContexts;
}

void OOXMLParserState::setDocument(OOXMLDocumentImpl* pDocument)
{
    mpDocument = pDocument;
}


void OOXMLParserState::setXNoteId(const sal_Int32 nId)
{
    mpDocument->setXNoteId(nId);
}

sal_Int32 OOXMLParserState::getXNoteId() const
{
    return mpDocument->getXNoteId();
}

const OUString & OOXMLParserState::getTarget() const
{
    return mpDocument->getTarget();
}

void OOXMLParserState::resolveCharacterProperties(Stream & rStream)
{
    if (mpCharacterProps.get() != nullptr)
    {
        rStream.props(mpCharacterProps);
        mpCharacterProps.reset(new OOXMLPropertySet);
    }
}

void OOXMLParserState::setCharacterProperties(const OOXMLPropertySet::Pointer_t& pProps)
{
    if (mpCharacterProps.get() == nullptr)
        mpCharacterProps = pProps;
    else
        mpCharacterProps->add(pProps);
}

void OOXMLParserState::setCellProperties(const OOXMLPropertySet::Pointer_t& pProps)
{
    if (!mCellProps.empty())
    {
        OOXMLPropertySet::Pointer_t & rCellProps = mCellProps.top();

        if (rCellProps.get() == nullptr)
            rCellProps = pProps;
        else
            rCellProps->add(pProps);
    }
}

void OOXMLParserState::setRowProperties(const OOXMLPropertySet::Pointer_t& pProps)
{
    if (!mRowProps.empty())
    {
        OOXMLPropertySet::Pointer_t & rRowProps = mRowProps.top();

        if (rRowProps.get() == nullptr)
            rRowProps = pProps;
        else
            rRowProps->add(pProps);
    }
}

void OOXMLParserState::resolveCellProperties(Stream & rStream)
{
    if (!mCellProps.empty())
    {
        OOXMLPropertySet::Pointer_t & rCellProps = mCellProps.top();

        if (rCellProps.get() != nullptr)
        {
            rStream.props(rCellProps);
            rCellProps.reset(new OOXMLPropertySet);
        }
    }
}

void OOXMLParserState::resolveRowProperties(Stream & rStream)
{
    if (!mRowProps.empty())
    {
        OOXMLPropertySet::Pointer_t & rRowProps = mRowProps.top();

        if (rRowProps.get() != nullptr)
        {
            rStream.props(rRowProps);
            rRowProps.reset(new OOXMLPropertySet);
        }
    }
}

void OOXMLParserState::resolveTableProperties(Stream & rStream)
{
    if (!mTableProps.empty())
    {
        OOXMLPropertySet::Pointer_t & rTableProps = mTableProps.top();

        if (rTableProps.get() != nullptr)
        {
            rStream.props(rTableProps);
            // Don't clean the table props to send them again for each row
            // This mimics the behaviour from RTF tokenizer.
        }
    }
}

void OOXMLParserState::setTableProperties(const OOXMLPropertySet::Pointer_t& pProps)
{
    if (!mTableProps.empty())
    {
        OOXMLPropertySet::Pointer_t & rTableProps = mTableProps.top();
        if (rTableProps.get() == nullptr)
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

void OOXMLParserState::startTxbxContent()
{
    SAL_WARN_IF(inTxbxContent, "writerfilter", "Nested w:txbxContent");

    inTxbxContent = true;
    // Do not save and reset section group state, it'd cause a new page.
//    savedInSectionGroup = mbInSectionGroup;
    savedInParagraphGroup = mbInParagraphGroup;
    savedInCharacterGroup = mbInCharacterGroup;
    savedLastParagraphInSection = mbLastParagraphInSection;
//    mbInSectionGroup = false;
    mbInParagraphGroup = false;
    mbInCharacterGroup = false;
    mbLastParagraphInSection = false;
}

void OOXMLParserState::endTxbxContent()
{
    if( !inTxbxContent )
    {
        SAL_WARN( "writerfilter", "Non-matching closing w:txbxContent" );
        return;
    }
//    mbInSectionGroup = savedInSectionGroup;
    mbInParagraphGroup = savedInParagraphGroup;
    mbInCharacterGroup = savedInCharacterGroup;
    mbLastParagraphInSection = savedLastParagraphInSection;
    inTxbxContent = false;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
