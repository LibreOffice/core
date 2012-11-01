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

#include <sfx2/bindings.hxx>

#include "olinefun.hxx"

#include "docsh.hxx"
#include "olinetab.hxx"
#include "undodat.hxx"
#include "globstr.hrc"
#include "sc.hrc"


//========================================================================

static void lcl_InvalidateOutliner( SfxBindings* pBindings )
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

static void lcl_PaintWidthHeight( ScDocShell& rDocShell, SCTAB nTab,
                                    sal_Bool bColumns, SCCOLROW nStart, SCCOLROW nEnd )
{
    ScDocument* pDoc = rDocShell.GetDocument();

    sal_uInt16 nParts = PAINT_GRID;
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

sal_Bool ScOutlineDocFunc::MakeOutline( const ScRange& rRange, sal_Bool bColumns, sal_Bool bRecord, sal_Bool bApi )
{
    sal_Bool bSuccess = false;
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nTab = rRange.aStart.Tab();

    ScDocument* pDoc = rDocShell.GetDocument();
    ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab, sal_True );
    ScOutlineTable* pUndoTab = NULL;

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;

    if (bRecord)
        pUndoTab = new ScOutlineTable( *pTable );

    ScOutlineArray* pArray = bColumns ? pTable->GetColArray() : pTable->GetRowArray();

    bool bRes;
    bool bSize = false;
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
                                        pUndoTab, bColumns, sal_True ) );
        }

        if (pDoc->IsStreamValid(nTab))
            pDoc->SetStreamValid(nTab, false);

        sal_uInt16 nParts = 0;              // Datenbereich nicht geaendert
        if ( bColumns )
            nParts |= PAINT_TOP;
        else
            nParts |= PAINT_LEFT;
        if ( bSize )
            nParts |= PAINT_SIZE;

        rDocShell.PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, nParts );
        rDocShell.SetDocumentModified();
        lcl_InvalidateOutliner( rDocShell.GetViewBindings() );
        bSuccess = sal_True;
    }
    else
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_MSSG_MAKEOUTLINE_0); // "Gruppierung nicht moeglich"
        delete pUndoTab;
    }

    return bSuccess;
}

sal_Bool ScOutlineDocFunc::RemoveOutline( const ScRange& rRange, sal_Bool bColumns, sal_Bool bRecord, sal_Bool bApi )
{
    sal_Bool bDone = false;

    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nTab = rRange.aStart.Tab();

    ScDocument* pDoc = rDocShell.GetDocument();

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;
    ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );
    if (pTable)
    {
        ScOutlineTable* pUndoTab = NULL;
        if (bRecord)
            pUndoTab = new ScOutlineTable( *pTable );

        ScOutlineArray* pArray = bColumns ? pTable->GetColArray() : pTable->GetRowArray();

        bool bRes;
        bool bSize = false;
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
                                            pUndoTab, bColumns, false ) );
            }

            if (pDoc->IsStreamValid(nTab))
                pDoc->SetStreamValid(nTab, false);

            sal_uInt16 nParts = 0;              // Datenbereich nicht geaendert
            if ( bColumns )
                nParts |= PAINT_TOP;
            else
                nParts |= PAINT_LEFT;
            if ( bSize )
                nParts |= PAINT_SIZE;

            rDocShell.PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, nParts );
            rDocShell.SetDocumentModified();
            bDone = sal_True;
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

sal_Bool ScOutlineDocFunc::RemoveAllOutlines( SCTAB nTab, sal_Bool bRecord, sal_Bool bApi )
{
    sal_Bool bSuccess = false;
    ScDocument* pDoc = rDocShell.GetDocument();

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;
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
            pUndoDoc->InitUndo( pDoc, nTab, nTab, sal_True, sal_True );
            pDoc->CopyToDocument( nStartCol, 0, nTab, nEndCol, MAXROW, nTab, IDF_NONE, false, pUndoDoc );
            pDoc->CopyToDocument( 0, nStartRow, nTab, MAXCOL, nEndRow, nTab, IDF_NONE, false, pUndoDoc );

            ScOutlineTable* pUndoTab = new ScOutlineTable( *pTable );

            rDocShell.GetUndoManager()->AddUndoAction(
                new ScUndoRemoveAllOutlines( &rDocShell,
                                                nStartCol, nStartRow, nTab,
                                                nEndCol, nEndRow, nTab,
                                                pUndoDoc, pUndoTab ) );
        }

        SelectLevel( nTab, sal_True,  pTable->GetColArray()->GetDepth(), false, false, bApi );
        SelectLevel( nTab, false, pTable->GetRowArray()->GetDepth(), false, false, bApi );
        pDoc->SetOutlineTable( nTab, NULL );

        pDoc->UpdatePageBreaks( nTab );

        if (pDoc->IsStreamValid(nTab))
            pDoc->SetStreamValid(nTab, false);

        rDocShell.PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab,
                                    PAINT_GRID | PAINT_LEFT | PAINT_TOP | PAINT_SIZE );
        rDocShell.SetDocumentModified();
        lcl_InvalidateOutliner( rDocShell.GetViewBindings() );
        bSuccess = sal_True;
    }

    return bSuccess;
}

//------------------------------------------------------------------------

sal_Bool ScOutlineDocFunc::AutoOutline( const ScRange& rRange, sal_Bool bRecord, sal_Bool bApi )
{
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nTab = rRange.aStart.Tab();

    ScDocument* pDoc = rDocShell.GetDocument();

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;
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
            SCCOL nOutStartCol = static_cast<SCCOL>(nCol1);
            SCROW nOutStartRow = nRow1;
            SCCOL nOutEndCol = static_cast<SCCOL>(nCol2);
            SCROW nOutEndRow = nRow2;

            pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( pDoc, nTab, nTab, sal_True, sal_True );
            pDoc->CopyToDocument( nOutStartCol, 0, nTab, nOutEndCol, MAXROW, nTab, IDF_NONE, false, pUndoDoc );
            pDoc->CopyToDocument( 0, nOutStartRow, nTab, MAXCOL, nOutEndRow, nTab, IDF_NONE, false, pUndoDoc );
        }

        // einblenden
        SelectLevel( nTab, sal_True,  pTable->GetColArray()->GetDepth(), false, false, bApi );
        SelectLevel( nTab, false, pTable->GetRowArray()->GetDepth(), false, false, bApi );
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
        pDoc->SetStreamValid(nTab, false);

    rDocShell.PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, PAINT_LEFT | PAINT_TOP | PAINT_SIZE );
    rDocShell.SetDocumentModified();
    lcl_InvalidateOutliner( rDocShell.GetViewBindings() );

    return sal_True;
}

//------------------------------------------------------------------------

sal_Bool ScOutlineDocFunc::SelectLevel( SCTAB nTab, sal_Bool bColumns, sal_uInt16 nLevel,
                                    sal_Bool bRecord, sal_Bool bPaint, sal_Bool /* bApi */ )
{
    ScDocument* pDoc = rDocShell.GetDocument();

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;
    ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );             // ist schon da
    if (!pTable)
        return false;
    ScOutlineArray* pArray = bColumns ? pTable->GetColArray() : pTable->GetRowArray();
    if (!pArray)
        return false;

    SCCOLROW nStart, nEnd;
    pArray->GetRange( nStart, nEnd );

    if ( bRecord )
    {
        ScOutlineTable* pUndoTab = new ScOutlineTable( *pTable );
        ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        if (bColumns)
        {
            pUndoDoc->InitUndo( pDoc, nTab, nTab, sal_True, false );
            pDoc->CopyToDocument( static_cast<SCCOL>(nStart), 0, nTab,
                    static_cast<SCCOL>(nEnd), MAXROW, nTab, IDF_NONE, false,
                    pUndoDoc );
        }
        else
        {
            pUndoDoc->InitUndo( pDoc, nTab, nTab, false, sal_True );
            pDoc->CopyToDocument( 0, nStart, nTab, MAXCOL, nEnd, nTab, IDF_NONE, false, pUndoDoc );
        }

        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoOutlineLevel( &rDocShell,
                                    nStart, nEnd, nTab,             //! start und end berechnen
                                    pUndoDoc, pUndoTab,
                                    bColumns, nLevel ) );
    }

    pDoc->InitializeNoteCaptions(nTab);
    ScSubOutlineIterator aIter( pArray );                   // alle Eintraege
    ScOutlineEntry* pEntry;
    while ((pEntry=aIter.GetNext()) != NULL)
    {
        sal_uInt16 nThisLevel = aIter.LastLevel();
        sal_Bool bShow = (nThisLevel < nLevel);
        if (bShow)                                          // einblenden
        {
            pEntry->SetHidden( false );
            pEntry->SetVisible( sal_True );
        }
        else if ( nThisLevel == nLevel )                    // ausblenden
        {
            pEntry->SetHidden( sal_True );
            pEntry->SetVisible( sal_True );
        }
        else                                                // verdeckt
        {
            pEntry->SetVisible( false );
        }

        SCCOLROW nThisStart = pEntry->GetStart();
        SCCOLROW nThisEnd   = pEntry->GetEnd();
        for (SCCOLROW i=nThisStart; i<=nThisEnd; i++)
        {
            if ( bColumns )
                pDoc->ShowCol( static_cast<SCCOL>(i), nTab, bShow );
            else
            {
                // show several rows together, don't show filtered rows
                SCROW nFilterEnd = i;
                bool bFiltered = pDoc->RowFiltered( i, nTab, NULL, &nFilterEnd );
                nFilterEnd = std::min( nThisEnd, nFilterEnd );
                if ( !bShow || !bFiltered )
                    pDoc->ShowRows( i, nFilterEnd, nTab, bShow );
                i = nFilterEnd;
            }
        }
    }

    pDoc->SetDrawPageSize(nTab);
    pDoc->UpdatePageBreaks( nTab );

    if (bPaint)
        lcl_PaintWidthHeight( rDocShell, nTab, bColumns, nStart, nEnd );

    rDocShell.SetDocumentModified();
    lcl_InvalidateOutliner( rDocShell.GetViewBindings() );

    return sal_True;
}

//------------------------------------------------------------------------

sal_Bool ScOutlineDocFunc::ShowMarkedOutlines( const ScRange& rRange, sal_Bool bRecord )
{
    sal_Bool bDone = false;

    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nTab = rRange.aStart.Tab();

    ScDocument* pDoc = rDocShell.GetDocument();

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;
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
            pUndoDoc->InitUndo( pDoc, nTab, nTab, sal_True, sal_True );
            pDoc->CopyToDocument( nStartCol, 0, nTab, nEndCol, MAXROW, nTab, IDF_NONE, false, pUndoDoc );
            pDoc->CopyToDocument( 0, nStartRow, nTab, MAXCOL, nEndRow, nTab, IDF_NONE, false, pUndoDoc );

            rDocShell.GetUndoManager()->AddUndoAction(
                new ScUndoOutlineBlock( &rDocShell,
                                        nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab,
                                        pUndoDoc, pUndoTab, sal_True ) );
        }

        pDoc->InitializeNoteCaptions(nTab);

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
                pEntry->SetHidden( false );
                pEntry->SetVisible( sal_True );
                if (nStart<nMin) nMin=nStart;
                if (nEnd>nMax) nMax=nEnd;
            }
        }
        for ( i=nMin; i<=nMax; i++ )
            pDoc->ShowCol( static_cast<SCCOL>(i), nTab, sal_True );

        //  Zeilen

        nMin=MAXROW;
        nMax=0;
        pArray = pTable->GetRowArray();
        ScSubOutlineIterator aRowIter( pArray );
        pDoc->InitializeNoteCaptions(nTab);
        while ((pEntry=aRowIter.GetNext()) != NULL)
        {
            nStart = pEntry->GetStart();
            nEnd   = pEntry->GetEnd();
            if ( nStart>=nStartRow && nEnd<=nEndRow )
            {
                pEntry->SetHidden( false );
                pEntry->SetVisible( sal_True );
                if (nStart<nMin) nMin=nStart;
                if (nEnd>nMax) nMax=nEnd;
            }
        }
        for ( i=nMin; i<=nMax; i++ )
        {
            // show several rows together, don't show filtered rows
            SCROW nFilterEnd = i;
            bool bFiltered = pDoc->RowFiltered( i, nTab, NULL, &nFilterEnd );
            nFilterEnd = std::min( nMax, nFilterEnd );
            if ( !bFiltered )
                pDoc->ShowRows( i, nFilterEnd, nTab, sal_True );
            i = nFilterEnd;
        }

        pDoc->SetDrawPageSize(nTab);
        pDoc->UpdatePageBreaks( nTab );

        rDocShell.PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, PAINT_GRID | PAINT_LEFT | PAINT_TOP );
        rDocShell.SetDocumentModified();
        bDone = sal_True;

        lcl_InvalidateOutliner( rDocShell.GetViewBindings() );
    }

    return bDone;
}

sal_Bool ScOutlineDocFunc::HideMarkedOutlines( const ScRange& rRange, sal_Bool bRecord, sal_Bool bApi )
{
    sal_Bool bDone = false;

    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nTab = rRange.aStart.Tab();

    ScDocument* pDoc = rDocShell.GetDocument();

    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;
    ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );

    if (pTable)
    {
        const ScOutlineEntry* pEntry;
        size_t nColLevel;
        size_t nRowLevel;
        sal_uInt16 nCount;
        SCCOLROW nStart;
        SCCOLROW nEnd;
        sal_uInt16 i;

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
            pUndoDoc->InitUndo( pDoc, nTab, nTab, sal_True, sal_True );
            pDoc->CopyToDocument( static_cast<SCCOL>(nEffStartCol), 0, nTab,
                    static_cast<SCCOL>(nEffEndCol), MAXROW, nTab, IDF_NONE,
                    false, pUndoDoc );
            pDoc->CopyToDocument( 0, nEffStartRow, nTab, MAXCOL, nEffEndRow, nTab, IDF_NONE, false, pUndoDoc );

            rDocShell.GetUndoManager()->AddUndoAction(
                new ScUndoOutlineBlock( &rDocShell,
                                        nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab,
                                        pUndoDoc, pUndoTab, false ) );
        }

        pDoc->InitializeNoteCaptions(nTab);

        //  Spalten

        nCount = pColArray->GetCount(nColLevel);
        for ( i=0; i<nCount; i++ )
        {
            pEntry = pColArray->GetEntry(nColLevel,i);
            nStart = pEntry->GetStart();
            nEnd   = pEntry->GetEnd();

            if ( static_cast<SCCOLROW>(nStartCol)<=nEnd && static_cast<SCCOLROW>(nEndCol)>=nStart )
                HideOutline( nTab, sal_True, nColLevel, i, false, false, bApi );
        }

        //  Zeilen

        nCount = pRowArray->GetCount(nRowLevel);
        for ( i=0; i<nCount; i++ )
        {
            pEntry = pRowArray->GetEntry(nRowLevel,i);
            nStart = pEntry->GetStart();
            nEnd   = pEntry->GetEnd();

            if ( nStartRow<=nEnd && nEndRow>=nStart )
                HideOutline( nTab, false, nRowLevel, i, false, false, bApi );
        }

        pDoc->SetDrawPageSize(nTab);
        pDoc->UpdatePageBreaks( nTab );

        rDocShell.PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, PAINT_GRID | PAINT_LEFT | PAINT_TOP );

        rDocShell.SetDocumentModified();
        bDone = sal_True;

        lcl_InvalidateOutliner( rDocShell.GetViewBindings() );
    }

    return bDone;
}

//------------------------------------------------------------------------

sal_Bool ScOutlineDocFunc::ShowOutline( SCTAB nTab, sal_Bool bColumns, sal_uInt16 nLevel, sal_uInt16 nEntry,
                                    sal_Bool bRecord, sal_Bool bPaint, sal_Bool /* bApi */ )
{
    ScDocument* pDoc = rDocShell.GetDocument();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;

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
            pUndoDoc->InitUndo( pDoc, nTab, nTab, sal_True, false );
            pDoc->CopyToDocument( static_cast<SCCOL>(nStart), 0, nTab,
                    static_cast<SCCOL>(nEnd), MAXROW, nTab, IDF_NONE, false,
                    pUndoDoc );
        }
        else
        {
            pUndoDoc->InitUndo( pDoc, nTab, nTab, false, sal_True );
            pDoc->CopyToDocument( 0, nStart, nTab, MAXCOL, nEnd, nTab, IDF_NONE, false, pUndoDoc );
        }

        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoDoOutline( &rDocShell,
                                    nStart, nEnd, nTab, pUndoDoc,       //! start und end berechnen
                                    bColumns, nLevel, nEntry, sal_True ) );
    }

    pDoc->InitializeNoteCaptions(nTab);
    pEntry->SetHidden(false);
    SCCOLROW i;
    for ( i = nStart; i <= nEnd; i++ )
    {
        if ( bColumns )
            pDoc->ShowCol( static_cast<SCCOL>(i), nTab, sal_True );
        else
        {
            // show several rows together, don't show filtered rows
            SCROW nFilterEnd = i;
            bool bFiltered = pDoc->RowFiltered( i, nTab, NULL, &nFilterEnd );
            nFilterEnd = std::min( nEnd, nFilterEnd );
            if ( !bFiltered )
                pDoc->ShowRows( i, nFilterEnd, nTab, sal_True );
            i = nFilterEnd;
        }
    }

    ScSubOutlineIterator aIter( pArray, nLevel, nEntry );
    while ((pEntry=aIter.GetNext()) != NULL)
    {
        if ( pEntry->IsHidden() )
        {
            SCCOLROW nSubStart = pEntry->GetStart();
            SCCOLROW nSubEnd   = pEntry->GetEnd();
            if ( bColumns )
                for ( i = nSubStart; i <= nSubEnd; i++ )
                    pDoc->ShowCol( static_cast<SCCOL>(i), nTab, false );
            else
                pDoc->ShowRows( nSubStart, nSubEnd, nTab, sal_False );
        }
    }

    pArray->SetVisibleBelow( nLevel, nEntry, sal_True, sal_True );

    pDoc->SetDrawPageSize(nTab);
    pDoc->InvalidatePageBreaks(nTab);
    pDoc->UpdatePageBreaks( nTab );

    if (bPaint)
        lcl_PaintWidthHeight( rDocShell, nTab, bColumns, nStart, nEnd );

    rDocShell.SetDocumentModified();

    lcl_InvalidateOutliner( rDocShell.GetViewBindings() );

    return sal_True;        //! immer ???
}

sal_Bool ScOutlineDocFunc::HideOutline( SCTAB nTab, sal_Bool bColumns, sal_uInt16 nLevel, sal_uInt16 nEntry,
                                    sal_Bool bRecord, sal_Bool bPaint, sal_Bool /* bApi */ )
{
    ScDocument* pDoc = rDocShell.GetDocument();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;

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
            pUndoDoc->InitUndo( pDoc, nTab, nTab, sal_True, false );
            pDoc->CopyToDocument( static_cast<SCCOL>(nStart), 0, nTab,
                    static_cast<SCCOL>(nEnd), MAXROW, nTab, IDF_NONE, false,
                    pUndoDoc );
        }
        else
        {
            pUndoDoc->InitUndo( pDoc, nTab, nTab, false, sal_True );
            pDoc->CopyToDocument( 0, nStart, nTab, MAXCOL, nEnd, nTab, IDF_NONE, false, pUndoDoc );
        }

        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoDoOutline( &rDocShell,
                                    nStart, nEnd, nTab, pUndoDoc,
                                    bColumns, nLevel, nEntry, false ) );
    }

    pDoc->InitializeNoteCaptions(nTab);
    pEntry->SetHidden(true);
    SCCOLROW i;
    if ( bColumns )
        for ( i = nStart; i <= nEnd; i++ )
            pDoc->ShowCol( static_cast<SCCOL>(i), nTab, sal_False );
    else
        pDoc->ShowRows( nStart, nEnd, nTab, sal_False );

    pArray->SetVisibleBelow( nLevel, nEntry, false );

    pDoc->SetDrawPageSize(nTab);
    pDoc->InvalidatePageBreaks(nTab);
    pDoc->UpdatePageBreaks( nTab );

    if (bPaint)
        lcl_PaintWidthHeight( rDocShell, nTab, bColumns, nStart, nEnd );

    rDocShell.SetDocumentModified();

    lcl_InvalidateOutliner( rDocShell.GetViewBindings() );

    return sal_True;        //! immer ???
}





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
