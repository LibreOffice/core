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
