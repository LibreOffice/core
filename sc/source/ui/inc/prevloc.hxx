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

#ifndef SC_PREVLOC_HXX
#define SC_PREVLOC_HXX

#include <boost/ptr_container/ptr_list.hpp>

#include <sal/types.h>
#include <vcl/mapmod.hxx>

#include "address.hxx"

#define SC_PREVIEW_MAXRANGES    4
#define SC_PREVIEW_RANGE_EDGE   0
#define SC_PREVIEW_RANGE_REPCOL 1
#define SC_PREVIEW_RANGE_REPROW 2
#define SC_PREVIEW_RANGE_TAB    3

class OutputDevice;
class String;
class Point;
class Rectangle;
class ScAddress;
class ScRange;
class ScDocument;
class ScPreviewLocationEntry;

struct ScPreviewColRowInfo
{
    BOOL    bIsHeader;
    SCCOLROW    nDocIndex;
    long    nPixelStart;
    long    nPixelEnd;

    void Set( BOOL bHeader, SCCOLROW nIndex, long nStart, long nEnd )
    {
        bIsHeader   = bHeader;
        nDocIndex   = nIndex;
        nPixelStart = nStart;
        nPixelEnd   = nEnd;
    }
};

class ScPreviewTableInfo
{
    SCTAB                   nTab;
    SCCOL                   nCols;
    SCROW                   nRows;
    ScPreviewColRowInfo*    pColInfo;
    ScPreviewColRowInfo*    pRowInfo;

public:
            ScPreviewTableInfo();
            ~ScPreviewTableInfo();

    SCTAB                       GetTab() const      { return nTab; }
    SCCOL                       GetCols() const     { return nCols; }
    SCROW                       GetRows() const     { return nRows; }
    const ScPreviewColRowInfo*  GetColInfo() const  { return pColInfo; }
    const ScPreviewColRowInfo*  GetRowInfo() const  { return pRowInfo; }

    void    SetTab( SCTAB nNewTab );
    void    SetColInfo( SCCOL nCount, ScPreviewColRowInfo* pNewInfo );
    void    SetRowInfo( SCROW nCount, ScPreviewColRowInfo* pNewInfo );
    void    LimitToArea( const Rectangle& rPixelArea );
};


class ScPreviewLocationData
{
    OutputDevice* pWindow;
    ScDocument* pDoc;
    MapMode     aCellMapMode;
    MapMode     aDrawMapMode[SC_PREVIEW_MAXRANGES];
    Rectangle   aDrawRectangle[SC_PREVIEW_MAXRANGES];
        sal_uInt8       aDrawRangeId[SC_PREVIEW_MAXRANGES];
    USHORT      nDrawRanges;
    SCTAB       nPrintTab;
    boost::ptr_list<ScPreviewLocationEntry> aEntries;

    Rectangle   GetOffsetPixel( const ScAddress& rCellPos, const ScRange& rRange ) const;

public:
            ScPreviewLocationData( ScDocument* pDocument, OutputDevice* pWin );
            ~ScPreviewLocationData();

    void    SetCellMapMode( const MapMode& rMapMode );
    void    SetPrintTab( SCTAB nNew );
    void    Clear();
    void    AddCellRange( const Rectangle& rRect, const ScRange& rRange, BOOL bRepCol, BOOL bRepRow,
                            const MapMode& rDrawMap );
    void    AddColHeaders( const Rectangle& rRect, SCCOL nStartCol, SCCOL nEndCol, BOOL bRepCol );
    void    AddRowHeaders( const Rectangle& rRect, SCROW nStartRow, SCROW nEndRow, BOOL bRepRow );
    void    AddHeaderFooter( const Rectangle& rRect, BOOL bHeader, BOOL bLeft );
    void    AddNoteMark( const Rectangle& rRect, const ScAddress& rPos );
    void    AddNoteText( const Rectangle& rRect, const ScAddress& rPos );

    SCTAB   GetPrintTab() const     { return nPrintTab; }

    //  Get info on visible columns/rows in the visible area
    void    GetTableInfo( const Rectangle& rVisiblePixel, ScPreviewTableInfo& rInfo ) const;

    USHORT  GetDrawRanges() const   { return nDrawRanges; }
    void    GetDrawRange( USHORT nPos, Rectangle& rPixelRect, MapMode& rMapMode, sal_uInt8& rRangeId ) const;

    BOOL    GetHeaderPosition( Rectangle& rHeaderRect ) const;
    BOOL    GetFooterPosition( Rectangle& rFooterRect ) const;
    BOOL    IsHeaderLeft() const;
    BOOL    IsFooterLeft() const;

    long    GetNoteCountInRange( const Rectangle& rVisiblePixel, BOOL bNoteMarks ) const;
    BOOL    GetNoteInRange( const Rectangle& rVisiblePixel, long nIndex, BOOL bNoteMarks,
                            ScAddress& rCellPos, Rectangle& rNoteRect ) const;
    Rectangle GetNoteInRangeOutputRect(const Rectangle& rVisiblePixel, BOOL bNoteMarks,
                            const ScAddress& aCellPos) const;

    //  Check if any cells (including column/row headers) are in the visible area
    BOOL    HasCellsInRange( const Rectangle& rVisiblePixel ) const;

    BOOL    GetCellPosition( const ScAddress& rCellPos, Rectangle& rCellRect ) const;

    // returns the rectangle where the EditEngine draws the text of a Header Cell
    // if bColHeader is true it returns the rectangle of the header of the column in rCellPos
    // otherwise of the header of the row in rCellPos
    Rectangle GetHeaderCellOutputRect(const Rectangle& rVisRect, const ScAddress& rCellPos, sal_Bool bColHeader) const;
    Rectangle GetCellOutputRect(const ScAddress& rCellPos) const;

    // Query the range and rectangle of the main (non-repeat) cell range.
    // Returns FALSE if not contained.
    BOOL    GetMainCellRange( ScRange& rRange, Rectangle& rPixRect ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
