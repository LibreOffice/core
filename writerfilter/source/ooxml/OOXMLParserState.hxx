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
#ifndef INCLUDED_WRITERFILTER_SOURCE_OOXML_OOXMLPARSERSTATE_HXX
#define INCLUDED_WRITERFILTER_SOURCE_OOXML_OOXMLPARSERSTATE_HXX

#include <stack>
#include "OOXMLDocumentImpl.hxx"
#include "OOXMLPropertySet.hxx"

namespace writerfilter {
namespace ooxml
{

/**
 * Struct to store our 'alternate state'. If multiple mc:AlternateContent
 * elements arrive, then while the inner ones are active, the original state is
 * saved away, and once they inner goes out of scope, the original state is
 * restored.
 */
struct SavedAlternateState
{
    bool m_bDiscardChildren;
    bool m_bTookChoice; ///< Did we take the Choice or want Fallback instead?
};

class OOXMLParserState
{
    bool mbInSectionGroup;
    bool mbInParagraphGroup;
    bool mbInCharacterGroup;
    bool mbLastParagraphInSection;
    bool mbForwardEvents;
    unsigned int mnContexts;
    unsigned int mnHandle;
    OOXMLDocumentImpl* mpDocument;
    OOXMLPropertySet::Pointer_t mpCharacterProps;
    std::stack<OOXMLPropertySet::Pointer_t> mCellProps;
    std::stack<OOXMLPropertySet::Pointer_t> mRowProps;
    std::stack<OOXMLPropertySet::Pointer_t> mTableProps;
    bool inTxbxContent;
    // these 4 save when inTxbxContent
    bool savedInParagraphGroup;
    bool savedInCharacterGroup;
    bool savedLastParagraphInSection;
    std::vector<SavedAlternateState> maSavedAlternateStates;

public:
    typedef std::shared_ptr<OOXMLParserState> Pointer_t;

    OOXMLParserState();
    virtual ~OOXMLParserState();

    bool isInSectionGroup() const { return mbInSectionGroup;}
    void setInSectionGroup(bool bInSectionGroup);

    void setLastParagraphInSection(bool bLastParagraphInSection);
    bool isLastParagraphInSection() const { return mbLastParagraphInSection;}

    std::vector<SavedAlternateState>& getSavedAlternateStates() { return maSavedAlternateStates; }

    bool isInParagraphGroup() const { return mbInParagraphGroup;}
    void setInParagraphGroup(bool bInParagraphGroup);

    bool isInCharacterGroup() const { return mbInCharacterGroup;}
    void setInCharacterGroup(bool bInCharacterGroup);

    void setForwardEvents(bool bForwardEvents);
    bool isForwardEvents() const { return mbForwardEvents;}

    const std::string getHandle() const;
    void setHandle();

    void setDocument(OOXMLDocumentImpl* pDocument);
    OOXMLDocumentImpl* getDocument() const { return mpDocument;}

    void setXNoteId(const sal_Int32 rId);
    sal_Int32 getXNoteId() const;

    const OUString & getTarget() const;

    void resolveCharacterProperties(Stream & rStream);
    void setCharacterProperties(const OOXMLPropertySet::Pointer_t& pProps);
    void resolveCellProperties(Stream & rStream);
    void setCellProperties(const OOXMLPropertySet::Pointer_t& pProps);
    void resolveRowProperties(Stream & rStream);
    void setRowProperties(const OOXMLPropertySet::Pointer_t& pProps);
    void resolveTableProperties(Stream & rStream);
    void setTableProperties(const OOXMLPropertySet::Pointer_t& pProps);

    void startTable();
    void endTable();

    void incContextCount();

    void startTxbxContent();
    void endTxbxContent();

};

}}

#endif // INCLUDED_WRITERFILTER_SOURCE_OOXML_OOXMLPARSERSTATE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
