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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

#define _SV_MULTISEL_CXX

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

using namespace rtl;

//==================================================================

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
    for ( ULONG nPos = 0; nPos < pSel->aSels.Count(); ++nPos )
    {
        DbgOutf( "SubSel #%2ld:   %4ld-%4ld\n", nPos,
                 pSel->aSels.GetObject(nPos)->Min(),
                 pSel->aSels.GetObject(nPos)->Max() );
    }
    DbgOutf( "\n" );
    fclose( pFile );
}

#endif

// -----------------------------------------------------------------------

void MultiSelection::ImplClear()
{
    // no selected indexes
    nSelCount = 0;

    for ( size_t i = 0, n = aSels.size(); i < n; ++i ) {
        delete aSels[ i ];
    }
    aSels.clear();
}

// -----------------------------------------------------------------------

size_t MultiSelection::ImplFindSubSelection( long nIndex ) const
{
    // iterate through the sub selections
    size_t n = 0;
    for ( ;
          n < aSels.size() && nIndex > aSels[ n ]->Max();
          ++n ) {} /* empty loop */
    return n;
}

// -----------------------------------------------------------------------

BOOL MultiSelection::ImplMergeSubSelections( size_t nPos1, size_t nPos2 )
{
    // didn't a sub selection at nPos2 exist?
    if ( nPos2 >= aSels.size() )
        return FALSE;

    // did the sub selections touch each other?
    if ( (aSels[ nPos1 ]->Max() + 1) == aSels[ nPos2 ]->Min() )
    {
        // merge them
        aSels[ nPos1 ]->Max() = aSels[ nPos2 ]->Max();
        ImpSelList::iterator it = aSels.begin();
        ::std::advance( it, nPos2 );
        delete *it;
        aSels.erase( it );
        return TRUE;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

MultiSelection::MultiSelection():
    aTotRange( 0, -1 ),
    nCurSubSel(0),
    nSelCount(0),
    bCurValid(FALSE),
    bSelectNew(FALSE)
{
}

// -----------------------------------------------------------------------

MultiSelection::MultiSelection( const UniString& rString, sal_Unicode cRange, sal_Unicode cSep ):
    aTotRange(0,RANGE_MAX),
    nCurSubSel(0),
    nSelCount(0),
    bCurValid(FALSE),
    bSelectNew(FALSE)
{
    // Dies ist nur ein Schnellschuss und sollte bald optimiert,
    // an die verschiedenen Systeme (UNIX etc.)
    // und die gewuenschte Eingabe-Syntax angepasst werden.

    UniString           aStr( rString );
    sal_Unicode*        pStr   = aStr.GetBufferAccess();
    sal_Unicode*        pOld = pStr;
    BOOL                bReady = FALSE;
    BOOL                bUntil = FALSE;
    xub_StrLen          nCut   = 0;

    // Hier normieren wir den String, sodass nur Ziffern,
    // Semikola als Trenn- und Minus als VonBis-Zeichen
    // uebrigbleiben, z.B. "99-117;55;34;-17;37-43"
    while ( *pOld )
    {
        switch( *pOld )
        {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                DBG_ASSERT( *pOld != cRange, "digit for range char not allowed" );
                DBG_ASSERT( *pOld != cSep, "digit for separator not allowed" );
                if( bReady )
                {
                    *pStr++ = ';';
                    nCut++;
                    bReady = FALSE;
                }
                *pStr++ = *pOld;
                nCut++;
                bUntil = FALSE;
                break;

            case '-':
            case ':':
            case '/':
                if ( *pOld != cSep )
                {
                    if ( !bUntil )
                    {
                        *pStr++ = '-';
                        nCut++;
                        bUntil = TRUE;
                    }
                    bReady = FALSE;
                }
                else
                    bReady = TRUE;
                break;

            case ' ':
                DBG_ASSERT( *pOld != cRange, "SPACE for range char not allowed" );
                DBG_ASSERT( *pOld != cSep, "SPACE for separator not allowed" );
                bReady = !bUntil;
                break;

            default:
                if ( *pOld == cRange )
                {
                    if ( !bUntil )
                    {
                        *pStr++ = '-';
                        nCut++;
                        bUntil = TRUE;
                    }
                    bReady = FALSE;
                }
                else
                    bReady = TRUE;
                break;
        }

        pOld++;
    }
    aStr.ReleaseBufferAccess( nCut );

    // Jetzt wird der normierte String ausgewertet ..
    UniString           aNumStr;
    Range               aRg( 1, RANGE_MAX );
    const sal_Unicode*  pCStr = aStr.GetBuffer();
    long                nPage = 1;
    long                nNum  = 1;
    bUntil = FALSE;
    while ( *pCStr )
    {
        switch ( *pCStr )
        {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                aNumStr += *pCStr;
                break;
            case ';':
                nNum = aNumStr.ToInt32();
                if ( bUntil )
                {
                    if ( !aNumStr.Len() )
                        nNum = RANGE_MAX;
                    aRg.Min() = nPage;
                    aRg.Max() = nNum;
                    aRg.Justify();
                    Select( aRg );
                }
                else
                    Select( nNum );
                nPage = 0;
                aNumStr.Erase();
                bUntil = FALSE;
                break;

            case '-':
                nPage = aNumStr.ToInt32();
                aNumStr.Erase();
                bUntil = TRUE;
                break;
        }

        pCStr++;
    }

    nNum = aNumStr.ToInt32();
    if ( bUntil )
    {
        if ( !aNumStr.Len() )
            nNum = RANGE_MAX;
        aRg.Min() = nPage;
        aRg.Max() = nNum;
        aRg.Justify();
        Select( aRg );
    }
    else
        Select( nNum );
}

// -----------------------------------------------------------------------

MultiSelection::MultiSelection( const MultiSelection& rOrig ) :
    aTotRange(rOrig.aTotRange),
    nSelCount(rOrig.nSelCount),
    bCurValid(rOrig.bCurValid),
    bSelectNew(FALSE)
{
    if ( bCurValid )
    {
        nCurSubSel = rOrig.nCurSubSel;
        nCurIndex = rOrig.nCurIndex;
    }

    // copy the sub selections
    for ( size_t n = 0; n < rOrig.aSels.size(); ++n )
        aSels.push_back( new Range( *rOrig.aSels[ n ] ) );
}

// -----------------------------------------------------------------------

MultiSelection::MultiSelection( const Range& rRange ):
    aTotRange(rRange),
    nCurSubSel(0),
    nSelCount(0),
    bCurValid(FALSE),
    bSelectNew(FALSE)
{
}

// -----------------------------------------------------------------------

MultiSelection::~MultiSelection()
{
    for ( size_t i = 0, n = aSels.size(); i < n; ++i )
        delete aSels[ i ];
    aSels.clear();
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

BOOL MultiSelection::operator== ( MultiSelection& rWith )
{
    if ( aTotRange != rWith.aTotRange || nSelCount != rWith.nSelCount ||
         aSels.size() != rWith.aSels.size() )
        return FALSE;

    // compare the sub seletions
    for ( size_t n = 0; n < aSels.size(); ++n )
        if ( *aSels[ n ] != *rWith.aSels[ n ] )
            return FALSE;
    return TRUE;
}

// -----------------------------------------------------------------------

void MultiSelection::SelectAll( BOOL bSelect )
{
    DBG(DbgOutf( "::SelectAll(%s)\n", bSelect ? "TRUE" : "FALSE" ));

    ImplClear();
    if ( bSelect )
    {
        aSels.push_back( new Range(aTotRange) );
        nSelCount = aTotRange.Len();
    }

    DBG(Print( this ));
}

// -----------------------------------------------------------------------

BOOL MultiSelection::Select( long nIndex, BOOL bSelect )
{
    DBG_ASSERT( aTotRange.IsInside(nIndex), "selected index out of range" );

    // out of range?
    if ( !aTotRange.IsInside(nIndex) )
        return FALSE;

    // find the virtual target position
    size_t nSubSelPos = ImplFindSubSelection( nIndex );

    if ( bSelect )
    {
        // is it included in the found sub selection?
        if ( nSubSelPos < aSels.size() && aSels[ nSubSelPos ]->IsInside( nIndex ) )
            // already selected, nothing to do
            return FALSE;

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
        // is is at the beginning of the found sub selection
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
            DBG(Print( this ));
            return FALSE;
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
            DBG(Print( this ));
            return TRUE;
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

    DBG(Print( this ));

    return TRUE;
}

// -----------------------------------------------------------------------

void MultiSelection::Select( const Range& rIndexRange, BOOL bSelect )
{
    Range* pRange;
    long nOld;

    ULONG nTmpMin = rIndexRange.Min();
    ULONG nTmpMax = rIndexRange.Max();
    ULONG nCurMin = FirstSelected();
    ULONG nCurMax = LastSelected();
    DBG_ASSERT(aTotRange.IsInside(nTmpMax), "selected index out of range" );
    DBG_ASSERT(aTotRange.IsInside(nTmpMin), "selected index out of range" );

    // gesamte Selektion ersetzen ?
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
    // links erweitern ?
    if( nTmpMax < nCurMin )
    {
        if( bSelect )
        {
            // ersten Range erweitern ?
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
            bCurValid = FALSE;
        }
        return;
    }
    // rechts erweitern ?
    else if( nTmpMin > nCurMax )
    {
        if( bSelect )
        {
            // letzten Range erweitern ?
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
            bCurValid = FALSE;
        }
        return;
    }

    //HACK(Hier muss noch optimiert werden)
    while( nTmpMin <= nTmpMax )
    {
        Select( nTmpMin, bSelect );
        nTmpMin++;
    }
}

// -----------------------------------------------------------------------

BOOL MultiSelection::IsSelected( long nIndex ) const
{
    // find the virtual target position
    size_t nSubSelPos = ImplFindSubSelection( nIndex );

    return nSubSelPos < aSels.size() && aSels[ nSubSelPos ]->IsInside(nIndex);
}

// -----------------------------------------------------------------------

void MultiSelection::Insert( long nIndex, long nCount )
{
    DBG(DbgOutf( "::Insert(%ld, %ld)\n", nIndex, nCount ));

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

    bCurValid = FALSE;
    aTotRange.Max() += nCount;
    if ( bSelectNew )
        nSelCount += nCount;

    DBG(Print( this ));
}

// -----------------------------------------------------------------------

void MultiSelection::Remove( long nIndex )
{
    DBG(DbgOutf( "::Remove(%ld)\n", nIndex ));

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

    bCurValid = FALSE;
    aTotRange.Max() -= 1;

    DBG(Print( this ));
}

// -----------------------------------------------------------------------

void MultiSelection::Append( long nCount )
{
    long nPrevLast = aTotRange.Max();
    aTotRange.Max() += nCount;
    if ( bSelectNew )
    {
        nSelCount += nCount;
        aSels.push_back( new Range( nPrevLast+1, nPrevLast + nCount ) );
        if ( aSels.size() > 1 )
            ImplMergeSubSelections( aSels.size() - 2, aSels.size() );
    }
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

long MultiSelection::ImplBwdUnselected()
{
    if ( !bCurValid )
        return SFX_ENDOFSELECTION;

    if ( aSels[ nCurSubSel ]->Max() < nCurIndex )
        return nCurIndex;

    nCurIndex = aSels[ nCurSubSel-- ]->Min() - 1;
    if ( nCurIndex >= 0 )
        return nCurIndex;
    else
        return SFX_ENDOFSELECTION;
}

// -----------------------------------------------------------------------

long MultiSelection::FirstSelected( BOOL bInverse )
{
    bInverseCur = bInverse;
    nCurSubSel = 0;

    if ( bInverseCur )
    {
        bCurValid = nSelCount < ULONG(aTotRange.Len());
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

// -----------------------------------------------------------------------

long MultiSelection::LastSelected()
{
    nCurSubSel = aSels.size() - 1;
    bCurValid = !aSels.empty();

    if ( bCurValid )
        return nCurIndex = aSels[ nCurSubSel ]->Max();

    return SFX_ENDOFSELECTION;
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

long MultiSelection::PrevSelected()
{
    if ( !bCurValid )
        return SFX_ENDOFSELECTION;

    if ( bInverseCur )
    {
        --nCurIndex;
        return ImplBwdUnselected();
    }
    else
    {
        // is the previous index in the current sub selection too?
        if ( nCurIndex > aSels[ nCurSubSel ]->Min() )
            return --nCurIndex;

        // are there previous sub selections?
        if ( nCurSubSel > 0 )
        {
            --nCurSubSel;
            return nCurIndex = aSels[ nCurSubSel ]->Max();
        }

        // we are at the beginning!
        return SFX_ENDOFSELECTION;
    }
}

// -----------------------------------------------------------------------

void MultiSelection::SetTotalRange( const Range& rTotRange )
{
    aTotRange = rTotRange;

    // die untere Bereichsgrenze anpassen
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

    // die obere Bereichsgrenze anpassen
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

    // Selection-Count neu berechnen
    nSelCount = 0;
    for ( size_t i = 0, n = aSels.size(); i < n; ++ i ) {
        nSelCount += pRange->Len();
    }

    bCurValid = FALSE;
    nCurIndex = 0;
}

// -----------------------------------------------------------------------
//
// StringRangeEnumerator
//
// -----------------------------------------------------------------------
StringRangeEnumerator::StringRangeEnumerator( const rtl::OUString& i_rInput,
                                              sal_Int32 i_nMinNumber,
                                              sal_Int32 i_nMaxNumber,
                                              sal_Int32 i_nLogicalOffset
                                              )
    : mnCount( 0 )
    , mnMin( i_nMinNumber )
    , mnMax( i_nMaxNumber )
    , mnOffset( i_nLogicalOffset )
{
    setRange( i_rInput );
}

bool StringRangeEnumerator::checkValue( sal_Int32 i_nValue, const std::set< sal_Int32 >* i_pPossibleValues ) const
{
    if( mnMin >= 0 && i_nValue < mnMin )
        return false;
    if( mnMax >= 0 && i_nValue > mnMax )
        return false;
    if( i_nValue < 0 )
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
        if( i_nFirst == -1 )
            i_nFirst = mnMin;
        if( i_nLast == -1 )
            i_nLast = mnMax;
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
        if( i_nFirst >= 0 )
        {
            if( checkValue( i_nFirst ) )
            {
                maSequence.push_back( Range( i_nFirst, i_nFirst ) );
                mnCount++;
            }
            else
                bSuccess = false;
        }
        if( i_nLast >= 0 )
        {
            if( checkValue( i_nLast ) )
            {
                maSequence.push_back( Range( i_nLast, i_nLast ) );
                mnCount++;
            }
            else
                bSuccess = false;
        }
    }

    return bSuccess;
}

bool StringRangeEnumerator::setRange( const rtl::OUString& i_rNewRange, bool i_bStrict )
{
    mnCount = 0;
    maSequence.clear();

    // we love special cases
    if( i_rNewRange.getLength() == 0 )
    {
        if( mnMin >= 0 && mnMax >= 0 )
        {
            insertRange( mnMin, mnMax, mnMin != mnMax, ! i_bStrict );
        }
        return true;
    }

    const sal_Unicode* pInput = i_rNewRange.getStr();
    rtl::OUStringBuffer aNumberBuf( 16 );
    sal_Int32 nLastNumber = -1, nNumber = -1;
    bool bSequence = false;
    bool bSuccess = true;
    while( *pInput )
    {
        while( *pInput >= sal_Unicode('0') && *pInput <= sal_Unicode('9') )
            aNumberBuf.append( *pInput++ );
        if( aNumberBuf.getLength() )
        {
            if( nNumber != -1 )
            {
                if( bSequence )
                {
                    if( ! insertRange( nLastNumber, nNumber, true, ! i_bStrict ) && i_bStrict )
                    {
                        bSuccess = false;
                        break;
                    }
                    nLastNumber = -1;
                }
                else
                {
                    if( ! insertRange( nNumber, nNumber, false, ! i_bStrict ) && i_bStrict )
                    {
                        bSuccess = false;
                        break;
                    }
                }
            }
            nNumber = aNumberBuf.makeStringAndClear().toInt32();
            nNumber += mnOffset;
        }
        bool bInsertRange = false;
        if( *pInput == sal_Unicode('-') )
        {
            nLastNumber = nNumber;
            nNumber = -1;
            bSequence = true;
        }
        else if( *pInput == ' ' )
        {
        }
        else if( *pInput == sal_Unicode(',') || *pInput == sal_Unicode(';') )
            bInsertRange = true;
        else if( *pInput )
        {

            bSuccess = false;
            break; // parse error
        }

        if( bInsertRange )
        {
            if( ! insertRange( nLastNumber, nNumber, bSequence, ! i_bStrict ) && i_bStrict )
            {
                bSuccess = false;
                break;
            }
            nNumber = nLastNumber = -1;
            bSequence = false;
        }
        if( *pInput )
            pInput++;
    }
    // insert last entries
    insertRange( nLastNumber, nNumber, bSequence, ! i_bStrict );

    return bSuccess;
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
    StringRangeEnumerator aEnum;
    aEnum.setMin( i_nMinNumber );
    aEnum.setMax( i_nMaxNumber );
    aEnum.setLogicalOffset( i_nLogicalOffset );

    bool bRes = aEnum.setRange( i_rPageRange );
    if( bRes )
    {
        o_rPageVector.clear();
        o_rPageVector.reserve( aEnum.size() );
        for( StringRangeEnumerator::Iterator it = aEnum.begin( i_pPossibleValues );
             it != aEnum.end( i_pPossibleValues ); ++it )
        {
            o_rPageVector.push_back( *it );
        }
    }

    return bRes;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
