/*************************************************************************
 *
 *  $RCSfile: prevloc.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 11:38:35 $
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

#ifndef SC_PREVLOC_HXX
#define SC_PREVLOC_HXX

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif

#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif

#ifndef _SV_MAPMOD_HXX
#include <vcl/mapmod.hxx>
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif


#define SC_PREVIEW_MAXRANGES    4
#define SC_PREVIEW_RANGE_EDGE   0
#define SC_PREVIEW_RANGE_REPCOL 1
#define SC_PREVIEW_RANGE_REPROW 2
#define SC_PREVIEW_RANGE_TAB    3

class Window;
class String;
class Point;
class Rectangle;
class ScAddress;
class ScRange;
class ScDocument;

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
    Window*     pWindow;
    ScDocument* pDoc;
    MapMode     aCellMapMode;
    MapMode     aDrawMapMode[SC_PREVIEW_MAXRANGES];
    Rectangle   aDrawRectangle[SC_PREVIEW_MAXRANGES];
        sal_uInt8       aDrawRangeId[SC_PREVIEW_MAXRANGES];
    USHORT      nDrawRanges;
    SCTAB       nPrintTab;
    List        aEntries;

    ScAddress   GetCellFromRange( const Size& rOffsetPixel, const ScRange& rRange ) const;
    Rectangle   GetOffsetPixel( const ScAddress& rCellPos, const ScRange& rRange ) const;

public:
            ScPreviewLocationData( ScDocument* pDocument, Window* pWin );
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

    BOOL    GetCell( const Point& rPos, ScAddress& rCellPos, Rectangle& rCellRect ) const;
    BOOL    GetCellPosition( const ScAddress& rCellPos, Rectangle& rCellRect ) const;

    // returns the rectangle where the EditEngine draws the text of a Header Cell
    // if bColHeader is true it returns the rectangle of the header of the column in rCellPos
    // otherwise of the header of the row in rCellPos
    Rectangle GetHeaderCellOutputRect(const Rectangle& rVisRect, const ScAddress& rCellPos, sal_Bool bColHeader) const;
    Rectangle GetCellOutputRect(const ScAddress& rCellPos) const;
};

#endif
