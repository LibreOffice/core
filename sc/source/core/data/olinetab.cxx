/*************************************************************************
 *
 *  $RCSfile: olinetab.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:15 $
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

// System - Includes -----------------------------------------------------

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include <tools/debug.hxx>
#include <limits.h>

// INCLUDE ---------------------------------------------------------------

#include "olinetab.hxx"
#include "global.hxx"
#include "rechead.hxx"

//------------------------------------------------------------------------

ScOutlineEntry::ScOutlineEntry( USHORT nNewStart, USHORT nNewSize, BOOL bNewHidden ) :
    nStart  ( nNewStart ),
    nSize   ( nNewSize ),
    bHidden ( bNewHidden ),
    bVisible( TRUE )
{
}

ScOutlineEntry::ScOutlineEntry( const ScOutlineEntry& rEntry ) :
    nStart  ( rEntry.nStart ),
    nSize   ( rEntry.nSize ),
    bHidden ( rEntry.bHidden ),
    bVisible( rEntry.bVisible )
{
}

ScOutlineEntry::ScOutlineEntry( SvStream& rStream, ScMultipleReadHeader& rHdr )
{
    rHdr.StartEntry();

    rStream >> nStart;
    rStream >> nSize;
    rStream >> bHidden;
    rStream >> bVisible;

    rHdr.EndEntry();
}

void ScOutlineEntry::Store( SvStream& rStream, ScMultipleWriteHeader& rHdr )
{
    rHdr.StartEntry();

    rStream << nStart;
    rStream << nSize;
    rStream << bHidden;
    rStream << bVisible;

    rHdr.EndEntry();
}

DataObject* ScOutlineEntry::Clone() const
{
    return new ScOutlineEntry( *this );
}

void ScOutlineEntry::Move( short nDelta )
{
    short nNewPos = ((short) nStart) + nDelta;
    if (nNewPos<0)
    {
        DBG_ERROR("OutlineEntry < 0");
        nNewPos = 0;
    }
    nStart = (USHORT) nNewPos;
}

void ScOutlineEntry::SetSize( USHORT nNewSize )
{
    if (nNewSize)
        nSize = nNewSize;
    else
        DBG_ERROR("ScOutlineEntry Size == 0");
}

void ScOutlineEntry::SetPosSize( USHORT nNewPos, USHORT nNewSize )
{
    nStart = nNewPos;
    SetSize( nNewSize );
}

void ScOutlineEntry::SetHidden( BOOL bNewHidden )
{
    bHidden = bNewHidden;
}

void ScOutlineEntry::SetVisible( BOOL bNewVisible )
{
    bVisible = bNewVisible;
}

//------------------------------------------------------------------------

ScOutlineCollection::ScOutlineCollection() :
    SortedCollection( 4,4,FALSE )
{
}

inline short IntCompare( USHORT nX, USHORT nY )
{
    if ( nX==nY ) return 0;
    else if ( nX<nY ) return -1;
    else return 1;
}

short ScOutlineCollection::Compare(DataObject* pKey1, DataObject* pKey2) const
{
    return IntCompare( ((ScOutlineEntry*)pKey1)->GetStart(),
                        ((ScOutlineEntry*)pKey2)->GetStart() );
}

USHORT ScOutlineCollection::FindStart( USHORT nMinStart )
{
    //!                 binaer suchen ?

    USHORT nPos = 0;
    USHORT nCount = GetCount();
    while ( (nPos<nCount) ? (((ScOutlineEntry*)At(nPos))->GetStart() < nMinStart) : FALSE )
        ++nPos;

    return nPos;
}

//------------------------------------------------------------------------

ScOutlineArray::ScOutlineArray() :
    nDepth( 0 )
{
}

ScOutlineArray::ScOutlineArray( const ScOutlineArray& rArray ) :
    nDepth( rArray.nDepth )
{
    for (USHORT nLevel=0; nLevel<nDepth; nLevel++)
    {
        USHORT nCount = rArray.aCollections[nLevel].GetCount();
        for (USHORT nEntry=0; nEntry<nCount; nEntry++)
        {
            ScOutlineEntry* pEntry = (ScOutlineEntry*) rArray.aCollections[nLevel].At(nEntry);
            aCollections[nLevel].Insert( new ScOutlineEntry( *pEntry ) );
        }
    }
}

void ScOutlineArray::FindEntry( USHORT nSearchPos, USHORT& rFindLevel, USHORT& rFindIndex,
                                USHORT nMaxLevel )
{
    rFindLevel = rFindIndex = 0;

    if (nMaxLevel > nDepth)
        nMaxLevel = nDepth;

    for (USHORT nLevel=0; nLevel<nMaxLevel; nLevel++)               //! rueckwaerts suchen ?
    {
        ScOutlineCollection* pCollect = &aCollections[nLevel];
        USHORT nCount = pCollect->GetCount();
        for (USHORT i=0; i<nCount; i++)
        {
            ScOutlineEntry* pEntry = (ScOutlineEntry*) pCollect->At(i);
            if ( pEntry->GetStart() <= nSearchPos && pEntry->GetEnd() >= nSearchPos )
            {
                rFindLevel = nLevel + 1;            // naechster Level (zum Einfuegen)
                rFindIndex = i;
            }
        }
    }
}

BOOL ScOutlineArray::Insert( USHORT nStartCol, USHORT nEndCol, BOOL& rSizeChanged,
                                BOOL bHidden, BOOL bVisible )
{
    rSizeChanged = FALSE;

    USHORT nStartLevel;
    USHORT nStartIndex;
    USHORT nEndLevel;
    USHORT nEndIndex;
    BOOL bFound = FALSE;

    BOOL bCont;
    USHORT nFindMax;
    FindEntry( nStartCol, nStartLevel, nStartIndex );       // nLevel = neuer Level (alter+1) !!!
    FindEntry( nEndCol, nEndLevel, nEndIndex );
    nFindMax = Max(nStartLevel,nEndLevel);
    do
    {
        bCont = FALSE;

        if ( nStartLevel == nEndLevel && nStartIndex == nEndIndex && nStartLevel < SC_OL_MAXDEPTH )
            bFound = TRUE;

        if (!bFound)
        {
            if (nFindMax>0)
            {
                --nFindMax;
                if (nStartLevel)
                    if ( ((ScOutlineEntry*)aCollections[nStartLevel-1].At(nStartIndex))->
                                GetStart() == nStartCol )
                        FindEntry( nStartCol, nStartLevel, nStartIndex, nFindMax );
                if (nEndLevel)
                    if ( ((ScOutlineEntry*)aCollections[nEndLevel-1].At(nEndIndex))->
                                GetEnd() == nEndCol )
                        FindEntry( nEndCol, nEndLevel, nEndIndex, nFindMax );
                bCont = TRUE;
            }
        }
    }
    while ( !bFound && bCont );

    if (!bFound)
        return FALSE;

    USHORT nLevel = nStartLevel;

    //  untere verschieben

    BOOL bNeedSize = FALSE;
    for ( short nMoveLevel = nDepth-1; nMoveLevel >= (short) nLevel; nMoveLevel-- )
    {
        USHORT nCount = aCollections[nMoveLevel].GetCount();
        BOOL bMoved = FALSE;
        for ( USHORT i=0; i<nCount; i += bMoved ? 0 : 1 )
        {
            ScOutlineEntry* pEntry = (ScOutlineEntry*) aCollections[nMoveLevel].At(i);
            USHORT nEntryStart = pEntry->GetStart();
            if ( nEntryStart >= nStartCol && nEntryStart <= nEndCol )
            {
                if (nMoveLevel >= SC_OL_MAXDEPTH - 1)
                {
                    rSizeChanged = FALSE;               // kein Platz
                    return FALSE;
                }
                aCollections[nMoveLevel+1].Insert( new ScOutlineEntry( *pEntry ) );
                aCollections[nMoveLevel].AtFree( i );
                nCount = aCollections[nMoveLevel].GetCount();
                bMoved = TRUE;
                if (nMoveLevel == (short) nDepth - 1)
                    bNeedSize = TRUE;
            }
            else
                bMoved = FALSE;
        }
    }

    if (bNeedSize)
    {
        ++nDepth;
        rSizeChanged = TRUE;
    }

    if (nDepth <= nLevel)
    {
        nDepth = nLevel+1;
        rSizeChanged = TRUE;
    }

/*          nicht zusammenfassen!

    //  zusammenfassen

    USHORT nCount = aCollections[nLevel].GetCount();
    USHORT nIndex;
    bFound = FALSE;
    for ( nIndex=0; nIndex<nCount && !bFound; nIndex++ )
    {
        if ( ((ScOutlineEntry*) aCollections[nLevel].At(nIndex))->GetEnd() + 1 == nStartCol )
        {
            nStartCol = ((ScOutlineEntry*) aCollections[nLevel].At(nIndex))->GetStart();
            aCollections[nLevel].AtFree(nIndex);
            nCount = aCollections[nLevel].GetCount();       // Daten geaendert
            bFound = TRUE;
        }
    }

    bFound = FALSE;
    for ( nIndex=0; nIndex<nCount && !bFound; nIndex++ )
    {
        if ( ((ScOutlineEntry*) aCollections[nLevel].At(nIndex))->GetStart() == nEndCol + 1 )
        {
            nEndCol = ((ScOutlineEntry*) aCollections[nLevel].At(nIndex))->GetEnd();
            aCollections[nLevel].AtFree(nIndex);
            bFound = TRUE;
        }
    }
*/
    ScOutlineEntry* pNewEntry = new ScOutlineEntry( nStartCol, nEndCol+1-nStartCol, bHidden );
    pNewEntry->SetVisible( bVisible );
    aCollections[nLevel].Insert( pNewEntry );

    return TRUE;
}

BOOL ScOutlineArray::FindTouchedLevel( USHORT nBlockStart, USHORT nBlockEnd, USHORT& rFindLevel )
{
    BOOL bFound = FALSE;
    rFindLevel = 0;

    for (USHORT nLevel=0; nLevel<nDepth; nLevel++)
    {
        ScOutlineCollection* pCollect = &aCollections[nLevel];
        USHORT nCount = pCollect->GetCount();
        for (USHORT i=0; i<nCount; i++)
        {
            ScOutlineEntry* pEntry = (ScOutlineEntry*) pCollect->At(i);
            USHORT nStart = pEntry->GetStart();
            USHORT nEnd   = pEntry->GetEnd();

            if ( ( nBlockStart>=nStart && nBlockStart<=nEnd ) ||
                 ( nBlockEnd  >=nStart && nBlockEnd  <=nEnd ) )
            {
                rFindLevel = nLevel;            // wirklicher Level
                bFound = TRUE;
            }
        }
    }

    return bFound;
}

void ScOutlineArray::RemoveSub( USHORT nStartPos, USHORT nEndPos, USHORT nLevel )
{
    ScOutlineCollection* pCollect = &aCollections[nLevel];
    USHORT nCount = pCollect->GetCount();
    BOOL bFound = FALSE;
    for ( USHORT i=0; i<nCount; i += ( bFound ? 0 : 1 ) )
    {
        bFound = FALSE;
        ScOutlineEntry* pEntry = (ScOutlineEntry*) pCollect->At(i);
        USHORT nStart = pEntry->GetStart();
        USHORT nEnd   = pEntry->GetEnd();

        if ( nStart>=nStartPos && nEnd<=nEndPos )
        {
            RemoveSub( nStart, nEnd, nLevel+1 );
            pCollect->AtFree(i);
            nCount = pCollect->GetCount();
            bFound = TRUE;
        }
    }
}

void ScOutlineArray::PromoteSub( USHORT nStartPos, USHORT nEndPos, USHORT nStartLevel )
{
    if (nStartLevel==0)
    {
        DBG_ERROR("PromoteSub mit Level 0");
        return;
    }

    for (USHORT nLevel = nStartLevel; nLevel < nDepth; nLevel++)
    {
        ScOutlineCollection* pCollect = &aCollections[nLevel];
        USHORT nCount = pCollect->GetCount();
        BOOL bFound = FALSE;
        for ( USHORT i=0; i<nCount; i += ( bFound ? 0 : 1 ) )
        {
            bFound = FALSE;
            ScOutlineEntry* pEntry = (ScOutlineEntry*) pCollect->At(i);
            USHORT nStart = pEntry->GetStart();
            USHORT nEnd   = pEntry->GetEnd();

            if ( nStart>=nStartPos && nEnd<=nEndPos )
            {
                aCollections[nLevel-1].Insert( new ScOutlineEntry( *pEntry ) );
                pCollect->AtFree(i);
                nCount = pCollect->GetCount();
                bFound = TRUE;
            }
        }
    }
}

BOOL ScOutlineArray::DecDepth()                         // nDepth auf leere Levels anpassen
{
    BOOL bChanged = FALSE;
    BOOL bCont;
    do
    {
        bCont = FALSE;
        if (nDepth)
            if (aCollections[nDepth-1].GetCount() == 0)
            {
                --nDepth;
                bChanged = TRUE;
                bCont = TRUE;
            }
    }
    while (bCont);
    return bChanged;
}

BOOL ScOutlineArray::Remove( USHORT nBlockStart, USHORT nBlockEnd, BOOL& rSizeChanged )
{
    USHORT nLevel;
    FindTouchedLevel( nBlockStart, nBlockEnd, nLevel );

    ScOutlineCollection* pCollect = &aCollections[nLevel];
    USHORT nCount = pCollect->GetCount();
    BOOL bFound = FALSE;
    BOOL bAny = FALSE;
    for ( USHORT i=0; i<nCount; i += ( bFound ? 0 : 1 ) )
    {
        bFound = FALSE;
        ScOutlineEntry* pEntry = (ScOutlineEntry*) pCollect->At(i);
        USHORT nStart = pEntry->GetStart();
        USHORT nEnd   = pEntry->GetEnd();

        if ( nBlockStart<=nEnd && nBlockEnd>=nStart )
        {
//          RemoveSub( nStart, nEnd, nLevel+1 );
            pCollect->AtFree(i);
            PromoteSub( nStart, nEnd, nLevel+1 );
            nCount = pCollect->GetCount();
            i = pCollect->FindStart( nEnd+1 );
            bFound = TRUE;
            bAny = TRUE;
        }
    }

    if (bAny)                                   // Depth anpassen
        if (DecDepth())
            rSizeChanged = TRUE;

    return bAny;
}

ScOutlineEntry* ScOutlineArray::GetEntry( USHORT nLevel, USHORT nIndex )
{
    return (ScOutlineEntry*) aCollections[nLevel].At(nIndex);
}

USHORT ScOutlineArray::GetCount( USHORT nLevel )
{
    return aCollections[nLevel].GetCount();
}

ScOutlineEntry* ScOutlineArray::GetEntryByPos( USHORT nLevel, USHORT nPos )
{
    USHORT          nCount  = GetCount( nLevel );
    ScOutlineEntry* pEntry;

    for (USHORT nIndex = 0; nIndex < nCount; nIndex++)
    {
        pEntry = GetEntry( nLevel, nIndex );
        if ((pEntry->GetStart() <= nPos) && (nPos <= pEntry->GetEnd()))
            return pEntry;
    }
    return NULL;
}

void ScOutlineArray::SetVisibleBelow( USHORT nLevel, USHORT nEntry, BOOL bValue, BOOL bSkipHidden )
{
    ScOutlineEntry* pEntry = (ScOutlineEntry*) aCollections[nLevel].At(nEntry);
    USHORT nStart = pEntry->GetStart();
    USHORT nEnd   = pEntry->GetEnd();

    for (USHORT nSubLevel=nLevel+1; nSubLevel<nDepth; nSubLevel++)
    {
        USHORT i = 0;
        pEntry = (ScOutlineEntry*) aCollections[nSubLevel].At(i);
        while (pEntry)
        {
            if (pEntry->GetStart() >= nStart && pEntry->GetEnd() <= nEnd)
            {
                pEntry->SetVisible(bValue);

                if (bSkipHidden)
                    if (!pEntry->IsHidden())
                        SetVisibleBelow( nSubLevel, i, bValue, TRUE );
            }

            ++i;
            pEntry = (ScOutlineEntry*) aCollections[nSubLevel].At(i);
        }

        if (bSkipHidden)
            nSubLevel = nDepth;             // Abbruch
    }
}

void ScOutlineArray::GetRange( USHORT& rStart, USHORT& rEnd )
{
    USHORT nCount = aCollections[0].GetCount();
    if (nCount)
    {
        rStart = ((ScOutlineEntry*) aCollections[0].At(0))->GetStart();
        rEnd   = ((ScOutlineEntry*) aCollections[0].At(nCount-1))->GetEnd();
    }
    else
        rStart = rEnd = 0;
}

void ScOutlineArray::ExtendBlock( USHORT nLevel, USHORT& rBlkStart, USHORT& rBlkEnd )
{
    USHORT  nCount;
    USHORT  nStart;
    USHORT  nEnd;
    USHORT  i;
    ScOutlineEntry* pEntry;

    nCount = GetCount(nLevel);
    for ( i=0; i<nCount; i++ )
    {
        pEntry = (ScOutlineEntry*) aCollections[nLevel].At(i);
        nStart = pEntry->GetStart();
        nEnd   = pEntry->GetEnd();

        if ( rBlkStart<=nEnd && rBlkEnd>=nStart )
        {
            if (nStart<rBlkStart) rBlkStart = nStart;
            if (nEnd>rBlkEnd) rBlkEnd = nEnd;
        }
    }
}

BOOL ScOutlineArray::TestInsertSpace( USHORT nSize, USHORT nMaxVal )
{
    USHORT nCount = aCollections[0].GetCount();
    if (nCount)
    {
        USHORT nEnd = ((ScOutlineEntry*) aCollections[0].At(nCount-1))->GetEnd();
        return ( nEnd+nSize <= nMaxVal );
    }

    return TRUE;
}

void ScOutlineArray::InsertSpace( USHORT nStartPos, USHORT nSize )
{
    ScSubOutlineIterator aIter( this );
    ScOutlineEntry* pEntry;
    while((pEntry=aIter.GetNext())!=NULL)
    {
        if ( pEntry->GetStart() >= nStartPos )
            pEntry->Move(nSize);
        else
        {
            USHORT nEnd = pEntry->GetEnd();
            //  immer erweitern, wenn innerhalb der Gruppe eingefuegt
            //  beim Einfuegen am Ende nur, wenn die Gruppe nicht ausgeblendet ist
            if ( nEnd >= nStartPos || ( nEnd+1 >= nStartPos && !pEntry->IsHidden() ) )
            {
                USHORT nEntrySize = pEntry->GetSize();
                nEntrySize += nSize;
                pEntry->SetSize( nEntrySize );
            }
        }
    }
}

BOOL ScOutlineArray::DeleteSpace( USHORT nStartPos, USHORT nSize )
{
    USHORT nEndPos = nStartPos + nSize - 1;
    BOOL bNeedSave = FALSE;                         // Original fuer Undo benoetigt?
    BOOL bChanged = FALSE;                          // fuer Test auf Level

    ScSubOutlineIterator aIter( this );
    ScOutlineEntry* pEntry;
    while((pEntry=aIter.GetNext())!=NULL)
    {
        USHORT nEntryStart = pEntry->GetStart();
        USHORT nEntryEnd   = pEntry->GetEnd();
        USHORT nEntrySize  = pEntry->GetSize();

        if ( nEntryEnd >= nStartPos )
        {
            if ( nEntryStart > nEndPos )                                        // rechts
                pEntry->Move(-(short)nSize);
            else if ( nEntryStart < nStartPos && nEntryEnd >= nEndPos )         // aussen
                pEntry->SetSize( nEntrySize-nSize );
            else
            {
                bNeedSave = TRUE;
                if ( nEntryStart >= nStartPos && nEntryEnd <= nEndPos )             // innen
                {
                    aIter.DeleteLast();
                    bChanged = TRUE;
                }
                else if ( nEntryStart >= nStartPos )                                // rechts ueber
                    pEntry->SetPosSize( nStartPos, nEntryEnd-nEndPos );
                else                                                                // links ueber
                    pEntry->SetSize( nStartPos-nEntryStart );
            }
        }
    }

    if (bChanged)
        DecDepth();

    return bNeedSave;
}

BOOL ScOutlineArray::ManualAction( USHORT nStartPos, USHORT nEndPos, BOOL bShow, BYTE* pHiddenFlags )
{
    BOOL bModified = FALSE;
    ScSubOutlineIterator aIter( this );
    ScOutlineEntry* pEntry;
    while((pEntry=aIter.GetNext())!=NULL)
    {
        USHORT nEntryStart = pEntry->GetStart();
        USHORT nEntryEnd   = pEntry->GetEnd();

        if (nEntryEnd>=nStartPos && nEntryStart<=nEndPos)
        {
            if ( pEntry->IsHidden() == bShow )
            {
                USHORT i;
                BOOL bToggle = TRUE;

                for (i=nEntryStart; i<=nEntryEnd && bToggle; i++)
                {
                    BOOL bEntryHidden = (pHiddenFlags[i] & CR_HIDDEN) != 0;
                    if ( bEntryHidden == bShow )
                        bToggle = FALSE;
                }

                if (bToggle)
                {
                    pEntry->SetHidden( !bShow );
                    SetVisibleBelow( aIter.LastLevel(), aIter.LastEntry(), bShow, bShow );
                    bModified = TRUE;
                }
            }
        }
    }
    return bModified;
}

void ScOutlineArray::RemoveAll()
{
    for (USHORT nLevel=0; nLevel<nDepth; nLevel++)
        aCollections[nLevel].FreeAll();

    nDepth = 0;
}

void ScOutlineArray::Load( SvStream& rStream )
{
    ScMultipleReadHeader aHdr( rStream );

    rStream >> nDepth;
    for (USHORT nLevel=0; nLevel<nDepth; nLevel++)
    {
        USHORT nCount;
        rStream >> nCount;
        for (USHORT nIndex=0; nIndex<nCount; nIndex++)
        {
            ScOutlineEntry* pEntry = new ScOutlineEntry( rStream, aHdr );
            aCollections[nLevel].Insert( pEntry );
        }
    }
}

void ScOutlineArray::Store( SvStream& rStream )
{
    ScMultipleWriteHeader aHdr( rStream );

    rStream << nDepth;
    for (USHORT nLevel=0; nLevel<nDepth; nLevel++)
    {
        USHORT nCount = aCollections[nLevel].GetCount();
        rStream << nCount;
        for (USHORT nIndex=0; nIndex<nCount; nIndex++)
            ((ScOutlineEntry*) aCollections[nLevel].At(nIndex))->Store( rStream, aHdr );
    }
}

//------------------------------------------------------------------------

ScOutlineTable::ScOutlineTable()
{
}

ScOutlineTable::ScOutlineTable( const ScOutlineTable& rOutline ) :
    aColOutline( rOutline.aColOutline ),
    aRowOutline( rOutline.aRowOutline )
{
}

BOOL ScOutlineTable::TestInsertCol( USHORT nSize )
{
    return aColOutline.TestInsertSpace( nSize, MAXCOL );
}

void ScOutlineTable::InsertCol( USHORT nStartCol, USHORT nSize )
{
    aColOutline.InsertSpace( nStartCol, nSize );
}

BOOL ScOutlineTable::DeleteCol( USHORT nStartCol, USHORT nSize )
{
    return aColOutline.DeleteSpace( nStartCol, nSize );
}

BOOL ScOutlineTable::TestInsertRow( USHORT nSize )
{
    return aRowOutline.TestInsertSpace( nSize, MAXROW );
}

void ScOutlineTable::InsertRow( USHORT nStartRow, USHORT nSize )
{
    aRowOutline.InsertSpace( nStartRow, nSize );
}

BOOL ScOutlineTable::DeleteRow( USHORT nStartRow, USHORT nSize )
{
    return aRowOutline.DeleteSpace( nStartRow, nSize );
}

void ScOutlineTable::Load( SvStream& rStream )
{
    DBG_ASSERT( aColOutline.GetDepth()==0 && aRowOutline.GetDepth()==0,
                    "Load auf nicht leere ScOutlineTable" );
    aColOutline.Load( rStream );
    aRowOutline.Load( rStream );
}

void ScOutlineTable::Store( SvStream& rStream )
{
    aColOutline.Store( rStream );
    aRowOutline.Store( rStream );
}

//------------------------------------------------------------------------

ScSubOutlineIterator::ScSubOutlineIterator( ScOutlineArray* pOutlineArray ) :
        pArray( pOutlineArray ),
        nStart( 0 ),
        nEnd( USHRT_MAX ),                          // alle durchgehen
        nSubLevel( 0 ),
        nSubEntry( 0 )
{
    nDepth = pArray->nDepth;
}

ScSubOutlineIterator::ScSubOutlineIterator( ScOutlineArray* pOutlineArray,
                                            USHORT nLevel, USHORT nEntry ) :
        pArray( pOutlineArray )
{
    ScOutlineEntry* pEntry = (ScOutlineEntry*) pArray->aCollections[nLevel].At(nEntry);
    nStart = pEntry->GetStart();
    nEnd   = pEntry->GetEnd();
    nSubLevel = nLevel + 1;
    nSubEntry = 0;
    nDepth = pArray->nDepth;
}

ScOutlineEntry* ScSubOutlineIterator::GetNext()
{
    ScOutlineEntry* pEntry;
    BOOL bFound = FALSE;
    do
    {
        if (nSubLevel >= nDepth)
            return NULL;

        pEntry = (ScOutlineEntry*) pArray->aCollections[nSubLevel].At(nSubEntry);
        if (!pEntry)
        {
            nSubEntry = 0;
            ++nSubLevel;
        }
        else
        {
            if ( pEntry->GetStart() >= nStart && pEntry->GetEnd() <= nEnd )
                bFound = TRUE;
            ++nSubEntry;
        }
    }
    while (!bFound);
    return pEntry;                  // nSubLevel gueltig, wenn pEntry != 0
}

USHORT ScSubOutlineIterator::LastLevel() const
{
    return nSubLevel;
}

USHORT ScSubOutlineIterator::LastEntry() const
{
    if (nSubEntry == 0)
    {
        DBG_ERROR("ScSubOutlineIterator::LastEntry vor GetNext");
        return 0;
    }
    return nSubEntry-1;
}

void ScSubOutlineIterator::DeleteLast()
{
    if (nSubLevel >= nDepth)
    {
        DBG_ERROR("ScSubOutlineIterator::DeleteLast nach Ende");
        return;
    }
    if (nSubEntry == 0)
    {
        DBG_ERROR("ScSubOutlineIterator::DeleteLast vor GetNext");
        return;
    }

    --nSubEntry;
    pArray->aCollections[nSubLevel].AtFree(nSubEntry);
}


