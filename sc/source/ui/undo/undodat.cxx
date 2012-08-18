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

#include <sfx2/app.hxx>

#include "undodat.hxx"
#include "undoutil.hxx"
#include "undoolk.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "tabvwsh.hxx"
#include "olinetab.hxx"
#include "dbdata.hxx"
#include "rangenam.hxx"
#include "pivot.hxx"
#include "globstr.hrc"
#include "global.hxx"
#include "globalnames.hxx"
#include "target.hxx"
#include "chartarr.hxx"
#include "dbdocfun.hxx"
#include "olinefun.hxx"
#include "dpobject.hxx"
#include "attrib.hxx"
#include "hints.hxx"
#include "sc.hrc"
#include "chgtrack.hxx"
#include "refundo.hxx"
#include "markdata.hxx"

// -----------------------------------------------------------------------

TYPEINIT1(ScUndoDoOutline,          ScSimpleUndo);
TYPEINIT1(ScUndoMakeOutline,        ScSimpleUndo);
TYPEINIT1(ScUndoOutlineLevel,       ScSimpleUndo);
TYPEINIT1(ScUndoOutlineBlock,       ScSimpleUndo);
TYPEINIT1(ScUndoRemoveAllOutlines,  ScSimpleUndo);
TYPEINIT1(ScUndoAutoOutline,        ScSimpleUndo);
TYPEINIT1(ScUndoSubTotals,          ScDBFuncUndo);
TYPEINIT1(ScUndoSort,               ScDBFuncUndo);
TYPEINIT1(ScUndoQuery,              ScDBFuncUndo);
TYPEINIT1(ScUndoAutoFilter,         ScDBFuncUndo);
TYPEINIT1(ScUndoDBData,             ScSimpleUndo);
TYPEINIT1(ScUndoImportData,         ScSimpleUndo);
TYPEINIT1(ScUndoRepeatDB,           ScSimpleUndo);
TYPEINIT1(ScUndoDataPilot,          ScSimpleUndo);
TYPEINIT1(ScUndoConsolidate,        ScSimpleUndo);
TYPEINIT1(ScUndoChartData,          ScSimpleUndo);
TYPEINIT1(ScUndoDataForm,           SfxUndoAction);

// -----------------------------------------------------------------------


//
//      Outline-Gruppen ein- oder ausblenden
//

ScUndoDoOutline::ScUndoDoOutline( ScDocShell* pNewDocShell,
                            SCCOLROW nNewStart, SCCOLROW nNewEnd, SCTAB nNewTab,
                            ScDocument* pNewUndoDoc, sal_Bool bNewColumns,
                            sal_uInt16 nNewLevel, sal_uInt16 nNewEntry, sal_Bool bNewShow ) :
    ScSimpleUndo( pNewDocShell ),
    nStart( nNewStart ),
    nEnd( nNewEnd ),
    nTab( nNewTab ),
    pUndoDoc( pNewUndoDoc ),
    bColumns( bNewColumns ),
    nLevel( nNewLevel ),
    nEntry( nNewEntry ),
    bShow( bNewShow )
{
}

ScUndoDoOutline::~ScUndoDoOutline()
{
    delete pUndoDoc;
}

rtl::OUString ScUndoDoOutline::GetComment() const
{   // Detail einblenden" "Detail ausblenden"
    return bShow ?
        ScGlobal::GetRscString( STR_UNDO_DOOUTLINE ) :
        ScGlobal::GetRscString( STR_UNDO_REDOOUTLINE );
}

void ScUndoDoOutline::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    //  Tabelle muss vorher umgeschaltet sein (#46952#) !!!

    SCTAB nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

    //  inverse Funktion ausfuehren

    if (bShow)
        pViewShell->HideOutline( bColumns, nLevel, nEntry, false, false );
    else
        pViewShell->ShowOutline( bColumns, nLevel, nEntry, false, false );

    //  Original Spalten-/Zeilenstatus

    if (bColumns)
        pUndoDoc->CopyToDocument( static_cast<SCCOL>(nStart), 0, nTab,
                static_cast<SCCOL>(nEnd), MAXROW, nTab, IDF_NONE, false, pDoc);
    else
        pUndoDoc->CopyToDocument( 0, nStart, nTab, MAXCOL, nEnd, nTab, IDF_NONE, false, pDoc );

    pViewShell->UpdateScrollBars();

    pDocShell->PostPaint(0,0,nTab,MAXCOL,MAXROW,nTab,PAINT_GRID|PAINT_LEFT|PAINT_TOP);

    EndUndo();
}

void ScUndoDoOutline::Redo()
{
    BeginRedo();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    //  Tabelle muss vorher umgeschaltet sein (#46952#) !!!

    SCTAB nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

    if (bShow)
        pViewShell->ShowOutline( bColumns, nLevel, nEntry, false );
    else
        pViewShell->HideOutline( bColumns, nLevel, nEntry, false );

    EndRedo();
}

void ScUndoDoOutline::Repeat(SfxRepeatTarget& /* rTarget */)
{
}

sal_Bool ScUndoDoOutline::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;                       // geht nicht
}

//
//      Outline-Gruppen erzeugen oder loeschen
//

ScUndoMakeOutline::ScUndoMakeOutline( ScDocShell* pNewDocShell,
                            SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                            SCCOL nEndX, SCROW nEndY, SCTAB nEndZ,
                            ScOutlineTable* pNewUndoTab, sal_Bool bNewColumns, sal_Bool bNewMake ) :
    ScSimpleUndo( pNewDocShell ),
    aBlockStart( nStartX, nStartY, nStartZ ),
    aBlockEnd( nEndX, nEndY, nEndZ ),
    pUndoTable( pNewUndoTab ),
    bColumns( bNewColumns ),
    bMake( bNewMake )
{
}

ScUndoMakeOutline::~ScUndoMakeOutline()
{
    delete pUndoTable;
}

rtl::OUString ScUndoMakeOutline::GetComment() const
{   // "Gruppierung" "Gruppierung aufheben"
    return bMake ?
        ScGlobal::GetRscString( STR_UNDO_MAKEOUTLINE ) :
        ScGlobal::GetRscString( STR_UNDO_REMAKEOUTLINE );
}

void ScUndoMakeOutline::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    SCTAB nTab = aBlockStart.Tab();

    ScUndoUtil::MarkSimpleBlock( pDocShell, aBlockStart, aBlockEnd );

    pDoc->SetOutlineTable( nTab, pUndoTable );

    SCTAB nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

    pDocShell->PostPaint(0,0,nTab,MAXCOL,MAXROW,nTab,PAINT_GRID|PAINT_LEFT|PAINT_TOP|PAINT_SIZE);

    EndUndo();
}

void ScUndoMakeOutline::Redo()
{
    BeginRedo();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    ScUndoUtil::MarkSimpleBlock( pDocShell, aBlockStart, aBlockEnd );

    if (bMake)
        pViewShell->MakeOutline( bColumns, false );
    else
        pViewShell->RemoveOutline( bColumns, false );

    pDocShell->PostPaint(0,0,aBlockStart.Tab(),MAXCOL,MAXROW,aBlockEnd.Tab(),PAINT_GRID);

    EndRedo();
}

void ScUndoMakeOutline::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
    {
        ScTabViewShell& rViewShell = *((ScTabViewTarget&)rTarget).GetViewShell();

        if (bMake)
            rViewShell.MakeOutline( bColumns, sal_True );
        else
            rViewShell.RemoveOutline( bColumns, sal_True );
    }
}

sal_Bool ScUndoMakeOutline::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

//
//      Outline-Ebene auswaehlen
//

ScUndoOutlineLevel::ScUndoOutlineLevel( ScDocShell* pNewDocShell,
                        SCCOLROW nNewStart, SCCOLROW nNewEnd, SCTAB nNewTab,
                        ScDocument* pNewUndoDoc, ScOutlineTable* pNewUndoTab,
                        sal_Bool bNewColumns, sal_uInt16 nNewLevel ) :
    ScSimpleUndo( pNewDocShell ),
    nStart( nNewStart ),
    nEnd( nNewEnd ),
    nTab( nNewTab ),
    pUndoDoc( pNewUndoDoc ),
    pUndoTable( pNewUndoTab ),
    bColumns( bNewColumns ),
    nLevel( nNewLevel )
{
}

ScUndoOutlineLevel::~ScUndoOutlineLevel()
{
    delete pUndoDoc;
    delete pUndoTable;
}

rtl::OUString ScUndoOutlineLevel::GetComment() const
{   // "Gliederungsebene auswaehlen";
    return ScGlobal::GetRscString( STR_UNDO_OUTLINELEVEL );
}

void ScUndoOutlineLevel::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    //  Original Outline-Table

    pDoc->SetOutlineTable( nTab, pUndoTable );

    //  Original Spalten-/Zeilenstatus

    if (bColumns)
        pUndoDoc->CopyToDocument( static_cast<SCCOL>(nStart), 0, nTab,
                static_cast<SCCOL>(nEnd), MAXROW, nTab, IDF_NONE, false, pDoc);
    else
        pUndoDoc->CopyToDocument( 0, nStart, nTab, MAXCOL, nEnd, nTab, IDF_NONE, false, pDoc );

    pDoc->UpdatePageBreaks( nTab );

    pViewShell->UpdateScrollBars();

    SCTAB nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

    pDocShell->PostPaint(0,0,nTab,MAXCOL,MAXROW,nTab,PAINT_GRID|PAINT_LEFT|PAINT_TOP);

    EndUndo();
}

void ScUndoOutlineLevel::Redo()
{
    BeginRedo();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    //  Tabelle muss vorher umgeschaltet sein (#46952#) !!!

    SCTAB nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

    pViewShell->SelectLevel( bColumns, nLevel, false );

    EndRedo();
}

void ScUndoOutlineLevel::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->SelectLevel( bColumns, nLevel, sal_True );
}

sal_Bool ScUndoOutlineLevel::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

//
//      Outline ueber Blockmarken ein- oder ausblenden
//

ScUndoOutlineBlock::ScUndoOutlineBlock( ScDocShell* pNewDocShell,
                        SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                        SCCOL nEndX, SCROW nEndY, SCTAB nEndZ,
                        ScDocument* pNewUndoDoc, ScOutlineTable* pNewUndoTab, sal_Bool bNewShow ) :
    ScSimpleUndo( pNewDocShell ),
    aBlockStart( nStartX, nStartY, nStartZ ),
    aBlockEnd( nEndX, nEndY, nEndZ ),
    pUndoDoc( pNewUndoDoc ),
    pUndoTable( pNewUndoTab ),
    bShow( bNewShow )
{
}

ScUndoOutlineBlock::~ScUndoOutlineBlock()
{
    delete pUndoDoc;
    delete pUndoTable;
}

rtl::OUString ScUndoOutlineBlock::GetComment() const
{   // "Detail einblenden" "Detail ausblenden"
    return bShow ?
        ScGlobal::GetRscString( STR_UNDO_DOOUTLINEBLK ) :
        ScGlobal::GetRscString( STR_UNDO_REDOOUTLINEBLK );
}

void ScUndoOutlineBlock::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    SCTAB nTab = aBlockStart.Tab();

    //  Original Outline-Table

    pDoc->SetOutlineTable( nTab, pUndoTable );

    //  Original Spalten-/Zeilenstatus

    SCCOLROW    nStartCol = aBlockStart.Col();
    SCCOLROW    nEndCol = aBlockEnd.Col();
    SCCOLROW    nStartRow = aBlockStart.Row();
    SCCOLROW    nEndRow = aBlockEnd.Row();

    if (!bShow)
    {                               //  Groesse des ausgeblendeten Blocks
        size_t nLevel;
        pUndoTable->GetColArray()->FindTouchedLevel( nStartCol, nEndCol, nLevel );
        pUndoTable->GetColArray()->ExtendBlock( nLevel, nStartCol, nEndCol );
        pUndoTable->GetRowArray()->FindTouchedLevel( nStartRow, nEndRow, nLevel );
        pUndoTable->GetRowArray()->ExtendBlock( nLevel, nStartRow, nEndRow );
    }

    pUndoDoc->CopyToDocument( static_cast<SCCOL>(nStartCol), 0, nTab,
            static_cast<SCCOL>(nEndCol), MAXROW, nTab, IDF_NONE, false, pDoc );
    pUndoDoc->CopyToDocument( 0, nStartRow, nTab, MAXCOL, nEndRow, nTab, IDF_NONE, false, pDoc );

    pDoc->UpdatePageBreaks( nTab );

    pViewShell->UpdateScrollBars();

    SCTAB nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

    pDocShell->PostPaint(0,0,nTab,MAXCOL,MAXROW,nTab,PAINT_GRID|PAINT_LEFT|PAINT_TOP);

    EndUndo();
}

void ScUndoOutlineBlock::Redo()
{
    BeginRedo();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    ScUndoUtil::MarkSimpleBlock( pDocShell, aBlockStart, aBlockEnd );
    if (bShow)
        pViewShell->ShowMarkedOutlines( false );
    else
        pViewShell->HideMarkedOutlines( false );

    EndRedo();
}

void ScUndoOutlineBlock::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
    {
        ScTabViewShell& rViewShell = *((ScTabViewTarget&)rTarget).GetViewShell();

        if (bShow)
            rViewShell.ShowMarkedOutlines( sal_True );
        else
            rViewShell.HideMarkedOutlines( sal_True );
    }
}

sal_Bool ScUndoOutlineBlock::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

//
//      alle Outlines loeschen
//

ScUndoRemoveAllOutlines::ScUndoRemoveAllOutlines( ScDocShell* pNewDocShell,
                                    SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                                    SCCOL nEndX, SCROW nEndY, SCTAB nEndZ,
                                    ScDocument* pNewUndoDoc, ScOutlineTable* pNewUndoTab ) :
    ScSimpleUndo( pNewDocShell ),
    aBlockStart( nStartX, nStartY, nStartZ ),
    aBlockEnd( nEndX, nEndY, nEndZ ),
    pUndoDoc( pNewUndoDoc ),
    pUndoTable( pNewUndoTab )
{
}

ScUndoRemoveAllOutlines::~ScUndoRemoveAllOutlines()
{
    delete pUndoDoc;
    delete pUndoTable;
}

rtl::OUString ScUndoRemoveAllOutlines::GetComment() const
{   // "Gliederung entfernen"
    return ScGlobal::GetRscString( STR_UNDO_REMOVEALLOTLNS );
}

void ScUndoRemoveAllOutlines::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    SCTAB nTab = aBlockStart.Tab();

    //  Original Outline-Table

    pDoc->SetOutlineTable( nTab, pUndoTable );

    //  Original Spalten-/Zeilenstatus

    SCCOL   nStartCol = aBlockStart.Col();
    SCCOL   nEndCol = aBlockEnd.Col();
    SCROW   nStartRow = aBlockStart.Row();
    SCROW   nEndRow = aBlockEnd.Row();

    pUndoDoc->CopyToDocument( nStartCol, 0, nTab, nEndCol, MAXROW, nTab, IDF_NONE, false, pDoc );
    pUndoDoc->CopyToDocument( 0, nStartRow, nTab, MAXCOL, nEndRow, nTab, IDF_NONE, false, pDoc );

    pDoc->UpdatePageBreaks( nTab );

    pViewShell->UpdateScrollBars();

    SCTAB nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

    pDocShell->PostPaint(0,0,nTab,MAXCOL,MAXROW,nTab,PAINT_GRID|PAINT_LEFT|PAINT_TOP|PAINT_SIZE);

    EndUndo();
}

void ScUndoRemoveAllOutlines::Redo()
{
    BeginRedo();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    //  Tabelle muss vorher umgeschaltet sein (#46952#) !!!

    SCTAB nTab = aBlockStart.Tab();
    SCTAB nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

    pViewShell->RemoveAllOutlines( false );

    EndRedo();
}

void ScUndoRemoveAllOutlines::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->RemoveAllOutlines( sal_True );
}

sal_Bool ScUndoRemoveAllOutlines::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

//
//      Auto-Outline
//

ScUndoAutoOutline::ScUndoAutoOutline( ScDocShell* pNewDocShell,
                                    SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                                    SCCOL nEndX, SCROW nEndY, SCTAB nEndZ,
                                    ScDocument* pNewUndoDoc, ScOutlineTable* pNewUndoTab ) :
    ScSimpleUndo( pNewDocShell ),
    aBlockStart( nStartX, nStartY, nStartZ ),
    aBlockEnd( nEndX, nEndY, nEndZ ),
    pUndoDoc( pNewUndoDoc ),
    pUndoTable( pNewUndoTab )
{
}

ScUndoAutoOutline::~ScUndoAutoOutline()
{
    delete pUndoDoc;
    delete pUndoTable;
}

rtl::OUString ScUndoAutoOutline::GetComment() const
{   // "Auto-Gliederung"
    return ScGlobal::GetRscString( STR_UNDO_AUTOOUTLINE );
}

void ScUndoAutoOutline::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    SCTAB nTab = aBlockStart.Tab();

    //  Original Outline-Table

    pDoc->SetOutlineTable( nTab, pUndoTable );

    //  Original Spalten-/Zeilenstatus

    if (pUndoDoc && pUndoTable)
    {
        SCCOLROW nStartCol;
        SCCOLROW nStartRow;
        SCCOLROW nEndCol;
        SCCOLROW nEndRow;
        pUndoTable->GetColArray()->GetRange( nStartCol, nEndCol );
        pUndoTable->GetRowArray()->GetRange( nStartRow, nEndRow );

        pUndoDoc->CopyToDocument( static_cast<SCCOL>(nStartCol), 0, nTab,
                static_cast<SCCOL>(nEndCol), MAXROW, nTab, IDF_NONE, false,
                pDoc);
        pUndoDoc->CopyToDocument( 0, nStartRow, nTab, MAXCOL, nEndRow, nTab, IDF_NONE, false, pDoc );

        pViewShell->UpdateScrollBars();
    }

    SCTAB nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

    pDocShell->PostPaint(0,0,nTab,MAXCOL,MAXROW,nTab,PAINT_GRID|PAINT_LEFT|PAINT_TOP|PAINT_SIZE);

    EndUndo();
}

void ScUndoAutoOutline::Redo()
{
    BeginRedo();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    SCTAB nTab = aBlockStart.Tab();
    if (pViewShell)
    {
        //  Tabelle muss vorher umgeschaltet sein (#46952#) !!!

        SCTAB nVisTab = pViewShell->GetViewData()->GetTabNo();
        if ( nVisTab != nTab )
            pViewShell->SetTabNo( nTab );
    }

    ScRange aRange( aBlockStart.Col(), aBlockStart.Row(), nTab,
                    aBlockEnd.Col(),   aBlockEnd.Row(),   nTab );
    ScOutlineDocFunc aFunc( *pDocShell );
    aFunc.AutoOutline( aRange, false, false );

    //  auf der View markieren
    //  Wenn's beim Aufruf eine Mehrfachselektion war, ist es jetzt der
    //  umschliessende Bereich...

    if (pViewShell)
        pViewShell->MarkRange( aRange );

    EndRedo();
}

void ScUndoAutoOutline::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->AutoOutline( sal_True );
}

sal_Bool ScUndoAutoOutline::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

//
//      Zwischenergebnisse
//

ScUndoSubTotals::ScUndoSubTotals( ScDocShell* pNewDocShell, SCTAB nNewTab,
                                const ScSubTotalParam& rNewParam, SCROW nNewEndY,
                                ScDocument* pNewUndoDoc, ScOutlineTable* pNewUndoTab,
                                ScRangeName* pNewUndoRange, ScDBCollection* pNewUndoDB ) :
    ScDBFuncUndo( pNewDocShell, ScRange( rNewParam.nCol1, rNewParam.nRow1, nNewTab,
                                         rNewParam.nCol2, rNewParam.nRow2, nNewTab ) ),
    nTab( nNewTab ),
    aParam( rNewParam ),
    nNewEndRow( nNewEndY ),
    pUndoDoc( pNewUndoDoc ),
    pUndoTable( pNewUndoTab ),
    pUndoRange( pNewUndoRange ),
    pUndoDB( pNewUndoDB )
{
}

ScUndoSubTotals::~ScUndoSubTotals()
{
    delete pUndoDoc;
    delete pUndoTable;
    delete pUndoRange;
    delete pUndoDB;
}

rtl::OUString ScUndoSubTotals::GetComment() const
{   // "Teilergebnisse"
    return ScGlobal::GetRscString( STR_UNDO_SUBTOTALS );
}

void ScUndoSubTotals::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    if (nNewEndRow > aParam.nRow2)
    {
        pDoc->DeleteRow( 0,nTab, MAXCOL,nTab, aParam.nRow2+1, static_cast<SCSIZE>(nNewEndRow-aParam.nRow2) );
    }
    else if (nNewEndRow < aParam.nRow2)
    {
        pDoc->InsertRow( 0,nTab, MAXCOL,nTab, nNewEndRow+1, static_cast<SCSIZE>(aParam.nRow2-nNewEndRow) );
    }


    //  Original Outline-Table

    pDoc->SetOutlineTable( nTab, pUndoTable );

    //  Original Spalten-/Zeilenstatus

    if (pUndoDoc && pUndoTable)
    {
        SCCOLROW nStartCol;
        SCCOLROW nStartRow;
        SCCOLROW nEndCol;
        SCCOLROW nEndRow;
        pUndoTable->GetColArray()->GetRange( nStartCol, nEndCol );
        pUndoTable->GetRowArray()->GetRange( nStartRow, nEndRow );

        pUndoDoc->CopyToDocument( static_cast<SCCOL>(nStartCol), 0, nTab,
                static_cast<SCCOL>(nEndCol), MAXROW, nTab, IDF_NONE, false,
                pDoc);
        pUndoDoc->CopyToDocument( 0, nStartRow, nTab, MAXCOL, nEndRow, nTab, IDF_NONE, false, pDoc );

        pViewShell->UpdateScrollBars();
    }

    //  Original-Daten & Referenzen

    ScUndoUtil::MarkSimpleBlock( pDocShell, 0, aParam.nRow1+1, nTab,
                                            MAXCOL, aParam.nRow2, nTab );

    pDoc->DeleteAreaTab( 0,aParam.nRow1+1, MAXCOL,aParam.nRow2, nTab, IDF_ALL );

    pUndoDoc->CopyToDocument( 0, aParam.nRow1+1, nTab, MAXCOL, aParam.nRow2, nTab,
                                                            IDF_NONE, false, pDoc );    // Flags
    pUndoDoc->UndoToDocument( 0, aParam.nRow1+1, nTab, MAXCOL, aParam.nRow2, nTab,
                                                            IDF_ALL, false, pDoc );

    ScUndoUtil::MarkSimpleBlock( pDocShell, aParam.nCol1,aParam.nRow1,nTab,
                                            aParam.nCol2,aParam.nRow2,nTab );

    if (pUndoRange)
        pDoc->SetRangeName( new ScRangeName( *pUndoRange ) );
    if (pUndoDB)
        pDoc->SetDBCollection( new ScDBCollection( *pUndoDB ), sal_True );

    SCTAB nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

    pDocShell->PostPaint(0,0,nTab,MAXCOL,MAXROW,nTab,PAINT_GRID|PAINT_LEFT|PAINT_TOP|PAINT_SIZE);
    pDocShell->PostDataChanged();

    EndUndo();
}

void ScUndoSubTotals::Redo()
{
    BeginRedo();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    SCTAB nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

    ScUndoUtil::MarkSimpleBlock( pDocShell, aParam.nCol1,aParam.nRow1,nTab,
                                            aParam.nCol2,aParam.nRow2,nTab );
    pViewShell->DoSubTotals( aParam, false );

    EndRedo();
}

void ScUndoSubTotals::Repeat(SfxRepeatTarget& /* rTarget */)
{
}

sal_Bool ScUndoSubTotals::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;                       // geht nicht wegen Spaltennummern
}

//
//      Sortieren
//

ScUndoSort::ScUndoSort( ScDocShell* pNewDocShell,
                        SCTAB nNewTab, const ScSortParam& rParam,
                        sal_Bool bQuery, ScDocument* pNewUndoDoc, ScDBCollection* pNewUndoDB,
                        const ScRange* pDest ) :
    ScDBFuncUndo( pNewDocShell, ScRange( rParam.nCol1, rParam.nRow1, nNewTab,
                                         rParam.nCol2, rParam.nRow2, nNewTab ) ),
    nTab( nNewTab ),
    aSortParam( rParam ),
    bRepeatQuery( bQuery ),
    pUndoDoc( pNewUndoDoc ),
    pUndoDB( pNewUndoDB ),
    bDestArea( false )
{
    if ( pDest )
    {
        bDestArea = sal_True;
        aDestRange = *pDest;
    }
}

ScUndoSort::~ScUndoSort()
{
    delete pUndoDoc;
    delete pUndoDB;
}

rtl::OUString ScUndoSort::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_SORT );
}

void ScUndoSort::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    SCCOL nStartCol = aSortParam.nCol1;
    SCROW nStartRow = aSortParam.nRow1;
    SCCOL nEndCol    = aSortParam.nCol2;
    SCROW nEndRow    = aSortParam.nRow2;
    SCTAB nSortTab  = nTab;
    if ( !aSortParam.bInplace )
    {
        nStartCol = aSortParam.nDestCol;
        nStartRow = aSortParam.nDestRow;
        nEndCol   = nStartCol + ( aSortParam.nCol2 - aSortParam.nCol1 );
        nEndRow   = nStartRow + ( aSortParam.nRow2 - aSortParam.nRow1 );
        nSortTab  = aSortParam.nDestTab;
    }

    ScUndoUtil::MarkSimpleBlock( pDocShell, nStartCol, nStartRow, nSortTab,
                                 nEndCol, nEndRow, nSortTab );

    // do not delete/copy note captions, they are handled in drawing undo (ScDBFuncUndo::mpDrawUndo)
    pDoc->DeleteAreaTab( nStartCol,nStartRow, nEndCol,nEndRow, nSortTab, IDF_ALL|IDF_NOCAPTIONS );
    pUndoDoc->CopyToDocument( nStartCol, nStartRow, nSortTab, nEndCol, nEndRow, nSortTab,
                                IDF_ALL|IDF_NOCAPTIONS, false, pDoc );

    if (bDestArea)
    {
        // do not delete/copy note captions, they are handled in drawing undo (ScDBFuncUndo::mpDrawUndo)
        pDoc->DeleteAreaTab( aDestRange, IDF_ALL|IDF_NOCAPTIONS );
        pUndoDoc->CopyToDocument( aDestRange, IDF_ALL|IDF_NOCAPTIONS, false, pDoc );
    }

    //  Zeilenhoehen immer (wegen automatischer Anpassung)
    //! auf ScBlockUndo umstellen
//  if (bRepeatQuery)
        pUndoDoc->CopyToDocument( 0, nStartRow, nSortTab, MAXCOL, nEndRow, nSortTab,
                                IDF_NONE, false, pDoc );

    if (pUndoDB)
        pDoc->SetDBCollection( new ScDBCollection( *pUndoDB ), sal_True );

    SCTAB nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nSortTab )
        pViewShell->SetTabNo( nSortTab );

    pDocShell->PostPaint(0,0,nTab,MAXCOL,MAXROW,nTab,PAINT_GRID|PAINT_LEFT|PAINT_TOP|PAINT_SIZE);
    pDocShell->PostDataChanged();

    EndUndo();
}

void ScUndoSort::Redo()
{
    BeginRedo();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    SCTAB nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

    pViewShell->MarkRange( ScRange( aSortParam.nCol1, aSortParam.nRow1, nTab,
                                      aSortParam.nCol2, aSortParam.nRow2, nTab ) );

    pViewShell->Sort( aSortParam, false );

    //  Quellbereich painten wegen Markierung
    if ( !aSortParam.bInplace )
        pDocShell->PostPaint( aSortParam.nCol1, aSortParam.nRow1, nTab,
                              aSortParam.nCol2, aSortParam.nRow2, nTab, PAINT_GRID );

    EndRedo();
}

void ScUndoSort::Repeat(SfxRepeatTarget& /* rTarget */)
{
}

sal_Bool ScUndoSort::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;                       // geht nicht wegen Spaltennummern
}

//
//      Filtern
//

ScUndoQuery::ScUndoQuery( ScDocShell* pNewDocShell, SCTAB nNewTab, const ScQueryParam& rParam,
                            ScDocument* pNewUndoDoc, ScDBCollection* pNewUndoDB,
                            const ScRange* pOld, sal_Bool bSize, const ScRange* pAdvSrc ) :
    ScDBFuncUndo( pNewDocShell, ScRange( rParam.nCol1, rParam.nRow1, nNewTab,
                                         rParam.nCol2, rParam.nRow2, nNewTab ) ),
    pDrawUndo( NULL ),
    nTab( nNewTab ),
    aQueryParam( rParam ),
    pUndoDoc( pNewUndoDoc ),
    pUndoDB( pNewUndoDB ),
    bIsAdvanced( false ),
    bDestArea( false ),
    bDoSize( bSize )
{
    if ( pOld )
    {
        bDestArea = sal_True;
        aOldDest = *pOld;
    }
    if ( pAdvSrc )
    {
        bIsAdvanced = sal_True;
        aAdvSource = *pAdvSrc;
    }

    pDrawUndo = GetSdrUndoAction( pDocShell->GetDocument() );
}

ScUndoQuery::~ScUndoQuery()
{
    delete pUndoDoc;
    delete pUndoDB;
    DeleteSdrUndoAction( pDrawUndo );
}

rtl::OUString ScUndoQuery::GetComment() const
{   // "Filtern";
    return ScGlobal::GetRscString( STR_UNDO_QUERY );
}

void ScUndoQuery::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    sal_Bool bCopy = !aQueryParam.bInplace;
    SCCOL nDestEndCol = 0;
    SCROW nDestEndRow = 0;
    if (bCopy)
    {
        nDestEndCol = aQueryParam.nDestCol + ( aQueryParam.nCol2-aQueryParam.nCol1 );
        nDestEndRow = aQueryParam.nDestRow + ( aQueryParam.nRow2-aQueryParam.nRow1 );

        ScDBData* pData = pDoc->GetDBAtCursor( aQueryParam.nDestCol, aQueryParam.nDestRow,
                                                aQueryParam.nDestTab, sal_True );
        if (pData)
        {
            ScRange aNewDest;
            pData->GetArea( aNewDest );
            nDestEndCol = aNewDest.aEnd.Col();
            nDestEndRow = aNewDest.aEnd.Row();
        }

        if ( bDoSize && bDestArea )
        {
            //  aDestRange ist der alte Bereich
            pDoc->FitBlock( ScRange(
                                aQueryParam.nDestCol, aQueryParam.nDestRow, aQueryParam.nDestTab,
                                nDestEndCol, nDestEndRow, aQueryParam.nDestTab ),
                            aOldDest );
        }

        ScUndoUtil::MarkSimpleBlock( pDocShell,
                                    aQueryParam.nDestCol, aQueryParam.nDestRow, aQueryParam.nDestTab,
                                    nDestEndCol, nDestEndRow, aQueryParam.nDestTab );
        pDoc->DeleteAreaTab( aQueryParam.nDestCol, aQueryParam.nDestRow,
                            nDestEndCol, nDestEndRow, aQueryParam.nDestTab, IDF_ALL );

        pViewShell->DoneBlockMode();

        pUndoDoc->CopyToDocument( aQueryParam.nDestCol, aQueryParam.nDestRow, aQueryParam.nDestTab,
                                    nDestEndCol, nDestEndRow, aQueryParam.nDestTab,
                                    IDF_ALL, false, pDoc );
        //  Attribute werden immer mitkopiert (#49287#)

        //  Rest von altem Bereich
        if ( bDestArea && !bDoSize )
        {
            pDoc->DeleteAreaTab( aOldDest, IDF_ALL );
            pUndoDoc->CopyToDocument( aOldDest, IDF_ALL, false, pDoc );
        }
    }
    else
        pUndoDoc->CopyToDocument( 0, aQueryParam.nRow1, nTab, MAXCOL, aQueryParam.nRow2, nTab,
                                        IDF_NONE, false, pDoc );

    if (pUndoDB)
        pDoc->SetDBCollection( new ScDBCollection( *pUndoDB ), sal_True );

    if (!bCopy)
    {
        pDoc->InvalidatePageBreaks(nTab);
        pDoc->UpdatePageBreaks( nTab );
    }

    ScRange aDirtyRange( 0 , aQueryParam.nRow1, nTab,
        MAXCOL, aQueryParam.nRow2, nTab );
    pDoc->SetDirty( aDirtyRange );

    DoSdrUndoAction( pDrawUndo, pDoc );

    SCTAB nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

        //  Paint

    if (bCopy)
    {
        SCCOL nEndX = nDestEndCol;
        SCROW nEndY = nDestEndRow;
        if (bDestArea)
        {
            if ( aOldDest.aEnd.Col() > nEndX )
                nEndX = aOldDest.aEnd.Col();
            if ( aOldDest.aEnd.Row() > nEndY )
                nEndY = aOldDest.aEnd.Row();
        }
        if (bDoSize)
            nEndY = MAXROW;
        pDocShell->PostPaint( aQueryParam.nDestCol, aQueryParam.nDestRow, aQueryParam.nDestTab,
                                    nEndX, nEndY, aQueryParam.nDestTab, PAINT_GRID );
    }
    else
        pDocShell->PostPaint( 0, aQueryParam.nRow1, nTab, MAXCOL, MAXROW, nTab,
                                                    PAINT_GRID | PAINT_LEFT );
    pDocShell->PostDataChanged();

    EndUndo();
}

void ScUndoQuery::Redo()
{
    BeginRedo();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    SCTAB nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

    if ( bIsAdvanced )
        pViewShell->Query( aQueryParam, &aAdvSource, false );
    else
        pViewShell->Query( aQueryParam, NULL, false );

    EndRedo();
}

void ScUndoQuery::Repeat(SfxRepeatTarget& /* rTarget */)
{
}

sal_Bool ScUndoQuery::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;                       // geht nicht wegen Spaltennummern
}

//
//      Show or hide AutoFilter buttons (doesn't include filter settings)
//

ScUndoAutoFilter::ScUndoAutoFilter( ScDocShell* pNewDocShell, const ScRange& rRange,
                                    const ::rtl::OUString& rName, bool bSet ) :
    ScDBFuncUndo( pNewDocShell, rRange ),
    aDBName( rName ),
    bFilterSet( bSet )
{
}

ScUndoAutoFilter::~ScUndoAutoFilter()
{
}

rtl::OUString ScUndoAutoFilter::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_QUERY );    // same as ScUndoQuery
}

void ScUndoAutoFilter::DoChange( bool bUndo )
{
    bool bNewFilter = bUndo ? !bFilterSet : bFilterSet;

    ScDocument* pDoc = pDocShell->GetDocument();
    ScDBData* pDBData=NULL;
    if (aDBName.equalsAscii(STR_DB_LOCAL_NONAME))
    {
        SCTAB nTab = aOriginalRange.aStart.Tab();
        pDBData = pDoc->GetAnonymousDBData(nTab);
    }
    else
    {
        ScDBCollection* pColl = pDoc->GetDBCollection();
        pDBData = pColl->getNamedDBs().findByName(aDBName);
    }

    if ( pDBData )
    {
        pDBData->SetAutoFilter( bNewFilter );

        SCCOL nRangeX1;
        SCROW nRangeY1;
        SCCOL nRangeX2;
        SCROW nRangeY2;
        SCTAB nRangeTab;
        pDBData->GetArea( nRangeTab, nRangeX1, nRangeY1, nRangeX2, nRangeY2 );

        if ( bNewFilter )
            pDoc->ApplyFlagsTab( nRangeX1, nRangeY1, nRangeX2, nRangeY1, nRangeTab, SC_MF_AUTO );
        else
            pDoc->RemoveFlagsTab( nRangeX1, nRangeY1, nRangeX2, nRangeY1, nRangeTab, SC_MF_AUTO );

        pDocShell->PostPaint( nRangeX1, nRangeY1, nRangeTab, nRangeX2, nRangeY1, nRangeTab, PAINT_GRID );
    }
}

void ScUndoAutoFilter::Undo()
{
    BeginUndo();
    DoChange( sal_True );
    EndUndo();
}

void ScUndoAutoFilter::Redo()
{
    BeginRedo();
    DoChange( false );
    EndRedo();
}

void ScUndoAutoFilter::Repeat(SfxRepeatTarget& /* rTarget */)
{
}

sal_Bool ScUndoAutoFilter::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;
}

//
//      Datenbankbereiche aendern (Dialog)
//

ScUndoDBData::ScUndoDBData( ScDocShell* pNewDocShell,
                            ScDBCollection* pNewUndoColl, ScDBCollection* pNewRedoColl ) :
    ScSimpleUndo( pNewDocShell ),
    pUndoColl( pNewUndoColl ),
    pRedoColl( pNewRedoColl )
{
}

ScUndoDBData::~ScUndoDBData()
{
    delete pUndoColl;
    delete pRedoColl;
}

rtl::OUString ScUndoDBData::GetComment() const
{   // "Datenbankbereiche aendern";
    return ScGlobal::GetRscString( STR_UNDO_DBDATA );
}

void ScUndoDBData::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();

    sal_Bool bOldAutoCalc = pDoc->GetAutoCalc();
    pDoc->SetAutoCalc( false );         // unnoetige Berechnungen vermeiden
    pDoc->CompileDBFormula( sal_True );     // CreateFormulaString
    pDoc->SetDBCollection( new ScDBCollection(*pUndoColl), sal_True );
    pDoc->CompileDBFormula( false );    // CompileFormulaString
    pDoc->SetAutoCalc( bOldAutoCalc );

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_DBAREAS_CHANGED ) );

    EndUndo();
}

void ScUndoDBData::Redo()
{
    BeginRedo();

    ScDocument* pDoc = pDocShell->GetDocument();

    sal_Bool bOldAutoCalc = pDoc->GetAutoCalc();
    pDoc->SetAutoCalc( false );         // unnoetige Berechnungen vermeiden
    pDoc->CompileDBFormula( sal_True );     // CreateFormulaString
    pDoc->SetDBCollection( new ScDBCollection(*pRedoColl), sal_True );
    pDoc->CompileDBFormula( false );    // CompileFormulaString
    pDoc->SetAutoCalc( bOldAutoCalc );

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_DBAREAS_CHANGED ) );

    EndRedo();
}

void ScUndoDBData::Repeat(SfxRepeatTarget& /* rTarget */)
{
}

sal_Bool ScUndoDBData::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;                       // geht nicht
}

//
//      Import
//

ScUndoImportData::ScUndoImportData( ScDocShell* pNewDocShell, SCTAB nNewTab,
                                const ScImportParam& rParam, SCCOL nNewEndX, SCROW nNewEndY,
                                SCCOL nNewFormula,
                                ScDocument* pNewUndoDoc, ScDocument* pNewRedoDoc,
                                ScDBData* pNewUndoData, ScDBData* pNewRedoData ) :
    ScSimpleUndo( pNewDocShell ),
    nTab( nNewTab ),
    aImportParam( rParam ),
    nEndCol( nNewEndX ),
    nEndRow( nNewEndY ),
    pUndoDoc( pNewUndoDoc ),
    pRedoDoc( pNewRedoDoc ),
    pUndoDBData( pNewUndoData ),
    pRedoDBData( pNewRedoData ),
    nFormulaCols( nNewFormula ),
    bRedoFilled( false )
{
    // redo doc doesn't contain imported data (but everything else)
}

ScUndoImportData::~ScUndoImportData()
{
    delete pUndoDoc;
    delete pRedoDoc;
    delete pUndoDBData;
    delete pRedoDBData;
}

rtl::OUString ScUndoImportData::GetComment() const
{   // "Importieren";
    return ScGlobal::GetRscString( STR_UNDO_IMPORTDATA );
}

void ScUndoImportData::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    ScUndoUtil::MarkSimpleBlock( pDocShell, aImportParam.nCol1,aImportParam.nRow1,nTab,
                                                        nEndCol,nEndRow,nTab );

    SCTAB nTable;
    SCCOL nCol1, nCol2;
    SCROW nRow1, nRow2;
    ScDBData* pCurrentData = NULL;
    if (pUndoDBData && pRedoDBData)
    {
        pRedoDBData->GetArea( nTable, nCol1, nRow1, nCol2, nRow2 );
        pCurrentData = ScUndoUtil::GetOldDBData( pRedoDBData, pDoc, nTab,
                                                    nCol1, nRow1, nCol2, nRow2 );

        if ( !bRedoFilled )
        {
            //  read redo data from document at first undo
            //  imported data is deleted later anyway,
            //  so now delete each column after copying to save memory (#41216#)

            sal_Bool bOldAutoCalc = pDoc->GetAutoCalc();
            pDoc->SetAutoCalc( false );             // outside of the loop
            for (SCCOL nCopyCol = nCol1; nCopyCol <= nCol2; nCopyCol++)
            {
                pDoc->CopyToDocument( nCopyCol,nRow1,nTab, nCopyCol,nRow2,nTab,
                                        IDF_CONTENTS & ~IDF_NOTE, false, pRedoDoc );
                pDoc->DeleteAreaTab( nCopyCol,nRow1, nCopyCol,nRow2, nTab, IDF_CONTENTS & ~IDF_NOTE );
                pDoc->DoColResize( nTab, nCopyCol, nCopyCol, 0 );
            }
            pDoc->SetAutoCalc( bOldAutoCalc );
            bRedoFilled = sal_True;
        }
    }
    sal_Bool bMoveCells = pUndoDBData && pRedoDBData &&
                        pRedoDBData->IsDoSize();        // in alt und neu gleich
    if (bMoveCells)
    {
        //  Undo: erst die neuen Daten loeschen, dann FitBlock rueckwaerts

        ScRange aOld, aNew;
        pUndoDBData->GetArea( aOld );
        pRedoDBData->GetArea( aNew );

        pDoc->DeleteAreaTab( aNew.aStart.Col(), aNew.aStart.Row(),
                                aNew.aEnd.Col(), aNew.aEnd.Row(), nTab, IDF_ALL & ~IDF_NOTE );

        aOld.aEnd.SetCol( aOld.aEnd.Col() + nFormulaCols );     // FitBlock auch fuer Formeln
        aNew.aEnd.SetCol( aNew.aEnd.Col() + nFormulaCols );
        pDoc->FitBlock( aNew, aOld, false );                    // rueckwaerts
    }
    else
        pDoc->DeleteAreaTab( aImportParam.nCol1,aImportParam.nRow1,
                                nEndCol,nEndRow, nTab, IDF_ALL & ~IDF_NOTE );

    pUndoDoc->CopyToDocument( aImportParam.nCol1,aImportParam.nRow1,nTab,
                                nEndCol+nFormulaCols,nEndRow,nTab,
                                IDF_ALL & ~IDF_NOTE, false, pDoc );

    if (pCurrentData)
    {
        *pCurrentData = *pUndoDBData;

        pUndoDBData->GetArea( nTable, nCol1, nRow1, nCol2, nRow2 );
        ScUndoUtil::MarkSimpleBlock( pDocShell, nCol1, nRow1, nTable, nCol2, nRow2, nTable );
    }

// erack! it's broadcasted
//  pDoc->SetDirty();

    SCTAB nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

    if (bMoveCells)
        pDocShell->PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, PAINT_GRID );
    else
        pDocShell->PostPaint( aImportParam.nCol1,aImportParam.nRow1,nTab,
                                nEndCol,nEndRow,nTab, PAINT_GRID );
    pDocShell->PostDataChanged();

    EndUndo();
}

void ScUndoImportData::Redo()
{
    BeginRedo();

    ScDocument* pDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    ScUndoUtil::MarkSimpleBlock( pDocShell, aImportParam.nCol1,aImportParam.nRow1,nTab,
                                                        nEndCol,nEndRow,nTab );

    SCTAB nTable;
    SCCOL nCol1, nCol2;
    SCROW nRow1, nRow2;
    ScDBData* pCurrentData = NULL;
    if (pUndoDBData && pRedoDBData)
    {
        pUndoDBData->GetArea( nTable, nCol1, nRow1, nCol2, nRow2 );
        pCurrentData = ScUndoUtil::GetOldDBData( pUndoDBData, pDoc, nTab,
                                                    nCol1, nRow1, nCol2, nRow2 );
    }
    sal_Bool bMoveCells = pUndoDBData && pRedoDBData &&
                        pRedoDBData->IsDoSize();        // in alt und neu gleich
    if (bMoveCells)
    {
        //  Redo: FitBlock, dann Daten loeschen (noetig fuer CopyToDocument)

        ScRange aOld, aNew;
        pUndoDBData->GetArea( aOld );
        pRedoDBData->GetArea( aNew );

        aOld.aEnd.SetCol( aOld.aEnd.Col() + nFormulaCols );     // FitBlock auch fuer Formeln
        aNew.aEnd.SetCol( aNew.aEnd.Col() + nFormulaCols );
        pDoc->FitBlock( aOld, aNew );

        pDoc->DeleteAreaTab( aNew.aStart.Col(), aNew.aStart.Row(),
                                aNew.aEnd.Col(), aNew.aEnd.Row(), nTab, IDF_ALL & ~IDF_NOTE );

        pRedoDoc->CopyToDocument( aNew, IDF_ALL & ~IDF_NOTE, false, pDoc );        // incl. Formeln
    }
    else
    {
        pDoc->DeleteAreaTab( aImportParam.nCol1,aImportParam.nRow1,
                                nEndCol,nEndRow, nTab, IDF_ALL & ~IDF_NOTE );
        pRedoDoc->CopyToDocument( aImportParam.nCol1,aImportParam.nRow1,nTab,
                                nEndCol,nEndRow,nTab, IDF_ALL & ~IDF_NOTE, false, pDoc );
    }

    if (pCurrentData)
    {
        *pCurrentData = *pRedoDBData;

        pRedoDBData->GetArea( nTable, nCol1, nRow1, nCol2, nRow2 );
        ScUndoUtil::MarkSimpleBlock( pDocShell, nCol1, nRow1, nTable, nCol2, nRow2, nTable );
    }

// erack! it's broadcasted
//  pDoc->SetDirty();

    SCTAB nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

    if (bMoveCells)
        pDocShell->PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, PAINT_GRID );
    else
        pDocShell->PostPaint( aImportParam.nCol1,aImportParam.nRow1,nTab,
                                nEndCol,nEndRow,nTab, PAINT_GRID );
    pDocShell->PostDataChanged();

    EndRedo();
}

void ScUndoImportData::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
    {
        ScTabViewShell& rViewShell = *((ScTabViewTarget&)rTarget).GetViewShell();

        SCTAB nDummy;
        ScImportParam aNewParam(aImportParam);
        ScDBData* pDBData = rViewShell.GetDBData();
        pDBData->GetArea( nDummy, aNewParam.nCol1,aNewParam.nRow1, aNewParam.nCol2,aNewParam.nRow2 );

        rViewShell.ImportData( aNewParam );
    }
}

sal_Bool ScUndoImportData::CanRepeat(SfxRepeatTarget& rTarget) const
{
    //  Repeat nur fuer Import per DB-Bereich, dann ist pUndoDBData gesetzt

    if (pUndoDBData)
        return (rTarget.ISA(ScTabViewTarget));
    else
        return false;       // Adressbuch
}

//
//      Operationen wiederholen
//

ScUndoRepeatDB::ScUndoRepeatDB( ScDocShell* pNewDocShell, SCTAB nNewTab,
                                SCCOL nStartX, SCROW nStartY, SCCOL nEndX, SCROW nEndY,
                                SCROW nResultEndRow, SCCOL nCurX, SCROW nCurY,
                                ScDocument* pNewUndoDoc, ScOutlineTable* pNewUndoTab,
                                ScRangeName* pNewUndoRange, ScDBCollection* pNewUndoDB,
                                const ScRange* pOldQ, const ScRange* pNewQ ) :
    ScSimpleUndo( pNewDocShell ),
    aBlockStart( nStartX,nStartY,nNewTab ),
    aBlockEnd( nEndX,nEndY,nNewTab ),
    nNewEndRow( nResultEndRow ),
    aCursorPos( nCurX,nCurY,nNewTab ),
    pUndoDoc( pNewUndoDoc ),
    pUndoTable( pNewUndoTab ),
    pUndoRange( pNewUndoRange ),
    pUndoDB( pNewUndoDB ),
    bQuerySize( false )
{
    if ( pOldQ && pNewQ )
    {
        aOldQuery = *pOldQ;
        aNewQuery = *pNewQ;
        bQuerySize = sal_True;;
    }
}

ScUndoRepeatDB::~ScUndoRepeatDB()
{
    delete pUndoDoc;
    delete pUndoTable;
    delete pUndoRange;
    delete pUndoDB;
}

rtl::OUString ScUndoRepeatDB::GetComment() const
{   // "Wiederholen";       //! bessere Beschreibung!
    return ScGlobal::GetRscString( STR_UNDO_REPEATDB );
}

void ScUndoRepeatDB::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    SCTAB nTab = aBlockStart.Tab();

    if (bQuerySize)
    {
        pDoc->FitBlock( aNewQuery, aOldQuery, false );

        if ( aNewQuery.aEnd.Col() == aOldQuery.aEnd.Col() )
        {
            SCCOL nFormulaCols = 0;
            SCCOL nCol = aOldQuery.aEnd.Col() + 1;
            SCROW nRow = aOldQuery.aStart.Row() + 1;        //! Header testen
            while ( nCol <= MAXCOL &&
                    pDoc->GetCellType(ScAddress( nCol, nRow, nTab )) == CELLTYPE_FORMULA )
                ++nCol, ++nFormulaCols;

            if ( nFormulaCols > 0 )
            {
                ScRange aOldForm = aOldQuery;
                aOldForm.aStart.SetCol( aOldQuery.aEnd.Col() + 1 );
                aOldForm.aEnd.SetCol( aOldQuery.aEnd.Col() + nFormulaCols );
                ScRange aNewForm = aOldForm;
                aNewForm.aEnd.SetRow( aNewQuery.aEnd.Row() );
                pDoc->FitBlock( aNewForm, aOldForm, false );
            }
        }
    }

    //!     Daten von Filter in anderen Bereich fehlen noch !!!!!!!!!!!!!!!!!

    if (nNewEndRow > aBlockEnd.Row())
    {
        pDoc->DeleteRow( 0,nTab, MAXCOL,nTab, aBlockEnd.Row()+1, static_cast<SCSIZE>(nNewEndRow-aBlockEnd.Row()) );
    }
    else if (nNewEndRow < aBlockEnd.Row())
    {
        pDoc->InsertRow( 0,nTab, MAXCOL,nTab, nNewEndRow+1, static_cast<SCSIZE>(nNewEndRow-aBlockEnd.Row()) );
    }

    //  Original Outline-Table

    pDoc->SetOutlineTable( nTab, pUndoTable );

    //  Original Spalten-/Zeilenstatus

    if (pUndoDoc && pUndoTable)
    {
        SCCOLROW nStartCol;
        SCCOLROW nStartRow;
        SCCOLROW nEndCol;
        SCCOLROW nEndRow;
        pUndoTable->GetColArray()->GetRange( nStartCol, nEndCol );
        pUndoTable->GetRowArray()->GetRange( nStartRow, nEndRow );

        pUndoDoc->CopyToDocument( static_cast<SCCOL>(nStartCol), 0, nTab,
                static_cast<SCCOL>(nEndCol), MAXROW, nTab, IDF_NONE, false,
                pDoc );
        pUndoDoc->CopyToDocument( 0, nStartRow, nTab, MAXCOL, nEndRow, nTab, IDF_NONE, false, pDoc );

        pViewShell->UpdateScrollBars();
    }

    //  Original-Daten & Referenzen

    ScUndoUtil::MarkSimpleBlock( pDocShell, 0, aBlockStart.Row(), nTab,
                                            MAXCOL, aBlockEnd.Row(), nTab );
    pDoc->DeleteAreaTab( 0, aBlockStart.Row(),
                            MAXCOL, aBlockEnd.Row(), nTab, IDF_ALL );

    pUndoDoc->CopyToDocument( 0, aBlockStart.Row(), nTab, MAXCOL, aBlockEnd.Row(), nTab,
                                                            IDF_NONE, false, pDoc );            // Flags
    pUndoDoc->UndoToDocument( 0, aBlockStart.Row(), nTab, MAXCOL, aBlockEnd.Row(), nTab,
                                                            IDF_ALL, false, pDoc );

    ScUndoUtil::MarkSimpleBlock( pDocShell, aBlockStart.Col(),aBlockStart.Row(),nTab,
                                            aBlockEnd.Col(),aBlockEnd.Row(),nTab );

    if (pUndoRange)
        pDoc->SetRangeName( new ScRangeName( *pUndoRange ) );
    if (pUndoDB)
        pDoc->SetDBCollection( new ScDBCollection( *pUndoDB ), sal_True );

// erack! it's broadcasted
//  pDoc->SetDirty();

    SCTAB nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

    pDocShell->PostPaint(0,0,nTab,MAXCOL,MAXROW,nTab,PAINT_GRID|PAINT_LEFT|PAINT_TOP|PAINT_SIZE);
    pDocShell->PostDataChanged();

    EndUndo();
}

void ScUndoRepeatDB::Redo()
{
    BeginRedo();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    SCTAB nTab = aBlockStart.Tab();

    SCTAB nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

    ScUndoUtil::MarkSimpleBlock( pDocShell, aBlockStart.Col(),aBlockStart.Row(),nTab,
                                            aBlockEnd.Col(),aBlockEnd.Row(),nTab );
    pViewShell->SetCursor( aCursorPos.Col(), aCursorPos.Row() );

    pViewShell->RepeatDB( false );

    EndRedo();
}

void ScUndoRepeatDB::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->RepeatDB( sal_True );
}

sal_Bool ScUndoRepeatDB::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

//
//      data pilot
//

ScUndoDataPilot::ScUndoDataPilot( ScDocShell* pNewDocShell,
                            ScDocument* pOldDoc, ScDocument* pNewDoc,
                            const ScDPObject* pOldObj, const ScDPObject* pNewObj, sal_Bool bMove ) :
    ScSimpleUndo( pNewDocShell ),
    pOldUndoDoc( pOldDoc ),
    pNewUndoDoc( pNewDoc ),
    pOldDPObject( NULL ),
    pNewDPObject( NULL ),
    bAllowMove( bMove )
{
    if (pOldObj)
        pOldDPObject = new ScDPObject( *pOldObj );
    if (pNewObj)
        pNewDPObject = new ScDPObject( *pNewObj );
}

ScUndoDataPilot::~ScUndoDataPilot()
{
    delete pOldDPObject;
    delete pNewDPObject;
    delete pOldUndoDoc;
    delete pNewUndoDoc;
}

rtl::OUString ScUndoDataPilot::GetComment() const
{
    sal_uInt16 nIndex;
    if ( pOldUndoDoc && pNewUndoDoc )
        nIndex = STR_UNDO_PIVOT_MODIFY;
    else if ( pNewUndoDoc )
        nIndex = STR_UNDO_PIVOT_NEW;
    else
        nIndex = STR_UNDO_PIVOT_DELETE;

    return ScGlobal::GetRscString( nIndex );
}

void ScUndoDataPilot::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();

    ScRange aOldRange;
    ScRange aNewRange;

    if ( pNewDPObject && pNewUndoDoc )
    {
        aNewRange = pNewDPObject->GetOutRange();
        pDoc->DeleteAreaTab( aNewRange, IDF_ALL );
        pNewUndoDoc->CopyToDocument( aNewRange, IDF_ALL, false, pDoc );
    }
    if ( pOldDPObject && pOldUndoDoc )
    {
        aOldRange = pOldDPObject->GetOutRange();
        pDoc->DeleteAreaTab( aOldRange, IDF_ALL );
        pOldUndoDoc->CopyToDocument( aOldRange, IDF_ALL, false, pDoc );
    }

    //  update objects in collection

    if ( pNewDPObject )
    {
        //  find updated object
        //! find by name!

        ScDPObject* pDocObj = pDoc->GetDPAtCursor(
                            aNewRange.aStart.Col(), aNewRange.aStart.Row(), aNewRange.aStart.Tab() );
        OSL_ENSURE(pDocObj, "DPObject not found");
        if (pDocObj)
        {
            if ( pOldDPObject )
            {
                //  restore old settings
                pOldDPObject->WriteSourceDataTo( *pDocObj );
                ScDPSaveData* pData = pOldDPObject->GetSaveData();
                if (pData)
                    pDocObj->SetSaveData(*pData);
                pDocObj->SetOutRange( pOldDPObject->GetOutRange() );
                pOldDPObject->WriteTempDataTo( *pDocObj );
            }
            else
            {
                //  delete inserted object
                pDoc->GetDPCollection()->FreeTable(pDocObj);
            }
        }
    }
    else if ( pOldDPObject )
    {
        //  re-insert deleted object

        ScDPObject* pDestObj = new ScDPObject( *pOldDPObject );
        pDestObj->SetAlive(sal_True);
        if ( !pDoc->GetDPCollection()->InsertNewTable(pDestObj) )
        {
            OSL_FAIL("cannot insert DPObject");
            DELETEZ( pDestObj );
        }
    }

    if (pNewUndoDoc)
        pDocShell->PostPaint( aNewRange, PAINT_GRID, SC_PF_LINES );
    if (pOldUndoDoc)
        pDocShell->PostPaint( aOldRange, PAINT_GRID, SC_PF_LINES );
    pDocShell->PostDataChanged();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        //! set current sheet
    }

    if (pNewDPObject)
    {
        // notify API objects
        pDoc->BroadcastUno( ScDataPilotModifiedHint( pNewDPObject->GetName() ) );
    }

    EndUndo();
}

void ScUndoDataPilot::Redo()
{
    BeginRedo();

    //! copy output data instead of repeating the change,
    //! in case external data have changed!

    ScDocument* pDoc = pDocShell->GetDocument();

    ScDPObject* pSourceObj = NULL;
    if ( pOldDPObject )
    {
        //  find object to modify
        //! find by name!

        ScRange aOldRange = pOldDPObject->GetOutRange();
        pSourceObj = pDoc->GetDPAtCursor(
                        aOldRange.aStart.Col(), aOldRange.aStart.Row(), aOldRange.aStart.Tab() );
        OSL_ENSURE(pSourceObj, "DPObject not found");
    }

    ScDBDocFunc aFunc( *pDocShell );
    aFunc.DataPilotUpdate( pSourceObj, pNewDPObject, false, false, bAllowMove );    // no new undo action

    EndRedo();
}

void ScUndoDataPilot::Repeat(SfxRepeatTarget& /* rTarget */)
{
    //! allow deletion
}

sal_Bool ScUndoDataPilot::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    //! allow deletion
    return false;
}


//
//      Konsolidieren
//

ScUndoConsolidate::ScUndoConsolidate( ScDocShell* pNewDocShell, const ScArea& rArea,
                    const ScConsolidateParam& rPar, ScDocument* pNewUndoDoc,
                    sal_Bool bReference, SCROW nInsCount, ScOutlineTable* pTab,
                    ScDBData* pData ) :
    ScSimpleUndo( pNewDocShell ),
    aDestArea( rArea ),
    pUndoDoc( pNewUndoDoc ),
    aParam( rPar ),
    bInsRef( bReference ),
    nInsertCount( nInsCount ),
    pUndoTab( pTab ),
    pUndoData( pData )
{
}

ScUndoConsolidate::~ScUndoConsolidate()
{
    delete pUndoDoc;
    delete pUndoTab;
    delete pUndoData;
}

rtl::OUString ScUndoConsolidate::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_CONSOLIDATE );
}

void ScUndoConsolidate::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    SCTAB nTab = aDestArea.nTab;

    ScRange aOldRange;
    if (pUndoData)
        pUndoData->GetArea(aOldRange);

    if (bInsRef)
    {
        //  Zeilen loeschen
        pDoc->DeleteRow( 0,nTab, MAXCOL,nTab, aDestArea.nRowStart, nInsertCount );

        //  Outlines
        pDoc->SetOutlineTable( nTab, pUndoTab );

        //  Zeilenstatus
        pUndoDoc->CopyToDocument( 0,0,nTab, MAXCOL,MAXROW,nTab, IDF_NONE, false, pDoc );

        //  Daten & Referenzen
        pDoc->DeleteAreaTab( 0,aDestArea.nRowStart, MAXCOL,aDestArea.nRowEnd, nTab, IDF_ALL );
        pUndoDoc->UndoToDocument( 0, aDestArea.nRowStart, nTab,
                                    MAXCOL, aDestArea.nRowEnd, nTab,
                                    IDF_ALL, false, pDoc );

        //  Original-Bereich
        if (pUndoData)
        {
            pDoc->DeleteAreaTab(aOldRange, IDF_ALL);
            pUndoDoc->CopyToDocument(aOldRange, IDF_ALL, false, pDoc);
        }

        pDocShell->PostPaint( 0,aDestArea.nRowStart,nTab, MAXCOL,MAXROW,nTab,
                                PAINT_GRID | PAINT_LEFT | PAINT_SIZE );
    }
    else
    {
        pDoc->DeleteAreaTab( aDestArea.nColStart,aDestArea.nRowStart,
                                aDestArea.nColEnd,aDestArea.nRowEnd, nTab, IDF_ALL );
        pUndoDoc->CopyToDocument( aDestArea.nColStart, aDestArea.nRowStart, nTab,
                                    aDestArea.nColEnd, aDestArea.nRowEnd, nTab,
                                    IDF_ALL, false, pDoc );

        //  Original-Bereich
        if (pUndoData)
        {
            pDoc->DeleteAreaTab(aOldRange, IDF_ALL);
            pUndoDoc->CopyToDocument(aOldRange, IDF_ALL, false, pDoc);
        }

        SCCOL nEndX = aDestArea.nColEnd;
        SCROW nEndY = aDestArea.nRowEnd;
        if ( pUndoData )
        {
            if ( aOldRange.aEnd.Col() > nEndX )
                nEndX = aOldRange.aEnd.Col();
            if ( aOldRange.aEnd.Row() > nEndY )
                nEndY = aOldRange.aEnd.Row();
        }
        pDocShell->PostPaint( aDestArea.nColStart, aDestArea.nRowStart, nTab,
                                    nEndX, nEndY, nTab, PAINT_GRID );
    }

    //  DB-Bereich wieder anpassen
    if (pUndoData)
    {
        ScDBCollection* pColl = pDoc->GetDBCollection();
        if (pColl)
        {
            ScDBData* pDocData = pColl->getNamedDBs().findByName(pUndoData->GetName());
            if (pDocData)
                *pDocData = *pUndoData;
        }
    }

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        SCTAB nViewTab = pViewShell->GetViewData()->GetTabNo();
        if ( nViewTab != nTab )
            pViewShell->SetTabNo( nTab );
    }

    EndUndo();
}

void ScUndoConsolidate::Redo()
{
    BeginRedo();

    pDocShell->DoConsolidate( aParam, false );

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        SCTAB nViewTab = pViewShell->GetViewData()->GetTabNo();
        if ( nViewTab != aParam.nTab )
            pViewShell->SetTabNo( aParam.nTab );
    }

    EndRedo();
}

void ScUndoConsolidate::Repeat(SfxRepeatTarget& /* rTarget */)
{
}

sal_Bool ScUndoConsolidate::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;
}


//
//      Quell-Daten von Chart aendern
//

void ScUndoChartData::Init()
{
    ScDocument* pDoc = pDocShell->GetDocument();
    aOldRangeListRef = new ScRangeList;
    pDoc->GetOldChartParameters( aChartName, *aOldRangeListRef, bOldColHeaders, bOldRowHeaders );
}

ScUndoChartData::ScUndoChartData( ScDocShell* pNewDocShell, const String& rName,
                                    const ScRange& rNew, bool bColHdr, bool bRowHdr,
                                    bool bAdd ) :
    ScSimpleUndo( pNewDocShell ),
    aChartName( rName ),
    bNewColHeaders( bColHdr ),
    bNewRowHeaders( bRowHdr ),
    bAddRange( bAdd )
{
    aNewRangeListRef = new ScRangeList;
    aNewRangeListRef->Append( rNew );

    Init();
}

ScUndoChartData::ScUndoChartData( ScDocShell* pNewDocShell, const String& rName,
                                    const ScRangeListRef& rNew, bool bColHdr, bool bRowHdr,
                                    bool bAdd ) :
    ScSimpleUndo( pNewDocShell ),
    aChartName( rName ),
    aNewRangeListRef( rNew ),
    bNewColHeaders( bColHdr ),
    bNewRowHeaders( bRowHdr ),
    bAddRange( bAdd )
{
    Init();
}

ScUndoChartData::~ScUndoChartData()
{
}

rtl::OUString ScUndoChartData::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_CHARTDATA );
}

void ScUndoChartData::Undo()
{
    BeginUndo();

    pDocShell->GetDocument()->UpdateChartArea( aChartName, aOldRangeListRef,
                                bOldColHeaders, bOldRowHeaders, false );

    EndUndo();
}

void ScUndoChartData::Redo()
{
    BeginRedo();

    pDocShell->GetDocument()->UpdateChartArea( aChartName, aNewRangeListRef,
                                bNewColHeaders, bNewRowHeaders, bAddRange );

    EndRedo();
}

void ScUndoChartData::Repeat(SfxRepeatTarget& /* rTarget */)
{
}

sal_Bool ScUndoChartData::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;
}

ScUndoDataForm::ScUndoDataForm( ScDocShell* pNewDocShell,
                                SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                                SCCOL nEndX, SCROW nEndY, SCTAB nEndZ,
                                const ScMarkData& rMark,
                                ScDocument* pNewUndoDoc, ScDocument* pNewRedoDoc,
                                sal_uInt16 nNewFlags,
                                ScRefUndoData* pRefData,
                                void* /*pFill1*/, void* /*pFill2*/, void* /*pFill3*/,
                                sal_Bool bRedoIsFilled ) :
        ScBlockUndo( pNewDocShell, ScRange( nStartX, nStartY, nStartZ, nEndX, nEndY, nEndZ ), SC_UNDO_SIMPLE ),
        mpMarkData(new ScMarkData(rMark)),
        pUndoDoc( pNewUndoDoc ),
        pRedoDoc( pNewRedoDoc ),
        nFlags( nNewFlags ),
        pRefUndoData( pRefData ),
        pRefRedoData( NULL ),
        bRedoFilled( bRedoIsFilled )
{
        //      pFill1,pFill2,pFill3 are there so the ctor calls for simple paste (without cutting)
        //      don't have to be changed and branched for 641.
        //      They can be removed later.

        if (!mpMarkData->IsMarked())                            // no cell marked:
                mpMarkData->SetMarkArea(aBlockRange);   //  mark paste block

        if ( pRefUndoData )
                pRefUndoData->DeleteUnchanged( pDocShell->GetDocument() );

        SetChangeTrack();
}

ScUndoDataForm::~ScUndoDataForm()
{
        delete pUndoDoc;
        delete pRedoDoc;
        delete pRefUndoData;
        delete pRefRedoData;
}

rtl::OUString ScUndoDataForm::GetComment() const
{
        return ScGlobal::GetRscString( STR_UNDO_PASTE );
}

void ScUndoDataForm::SetChangeTrack()
{
        ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
        if ( pChangeTrack && (nFlags & IDF_CONTENTS) )
                pChangeTrack->AppendContentRange( aBlockRange, pUndoDoc,
                        nStartChangeAction, nEndChangeAction, SC_CACM_PASTE );
        else
                nStartChangeAction = nEndChangeAction = 0;
}


void ScUndoDataForm::Undo()
{
        BeginUndo();
        DoChange( true );
        ShowTable( aBlockRange );
        EndUndo();
        SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );
}

void ScUndoDataForm::Redo()
{
        BeginRedo();
        ScDocument* pDoc = pDocShell->GetDocument();
        EnableDrawAdjust( pDoc, false );                                //! include in ScBlockUndo?
        DoChange( false );
        EnableDrawAdjust( pDoc, true );                                 //! include in ScBlockUndo?
        EndRedo();
        SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );
}

void ScUndoDataForm::Repeat(SfxRepeatTarget& /*rTarget*/)
{
}

sal_Bool ScUndoDataForm::CanRepeat(SfxRepeatTarget& rTarget) const
{
        return (rTarget.ISA(ScTabViewTarget));
}

void ScUndoDataForm::DoChange( const sal_Bool bUndo )
{
    ScDocument* pDoc = pDocShell->GetDocument();

    //      RefUndoData for redo is created before first undo
    //      (with DeleteUnchanged after the DoUndo call)
    sal_Bool bCreateRedoData = ( bUndo && pRefUndoData && !pRefRedoData );
    if ( bCreateRedoData )
            pRefRedoData = new ScRefUndoData( pDoc );

    ScRefUndoData* pWorkRefData = bUndo ? pRefUndoData : pRefRedoData;

    //      fuer Undo immer alle oder keine Inhalte sichern
    sal_uInt16 nUndoFlags = IDF_NONE;
    if (nFlags & IDF_CONTENTS)
            nUndoFlags |= IDF_CONTENTS;
    if (nFlags & IDF_ATTRIB)
            nUndoFlags |= IDF_ATTRIB;

    sal_Bool bPaintAll = false;

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    SCTAB nTabCount = pDoc->GetTableCount();
    if ( bUndo && !bRedoFilled )
    {
        if (!pRedoDoc)
        {
            sal_Bool bColInfo = ( aBlockRange.aStart.Row()==0 && aBlockRange.aEnd.Row()==MAXROW );
            sal_Bool bRowInfo = ( aBlockRange.aStart.Col()==0 && aBlockRange.aEnd.Col()==MAXCOL );

            pRedoDoc = new ScDocument( SCDOCMODE_UNDO );
            pRedoDoc->InitUndoSelected( pDoc, *mpMarkData, bColInfo, bRowInfo );
        }
        //  read "redo" data from the document in the first undo
            //  all sheets - CopyToDocument skips those that don't exist in pRedoDoc
        ScRange aCopyRange = aBlockRange;
        aCopyRange.aStart.SetTab(0);
        aCopyRange.aEnd.SetTab(nTabCount-1);
        pDoc->CopyToDocument( aCopyRange, 1, false, pRedoDoc );
        bRedoFilled = true;
    }

    sal_uInt16 nExtFlags = 0;
    pDocShell->UpdatePaintExt( nExtFlags, aBlockRange );

    for ( sal_uInt16 i=0; i <= ( aBlockRange.aEnd.Col() - aBlockRange.aStart.Col() ); i++ )
    {
        String aOldString;
        pUndoDoc->GetString( aBlockRange.aStart.Col()+i , aBlockRange.aStart.Row() , aBlockRange.aStart.Tab() , aOldString );
        pDoc->SetString( aBlockRange.aStart.Col()+i , aBlockRange.aStart.Row() , aBlockRange.aStart.Tab() , aOldString );
    }

    if (pWorkRefData)
    {
        pWorkRefData->DoUndo( pDoc, true );             // TRUE = bSetChartRangeLists for SetChartListenerCollection
        if ( pDoc->RefreshAutoFilter( 0,0, MAXCOL,MAXROW, aBlockRange.aStart.Tab() ) )
            bPaintAll = true;
    }

    if ( bCreateRedoData && pRefRedoData )
            pRefRedoData->DeleteUnchanged( pDoc );

    if ( bUndo )
    {
        ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
        if ( pChangeTrack )
            pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );
    }
    else
        SetChangeTrack();

    ScRange aDrawRange( aBlockRange );
    pDoc->ExtendMerge( aDrawRange, true );      // only needed for single sheet (text/rtf etc.)
    sal_uInt16 nPaint = PAINT_GRID;
    if (bPaintAll)
    {
        aDrawRange.aStart.SetCol(0);
        aDrawRange.aStart.SetRow(0);
        aDrawRange.aEnd.SetCol(MAXCOL);
        aDrawRange.aEnd.SetRow(MAXROW);
        nPaint |= PAINT_TOP | PAINT_LEFT;
/*A*/   if (pViewShell)
            pViewShell->AdjustBlockHeight(false);
    }
    else
    {
        if ( aBlockRange.aStart.Row() == 0 && aBlockRange.aEnd.Row() == MAXROW )        // ganze Spalte
        {
            nPaint |= PAINT_TOP;
            aDrawRange.aEnd.SetCol(MAXCOL);
        }
        if ( aBlockRange.aStart.Col() == 0 && aBlockRange.aEnd.Col() == MAXCOL )        // ganze Zeile
        {
            nPaint |= PAINT_LEFT;
            aDrawRange.aEnd.SetRow(MAXROW);
        }
/*A*/   if ((pViewShell) && pViewShell->AdjustBlockHeight(false))
        {
            aDrawRange.aStart.SetCol(0);
            aDrawRange.aStart.SetRow(0);
            aDrawRange.aEnd.SetCol(MAXCOL);
            aDrawRange.aEnd.SetRow(MAXROW);
            nPaint |= PAINT_LEFT;
        }
        pDocShell->UpdatePaintExt( nExtFlags, aDrawRange );
    }

    if ( !bUndo )                               //      draw redo after updating row heights
        RedoSdrUndoAction( pDrawUndo );                 //!     include in ScBlockUndo?

    pDocShell->PostPaint( aDrawRange, nPaint, nExtFlags );

    pDocShell->PostDataChanged();
    if (pViewShell)
        pViewShell->CellContentChanged();
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
