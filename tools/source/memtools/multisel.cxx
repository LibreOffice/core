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

#include <tools/debug.hxx>
#include <tools/multisel.hxx>

#include <rtl/ustrbuf.hxx>

void MultiSelection::ImplClear()
{
    // no selected indexes
    nSelCount = 0;

    for ( size_t i = 0, n = aSels.size(); i < n; ++i ) {
        delete aSels[ i ];
    }
    aSels.clear();
}

size_t MultiSelection::ImplFindSubSelection( long nIndex ) const
{
    // iterate through the sub selections
    size_t n = 0;
    for ( ;
          n < aSels.size() && nIndex > aSels[ n ]->Max();
          ++n ) {} /* empty loop */
    return n;
}

bool MultiSelection::ImplMergeSubSelections( size_t nPos1, size_t nPos2 )
{
    // didn't a sub selection at nPos2 exist?
    if ( nPos2 >= aSels.size() )
        return false;

    // did the sub selections touch each other?
    if ( (aSels[ nPos1 ]->Max() + 1) == aSels[ nPos2 ]->Min() )
    {
        // merge them
        aSels[ nPos1 ]->Max() = aSels[ nPos2 ]->Max();
        ImpSelList::iterator it = aSels.begin();
        ::std::advance( it, nPos2 );
        delete *it;
        aSels.erase( it );
        return true;
    }

    return false;
}

MultiSelection::MultiSelection():
    aTotRange( 0, -1 ),
    nCurSubSel(0),
    nCurIndex(0),
    nSelCount(0),
    bInverseCur(false),
    bCurValid(false),
    bSelectNew(false)
{
}

MultiSelection::MultiSelection( const MultiSelection& rOrig ) :
    aTotRange(rOrig.aTotRange),
    nSelCount(rOrig.nSelCount),
    bCurValid(rOrig.bCurValid),
    bSelectNew(false)
{
    if ( bCurValid )
    {
        nCurSubSel = rOrig.nCurSubSel;
        nCurIndex = rOrig.nCurIndex;
        bInverseCur = rOrig.bInverseCur;
    }
    else
    {
        nCurSubSel = 0;
        nCurIndex = 0;
        bInverseCur = false;
    }

    // copy the sub selections
    for ( size_t n = 0; n < rOrig.aSels.size(); ++n )
        aSels.push_back( new Range( *rOrig.aSels[ n ] ) );
}

MultiSelection::MultiSelection( const Range& rRange ):
    aTotRange(rRange),
    nCurSubSel(0),
    nCurIndex(0),
    nSelCount(0),
    bInverseCur(false),
    bCurValid(false),
    bSelectNew(false)
{
}

MultiSelection::~MultiSelection()
{
    for ( size_t i = 0, n = aSels.size(); i < n; ++i )
        delete aSels[ i ];
    aSels.clear();
}

MultiSelection& MultiSelection::operator= ( const MultiSelection& rOrig )
{
    aTotRange = rOrig.aTotRange;
    bCurValid = rOrig.bCurValid;
    if ( bCurValid )
    {
        nCurSubSel = rOrig.nCurSubSel;
        nCurIndex = rOrig.nCurIndex;
    }

    // clear the old and copy the sub selections
    ImplClear();
    for ( size_t n = 0; n < rOrig.aSels.size(); ++n )
        aSels.push_back( new Range( *rOrig.aSels[ n ] ) );
    nSelCount = rOrig.nSelCount;

    return *this;
}

bool MultiSelection::operator== ( MultiSelection& rWith )
{
    if ( aTotRange != rWith.aTotRange || nSelCount != rWith.nSelCount ||
         aSels.size() != rWith.aSels.size() )
        return false;

    // compare the sub selections
    for ( size_t n = 0; n < aSels.size(); ++n )
        if ( *aSels[ n ] != *rWith.aSels[ n ] )
            return false;
    return true;
}

void MultiSelection::SelectAll( bool bSelect )
{
    ImplClear();
    if ( bSelect )
    {
        aSels.push_back( new Range(aTotRange) );
        nSelCount = aTotRange.Len();
    }
}

bool MultiSelection::Select( long nIndex, bool bSelect )
{
    DBG_ASSERT( aTotRange.IsInside(nIndex), "selected index out of range" );

    // out of range?
    if ( !aTotRange.IsInside(nIndex) )
        return false;

    // find the virtual target position
    size_t nSubSelPos = ImplFindSubSelection( nIndex );

    if ( bSelect )
    {
        // is it included in the found sub selection?
        if ( nSubSelPos < aSels.size() && aSels[ nSubSelPos ]->IsInside( nIndex ) )
            // already selected, nothing to do
            return false;

        // it will become selected
        ++nSelCount;

        // is it at the end of the previous sub selection
        if ( nSubSelPos > 0 &&
             aSels[ nSubSelPos-1 ]->Max() == (nIndex-1) )
        {
            // expand the previous sub selection
            aSels[ nSubSelPos-1 ]->Max() = nIndex;

            // try to merge the previous sub selection
            ImplMergeSubSelections( nSubSelPos-1, nSubSelPos );
        }
        // is it at the beginning of the found sub selection
        else if (  nSubSelPos < aSels.size()
                && aSels[ nSubSelPos ]->Min() == (nIndex+1)
        )
            // expand the found sub selection
            aSels[ nSubSelPos ]->Min() = nIndex;
        else
        {
            // create a new sub selection
            if ( nSubSelPos < aSels.size() ) {
                ImpSelList::iterator it = aSels.begin();
                ::std::advance( it, nSubSelPos );
                aSels.insert( it, new Range( nIndex, nIndex ) );
            } else {
                aSels.push_back( new Range( nIndex, nIndex ) );
            }
            if ( bCurValid && nCurSubSel >= nSubSelPos )
                ++nCurSubSel;
        }
    }
    else
    {
        // is it excluded from the found sub selection?
        if (  nSubSelPos >= aSels.size()
           || !aSels[ nSubSelPos ]->IsInside( nIndex )
        ) {
            // not selected, nothing to do
            return false;
        }

        // it will become deselected
        --nSelCount;

        // is it the only index in the found sub selection?
        if ( aSels[ nSubSelPos ]->Len() == 1 )
        {
            // remove the complete sub selection
            ImpSelList::iterator it = aSels.begin();
            ::std::advance( it, nSubSelPos );
            delete *it;
            aSels.erase( it );
            return true;
        }

        // is it at the beginning of the found sub selection?
        if ( aSels[ nSubSelPos ]->Min() == nIndex )
            ++aSels[ nSubSelPos ]->Min();
        // is it at the end of the found sub selection?
        else if ( aSels[ nSubSelPos ]->Max() == nIndex )
            --aSels[ nSubSelPos ]->Max();
        // it is in the middle of the found sub selection?
        else
        {
            // split the sub selection
            if ( nSubSelPos < aSels.size() ) {
                ImpSelList::iterator it = aSels.begin();
                ::std::advance( it, nSubSelPos );
                aSels.insert( it, new Range( aSels[ nSubSelPos ]->Min(), nIndex-1 ) );
            } else {
                aSels.push_back( new Range( aSels[ nSubSelPos ]->Min(), nIndex-1 ) );
            }
            aSels[ nSubSelPos+1 ]->Min() = nIndex + 1;
        }
    }

    return true;
}

void MultiSelection::Select( const Range& rIndexRange, bool bSelect )
{
    Range* pRange;
    long nOld;

    sal_uIntPtr nTmpMin = rIndexRange.Min();
    sal_uIntPtr nTmpMax = rIndexRange.Max();
    sal_uIntPtr nCurMin = FirstSelected();
    sal_uIntPtr nCurMax = LastSelected();
    DBG_ASSERT(aTotRange.IsInside(nTmpMax), "selected index out of range" );
    DBG_ASSERT(aTotRange.IsInside(nTmpMin), "selected index out of range" );

    // replace whole selection?
    if( nTmpMin <= nCurMin && nTmpMax >= nCurMax )
    {
        ImplClear();
        if ( bSelect )
        {
            aSels.push_back( new Range(rIndexRange) );
            nSelCount = rIndexRange.Len();
        }
        return;
    }
    // expand on left side?
    if( nTmpMax < nCurMin )
    {
        if( bSelect )
        {
            // extend first range?
            if( nCurMin > (nTmpMax+1)  )
            {
                pRange = new Range( rIndexRange );
                aSels.insert( aSels.begin() , pRange );
                nSelCount += pRange->Len();
            }
            else
            {
                pRange = aSels.front();
                nOld = pRange->Min();
                pRange->Min() = (long)nTmpMin;
                nSelCount += ( nOld - nTmpMin );
            }
            bCurValid = false;
        }
        return;
    }
    // expand on right side?
    else if( nTmpMin > nCurMax )
    {
        if( bSelect )
        {
            // extend last range?
            if( nTmpMin > (nCurMax+1) )
            {
                pRange = new Range( rIndexRange );
                aSels.push_back( pRange );
                nSelCount += pRange->Len();
            }
            else
            {
                pRange = aSels.back();
                nOld = pRange->Max();
                pRange->Max() = (long)nTmpMax;
                nSelCount += ( nTmpMax - nOld );
            }
            bCurValid = false;
        }
        return;
    }

    // TODO here is potential for optimization
    while( nTmpMin <= nTmpMax )
    {
        Select( nTmpMin, bSelect );
        nTmpMin++;
    }
}

bool MultiSelection::IsSelected( long nIndex ) const
{
    // find the virtual target position
    size_t nSubSelPos = ImplFindSubSelection( nIndex );

    return nSubSelPos < aSels.size() && aSels[ nSubSelPos ]->IsInside(nIndex);
}

void MultiSelection::Insert( long nIndex, long nCount )
{
    // find the virtual target position
    size_t nSubSelPos = ImplFindSubSelection( nIndex );

    // did we need to shift the sub selections?
    if ( nSubSelPos < aSels.size() )
    {   // did we insert an unselected into an existing sub selection?
        if (  !bSelectNew
           && aSels[ nSubSelPos ]->Min() != nIndex
           && aSels[ nSubSelPos ]->IsInside(nIndex)
        ) { // split the sub selection
            if ( nSubSelPos < aSels.size() ) {
                ImpSelList::iterator it = aSels.begin();
                ::std::advance( it, nSubSelPos );
                aSels.insert( it, new Range( aSels[ nSubSelPos ]->Min(), nIndex-1 ) );
            } else {
                aSels.push_back( new Range( aSels[ nSubSelPos ]->Min(), nIndex-1 ) );
            }
            ++nSubSelPos;
            aSels[ nSubSelPos ]->Min() = nIndex;
        }

        // did we append an selected to an existing sub selection?
        else if (  bSelectNew
                && nSubSelPos > 0
                && aSels[ nSubSelPos ]->Max() == nIndex-1
        )   // expand the previous sub selection
            aSels[ nSubSelPos-1 ]->Max() += nCount;

        // did we insert an selected into an existing sub selection?
        else if (  bSelectNew
                && aSels[ nSubSelPos ]->Min() == nIndex
        ) { // expand the sub selection
            aSels[ nSubSelPos ]->Max() += nCount;
            ++nSubSelPos;
        }

        // shift the sub selections behind the inserting position
        for ( size_t nPos = nSubSelPos; nPos < aSels.size(); ++nPos )
        {
            aSels[ nPos ]->Min() += nCount;
            aSels[ nPos ]->Max() += nCount;
        }
    }

    bCurValid = false;
    aTotRange.Max() += nCount;
    if ( bSelectNew )
        nSelCount += nCount;
}

void MultiSelection::Remove( long nIndex )
{
    // find the virtual target position
    size_t nSubSelPos = ImplFindSubSelection( nIndex );

    // did we remove from an existing sub selection?
    if (  nSubSelPos < aSels.size()
       && aSels[ nSubSelPos ]->IsInside(nIndex)
    ) {
        // does this sub selection only contain the index to be deleted
        if ( aSels[ nSubSelPos ]->Len() == 1 ) {
            // completely remove the sub selection
            ImpSelList::iterator it = aSels.begin();
            ::std::advance( it, nSubSelPos );
            delete *it;
            aSels.erase( it );
        } else {
            // shorten this sub selection
            --( aSels[ nSubSelPos++ ]->Max() );
        }

        // adjust the selected counter
        --nSelCount;
    }

    // shift the sub selections behind the removed index
    for ( size_t nPos = nSubSelPos; nPos < aSels.size(); ++nPos )
    {
        --( aSels[ nPos ]->Min() );
        --( aSels[ nPos ]->Max() );
    }

    bCurValid = false;
    aTotRange.Max() -= 1;
}

long MultiSelection::ImplFwdUnselected()
{
    if ( !bCurValid )
        return SFX_ENDOFSELECTION;

    if (  ( nCurSubSel < aSels.size() )
       && ( aSels[ nCurSubSel ]->Min() <= nCurIndex )
    )
        nCurIndex = aSels[ nCurSubSel++ ]->Max() + 1;

    if ( nCurIndex <= aTotRange.Max() )
        return nCurIndex;
    else
        return SFX_ENDOFSELECTION;
}

long MultiSelection::FirstSelected( bool bInverse )
{
    bInverseCur = bInverse;
    nCurSubSel = 0;

    if ( bInverseCur )
    {
        bCurValid = nSelCount < sal_uIntPtr(aTotRange.Len());
        if ( bCurValid )
        {
            nCurIndex = 0;
            return ImplFwdUnselected();
        }
    }
    else
    {
        bCurValid = !aSels.empty();
        if ( bCurValid )
            return nCurIndex = aSels[ 0 ]->Min();
    }

    return SFX_ENDOFSELECTION;
}

long MultiSelection::LastSelected()
{
    nCurSubSel = aSels.size() - 1;
    bCurValid = !aSels.empty();

    if ( bCurValid )
        return nCurIndex = aSels[ nCurSubSel ]->Max();

    return SFX_ENDOFSELECTION;
}

long MultiSelection::NextSelected()
{
    if ( !bCurValid )
        return SFX_ENDOFSELECTION;

    if ( bInverseCur )
    {
        ++nCurIndex;
        return ImplFwdUnselected();
    }
    else
    {
        // is the next index in the current sub selection too?
        if ( nCurIndex < aSels[ nCurSubSel ]->Max() )
            return ++nCurIndex;

        // are there further sub selections?
        if ( ++nCurSubSel < aSels.size() )
            return nCurIndex = aSels[ nCurSubSel ]->Min();

        // we are at the end!
        return SFX_ENDOFSELECTION;
    }
}

void MultiSelection::SetTotalRange( const Range& rTotRange )
{
    aTotRange = rTotRange;

    // adjust lower boundary
    Range* pRange = aSels.empty() ? nullptr : aSels.front();
    while( pRange )
    {
        if( pRange->Max() < aTotRange.Min() )
        {
            delete pRange;
            aSels.erase( aSels.begin() );
        }
        else if( pRange->Min() < aTotRange.Min() )
        {
            pRange->Min() = aTotRange.Min();
            break;
        }
        else
            break;

        pRange = aSels.empty() ? nullptr : aSels.front();
    }

    // adjust upper boundary
    size_t nCount = aSels.size();
    while( nCount )
    {
        pRange = aSels[ nCount - 1 ];
        if( pRange->Min() > aTotRange.Max() )
        {
            delete pRange;
            aSels.pop_back();
        }
        else if( pRange->Max() > aTotRange.Max() )
        {
            pRange->Max() = aTotRange.Max();
            break;
        }
        else
            break;

        nCount = aSels.size();
    }

    // re-calculate selection count
    nSelCount = 0;
    for ( size_t i = 0, n = aSels.size(); i < n; ++ i )
        nSelCount += aSels[i]->Len();

    bCurValid = false;
    nCurIndex = 0;
}

// StringRangeEnumerator

StringRangeEnumerator::StringRangeEnumerator( const OUString& i_rInput,
                                              sal_Int32 i_nMinNumber,
                                              sal_Int32 i_nMaxNumber,
                                              sal_Int32 i_nLogicalOffset
                                              )
    : mnCount( 0 )
    , mnMin( i_nMinNumber )
    , mnMax( i_nMaxNumber )
    , mnOffset( i_nLogicalOffset )
    , mbValidInput( false )
{
    // Parse string only if boundaries are valid.
    if( mnMin >= 0 && mnMax >= 0 && mnMin <= mnMax )
        mbValidInput = setRange( i_rInput );
}

bool StringRangeEnumerator::checkValue( sal_Int32 i_nValue, const std::set< sal_Int32 >* i_pPossibleValues ) const
{
    if( i_nValue < 0 || i_nValue < mnMin || i_nValue > mnMax )
        return false;
    if( i_pPossibleValues && i_pPossibleValues->find( i_nValue ) == i_pPossibleValues->end() )
        return false;
    return true;
}

bool StringRangeEnumerator::insertRange( sal_Int32 i_nFirst, sal_Int32 i_nLast, bool bSequence, bool bMayAdjust )
{
    bool bSuccess = true;
    if( bSequence )
    {
        if( bMayAdjust )
        {
            if( i_nFirst < mnMin )
                i_nFirst = mnMin;
            if( i_nFirst > mnMax )
                i_nFirst = mnMax;
            if( i_nLast < mnMin )
                i_nLast = mnMin;
            if( i_nLast > mnMax )
                i_nLast = mnMax;
        }
        if( checkValue( i_nFirst ) && checkValue( i_nLast ) )
        {
            maSequence.push_back( Range( i_nFirst, i_nLast ) );
            sal_Int32 nNumber = i_nLast - i_nFirst;
            nNumber = nNumber < 0 ? -nNumber : nNumber;
            mnCount += nNumber + 1;
        }
        else
            bSuccess = false;
    }
    else
    {
        if( checkValue( i_nFirst ) )
        {
            maSequence.push_back( Range( i_nFirst, i_nFirst ) );
            mnCount++;
        }
        else if( checkValue( i_nLast ) )
        {
            maSequence.push_back( Range( i_nLast, i_nLast ) );
            mnCount++;
        }
        else
            bSuccess = false;
    }

    return bSuccess;
}

bool StringRangeEnumerator::insertJoinedRanges(
    const std::vector< sal_Int32 >& rNumbers, bool i_bStrict )
{
    size_t nCount = rNumbers.size();
    if( nCount == 0 )
        return true;

    if( nCount == 1 )
        return insertRange( rNumbers[0], -1, false, ! i_bStrict );

    for( size_t i = 0; i < nCount - 1; i++ )
    {
        sal_Int32 nFirst = rNumbers[i];
        sal_Int32 nLast  = rNumbers[i + 1];
        if( i > 0 )
        {
            if     ( nFirst > nLast ) nFirst--;
            else if( nFirst < nLast ) nFirst++;
        }

        if ( ! insertRange( nFirst, nLast, nFirst != nLast, ! i_bStrict ) && i_bStrict)
            return false;
    }

    return true;
}

bool StringRangeEnumerator::setRange( const OUString& i_rNewRange, bool i_bStrict )
{
    mnCount = 0;
    maSequence.clear();

    const sal_Unicode* pInput = i_rNewRange.getStr();
    OUStringBuffer aNumberBuf( 16 );
    std::vector< sal_Int32 > aNumbers;
    bool bSequence = false;
    while( *pInput )
    {
        while( *pInput >= '0' && *pInput <= '9' )
            aNumberBuf.append( *pInput++ );
        if( !aNumberBuf.isEmpty() )
        {
            sal_Int32 nNumber = aNumberBuf.makeStringAndClear().toInt32() + mnOffset;
            aNumbers.push_back( nNumber );
            bSequence = false;
        }

        if( *pInput == '-' )
        {
            bSequence = true;
            if( aNumbers.empty() )
                aNumbers.push_back( mnMin );
        }
        else if( *pInput == ',' || *pInput == ';' )
        {
            if( bSequence && !aNumbers.empty() )
                aNumbers.push_back( mnMax );
            if( ! insertJoinedRanges( aNumbers, i_bStrict ) && i_bStrict )
                return false;

            aNumbers.clear();
            bSequence = false;
        }
        else if( *pInput && *pInput != ' ' )
            return false; // parse error

        if( *pInput )
            pInput++;
    }
    // insert last entries
    if( bSequence && !aNumbers.empty() )
        aNumbers.push_back( mnMax );
    if( ! insertJoinedRanges( aNumbers, i_bStrict ) && i_bStrict )
        return false;

    return true;
}

bool StringRangeEnumerator::hasValue( sal_Int32 i_nValue, const std::set< sal_Int32 >* i_pPossibleValues ) const
{
    if( i_pPossibleValues && i_pPossibleValues->find( i_nValue ) == i_pPossibleValues->end() )
        return false;
    size_t n = maSequence.size();
    for( size_t i= 0; i < n; ++i )
    {
        const StringRangeEnumerator::Range rRange( maSequence[i] );
        if( rRange.nFirst < rRange.nLast )
        {
            if( i_nValue >= rRange.nFirst && i_nValue <= rRange.nLast )
                return true;
        }
        else
        {
            if( i_nValue >= rRange.nLast && i_nValue <= rRange.nFirst )
                return true;
        }
    }
    return false;
}

StringRangeEnumerator::Iterator& StringRangeEnumerator::Iterator::operator++()
{
    if( nRangeIndex >= 0 && nCurrent >= 0 && pEnumerator )
    {
        const StringRangeEnumerator::Range& rRange( pEnumerator->maSequence[nRangeIndex] );
        bool bRangeChange = false;
        if( rRange.nLast < rRange.nFirst )
        {
            // backward range
            if( nCurrent > rRange.nLast )
                nCurrent--;
            else
                bRangeChange = true;
        }
        else
        {
            // forward range
            if( nCurrent < rRange.nLast )
                nCurrent++;
            else
                bRangeChange = true;
        }
        if( bRangeChange )
        {
            nRangeIndex++;
            if( size_t(nRangeIndex) == pEnumerator->maSequence.size() )
            {
                // reached the end
                nRangeIndex = nCurrent = -1;
            }
            else
                nCurrent = pEnumerator->maSequence[nRangeIndex].nFirst;
        }
        if( nRangeIndex != -1 && nCurrent != -1 )
        {
            if( ! pEnumerator->checkValue( nCurrent, pPossibleValues ) )
                return ++(*this);
        }
    }
    return *this;
}


bool StringRangeEnumerator::Iterator::operator==( const Iterator& i_rCompare ) const
{
    return i_rCompare.pEnumerator == pEnumerator && i_rCompare.nRangeIndex == nRangeIndex && i_rCompare.nCurrent == nCurrent;
}

StringRangeEnumerator::Iterator StringRangeEnumerator::begin( const std::set< sal_Int32 >* i_pPossibleValues ) const
{
    StringRangeEnumerator::Iterator it( this,
                                        i_pPossibleValues,
                                        maSequence.empty() ? -1 : 0,
                                        maSequence.empty() ? -1 : maSequence[0].nFirst );
    if( ! checkValue(*it, i_pPossibleValues ) )
        ++it;
    return it;
}

StringRangeEnumerator::Iterator StringRangeEnumerator::end( const std::set< sal_Int32 >* i_pPossibleValues ) const
{
    return StringRangeEnumerator::Iterator( this, i_pPossibleValues, -1, -1 );
}

bool StringRangeEnumerator::getRangesFromString( const OUString& i_rPageRange,
                                                 std::vector< sal_Int32 >& o_rPageVector,
                                                 sal_Int32 i_nMinNumber,
                                                 sal_Int32 i_nMaxNumber,
                                                 sal_Int32 i_nLogicalOffset,
                                                 std::set< sal_Int32 >* i_pPossibleValues
                                               )
{
    o_rPageVector.clear();

    StringRangeEnumerator aEnum( i_rPageRange, i_nMinNumber, i_nMaxNumber, i_nLogicalOffset ) ;

    //Even if the input range wasn't completely valid, return what ranges could
    //be extracted from the input.
    o_rPageVector.reserve( static_cast< size_t >( aEnum.size() ) );
    for( StringRangeEnumerator::Iterator it = aEnum.begin( i_pPossibleValues );
         it != aEnum.end( i_pPossibleValues ); ++it )
    {
        o_rPageVector.push_back( *it );
    }

    return aEnum.isValidInput();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
