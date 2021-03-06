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

#pragma once

#include "scdllapi.h"
#include "address.hxx"

#include <map>

#define SC_OL_MAXDEPTH      7

class ScTable;

class ScOutlineEntry
{
    SCCOLROW    nStart;
    SCSIZE      nSize;
    bool        bHidden;
    bool        bVisible;

public:
    ScOutlineEntry( SCCOLROW nNewStart, SCCOLROW nNewSize, bool bNewHidden );
    ScOutlineEntry( const ScOutlineEntry& rEntry );

    SC_DLLPUBLIC SCCOLROW GetStart() const { return nStart;}
    SCSIZE GetSize() const { return nSize;}
    SC_DLLPUBLIC SCCOLROW GetEnd() const;

    /**
     * @return true is the group is hidden, false otherwise.
     */
    SC_DLLPUBLIC bool IsHidden() const { return bHidden;}

    /**
     * @return true if the control is visible, false otherwise.
     */
    SC_DLLPUBLIC bool IsVisible() const { return bVisible;}

    void                    Move( SCCOLROW nDelta );
    void                    SetSize( SCSIZE nNewSize );
    void                    SetPosSize( SCCOLROW nNewPos, SCSIZE nNewSize );
    void                    SetHidden( bool bNewHidden );
    void                    SetVisible( bool bNewVisible );

    OString                 dumpAsString() const;
};

class ScOutlineCollection
{
    typedef std::map<SCCOLROW, ScOutlineEntry> MapType;
    MapType m_Entries;

public:
    typedef MapType::iterator iterator;
    typedef MapType::const_iterator const_iterator;

    ScOutlineCollection();

    size_t size() const;
    void clear();
    void insert(ScOutlineEntry const& rEntry);
    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
    void erase(const iterator& pos);
    bool empty() const;

    iterator FindStart(SCCOLROW nMinStart);

    OString dumpAsString() const;
};

class SC_DLLPUBLIC ScOutlineArray
{
friend class ScSubOutlineIterator;

private:
    size_t nDepth;
    ScOutlineCollection     aCollections[SC_OL_MAXDEPTH];

    bool DecDepth();
    void FindEntry(
        SCCOLROW nSearchPos, size_t& rFindLevel, size_t& rFindIndex,
        size_t nMaxLevel = SC_OL_MAXDEPTH);

    void PromoteSub(SCCOLROW nStartPos, SCCOLROW nEndPos, size_t nStartLevel);

public:
    ScOutlineArray();
    ScOutlineArray( const ScOutlineArray& rArray );

    size_t GetDepth() const { return nDepth;}

    bool FindTouchedLevel(
        SCCOLROW nBlockStart, SCCOLROW nBlockEnd, size_t& rFindLevel) const;

    bool Insert( SCCOLROW nStartPos, SCCOLROW nEndPos, bool& rSizeChanged,
                 bool bHidden = false );
    bool Remove( SCCOLROW nBlockStart, SCCOLROW nBlockEnd, bool& rSizeChanged );

    ScOutlineEntry* GetEntry(size_t nLevel, size_t nIndex);
    const ScOutlineEntry* GetEntry(size_t nLevel, size_t nIndex) const;
    size_t GetCount(size_t nLevel) const;
    const ScOutlineEntry* GetEntryByPos(size_t nLevel, SCCOLROW nPos) const;

    bool GetEntryIndex(size_t nLevel, SCCOLROW nPos, size_t& rnIndex) const;
    bool GetEntryIndexInRange(
        size_t nLevel, SCCOLROW nBlockStart, SCCOLROW nBlockEnd, size_t& rnIndex) const;

    void SetVisibleBelow(
        size_t nLevel, size_t nEntry, bool bValue, bool bSkipHidden = false);

    void GetRange(SCCOLROW& rStart, SCCOLROW& rEnd) const;
    void ExtendBlock(size_t nLevel, SCCOLROW& rBlkStart, SCCOLROW& rBlkEnd);

    bool TestInsertSpace(SCSIZE nSize, SCCOLROW nMaxVal) const;
    void InsertSpace(SCCOLROW nStartPos, SCSIZE nSize);
    bool DeleteSpace(SCCOLROW nStartPos, SCSIZE nSize);

    bool ManualAction(
        SCCOLROW nStartPos, SCCOLROW nEndPos, bool bShow, const ScTable& rTable, bool bCol);

    void finalizeImport(const ScTable& rTable);

    void RemoveAll();

    OString dumpAsString() const;
};

class ScOutlineTable
{
private:
    ScOutlineArray          aColOutline;
    ScOutlineArray          aRowOutline;

public:
                            ScOutlineTable();
                            ScOutlineTable( const ScOutlineTable& rOutline );

    const ScOutlineArray&   GetColArray() const     { return aColOutline; }
    ScOutlineArray&         GetColArray()           { return aColOutline; }
    const ScOutlineArray&   GetRowArray() const     { return aRowOutline; }
    ScOutlineArray&         GetRowArray()           { return aRowOutline; }

    bool                    TestInsertCol( SCSIZE nSize );
    void                    InsertCol( SCCOL nStartCol, SCSIZE nSize );
    bool                    DeleteCol( SCCOL nStartCol, SCSIZE nSize ); // TRUE: Undo only using original
    bool                    TestInsertRow( SCSIZE nSize );
    void                    InsertRow( SCROW nStartRow, SCSIZE nSize );
    bool                    DeleteRow( SCROW nStartRow, SCSIZE nSize );
};

class ScSubOutlineIterator
{
private:
    ScOutlineArray*         pArray;
    SCCOLROW                nStart;
    SCCOLROW                nEnd;
    size_t                  nSubLevel;
    size_t                  nSubEntry;
    size_t                  nDepth;

public:
    ScSubOutlineIterator( ScOutlineArray* pOutlineArray );
    ScSubOutlineIterator( ScOutlineArray* pOutlineArray, size_t nLevel, size_t nEntry );

    ScOutlineEntry* GetNext();
    size_t LastLevel() const { return nSubLevel;}
    size_t LastEntry() const;
    void DeleteLast();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
