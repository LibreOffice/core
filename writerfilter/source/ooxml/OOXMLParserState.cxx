/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    pTag->addAttr("XNoteId", getDocument()->getIDForXNoteStream() );
    if (mpCharacterProps != OOXMLPropertySet::Pointer_t())
        pTag->chars(mpCharacterProps->toString());

    return pTag;
 }

XPathLogger & OOXMLParserState::getXPathLogger()
{
    return m_xPathLogger;
}
#endif

}}
