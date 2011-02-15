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

#include <sfx2/app.hxx>

#include "undodat.hxx"
#include "undoutil.hxx"
#include "undoolk.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "tabvwsh.hxx"
#include "olinetab.hxx"
#include "dbcolect.hxx"
#include "rangenam.hxx"
#include "pivot.hxx"
#include "globstr.hrc"
#include "global.hxx"
#include "target.hxx"
#include "chartarr.hxx"
#include "dbdocfun.hxx"
#include "olinefun.hxx"
#include "dpobject.hxx"
#include "attrib.hxx"
#include "hints.hxx"
#include "sc.hrc"

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
//UNUSED2008-05  TYPEINIT1(ScUndoPivot,              ScSimpleUndo);
TYPEINIT1(ScUndoDataPilot,          ScSimpleUndo);
TYPEINIT1(ScUndoConsolidate,        ScSimpleUndo);
TYPEINIT1(ScUndoChartData,          ScSimpleUndo);

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

__EXPORT ScUndoDoOutline::~ScUndoDoOutline()
{
    delete pUndoDoc;
}

String __EXPORT ScUndoDoOutline::GetComment() const
{   // Detail einblenden" "Detail ausblenden"
    return bShow ?
        ScGlobal::GetRscString( STR_UNDO_DOOUTLINE ) :
        ScGlobal::GetRscString( STR_UNDO_REDOOUTLINE );
}

void __EXPORT ScUndoDoOutline::Undo()
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
        pViewShell->HideOutline( bColumns, nLevel, nEntry, sal_False, sal_False );
    else
        pViewShell->ShowOutline( bColumns, nLevel, nEntry, sal_False, sal_False );

    //  Original Spalten-/Zeilenstatus

    if (bColumns)
        pUndoDoc->CopyToDocument( static_cast<SCCOL>(nStart), 0, nTab,
                static_cast<SCCOL>(nEnd), MAXROW, nTab, IDF_NONE, sal_False, pDoc);
    else
        pUndoDoc->CopyToDocument( 0, nStart, nTab, MAXCOL, nEnd, nTab, IDF_NONE, sal_False, pDoc );

    pViewShell->UpdateScrollBars();

    pDocShell->PostPaint(0,0,nTab,MAXCOL,MAXROW,nTab,PAINT_GRID|PAINT_LEFT|PAINT_TOP);

    EndUndo();
}

void __EXPORT ScUndoDoOutline::Redo()
{
    BeginRedo();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    //  Tabelle muss vorher umgeschaltet sein (#46952#) !!!

    SCTAB nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

    if (bShow)
        pViewShell->ShowOutline( bColumns, nLevel, nEntry, sal_False );
    else
        pViewShell->HideOutline( bColumns, nLevel, nEntry, sal_False );

    EndRedo();
}

void __EXPORT ScUndoDoOutline::Repeat(SfxRepeatTarget& /* rTarget */)
{
}

sal_Bool __EXPORT ScUndoDoOutline::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return sal_False;                       // geht nicht
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

__EXPORT ScUndoMakeOutline::~ScUndoMakeOutline()
{
    delete pUndoTable;
}

String __EXPORT ScUndoMakeOutline::GetComment() const
{   // "Gruppierung" "Gruppierung aufheben"
    return bMake ?
        ScGlobal::GetRscString( STR_UNDO_MAKEOUTLINE ) :
        ScGlobal::GetRscString( STR_UNDO_REMAKEOUTLINE );
}

void __EXPORT ScUndoMakeOutline::Undo()
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

void __EXPORT ScUndoMakeOutline::Redo()
{
    BeginRedo();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    ScUndoUtil::MarkSimpleBlock( pDocShell, aBlockStart, aBlockEnd );

    if (bMake)
        pViewShell->MakeOutline( bColumns, sal_False );
    else
        pViewShell->RemoveOutline( bColumns, sal_False );

    pDocShell->PostPaint(0,0,aBlockStart.Tab(),MAXCOL,MAXROW,aBlockEnd.Tab(),PAINT_GRID);

    EndRedo();
}

void __EXPORT ScUndoMakeOutline::Repeat(SfxRepeatTarget& rTarget)
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

sal_Bool __EXPORT ScUndoMakeOutline::CanRepeat(SfxRepeatTarget& rTarget) const
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

__EXPORT ScUndoOutlineLevel::~ScUndoOutlineLevel()
{
    delete pUndoDoc;
    delete pUndoTable;
}

String __EXPORT ScUndoOutlineLevel::GetComment() const
{   // "Gliederungsebene auswaehlen";
    return ScGlobal::GetRscString( STR_UNDO_OUTLINELEVEL );
}

void __EXPORT ScUndoOutlineLevel::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    //  Original Outline-Table

    pDoc->SetOutlineTable( nTab, pUndoTable );

    //  Original Spalten-/Zeilenstatus

    if (bColumns)
        pUndoDoc->CopyToDocument( static_cast<SCCOL>(nStart), 0, nTab,
                static_cast<SCCOL>(nEnd), MAXROW, nTab, IDF_NONE, sal_False, pDoc);
    else
        pUndoDoc->CopyToDocument( 0, nStart, nTab, MAXCOL, nEnd, nTab, IDF_NONE, sal_False, pDoc );

    pDoc->UpdatePageBreaks( nTab );

    pViewShell->UpdateScrollBars();

    SCTAB nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

    pDocShell->PostPaint(0,0,nTab,MAXCOL,MAXROW,nTab,PAINT_GRID|PAINT_LEFT|PAINT_TOP);

    EndUndo();
}

void __EXPORT ScUndoOutlineLevel::Redo()
{
    BeginRedo();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    //  Tabelle muss vorher umgeschaltet sein (#46952#) !!!

    SCTAB nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

    pViewShell->SelectLevel( bColumns, nLevel, sal_False );

    EndRedo();
}

void __EXPORT ScUndoOutlineLevel::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->SelectLevel( bColumns, nLevel, sal_True );
}

sal_Bool __EXPORT ScUndoOutlineLevel::CanRepeat(SfxRepeatTarget& rTarget) const
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

__EXPORT ScUndoOutlineBlock::~ScUndoOutlineBlock()
{
    delete pUndoDoc;
    delete pUndoTable;
}

String __EXPORT ScUndoOutlineBlock::GetComment() const
{   // "Detail einblenden" "Detail ausblenden"
    return bShow ?
        ScGlobal::GetRscString( STR_UNDO_DOOUTLINEBLK ) :
        ScGlobal::GetRscString( STR_UNDO_REDOOUTLINEBLK );
}

void __EXPORT ScUndoOutlineBlock::Undo()
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
        sal_uInt16 nLevel;
        pUndoTable->GetColArray()->FindTouchedLevel( nStartCol, nEndCol, nLevel );
        pUndoTable->GetColArray()->ExtendBlock( nLevel, nStartCol, nEndCol );
        pUndoTable->GetRowArray()->FindTouchedLevel( nStartRow, nEndRow, nLevel );
        pUndoTable->GetRowArray()->ExtendBlock( nLevel, nStartRow, nEndRow );
    }

    pUndoDoc->CopyToDocument( static_cast<SCCOL>(nStartCol), 0, nTab,
            static_cast<SCCOL>(nEndCol), MAXROW, nTab, IDF_NONE, sal_False, pDoc );
    pUndoDoc->CopyToDocument( 0, nStartRow, nTab, MAXCOL, nEndRow, nTab, IDF_NONE, sal_False, pDoc );

    pDoc->UpdatePageBreaks( nTab );

    pViewShell->UpdateScrollBars();

    SCTAB nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

    pDocShell->PostPaint(0,0,nTab,MAXCOL,MAXROW,nTab,PAINT_GRID|PAINT_LEFT|PAINT_TOP);

    EndUndo();
}

void __EXPORT ScUndoOutlineBlock::Redo()
{
    BeginRedo();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    ScUndoUtil::MarkSimpleBlock( pDocShell, aBlockStart, aBlockEnd );
    if (bShow)
        pViewShell->ShowMarkedOutlines( sal_False );
    else
        pViewShell->HideMarkedOutlines( sal_False );

    EndRedo();
}

void __EXPORT ScUndoOutlineBlock::Repeat(SfxRepeatTarget& rTarget)
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

sal_Bool __EXPORT ScUndoOutlineBlock::CanRepeat(SfxRepeatTarget& rTarget) const
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

__EXPORT ScUndoRemoveAllOutlines::~ScUndoRemoveAllOutlines()
{
    delete pUndoDoc;
    delete pUndoTable;
}

String __EXPORT ScUndoRemoveAllOutlines::GetComment() const
{   // "Gliederung entfernen"
    return ScGlobal::GetRscString( STR_UNDO_REMOVEALLOTLNS );
}

void __EXPORT ScUndoRemoveAllOutlines::Undo()
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

    pUndoDoc->CopyToDocument( nStartCol, 0, nTab, nEndCol, MAXROW, nTab, IDF_NONE, sal_False, pDoc );
    pUndoDoc->CopyToDocument( 0, nStartRow, nTab, MAXCOL, nEndRow, nTab, IDF_NONE, sal_False, pDoc );

    pDoc->UpdatePageBreaks( nTab );

    pViewShell->UpdateScrollBars();

    SCTAB nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

    pDocShell->PostPaint(0,0,nTab,MAXCOL,MAXROW,nTab,PAINT_GRID|PAINT_LEFT|PAINT_TOP|PAINT_SIZE);

    EndUndo();
}

void __EXPORT ScUndoRemoveAllOutlines::Redo()
{
    BeginRedo();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    //  Tabelle muss vorher umgeschaltet sein (#46952#) !!!

    SCTAB nTab = aBlockStart.Tab();
    SCTAB nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

    pViewShell->RemoveAllOutlines( sal_False );

    EndRedo();
}

void __EXPORT ScUndoRemoveAllOutlines::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->RemoveAllOutlines( sal_True );
}

sal_Bool __EXPORT ScUndoRemoveAllOutlines::CanRepeat(SfxRepeatTarget& rTarget) const
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

__EXPORT ScUndoAutoOutline::~ScUndoAutoOutline()
{
    delete pUndoDoc;
    delete pUndoTable;
}

String __EXPORT ScUndoAutoOutline::GetComment() const
{   // "Auto-Gliederung"
    return ScGlobal::GetRscString( STR_UNDO_AUTOOUTLINE );
}

void __EXPORT ScUndoAutoOutline::Undo()
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
                static_cast<SCCOL>(nEndCol), MAXROW, nTab, IDF_NONE, sal_False,
                pDoc);
        pUndoDoc->CopyToDocument( 0, nStartRow, nTab, MAXCOL, nEndRow, nTab, IDF_NONE, sal_False, pDoc );

        pViewShell->UpdateScrollBars();
    }

    SCTAB nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

    pDocShell->PostPaint(0,0,nTab,MAXCOL,MAXROW,nTab,PAINT_GRID|PAINT_LEFT|PAINT_TOP|PAINT_SIZE);

    EndUndo();
}

void __EXPORT ScUndoAutoOutline::Redo()
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
    aFunc.AutoOutline( aRange, sal_False, sal_False );

    //  auf der View markieren
    //  Wenn's beim Aufruf eine Mehrfachselektion war, ist es jetzt der
    //  umschliessende Bereich...

    if (pViewShell)
        pViewShell->MarkRange( aRange );

    EndRedo();
}

void __EXPORT ScUndoAutoOutline::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->AutoOutline( sal_True );
}

sal_Bool __EXPORT ScUndoAutoOutline::CanRepeat(SfxRepeatTarget& rTarget) const
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

__EXPORT ScUndoSubTotals::~ScUndoSubTotals()
{
    delete pUndoDoc;
    delete pUndoTable;
    delete pUndoRange;
    delete pUndoDB;
}

String __EXPORT ScUndoSubTotals::GetComment() const
{   // "Teilergebnisse"
    return ScGlobal::GetRscString( STR_UNDO_SUBTOTALS );
}

void __EXPORT ScUndoSubTotals::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    //  um einzelnen DB-Bereich anzupassen
/*  ScDBData* pOldDBData = ScUndoUtil::GetOldDBData( pUndoDBData, pDoc, nTab,
                                        aParam.nCol1, aParam.nRow1, aParam.nCol2, nNewEndRow );
*/

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
                static_cast<SCCOL>(nEndCol), MAXROW, nTab, IDF_NONE, sal_False,
                pDoc);
        pUndoDoc->CopyToDocument( 0, nStartRow, nTab, MAXCOL, nEndRow, nTab, IDF_NONE, sal_False, pDoc );

        pViewShell->UpdateScrollBars();
    }

    //  Original-Daten & Referenzen

    ScUndoUtil::MarkSimpleBlock( pDocShell, 0, aParam.nRow1+1, nTab,
                                            MAXCOL, aParam.nRow2, nTab );

    pDoc->DeleteAreaTab( 0,aParam.nRow1+1, MAXCOL,aParam.nRow2, nTab, IDF_ALL );

    pUndoDoc->CopyToDocument( 0, aParam.nRow1+1, nTab, MAXCOL, aParam.nRow2, nTab,
                                                            IDF_NONE, sal_False, pDoc );    // Flags
    pUndoDoc->UndoToDocument( 0, aParam.nRow1+1, nTab, MAXCOL, aParam.nRow2, nTab,
                                                            IDF_ALL, sal_False, pDoc );

    ScUndoUtil::MarkSimpleBlock( pDocShell, aParam.nCol1,aParam.nRow1,nTab,
                                            aParam.nCol2,aParam.nRow2,nTab );

/*  if (pUndoDBData)
        *pOldDBData = *pUndoDBData;
*/
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

void __EXPORT ScUndoSubTotals::Redo()
{
    BeginRedo();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    SCTAB nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

    ScUndoUtil::MarkSimpleBlock( pDocShell, aParam.nCol1,aParam.nRow1,nTab,
                                            aParam.nCol2,aParam.nRow2,nTab );
    pViewShell->DoSubTotals( aParam, sal_False );

    EndRedo();
}

void __EXPORT ScUndoSubTotals::Repeat(SfxRepeatTarget& /* rTarget */)
{
}

sal_Bool __EXPORT ScUndoSubTotals::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return sal_False;                       // geht nicht wegen Spaltennummern
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
    bDestArea( sal_False )
{
    if ( pDest )
    {
        bDestArea = sal_True;
        aDestRange = *pDest;
    }
}

__EXPORT ScUndoSort::~ScUndoSort()
{
    delete pUndoDoc;
    delete pUndoDB;
}

String __EXPORT ScUndoSort::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_SORT );
}

void __EXPORT ScUndoSort::Undo()
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
                                IDF_ALL|IDF_NOCAPTIONS, sal_False, pDoc );

    if (bDestArea)
    {
        // do not delete/copy note captions, they are handled in drawing undo (ScDBFuncUndo::mpDrawUndo)
        pDoc->DeleteAreaTab( aDestRange, IDF_ALL|IDF_NOCAPTIONS );
        pUndoDoc->CopyToDocument( aDestRange, IDF_ALL|IDF_NOCAPTIONS, sal_False, pDoc );
    }

    //  Zeilenhoehen immer (wegen automatischer Anpassung)
    //! auf ScBlockUndo umstellen
//  if (bRepeatQuery)
        pUndoDoc->CopyToDocument( 0, nStartRow, nSortTab, MAXCOL, nEndRow, nSortTab,
                                IDF_NONE, sal_False, pDoc );

    if (pUndoDB)
        pDoc->SetDBCollection( new ScDBCollection( *pUndoDB ), sal_True );

    SCTAB nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nSortTab )
        pViewShell->SetTabNo( nSortTab );

    pDocShell->PostPaint(0,0,nTab,MAXCOL,MAXROW,nTab,PAINT_GRID|PAINT_LEFT|PAINT_TOP|PAINT_SIZE);
    pDocShell->PostDataChanged();

    EndUndo();
}

void __EXPORT ScUndoSort::Redo()
{
    BeginRedo();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    SCTAB nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

//  pViewShell->DoneBlockMode();
//  pViewShell->InitOwnBlockMode();
//  pViewShell->GetViewData()->GetMarkData() = aMarkData;   // CopyMarksTo

    pViewShell->MarkRange( ScRange( aSortParam.nCol1, aSortParam.nRow1, nTab,
                                      aSortParam.nCol2, aSortParam.nRow2, nTab ) );

    pViewShell->Sort( aSortParam, sal_False );

    //  Quellbereich painten wegen Markierung
    if ( !aSortParam.bInplace )
        pDocShell->PostPaint( aSortParam.nCol1, aSortParam.nRow1, nTab,
                              aSortParam.nCol2, aSortParam.nRow2, nTab, PAINT_GRID );

    EndRedo();
}

void __EXPORT ScUndoSort::Repeat(SfxRepeatTarget& /* rTarget */)
{
}

sal_Bool __EXPORT ScUndoSort::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return sal_False;                       // geht nicht wegen Spaltennummern
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
//  pUndoDBData( pNewData )
    pUndoDB( pNewUndoDB ),
    bIsAdvanced( sal_False ),
    bDestArea( sal_False ),
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

__EXPORT ScUndoQuery::~ScUndoQuery()
{
    delete pUndoDoc;
//  delete pUndoDBData;
    delete pUndoDB;
    DeleteSdrUndoAction( pDrawUndo );
}

String __EXPORT ScUndoQuery::GetComment() const
{   // "Filtern";
    return ScGlobal::GetRscString( STR_UNDO_QUERY );
}

void __EXPORT ScUndoQuery::Undo()
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
                                    IDF_ALL, sal_False, pDoc );
        //  Attribute werden immer mitkopiert (#49287#)

        //  Rest von altem Bereich
        if ( bDestArea && !bDoSize )
        {
            pDoc->DeleteAreaTab( aOldDest, IDF_ALL );
            pUndoDoc->CopyToDocument( aOldDest, IDF_ALL, sal_False, pDoc );
        }
    }
    else
        pUndoDoc->CopyToDocument( 0, aQueryParam.nRow1, nTab, MAXCOL, aQueryParam.nRow2, nTab,
                                        IDF_NONE, sal_False, pDoc );

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

void __EXPORT ScUndoQuery::Redo()
{
    BeginRedo();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    SCTAB nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

    if ( bIsAdvanced )
        pViewShell->Query( aQueryParam, &aAdvSource, sal_False );
    else
        pViewShell->Query( aQueryParam, NULL, sal_False );

    EndRedo();
}

void __EXPORT ScUndoQuery::Repeat(SfxRepeatTarget& /* rTarget */)
{
}

sal_Bool __EXPORT ScUndoQuery::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return sal_False;                       // geht nicht wegen Spaltennummern
}

//
//      Show or hide AutoFilter buttons (doesn't include filter settings)
//

ScUndoAutoFilter::ScUndoAutoFilter( ScDocShell* pNewDocShell, const ScRange& rRange,
                                    const String& rName, sal_Bool bSet ) :
    ScDBFuncUndo( pNewDocShell, rRange ),
    aDBName( rName ),
    bFilterSet( bSet )
{
}

ScUndoAutoFilter::~ScUndoAutoFilter()
{
}

String ScUndoAutoFilter::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_QUERY );    // same as ScUndoQuery
}

void ScUndoAutoFilter::DoChange( sal_Bool bUndo )
{
    sal_Bool bNewFilter = bUndo ? !bFilterSet : bFilterSet;

    sal_uInt16 nIndex;
    ScDocument* pDoc = pDocShell->GetDocument();
    ScDBCollection* pColl = pDoc->GetDBCollection();
    if ( pColl->SearchName( aDBName, nIndex ) )
    {
        ScDBData* pDBData = (*pColl)[nIndex];
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
    DoChange( sal_False );
    EndRedo();
}

void ScUndoAutoFilter::Repeat(SfxRepeatTarget& /* rTarget */)
{
}

sal_Bool ScUndoAutoFilter::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return sal_False;
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

__EXPORT ScUndoDBData::~ScUndoDBData()
{
    delete pUndoColl;
    delete pRedoColl;
}

String __EXPORT ScUndoDBData::GetComment() const
{   // "Datenbankbereiche aendern";
    return ScGlobal::GetRscString( STR_UNDO_DBDATA );
}

void __EXPORT ScUndoDBData::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();

    sal_Bool bOldAutoCalc = pDoc->GetAutoCalc();
    pDoc->SetAutoCalc( sal_False );         // unnoetige Berechnungen vermeiden
    pDoc->CompileDBFormula( sal_True );     // CreateFormulaString
    pDoc->SetDBCollection( new ScDBCollection(*pUndoColl), sal_True );
    pDoc->CompileDBFormula( sal_False );    // CompileFormulaString
    pDoc->SetAutoCalc( bOldAutoCalc );

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_DBAREAS_CHANGED ) );

    EndUndo();
}

void __EXPORT ScUndoDBData::Redo()
{
    BeginRedo();

    ScDocument* pDoc = pDocShell->GetDocument();

    sal_Bool bOldAutoCalc = pDoc->GetAutoCalc();
    pDoc->SetAutoCalc( sal_False );         // unnoetige Berechnungen vermeiden
    pDoc->CompileDBFormula( sal_True );     // CreateFormulaString
    pDoc->SetDBCollection( new ScDBCollection(*pRedoColl), sal_True );
    pDoc->CompileDBFormula( sal_False );    // CompileFormulaString
    pDoc->SetAutoCalc( bOldAutoCalc );

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_DBAREAS_CHANGED ) );

    EndRedo();
}

void __EXPORT ScUndoDBData::Repeat(SfxRepeatTarget& /* rTarget */)
{
}

sal_Bool __EXPORT ScUndoDBData::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return sal_False;                       // geht nicht
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
    bRedoFilled( sal_False )
{
    // redo doc doesn't contain imported data (but everything else)
}

__EXPORT ScUndoImportData::~ScUndoImportData()
{
    delete pUndoDoc;
    delete pRedoDoc;
    delete pUndoDBData;
    delete pRedoDBData;
}

String __EXPORT ScUndoImportData::GetComment() const
{   // "Importieren";
    return ScGlobal::GetRscString( STR_UNDO_IMPORTDATA );
}

void __EXPORT ScUndoImportData::Undo()
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
            pDoc->SetAutoCalc( sal_False );             // outside of the loop
            for (SCCOL nCopyCol = nCol1; nCopyCol <= nCol2; nCopyCol++)
            {
                pDoc->CopyToDocument( nCopyCol,nRow1,nTab, nCopyCol,nRow2,nTab,
                                        IDF_CONTENTS & ~IDF_NOTE, sal_False, pRedoDoc );
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
        pDoc->FitBlock( aNew, aOld, sal_False );                    // rueckwaerts
    }
    else
        pDoc->DeleteAreaTab( aImportParam.nCol1,aImportParam.nRow1,
                                nEndCol,nEndRow, nTab, IDF_ALL & ~IDF_NOTE );

    pUndoDoc->CopyToDocument( aImportParam.nCol1,aImportParam.nRow1,nTab,
                                nEndCol+nFormulaCols,nEndRow,nTab,
                                IDF_ALL & ~IDF_NOTE, sal_False, pDoc );

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

void __EXPORT ScUndoImportData::Redo()
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

        pRedoDoc->CopyToDocument( aNew, IDF_ALL & ~IDF_NOTE, sal_False, pDoc );        // incl. Formeln
    }
    else
    {
        pDoc->DeleteAreaTab( aImportParam.nCol1,aImportParam.nRow1,
                                nEndCol,nEndRow, nTab, IDF_ALL & ~IDF_NOTE );
        pRedoDoc->CopyToDocument( aImportParam.nCol1,aImportParam.nRow1,nTab,
                                nEndCol,nEndRow,nTab, IDF_ALL & ~IDF_NOTE, sal_False, pDoc );
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

void __EXPORT ScUndoImportData::Repeat(SfxRepeatTarget& rTarget)
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

sal_Bool __EXPORT ScUndoImportData::CanRepeat(SfxRepeatTarget& rTarget) const
{
    //  Repeat nur fuer Import per DB-Bereich, dann ist pUndoDBData gesetzt

    if (pUndoDBData)
        return (rTarget.ISA(ScTabViewTarget));
    else
        return sal_False;       // Adressbuch
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
    bQuerySize( sal_False )
{
    if ( pOldQ && pNewQ )
    {
        aOldQuery = *pOldQ;
        aNewQuery = *pNewQ;
        bQuerySize = sal_True;;
    }
}

__EXPORT ScUndoRepeatDB::~ScUndoRepeatDB()
{
    delete pUndoDoc;
    delete pUndoTable;
    delete pUndoRange;
    delete pUndoDB;
}

String __EXPORT ScUndoRepeatDB::GetComment() const
{   // "Wiederholen";       //! bessere Beschreibung!
    return ScGlobal::GetRscString( STR_UNDO_REPEATDB );
}

void __EXPORT ScUndoRepeatDB::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    SCTAB nTab = aBlockStart.Tab();

    if (bQuerySize)
    {
        pDoc->FitBlock( aNewQuery, aOldQuery, sal_False );

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
                pDoc->FitBlock( aNewForm, aOldForm, sal_False );
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
                static_cast<SCCOL>(nEndCol), MAXROW, nTab, IDF_NONE, sal_False,
                pDoc );
        pUndoDoc->CopyToDocument( 0, nStartRow, nTab, MAXCOL, nEndRow, nTab, IDF_NONE, sal_False, pDoc );

        pViewShell->UpdateScrollBars();
    }

    //  Original-Daten & Referenzen

    ScUndoUtil::MarkSimpleBlock( pDocShell, 0, aBlockStart.Row(), nTab,
                                            MAXCOL, aBlockEnd.Row(), nTab );
    pDoc->DeleteAreaTab( 0, aBlockStart.Row(),
                            MAXCOL, aBlockEnd.Row(), nTab, IDF_ALL );

    pUndoDoc->CopyToDocument( 0, aBlockStart.Row(), nTab, MAXCOL, aBlockEnd.Row(), nTab,
                                                            IDF_NONE, sal_False, pDoc );            // Flags
    pUndoDoc->UndoToDocument( 0, aBlockStart.Row(), nTab, MAXCOL, aBlockEnd.Row(), nTab,
                                                            IDF_ALL, sal_False, pDoc );

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

void __EXPORT ScUndoRepeatDB::Redo()
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

    pViewShell->RepeatDB( sal_False );

    EndRedo();
}

void __EXPORT ScUndoRepeatDB::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->RepeatDB( sal_True );
}

sal_Bool __EXPORT ScUndoRepeatDB::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

//UNUSED2008-05  //
//UNUSED2008-05  //     Pivot-Tabellen
//UNUSED2008-05  //
//UNUSED2008-05
//UNUSED2008-05  ScUndoPivot::ScUndoPivot( ScDocShell* pNewDocShell,
//UNUSED2008-05                              const ScArea& rOld, const ScArea& rNew,
//UNUSED2008-05                              ScDocument* pOldDoc, ScDocument* pNewDoc,
//UNUSED2008-05                              const ScPivot* pOldPivot, const ScPivot* pNewPivot ) :
//UNUSED2008-05      ScSimpleUndo( pNewDocShell ),
//UNUSED2008-05      aOldArea( rOld ),
//UNUSED2008-05      aNewArea( rNew ),
//UNUSED2008-05      pOldUndoDoc( pOldDoc ),
//UNUSED2008-05      pNewUndoDoc( pNewDoc )
//UNUSED2008-05  {
//UNUSED2008-05      if (pNewPivot)
//UNUSED2008-05      {
//UNUSED2008-05          pNewPivot->GetParam( aNewParam, aNewQuery, aNewSrc );
//UNUSED2008-05          aNewName = pNewPivot->GetName();
//UNUSED2008-05          aNewTag = pNewPivot->GetTag();
//UNUSED2008-05      }
//UNUSED2008-05      if (pOldPivot)
//UNUSED2008-05      {
//UNUSED2008-05          pOldPivot->GetParam( aOldParam, aOldQuery, aOldSrc );
//UNUSED2008-05          aOldName = pOldPivot->GetName();
//UNUSED2008-05          aOldTag = pOldPivot->GetTag();
//UNUSED2008-05      }
//UNUSED2008-05  }
//UNUSED2008-05
//UNUSED2008-05  __EXPORT ScUndoPivot::~ScUndoPivot()
//UNUSED2008-05  {
//UNUSED2008-05      delete pOldUndoDoc;
//UNUSED2008-05      delete pNewUndoDoc;
//UNUSED2008-05  }
//UNUSED2008-05
//UNUSED2008-05  String __EXPORT ScUndoPivot::GetComment() const
//UNUSED2008-05  {
//UNUSED2008-05      sal_uInt16 nIndex;
//UNUSED2008-05      if ( pOldUndoDoc && pNewUndoDoc )
//UNUSED2008-05          nIndex = STR_UNDO_PIVOT_MODIFY;
//UNUSED2008-05      else if ( pNewUndoDoc )
//UNUSED2008-05          nIndex = STR_UNDO_PIVOT_NEW;
//UNUSED2008-05      else
//UNUSED2008-05          nIndex = STR_UNDO_PIVOT_DELETE;
//UNUSED2008-05
//UNUSED2008-05      return ScGlobal::GetRscString( nIndex );
//UNUSED2008-05  }
//UNUSED2008-05
//UNUSED2008-05  void __EXPORT ScUndoPivot::Undo()
//UNUSED2008-05  {
//UNUSED2008-05      BeginUndo();
//UNUSED2008-05
//UNUSED2008-05      ScDocument* pDoc = pDocShell->GetDocument();
//UNUSED2008-05
//UNUSED2008-05      if (pNewUndoDoc)
//UNUSED2008-05      {
//UNUSED2008-05          pDoc->DeleteAreaTab( aNewArea.nColStart,aNewArea.nRowStart,
//UNUSED2008-05                              aNewArea.nColEnd,aNewArea.nRowEnd, aNewArea.nTab, IDF_ALL );
//UNUSED2008-05          pNewUndoDoc->CopyToDocument( aNewArea.nColStart, aNewArea.nRowStart, aNewArea.nTab,
//UNUSED2008-05                                  aNewArea.nColEnd, aNewArea.nRowEnd, aNewArea.nTab,
//UNUSED2008-05                                  IDF_ALL, sal_False, pDoc );
//UNUSED2008-05      }
//UNUSED2008-05      if (pOldUndoDoc)
//UNUSED2008-05      {
//UNUSED2008-05          pDoc->DeleteAreaTab( aOldArea.nColStart,aOldArea.nRowStart,
//UNUSED2008-05                              aOldArea.nColEnd,aOldArea.nRowEnd, aOldArea.nTab, IDF_ALL );
//UNUSED2008-05          pOldUndoDoc->CopyToDocument( aOldArea.nColStart, aOldArea.nRowStart, aOldArea.nTab,
//UNUSED2008-05                                  aOldArea.nColEnd, aOldArea.nRowEnd, aOldArea.nTab,
//UNUSED2008-05                                  IDF_ALL, sal_False, pDoc );
//UNUSED2008-05      }
//UNUSED2008-05
//UNUSED2008-05      ScPivotCollection* pPivotCollection = pDoc->GetPivotCollection();
//UNUSED2008-05      if ( pNewUndoDoc )
//UNUSED2008-05      {
//UNUSED2008-05          ScPivot* pNewPivot = pPivotCollection->GetPivotAtCursor(
//UNUSED2008-05                                  aNewParam.nCol, aNewParam.nRow, aNewParam.nTab );
//UNUSED2008-05          if (pNewPivot)
//UNUSED2008-05              pPivotCollection->Free( pNewPivot );
//UNUSED2008-05      }
//UNUSED2008-05      if ( pOldUndoDoc )
//UNUSED2008-05      {
//UNUSED2008-05          ScPivot* pOldPivot = new ScPivot( pDoc );
//UNUSED2008-05          pOldPivot->SetParam( aOldParam, aOldQuery, aOldSrc );
//UNUSED2008-05          pOldPivot->SetName( aOldName );
//UNUSED2008-05          pOldPivot->SetTag( aOldTag );
//UNUSED2008-05          if (pOldPivot->CreateData())                            // Felder berechnen
//UNUSED2008-05              pOldPivot->ReleaseData();
//UNUSED2008-05          pPivotCollection->Insert( pOldPivot );
//UNUSED2008-05      }
//UNUSED2008-05
//UNUSED2008-05  // erack! it's broadcasted
//UNUSED2008-05  // pDoc->SetDirty();
//UNUSED2008-05      if (pNewUndoDoc)
//UNUSED2008-05          pDocShell->PostPaint( aNewArea.nColStart, aNewArea.nRowStart, aNewArea.nTab,
//UNUSED2008-05                                  aNewArea.nColEnd, aNewArea.nRowEnd, aNewArea.nTab,
//UNUSED2008-05                                  PAINT_GRID, SC_PF_LINES );
//UNUSED2008-05      if (pOldUndoDoc)
//UNUSED2008-05          pDocShell->PostPaint( aOldArea.nColStart, aOldArea.nRowStart, aOldArea.nTab,
//UNUSED2008-05                                  aOldArea.nColEnd, aOldArea.nRowEnd, aOldArea.nTab,
//UNUSED2008-05                                  PAINT_GRID, SC_PF_LINES );
//UNUSED2008-05      pDocShell->PostDataChanged();
//UNUSED2008-05
//UNUSED2008-05      ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
//UNUSED2008-05      if (pViewShell)
//UNUSED2008-05      {
//UNUSED2008-05          SCTAB nTab = pViewShell->GetViewData()->GetTabNo();
//UNUSED2008-05          if ( pOldUndoDoc )
//UNUSED2008-05          {
//UNUSED2008-05              if ( nTab != aOldArea.nTab )
//UNUSED2008-05                  pViewShell->SetTabNo( aOldArea.nTab );
//UNUSED2008-05          }
//UNUSED2008-05          else if ( pNewUndoDoc )
//UNUSED2008-05          {
//UNUSED2008-05              if ( nTab != aNewArea.nTab )
//UNUSED2008-05                  pViewShell->SetTabNo( aNewArea.nTab );
//UNUSED2008-05          }
//UNUSED2008-05      }
//UNUSED2008-05
//UNUSED2008-05      EndUndo();
//UNUSED2008-05  }
//UNUSED2008-05
//UNUSED2008-05  void __EXPORT ScUndoPivot::Redo()
//UNUSED2008-05  {
//UNUSED2008-05      BeginRedo();
//UNUSED2008-05
//UNUSED2008-05      ScDocument* pDoc = pDocShell->GetDocument();
//UNUSED2008-05      ScPivotCollection* pPivotCollection = pDoc->GetPivotCollection();
//UNUSED2008-05      ScPivot* pOldPivot = pPivotCollection->GetPivotAtCursor(
//UNUSED2008-05                                              aOldParam.nCol, aOldParam.nRow, aOldParam.nTab );
//UNUSED2008-05
//UNUSED2008-05      ScPivot* pNewPivot = NULL;
//UNUSED2008-05      if (pNewUndoDoc)
//UNUSED2008-05      {
//UNUSED2008-05          pNewPivot = new ScPivot( pDoc );
//UNUSED2008-05          pNewPivot->SetParam( aNewParam, aNewQuery, aNewSrc );
//UNUSED2008-05          pNewPivot->SetName( aNewName );
//UNUSED2008-05          pNewPivot->SetTag( aNewTag );
//UNUSED2008-05      }
//UNUSED2008-05
//UNUSED2008-05      pDocShell->PivotUpdate( pOldPivot, pNewPivot, sal_False );
//UNUSED2008-05
//UNUSED2008-05      EndRedo();
//UNUSED2008-05  }
//UNUSED2008-05
//UNUSED2008-05  void __EXPORT ScUndoPivot::Repeat(SfxRepeatTarget& rTarget)
//UNUSED2008-05  {
//UNUSED2008-05      //  Wiederholen: nur loeschen
//UNUSED2008-05
//UNUSED2008-05      if ( pOldUndoDoc && !pNewUndoDoc && rTarget.ISA(ScTabViewTarget) )
//UNUSED2008-05          ((ScTabViewTarget&)rTarget).GetViewShell()->DeletePivotTable();
//UNUSED2008-05  }
//UNUSED2008-05
//UNUSED2008-05  sal_Bool __EXPORT ScUndoPivot::CanRepeat(SfxRepeatTarget& rTarget) const
//UNUSED2008-05  {
//UNUSED2008-05      //  Wiederholen: nur loeschen
//UNUSED2008-05
//UNUSED2008-05      return ( pOldUndoDoc && !pNewUndoDoc && rTarget.ISA(ScTabViewTarget) );
//UNUSED2008-05  }

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

__EXPORT ScUndoDataPilot::~ScUndoDataPilot()
{
    delete pOldDPObject;
    delete pNewDPObject;
    delete pOldUndoDoc;
    delete pNewUndoDoc;
}

String __EXPORT ScUndoDataPilot::GetComment() const
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

void __EXPORT ScUndoDataPilot::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();

    ScRange aOldRange;
    ScRange aNewRange;

    if ( pNewDPObject && pNewUndoDoc )
    {
        aNewRange = pNewDPObject->GetOutRange();
        pDoc->DeleteAreaTab( aNewRange, IDF_ALL );
        pNewUndoDoc->CopyToDocument( aNewRange, IDF_ALL, sal_False, pDoc );
    }
    if ( pOldDPObject && pOldUndoDoc )
    {
        aOldRange = pOldDPObject->GetOutRange();
        pDoc->DeleteAreaTab( aOldRange, IDF_ALL );
        pOldUndoDoc->CopyToDocument( aOldRange, IDF_ALL, sal_False, pDoc );
    }

    //  update objects in collection

    if ( pNewDPObject )
    {
        //  find updated object
        //! find by name!

        ScDPObject* pDocObj = pDoc->GetDPAtCursor(
                            aNewRange.aStart.Col(), aNewRange.aStart.Row(), aNewRange.aStart.Tab() );
        DBG_ASSERT(pDocObj, "DPObject not found");
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
            DBG_ERROR("cannot insert DPObject");
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

void __EXPORT ScUndoDataPilot::Redo()
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
        DBG_ASSERT(pSourceObj, "DPObject not found");
    }

    ScDBDocFunc aFunc( *pDocShell );
    aFunc.DataPilotUpdate( pSourceObj, pNewDPObject, sal_False, sal_False, bAllowMove );    // no new undo action

    EndRedo();
}

void __EXPORT ScUndoDataPilot::Repeat(SfxRepeatTarget& /* rTarget */)
{
    //! allow deletion
}

sal_Bool __EXPORT ScUndoDataPilot::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    //! allow deletion
    return sal_False;
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

__EXPORT ScUndoConsolidate::~ScUndoConsolidate()
{
    delete pUndoDoc;
    delete pUndoTab;
    delete pUndoData;
}

String __EXPORT ScUndoConsolidate::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_CONSOLIDATE );
}

void __EXPORT ScUndoConsolidate::Undo()
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
        pUndoDoc->CopyToDocument( 0,0,nTab, MAXCOL,MAXROW,nTab, IDF_NONE, sal_False, pDoc );

        //  Daten & Referenzen
        pDoc->DeleteAreaTab( 0,aDestArea.nRowStart, MAXCOL,aDestArea.nRowEnd, nTab, IDF_ALL );
        pUndoDoc->UndoToDocument( 0, aDestArea.nRowStart, nTab,
                                    MAXCOL, aDestArea.nRowEnd, nTab,
                                    IDF_ALL, sal_False, pDoc );

        //  Original-Bereich
        if (pUndoData)
        {
            pDoc->DeleteAreaTab(aOldRange, IDF_ALL);
            pUndoDoc->CopyToDocument(aOldRange, IDF_ALL, sal_False, pDoc);
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
                                    IDF_ALL, sal_False, pDoc );

        //  Original-Bereich
        if (pUndoData)
        {
            pDoc->DeleteAreaTab(aOldRange, IDF_ALL);
            pUndoDoc->CopyToDocument(aOldRange, IDF_ALL, sal_False, pDoc);
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
            sal_uInt16 nIndex;
            if (pColl->SearchName( pUndoData->GetName(), nIndex ))
            {
                ScDBData* pDocData = (*pColl)[nIndex];
                if (pDocData)
                    *pDocData = *pUndoData;
            }
            else
            {
                DBG_ERROR("alte DB-Daten nicht gefunden");
            }
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

void __EXPORT ScUndoConsolidate::Redo()
{
    BeginRedo();

    pDocShell->DoConsolidate( aParam, sal_False );

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        SCTAB nViewTab = pViewShell->GetViewData()->GetTabNo();
        if ( nViewTab != aParam.nTab )
            pViewShell->SetTabNo( aParam.nTab );
    }

    EndRedo();
}

void __EXPORT ScUndoConsolidate::Repeat(SfxRepeatTarget& /* rTarget */)
{
}

sal_Bool __EXPORT ScUndoConsolidate::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return sal_False;
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
                                    const ScRange& rNew, sal_Bool bColHdr, sal_Bool bRowHdr,
                                    sal_Bool bAdd ) :
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
                                    const ScRangeListRef& rNew, sal_Bool bColHdr, sal_Bool bRowHdr,
                                    sal_Bool bAdd ) :
    ScSimpleUndo( pNewDocShell ),
    aChartName( rName ),
    aNewRangeListRef( rNew ),
    bNewColHeaders( bColHdr ),
    bNewRowHeaders( bRowHdr ),
    bAddRange( bAdd )
{
    Init();
}

__EXPORT ScUndoChartData::~ScUndoChartData()
{
}

String __EXPORT ScUndoChartData::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_CHARTDATA );
}

void __EXPORT ScUndoChartData::Undo()
{
    BeginUndo();

    pDocShell->GetDocument()->UpdateChartArea( aChartName, aOldRangeListRef,
                                bOldColHeaders, bOldRowHeaders, sal_False );

    EndUndo();
}

void __EXPORT ScUndoChartData::Redo()
{
    BeginRedo();

    pDocShell->GetDocument()->UpdateChartArea( aChartName, aNewRangeListRef,
                                bNewColHeaders, bNewRowHeaders, bAddRange );

    EndRedo();
}

void __EXPORT ScUndoChartData::Repeat(SfxRepeatTarget& /* rTarget */)
{
}

sal_Bool __EXPORT ScUndoChartData::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return sal_False;
}






