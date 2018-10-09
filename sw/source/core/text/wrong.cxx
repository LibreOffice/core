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

#include <swtypes.hxx>

#include <SwGrammarMarkUp.hxx>
#include <ndtxt.hxx>
#include <txtfrm.hxx>

#include <osl/diagnose.h>

SwWrongArea::SwWrongArea( const OUString& rType, WrongListType listType,
        css::uno::Reference< css::container::XStringKeyMap > const & xPropertyBag,
        sal_Int32 nPos,
        sal_Int32 nLen)
: maType(rType), mxPropertyBag(xPropertyBag), mnPos(nPos), mnLen(nLen), mpSubList(nullptr)
{
    mColor =  getWrongAreaColor(listType, xPropertyBag);
    mLineType = getWrongAreaLineType(listType, xPropertyBag);
}

SwWrongArea::SwWrongArea( const OUString& rType,
        css::uno::Reference< css::container::XStringKeyMap > const & xPropertyBag,
        sal_Int32 nPos,
        sal_Int32 nLen,
        SwWrongList* pSubList)
: maType(rType), mxPropertyBag(xPropertyBag), mnPos(nPos), mnLen(nLen), mpSubList(pSubList), mLineType(WRONGAREA_NONE)
{
    if (pSubList != nullptr)
    {
        mColor =  getWrongAreaColor(pSubList->GetWrongListType(), xPropertyBag);
        mLineType = getWrongAreaLineType(pSubList->GetWrongListType(), xPropertyBag);
    }
}

SwWrongList::SwWrongList( WrongListType eType ) :
    meType       (eType),
    mnBeginInvalid(COMPLETE_STRING),  // everything correct... (the invalid area starts beyond the string)
    mnEndInvalid  (COMPLETE_STRING)
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
    mnBeginInvalid = rCopy.mnBeginInvalid;
    mnEndInvalid = rCopy.mnEndInvalid;
    for(SwWrongArea & i : maList)
    {
        if( i.mpSubList )
            i.mpSubList = i.mpSubList->Clone();
    }
}

void SwWrongList::ClearList()
{
    for (SwWrongArea & i : maList)
    {
        delete i.mpSubList;
        i.mpSubList = nullptr;
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
        nRet = std::max(nChk, GetBeginInv());
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

void SwWrongList::Invalidate_( sal_Int32 nBegin, sal_Int32 nEnd )
{
    if ( nBegin < GetBeginInv() )
        mnBeginInvalid = nBegin;
    if ( nEnd > GetEndInv() || GetEndInv() == COMPLETE_STRING )
        mnEndInvalid = nEnd;
}

void SwWrongList::SetInvalid( sal_Int32 nBegin, sal_Int32 nEnd )
{
    mnBeginInvalid = nBegin;
    mnEndInvalid = nEnd;
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
            ShiftLeft( mnBeginInvalid, nPos, nEnd );
            if( mnEndInvalid != COMPLETE_STRING )
                ShiftLeft( mnEndInvalid, nPos, nEnd );
            Invalidate_( nPos ? nPos - 1 : nPos, nPos + 1 );
        }
    }
    else
    {
        const sal_Int32 nEnd = nPos + nDiff;
        if( COMPLETE_STRING != GetBeginInv() )
        {
            if( mnBeginInvalid > nPos )
                mnBeginInvalid += nDiff;
            if( mnEndInvalid >= nPos && mnEndInvalid != COMPLETE_STRING )
                mnEndInvalid += nDiff;
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
    FreshState eRet = nLen
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
        Invalidate_( nBegin, nEnd );
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
    SwWrongList *pRet = nullptr;
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
        pRet->Invalidate_( nSplitPos ? nSplitPos - 1 : nSplitPos, nSplitPos );
        Remove( 0, nLst );
    }
    if( COMPLETE_STRING == GetBeginInv() )
        SetInvalid( 0, 1 );
    else
    {
        ShiftLeft( mnBeginInvalid, 0, nSplitPos );
        if( mnEndInvalid != COMPLETE_STRING )
            ShiftLeft( mnEndInvalid, 0, nSplitPos );
        Invalidate_( 0, 1 );
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
    maList.insert(i, SwWrongArea( OUString(), nullptr, nNewPos, nNewLen, pSubList ) );
}

// New functions: Necessary because SwWrongList has been changed to use std::vector
void SwWrongList::Insert(sal_uInt16 nWhere, std::vector<SwWrongArea>::iterator startPos, std::vector<SwWrongArea>::iterator const & endPos)
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
        (*startPos).mpSubList = nullptr;
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
        delete (*iLoop).mpSubList;
        ++iLoop;
    }

#if OSL_DEBUG_LEVEL > 0
    const int nOldSize = Count();
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
    return aIter != maList.end()
           && nBegin == (*aIter).mnPos
           && nEnd == (*aIter).mnPos + (*aIter).mnLen;
}

void SwWrongList::Insert( const OUString& rType,
                          css::uno::Reference< css::container::XStringKeyMap > const & xPropertyBag,
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

namespace sw {

WrongListIteratorBase::WrongListIteratorBase(SwTextFrame const& rFrame,
        SwWrongList const* (SwTextNode::*pGetWrongList)() const)
    : m_pGetWrongList(pGetWrongList)
    , m_pMergedPara(rFrame.GetMergedPara())
    , m_CurrentExtent(0)
    , m_CurrentIndex(0)
    , m_CurrentNodeIndex(0)
    , m_pWrongList(m_pMergedPara
                    ? nullptr
                    : (rFrame.GetTextNodeFirst()->*pGetWrongList)())
{
}

WrongListIteratorBase::WrongListIteratorBase(SwWrongList const& rWrongList)
    : m_pGetWrongList(nullptr)
    , m_pMergedPara(nullptr)
    , m_CurrentExtent(0)
    , m_CurrentIndex(0)
    , m_CurrentNodeIndex(0)
    , m_pWrongList(&rWrongList)
{
}

WrongListIterator::WrongListIterator(SwTextFrame const& rFrame,
        SwWrongList const* (SwTextNode::*pGetWrongList)() const)
    : WrongListIteratorBase(rFrame, pGetWrongList)
{
}

WrongListIterator::WrongListIterator(SwWrongList const& rWrongList)
    : WrongListIteratorBase(rWrongList)
{
}

bool WrongListIterator::Check(TextFrameIndex & rStart, TextFrameIndex & rLen)
{
    if (m_pMergedPara)
    {
        if (rStart < m_CurrentIndex)
        {   // rewind
            m_CurrentExtent = 0;
            m_CurrentIndex = TextFrameIndex(0);
            m_CurrentNodeIndex = TextFrameIndex(0);
        }
        while (m_CurrentExtent < m_pMergedPara->extents.size())
        {
            sw::Extent const& rExtent(m_pMergedPara->extents[m_CurrentExtent]);
            if (rStart + rLen <= m_CurrentIndex)
            {
                return false;
            }
            else if (rStart < m_CurrentIndex)
            {
                rLen -= (m_CurrentIndex - rStart);
                assert(0 < sal_Int32(rLen));
                rStart = m_CurrentIndex;
            }
            if (m_CurrentIndex <= rStart &&
                rStart < m_CurrentIndex + TextFrameIndex(rExtent.nEnd - rExtent.nStart))
            {
                SwWrongList const*const pWrongList((rExtent.pNode->*m_pGetWrongList)());
                // found the extent containing start - first, call Check
                sal_Int32 nStart(rExtent.nStart + sal_Int32(rStart - m_CurrentIndex)); // (m_CurrentIndex - m_CurrentNodeIndex));
                sal_Int32 nLen;
                if (sal_Int32(rLen) < rExtent.nEnd - nStart)
                {
                    nLen = sal_Int32(rLen);
                }
                else
                {
                    sal_Int32 nInLen(rLen);
                    nLen = rExtent.nEnd - nStart;
                    nInLen -= nLen;
                    for (size_t i = m_CurrentExtent + 1;
                         i < m_pMergedPara->extents.size(); ++i)
                    {
                        sw::Extent const& rExtentEnd(m_pMergedPara->extents[i]);
                        if (rExtentEnd.pNode != rExtent.pNode)
                        {
                            nInLen = 0;
                            break;
                        }
                        // add gap too
                        nLen += rExtentEnd.nStart - m_pMergedPara->extents[i-1].nEnd;
                        if (nInLen <= rExtentEnd.nEnd - rExtentEnd.nStart)
                        {
                            nLen += nInLen;
                            nInLen = 0;
                            break;
                        }
                        nLen += rExtentEnd.nEnd - rExtentEnd.nStart;
                        nInLen -= rExtentEnd.nEnd - rExtentEnd.nStart;
                    }
                }
                if (pWrongList && pWrongList->Check(nStart, nLen))
                {
                    // check if there's overlap with this extent
                    if (rExtent.nStart <= nStart && nStart < rExtent.nEnd)
                    {
                        // yes - now compute end position / length
                        sal_Int32 const nEnd(nStart + nLen);
                        rStart = m_CurrentIndex + TextFrameIndex(nStart - rExtent.nStart);
                        TextFrameIndex const nOrigLen(rLen);
                        if (nEnd <= rExtent.nEnd)
                        {
                            rLen = TextFrameIndex(nEnd - nStart);
                        }
                        else // have to search other extents for the end...
                        {
                            rLen = TextFrameIndex(rExtent.nEnd - nStart);
                            for (size_t i = m_CurrentExtent + 1;
                                 i < m_pMergedPara->extents.size(); ++i)
                            {
                                sw::Extent const& rExtentEnd(m_pMergedPara->extents[i]);
                                if (rExtentEnd.pNode != rExtent.pNode
                                    || nEnd <= rExtentEnd.nStart)
                                {
                                    break;
                                }
                                if (nEnd <= rExtentEnd.nEnd)
                                {
                                    rLen += TextFrameIndex(nEnd - rExtentEnd.nStart);
                                    break;
                                }
                                rLen += TextFrameIndex(rExtentEnd.nEnd - rExtentEnd.nStart);
                            }
                        }
                        assert(rLen <= nOrigLen); (void) nOrigLen;
                        return true;
                    }
                }
            }
            m_CurrentIndex += TextFrameIndex(rExtent.nEnd - rExtent.nStart);
            ++m_CurrentExtent;
            if (m_CurrentExtent < m_pMergedPara->extents.size() &&
                rExtent.pNode != m_pMergedPara->extents[m_CurrentExtent].pNode)
            {
                m_CurrentNodeIndex = m_CurrentIndex; // reset
            }
        }
        return false;
    }
    else if (m_pWrongList)
    {
        sal_Int32 nStart(rStart);
        sal_Int32 nLen(rLen);
        bool const bRet(m_pWrongList->Check(nStart, nLen));
        rStart = TextFrameIndex(nStart);
        rLen = TextFrameIndex(nLen);
        return bRet;
    }
    return false;
}

const SwWrongArea*
WrongListIterator::GetWrongElement(TextFrameIndex const nStart)
{
    if (m_pMergedPara)
    {
        if (nStart < m_CurrentIndex)
        {   // rewind
            m_CurrentExtent = 0;
            m_CurrentIndex = TextFrameIndex(0);
            m_CurrentNodeIndex = TextFrameIndex(0);
        }
        while (m_CurrentExtent < m_pMergedPara->extents.size())
        {
            sw::Extent const& rExtent(m_pMergedPara->extents[m_CurrentExtent]);
            if (m_CurrentIndex <= nStart &&
                nStart <= m_CurrentIndex + TextFrameIndex(rExtent.nEnd - rExtent.nStart))
            {
                // note: the returned object isn't wrapped because fntcache.cxx
                // does not look at its positions, only its formatting props
                SwWrongList const*const pWrongList((rExtent.pNode->*m_pGetWrongList)());
                if (pWrongList)
                {
                    sal_Int32 const nNStart(rExtent.nStart + sal_Int32(nStart - m_CurrentIndex)); // (m_CurrentIndex - m_CurrentNodeIndex));
                    sal_Int16 const nPos(pWrongList->GetWrongPos(nNStart));
                    return pWrongList->GetElement(nPos);
                }
            }
            m_CurrentIndex += TextFrameIndex(rExtent.nEnd - rExtent.nStart);
            ++m_CurrentExtent;
            if (m_CurrentExtent < m_pMergedPara->extents.size() &&
                rExtent.pNode != m_pMergedPara->extents[m_CurrentExtent].pNode)
            {
                m_CurrentNodeIndex = m_CurrentIndex; // reset
            }
        }
        return nullptr;
    }
    else if (m_pWrongList)
    {
        sal_Int16 const nPos(m_pWrongList->GetWrongPos(sal_Int32(nStart)));
        return m_pWrongList->GetElement(nPos);
    }
    return nullptr;
}

WrongListIteratorCounter::WrongListIteratorCounter(SwTextFrame const& rFrame,
        SwWrongList const* (SwTextNode::*pGetWrongList)() const)
    : WrongListIteratorBase(rFrame, pGetWrongList)
{
}

WrongListIteratorCounter::WrongListIteratorCounter(SwWrongList const& rWrongList)
    : WrongListIteratorBase(rWrongList)
{
}

sal_uInt16 WrongListIteratorCounter::GetElementCount()
{
    if (m_pMergedPara)
    {
        sal_uInt16 nRet(0);
        m_CurrentExtent = 0;
        m_CurrentIndex = TextFrameIndex(0);
        SwNode const* pNode(nullptr);
        sal_uInt16 InCurrentNode(0);
        while (m_CurrentExtent < m_pMergedPara->extents.size())
        {
            sw::Extent const& rExtent(m_pMergedPara->extents[m_CurrentExtent]);
            if (rExtent.pNode != pNode)
            {
                InCurrentNode = 0;
                pNode = rExtent.pNode;
            }
            SwWrongList const*const pWrongList((rExtent.pNode->*m_pGetWrongList)());
            for (; pWrongList && InCurrentNode < pWrongList->Count(); ++InCurrentNode)
            {
                SwWrongArea const*const pWrong(pWrongList->GetElement(InCurrentNode));
                TextFrameIndex const nExtentEnd(
                    m_CurrentIndex + TextFrameIndex(rExtent.nEnd - rExtent.nStart));
                if (nExtentEnd <= TextFrameIndex(pWrong->mnPos))
                {
                    break; // continue outer loop
                }
                if (m_CurrentIndex < TextFrameIndex(pWrong->mnPos + pWrong->mnLen))
                {
                    ++nRet;
                }
            }
            m_CurrentIndex += TextFrameIndex(rExtent.nEnd - rExtent.nStart);
            ++m_CurrentExtent;
        }
        return nRet;
    }
    else if (m_pWrongList)
    {
        return m_pWrongList->Count();
    }
    return 0;
}

boost::optional<std::pair<TextFrameIndex, TextFrameIndex>>
WrongListIteratorCounter::GetElementAt(sal_uInt16 nIndex)
{
    if (m_pMergedPara)
    {
        m_CurrentExtent = 0;
        m_CurrentIndex = TextFrameIndex(0);
        SwNode const* pNode(nullptr);
        sal_uInt16 InCurrentNode(0);
        while (m_CurrentExtent < m_pMergedPara->extents.size())
        {
            sw::Extent const& rExtent(m_pMergedPara->extents[m_CurrentExtent]);
            if (rExtent.pNode != pNode)
            {
                InCurrentNode = 0;
                pNode = rExtent.pNode;
            }
            SwWrongList const*const pWrongList((rExtent.pNode->*m_pGetWrongList)());
            for (; pWrongList && InCurrentNode < pWrongList->Count(); ++InCurrentNode)
            {
                SwWrongArea const*const pWrong(pWrongList->GetElement(InCurrentNode));
                TextFrameIndex const nExtentEnd(
                    m_CurrentIndex + TextFrameIndex(rExtent.nEnd - rExtent.nStart));
                if (nExtentEnd <= TextFrameIndex(pWrong->mnPos))
                {
                    break; // continue outer loop
                }
                if (m_CurrentIndex < TextFrameIndex(pWrong->mnPos + pWrong->mnLen))
                {
                    if (nIndex == 0)
                    {
                        return boost::optional<std::pair<TextFrameIndex, TextFrameIndex>>(
                            std::pair<TextFrameIndex, TextFrameIndex>(
                                m_CurrentIndex - TextFrameIndex(rExtent.nStart -
                                    std::max(rExtent.nStart, pWrong->mnPos)),
                                m_CurrentIndex - TextFrameIndex(rExtent.nStart -
                                    std::min(pWrong->mnPos + pWrong->mnLen, rExtent.nEnd))));
                    }
                    --nIndex;
                }
            }
            m_CurrentIndex += TextFrameIndex(rExtent.nEnd - rExtent.nStart);
            ++m_CurrentExtent;
        }
        return boost::optional<std::pair<TextFrameIndex, TextFrameIndex>>();
    }
    else if (m_pWrongList)
    {
        SwWrongArea const*const pWrong(m_pWrongList->GetElement(nIndex));
        return boost::optional<std::pair<TextFrameIndex, TextFrameIndex>>(
            std::pair<TextFrameIndex, TextFrameIndex>(
                    TextFrameIndex(pWrong->mnPos),
                    TextFrameIndex(pWrong->mnPos + pWrong->mnLen)));
    }
    return boost::optional<std::pair<TextFrameIndex, TextFrameIndex>>();
}

} // namespace sw

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
