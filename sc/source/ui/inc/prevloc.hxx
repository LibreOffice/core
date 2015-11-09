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

#ifndef INCLUDED_SC_SOURCE_UI_INC_PREVLOC_HXX
#define INCLUDED_SC_SOURCE_UI_INC_PREVLOC_HXX

#include <sal/types.h>
#include <vcl/mapmod.hxx>

#include "address.hxx"

#include <memory>
#include <list>

#define SC_PREVIEW_MAXRANGES    4
#define SC_PREVIEW_RANGE_EDGE   0
#define SC_PREVIEW_RANGE_REPCOL 1
#define SC_PREVIEW_RANGE_REPROW 2
#define SC_PREVIEW_RANGE_TAB    3

class OutputDevice;
class Rectangle;
class ScAddress;
class ScRange;
class ScDocument;
struct ScPreviewLocationEntry;

struct ScPreviewColRowInfo
{
    bool    bIsHeader;
    SCCOLROW    nDocIndex;
    long    nPixelStart;
    long    nPixelEnd;

    void Set( bool bHeader, SCCOLROW nIndex, long nStart, long nEnd )
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
public:
    typedef std::list<std::unique_ptr<ScPreviewLocationEntry>> Entries_t;
private:
    VclPtr<OutputDevice> pWindow;
    ScDocument* pDoc;
    MapMode     aCellMapMode;
    MapMode     aDrawMapMode[SC_PREVIEW_MAXRANGES];
    Rectangle   aDrawRectangle[SC_PREVIEW_MAXRANGES];
        sal_uInt8       aDrawRangeId[SC_PREVIEW_MAXRANGES];
    sal_uInt16      nDrawRanges;
    SCTAB       nPrintTab;
    Entries_t m_Entries;

    Rectangle   GetOffsetPixel( const ScAddress& rCellPos, const ScRange& rRange ) const;

public:
            ScPreviewLocationData( ScDocument* pDocument, OutputDevice* pWin );
            ~ScPreviewLocationData();

    void    SetCellMapMode( const MapMode& rMapMode );
    void    SetPrintTab( SCTAB nNew );
    void    Clear();
    void    AddCellRange( const Rectangle& rRect, const ScRange& rRange, bool bRepCol, bool bRepRow,
                            const MapMode& rDrawMap );
    void    AddColHeaders( const Rectangle& rRect, SCCOL nStartCol, SCCOL nEndCol, bool bRepCol );
    void    AddRowHeaders( const Rectangle& rRect, SCROW nStartRow, SCROW nEndRow, bool bRepRow );
    void    AddHeaderFooter( const Rectangle& rRect, bool bHeader, bool bLeft );
    void    AddNoteMark( const Rectangle& rRect, const ScAddress& rPos );
    void    AddNoteText( const Rectangle& rRect, const ScAddress& rPos );

    SCTAB   GetPrintTab() const     { return nPrintTab; }

    //  Get info on visible columns/rows in the visible area
    void    GetTableInfo( const Rectangle& rVisiblePixel, ScPreviewTableInfo& rInfo ) const;

    sal_uInt16  GetDrawRanges() const   { return nDrawRanges; }
    void    GetDrawRange( sal_uInt16 nPos, Rectangle& rPixelRect, MapMode& rMapMode, sal_uInt8& rRangeId ) const;

    bool    GetHeaderPosition( Rectangle& rHeaderRect ) const;
    bool    GetFooterPosition( Rectangle& rFooterRect ) const;
    bool    IsHeaderLeft() const;
    bool    IsFooterLeft() const;

    long    GetNoteCountInRange( const Rectangle& rVisiblePixel, bool bNoteMarks ) const;
    bool    GetNoteInRange( const Rectangle& rVisiblePixel, long nIndex, bool bNoteMarks,
                            ScAddress& rCellPos, Rectangle& rNoteRect ) const;
    Rectangle GetNoteInRangeOutputRect(const Rectangle& rVisiblePixel, bool bNoteMarks,
                            const ScAddress& aCellPos) const;

    //  Check if any cells (including column/row headers) are in the visible area
    bool    HasCellsInRange( const Rectangle& rVisiblePixel ) const;

    bool    GetCellPosition( const ScAddress& rCellPos, Rectangle& rCellRect ) const;

    // returns the rectangle where the EditEngine draws the text of a Header Cell
    // if bColHeader is true it returns the rectangle of the header of the column in rCellPos
    // otherwise of the header of the row in rCellPos
    Rectangle GetHeaderCellOutputRect(const Rectangle& rVisRect, const ScAddress& rCellPos, bool bColHeader) const;
    Rectangle GetCellOutputRect(const ScAddress& rCellPos) const;

    // Query the range and rectangle of the main (non-repeat) cell range.
    // Returns sal_False if not contained.
    bool    GetMainCellRange( ScRange& rRange, Rectangle& rPixRect ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
