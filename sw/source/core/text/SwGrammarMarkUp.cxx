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

