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

#ifndef _SWGRAMMARMARKUP_HXX
#define _SWGRAMMARMARKUP_HXX

#include <wrong.hxx>
#include <vector>

/* SwGrammarMarkUp extends the functionality of a "normal" SwWrongList by memorizing
   the start positions of sentences in the paragraph

   The whole class is only a temporary solution without usage of virtual functions.
   At the end the whole SwWrongList stuff should be reworked and replaced by interfaces
   to deal with all the different wronglists like
    spell, grammar, smarttag, sentence...
   "MarkUpList" would be a better name than WrongList.
*/

class SwGrammarMarkUp : public SwWrongList
{
    std::vector< xub_StrLen > maSentence;

public:
    SwGrammarMarkUp() : SwWrongList( WRONGLIST_GRAMMAR ) {}
    SwGrammarMarkUp( const SwGrammarMarkUp* );

    virtual ~SwGrammarMarkUp();
    virtual SwWrongList* Clone();
    virtual void CopyFrom( const SwWrongList& rCopy );

    /* SwWrongList::Move() + handling of maSentence */
    void MoveGrammar( xub_StrLen nPos, long nDiff );
    /* SwWrongList::SplitList() + handling of maSentence */
    SwGrammarMarkUp* SplitGrammarList( xub_StrLen nSplitPos );
    /* SwWrongList::JoinList() + handling of maSentence */
    void JoinGrammarList( SwGrammarMarkUp* pNext, xub_StrLen nInsertPos );
    /* SwWrongList::ClearList() + handling of maSentence */
    void ClearGrammarList( xub_StrLen nSentenceEnd = STRING_LEN );
    /* setSentence to define the start positionof a sentence,
       at the moment the end position is given by the next start position */
    void setSentence( xub_StrLen nStart );
    /* getSentenceStart returns the last start position of a sentence
       which is lower or equal to the given parameter */
    xub_StrLen getSentenceStart( xub_StrLen nPos );
    /* getSentenceEnd returns the first start position of a sentence
       which is greater than the given parameter */
    xub_StrLen getSentenceEnd( xub_StrLen nPos );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
