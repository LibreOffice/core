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

#include <limits.h>

#include "olinetab.hxx"
#include "global.hxx"
#include "rechead.hxx"
#include "address.hxx"
#include "table.hxx"

//------------------------------------------------------------------------

ScOutlineEntry::ScOutlineEntry( SCCOLROW nNewStart, SCCOLROW nNewSize, bool bNewHidden ) :
    nStart  ( nNewStart ),
    nSize   ( nNewSize ),
    bHidden ( bNewHidden ),
    bVisible( true )
{
}

ScOutlineEntry::ScOutlineEntry( const ScOutlineEntry& rEntry ) :
    nStart  ( rEntry.nStart ),
    nSize   ( rEntry.nSize ),
    bHidden ( rEntry.bHidden ),
    bVisible( rEntry.bVisible )
{
}

SCCOLROW ScOutlineEntry::GetStart() const
{
    return nStart;
}

SCSIZE ScOutlineEntry::GetSize() const
{
    return nSize;
}

SCCOLROW ScOutlineEntry::GetEnd() const
{
    return nStart+nSize-1;
}
bool ScOutlineEntry::IsHidden() const
{
    return bHidden;
}

bool ScOutlineEntry::IsVisible() const
{
    return bVisible;
}

void ScOutlineEntry::Move( SCsCOLROW nDelta )
{
    SCCOLROW nNewPos = nStart + nDelta;
    if (nNewPos<0)
    {
        OSL_FAIL("OutlineEntry < 0");
        nNewPos = 0;
    }
    nStart = nNewPos;
}

void ScOutlineEntry::SetSize( SCSIZE nNewSize )
{
    if (nNewSize>0)
        nSize = nNewSize;
    else
    {
        OSL_FAIL("ScOutlineEntry Size == 0");
    }
}

void ScOutlineEntry::SetPosSize( SCCOLROW nNewPos, SCSIZE nNewSize )
{
    nStart = nNewPos;
    SetSize( nNewSize );
}

void ScOutlineEntry::SetHidden( bool bNewHidden )
{
    bHidden = bNewHidden;
}

void ScOutlineEntry::SetVisible( bool bNewVisible )
{
    bVisible = bNewVisible;
}

//------------------------------------------------------------------------

ScOutlineCollection::ScOutlineCollection() {}

size_t ScOutlineCollection::size() const
{
    return maEntries.size();
}

void ScOutlineCollection::clear()
{
    maEntries.clear();
}

void ScOutlineCollection::insert(ScOutlineEntry* pEntry)
{
    SCCOLROW nStart = pEntry->GetStart();
    maEntries.insert(nStart, pEntry);
}

ScOutlineCollection::iterator ScOutlineCollection::begin()
{
    return maEntries.begin();
}

ScOutlineCollection::iterator ScOutlineCollection::end()
{
    return maEntries.end();
}

ScOutlineCollection::const_iterator ScOutlineCollection::begin() const
{
    return maEntries.begin();
}

ScOutlineCollection::const_iterator ScOutlineCollection::end() const
{
    return maEntries.end();
}

void ScOutlineCollection::erase(iterator pos)
{
    maEntries.erase(pos);
}

bool ScOutlineCollection::empty() const
{
    return maEntries.empty();
}

ScOutlineCollection::iterator ScOutlineCollection::FindStart(SCCOLROW nMinStart)
{
    return maEntries.lower_bound(nMinStart);
}

//------------------------------------------------------------------------

ScOutlineArray::ScOutlineArray() :
    nDepth(0) {}

ScOutlineArray::ScOutlineArray( const ScOutlineArray& rArray ) :
    nDepth( rArray.nDepth )
{
    for (size_t nLevel = 0; nLevel < nDepth; ++nLevel)
    {
        const ScOutlineCollection& rColl = rArray.aCollections[nLevel];
        ScOutlineCollection::const_iterator it = rColl.begin(), itEnd = rColl.end();
        for (; it != itEnd; ++it)
        {
            const ScOutlineEntry* pEntry = it->second;
            aCollections[nLevel].insert(new ScOutlineEntry(*pEntry));
        }
    }
}

void ScOutlineArray::FindEntry(
    SCCOLROW nSearchPos, size_t& rFindLevel, size_t& rFindIndex,
    size_t nMaxLevel )
{
    rFindLevel = rFindIndex = 0;

    if (nMaxLevel > nDepth)
        nMaxLevel = nDepth;

    for (size_t nLevel = 0; nLevel < nMaxLevel; ++nLevel)               //! rueckwaerts suchen ?
    {
        ScOutlineCollection* pCollect = &aCollections[nLevel];
        ScOutlineCollection::iterator it = pCollect->begin(), itEnd = pCollect->end();
        for (; it != itEnd; ++it)
        {
            ScOutlineEntry* pEntry = it->second;
            if (pEntry->GetStart() <= nSearchPos && pEntry->GetEnd() >= nSearchPos)
            {
                rFindLevel = nLevel + 1;            // naechster Level (zum Einfuegen)
                rFindIndex = std::distance(pCollect->begin(), it);
            }
        }
    }
}

bool ScOutlineArray::Insert(
    SCCOLROW nStartCol, SCCOLROW nEndCol, bool& rSizeChanged, bool bHidden, bool bVisible )
{
    rSizeChanged = false;

    size_t nStartLevel, nEndLevel, nStartIndex, nEndIndex;
    bool bFound = false;

    bool bCont;
    sal_uInt16 nFindMax;
    FindEntry( nStartCol, nStartLevel, nStartIndex );       // nLevel = neuer Level (alter+1) !!!
    FindEntry( nEndCol, nEndLevel, nEndIndex );
    nFindMax = std::max(nStartLevel,nEndLevel);
    do
    {
        bCont = false;

        if (nStartLevel == nEndLevel && nStartIndex == nEndIndex && nStartLevel < SC_OL_MAXDEPTH)
            bFound = true;

        if (!bFound)
        {
            if (nFindMax>0)
            {
                --nFindMax;
                if (nStartLevel)
                {
                    ScOutlineCollection::const_iterator it = aCollections[nStartLevel-1].begin();
                    std::advance(it, nStartIndex);
                    if (it->second->GetStart() == nStartCol)
                        FindEntry(nStartCol, nStartLevel, nStartIndex, nFindMax);
                }

                if (nEndLevel)
                {
                    ScOutlineCollection::const_iterator it = aCollections[nEndLevel-1].begin();
                    std::advance(it, nEndIndex);
                    if (it->second->GetEnd() == nEndCol)
                        FindEntry(nEndCol, nEndLevel, nEndIndex, nFindMax);
                }
                bCont = true;
            }
        }
    }
    while ( !bFound && bCont );

    if (!bFound)
        return false;

    size_t nLevel = nStartLevel;

    //  untere verschieben

    bool bNeedSize = false;
    if (nDepth > 0)
    {
        for (size_t nMoveLevel = nDepth-1; nMoveLevel >= nLevel; --nMoveLevel)
        {
            ScOutlineCollection& rColl = aCollections[nMoveLevel];
            ScOutlineCollection::iterator it = rColl.begin(), itEnd = rColl.end();
            while (it != itEnd)
            {
                ScOutlineEntry* pEntry = it->second;
                SCCOLROW nEntryStart = pEntry->GetStart();
                if (nEntryStart >= nStartCol && nEntryStart <= nEndCol)
                {
                    if (nMoveLevel >= SC_OL_MAXDEPTH - 1)
                    {
                        rSizeChanged = false;               // kein Platz
                        return false;
                    }
                    aCollections[nMoveLevel+1].insert(new ScOutlineEntry(*pEntry));
                    size_t nPos = std::distance(rColl.begin(), it);
                    rColl.erase(it);
                    it = rColl.begin();
                    std::advance(it, nPos);
                    itEnd = rColl.end();
                    if (nMoveLevel == nDepth - 1)
                        bNeedSize = true;
                }
                else
                    ++it;
            }
            if (nMoveLevel == 0)
                break;
        }
    }

    if (bNeedSize)
    {
        ++nDepth;
        rSizeChanged = true;
    }

    if (nDepth <= nLevel)
    {
        nDepth = nLevel+1;
        rSizeChanged = true;
    }

    ScOutlineEntry* pNewEntry = new ScOutlineEntry( nStartCol, nEndCol+1-nStartCol, bHidden );
    pNewEntry->SetVisible( bVisible );
    aCollections[nLevel].insert(pNewEntry);

    return true;
}

size_t ScOutlineArray::GetDepth() const
{
    return nDepth;
}

bool ScOutlineArray::FindTouchedLevel(
    SCCOLROW nBlockStart, SCCOLROW nBlockEnd, size_t& rFindLevel) const
{
    bool bFound = false;
    rFindLevel = 0;

    for (size_t nLevel = 0; nLevel < nDepth; ++nLevel)
    {
        const ScOutlineCollection* pCollect = &aCollections[nLevel];
        ScOutlineCollection::const_iterator it = pCollect->begin(), itEnd = pCollect->end();
        for (; it != itEnd; ++it)
        {
            const ScOutlineEntry* pEntry = it->second;
            SCCOLROW nStart = pEntry->GetStart();
            SCCOLROW nEnd   = pEntry->GetEnd();

            if ( ( nBlockStart>=nStart && nBlockStart<=nEnd ) ||
                 ( nBlockEnd  >=nStart && nBlockEnd  <=nEnd ) )
            {
                rFindLevel = nLevel;            // wirklicher Level
                bFound = true;
            }
        }
    }

    return bFound;
}

void ScOutlineArray::RemoveSub(SCCOLROW nStartPos, SCCOLROW nEndPos, size_t nLevel)
{
    if ( nLevel >= nDepth )
        return;

    ScOutlineCollection& rColl = aCollections[nLevel];

    ScOutlineCollection::iterator it = rColl.begin(), itEnd = rColl.end();
    while (it != itEnd)
    {
        ScOutlineEntry* pEntry = it->second;
        SCCOLROW nStart = pEntry->GetStart();
        SCCOLROW nEnd   = pEntry->GetEnd();
        if (nStart >= nStartPos && nEnd <= nEndPos)
        {
            // Overlaps.
            RemoveSub( nStart, nEnd, nLevel+1 );

            // Re-calc iterator positions after the tree gets invalidated.
            size_t nPos = std::distance(rColl.begin(), it);
            rColl.erase(it);
            it = rColl.begin();
            std::advance(it, nPos);
            itEnd = rColl.end();
        }
        else
            ++it;
    }

    it = rColl.begin();
    itEnd = rColl.end();

    while (it != itEnd)
    {
        ScOutlineEntry* pEntry = it->second;
        SCCOLROW nStart = pEntry->GetStart();
        SCCOLROW nEnd   = pEntry->GetEnd();

        if (nStart >= nStartPos && nEnd <= nEndPos)
        {
            RemoveSub( nStart, nEnd, nLevel+1 );

            // Re-calc iterator positions after the tree gets invalidated.
            size_t nPos = std::distance(rColl.begin(), it);
            rColl.erase(it);
            it = rColl.begin();
            std::advance(it, nPos);
            itEnd = rColl.end();
        }
        else
            ++it;
    }
}

void ScOutlineArray::PromoteSub(SCCOLROW nStartPos, SCCOLROW nEndPos, size_t nStartLevel)
{
    if (nStartLevel==0)
    {
        OSL_FAIL("PromoteSub mit Level 0");
        return;
    }

    for (size_t nLevel = nStartLevel; nLevel < nDepth; ++nLevel)
    {
        ScOutlineCollection& rColl = aCollections[nLevel];
        ScOutlineCollection::iterator it = rColl.begin(), itEnd = rColl.end();
        while (it != itEnd)
        {
            ScOutlineEntry* pEntry = it->second;
            SCCOLROW nStart = pEntry->GetStart();
            SCCOLROW nEnd   = pEntry->GetEnd();
            if (nStart >= nStartPos && nEnd <= nEndPos)
            {
                aCollections[nLevel-1].insert(new ScOutlineEntry(*pEntry));

                // Re-calc iterator positions after the tree gets invalidated.
                size_t nPos = std::distance(rColl.begin(), it);
                rColl.erase(it);
                it = rColl.begin();
                std::advance(it, nPos);
                itEnd = rColl.end();
            }
            else
                ++it;
        }

        it = rColl.begin();
        itEnd = rColl.end();

        while (it != itEnd)
        {
            ScOutlineEntry* pEntry = it->second;
            SCCOLROW nStart = pEntry->GetStart();
            SCCOLROW nEnd   = pEntry->GetEnd();
            if (nStart >= nStartPos && nEnd <= nEndPos)
            {
                aCollections[nLevel-1].insert(new ScOutlineEntry(*pEntry));

                // Re-calc iterator positions after the tree gets invalidated.
                size_t nPos = std::distance(rColl.begin(), it);
                rColl.erase(it);
                it = rColl.begin();
                std::advance(it, nPos);
                itEnd = rColl.end();
            }
            else
                ++it;
        }
    }
}

bool ScOutlineArray::DecDepth()                         // nDepth auf leere Levels anpassen
{
    bool bChanged = false;
    bool bCont;
    do
    {
        bCont = false;
        if (nDepth)
        {
            if (aCollections[nDepth-1].empty())
            {
                --nDepth;
                bChanged = true;
                bCont = true;
            }
        }
    }
    while (bCont);

    return bChanged;
}

bool ScOutlineArray::Remove( SCCOLROW nBlockStart, SCCOLROW nBlockEnd, bool& rSizeChanged )
{
    size_t nLevel;
    FindTouchedLevel( nBlockStart, nBlockEnd, nLevel );

    ScOutlineCollection* pCollect = &aCollections[nLevel];
    ScOutlineCollection::iterator it = pCollect->begin(), itEnd = pCollect->end();
    bool bAny = false;
    while (it != itEnd)
    {
        ScOutlineEntry* pEntry = it->second;
        SCCOLROW nStart = pEntry->GetStart();
        SCCOLROW nEnd   = pEntry->GetEnd();
        if (nBlockStart <= nEnd && nBlockEnd >= nStart)
        {
            // Overlaps.
            pCollect->erase(it);
            PromoteSub( nStart, nEnd, nLevel+1 );
            itEnd = pCollect->end();
            it = pCollect->FindStart( nEnd+1 );
            bAny = true;
        }
        else
            ++it;
    }

    if (bAny)                                   // Depth anpassen
        if (DecDepth())
            rSizeChanged = true;

    return bAny;
}

ScOutlineEntry* ScOutlineArray::GetEntry(size_t nLevel, size_t nIndex)
{
    if (nLevel >= nDepth)
        return NULL;

    ScOutlineCollection& rColl = aCollections[nLevel];
    if (nIndex >= rColl.size())
        return NULL;

    ScOutlineCollection::iterator it = rColl.begin();
    std::advance(it, nIndex);
    return it->second;
}

const ScOutlineEntry* ScOutlineArray::GetEntry(size_t nLevel, size_t nIndex) const
{
    if (nLevel >= nDepth)
        return NULL;

    const ScOutlineCollection& rColl = aCollections[nLevel];
    if (nIndex >= rColl.size())
        return NULL;

    ScOutlineCollection::const_iterator it = rColl.begin();
    std::advance(it, nIndex);
    return it->second;
}

size_t ScOutlineArray::GetCount(size_t nLevel) const
{
    if (nLevel >= nDepth)
        return 0;

    return aCollections[nLevel].size();
}

const ScOutlineEntry* ScOutlineArray::GetEntryByPos(size_t nLevel, SCCOLROW nPos) const
{
    if (nLevel >= nDepth)
        return NULL;

    const ScOutlineCollection& rColl = aCollections[nLevel];
    ScOutlineCollection::const_iterator it = rColl.begin(), itEnd = rColl.end();
    for (; it != itEnd; ++it)
    {
        const ScOutlineEntry* pEntry = it->second;
        if (pEntry->GetStart() <= nPos && nPos <= pEntry->GetEnd())
            return pEntry;
    }

    return NULL;
}

bool ScOutlineArray::GetEntryIndex(size_t nLevel, SCCOLROW nPos, size_t& rnIndex) const
{
    if (nLevel >= nDepth)
        return false;

    // found entry contains passed position
    const ScOutlineCollection& rColl = aCollections[nLevel];
    ScOutlineCollection::const_iterator it = rColl.begin(), itEnd = rColl.end();
    for (; it != itEnd; ++it)
    {
        const ScOutlineEntry* p = it->second;
        if (p->GetStart() <= nPos && nPos <= p->GetEnd())
        {
            rnIndex = std::distance(rColl.begin(), it);
            return true;
        }
    }
    return false;
}

bool ScOutlineArray::GetEntryIndexInRange(
    size_t nLevel, SCCOLROW nBlockStart, SCCOLROW nBlockEnd, size_t& rnIndex) const
{
    if (nLevel >= nDepth)
        return false;

    // found entry will be completely inside of passed range
    const ScOutlineCollection& rColl = aCollections[nLevel];
    ScOutlineCollection::const_iterator it = rColl.begin(), itEnd = rColl.end();
    for (; it != itEnd; ++it)
    {
        const ScOutlineEntry* p = it->second;
        if (nBlockStart <= p->GetStart() && p->GetEnd() <= nBlockEnd)
        {
            rnIndex = std::distance(rColl.begin(), it);
            return true;
        }
    }
    return false;
}

void ScOutlineArray::SetVisibleBelow(
    size_t nLevel, size_t nEntry, bool bValue, bool bSkipHidden)
{
    const ScOutlineEntry* pEntry = GetEntry( nLevel, nEntry );
    if (!pEntry)
        return;

    SCCOLROW nStart = pEntry->GetStart();
    SCCOLROW nEnd   = pEntry->GetEnd();

    for (size_t nSubLevel = nLevel+1; nSubLevel < nDepth; ++nSubLevel)
    {
        ScOutlineCollection& rColl = aCollections[nSubLevel];
        ScOutlineCollection::iterator it = rColl.begin(), itEnd = rColl.end();
        for (; it != itEnd; ++it)
        {
            ScOutlineEntry* p = it->second;
            if (p->GetStart() >= nStart && p->GetEnd() <= nEnd)
            {
                p->SetVisible(bValue);
                if (bSkipHidden && !p->IsHidden())
                {
                    size_t nPos = std::distance(rColl.begin(), it);
                    SetVisibleBelow(nSubLevel, nPos, bValue, true);
                }
            }
        }

        if (bSkipHidden)
            nSubLevel = nDepth;             // Abbruch
    }
}

void ScOutlineArray::GetRange(SCCOLROW& rStart, SCCOLROW& rEnd) const
{
    const ScOutlineCollection& rColl = aCollections[0];
    if (!rColl.empty())
    {
        ScOutlineCollection::const_iterator it = rColl.begin();
        rStart = it->second->GetStart();
        std::advance(it, rColl.size()-1);
        rEnd = it->second->GetEnd();
    }
    else
        rStart = rEnd = 0;
}

void ScOutlineArray::ExtendBlock(size_t nLevel, SCCOLROW& rBlkStart, SCCOLROW& rBlkEnd)
{
    if (nLevel >= nDepth)
        return;

    const ScOutlineCollection& rColl = aCollections[nLevel];
    ScOutlineCollection::const_iterator it = rColl.begin(), itEnd = rColl.end();
    for (; it != itEnd; ++it)
    {
        const ScOutlineEntry* pEntry = it->second;
        SCCOLROW nStart = pEntry->GetStart();
        SCCOLROW nEnd   = pEntry->GetEnd();

        if (rBlkStart <= nEnd && rBlkEnd >= nStart)
        {
            if (nStart < rBlkStart)
                rBlkStart = nStart;
            if (nEnd > rBlkEnd)
                rBlkEnd = nEnd;
        }
    }
}

bool ScOutlineArray::TestInsertSpace(SCSIZE nSize, SCCOLROW nMaxVal) const
{
    const ScOutlineCollection& rColl = aCollections[0];
    if (rColl.empty())
        return true;

    ScOutlineCollection::const_iterator it = rColl.begin();
    std::advance(it, rColl.size()-1);
    SCCOLROW nEnd = it->second->GetEnd();
    return sal::static_int_cast<SCCOLROW>(nEnd+nSize) <= nMaxVal;
}

void ScOutlineArray::InsertSpace(SCCOLROW nStartPos, SCSIZE nSize)
{
    ScSubOutlineIterator aIter( this );
    ScOutlineEntry* pEntry;
    while ((pEntry = aIter.GetNext()) != NULL)
    {
        if ( pEntry->GetStart() >= nStartPos )
            pEntry->Move(static_cast<SCsCOLROW>(nSize));
        else
        {
            SCCOLROW nEnd = pEntry->GetEnd();
            //  immer erweitern, wenn innerhalb der Gruppe eingefuegt
            //  beim Einfuegen am Ende nur, wenn die Gruppe nicht ausgeblendet ist
            if ( nEnd >= nStartPos || ( nEnd+1 >= nStartPos && !pEntry->IsHidden() ) )
            {
                SCSIZE nEntrySize = pEntry->GetSize();
                nEntrySize += nSize;
                pEntry->SetSize( nEntrySize );
            }
        }
    }
}

bool ScOutlineArray::DeleteSpace(SCCOLROW nStartPos, SCSIZE nSize)
{
    SCCOLROW nEndPos = nStartPos + nSize - 1;
    sal_Bool bNeedSave = false;                         // Original fuer Undo benoetigt?
    sal_Bool bChanged = false;                          // fuer Test auf Level

    ScSubOutlineIterator aIter( this );
    ScOutlineEntry* pEntry;
    while((pEntry=aIter.GetNext())!=NULL)
    {
        SCCOLROW nEntryStart = pEntry->GetStart();
        SCCOLROW nEntryEnd   = pEntry->GetEnd();
        SCSIZE nEntrySize    = pEntry->GetSize();

        if ( nEntryEnd >= nStartPos )
        {
            if ( nEntryStart > nEndPos )                                        // rechts
                pEntry->Move(-(static_cast<SCsCOLROW>(nSize)));
            else if ( nEntryStart < nStartPos && nEntryEnd >= nEndPos )         // aussen
                pEntry->SetSize( nEntrySize-nSize );
            else
            {
                bNeedSave = true;
                if ( nEntryStart >= nStartPos && nEntryEnd <= nEndPos )             // innen
                {
                    aIter.DeleteLast();
                    bChanged = true;
                }
                else if ( nEntryStart >= nStartPos )                                // rechts ueber
                    pEntry->SetPosSize( nStartPos, static_cast<SCSIZE>(nEntryEnd-nEndPos) );
                else                                                                // links ueber
                    pEntry->SetSize( static_cast<SCSIZE>(nStartPos-nEntryStart) );
            }
        }
    }

    if (bChanged)
        DecDepth();

    return bNeedSave;
}

bool ScOutlineArray::ManualAction(
    SCCOLROW nStartPos, SCCOLROW nEndPos, bool bShow, const ScTable& rTable, bool bCol)
{
    bool bModified = false;
    ScSubOutlineIterator aIter( this );
    ScOutlineEntry* pEntry;
    while((pEntry=aIter.GetNext())!=NULL)
    {
        SCCOLROW nEntryStart = pEntry->GetStart();
        SCCOLROW nEntryEnd   = pEntry->GetEnd();

        if (nEntryEnd>=nStartPos && nEntryStart<=nEndPos)
        {
            if ( pEntry->IsHidden() == bShow )
            {
                //  #i12341# hide if all columns/rows are hidden, show if at least one
                //  is visible
                SCCOLROW nEnd = rTable.LastHiddenColRow(nEntryStart, bCol);
                bool bAllHidden = (nEntryEnd <= nEnd && nEnd <
                        ::std::numeric_limits<SCCOLROW>::max());

                bool bToggle = ( bShow != bAllHidden );
                if ( bToggle )
                {
                    pEntry->SetHidden( !bShow );
                    SetVisibleBelow( aIter.LastLevel(), aIter.LastEntry(), bShow, bShow );
                    bModified = true;
                }
            }
        }
    }
    return bModified;
}

void ScOutlineArray::RemoveAll()
{
    for (size_t nLevel = 0; nLevel < nDepth; ++nLevel)
        aCollections[nLevel].clear();

    nDepth = 0;
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

sal_Bool ScOutlineTable::TestInsertCol( SCSIZE nSize )
{
    return aColOutline.TestInsertSpace( nSize, MAXCOL );
}

void ScOutlineTable::InsertCol( SCCOL nStartCol, SCSIZE nSize )
{
    aColOutline.InsertSpace( nStartCol, nSize );
}

sal_Bool ScOutlineTable::DeleteCol( SCCOL nStartCol, SCSIZE nSize )
{
    return aColOutline.DeleteSpace( nStartCol, nSize );
}

sal_Bool ScOutlineTable::TestInsertRow( SCSIZE nSize )
{
    return aRowOutline.TestInsertSpace( nSize, MAXROW );
}

void ScOutlineTable::InsertRow( SCROW nStartRow, SCSIZE nSize )
{
    aRowOutline.InsertSpace( nStartRow, nSize );
}

sal_Bool ScOutlineTable::DeleteRow( SCROW nStartRow, SCSIZE nSize )
{
    return aRowOutline.DeleteSpace( nStartRow, nSize );
}

//------------------------------------------------------------------------

ScSubOutlineIterator::ScSubOutlineIterator( ScOutlineArray* pOutlineArray ) :
        pArray( pOutlineArray ),
        nStart( 0 ),
        nEnd( SCCOLROW_MAX ),                           // alle durchgehen
        nSubLevel( 0 ),
        nSubEntry( 0 )
{
    nDepth = pArray->nDepth;
}

ScSubOutlineIterator::ScSubOutlineIterator(
    ScOutlineArray* pOutlineArray, size_t nLevel, size_t nEntry ) :
        pArray( pOutlineArray )
{
    const ScOutlineCollection& rColl = pArray->aCollections[nLevel];
    ScOutlineCollection::const_iterator it = rColl.begin();
    std::advance(it, nEntry);
    const ScOutlineEntry* pEntry = it->second;
    nStart = pEntry->GetStart();
    nEnd   = pEntry->GetEnd();
    nSubLevel = nLevel + 1;
    nSubEntry = 0;
    nDepth = pArray->nDepth;
}

ScOutlineEntry* ScSubOutlineIterator::GetNext()
{
    ScOutlineEntry* pEntry = NULL;
    bool bFound = false;
    do
    {
        if (nSubLevel >= nDepth)
            return NULL;

        ScOutlineCollection& rColl = pArray->aCollections[nSubLevel];
        if (nSubEntry < rColl.size())
        {
            ScOutlineCollection::iterator it = rColl.begin();
            std::advance(it, nSubEntry);
            pEntry = it->second;

            if (pEntry->GetStart() >= nStart && pEntry->GetEnd() <= nEnd)
                bFound = true;

            ++nSubEntry;
        }
        else
        {
            // Go to the next sub-level.
            nSubEntry = 0;
            ++nSubLevel;
        }
    }
    while (!bFound);
    return pEntry;                  // nSubLevel gueltig, wenn pEntry != 0
}

size_t ScSubOutlineIterator::LastLevel() const
{
    return nSubLevel;
}

size_t ScSubOutlineIterator::LastEntry() const
{
    if (nSubEntry == 0)
    {
        OSL_FAIL("ScSubOutlineIterator::LastEntry before GetNext");
        return 0;
    }
    return nSubEntry-1;
}

void ScSubOutlineIterator::DeleteLast()
{
    if (nSubLevel >= nDepth)
    {
        OSL_FAIL("ScSubOutlineIterator::DeleteLast after End");
        return;
    }
    if (nSubEntry == 0)
    {
        OSL_FAIL("ScSubOutlineIterator::DeleteLast before GetNext");
        return;
    }

    --nSubEntry;
    ScOutlineCollection& rColl = pArray->aCollections[nSubLevel];
    OSL_ASSERT(nSubEntry < rColl.size());
    ScOutlineCollection::iterator it = rColl.begin();
    std::advance(it, nSubEntry);
    rColl.erase(it);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
