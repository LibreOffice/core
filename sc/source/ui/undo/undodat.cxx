/*************************************************************************
 *
 *  $RCSfile: undodat.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:07 $
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

// System - Includes -----------------------------------------------------

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include <sfx2/app.hxx>

#include "undodat.hxx"
#include "undoutil.hxx"
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
#include "sc.hrc"

// -----------------------------------------------------------------------

TYPEINIT1(ScUndoDoOutline,          SfxUndoAction);
TYPEINIT1(ScUndoMakeOutline,        SfxUndoAction);
TYPEINIT1(ScUndoOutlineLevel,       SfxUndoAction);
TYPEINIT1(ScUndoOutlineBlock,       SfxUndoAction);
TYPEINIT1(ScUndoRemoveAllOutlines,  SfxUndoAction);
TYPEINIT1(ScUndoAutoOutline,        SfxUndoAction);
TYPEINIT1(ScUndoSubTotals,          SfxUndoAction);
TYPEINIT1(ScUndoSort,               SfxUndoAction);
TYPEINIT1(ScUndoQuery,              SfxUndoAction);
TYPEINIT1(ScUndoDBData,             SfxUndoAction);
TYPEINIT1(ScUndoImportData,         SfxUndoAction);
TYPEINIT1(ScUndoRepeatDB,           SfxUndoAction);
TYPEINIT1(ScUndoPivot,              SfxUndoAction);
TYPEINIT1(ScUndoDataPilot,          SfxUndoAction);
TYPEINIT1(ScUndoConsolidate,        SfxUndoAction);
TYPEINIT1(ScUndoChartData,          SfxUndoAction);

// -----------------------------------------------------------------------


//
//      Outline-Gruppen ein- oder ausblenden
//

ScUndoDoOutline::ScUndoDoOutline( ScDocShell* pNewDocShell,
                            USHORT nNewStart, USHORT nNewEnd, USHORT nNewTab,
                            ScDocument* pNewUndoDoc, BOOL bNewColumns,
                            USHORT nNewLevel, USHORT nNewEntry, BOOL bNewShow ) :
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

    USHORT nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

    //  inverse Funktion ausfuehren

    if (bShow)
        pViewShell->HideOutline( bColumns, nLevel, nEntry, FALSE, FALSE );
    else
        pViewShell->ShowOutline( bColumns, nLevel, nEntry, FALSE, FALSE );

    //  Original Spalten-/Zeilenstatus

    if (bColumns)
        pUndoDoc->CopyToDocument( nStart, 0, nTab, nEnd, MAXROW, nTab, IDF_NONE, FALSE, pDoc );
    else
        pUndoDoc->CopyToDocument( 0, nStart, nTab, MAXCOL, nEnd, nTab, IDF_NONE, FALSE, pDoc );

    pViewShell->UpdateScrollBars();

    pDocShell->PostPaint(0,0,nTab,MAXCOL,MAXROW,nTab,PAINT_GRID|PAINT_LEFT|PAINT_TOP);

    EndUndo();
}

void __EXPORT ScUndoDoOutline::Redo()
{
    BeginRedo();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    //  Tabelle muss vorher umgeschaltet sein (#46952#) !!!

    USHORT nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

    if (bShow)
        pViewShell->ShowOutline( bColumns, nLevel, nEntry, FALSE );
    else
        pViewShell->HideOutline( bColumns, nLevel, nEntry, FALSE );

    EndRedo();
}

void __EXPORT ScUndoDoOutline::Repeat(SfxRepeatTarget& rTarget)
{
}

BOOL __EXPORT ScUndoDoOutline::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return FALSE;                       // geht nicht
}

//
//      Outline-Gruppen erzeugen oder loeschen
//

ScUndoMakeOutline::ScUndoMakeOutline( ScDocShell* pNewDocShell,
                            USHORT nStartX, USHORT nStartY, USHORT nStartZ,
                            USHORT nEndX, USHORT nEndY, USHORT nEndZ,
                            ScOutlineTable* pNewUndoTab, BOOL bNewColumns, BOOL bNewMake ) :
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
    USHORT nTab = aBlockStart.GetTab();

    ScUndoUtil::MarkSimpleBlock( pDocShell, aBlockStart, aBlockEnd );

    pDoc->SetOutlineTable( nTab, pUndoTable );

    USHORT nVisTab = pViewShell->GetViewData()->GetTabNo();
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
        pViewShell->MakeOutline( bColumns, FALSE );
    else
        pViewShell->RemoveOutline( bColumns, FALSE );

    pDocShell->PostPaint(0,0,aBlockStart.GetTab(),MAXCOL,MAXROW,aBlockEnd.GetTab(),PAINT_GRID);

    EndRedo();
}

void __EXPORT ScUndoMakeOutline::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
    {
        ScTabViewShell& rViewShell = *((ScTabViewTarget&)rTarget).GetViewShell();

        if (bMake)
            rViewShell.MakeOutline( bColumns, TRUE );
        else
            rViewShell.RemoveOutline( bColumns, TRUE );
    }
}

BOOL __EXPORT ScUndoMakeOutline::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

//
//      Outline-Ebene auswaehlen
//

ScUndoOutlineLevel::ScUndoOutlineLevel( ScDocShell* pNewDocShell,
                        USHORT nNewStart, USHORT nNewEnd, USHORT nNewTab,
                        ScDocument* pNewUndoDoc, ScOutlineTable* pNewUndoTab,
                        BOOL bNewColumns, USHORT nNewLevel ) :
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
        pUndoDoc->CopyToDocument( nStart, 0, nTab, nEnd, MAXROW, nTab, IDF_NONE, FALSE, pDoc );
    else
        pUndoDoc->CopyToDocument( 0, nStart, nTab, MAXCOL, nEnd, nTab, IDF_NONE, FALSE, pDoc );

    pDoc->UpdatePageBreaks( nTab );

    pViewShell->UpdateScrollBars();

    USHORT nVisTab = pViewShell->GetViewData()->GetTabNo();
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

    USHORT nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

    pViewShell->SelectLevel( bColumns, nLevel, FALSE );

    EndRedo();
}

void __EXPORT ScUndoOutlineLevel::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->SelectLevel( bColumns, nLevel, TRUE );
}

BOOL __EXPORT ScUndoOutlineLevel::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

//
//      Outline ueber Blockmarken ein- oder ausblenden
//

ScUndoOutlineBlock::ScUndoOutlineBlock( ScDocShell* pNewDocShell,
                        USHORT nStartX, USHORT nStartY, USHORT nStartZ,
                        USHORT nEndX, USHORT nEndY, USHORT nEndZ,
                        ScDocument* pNewUndoDoc, ScOutlineTable* pNewUndoTab, BOOL bNewShow ) :
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
    USHORT nTab = aBlockStart.GetTab();

    //  Original Outline-Table

    pDoc->SetOutlineTable( nTab, pUndoTable );

    //  Original Spalten-/Zeilenstatus

    USHORT  nStartCol = aBlockStart.GetCol();
    USHORT  nEndCol = aBlockEnd.GetCol();
    USHORT  nStartRow = aBlockStart.GetRow();
    USHORT  nEndRow = aBlockEnd.GetRow();

    if (!bShow)
    {                               //  Groesse des ausgeblendeten Blocks
        USHORT nLevel;
        pUndoTable->GetColArray()->FindTouchedLevel( nStartCol, nEndCol, nLevel );
        pUndoTable->GetColArray()->ExtendBlock( nLevel, nStartCol, nEndCol );
        pUndoTable->GetRowArray()->FindTouchedLevel( nStartRow, nEndRow, nLevel );
        pUndoTable->GetRowArray()->ExtendBlock( nLevel, nStartRow, nEndRow );
    }

    pUndoDoc->CopyToDocument( nStartCol, 0, nTab, nEndCol, MAXROW, nTab, IDF_NONE, FALSE, pDoc );
    pUndoDoc->CopyToDocument( 0, nStartRow, nTab, MAXCOL, nEndRow, nTab, IDF_NONE, FALSE, pDoc );

    pDoc->UpdatePageBreaks( nTab );

    pViewShell->UpdateScrollBars();

    USHORT nVisTab = pViewShell->GetViewData()->GetTabNo();
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
        pViewShell->ShowMarkedOutlines( FALSE );
    else
        pViewShell->HideMarkedOutlines( FALSE );

    EndRedo();
}

void __EXPORT ScUndoOutlineBlock::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
    {
        ScTabViewShell& rViewShell = *((ScTabViewTarget&)rTarget).GetViewShell();

        if (bShow)
            rViewShell.ShowMarkedOutlines( TRUE );
        else
            rViewShell.HideMarkedOutlines( TRUE );
    }
}

BOOL __EXPORT ScUndoOutlineBlock::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

//
//      alle Outlines loeschen
//

ScUndoRemoveAllOutlines::ScUndoRemoveAllOutlines( ScDocShell* pNewDocShell,
                                    USHORT nStartX, USHORT nStartY, USHORT nStartZ,
                                    USHORT nEndX, USHORT nEndY, USHORT nEndZ,
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
    USHORT nTab = aBlockStart.GetTab();

    //  Original Outline-Table

    pDoc->SetOutlineTable( nTab, pUndoTable );

    //  Original Spalten-/Zeilenstatus

    USHORT  nStartCol = aBlockStart.GetCol();
    USHORT  nEndCol = aBlockEnd.GetCol();
    USHORT  nStartRow = aBlockStart.GetRow();
    USHORT  nEndRow = aBlockEnd.GetRow();

    pUndoDoc->CopyToDocument( nStartCol, 0, nTab, nEndCol, MAXROW, nTab, IDF_NONE, FALSE, pDoc );
    pUndoDoc->CopyToDocument( 0, nStartRow, nTab, MAXCOL, nEndRow, nTab, IDF_NONE, FALSE, pDoc );

    pDoc->UpdatePageBreaks( nTab );

    pViewShell->UpdateScrollBars();

    USHORT nVisTab = pViewShell->GetViewData()->GetTabNo();
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

    USHORT nTab = aBlockStart.GetTab();
    USHORT nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

    pViewShell->RemoveAllOutlines( FALSE );

    EndRedo();
}

void __EXPORT ScUndoRemoveAllOutlines::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->RemoveAllOutlines( TRUE );
}

BOOL __EXPORT ScUndoRemoveAllOutlines::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

//
//      Auto-Outline
//

ScUndoAutoOutline::ScUndoAutoOutline( ScDocShell* pNewDocShell,
                                    USHORT nStartX, USHORT nStartY, USHORT nStartZ,
                                    USHORT nEndX, USHORT nEndY, USHORT nEndZ,
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
    USHORT nTab = aBlockStart.GetTab();

    //  Original Outline-Table

    pDoc->SetOutlineTable( nTab, pUndoTable );

    //  Original Spalten-/Zeilenstatus

    if (pUndoDoc && pUndoTable)
    {
        USHORT nStartCol;
        USHORT nStartRow;
        USHORT nEndCol;
        USHORT nEndRow;
        pUndoTable->GetColArray()->GetRange( nStartCol, nEndCol );
        pUndoTable->GetRowArray()->GetRange( nStartRow, nEndRow );

        pUndoDoc->CopyToDocument( nStartCol, 0, nTab, nEndCol, MAXROW, nTab, IDF_NONE, FALSE, pDoc );
        pUndoDoc->CopyToDocument( 0, nStartRow, nTab, MAXCOL, nEndRow, nTab, IDF_NONE, FALSE, pDoc );

        pViewShell->UpdateScrollBars();
    }

    USHORT nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

    pDocShell->PostPaint(0,0,nTab,MAXCOL,MAXROW,nTab,PAINT_GRID|PAINT_LEFT|PAINT_TOP|PAINT_SIZE);

    EndUndo();
}

void __EXPORT ScUndoAutoOutline::Redo()
{
    BeginRedo();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    ScDocument* pDoc = pDocShell->GetDocument();

    USHORT nTab = aBlockStart.GetTab();
    if (pViewShell)
    {
        //  Tabelle muss vorher umgeschaltet sein (#46952#) !!!

        USHORT nVisTab = pViewShell->GetViewData()->GetTabNo();
        if ( nVisTab != nTab )
            pViewShell->SetTabNo( nTab );
    }

    ScRange aRange( aBlockStart.GetCol(), aBlockStart.GetRow(), nTab,
                    aBlockEnd.GetCol(),   aBlockEnd.GetRow(),   nTab );
    ScOutlineDocFunc aFunc( *pDocShell );
    aFunc.AutoOutline( aRange, FALSE, FALSE );

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
        ((ScTabViewTarget&)rTarget).GetViewShell()->AutoOutline( TRUE );
}

BOOL __EXPORT ScUndoAutoOutline::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

//
//      Zwischenergebnisse
//

ScUndoSubTotals::ScUndoSubTotals( ScDocShell* pNewDocShell, USHORT nNewTab,
                                const ScSubTotalParam& rNewParam, USHORT nNewEndY,
                                ScDocument* pNewUndoDoc, ScOutlineTable* pNewUndoTab,
                                ScRangeName* pNewUndoRange, ScDBCollection* pNewUndoDB ) :
    ScSimpleUndo( pNewDocShell ),
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
        pDoc->DeleteRow( 0,nTab, MAXCOL,nTab, aParam.nRow2+1, nNewEndRow-aParam.nRow2 );
    }
    else if (nNewEndRow < aParam.nRow2)
    {
        pDoc->InsertRow( 0,nTab, MAXCOL,nTab, nNewEndRow+1, nNewEndRow-aParam.nRow2 );
    }


    //  Original Outline-Table

    pDoc->SetOutlineTable( nTab, pUndoTable );

    //  Original Spalten-/Zeilenstatus

    if (pUndoDoc && pUndoTable)
    {
        USHORT nStartCol;
        USHORT nStartRow;
        USHORT nEndCol;
        USHORT nEndRow;
        pUndoTable->GetColArray()->GetRange( nStartCol, nEndCol );
        pUndoTable->GetRowArray()->GetRange( nStartRow, nEndRow );

        pUndoDoc->CopyToDocument( nStartCol, 0, nTab, nEndCol, MAXROW, nTab, IDF_NONE, FALSE, pDoc );
        pUndoDoc->CopyToDocument( 0, nStartRow, nTab, MAXCOL, nEndRow, nTab, IDF_NONE, FALSE, pDoc );

        pViewShell->UpdateScrollBars();
    }

    //  Original-Daten & Referenzen

    ScUndoUtil::MarkSimpleBlock( pDocShell, 0, aParam.nRow1+1, nTab,
                                            MAXCOL, aParam.nRow2, nTab );

    pDoc->DeleteAreaTab( 0,aParam.nRow1+1, MAXCOL,aParam.nRow2, nTab, IDF_ALL );

    pUndoDoc->CopyToDocument( 0, aParam.nRow1+1, nTab, MAXCOL, aParam.nRow2, nTab,
                                                            IDF_NONE, FALSE, pDoc );    // Flags
    pUndoDoc->UndoToDocument( 0, aParam.nRow1+1, nTab, MAXCOL, aParam.nRow2, nTab,
                                                            IDF_ALL, FALSE, pDoc );

    ScUndoUtil::MarkSimpleBlock( pDocShell, aParam.nCol1,aParam.nRow1,nTab,
                                            aParam.nCol2,aParam.nRow2,nTab );

/*  if (pUndoDBData)
        *pOldDBData = *pUndoDBData;
*/
    if (pUndoRange)
        pDoc->SetRangeName( new ScRangeName( *pUndoRange ) );
    if (pUndoDB)
        pDoc->SetDBCollection( new ScDBCollection( *pUndoDB ) );

    USHORT nVisTab = pViewShell->GetViewData()->GetTabNo();
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
    ScDocument* pDoc = pDocShell->GetDocument();

    USHORT nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

    ScUndoUtil::MarkSimpleBlock( pDocShell, aParam.nCol1,aParam.nRow1,nTab,
                                            aParam.nCol2,aParam.nRow2,nTab );
    pViewShell->DoSubTotals( aParam, FALSE );

    EndRedo();
}

void __EXPORT ScUndoSubTotals::Repeat(SfxRepeatTarget& rTarget)
{
}

BOOL __EXPORT ScUndoSubTotals::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return FALSE;                       // geht nicht wegen Spaltennummern
}

//
//      Sortieren
//

ScUndoSort::ScUndoSort( ScDocShell* pNewDocShell,
                        USHORT nNewTab, const ScSortParam& rParam,
                        BOOL bQuery, ScDocument* pNewUndoDoc, ScDBCollection* pNewUndoDB,
                        const ScRange* pDest ) :
    ScSimpleUndo( pNewDocShell ),
    nTab( nNewTab ),
    aSortParam( rParam ),
    bRepeatQuery( bQuery ),
    pUndoDoc( pNewUndoDoc ),
    pUndoDB( pNewUndoDB ),
    bDestArea( FALSE )
{
    if ( pDest )
    {
        bDestArea = TRUE;
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

    USHORT nStartCol = aSortParam.nCol1;
    USHORT nStartRow = aSortParam.nRow1;
    USHORT nEndCol   = aSortParam.nCol2;
    USHORT nEndRow   = aSortParam.nRow2;
    USHORT nSortTab  = nTab;
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

    pDoc->DeleteAreaTab( nStartCol,nStartRow, nEndCol,nEndRow, nSortTab, IDF_ALL );

    pUndoDoc->CopyToDocument( nStartCol, nStartRow, nSortTab, nEndCol, nEndRow, nSortTab,
                                IDF_ALL, FALSE, pDoc );

    if (bDestArea)
    {
        pDoc->DeleteAreaTab( aDestRange, IDF_ALL );
        pUndoDoc->CopyToDocument( aDestRange, IDF_ALL, FALSE, pDoc );
    }

    //  Zeilenhoehen immer (wegen automatischer Anpassung)
    //! auf ScBlockUndo umstellen
//  if (bRepeatQuery)
        pUndoDoc->CopyToDocument( 0, nStartRow, nSortTab, MAXCOL, nEndRow, nSortTab,
                                IDF_NONE, FALSE, pDoc );

    if (pUndoDB)
        pDoc->SetDBCollection( new ScDBCollection( *pUndoDB ) );

    USHORT nVisTab = pViewShell->GetViewData()->GetTabNo();
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
    ScDocument* pDoc = pDocShell->GetDocument();

    USHORT nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

//  pViewShell->DoneBlockMode();
//  pViewShell->InitOwnBlockMode();
//  pViewShell->GetViewData()->GetMarkData() = aMarkData;   // CopyMarksTo

    pViewShell->MarkRange( ScRange( aSortParam.nCol1, aSortParam.nRow1, nTab,
                                      aSortParam.nCol2, aSortParam.nRow2, nTab ) );

    pViewShell->Sort( aSortParam, FALSE );

    //  Quellbereich painten wegen Markierung
    if ( !aSortParam.bInplace )
        pDocShell->PostPaint( aSortParam.nCol1, aSortParam.nRow1, nTab,
                              aSortParam.nCol2, aSortParam.nRow2, nTab, PAINT_GRID );

    EndRedo();
}

void __EXPORT ScUndoSort::Repeat(SfxRepeatTarget& rTarget)
{
}

BOOL __EXPORT ScUndoSort::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return FALSE;                       // geht nicht wegen Spaltennummern
}

//
//      Filtern
//

ScUndoQuery::ScUndoQuery( ScDocShell* pNewDocShell, USHORT nNewTab, const ScQueryParam& rParam,
                            ScDocument* pNewUndoDoc, ScDBCollection* pNewUndoDB,
                            const ScRange* pOld, BOOL bSize, const ScRange* pAdvSrc ) :
    ScSimpleUndo( pNewDocShell ),
    nTab( nNewTab ),
    aQueryParam( rParam ),
    pUndoDoc( pNewUndoDoc ),
//  pUndoDBData( pNewData )
    pUndoDB( pNewUndoDB ),
    bIsAdvanced( FALSE ),
    bDestArea( FALSE ),
    bDoSize( bSize )
{
    if ( pOld )
    {
        bDestArea = TRUE;
        aOldDest = *pOld;
    }
    if ( pAdvSrc )
    {
        bIsAdvanced = TRUE;
        aAdvSource = *pAdvSrc;
    }
}

__EXPORT ScUndoQuery::~ScUndoQuery()
{
    delete pUndoDoc;
//  delete pUndoDBData;
    delete pUndoDB;
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

    BOOL bCopy = !aQueryParam.bInplace;
    USHORT nDestEndCol = 0;
    USHORT nDestEndRow = 0;
    if (bCopy)
    {
        nDestEndCol = aQueryParam.nDestCol + ( aQueryParam.nCol2-aQueryParam.nCol1 );
        nDestEndRow = aQueryParam.nDestRow + ( aQueryParam.nRow2-aQueryParam.nRow1 );

        ScDBData* pData = pDoc->GetDBAtCursor( aQueryParam.nDestCol, aQueryParam.nDestRow,
                                                aQueryParam.nDestTab, TRUE );
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
                                    IDF_ALL, FALSE, pDoc );
        //  Attribute werden immer mitkopiert (#49287#)

        //  Rest von altem Bereich
        if ( bDestArea && !bDoSize )
        {
            pDoc->DeleteAreaTab( aOldDest, IDF_ALL );
            pUndoDoc->CopyToDocument( aOldDest, IDF_ALL, FALSE, pDoc );
        }
    }
    else
        pUndoDoc->CopyToDocument( 0, aQueryParam.nRow1, nTab, MAXCOL, aQueryParam.nRow2, nTab,
                                        IDF_NONE, FALSE, pDoc );

    if (pUndoDB)
        pDoc->SetDBCollection( new ScDBCollection( *pUndoDB ) );

    if (!bCopy)
        pDoc->UpdatePageBreaks( nTab );

    USHORT nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

        //  Paint

    if (bCopy)
    {
        USHORT nEndX = nDestEndCol;
        USHORT nEndY = nDestEndRow;
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
                                    nEndY, nEndY, aQueryParam.nDestTab, PAINT_GRID );
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
    ScDocument* pDoc = pDocShell->GetDocument();

    USHORT nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

    if ( bIsAdvanced )
        pViewShell->Query( aQueryParam, &aAdvSource, FALSE );
    else
        pViewShell->Query( aQueryParam, NULL, FALSE );

    EndRedo();
}

void __EXPORT ScUndoQuery::Repeat(SfxRepeatTarget& rTarget)
{
}

BOOL __EXPORT ScUndoQuery::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return FALSE;                       // geht nicht wegen Spaltennummern
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

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    ScDocument* pDoc = pDocShell->GetDocument();

    BOOL bOldAutoCalc = pDoc->GetAutoCalc();
    pDoc->SetAutoCalc( FALSE );         // unnoetige Berechnungen vermeiden
    pDoc->CompileDBFormula( TRUE );     // CreateFormulaString
    pDoc->SetDBCollection( new ScDBCollection(*pUndoColl) );
    pDoc->CompileDBFormula( FALSE );    // CompileFormulaString
    pDoc->SetAutoCalc( bOldAutoCalc );

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_DBAREAS_CHANGED ) );

    EndUndo();
}

void __EXPORT ScUndoDBData::Redo()
{
    BeginRedo();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    ScDocument* pDoc = pDocShell->GetDocument();

    BOOL bOldAutoCalc = pDoc->GetAutoCalc();
    pDoc->SetAutoCalc( FALSE );         // unnoetige Berechnungen vermeiden
    pDoc->CompileDBFormula( TRUE );     // CreateFormulaString
    pDoc->SetDBCollection( new ScDBCollection(*pRedoColl) );
    pDoc->CompileDBFormula( FALSE );    // CompileFormulaString
    pDoc->SetAutoCalc( bOldAutoCalc );

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_DBAREAS_CHANGED ) );

    EndRedo();
}

void __EXPORT ScUndoDBData::Repeat(SfxRepeatTarget& rTarget)
{
}

BOOL __EXPORT ScUndoDBData::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return FALSE;                       // geht nicht
}

//
//      Import
//

ScUndoImportData::ScUndoImportData( ScDocShell* pNewDocShell, USHORT nNewTab,
                                const ScImportParam& rParam, USHORT nNewEndX, USHORT nNewEndY,
                                USHORT nNewFormula,
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
    bRedoFilled( FALSE )
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

    USHORT nTable, nCol1, nRow1, nCol2, nRow2;
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

            BOOL bOldAutoCalc = pDoc->GetAutoCalc();
            pDoc->SetAutoCalc( FALSE );             // outside of the loop
            for (USHORT nCopyCol = nCol1; nCopyCol <= nCol2; nCopyCol++)
            {
                pDoc->CopyToDocument( nCopyCol,nRow1,nTab, nCopyCol,nRow2,nTab,
                                        IDF_CONTENTS, FALSE, pRedoDoc );
                pDoc->DeleteAreaTab( nCopyCol,nRow1, nCopyCol,nRow2, nTab, IDF_CONTENTS );
                pDoc->DoColResize( nTab, nCopyCol, nCopyCol, 0 );
            }
            pDoc->SetAutoCalc( bOldAutoCalc );
            bRedoFilled = TRUE;
        }
    }
    BOOL bMoveCells = pUndoDBData && pRedoDBData &&
                        pRedoDBData->IsDoSize();        // in alt und neu gleich
    if (bMoveCells)
    {
        //  Undo: erst die neuen Daten loeschen, dann FitBlock rueckwaerts

        ScRange aOld, aNew;
        pUndoDBData->GetArea( aOld );
        pRedoDBData->GetArea( aNew );

        pDoc->DeleteAreaTab( aNew.aStart.Col(), aNew.aStart.Row(),
                                aNew.aEnd.Col(), aNew.aEnd.Row(), nTab, IDF_ALL );

        aOld.aEnd.SetCol( aOld.aEnd.Col() + nFormulaCols );     // FitBlock auch fuer Formeln
        aNew.aEnd.SetCol( aNew.aEnd.Col() + nFormulaCols );
        pDoc->FitBlock( aNew, aOld, FALSE );                    // rueckwaerts
    }
    else
        pDoc->DeleteAreaTab( aImportParam.nCol1,aImportParam.nRow1,
                                nEndCol,nEndRow, nTab, IDF_ALL );

    pUndoDoc->CopyToDocument( aImportParam.nCol1,aImportParam.nRow1,nTab,
                                nEndCol+nFormulaCols,nEndRow,nTab,
                                IDF_ALL, FALSE, pDoc );

    if (pCurrentData)
    {
        *pCurrentData = *pUndoDBData;

        pUndoDBData->GetArea( nTable, nCol1, nRow1, nCol2, nRow2 );
        ScUndoUtil::MarkSimpleBlock( pDocShell, nCol1, nRow1, nTable, nCol2, nRow2, nTable );
    }

// erack! it's broadcasted
//  pDoc->SetDirty();

    USHORT nVisTab = pViewShell->GetViewData()->GetTabNo();
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

    USHORT nTable, nCol1, nRow1, nCol2, nRow2;
    ScDBData* pCurrentData = NULL;
    if (pUndoDBData && pRedoDBData)
    {
        pUndoDBData->GetArea( nTable, nCol1, nRow1, nCol2, nRow2 );
        pCurrentData = ScUndoUtil::GetOldDBData( pUndoDBData, pDoc, nTab,
                                                    nCol1, nRow1, nCol2, nRow2 );
    }
    BOOL bMoveCells = pUndoDBData && pRedoDBData &&
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
                                aNew.aEnd.Col(), aNew.aEnd.Row(), nTab, IDF_ALL );

        pRedoDoc->CopyToDocument( aNew, IDF_ALL, FALSE, pDoc );     // incl. Formeln
    }
    else
    {
        pDoc->DeleteAreaTab( aImportParam.nCol1,aImportParam.nRow1,
                                nEndCol,nEndRow, nTab, IDF_ALL );
        pRedoDoc->CopyToDocument( aImportParam.nCol1,aImportParam.nRow1,nTab,
                                nEndCol,nEndRow,nTab, IDF_ALL, FALSE, pDoc );
    }

    if (pCurrentData)
    {
        *pCurrentData = *pRedoDBData;

        pRedoDBData->GetArea( nTable, nCol1, nRow1, nCol2, nRow2 );
        ScUndoUtil::MarkSimpleBlock( pDocShell, nCol1, nRow1, nTable, nCol2, nRow2, nTable );
    }

// erack! it's broadcasted
//  pDoc->SetDirty();

    USHORT nVisTab = pViewShell->GetViewData()->GetTabNo();
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

        USHORT nDummy;
        ScImportParam aNewParam(aImportParam);
        ScDBData* pDBData = rViewShell.GetDBData();
        pDBData->GetArea( nDummy, aNewParam.nCol1,aNewParam.nRow1, aNewParam.nCol2,aNewParam.nRow2 );

        rViewShell.ImportData( aNewParam );
    }
}

BOOL __EXPORT ScUndoImportData::CanRepeat(SfxRepeatTarget& rTarget) const
{
    //  Repeat nur fuer Import per DB-Bereich, dann ist pUndoDBData gesetzt

    if (pUndoDBData)
        return (rTarget.ISA(ScTabViewTarget));
    else
        return FALSE;       // Adressbuch
}

//
//      Operationen wiederholen
//

ScUndoRepeatDB::ScUndoRepeatDB( ScDocShell* pNewDocShell, USHORT nNewTab,
                                USHORT nStartX, USHORT nStartY, USHORT nEndX, USHORT nEndY,
                                USHORT nResultEndRow, USHORT nCurX, USHORT nCurY,
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
    bQuerySize( FALSE )
{
    if ( pOldQ && pNewQ )
    {
        aOldQuery = *pOldQ;
        aNewQuery = *pNewQ;
        bQuerySize = TRUE;;
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
    USHORT nTab = aBlockStart.GetTab();

    if (bQuerySize)
    {
        pDoc->FitBlock( aNewQuery, aOldQuery, FALSE );

        if ( aNewQuery.aEnd.Col() == aOldQuery.aEnd.Col() )
        {
            USHORT nFormulaCols = 0;
            USHORT nCol = aOldQuery.aEnd.Col() + 1;
            USHORT nRow = aOldQuery.aStart.Row() + 1;       //! Header testen
            while ( nCol <= MAXCOL &&
                    pDoc->GetCellType(ScAddress( nCol, nRow, nTab )) == CELLTYPE_FORMULA )
                ++nCol, ++nFormulaCols;

            if ( nFormulaCols )
            {
                ScRange aOldForm = aOldQuery;
                aOldForm.aStart.SetCol( aOldQuery.aEnd.Col() + 1 );
                aOldForm.aEnd.SetCol( aOldQuery.aEnd.Col() + nFormulaCols );
                ScRange aNewForm = aOldForm;
                aNewForm.aEnd.SetRow( aNewQuery.aEnd.Row() );
                pDoc->FitBlock( aNewForm, aOldForm, FALSE );
            }
        }
    }

    //!     Daten von Filter in anderen Bereich fehlen noch !!!!!!!!!!!!!!!!!

    if (nNewEndRow > aBlockEnd.GetRow())
    {
        pDoc->DeleteRow( 0,nTab, MAXCOL,nTab, aBlockEnd.GetRow()+1, nNewEndRow-aBlockEnd.GetRow() );
    }
    else if (nNewEndRow < aBlockEnd.GetRow())
    {
        pDoc->InsertRow( 0,nTab, MAXCOL,nTab, nNewEndRow+1, nNewEndRow-aBlockEnd.GetRow() );
    }

    //  Original Outline-Table

    pDoc->SetOutlineTable( nTab, pUndoTable );

    //  Original Spalten-/Zeilenstatus

    if (pUndoDoc && pUndoTable)
    {
        USHORT nStartCol;
        USHORT nStartRow;
        USHORT nEndCol;
        USHORT nEndRow;
        pUndoTable->GetColArray()->GetRange( nStartCol, nEndCol );
        pUndoTable->GetRowArray()->GetRange( nStartRow, nEndRow );

        pUndoDoc->CopyToDocument( nStartCol, 0, nTab, nEndCol, MAXROW, nTab, IDF_NONE, FALSE, pDoc );
        pUndoDoc->CopyToDocument( 0, nStartRow, nTab, MAXCOL, nEndRow, nTab, IDF_NONE, FALSE, pDoc );

        pViewShell->UpdateScrollBars();
    }

    //  Original-Daten & Referenzen

    ScUndoUtil::MarkSimpleBlock( pDocShell, 0, aBlockStart.GetRow(), nTab,
                                            MAXCOL, aBlockEnd.GetRow(), nTab );
    pDoc->DeleteAreaTab( 0, aBlockStart.GetRow(),
                            MAXCOL, aBlockEnd.GetRow(), nTab, IDF_ALL );

    pUndoDoc->CopyToDocument( 0, aBlockStart.GetRow(), nTab, MAXCOL, aBlockEnd.GetRow(), nTab,
                                                            IDF_NONE, FALSE, pDoc );            // Flags
    pUndoDoc->UndoToDocument( 0, aBlockStart.GetRow(), nTab, MAXCOL, aBlockEnd.GetRow(), nTab,
                                                            IDF_ALL, FALSE, pDoc );

    ScUndoUtil::MarkSimpleBlock( pDocShell, aBlockStart.GetCol(),aBlockStart.GetRow(),nTab,
                                            aBlockEnd.GetCol(),aBlockEnd.GetRow(),nTab );

    if (pUndoRange)
        pDoc->SetRangeName( new ScRangeName( *pUndoRange ) );
    if (pUndoDB)
        pDoc->SetDBCollection( new ScDBCollection( *pUndoDB ) );

// erack! it's broadcasted
//  pDoc->SetDirty();

    USHORT nVisTab = pViewShell->GetViewData()->GetTabNo();
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
    USHORT nTab = aBlockStart.GetTab();

    USHORT nVisTab = pViewShell->GetViewData()->GetTabNo();
    if ( nVisTab != nTab )
        pViewShell->SetTabNo( nTab );

    ScUndoUtil::MarkSimpleBlock( pDocShell, aBlockStart.GetCol(),aBlockStart.GetRow(),nTab,
                                            aBlockEnd.GetCol(),aBlockEnd.GetRow(),nTab );
    pViewShell->SetCursor( aCursorPos.GetCol(), aCursorPos.GetRow() );

    pViewShell->RepeatDB( FALSE );

    EndRedo();
}

void __EXPORT ScUndoRepeatDB::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->RepeatDB( TRUE );
}

BOOL __EXPORT ScUndoRepeatDB::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

//
//      Pivot-Tabellen
//

ScUndoPivot::ScUndoPivot( ScDocShell* pNewDocShell,
                            const ScArea& rOld, const ScArea& rNew,
                            ScDocument* pOldDoc, ScDocument* pNewDoc,
                            const ScPivot* pOldPivot, const ScPivot* pNewPivot ) :
    ScSimpleUndo( pNewDocShell ),
    aOldArea( rOld ),
    aNewArea( rNew ),
    pOldUndoDoc( pOldDoc ),
    pNewUndoDoc( pNewDoc )
{
    if (pNewPivot)
    {
        pNewPivot->GetParam( aNewParam, aNewQuery, aNewSrc );
        aNewName = pNewPivot->GetName();
        aNewTag = pNewPivot->GetTag();
    }
    if (pOldPivot)
    {
        pOldPivot->GetParam( aOldParam, aOldQuery, aOldSrc );
        aOldName = pOldPivot->GetName();
        aOldTag = pOldPivot->GetTag();
    }
}

__EXPORT ScUndoPivot::~ScUndoPivot()
{
    delete pOldUndoDoc;
    delete pNewUndoDoc;
}

String __EXPORT ScUndoPivot::GetComment() const
{
    USHORT nIndex;
    if ( pOldUndoDoc && pNewUndoDoc )
        nIndex = STR_UNDO_PIVOT_MODIFY;
    else if ( pNewUndoDoc )
        nIndex = STR_UNDO_PIVOT_NEW;
    else
        nIndex = STR_UNDO_PIVOT_DELETE;

    return ScGlobal::GetRscString( nIndex );
}

void __EXPORT ScUndoPivot::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();

    if (pNewUndoDoc)
    {
        pDoc->DeleteAreaTab( aNewArea.nColStart,aNewArea.nRowStart,
                            aNewArea.nColEnd,aNewArea.nRowEnd, aNewArea.nTab, IDF_ALL );
        pNewUndoDoc->CopyToDocument( aNewArea.nColStart, aNewArea.nRowStart, aNewArea.nTab,
                                aNewArea.nColEnd, aNewArea.nRowEnd, aNewArea.nTab,
                                IDF_ALL, FALSE, pDoc );
    }
    if (pOldUndoDoc)
    {
        pDoc->DeleteAreaTab( aOldArea.nColStart,aOldArea.nRowStart,
                            aOldArea.nColEnd,aOldArea.nRowEnd, aOldArea.nTab, IDF_ALL );
        pOldUndoDoc->CopyToDocument( aOldArea.nColStart, aOldArea.nRowStart, aOldArea.nTab,
                                aOldArea.nColEnd, aOldArea.nRowEnd, aOldArea.nTab,
                                IDF_ALL, FALSE, pDoc );
    }

    ScPivotCollection* pPivotCollection = pDoc->GetPivotCollection();
    if ( pNewUndoDoc )
    {
        ScPivot* pNewPivot = pPivotCollection->GetPivotAtCursor(
                                aNewParam.nCol, aNewParam.nRow, aNewParam.nTab );
        if (pNewPivot)
            pPivotCollection->Free( pNewPivot );
    }
    if ( pOldUndoDoc )
    {
        ScPivot* pOldPivot = new ScPivot( pDoc );
        pOldPivot->SetParam( aOldParam, aOldQuery, aOldSrc );
        pOldPivot->SetName( aOldName );
        pOldPivot->SetTag( aOldTag );
        if (pOldPivot->CreateData())                            // Felder berechnen
            pOldPivot->ReleaseData();
        pPivotCollection->Insert( pOldPivot );
    }

// erack! it's broadcasted
//  pDoc->SetDirty();
    if (pNewUndoDoc)
        pDocShell->PostPaint( aNewArea.nColStart, aNewArea.nRowStart, aNewArea.nTab,
                                aNewArea.nColEnd, aNewArea.nRowEnd, aNewArea.nTab,
                                PAINT_GRID, SC_PF_LINES );
    if (pOldUndoDoc)
        pDocShell->PostPaint( aOldArea.nColStart, aOldArea.nRowStart, aOldArea.nTab,
                                aOldArea.nColEnd, aOldArea.nRowEnd, aOldArea.nTab,
                                PAINT_GRID, SC_PF_LINES );
    pDocShell->PostDataChanged();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        USHORT nTab = pViewShell->GetViewData()->GetTabNo();
        if ( pOldUndoDoc )
        {
            if ( nTab != aOldArea.nTab )
                pViewShell->SetTabNo( aOldArea.nTab );
        }
        else if ( pNewUndoDoc )
        {
            if ( nTab != aNewArea.nTab )
                pViewShell->SetTabNo( aNewArea.nTab );
        }
    }

    EndUndo();
}

void __EXPORT ScUndoPivot::Redo()
{
    BeginRedo();

    ScDocument* pDoc = pDocShell->GetDocument();
    ScPivotCollection* pPivotCollection = pDoc->GetPivotCollection();
    ScPivot* pOldPivot = pPivotCollection->GetPivotAtCursor(
                                            aOldParam.nCol, aOldParam.nRow, aOldParam.nTab );

    ScPivot* pNewPivot = NULL;
    if (pNewUndoDoc)
    {
        pNewPivot = new ScPivot( pDoc );
        pNewPivot->SetParam( aNewParam, aNewQuery, aNewSrc );
        pNewPivot->SetName( aNewName );
        pNewPivot->SetTag( aNewTag );
    }

    pDocShell->PivotUpdate( pOldPivot, pNewPivot, FALSE );

    EndRedo();
}

void __EXPORT ScUndoPivot::Repeat(SfxRepeatTarget& rTarget)
{
    //  Wiederholen: nur loeschen

    if ( pOldUndoDoc && !pNewUndoDoc && rTarget.ISA(ScTabViewTarget) )
        ((ScTabViewTarget&)rTarget).GetViewShell()->DeletePivotTable();
}

BOOL __EXPORT ScUndoPivot::CanRepeat(SfxRepeatTarget& rTarget) const
{
    //  Wiederholen: nur loeschen

    return ( pOldUndoDoc && !pNewUndoDoc && rTarget.ISA(ScTabViewTarget) );
}


//
//      data pilot
//

ScUndoDataPilot::ScUndoDataPilot( ScDocShell* pNewDocShell,
                            ScDocument* pOldDoc, ScDocument* pNewDoc,
                            const ScDPObject* pOldObj, const ScDPObject* pNewObj ) :
    ScSimpleUndo( pNewDocShell ),
    pOldDPObject( NULL ),
    pNewDPObject( NULL ),
    pOldUndoDoc( pOldDoc ),
    pNewUndoDoc( pNewDoc )
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
    USHORT nIndex;
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
        pNewUndoDoc->CopyToDocument( aNewRange, IDF_ALL, FALSE, pDoc );
    }
    if ( pOldDPObject && pOldUndoDoc )
    {
        aOldRange = pOldDPObject->GetOutRange();
        pDoc->DeleteAreaTab( aOldRange, IDF_ALL );
        pOldUndoDoc->CopyToDocument( aOldRange, IDF_ALL, FALSE, pDoc );
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
            }
            else
            {
                //  delete inserted object
                pDoc->GetDPCollection()->Free(pDocObj);
            }
        }
    }
    else if ( pOldDPObject )
    {
        //  re-insert deleted object

        ScDPObject* pDestObj = new ScDPObject( *pOldDPObject );
        pDestObj->SetAlive(TRUE);
        if ( !pDoc->GetDPCollection()->Insert(pDestObj) )
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
    aFunc.DataPilotUpdate( pSourceObj, pNewDPObject, FALSE, FALSE );    // no new undo action

    EndRedo();
}

void __EXPORT ScUndoDataPilot::Repeat(SfxRepeatTarget& rTarget)
{
    //! allow deletion
}

BOOL __EXPORT ScUndoDataPilot::CanRepeat(SfxRepeatTarget& rTarget) const
{
    //! allow deletion
    return FALSE;
}


//
//      Konsolidieren
//

ScUndoConsolidate::ScUndoConsolidate( ScDocShell* pNewDocShell, const ScArea& rArea,
                    const ScConsolidateParam& rPar, ScDocument* pNewUndoDoc,
                    BOOL bReference, USHORT nInsCount, ScOutlineTable* pTab,
                    ScDBData* pData ) :
    ScSimpleUndo( pNewDocShell ),
    aDestArea( rArea ),
    aParam( rPar ),
    pUndoDoc( pNewUndoDoc ),
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
    USHORT nTab = aDestArea.nTab;

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
        pUndoDoc->CopyToDocument( 0,0,nTab, MAXCOL,MAXROW,nTab, IDF_NONE, FALSE, pDoc );

        //  Daten & Referenzen
        pDoc->DeleteAreaTab( 0,aDestArea.nRowStart, MAXCOL,aDestArea.nRowEnd, nTab, IDF_ALL );
        pUndoDoc->UndoToDocument( 0, aDestArea.nRowStart, nTab,
                                    MAXCOL, aDestArea.nRowEnd, nTab,
                                    IDF_ALL, FALSE, pDoc );

        //  Original-Bereich
        if (pUndoData)
        {
            pDoc->DeleteAreaTab(aOldRange, IDF_ALL);
            pUndoDoc->CopyToDocument(aOldRange, IDF_ALL, FALSE, pDoc);
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
                                    IDF_ALL, FALSE, pDoc );

        //  Original-Bereich
        if (pUndoData)
        {
            pDoc->DeleteAreaTab(aOldRange, IDF_ALL);
            pUndoDoc->CopyToDocument(aOldRange, IDF_ALL, FALSE, pDoc);
        }

        USHORT nEndX = aDestArea.nColEnd;
        USHORT nEndY = aDestArea.nRowEnd;
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
            USHORT nIndex;
            if (pColl->SearchName( pUndoData->GetName(), nIndex ))
            {
                ScDBData* pDocData = (*pColl)[nIndex];
                if (pDocData)
                    *pDocData = *pUndoData;
            }
            else
                DBG_ERROR("alte DB-Daten nicht gefunden");
        }
    }

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        USHORT nViewTab = pViewShell->GetViewData()->GetTabNo();
        if ( nViewTab != nTab )
            pViewShell->SetTabNo( nTab );
    }

    EndUndo();
}

void __EXPORT ScUndoConsolidate::Redo()
{
    BeginRedo();

    pDocShell->DoConsolidate( aParam, FALSE );

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        USHORT nViewTab = pViewShell->GetViewData()->GetTabNo();
        if ( nViewTab != aParam.nTab )
            pViewShell->SetTabNo( aParam.nTab );
    }

    EndRedo();
}

void __EXPORT ScUndoConsolidate::Repeat(SfxRepeatTarget& rTarget)
{
}

BOOL __EXPORT ScUndoConsolidate::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return FALSE;
}


//
//      Quell-Daten von Chart aendern
//

void ScUndoChartData::Init()
{
    ScDocument* pDoc = pDocShell->GetDocument();
    BOOL bFound = FALSE;;

    SchMemChart* pOld = pDoc->FindChartData(aChartName);
    if (pOld)
    {
        ScChartArray aData(pDoc,*pOld);
        aOldRangeListRef = aData.GetRangeList();
        bOldColHeaders = aData.HasColHeaders();
        bOldRowHeaders = aData.HasRowHeaders();
    }
    else                                //  war vorher nicht initialisiert
    {                                   //! bei Undo zuruecksetzen ?
        aOldRangeListRef = aNewRangeListRef;
        bOldColHeaders = bNewColHeaders;
        bOldRowHeaders = bNewRowHeaders;
    }
}

ScUndoChartData::ScUndoChartData( ScDocShell* pNewDocShell, const String& rName,
                                    const ScRange& rNew, BOOL bColHdr, BOOL bRowHdr,
                                    BOOL bAdd ) :
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
                                    const ScRangeListRef& rNew, BOOL bColHdr, BOOL bRowHdr,
                                    BOOL bAdd ) :
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

    Window* pDataWin = NULL;
    ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
    if (pViewSh)
        pDataWin = pViewSh->GetActiveWin();

    pDocShell->GetDocument()->UpdateChartArea( aChartName, aOldRangeListRef,
                                bOldColHeaders, bOldRowHeaders, FALSE, pDataWin );

    EndUndo();
}

void __EXPORT ScUndoChartData::Redo()
{
    BeginRedo();

    Window* pDataWin = NULL;
    ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
    if (pViewSh)
        pDataWin = pViewSh->GetActiveWin();

    pDocShell->GetDocument()->UpdateChartArea( aChartName, aNewRangeListRef,
                                bNewColHeaders, bNewRowHeaders, bAddRange, pDataWin );

    EndRedo();
}

void __EXPORT ScUndoChartData::Repeat(SfxRepeatTarget& rTarget)
{
}

BOOL __EXPORT ScUndoChartData::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return FALSE;
}






