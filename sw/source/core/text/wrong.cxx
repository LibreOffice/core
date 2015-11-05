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

#include "swtypes.hxx"

#include "SwGrammarMarkUp.hxx"
#include <osl/diagnose.h>

SwWrongArea::SwWrongArea( const OUString& rType, WrongListType listType,
        css::uno::Reference< css::container::XStringKeyMap > xPropertyBag,
        sal_Int32 nPos,
        sal_Int32 nLen)
: maType(rType), mxPropertyBag(xPropertyBag), mnPos(nPos), mnLen(nLen), mpSubList(0)
{
    mColor =  getWrongAreaColor(listType, xPropertyBag);
    mLineType = getWrongAreaLineType(listType, xPropertyBag);
}

SwWrongArea::SwWrongArea( const OUString& rType,
        css::uno::Reference< css::container::XStringKeyMap > xPropertyBag,
        sal_Int32 nPos,
        sal_Int32 nLen,
        SwWrongList* pSubList)
: maType(rType), mxPropertyBag(xPropertyBag), mnPos(nPos), mnLen(nLen), mpSubList(pSubList), mLineType(WRONGAREA_NONE)
{
    if (pSubList != 0)
    {
        mColor =  getWrongAreaColor(pSubList->GetWrongListType(), xPropertyBag);
        mLineType = getWrongAreaLineType(pSubList->GetWrongListType(), xPropertyBag);
    }
}

SwWrongList::SwWrongList( WrongListType eType ) :
    meType       (eType),
    nBeginInvalid(COMPLETE_STRING),  // everything correct... (the invalid area starts beyond the string)
    nEndInvalid  (COMPLETE_STRING)
{
    maList.reserve( 5 );
}

SwWrongList::~SwWrongList()
{
    ClearList();
}

SwWrongList* SwWrongList::Clone()
{
    SwWrongList* pClone = new SwWrongList( meType );
    pClone->CopyFrom( *this );
    return pClone;
}

void SwWrongList::CopyFrom( const SwWrongList& rCopy )
{
    maList = rCopy.maList;
    meType = rCopy.meType;
    nBeginInvalid = rCopy.nBeginInvalid;
    nEndInvalid = rCopy.nEndInvalid;
    for( size_t i = 0; i < maList.size(); ++i )
    {
        if( maList[i].mpSubList )
            maList[i].mpSubList = maList[i].mpSubList->Clone();
    }
}

void SwWrongList::ClearList()
{
    for ( size_t i = 0; i < maList.size(); ++i)
    {
        if (maList[i].mpSubList)
            delete maList[i].mpSubList;
        maList[i].mpSubList = NULL;
    }
    maList.clear();
}

/** If a word is incorrectly selected, this method returns begin and length of it.

    @param[in,out] rChk starting position of the word to check
    @param[out]    rLn  length of the word

    @return <true> if incorrectly selected, <false> otherwise
 */
bool SwWrongList::InWrongWord( sal_Int32 &rChk, sal_Int32 &rLn ) const
{
    const sal_uInt16 nPos = GetWrongPos( rChk );
    if ( nPos >= Count() )
        return false;
    const sal_Int32 nWrPos = Pos( nPos );
    if ( nWrPos <= rChk )
    {
        rLn = Len( nPos );
        if( nWrPos + rLn <= rChk )
            return false;
        rChk = nWrPos;
        return true;
    }
    return false;
}

/** Calculate first incorrectly selected area.

    @param[in,out] rChk starting position of the word to check
    @param[in,out] rLn  length of the word

    @return <true> if incorrectly selected area was found, <false> otherwise
 */
bool SwWrongList::Check( sal_Int32 &rChk, sal_Int32 &rLn ) const
{
    sal_uInt16 nPos = GetWrongPos( rChk );
    rLn += rChk;

    if( nPos == Count() )
        return false;

    sal_Int32 nWrPos = Pos( nPos );
    sal_Int32 nEnd = nWrPos + Len( nPos );
    if( nEnd == rChk )
    {
        ++nPos;
        if( nPos == Count() )
            return false;

        nWrPos = Pos( nPos );
        nEnd = nWrPos + Len( nPos );
    }
    if( nEnd > rChk && nWrPos < rLn )
    {
        if( nWrPos > rChk )
            rChk = nWrPos;
        if( nEnd < rLn )
            rLn = nEnd;
        rLn -= rChk;
        return 0 != rLn;
    }
    return false;
}

/** Find next incorrectly selected position.

    @param[in] rChk starting position of the word to check

    @return starting position of incorrectly selected area, <COMPLETE_STRING> otherwise
 */
sal_Int32 SwWrongList::NextWrong( sal_Int32 nChk ) const
{
    sal_Int32 nRet = COMPLETE_STRING;
    sal_uInt16 nPos = GetWrongPos( nChk );
    if( nPos < Count() )
    {
        nRet = Pos( nPos );
        if( nRet < nChk && nRet + Len( nPos ) <= nChk )
        {
            if( ++nPos < Count() )
                nRet = Pos( nPos );
            else
                nRet = COMPLETE_STRING;
        }
    }
    if( nRet > GetBeginInv() && nChk < GetEndInv() )
        nRet = nChk > GetBeginInv() ? nChk : GetBeginInv();
    return nRet;
}

/** Find the first position that is greater or equal to the given value.

    @note Resulting position might be behind the last element of the array.
    @param[in] nValue value for comparison

    @return first position that is greater or equal to the given value
 */
sal_uInt16 SwWrongList::GetWrongPos( sal_Int32 nValue ) const
{
    sal_uInt16 nMax = Count();
    sal_uInt16 nMin = 0;

    if( nMax > 0 )
    {
        // For smart tag lists, we may not use a binary search. We return the
        // position of the first smart tag which covers nValue
        if ( !maList[0].maType.isEmpty() || maList[0].mpSubList )
        {
            for (std::vector<SwWrongArea>::const_iterator aIter(maList.begin()), aEnd(maList.end()); aIter != aEnd; ++aIter)
            {
                const sal_Int32 nSTPos = (*aIter).mnPos;
                const sal_Int32 nSTLen = (*aIter).mnLen;
                if ( nSTPos <= nValue && nValue < nSTPos + nSTLen )
                    break;
                if ( nSTPos > nValue )
                    break;

                ++nMin;
            }
            return nMin;
        }

        --nMax;
        sal_uInt16 nMid = 0;
        while( nMin <= nMax )
        {
            nMid = nMin + ( nMax - nMin ) / 2;
            const sal_Int32 nTmp = Pos( nMid );
            if( nTmp == nValue )
            {
                nMin = nMid;
                break;
            }
            else if( nTmp < nValue )
            {
                if( nTmp + Len( nMid ) >= nValue )
                {
                    nMin = nMid;
                    break;
                }
                nMin = nMid + 1;
            }
            else if( nMid == 0 )
            {
                break;
            }
            else
                nMax = nMid - 1;
        }
    }

    // nMin now points to an index i into the wrong list which
    // 1. nValue is inside [ Area[i].pos, Area[i].pos + Area[i].len ] (inclusive!!!)
    // 2. nValue < Area[i].pos

    return nMin;
}

void SwWrongList::_Invalidate( sal_Int32 nBegin, sal_Int32 nEnd )
{
    if ( nBegin < GetBeginInv() )
        nBeginInvalid = nBegin;
    if ( nEnd > GetEndInv() || GetEndInv() == COMPLETE_STRING )
        nEndInvalid = nEnd;
}

void SwWrongList::SetInvalid( sal_Int32 nBegin, sal_Int32 nEnd )
{
    nBeginInvalid = nBegin;
    nEndInvalid = nEnd;
}

/** Change all values after the given position.

   Needed after insert/deletion of characters.

   @param nPos  position after that everything should be changed
   @param nDiff amount how much the positions should be moved
 */
void SwWrongList::Move( sal_Int32 nPos, sal_Int32 nDiff )
{
    sal_uInt16 i = GetWrongPos( nPos );
    if( nDiff < 0 )
    {
        const sal_Int32 nEnd = nPos - nDiff;
        sal_uInt16 nLst = i;
        bool bJump = false;
        while( nLst < Count() && Pos( nLst ) < nEnd )
            ++nLst;
        if( nLst > i )
        {
            const sal_Int32 nWrPos = Pos( nLst - 1 );
            if ( nWrPos <= nPos )
            {
                sal_Int32 nWrLen = Len( nLst - 1 );
                // calculate new length of word
                nWrLen = ( nEnd > nWrPos + nWrLen ) ?
                        nPos - nWrPos :
                        nWrLen + nDiff;
                if( nWrLen )
                {
                    maList[--nLst].mnLen = nWrLen;
                    bJump = true;
                }
            }
        }
        Remove( i, nLst - i );

        if ( bJump )
            ++i;
        if( COMPLETE_STRING == GetBeginInv() )
            SetInvalid( nPos ? nPos - 1 : nPos, nPos + 1 );
        else
        {
            ShiftLeft( nBeginInvalid, nPos, nEnd );
            if( nEndInvalid != COMPLETE_STRING )
                ShiftLeft( nEndInvalid, nPos, nEnd );
            _Invalidate( nPos ? nPos - 1 : nPos, nPos + 1 );
        }
    }
    else
    {
        const sal_Int32 nEnd = nPos + nDiff;
        if( COMPLETE_STRING != GetBeginInv() )
        {
            if( nBeginInvalid > nPos )
                nBeginInvalid += nDiff;
            if( nEndInvalid >= nPos && nEndInvalid != COMPLETE_STRING )
                nEndInvalid += nDiff;
        }
        // If the pointer is in the middle of a wrong word,
        // invalidation must happen from the beginning of that word.
        if( i < Count() )
        {
            const sal_Int32 nWrPos = Pos( i );
            if (nPos >= nWrPos)
            {
                Invalidate( nWrPos, nEnd );
                const sal_Int32 nWrLen = Len( i ) + nDiff;
                maList[i++].mnLen = nWrLen;
                Invalidate( nWrPos, nWrPos + nWrLen );
            }
        }
        else
            Invalidate( nPos, nEnd );
    }
    while( i < Count() )
    {
        maList[i++].mnPos += nDiff;
    }
}

// TODO: Complete documentation
/** Remove given range of entries

   For a given range [nPos, nPos + nLen[ and an index nIndex, this function
   basically counts the number of SwWrongArea entries starting with nIndex
   up to nPos + nLen. All these entries are removed.

   @param rStart     ???
   @param rEnd       ???
   @param nPos       starting position of the range
   @param nLen       length of the range
   @param nIndex     index to start lookup at
   @param nCursorPos ???

   @return <true> if ???
 */
auto SwWrongList::Fresh( sal_Int32 &rStart, sal_Int32 &rEnd, sal_Int32 nPos,
     sal_Int32 nLen, sal_uInt16 nIndex, sal_Int32 nCursorPos ) -> FreshState
{
    // length of word must be greater than 0
    // only report a spelling error if the cursor position is outside the word,
    // so that the user is not annoyed while typing
    FreshState eRet = (nLen)
        ? (nCursorPos > nPos + nLen || nCursorPos < nPos)
            ? FreshState::FRESH
            : FreshState::CURSOR
        : FreshState::NOTHING;

    sal_Int32 nWrPos = 0;
    sal_Int32 nWrEnd = rEnd;
    sal_uInt16 nCnt = nIndex;
    if( nCnt < Count() )
    {
        nWrPos = Pos( nCnt );
        if( nWrPos < nPos && rStart > nWrPos )
            rStart = nWrPos;
    }

    while( nCnt < Count() )
    {
        nWrPos = Pos( nCnt );
        if ( nWrPos >= nPos )
            break;
        nWrEnd = nWrPos + Len( nCnt++ );
    }

    if( nCnt < Count() && nWrPos == nPos && Len( nCnt ) == nLen )
    {
        ++nCnt;
        eRet = FreshState::FRESH;
    }
    else
    {
        if (FreshState::FRESH == eRet)
        {
            if( rStart > nPos )
                rStart = nPos;
            nWrEnd = nPos + nLen;
        }
    }

    nPos += nLen;

    if( nCnt < Count() )
    {
        nWrPos = Pos( nCnt );
        if( nWrPos < nPos && rStart > nWrPos )
            rStart = nWrPos;
    }

    while( nCnt < Count() )
    {
        nWrPos = Pos( nCnt );
        if ( nWrPos >= nPos )
            break;
        nWrEnd = nWrPos + Len( nCnt++ );
    }

    if( rEnd < nWrEnd )
        rEnd = nWrEnd;

    Remove( nIndex, nCnt - nIndex );

    return eRet;
}

void SwWrongList::Invalidate( sal_Int32 nBegin, sal_Int32 nEnd )
{
    if (COMPLETE_STRING == GetBeginInv())
        SetInvalid( nBegin, nEnd );
    else
        _Invalidate( nBegin, nEnd );
}

bool SwWrongList::InvalidateWrong( )
{
    if( Count() )
    {
        const sal_Int32 nFirst = Pos( 0 );
        const sal_Int32 nLast = Pos( Count() - 1 ) + Len( Count() - 1 );
        Invalidate( nFirst, nLast );
        return true;
    }
    return false;
}

SwWrongList* SwWrongList::SplitList( sal_Int32 nSplitPos )
{
    SwWrongList *pRet = NULL;
    sal_uInt16 nLst = 0;
    while( nLst < Count() && Pos( nLst ) < nSplitPos )
        ++nLst;
    if( nLst )
    {
        sal_Int32 nWrPos = Pos( nLst - 1 );
        sal_Int32 nWrLen = Len( nLst - 1 );
        if ( nWrPos+nWrLen > nSplitPos )
        {
            nWrLen += nWrPos - nSplitPos;
            maList[--nLst].mnPos = nSplitPos;
            maList[nLst].mnLen = nWrLen;
        }
    }
    if( nLst )
    {
        if( WRONGLIST_GRAMMAR == GetWrongListType() )
            pRet = new SwGrammarMarkUp();
        else
            pRet = new SwWrongList( GetWrongListType() );
        pRet->Insert(0, maList.begin(), ( nLst >= maList.size() ? maList.end() : maList.begin() + nLst ) );
        pRet->SetInvalid( GetBeginInv(), GetEndInv() );
        pRet->_Invalidate( nSplitPos ? nSplitPos - 1 : nSplitPos, nSplitPos );
        Remove( 0, nLst );
    }
    if( COMPLETE_STRING == GetBeginInv() )
        SetInvalid( 0, 1 );
    else
    {
        ShiftLeft( nBeginInvalid, 0, nSplitPos );
        if( nEndInvalid != COMPLETE_STRING )
            ShiftLeft( nEndInvalid, 0, nSplitPos );
        _Invalidate( 0, 1 );
    }
    for (nLst = 0; nLst < Count(); ++nLst )
    {
        maList[nLst].mnPos -= nSplitPos;
    }
    return pRet;
}

void SwWrongList::JoinList( SwWrongList* pNext, sal_Int32 nInsertPos )
{
    if (pNext)
    {
        OSL_ENSURE( GetWrongListType() == pNext->GetWrongListType(), "type mismatch with next list" );
    }
    if( pNext )
    {
        sal_uInt16 nCnt = Count();
        pNext->Move( 0, nInsertPos );
        Insert(nCnt, pNext->maList.begin(), pNext->maList.end());

        Invalidate( pNext->GetBeginInv(), pNext->GetEndInv() );
        if( nCnt && Count() > nCnt )
        {
            sal_Int32 nWrPos = Pos( nCnt );
            sal_Int32 nWrLen = Len( nCnt );
            if( !nWrPos )
            {
                nWrPos += nInsertPos;
                nWrLen -= nInsertPos;
                maList[nCnt].mnPos = nWrPos;
                maList[nCnt].mnLen = nWrLen;
            }
            if( nWrPos == Pos( nCnt - 1 ) + Len( nCnt - 1 ) )
            {
                nWrLen += Len( nCnt - 1 );
                maList[nCnt - 1].mnLen = nWrLen;
                Remove( nCnt, 1 );
            }
        }
    }
    Invalidate( nInsertPos ? nInsertPos - 1 : nInsertPos, nInsertPos + 1 );
}

void SwWrongList::InsertSubList( sal_Int32 nNewPos, sal_Int32 nNewLen, sal_uInt16 nWhere, SwWrongList* pSubList )
{
    if (pSubList)
    {
        OSL_ENSURE( GetWrongListType() == pSubList->GetWrongListType(), "type mismatch with sub list" );
    }
    std::vector<SwWrongArea>::iterator i = maList.begin();
    if ( nWhere >= maList.size() )
        i = maList.end(); // robust
    else
        i += nWhere;
    maList.insert(i, SwWrongArea( OUString(), 0, nNewPos, nNewLen, pSubList ) );
}

// New functions: Necessary because SwWrongList has been changed to use std::vector
void SwWrongList::Insert(sal_uInt16 nWhere, std::vector<SwWrongArea>::iterator startPos, std::vector<SwWrongArea>::iterator endPos)
{
    std::vector<SwWrongArea>::iterator i = maList.begin();
    if ( nWhere >= maList.size() )
        i = maList.end(); // robust
    else
        i += nWhere;
    maList.insert(i, startPos, endPos); // insert [startPos, endPos[ before i

    // ownership of the sublist is passed to maList, therefore we have to set the
    // pSubList-Pointers to 0
    while ( startPos != endPos )
    {
        (*startPos).mpSubList = 0;
        ++startPos;
    }
}

void SwWrongList::Remove(sal_uInt16 nIdx, sal_uInt16 nLen )
{
    if ( nIdx >= maList.size() ) return;
    std::vector<SwWrongArea>::iterator i1 = maList.begin();
    i1 += nIdx;

    std::vector<SwWrongArea>::iterator i2 = i1;
    if ( nIdx + nLen >= static_cast<sal_uInt16>(maList.size()) )
        i2 = maList.end(); // robust
    else
        i2 += nLen;

    std::vector<SwWrongArea>::iterator iLoop = i1;
    while ( iLoop != i2 )
    {
        if ( (*iLoop).mpSubList )
            delete (*iLoop).mpSubList;
        ++iLoop;
    }

#if OSL_DEBUG_LEVEL > 0
    const int nOldSize = Count();
    (void) nOldSize;
#endif

    maList.erase(i1, i2);

#if OSL_DEBUG_LEVEL > 0
    OSL_ENSURE( Count() + nLen == nOldSize, "SwWrongList::Remove() trouble" );
#endif
}

void SwWrongList::RemoveEntry( sal_Int32 nBegin, sal_Int32 nEnd ) {
    sal_uInt16 nDelPos = 0;
    sal_uInt16 nDel = 0;
    std::vector<SwWrongArea>::const_iterator aIter(maList.begin()), aEnd(maList.end());
    while( aIter != aEnd && (*aIter).mnPos < nBegin )
    {
        ++aIter;
        ++nDelPos;
    }
    if( WRONGLIST_GRAMMAR == GetWrongListType() )
    {
        while( aIter != aEnd && nBegin < nEnd && nEnd > (*aIter).mnPos )
        {
            ++aIter;
            ++nDel;
        }
    }
    else
    {
        while( aIter != aEnd && nBegin == (*aIter).mnPos && nEnd == (*aIter).mnPos +(*aIter).mnLen )
        {
            ++aIter;
            ++nDel;
        }
    }
    if( nDel )
        Remove( nDelPos, nDel );
}

bool SwWrongList::LookForEntry( sal_Int32 nBegin, sal_Int32 nEnd ) {
    std::vector<SwWrongArea>::iterator aIter = maList.begin();
    while( aIter != maList.end() && (*aIter).mnPos < nBegin )
        ++aIter;
    if( aIter != maList.end() && nBegin == (*aIter).mnPos && nEnd == (*aIter).mnPos +(*aIter).mnLen )
        return true;
    return false;
}

void SwWrongList::Insert( const OUString& rType,
                          css::uno::Reference< css::container::XStringKeyMap > xPropertyBag,
                          sal_Int32 nNewPos, sal_Int32 nNewLen )
{
    std::vector<SwWrongArea>::iterator aIter = maList.begin();

    while ( aIter != maList.end() )
    {
        const sal_Int32 nSTPos = (*aIter).mnPos;

        if ( nNewPos < nSTPos )
        {
            // insert at current position
            break;
        }
        else if ( nNewPos == nSTPos )
        {
            while ( aIter != maList.end() && (*aIter).mnPos == nSTPos )
            {
                if ( nNewLen < (*aIter).mnLen )
                {
                    // insert at current position
                    break;
                }
                ++aIter;
            }
            break;
        }
        ++aIter;
    }

    maList.insert(aIter, SwWrongArea( rType, meType, xPropertyBag, nNewPos, nNewLen) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
