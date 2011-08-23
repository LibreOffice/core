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
class SvStream;
namespace binfilter {


#define SC_OL_MAXDEPTH		7

class ScMultipleReadHeader;
class ScMultipleWriteHeader;


class ScOutlineEntry : public DataObject
{
    USHORT		nStart;
    USHORT		nSize;
    BOOL		bHidden;
    BOOL		bVisible;

public:
                            ScOutlineEntry( USHORT nNewStart, USHORT nNewSize,
                                                 BOOL bNewHidden = FALSE );
                            ScOutlineEntry( const ScOutlineEntry& rEntry );
                            ScOutlineEntry( SvStream& rStream, ScMultipleReadHeader& rHdr );

    void					Store( SvStream& rStream, ScMultipleWriteHeader& rHdr );
    virtual	DataObject*		Clone() const;
    USHORT					GetStart() const	{ return nStart; }
/*N*/ 	USHORT					GetSize() const		{ return nSize; }
    USHORT					GetEnd() const		{ return nStart+nSize-1; }
    BOOL					IsHidden() const	{ return bHidden; }				// Gruppe versteckt
/*N*/ 	void					Move( short nDelta );
/*N*/ 	void					SetSize( USHORT nNewSize );
/*N*/ 	void					SetPosSize( USHORT nNewPos, USHORT nNewSize );
    void					SetVisible( BOOL bNewVisible );
};


class ScOutlineCollection : public SortedCollection
{
public:
                            ScOutlineCollection();
 
    virtual	short			Compare(DataObject* pKey1, DataObject* pKey2) const;

};


class ScOutlineArray
{
friend class ScSubOutlineIterator;
 
private:
    USHORT					nDepth;
    ScOutlineCollection		aCollections[SC_OL_MAXDEPTH];
 
/*N*/ 	BOOL					DecDepth();
    void					FindEntry( USHORT nSearchPos, USHORT& rFindLevel, USHORT& rFindIndex,
                                        USHORT nMaxLevel = SC_OL_MAXDEPTH );
public:
                            ScOutlineArray();
                             ScOutlineArray( const ScOutlineArray& rArray );

    USHORT					GetDepth() const		 { return nDepth; }


    BOOL					Insert( USHORT nStartCol, USHORT nEndCol, BOOL& rSizeChanged,
                                    BOOL bHidden = FALSE, BOOL bVisible = TRUE );

    ScOutlineEntry*         GetEntry( USHORT nLevel, USHORT nIndex ) const;
    USHORT                  GetCount( USHORT nLevel ) const;

    void					SetVisibleBelow( USHORT nLevel, USHORT nEntry, BOOL bValue,
                                                BOOL bSkipHidden = FALSE );


/*N*/    BOOL                    TestInsertSpace( USHORT nSize, USHORT nMaxVal ) const;
/*N*/ 	void					InsertSpace( USHORT nStartPos, USHORT nSize );
/*N*/ 	BOOL					DeleteSpace( USHORT nStartPos, USHORT nSize );

    BOOL					ManualAction( USHORT nStartPos, USHORT nEndPos,
                                            BOOL bShow, BYTE* pHiddenFlags );
     void					Load( SvStream& rStream );
    void					Store( SvStream& rStream );
};


class ScOutlineTable
{
private:
    ScOutlineArray			aColOutline;
    ScOutlineArray			aRowOutline;
 
public:
                            ScOutlineTable();
                            ScOutlineTable( const ScOutlineTable& rOutline );
  
    const ScOutlineArray*	GetColArray() const		{ return &aColOutline; }
    ScOutlineArray*			GetColArray()			{ return &aColOutline; }
    const ScOutlineArray*	GetRowArray() const		{ return &aRowOutline; }
     ScOutlineArray*			GetRowArray()			{ return &aRowOutline; }

/*N*/ 	BOOL					TestInsertCol( USHORT nSize );
/*N*/ 	void					InsertCol( USHORT nStartCol, USHORT nSize );
/*N*/ 	BOOL					DeleteCol( USHORT nStartCol, USHORT nSize );	// TRUE: Undo nur ueber Original
/*N*/ 	BOOL					TestInsertRow( USHORT nSize );
/*N*/ 	void					InsertRow( USHORT nStartRow, USHORT nSize );
/*N*/ 	BOOL					DeleteRow( USHORT nStartRow, USHORT nSize );
    void					Load( SvStream& rStream );
     void					Store( SvStream& rStream );
};


class ScSubOutlineIterator
{
private:
    ScOutlineArray*			pArray;
    USHORT					nStart;
    USHORT					nEnd;
    USHORT					nSubLevel;
    USHORT					nSubEntry;
    USHORT					nCount;
    USHORT					nDepth;

public:
                            ScSubOutlineIterator( ScOutlineArray* pOutlineArray );
    ScOutlineEntry*			GetNext();
/*N*/ 	void					DeleteLast();
};

} //namespace binfilter
#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
