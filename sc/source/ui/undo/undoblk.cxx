/*************************************************************************
 *
 *  $RCSfile: undoblk.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: nn $ $Date: 2001-03-27 08:47:33 $
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

//------------------------------------------------------------------

#define _MACRODLG_HXX
#define _BIGINT_HXX
#define _SVDXOUT_HXX
#define _SVDATTR_HXX
#define _SVDSURO_HXX

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <vcl/virdev.hxx>
#include <vcl/waitobj.hxx>
#include <svx/boxitem.hxx>

#include "undoblk.hxx"
#include "undoutil.hxx"
#include "document.hxx"
#include "patattr.hxx"
#include "docsh.hxx"
#include "tabvwsh.hxx"
#include "rangenam.hxx"
#include "rangeutl.hxx"
#include "dbcolect.hxx"
#include "stlpool.hxx"
#include "stlsheet.hxx"
#include "globstr.hrc"
#include "global.hxx"
#include "target.hxx"
#include "docpool.hxx"
#include "docfunc.hxx"
#include "attrib.hxx"
#include "chgtrack.hxx"
#include "transobj.hxx"


// STATIC DATA -----------------------------------------------------------

TYPEINIT1(ScUndoInsertCells,        SfxUndoAction);
TYPEINIT1(ScUndoDeleteCells,        SfxUndoAction);
TYPEINIT1(ScUndoDeleteMulti,        SfxUndoAction);
TYPEINIT1(ScUndoCut,                ScBlockUndo);
TYPEINIT1(ScUndoPaste,              SfxUndoAction);
TYPEINIT1(ScUndoDragDrop,           SfxUndoAction);
TYPEINIT1(ScUndoListNames,          SfxUndoAction);
TYPEINIT1(ScUndoUseScenario,        SfxUndoAction);
TYPEINIT1(ScUndoSelectionStyle,     SfxUndoAction);
TYPEINIT1(ScUndoEnterMatrix,        ScBlockUndo);
TYPEINIT1(ScUndoIndent,             ScBlockUndo);
TYPEINIT1(ScUndoTransliterate,      ScBlockUndo);
TYPEINIT1(ScUndoClearItems,         ScBlockUndo);
TYPEINIT1(ScUndoRemoveBreaks,       SfxUndoAction);
TYPEINIT1(ScUndoRemoveMerge,        ScBlockUndo);
TYPEINIT1(ScUndoBorder,             ScBlockUndo);



// To Do:
/*A*/   // SetOptimalHeight auf Dokument, wenn keine View
/*B*/   // gelinkte Tabellen
/*C*/   // ScArea
//?     // spaeter mal pruefen


// -----------------------------------------------------------------------
//
//      Zellen einfuegen
//      Zeilen einfügen
//      einzeln oder Block
//

ScUndoInsertCells::ScUndoInsertCells( ScDocShell* pNewDocShell,
                                const ScRange& rRange, InsCellCmd eNewCmd,
                                ScDocument* pUndoDocument, ScRefUndoData* pRefData ) :
    ScMoveUndo( pNewDocShell, pUndoDocument, pRefData, SC_UNDO_REFLAST ),
    aEffRange( rRange ),
    eCmd( eNewCmd )
{
    if (eCmd == INS_INSROWS)            // ganze Zeilen?
    {
        aEffRange.aStart.SetCol(0);
        aEffRange.aEnd.SetCol(MAXCOL);
    }

    if (eCmd == INS_INSCOLS)            // ganze Spalten?
    {
        aEffRange.aStart.SetRow(0);
        aEffRange.aEnd.SetRow(MAXROW);
    }

    SetChangeTrack();
}

__EXPORT ScUndoInsertCells::~ScUndoInsertCells()
{
}

String __EXPORT ScUndoInsertCells::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_INSERTCELLS ); // "Einfuegen"
}

void ScUndoInsertCells::SetChangeTrack()
{
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
    if ( pChangeTrack )
    {
        pChangeTrack->AppendInsert( aEffRange );
        nEndChangeAction = pChangeTrack->GetActionMax();
    }
    else
        nEndChangeAction = 0;
}

void ScUndoInsertCells::DoChange( const BOOL bUndo )
{
    ScDocument* pDoc = pDocShell->GetDocument();

    if ( bUndo )
    {
        ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
        if ( pChangeTrack )
            pChangeTrack->Undo( nEndChangeAction, nEndChangeAction );
    }
    else
        SetChangeTrack();

    ScRange aWorkRange( aEffRange );
    if ( pDoc->HasAttrib( aWorkRange, HASATTR_MERGED ) )    // zusammengefasste Zellen?
        pDoc->ExtendMerge( aWorkRange, TRUE );

    switch (eCmd)
    {
        case INS_INSROWS:
        case INS_CELLSDOWN:
            if (bUndo)
                pDoc->DeleteRow( aEffRange );
            else
                pDoc->InsertRow( aEffRange );
            break;
        case INS_INSCOLS:
        case INS_CELLSRIGHT:
            if (bUndo)
                pDoc->DeleteCol( aEffRange );
            else
                pDoc->InsertCol( aEffRange );
            break;
    }

//? Undo fuer herausgeschobene Attribute ?

    USHORT nPaint = PAINT_GRID;
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    switch (eCmd)
    {
        case INS_INSROWS:
            nPaint |= PAINT_LEFT;
            aWorkRange.aEnd.SetRow(MAXROW);
            break;
        case INS_CELLSDOWN:
            aWorkRange.aEnd.SetRow(MAXROW);     // bis ganz nach unten
/*A*/       if ( (pViewShell) && pViewShell->AdjustRowHeight(
                    aWorkRange.aStart.Row(), aWorkRange.aEnd.Row(), FALSE ) )
            {
                aWorkRange.aStart.SetCol(0);
                aWorkRange.aEnd.SetCol(MAXCOL);
                nPaint |= PAINT_LEFT;
            }
            break;
        case INS_INSCOLS:
            nPaint |= PAINT_TOP;                // obere Leiste
        case INS_CELLSRIGHT:
            aWorkRange.aEnd.SetCol(MAXCOL);     // bis ganz nach rechts
/*A*/       if ( (pViewShell) && pViewShell->AdjustRowHeight(
                    aWorkRange.aStart.Row(), aWorkRange.aEnd.Row(), FALSE ) )
            {                                   // AdjustDraw zeichnet PAINT_TOP nicht,
                aWorkRange.aStart.SetCol(0);    // daher so geloest
                aWorkRange.aEnd.SetRow(MAXROW);
                nPaint |= PAINT_LEFT;
            }
            break;
    }
    pDocShell->PostPaint( aWorkRange, nPaint );
    pDocShell->PostDataChanged();
    if (pViewShell)
        pViewShell->CellContentChanged();

    ShowTable( aEffRange.aStart.Tab() );
}

void __EXPORT ScUndoInsertCells::Undo()
{
    WaitObject aWait( pDocShell->GetDialogParent() );       // wichtig wegen TrackFormulas bei UpdateReference
    BeginUndo();
    DoChange( TRUE );
    EndUndo();
}

void __EXPORT ScUndoInsertCells::Redo()
{
    WaitObject aWait( pDocShell->GetDialogParent() );       // wichtig wegen TrackFormulas bei UpdateReference
    BeginRedo();
    DoChange( FALSE );
    EndRedo();
}

void __EXPORT ScUndoInsertCells::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->InsertCells( eCmd, TRUE );
}

BOOL __EXPORT ScUndoInsertCells::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}


// -----------------------------------------------------------------------
//
//      Zellen loeschen
//      Zeilen löschen
//      einzeln oder Block
//

ScUndoDeleteCells::ScUndoDeleteCells( ScDocShell* pNewDocShell,
                                const ScRange& rRange, DelCellCmd eNewCmd,
                                ScDocument* pUndoDocument, ScRefUndoData* pRefData ) :
    ScMoveUndo( pNewDocShell, pUndoDocument, pRefData, SC_UNDO_REFLAST ),
    aEffRange( rRange ),
    eCmd( eNewCmd )
{
    if (eCmd == DEL_DELROWS)            // gaze Zeilen?
    {
        aEffRange.aStart.SetCol(0);
        aEffRange.aEnd.SetCol(MAXCOL);
    }

    if (eCmd == DEL_DELCOLS)            // ganze Spalten?
    {
        aEffRange.aStart.SetRow(0);
        aEffRange.aEnd.SetRow(MAXROW);
    }

    SetChangeTrack();
}

__EXPORT ScUndoDeleteCells::~ScUndoDeleteCells()
{
}

String __EXPORT ScUndoDeleteCells::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_DELETECELLS ); // "Loeschen"
}

void ScUndoDeleteCells::SetChangeTrack()
{
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->AppendDeleteRange( aEffRange, pRefUndoDoc,
            nStartChangeAction, nEndChangeAction );
    else
        nStartChangeAction = nEndChangeAction = 0;
}

void ScUndoDeleteCells::DoChange( const BOOL bUndo )
{
    ScDocument* pDoc = pDocShell->GetDocument();

    if ( bUndo )
    {
        ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
        if ( pChangeTrack )
            pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );
    }
    else
        SetChangeTrack();

    // Ausfuehren
    switch (eCmd)
    {
        case DEL_DELROWS:
        case DEL_CELLSUP:
            if (bUndo)
                pDoc->InsertRow( aEffRange );
            else
                pDoc->DeleteRow( aEffRange );
            break;
        case DEL_DELCOLS:
        case DEL_CELLSLEFT:
            if (bUndo)
                pDoc->InsertCol( aEffRange );
            else
                pDoc->DeleteCol( aEffRange );
            break;
    }

    // bei Undo Referenzen wiederherstellen
    if (bUndo)
        pRefUndoDoc->CopyToDocument( aEffRange, IDF_ALL, FALSE, pDoc );

//? Datenbank muss vor ExtendMerge sein ?????

    // Zusammengefasste Zellen?
    ScRange aWorkRange( aEffRange );
    if ( pDoc->HasAttrib( aWorkRange, HASATTR_MERGED ) )
    {
/*?     if ( !bUndo && ( eCmd==DEL_DELCOLS || eCmd==DEL_DELROWS ) )
        {
            if (eCmd==DEL_DELCOLS) aWorkRange.aEnd.SetCol(MAXCOL);
            if (eCmd==DEL_DELROWS) aWorkRange.aEnd.SetRow(MAXROW);
            ScMarkData aMarkData;
            aMarkData.SelectOneTable( aWorkRange.aStart.Tab() );
            ScPatternAttr aPattern( pDoc->GetPool() );
            aPattern.GetItemSet().Put( ScMergeFlagAttr() );
            pDoc->ApplyPatternArea( aWorkRange.aStart.Col(), aWorkRange.aStart.Row(),
                                    aWorkRange.aEnd.Col(),   aWorkRange.aEnd.Row(),
                                    aMarkData, aPattern );
        }
?*/
        pDoc->ExtendMerge( aWorkRange, TRUE );
    }

    // Zeichnen
    USHORT nPaint = PAINT_GRID;
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    switch (eCmd)
    {
        case DEL_DELROWS:
            nPaint |= PAINT_LEFT;
            aWorkRange.aEnd.SetRow(MAXROW);
            break;
        case DEL_CELLSUP:
            aWorkRange.aEnd.SetRow(MAXROW);
/*A*/       if ( (pViewShell) && pViewShell->AdjustRowHeight(
                    aWorkRange.aStart.Row(), aWorkRange.aEnd.Row(), FALSE   ) )
            {
                aWorkRange.aStart.SetCol(0);
                aWorkRange.aEnd.SetCol(MAXCOL);
                nPaint |= PAINT_LEFT;
            }
            break;
        case DEL_DELCOLS:
            nPaint |= PAINT_TOP;                // obere Leiste
        case DEL_CELLSLEFT:
            aWorkRange.aEnd.SetCol(MAXCOL);     // bis ganz nach rechts
/*A*/       if ( (pViewShell) && pViewShell->AdjustRowHeight(
                    aWorkRange.aStart.Row(), aWorkRange.aEnd.Row(), FALSE ) )
            {
                aWorkRange.aStart.SetCol(0);
                aWorkRange.aEnd.SetRow(MAXROW);
                nPaint |= PAINT_LEFT;
            }
    }
    pDocShell->PostPaint( aWorkRange, nPaint, SC_PF_LINES );    //! auf Lines testen

    // Markierung erst nach EndUndo

    pDocShell->PostDataChanged();
    //  CellContentChanged kommt mit der Markierung

    ShowTable( aEffRange.aStart.Tab() );
}

void __EXPORT ScUndoDeleteCells::Undo()
{
    WaitObject aWait( pDocShell->GetDialogParent() );       // wichtig wegen TrackFormulas bei UpdateReference
    BeginUndo();
    DoChange( TRUE );
    EndUndo();

    // Markierung erst nach EndUndo
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->MarkRange( aEffRange );
}

void __EXPORT ScUndoDeleteCells::Redo()
{
    WaitObject aWait( pDocShell->GetDialogParent() );       // wichtig wegen TrackFormulas bei UpdateReference
    BeginRedo();
    DoChange( FALSE);
    EndRedo();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->DoneBlockMode();            // aktuelle weg
}

void __EXPORT ScUndoDeleteCells::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->DeleteCells( eCmd, TRUE );
}

BOOL __EXPORT ScUndoDeleteCells::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}


// -----------------------------------------------------------------------
//
//      Zellen loeschen auf Mehrfachselektion
//

ScUndoDeleteMulti::ScUndoDeleteMulti( ScDocShell* pNewDocShell,
                                        BOOL bNewRows, USHORT nNewTab,
                                        const USHORT* pRng, USHORT nRngCnt,
                                        ScDocument* pUndoDocument, ScRefUndoData* pRefData ) :
    ScMoveUndo( pNewDocShell, pUndoDocument, pRefData, SC_UNDO_REFLAST ),
    bRows( bNewRows ),
    nTab( nNewTab ),
    nRangeCnt( nRngCnt )
{
    pRanges = new USHORT[ 2 * nRangeCnt ];
    memcpy(pRanges,pRng,nRangeCnt*2*sizeof(USHORT));
    SetChangeTrack();
}

__EXPORT ScUndoDeleteMulti::~ScUndoDeleteMulti()
{
    delete pRanges;
}

String __EXPORT ScUndoDeleteMulti::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_DELETECELLS );  // wie DeleteCells
}

void ScUndoDeleteMulti::DoChange() const
{
    if (bRows)
        pDocShell->PostPaint( 0,pRanges[0],nTab, MAXCOL,MAXROW,nTab, PAINT_GRID | PAINT_LEFT );
    else
        pDocShell->PostPaint( pRanges[0],0,nTab, MAXCOL,MAXROW,nTab, PAINT_GRID | PAINT_TOP );

    pDocShell->PostDataChanged();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->CellContentChanged();

    ShowTable( nTab );
}

void ScUndoDeleteMulti::SetChangeTrack()
{
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
    if ( pChangeTrack )
    {
        nStartChangeAction = pChangeTrack->GetActionMax() + 1;
        ScRange aRange( 0, 0, nTab, 0, 0, nTab );
        if ( bRows )
            aRange.aEnd.SetCol( MAXCOL );
        else
            aRange.aEnd.SetRow( MAXROW );
        // rueckwaerts loeschen
        USHORT* pOneRange = &pRanges[2*nRangeCnt];
        for ( USHORT nRangeNo=0; nRangeNo<nRangeCnt; nRangeNo++ )
        {
            USHORT nEnd = *(--pOneRange);
            USHORT nStart = *(--pOneRange);
            if ( bRows )
            {
                aRange.aStart.SetRow( nStart );
                aRange.aEnd.SetRow( nEnd );
            }
            else
            {
                aRange.aStart.SetCol( nStart );
                aRange.aEnd.SetCol( nEnd );
            }
            ULONG nDummyStart;
            pChangeTrack->AppendDeleteRange( aRange, pRefUndoDoc,
                nDummyStart, nEndChangeAction );
        }
    }
    else
        nStartChangeAction = nEndChangeAction = 0;
}

void __EXPORT ScUndoDeleteMulti::Undo()
{
    WaitObject aWait( pDocShell->GetDialogParent() );       // wichtig wegen TrackFormulas bei UpdateReference
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    USHORT* pOneRange;
    USHORT nRangeNo;

    //  rueckwaerts geloescht -> vorwaerts einfuegen
    pOneRange = pRanges;
    for (nRangeNo=0; nRangeNo<nRangeCnt; nRangeNo++)
    {
        USHORT nStart = *(pOneRange++);
        USHORT nEnd = *(pOneRange++);
        if (bRows)
            pDoc->InsertRow( 0,nTab, MAXCOL,nTab, nStart,nEnd-nStart+1 );
        else
            pDoc->InsertCol( 0,nTab, MAXROW,nTab, nStart,nEnd-nStart+1 );
    }

    pOneRange = pRanges;
    for (nRangeNo=0; nRangeNo<nRangeCnt; nRangeNo++)
    {
        USHORT nStart = *(pOneRange++);
        USHORT nEnd = *(pOneRange++);
        if (bRows)
            pRefUndoDoc->CopyToDocument( 0,nStart,nTab, MAXCOL,nEnd,nTab, IDF_ALL,FALSE,pDoc );
        else
            pRefUndoDoc->CopyToDocument( nStart,0,nTab, nEnd,MAXROW,nTab, IDF_ALL,FALSE,pDoc );
    }

    ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );

    DoChange();

    //! Markierung wieder einzeichnen
    //! geht im Moment nicht, da keine Daten fuer Markierung vorhanden!

    EndUndo();
}

void __EXPORT ScUndoDeleteMulti::Redo()
{
    WaitObject aWait( pDocShell->GetDialogParent() );       // wichtig wegen TrackFormulas bei UpdateReference
    BeginRedo();

    ScDocument* pDoc = pDocShell->GetDocument();

    // rueckwaerts loeschen
    USHORT* pOneRange = &pRanges[2*nRangeCnt];
    for (USHORT nRangeNo=0; nRangeNo<nRangeCnt; nRangeNo++)
    {
        USHORT nEnd = *(--pOneRange);
        USHORT nStart = *(--pOneRange);
        if (bRows)
            pDoc->DeleteRow( 0,nTab, MAXCOL,nTab, nStart,nEnd-nStart+1 );
        else
            pDoc->DeleteCol( 0,nTab, MAXROW,nTab, nStart,nEnd-nStart+1 );
    }

    SetChangeTrack();

    DoChange();

//! Markierung loeschen, derzeit unnoetig (s.o.)
//! ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
//! if (pViewShell)
//!     DoneBlockMode();

    EndRedo();
}

void __EXPORT ScUndoDeleteMulti::Repeat(SfxRepeatTarget& rTarget)
{
    //  DeleteCells, falls einfache Selektion
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->DeleteCells( DEL_DELROWS, TRUE );
}

BOOL __EXPORT ScUndoDeleteMulti::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}


// -----------------------------------------------------------------------
//
//      Ausschneiden (Cut)
//

ScUndoCut::ScUndoCut( ScDocShell* pNewDocShell,
                ScRange aRange, ScAddress aOldEnd,
                ScDocument* pNewUndoDoc ) :
    ScBlockUndo( pNewDocShell, ScRange(aRange.aStart, aOldEnd), SC_UNDO_AUTOHEIGHT ),
    aExtendedRange( aRange ),
    pUndoDoc( pNewUndoDoc )
{
    SetChangeTrack();
}

__EXPORT ScUndoCut::~ScUndoCut()
{
    delete pUndoDoc;
}

String __EXPORT ScUndoCut::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_CUT ); // "Ausschneiden"
}

void ScUndoCut::SetChangeTrack()
{
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->AppendContentRange( aBlockRange, pUndoDoc,
            nStartChangeAction, nEndChangeAction, SC_CACM_CUT );
    else
        nStartChangeAction = nEndChangeAction = 0;
}

void ScUndoCut::DoChange( const BOOL bUndo )
{
    ScDocument* pDoc = pDocShell->GetDocument();
    USHORT nExtFlags = 0;

    if (bUndo)  // nur bei Undo
    {
        pUndoDoc->CopyToDocument( aExtendedRange, IDF_ALL, FALSE, pDoc );
        ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
        if ( pChangeTrack )
            pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );
    }
    else        // nur bei Redo
    {
        if (pDoc->HasAttrib( aExtendedRange, HASATTR_PAINTEXT ))
            nExtFlags = SC_PF_LINES;
        pDoc->DeleteAreaTab( aBlockRange, IDF_ALL );
        SetChangeTrack();
    }

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if ( !( (pViewShell) && pViewShell->AdjustBlockHeight() ) )
/*A*/   pDocShell->PostPaint( aExtendedRange, PAINT_GRID, nExtFlags );

    pDocShell->PostDataChanged();
    if (pViewShell)
        pViewShell->CellContentChanged();
}

void __EXPORT ScUndoCut::Undo()
{
    BeginUndo();
    DoChange( TRUE );
    EndUndo();
}

void __EXPORT ScUndoCut::Redo()
{
    BeginRedo();
    DoChange( FALSE );
    EndRedo();
}

void __EXPORT ScUndoCut::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->CutToClip();
}

BOOL __EXPORT ScUndoCut::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}


// -----------------------------------------------------------------------
//
//      Einfuegen (Paste)
//

ScUndoPaste::ScUndoPaste( ScDocShell* pNewDocShell,
                USHORT nStartX, USHORT nStartY, USHORT nStartZ,
                USHORT nEndX, USHORT nEndY, USHORT nEndZ,
                const ScMarkData& rMark,
                ScDocument* pNewUndoDoc, ScDocument* pNewRedoDoc,
                USHORT nNewFlags,
                ScRangeName* pNewUndoRange, ScRangeName* pNewRedoRange,
                ScDBCollection* pNewUndoDB, ScDBCollection* pNewRedoDB,
                BOOL bRedoIsFilled ) :
    ScSimpleUndo( pNewDocShell ),
    aRange( nStartX, nStartY, nStartZ, nEndX, nEndY, nEndZ ),
    aMarkData( rMark ),
    pUndoDoc( pNewUndoDoc ),
    pRedoDoc( pNewRedoDoc ),
    nFlags( nNewFlags ),
    pUndoRange( pNewUndoRange ),
    pRedoRange( pNewRedoRange ),
    pUndoDB( pNewUndoDB ),
    pRedoDB( pNewRedoDB ),
    bRedoFilled( bRedoIsFilled )
{
    if ( !aMarkData.IsMarked() )            // keine Zelle markiert:
        aMarkData.SetMarkArea( aRange );    //  Paste-Block markieren
    SetChangeTrack();
}

__EXPORT ScUndoPaste::~ScUndoPaste()
{
    delete pUndoDoc;
    delete pRedoDoc;
    delete pUndoRange;
    delete pRedoRange;
    delete pUndoDB;
    delete pRedoDB;
}

String __EXPORT ScUndoPaste::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_PASTE ); // "Einfuegen"
}

void ScUndoPaste::SetChangeTrack()
{
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
    if ( pChangeTrack && (nFlags & IDF_CONTENTS) )
        pChangeTrack->AppendContentRange( aRange, pUndoDoc,
            nStartChangeAction, nEndChangeAction, SC_CACM_PASTE );
    else
        nStartChangeAction = nEndChangeAction = 0;
}

void ScUndoPaste::DoChange( const BOOL bUndo )
{
    ScRangeName*    pWorkRange = bUndo ? pUndoRange : pRedoRange;
    ScDBCollection* pWorkDB    = bUndo ? pUndoDB    : pRedoDB;

        //  fuer Undo immer alle oder keine Inhalte sichern
    USHORT nUndoFlags = IDF_NONE;
    if (nFlags & IDF_CONTENTS)
        nUndoFlags |= IDF_CONTENTS;
    if (nFlags & IDF_ATTRIB)
        nUndoFlags |= IDF_ATTRIB;

    BOOL bPaintAll = FALSE;

    ScDocument* pDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    ScUndoUtil::MarkSimpleBlock( pDocShell, aRange );

    if ( bUndo && !bRedoFilled )
    {
        if (!pRedoDoc)
        {
            BOOL bColInfo = ( aRange.aStart.Row()==0 && aRange.aEnd.Row()==MAXROW );
            BOOL bRowInfo = ( aRange.aStart.Col()==0 && aRange.aEnd.Col()==MAXCOL );
            pRedoDoc = new ScDocument( SCDOCMODE_UNDO );
            pRedoDoc->InitUndo( pDoc, aRange.aStart.Tab(), aRange.aStart.Tab(),
                                bColInfo, bRowInfo );
        }
        //  Redo-Daten beim ersten Undo aus dem Dokument lesen
        pDoc->CopyToDocument( aRange, nUndoFlags, FALSE, pRedoDoc );
        bRedoFilled = TRUE;
    }

    BOOL bHasLines = pDoc->HasAttrib( aRange, HASATTR_PAINTEXT );

    aMarkData.MarkToMulti();
    pDoc->DeleteSelection( nUndoFlags, aMarkData );
    aMarkData.MarkToSimple();

    if ( !bUndo && pRedoDoc )       // Redo: UndoToDocument vorher
        pRedoDoc->UndoToDocument( aRange, nUndoFlags, FALSE, pDoc );

    if (pWorkRange)
        pDoc->SetRangeName( new ScRangeName( *pWorkRange ) );
    if (pWorkDB)
    {
        pDoc->SetDBCollection( new ScDBCollection( *pWorkDB ) );
        if ( pDoc->RefreshAutoFilter( 0,0, MAXCOL,MAXROW, aRange.aStart.Tab() ) )
            bPaintAll = TRUE;
    }

    if (bUndo)      // Undo: UndoToDocument hinterher
        pUndoDoc->UndoToDocument( aRange, nUndoFlags, FALSE, pDoc );

    if ( bUndo )
    {
        ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
        if ( pChangeTrack )
            pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );
    }
    else
        SetChangeTrack();

    ScRange aDrawRange( aRange );
    USHORT nPaint = PAINT_GRID;
    USHORT nPaintExt = 0;
    if (bPaintAll)
    {
        aDrawRange.aStart.SetCol(0);
        aDrawRange.aStart.SetRow(0);
        aDrawRange.aEnd.SetCol(MAXCOL);
        aDrawRange.aEnd.SetRow(MAXROW);
        nPaint |= PAINT_TOP | PAINT_LEFT;
/*A*/   if (pViewShell)
            pViewShell->AdjustBlockHeight(FALSE);
    }
    else
    {
        if ( aRange.aStart.Row() == 0 && aRange.aEnd.Row() == MAXROW )  // ganze Spalte
        {
            nPaint |= PAINT_TOP;
            aDrawRange.aEnd.SetCol(MAXCOL);
        }
        if ( aRange.aStart.Col() == 0 && aRange.aEnd.Col() == MAXCOL )  // ganze Zeile
        {
            nPaint |= PAINT_LEFT;
            aDrawRange.aEnd.SetRow(MAXROW);
        }
/*A*/   if ((pViewShell) && pViewShell->AdjustBlockHeight(FALSE))
        {
            aDrawRange.aStart.SetCol(0);
            aDrawRange.aStart.SetRow(0);
            aDrawRange.aEnd.SetCol(MAXCOL);
            aDrawRange.aEnd.SetRow(MAXROW);
            nPaint |= PAINT_LEFT;
        }
        bHasLines |= pDoc->HasAttrib( aDrawRange, HASATTR_PAINTEXT );
        if (bHasLines) nPaintExt = SC_PF_LINES;
    }
    pDocShell->PostPaint( aDrawRange, nPaint, nPaintExt );

    pDocShell->PostDataChanged();
    if (pViewShell)
        pViewShell->CellContentChanged();
}

void __EXPORT ScUndoPaste::Undo()
{
    BeginUndo();
    DoChange( TRUE );
    ShowTable( aRange );
    EndUndo();
}

void __EXPORT ScUndoPaste::Redo()
{
    BeginRedo();
    DoChange( FALSE );
    EndRedo();
}

void __EXPORT ScUndoPaste::Repeat(SfxRepeatTarget& rTarget)
{
//? Extra-Flags sichern ?

    if (rTarget.ISA(ScTabViewTarget))
    {
        ScTransferObj* pOwnClip = ScTransferObj::GetOwnClipboard();
        if (pOwnClip)
            ((ScTabViewTarget&)rTarget).GetViewShell()->PasteFromClip( nFlags, pOwnClip->GetDocument() );
    }
}

BOOL __EXPORT ScUndoPaste::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}


// -----------------------------------------------------------------------
//
//      Verschieben/Kopieren (Drag & Drop)
//

ScUndoDragDrop::ScUndoDragDrop( ScDocShell* pNewDocShell,
                    const ScRange& rRange, ScAddress aNewDestPos, BOOL bNewCut,
                    ScDocument* pUndoDocument, ScRefUndoData* pRefData, BOOL bScenario ) :
    ScMoveUndo( pNewDocShell, pUndoDocument, pRefData, SC_UNDO_REFFIRST ),
    aSrcRange( rRange ),
    bCut( bNewCut ),
    bKeepScenarioFlags( bScenario )
{
    ScAddress aDestEnd(aNewDestPos);
    aDestEnd.IncRow(aSrcRange.aEnd.Row() - aSrcRange.aStart.Row());
    aDestEnd.IncCol(aSrcRange.aEnd.Col() - aSrcRange.aStart.Col());
    aDestEnd.IncTab(aSrcRange.aEnd.Tab() - aSrcRange.aStart.Tab());

    aDestRange.aStart = aNewDestPos;
    aDestRange.aEnd = aDestEnd;

    SetChangeTrack();
}

__EXPORT ScUndoDragDrop::~ScUndoDragDrop()
{
}

String __EXPORT ScUndoDragDrop::GetComment() const
{   // "Verschieben" : "Kopieren"
    return bCut ?
        ScGlobal::GetRscString( STR_UNDO_MOVE ) :
        ScGlobal::GetRscString( STR_UNDO_COPY );
}

void ScUndoDragDrop::SetChangeTrack()
{
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
    if ( pChangeTrack )
    {
        if ( bCut )
        {
            nStartChangeAction = pChangeTrack->GetActionMax() + 1;
            pChangeTrack->AppendMove( aSrcRange, aDestRange, pRefUndoDoc );
            nEndChangeAction = pChangeTrack->GetActionMax();
        }
        else
            pChangeTrack->AppendContentRange( aDestRange, pRefUndoDoc,
                nStartChangeAction, nEndChangeAction );
    }
    else
        nStartChangeAction = nEndChangeAction = 0;
}

void ScUndoDragDrop::PaintArea( ScRange aRange ) const
{
    USHORT nExtFlags = PAINT_GRID;
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    ScDocument* pDoc = pDocShell->GetDocument();

    if (pViewShell)
    {
        VirtualDevice aVirtDev;
        ScViewData* pViewData = pViewShell->GetViewData();

        if ( pDoc->SetOptimalHeight( aRange.aStart.Row(), aRange.aEnd.Row(),
                                     aRange.aStart.Tab(), 0, &aVirtDev,
                                     pViewData->GetPPTX(),  pViewData->GetPPTY(),
                                     pViewData->GetZoomX(), pViewData->GetZoomY(),
                                     FALSE ) )
        {
            aRange.aStart.SetCol(0);
            aRange.aEnd.SetCol(MAXCOL);
            aRange.aEnd.SetRow(MAXROW);
            nExtFlags |= PAINT_LEFT;
        }
    }

    if ( bKeepScenarioFlags )
    {
        //  Szenarien mitkopiert -> auch Szenario-Rahmen painten
        aRange.aStart.SetCol(0);
        aRange.aStart.SetRow(0);
        aRange.aEnd.SetCol(MAXCOL);
        aRange.aEnd.SetRow(MAXROW);
    }

    pDocShell->PostPaint( aRange, nExtFlags );
}


void ScUndoDragDrop::DoUndo( ScRange aRange ) const
{
    ScDocument* pDoc = pDocShell->GetDocument();

    ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );

//? DB-Areas vor Daten, damit bei ExtendMerge die Autofilter-Knoepfe stimmen

    ScRange aPaintRange = aRange;
    pDoc->ExtendMerge( aPaintRange );           // vor dem Loeschen

    pDoc->DeleteAreaTab( aRange, IDF_ALL );
    pRefUndoDoc->CopyToDocument( aRange, IDF_ALL, FALSE, pDoc );
    if ( pDoc->HasAttrib( aRange, HASATTR_MERGED ) )
        pDoc->ExtendMerge( aRange, TRUE );

    aPaintRange.aEnd.SetCol( Max( aPaintRange.aEnd.Col(), aRange.aEnd.Col() ) );
    aPaintRange.aEnd.SetRow( Max( aPaintRange.aEnd.Row(), aRange.aEnd.Row() ) );
    PaintArea( aPaintRange );
}

void __EXPORT ScUndoDragDrop::Undo()
{
    BeginUndo();
    DoUndo(aDestRange);
    if (bCut)
        DoUndo(aSrcRange);
    EndUndo();
}

void __EXPORT ScUndoDragDrop::Redo()
{
    BeginRedo();

    ScDocument* pDoc = pDocShell->GetDocument();
    ScDocument* pClipDoc = new ScDocument( SCDOCMODE_CLIP );

    USHORT nTab;
    ScMarkData aSourceMark;
    for (nTab=aSrcRange.aStart.Tab(); nTab<=aSrcRange.aEnd.Tab(); nTab++)
        aSourceMark.SelectTable( nTab, TRUE );
    pDoc->CopyToClip( aSrcRange.aStart.Col(), aSrcRange.aStart.Row(),
                      aSrcRange.aEnd.Col(),   aSrcRange.aEnd.Row(),
                      bCut, pClipDoc, FALSE, &aSourceMark, bKeepScenarioFlags );

    if (bCut)
    {
        ScRange aSrcPaintRange = aSrcRange;
        pDoc->ExtendMerge( aSrcPaintRange );            // vor dem Loeschen
        pDoc->DeleteAreaTab( aSrcRange, IDF_ALL );
        PaintArea( aSrcPaintRange );
    }

    ScMarkData aDestMark;
    for (nTab=aDestRange.aStart.Tab(); nTab<=aDestRange.aEnd.Tab(); nTab++)
        aDestMark.SelectTable( nTab, TRUE );
    pDoc->CopyFromClip( aDestRange, aDestMark, IDF_ALL, NULL, pClipDoc, TRUE );
    for (nTab=aDestRange.aStart.Tab(); nTab<=aDestRange.aEnd.Tab(); nTab++)
    {
        USHORT nEndCol = aDestRange.aEnd.Col();
        USHORT nEndRow = aDestRange.aEnd.Row();
        pDoc->ExtendMerge( aDestRange.aStart.Col(), aDestRange.aStart.Row(),
                            nEndCol, nEndRow, nTab, TRUE );
        PaintArea( ScRange( aDestRange.aStart.Col(), aDestRange.aStart.Row(), nTab,
                            nEndCol, nEndRow, nTab ) );
    }

    SetChangeTrack();

    delete pClipDoc;
    ShowTable( aDestRange.aStart.Tab() );
    EndRedo();
}

void __EXPORT ScUndoDragDrop::Repeat(SfxRepeatTarget& rTarget)
{
}

BOOL __EXPORT ScUndoDragDrop::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return FALSE;           // geht nicht
}


// -----------------------------------------------------------------------
//
//      Liste der Bereichsnamen einfuegen
//      (Einfuegen|Name|Einfuegen =>[Liste])
//

ScUndoListNames::ScUndoListNames( ScDocShell* pNewDocShell, const ScRange& rRange,
                ScDocument* pNewUndoDoc, ScDocument* pNewRedoDoc ) :
    ScBlockUndo( pNewDocShell, rRange, SC_UNDO_AUTOHEIGHT ),
    pUndoDoc( pNewUndoDoc ),
    pRedoDoc( pNewRedoDoc )
{
}

__EXPORT ScUndoListNames::~ScUndoListNames()
{
    delete pUndoDoc;
    delete pRedoDoc;
}

String __EXPORT ScUndoListNames::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_LISTNAMES );
}

void ScUndoListNames::DoChange( ScDocument* pSrcDoc ) const
{
    ScDocument* pDoc = pDocShell->GetDocument();

    pDoc->DeleteAreaTab( aBlockRange, IDF_ALL );
    pSrcDoc->CopyToDocument( aBlockRange, IDF_ALL, FALSE, pDoc );
    pDocShell->PostPaint( aBlockRange, PAINT_GRID );
    pDocShell->PostDataChanged();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->CellContentChanged();
}

void __EXPORT ScUndoListNames::Undo()
{
    BeginUndo();
    DoChange(pUndoDoc);
    EndUndo();
}

void __EXPORT ScUndoListNames::Redo()
{
    BeginRedo();
    DoChange(pRedoDoc);
    EndRedo();
}

void __EXPORT ScUndoListNames::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->InsertNameList();
}

BOOL __EXPORT ScUndoListNames::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}


// -----------------------------------------------------------------------
//
//      Szenario anwenden
//      (Extras|Szenarien)
//

ScUndoUseScenario::ScUndoUseScenario( ScDocShell* pNewDocShell,
                        const ScMarkData& rMark,
/*C*/                   const ScArea& rDestArea,
                              ScDocument* pNewUndoDoc,
                        const String& rNewName ) :
    ScSimpleUndo( pNewDocShell ),
    pUndoDoc( pNewUndoDoc ),
    aMarkData( rMark ),
    aName( rNewName )
{
    aRange.aStart.SetCol(rDestArea.nColStart);
    aRange.aStart.SetRow(rDestArea.nRowStart);
    aRange.aStart.SetTab(rDestArea.nTab);
    aRange.aEnd.SetCol(rDestArea.nColEnd);
    aRange.aEnd.SetRow(rDestArea.nRowEnd);
    aRange.aEnd.SetTab(rDestArea.nTab);
}

__EXPORT ScUndoUseScenario::~ScUndoUseScenario()
{
    delete pUndoDoc;
}

String __EXPORT ScUndoUseScenario::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_USESCENARIO );
}

void __EXPORT ScUndoUseScenario::Undo()
{
    BeginUndo();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        pViewShell->DoneBlockMode();
        pViewShell->InitOwnBlockMode();
    }

    ScDocument* pDoc = pDocShell->GetDocument();
    pDoc->DeleteSelection( IDF_ALL, aMarkData );
    pUndoDoc->CopyToDocument( aRange, IDF_ALL, TRUE, pDoc, &aMarkData );

    //  Szenario-Tabellen
    BOOL bFrame = FALSE;
    USHORT nTab = aRange.aStart.Tab();
    USHORT nEndTab = nTab;
    while ( pUndoDoc->HasTable(nEndTab+1) && pUndoDoc->IsScenario(nEndTab+1) )
        ++nEndTab;
    for (USHORT i = nTab+1; i<=nEndTab; i++)
    {
        //  Flags immer
        String aComment;
        Color  aColor;
        USHORT nScenFlags;
        pUndoDoc->GetScenarioData( i, aComment, aColor, nScenFlags );
        pDoc->SetScenarioData( i, aComment, aColor, nScenFlags );
        BOOL bActive = pUndoDoc->IsActiveScenario( i );
        pDoc->SetActiveScenario( i, bActive );
        //  Bei Zurueckkopier-Szenarios auch Inhalte
        if ( nScenFlags & SC_SCENARIO_TWOWAY )
        {
            pDoc->DeleteAreaTab( 0,0, MAXCOL,MAXROW, i, IDF_ALL );
            pUndoDoc->CopyToDocument( 0,0,i, MAXCOL,MAXROW,i, IDF_ALL,FALSE, pDoc );
        }
        if ( nScenFlags & SC_SCENARIO_SHOWFRAME )
            bFrame = TRUE;
    }

    //  Wenn sichtbare Rahmen, dann alles painten
    if (bFrame)
        pDocShell->PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, PAINT_GRID | PAINT_EXTRAS );
    else
        pDocShell->PostPaint( aRange, PAINT_GRID | PAINT_EXTRAS );
    pDocShell->PostDataChanged();
    if (pViewShell)
        pViewShell->CellContentChanged();

    ShowTable( aRange.aStart.Tab() );

    EndUndo();
}

void __EXPORT ScUndoUseScenario::Redo()
{
    USHORT nTab = aRange.aStart.Tab();
    BeginRedo();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        pViewShell->SetTabNo( nTab );
        pViewShell->DoneBlockMode();
        pViewShell->InitOwnBlockMode();
    }

    pDocShell->UseScenario( nTab, aName, FALSE );

    EndRedo();
}

void __EXPORT ScUndoUseScenario::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
    {
        String aTemp = aName;
        ((ScTabViewTarget&)rTarget).GetViewShell()->UseScenario(aTemp);
    }
}

BOOL __EXPORT ScUndoUseScenario::CanRepeat(SfxRepeatTarget& rTarget) const
{
    if (rTarget.ISA(ScTabViewTarget))
    {
        ScViewData* pViewData = ((ScTabViewTarget&)rTarget).GetViewShell()->GetViewData();
        return !pViewData->GetDocument()->IsScenario( pViewData->GetTabNo() );
    }
    return FALSE;
}


// -----------------------------------------------------------------------
//
//      Vorlage anwenden
//      (Format|Vorlagenkatalog)
//

ScUndoSelectionStyle::ScUndoSelectionStyle( ScDocShell* pNewDocShell,
                                      const ScMarkData& rMark,
                                      const ScRange& rRange,
                                      const String& rName,
                                            ScDocument* pNewUndoDoc ) :
    ScSimpleUndo( pNewDocShell ),
    aMarkData( rMark ),
    aRange( rRange ),
    aStyleName( rName ),
    pUndoDoc( pNewUndoDoc )
{
    aMarkData.MarkToMulti();
}

__EXPORT ScUndoSelectionStyle::~ScUndoSelectionStyle()
{
    delete pUndoDoc;
}

String __EXPORT ScUndoSelectionStyle::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_APPLYCELLSTYLE );
}

void ScUndoSelectionStyle::DoChange( const BOOL bUndo )
{
    ScDocument* pDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    if (pViewShell)
    {
        pViewShell->DoneBlockMode();
        pViewShell->InitOwnBlockMode();
        pViewShell->GetViewData()->GetMarkData() = aMarkData;   // CopyMarksTo
    }

    ScRange aWorkRange( aRange );
    if ( pDoc->HasAttrib( aWorkRange, HASATTR_MERGED ) )        // zusammengefasste Zellen?
        pDoc->ExtendMerge( aWorkRange, TRUE );

    BOOL bHasLines = pDoc->HasAttrib( aWorkRange, HASATTR_PAINTEXT );

    if (bUndo)      // bei Undo alte Daten wieder reinschubsen
    {
        USHORT nTabCount = pDoc->GetTableCount();
        ScRange aCopyRange = aWorkRange;
        aCopyRange.aStart.SetTab(0);
        aCopyRange.aEnd.SetTab(nTabCount-1);
        pUndoDoc->CopyToDocument( aCopyRange, IDF_ATTRIB, TRUE, pDoc, &aMarkData );
    }
    else            // bei Redo Style wieder zuweisen
    {
        ScStyleSheetPool* pStlPool = pDoc->GetStyleSheetPool();
        ScStyleSheet* pStyleSheet =
            (ScStyleSheet*) pStlPool->Find( aStyleName, SFX_STYLE_FAMILY_PARA );
        if (!pStyleSheet)
        {
            DBG_ERROR("StyleSheet not found");
            return;
        }
        pDoc->ApplySelectionStyle( *pStyleSheet, aMarkData );
    }

    bHasLines = bHasLines || pDoc->HasAttrib( aWorkRange, HASATTR_PAINTEXT );

    if ( !( (pViewShell) && pViewShell->AdjustBlockHeight() ) )
/*A*/   pDocShell->PostPaint( aWorkRange, PAINT_GRID | PAINT_EXTRAS, (bHasLines ? SC_PF_LINES : 0) );

    ShowTable( aWorkRange.aStart.Tab() );
}

void __EXPORT ScUndoSelectionStyle::Undo()
{
    BeginUndo();
    DoChange( TRUE );
    EndUndo();
}

void __EXPORT ScUndoSelectionStyle::Redo()
{
    BeginRedo();
    DoChange( FALSE );
    EndRedo();
}

void __EXPORT ScUndoSelectionStyle::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScStyleSheetPool* pStlPool = pDoc->GetStyleSheetPool();
        ScStyleSheet* pStyleSheet = (ScStyleSheet*) pStlPool->
                                            Find( aStyleName, SFX_STYLE_FAMILY_PARA );
        if (!pStyleSheet)
        {
            DBG_ERROR("StyleSheet not found");
            return;
        }

        ScTabViewShell& rViewShell = *((ScTabViewTarget&)rTarget).GetViewShell();
        rViewShell.SetStyleSheetToMarked( pStyleSheet, TRUE );
    }
}

BOOL __EXPORT ScUndoSelectionStyle::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

USHORT __EXPORT ScUndoSelectionStyle::GetId() const
{
    return STR_UNDO_APPLYCELLSTYLE;
}


// -----------------------------------------------------------------------
//
//      Matrix-Formel eingeben
//

ScUndoEnterMatrix::ScUndoEnterMatrix( ScDocShell* pNewDocShell, const ScRange& rArea,
                                      ScDocument* pNewUndoDoc, const String& rForm ) :
    ScBlockUndo( pNewDocShell, rArea, SC_UNDO_SIMPLE ),
    pUndoDoc( pNewUndoDoc ),
    aFormula( rForm )
{
    SetChangeTrack();
}

__EXPORT ScUndoEnterMatrix::~ScUndoEnterMatrix()
{
    delete pUndoDoc;
}

String __EXPORT ScUndoEnterMatrix::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_ENTERMATRIX );
}

void ScUndoEnterMatrix::SetChangeTrack()
{
    ScDocument* pDoc = pDocShell->GetDocument();
    ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->AppendContentRange( aBlockRange, pUndoDoc,
            nStartChangeAction, nEndChangeAction );
    else
        nStartChangeAction = nEndChangeAction = 0;
}

void __EXPORT ScUndoEnterMatrix::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();

    pDoc->DeleteAreaTab( aBlockRange, IDF_ALL );
    pUndoDoc->CopyToDocument( aBlockRange, IDF_ALL, FALSE, pDoc );
    pDocShell->PostPaint( aBlockRange, PAINT_GRID );
    pDocShell->PostDataChanged();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->CellContentChanged();

    ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );

    EndUndo();
}

void __EXPORT ScUndoEnterMatrix::Redo()
{
    BeginRedo();

    ScDocument* pDoc = pDocShell->GetDocument();

    ScMarkData aDestMark;
    aDestMark.SelectOneTable( aBlockRange.aStart.Tab() );
    aDestMark.SetMarkArea( aBlockRange );

    pDoc->InsertMatrixFormula( aBlockRange.aStart.Col(), aBlockRange.aStart.Row(),
                               aBlockRange.aEnd.Col(),   aBlockRange.aEnd.Row(),
                               aDestMark, aFormula );
//  pDocShell->PostPaint( aBlockRange, PAINT_GRID );    // nicht noetig ???

    SetChangeTrack();

    EndRedo();
}

void __EXPORT ScUndoEnterMatrix::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
    {
        String aTemp = aFormula;
        ((ScTabViewTarget&)rTarget).GetViewShell()->EnterMatrix(aTemp);
    }
}

BOOL __EXPORT ScUndoEnterMatrix::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

// -----------------------------------------------------------------------
//
//      Einzug vermindern / erhoehen
//

ScRange lcl_GetMultiMarkRange( const ScMarkData& rMark )
{
    DBG_ASSERT( rMark.IsMultiMarked(), "wrong mark type" );

    ScRange aRange;
    rMark.GetMultiMarkArea( aRange );
    return aRange;
}

ScUndoIndent::ScUndoIndent( ScDocShell* pNewDocShell, const ScMarkData& rMark,
                            ScDocument* pNewUndoDoc, BOOL bIncrement ) :
    ScBlockUndo( pNewDocShell, lcl_GetMultiMarkRange(rMark), SC_UNDO_AUTOHEIGHT ),
    aMarkData( rMark ),
    pUndoDoc( pNewUndoDoc ),
    bIsIncrement( bIncrement )
{
}

__EXPORT ScUndoIndent::~ScUndoIndent()
{
    delete pUndoDoc;
}

String __EXPORT ScUndoIndent::GetComment() const
{
    USHORT nId = bIsIncrement ? STR_UNDO_INC_INDENT : STR_UNDO_DEC_INDENT;
    return ScGlobal::GetRscString( nId );
}

void __EXPORT ScUndoIndent::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    USHORT nTabCount = pDoc->GetTableCount();
    ScRange aCopyRange = aBlockRange;
    aCopyRange.aStart.SetTab(0);
    aCopyRange.aEnd.SetTab(nTabCount-1);
    pUndoDoc->CopyToDocument( aCopyRange, IDF_ATTRIB, TRUE, pDoc, &aMarkData );
    pDocShell->PostPaint( aBlockRange, PAINT_GRID, SC_PF_LINES | SC_PF_TESTMERGE );

    EndUndo();
}

void __EXPORT ScUndoIndent::Redo()
{
    BeginRedo();

    ScDocument* pDoc = pDocShell->GetDocument();
    pDoc->ChangeSelectionIndent( bIsIncrement, aMarkData );
    pDocShell->PostPaint( aBlockRange, PAINT_GRID, SC_PF_LINES | SC_PF_TESTMERGE );

    EndRedo();
}

void __EXPORT ScUndoIndent::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->ChangeIndent( bIsIncrement );
}

BOOL __EXPORT ScUndoIndent::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

// -----------------------------------------------------------------------
//
//      Transliteration for cells
//

ScUndoTransliterate::ScUndoTransliterate( ScDocShell* pNewDocShell, const ScMarkData& rMark,
                            ScDocument* pNewUndoDoc, sal_Int32 nType ) :
    ScBlockUndo( pNewDocShell, lcl_GetMultiMarkRange(rMark), SC_UNDO_AUTOHEIGHT ),
    aMarkData( rMark ),
    pUndoDoc( pNewUndoDoc ),
    nTransliterationType( nType )
{
}

__EXPORT ScUndoTransliterate::~ScUndoTransliterate()
{
    delete pUndoDoc;
}

String __EXPORT ScUndoTransliterate::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_TRANSLITERATE );
}

void __EXPORT ScUndoTransliterate::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    USHORT nTabCount = pDoc->GetTableCount();
    ScRange aCopyRange = aBlockRange;
    aCopyRange.aStart.SetTab(0);
    aCopyRange.aEnd.SetTab(nTabCount-1);
    pUndoDoc->CopyToDocument( aCopyRange, IDF_CONTENTS, TRUE, pDoc, &aMarkData );
    pDocShell->PostPaint( aBlockRange, PAINT_GRID, SC_PF_LINES | SC_PF_TESTMERGE );

    EndUndo();
}

void __EXPORT ScUndoTransliterate::Redo()
{
    BeginRedo();

    ScDocument* pDoc = pDocShell->GetDocument();
    pDoc->TransliterateText( aMarkData, nTransliterationType );
    pDocShell->PostPaint( aBlockRange, PAINT_GRID, SC_PF_LINES | SC_PF_TESTMERGE );

    EndRedo();
}

void __EXPORT ScUndoTransliterate::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->TransliterateText( nTransliterationType );
}

BOOL __EXPORT ScUndoTransliterate::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

// -----------------------------------------------------------------------
//
//      einzelne Items per Which-IDs aus Bereich loeschen
//

ScUndoClearItems::ScUndoClearItems( ScDocShell* pNewDocShell, const ScMarkData& rMark,
                            ScDocument* pNewUndoDoc, const USHORT* pW ) :
    ScBlockUndo( pNewDocShell, lcl_GetMultiMarkRange(rMark), SC_UNDO_AUTOHEIGHT ),
    aMarkData( rMark ),
    pUndoDoc( pNewUndoDoc ),
    pWhich( NULL )
{
    DBG_ASSERT( pW, "ScUndoClearItems: Which-Pointer ist 0" );

    USHORT nCount = 0;
    while ( pW[nCount] )
        ++nCount;
    pWhich = new USHORT[nCount+1];
    for (USHORT i=0; i<=nCount; i++)
        pWhich[i] = pW[i];
}

__EXPORT ScUndoClearItems::~ScUndoClearItems()
{
    delete pUndoDoc;
    delete pWhich;
}

String __EXPORT ScUndoClearItems::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_DELETECONTENTS );
}

void __EXPORT ScUndoClearItems::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    pUndoDoc->CopyToDocument( aBlockRange, IDF_ATTRIB, TRUE, pDoc, &aMarkData );
    pDocShell->PostPaint( aBlockRange, PAINT_GRID, SC_PF_LINES | SC_PF_TESTMERGE );

    EndUndo();
}

void __EXPORT ScUndoClearItems::Redo()
{
    BeginRedo();

    ScDocument* pDoc = pDocShell->GetDocument();
    pDoc->ClearSelectionItems( pWhich, aMarkData );
    pDocShell->PostPaint( aBlockRange, PAINT_GRID, SC_PF_LINES | SC_PF_TESTMERGE );

    EndRedo();
}

void __EXPORT ScUndoClearItems::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
    {
        ScViewData* pViewData = ((ScTabViewTarget&)rTarget).GetViewShell()->GetViewData();
        ScDocFunc aFunc(*pViewData->GetDocShell());
        aFunc.ClearItems( pViewData->GetMarkData(), pWhich, FALSE );
    }
}

BOOL __EXPORT ScUndoClearItems::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

// -----------------------------------------------------------------------
//
//      Alle Umbrueche einer Tabelle loeschen
//

ScUndoRemoveBreaks::ScUndoRemoveBreaks( ScDocShell* pNewDocShell,
                                    USHORT nNewTab, ScDocument* pNewUndoDoc ) :
    ScSimpleUndo( pNewDocShell ),
    nTab( nNewTab ),
    pUndoDoc( pNewUndoDoc )
{
}

__EXPORT ScUndoRemoveBreaks::~ScUndoRemoveBreaks()
{
    delete pUndoDoc;
}

String __EXPORT ScUndoRemoveBreaks::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_REMOVEBREAKS );
}

void __EXPORT ScUndoRemoveBreaks::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    pUndoDoc->CopyToDocument( 0,0,nTab, MAXCOL,MAXROW,nTab, IDF_NONE, FALSE, pDoc );
    if (pViewShell)
        pViewShell->UpdatePageBreakData( TRUE );
    pDocShell->PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, PAINT_GRID );

    EndUndo();
}

void __EXPORT ScUndoRemoveBreaks::Redo()
{
    BeginRedo();

    ScDocument* pDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    pDoc->RemoveManualBreaks(nTab);
    pDoc->UpdatePageBreaks(nTab);
    if (pViewShell)
        pViewShell->UpdatePageBreakData( TRUE );
    pDocShell->PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, PAINT_GRID );

    EndRedo();
}

void __EXPORT ScUndoRemoveBreaks::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
    {
        ScTabViewShell& rViewShell = *((ScTabViewTarget&)rTarget).GetViewShell();
        rViewShell.RemoveManualBreaks();
    }
}

BOOL __EXPORT ScUndoRemoveBreaks::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

// -----------------------------------------------------------------------
//
//      Zusammenfassung aufheben (fuer einen ganzen Bereich)
//

ScUndoRemoveMerge::ScUndoRemoveMerge( ScDocShell* pNewDocShell,
                                       const ScRange& rArea, ScDocument* pNewUndoDoc ) :
    ScBlockUndo( pNewDocShell, rArea, SC_UNDO_SIMPLE ),
    pUndoDoc( pNewUndoDoc )
{
}

__EXPORT ScUndoRemoveMerge::~ScUndoRemoveMerge()
{
    delete pUndoDoc;
}

String __EXPORT ScUndoRemoveMerge::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_REMERGE );  // "Zusammenfassung aufheben"
}

void __EXPORT ScUndoRemoveMerge::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();

    ScRange aExtended = aBlockRange;
    pUndoDoc->ExtendMerge( aExtended );

    pDoc->DeleteAreaTab( aExtended, IDF_ATTRIB );
    pUndoDoc->CopyToDocument( aExtended, IDF_ATTRIB, FALSE, pDoc );

    BOOL bDidPaint = FALSE;
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if ( pViewShell )
    {
        pViewShell->SetTabNo( aExtended.aStart.Tab() );
        bDidPaint = pViewShell->AdjustRowHeight( aExtended.aStart.Row(), aExtended.aEnd.Row() );
    }
    if (!bDidPaint)
        ScUndoUtil::PaintMore( pDocShell, aExtended );

    EndUndo();
}

void __EXPORT ScUndoRemoveMerge::Redo()
{
    BeginRedo();

    USHORT nTab = aBlockRange.aStart.Tab();
    ScDocument* pDoc = pDocShell->GetDocument();
    ScRange aExtended = aBlockRange;
    pDoc->ExtendMerge( aExtended );
    ScRange aRefresh = aExtended;
    pDoc->ExtendOverlapped( aRefresh );

    //  ausfuehren

    const SfxPoolItem& rDefAttr = pDoc->GetPool()->GetDefaultItem( ATTR_MERGE );
    ScPatternAttr aPattern( pDoc->GetPool() );
    aPattern.GetItemSet().Put( rDefAttr );
    pDoc->ApplyPatternAreaTab( aBlockRange.aStart.Col(), aBlockRange.aStart.Row(),
                                aBlockRange.aEnd.Col(), aBlockRange.aEnd.Row(), nTab,
                                aPattern );

    pDoc->RemoveFlagsTab( aExtended.aStart.Col(), aExtended.aStart.Row(),
                            aExtended.aEnd.Col(), aExtended.aEnd.Row(), nTab,
                            SC_MF_HOR | SC_MF_VER );

    pDoc->ExtendMerge( aRefresh, TRUE, FALSE );

    //  Paint

    BOOL bDidPaint = FALSE;
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if ( pViewShell )
    {
        pViewShell->SetTabNo( aExtended.aStart.Tab() );
        bDidPaint = pViewShell->AdjustRowHeight( aExtended.aStart.Row(), aExtended.aEnd.Row() );
    }
    if (!bDidPaint)
        ScUndoUtil::PaintMore( pDocShell, aExtended );

    EndRedo();
}

void __EXPORT ScUndoRemoveMerge::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->RemoveMerge();
}

BOOL __EXPORT ScUndoRemoveMerge::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

// -----------------------------------------------------------------------
//
//      nur Umrandung setzen, per ScRangeList (StarOne)
//

ScRange lcl_TotalRange( const ScRangeList& rRanges )
{
    ScRange aTotal;
    ULONG nCount = rRanges.Count();
    for (ULONG i=0; i<nCount; i++)
    {
        ScRange aRange = *rRanges.GetObject(i);
        if (i==0)
            aTotal = aRange;
        else
        {
            if (aRange.aStart.Col() < aTotal.aStart.Col())
                aTotal.aStart.SetCol(aRange.aStart.Col());
            if (aRange.aStart.Row() < aTotal.aStart.Row())
                aTotal.aStart.SetRow(aRange.aStart.Row());
            if (aRange.aStart.Tab() < aTotal.aStart.Tab())
                aTotal.aStart.SetTab(aRange.aStart.Tab());
            if (aRange.aEnd.Col() > aTotal.aEnd.Col())
                aTotal.aEnd.SetCol(aRange.aEnd.Col());
            if (aRange.aEnd.Row() > aTotal.aEnd.Row())
                aTotal.aEnd.SetRow(aRange.aEnd.Row());
            if (aRange.aEnd.Tab() > aTotal.aEnd.Tab())
                aTotal.aEnd.SetTab(aRange.aEnd.Tab());
        }
    }
    return aTotal;
}

ScUndoBorder::ScUndoBorder( ScDocShell* pNewDocShell,
                            const ScRangeList& rRangeList, ScDocument* pNewUndoDoc,
                            const SvxBoxItem& rNewOuter, const SvxBoxInfoItem& rNewInner ) :
    ScBlockUndo( pNewDocShell, lcl_TotalRange(rRangeList), SC_UNDO_SIMPLE ),
    pUndoDoc( pNewUndoDoc )
{
    pRanges = new ScRangeList(rRangeList);
    pOuter = new SvxBoxItem(rNewOuter);
    pInner = new SvxBoxInfoItem(rNewInner);
}

__EXPORT ScUndoBorder::~ScUndoBorder()
{
    delete pUndoDoc;
    delete pRanges;
    delete pOuter;
    delete pInner;
}

String __EXPORT ScUndoBorder::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_SELATTRLINES );     //! eigener String?
}

void __EXPORT ScUndoBorder::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    ScMarkData aMarkData;
    aMarkData.MarkFromRangeList( *pRanges, FALSE );
    pUndoDoc->CopyToDocument( aBlockRange, IDF_ATTRIB, TRUE, pDoc, &aMarkData );
    pDocShell->PostPaint( aBlockRange, PAINT_GRID, SC_PF_LINES | SC_PF_TESTMERGE );

    EndUndo();
}

void __EXPORT ScUndoBorder::Redo()
{
    BeginRedo();

    ScDocument* pDoc = pDocShell->GetDocument();        //! Funktion an docfunc aufrufen
    ULONG nCount = pRanges->Count();
    ULONG i;
    for (i=0; i<nCount; i++)
    {
        ScRange aRange = *pRanges->GetObject(i);
        USHORT nTab = aRange.aStart.Tab();

        ScMarkData aMark;
        aMark.SetMarkArea( aRange );
        aMark.SelectTable( nTab, TRUE );

        pDoc->ApplySelectionFrame( aMark, pOuter, pInner );
    }
    for (i=0; i<nCount; i++)
        pDocShell->PostPaint( *pRanges->GetObject(i), PAINT_GRID, SC_PF_LINES | SC_PF_TESTMERGE );

    EndRedo();
}

void __EXPORT ScUndoBorder::Repeat(SfxRepeatTarget& rTarget)
{
    //! spaeter (wenn die Funktion aus cellsuno nach docfunc gewandert ist)
}

BOOL __EXPORT ScUndoBorder::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return FALSE;   // s.o.
}




