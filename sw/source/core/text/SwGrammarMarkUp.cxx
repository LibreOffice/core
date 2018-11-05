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

#include <SwGrammarMarkUp.hxx>

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
    maSentence = static_cast<const SwGrammarMarkUp&>(rCopy).maSentence;
    SwWrongList::CopyFrom( rCopy );
}

void SwGrammarMarkUp::MoveGrammar( sal_Int32 nPos, sal_Int32 nDiff )
{
    Move( nPos, nDiff );
    if( maSentence.empty() )
        return;
    auto pIter = std::find_if(maSentence.begin(), maSentence.end(),
        [nPos](const sal_Int32& rPos) { return rPos >= nPos; });
    const sal_Int32 nEnd = nDiff < 0 ? nPos-nDiff : nPos;
    while( pIter != maSentence.end() )
    {
        if( *pIter >= nEnd )
            *pIter += nDiff;
        else
            *pIter = nPos;
        ++pIter;
    }
}

SwGrammarMarkUp* SwGrammarMarkUp::SplitGrammarList( sal_Int32 nSplitPos )
{
    SwGrammarMarkUp* pNew = static_cast<SwGrammarMarkUp*>(SplitList( nSplitPos ));
    if( maSentence.empty() )
        return pNew;
    auto pIter = std::find_if(maSentence.begin(), maSentence.end(),
        [nSplitPos](const sal_Int32& rPos) { return rPos >= nSplitPos; });
    if( pIter != maSentence.begin() )
    {
        if( !pNew ) {
            pNew = new SwGrammarMarkUp();
            pNew->SetInvalid( 0, COMPLETE_STRING );
        }
        pNew->maSentence.insert( pNew->maSentence.begin(), maSentence.begin(), pIter );
        maSentence.erase( maSentence.begin(), pIter );
    }
    return pNew;
}

void SwGrammarMarkUp::JoinGrammarList( SwGrammarMarkUp* pNext, sal_Int32 nInsertPos )
{
    JoinList( pNext, nInsertPos );
    if (pNext)
    {
        if( pNext->maSentence.empty() )
            return;
        for( auto& rPos : pNext->maSentence )
        {
            rPos += nInsertPos;
        }
        maSentence.insert( maSentence.end(), pNext->maSentence.begin(), pNext->maSentence.end() );
    }
}

void SwGrammarMarkUp::ClearGrammarList( sal_Int32 nSentenceEnd )
{
    if( COMPLETE_STRING == nSentenceEnd ) {
        ClearList();
        maSentence.clear();
        Validate();
    } else if( GetBeginInv() <= nSentenceEnd ) {
        std::vector< sal_Int32 >::iterator pIter = maSentence.begin();
        sal_Int32 nStart = 0;
        while( pIter != maSentence.end() && *pIter < GetBeginInv() )
        {
            nStart = *pIter;
            ++pIter;
        }
        auto pLast = std::find_if(pIter, maSentence.end(),
            [nSentenceEnd](const sal_Int32& rPos) { return rPos > nSentenceEnd; });
        maSentence.erase( pIter, pLast );
        RemoveEntry( nStart, nSentenceEnd );
        SetInvalid( nSentenceEnd + 1, COMPLETE_STRING );
    }
}

void SwGrammarMarkUp::setSentence( sal_Int32 nStart )
{
    auto pIter = std::find_if(maSentence.begin(), maSentence.end(),
        [nStart](const sal_Int32& rPos) { return rPos >= nStart; });
    if( pIter == maSentence.end() || *pIter > nStart )
        maSentence.insert( pIter, nStart );
}

sal_Int32 SwGrammarMarkUp::getSentenceStart( sal_Int32 nPos )
{
    if( maSentence.empty() )
        return 0;
    auto pIter = std::find_if(maSentence.begin(), maSentence.end(),
        [nPos](const sal_Int32& rPos) { return rPos >= nPos; });
    if( pIter != maSentence.begin() )
        --pIter;
    if( pIter != maSentence.end() && *pIter < nPos )
        return *pIter;
    return 0;
}

sal_Int32 SwGrammarMarkUp::getSentenceEnd( sal_Int32 nPos )
{
    if( maSentence.empty() )
        return COMPLETE_STRING;
    auto pIter = std::find_if(maSentence.begin(), maSentence.end(),
        [nPos](const sal_Int32& rPos) { return rPos > nPos; });
    if( pIter != maSentence.end() )
        return *pIter;
    return COMPLETE_STRING;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
