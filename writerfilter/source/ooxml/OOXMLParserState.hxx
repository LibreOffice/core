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
#ifndef INCLUDE_OOXML_PARSER_STATE_HXX
#define INCLUDE_OOXML_PARSER_STATE_HXX

#include <stack>
#include <ooxml/OOXMLDocument.hxx>
#include "OOXMLPropertySetImpl.hxx"

#ifdef DEBUG
#include <resourcemodel/TagLogger.hxx>
#endif

namespace writerfilter {
namespace ooxml
{

using ::std::stack;

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
    stack<OOXMLPropertySet::Pointer_t> mCellProps;
    stack<OOXMLPropertySet::Pointer_t> mRowProps;
    stack<OOXMLPropertySet::Pointer_t> mTableProps;

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
    
    const string getHandle() const;
    void setHandle();

    void setDocument(OOXMLDocument * pDocument);
    OOXMLDocument * getDocument() const;

    void setXNoteId(const rtl::OUString & rId);
    const rtl::OUString & getXNoteId() const;

    const rtl::OUString & getTarget() const;

    void resolveCharacterProperties(Stream & rStream);
    void setCharacterProperties(OOXMLPropertySet::Pointer_t pProps);
    void resolveCellProperties(Stream & rStream);
    void setCellProperties(OOXMLPropertySet::Pointer_t pProps);
    void resolveRowProperties(Stream & rStream);
    void setRowProperties(OOXMLPropertySet::Pointer_t pProps);
    void resolveTableProperties(Stream & rStream);
    void setTableProperties(OOXMLPropertySet::Pointer_t pProps);

    void startTable();
    void endTable();
    
    void incContextCount();

#ifdef DEBUG
public:
    unsigned int getContextCount() const;
    string toString() const;
    XMLTag::Pointer_t toTag() const;
#endif

};

}}

#endif // INCLUDE_OOXML_PARSER_STATE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
