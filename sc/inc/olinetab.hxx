/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
                                                bool bNewHidden = sal_False );
                            ScOutlineEntry( const ScOutlineEntry& rEntry );

    virtual ScDataObject*       Clone() const;

    SCCOLROW                GetStart() const    { return nStart; }
    SCSIZE                  GetSize() const     { return nSize; }
    SCCOLROW                GetEnd() const      { return nStart+nSize-1; }
    bool                    IsHidden() const    { return bHidden; }             // Gruppe versteckt
    bool                    IsVisible() const   { return bVisible; }            // Control sichtbar?

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

    sal_uInt16                  FindStart( SCCOLROW nMinStart );
};


class SC_DLLPUBLIC ScOutlineArray
{
friend class ScSubOutlineIterator;

private:
    sal_uInt16                  nDepth;
    ScOutlineCollection     aCollections[SC_OL_MAXDEPTH];

    sal_Bool                    DecDepth();
    void                    FindEntry( SCCOLROW nSearchPos, sal_uInt16& rFindLevel, sal_uInt16& rFindIndex,
                                        sal_uInt16 nMaxLevel = SC_OL_MAXDEPTH );
    void                    RemoveSub( SCCOLROW nStartPos, SCCOLROW nEndPos, sal_uInt16 nLevel );
    void                    PromoteSub( SCCOLROW nStartPos, SCCOLROW nEndPos, sal_uInt16 nStartLevel );

public:
                            ScOutlineArray();
                            ScOutlineArray( const ScOutlineArray& rArray );

    sal_uInt16                  GetDepth() const         { return nDepth; }

    sal_Bool                    FindTouchedLevel( SCCOLROW nBlockStart, SCCOLROW nBlockEnd,
                                                sal_uInt16& rFindLevel ) const;

    sal_Bool                    Insert( SCCOLROW nStartPos, SCCOLROW nEndPos, sal_Bool& rSizeChanged,
                                    sal_Bool bHidden = sal_False, sal_Bool bVisible = sal_True );
    sal_Bool                    Remove( SCCOLROW nBlockStart, SCCOLROW nBlockEnd, sal_Bool& rSizeChanged );

    ScOutlineEntry*         GetEntry( sal_uInt16 nLevel, sal_uInt16 nIndex ) const;
    sal_uInt16                  GetCount( sal_uInt16 nLevel ) const;
    ScOutlineEntry*         GetEntryByPos( sal_uInt16 nLevel, SCCOLROW nPos ) const;

    sal_Bool                    GetEntryIndex( sal_uInt16 nLevel, SCCOLROW nPos, sal_uInt16& rnIndex ) const;
    sal_Bool                    GetEntryIndexInRange(
                                sal_uInt16 nLevel, SCCOLROW nBlockStart, SCCOLROW nBlockEnd,
                                sal_uInt16& rnIndex ) const;

    void                    SetVisibleBelow( sal_uInt16 nLevel, sal_uInt16 nEntry, sal_Bool bValue,
                                                sal_Bool bSkipHidden = sal_False );

    void                    GetRange( SCCOLROW& rStart, SCCOLROW& rEnd ) const;
    void                    ExtendBlock( sal_uInt16 nLevel, SCCOLROW& rBlkStart, SCCOLROW& rBlkEnd );

    sal_Bool                    TestInsertSpace( SCSIZE nSize, SCCOLROW nMaxVal ) const;
    void                    InsertSpace( SCCOLROW nStartPos, SCSIZE nSize );
    sal_Bool                    DeleteSpace( SCCOLROW nStartPos, SCSIZE nSize );

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

    sal_Bool                    TestInsertCol( SCSIZE nSize );
    void                    InsertCol( SCCOL nStartCol, SCSIZE nSize );
    sal_Bool                    DeleteCol( SCCOL nStartCol, SCSIZE nSize ); // TRUE: Undo nur ueber Original
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
    sal_uInt16                  nSubLevel;
    sal_uInt16                  nSubEntry;
    sal_uInt16                  nCount;
    sal_uInt16                  nDepth;

public:
                            ScSubOutlineIterator( ScOutlineArray* pOutlineArray );
                            ScSubOutlineIterator( ScOutlineArray* pOutlineArray,
                                                    sal_uInt16 nLevel, sal_uInt16 nEntry );
    ScOutlineEntry*         GetNext();
    sal_uInt16                  LastLevel() const;
    sal_uInt16                  LastEntry() const;
    void                    DeleteLast();
};

#endif


