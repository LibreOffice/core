/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
#include <brdcst.hxx>
#include "sc.hrc"
#include <rowheightcontext.hxx>
#include <refhint.hxx>
#include <refupdatecontext.hxx>
#include <validat.hxx>
#include <gridwin.hxx>
#include <svl/listener.hxx>

#include <memory>
#include <set>

// TODO:
/*A*/   // SetOptimalHeight on Document, if no View
/*B*/   // linked sheets
/*C*/   // ScArea
//?     // check later

ScUndoInsertCells::ScUndoInsertCells( ScDocShell* pNewDocShell,
                                const ScRange& rRange, SCTAB nNewCount, SCTAB* pNewTabs, SCTAB* pNewScenarios,
                                InsCellCmd eNewCmd, ScDocument* pUndoDocument, ScRefUndoData* pRefData,
                                bool bNewPartOfPaste ) :
    ScMoveUndo( pNewDocShell, pUndoDocument, pRefData, SC_UNDO_REFLAST ),
    aEffRange( rRange ),
    nCount( nNewCount ),
    pTabs( pNewTabs ),
    pScenarios( pNewScenarios ),
    eCmd( eNewCmd ),
    bPartOfPaste( bNewPartOfPaste ),
    pPasteUndo( nullptr )
{
    if (eCmd == INS_INSROWS_BEFORE || eCmd == INS_INSROWS_AFTER)            // whole row?
    {
        aEffRange.aStart.SetCol(0);
        aEffRange.aEnd.SetCol(MAXCOL);
    }

    if (eCmd == INS_INSCOLS_BEFORE || eCmd == INS_INSCOLS_AFTER)            // whole column?
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

OUString ScUndoInsertCells::GetComment() const
{
    return ScGlobal::GetRscString( pPasteUndo ? STR_UNDO_PASTE : STR_UNDO_INSERTCELLS );
}

bool ScUndoInsertCells::Merge( SfxUndoAction* pNextAction )
{
    //  If a paste undo action has already been added, append (detective) action there.
    if ( pPasteUndo )
        return pPasteUndo->Merge( pNextAction );

    if ( bPartOfPaste && dynamic_cast<const ScUndoWrapper*>( pNextAction) !=  nullptr )
    {
        ScUndoWrapper* pWrapper = static_cast<ScUndoWrapper*>(pNextAction);
        SfxUndoAction* pWrappedAction = pWrapper->GetWrappedUndo();
        if ( pWrappedAction && dynamic_cast<const ScUndoPaste*>( pWrappedAction) !=  nullptr )
        {
            //  Store paste action if this is part of paste with inserting cells.
            //  A list action isn't used because Repeat wouldn't work (insert wrong cells).

            pPasteUndo = pWrappedAction;
            pWrapper->ForgetWrappedUndo();      // pWrapper is deleted by UndoManager
            return true;
        }
    }

    //  Call base class for detective handling
    return ScMoveUndo::Merge( pNextAction );
}

void ScUndoInsertCells::SetChangeTrack()
{
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument().GetChangeTrack();
    if ( pChangeTrack )
    {
        pChangeTrack->AppendInsert( aEffRange );
        nEndChangeAction = pChangeTrack->GetActionMax();
    }
    else
        nEndChangeAction = 0;
}

void ScUndoInsertCells::DoChange( const bool bUndo )
{
    ScDocument& rDoc = pDocShell->GetDocument();
    SCTAB i;

    if ( bUndo )
    {
        ScChangeTrack* pChangeTrack = rDoc.GetChangeTrack();
        if ( pChangeTrack )
            pChangeTrack->Undo( nEndChangeAction, nEndChangeAction );
    }
    else
        SetChangeTrack();

    // refresh of merged cells has to be after inserting/deleting

    switch (eCmd)
    {
        case INS_INSROWS_BEFORE:
        case INS_INSROWS_AFTER:
        case INS_CELLSDOWN:
            for( i=0; i<nCount; i++ )
            {
                if (bUndo)
                    rDoc.DeleteRow( aEffRange.aStart.Col(), pTabs[i], aEffRange.aEnd.Col(), pTabs[i]+pScenarios[i],
                    aEffRange.aStart.Row(), static_cast<SCSIZE>(aEffRange.aEnd.Row()-aEffRange.aStart.Row()+1));
                else
                    rDoc.InsertRow( aEffRange.aStart.Col(), pTabs[i], aEffRange.aEnd.Col(), pTabs[i]+pScenarios[i],
                    aEffRange.aStart.Row(), static_cast<SCSIZE>(aEffRange.aEnd.Row()-aEffRange.aStart.Row()+1));
            }
            break;
        case INS_INSCOLS_BEFORE:
        case INS_INSCOLS_AFTER:
        case INS_CELLSRIGHT:
            for( i=0; i<nCount; i++ )
            {
                if (bUndo)
                    rDoc.DeleteCol( aEffRange.aStart.Row(), pTabs[i], aEffRange.aEnd.Row(), pTabs[i]+pScenarios[i],
                    aEffRange.aStart.Col(), static_cast<SCSIZE>(aEffRange.aEnd.Col()-aEffRange.aStart.Col()+1));
                else
                    rDoc.InsertCol( aEffRange.aStart.Row(), pTabs[i], aEffRange.aEnd.Row(), pTabs[i]+pScenarios[i],
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
        if ( rDoc.HasAttrib( aWorkRange.aStart.Col(), aWorkRange.aStart.Row(), pTabs[i],
            aWorkRange.aEnd.Col(), aWorkRange.aEnd.Row(), pTabs[i], HASATTR_MERGED ) )
        {
            SCCOL nEndCol = aWorkRange.aEnd.Col();
            SCROW nEndRow = aWorkRange.aEnd.Row();
            rDoc.ExtendMerge( aWorkRange.aStart.Col(), aWorkRange.aStart.Row(), nEndCol, nEndRow, pTabs[i], true );
        }
    }

    // Undo for displaced attributes?

    sal_uInt16 nPaint = PAINT_GRID;
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    switch (eCmd)
    {
        case INS_INSROWS_BEFORE:
        case INS_INSROWS_AFTER:
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
        case INS_INSCOLS_BEFORE:
        case INS_INSCOLS_AFTER:
            nPaint |= PAINT_TOP;                // top bar
            SAL_FALLTHROUGH;
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

    WaitObject aWait( ScDocShell::GetActiveDialogParent() );     // important due to TrackFormulas in UpdateReference
    BeginUndo();
    DoChange( true );
    EndUndo();

    ScDocument& rDoc = pDocShell->GetDocument();
    for (SCTAB i = 0; i < nCount; ++i)
        rDoc.SetDrawPageSize(pTabs[i]);
}

void ScUndoInsertCells::Redo()
{
    WaitObject aWait( ScDocShell::GetActiveDialogParent() );     // important due to TrackFormulas in UpdateReference
    BeginRedo();
    DoChange( false );
    EndRedo();

    if ( pPasteUndo )
        pPasteUndo->Redo();     // redo paste last

    ScDocument& rDoc = pDocShell->GetDocument();
    for (SCTAB i = 0; i < nCount; ++i)
        rDoc.SetDrawPageSize(pTabs[i]);
}

void ScUndoInsertCells::Repeat(SfxRepeatTarget& rTarget)
{
    if (dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr)
    {
        if ( pPasteUndo )
        {
            //  Repeat for paste with inserting cells is handled completely
            //  by the Paste undo action

            pPasteUndo->Repeat( rTarget );
        }
        else
            static_cast<ScTabViewTarget&>(rTarget).GetViewShell()->InsertCells( eCmd );
    }
}

bool ScUndoInsertCells::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
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

OUString ScUndoDeleteCells::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_DELETECELLS ); // "Delete"
}

void ScUndoDeleteCells::SetChangeTrack()
{
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument().GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->AppendDeleteRange( aEffRange, pRefUndoDoc,
            nStartChangeAction, nEndChangeAction );
    else
        nStartChangeAction = nEndChangeAction = 0;
}

void ScUndoDeleteCells::DoChange( const bool bUndo )
{
    ScDocument& rDoc = pDocShell->GetDocument();
    SCTAB i;

    if ( bUndo )
    {
        ScChangeTrack* pChangeTrack = rDoc.GetChangeTrack();
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
                    rDoc.InsertRow( aEffRange.aStart.Col(), pTabs[i], aEffRange.aEnd.Col(), pTabs[i]+pScenarios[i],
                    aEffRange.aStart.Row(), static_cast<SCSIZE>(aEffRange.aEnd.Row()-aEffRange.aStart.Row()+1));
                else
                    rDoc.DeleteRow( aEffRange.aStart.Col(), pTabs[i], aEffRange.aEnd.Col(), pTabs[i]+pScenarios[i],
                    aEffRange.aStart.Row(), static_cast<SCSIZE>(aEffRange.aEnd.Row()-aEffRange.aStart.Row()+1));
            }
            break;
        case DEL_DELCOLS:
        case DEL_CELLSLEFT:
            for( i=0; i<nCount; i++ )
            {
                if (bUndo)
                    rDoc.InsertCol( aEffRange.aStart.Row(), pTabs[i], aEffRange.aEnd.Row(), pTabs[i]+pScenarios[i],
                    aEffRange.aStart.Col(), static_cast<SCSIZE>(aEffRange.aEnd.Col()-aEffRange.aStart.Col()+1));
                else
                    rDoc.DeleteCol( aEffRange.aStart.Row(), pTabs[i], aEffRange.aEnd.Row(), pTabs[i]+pScenarios[i],
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
            InsertDeleteFlags::ALL | InsertDeleteFlags::NOCAPTIONS, false, &rDoc );
    }

    ScRange aWorkRange( aEffRange );
    if ( eCmd == DEL_CELLSLEFT )        // only "shift left" requires refresh of the moved area
        aWorkRange.aEnd.SetCol(MAXCOL);

    for( i=0; i<nCount; i++ )
    {
        if ( rDoc.HasAttrib( aWorkRange.aStart.Col(), aWorkRange.aStart.Row(), pTabs[i],
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
                ScPatternAttr aPattern( rDoc.GetPool() );
                aPattern.GetItemSet().Put( ScMergeFlagAttr() );
                rDoc.ApplyPatternArea( aWorkRange.aStart.Col(), aWorkRange.aStart.Row(),
                    aWorkRange.aEnd.Col(),   aWorkRange.aEnd.Row(),
                    aMarkData, aPattern );
            }

            SCCOL nEndCol = aWorkRange.aEnd.Col();
            SCROW nEndRow = aWorkRange.aEnd.Row();
            rDoc.ExtendMerge( aWorkRange.aStart.Col(), aWorkRange.aStart.Row(), nEndCol, nEndRow, pTabs[i], true );
        }
    }

    // paint
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
            SAL_FALLTHROUGH;
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
    WaitObject aWait( ScDocShell::GetActiveDialogParent() );     // important because of TrackFormulas in UpdateReference
    BeginUndo();
    DoChange( true );
    EndUndo();
    SfxGetpApp()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );

    // Selection not until EndUndo
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        for( SCTAB i=0; i<nCount; i++ )
        {
            pViewShell->MarkRange( ScRange(aEffRange.aStart.Col(), aEffRange.aStart.Row(), pTabs[i], aEffRange.aEnd.Col(), aEffRange.aEnd.Row(), pTabs[i]+pScenarios[i]) );
        }
    }

    ScDocument& rDoc = pDocShell->GetDocument();
    for (SCTAB i = 0; i < nCount; ++i)
        rDoc.SetDrawPageSize(pTabs[i]);
}

void ScUndoDeleteCells::Redo()
{
    WaitObject aWait( ScDocShell::GetActiveDialogParent() );     // important because of TrackFormulas in UpdateReference
    BeginRedo();
    DoChange( false);
    EndRedo();
    SfxGetpApp()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->DoneBlockMode();            // current way

    ScDocument& rDoc = pDocShell->GetDocument();
    for (SCTAB i = 0; i < nCount; ++i)
        rDoc.SetDrawPageSize(pTabs[i]);
}

void ScUndoDeleteCells::Repeat(SfxRepeatTarget& rTarget)
{
    if (dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr)
        static_cast<ScTabViewTarget&>(rTarget).GetViewShell()->DeleteCells( eCmd );
}

bool ScUndoDeleteCells::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
}

// delete cells in multiselection
ScUndoDeleteMulti::ScUndoDeleteMulti(
    ScDocShell* pNewDocShell,
    bool bNewRows, bool bNeedsRefresh, SCTAB nNewTab,
    const std::vector<sc::ColRowSpan>& rSpans,
    ScDocument* pUndoDocument, ScRefUndoData* pRefData ) :
    ScMoveUndo( pNewDocShell, pUndoDocument, pRefData, SC_UNDO_REFLAST ),
    mbRows(bNewRows),
    mbRefresh(bNeedsRefresh),
    nTab( nNewTab ),
    maSpans(rSpans)
{
    SetChangeTrack();
}

ScUndoDeleteMulti::~ScUndoDeleteMulti()
{
}

OUString ScUndoDeleteMulti::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_DELETECELLS );  // like DeleteCells
}

void ScUndoDeleteMulti::DoChange() const
{
    SCCOL nStartCol;
    SCROW nStartRow;
    sal_uInt16 nPaint;
    if (mbRows)
    {
        nStartCol = 0;
        nStartRow = static_cast<SCROW>(maSpans[0].mnStart);
        nPaint = PAINT_GRID | PAINT_LEFT;
    }
    else
    {
        nStartCol = static_cast<SCCOL>(maSpans[0].mnStart);
        nStartRow = 0;
        nPaint = PAINT_GRID | PAINT_TOP;
    }

    if (mbRefresh)
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        SCCOL nEndCol = MAXCOL;
        SCROW nEndRow = MAXROW;
        rDoc.RemoveFlagsTab( nStartCol, nStartRow, nEndCol, nEndRow, nTab, ScMF::Hor | ScMF::Ver );
        rDoc.ExtendMerge( nStartCol, nStartRow, nEndCol, nEndRow, nTab, true );
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
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument().GetChangeTrack();
    if ( pChangeTrack )
    {
        nStartChangeAction = pChangeTrack->GetActionMax() + 1;
        ScRange aRange( 0, 0, nTab, 0, 0, nTab );
        if (mbRows)
            aRange.aEnd.SetCol( MAXCOL );
        else
            aRange.aEnd.SetRow( MAXROW );
        // delete in reverse
        std::vector<sc::ColRowSpan>::const_reverse_iterator ri = maSpans.rbegin(), riEnd = maSpans.rend();
        for (; ri != riEnd; ++ri)
        {
            SCCOLROW nEnd = ri->mnEnd;
            SCCOLROW nStart = ri->mnStart;
            if (mbRows)
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
    WaitObject aWait( ScDocShell::GetActiveDialogParent() );     // important because of TrackFormulas in UpdateReference
    BeginUndo();

    ScDocument& rDoc = pDocShell->GetDocument();

    // reverse delete -> forward insert
    std::vector<sc::ColRowSpan>::const_iterator it = maSpans.begin(), itEnd = maSpans.end();
    for (; it != itEnd; ++it)
    {
        SCCOLROW nStart = it->mnStart;
        SCCOLROW nEnd = it->mnEnd;
        if (mbRows)
            rDoc.InsertRow( 0,nTab, MAXCOL,nTab, nStart,static_cast<SCSIZE>(nEnd-nStart+1) );
        else
            rDoc.InsertCol( 0,nTab, MAXROW,nTab, static_cast<SCCOL>(nStart), static_cast<SCSIZE>(nEnd-nStart+1) );
    }

    it = maSpans.begin();
    for (; it != itEnd; ++it)
    {
        SCCOLROW nStart = it->mnStart;
        SCCOLROW nEnd = it->mnEnd;
        if (mbRows)
            pRefUndoDoc->CopyToDocument( 0,nStart,nTab, MAXCOL,nEnd,nTab, InsertDeleteFlags::ALL,false, &rDoc );
        else
            pRefUndoDoc->CopyToDocument( static_cast<SCCOL>(nStart),0,nTab,
                    static_cast<SCCOL>(nEnd),MAXROW,nTab, InsertDeleteFlags::ALL,false, &rDoc );
    }

    ScChangeTrack* pChangeTrack = rDoc.GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );

    DoChange();

    //! redrawing the selection is not possible at the moment
    //! since no data for selection exist

    EndUndo();
    SfxGetpApp()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );
}

void ScUndoDeleteMulti::Redo()
{
    WaitObject aWait( ScDocShell::GetActiveDialogParent() );     // important because of TrackFormulas in UpdateReference
    BeginRedo();

    ScDocument& rDoc = pDocShell->GetDocument();

    // reverse delete
    std::vector<sc::ColRowSpan>::const_reverse_iterator ri = maSpans.rbegin(), riEnd = maSpans.rend();
    for (; ri != riEnd; ++ri)
    {
        SCCOLROW nEnd = ri->mnEnd;
        SCCOLROW nStart = ri->mnStart;
        if (mbRows)
            rDoc.DeleteRow( 0,nTab, MAXCOL,nTab, nStart,static_cast<SCSIZE>(nEnd-nStart+1) );
        else
            rDoc.DeleteCol( 0,nTab, MAXROW,nTab, static_cast<SCCOL>(nStart), static_cast<SCSIZE>(nEnd-nStart+1) );
    }

    SetChangeTrack();

    DoChange();

    EndRedo();
    SfxGetpApp()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );
}

void ScUndoDeleteMulti::Repeat(SfxRepeatTarget& rTarget)
{
    // if single selection
    if (dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr)
        static_cast<ScTabViewTarget&>(rTarget).GetViewShell()->DeleteCells( DEL_DELROWS );
}

bool ScUndoDeleteMulti::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
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

OUString ScUndoCut::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_CUT ); // "cut"
}

void ScUndoCut::SetChangeTrack()
{
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument().GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->AppendContentRange( aBlockRange, pUndoDoc,
            nStartChangeAction, nEndChangeAction, SC_CACM_CUT );
    else
        nStartChangeAction = nEndChangeAction = 0;
}

void ScUndoCut::DoChange( const bool bUndo )
{
    ScDocument& rDoc = pDocShell->GetDocument();
    sal_uInt16 nExtFlags = 0;

    // do not undo/redo objects and note captions, they are handled via drawing undo
    InsertDeleteFlags nUndoFlags = (InsertDeleteFlags::ALL & ~InsertDeleteFlags::OBJECTS) | InsertDeleteFlags::NOCAPTIONS;

    if (bUndo)  // only for Undo
    {
        //  all sheets - CopyToDocument skips those that don't exist in pUndoDoc
        SCTAB nTabCount = rDoc.GetTableCount();
        ScRange aCopyRange = aExtendedRange;
        aCopyRange.aStart.SetTab(0);
        aCopyRange.aEnd.SetTab(nTabCount-1);
        pUndoDoc->CopyToDocument( aCopyRange, nUndoFlags, false, &rDoc );
        ScChangeTrack* pChangeTrack = rDoc.GetChangeTrack();
        if ( pChangeTrack )
            pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );

        BroadcastChanges(aCopyRange);
    }
    else        // only for Redo
    {
        pDocShell->UpdatePaintExt( nExtFlags, aExtendedRange );
        rDoc.DeleteArea( aBlockRange.aStart.Col(), aBlockRange.aStart.Row(),
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
    DoChange( true );
    EndUndo();
}

void ScUndoCut::Redo()
{
    BeginRedo();
    ScDocument& rDoc = pDocShell->GetDocument();
    EnableDrawAdjust( &rDoc, false );                //! include in ScBlockUndo?
    DoChange( false );
    EnableDrawAdjust( &rDoc, true );                 //! include in ScBlockUndo?
    EndRedo();
}

void ScUndoCut::Repeat(SfxRepeatTarget& rTarget)
{
    if (dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr)
        static_cast<ScTabViewTarget&>(rTarget).GetViewShell()->CutToClip();
}

bool ScUndoCut::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
}

ScUndoPaste::ScUndoPaste( ScDocShell* pNewDocShell, const ScRangeList& rRanges,
                const ScMarkData& rMark,
                ScDocument* pNewUndoDoc, ScDocument* pNewRedoDoc,
                InsertDeleteFlags nNewFlags,
                ScRefUndoData* pRefData,
                bool bRedoIsFilled, const ScUndoPasteOptions* pOptions ) :
    ScMultiBlockUndo( pNewDocShell, rRanges ),
    aMarkData( rMark ),
    pUndoDoc( pNewUndoDoc ),
    pRedoDoc( pNewRedoDoc ),
    nFlags( nNewFlags ),
    pRefUndoData( pRefData ),
    pRefRedoData( nullptr ),
    bRedoFilled( bRedoIsFilled )
{
    if ( pRefUndoData )
        pRefUndoData->DeleteUnchanged( &pDocShell->GetDocument() );

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

OUString ScUndoPaste::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_PASTE ); // "paste"
}

void ScUndoPaste::SetChangeTrack()
{
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument().GetChangeTrack();
    if ( pChangeTrack && (nFlags & InsertDeleteFlags::CONTENTS) )
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
    ScDocument& rDoc = pDocShell->GetDocument();

    //  RefUndoData for redo is created before first undo
    //  (with DeleteUnchanged after the DoUndo call)
    bool bCreateRedoData = ( bUndo && pRefUndoData && !pRefRedoData );
    if ( bCreateRedoData )
        pRefRedoData = new ScRefUndoData( &rDoc );

    ScRefUndoData* pWorkRefData = bUndo ? pRefUndoData : pRefRedoData;

    // Always back-up either all or none of the content for Undo
    InsertDeleteFlags nUndoFlags = InsertDeleteFlags::NONE;
    if (nFlags & InsertDeleteFlags::CONTENTS)
        nUndoFlags |= InsertDeleteFlags::CONTENTS;
    if (nFlags & InsertDeleteFlags::ATTRIB)
        nUndoFlags |= InsertDeleteFlags::ATTRIB;

    // do not undo/redo objects and note captions, they are handled via drawing undo
    nUndoFlags &= ~InsertDeleteFlags::OBJECTS;
    nUndoFlags |= InsertDeleteFlags::NOCAPTIONS;

    bool bPaintAll = false;

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    SCTAB nTabCount = rDoc.GetTableCount();
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
            pRedoDoc->InitUndoSelected( &rDoc, aMarkData, bColInfo, bRowInfo );
        }
        //  read "redo" data from the document in the first undo
        //  all sheets - CopyToDocument skips those that don't exist in pRedoDoc
        for (size_t i = 0, n = maBlockRanges.size(); i < n; ++i)
        {
            ScRange aCopyRange = *maBlockRanges[i];
            aCopyRange.aStart.SetTab(0);
            aCopyRange.aEnd.SetTab(nTabCount-1);
            rDoc.CopyToDocument( aCopyRange, nUndoFlags, false, pRedoDoc );
            bRedoFilled = true;
        }
    }

    sal_uInt16 nExtFlags = 0;
    pDocShell->UpdatePaintExt(nExtFlags, maBlockRanges.Combine());

    rDoc.ForgetNoteCaptions(maBlockRanges);
    aMarkData.MarkToMulti();
    rDoc.DeleteSelection(nUndoFlags, aMarkData, false); // no broadcasting here
    for (size_t i = 0, n = maBlockRanges.size(); i < n; ++i)
        rDoc.BroadcastCells(*maBlockRanges[i], SC_HINT_DATACHANGED);

    aMarkData.MarkToSimple();

    SCTAB nFirstSelected = aMarkData.GetFirstSelected();

    if ( !bUndo && pRedoDoc )       // Redo: UndoToDocument before handling RefData
    {
        for (size_t i = 0, n = maBlockRanges.size(); i < n; ++i)
        {
            ScRange aRange = *maBlockRanges[i];
            aRange.aStart.SetTab(nFirstSelected);
            aRange.aEnd.SetTab(nFirstSelected);
            pRedoDoc->UndoToDocument(aRange, nUndoFlags, false, &rDoc);
            ScMarkData::iterator itr = aMarkData.begin(), itrEnd = aMarkData.end();
            for (; itr != itrEnd && *itr < nTabCount; ++itr)
            {
                if (*itr == nFirstSelected)
                    continue;

                aRange.aStart.SetTab(*itr);
                aRange.aEnd.SetTab(*itr);
                pRedoDoc->CopyToDocument( aRange, nUndoFlags, false, &rDoc );
            }
        }
    }

    if (pWorkRefData)
    {
        pWorkRefData->DoUndo( &rDoc, true );     // true = bSetChartRangeLists for SetChartListenerCollection
        if (!maBlockRanges.empty() &&
            rDoc.RefreshAutoFilter(0, 0, MAXCOL, MAXROW, maBlockRanges[0]->aStart.Tab()))
            bPaintAll = true;
    }

    if ( bCreateRedoData && pRefRedoData )
        pRefRedoData->DeleteUnchanged( &rDoc );

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
                pUndoDoc->UndoToDocument(aRange, nUndoFlags, false, &rDoc);
            }
        }
    }

    if ( bUndo )
    {
        ScChangeTrack* pChangeTrack = rDoc.GetChangeTrack();
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
        rDoc.ExtendMerge(rDrawRange, true);      // only needed for single sheet (text/rtf etc.)
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
    SfxGetpApp()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );
}

void ScUndoPaste::Redo()
{
    BeginRedo();
    ScDocument& rDoc = pDocShell->GetDocument();
    EnableDrawAdjust( &rDoc, false );                //! include in ScBlockUndo?
    DoChange( false );
    EnableDrawAdjust( &rDoc, true );                 //! include in ScBlockUndo?
    EndRedo();
    SfxGetpApp()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );
}

void ScUndoPaste::Repeat(SfxRepeatTarget& rTarget)
{
    if (dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr)
    {
        ScTabViewShell* pViewSh = static_cast<ScTabViewTarget&>(rTarget).GetViewShell();
        ScTransferObj* pOwnClip = ScTransferObj::GetOwnClipboard( pViewSh->GetActiveWin() );
        if (pOwnClip)
        {
            // keep a reference in case the clipboard is changed during PasteFromClip
            css::uno::Reference<css::datatransfer::XTransferable> aOwnClipRef( pOwnClip );
            pViewSh->PasteFromClip( nFlags, pOwnClip->GetDocument(),
                                    aPasteOptions.nFunction, aPasteOptions.bSkipEmpty, aPasteOptions.bTranspose,
                                    aPasteOptions.bAsLink, aPasteOptions.eMoveMode, InsertDeleteFlags::NONE,
                                    true );     // allow warning dialog
        }
    }
}

bool ScUndoPaste::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
}

ScUndoDragDrop::ScUndoDragDrop( ScDocShell* pNewDocShell,
                    const ScRange& rRange, ScAddress aNewDestPos, bool bNewCut,
                    ScDocument* pUndoDocument, ScRefUndoData* pRefData, bool bScenario ) :
    ScMoveUndo( pNewDocShell, pUndoDocument, pRefData, SC_UNDO_REFFIRST ),
    mnPaintExtFlags( 0 ),
    aSrcRange( rRange ),
    bCut( bNewCut ),
    bKeepScenarioFlags( bScenario )
{
    ScAddress aDestEnd(aNewDestPos);
    aDestEnd.IncRow(aSrcRange.aEnd.Row() - aSrcRange.aStart.Row());
    aDestEnd.IncCol(aSrcRange.aEnd.Col() - aSrcRange.aStart.Col());
    aDestEnd.IncTab(aSrcRange.aEnd.Tab() - aSrcRange.aStart.Tab());

    bool bIncludeFiltered = bCut;
    if ( !bIncludeFiltered )
    {
        // find number of non-filtered rows
        SCROW nPastedCount = pDocShell->GetDocument().CountNonFilteredRows(
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

OUString ScUndoDragDrop::GetComment() const
{   // "Move" : "Copy"
    return bCut ?
        ScGlobal::GetRscString( STR_UNDO_MOVE ) :
        ScGlobal::GetRscString( STR_UNDO_COPY );
}

void ScUndoDragDrop::SetChangeTrack()
{
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument().GetChangeTrack();
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
    ScDocument& rDoc = pDocShell->GetDocument();

    if (pViewShell)
    {
        ScopedVclPtrInstance< VirtualDevice > pVirtDev;
        ScViewData& rViewData = pViewShell->GetViewData();
        sc::RowHeightContext aCxt(
            rViewData.GetPPTX(), rViewData.GetPPTY(), rViewData.GetZoomX(), rViewData.GetZoomY(),
            pVirtDev);

        if (rDoc.SetOptimalHeight(aCxt, aRange.aStart.Row(), aRange.aEnd.Row(), aRange.aStart.Tab()))
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

void ScUndoDragDrop::DoUndo( ScRange aRange )
{
    ScDocument& rDoc = pDocShell->GetDocument();

    ScChangeTrack* pChangeTrack = rDoc.GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );

    // Database range before data, so that the Autofilter button match up in ExtendMerge

    ScRange aPaintRange = aRange;
    rDoc.ExtendMerge( aPaintRange );           // before deleting

    pDocShell->UpdatePaintExt(mnPaintExtFlags, aPaintRange);

    // do not undo objects and note captions, they are handled via drawing undo
    InsertDeleteFlags nUndoFlags = (InsertDeleteFlags::ALL & ~InsertDeleteFlags::OBJECTS) | InsertDeleteFlags::NOCAPTIONS;

    // Additionally discard/forget caption ownership during deletion, as
    // Drag&Drop is a special case in that the Undo holds captions of the
    // transferred target range, which would get deleted and
    // SdrGroupUndo::Undo() would attempt to access invalidated captions and
    // crash, tdf#92995
    InsertDeleteFlags nDelFlags = nUndoFlags | InsertDeleteFlags::FORGETCAPTIONS;

    rDoc.DeleteAreaTab( aRange, nDelFlags );
    pRefUndoDoc->CopyToDocument( aRange, nUndoFlags, false, &rDoc );
    if ( rDoc.HasAttrib( aRange, HASATTR_MERGED ) )
        rDoc.ExtendMerge( aRange, true );

    aPaintRange.aEnd.SetCol( std::max( aPaintRange.aEnd.Col(), aRange.aEnd.Col() ) );
    aPaintRange.aEnd.SetRow( std::max( aPaintRange.aEnd.Row(), aRange.aEnd.Row() ) );

    pDocShell->UpdatePaintExt(mnPaintExtFlags, aPaintRange);
    maPaintRanges.Join(aPaintRange);
}

namespace {

class DataChangeNotifier : public std::unary_function<SvtListener*, void>
{
    ScHint maHint;
public:
    DataChangeNotifier() : maHint(SC_HINT_DATACHANGED, ScAddress()) {}

    void operator() ( SvtListener* p )
    {
        p->Notify(maHint);
    }
};

}

void ScUndoDragDrop::Undo()
{
    mnPaintExtFlags = 0;
    maPaintRanges.RemoveAll();

    BeginUndo();

    if (bCut)
    {
        // During undo, we move cells from aDestRange to aSrcRange.

        ScDocument& rDoc = pDocShell->GetDocument();

        SCCOL nColDelta = aSrcRange.aStart.Col() - aDestRange.aStart.Col();
        SCROW nRowDelta = aSrcRange.aStart.Row() - aDestRange.aStart.Row();
        SCTAB nTabDelta = aSrcRange.aStart.Tab() - aDestRange.aStart.Tab();

        sc::RefUpdateContext aCxt(rDoc);
        aCxt.meMode = URM_MOVE;
        aCxt.maRange = aSrcRange;
        aCxt.mnColDelta = nColDelta;
        aCxt.mnRowDelta = nRowDelta;
        aCxt.mnTabDelta = nTabDelta;

        // Global range names.
        ScRangeName* pName = rDoc.GetRangeName();
        if (pName)
            pName->UpdateReference(aCxt);

        SCTAB nTabCount = rDoc.GetTableCount();
        for (SCTAB nTab = 0; nTab < nTabCount; ++nTab)
        {
            // Sheet-local range names.
            pName = rDoc.GetRangeName(nTab);
            if (pName)
                pName->UpdateReference(aCxt, nTab);
        }

        // Notify all listeners of the destination range, and have them update their references.
        sc::RefMovedHint aHint(aDestRange, ScAddress(nColDelta, nRowDelta, nTabDelta), aCxt);
        rDoc.BroadcastRefMoved(aHint);

        ScValidationDataList* pValidList = rDoc.GetValidationList();
        if (pValidList)
        {
            // Update the references of validation entries.
            pValidList->UpdateReference(aCxt);
        }

        DoUndo(aDestRange);
        DoUndo(aSrcRange);

        // Notify all area listeners whose listened areas are partially moved, to
        // recalculate.
        std::vector<SvtListener*> aListeners;
        rDoc.CollectAllAreaListeners(aListeners, aSrcRange, sc::AreaPartialOverlap);

        // Remove any duplicate listener entries.  We must ensure that we notify
        // each unique listener only once.
        std::sort(aListeners.begin(), aListeners.end());
        aListeners.erase(std::unique(aListeners.begin(), aListeners.end()), aListeners.end());

        std::for_each(aListeners.begin(), aListeners.end(), DataChangeNotifier());
    }
    else
        DoUndo(aDestRange);

    for (size_t i = 0; i < maPaintRanges.size(); ++i)
    {
        const ScRange* p = maPaintRanges[i];
        PaintArea(*p, mnPaintExtFlags);
    }

    EndUndo();
    SfxGetpApp()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );
}

void ScUndoDragDrop::Redo()
{
    BeginRedo();

    ScDocument& rDoc = pDocShell->GetDocument();
    std::unique_ptr<ScDocument> pClipDoc(new ScDocument( SCDOCMODE_CLIP ));

    EnableDrawAdjust( &rDoc, false );                //! include in ScBlockUndo?

    // do not undo/redo objects and note captions, they are handled via drawing undo
    InsertDeleteFlags nRedoFlags = (InsertDeleteFlags::ALL & ~InsertDeleteFlags::OBJECTS) | InsertDeleteFlags::NOCAPTIONS;

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
        aSourceMark.SelectTable( nTab, true );

    // do not clone objects and note captions into clipdoc (see above)
    // but at least copy notes
    ScClipParam aClipParam(aSrcRange, bCut);
    rDoc.CopyToClip(aClipParam, pClipDoc.get(), &aSourceMark, bKeepScenarioFlags, false);

    if (bCut)
    {
        ScRange aSrcPaintRange = aSrcRange;
        rDoc.ExtendMerge( aSrcPaintRange );            // before deleting
        sal_uInt16 nExtFlags = 0;
        pDocShell->UpdatePaintExt( nExtFlags, aSrcPaintRange );
        rDoc.DeleteAreaTab( aSrcRange, nRedoFlags );
        PaintArea( aSrcPaintRange, nExtFlags );
    }

    ScMarkData aDestMark;
    for (nTab=aDestRange.aStart.Tab(); nTab<=aDestRange.aEnd.Tab(); nTab++)
        aDestMark.SelectTable( nTab, true );

    bool bIncludeFiltered = bCut;
    // TODO: restore old note captions instead of cloning new captions...
    rDoc.CopyFromClip( aDestRange, aDestMark, InsertDeleteFlags::ALL & ~InsertDeleteFlags::OBJECTS, nullptr, pClipDoc.get(), true, false, bIncludeFiltered );

    if (bCut)
        for (nTab=aSrcRange.aStart.Tab(); nTab<=aSrcRange.aEnd.Tab(); nTab++)
            rDoc.RefreshAutoFilter( aSrcRange.aStart.Col(), aSrcRange.aStart.Row(),
                                     aSrcRange.aEnd.Col(),   aSrcRange.aEnd.Row(), nTab );

    // skipped rows and merged cells don't mix
    if ( !bIncludeFiltered && pClipDoc->HasClipFilteredRows() )
        pDocShell->GetDocFunc().UnmergeCells( aDestRange, false );

    for (nTab=aDestRange.aStart.Tab(); nTab<=aDestRange.aEnd.Tab(); nTab++)
    {
        SCCOL nEndCol = aDestRange.aEnd.Col();
        SCROW nEndRow = aDestRange.aEnd.Row();
        rDoc.ExtendMerge( aDestRange.aStart.Col(), aDestRange.aStart.Row(),
                            nEndCol, nEndRow, nTab, true );
        PaintArea( ScRange( aDestRange.aStart.Col(), aDestRange.aStart.Row(), nTab,
                            nEndCol, nEndRow, nTab ), 0 );
    }

    SetChangeTrack();

    pClipDoc.reset();
    ShowTable( aDestRange.aStart.Tab() );

    RedoSdrUndoAction( pDrawUndo );             //! include in ScBlockUndo?
    EnableDrawAdjust( &rDoc, true );             //! include in ScBlockUndo?

    EndRedo();
    SfxGetpApp()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );
}

void ScUndoDragDrop::Repeat(SfxRepeatTarget& /* rTarget */)
{
}

bool ScUndoDragDrop::CanRepeat(SfxRepeatTarget& /* rTarget */) const
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

OUString ScUndoListNames::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_LISTNAMES );
}

void ScUndoListNames::DoChange( ScDocument* pSrcDoc ) const
{
    ScDocument& rDoc = pDocShell->GetDocument();

    rDoc.DeleteAreaTab( aBlockRange, InsertDeleteFlags::ALL );
    pSrcDoc->CopyToDocument( aBlockRange, InsertDeleteFlags::ALL, false, &rDoc );
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
    if (dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr)
        static_cast<ScTabViewTarget&>(rTarget).GetViewShell()->InsertNameList();
}

bool ScUndoListNames::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
}

ScUndoConditionalFormat::ScUndoConditionalFormat(ScDocShell* pNewDocShell,
        ScDocument* pUndoDoc, ScDocument* pRedoDoc, const ScRange& rRange):
    ScSimpleUndo( pNewDocShell ),
    mpUndoDoc(pUndoDoc),
    mpRedoDoc(pRedoDoc),
    maRange(rRange)
{
}

ScUndoConditionalFormat::~ScUndoConditionalFormat()
{
}

OUString ScUndoConditionalFormat::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_CONDFORMAT );
}

void ScUndoConditionalFormat::Undo()
{
    DoChange(mpUndoDoc.get());
}

void ScUndoConditionalFormat::Redo()
{
    DoChange(mpRedoDoc.get());
}

void ScUndoConditionalFormat::DoChange(ScDocument* pSrcDoc)
{
    ScDocument& rDoc = pDocShell->GetDocument();

    rDoc.DeleteAreaTab( maRange, InsertDeleteFlags::ALL );
    pSrcDoc->CopyToDocument( maRange, InsertDeleteFlags::ALL, false, &rDoc );
    pDocShell->PostPaint( maRange, PAINT_GRID );
    pDocShell->PostDataChanged();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->CellContentChanged();
}

void ScUndoConditionalFormat::Repeat(SfxRepeatTarget& )
{
}

bool ScUndoConditionalFormat::CanRepeat(SfxRepeatTarget& ) const
{
    return false;
}

ScUndoUseScenario::ScUndoUseScenario( ScDocShell* pNewDocShell,
                        const ScMarkData& rMark,
/*C*/                   const ScArea& rDestArea,
                              ScDocument* pNewUndoDoc,
                        const OUString& rNewName ) :
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

OUString ScUndoUseScenario::GetComment() const
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

    ScDocument& rDoc = pDocShell->GetDocument();
    rDoc.DeleteSelection( InsertDeleteFlags::ALL, aMarkData );
    pUndoDoc->CopyToDocument( aRange, InsertDeleteFlags::ALL, true, &rDoc, &aMarkData );

    // scenario table
    bool bFrame = false;
    SCTAB nTab = aRange.aStart.Tab();
    SCTAB nEndTab = nTab;
    while ( pUndoDoc->HasTable(nEndTab+1) && pUndoDoc->IsScenario(nEndTab+1) )
        ++nEndTab;
    for (SCTAB i = nTab+1; i<=nEndTab; i++)
    {
        // Flags always
        OUString aComment;
        Color  aColor;
        ScScenarioFlags nScenFlags;
        pUndoDoc->GetScenarioData( i, aComment, aColor, nScenFlags );
        rDoc.SetScenarioData( i, aComment, aColor, nScenFlags );
        bool bActive = pUndoDoc->IsActiveScenario( i );
        rDoc.SetActiveScenario( i, bActive );
        //  For copy-back scenario also consider content
        if ( nScenFlags & ScScenarioFlags::TwoWay )
        {
            rDoc.DeleteAreaTab( 0,0, MAXCOL,MAXROW, i, InsertDeleteFlags::ALL );
            pUndoDoc->CopyToDocument( 0,0,i, MAXCOL,MAXROW,i, InsertDeleteFlags::ALL,false, &rDoc );
        }
        if ( nScenFlags & ScScenarioFlags::ShowFrame )
            bFrame = true;
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
    if (dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr)
    {
        OUString aTemp = aName;
        static_cast<ScTabViewTarget&>(rTarget).GetViewShell()->UseScenario(aTemp);
    }
}

bool ScUndoUseScenario::CanRepeat(SfxRepeatTarget& rTarget) const
{
    if (dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr)
    {
        ScViewData& rViewData = static_cast<ScTabViewTarget&>(rTarget).GetViewShell()->GetViewData();
        return !rViewData.GetDocument()->IsScenario( rViewData.GetTabNo() );
    }
    return false;
}

ScUndoSelectionStyle::ScUndoSelectionStyle( ScDocShell* pNewDocShell,
                                      const ScMarkData& rMark,
                                      const ScRange& rRange,
                                      const OUString& rName,
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

OUString ScUndoSelectionStyle::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_APPLYCELLSTYLE );
}

void ScUndoSelectionStyle::DoChange( const bool bUndo )
{
    ScDocument& rDoc = pDocShell->GetDocument();

    SetViewMarkData( aMarkData );

    ScRange aWorkRange( aRange );
    if ( rDoc.HasAttrib( aWorkRange, HASATTR_MERGED ) )        // Merged cells?
        rDoc.ExtendMerge( aWorkRange, true );

    sal_uInt16 nExtFlags = 0;
    pDocShell->UpdatePaintExt( nExtFlags, aWorkRange );

    if (bUndo)      // if Undo then push back all old data again
    {
        SCTAB nTabCount = rDoc.GetTableCount();
        ScRange aCopyRange = aWorkRange;
        aCopyRange.aStart.SetTab(0);
        aCopyRange.aEnd.SetTab(nTabCount-1);
        pUndoDoc->CopyToDocument( aCopyRange, InsertDeleteFlags::ATTRIB, true, &rDoc, &aMarkData );
    }
    else            // if Redo, then reapply style
    {
        ScStyleSheetPool* pStlPool = rDoc.GetStyleSheetPool();
        ScStyleSheet* pStyleSheet =
            static_cast<ScStyleSheet*>( pStlPool->Find( aStyleName, SfxStyleFamily::Para ) );
        if (!pStyleSheet)
        {
            OSL_FAIL("StyleSheet not found");
            return;
        }
        rDoc.ApplySelectionStyle( *pStyleSheet, aMarkData );
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
    DoChange( true );
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
    if (dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr)
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        ScStyleSheetPool* pStlPool = rDoc.GetStyleSheetPool();
        ScStyleSheet* pStyleSheet = static_cast<ScStyleSheet*>( pStlPool->
                                            Find( aStyleName, SfxStyleFamily::Para ));
        if (!pStyleSheet)
        {
            OSL_FAIL("StyleSheet not found");
            return;
        }

        ScTabViewShell& rViewShell = *static_cast<ScTabViewTarget&>(rTarget).GetViewShell();
        rViewShell.SetStyleSheetToMarked( pStyleSheet );
    }
}

bool ScUndoSelectionStyle::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
}

sal_uInt16 ScUndoSelectionStyle::GetId() const
{
    return STR_UNDO_APPLYCELLSTYLE;
}

ScUndoEnterMatrix::ScUndoEnterMatrix( ScDocShell* pNewDocShell, const ScRange& rArea,
                                      ScDocument* pNewUndoDoc, const OUString& rForm ) :
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

OUString ScUndoEnterMatrix::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_ENTERMATRIX );
}

void ScUndoEnterMatrix::SetChangeTrack()
{
    ScDocument& rDoc = pDocShell->GetDocument();
    ScChangeTrack* pChangeTrack = rDoc.GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->AppendContentRange( aBlockRange, pUndoDoc,
            nStartChangeAction, nEndChangeAction );
    else
        nStartChangeAction = nEndChangeAction = 0;
}

void ScUndoEnterMatrix::Undo()
{
    BeginUndo();

    ScDocument& rDoc = pDocShell->GetDocument();

    rDoc.DeleteAreaTab( aBlockRange, InsertDeleteFlags::ALL & ~InsertDeleteFlags::NOTE );
    pUndoDoc->CopyToDocument( aBlockRange, InsertDeleteFlags::ALL & ~InsertDeleteFlags::NOTE, false, &rDoc );
    pDocShell->PostPaint( aBlockRange, PAINT_GRID );
    pDocShell->PostDataChanged();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->CellContentChanged();

    ScChangeTrack* pChangeTrack = rDoc.GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );

    EndUndo();
}

void ScUndoEnterMatrix::Redo()
{
    BeginRedo();

    ScDocument& rDoc = pDocShell->GetDocument();

    ScMarkData aDestMark;
    aDestMark.SelectOneTable( aBlockRange.aStart.Tab() );
    aDestMark.SetMarkArea( aBlockRange );

    rDoc.InsertMatrixFormula( aBlockRange.aStart.Col(), aBlockRange.aStart.Row(),
                               aBlockRange.aEnd.Col(),   aBlockRange.aEnd.Row(),
                               aDestMark, aFormula );

    SetChangeTrack();

    EndRedo();
}

void ScUndoEnterMatrix::Repeat(SfxRepeatTarget& rTarget)
{
    if (dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr)
    {
        OUString aTemp = aFormula;
        ScDocument& rDoc = pDocShell->GetDocument();
        static_cast<ScTabViewTarget&>(rTarget).GetViewShell()->EnterMatrix(aTemp, rDoc.GetGrammar());
    }
}

bool ScUndoEnterMatrix::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
}

static ScRange lcl_GetMultiMarkRange( const ScMarkData& rMark )
{
    OSL_ENSURE( rMark.IsMultiMarked(), "wrong mark type" );

    ScRange aRange;
    rMark.GetMultiMarkArea( aRange );
    return aRange;
}

ScUndoIndent::ScUndoIndent( ScDocShell* pNewDocShell, const ScMarkData& rMark,
                            ScDocument* pNewUndoDoc, bool bIncrement ) :
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

OUString ScUndoIndent::GetComment() const
{
    sal_uInt16 nId = bIsIncrement ? STR_UNDO_INC_INDENT : STR_UNDO_DEC_INDENT;
    return ScGlobal::GetRscString( nId );
}

void ScUndoIndent::Undo()
{
    BeginUndo();

    ScDocument& rDoc = pDocShell->GetDocument();
    SCTAB nTabCount = rDoc.GetTableCount();
    ScRange aCopyRange = aBlockRange;
    aCopyRange.aStart.SetTab(0);
    aCopyRange.aEnd.SetTab(nTabCount-1);
    pUndoDoc->CopyToDocument( aCopyRange, InsertDeleteFlags::ATTRIB, true, &rDoc, &aMarkData );
    pDocShell->PostPaint( aBlockRange, PAINT_GRID, SC_PF_LINES | SC_PF_TESTMERGE );

    EndUndo();
}

void ScUndoIndent::Redo()
{
    BeginRedo();

    ScDocument& rDoc = pDocShell->GetDocument();
    rDoc.ChangeSelectionIndent( bIsIncrement, aMarkData );
    pDocShell->PostPaint( aBlockRange, PAINT_GRID, SC_PF_LINES | SC_PF_TESTMERGE );

    EndRedo();
}

void ScUndoIndent::Repeat(SfxRepeatTarget& rTarget)
{
    if (dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr)
        static_cast<ScTabViewTarget&>(rTarget).GetViewShell()->ChangeIndent( bIsIncrement );
}

bool ScUndoIndent::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
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

OUString ScUndoTransliterate::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_TRANSLITERATE );
}

void ScUndoTransliterate::Undo()
{
    BeginUndo();

    ScDocument& rDoc = pDocShell->GetDocument();
    SCTAB nTabCount = rDoc.GetTableCount();
    ScRange aCopyRange = aBlockRange;
    aCopyRange.aStart.SetTab(0);
    aCopyRange.aEnd.SetTab(nTabCount-1);
    pUndoDoc->CopyToDocument( aCopyRange, InsertDeleteFlags::CONTENTS, true, &rDoc, &aMarkData );
    pDocShell->PostPaint( aBlockRange, PAINT_GRID, SC_PF_LINES | SC_PF_TESTMERGE );

    EndUndo();
}

void ScUndoTransliterate::Redo()
{
    BeginRedo();

    ScDocument& rDoc = pDocShell->GetDocument();
    rDoc.TransliterateText( aMarkData, nTransliterationType );
    pDocShell->PostPaint( aBlockRange, PAINT_GRID, SC_PF_LINES | SC_PF_TESTMERGE );

    EndRedo();
}

void ScUndoTransliterate::Repeat(SfxRepeatTarget& rTarget)
{
    if (dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr)
        static_cast<ScTabViewTarget&>(rTarget).GetViewShell()->TransliterateText( nTransliterationType );
}

bool ScUndoTransliterate::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
}

ScUndoClearItems::ScUndoClearItems( ScDocShell* pNewDocShell, const ScMarkData& rMark,
                            ScDocument* pNewUndoDoc, const sal_uInt16* pW ) :
    ScBlockUndo( pNewDocShell, lcl_GetMultiMarkRange(rMark), SC_UNDO_AUTOHEIGHT ),
    aMarkData( rMark ),
    pUndoDoc( pNewUndoDoc ),
    pWhich( nullptr )
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

OUString ScUndoClearItems::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_DELETECONTENTS );
}

void ScUndoClearItems::Undo()
{
    BeginUndo();

    ScDocument& rDoc = pDocShell->GetDocument();
    pUndoDoc->CopyToDocument( aBlockRange, InsertDeleteFlags::ATTRIB, true, &rDoc, &aMarkData );
    pDocShell->PostPaint( aBlockRange, PAINT_GRID, SC_PF_LINES | SC_PF_TESTMERGE );

    EndUndo();
}

void ScUndoClearItems::Redo()
{
    BeginRedo();

    ScDocument& rDoc = pDocShell->GetDocument();
    rDoc.ClearSelectionItems( pWhich, aMarkData );
    pDocShell->PostPaint( aBlockRange, PAINT_GRID, SC_PF_LINES | SC_PF_TESTMERGE );

    EndRedo();
}

void ScUndoClearItems::Repeat(SfxRepeatTarget& rTarget)
{
    if (dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr)
    {
        ScViewData& rViewData = static_cast<ScTabViewTarget&>(rTarget).GetViewShell()->GetViewData();
        rViewData.GetDocFunc().ClearItems( rViewData.GetMarkData(), pWhich, false );
    }
}

bool ScUndoClearItems::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
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

OUString ScUndoRemoveBreaks::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_REMOVEBREAKS );
}

void ScUndoRemoveBreaks::Undo()
{
    BeginUndo();

    ScDocument& rDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    pUndoDoc->CopyToDocument( 0,0,nTab, MAXCOL,MAXROW,nTab, InsertDeleteFlags::NONE, false, &rDoc );
    if (pViewShell)
        pViewShell->UpdatePageBreakData( true );
    pDocShell->PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, PAINT_GRID );

    EndUndo();
}

void ScUndoRemoveBreaks::Redo()
{
    BeginRedo();

    ScDocument& rDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    rDoc.RemoveManualBreaks(nTab);
    rDoc.UpdatePageBreaks(nTab);
    if (pViewShell)
        pViewShell->UpdatePageBreakData( true );
    pDocShell->PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, PAINT_GRID );

    EndRedo();
}

void ScUndoRemoveBreaks::Repeat(SfxRepeatTarget& rTarget)
{
    if (dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr)
    {
        ScTabViewShell& rViewShell = *static_cast<ScTabViewTarget&>(rTarget).GetViewShell();
        rViewShell.RemoveManualBreaks();
    }
}

bool ScUndoRemoveBreaks::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
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

OUString ScUndoRemoveMerge::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_REMERGE );  // "remove merge"
}

void ScUndoRemoveMerge::Undo()
{
    using ::std::set;

    SetCurTab();
    BeginUndo();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    ScDocument& rDoc = pDocShell->GetDocument();
    for (set<SCTAB>::const_iterator itr = maOption.maTabs.begin(), itrEnd = maOption.maTabs.end();
          itr != itrEnd; ++itr)
    {
        OSL_ENSURE(pUndoDoc, "NULL pUndoDoc!");
        if (!pUndoDoc)
            continue;
        // There is no need to extend merge area because it's already been extended.
        ScRange aRange = maOption.getSingleRange(*itr);
        rDoc.DeleteAreaTab(aRange, InsertDeleteFlags::ATTRIB);
        pUndoDoc->CopyToDocument(aRange, InsertDeleteFlags::ATTRIB, false, &rDoc);

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

    ScDocument& rDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    for (set<SCTAB>::const_iterator itr = maOption.maTabs.begin(), itrEnd = maOption.maTabs.end();
          itr != itrEnd; ++itr)
    {
        SCTAB nTab = *itr;
        // There is no need to extend merge area because it's already been extended.
        ScRange aRange = maOption.getSingleRange(nTab);

        const SfxPoolItem& rDefAttr = rDoc.GetPool()->GetDefaultItem( ATTR_MERGE );
        ScPatternAttr aPattern( rDoc.GetPool() );
        aPattern.GetItemSet().Put( rDefAttr );
        rDoc.ApplyPatternAreaTab( maOption.mnStartCol, maOption.mnStartRow,
                                   maOption.mnEndCol, maOption.mnEndRow, nTab,
                                   aPattern );

        rDoc.RemoveFlagsTab( maOption.mnStartCol, maOption.mnStartRow,
                              maOption.mnEndCol, maOption.mnEndRow, nTab,
                              ScMF::Hor | ScMF::Ver );

        rDoc.ExtendMerge(aRange, true);

        //  Paint

        bool bDidPaint = false;
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
    if (dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr)
        static_cast<ScTabViewTarget&>(rTarget).GetViewShell()->RemoveMerge();
}

bool ScUndoRemoveMerge::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
}

void ScUndoRemoveMerge::SetCurTab()
{
    SCTAB nCurTab = ScDocShell::GetCurTab();
    aBlockRange.aStart.SetTab(nCurTab);
    aBlockRange.aEnd.SetTab(nCurTab);
}

/** set only border, for ScRangeList (StarOne) */
static ScRange lcl_TotalRange( const ScRangeList& rRanges )
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

OUString ScUndoBorder::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_SELATTRLINES );     //! own string?
}

void ScUndoBorder::Undo()
{
    BeginUndo();

    ScDocument& rDoc = pDocShell->GetDocument();
    ScMarkData aMarkData;
    aMarkData.MarkFromRangeList( *pRanges, false );
    pUndoDoc->CopyToDocument( aBlockRange, InsertDeleteFlags::ATTRIB, true, &rDoc, &aMarkData );
    pDocShell->PostPaint( aBlockRange, PAINT_GRID, SC_PF_LINES | SC_PF_TESTMERGE );

    EndUndo();
}

void ScUndoBorder::Redo()
{
    BeginRedo();

    ScDocument& rDoc = pDocShell->GetDocument();        // call function at docfunc
    size_t nCount = pRanges->size();
    for (size_t i = 0; i < nCount; ++i )
    {
        ScRange aRange = *(*pRanges)[i];
        SCTAB nTab = aRange.aStart.Tab();

        ScMarkData aMark;
        aMark.SetMarkArea( aRange );
        aMark.SelectTable( nTab, true );

        rDoc.ApplySelectionFrame( aMark, pOuter, pInner );
    }
    for (size_t i = 0; i < nCount; ++i)
        pDocShell->PostPaint( *(*pRanges)[i], PAINT_GRID, SC_PF_LINES | SC_PF_TESTMERGE );

    EndRedo();
}

void ScUndoBorder::Repeat(SfxRepeatTarget& /* rTarget */)
{
    //TODO later (when the function has moved from cellsuno to docfunc)
}

bool ScUndoBorder::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;   // See above
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
