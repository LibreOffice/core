/*************************************************************************
 *
 *  $RCSfile: olinetab.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 10:11:45 $
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

#ifndef SC_OUTLINETAB_HXX
#define SC_OUTLINETAB_HXX


#ifndef SC_COLLECT_HXX
#include "collect.hxx"
#endif


#define SC_OL_MAXDEPTH      7

class SvStream;
class ScMultipleReadHeader;
class ScMultipleWriteHeader;


class ScOutlineEntry : public DataObject
{
    SCCOLROW    nStart;
    SCSIZE      nSize;
    BOOL        bHidden;
    BOOL        bVisible;

public:
                            ScOutlineEntry( SCCOLROW nNewStart, SCCOLROW nNewSize,
                                                BOOL bNewHidden = FALSE );
                            ScOutlineEntry( const ScOutlineEntry& rEntry );
                            ScOutlineEntry( SvStream& rStream, ScMultipleReadHeader& rHdr );

    void                    Store( SvStream& rStream, ScMultipleWriteHeader& rHdr );

    virtual DataObject*     Clone() const;

    SCCOLROW                GetStart() const    { return nStart; }
    SCSIZE                  GetSize() const     { return nSize; }
    SCCOLROW                GetEnd() const      { return nStart+nSize-1; }
    BOOL                    IsHidden() const    { return bHidden; }             // Gruppe versteckt
    BOOL                    IsVisible() const   { return bVisible; }            // Control sichtbar?

    void                    Move( SCsCOLROW nDelta );
    void                    SetSize( SCSIZE nNewSize );
    void                    SetPosSize( SCCOLROW nNewPos, SCSIZE nNewSize );
    void                    SetHidden( BOOL bNewHidden );
    void                    SetVisible( BOOL bNewVisible );
};


class ScOutlineCollection : public SortedCollection
{
public:
                            ScOutlineCollection();

    virtual short           Compare(DataObject* pKey1, DataObject* pKey2) const;

    USHORT                  FindStart( SCCOLROW nMinStart );
};


class ScOutlineArray
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

    BOOL                    ManualAction( SCCOLROW nStartPos, SCCOLROW nEndPos,
                                            BOOL bShow, BYTE* pHiddenFlags );

    void                    Load( SvStream& rStream );
    void                    Store( SvStream& rStream );

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
    BOOL                    DeleteCol( SCCOL nStartCol, SCSIZE nSize ); // TRUE: Undo nur ueber Original
    BOOL                    TestInsertRow( SCSIZE nSize );
    void                    InsertRow( SCROW nStartRow, SCSIZE nSize );
    BOOL                    DeleteRow( SCROW nStartRow, SCSIZE nSize );

    void                    Load( SvStream& rStream );
    void                    Store( SvStream& rStream );
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


