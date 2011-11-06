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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include "SwGrammarMarkUp.hxx"

SwGrammarMarkUp::~SwGrammarMarkUp()
{
}

SwWrongList* SwGrammarMarkUp::Clone()
{
    SwWrongList* pClone = new SwGrammarMarkUp();
    pClone->CopyFrom( *this );
    return pClone;
}

void SwGrammarMarkUp::CopyFrom( const SwWrongList& rCopy )
{
    maSentence = ((const SwGrammarMarkUp&)rCopy).maSentence;
    SwWrongList::CopyFrom( rCopy );
}


void SwGrammarMarkUp::MoveGrammar( xub_StrLen nPos, long nDiff )
{
    Move( nPos, nDiff );
    if( !maSentence.size() )
        return;
    std::vector< xub_StrLen >::iterator pIter = maSentence.begin();
    while( pIter != maSentence.end() && *pIter < nPos )
        ++pIter;
    xub_StrLen nEnd = nDiff < 0 ? xub_StrLen(nPos - nDiff) : nPos;
    while( pIter != maSentence.end() )
    {
        if( *pIter >= nEnd )
            *pIter = xub_StrLen( *pIter + nDiff );
        else
            *pIter = nPos;
        ++pIter;
    }
}

SwGrammarMarkUp* SwGrammarMarkUp::SplitGrammarList( xub_StrLen nSplitPos )
{
    SwGrammarMarkUp* pNew = (SwGrammarMarkUp*)SplitList( nSplitPos );
    if( !maSentence.size() )
        return pNew;
    std::vector< xub_StrLen >::iterator pIter = maSentence.begin();
    while( pIter != maSentence.end() && *pIter < nSplitPos )
        ++pIter;
    if( pIter != maSentence.begin() )
    {
        if( !pNew ) {
            pNew = new SwGrammarMarkUp();
            pNew->SetInvalid( 0, STRING_LEN );
        }
        pNew->maSentence.insert( pNew->maSentence.begin(), maSentence.begin(), pIter );
        maSentence.erase( maSentence.begin(), pIter );
    }
    return pNew;
}

void SwGrammarMarkUp::JoinGrammarList( SwGrammarMarkUp* pNext, xub_StrLen nInsertPos )
{
    JoinList( pNext, nInsertPos );
    if (pNext)
    {
        if( !pNext->maSentence.size() )
            return;
        std::vector< xub_StrLen >::iterator pIter = pNext->maSentence.begin();
        while( pIter != pNext->maSentence.end() )
        {
            *pIter = *pIter + nInsertPos;
            ++pIter;
        }
        maSentence.insert( maSentence.end(), pNext->maSentence.begin(), pNext->maSentence.end() );
    }
}

void SwGrammarMarkUp::ClearGrammarList( xub_StrLen nSentenceEnd )
{
    if( STRING_LEN == nSentenceEnd ) {
        ClearList();
        maSentence.clear();
        Validate();
    } else if( GetBeginInv() <= nSentenceEnd ) {
        std::vector< xub_StrLen >::iterator pIter = maSentence.begin();
        xub_StrLen nStart = 0;
        while( pIter != maSentence.end() && *pIter < GetBeginInv() )
        {
            nStart = *pIter;
            ++pIter;
        }
        std::vector< xub_StrLen >::iterator pLast = pIter;
        while( pLast != maSentence.end() && *pLast <= nSentenceEnd )
            ++pLast;
        maSentence.erase( pIter, pLast );
        RemoveEntry( nStart, nSentenceEnd );
        SetInvalid( nSentenceEnd + 1, STRING_LEN );
    }
}

void SwGrammarMarkUp::setSentence( xub_StrLen nStart )
{
    std::vector< xub_StrLen >::iterator pIter = maSentence.begin();
    while( pIter != maSentence.end() && *pIter < nStart )
        ++pIter;
    if( pIter == maSentence.end() || *pIter > nStart )
        maSentence.insert( pIter, nStart );
}

xub_StrLen SwGrammarMarkUp::getSentenceStart( xub_StrLen nPos )
{
    if( !maSentence.size() )
        return 0;
    std::vector< xub_StrLen >::iterator pIter = maSentence.begin();
    while( pIter != maSentence.end() && *pIter < nPos )
        ++pIter;
    if( pIter != maSentence.begin() )
        --pIter;
    xub_StrLen nRet = 0;
    if( pIter != maSentence.end() && *pIter < nPos )
        nRet = *pIter;
    return nRet;
}

xub_StrLen SwGrammarMarkUp::getSentenceEnd( xub_StrLen nPos )
{
    if( !maSentence.size() )
        return STRING_LEN;
    std::vector< xub_StrLen >::iterator pIter = maSentence.begin();
    while( pIter != maSentence.end() && *pIter <= nPos )
        ++pIter;
    xub_StrLen nRet = STRING_LEN;
    if( pIter != maSentence.end() )
        nRet = *pIter;
    return nRet;
}

