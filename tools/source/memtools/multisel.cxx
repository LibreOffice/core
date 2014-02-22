/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#ifdef MI_DEBUG
#define private public
#include <stdio.h>
#endif

#include <tools/debug.hxx>
#include <tools/multisel.hxx>

#include "rtl/ustrbuf.hxx"

#ifdef MI_DEBUG
#define DBG(x) x
#else
#define DBG(x)
#endif


#ifdef MI_DEBUG
static void Print( const MultiSelection* pSel )
{
    DbgOutf( "TotRange:     %4ld-%4ld\n",
             pSel->aTotRange.Min(), pSel->aTotRange.Max() );
    if ( pSel->bCurValid )
    {
        DbgOutf( "CurSubSel:    %4ld\n", pSel->nCurSubSel );
        DbgOutf( "CurIndex:     %4ld\n", pSel->nCurIndex );
    }
    DbgOutf( "SelCount:     %4ld\n", pSel->nSelCount );
    DbgOutf( "SubCount:     %4ld\n", pSel->aSels.Count() );
    for ( sal_uIntPtr nPos = 0; nPos < pSel->aSels.Count(); ++nPos )
    {
        DbgOutf( "SubSel #%2ld:   %4ld-%4ld\n", nPos,
                 pSel->aSels.GetObject(nPos)->Min(),
                 pSel->aSels.GetObject(nPos)->Max() );
    }
    DbgOutf( "\n" );
    fclose( pFile );
}
#endif

void MultiSelection::ImplClear()
{
    
    nSelCount = 0;

    for ( size_t i = 0, n = aSels.size(); i < n; ++i ) {
        delete aSels[ i ];
    }
    aSels.clear();
}

size_t MultiSelection::ImplFindSubSelection( long nIndex ) const
{
    
    size_t n = 0;
    for ( ;
          n < aSels.size() && nIndex > aSels[ n ]->Max();
          ++n ) {} /* empty loop */
    return n;
}

bool MultiSelection::ImplMergeSubSelections( size_t nPos1, size_t nPos2 )
{
    
    if ( nPos2 >= aSels.size() )
        return false;

    
    if ( (aSels[ nPos1 ]->Max() + 1) == aSels[ nPos2 ]->Min() )
    {
        
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
    nSelCount(0),
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
    }

    
    for ( size_t n = 0; n < rOrig.aSels.size(); ++n )
        aSels.push_back( new Range( *rOrig.aSels[ n ] ) );
}

MultiSelection::MultiSelection( const Range& rRange ):
    aTotRange(rRange),
    nCurSubSel(0),
    nSelCount(0),
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

    
    for ( size_t n = 0; n < aSels.size(); ++n )
        if ( *aSels[ n ] != *rWith.aSels[ n ] )
            return false;
    return true;
}

void MultiSelection::SelectAll( bool bSelect )
{
    DBG(DbgOutf( "::SelectAll(%s)\n", bSelect ? "sal_True" : "sal_False" ));

    ImplClear();
    if ( bSelect )
    {
        aSels.push_back( new Range(aTotRange) );
        nSelCount = aTotRange.Len();
    }

    DBG(Print( this ));
}

bool MultiSelection::Select( long nIndex, bool bSelect )
{
    DBG_ASSERT( aTotRange.IsInside(nIndex), "selected index out of range" );

    
    if ( !aTotRange.IsInside(nIndex) )
        return false;

    
    size_t nSubSelPos = ImplFindSubSelection( nIndex );

    if ( bSelect )
    {
        
        if ( nSubSelPos < aSels.size() && aSels[ nSubSelPos ]->IsInside( nIndex ) )
            
            return false;

        
        ++nSelCount;

        
        if ( nSubSelPos > 0 &&
             aSels[ nSubSelPos-1 ]->Max() == (nIndex-1) )
        {
            
            aSels[ nSubSelPos-1 ]->Max() = nIndex;

            
            ImplMergeSubSelections( nSubSelPos-1, nSubSelPos );
        }
        
        else if (  nSubSelPos < aSels.size()
                && aSels[ nSubSelPos ]->Min() == (nIndex+1)
        )
            
            aSels[ nSubSelPos ]->Min() = nIndex;
        else
        {
            
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
        
        if (  nSubSelPos >= aSels.size()
           || !aSels[ nSubSelPos ]->IsInside( nIndex )
        ) {
            
            DBG(Print( this ));
            return false;
        }

        
        --nSelCount;

        
        if ( aSels[ nSubSelPos ]->Len() == 1 )
        {
            
            ImpSelList::iterator it = aSels.begin();
            ::std::advance( it, nSubSelPos );
            delete *it;
            aSels.erase( it );
            DBG(Print( this ));
            return true;
        }

        
        if ( aSels[ nSubSelPos ]->Min() == nIndex )
            ++aSels[ nSubSelPos ]->Min();
        
        else if ( aSels[ nSubSelPos ]->Max() == nIndex )
            --aSels[ nSubSelPos ]->Max();
        
        else
        {
            
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

    DBG(Print( this ));

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
    
    if( nTmpMax < nCurMin )
    {
        if( bSelect )
        {
            
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
    
    else if( nTmpMin > nCurMax )
    {
        if( bSelect )
        {
            
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

    
    while( nTmpMin <= nTmpMax )
    {
        Select( nTmpMin, bSelect );
        nTmpMin++;
    }
}

bool MultiSelection::IsSelected( long nIndex ) const
{
    
    size_t nSubSelPos = ImplFindSubSelection( nIndex );

    return nSubSelPos < aSels.size() && aSels[ nSubSelPos ]->IsInside(nIndex);
}

void MultiSelection::Insert( long nIndex, long nCount )
{
    DBG(DbgOutf( "::Insert(%ld, %ld)\n", nIndex, nCount ));

    
    size_t nSubSelPos = ImplFindSubSelection( nIndex );

    
    if ( nSubSelPos < aSels.size() )
    {   
        if (  !bSelectNew
           && aSels[ nSubSelPos ]->Min() != nIndex
           && aSels[ nSubSelPos ]->IsInside(nIndex)
        ) { 
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

        
        else if (  bSelectNew
                && nSubSelPos > 0
                && aSels[ nSubSelPos ]->Max() == nIndex-1
        )   
            aSels[ nSubSelPos-1 ]->Max() += nCount;

        
        else if (  bSelectNew
                && aSels[ nSubSelPos ]->Min() == nIndex
        ) { 
            aSels[ nSubSelPos ]->Max() += nCount;
            ++nSubSelPos;
        }

        
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

    DBG(Print( this ));
}

void MultiSelection::Remove( long nIndex )
{
    DBG(DbgOutf( "::Remove(%ld)\n", nIndex ));

    
    size_t nSubSelPos = ImplFindSubSelection( nIndex );

    
    if (  nSubSelPos < aSels.size()
       && aSels[ nSubSelPos ]->IsInside(nIndex)
    ) {
        
        if ( aSels[ nSubSelPos ]->Len() == 1 ) {
            
            ImpSelList::iterator it = aSels.begin();
            ::std::advance( it, nSubSelPos );
            delete *it;
            aSels.erase( it );
        } else {
            
            --( aSels[ nSubSelPos++ ]->Max() );
        }

        
        --nSelCount;
    }

    
    for ( size_t nPos = nSubSelPos; nPos < aSels.size(); ++nPos )
    {
        --( aSels[ nPos ]->Min() );
        --( aSels[ nPos ]->Max() );
    }

    bCurValid = false;
    aTotRange.Max() -= 1;

    DBG(Print( this ));
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
        
        if ( nCurIndex < aSels[ nCurSubSel ]->Max() )
            return ++nCurIndex;

        
        if ( ++nCurSubSel < aSels.size() )
            return nCurIndex = aSels[ nCurSubSel ]->Min();

        
        return SFX_ENDOFSELECTION;
    }
}

void MultiSelection::SetTotalRange( const Range& rTotRange )
{
    aTotRange = rTotRange;

    
    Range* pRange = aSels.empty() ? NULL : aSels.front();
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

        pRange = aSels.empty() ? NULL : aSels.front();
    }

    
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

    
    nSelCount = 0;
    for ( size_t i = 0, n = aSels.size(); i < n; ++ i )
        nSelCount += aSels[i]->Len();

    bCurValid = false;
    nCurIndex = 0;
}



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
            return false; 

        if( *pInput )
            pInput++;
    }
    
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
            
            if( nCurrent > rRange.nLast )
                nCurrent--;
            else
                bRangeChange = true;
        }
        else
        {
            
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

sal_Int32 StringRangeEnumerator::Iterator::operator*() const
{
    return nCurrent;
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

    
    
    o_rPageVector.reserve( static_cast< size_t >( aEnum.size() ) );
    for( StringRangeEnumerator::Iterator it = aEnum.begin( i_pPossibleValues );
         it != aEnum.end( i_pPossibleValues ); ++it )
    {
        o_rPageVector.push_back( *it );
    }

    return aEnum.isValidInput();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
