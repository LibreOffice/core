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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_SWGRAMMARMARKUP_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_SWGRAMMARMARKUP_HXX

#include "wrong.hxx"
#include <vector>

/* SwGrammarMarkUp extends the functionality of a "normal" SwWrongList by memorizing
   the start positions of sentences in the paragraph

   The whole class is only a temporary solution without usage of virtual functions.
   At the end the whole SwWrongList stuff should be reworked and replaced by interfaces
   to deal with all the different wronglists like
    spell, grammar, smarttag, sentence...
   "MarkUpList" would be a better name than WrongList.
*/

class SwGrammarMarkUp final : public SwWrongList
{
    std::vector<sal_Int32> maSentence;

public:
    SwGrammarMarkUp()
        : SwWrongList(WRONGLIST_GRAMMAR)
    {
    }

    virtual ~SwGrammarMarkUp() override;
    virtual SwWrongList* Clone() override;
    virtual void CopyFrom(const SwWrongList& rCopy) override;

    /* SwWrongList::Move() + handling of maSentence */
    void MoveGrammar(sal_Int32 nPos, sal_Int32 nDiff);
    /* SwWrongList::SplitList() + handling of maSentence */
    SwGrammarMarkUp* SplitGrammarList(sal_Int32 nSplitPos);
    /* SwWrongList::JoinList() + handling of maSentence */
    void JoinGrammarList(SwGrammarMarkUp* pNext, sal_Int32 nInsertPos);
    /* SwWrongList::ClearList() + handling of maSentence */
    void ClearGrammarList(sal_Int32 nSentenceEnd = COMPLETE_STRING);
    /* setSentence to define the start position of a sentence,
       at the moment the end position is given by the next start position */
    void setSentence(sal_Int32 nStart);
    /* getSentenceStart returns the last start position of a sentence
       which is lower or equal to the given parameter */
    sal_Int32 getSentenceStart(sal_Int32 nPos);
    /* getSentenceEnd returns the first start position of a sentence
       which is greater than the given parameter */
    sal_Int32 getSentenceEnd(sal_Int32 nPos);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
