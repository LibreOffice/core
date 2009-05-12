/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OOXMLParserState.hxx,v $
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
#ifndef INCLUDE_OOXML_PARSER_STATE_HXX
#define INCLUDE_OOXML_PARSER_STATE_HXX

#include <ooxml/OOXMLDocument.hxx>
#include <resourcemodel/TagLogger.hxx>
#include "OOXMLPropertySetImpl.hxx"

namespace writerfilter {
namespace ooxml
{

class OOXMLParserState
{
    bool mbInSectionGroup;
    bool mbInParagraphGroup;
    bool mbInCharacterGroup;
    bool mbLastParagraphInSection;
    bool mbForwardEvents;
    unsigned int mnContexts;
    unsigned int mnHandle;
    OOXMLDocument * mpDocument;
    rtl::OUString msXNoteId;
    rtl::OUString msTarget;
    OOXMLPropertySet::Pointer_t mpCharacterProps;
    OOXMLPropertySet::Pointer_t mpTableProps;

public:
    typedef boost::shared_ptr<OOXMLParserState> Pointer_t;

    OOXMLParserState();
    virtual ~OOXMLParserState();

    bool isInSectionGroup() const;
    void setInSectionGroup(bool bInSectionGroup);

    void setLastParagraphInSection(bool bLastParagraphInSection);
    bool isLastParagraphInSection() const;

    bool isInParagraphGroup() const;
    void setInParagraphGroup(bool bInParagraphGroup);

    bool isInCharacterGroup() const;
    void setInCharacterGroup(bool bInCharacterGroup);

    void setForwardEvents(bool bForwardEvents);
    bool isForwardEvents() const;

    void incContextCount();
    const string getHandle() const;
    void setHandle();
    unsigned int getContextCount() const;

    void setDocument(OOXMLDocument * pDocument);
    OOXMLDocument * getDocument() const;

    void setXNoteId(const rtl::OUString & rId);
    const rtl::OUString & getXNoteId() const;
    void setXNoteType(const Id & rId);
    const Id & getXNoteType() const;

    const rtl::OUString & getTarget() const;

    void newCharacterProperty(const Id & rId, OOXMLValue::Pointer_t pVal);
    void resolveCharacterProperties(Stream & rStream);
    OOXMLPropertySet::Pointer_t getCharacterProperties() const;
    void setCharacterProperties(OOXMLPropertySet::Pointer_t pProps);
    void resolveTableProperties(Stream & rStream);
    void setTableProperties(OOXMLPropertySet::Pointer_t pProps);

    string toString() const;
    XMLTag::Pointer_t toTag() const;
};

}}

#endif // INCLUDE_OOXML_PARSER_STATE_HXX
