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

#include "scitems.hxx"
#include <vcl/virdev.hxx>
#include <vcl/waitobj.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/justifyitem.hxx>
#include <sfx2/app.hxx>

#include "undoblk.hxx"
#include "undoutil.hxx"
#include "document.hxx"
#include "patattr.hxx"
#include "docsh.hxx"
#include "tabvwsh.hxx"
#include "rangenam.hxx"
#include "rangeutl.hxx"
#include "dbdata.hxx"
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
#include "refundo.hxx"
#include "undoolk.hxx"
#include "clipparam.hxx"
#include "sc.hrc"

#include <set>

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

// TODO:
/*A*/   // SetOptimalHeight on Document, if no View
/*B*/   // linked sheets
/*C*/   // ScArea
//?     // check later

ScUndoInsertCells::ScUndoInsertCells( ScDocShell* pNewDocShell,
                                const ScRange& rRange, SCTAB nNewCount, SCTAB* pNewTabs, SCTAB* pNewScenarios,
                                InsCellCmd eNewCmd, ScDocument* pUndoDocument, ScRefUndoData* pRefData,
                                sal_Bool bNewPartOfPaste ) :
    ScMoveUndo( pNewDocShell, pUndoDocument, pRefData, SC_UNDO_REFLAST ),
    aEffRange( rRange ),
    nCount( nNewCount ),
    pTabs( pNewTabs ),
    pScenarios( pNewScenarios ),
    eCmd( eNewCmd ),
    bPartOfPaste( bNewPartOfPaste ),
    pPasteUndo( NULL )
{
    if (eCmd == INS_INSROWS)            // whole row?
    {
        aEffRange.aStart.SetCol(0);
        aEffRange.aEnd.SetCol(MAXCOL);
    }

    if (eCmd == INS_INSCOLS)            // whole column?
    {
        aEffRange.aStart.SetRow(0);
        aEffRange.aEnd.SetRow(MAXROW);
    }

    SetChangeTrack();
}

ScUndoInsertCells::~ScUndoInsertCells()
{
    delete pPasteUndo;
    delete []pTabs;
    delete []pScenarios;
}

rtl::OUString ScUndoInsertCells::GetComment() const
{
    return ScGlobal::GetRscString( pPasteUndo ? STR_UNDO_PASTE : STR_UNDO_INSERTCELLS );
}

sal_Bool ScUndoInsertCells::Merge( SfxUndoAction* pNextAction )
{
    //  If a paste undo action has already been added, append (detective) action there.
    if ( pPasteUndo )
        return pPasteUndo->Merge( pNextAction );

    if ( bPartOfPaste && pNextAction->ISA( ScUndoWrapper ) )
    {
        ScUndoWrapper* pWrapper = (ScUndoWrapper*)pNextAction;
        SfxUndoAction* pWrappedAction = pWrapper->GetWrappedUndo();
        if ( pWrappedAction && pWrappedAction->ISA( ScUndoPaste ) )
        {
            //  Store paste action if this is part of paste with inserting cells.
            //  A list action isn't used because Repeat wouldn't work (insert wrong cells).

            pPasteUndo = pWrappedAction;
            pWrapper->ForgetWrappedUndo();      // pWrapper is deleted by UndoManager
            return sal_True;
        }
    }

    //  Call base class for detective handling
    return ScMoveUndo::Merge( pNextAction );
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

void ScUndoInsertCells::DoChange( const sal_Bool bUndo )
{
    ScDocument* pDoc = pDocShell->GetDocument();
    SCTAB i;

    if ( bUndo )
    {
        ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
        if ( pChangeTrack )
            pChangeTrack->Undo( nEndChangeAction, nEndChangeAction );
    }
    else
        SetChangeTrack();

    // refresh of merged cells has to be after inserting/deleting

    switch (eCmd)
    {
        case INS_INSROWS:
        case INS_CELLSDOWN:
            for( i=0; i<nCount; i++ )
            {
                if (bUndo)
                    pDoc->DeleteRow( aEffRange.aStart.Col(), pTabs[i], aEffRange.aEnd.Col(), pTabs[i]+pScenarios[i],
                    aEffRange.aStart.Row(), static_cast<SCSIZE>(aEffRange.aEnd.Row()-aEffRange.aStart.Row()+1));
                else
                    pDoc->InsertRow( aEffRange.aStart.Col(), pTabs[i], aEffRange.aEnd.Col(), pTabs[i]+pScenarios[i],
                    aEffRange.aStart.Row(), static_cast<SCSIZE>(aEffRange.aEnd.Row()-aEffRange.aStart.Row()+1));
            }
            break;
        case INS_INSCOLS:
        case INS_CELLSRIGHT:
            for( i=0; i<nCount; i++ )
            {
                if (bUndo)
                    pDoc->DeleteCol( aEffRange.aStart.Row(), pTabs[i], aEffRange.aEnd.Row(), pTabs[i]+pScenarios[i],
                    aEffRange.aStart.Col(), static_cast<SCSIZE>(aEffRange.aEnd.Col()-aEffRange.aStart.Col()+1));
                else
                    pDoc->InsertCol( aEffRange.aStart.Row(), pTabs[i], aEffRange.aEnd.Row(), pTabs[i]+pScenarios[i],
                    aEffRange.aStart.Col(), static_cast<SCSIZE>(aEffRange.aEnd.Col()-aEffRange.aStart.Col()+1));
            }
            break;
        default:
        {
            // added to avoid warnings
        }
    }

    ScRange aWorkRange( aEffRange );
    if ( eCmd == INS_CELLSRIGHT )                   // only "shift right" requires refresh of the moved area
        aWorkRange.aEnd.SetCol(MAXCOL);
    for( i=0; i<nCount; i++ )
    {
        if ( pDoc->HasAttrib( aWorkRange.aStart.Col(), aWorkRange.aStart.Row(), pTabs[i],
            aWorkRange.aEnd.Col(), aWorkRange.aEnd.Row(), pTabs[i], HASATTR_MERGED ) )
        {
            SCCOL nEndCol = aWorkRange.aEnd.Col();
            SCROW nEndRow = aWorkRange.aEnd.Row();
            pDoc->ExtendMerge( aWorkRange.aStart.Col(), aWorkRange.aStart.Row(), nEndCol, nEndRow, pTabs[i], sal_True );
        }
    }

    // Undo for displaced attributes?

    sal_uInt16 nPaint = PAINT_GRID;
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    switch (eCmd)
    {
        case INS_INSROWS:
            nPaint |= PAINT_LEFT;
            aWorkRange.aEnd.SetRow(MAXROW);
            break;
        case INS_CELLSDOWN:
            for( i=0; i<nCount; i++ )
            {
                aWorkRange.aEnd.SetRow(MAXROW);
                if ( pDocShell->AdjustRowHeight( aWorkRange.aStart.Row(), aWorkRange.aEnd.Row(), pTabs[i] ))
                {
                    aWorkRange.aStart.SetCol(0);
                    aWorkRange.aEnd.SetCol(MAXCOL);
                    nPaint |= PAINT_LEFT;
                }
            }
            break;
        case INS_INSCOLS:
            nPaint |= PAINT_TOP;                // top bar
        case INS_CELLSRIGHT:
            for( i=0; i<nCount; i++ )
            {
                aWorkRange.aEnd.SetCol(MAXCOL);     // to the far right
                if ( pDocShell->AdjustRowHeight( aWorkRange.aStart.Row(), aWorkRange.aEnd.Row(), pTabs[i]) )
                {                                   // AdjustDraw does not paint PAINT_TOP,
                    aWorkRange.aStart.SetCol(0);    // thus solved like this
                    aWorkRange.aEnd.SetRow(MAXROW);
                    nPaint |= PAINT_LEFT;
                }
            }
            break;
        default:
        {
            // added to avoid warnings
        }
    }

    for( i=0; i<nCount; i++ )
    {
        pDocShell->PostPaint( aWorkRange.aStart.Col(), aWorkRange.aStart.Row(), pTabs[i],
            aWorkRange.aEnd.Col(), aWorkRange.aEnd.Row(), pTabs[i]+pScenarios[i], nPaint );
    }
    pDocShell->PostDataChanged();
    if (pViewShell)
        pViewShell->CellContentChanged();
}

void ScUndoInsertCells::Undo()
{
    if ( pPasteUndo )
        pPasteUndo->Undo();     // undo paste first

    WaitObject aWait( pDocShell->GetActiveDialogParent() );     // important due to TrackFormulas in UpdateReference
    BeginUndo();
    DoChange( sal_True );
    EndUndo();
}

void ScUndoInsertCells::Redo()
{
    WaitObject aWait( pDocShell->GetActiveDialogParent() );     // important due to TrackFormulas in UpdateReference
    BeginRedo();
    DoChange( false );
    EndRedo();

    if ( pPasteUndo )
        pPasteUndo->Redo();     // redo paste last
}

void ScUndoInsertCells::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
    {
        if ( pPasteUndo )
        {
            //  Repeat for paste with inserting cells is handled completely
            //  by the Paste undo action

            pPasteUndo->Repeat( rTarget );
        }
        else
            ((ScTabViewTarget&)rTarget).GetViewShell()->InsertCells( eCmd, sal_True );
    }
}

sal_Bool ScUndoInsertCells::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}


ScUndoDeleteCells::ScUndoDeleteCells( ScDocShell* pNewDocShell,
                                const ScRange& rRange, SCTAB nNewCount, SCTAB* pNewTabs, SCTAB* pNewScenarios,
                                DelCellCmd eNewCmd, ScDocument* pUndoDocument, ScRefUndoData* pRefData ) :
    ScMoveUndo( pNewDocShell, pUndoDocument, pRefData, SC_UNDO_REFLAST ),
    aEffRange( rRange ),
    nCount( nNewCount ),
    pTabs( pNewTabs ),
    pScenarios( pNewScenarios ),
    eCmd( eNewCmd )
{
    if (eCmd == DEL_DELROWS)            // whole row?
    {
        aEffRange.aStart.SetCol(0);
        aEffRange.aEnd.SetCol(MAXCOL);
    }

    if (eCmd == DEL_DELCOLS)            // whole column?
    {
        aEffRange.aStart.SetRow(0);
        aEffRange.aEnd.SetRow(MAXROW);
    }

    SetChangeTrack();
}

ScUndoDeleteCells::~ScUndoDeleteCells()
{
    delete []pTabs;
    delete []pScenarios;
}

rtl::OUString ScUndoDeleteCells::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_DELETECELLS ); // "Delete"
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

void ScUndoDeleteCells::DoChange( const sal_Bool bUndo )
{
    ScDocument* pDoc = pDocShell->GetDocument();
    SCTAB i;

    if ( bUndo )
    {
        ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
        if ( pChangeTrack )
            pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );
    }
    else
        SetChangeTrack();

    switch (eCmd)
    {
        case DEL_DELROWS:
        case DEL_CELLSUP:
            for( i=0; i<nCount; i++ )
            {
                if (bUndo)
                    pDoc->InsertRow( aEffRange.aStart.Col(), pTabs[i], aEffRange.aEnd.Col(), pTabs[i]+pScenarios[i],
                    aEffRange.aStart.Row(), static_cast<SCSIZE>(aEffRange.aEnd.Row()-aEffRange.aStart.Row()+1));
                else
                    pDoc->DeleteRow( aEffRange.aStart.Col(), pTabs[i], aEffRange.aEnd.Col(), pTabs[i]+pScenarios[i],
                    aEffRange.aStart.Row(), static_cast<SCSIZE>(aEffRange.aEnd.Row()-aEffRange.aStart.Row()+1));
            }
            break;
        case DEL_DELCOLS:
        case DEL_CELLSLEFT:
            for( i=0; i<nCount; i++ )
            {
                if (bUndo)
                    pDoc->InsertCol( aEffRange.aStart.Row(), pTabs[i], aEffRange.aEnd.Row(), pTabs[i]+pScenarios[i],
                    aEffRange.aStart.Col(), static_cast<SCSIZE>(aEffRange.aEnd.Col()-aEffRange.aStart.Col()+1));
                else
                    pDoc->DeleteCol( aEffRange.aStart.Row(), pTabs[i], aEffRange.aEnd.Row(), pTabs[i]+pScenarios[i],
                    aEffRange.aStart.Col(), static_cast<SCSIZE>(aEffRange.aEnd.Col()-aEffRange.aStart.Col()+1));
            }
            break;
        default:
        {
            // added to avoid warnings
        }
    }

    // if Undo, restore references
    for( i=0; i<nCount && bUndo; i++ )
    {
        pRefUndoDoc->CopyToDocument( aEffRange.aStart.Col(), aEffRange.aStart.Row(), pTabs[i], aEffRange.aEnd.Col(), aEffRange.aEnd.Row(), pTabs[i]+pScenarios[i],
            IDF_ALL | IDF_NOCAPTIONS, false, pDoc );
    }

    ScRange aWorkRange( aEffRange );
    if ( eCmd == DEL_CELLSLEFT )        // only "shift left" requires refresh of the moved area
        aWorkRange.aEnd.SetCol(MAXCOL);

    for( i=0; i<nCount; i++ )
    {
        if ( pDoc->HasAttrib( aWorkRange.aStart.Col(), aWorkRange.aStart.Row(), pTabs[i],
            aWorkRange.aEnd.Col(), aWorkRange.aEnd.Row(), pTabs[i], HASATTR_MERGED | HASATTR_OVERLAPPED ) )
        {
            // #i51445# old merge flag attributes must be deleted also for single cells,
            // not only for whole columns/rows

            if ( !bUndo )
            {
                if ( eCmd==DEL_DELCOLS || eCmd==DEL_CELLSLEFT )
                    aWorkRange.aEnd.SetCol(MAXCOL);
                if ( eCmd==DEL_DELROWS || eCmd==DEL_CELLSUP )
                    aWorkRange.aEnd.SetRow(MAXROW);
                ScMarkData aMarkData;
                aMarkData.SelectOneTable( aWorkRange.aStart.Tab() );
                ScPatternAttr aPattern( pDoc->GetPool() );
                aPattern.GetItemSet().Put( ScMergeFlagAttr() );
                pDoc->ApplyPatternArea( aWorkRange.aStart.Col(), aWorkRange.aStart.Row(),
                    aWorkRange.aEnd.Col(),   aWorkRange.aEnd.Row(),
                    aMarkData, aPattern );
            }

            SCCOL nEndCol = aWorkRange.aEnd.Col();
            SCROW nEndRow = aWorkRange.aEnd.Row();
            pDoc->ExtendMerge( aWorkRange.aStart.Col(), aWorkRange.aStart.Row(), nEndCol, nEndRow, pTabs[i], sal_True );
        }
    }

    // Zeichnen
    sal_uInt16 nPaint = PAINT_GRID;
    switch (eCmd)
    {
        case DEL_DELROWS:
            nPaint |= PAINT_LEFT;
            aWorkRange.aEnd.SetRow(MAXROW);
            break;
        case DEL_CELLSUP:
            for( i=0; i<nCount; i++ )
            {
                aWorkRange.aEnd.SetRow(MAXROW);
                if ( pDocShell->AdjustRowHeight( aWorkRange.aStart.Row(), aWorkRange.aEnd.Row(), pTabs[i] ))
                {
                    aWorkRange.aStart.SetCol(0);
                    aWorkRange.aEnd.SetCol(MAXCOL);
                    nPaint |= PAINT_LEFT;
                }
            }
            break;
        case DEL_DELCOLS:
            nPaint |= PAINT_TOP;                // top bar
        case DEL_CELLSLEFT:
            for( i=0; i<nCount; i++ )
            {
                aWorkRange.aEnd.SetCol(MAXCOL);     // to the far right
                if ( pDocShell->AdjustRowHeight( aWorkRange.aStart.Row(), aWorkRange.aEnd.Row(), pTabs[i] ) )
                {
                    aWorkRange.aStart.SetCol(0);
                    aWorkRange.aEnd.SetRow(MAXROW);
                    nPaint |= PAINT_LEFT;
                }
            }
            break;
        default:
        {
            // added to avoid warnings
        }
    }

    for( i=0; i<nCount; i++ )
    {
        pDocShell->PostPaint( aWorkRange.aStart.Col(), aWorkRange.aStart.Row(), pTabs[i],
            aWorkRange.aEnd.Col(), aWorkRange.aEnd.Row(), pTabs[i]+pScenarios[i], nPaint, SC_PF_LINES );
    }
    // Selection not until EndUndo

    pDocShell->PostDataChanged();
    //  CellContentChanged comes with the selection
}

void ScUndoDeleteCells::Undo()
{
    WaitObject aWait( pDocShell->GetActiveDialogParent() );     // important because of TrackFormulas in UpdateReference
    BeginUndo();
    DoChange( sal_True );
    EndUndo();
    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );

    // Selection not until EndUndo
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        for( SCTAB i=0; i<nCount; i++ )
        {
            pViewShell->MarkRange( ScRange(aEffRange.aStart.Col(), aEffRange.aStart.Row(), pTabs[i], aEffRange.aEnd.Col(), aEffRange.aEnd.Row(), pTabs[i]+pScenarios[i]) );
        }
    }
}

void ScUndoDeleteCells::Redo()
{
    WaitObject aWait( pDocShell->GetActiveDialogParent() );     // important because of TrackFormulas in UpdateReference
    BeginRedo();
    DoChange( false);
    EndRedo();
    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->DoneBlockMode();            // current way
}

void ScUndoDeleteCells::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->DeleteCells( eCmd, sal_True );
}

sal_Bool ScUndoDeleteCells::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

// delete cells in multiselection
ScUndoDeleteMulti::ScUndoDeleteMulti( ScDocShell* pNewDocShell,
                                        sal_Bool bNewRows, sal_Bool bNeedsRefresh, SCTAB nNewTab,
                                        const SCCOLROW* pRng, SCCOLROW nRngCnt,
                                        ScDocument* pUndoDocument, ScRefUndoData* pRefData ) :
    ScMoveUndo( pNewDocShell, pUndoDocument, pRefData, SC_UNDO_REFLAST ),
    bRows( bNewRows ),
    bRefresh( bNeedsRefresh ),
    nTab( nNewTab ),
    nRangeCnt( nRngCnt )
{
    pRanges = new SCCOLROW[ 2 * nRangeCnt ];
    memcpy(pRanges,pRng,nRangeCnt*2*sizeof(SCCOLROW));
    SetChangeTrack();
}

ScUndoDeleteMulti::~ScUndoDeleteMulti()
{
    delete [] pRanges;
}

rtl::OUString ScUndoDeleteMulti::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_DELETECELLS );  // like DeleteCells
}

void ScUndoDeleteMulti::DoChange() const
{
    SCCOL nStartCol;
    SCROW nStartRow;
    sal_uInt16 nPaint;
    if (bRows)
    {
        nStartCol = 0;
        nStartRow = static_cast<SCROW>(pRanges[0]);
        nPaint = PAINT_GRID | PAINT_LEFT;
    }
    else
    {
        nStartCol = static_cast<SCCOL>(pRanges[0]);
        nStartRow = 0;
        nPaint = PAINT_GRID | PAINT_TOP;
    }

    if ( bRefresh )
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        SCCOL nEndCol = MAXCOL;
        SCROW nEndRow = MAXROW;
        pDoc->RemoveFlagsTab( nStartCol, nStartRow, nEndCol, nEndRow, nTab, SC_MF_HOR | SC_MF_VER );
        pDoc->ExtendMerge( nStartCol, nStartRow, nEndCol, nEndRow, nTab, sal_True );
    }

    pDocShell->PostPaint( nStartCol, nStartRow, nTab, MAXCOL, MAXROW, nTab, nPaint );
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
        // delete in reverse
        SCCOLROW* pOneRange = &pRanges[2*nRangeCnt];
        for ( SCCOLROW nRangeNo=0; nRangeNo<nRangeCnt; nRangeNo++ )
        {
            SCCOLROW nEnd = *(--pOneRange);
            SCCOLROW nStart = *(--pOneRange);
            if ( bRows )
            {
                aRange.aStart.SetRow( nStart );
                aRange.aEnd.SetRow( nEnd );
            }
            else
            {
                aRange.aStart.SetCol( static_cast<SCCOL>(nStart) );
                aRange.aEnd.SetCol( static_cast<SCCOL>(nEnd) );
            }
            sal_uLong nDummyStart;
            pChangeTrack->AppendDeleteRange( aRange, pRefUndoDoc,
                nDummyStart, nEndChangeAction );
        }
    }
    else
        nStartChangeAction = nEndChangeAction = 0;
}

void ScUndoDeleteMulti::Undo()
{
    WaitObject aWait( pDocShell->GetActiveDialogParent() );     // important because of TrackFormulas in UpdateReference
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    SCCOLROW* pOneRange;
    SCCOLROW nRangeNo;

    // reverse delete -> forward insert
    pOneRange = pRanges;
    for (nRangeNo=0; nRangeNo<nRangeCnt; nRangeNo++)
    {
        SCCOLROW nStart = *(pOneRange++);
        SCCOLROW nEnd = *(pOneRange++);
        if (bRows)
            pDoc->InsertRow( 0,nTab, MAXCOL,nTab, nStart,static_cast<SCSIZE>(nEnd-nStart+1) );
        else
            pDoc->InsertCol( 0,nTab, MAXROW,nTab, static_cast<SCCOL>(nStart), static_cast<SCSIZE>(nEnd-nStart+1) );
    }

    pOneRange = pRanges;
    for (nRangeNo=0; nRangeNo<nRangeCnt; nRangeNo++)
    {
        SCCOLROW nStart = *(pOneRange++);
        SCCOLROW nEnd = *(pOneRange++);
        if (bRows)
            pRefUndoDoc->CopyToDocument( 0,nStart,nTab, MAXCOL,nEnd,nTab, IDF_ALL,false,pDoc );
        else
            pRefUndoDoc->CopyToDocument( static_cast<SCCOL>(nStart),0,nTab,
                    static_cast<SCCOL>(nEnd),MAXROW,nTab, IDF_ALL,false,pDoc );
    }

    ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );

    DoChange();

    //! redrawing the selection is not possible at the moment
    //! since no data for selection exist

    EndUndo();
    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );
}

void ScUndoDeleteMulti::Redo()
{
    WaitObject aWait( pDocShell->GetActiveDialogParent() );     // important because of TrackFormulas in UpdateReference
    BeginRedo();

    ScDocument* pDoc = pDocShell->GetDocument();

    // reverese delet
    SCCOLROW* pOneRange = &pRanges[2*nRangeCnt];
    for (SCCOLROW nRangeNo=0; nRangeNo<nRangeCnt; nRangeNo++)
    {
        SCCOLROW nEnd = *(--pOneRange);
        SCCOLROW nStart = *(--pOneRange);
        if (bRows)
            pDoc->DeleteRow( 0,nTab, MAXCOL,nTab, nStart,static_cast<SCSIZE>(nEnd-nStart+1) );
        else
            pDoc->DeleteCol( 0,nTab, MAXROW,nTab, static_cast<SCCOL>(nStart), static_cast<SCSIZE>(nEnd-nStart+1) );
    }

    SetChangeTrack();

    DoChange();

    EndRedo();
    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );
}

void ScUndoDeleteMulti::Repeat(SfxRepeatTarget& rTarget)
{
    // if single selection
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->DeleteCells( DEL_DELROWS, sal_True );
}

sal_Bool ScUndoDeleteMulti::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}


ScUndoCut::ScUndoCut( ScDocShell* pNewDocShell,
                ScRange aRange, ScAddress aOldEnd, const ScMarkData& rMark,
                ScDocument* pNewUndoDoc ) :
    ScBlockUndo( pNewDocShell, ScRange(aRange.aStart, aOldEnd), SC_UNDO_AUTOHEIGHT ),
    aMarkData( rMark ),
    pUndoDoc( pNewUndoDoc ),
    aExtendedRange( aRange )
{
    SetChangeTrack();
}

ScUndoCut::~ScUndoCut()
{
    delete pUndoDoc;
}

rtl::OUString ScUndoCut::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_CUT ); // "cut"
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

void ScUndoCut::DoChange( const sal_Bool bUndo )
{
    ScDocument* pDoc = pDocShell->GetDocument();
    sal_uInt16 nExtFlags = 0;

    // do not undo/redo objects and note captions, they are handled via drawing undo
    sal_uInt16 nUndoFlags = (IDF_ALL & ~IDF_OBJECTS) | IDF_NOCAPTIONS;

    if (bUndo)  // only for Undo
    {
        //  all sheets - CopyToDocument skips those that don't exist in pUndoDoc
        SCTAB nTabCount = pDoc->GetTableCount();
        ScRange aCopyRange = aExtendedRange;
        aCopyRange.aStart.SetTab(0);
        aCopyRange.aEnd.SetTab(nTabCount-1);
        pUndoDoc->CopyToDocument( aCopyRange, nUndoFlags, false, pDoc );
        ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
        if ( pChangeTrack )
            pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );
    }
    else        // only for Redo
    {
        pDocShell->UpdatePaintExt( nExtFlags, aExtendedRange );
        pDoc->DeleteArea( aBlockRange.aStart.Col(), aBlockRange.aStart.Row(),
                          aBlockRange.aEnd.Col(), aBlockRange.aEnd.Row(), aMarkData, nUndoFlags );
        SetChangeTrack();
    }

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if ( !( (pViewShell) && pViewShell->AdjustBlockHeight() ) )
/*A*/   pDocShell->PostPaint( aExtendedRange, PAINT_GRID, nExtFlags );

    if ( !bUndo )                               //   draw redo after updating row heights
        RedoSdrUndoAction( pDrawUndo );         //! include in ScBlockUndo?

    pDocShell->PostDataChanged();
    if (pViewShell)
        pViewShell->CellContentChanged();
}

void ScUndoCut::Undo()
{
    BeginUndo();
    DoChange( sal_True );
    EndUndo();
}

void ScUndoCut::Redo()
{
    BeginRedo();
    ScDocument* pDoc = pDocShell->GetDocument();
    EnableDrawAdjust( pDoc, false );                //! include in ScBlockUndo?
    DoChange( false );
    EnableDrawAdjust( pDoc, sal_True );                 //! include in ScBlockUndo?
    EndRedo();
}

void ScUndoCut::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->CutToClip( NULL, sal_True );
}

sal_Bool ScUndoCut::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

ScUndoPaste::ScUndoPaste( ScDocShell* pNewDocShell, const ScRangeList& rRanges,
                const ScMarkData& rMark,
                ScDocument* pNewUndoDoc, ScDocument* pNewRedoDoc,
                sal_uInt16 nNewFlags,
                ScRefUndoData* pRefData,
                bool bRedoIsFilled, const ScUndoPasteOptions* pOptions ) :
    ScMultiBlockUndo( pNewDocShell, rRanges, SC_UNDO_SIMPLE ),
    aMarkData( rMark ),
    pUndoDoc( pNewUndoDoc ),
    pRedoDoc( pNewRedoDoc ),
    nFlags( nNewFlags ),
    pRefUndoData( pRefData ),
    pRefRedoData( NULL ),
    bRedoFilled( bRedoIsFilled )
{
    if ( pRefUndoData )
        pRefUndoData->DeleteUnchanged( pDocShell->GetDocument() );

    if ( pOptions )
        aPasteOptions = *pOptions;      // used only for Repeat

    SetChangeTrack();
}

ScUndoPaste::~ScUndoPaste()
{
    delete pUndoDoc;
    delete pRedoDoc;
    delete pRefUndoData;
    delete pRefRedoData;
}

rtl::OUString ScUndoPaste::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_PASTE ); // "paste"
}

void ScUndoPaste::SetChangeTrack()
{
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
    if ( pChangeTrack && (nFlags & IDF_CONTENTS) )
    {
        for (size_t i = 0, n = maBlockRanges.size(); i < n; ++i)
        {
            pChangeTrack->AppendContentRange(*maBlockRanges[i], pUndoDoc,
                nStartChangeAction, nEndChangeAction, SC_CACM_PASTE );
        }
    }
    else
        nStartChangeAction = nEndChangeAction = 0;
}

void ScUndoPaste::DoChange(bool bUndo)
{
    ScDocument* pDoc = pDocShell->GetDocument();

    //  RefUndoData for redo is created before first undo
    //  (with DeleteUnchanged after the DoUndo call)
    bool bCreateRedoData = ( bUndo && pRefUndoData && !pRefRedoData );
    if ( bCreateRedoData )
        pRefRedoData = new ScRefUndoData( pDoc );

    ScRefUndoData* pWorkRefData = bUndo ? pRefUndoData : pRefRedoData;

    // Always back-up either all or none of the content for Undo
    sal_uInt16 nUndoFlags = IDF_NONE;
    if (nFlags & IDF_CONTENTS)
        nUndoFlags |= IDF_CONTENTS;
    if (nFlags & IDF_ATTRIB)
        nUndoFlags |= IDF_ATTRIB;

    // do not undo/redo objects and note captions, they are handled via drawing undo
    (nUndoFlags &= ~IDF_OBJECTS) |= IDF_NOCAPTIONS;

    bool bPaintAll = false;

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    SCTAB nTabCount = pDoc->GetTableCount();
    if ( bUndo && !bRedoFilled )
    {
        if (!pRedoDoc)
        {
            bool bColInfo = true;
            bool bRowInfo = true;
            for (size_t i = 0, n = maBlockRanges.size(); i < n; ++i)
            {
                const ScRange& r = *maBlockRanges[i];
                bColInfo &= (r.aStart.Row() == 0 && r.aEnd.Row() == MAXROW);
                bRowInfo &= (r.aStart.Col() == 0 && r.aEnd.Col() == MAXCOL);
                if (!bColInfo && !bRowInfo)
                    break;
            }

            pRedoDoc = new ScDocument( SCDOCMODE_UNDO );
            pRedoDoc->InitUndoSelected( pDoc, aMarkData, bColInfo, bRowInfo );
        }
        //  read "redo" data from the document in the first undo
        //  all sheets - CopyToDocument skips those that don't exist in pRedoDoc
        for (size_t i = 0, n = maBlockRanges.size(); i < n; ++i)
        {
            ScRange aCopyRange = *maBlockRanges[i];
            aCopyRange.aStart.SetTab(0);
            aCopyRange.aEnd.SetTab(nTabCount-1);
            pDoc->CopyToDocument( aCopyRange, nUndoFlags, false, pRedoDoc );
            bRedoFilled = true;
        }
    }

    sal_uInt16 nExtFlags = 0;
    pDocShell->UpdatePaintExt(nExtFlags, maBlockRanges.Combine());

    aMarkData.MarkToMulti();
    pDoc->DeleteSelection( nUndoFlags, aMarkData );
    aMarkData.MarkToSimple();

    SCTAB nFirstSelected = aMarkData.GetFirstSelected();

    if ( !bUndo && pRedoDoc )       // Redo: UndoToDocument before handling RefData
    {
        for (size_t i = 0, n = maBlockRanges.size(); i < n; ++i)
        {
            ScRange aRange = *maBlockRanges[i];
            aRange.aStart.SetTab(nFirstSelected);
            aRange.aEnd.SetTab(nFirstSelected);
            pRedoDoc->UndoToDocument(aRange, nUndoFlags, false, pDoc);
            ScMarkData::iterator itr = aMarkData.begin(), itrEnd = aMarkData.end();
            for (; itr != itrEnd && *itr < nTabCount; ++itr)
            {
                if (*itr == nFirstSelected)
                    continue;

                aRange.aStart.SetTab(*itr);
                aRange.aEnd.SetTab(*itr);
                pRedoDoc->CopyToDocument( aRange, nUndoFlags, false, pDoc );
            }
        }
    }

    if (pWorkRefData)
    {
        pWorkRefData->DoUndo( pDoc, true );     // true = bSetChartRangeLists for SetChartListenerCollection
        if (!maBlockRanges.empty() &&
            pDoc->RefreshAutoFilter(0, 0, MAXCOL, MAXROW, maBlockRanges[0]->aStart.Tab()))
            bPaintAll = true;
    }

    if ( bCreateRedoData && pRefRedoData )
        pRefRedoData->DeleteUnchanged( pDoc );

    if (bUndo)      // Undo: UndoToDocument after handling RefData
    {
        for (size_t i = 0, n = maBlockRanges.size(); i < n; ++i)
        {
            ScRange aRange = *maBlockRanges[i];
            ScMarkData::iterator itr = aMarkData.begin(), itrEnd = aMarkData.end();
            for (; itr != itrEnd && *itr < nTabCount; ++itr)
            {
                aRange.aStart.SetTab(*itr);
                aRange.aEnd.SetTab(*itr);
                pUndoDoc->UndoToDocument(aRange, nUndoFlags, false, pDoc);
            }
        }
    }

    if ( bUndo )
    {
        ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
        if ( pChangeTrack )
            pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );
    }
    else
        SetChangeTrack();

    ScRangeList aDrawRanges(maBlockRanges);
    sal_uInt16 nPaint = PAINT_GRID;
    for (size_t i = 0, n = aDrawRanges.size(); i < n; ++i)
    {
        ScRange& rDrawRange = *aDrawRanges[i];
        pDoc->ExtendMerge(rDrawRange, true);      // only needed for single sheet (text/rtf etc.)
        if (bPaintAll)
        {
            rDrawRange.aStart.SetCol(0);
            rDrawRange.aStart.SetRow(0);
            rDrawRange.aEnd.SetCol(MAXCOL);
            rDrawRange.aEnd.SetRow(MAXROW);
            nPaint |= PAINT_TOP | PAINT_LEFT;
            if (pViewShell)
                pViewShell->AdjustBlockHeight(false);
        }
        else
        {
            if (maBlockRanges[i]->aStart.Row() == 0 && maBlockRanges[i]->aEnd.Row() == MAXROW) // whole column
            {
                nPaint |= PAINT_TOP;
                rDrawRange.aEnd.SetCol(MAXCOL);
            }
            if (maBlockRanges[i]->aStart.Col() == 0 && maBlockRanges[i]->aEnd.Col() == MAXCOL) // whole row
            {
                nPaint |= PAINT_LEFT;
                rDrawRange.aEnd.SetRow(MAXROW);
            }
            if (pViewShell && pViewShell->AdjustBlockHeight(false))
            {
                rDrawRange.aStart.SetCol(0);
                rDrawRange.aStart.SetRow(0);
                rDrawRange.aEnd.SetCol(MAXCOL);
                rDrawRange.aEnd.SetRow(MAXROW);
                nPaint |= PAINT_LEFT;
            }
            pDocShell->UpdatePaintExt(nExtFlags, rDrawRange);
        }
    }

    if ( !bUndo )                               //   draw redo after updating row heights
        RedoSdrUndoAction(mpDrawUndo);

    pDocShell->PostPaint(aDrawRanges, nPaint, nExtFlags);

    pDocShell->PostDataChanged();
    if (pViewShell)
        pViewShell->CellContentChanged();
}

void ScUndoPaste::Undo()
{
    BeginUndo();
    DoChange(true);
    if (!maBlockRanges.empty())
        ShowTable(*maBlockRanges.front());
    EndUndo();
    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );
}

void ScUndoPaste::Redo()
{
    BeginRedo();
    ScDocument* pDoc = pDocShell->GetDocument();
    EnableDrawAdjust( pDoc, false );                //! include in ScBlockUndo?
    DoChange( false );
    EnableDrawAdjust( pDoc, sal_True );                 //! include in ScBlockUndo?
    EndRedo();
    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );
}

void ScUndoPaste::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
    {
        ScTabViewShell* pViewSh = ((ScTabViewTarget&)rTarget).GetViewShell();
        ScTransferObj* pOwnClip = ScTransferObj::GetOwnClipboard( pViewSh->GetActiveWin() );
        if (pOwnClip)
        {
            // keep a reference in case the clipboard is changed during PasteFromClip
            com::sun::star::uno::Reference<com::sun::star::datatransfer::XTransferable> aOwnClipRef( pOwnClip );
            pViewSh->PasteFromClip( nFlags, pOwnClip->GetDocument(),
                                    aPasteOptions.nFunction, aPasteOptions.bSkipEmpty, aPasteOptions.bTranspose,
                                    aPasteOptions.bAsLink, aPasteOptions.eMoveMode, IDF_NONE,
                                    sal_True );     // allow warning dialog
        }
    }
}

sal_Bool ScUndoPaste::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

ScUndoDragDrop::ScUndoDragDrop( ScDocShell* pNewDocShell,
                    const ScRange& rRange, ScAddress aNewDestPos, sal_Bool bNewCut,
                    ScDocument* pUndoDocument, ScRefUndoData* pRefData, sal_Bool bScenario ) :
    ScMoveUndo( pNewDocShell, pUndoDocument, pRefData, SC_UNDO_REFFIRST ),
    aSrcRange( rRange ),
    bCut( bNewCut ),
    bKeepScenarioFlags( bScenario )
{
    ScAddress aDestEnd(aNewDestPos);
    aDestEnd.IncRow(aSrcRange.aEnd.Row() - aSrcRange.aStart.Row());
    aDestEnd.IncCol(aSrcRange.aEnd.Col() - aSrcRange.aStart.Col());
    aDestEnd.IncTab(aSrcRange.aEnd.Tab() - aSrcRange.aStart.Tab());

    sal_Bool bIncludeFiltered = bCut;
    if ( !bIncludeFiltered )
    {
        // find number of non-filtered rows
        SCROW nPastedCount = pDocShell->GetDocument()->CountNonFilteredRows(
            aSrcRange.aStart.Row(), aSrcRange.aEnd.Row(), aSrcRange.aStart.Tab());

        if ( nPastedCount == 0 )
            nPastedCount = 1;
        aDestEnd.SetRow( aNewDestPos.Row() + nPastedCount - 1 );
    }

    aDestRange.aStart = aNewDestPos;
    aDestRange.aEnd = aDestEnd;

    SetChangeTrack();
}

ScUndoDragDrop::~ScUndoDragDrop()
{
}

rtl::OUString ScUndoDragDrop::GetComment() const
{   // "Move" : "Copy"
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

void ScUndoDragDrop::PaintArea( ScRange aRange, sal_uInt16 nExtFlags ) const
{
    sal_uInt16 nPaint = PAINT_GRID;
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
                                     false ) )
        {
            aRange.aStart.SetCol(0);
            aRange.aEnd.SetCol(MAXCOL);
            aRange.aEnd.SetRow(MAXROW);
            nPaint |= PAINT_LEFT;
        }
    }

    if ( bKeepScenarioFlags )
    {
        //  Copy scenario -> also paint scenario boarder
        aRange.aStart.SetCol(0);
        aRange.aStart.SetRow(0);
        aRange.aEnd.SetCol(MAXCOL);
        aRange.aEnd.SetRow(MAXROW);
    }

    //  column/row info (width/height) included if whole columns/rows were copied
    if ( aSrcRange.aStart.Col() == 0 && aSrcRange.aEnd.Col() == MAXCOL )
    {
        nPaint |= PAINT_LEFT;
        aRange.aEnd.SetRow(MAXROW);
    }
    if ( aSrcRange.aStart.Row() == 0 && aSrcRange.aEnd.Row() == MAXROW )
    {
        nPaint |= PAINT_TOP;
        aRange.aEnd.SetCol(MAXCOL);
    }

    pDocShell->PostPaint( aRange, nPaint, nExtFlags );
}

void ScUndoDragDrop::DoUndo( ScRange aRange ) const
{
    ScDocument* pDoc = pDocShell->GetDocument();

    ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );

    // Database range before data, so that the Autofilter button match up in ExtendMerge

    ScRange aPaintRange = aRange;
    pDoc->ExtendMerge( aPaintRange );           // before deleting

    sal_uInt16 nExtFlags = 0;
    pDocShell->UpdatePaintExt( nExtFlags, aPaintRange );

    // do not undo objects and note captions, they are handled via drawing undo
    sal_uInt16 nUndoFlags = (IDF_ALL & ~IDF_OBJECTS) | IDF_NOCAPTIONS;

    pDoc->DeleteAreaTab( aRange, nUndoFlags );
    pRefUndoDoc->CopyToDocument( aRange, nUndoFlags, false, pDoc );
    if ( pDoc->HasAttrib( aRange, HASATTR_MERGED ) )
        pDoc->ExtendMerge( aRange, sal_True );

    aPaintRange.aEnd.SetCol( Max( aPaintRange.aEnd.Col(), aRange.aEnd.Col() ) );
    aPaintRange.aEnd.SetRow( Max( aPaintRange.aEnd.Row(), aRange.aEnd.Row() ) );

    pDocShell->UpdatePaintExt( nExtFlags, aPaintRange );
    PaintArea( aPaintRange, nExtFlags );
}

void ScUndoDragDrop::Undo()
{
    BeginUndo();
    DoUndo(aDestRange);
    if (bCut)
        DoUndo(aSrcRange);
    EndUndo();
    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );
}

void ScUndoDragDrop::Redo()
{
    BeginRedo();

    ScDocument* pDoc = pDocShell->GetDocument();
    ScDocument* pClipDoc = new ScDocument( SCDOCMODE_CLIP );

    EnableDrawAdjust( pDoc, false );                //! include in ScBlockUndo?

    // do not undo/redo objects and note captions, they are handled via drawing undo
    sal_uInt16 nRedoFlags = (IDF_ALL & ~IDF_OBJECTS) | IDF_NOCAPTIONS;

    /*  TODO: Redoing note captions is quite tricky due to the fact that a
        helper clip document is used. While (re-)pasting the contents to the
        destination area, the original pointers to the captions created while
        dropping have to be restored. A simple CopyFromClip() would create new
        caption objects that are not tracked by drawing undo, and the captions
        restored by drawing redo would live without cell note objects pointing
        to them. So, first, CopyToClip() and CopyFromClip() are called without
        cloning the caption objects. This leads to cell notes pointing to the
        wrong captions from source area that will be removed by drawing redo
        later. Second, the pointers to the new captions have to be restored.
        Sadly, currently these pointers are not stored anywhere but in the list
        of drawing undo actions. */

    SCTAB nTab;
    ScMarkData aSourceMark;
    for (nTab=aSrcRange.aStart.Tab(); nTab<=aSrcRange.aEnd.Tab(); nTab++)
        aSourceMark.SelectTable( nTab, sal_True );

    // do not clone objects and note captions into clipdoc (see above)
    ScClipParam aClipParam(aSrcRange, bCut);
    pDoc->CopyToClip(aClipParam, pClipDoc, &aSourceMark, false, bKeepScenarioFlags, false, false);

    if (bCut)
    {
        ScRange aSrcPaintRange = aSrcRange;
        pDoc->ExtendMerge( aSrcPaintRange );            // before deleting
        sal_uInt16 nExtFlags = 0;
        pDocShell->UpdatePaintExt( nExtFlags, aSrcPaintRange );
        pDoc->DeleteAreaTab( aSrcRange, nRedoFlags );
        PaintArea( aSrcPaintRange, nExtFlags );
    }

    ScMarkData aDestMark;
    for (nTab=aDestRange.aStart.Tab(); nTab<=aDestRange.aEnd.Tab(); nTab++)
        aDestMark.SelectTable( nTab, sal_True );

    sal_Bool bIncludeFiltered = bCut;
    // TODO: restore old note captions instead of cloning new captions...
    pDoc->CopyFromClip( aDestRange, aDestMark, IDF_ALL & ~IDF_OBJECTS, NULL, pClipDoc, sal_True, false, bIncludeFiltered );

    if (bCut)
        for (nTab=aSrcRange.aStart.Tab(); nTab<=aSrcRange.aEnd.Tab(); nTab++)
            pDoc->RefreshAutoFilter( aSrcRange.aStart.Col(), aSrcRange.aStart.Row(),
                                     aSrcRange.aEnd.Col(),   aSrcRange.aEnd.Row(), nTab );

    // skipped rows and merged cells don't mix
    if ( !bIncludeFiltered && pClipDoc->HasClipFilteredRows() )
        pDocShell->GetDocFunc().UnmergeCells( aDestRange, false );

    for (nTab=aDestRange.aStart.Tab(); nTab<=aDestRange.aEnd.Tab(); nTab++)
    {
        SCCOL nEndCol = aDestRange.aEnd.Col();
        SCROW nEndRow = aDestRange.aEnd.Row();
        pDoc->ExtendMerge( aDestRange.aStart.Col(), aDestRange.aStart.Row(),
                            nEndCol, nEndRow, nTab, sal_True );
        PaintArea( ScRange( aDestRange.aStart.Col(), aDestRange.aStart.Row(), nTab,
                            nEndCol, nEndRow, nTab ), 0 );
    }

    SetChangeTrack();

    delete pClipDoc;
    ShowTable( aDestRange.aStart.Tab() );

    RedoSdrUndoAction( pDrawUndo );             //! include in ScBlockUndo?
    EnableDrawAdjust( pDoc, sal_True );             //! include in ScBlockUndo?

    EndRedo();
    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );
}

void ScUndoDragDrop::Repeat(SfxRepeatTarget& /* rTarget */)
{
}

sal_Bool ScUndoDragDrop::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;           // not possible
}

//      Insert list containing range names
//      (Insert|Name|Insert =>[List])
ScUndoListNames::ScUndoListNames( ScDocShell* pNewDocShell, const ScRange& rRange,
                ScDocument* pNewUndoDoc, ScDocument* pNewRedoDoc ) :
    ScBlockUndo( pNewDocShell, rRange, SC_UNDO_AUTOHEIGHT ),
    pUndoDoc( pNewUndoDoc ),
    pRedoDoc( pNewRedoDoc )
{
}

ScUndoListNames::~ScUndoListNames()
{
    delete pUndoDoc;
    delete pRedoDoc;
}

rtl::OUString ScUndoListNames::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_LISTNAMES );
}

void ScUndoListNames::DoChange( ScDocument* pSrcDoc ) const
{
    ScDocument* pDoc = pDocShell->GetDocument();

    pDoc->DeleteAreaTab( aBlockRange, IDF_ALL );
    pSrcDoc->CopyToDocument( aBlockRange, IDF_ALL, false, pDoc );
    pDocShell->PostPaint( aBlockRange, PAINT_GRID );
    pDocShell->PostDataChanged();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->CellContentChanged();
}

void ScUndoListNames::Undo()
{
    BeginUndo();
    DoChange(pUndoDoc);
    EndUndo();
}

void ScUndoListNames::Redo()
{
    BeginRedo();
    DoChange(pRedoDoc);
    EndRedo();
}

void ScUndoListNames::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->InsertNameList();
}

sal_Bool ScUndoListNames::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

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

ScUndoUseScenario::~ScUndoUseScenario()
{
    delete pUndoDoc;
}

rtl::OUString ScUndoUseScenario::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_USESCENARIO );
}

void ScUndoUseScenario::Undo()
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
    pUndoDoc->CopyToDocument( aRange, IDF_ALL, sal_True, pDoc, &aMarkData );

    // scenario table
    sal_Bool bFrame = false;
    SCTAB nTab = aRange.aStart.Tab();
    SCTAB nEndTab = nTab;
    while ( pUndoDoc->HasTable(nEndTab+1) && pUndoDoc->IsScenario(nEndTab+1) )
        ++nEndTab;
    for (SCTAB i = nTab+1; i<=nEndTab; i++)
    {
        // Flags always
        rtl::OUString aComment;
        Color  aColor;
        sal_uInt16 nScenFlags;
        pUndoDoc->GetScenarioData( i, aComment, aColor, nScenFlags );
        pDoc->SetScenarioData( i, aComment, aColor, nScenFlags );
        sal_Bool bActive = pUndoDoc->IsActiveScenario( i );
        pDoc->SetActiveScenario( i, bActive );
        //  For copy-back scenario also consider content
        if ( nScenFlags & SC_SCENARIO_TWOWAY )
        {
            pDoc->DeleteAreaTab( 0,0, MAXCOL,MAXROW, i, IDF_ALL );
            pUndoDoc->CopyToDocument( 0,0,i, MAXCOL,MAXROW,i, IDF_ALL,false, pDoc );
        }
        if ( nScenFlags & SC_SCENARIO_SHOWFRAME )
            bFrame = sal_True;
    }

    // if visible borders, then paint all
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

void ScUndoUseScenario::Redo()
{
    SCTAB nTab = aRange.aStart.Tab();
    BeginRedo();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        pViewShell->SetTabNo( nTab );
        pViewShell->DoneBlockMode();
        pViewShell->InitOwnBlockMode();
    }

    pDocShell->UseScenario( nTab, aName, false );

    EndRedo();
}

void ScUndoUseScenario::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
    {
        String aTemp = aName;
        ((ScTabViewTarget&)rTarget).GetViewShell()->UseScenario(aTemp);
    }
}

sal_Bool ScUndoUseScenario::CanRepeat(SfxRepeatTarget& rTarget) const
{
    if (rTarget.ISA(ScTabViewTarget))
    {
        ScViewData* pViewData = ((ScTabViewTarget&)rTarget).GetViewShell()->GetViewData();
        return !pViewData->GetDocument()->IsScenario( pViewData->GetTabNo() );
    }
    return false;
}

ScUndoSelectionStyle::ScUndoSelectionStyle( ScDocShell* pNewDocShell,
                                      const ScMarkData& rMark,
                                      const ScRange& rRange,
                                      const String& rName,
                                            ScDocument* pNewUndoDoc ) :
    ScSimpleUndo( pNewDocShell ),
    aMarkData( rMark ),
    pUndoDoc( pNewUndoDoc ),
    aStyleName( rName ),
    aRange( rRange )
{
    aMarkData.MarkToMulti();
}

ScUndoSelectionStyle::~ScUndoSelectionStyle()
{
    delete pUndoDoc;
}

rtl::OUString ScUndoSelectionStyle::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_APPLYCELLSTYLE );
}

void ScUndoSelectionStyle::DoChange( const sal_Bool bUndo )
{
    ScDocument* pDoc = pDocShell->GetDocument();

    SetViewMarkData( aMarkData );

    ScRange aWorkRange( aRange );
    if ( pDoc->HasAttrib( aWorkRange, HASATTR_MERGED ) )        // Merged cells?
        pDoc->ExtendMerge( aWorkRange, sal_True );

    sal_uInt16 nExtFlags = 0;
    pDocShell->UpdatePaintExt( nExtFlags, aWorkRange );

    if (bUndo)      // if Undo then push back all old data again
    {
        SCTAB nTabCount = pDoc->GetTableCount();
        ScRange aCopyRange = aWorkRange;
        aCopyRange.aStart.SetTab(0);
        aCopyRange.aEnd.SetTab(nTabCount-1);
        pUndoDoc->CopyToDocument( aCopyRange, IDF_ATTRIB, sal_True, pDoc, &aMarkData );
    }
    else            // if Redo, then reapply style
    {
        ScStyleSheetPool* pStlPool = pDoc->GetStyleSheetPool();
        ScStyleSheet* pStyleSheet =
            (ScStyleSheet*) pStlPool->Find( aStyleName, SFX_STYLE_FAMILY_PARA );
        if (!pStyleSheet)
        {
            OSL_FAIL("StyleSheet not found");
            return;
        }
        pDoc->ApplySelectionStyle( *pStyleSheet, aMarkData );
    }

    pDocShell->UpdatePaintExt( nExtFlags, aWorkRange );

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if ( !( (pViewShell) && pViewShell->AdjustBlockHeight() ) )
/*A*/   pDocShell->PostPaint( aWorkRange, PAINT_GRID | PAINT_EXTRAS, nExtFlags );

    ShowTable( aWorkRange.aStart.Tab() );
}

void ScUndoSelectionStyle::Undo()
{
    BeginUndo();
    DoChange( sal_True );
    EndUndo();
}

void ScUndoSelectionStyle::Redo()
{
    BeginRedo();
    DoChange( false );
    EndRedo();
}

void ScUndoSelectionStyle::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScStyleSheetPool* pStlPool = pDoc->GetStyleSheetPool();
        ScStyleSheet* pStyleSheet = (ScStyleSheet*) pStlPool->
                                            Find( aStyleName, SFX_STYLE_FAMILY_PARA );
        if (!pStyleSheet)
        {
            OSL_FAIL("StyleSheet not found");
            return;
        }

        ScTabViewShell& rViewShell = *((ScTabViewTarget&)rTarget).GetViewShell();
        rViewShell.SetStyleSheetToMarked( pStyleSheet, sal_True );
    }
}

sal_Bool ScUndoSelectionStyle::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

sal_uInt16 ScUndoSelectionStyle::GetId() const
{
    return STR_UNDO_APPLYCELLSTYLE;
}

ScUndoEnterMatrix::ScUndoEnterMatrix( ScDocShell* pNewDocShell, const ScRange& rArea,
                                      ScDocument* pNewUndoDoc, const String& rForm ) :
    ScBlockUndo( pNewDocShell, rArea, SC_UNDO_SIMPLE ),
    pUndoDoc( pNewUndoDoc ),
    aFormula( rForm )
{
    SetChangeTrack();
}

ScUndoEnterMatrix::~ScUndoEnterMatrix()
{
    delete pUndoDoc;
}

rtl::OUString ScUndoEnterMatrix::GetComment() const
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

void ScUndoEnterMatrix::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();

    pDoc->DeleteAreaTab( aBlockRange, IDF_ALL & ~IDF_NOTE );
    pUndoDoc->CopyToDocument( aBlockRange, IDF_ALL & ~IDF_NOTE, false, pDoc );
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

void ScUndoEnterMatrix::Redo()
{
    BeginRedo();

    ScDocument* pDoc = pDocShell->GetDocument();

    ScMarkData aDestMark;
    aDestMark.SelectOneTable( aBlockRange.aStart.Tab() );
    aDestMark.SetMarkArea( aBlockRange );

    pDoc->InsertMatrixFormula( aBlockRange.aStart.Col(), aBlockRange.aStart.Row(),
                               aBlockRange.aEnd.Col(),   aBlockRange.aEnd.Row(),
                               aDestMark, aFormula );

    SetChangeTrack();

    EndRedo();
}

void ScUndoEnterMatrix::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
    {
        String aTemp = aFormula;
        ScDocument* pDoc = pDocShell->GetDocument();
        ((ScTabViewTarget&)rTarget).GetViewShell()->EnterMatrix(aTemp, pDoc->GetGrammar());
    }
}

sal_Bool ScUndoEnterMatrix::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

ScRange lcl_GetMultiMarkRange( const ScMarkData& rMark )
{
    OSL_ENSURE( rMark.IsMultiMarked(), "wrong mark type" );

    ScRange aRange;
    rMark.GetMultiMarkArea( aRange );
    return aRange;
}

ScUndoIndent::ScUndoIndent( ScDocShell* pNewDocShell, const ScMarkData& rMark,
                            ScDocument* pNewUndoDoc, sal_Bool bIncrement ) :
    ScBlockUndo( pNewDocShell, lcl_GetMultiMarkRange(rMark), SC_UNDO_AUTOHEIGHT ),
    aMarkData( rMark ),
    pUndoDoc( pNewUndoDoc ),
    bIsIncrement( bIncrement )
{
}

ScUndoIndent::~ScUndoIndent()
{
    delete pUndoDoc;
}

rtl::OUString ScUndoIndent::GetComment() const
{
    sal_uInt16 nId = bIsIncrement ? STR_UNDO_INC_INDENT : STR_UNDO_DEC_INDENT;
    return ScGlobal::GetRscString( nId );
}

void ScUndoIndent::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    SCTAB nTabCount = pDoc->GetTableCount();
    ScRange aCopyRange = aBlockRange;
    aCopyRange.aStart.SetTab(0);
    aCopyRange.aEnd.SetTab(nTabCount-1);
    pUndoDoc->CopyToDocument( aCopyRange, IDF_ATTRIB, sal_True, pDoc, &aMarkData );
    pDocShell->PostPaint( aBlockRange, PAINT_GRID, SC_PF_LINES | SC_PF_TESTMERGE );

    EndUndo();
}

void ScUndoIndent::Redo()
{
    BeginRedo();

    ScDocument* pDoc = pDocShell->GetDocument();
    pDoc->ChangeSelectionIndent( bIsIncrement, aMarkData );
    pDocShell->PostPaint( aBlockRange, PAINT_GRID, SC_PF_LINES | SC_PF_TESTMERGE );

    EndRedo();
}

void ScUndoIndent::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->ChangeIndent( bIsIncrement );
}

sal_Bool ScUndoIndent::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

ScUndoTransliterate::ScUndoTransliterate( ScDocShell* pNewDocShell, const ScMarkData& rMark,
                            ScDocument* pNewUndoDoc, sal_Int32 nType ) :
    ScBlockUndo( pNewDocShell, lcl_GetMultiMarkRange(rMark), SC_UNDO_AUTOHEIGHT ),
    aMarkData( rMark ),
    pUndoDoc( pNewUndoDoc ),
    nTransliterationType( nType )
{
}

ScUndoTransliterate::~ScUndoTransliterate()
{
    delete pUndoDoc;
}

rtl::OUString ScUndoTransliterate::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_TRANSLITERATE );
}

void ScUndoTransliterate::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    SCTAB nTabCount = pDoc->GetTableCount();
    ScRange aCopyRange = aBlockRange;
    aCopyRange.aStart.SetTab(0);
    aCopyRange.aEnd.SetTab(nTabCount-1);
    pUndoDoc->CopyToDocument( aCopyRange, IDF_CONTENTS, sal_True, pDoc, &aMarkData );
    pDocShell->PostPaint( aBlockRange, PAINT_GRID, SC_PF_LINES | SC_PF_TESTMERGE );

    EndUndo();
}

void ScUndoTransliterate::Redo()
{
    BeginRedo();

    ScDocument* pDoc = pDocShell->GetDocument();
    pDoc->TransliterateText( aMarkData, nTransliterationType );
    pDocShell->PostPaint( aBlockRange, PAINT_GRID, SC_PF_LINES | SC_PF_TESTMERGE );

    EndRedo();
}

void ScUndoTransliterate::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->TransliterateText( nTransliterationType );
}

sal_Bool ScUndoTransliterate::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

ScUndoClearItems::ScUndoClearItems( ScDocShell* pNewDocShell, const ScMarkData& rMark,
                            ScDocument* pNewUndoDoc, const sal_uInt16* pW ) :
    ScBlockUndo( pNewDocShell, lcl_GetMultiMarkRange(rMark), SC_UNDO_AUTOHEIGHT ),
    aMarkData( rMark ),
    pUndoDoc( pNewUndoDoc ),
    pWhich( NULL )
{
    OSL_ENSURE( pW, "ScUndoClearItems: Which-Pointer ist 0" );

    sal_uInt16 nCount = 0;
    while ( pW[nCount] )
        ++nCount;
    pWhich = new sal_uInt16[nCount+1];
    for (sal_uInt16 i=0; i<=nCount; i++)
        pWhich[i] = pW[i];
}

ScUndoClearItems::~ScUndoClearItems()
{
    delete pUndoDoc;
    delete pWhich;
}

rtl::OUString ScUndoClearItems::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_DELETECONTENTS );
}

void ScUndoClearItems::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    pUndoDoc->CopyToDocument( aBlockRange, IDF_ATTRIB, sal_True, pDoc, &aMarkData );
    pDocShell->PostPaint( aBlockRange, PAINT_GRID, SC_PF_LINES | SC_PF_TESTMERGE );

    EndUndo();
}

void ScUndoClearItems::Redo()
{
    BeginRedo();

    ScDocument* pDoc = pDocShell->GetDocument();
    pDoc->ClearSelectionItems( pWhich, aMarkData );
    pDocShell->PostPaint( aBlockRange, PAINT_GRID, SC_PF_LINES | SC_PF_TESTMERGE );

    EndRedo();
}

void ScUndoClearItems::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
    {
        ScViewData* pViewData = ((ScTabViewTarget&)rTarget).GetViewShell()->GetViewData();
        pViewData->GetDocFunc().ClearItems( pViewData->GetMarkData(), pWhich, false );
    }
}

sal_Bool ScUndoClearItems::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

// remove all line breaks of a table
ScUndoRemoveBreaks::ScUndoRemoveBreaks( ScDocShell* pNewDocShell,
                                    SCTAB nNewTab, ScDocument* pNewUndoDoc ) :
    ScSimpleUndo( pNewDocShell ),
    nTab( nNewTab ),
    pUndoDoc( pNewUndoDoc )
{
}

ScUndoRemoveBreaks::~ScUndoRemoveBreaks()
{
    delete pUndoDoc;
}

rtl::OUString ScUndoRemoveBreaks::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_REMOVEBREAKS );
}

void ScUndoRemoveBreaks::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    pUndoDoc->CopyToDocument( 0,0,nTab, MAXCOL,MAXROW,nTab, IDF_NONE, false, pDoc );
    if (pViewShell)
        pViewShell->UpdatePageBreakData( sal_True );
    pDocShell->PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, PAINT_GRID );

    EndUndo();
}

void ScUndoRemoveBreaks::Redo()
{
    BeginRedo();

    ScDocument* pDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    pDoc->RemoveManualBreaks(nTab);
    pDoc->UpdatePageBreaks(nTab);
    if (pViewShell)
        pViewShell->UpdatePageBreakData( sal_True );
    pDocShell->PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, PAINT_GRID );

    EndRedo();
}

void ScUndoRemoveBreaks::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
    {
        ScTabViewShell& rViewShell = *((ScTabViewTarget&)rTarget).GetViewShell();
        rViewShell.RemoveManualBreaks();
    }
}

sal_Bool ScUndoRemoveBreaks::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

ScUndoRemoveMerge::ScUndoRemoveMerge( ScDocShell* pNewDocShell,
                                      const ScCellMergeOption& rOption, ScDocument* pNewUndoDoc ) :
    ScBlockUndo( pNewDocShell, rOption.getFirstSingleRange(), SC_UNDO_SIMPLE ),
    maOption(rOption),
    pUndoDoc( pNewUndoDoc )
{
}

ScUndoRemoveMerge::~ScUndoRemoveMerge()
{
    delete pUndoDoc;
}

rtl::OUString ScUndoRemoveMerge::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_REMERGE );  // "remove merge"
}

void ScUndoRemoveMerge::Undo()
{
    using ::std::set;

    SetCurTab();
    BeginUndo();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    ScDocument* pDoc = pDocShell->GetDocument();
    for (set<SCTAB>::const_iterator itr = maOption.maTabs.begin(), itrEnd = maOption.maTabs.end();
          itr != itrEnd; ++itr)
    {
        OSL_ENSURE(pUndoDoc, "NULL pUndoDoc!");
        if (!pUndoDoc)
            continue;
        // There is no need to extend merge area because it's already been extended.
        ScRange aRange = maOption.getSingleRange(*itr);
        pDoc->DeleteAreaTab(aRange, IDF_ATTRIB);
        pUndoDoc->CopyToDocument(aRange, IDF_ATTRIB, false, pDoc);

        bool bDidPaint = false;
        if ( pViewShell )
        {
            pViewShell->SetTabNo(*itr);
            bDidPaint = pViewShell->AdjustRowHeight(maOption.mnStartRow, maOption.mnEndRow);
        }
        if (!bDidPaint)
            ScUndoUtil::PaintMore(pDocShell, aRange);
    }

    EndUndo();
}

void ScUndoRemoveMerge::Redo()
{
    using ::std::set;

    SetCurTab();
    BeginRedo();

    ScDocument* pDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    for (set<SCTAB>::const_iterator itr = maOption.maTabs.begin(), itrEnd = maOption.maTabs.end();
          itr != itrEnd; ++itr)
    {
        SCTAB nTab = *itr;
        // There is no need to extend merge area because it's already been extended.
        ScRange aRange = maOption.getSingleRange(nTab);

        const SfxPoolItem& rDefAttr = pDoc->GetPool()->GetDefaultItem( ATTR_MERGE );
        ScPatternAttr aPattern( pDoc->GetPool() );
        aPattern.GetItemSet().Put( rDefAttr );
        pDoc->ApplyPatternAreaTab( maOption.mnStartCol, maOption.mnStartRow,
                                   maOption.mnEndCol, maOption.mnEndRow, nTab,
                                   aPattern );

        pDoc->RemoveFlagsTab( maOption.mnStartCol, maOption.mnStartRow,
                              maOption.mnEndCol, maOption.mnEndRow, nTab,
                              SC_MF_HOR | SC_MF_VER );

        pDoc->ExtendMerge(aRange, true);

        //  Paint

        sal_Bool bDidPaint = false;
        if ( pViewShell )
        {
            pViewShell->SetTabNo(nTab);
            bDidPaint = pViewShell->AdjustRowHeight(maOption.mnStartRow, maOption.mnEndRow);
        }
        if (!bDidPaint)
            ScUndoUtil::PaintMore(pDocShell, aRange);
    }

    EndRedo();
}

void ScUndoRemoveMerge::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->RemoveMerge();
}

sal_Bool ScUndoRemoveMerge::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

void ScUndoRemoveMerge::SetCurTab()
{
    SCTAB nCurTab = pDocShell->GetCurTab();
    aBlockRange.aStart.SetTab(nCurTab);
    aBlockRange.aEnd.SetTab(nCurTab);
}

/** set only border, for ScRangeList (StarOne) */
ScRange lcl_TotalRange( const ScRangeList& rRanges )
{
    ScRange aTotal;
    if ( !rRanges.empty() )
    {
        aTotal = *rRanges[ 0 ];
        for ( size_t i = 1, nCount = rRanges.size(); i < nCount; ++i )
        {
            ScRange aRange = *rRanges[ i ];
            if (aRange.aStart.Col() < aTotal.aStart.Col()) aTotal.aStart.SetCol(aRange.aStart.Col());
            if (aRange.aStart.Row() < aTotal.aStart.Row()) aTotal.aStart.SetRow(aRange.aStart.Row());
            if (aRange.aStart.Tab() < aTotal.aStart.Tab()) aTotal.aStart.SetTab(aRange.aStart.Tab());
            if (aRange.aEnd.Col()   > aTotal.aEnd.Col()  ) aTotal.aEnd.SetCol(  aRange.aEnd.Col()  );
            if (aRange.aEnd.Row()   > aTotal.aEnd.Row()  ) aTotal.aEnd.SetRow(  aRange.aEnd.Row()  );
            if (aRange.aEnd.Tab()   > aTotal.aEnd.Tab()  ) aTotal.aEnd.SetTab(aRange.aEnd.Tab()    );
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

ScUndoBorder::~ScUndoBorder()
{
    delete pUndoDoc;
    delete pRanges;
    delete pOuter;
    delete pInner;
}

rtl::OUString ScUndoBorder::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_SELATTRLINES );     //! eigener String?
}

void ScUndoBorder::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    ScMarkData aMarkData;
    aMarkData.MarkFromRangeList( *pRanges, false );
    pUndoDoc->CopyToDocument( aBlockRange, IDF_ATTRIB, sal_True, pDoc, &aMarkData );
    pDocShell->PostPaint( aBlockRange, PAINT_GRID, SC_PF_LINES | SC_PF_TESTMERGE );

    EndUndo();
}

void ScUndoBorder::Redo()
{
    BeginRedo();

    ScDocument* pDoc = pDocShell->GetDocument();        // call function at docfunc
    size_t nCount = pRanges->size();
    for (size_t i = 0; i < nCount; ++i )
    {
        ScRange aRange = *(*pRanges)[i];
        SCTAB nTab = aRange.aStart.Tab();

        ScMarkData aMark;
        aMark.SetMarkArea( aRange );
        aMark.SelectTable( nTab, sal_True );

        pDoc->ApplySelectionFrame( aMark, pOuter, pInner );
    }
    for (size_t i = 0; i < nCount; ++i)
        pDocShell->PostPaint( *(*pRanges)[i], PAINT_GRID, SC_PF_LINES | SC_PF_TESTMERGE );

    EndRedo();
}

void ScUndoBorder::Repeat(SfxRepeatTarget& /* rTarget */)
{
    //TODO later (when the function has moved from cellsuno to docfunc)
}

sal_Bool ScUndoBorder::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;   // See above
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
