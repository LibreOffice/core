/*************************************************************************
 *
 *  $RCSfile: olinefun.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-09-22 18:40:59 $
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

#include <vcl/sound.hxx>
#include <sfx2/bindings.hxx>

#include "olinefun.hxx"

#include "docsh.hxx"
#include "olinetab.hxx"
#include "undodat.hxx"
#include "globstr.hrc"
#include "sc.hrc"


//========================================================================

void lcl_InvalidateOutliner( SfxBindings* pBindings )
{
    if ( pBindings )
    {
        pBindings->Invalidate( SID_OUTLINE_SHOW );
        pBindings->Invalidate( SID_OUTLINE_HIDE );
        pBindings->Invalidate( SID_OUTLINE_REMOVE );

        pBindings->Invalidate( SID_STATUS_SUM );            // wegen ein-/ausblenden
        pBindings->Invalidate( SID_ATTR_SIZE );
    }
}

//------------------------------------------------------------------------

//! PaintWidthHeight zur DocShell verschieben?

void lcl_PaintWidthHeight( ScDocShell& rDocShell, USHORT nTab,
                                    BOOL bColumns, USHORT nStart, USHORT nEnd )
{
    ScDocument* pDoc = rDocShell.GetDocument();

    USHORT nParts = PAINT_GRID;
    USHORT nStartCol = 0;
    USHORT nStartRow = 0;
    USHORT nEndCol = MAXCOL;            // fuer Test auf Merge
    USHORT nEndRow = MAXROW;
    if ( bColumns )
    {
        nParts |= PAINT_TOP;
        nStartCol = nStart;
        nEndCol = nEnd;
    }
    else
    {
        nParts |= PAINT_LEFT;
        nStartRow = nStart;
        nEndRow = nEnd;
    }
    if (pDoc->HasAttrib( nStartCol,nStartRow,nTab, nEndCol,nEndRow,nTab,
                            HASATTR_MERGED | HASATTR_OVERLAPPED ))
    {
        nStartCol = 0;
        nStartRow = 0;
    }
    rDocShell.PostPaint( nStartCol,nStartRow,nTab, MAXCOL,MAXROW,nTab, nParts );
}

//------------------------------------------------------------------------

BOOL ScOutlineDocFunc::MakeOutline( const ScRange& rRange, BOOL bColumns, BOOL bRecord, BOOL bApi )
{
    BOOL bSuccess = FALSE;
    USHORT nStartCol = rRange.aStart.Col();
    USHORT nStartRow = rRange.aStart.Row();
    USHORT nEndCol = rRange.aEnd.Col();
    USHORT nEndRow = rRange.aEnd.Row();
    USHORT nTab = rRange.aStart.Tab();

    ScDocument* pDoc = rDocShell.GetDocument();
    ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab, TRUE );
    ScOutlineTable* pUndoTab = NULL;

    if (bRecord)
        pUndoTab = new ScOutlineTable( *pTable );

    ScOutlineArray* pArray = bColumns ? pTable->GetColArray() : pTable->GetRowArray();

    BOOL bRes;
    BOOL bSize = FALSE;
    if ( bColumns )
        bRes = pArray->Insert( nStartCol, nEndCol, bSize );
    else
        bRes = pArray->Insert( nStartRow, nEndRow, bSize );

    if ( bRes )
    {
        if (bRecord)
        {
            rDocShell.GetUndoManager()->AddUndoAction(
                new ScUndoMakeOutline( &rDocShell,
                                        nStartCol,nStartRow,nTab,nEndCol,nEndRow,nTab,
                                        pUndoTab, bColumns, TRUE ) );
        }

        USHORT nParts = 0;              // Datenbereich nicht geaendert
        if ( bColumns )
            nParts |= PAINT_TOP;
        else
            nParts |= PAINT_LEFT;
        if ( bSize )
            nParts |= PAINT_SIZE;

        rDocShell.PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, nParts );
        rDocShell.SetDocumentModified();
        lcl_InvalidateOutliner( rDocShell.GetViewBindings() );
        bSuccess = TRUE;
    }
    else
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_MSSG_MAKEOUTLINE_0); // "Gruppierung nicht moeglich"
        delete pUndoTab;
    }

    return bSuccess;
}

BOOL ScOutlineDocFunc::RemoveOutline( const ScRange& rRange, BOOL bColumns, BOOL bRecord, BOOL bApi )
{
    BOOL bDone = FALSE;

    USHORT nStartCol = rRange.aStart.Col();
    USHORT nStartRow = rRange.aStart.Row();
    USHORT nEndCol = rRange.aEnd.Col();
    USHORT nEndRow = rRange.aEnd.Row();
    USHORT nTab = rRange.aStart.Tab();

    ScDocument* pDoc = rDocShell.GetDocument();
    ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );
    if (pTable)
    {
        ScOutlineTable* pUndoTab = NULL;
        if (bRecord)
            pUndoTab = new ScOutlineTable( *pTable );

        ScOutlineArray* pArray = bColumns ? pTable->GetColArray() : pTable->GetRowArray();

        BOOL bRes;
        BOOL bSize = FALSE;
        if ( bColumns )
            bRes = pArray->Remove( nStartCol, nEndCol, bSize );
        else
            bRes = pArray->Remove( nStartRow, nEndRow, bSize );

        if ( bRes )
        {
            if (bRecord)
            {
                rDocShell.GetUndoManager()->AddUndoAction(
                    new ScUndoMakeOutline( &rDocShell,
                                            nStartCol,nStartRow,nTab, nEndCol,nEndRow,nTab,
                                            pUndoTab, bColumns, FALSE ) );
            }

            USHORT nParts = 0;              // Datenbereich nicht geaendert
            if ( bColumns )
                nParts |= PAINT_TOP;
            else
                nParts |= PAINT_LEFT;
            if ( bSize )
                nParts |= PAINT_SIZE;

            rDocShell.PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, nParts );
            rDocShell.SetDocumentModified();
            bDone = TRUE;
            lcl_InvalidateOutliner( rDocShell.GetViewBindings() );

            // es wird nicht wieder eingeblendet -> kein UpdatePageBreaks
        }
        else
            delete pUndoTab;
    }

    if (!bDone && !bApi)
        rDocShell.ErrorMessage(STR_MSSG_REMOVEOUTLINE_0);   // "Aufheben nicht moeglich"

    return bDone;
}

BOOL ScOutlineDocFunc::RemoveAllOutlines( USHORT nTab, BOOL bRecord, BOOL bApi )
{
    BOOL bSuccess = FALSE;
    ScDocument* pDoc = rDocShell.GetDocument();
    ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );
    if (pTable)
    {
        USHORT nStartCol;
        USHORT nStartRow;
        USHORT nEndCol;
        USHORT nEndRow;
        pTable->GetColArray()->GetRange( nStartCol, nEndCol );
        pTable->GetRowArray()->GetRange( nStartRow, nEndRow );

        if (bRecord)
        {
            ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( pDoc, nTab, nTab, TRUE, TRUE );
            pDoc->CopyToDocument( nStartCol, 0, nTab, nEndCol, MAXROW, nTab, IDF_NONE, FALSE, pUndoDoc );
            pDoc->CopyToDocument( 0, nStartRow, nTab, MAXCOL, nEndRow, nTab, IDF_NONE, FALSE, pUndoDoc );

            ScOutlineTable* pUndoTab = new ScOutlineTable( *pTable );

            rDocShell.GetUndoManager()->AddUndoAction(
                new ScUndoRemoveAllOutlines( &rDocShell,
                                                nStartCol, nStartRow, nTab,
                                                nEndCol, nEndRow, nTab,
                                                pUndoDoc, pUndoTab ) );
        }

        SelectLevel( nTab, TRUE,  pTable->GetColArray()->GetDepth(), FALSE, FALSE, bApi );
        SelectLevel( nTab, FALSE, pTable->GetRowArray()->GetDepth(), FALSE, FALSE, bApi );
        pDoc->SetOutlineTable( nTab, NULL );

        pDoc->UpdatePageBreaks( nTab );

        rDocShell.PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab,
                                    PAINT_GRID | PAINT_LEFT | PAINT_TOP | PAINT_SIZE );
        rDocShell.SetDocumentModified();
        lcl_InvalidateOutliner( rDocShell.GetViewBindings() );
        bSuccess = TRUE;
    }
    else if (!bApi)
        Sound::Beep();

    return bSuccess;
}

//------------------------------------------------------------------------

BOOL ScOutlineDocFunc::AutoOutline( const ScRange& rRange, BOOL bRecord, BOOL bApi )
{
    USHORT nStartCol = rRange.aStart.Col();
    USHORT nStartRow = rRange.aStart.Row();
    USHORT nEndCol = rRange.aEnd.Col();
    USHORT nEndRow = rRange.aEnd.Row();
    USHORT nTab = rRange.aStart.Tab();

    ScDocument* pDoc = rDocShell.GetDocument();
    ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );

    ScDocument* pUndoDoc = NULL;
    ScOutlineTable* pUndoTab = NULL;

    if ( pTable )
    {
        if ( bRecord )
        {
            pUndoTab = new ScOutlineTable( *pTable );

            USHORT nOutStartCol;
            USHORT nOutStartRow;
            USHORT nOutEndCol;
            USHORT nOutEndRow;
            pTable->GetColArray()->GetRange( nOutStartCol, nOutEndCol );
            pTable->GetRowArray()->GetRange( nOutStartRow, nOutEndRow );

            pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( pDoc, nTab, nTab, TRUE, TRUE );
            pDoc->CopyToDocument( nOutStartCol, 0, nTab, nOutEndCol, MAXROW, nTab, IDF_NONE, FALSE, pUndoDoc );
            pDoc->CopyToDocument( 0, nOutStartRow, nTab, MAXCOL, nOutEndRow, nTab, IDF_NONE, FALSE, pUndoDoc );
        }

        // einblenden
        SelectLevel( nTab, TRUE,  pTable->GetColArray()->GetDepth(), FALSE, FALSE, bApi );
        SelectLevel( nTab, FALSE, pTable->GetRowArray()->GetDepth(), FALSE, FALSE, bApi );
        pDoc->SetOutlineTable( nTab, NULL );
    }

    pDoc->DoAutoOutline( nStartCol,nStartRow, nEndCol,nEndRow, nTab );

    if (bRecord)
    {
        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoAutoOutline( &rDocShell,
                                    nStartCol, nStartRow, nTab,
                                    nEndCol, nEndRow, nTab,
                                    pUndoDoc, pUndoTab ) );
    }

    rDocShell.PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, PAINT_LEFT | PAINT_TOP | PAINT_SIZE );
    rDocShell.SetDocumentModified();
    lcl_InvalidateOutliner( rDocShell.GetViewBindings() );

    return TRUE;
}

//------------------------------------------------------------------------

BOOL ScOutlineDocFunc::SelectLevel( USHORT nTab, BOOL bColumns, USHORT nLevel,
                                    BOOL bRecord, BOOL bPaint, BOOL bApi )
{
    ScDocument* pDoc = rDocShell.GetDocument();
    ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );             // ist schon da
    if (!pTable)
        return FALSE;
    ScOutlineArray* pArray = bColumns ? pTable->GetColArray() : pTable->GetRowArray();
    if (!pArray)
        return FALSE;

    USHORT nStart;
    USHORT nEnd;
    pArray->GetRange( nStart, nEnd );

    if ( bRecord )
    {
        ScOutlineTable* pUndoTab = new ScOutlineTable( *pTable );
        ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        if (bColumns)
        {
            pUndoDoc->InitUndo( pDoc, nTab, nTab, TRUE, FALSE );
            pDoc->CopyToDocument( nStart, 0, nTab, nEnd, MAXROW, nTab, IDF_NONE, FALSE, pUndoDoc );
        }
        else
        {
            pUndoDoc->InitUndo( pDoc, nTab, nTab, FALSE, TRUE );
            pDoc->CopyToDocument( 0, nStart, nTab, MAXCOL, nEnd, nTab, IDF_NONE, FALSE, pUndoDoc );
        }

        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoOutlineLevel( &rDocShell,
                                    nStart, nEnd, nTab,             //! start und end berechnen
                                    pUndoDoc, pUndoTab,
                                    bColumns, nLevel ) );
    }

    ScSubOutlineIterator aIter( pArray );                   // alle Eintraege
    ScOutlineEntry* pEntry;
    while ((pEntry=aIter.GetNext()) != NULL)
    {
        USHORT nThisLevel = aIter.LastLevel();
        BOOL bShow = (nThisLevel < nLevel);
        if (bShow)                                          // einblenden
        {
            pEntry->SetHidden( FALSE );
            pEntry->SetVisible( TRUE );
        }
        else if ( nThisLevel == nLevel )                    // ausblenden
        {
            pEntry->SetHidden( TRUE );
            pEntry->SetVisible( TRUE );
        }
        else                                                // verdeckt
        {
            pEntry->SetVisible( FALSE );
        }

        USHORT nThisStart = pEntry->GetStart();
        USHORT nThisEnd   = pEntry->GetEnd();
        for (USHORT i=nThisStart; i<=nThisEnd; i++)
        {
            if ( bColumns )
                pDoc->ShowCol( i, nTab, bShow );
            else
                if ( !bShow || !pDoc->IsFiltered( i,nTab ) )
                    pDoc->ShowRow( i, nTab, bShow );
        }
    }

    pDoc->UpdatePageBreaks( nTab );

    if (bPaint)
        lcl_PaintWidthHeight( rDocShell, nTab, bColumns, nStart, nEnd );

    rDocShell.SetDocumentModified();
    lcl_InvalidateOutliner( rDocShell.GetViewBindings() );

    return TRUE;
}

//------------------------------------------------------------------------

BOOL ScOutlineDocFunc::ShowMarkedOutlines( const ScRange& rRange, BOOL bRecord, BOOL bApi )
{
    BOOL bDone = FALSE;

    USHORT nStartCol = rRange.aStart.Col();
    USHORT nStartRow = rRange.aStart.Row();
    USHORT nEndCol = rRange.aEnd.Col();
    USHORT nEndRow = rRange.aEnd.Row();
    USHORT nTab = rRange.aStart.Tab();

    ScDocument* pDoc = rDocShell.GetDocument();
    ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );

    if (pTable)
    {
        ScOutlineArray* pArray;
        ScOutlineEntry* pEntry;
        USHORT nStart;
        USHORT nEnd;
        USHORT nMin;
        USHORT nMax;
        USHORT i;

        if ( bRecord )
        {
            ScOutlineTable* pUndoTab = new ScOutlineTable( *pTable );
            ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( pDoc, nTab, nTab, TRUE, TRUE );
            pDoc->CopyToDocument( nStartCol, 0, nTab, nEndCol, MAXROW, nTab, IDF_NONE, FALSE, pUndoDoc );
            pDoc->CopyToDocument( 0, nStartRow, nTab, MAXCOL, nEndRow, nTab, IDF_NONE, FALSE, pUndoDoc );

            rDocShell.GetUndoManager()->AddUndoAction(
                new ScUndoOutlineBlock( &rDocShell,
                                        nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab,
                                        pUndoDoc, pUndoTab, TRUE ) );
        }

        //  Spalten

        nMin=MAXCOL;
        nMax=0;
        pArray = pTable->GetColArray();
        ScSubOutlineIterator aColIter( pArray );
        while ((pEntry=aColIter.GetNext()) != NULL)
        {
            nStart = pEntry->GetStart();
            nEnd   = pEntry->GetEnd();
            if ( nStart>=nStartCol && nEnd<=nEndCol )
            {
                pEntry->SetHidden( FALSE );
                pEntry->SetVisible( TRUE );
                if (nStart<nMin) nMin=nStart;
                if (nEnd>nMax) nMax=nEnd;
            }
        }
        for ( i=nMin; i<=nMax; i++ )
            pDoc->ShowCol( i, nTab, TRUE );

        //  Zeilen

        nMin=MAXROW;
        nMax=0;
        pArray = pTable->GetRowArray();
        ScSubOutlineIterator aRowIter( pArray );
        while ((pEntry=aRowIter.GetNext()) != NULL)
        {
            nStart = pEntry->GetStart();
            nEnd   = pEntry->GetEnd();
            if ( nStart>=nStartRow && nEnd<=nEndRow )
            {
                pEntry->SetHidden( FALSE );
                pEntry->SetVisible( TRUE );
                if (nStart<nMin) nMin=nStart;
                if (nEnd>nMax) nMax=nEnd;
            }
        }
        for ( i=nMin; i<=nMax; i++ )
            if ( !pDoc->IsFiltered( i,nTab ) )              // weggefilterte nicht einblenden
                pDoc->ShowRow( i, nTab, TRUE );

        pDoc->UpdatePageBreaks( nTab );

        rDocShell.PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, PAINT_GRID | PAINT_LEFT | PAINT_TOP );

        rDocShell.SetDocumentModified();
        bDone = TRUE;

        lcl_InvalidateOutliner( rDocShell.GetViewBindings() );
    }

    if (!bDone && !bApi)
        Sound::Beep();

    return bDone;
}

BOOL ScOutlineDocFunc::HideMarkedOutlines( const ScRange& rRange, BOOL bRecord, BOOL bApi )
{
    BOOL bDone = FALSE;

    USHORT nStartCol = rRange.aStart.Col();
    USHORT nStartRow = rRange.aStart.Row();
    USHORT nEndCol = rRange.aEnd.Col();
    USHORT nEndRow = rRange.aEnd.Row();
    USHORT nTab = rRange.aStart.Tab();

    ScDocument* pDoc = rDocShell.GetDocument();
    ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );

    if (pTable)
    {
        ScOutlineEntry* pEntry;
        USHORT nColLevel;
        USHORT nRowLevel;
        USHORT nCount;
        USHORT nStart;
        USHORT nEnd;
        USHORT i;

        USHORT nEffStartCol = nStartCol;
        USHORT nEffEndCol   = nEndCol;
        ScOutlineArray* pColArray = pTable->GetColArray();
        pColArray->FindTouchedLevel( nStartCol, nEndCol, nColLevel );
        pColArray->ExtendBlock( nColLevel, nEffStartCol, nEffEndCol );
        USHORT nEffStartRow = nStartRow;
        USHORT nEffEndRow   = nEndRow;
        ScOutlineArray* pRowArray = pTable->GetRowArray();
        pRowArray->FindTouchedLevel( nStartRow, nEndRow, nRowLevel );
        pRowArray->ExtendBlock( nRowLevel, nEffStartRow, nEffEndRow );

        if ( bRecord )
        {
            ScOutlineTable* pUndoTab = new ScOutlineTable( *pTable );
            ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( pDoc, nTab, nTab, TRUE, TRUE );
            pDoc->CopyToDocument( nEffStartCol, 0, nTab, nEffEndCol, MAXROW, nTab, IDF_NONE, FALSE, pUndoDoc );
            pDoc->CopyToDocument( 0, nEffStartRow, nTab, MAXCOL, nEffEndRow, nTab, IDF_NONE, FALSE, pUndoDoc );

            rDocShell.GetUndoManager()->AddUndoAction(
                new ScUndoOutlineBlock( &rDocShell,
                                        nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab,
                                        pUndoDoc, pUndoTab, FALSE ) );
        }

        //  Spalten

        nCount = pColArray->GetCount(nColLevel);
        for ( i=0; i<nCount; i++ )
        {
            pEntry = pColArray->GetEntry(nColLevel,i);
            nStart = pEntry->GetStart();
            nEnd   = pEntry->GetEnd();

            if ( nStartCol<=nEnd && nEndCol>=nStart )
                HideOutline( nTab, TRUE, nColLevel, i, FALSE, FALSE, bApi );
        }

        //  Zeilen

        nCount = pRowArray->GetCount(nRowLevel);
        for ( i=0; i<nCount; i++ )
        {
            pEntry = pRowArray->GetEntry(nRowLevel,i);
            nStart = pEntry->GetStart();
            nEnd   = pEntry->GetEnd();

            if ( nStartRow<=nEnd && nEndRow>=nStart )
                HideOutline( nTab, FALSE, nRowLevel, i, FALSE, FALSE, bApi );
        }

        pDoc->UpdatePageBreaks( nTab );

        rDocShell.PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, PAINT_GRID | PAINT_LEFT | PAINT_TOP );

        rDocShell.SetDocumentModified();
        bDone = TRUE;

        lcl_InvalidateOutliner( rDocShell.GetViewBindings() );
    }

    if (!bDone && !bApi)
        Sound::Beep();

    return bDone;
}

//------------------------------------------------------------------------

BOOL ScOutlineDocFunc::ShowOutline( USHORT nTab, BOOL bColumns, USHORT nLevel, USHORT nEntry,
                                    BOOL bRecord, BOOL bPaint, BOOL bApi )
{
    ScDocument* pDoc = rDocShell.GetDocument();

    ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );
    ScOutlineArray* pArray = bColumns ? pTable->GetColArray() : pTable->GetRowArray();
    ScOutlineEntry* pEntry = pArray->GetEntry( nLevel, nEntry );
    USHORT nStart = pEntry->GetStart();
    USHORT nEnd   = pEntry->GetEnd();

    if ( bRecord )
    {
        ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        if (bColumns)
        {
            pUndoDoc->InitUndo( pDoc, nTab, nTab, TRUE, FALSE );
            pDoc->CopyToDocument( nStart, 0, nTab, nEnd, MAXROW, nTab, IDF_NONE, FALSE, pUndoDoc );
        }
        else
        {
            pUndoDoc->InitUndo( pDoc, nTab, nTab, FALSE, TRUE );
            pDoc->CopyToDocument( 0, nStart, nTab, MAXCOL, nEnd, nTab, IDF_NONE, FALSE, pUndoDoc );
        }

        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoDoOutline( &rDocShell,
                                    nStart, nEnd, nTab, pUndoDoc,       //! start und end berechnen
                                    bColumns, nLevel, nEntry, TRUE ) );
    }

//! HideCursor();

    pEntry->SetHidden(FALSE);
    USHORT i;
    for ( i = nStart; i <= nEnd; i++ )
    {
        if ( bColumns )
            pDoc->ShowCol( i, nTab, TRUE );
        else
            if ( !pDoc->IsFiltered( i,nTab ) )              // weggefilterte nicht einblenden
                pDoc->ShowRow( i, nTab, TRUE );
    }

    ScSubOutlineIterator aIter( pArray, nLevel, nEntry );
    while ((pEntry=aIter.GetNext()) != NULL)
    {
        if ( pEntry->IsHidden() )
        {
            USHORT nSubStart = pEntry->GetStart();
            USHORT nSubEnd   = pEntry->GetEnd();
            for ( i = nSubStart; i <= nSubEnd; i++ )
            {
                if ( bColumns )
                    pDoc->ShowCol( i, nTab, FALSE );
                else
                    pDoc->ShowRow( i, nTab, FALSE );
            }
        }
    }

    pArray->SetVisibleBelow( nLevel, nEntry, TRUE, TRUE );

    pDoc->UpdatePageBreaks( nTab );

    if (bPaint)
        lcl_PaintWidthHeight( rDocShell, nTab, bColumns, nStart, nEnd );

//! ShowCursor();
    rDocShell.SetDocumentModified();

//! if (bPaint)
//!     UpdateScrollBars();

    lcl_InvalidateOutliner( rDocShell.GetViewBindings() );

    return TRUE;        //! immer ???
}

BOOL ScOutlineDocFunc::HideOutline( USHORT nTab, BOOL bColumns, USHORT nLevel, USHORT nEntry,
                                    BOOL bRecord, BOOL bPaint, BOOL bApi )
{
    ScDocument* pDoc = rDocShell.GetDocument();

    ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );
    ScOutlineArray* pArray = bColumns ? pTable->GetColArray() : pTable->GetRowArray();
    ScOutlineEntry* pEntry = pArray->GetEntry( nLevel, nEntry );
    USHORT nStart = pEntry->GetStart();
    USHORT nEnd   = pEntry->GetEnd();

    if ( bRecord )
    {
        ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        if (bColumns)
        {
            pUndoDoc->InitUndo( pDoc, nTab, nTab, TRUE, FALSE );
            pDoc->CopyToDocument( nStart, 0, nTab, nEnd, MAXROW, nTab, IDF_NONE, FALSE, pUndoDoc );
        }
        else
        {
            pUndoDoc->InitUndo( pDoc, nTab, nTab, FALSE, TRUE );
            pDoc->CopyToDocument( 0, nStart, nTab, MAXCOL, nEnd, nTab, IDF_NONE, FALSE, pUndoDoc );
        }

        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoDoOutline( &rDocShell,
                                    nStart, nEnd, nTab, pUndoDoc,
                                    bColumns, nLevel, nEntry, FALSE ) );
    }

//! HideCursor();

    pEntry->SetHidden(TRUE);
    USHORT i;
    for ( i = nStart; i <= nEnd; i++ )
    {
        if ( bColumns )
            pDoc->ShowCol( i, nTab, FALSE );
        else
            pDoc->ShowRow( i, nTab, FALSE );
    }

    pArray->SetVisibleBelow( nLevel, nEntry, FALSE );

    pDoc->UpdatePageBreaks( nTab );

    if (bPaint)
        lcl_PaintWidthHeight( rDocShell, nTab, bColumns, nStart, nEnd );

//! ShowCursor();
    rDocShell.SetDocumentModified();

//! if (bPaint)
//!     UpdateScrollBars();

    lcl_InvalidateOutliner( rDocShell.GetViewBindings() );

    return TRUE;        //! immer ???
}





