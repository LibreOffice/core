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

#ifndef SC_OUTLINETAB_HXX
#define SC_OUTLINETAB_HXX

#include "scdllapi.h"
#include "address.hxx"

#include <boost/ptr_container/ptr_map.hpp>

#define SC_OL_MAXDEPTH      7

class ScTable;


class ScOutlineEntry
{
    SCCOLROW    nStart;
    SCSIZE      nSize;
    bool        bHidden;
    bool        bVisible;

public:
    ScOutlineEntry( SCCOLROW nNewStart, SCCOLROW nNewSize, bool bNewHidden = false );
    ScOutlineEntry( const ScOutlineEntry& rEntry );

    SCCOLROW GetStart() const;
    SCSIZE GetSize() const;
    SC_DLLPUBLIC SCCOLROW GetEnd() const;

    /**
     * @return true is the group is hidden, false otherwise.
     */
    SC_DLLPUBLIC bool IsHidden() const;

    /**
     * @return true if the control is visible, false otherwise.
     */
    bool IsVisible() const;

    void                    Move( SCsCOLROW nDelta );
    void                    SetSize( SCSIZE nNewSize );
    void                    SetPosSize( SCCOLROW nNewPos, SCSIZE nNewSize );
    void                    SetHidden( bool bNewHidden );
    void                    SetVisible( bool bNewVisible );
};

class ScOutlineCollection
{
    typedef boost::ptr_map<SCCOLROW, ScOutlineEntry> MapType;
    MapType maEntries;

public:
    typedef MapType::iterator iterator;
    typedef MapType::const_iterator const_iterator;

    ScOutlineCollection();

    size_t size() const;
    void clear();
    void insert(ScOutlineEntry* pEntry);
    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
    void erase(iterator pos);
    bool empty() const;

    iterator FindStart(SCCOLROW nMinStart);
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

    void RemoveSub(SCCOLROW nStartPos, SCCOLROW nEndPos, size_t nLevel);
    void PromoteSub(SCCOLROW nStartPos, SCCOLROW nEndPos, size_t nStartLevel);

public:
    ScOutlineArray();
    ScOutlineArray( const ScOutlineArray& rArray );

    size_t GetDepth() const;

    bool FindTouchedLevel(
        SCCOLROW nBlockStart, SCCOLROW nBlockEnd, size_t& rFindLevel) const;

    bool Insert( SCCOLROW nStartPos, SCCOLROW nEndPos, bool& rSizeChanged,
                 bool bHidden = false, bool bVisible = true );
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

    void RemoveAll();
};


class ScOutlineTable
{
private:
    ScOutlineArray          aColOutline;
    ScOutlineArray          aRowOutline;

public:
                            ScOutlineTable();
                            ScOutlineTable( const ScOutlineTable& rOutline );

    const ScOutlineArray*   GetColArray() const     { return &aColOutline; }
    ScOutlineArray*         GetColArray()           { return &aColOutline; }
    const ScOutlineArray*   GetRowArray() const     { return &aRowOutline; }
    ScOutlineArray*         GetRowArray()           { return &aRowOutline; }

    sal_Bool                    TestInsertCol( SCSIZE nSize );
    void                    InsertCol( SCCOL nStartCol, SCSIZE nSize );
    sal_Bool                    DeleteCol( SCCOL nStartCol, SCSIZE nSize ); // TRUE: Undo only using original
    sal_Bool                    TestInsertRow( SCSIZE nSize );
    void                    InsertRow( SCROW nStartRow, SCSIZE nSize );
    sal_Bool                    DeleteRow( SCROW nStartRow, SCSIZE nSize );
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
    size_t LastLevel() const;
    size_t LastEntry() const;
    void DeleteLast();
};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
