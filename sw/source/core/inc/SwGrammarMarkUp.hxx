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
