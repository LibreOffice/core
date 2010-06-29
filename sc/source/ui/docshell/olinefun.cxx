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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



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

void lcl_PaintWidthHeight( ScDocShell& rDocShell, SCTAB nTab,
                                    BOOL bColumns, SCCOLROW nStart, SCCOLROW nEnd )
{
    ScDocument* pDoc = rDocShell.GetDocument();

    USHORT nParts = PAINT_GRID;
    SCCOL nStartCol = 0;
    SCROW nStartRow = 0;
    SCCOL nEndCol = MAXCOL;         // fuer Test auf Merge
    SCROW nEndRow = MAXROW;
    if ( bColumns )
    {
        nParts |= PAINT_TOP;
        nStartCol = static_cast<SCCOL>(nStart);
        nEndCol = static_cast<SCCOL>(nEnd);
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
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nTab = rRange.aStart.Tab();

    ScDocument* pDoc = rDocShell.GetDocument();
    ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab, TRUE );
    ScOutlineTable* pUndoTab = NULL;

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = FALSE;

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

        if (pDoc->IsStreamValid(nTab))
            pDoc->SetStreamValid(nTab, FALSE);

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

    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nTab = rRange.aStart.Tab();

    ScDocument* pDoc = rDocShell.GetDocument();

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = FALSE;
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

            if (pDoc->IsStreamValid(nTab))
                pDoc->SetStreamValid(nTab, FALSE);

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

BOOL ScOutlineDocFunc::RemoveAllOutlines( SCTAB nTab, BOOL bRecord, BOOL bApi )
{
    BOOL bSuccess = FALSE;
    ScDocument* pDoc = rDocShell.GetDocument();

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = FALSE;
    ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );
    if (pTable)
    {
        if (bRecord)
        {
            SCCOLROW nCol1, nCol2, nRow1, nRow2;
            pTable->GetColArray()->GetRange( nCol1, nCol2 );
            pTable->GetRowArray()->GetRange( nRow1, nRow2 );
            SCCOL nStartCol = static_cast<SCCOL>(nCol1);
            SCROW nStartRow = nRow1;
            SCCOL nEndCol = static_cast<SCCOL>(nCol2);
            SCROW nEndRow = nRow2;

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

        if (pDoc->IsStreamValid(nTab))
            pDoc->SetStreamValid(nTab, FALSE);

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
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nTab = rRange.aStart.Tab();

    ScDocument* pDoc = rDocShell.GetDocument();

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = FALSE;
    ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );

    ScDocument* pUndoDoc = NULL;
    ScOutlineTable* pUndoTab = NULL;

    if ( pTable )
    {
        if ( bRecord )
        {
            pUndoTab = new ScOutlineTable( *pTable );

            SCCOLROW nCol1, nCol2, nRow1, nRow2;
            pTable->GetColArray()->GetRange( nCol1, nCol2 );
            pTable->GetRowArray()->GetRange( nRow1, nRow2 );
            SCCOL nOutStartCol = static_cast<SCCOL>(nCol1);;
            SCROW nOutStartRow = nRow1;
            SCCOL nOutEndCol = static_cast<SCCOL>(nCol2);;
            SCROW nOutEndRow = nRow2;

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

    if (pDoc->IsStreamValid(nTab))
        pDoc->SetStreamValid(nTab, FALSE);

    rDocShell.PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, PAINT_LEFT | PAINT_TOP | PAINT_SIZE );
    rDocShell.SetDocumentModified();
    lcl_InvalidateOutliner( rDocShell.GetViewBindings() );

    return TRUE;
}

//------------------------------------------------------------------------

BOOL ScOutlineDocFunc::SelectLevel( SCTAB nTab, BOOL bColumns, USHORT nLevel,
                                    BOOL bRecord, BOOL bPaint, BOOL /* bApi */ )
{
    ScDocument* pDoc = rDocShell.GetDocument();

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = FALSE;
    ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );             // ist schon da
    if (!pTable)
        return FALSE;
    ScOutlineArray* pArray = bColumns ? pTable->GetColArray() : pTable->GetRowArray();
    if (!pArray)
        return FALSE;

    SCCOLROW nStart, nEnd;
    pArray->GetRange( nStart, nEnd );

    if ( bRecord )
    {
        ScOutlineTable* pUndoTab = new ScOutlineTable( *pTable );
        ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        if (bColumns)
        {
            pUndoDoc->InitUndo( pDoc, nTab, nTab, TRUE, FALSE );
            pDoc->CopyToDocument( static_cast<SCCOL>(nStart), 0, nTab,
                    static_cast<SCCOL>(nEnd), MAXROW, nTab, IDF_NONE, FALSE,
                    pUndoDoc );
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

        SCCOLROW nThisStart = pEntry->GetStart();
        SCCOLROW nThisEnd   = pEntry->GetEnd();
        for (SCCOLROW i=nThisStart; i<=nThisEnd; i++)
        {
            if ( bColumns )
                pDoc->ShowCol( static_cast<SCCOL>(i), nTab, bShow );
            else
                if ( !bShow || !pDoc->RowFiltered( i,nTab ) )
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

    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nTab = rRange.aStart.Tab();

    ScDocument* pDoc = rDocShell.GetDocument();

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = FALSE;
    ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );

    if (pTable)
    {
        ScOutlineArray* pArray;
        ScOutlineEntry* pEntry;
        SCCOLROW nStart;
        SCCOLROW nEnd;
        SCCOLROW nMin;
        SCCOLROW nMax;
        SCCOLROW i;

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
            pDoc->ShowCol( static_cast<SCCOL>(i), nTab, TRUE );

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
            if ( !pDoc->RowFiltered( i,nTab ) )             // weggefilterte nicht einblenden
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

    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nTab = rRange.aStart.Tab();

    ScDocument* pDoc = rDocShell.GetDocument();

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = FALSE;
    ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );

    if (pTable)
    {
        ScOutlineEntry* pEntry;
        USHORT nColLevel;
        USHORT nRowLevel;
        USHORT nCount;
        SCCOLROW nStart;
        SCCOLROW nEnd;
        USHORT i;

        SCCOLROW nEffStartCol = nStartCol;
        SCCOLROW nEffEndCol   = nEndCol;
        ScOutlineArray* pColArray = pTable->GetColArray();
        pColArray->FindTouchedLevel( nStartCol, nEndCol, nColLevel );
        pColArray->ExtendBlock( nColLevel, nEffStartCol, nEffEndCol );
        SCCOLROW nEffStartRow = nStartRow;
        SCCOLROW nEffEndRow   = nEndRow;
        ScOutlineArray* pRowArray = pTable->GetRowArray();
        pRowArray->FindTouchedLevel( nStartRow, nEndRow, nRowLevel );
        pRowArray->ExtendBlock( nRowLevel, nEffStartRow, nEffEndRow );

        if ( bRecord )
        {
            ScOutlineTable* pUndoTab = new ScOutlineTable( *pTable );
            ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( pDoc, nTab, nTab, TRUE, TRUE );
            pDoc->CopyToDocument( static_cast<SCCOL>(nEffStartCol), 0, nTab,
                    static_cast<SCCOL>(nEffEndCol), MAXROW, nTab, IDF_NONE,
                    FALSE, pUndoDoc );
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

            if ( static_cast<SCCOLROW>(nStartCol)<=nEnd && static_cast<SCCOLROW>(nEndCol)>=nStart )
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

BOOL ScOutlineDocFunc::ShowOutline( SCTAB nTab, BOOL bColumns, USHORT nLevel, USHORT nEntry,
                                    BOOL bRecord, BOOL bPaint, BOOL /* bApi */ )
{
    ScDocument* pDoc = rDocShell.GetDocument();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = FALSE;

    ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );
    ScOutlineArray* pArray = bColumns ? pTable->GetColArray() : pTable->GetRowArray();
    ScOutlineEntry* pEntry = pArray->GetEntry( nLevel, nEntry );
    SCCOLROW nStart = pEntry->GetStart();
    SCCOLROW nEnd   = pEntry->GetEnd();

    if ( bRecord )
    {
        ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        if (bColumns)
        {
            pUndoDoc->InitUndo( pDoc, nTab, nTab, TRUE, FALSE );
            pDoc->CopyToDocument( static_cast<SCCOL>(nStart), 0, nTab,
                    static_cast<SCCOL>(nEnd), MAXROW, nTab, IDF_NONE, FALSE,
                    pUndoDoc );
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
    SCCOLROW i;
    for ( i = nStart; i <= nEnd; i++ )
    {
        if ( bColumns )
            pDoc->ShowCol( static_cast<SCCOL>(i), nTab, TRUE );
        else
            if ( !pDoc->RowFiltered( i,nTab ) )             // weggefilterte nicht einblenden
                pDoc->ShowRow( i, nTab, TRUE );
    }

    ScSubOutlineIterator aIter( pArray, nLevel, nEntry );
    while ((pEntry=aIter.GetNext()) != NULL)
    {
        if ( pEntry->IsHidden() )
        {
            SCCOLROW nSubStart = pEntry->GetStart();
            SCCOLROW nSubEnd   = pEntry->GetEnd();
            for ( i = nSubStart; i <= nSubEnd; i++ )
            {
                if ( bColumns )
                    pDoc->ShowCol( static_cast<SCCOL>(i), nTab, FALSE );
                else
                    pDoc->ShowRow( i, nTab, FALSE );
            }
        }
    }

    pArray->SetVisibleBelow( nLevel, nEntry, TRUE, TRUE );

    pDoc->InvalidatePageBreaks(nTab);
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

BOOL ScOutlineDocFunc::HideOutline( SCTAB nTab, BOOL bColumns, USHORT nLevel, USHORT nEntry,
                                    BOOL bRecord, BOOL bPaint, BOOL /* bApi */ )
{
    ScDocument* pDoc = rDocShell.GetDocument();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = FALSE;

    ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );
    ScOutlineArray* pArray = bColumns ? pTable->GetColArray() : pTable->GetRowArray();
    ScOutlineEntry* pEntry = pArray->GetEntry( nLevel, nEntry );
    SCCOLROW nStart = pEntry->GetStart();
    SCCOLROW nEnd   = pEntry->GetEnd();

    if ( bRecord )
    {
        ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        if (bColumns)
        {
            pUndoDoc->InitUndo( pDoc, nTab, nTab, TRUE, FALSE );
            pDoc->CopyToDocument( static_cast<SCCOL>(nStart), 0, nTab,
                    static_cast<SCCOL>(nEnd), MAXROW, nTab, IDF_NONE, FALSE,
                    pUndoDoc );
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
    SCCOLROW i;
    for ( i = nStart; i <= nEnd; i++ )
    {
        if ( bColumns )
            pDoc->ShowCol( static_cast<SCCOL>(i), nTab, FALSE );
        else
            pDoc->ShowRow( i, nTab, FALSE );
    }

    pArray->SetVisibleBelow( nLevel, nEntry, FALSE );

    pDoc->InvalidatePageBreaks(nTab);
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





