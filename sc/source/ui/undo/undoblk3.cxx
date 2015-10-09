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

#include <sal/config.h>

#include <memory>
#include <utility>

#include "scitems.hxx"
#include <svx/algitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/justifyitem.hxx>
#include <svl/srchitem.hxx>
#include <sfx2/linkmgr.hxx>
#include <sfx2/bindings.hxx>
#include <vcl/virdev.hxx>
#include <sfx2/app.hxx>

#include "undoblk.hxx"
#include "sc.hrc"
#include "globstr.hrc"
#include "global.hxx"
#include "rangenam.hxx"
#include "arealink.hxx"
#include "patattr.hxx"
#include "target.hxx"
#include "document.hxx"
#include "docpool.hxx"
#include "table.hxx"
#include "docsh.hxx"
#include "tabvwsh.hxx"
#include "undoolk.hxx"
#include "undoutil.hxx"
#include "chgtrack.hxx"
#include "dociter.hxx"
#include "formulacell.hxx"
#include "paramisc.hxx"
#include "postit.hxx"
#include "docuno.hxx"
#include "progress.hxx"
#include "editutil.hxx"
#include "editdataarray.hxx"
#include <rowheightcontext.hxx>

// STATIC DATA ---------------------------------------------------------------

TYPEINIT1(ScUndoDeleteContents,     SfxUndoAction);
TYPEINIT1(ScUndoFillTable,          SfxUndoAction);
TYPEINIT1(ScUndoSelectionAttr,      SfxUndoAction);
TYPEINIT1(ScUndoAutoFill,           SfxUndoAction);
TYPEINIT1(ScUndoMerge,              SfxUndoAction);
TYPEINIT1(ScUndoAutoFormat,         SfxUndoAction);
TYPEINIT1(ScUndoReplace,            SfxUndoAction);
TYPEINIT1(ScUndoTabOp,              SfxUndoAction);
TYPEINIT1(ScUndoConversion,         SfxUndoAction);
TYPEINIT1(ScUndoRefConversion,      SfxUndoAction);
TYPEINIT1(ScUndoRefreshLink,        SfxUndoAction);
TYPEINIT1(ScUndoInsertAreaLink,     SfxUndoAction);
TYPEINIT1(ScUndoRemoveAreaLink,     SfxUndoAction);
TYPEINIT1(ScUndoUpdateAreaLink,     SfxUndoAction);

// TODO:
/*A*/   // SetOptimalHeight on Document, when no View

ScUndoDeleteContents::ScUndoDeleteContents(
                ScDocShell* pNewDocShell,
                const ScMarkData& rMark, const ScRange& rRange,
                std::unique_ptr<ScDocument>&& pNewUndoDoc, bool bNewMulti,
                InsertDeleteFlags nNewFlags, bool bObjects )
    :   ScSimpleUndo( pNewDocShell ),
        aRange      ( rRange ),
        aMarkData   ( rMark ),
        pUndoDoc    ( std::move(pNewUndoDoc) ),
        pDrawUndo   ( NULL ),
        nFlags      ( nNewFlags ),
        bMulti      ( bNewMulti )   // unnecessary
{
    if (bObjects)
        pDrawUndo = GetSdrUndoAction( &pDocShell->GetDocument() );

    if ( !(aMarkData.IsMarked() || aMarkData.IsMultiMarked()) )     // if no cell is selected:
        aMarkData.SetMarkArea( aRange );                            // select cell under cursor

    SetChangeTrack();
}

ScUndoDeleteContents::~ScUndoDeleteContents()
{
    pUndoDoc.reset();
    DeleteSdrUndoAction( pDrawUndo );
}

OUString ScUndoDeleteContents::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_DELETECONTENTS );    // "Delete"
}

void ScUndoDeleteContents::SetDataSpans( const std::shared_ptr<DataSpansType>& pSpans )
{
    mpDataSpans = pSpans;
}

void ScUndoDeleteContents::SetChangeTrack()
{
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument().GetChangeTrack();
    if ( pChangeTrack && (nFlags & IDF_CONTENTS) )
        pChangeTrack->AppendContentRange( aRange, pUndoDoc.get(),
            nStartChangeAction, nEndChangeAction );
    else
        nStartChangeAction = nEndChangeAction = 0;
}

void ScUndoDeleteContents::DoChange( const bool bUndo )
{
    ScDocument& rDoc = pDocShell->GetDocument();

    SetViewMarkData( aMarkData );

    sal_uInt16 nExtFlags = 0;

    if (bUndo)  // only Undo
    {
        InsertDeleteFlags nUndoFlags = IDF_NONE; // copy either all or none of the content
        if (nFlags & IDF_CONTENTS)        // (Only the correct ones have been copied into UndoDoc)
            nUndoFlags |= IDF_CONTENTS;
        if (nFlags & IDF_ATTRIB)
            nUndoFlags |= IDF_ATTRIB;
        if (nFlags & IDF_EDITATTR)          // Edit-Engine attribute
            nUndoFlags |= IDF_STRING;       // -> Cells will be changed
        // do not create clones of note captions, they will be restored via drawing undo
        nUndoFlags |= IDF_NOCAPTIONS;

        ScRange aCopyRange = aRange;
        SCTAB nTabCount = rDoc.GetTableCount();
        aCopyRange.aStart.SetTab(0);
        aCopyRange.aEnd.SetTab(nTabCount-1);

        pUndoDoc->CopyToDocument( aCopyRange, nUndoFlags, bMulti, &rDoc, &aMarkData );

        DoSdrUndoAction( pDrawUndo, &rDoc );

        ScChangeTrack* pChangeTrack = rDoc.GetChangeTrack();
        if ( pChangeTrack )
            pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );

        pDocShell->UpdatePaintExt( nExtFlags, aRange );             // content after the change
    }
    else        // only Redo
    {
        pDocShell->UpdatePaintExt( nExtFlags, aRange );             // content before the change

        aMarkData.MarkToMulti();
        RedoSdrUndoAction( pDrawUndo );
        // do not delete objects and note captions, they have been removed via drawing undo
        InsertDeleteFlags nRedoFlags = (nFlags & ~IDF_OBJECTS) | IDF_NOCAPTIONS;
        rDoc.DeleteSelection( nRedoFlags, aMarkData );
        aMarkData.MarkToSimple();

        SetChangeTrack();
    }

    if (nFlags & IDF_CONTENTS)
    {
        // Broadcast only when the content changes. fdo#74687
        if (mpDataSpans)
            BroadcastChanges(*mpDataSpans);
        else
            BroadcastChanges(aRange);
    }

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if ( !( (pViewShell) && pViewShell->AdjustRowHeight(
                                aRange.aStart.Row(), aRange.aEnd.Row() ) ) )
/*A*/   pDocShell->PostPaint( aRange, PAINT_GRID | PAINT_EXTRAS, nExtFlags );

    if (pViewShell)
        pViewShell->CellContentChanged();

    ShowTable( aRange );
}

void ScUndoDeleteContents::Undo()
{
    BeginUndo();
    DoChange( true );
    EndUndo();

    HelperNotifyChanges::NotifyIfChangesListeners(*pDocShell, aRange);
}

void ScUndoDeleteContents::Redo()
{
    BeginRedo();
    DoChange( false );
    EndRedo();

    HelperNotifyChanges::NotifyIfChangesListeners(*pDocShell, aRange);
}

void ScUndoDeleteContents::Repeat(SfxRepeatTarget& rTarget)
{
    if (dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr)
        static_cast<ScTabViewTarget&>(rTarget).GetViewShell()->DeleteContents( nFlags );
}

bool ScUndoDeleteContents::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
}

ScUndoFillTable::ScUndoFillTable( ScDocShell* pNewDocShell,
                const ScMarkData& rMark,
                SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                SCCOL nEndX, SCROW nEndY, SCTAB nEndZ,
                ScDocument* pNewUndoDoc, bool bNewMulti, SCTAB nSrc,
                InsertDeleteFlags nFlg, ScPasteFunc nFunc, bool bSkip, bool bLink )
    :   ScSimpleUndo( pNewDocShell ),
        aRange      ( nStartX, nStartY, nStartZ, nEndX, nEndY, nEndZ ),
        aMarkData   ( rMark ),
        pUndoDoc    ( pNewUndoDoc ),
        nFlags      ( nFlg ),
        nFunction   ( nFunc ),
        nSrcTab     ( nSrc ),
        bMulti      ( bNewMulti ),
        bSkipEmpty  ( bSkip ),
        bAsLink     ( bLink )
{
    SetChangeTrack();
}

ScUndoFillTable::~ScUndoFillTable()
{
    delete pUndoDoc;
}

OUString ScUndoFillTable::GetComment() const
{
    return ScGlobal::GetRscString( STR_FILL_TAB );
}

void ScUndoFillTable::SetChangeTrack()
{
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument().GetChangeTrack();
    if ( pChangeTrack )
    {
        SCTAB nTabCount = pDocShell->GetDocument().GetTableCount();
        ScRange aWorkRange(aRange);
        nStartChangeAction = 0;
        sal_uLong nTmpAction;
        ScMarkData::iterator itr = aMarkData.begin(), itrEnd = aMarkData.end();
        for (; itr != itrEnd && *itr < nTabCount; ++itr)
        {
            if (*itr != nSrcTab)
            {
                aWorkRange.aStart.SetTab(*itr);
                aWorkRange.aEnd.SetTab(*itr);
                pChangeTrack->AppendContentRange( aWorkRange, pUndoDoc,
                    nTmpAction, nEndChangeAction );
                if ( !nStartChangeAction )
                    nStartChangeAction = nTmpAction;
            }
        }
    }
    else
        nStartChangeAction = nEndChangeAction = 0;
}

void ScUndoFillTable::DoChange( const bool bUndo )
{
    ScDocument& rDoc = pDocShell->GetDocument();

    SetViewMarkData( aMarkData );

    if (bUndo)  // only Undo
    {
        SCTAB nTabCount = rDoc.GetTableCount();
        ScRange aWorkRange(aRange);
        ScMarkData::iterator itr = aMarkData.begin(), itrEnd = aMarkData.end();
        for (; itr != itrEnd && *itr < nTabCount; ++itr)
            if (*itr != nSrcTab)
            {
                aWorkRange.aStart.SetTab(*itr);
                aWorkRange.aEnd.SetTab(*itr);
                if (bMulti)
                    rDoc.DeleteSelectionTab( *itr, IDF_ALL, aMarkData );
                else
                    rDoc.DeleteAreaTab( aWorkRange, IDF_ALL );
                pUndoDoc->CopyToDocument( aWorkRange, IDF_ALL, bMulti, &rDoc, &aMarkData );
            }

        ScChangeTrack* pChangeTrack = rDoc.GetChangeTrack();
        if ( pChangeTrack )
            pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );
    }
    else        // only Redo
    {
        aMarkData.MarkToMulti();
        rDoc.FillTabMarked( nSrcTab, aMarkData, nFlags, nFunction, bSkipEmpty, bAsLink );
        aMarkData.MarkToSimple();
        SetChangeTrack();
    }

    pDocShell->PostPaint(0,0,0,MAXCOL,MAXROW,MAXTAB, PAINT_GRID|PAINT_EXTRAS);
    pDocShell->PostDataChanged();

    //  CellContentChanged comes with the selection

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        SCTAB nTab = pViewShell->GetViewData().GetTabNo();
        if ( !aMarkData.GetTableSelect(nTab) )
            pViewShell->SetTabNo( nSrcTab );

        pViewShell->DoneBlockMode();    // causes problems otherwise since selection is on the wrong sheet.
    }
}

void ScUndoFillTable::Undo()
{
    BeginUndo();
    DoChange( true );
    EndUndo();
}

void ScUndoFillTable::Redo()
{
    BeginRedo();
    DoChange( false );
    EndRedo();
}

void ScUndoFillTable::Repeat(SfxRepeatTarget& rTarget)
{
    if (dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr)
        static_cast<ScTabViewTarget&>(rTarget).GetViewShell()->FillTab( nFlags, nFunction, bSkipEmpty, bAsLink );
}

bool ScUndoFillTable::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
}

ScUndoSelectionAttr::ScUndoSelectionAttr( ScDocShell* pNewDocShell,
                const ScMarkData& rMark,
                SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                SCCOL nEndX, SCROW nEndY, SCTAB nEndZ,
                ScDocument* pNewUndoDoc, bool bNewMulti,
                const ScPatternAttr* pNewApply,
                const SvxBoxItem* pNewOuter, const SvxBoxInfoItem* pNewInner )
    :   ScSimpleUndo( pNewDocShell ),
        aMarkData   ( rMark ),
        aRange      ( nStartX, nStartY, nStartZ, nEndX, nEndY, nEndZ ),
        mpDataArray(new ScEditDataArray),
        pUndoDoc    ( pNewUndoDoc ),
        bMulti      ( bNewMulti )
{
    ScDocumentPool* pPool = pDocShell->GetDocument().GetPool();
    pApplyPattern = const_cast<ScPatternAttr*>(static_cast<const ScPatternAttr*>( &pPool->Put( *pNewApply ) ));
    pLineOuter = pNewOuter ? const_cast<SvxBoxItem*>(static_cast<const SvxBoxItem*>( &pPool->Put( *pNewOuter ) )) : NULL;
    pLineInner = pNewInner ? const_cast<SvxBoxInfoItem*>(static_cast<const SvxBoxInfoItem*>( &pPool->Put( *pNewInner ) )) : NULL;
}

ScUndoSelectionAttr::~ScUndoSelectionAttr()
{
    ScDocumentPool* pPool = pDocShell->GetDocument().GetPool();
    pPool->Remove(*pApplyPattern);
    if (pLineOuter)
        pPool->Remove(*pLineOuter);
    if (pLineInner)
        pPool->Remove(*pLineInner);

    delete pUndoDoc;
}

OUString ScUndoSelectionAttr::GetComment() const
{
    //"Attribute" "/Lines"
    return ScGlobal::GetRscString( pLineOuter ? STR_UNDO_SELATTRLINES : STR_UNDO_SELATTR );
}

ScEditDataArray* ScUndoSelectionAttr::GetDataArray()
{
    return mpDataArray.get();
}

void ScUndoSelectionAttr::DoChange( const bool bUndo )
{
    ScDocument& rDoc = pDocShell->GetDocument();

    SetViewMarkData( aMarkData );

    ScRange aEffRange( aRange );
    if ( rDoc.HasAttrib( aEffRange, HASATTR_MERGED ) )         // merged cells?
        rDoc.ExtendMerge( aEffRange );

    sal_uInt16 nExtFlags = 0;
    pDocShell->UpdatePaintExt( nExtFlags, aEffRange );

    ChangeEditData(bUndo);

    if (bUndo)  // only for Undo
    {
        ScRange aCopyRange = aRange;
        SCTAB nTabCount = rDoc.GetTableCount();
        aCopyRange.aStart.SetTab(0);
        aCopyRange.aEnd.SetTab(nTabCount-1);
        pUndoDoc->CopyToDocument( aCopyRange, IDF_ATTRIB, bMulti, &rDoc, &aMarkData );
    }
    else        // only for Redo
    {
        aMarkData.MarkToMulti();
        rDoc.ApplySelectionPattern( *pApplyPattern, aMarkData );
        aMarkData.MarkToSimple();

        if (pLineOuter)
            rDoc.ApplySelectionFrame( aMarkData, pLineOuter, pLineInner );
    }

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if ( !( (pViewShell) && pViewShell->AdjustBlockHeight() ) )
/*A*/   pDocShell->PostPaint( aEffRange, PAINT_GRID | PAINT_EXTRAS, nExtFlags );

    ShowTable( aRange );
}

void ScUndoSelectionAttr::ChangeEditData( const bool bUndo )
{
    ScDocument& rDoc = pDocShell->GetDocument();
    for (const ScEditDataArray::Item* pItem = mpDataArray->First(); pItem; pItem = mpDataArray->Next())
    {
        ScAddress aPos(pItem->GetCol(), pItem->GetRow(), pItem->GetTab());
        if (rDoc.GetCellType(aPos) != CELLTYPE_EDIT)
            continue;

        if (bUndo)
        {
            if (pItem->GetOldData())
                rDoc.SetEditText(aPos, *pItem->GetOldData(), NULL);
            else
                rDoc.SetEmptyCell(aPos);
        }
        else
        {
            if (pItem->GetNewData())
                rDoc.SetEditText(aPos, *pItem->GetNewData(), NULL);
            else
                rDoc.SetEmptyCell(aPos);
        }
    }
}

void ScUndoSelectionAttr::Undo()
{
    BeginUndo();
    DoChange( true );
    EndUndo();
}

void ScUndoSelectionAttr::Redo()
{
    BeginRedo();
    DoChange( false );
    EndRedo();
}

void ScUndoSelectionAttr::Repeat(SfxRepeatTarget& rTarget)
{
    if (dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr)
    {
        ScTabViewShell& rViewShell = *static_cast<ScTabViewTarget&>(rTarget).GetViewShell();
        if (pLineOuter)
            rViewShell.ApplyPatternLines( *pApplyPattern, pLineOuter, pLineInner );
        else
            rViewShell.ApplySelectionPattern( *pApplyPattern, true );
    }
}

bool ScUndoSelectionAttr::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
}

ScUndoAutoFill::ScUndoAutoFill( ScDocShell* pNewDocShell,
                const ScRange& rRange, const ScRange& rSourceArea,
                ScDocument* pNewUndoDoc, const ScMarkData& rMark,
                FillDir eNewFillDir, FillCmd eNewFillCmd, FillDateCmd eNewFillDateCmd,
                double fNewStartValue, double fNewStepValue, double fNewMaxValue )
    :   ScBlockUndo( pNewDocShell, rRange, SC_UNDO_AUTOHEIGHT ),
        aSource         ( rSourceArea ),
        aMarkData       ( rMark ),
        pUndoDoc        ( pNewUndoDoc ),
        eFillDir        ( eNewFillDir ),
        eFillCmd        ( eNewFillCmd ),
        eFillDateCmd    ( eNewFillDateCmd ),
        fStartValue     ( fNewStartValue ),
        fStepValue      ( fNewStepValue ),
        fMaxValue       ( fNewMaxValue )
{
    SetChangeTrack();
}

ScUndoAutoFill::~ScUndoAutoFill()
{
    delete pUndoDoc;
}

OUString ScUndoAutoFill::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_AUTOFILL ); //"Fill"
}

void ScUndoAutoFill::SetChangeTrack()
{
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument().GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->AppendContentRange( aBlockRange, pUndoDoc,
            nStartChangeAction, nEndChangeAction );
    else
        nStartChangeAction = nEndChangeAction = 0;
}

void ScUndoAutoFill::Undo()
{
    BeginUndo();

    ScDocument& rDoc = pDocShell->GetDocument();

    SCTAB nTabCount = rDoc.GetTableCount();
    ScMarkData::iterator itr = aMarkData.begin(), itrEnd = aMarkData.end();
    for (; itr != itrEnd && *itr < nTabCount; ++itr)
    {
        ScRange aWorkRange = aBlockRange;
        aWorkRange.aStart.SetTab(*itr);
        aWorkRange.aEnd.SetTab(*itr);

        sal_uInt16 nExtFlags = 0;
        pDocShell->UpdatePaintExt( nExtFlags, aWorkRange );
        rDoc.DeleteAreaTab( aWorkRange, IDF_AUTOFILL );
        pUndoDoc->CopyToDocument( aWorkRange, IDF_AUTOFILL, false, &rDoc );

        rDoc.ExtendMerge( aWorkRange, true );
        pDocShell->PostPaint( aWorkRange, PAINT_GRID, nExtFlags );
    }
    pDocShell->PostDataChanged();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->CellContentChanged();

    ScChangeTrack* pChangeTrack = rDoc.GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );

    EndUndo();
}

void ScUndoAutoFill::Redo()
{
    BeginRedo();

//! Select sheet

    SCCOLROW nCount = 0;
    switch (eFillDir)
    {
        case FILL_TO_BOTTOM:
            nCount = aBlockRange.aEnd.Row() - aSource.aEnd.Row();
            break;
        case FILL_TO_RIGHT:
            nCount = aBlockRange.aEnd.Col() - aSource.aEnd.Col();
            break;
        case FILL_TO_TOP:
            nCount = aSource.aStart.Row() - aBlockRange.aStart.Row();
            break;
        case FILL_TO_LEFT:
            nCount = aSource.aStart.Col() - aBlockRange.aStart.Col();
            break;
    }

    ScDocument& rDoc = pDocShell->GetDocument();
    if ( fStartValue != MAXDOUBLE )
    {
        SCCOL nValX = (eFillDir == FILL_TO_LEFT) ? aSource.aEnd.Col() : aSource.aStart.Col();
        SCROW nValY = (eFillDir == FILL_TO_TOP ) ? aSource.aEnd.Row() : aSource.aStart.Row();
        SCTAB nTab = aSource.aStart.Tab();
        rDoc.SetValue( nValX, nValY, nTab, fStartValue );
    }
    sal_uLong nProgCount;
    if (eFillDir == FILL_TO_BOTTOM || eFillDir == FILL_TO_TOP)
        nProgCount = aSource.aEnd.Col() - aSource.aStart.Col() + 1;
    else
        nProgCount = aSource.aEnd.Row() - aSource.aStart.Row() + 1;
    nProgCount *= nCount;
    ScProgress aProgress( rDoc.GetDocumentShell(),
            ScGlobal::GetRscString(STR_FILL_SERIES_PROGRESS), nProgCount );

    rDoc.Fill( aSource.aStart.Col(), aSource.aStart.Row(),
            aSource.aEnd.Col(), aSource.aEnd.Row(), &aProgress,
            aMarkData, nCount,
            eFillDir, eFillCmd, eFillDateCmd,
            fStepValue, fMaxValue );

    SetChangeTrack();

    pDocShell->PostPaint( aBlockRange, PAINT_GRID );
    pDocShell->PostDataChanged();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->CellContentChanged();

    EndRedo();
}

void ScUndoAutoFill::Repeat(SfxRepeatTarget& rTarget)
{
    if (dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr)
    {
        ScTabViewShell& rViewShell = *static_cast<ScTabViewTarget&>(rTarget).GetViewShell();
        if (eFillCmd==FILL_SIMPLE)
            rViewShell.FillSimple( eFillDir );
        else
            rViewShell.FillSeries( eFillDir, eFillCmd, eFillDateCmd,
                                   fStartValue, fStepValue, fMaxValue );
    }
}

bool ScUndoAutoFill::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
}

ScUndoMerge::ScUndoMerge( ScDocShell* pNewDocShell, const ScCellMergeOption& rOption,
                          bool bMergeContents, ScDocument* pUndoDoc, SdrUndoAction* pDrawUndo )
    :   ScSimpleUndo( pNewDocShell ),
        maOption(rOption),
        mbMergeContents( bMergeContents ),
        mpUndoDoc( pUndoDoc ),
        mpDrawUndo( pDrawUndo )
{
}

ScUndoMerge::~ScUndoMerge()
{
    delete mpUndoDoc;
    DeleteSdrUndoAction( mpDrawUndo );
}

OUString ScUndoMerge::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_MERGE );
}

void ScUndoMerge::DoChange( bool bUndo ) const
{
    using ::std::set;

    if (maOption.maTabs.empty())
        // Nothing to do.
        return;

    ScDocument& rDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    ScRange aCurRange = maOption.getSingleRange(ScDocShell::GetCurTab());
    ScUndoUtil::MarkSimpleBlock(pDocShell, aCurRange);

    for (set<SCTAB>::const_iterator itr = maOption.maTabs.begin(), itrEnd = maOption.maTabs.end();
          itr != itrEnd; ++itr)
    {
        SCTAB nTab = *itr;
        ScRange aRange = maOption.getSingleRange(nTab);

        if (bUndo)
            // remove merge (contents are copied back below from undo document)
            rDoc.RemoveMerge( aRange.aStart.Col(), aRange.aStart.Row(), aRange.aStart.Tab() );
        else
        {
            // repeat merge, but do not remove note captions (will be done by drawing redo below)
            rDoc.DoMerge( aRange.aStart.Tab(),
                           aRange.aStart.Col(), aRange.aStart.Row(),
                           aRange.aEnd.Col(),   aRange.aEnd.Row(), false );

            if (maOption.mbCenter)
            {
                rDoc.ApplyAttr( aRange.aStart.Col(), aRange.aStart.Row(),
                                 aRange.aStart.Tab(),
                                 SvxHorJustifyItem( SVX_HOR_JUSTIFY_CENTER, ATTR_HOR_JUSTIFY ) );
                rDoc.ApplyAttr( aRange.aStart.Col(), aRange.aStart.Row(),
                                 aRange.aStart.Tab(),
                                 SvxVerJustifyItem( SVX_VER_JUSTIFY_CENTER, ATTR_VER_JUSTIFY ) );
            }
        }

        // undo -> copy back deleted contents
        if (bUndo && mpUndoDoc)
        {
            rDoc.DeleteAreaTab( aRange, IDF_CONTENTS|IDF_NOCAPTIONS );
            mpUndoDoc->CopyToDocument( aRange, IDF_ALL|IDF_NOCAPTIONS, false, &rDoc );
        }

        // redo -> merge contents again
        else if (!bUndo && mbMergeContents)
        {
            rDoc.DoMergeContents( aRange.aStart.Tab(),
                                   aRange.aStart.Col(), aRange.aStart.Row(),
                                   aRange.aEnd.Col(), aRange.aEnd.Row() );
        }

        if (bUndo)
            DoSdrUndoAction( mpDrawUndo, &rDoc );
        else
            RedoSdrUndoAction( mpDrawUndo );

        bool bDidPaint = false;
        if ( pViewShell )
        {
            pViewShell->SetTabNo(nTab);
            bDidPaint = pViewShell->AdjustRowHeight(maOption.mnStartRow, maOption.mnEndRow);
        }

        if (!bDidPaint)
            ScUndoUtil::PaintMore(pDocShell, aRange);
    }

    ShowTable(aCurRange);
}

void ScUndoMerge::Undo()
{
    BeginUndo();
    DoChange( true );
    EndUndo();
}

void ScUndoMerge::Redo()
{
    BeginRedo();
    DoChange( false );
    EndRedo();
}

void ScUndoMerge::Repeat(SfxRepeatTarget& rTarget)
{
    if (dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr)
    {
        ScTabViewShell& rViewShell = *static_cast<ScTabViewTarget&>(rTarget).GetViewShell();
        bool bCont = false;
        rViewShell.MergeCells( false, bCont, true );
    }
}

bool ScUndoMerge::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
}

ScUndoAutoFormat::ScUndoAutoFormat( ScDocShell* pNewDocShell,
                        const ScRange& rRange, ScDocument* pNewUndoDoc,
                        const ScMarkData& rMark, bool bNewSize, sal_uInt16 nNewFormatNo )
    :   ScBlockUndo( pNewDocShell, rRange, bNewSize ? SC_UNDO_MANUALHEIGHT : SC_UNDO_AUTOHEIGHT ),
        pUndoDoc    ( pNewUndoDoc ),
        aMarkData   ( rMark ),
        bSize       ( bNewSize ),
        nFormatNo   ( nNewFormatNo )
{
}

ScUndoAutoFormat::~ScUndoAutoFormat()
{
    delete pUndoDoc;
}

OUString ScUndoAutoFormat::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_AUTOFORMAT );   //"Auto-Format"
}

void ScUndoAutoFormat::Undo()
{
    BeginUndo();

    ScDocument& rDoc = pDocShell->GetDocument();

    SCTAB nTabCount = rDoc.GetTableCount();
    rDoc.DeleteArea( aBlockRange.aStart.Col(), aBlockRange.aStart.Row(),
                      aBlockRange.aEnd.Col(), aBlockRange.aEnd.Row(),
                      aMarkData, IDF_ATTRIB );
    ScRange aCopyRange = aBlockRange;
    aCopyRange.aStart.SetTab(0);
    aCopyRange.aEnd.SetTab(nTabCount-1);
    pUndoDoc->CopyToDocument( aCopyRange, IDF_ATTRIB, false, &rDoc, &aMarkData );

    // cell heights and widths (IDF_NONE)
    if (bSize)
    {
        SCCOL nStartX = aBlockRange.aStart.Col();
        SCROW nStartY = aBlockRange.aStart.Row();
        SCTAB nStartZ = aBlockRange.aStart.Tab();
        SCCOL nEndX = aBlockRange.aEnd.Col();
        SCROW nEndY = aBlockRange.aEnd.Row();
        SCTAB nEndZ = aBlockRange.aEnd.Tab();

        pUndoDoc->CopyToDocument( nStartX, 0, 0, nEndX, MAXROW, nTabCount-1,
                                    IDF_NONE, false, &rDoc, &aMarkData );
        pUndoDoc->CopyToDocument( 0, nStartY, 0, MAXCOL, nEndY, nTabCount-1,
                                    IDF_NONE, false, &rDoc, &aMarkData );
        pDocShell->PostPaint( 0, 0, nStartZ, MAXCOL, MAXROW, nEndZ,
                              PAINT_GRID | PAINT_LEFT | PAINT_TOP, SC_PF_LINES );
    }
    else
        pDocShell->PostPaint( aBlockRange, PAINT_GRID, SC_PF_LINES );

    EndUndo();
}

void ScUndoAutoFormat::Redo()
{
    BeginRedo();

    ScDocument& rDoc = pDocShell->GetDocument();

    SCCOL nStartX = aBlockRange.aStart.Col();
    SCROW nStartY = aBlockRange.aStart.Row();
    SCTAB nStartZ = aBlockRange.aStart.Tab();
    SCCOL nEndX = aBlockRange.aEnd.Col();
    SCROW nEndY = aBlockRange.aEnd.Row();
    SCTAB nEndZ = aBlockRange.aEnd.Tab();

    rDoc.AutoFormat( nStartX, nStartY, nEndX, nEndY, nFormatNo, aMarkData );

    if (bSize)
    {
        ScopedVclPtrInstance< VirtualDevice > pVirtDev;
        Fraction aZoomX(1,1);
        Fraction aZoomY = aZoomX;
        double nPPTX,nPPTY;
        ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
        if (pViewShell)
        {
            ScViewData& rData = pViewShell->GetViewData();
            nPPTX = rData.GetPPTX();
            nPPTY = rData.GetPPTY();
            aZoomX = rData.GetZoomX();
            aZoomY = rData.GetZoomY();
        }
        else
        {
            // Keep zoom at 100
            nPPTX = ScGlobal::nScreenPPTX;
            nPPTY = ScGlobal::nScreenPPTY;
        }

        bool bFormula = false;  // remember

        sc::RowHeightContext aCxt(nPPTX, nPPTY, aZoomX, aZoomY, pVirtDev);
        for (SCTAB nTab=nStartZ; nTab<=nEndZ; nTab++)
        {
            ScMarkData aDestMark;
            aDestMark.SelectOneTable( nTab );
            aDestMark.SetMarkArea( ScRange( nStartX, nStartY, nTab, nEndX, nEndY, nTab ) );
            aDestMark.MarkToMulti();

            // as SC_SIZE_VISOPT
            for (SCROW nRow=nStartY; nRow<=nEndY; nRow++)
            {
                sal_uInt8 nOld = rDoc.GetRowFlags(nRow,nTab);
                bool bHidden = rDoc.RowHidden(nRow, nTab);
                if ( !bHidden && ( nOld & CR_MANUALSIZE ) )
                    rDoc.SetRowFlags( nRow, nTab, nOld & ~CR_MANUALSIZE );
            }

            rDoc.SetOptimalHeight(aCxt, nStartY, nEndY, nTab);

            for (SCCOL nCol=nStartX; nCol<=nEndX; nCol++)
                if (!rDoc.ColHidden(nCol, nTab))
                {
                    sal_uInt16 nThisSize = STD_EXTRA_WIDTH + rDoc.GetOptimalColWidth( nCol, nTab,
                                                pVirtDev, nPPTX, nPPTY, aZoomX, aZoomY, bFormula,
                                                &aDestMark );
                    rDoc.SetColWidth( nCol, nTab, nThisSize );
                    rDoc.ShowCol( nCol, nTab, true );
                }
        }

        pDocShell->PostPaint( 0,      0,      nStartZ,
                              MAXCOL, MAXROW, nEndZ,
                              PAINT_GRID | PAINT_LEFT | PAINT_TOP, SC_PF_LINES);
    }
    else
        pDocShell->PostPaint( aBlockRange, PAINT_GRID, SC_PF_LINES );

    EndRedo();
}

void ScUndoAutoFormat::Repeat(SfxRepeatTarget& rTarget)
{
    if (dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr)
        static_cast<ScTabViewTarget&>(rTarget).GetViewShell()->AutoFormat( nFormatNo );
}

bool ScUndoAutoFormat::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
}

ScUndoReplace::ScUndoReplace( ScDocShell* pNewDocShell, const ScMarkData& rMark,
                                    SCCOL nCurX, SCROW nCurY, SCTAB nCurZ,
                                    const OUString& rNewUndoStr, ScDocument* pNewUndoDoc,
                                    const SvxSearchItem* pItem )
    :   ScSimpleUndo( pNewDocShell ),
        aCursorPos  ( nCurX, nCurY, nCurZ ),
        aMarkData   ( rMark ),
        aUndoStr    ( rNewUndoStr ),
        pUndoDoc    ( pNewUndoDoc )
{
    pSearchItem = new SvxSearchItem( *pItem );
    SetChangeTrack();
}

ScUndoReplace::~ScUndoReplace()
{
    delete pUndoDoc;
    delete pSearchItem;
}

void ScUndoReplace::SetChangeTrack()
{
    ScDocument& rDoc = pDocShell->GetDocument();
    ScChangeTrack* pChangeTrack = rDoc.GetChangeTrack();
    if ( pChangeTrack )
    {
        if ( pUndoDoc )
        {   //! UndoDoc includes only the changed cells,
            // that is why an Iterator can be used
            pChangeTrack->AppendContentsIfInRefDoc( pUndoDoc,
                nStartChangeAction, nEndChangeAction );
        }
        else
        {
            nStartChangeAction = pChangeTrack->GetActionMax() + 1;
            ScChangeActionContent* pContent = new ScChangeActionContent(
                ScRange( aCursorPos) );
            ScCellValue aCell;
            aCell.assign(rDoc, aCursorPos);
            pContent->SetOldValue( aUndoStr, &rDoc );
            pContent->SetNewValue(aCell, &rDoc);
            pChangeTrack->Append( pContent );
            nEndChangeAction = pChangeTrack->GetActionMax();
        }
    }
    else
        nStartChangeAction = nEndChangeAction = 0;
}

OUString ScUndoReplace::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_REPLACE );  // "Replace"
}

void ScUndoReplace::Undo()
{
    BeginUndo();

    ScDocument& rDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    ShowTable( aCursorPos.Tab() );

    if (pUndoDoc)       // only for ReplaceAll !!
    {
        OSL_ENSURE(pSearchItem->GetCommand() == SvxSearchCmd::REPLACE_ALL,
                   "ScUndoReplace:: Wrong Mode");

        SetViewMarkData( aMarkData );

//! selected sheet
//! select range ?

        // Undo document has no row/column information, thus copy with
        // bColRowFlags = FALSE to not destroy Outline groups

        InsertDeleteFlags nUndoFlags = (pSearchItem->GetPattern()) ? IDF_ATTRIB : IDF_CONTENTS;
        pUndoDoc->CopyToDocument( 0,      0,      0,
                                  MAXCOL, MAXROW, MAXTAB,
                                  nUndoFlags, false, &rDoc, NULL, false );   // without row flags
        pDocShell->PostPaintGridAll();
    }
    else if (pSearchItem->GetPattern() &&
             pSearchItem->GetCommand() == SvxSearchCmd::REPLACE)
    {
        OUString aTempStr = pSearchItem->GetSearchString();       // toggle
        pSearchItem->SetSearchString(pSearchItem->GetReplaceString());
        pSearchItem->SetReplaceString(aTempStr);
        rDoc.ReplaceStyle( *pSearchItem,
                            aCursorPos.Col(), aCursorPos.Row(), aCursorPos.Tab(),
                            aMarkData, true);
        pSearchItem->SetReplaceString(pSearchItem->GetSearchString());
        pSearchItem->SetSearchString(aTempStr);
        if (pViewShell)
            pViewShell->MoveCursorAbs( aCursorPos.Col(), aCursorPos.Row(),
                                       SC_FOLLOW_JUMP, false, false );
        pDocShell->PostPaintGridAll();
    }
    else if (pSearchItem->GetCellType() == SvxSearchCellType::NOTE)
    {
        ScPostIt* pNote = rDoc.GetNote(aCursorPos);
        OSL_ENSURE( pNote, "ScUndoReplace::Undo - cell does not contain a note" );
        if (pNote)
            pNote->SetText( aCursorPos, aUndoStr );
        if (pViewShell)
            pViewShell->MoveCursorAbs( aCursorPos.Col(), aCursorPos.Row(),
                                       SC_FOLLOW_JUMP, false, false );
    }
    else
    {
        // aUndoStr may contain line breaks
        if ( aUndoStr.indexOf('\n') != -1 )
        {
            ScFieldEditEngine& rEngine = rDoc.GetEditEngine();
            rEngine.SetText(aUndoStr);
            rDoc.SetEditText(aCursorPos, rEngine.CreateTextObject());
        }
        else
            rDoc.SetString( aCursorPos.Col(), aCursorPos.Row(), aCursorPos.Tab(), aUndoStr );
        if (pViewShell)
            pViewShell->MoveCursorAbs( aCursorPos.Col(), aCursorPos.Row(),
                                       SC_FOLLOW_JUMP, false, false );
        pDocShell->PostPaintGridAll();
    }

    ScChangeTrack* pChangeTrack = rDoc.GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );

    EndUndo();
}

void ScUndoReplace::Redo()
{
    BeginRedo();

    ScDocument& rDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    if (pViewShell)
        pViewShell->MoveCursorAbs( aCursorPos.Col(), aCursorPos.Row(),
                                   SC_FOLLOW_JUMP, false, false );
    if (pUndoDoc)
    {
        if (pViewShell)
        {
            SetViewMarkData( aMarkData );

            pViewShell->SearchAndReplace( pSearchItem, false, true );
        }
    }
    else if (pSearchItem->GetPattern() &&
             pSearchItem->GetCommand() == SvxSearchCmd::REPLACE)
    {
        rDoc.ReplaceStyle( *pSearchItem,
                            aCursorPos.Col(), aCursorPos.Row(), aCursorPos.Tab(),
                            aMarkData, true);
        pDocShell->PostPaintGridAll();
    }
    else
        if (pViewShell)
            pViewShell->SearchAndReplace( pSearchItem, false, true );

    SetChangeTrack();

    EndRedo();
}

void ScUndoReplace::Repeat(SfxRepeatTarget& rTarget)
{
    if (dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr)
        static_cast<ScTabViewTarget&>(rTarget).GetViewShell()->SearchAndReplace( pSearchItem, true, false );
}

bool ScUndoReplace::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
}

// multi-operation (only simple blocks)
ScUndoTabOp::ScUndoTabOp( ScDocShell* pNewDocShell,
                SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                SCCOL nEndX, SCROW nEndY, SCTAB nEndZ, ScDocument* pNewUndoDoc,
                const ScRefAddress& rFormulaCell,
                const ScRefAddress& rFormulaEnd,
                const ScRefAddress& rRowCell,
                const ScRefAddress& rColCell,
                ScTabOpParam::Mode eMode )
    :   ScSimpleUndo( pNewDocShell ),
        aRange          ( nStartX, nStartY, nStartZ, nEndX, nEndY, nEndZ ),
        pUndoDoc        ( pNewUndoDoc ),
        theFormulaCell  ( rFormulaCell ),
        theFormulaEnd   ( rFormulaEnd ),
        theRowCell      ( rRowCell ),
        theColCell      ( rColCell ),
        meMode(eMode)
{
}

ScUndoTabOp::~ScUndoTabOp()
{
    delete pUndoDoc;
}

OUString ScUndoTabOp::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_TABOP );    // "Multiple operation"
}

void ScUndoTabOp::Undo()
{
    BeginUndo();

    ScUndoUtil::MarkSimpleBlock( pDocShell, aRange );

    sal_uInt16 nExtFlags = 0;
    pDocShell->UpdatePaintExt( nExtFlags, aRange );

    ScDocument& rDoc = pDocShell->GetDocument();
    rDoc.DeleteAreaTab( aRange,IDF_ALL & ~IDF_NOTE );
    pUndoDoc->CopyToDocument( aRange, IDF_ALL & ~IDF_NOTE, false, &rDoc );
    pDocShell->PostPaint( aRange, PAINT_GRID, nExtFlags );
    pDocShell->PostDataChanged();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->CellContentChanged();

    EndUndo();
}

void ScUndoTabOp::Redo()
{
    BeginRedo();

    ScUndoUtil::MarkSimpleBlock( pDocShell, aRange );

    ScTabOpParam aParam(theFormulaCell, theFormulaEnd, theRowCell, theColCell, meMode);

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->TabOp( aParam, false);

    EndRedo();
}

void ScUndoTabOp::Repeat(SfxRepeatTarget& /* rTarget */)
{
}

bool ScUndoTabOp::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;
}

ScUndoConversion::ScUndoConversion(
        ScDocShell* pNewDocShell, const ScMarkData& rMark,
        SCCOL nCurX, SCROW nCurY, SCTAB nCurZ, ScDocument* pNewUndoDoc,
        SCCOL nNewX, SCROW nNewY, SCTAB nNewZ, ScDocument* pNewRedoDoc,
        const ScConversionParam& rConvParam ) :
    ScSimpleUndo( pNewDocShell ),
    aMarkData( rMark ),
    aCursorPos( nCurX, nCurY, nCurZ ),
    pUndoDoc( pNewUndoDoc ),
    aNewCursorPos( nNewX, nNewY, nNewZ ),
    pRedoDoc( pNewRedoDoc ),
    maConvParam( rConvParam )
{
    SetChangeTrack();
}

ScUndoConversion::~ScUndoConversion()
{
    delete pUndoDoc;
    delete pRedoDoc;
}

void ScUndoConversion::SetChangeTrack()
{
    ScDocument& rDoc = pDocShell->GetDocument();
    ScChangeTrack* pChangeTrack = rDoc.GetChangeTrack();
    if ( pChangeTrack )
    {
        if ( pUndoDoc )
            pChangeTrack->AppendContentsIfInRefDoc( pUndoDoc,
                nStartChangeAction, nEndChangeAction );
        else
        {
            OSL_FAIL( "ScUndoConversion::SetChangeTrack: no UndoDoc" );
            nStartChangeAction = nEndChangeAction = 0;
        }
    }
    else
        nStartChangeAction = nEndChangeAction = 0;
}

OUString ScUndoConversion::GetComment() const
{
    OUString aText;
    switch( maConvParam.GetType() )
    {
        case SC_CONVERSION_SPELLCHECK:      aText = ScGlobal::GetRscString( STR_UNDO_SPELLING );    break;
        case SC_CONVERSION_HANGULHANJA:     aText = ScGlobal::GetRscString( STR_UNDO_HANGULHANJA ); break;
        case SC_CONVERSION_CHINESE_TRANSL:  aText = ScGlobal::GetRscString( STR_UNDO_CHINESE_TRANSLATION ); break;
        default: OSL_FAIL( "ScUndoConversion::GetComment - unknown conversion type" );
    }
    return aText;
}

void ScUndoConversion::DoChange( ScDocument* pRefDoc, const ScAddress& rCursorPos )
{
    if (pRefDoc)
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        ShowTable( rCursorPos.Tab() );

        SetViewMarkData( aMarkData );

        SCTAB nTabCount = rDoc.GetTableCount();
        //  Undo/Redo-doc has only selected tables

        bool bMulti = aMarkData.IsMultiMarked();
        pRefDoc->CopyToDocument( 0,      0,      0,
                                 MAXCOL, MAXROW, nTabCount-1,
                                 IDF_CONTENTS, bMulti, &rDoc, &aMarkData );
        pDocShell->PostPaintGridAll();
    }
    else
    {
        OSL_FAIL("no Un-/RedoDoc for Un-/RedoSpelling");
    }
}

void ScUndoConversion::Undo()
{
    BeginUndo();
    DoChange( pUndoDoc, aCursorPos );
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument().GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );
    EndUndo();
}

void ScUndoConversion::Redo()
{
    BeginRedo();
    DoChange( pRedoDoc, aNewCursorPos );
    SetChangeTrack();
    EndRedo();
}

void ScUndoConversion::Repeat( SfxRepeatTarget& rTarget )
{
    if( dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr )
        static_cast<ScTabViewTarget&>(rTarget).GetViewShell()->DoSheetConversion( maConvParam );
}

bool ScUndoConversion::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
}

ScUndoRefConversion::ScUndoRefConversion( ScDocShell* pNewDocShell,
                                         const ScRange& aMarkRange, const ScMarkData& rMark,
                                         ScDocument* pNewUndoDoc, ScDocument* pNewRedoDoc, bool bNewMulti, InsertDeleteFlags nNewFlag) :
ScSimpleUndo( pNewDocShell ),
aMarkData   ( rMark ),
pUndoDoc    ( pNewUndoDoc ),
pRedoDoc    ( pNewRedoDoc ),
aRange      ( aMarkRange ),
bMulti      ( bNewMulti ),
nFlags      ( nNewFlag )
{
    SetChangeTrack();
}

ScUndoRefConversion::~ScUndoRefConversion()
{
    delete pUndoDoc;
    delete pRedoDoc;
}

OUString ScUndoRefConversion::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_ENTERDATA ); // "Input"
}

void ScUndoRefConversion::SetChangeTrack()
{
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument().GetChangeTrack();
    if ( pChangeTrack && (nFlags & IDF_FORMULA) )
        pChangeTrack->AppendContentsIfInRefDoc( pUndoDoc,
            nStartChangeAction, nEndChangeAction );
    else
        nStartChangeAction = nEndChangeAction = 0;
}

void ScUndoRefConversion::DoChange( ScDocument* pRefDoc)
{
    ScDocument& rDoc = pDocShell->GetDocument();

    ShowTable(aRange);

    SetViewMarkData( aMarkData );

    ScRange aCopyRange = aRange;
    SCTAB nTabCount = rDoc.GetTableCount();
    aCopyRange.aStart.SetTab(0);
    aCopyRange.aEnd.SetTab(nTabCount-1);
    pRefDoc->CopyToDocument( aCopyRange, nFlags, bMulti, &rDoc, &aMarkData );
    pDocShell->PostPaint( aRange, PAINT_GRID);
    pDocShell->PostDataChanged();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->CellContentChanged();
}

void ScUndoRefConversion::Undo()
{
    BeginUndo();
    if (pUndoDoc)
        DoChange(pUndoDoc);
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument().GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );
    EndUndo();
}

void ScUndoRefConversion::Redo()
{
    BeginRedo();
    if (pRedoDoc)
        DoChange(pRedoDoc);
    SetChangeTrack();
    EndRedo();
}

void ScUndoRefConversion::Repeat(SfxRepeatTarget& rTarget)
{
    if (dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr)
        static_cast<ScTabViewTarget&>(rTarget).GetViewShell()->DoRefConversion();
}

bool ScUndoRefConversion::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
}

ScUndoRefreshLink::ScUndoRefreshLink( ScDocShell* pNewDocShell,
                                    ScDocument* pNewUndoDoc )
    :   ScSimpleUndo( pNewDocShell ),
        pUndoDoc( pNewUndoDoc ),
        pRedoDoc( NULL )
{
}

ScUndoRefreshLink::~ScUndoRefreshLink()
{
    delete pUndoDoc;
    delete pRedoDoc;
}

OUString ScUndoRefreshLink::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_UPDATELINK );
}

void ScUndoRefreshLink::Undo()
{
    BeginUndo();

    bool bMakeRedo = !pRedoDoc;
    if (bMakeRedo)
        pRedoDoc = new ScDocument( SCDOCMODE_UNDO );

    bool bFirst = true;
    ScDocument& rDoc = pDocShell->GetDocument();
    SCTAB nCount = rDoc.GetTableCount();
    for (SCTAB nTab=0; nTab<nCount; nTab++)
        if (pUndoDoc->HasTable(nTab))
        {
            ScRange aRange(0,0,nTab,MAXCOL,MAXROW,nTab);
            if (bMakeRedo)
            {
                if (bFirst)
                    pRedoDoc->InitUndo( &rDoc, nTab, nTab, true, true );
                else
                    pRedoDoc->AddUndoTab( nTab, nTab, true, true );
                bFirst = false;
                rDoc.CopyToDocument(aRange, IDF_ALL, false, pRedoDoc);
                pRedoDoc->SetLink( nTab,
                                   rDoc.GetLinkMode(nTab),
                                   rDoc.GetLinkDoc(nTab),
                                   rDoc.GetLinkFlt(nTab),
                                   rDoc.GetLinkOpt(nTab),
                                   rDoc.GetLinkTab(nTab),
                                   rDoc.GetLinkRefreshDelay(nTab) );
                pRedoDoc->SetTabBgColor( nTab, rDoc.GetTabBgColor(nTab) );
            }

            rDoc.DeleteAreaTab( aRange,IDF_ALL );
            pUndoDoc->CopyToDocument( aRange, IDF_ALL, false, &rDoc );
            rDoc.SetLink( nTab, pUndoDoc->GetLinkMode(nTab), pUndoDoc->GetLinkDoc(nTab),
                                 pUndoDoc->GetLinkFlt(nTab),  pUndoDoc->GetLinkOpt(nTab),
                                 pUndoDoc->GetLinkTab(nTab),
                                 pUndoDoc->GetLinkRefreshDelay(nTab) );
            rDoc.SetTabBgColor( nTab, pUndoDoc->GetTabBgColor(nTab) );
        }

    pDocShell->PostPaintGridAll();
	pDocShell->PostPaintExtras();

    EndUndo();
}

void ScUndoRefreshLink::Redo()
{
    OSL_ENSURE(pRedoDoc, "No RedoDoc for ScUndoRefreshLink::Redo");

    BeginUndo();

    ScDocument& rDoc = pDocShell->GetDocument();
    SCTAB nCount = rDoc.GetTableCount();
    for (SCTAB nTab=0; nTab<nCount; nTab++)
        if (pRedoDoc->HasTable(nTab))
        {
            ScRange aRange(0,0,nTab,MAXCOL,MAXROW,nTab);

            rDoc.DeleteAreaTab( aRange, IDF_ALL );
            pRedoDoc->CopyToDocument( aRange, IDF_ALL, false, &rDoc );
            rDoc.SetLink( nTab,
                           pRedoDoc->GetLinkMode(nTab),
                           pRedoDoc->GetLinkDoc(nTab),
                           pRedoDoc->GetLinkFlt(nTab),
                           pRedoDoc->GetLinkOpt(nTab),
                           pRedoDoc->GetLinkTab(nTab),
                           pRedoDoc->GetLinkRefreshDelay(nTab) );
            rDoc.SetTabBgColor( nTab, pRedoDoc->GetTabBgColor(nTab) );
        }

    pDocShell->PostPaintGridAll();
	pDocShell->PostPaintExtras();

    EndUndo();
}

void ScUndoRefreshLink::Repeat(SfxRepeatTarget& /* rTarget */)
{
    // makes no sense
}

bool ScUndoRefreshLink::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;
}

static ScAreaLink* lcl_FindAreaLink( sfx2::LinkManager* pLinkManager, const OUString& rDoc,
                            const OUString& rFlt, const OUString& rOpt,
                            const OUString& rSrc, const ScRange& rDest )
{
    const ::sfx2::SvBaseLinks& rLinks = pLinkManager->GetLinks();
    sal_uInt16 nCount = pLinkManager->GetLinks().size();
    for (sal_uInt16 i=0; i<nCount; i++)
    {
        ::sfx2::SvBaseLink* pBase = *rLinks[i];
        if (dynamic_cast<const ScAreaLink*>( pBase) !=  nullptr)
            if ( static_cast<ScAreaLink*>(pBase)->IsEqual( rDoc, rFlt, rOpt, rSrc, rDest ) )
                return static_cast<ScAreaLink*>(pBase);
    }

    OSL_FAIL("ScAreaLink not found");
    return NULL;
}

ScUndoInsertAreaLink::ScUndoInsertAreaLink( ScDocShell* pShell,
                            const OUString& rDoc,
                            const OUString& rFlt, const OUString& rOpt,
                            const OUString& rArea, const ScRange& rDestRange,
                            sal_uLong nRefresh )
    :   ScSimpleUndo    ( pShell ),
        aDocName        ( rDoc ),
        aFltName        ( rFlt ),
        aOptions        ( rOpt ),
        aAreaName       ( rArea ),
        aRange          ( rDestRange ),
        nRefreshDelay   ( nRefresh )
{
}

ScUndoInsertAreaLink::~ScUndoInsertAreaLink()
{
}

OUString ScUndoInsertAreaLink::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_INSERTAREALINK );
}

void ScUndoInsertAreaLink::Undo()
{
    ScDocument& rDoc = pDocShell->GetDocument();
    sfx2::LinkManager* pLinkManager = rDoc.GetLinkManager();

    ScAreaLink* pLink = lcl_FindAreaLink( pLinkManager, aDocName, aFltName, aOptions,
                                            aAreaName, aRange );
    if (pLink)
        pLinkManager->Remove( pLink );

    SfxGetpApp()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );     // Navigator
}

void ScUndoInsertAreaLink::Redo()
{
    ScDocument& rDoc = pDocShell->GetDocument();
    sfx2::LinkManager* pLinkManager = rDoc.GetLinkManager();

    ScAreaLink* pLink = new ScAreaLink( pDocShell, aDocName, aFltName, aOptions,
                                            aAreaName, aRange.aStart, nRefreshDelay );
    pLink->SetInCreate( true );
    pLink->SetDestArea( aRange );
    pLinkManager->InsertFileLink( *pLink, OBJECT_CLIENT_FILE, aDocName, &aFltName, &aAreaName );
    pLink->Update();
    pLink->SetInCreate( false );

    SfxGetpApp()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );     // Navigator
}

void ScUndoInsertAreaLink::Repeat(SfxRepeatTarget& /* rTarget */)
{
    // makes no sense
}

bool ScUndoInsertAreaLink::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;
}

ScUndoRemoveAreaLink::ScUndoRemoveAreaLink( ScDocShell* pShell,
                            const OUString& rDoc, const OUString& rFlt, const OUString& rOpt,
                            const OUString& rArea, const ScRange& rDestRange,
                            sal_uLong nRefresh )
    :   ScSimpleUndo    ( pShell ),
        aDocName        ( rDoc ),
        aFltName        ( rFlt ),
        aOptions        ( rOpt ),
        aAreaName       ( rArea ),
        aRange          ( rDestRange ),
        nRefreshDelay   ( nRefresh )
{
}

ScUndoRemoveAreaLink::~ScUndoRemoveAreaLink()
{
}

OUString ScUndoRemoveAreaLink::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_REMOVELINK );   //! own text ??
}

void ScUndoRemoveAreaLink::Undo()
{
    ScDocument& rDoc = pDocShell->GetDocument();
    sfx2::LinkManager* pLinkManager = rDoc.GetLinkManager();

    ScAreaLink* pLink = new ScAreaLink( pDocShell, aDocName, aFltName, aOptions,
                                        aAreaName, aRange.aStart, nRefreshDelay );
    pLink->SetInCreate( true );
    pLink->SetDestArea( aRange );
    pLinkManager->InsertFileLink( *pLink, OBJECT_CLIENT_FILE, aDocName, &aFltName, &aAreaName );
    pLink->Update();
    pLink->SetInCreate( false );

    SfxGetpApp()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );     // Navigator
}

void ScUndoRemoveAreaLink::Redo()
{
    ScDocument& rDoc = pDocShell->GetDocument();
    sfx2::LinkManager* pLinkManager = rDoc.GetLinkManager();

    ScAreaLink* pLink = lcl_FindAreaLink( pLinkManager, aDocName, aFltName, aOptions,
                                            aAreaName, aRange );
    if (pLink)
        pLinkManager->Remove( pLink );

    SfxGetpApp()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );     // Navigator
}

void ScUndoRemoveAreaLink::Repeat(SfxRepeatTarget& /* rTarget */)
{
    // makes no sense
}

bool ScUndoRemoveAreaLink::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;
}

ScUndoUpdateAreaLink::ScUndoUpdateAreaLink( ScDocShell* pShell,
                            const OUString& rOldD, const OUString& rOldF, const OUString& rOldO,
                            const OUString& rOldA, const ScRange& rOldR, sal_uLong nOldRD,
                            const OUString& rNewD, const OUString& rNewF, const OUString& rNewO,
                            const OUString& rNewA, const ScRange& rNewR, sal_uLong nNewRD,
                            ScDocument* pUndo, ScDocument* pRedo, bool bDoInsert )
    :   ScSimpleUndo( pShell ),
        aOldDoc     ( rOldD ),
        aOldFlt     ( rOldF ),
        aOldOpt     ( rOldO ),
        aOldArea    ( rOldA ),
        aOldRange   ( rOldR ),
        aNewDoc     ( rNewD ),
        aNewFlt     ( rNewF ),
        aNewOpt     ( rNewO ),
        aNewArea    ( rNewA ),
        aNewRange   ( rNewR ),
        pUndoDoc    ( pUndo ),
        pRedoDoc    ( pRedo ),
        nOldRefresh ( nOldRD ),
        nNewRefresh ( nNewRD ),
        bWithInsert ( bDoInsert )
{
    OSL_ENSURE( aOldRange.aStart == aNewRange.aStart, "AreaLink moved ?" );
}

ScUndoUpdateAreaLink::~ScUndoUpdateAreaLink()
{
    delete pUndoDoc;
    delete pRedoDoc;
}

OUString ScUndoUpdateAreaLink::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_UPDATELINK );   //! own text ??
}

void ScUndoUpdateAreaLink::DoChange( const bool bUndo ) const
{
    ScDocument& rDoc = pDocShell->GetDocument();

    SCCOL nEndX = std::max( aOldRange.aEnd.Col(), aNewRange.aEnd.Col() );
    SCROW nEndY = std::max( aOldRange.aEnd.Row(), aNewRange.aEnd.Row() );
    SCTAB nEndZ = std::max( aOldRange.aEnd.Tab(), aNewRange.aEnd.Tab() );    //?

    if ( bUndo )
    {
        if ( bWithInsert )
        {
            rDoc.FitBlock( aNewRange, aOldRange );
            rDoc.DeleteAreaTab( aOldRange, IDF_ALL & ~IDF_NOTE );
            pUndoDoc->UndoToDocument( aOldRange, IDF_ALL & ~IDF_NOTE, false, &rDoc );
        }
        else
        {
            ScRange aCopyRange( aOldRange.aStart, ScAddress(nEndX,nEndY,nEndZ) );
            rDoc.DeleteAreaTab( aCopyRange, IDF_ALL & ~IDF_NOTE );
            pUndoDoc->CopyToDocument( aCopyRange, IDF_ALL & ~IDF_NOTE, false, &rDoc );
        }
    }
    else
    {
        if ( bWithInsert )
        {
            rDoc.FitBlock( aOldRange, aNewRange );
            rDoc.DeleteAreaTab( aNewRange, IDF_ALL & ~IDF_NOTE );
            pRedoDoc->CopyToDocument( aNewRange, IDF_ALL & ~IDF_NOTE, false, &rDoc );
        }
        else
        {
            ScRange aCopyRange( aOldRange.aStart, ScAddress(nEndX,nEndY,nEndZ) );
            rDoc.DeleteAreaTab( aCopyRange, IDF_ALL & ~IDF_NOTE );
            pRedoDoc->CopyToDocument( aCopyRange, IDF_ALL & ~IDF_NOTE, false, &rDoc );
        }
    }

    ScRange aWorkRange( aNewRange.aStart, ScAddress( nEndX, nEndY, nEndZ ) );
    rDoc.ExtendMerge( aWorkRange, true );

    //  Paint

    if ( aNewRange.aEnd.Col() != aOldRange.aEnd.Col() )
        aWorkRange.aEnd.SetCol(MAXCOL);
    if ( aNewRange.aEnd.Row() != aOldRange.aEnd.Row() )
        aWorkRange.aEnd.SetRow(MAXROW);

    if ( !pDocShell->AdjustRowHeight( aWorkRange.aStart.Row(), aWorkRange.aEnd.Row(), aWorkRange.aStart.Tab() ) )
        pDocShell->PostPaint( aWorkRange, PAINT_GRID );

    pDocShell->PostDataChanged();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->CellContentChanged();
}

void ScUndoUpdateAreaLink::Undo()
{
    ScDocument& rDoc = pDocShell->GetDocument();
    sfx2::LinkManager* pLinkManager = rDoc.GetLinkManager();
    ScAreaLink* pLink = lcl_FindAreaLink( pLinkManager, aNewDoc, aNewFlt, aNewOpt,
                                            aNewArea, aNewRange );
    if (pLink)
    {
        pLink->SetSource( aOldDoc, aOldFlt, aOldOpt, aOldArea );        // old data in Link
        pLink->SetDestArea( aOldRange );
        pLink->SetRefreshDelay( nOldRefresh );
    }

    DoChange(true);
}

void ScUndoUpdateAreaLink::Redo()
{
    ScDocument& rDoc = pDocShell->GetDocument();
    sfx2::LinkManager* pLinkManager = rDoc.GetLinkManager();
    ScAreaLink* pLink = lcl_FindAreaLink( pLinkManager, aOldDoc, aOldFlt, aOldOpt,
                                            aOldArea, aOldRange );
    if (pLink)
    {
        pLink->SetSource( aNewDoc, aNewFlt, aNewOpt, aNewArea );        // new values in link
        pLink->SetDestArea( aNewRange );
        pLink->SetRefreshDelay( nNewRefresh );
    }

    DoChange(false);
}

void ScUndoUpdateAreaLink::Repeat(SfxRepeatTarget& /* rTarget */)
{
    // makes no sense
}

bool ScUndoUpdateAreaLink::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
