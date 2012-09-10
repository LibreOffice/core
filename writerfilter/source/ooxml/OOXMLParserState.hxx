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
#ifndef INCLUDE_OOXML_PARSER_STATE_HXX
#define INCLUDE_OOXML_PARSER_STATE_HXX

#include <stack>
#include <ooxml/OOXMLDocument.hxx>
#include "OOXMLPropertySetImpl.hxx"

#if OSL_DEBUG_LEVEL > 1
#include <resourcemodel/TagLogger.hxx>
#include <resourcemodel/XPathLogger.hxx>
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
    OUString msTarget;
    OOXMLPropertySet::Pointer_t mpCharacterProps;
    stack<OOXMLPropertySet::Pointer_t> mCellProps;
    stack<OOXMLPropertySet::Pointer_t> mRowProps;
    stack<OOXMLPropertySet::Pointer_t> mTableProps;
    bool inTxbxContent;
    // these 4 save when inTxbxContent
    bool savedInParagraphGroup;
    bool savedInCharacterGroup;
    bool savedLastParagraphInSection;
#if OSL_DEBUG_LEVEL > 1
    XPathLogger m_xPathLogger;
#endif

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

    void setXNoteId(const sal_Int32 rId);
    sal_Int32 getXNoteId() const;

    const OUString & getTarget() const;

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

    void startTxbxContent();
    void endTxbxContent();

#if OSL_DEBUG_LEVEL > 1
public:
    void dumpXml( const TagLogger::Pointer_t& pLogger );
    XPathLogger & getXPathLogger();
#endif

};

}}

#endif // INCLUDE_OOXML_PARSER_STATE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
