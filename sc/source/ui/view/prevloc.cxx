/*************************************************************************
 *
 *  $RCSfile: prevloc.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: nn $ $Date: 2002-02-22 09:57:40 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include <vcl/window.hxx>

#include "prevloc.hxx"
#include "document.hxx"

//==================================================================

enum ScPreviewLocationType
{
    SC_PLOC_CELLRANGE,
    SC_PLOC_COLHEADER,
    SC_PLOC_ROWHEADER,
    SC_PLOC_HEADERFOOTER,
    SC_PLOC_NOTEMARK,
    SC_PLOC_NOTETEXT
};

struct ScPreviewLocationEntry
{
    ScPreviewLocationType   eType;
    Rectangle               aPixelRect;
    ScRange                 aCellRange;

    ScPreviewLocationEntry( ScPreviewLocationType eNewType, const Rectangle& rPixel, const ScRange& rRange ) :
        eType( eNewType ), aPixelRect( rPixel ), aCellRange( rRange ) {}
};

//==================================================================

ScPreviewLocationData::ScPreviewLocationData( ScDocument* pDocument, Window* pWin ) :
    pDoc( pDocument ),
    pWindow( pWin )
{
}

ScPreviewLocationData::~ScPreviewLocationData()
{
    Clear();
}

void ScPreviewLocationData::SetCellMapMode( const MapMode& rMapMode )
{
    aCellMapMode = rMapMode;
}

void ScPreviewLocationData::Clear()
{
    void* pEntry = aEntries.First();
    while ( pEntry )
    {
        delete (ScPreviewLocationEntry*) pEntry;
        pEntry = aEntries.Next();
    }
    aEntries.Clear();
}

void ScPreviewLocationData::AddCellRange( const Rectangle& rRect, const ScRange& rRange )
{
    Rectangle aPixelRect( pWindow->LogicToPixel( rRect ) );
    aEntries.Insert( new ScPreviewLocationEntry( SC_PLOC_CELLRANGE, aPixelRect, rRange ) );
}

void ScPreviewLocationData::AddColHeaders( const Rectangle& rRect, USHORT nStartCol, USHORT nEndCol )
{
    USHORT nTab = 0;    //! ?
    ScRange aRange( nStartCol, 0, nTab, nEndCol, 0, nTab );
    Rectangle aPixelRect( pWindow->LogicToPixel( rRect ) );
    aEntries.Insert( new ScPreviewLocationEntry( SC_PLOC_COLHEADER, aPixelRect, aRange ) );
}

void ScPreviewLocationData::AddRowHeaders( const Rectangle& rRect, USHORT nStartRow, USHORT nEndRow )
{
    USHORT nTab = 0;    //! ?
    ScRange aRange( 0, nStartRow, nTab, 0, nEndRow, nTab );
    Rectangle aPixelRect( pWindow->LogicToPixel( rRect ) );
    aEntries.Insert( new ScPreviewLocationEntry( SC_PLOC_ROWHEADER, aPixelRect, aRange ) );
}

void ScPreviewLocationData::AddHeaderFooter( const Rectangle& rRect )
{
    ScRange aRange;     //! ?
    Rectangle aPixelRect( pWindow->LogicToPixel( rRect ) );
    aEntries.Insert( new ScPreviewLocationEntry( SC_PLOC_HEADERFOOTER, aPixelRect, aRange ) );
}

void ScPreviewLocationData::AddNoteMark( const Rectangle& rRect, const ScAddress& rPos )
{
    ScRange aRange( rPos );
    Rectangle aPixelRect( pWindow->LogicToPixel( rRect ) );
    aEntries.Insert( new ScPreviewLocationEntry( SC_PLOC_NOTEMARK, aPixelRect, aRange ) );
}

void ScPreviewLocationData::AddNoteText( const Rectangle& rRect, const ScAddress& rPos )
{
    ScRange aRange( rPos );
    Rectangle aPixelRect( pWindow->LogicToPixel( rRect ) );
    aEntries.Insert( new ScPreviewLocationEntry( SC_PLOC_NOTETEXT, aPixelRect, aRange ) );
}

//------------------------------------------------------------------

ScPreviewLocationEntry* lcl_GetEntryByPosition( const List& rEntries, const Point& rPos, ScPreviewLocationType eType )
{
    ULONG nCount = rEntries.Count();
    for (ULONG nListPos=0; nListPos<nCount; nListPos++)
    {
        ScPreviewLocationEntry* pEntry = (ScPreviewLocationEntry*)rEntries.GetObject(nListPos);
        if ( pEntry->eType == eType && pEntry->aPixelRect.IsInside( rPos ) )
            return pEntry;
    }
    return NULL;
}

ScPreviewLocationEntry* lcl_GetEntryByAddress( const List& rEntries, const ScAddress& rPos, ScPreviewLocationType eType )
{
    ULONG nCount = rEntries.Count();
    for (ULONG nListPos=0; nListPos<nCount; nListPos++)
    {
        ScPreviewLocationEntry* pEntry = (ScPreviewLocationEntry*)rEntries.GetObject(nListPos);
        if ( pEntry->eType == eType && pEntry->aCellRange.In( rPos ) )
            return pEntry;
    }
    return NULL;
}

ScAddress ScPreviewLocationData::GetCellFromRange( const Size& rOffsetPixel, const ScRange& rRange ) const
{
    const double nScaleX = HMM_PER_TWIPS;
    const double nScaleY = HMM_PER_TWIPS;

    Size aOffsetLogic = pWindow->PixelToLogic( rOffsetPixel, aCellMapMode );
    USHORT nTab = rRange.aStart.Tab();

    long nPosX = 0;
    USHORT nCol = rRange.aStart.Col();
    USHORT nEndCol = rRange.aEnd.Col();
    while ( nCol <= nEndCol && nPosX < aOffsetLogic.Width() )
    {
        USHORT nDocW = pDoc->GetColWidth( nCol, nTab );
        if (nDocW)
            nPosX += (long) (nDocW * nScaleX);
        ++nCol;
    }
    if ( nCol > rRange.aStart.Col() )
        --nCol;

    long nPosY = 0;
    USHORT nRow = rRange.aStart.Row();
    USHORT nEndRow = rRange.aEnd.Row();
    while ( nRow <= nEndRow && nPosY < aOffsetLogic.Height() )
    {
        USHORT nDocH = pDoc->FastGetRowHeight( nRow, nTab );
        if (nDocH)
            nPosY += (long) (nDocH * nScaleY);
        ++nRow;
    }
    if ( nRow > rRange.aStart.Row() )
        --nRow;

    return ScAddress( nCol, nRow, nTab );
}

Rectangle ScPreviewLocationData::GetOffsetPixel( const ScAddress& rCellPos, const ScRange& rRange ) const
{
    const double nScaleX = HMM_PER_TWIPS;
    const double nScaleY = HMM_PER_TWIPS;
    USHORT nTab = rRange.aStart.Tab();

    long nPosX = 0;
    USHORT nEndCol = rCellPos.Col();
    for (USHORT nCol = rRange.aStart.Col(); nCol < nEndCol; nCol++)
    {
        USHORT nDocW = pDoc->GetColWidth( nCol, nTab );
        if (nDocW)
            nPosX += (long) (nDocW * nScaleX);
    }
    long nSizeX = (long) ( pDoc->GetColWidth( nEndCol, nTab ) * nScaleX );

    long nPosY = 0;
    USHORT nEndRow = rCellPos.Row();
    for (USHORT nRow = rRange.aStart.Row(); nRow < nEndRow; nRow++)
    {
        USHORT nDocH = pDoc->FastGetRowHeight( nRow, nTab );
        if (nDocH)
            nPosY += (long) (nDocH * nScaleY);
    }
    long nSizeY = (long) ( pDoc->FastGetRowHeight( nEndRow, nTab ) * nScaleY );

    Size aOffsetLogic( nPosX, nPosY );
    Size aSizeLogic( nSizeX, nSizeY );
    Size aOffsetPixel = pWindow->LogicToPixel( aOffsetLogic, aCellMapMode );
    Size aSizePixel = pWindow->LogicToPixel( aSizeLogic, aCellMapMode );

    return Rectangle( Point( aOffsetPixel.Width(), aOffsetPixel.Height() ), aSizePixel );
}

BOOL ScPreviewLocationData::GetCell( const Point& rPos, ScAddress& rCellPos, Rectangle& rCellRect ) const
{
    ScPreviewLocationEntry* pEntry = lcl_GetEntryByPosition( aEntries, rPos, SC_PLOC_CELLRANGE );
    if ( pEntry )
    {
        Size aOffsetPixel( rPos.X() - pEntry->aPixelRect.Left(), rPos.Y() - pEntry->aPixelRect.Top() );
        rCellPos = GetCellFromRange( aOffsetPixel, pEntry->aCellRange );

        Rectangle aOffsetRect = GetOffsetPixel( rCellPos, pEntry->aCellRange );
        rCellRect = Rectangle( aOffsetRect.Left() + pEntry->aPixelRect.Left(),
                               aOffsetRect.Top() + pEntry->aPixelRect.Top(),
                               aOffsetRect.Right() + pEntry->aPixelRect.Left(),
                               aOffsetRect.Bottom() + pEntry->aPixelRect.Top() );
        return TRUE;
    }
    return FALSE;
}

BOOL ScPreviewLocationData::GetCellPosition( const ScAddress& rCellPos, Rectangle& rCellRect ) const
{
    ScPreviewLocationEntry* pEntry = lcl_GetEntryByAddress( aEntries, rCellPos, SC_PLOC_CELLRANGE );
    if ( pEntry )
    {
        Rectangle aOffsetRect = GetOffsetPixel( rCellPos, pEntry->aCellRange );
        rCellRect = Rectangle( aOffsetRect.Left() + pEntry->aPixelRect.Left(),
                               aOffsetRect.Top() + pEntry->aPixelRect.Top(),
                               aOffsetRect.Right() + pEntry->aPixelRect.Left(),
                               aOffsetRect.Bottom() + pEntry->aPixelRect.Top() );
        return TRUE;
    }
    return FALSE;
}

