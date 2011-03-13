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


#include "collect.hxx"
#include "scdllapi.h"

#define SC_OL_MAXDEPTH      7

class SvStream;
class ScTable;


class ScOutlineEntry : public ScDataObject
{
    SCCOLROW    nStart;
    SCSIZE      nSize;
    bool        bHidden;
    bool        bVisible;

public:
                            ScOutlineEntry( SCCOLROW nNewStart, SCCOLROW nNewSize,
                                                bool bNewHidden = FALSE );
                            ScOutlineEntry( const ScOutlineEntry& rEntry );

    virtual ScDataObject*       Clone() const;

    SCCOLROW                GetStart() const    { return nStart; }
    SCSIZE                  GetSize() const     { return nSize; }
    SCCOLROW                GetEnd() const      { return nStart+nSize-1; }
    bool                    IsHidden() const    { return bHidden; }             // group hidden
    bool                    IsVisible() const   { return bVisible; }            // control visible?

    void                    Move( SCsCOLROW nDelta );
    void                    SetSize( SCSIZE nNewSize );
    void                    SetPosSize( SCCOLROW nNewPos, SCSIZE nNewSize );
    void                    SetHidden( bool bNewHidden );
    void                    SetVisible( bool bNewVisible );
};


class ScOutlineCollection : public ScSortedCollection
{
public:
                            ScOutlineCollection();

    virtual short           Compare(ScDataObject* pKey1, ScDataObject* pKey2) const;

    USHORT                  FindStart( SCCOLROW nMinStart );
};


class SC_DLLPUBLIC ScOutlineArray
{
friend class ScSubOutlineIterator;

private:
    USHORT                  nDepth;
    ScOutlineCollection     aCollections[SC_OL_MAXDEPTH];

    BOOL                    DecDepth();
    void                    FindEntry( SCCOLROW nSearchPos, USHORT& rFindLevel, USHORT& rFindIndex,
                                        USHORT nMaxLevel = SC_OL_MAXDEPTH );
    void                    RemoveSub( SCCOLROW nStartPos, SCCOLROW nEndPos, USHORT nLevel );
    void                    PromoteSub( SCCOLROW nStartPos, SCCOLROW nEndPos, USHORT nStartLevel );

public:
                            ScOutlineArray();
                            ScOutlineArray( const ScOutlineArray& rArray );

    USHORT                  GetDepth() const         { return nDepth; }

    BOOL                    FindTouchedLevel( SCCOLROW nBlockStart, SCCOLROW nBlockEnd,
                                                USHORT& rFindLevel ) const;

    BOOL                    Insert( SCCOLROW nStartPos, SCCOLROW nEndPos, BOOL& rSizeChanged,
                                    BOOL bHidden = FALSE, BOOL bVisible = TRUE );
    BOOL                    Remove( SCCOLROW nBlockStart, SCCOLROW nBlockEnd, BOOL& rSizeChanged );

    ScOutlineEntry*         GetEntry( USHORT nLevel, USHORT nIndex ) const;
    USHORT                  GetCount( USHORT nLevel ) const;
    ScOutlineEntry*         GetEntryByPos( USHORT nLevel, SCCOLROW nPos ) const;

    BOOL                    GetEntryIndex( USHORT nLevel, SCCOLROW nPos, USHORT& rnIndex ) const;
    BOOL                    GetEntryIndexInRange(
                                USHORT nLevel, SCCOLROW nBlockStart, SCCOLROW nBlockEnd,
                                USHORT& rnIndex ) const;

    void                    SetVisibleBelow( USHORT nLevel, USHORT nEntry, BOOL bValue,
                                                BOOL bSkipHidden = FALSE );

    void                    GetRange( SCCOLROW& rStart, SCCOLROW& rEnd ) const;
    void                    ExtendBlock( USHORT nLevel, SCCOLROW& rBlkStart, SCCOLROW& rBlkEnd );

    BOOL                    TestInsertSpace( SCSIZE nSize, SCCOLROW nMaxVal ) const;
    void                    InsertSpace( SCCOLROW nStartPos, SCSIZE nSize );
    BOOL                    DeleteSpace( SCCOLROW nStartPos, SCSIZE nSize );

    bool                    ManualAction( SCCOLROW nStartPos, SCCOLROW nEndPos, bool bShow, ScTable& rTable, bool bCol );

    void                    RemoveAll();
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

    BOOL                    TestInsertCol( SCSIZE nSize );
    void                    InsertCol( SCCOL nStartCol, SCSIZE nSize );
    BOOL                    DeleteCol( SCCOL nStartCol, SCSIZE nSize ); // TRUE: Undo only using original
    BOOL                    TestInsertRow( SCSIZE nSize );
    void                    InsertRow( SCROW nStartRow, SCSIZE nSize );
    BOOL                    DeleteRow( SCROW nStartRow, SCSIZE nSize );
};


class ScSubOutlineIterator
{
private:
    ScOutlineArray*         pArray;
    SCCOLROW                nStart;
    SCCOLROW                nEnd;
    USHORT                  nSubLevel;
    USHORT                  nSubEntry;
    USHORT                  nCount;
    USHORT                  nDepth;

public:
                            ScSubOutlineIterator( ScOutlineArray* pOutlineArray );
                            ScSubOutlineIterator( ScOutlineArray* pOutlineArray,
                                                    USHORT nLevel, USHORT nEntry );
    ScOutlineEntry*         GetNext();
    USHORT                  LastLevel() const;
    USHORT                  LastEntry() const;
    void                    DeleteLast();
};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
