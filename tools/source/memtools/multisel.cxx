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
    aSels.clear();
}

sal_Int32 MultiSelection::ImplFindSubSelection( sal_Int32 nIndex ) const
{
    // iterate through the sub selections
    sal_Int32 n = 0;
    for ( ;
          n < sal_Int32(aSels.size()) && nIndex > aSels[ n ].Max();
          ++n ) {} /* empty loop */
    return n;
}

void MultiSelection::ImplMergeSubSelections( sal_Int32 nPos1, sal_Int32 nPos2 )
{
    // didn't a sub selection at nPos2 exist?
    if ( nPos2 >= sal_Int32(aSels.size()) )
        return;

    // did the sub selections touch each other?
    if ( (aSels[ nPos1 ].Max() + 1) == aSels[ nPos2 ].Min() )
    {
        // merge them
        aSels[ nPos1 ].Max() = aSels[ nPos2 ].Max();
        aSels.erase( aSels.begin() + nPos2 );
    }
}

MultiSelection::MultiSelection():
    aTotRange( 0, -1 ),
    nCurSubSel(0),
    nCurIndex(0),
    nSelCount(0),
    bCurValid(false)
{
}

void MultiSelection::Reset()
{
    aTotRange = Range(0, -1);
    bCurValid = false;
    // clear the old sub selections
    ImplClear();
}

MultiSelection::MultiSelection( const MultiSelection& rOrig ) :
    aTotRange(rOrig.aTotRange),
    nSelCount(rOrig.nSelCount),
    bCurValid(rOrig.bCurValid)
{
    if ( bCurValid )
    {
        nCurSubSel = rOrig.nCurSubSel;
        nCurIndex = rOrig.nCurIndex;
    }
    else
    {
        nCurSubSel = 0;
        nCurIndex = 0;
    }

    // copy the sub selections
    for (const Range & rSel : rOrig.aSels)
        aSels.push_back( rSel );
}

MultiSelection::MultiSelection( const Range& rRange ):
    aTotRange(rRange),
    nCurSubSel(0),
    nCurIndex(0),
    nSelCount(0),
    bCurValid(false)
{
}

MultiSelection::~MultiSelection()
{
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
    for (const Range& rSel : rOrig.aSels)
        aSels.push_back( rSel );
    nSelCount = rOrig.nSelCount;

    return *this;
}

void MultiSelection::SelectAll( bool bSelect )
{
    ImplClear();
    if ( bSelect )
    {
        aSels.push_back( aTotRange );
        nSelCount = aTotRange.Len();
    }
}

bool MultiSelection::Select( sal_Int32 nIndex, bool bSelect )
{
    DBG_ASSERT( aTotRange.IsInside(nIndex), "selected index out of range" );

    // out of range?
    if ( !aTotRange.IsInside(nIndex) )
        return false;

    // find the virtual target position
    sal_Int32 nSubSelPos = ImplFindSubSelection( nIndex );

    if ( bSelect )
    {
        // is it included in the found sub selection?
        if ( nSubSelPos < sal_Int32(aSels.size()) && aSels[ nSubSelPos ].IsInside( nIndex ) )
            // already selected, nothing to do
            return false;

        // it will become selected
        ++nSelCount;

        // is it at the end of the previous sub selection
        if ( nSubSelPos > 0 &&
             aSels[ nSubSelPos-1 ].Max() == (nIndex-1) )
        {
            // expand the previous sub selection
            aSels[ nSubSelPos-1 ].Max() = nIndex;

            // try to merge the previous sub selection
            ImplMergeSubSelections( nSubSelPos-1, nSubSelPos );
        }
        // is it at the beginning of the found sub selection
        else if (  nSubSelPos < sal_Int32(aSels.size())
                && aSels[ nSubSelPos ].Min() == (nIndex+1)
        )
            // expand the found sub selection
            aSels[ nSubSelPos ].Min() = nIndex;
        else
        {
            // create a new sub selection
            if ( nSubSelPos < sal_Int32(aSels.size()) ) {
                aSels.insert( aSels.begin() + nSubSelPos, Range( nIndex, nIndex ) );
            } else {
                aSels.push_back( Range( nIndex, nIndex ) );
            }
            if ( bCurValid && nCurSubSel >= nSubSelPos )
                ++nCurSubSel;
        }
    }
    else
    {
        // is it excluded from the found sub selection?
        if (  nSubSelPos >= sal_Int32(aSels.size())
           || !aSels[ nSubSelPos ].IsInside( nIndex )
        ) {
            // not selected, nothing to do
            return false;
        }

        // it will become deselected
        --nSelCount;

        // is it the only index in the found sub selection?
        if ( aSels[ nSubSelPos ].Len() == 1 )
        {
            // remove the complete sub selection
            aSels.erase( aSels.begin() + nSubSelPos );
            return true;
        }

        // is it at the beginning of the found sub selection?
        if ( aSels[ nSubSelPos ].Min() == nIndex )
            ++aSels[ nSubSelPos ].Min();
        // is it at the end of the found sub selection?
        else if ( aSels[ nSubSelPos ].Max() == nIndex )
            --aSels[ nSubSelPos ].Max();
        // it is in the middle of the found sub selection?
        else
        {
            // split the sub selection
            if ( nSubSelPos < sal_Int32(aSels.size()) ) {
                aSels.insert( aSels.begin() + nSubSelPos, Range( aSels[ nSubSelPos ].Min(), nIndex-1 ) );
            } else {
                aSels.push_back( Range( aSels[ nSubSelPos ].Min(), nIndex-1 ) );
            }
            aSels[ nSubSelPos+1 ].Min() = nIndex + 1;
        }
    }

    return true;
}

void MultiSelection::Select( const Range& rIndexRange, bool bSelect )
{
    sal_Int32 nOld;

    sal_Int32 nTmpMin = rIndexRange.Min();
    sal_Int32 nTmpMax = rIndexRange.Max();
    sal_Int32 nCurMin = FirstSelected();
    sal_Int32 nCurMax = LastSelected();
    DBG_ASSERT(aTotRange.IsInside(nTmpMax), "selected index out of range" );
    DBG_ASSERT(aTotRange.IsInside(nTmpMin), "selected index out of range" );

    // replace whole selection?
    if( nTmpMin <= nCurMin && nTmpMax >= nCurMax )
    {
        ImplClear();
        if ( bSelect )
        {
            aSels.push_back( rIndexRange );
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
                aSels.insert( aSels.begin(), rIndexRange );
                nSelCount += rIndexRange.Len();
            }
            else
            {
                auto & rRange = aSels.front();
                nOld = rRange.Min();
                rRange.Min() = nTmpMin;
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
                aSels.push_back( rIndexRange );
                nSelCount += rIndexRange.Len();
            }
            else
            {
                auto & rRange = aSels.back();
                nOld = rRange.Max();
                rRange.Max() = nTmpMax;
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

bool MultiSelection::IsSelected( sal_Int32 nIndex ) const
{
    // find the virtual target position
    sal_Int32 nSubSelPos = ImplFindSubSelection( nIndex );

    return nSubSelPos < sal_Int32(aSels.size()) && aSels[ nSubSelPos ].IsInside(nIndex);
}

void MultiSelection::Insert( sal_Int32 nIndex, sal_Int32 nCount )
{
    // find the virtual target position
    sal_Int32 nSubSelPos = ImplFindSubSelection( nIndex );

    // did we need to shift the sub selections?
    if ( nSubSelPos < sal_Int32(aSels.size()) )
    {   // did we insert an unselected into an existing sub selection?
        if (  aSels[ nSubSelPos ].Min() != nIndex
           && aSels[ nSubSelPos ].IsInside(nIndex)
        ) { // split the sub selection
            if ( nSubSelPos < sal_Int32(aSels.size()) ) {
                aSels.insert( aSels.begin() + nSubSelPos, Range( aSels[ nSubSelPos ].Min(), nIndex-1 ) );
            } else {
                aSels.push_back( Range( aSels[ nSubSelPos ].Min(), nIndex-1 ) );
            }
            ++nSubSelPos;
            aSels[ nSubSelPos ].Min() = nIndex;
        }

        // shift the sub selections behind the inserting position
        for ( sal_Int32 nPos = nSubSelPos; nPos < sal_Int32(aSels.size()); ++nPos )
        {
            aSels[ nPos ].Min() += nCount;
            aSels[ nPos ].Max() += nCount;
        }
    }

    bCurValid = false;
    aTotRange.Max() += nCount;
}

void MultiSelection::Remove( sal_Int32 nIndex )
{
    // find the virtual target position
    sal_Int32 nSubSelPos = ImplFindSubSelection( nIndex );

    // did we remove from an existing sub selection?
    if (  nSubSelPos < sal_Int32(aSels.size())
       && aSels[ nSubSelPos ].IsInside(nIndex)
    ) {
        // does this sub selection only contain the index to be deleted
        if ( aSels[ nSubSelPos ].Len() == 1 ) {
            // completely remove the sub selection
            aSels.erase( aSels.begin() + nSubSelPos );
        } else {
            // shorten this sub selection
            --( aSels[ nSubSelPos++ ].Max() );
        }

        // adjust the selected counter
        --nSelCount;
    }

    // shift the sub selections behind the removed index
    for ( sal_Int32 nPos = nSubSelPos; nPos < sal_Int32(aSels.size()); ++nPos )
    {
        --( aSels[ nPos ].Min() );
        --( aSels[ nPos ].Max() );
    }

    bCurValid = false;
    aTotRange.Max() -= 1;
}

sal_Int32 MultiSelection::FirstSelected()
{
    nCurSubSel = 0;

    bCurValid = !aSels.empty();
    if ( bCurValid )
        return nCurIndex = aSels[ 0 ].Min();

    return SFX_ENDOFSELECTION;
}

sal_Int32 MultiSelection::LastSelected()
{
    nCurSubSel = aSels.size() - 1;
    bCurValid = !aSels.empty();

    if ( bCurValid )
        return nCurIndex = aSels[ nCurSubSel ].Max();

    return SFX_ENDOFSELECTION;
}

sal_Int32 MultiSelection::NextSelected()
{
    if ( !bCurValid )
        return SFX_ENDOFSELECTION;

    // is the next index in the current sub selection too?
    if ( nCurIndex < aSels[ nCurSubSel ].Max() )
        return ++nCurIndex;

    // are there further sub selections?
    if ( ++nCurSubSel < sal_Int32(aSels.size()) )
        return nCurIndex = aSels[ nCurSubSel ].Min();

    // we are at the end!
    return SFX_ENDOFSELECTION;
}

void MultiSelection::SetTotalRange( const Range& rTotRange )
{
    aTotRange = rTotRange;

    // adjust lower boundary
    Range* pRange = aSels.empty() ? nullptr : &aSels.front();
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

        pRange = aSels.empty() ? nullptr : &aSels.front();
    }

    // adjust upper boundary
    sal_Int32 nCount = aSels.size();
    while( nCount )
    {
        pRange = &aSels[ nCount - 1 ];
        if( pRange->Min() > aTotRange.Max() )
        {
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
    for (Range const & rSel : aSels)
        nSelCount += rSel.Len();

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

bool StringRangeEnumerator::insertRange( sal_Int32 i_nFirst, sal_Int32 i_nLast, bool bSequence )
{
    bool bSuccess = true;
    if( bSequence )
    {
        // Check if the range is completely outside of possible pages range
        if ((i_nFirst < mnMin && i_nLast < mnMin) ||
            (i_nFirst > mnMax && i_nLast > mnMax))
            return false;
        if( i_nFirst < mnMin )
            i_nFirst = mnMin;
        if( i_nFirst > mnMax )
            i_nFirst = mnMax;
        if( i_nLast < mnMin )
            i_nLast = mnMin;
        if( i_nLast > mnMax )
            i_nLast = mnMax;
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
    const std::vector< sal_Int32 >& rNumbers )
{
    size_t nCount = rNumbers.size();
    if( nCount == 0 )
        return true;

    if( nCount == 1 )
        return insertRange( rNumbers[0], -1, false );

    for( size_t i = 0; i < nCount - 1; i++ )
    {
        sal_Int32 nFirst = rNumbers[i];
        sal_Int32 nLast  = rNumbers[i + 1];
        if( i > 0 )
        {
            if     ( nFirst > nLast ) nFirst--;
            else if( nFirst < nLast ) nFirst++;
        }

        insertRange( nFirst, nLast, nFirst != nLast );
    }

    return true;
}

bool StringRangeEnumerator::setRange( const OUString& i_rNewRange )
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
            {
                // push out-of-range small value, to exclude ranges totally outside of possible range
                aNumbers.push_back( mnMin-1 );
            }
        }
        else if( *pInput == ',' || *pInput == ';' )
        {
            if( bSequence && !aNumbers.empty() )
            {
                // push out-of-range large value, to exclude ranges totally outside of possible range
                aNumbers.push_back( mnMax+1 );
            }
            insertJoinedRanges( aNumbers );

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
    {
        // push out-of-range large value, to exclude ranges totally outside of possible range
        aNumbers.push_back( mnMax+1 );
    }
    insertJoinedRanges( aNumbers );

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
                                                 std::set< sal_Int32 > const * i_pPossibleValues
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

    return aEnum.mbValidInput;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
