/*************************************************************************
 *
 *  $RCSfile: multisel.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:08 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SV_MULTISEL_CXX

#ifdef MI_DEBUG
#define private public
#include <stdio.h>
#endif

#include <debug.hxx>
#include <multisel.hxx>

#pragma hdrstop

#ifdef MI_DEBUG
#define DBG(x) x
#else
#define DBG(x)
#endif

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

    Range* pRange = aSels.First();
    while ( pRange )
    {
        delete pRange;
        pRange = aSels.Next();
    }
    aSels.Clear();
}

// -----------------------------------------------------------------------

ULONG MultiSelection::ImplFindSubSelection( long nIndex ) const
{
    // iterate through the sub selections
    ULONG n = 0;
    for ( ;
          n < aSels.Count() && nIndex > aSels.GetObject(n)->Max();
          ++n ) {} /* empty loop */
    return n;
}

// -----------------------------------------------------------------------

BOOL MultiSelection::ImplMergeSubSelections( ULONG nPos1, ULONG nPos2 )
{
    // didn't a sub selection at nPos2 exist?
    if ( nPos2 >= aSels.Count() )
        return FALSE;

    // did the sub selections touch each other?
    if ( (aSels.GetObject(nPos1)->Max() + 1) == aSels.GetObject(nPos2)->Min() )
    {
        // merge them
        aSels.GetObject(nPos1)->Max() = aSels.GetObject(nPos2)->Max();
        delete aSels.Remove(nPos2);
        return TRUE;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

MultiSelection::MultiSelection():
    aTotRange( 0, -1 ),
    nSelCount(0),
    nCurSubSel(0),
    bCurValid(FALSE),
    bSelectNew(FALSE)
{
}

// -----------------------------------------------------------------------

MultiSelection::MultiSelection( const UniString& rString, sal_Unicode cRange, sal_Unicode cSep ):
    aTotRange(0,RANGE_MAX),
    nSelCount(0),
    nCurSubSel(0),
    bCurValid(FALSE),
    bSelectNew(FALSE)
{
    // Dies ist nur ein Schnellschuss und sollte bald optimiert,
    // an die verschiedenen Systeme (MAC, UNIX etc.)
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
    bCurValid(rOrig.bCurValid),
    nSelCount(rOrig.nSelCount),
    bSelectNew(FALSE)
{
    if ( bCurValid )
    {
        nCurSubSel = rOrig.nCurSubSel;
        nCurIndex = rOrig.nCurIndex;
    }

    // copy the sub selections
    for ( ULONG n = 0; n < rOrig.aSels.Count(); ++n )
        aSels.Insert( new Range( *rOrig.aSels.GetObject(n) ), LIST_APPEND );
}

// -----------------------------------------------------------------------

MultiSelection::MultiSelection( const Range& rRange ):
    aTotRange(rRange),
    nSelCount(0),
    nCurSubSel(0),
    bCurValid(FALSE),
    bSelectNew(FALSE)
{
}

// -----------------------------------------------------------------------

MultiSelection::~MultiSelection()
{
    Range* pRange = aSels.First();
    while ( pRange )
    {
        delete pRange;
        pRange = aSels.Next();
    }
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
    for ( ULONG n = 0; n < rOrig.aSels.Count(); ++n )
        aSels.Insert( new Range( *rOrig.aSels.GetObject(n) ), LIST_APPEND );
    nSelCount = rOrig.nSelCount;

    return *this;
}

// -----------------------------------------------------------------------

BOOL MultiSelection::operator== ( MultiSelection& rWith )
{
    if ( aTotRange != rWith.aTotRange || nSelCount != rWith.nSelCount ||
         aSels.Count() != rWith.aSels.Count() )
        return FALSE;

    // compare the sub seletions
    for ( ULONG n = 0; n < aSels.Count(); ++n )
        if ( *aSels.GetObject(n) != *rWith.aSels.GetObject(n) )
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
        aSels.Insert( new Range(aTotRange), LIST_APPEND );
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
    ULONG nSubSelPos = ImplFindSubSelection( nIndex );

    if ( bSelect )
    {
        // is it included in the found sub selection?
        if ( nSubSelPos < aSels.Count() &&
             aSels.GetObject(nSubSelPos)->IsInside( nIndex ) )
            // already selected, nothing to do
            return FALSE;

        // it will become selected
        ++nSelCount;

        // is it at the end of the previous sub selection
        if ( nSubSelPos > 0 &&
             aSels.GetObject(nSubSelPos-1)->Max() == (nIndex-1) )
        {
            // expand the previous sub selection
            aSels.GetObject(nSubSelPos-1)->Max() = nIndex;

            // try to merge the previous sub selection
            ImplMergeSubSelections( nSubSelPos-1, nSubSelPos );
        }
        // is is at the beginning of the found sub selection
        else if ( nSubSelPos < aSels.Count() &&
                  aSels.GetObject(nSubSelPos)->Min() == (nIndex+1) )
            // expand the found sub selection
            aSels.GetObject(nSubSelPos)->Min() = nIndex;
        else
        {
            // create a new sub selection
            aSels.Insert( new Range( nIndex, nIndex ), nSubSelPos );
            if ( bCurValid && nCurSubSel >= nSubSelPos )
                ++nCurSubSel;
        }
    }
    else
    {
        // is it excluded from the found sub selection?
        if ( nSubSelPos >= aSels.Count() ||
             !aSels.GetObject(nSubSelPos)->IsInside( nIndex ) )
        {
            // not selected, nothing to do
            DBG(Print( this ));
            return FALSE;
        }

        // it will become deselected
        --nSelCount;

        // is it the only index in the found sub selection?
        if ( aSels.GetObject(nSubSelPos)->Len() == 1 )
        {
            // remove the complete sub selection
            delete aSels.Remove( nSubSelPos );
            DBG(Print( this ));
            return TRUE;
        }

        // is it at the beginning of the found sub selection?
        if ( aSels.GetObject(nSubSelPos)->Min() == nIndex )
            ++aSels.GetObject(nSubSelPos)->Min();
        // is it at the end of the found sub selection?
        else if ( aSels.GetObject(nSubSelPos)->Max() == nIndex )
            --aSels.GetObject(nSubSelPos)->Max();
        // it is in the middle of the found sub selection?
        else
        {
            // split the sub selection
            aSels.Insert(
                new Range( aSels.GetObject(nSubSelPos)->Min(), nIndex-1 ),
                nSubSelPos );
            aSels.GetObject(nSubSelPos+1)->Min() = nIndex + 1;
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
            aSels.Insert( new Range(rIndexRange), LIST_APPEND );
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
                aSels.Insert( pRange, (ULONG)0 );
                nSelCount += pRange->Len();
            }
            else
            {
                pRange = aSels.First();
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
                aSels.Insert( pRange, LIST_APPEND );
                nSelCount += pRange->Len();
            }
            else
            {
                pRange = aSels.Last();
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
    ULONG nSubSelPos = ImplFindSubSelection( nIndex );

    return nSubSelPos < aSels.Count() &&
           aSels.GetObject(nSubSelPos)->IsInside(nIndex);
}

// -----------------------------------------------------------------------

void MultiSelection::Insert( long nIndex, long nCount )
{
    DBG(DbgOutf( "::Insert(%ld, %ld)\n", nIndex, nCount ));

    // find the virtual target position
    ULONG nSubSelPos = ImplFindSubSelection( nIndex );

    // did we need to shift the sub selections?
    if ( nSubSelPos < aSels.Count() )
    {
        // did we insert an unselected into an existing sub selection?
        if ( !bSelectNew && aSels.GetObject(nSubSelPos)->Min() != nIndex &&
                  aSels.GetObject(nSubSelPos)->IsInside(nIndex) )
        {
            // split the sub selection
            aSels.Insert(
                new Range( aSels.GetObject(nSubSelPos)->Min(), nIndex-1 ),
                nSubSelPos );
            ++nSubSelPos;
            aSels.GetObject(nSubSelPos)->Min() = nIndex;
        }

        // did we append an selected to an existing sub selection?
        else if ( bSelectNew && nSubSelPos > 0 &&
             aSels.GetObject(nSubSelPos)->Max() == nIndex-1 )
            // expand the previous sub selection
            aSels.GetObject(nSubSelPos-1)->Max() += nCount;

        // did we insert an selected into an existing sub selection?
        else if ( bSelectNew && aSels.GetObject(nSubSelPos)->Min() == nIndex )
        {
            // expand the sub selection
            aSels.GetObject(nSubSelPos)->Max() += nCount;
            ++nSubSelPos;
        }

        // shift the sub selections behind the inserting position
        for ( ULONG nPos = nSubSelPos; nPos < aSels.Count(); ++nPos )
        {
            aSels.GetObject(nPos)->Min() += nCount;
            aSels.GetObject(nPos)->Max() += nCount;
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
    ULONG nSubSelPos = ImplFindSubSelection( nIndex );

    // did we remove from an existing sub selection?
    if ( nSubSelPos < aSels.Count() &&
         aSels.GetObject(nSubSelPos)->IsInside(nIndex) )
    {
        // does this sub selection only contain the index to be deleted
        if ( aSels.GetObject(nSubSelPos)->Len() == 1 )
            // completely remove the sub selection
            aSels.Remove(nSubSelPos);
        else
            // shorten this sub selection
            --( aSels.GetObject(nSubSelPos++)->Max() );

        // adjust the selected counter
        --nSelCount;
    }

    // shift the sub selections behind the removed index
    for ( ULONG nPos = nSubSelPos; nPos < aSels.Count(); ++nPos )
    {
        --( aSels.GetObject(nPos)->Min() );
        --( aSels.GetObject(nPos)->Max() );
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
        aSels.Insert( new Range( nPrevLast+1, nPrevLast + nCount ),
                      LIST_APPEND );
        if ( aSels.Count() > 1 )
            ImplMergeSubSelections( aSels.Count() - 2, aSels.Count() );
    }
}

// -----------------------------------------------------------------------

long MultiSelection::ImplFwdUnselected()
{
    if ( !bCurValid )
        return SFX_ENDOFSELECTION;

    if ( ( nCurSubSel < aSels.Count() ) &&
         ( aSels.GetObject(nCurSubSel)->Min() <= nCurIndex ) )
        nCurIndex = aSels.GetObject(nCurSubSel++)->Max() + 1;

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

    if ( aSels.GetObject(nCurSubSel)->Max() < nCurIndex )
        return nCurIndex;

    nCurIndex = aSels.GetObject(nCurSubSel--)->Min() - 1;
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
        bCurValid = aSels.Count() > 0;
        if ( bCurValid )
            return nCurIndex = aSels.GetObject(0)->Min();
    }

    return SFX_ENDOFSELECTION;
}

// -----------------------------------------------------------------------

long MultiSelection::LastSelected( BOOL bInverse )
{
    nCurSubSel = aSels.Count() - 1;
    bCurValid = aSels.Count() > 0;

    if ( bCurValid )
        return nCurIndex = aSels.GetObject(nCurSubSel)->Max();

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
        if ( nCurIndex < aSels.GetObject(nCurSubSel)->Max() )
            return ++nCurIndex;

        // are there further sub selections?
        if ( ++nCurSubSel < aSels.Count() )
            return nCurIndex = aSels.GetObject(nCurSubSel)->Min();

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
        if ( nCurIndex > aSels.GetObject(nCurSubSel)->Min() )
            return --nCurIndex;

        // are there previous sub selections?
        if ( nCurSubSel > 0 )
        {
            --nCurSubSel;
            return nCurIndex = aSels.GetObject(nCurSubSel)->Max();
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
    Range* pRange = aSels.GetObject( 0 );
    while( pRange )
    {
        if( pRange->Max() < aTotRange.Min() )
        {
            delete pRange;
            aSels.Remove( (ULONG)0 );
        }
        else if( pRange->Min() < aTotRange.Min() )
        {
            pRange->Min() = aTotRange.Min();
            break;
        }
        else
            break;

        pRange = aSels.GetObject( 0 );
    }

    // die obere Bereichsgrenze anpassen
    ULONG nCount = aSels.Count();
    while( nCount )
    {
        pRange = aSels.GetObject( nCount - 1 );
        if( pRange->Min() > aTotRange.Max() )
        {
            delete pRange;
            aSels.Remove( (ULONG)(nCount - 1) );
        }
        else if( pRange->Max() > aTotRange.Max() )
        {
            pRange->Max() = aTotRange.Max();
            break;
        }
        else
            break;

        nCount = aSels.Count();
    }

    // Selection-Count neu berechnen
    nSelCount = 0;
    pRange = aSels.First();
    while( pRange )
    {
        nSelCount += pRange->Len();
        pRange = aSels.Next();
    }

    bCurValid = FALSE;
    nCurIndex = 0;
}
