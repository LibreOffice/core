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

#include <scitems.hxx>
#include <svx/algitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/justifyitem.hxx>
#include <svl/srchitem.hxx>
#include <sfx2/linkmgr.hxx>
#include <sfx2/bindings.hxx>
#include <vcl/virdev.hxx>
#include <sfx2/app.hxx>
#include <svx/svdundo.hxx>

#include <undoblk.hxx>
#include <sc.hrc>
#include <globstr.hrc>
#include <scresid.hxx>
#include <global.hxx>
#include <rangenam.hxx>
#include <arealink.hxx>
#include <patattr.hxx>
#include <target.hxx>
#include <document.hxx>
#include <docpool.hxx>
#include <table.hxx>
#include <docsh.hxx>
#include <tabvwsh.hxx>
#include <undoolk.hxx>
#include <undoutil.hxx>
#include <chgtrack.hxx>
#include <dociter.hxx>
#include <formulacell.hxx>
#include <paramisc.hxx>
#include <postit.hxx>
#include <docuno.hxx>
#include <progress.hxx>
#include <editutil.hxx>
#include <editdataarray.hxx>
#include <rowheightcontext.hxx>

// TODO:
/*A*/   // SetOptimalHeight on Document, when no View

ScUndoDeleteContents::ScUndoDeleteContents(
                ScDocShell* pNewDocShell,
                const ScMarkData& rMark, const ScRange& rRange,
                ScDocumentUniquePtr&& pNewUndoDoc, bool bNewMulti,
                InsertDeleteFlags nNewFlags, bool bObjects )
    :   ScSimpleUndo( pNewDocShell ),
        aRange      ( rRange ),
        aMarkData   ( rMark ),
        pUndoDoc    ( std::move(pNewUndoDoc) ),
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
    pDrawUndo.reset();
}

OUString ScUndoDeleteContents::GetComment() const
{
    return ScResId( STR_UNDO_DELETECONTENTS );    // "Delete"
}

void ScUndoDeleteContents::SetDataSpans( const std::shared_ptr<DataSpansType>& pSpans )
{
    mpDataSpans = pSpans;
}

void ScUndoDeleteContents::SetChangeTrack()
{
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument().GetChangeTrack();
    if ( pChangeTrack && (nFlags & InsertDeleteFlags::CONTENTS) )
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
        InsertDeleteFlags nUndoFlags = InsertDeleteFlags::NONE; // copy either all or none of the content
        if (nFlags & InsertDeleteFlags::CONTENTS)        // (Only the correct ones have been copied into UndoDoc)
            nUndoFlags |= InsertDeleteFlags::CONTENTS;
        if (nFlags & InsertDeleteFlags::ATTRIB)
            nUndoFlags |= InsertDeleteFlags::ATTRIB;
        if (nFlags & InsertDeleteFlags::EDITATTR)          // Edit-Engine attribute
            nUndoFlags |= InsertDeleteFlags::STRING;       // -> Cells will be changed
        // do not create clones of note captions, they will be restored via drawing undo
        nUndoFlags |= InsertDeleteFlags::NOCAPTIONS;

        ScRange aCopyRange = aRange;
        SCTAB nTabCount = rDoc.GetTableCount();
        aCopyRange.aStart.SetTab(0);
        aCopyRange.aEnd.SetTab(nTabCount-1);

        pUndoDoc->CopyToDocument(aCopyRange, nUndoFlags, bMulti, rDoc, &aMarkData);

        DoSdrUndoAction( pDrawUndo.get(), &rDoc );

        ScChangeTrack* pChangeTrack = rDoc.GetChangeTrack();
        if ( pChangeTrack )
            pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );

        pDocShell->UpdatePaintExt( nExtFlags, aRange );             // content after the change
    }
    else        // only Redo
    {
        pDocShell->UpdatePaintExt( nExtFlags, aRange );             // content before the change

        aMarkData.MarkToMulti();
        RedoSdrUndoAction( pDrawUndo.get() );
        // do not delete objects and note captions, they have been removed via drawing undo
        InsertDeleteFlags nRedoFlags = (nFlags & ~InsertDeleteFlags::OBJECTS) | InsertDeleteFlags::NOCAPTIONS;
        rDoc.DeleteSelection( nRedoFlags, aMarkData );
        aMarkData.MarkToSimple();

        SetChangeTrack();
    }

    if (nFlags & InsertDeleteFlags::CONTENTS)
    {
        // Broadcast only when the content changes. fdo#74687
        if (mpDataSpans)
            BroadcastChanges(*mpDataSpans);
        else
            BroadcastChanges(aRange);
    }

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if ( !( pViewShell && pViewShell->AdjustRowHeight(
                                aRange.aStart.Row(), aRange.aEnd.Row() ) ) )
/*A*/   pDocShell->PostPaint( aRange, PaintPartFlags::Grid | PaintPartFlags::Extras, nExtFlags );

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
                ScDocumentUniquePtr pNewUndoDoc, bool bNewMulti, SCTAB nSrc,
                InsertDeleteFlags nFlg, ScPasteFunc nFunc, bool bSkip, bool bLink )
    :   ScSimpleUndo( pNewDocShell ),
        aRange      ( nStartX, nStartY, nStartZ, nEndX, nEndY, nEndZ ),
        aMarkData   ( rMark ),
        pUndoDoc    ( std::move(pNewUndoDoc) ),
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
}

OUString ScUndoFillTable::GetComment() const
{
    return ScResId( STR_FILL_TAB );
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
        for (const auto& rTab : aMarkData)
        {
            if (rTab >= nTabCount)
                break;
            if (rTab != nSrcTab)
            {
                aWorkRange.aStart.SetTab(rTab);
                aWorkRange.aEnd.SetTab(rTab);
                pChangeTrack->AppendContentRange( aWorkRange, pUndoDoc.get(),
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
        for (const auto& rTab : aMarkData)
        {
            if (rTab >= nTabCount)
                break;
            if (rTab != nSrcTab)
            {
                aWorkRange.aStart.SetTab(rTab);
                aWorkRange.aEnd.SetTab(rTab);
                if (bMulti)
                    rDoc.DeleteSelectionTab( rTab, InsertDeleteFlags::ALL, aMarkData );
                else
                    rDoc.DeleteAreaTab( aWorkRange, InsertDeleteFlags::ALL );
                pUndoDoc->CopyToDocument(aWorkRange, InsertDeleteFlags::ALL, bMulti, rDoc, &aMarkData);
            }
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

    pDocShell->PostPaint(0,0,0,MAXCOL,MAXROW,MAXTAB, PaintPartFlags::Grid|PaintPartFlags::Extras);
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
                ScDocumentUniquePtr pNewUndoDoc, bool bNewMulti,
                const ScPatternAttr* pNewApply,
                const SvxBoxItem* pNewOuter, const SvxBoxInfoItem* pNewInner,
                const ScRange* pRangeCover )
    :   ScSimpleUndo( pNewDocShell ),
        aMarkData   ( rMark ),
        aRange      ( nStartX, nStartY, nStartZ, nEndX, nEndY, nEndZ ),
        mpDataArray(new ScEditDataArray),
        pUndoDoc    ( std::move(pNewUndoDoc) ),
        bMulti      ( bNewMulti )
{
    ScDocumentPool* pPool = pDocShell->GetDocument().GetPool();
    pApplyPattern = const_cast<ScPatternAttr*>(static_cast<const ScPatternAttr*>( &pPool->Put( *pNewApply ) ));
    pLineOuter = pNewOuter ? const_cast<SvxBoxItem*>(static_cast<const SvxBoxItem*>( &pPool->Put( *pNewOuter ) )) : nullptr;
    pLineInner = pNewInner ? const_cast<SvxBoxInfoItem*>(static_cast<const SvxBoxInfoItem*>( &pPool->Put( *pNewInner ) )) : nullptr;
    aRangeCover = pRangeCover ? *pRangeCover : aRange;
}

ScUndoSelectionAttr::~ScUndoSelectionAttr()
{
    ScDocumentPool* pPool = pDocShell->GetDocument().GetPool();
    pPool->Remove(*pApplyPattern);
    if (pLineOuter)
        pPool->Remove(*pLineOuter);
    if (pLineInner)
        pPool->Remove(*pLineInner);

    pUndoDoc.reset();
}

OUString ScUndoSelectionAttr::GetComment() const
{
    //"Attribute" "/Lines"
    return ScResId( pLineOuter ? STR_UNDO_SELATTRLINES : STR_UNDO_SELATTR );
}

ScEditDataArray* ScUndoSelectionAttr::GetDataArray()
{
    return mpDataArray.get();
}

void ScUndoSelectionAttr::DoChange( const bool bUndo )
{
    ScDocument& rDoc = pDocShell->GetDocument();

    SetViewMarkData( aMarkData );

    ScRange aEffRange( aRangeCover );
    if ( rDoc.HasAttrib( aEffRange, HasAttrFlags::Merged ) )         // merged cells?
        rDoc.ExtendMerge( aEffRange );

    sal_uInt16 nExtFlags = 0;
    pDocShell->UpdatePaintExt( nExtFlags, aEffRange );

    ChangeEditData(bUndo);

    if (bUndo)  // only for Undo
    {
        ScRange aCopyRange = aRangeCover;
        SCTAB nTabCount = rDoc.GetTableCount();
        aCopyRange.aStart.SetTab(0);
        aCopyRange.aEnd.SetTab(nTabCount-1);
        pUndoDoc->CopyToDocument(aCopyRange, InsertDeleteFlags::ATTRIB, bMulti, rDoc, &aMarkData);
    }
    else        // only for Redo
    {
        aMarkData.MarkToMulti();
        rDoc.ApplySelectionPattern( *pApplyPattern, aMarkData );
        aMarkData.MarkToSimple();

        if (pLineOuter)
            rDoc.ApplySelectionFrame(aMarkData, *pLineOuter, pLineInner);
    }

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if ( !( pViewShell && pViewShell->AdjustBlockHeight() ) )
/*A*/   pDocShell->PostPaint( aEffRange, PaintPartFlags::Grid | PaintPartFlags::Extras, nExtFlags );

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
                rDoc.SetEditText(aPos, *pItem->GetOldData(), nullptr);
            else
                rDoc.SetEmptyCell(aPos);
        }
        else
        {
            if (pItem->GetNewData())
                rDoc.SetEditText(aPos, *pItem->GetNewData(), nullptr);
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
            rViewShell.ApplyPatternLines(*pApplyPattern, *pLineOuter, pLineInner);
        else
            rViewShell.ApplySelectionPattern( *pApplyPattern );
    }
}

bool ScUndoSelectionAttr::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
}

ScUndoAutoFill::ScUndoAutoFill( ScDocShell* pNewDocShell,
                const ScRange& rRange, const ScRange& rSourceArea,
                ScDocumentUniquePtr pNewUndoDoc, const ScMarkData& rMark,
                FillDir eNewFillDir, FillCmd eNewFillCmd, FillDateCmd eNewFillDateCmd,
                double fNewStartValue, double fNewStepValue, double fNewMaxValue )
    :   ScBlockUndo( pNewDocShell, rRange, SC_UNDO_AUTOHEIGHT ),
        aSource         ( rSourceArea ),
        aMarkData       ( rMark ),
        pUndoDoc        ( std::move(pNewUndoDoc) ),
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
}

OUString ScUndoAutoFill::GetComment() const
{
    return ScResId( STR_UNDO_AUTOFILL ); //"Fill"
}

void ScUndoAutoFill::SetChangeTrack()
{
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument().GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->AppendContentRange( aBlockRange, pUndoDoc.get(),
            nStartChangeAction, nEndChangeAction );
    else
        nStartChangeAction = nEndChangeAction = 0;
}

void ScUndoAutoFill::Undo()
{
    BeginUndo();

    ScDocument& rDoc = pDocShell->GetDocument();

    SCTAB nTabCount = rDoc.GetTableCount();
    for (const auto& rTab : aMarkData)
    {
        if (rTab >= nTabCount)
            break;
        ScRange aWorkRange = aBlockRange;
        aWorkRange.aStart.SetTab(rTab);
        aWorkRange.aEnd.SetTab(rTab);

        sal_uInt16 nExtFlags = 0;
        pDocShell->UpdatePaintExt( nExtFlags, aWorkRange );
        rDoc.DeleteAreaTab( aWorkRange, InsertDeleteFlags::AUTOFILL );
        pUndoDoc->CopyToDocument(aWorkRange, InsertDeleteFlags::AUTOFILL, false, rDoc);

        // Actually we'd only need to broadcast the cells inserted during
        // CopyToDocument(), as DeleteAreaTab() broadcasts deleted cells. For
        // this we'd need to either record the span sets or let
        // CopyToDocument() broadcast.
        BroadcastChanges( aWorkRange);

        rDoc.ExtendMerge( aWorkRange, true );
        pDocShell->PostPaint( aWorkRange, PaintPartFlags::Grid, nExtFlags );
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
            ScResId(STR_FILL_SERIES_PROGRESS), nProgCount, true );

    rDoc.Fill( aSource.aStart.Col(), aSource.aStart.Row(),
            aSource.aEnd.Col(), aSource.aEnd.Row(), &aProgress,
            aMarkData, nCount,
            eFillDir, eFillCmd, eFillDateCmd,
            fStepValue, fMaxValue );

    SetChangeTrack();

    pDocShell->PostPaint( aBlockRange, PaintPartFlags::Grid );
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

ScUndoMerge::ScUndoMerge(ScDocShell* pNewDocShell, const ScCellMergeOption& rOption,
                         bool bMergeContents, ScDocumentUniquePtr pUndoDoc, std::unique_ptr<SdrUndoAction> pDrawUndo)
    : ScSimpleUndo(pNewDocShell)
    , maOption(rOption)
    , mbMergeContents(bMergeContents)
    , mxUndoDoc(std::move(pUndoDoc))
    , mpDrawUndo(std::move(pDrawUndo))
{
}

ScUndoMerge::~ScUndoMerge()
{
    mpDrawUndo.reset();
}

OUString ScUndoMerge::GetComment() const
{
    return ScResId( STR_UNDO_MERGE );
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

    for (const SCTAB nTab : maOption.maTabs)
    {
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
                                 SvxHorJustifyItem( SvxCellHorJustify::Center, ATTR_HOR_JUSTIFY ) );
                rDoc.ApplyAttr( aRange.aStart.Col(), aRange.aStart.Row(),
                                 aRange.aStart.Tab(),
                                 SvxVerJustifyItem( SvxCellVerJustify::Center, ATTR_VER_JUSTIFY ) );
            }
        }

        // undo -> copy back deleted contents
        if (bUndo && mxUndoDoc)
        {
            // If there are note captions to be deleted during Undo they were
            // kept or moved during the merge and copied to the Undo document
            // without cloning the caption. Forget the target area's caption
            // pointer that is identical to the one in the Undo document
            // instead of deleting it.
            rDoc.DeleteAreaTab( aRange,
                    InsertDeleteFlags::CONTENTS | InsertDeleteFlags::NOCAPTIONS | InsertDeleteFlags::FORGETCAPTIONS );
            mxUndoDoc->CopyToDocument(aRange, InsertDeleteFlags::ALL|InsertDeleteFlags::NOCAPTIONS, false, rDoc);
        }

        // redo -> merge contents again
        else if (!bUndo && mbMergeContents)
        {
            rDoc.DoMergeContents( aRange.aStart.Tab(),
                                   aRange.aStart.Col(), aRange.aStart.Row(),
                                   aRange.aEnd.Col(), aRange.aEnd.Row() );
        }

        if (bUndo)
            DoSdrUndoAction( mpDrawUndo.get(), &rDoc );
        else
            RedoSdrUndoAction( mpDrawUndo.get() );

        bool bDidPaint = false;
        if ( pViewShell )
        {
            pViewShell->SetTabNo(nTab);
            bDidPaint = pViewShell->AdjustRowHeight(maOption.mnStartRow, maOption.mnEndRow);
        }

        if (!bDidPaint)
            ScUndoUtil::PaintMore(pDocShell, aRange);

        rDoc.BroadcastCells(aRange, SfxHintId::ScDataChanged);
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
        rViewShell.MergeCells( false, bCont, false );
    }
}

bool ScUndoMerge::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
}

ScUndoAutoFormat::ScUndoAutoFormat( ScDocShell* pNewDocShell,
                        const ScRange& rRange, ScDocumentUniquePtr pNewUndoDoc,
                        const ScMarkData& rMark, bool bNewSize, sal_uInt16 nNewFormatNo )
    :   ScBlockUndo( pNewDocShell, rRange, bNewSize ? SC_UNDO_MANUALHEIGHT : SC_UNDO_AUTOHEIGHT ),
        pUndoDoc    ( std::move(pNewUndoDoc) ),
        aMarkData   ( rMark ),
        bSize       ( bNewSize ),
        nFormatNo   ( nNewFormatNo )
{
}

ScUndoAutoFormat::~ScUndoAutoFormat()
{
}

OUString ScUndoAutoFormat::GetComment() const
{
    return ScResId( STR_UNDO_AUTOFORMAT );   //"Auto-Format"
}

void ScUndoAutoFormat::Undo()
{
    BeginUndo();

    ScDocument& rDoc = pDocShell->GetDocument();

    SCTAB nTabCount = rDoc.GetTableCount();
    rDoc.DeleteArea( aBlockRange.aStart.Col(), aBlockRange.aStart.Row(),
                      aBlockRange.aEnd.Col(), aBlockRange.aEnd.Row(),
                      aMarkData, InsertDeleteFlags::ATTRIB );
    ScRange aCopyRange = aBlockRange;
    aCopyRange.aStart.SetTab(0);
    aCopyRange.aEnd.SetTab(nTabCount-1);
    pUndoDoc->CopyToDocument(aCopyRange, InsertDeleteFlags::ATTRIB, false, rDoc, &aMarkData);

    // cell heights and widths (InsertDeleteFlags::NONE)
    if (bSize)
    {
        SCCOL nStartX = aBlockRange.aStart.Col();
        SCROW nStartY = aBlockRange.aStart.Row();
        SCTAB nStartZ = aBlockRange.aStart.Tab();
        SCCOL nEndX = aBlockRange.aEnd.Col();
        SCROW nEndY = aBlockRange.aEnd.Row();
        SCTAB nEndZ = aBlockRange.aEnd.Tab();

        pUndoDoc->CopyToDocument( nStartX, 0, 0, nEndX, MAXROW, nTabCount-1,
                                    InsertDeleteFlags::NONE, false, rDoc, &aMarkData );
        pUndoDoc->CopyToDocument( 0, nStartY, 0, MAXCOL, nEndY, nTabCount-1,
                                    InsertDeleteFlags::NONE, false, rDoc, &aMarkData );
        pDocShell->PostPaint( 0, 0, nStartZ, MAXCOL, MAXROW, nEndZ,
                              PaintPartFlags::Grid | PaintPartFlags::Left | PaintPartFlags::Top, SC_PF_LINES );
    }
    else
        pDocShell->PostPaint( aBlockRange, PaintPartFlags::Grid, SC_PF_LINES );

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
                CRFlags nOld = rDoc.GetRowFlags(nRow,nTab);
                bool bHidden = rDoc.RowHidden(nRow, nTab);
                if ( !bHidden && ( nOld & CRFlags::ManualSize ) )
                    rDoc.SetRowFlags( nRow, nTab, nOld & ~CRFlags::ManualSize );
            }

            bool bChanged = rDoc.SetOptimalHeight(aCxt, nStartY, nEndY, nTab);

            for (SCCOL nCol=nStartX; nCol<=nEndX; nCol++)
                if (!rDoc.ColHidden(nCol, nTab))
                {
                    sal_uInt16 nThisSize = STD_EXTRA_WIDTH + rDoc.GetOptimalColWidth( nCol, nTab,
                                                pVirtDev, nPPTX, nPPTY, aZoomX, aZoomY, false/*bFormula*/,
                                                &aDestMark );
                    rDoc.SetColWidth( nCol, nTab, nThisSize );
                    rDoc.ShowCol( nCol, nTab, true );
                }

            // tdf#76183: recalculate objects' positions
            if (bChanged)
                rDoc.SetDrawPageSize(nTab);
        }

        pDocShell->PostPaint( 0,      0,      nStartZ,
                              MAXCOL, MAXROW, nEndZ,
                              PaintPartFlags::Grid | PaintPartFlags::Left | PaintPartFlags::Top, SC_PF_LINES);
    }
    else
        pDocShell->PostPaint( aBlockRange, PaintPartFlags::Grid, SC_PF_LINES );

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
                                    const OUString& rNewUndoStr, ScDocumentUniquePtr pNewUndoDoc,
                                    const SvxSearchItem* pItem )
    :   ScSimpleUndo( pNewDocShell ),
        aCursorPos  ( nCurX, nCurY, nCurZ ),
        aMarkData   ( rMark ),
        aUndoStr    ( rNewUndoStr ),
        pUndoDoc    ( std::move(pNewUndoDoc) )
{
    pSearchItem.reset( new SvxSearchItem( *pItem ) );
    SetChangeTrack();
}

ScUndoReplace::~ScUndoReplace()
{
    pUndoDoc.reset();
    pSearchItem.reset();
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
            pChangeTrack->AppendContentsIfInRefDoc( pUndoDoc.get(),
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
    return ScResId( STR_UNDO_REPLACE );  // "Replace"
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

        InsertDeleteFlags nUndoFlags = (pSearchItem->GetPattern()) ? InsertDeleteFlags::ATTRIB : InsertDeleteFlags::CONTENTS;
        pUndoDoc->CopyToDocument( 0,      0,      0,
                                  MAXCOL, MAXROW, MAXTAB,
                                  nUndoFlags, false, rDoc, nullptr, false );   // without row flags
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
                            aMarkData);
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

            pViewShell->SearchAndReplace( pSearchItem.get(), false, true );
        }
    }
    else if (pSearchItem->GetPattern() &&
             pSearchItem->GetCommand() == SvxSearchCmd::REPLACE)
    {
        rDoc.ReplaceStyle( *pSearchItem,
                            aCursorPos.Col(), aCursorPos.Row(), aCursorPos.Tab(),
                            aMarkData);
        pDocShell->PostPaintGridAll();
    }
    else
        if (pViewShell)
            pViewShell->SearchAndReplace( pSearchItem.get(), false, true );

    SetChangeTrack();

    EndRedo();
}

void ScUndoReplace::Repeat(SfxRepeatTarget& rTarget)
{
    if (dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr)
        static_cast<ScTabViewTarget&>(rTarget).GetViewShell()->SearchAndReplace( pSearchItem.get(), true, false );
}

bool ScUndoReplace::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
}

// multi-operation (only simple blocks)
ScUndoTabOp::ScUndoTabOp( ScDocShell* pNewDocShell,
                SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                SCCOL nEndX, SCROW nEndY, SCTAB nEndZ, ScDocumentUniquePtr pNewUndoDoc,
                const ScRefAddress& rFormulaCell,
                const ScRefAddress& rFormulaEnd,
                const ScRefAddress& rRowCell,
                const ScRefAddress& rColCell,
                ScTabOpParam::Mode eMode )
    :   ScSimpleUndo( pNewDocShell ),
        aRange          ( nStartX, nStartY, nStartZ, nEndX, nEndY, nEndZ ),
        pUndoDoc        ( std::move(pNewUndoDoc) ),
        theFormulaCell  ( rFormulaCell ),
        theFormulaEnd   ( rFormulaEnd ),
        theRowCell      ( rRowCell ),
        theColCell      ( rColCell ),
        meMode(eMode)
{
}

ScUndoTabOp::~ScUndoTabOp()
{
}

OUString ScUndoTabOp::GetComment() const
{
    return ScResId( STR_UNDO_TABOP );    // "Multiple operation"
}

void ScUndoTabOp::Undo()
{
    BeginUndo();

    ScUndoUtil::MarkSimpleBlock( pDocShell, aRange );

    sal_uInt16 nExtFlags = 0;
    pDocShell->UpdatePaintExt( nExtFlags, aRange );

    ScDocument& rDoc = pDocShell->GetDocument();
    rDoc.DeleteAreaTab( aRange,InsertDeleteFlags::ALL & ~InsertDeleteFlags::NOTE );
    pUndoDoc->CopyToDocument( aRange, InsertDeleteFlags::ALL & ~InsertDeleteFlags::NOTE, false, rDoc );
    pDocShell->PostPaint( aRange, PaintPartFlags::Grid, nExtFlags );
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
        SCCOL nCurX, SCROW nCurY, SCTAB nCurZ, ScDocumentUniquePtr pNewUndoDoc,
        SCCOL nNewX, SCROW nNewY, SCTAB nNewZ, ScDocumentUniquePtr pNewRedoDoc,
        const ScConversionParam& rConvParam ) :
    ScSimpleUndo( pNewDocShell ),
    aMarkData( rMark ),
    aCursorPos( nCurX, nCurY, nCurZ ),
    pUndoDoc( std::move(pNewUndoDoc) ),
    aNewCursorPos( nNewX, nNewY, nNewZ ),
    pRedoDoc( std::move(pNewRedoDoc) ),
    maConvParam( rConvParam )
{
    SetChangeTrack();
}

ScUndoConversion::~ScUndoConversion()
{
    pUndoDoc.reset();
    pRedoDoc.reset();
}

void ScUndoConversion::SetChangeTrack()
{
    ScDocument& rDoc = pDocShell->GetDocument();
    ScChangeTrack* pChangeTrack = rDoc.GetChangeTrack();
    if ( pChangeTrack )
    {
        if ( pUndoDoc )
            pChangeTrack->AppendContentsIfInRefDoc( pUndoDoc.get(),
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
        case SC_CONVERSION_SPELLCHECK:      aText = ScResId( STR_UNDO_SPELLING );    break;
        case SC_CONVERSION_HANGULHANJA:     aText = ScResId( STR_UNDO_HANGULHANJA ); break;
        case SC_CONVERSION_CHINESE_TRANSL:  aText = ScResId( STR_UNDO_CHINESE_TRANSLATION ); break;
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
                                 InsertDeleteFlags::CONTENTS, bMulti, rDoc, &aMarkData );
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
    DoChange( pUndoDoc.get(), aCursorPos );
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument().GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );
    EndUndo();
}

void ScUndoConversion::Redo()
{
    BeginRedo();
    DoChange( pRedoDoc.get(), aNewCursorPos );
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
                                         ScDocumentUniquePtr pNewUndoDoc, ScDocumentUniquePtr pNewRedoDoc, bool bNewMulti) :
ScSimpleUndo( pNewDocShell ),
aMarkData   ( rMark ),
pUndoDoc    ( std::move(pNewUndoDoc) ),
pRedoDoc    ( std::move(pNewRedoDoc) ),
aRange      ( aMarkRange ),
bMulti      ( bNewMulti )
{
    SetChangeTrack();
}

ScUndoRefConversion::~ScUndoRefConversion()
{
    pUndoDoc.reset();
    pRedoDoc.reset();
}

OUString ScUndoRefConversion::GetComment() const
{
    return ScResId( STR_UNDO_ENTERDATA ); // "Input"
}

void ScUndoRefConversion::SetChangeTrack()
{
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument().GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->AppendContentsIfInRefDoc( pUndoDoc.get(),
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
    pRefDoc->CopyToDocument( aCopyRange, InsertDeleteFlags::ALL, bMulti, rDoc, &aMarkData );
    pDocShell->PostPaint( aRange, PaintPartFlags::Grid);
    pDocShell->PostDataChanged();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->CellContentChanged();
}

void ScUndoRefConversion::Undo()
{
    BeginUndo();
    if (pUndoDoc)
        DoChange(pUndoDoc.get());
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument().GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );
    EndUndo();
}

void ScUndoRefConversion::Redo()
{
    BeginRedo();
    if (pRedoDoc)
        DoChange(pRedoDoc.get());
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

ScUndoRefreshLink::ScUndoRefreshLink(ScDocShell* pNewDocShell,
                                     ScDocumentUniquePtr pNewUndoDoc)
    : ScSimpleUndo(pNewDocShell)
    , xUndoDoc(std::move(pNewUndoDoc))
{
}

OUString ScUndoRefreshLink::GetComment() const
{
    return ScResId( STR_UNDO_UPDATELINK );
}

void ScUndoRefreshLink::Undo()
{
    BeginUndo();

    bool bMakeRedo = !xRedoDoc;
    if (bMakeRedo)
        xRedoDoc.reset(new ScDocument(SCDOCMODE_UNDO));

    bool bFirst = true;
    ScDocument& rDoc = pDocShell->GetDocument();
    SCTAB nCount = rDoc.GetTableCount();
    for (SCTAB nTab=0; nTab<nCount; nTab++)
        if (xUndoDoc->HasTable(nTab))
        {
            ScRange aRange(0,0,nTab,MAXCOL,MAXROW,nTab);
            if (bMakeRedo)
            {
                if (bFirst)
                    xRedoDoc->InitUndo(&rDoc, nTab, nTab, true, true);
                else
                    xRedoDoc->AddUndoTab(nTab, nTab, true, true);
                bFirst = false;
                rDoc.CopyToDocument(aRange, InsertDeleteFlags::ALL, false, *xRedoDoc);
                xRedoDoc->SetLink(nTab,
                                  rDoc.GetLinkMode(nTab),
                                  rDoc.GetLinkDoc(nTab),
                                  rDoc.GetLinkFlt(nTab),
                                  rDoc.GetLinkOpt(nTab),
                                  rDoc.GetLinkTab(nTab),
                                  rDoc.GetLinkRefreshDelay(nTab));
                xRedoDoc->SetTabBgColor( nTab, rDoc.GetTabBgColor(nTab) );
            }

            rDoc.DeleteAreaTab( aRange,InsertDeleteFlags::ALL );
            xUndoDoc->CopyToDocument(aRange, InsertDeleteFlags::ALL, false, rDoc);
            rDoc.SetLink(nTab, xUndoDoc->GetLinkMode(nTab), xUndoDoc->GetLinkDoc(nTab),
                               xUndoDoc->GetLinkFlt(nTab),  xUndoDoc->GetLinkOpt(nTab),
                               xUndoDoc->GetLinkTab(nTab),
                               xUndoDoc->GetLinkRefreshDelay(nTab) );
            rDoc.SetTabBgColor(nTab, xUndoDoc->GetTabBgColor(nTab));
        }

    pDocShell->PostPaintGridAll();
    pDocShell->PostPaintExtras();

    EndUndo();
}

void ScUndoRefreshLink::Redo()
{
    OSL_ENSURE(xRedoDoc, "No RedoDoc for ScUndoRefreshLink::Redo");

    BeginUndo();

    ScDocument& rDoc = pDocShell->GetDocument();
    SCTAB nCount = rDoc.GetTableCount();
    for (SCTAB nTab=0; nTab<nCount; nTab++)
        if (xRedoDoc->HasTable(nTab))
        {
            ScRange aRange(0,0,nTab,MAXCOL,MAXROW,nTab);

            rDoc.DeleteAreaTab( aRange, InsertDeleteFlags::ALL );
            xRedoDoc->CopyToDocument(aRange, InsertDeleteFlags::ALL, false, rDoc);
            rDoc.SetLink(nTab,
                         xRedoDoc->GetLinkMode(nTab),
                         xRedoDoc->GetLinkDoc(nTab),
                         xRedoDoc->GetLinkFlt(nTab),
                         xRedoDoc->GetLinkOpt(nTab),
                         xRedoDoc->GetLinkTab(nTab),
                         xRedoDoc->GetLinkRefreshDelay(nTab) );
            rDoc.SetTabBgColor(nTab, xRedoDoc->GetTabBgColor(nTab));
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

static ScAreaLink* lcl_FindAreaLink( const sfx2::LinkManager* pLinkManager, const OUString& rDoc,
                            const OUString& rFlt, const OUString& rOpt,
                            const OUString& rSrc, const ScRange& rDest )
{
    const ::sfx2::SvBaseLinks& rLinks = pLinkManager->GetLinks();
    sal_uInt16 nCount = pLinkManager->GetLinks().size();
    for (sal_uInt16 i=0; i<nCount; i++)
    {
        ::sfx2::SvBaseLink* pBase = rLinks[i].get();
        if (dynamic_cast<const ScAreaLink*>( pBase) !=  nullptr)
            if ( static_cast<ScAreaLink*>(pBase)->IsEqual( rDoc, rFlt, rOpt, rSrc, rDest ) )
                return static_cast<ScAreaLink*>(pBase);
    }

    OSL_FAIL("ScAreaLink not found");
    return nullptr;
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
    return ScResId( STR_UNDO_INSERTAREALINK );
}

void ScUndoInsertAreaLink::Undo()
{
    ScDocument& rDoc = pDocShell->GetDocument();
    sfx2::LinkManager* pLinkManager = rDoc.GetLinkManager();

    ScAreaLink* pLink = lcl_FindAreaLink( pLinkManager, aDocName, aFltName, aOptions,
                                            aAreaName, aRange );
    if (pLink)
        pLinkManager->Remove( pLink );

    SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScAreaLinksChanged ) );     // Navigator
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

    SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScAreaLinksChanged ) );     // Navigator
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
    return ScResId( STR_UNDO_REMOVELINK );   //! own text ??
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

    SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScAreaLinksChanged ) );     // Navigator
}

void ScUndoRemoveAreaLink::Redo()
{
    ScDocument& rDoc = pDocShell->GetDocument();
    sfx2::LinkManager* pLinkManager = rDoc.GetLinkManager();

    ScAreaLink* pLink = lcl_FindAreaLink( pLinkManager, aDocName, aFltName, aOptions,
                                            aAreaName, aRange );
    if (pLink)
        pLinkManager->Remove( pLink );

    SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScAreaLinksChanged ) );     // Navigator
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
                            ScDocumentUniquePtr pUndo, ScDocumentUniquePtr pRedo, bool bDoInsert )
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
        xUndoDoc    ( std::move(pUndo) ),
        xRedoDoc    ( std::move(pRedo) ),
        nOldRefresh ( nOldRD ),
        nNewRefresh ( nNewRD ),
        bWithInsert ( bDoInsert )
{
    OSL_ENSURE( aOldRange.aStart == aNewRange.aStart, "AreaLink moved ?" );
}

OUString ScUndoUpdateAreaLink::GetComment() const
{
    return ScResId( STR_UNDO_UPDATELINK );   //! own text ??
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
            rDoc.DeleteAreaTab( aOldRange, InsertDeleteFlags::ALL & ~InsertDeleteFlags::NOTE );
            xUndoDoc->UndoToDocument(aOldRange, InsertDeleteFlags::ALL & ~InsertDeleteFlags::NOTE, false, rDoc);
        }
        else
        {
            ScRange aCopyRange( aOldRange.aStart, ScAddress(nEndX,nEndY,nEndZ) );
            rDoc.DeleteAreaTab( aCopyRange, InsertDeleteFlags::ALL & ~InsertDeleteFlags::NOTE );
            xUndoDoc->CopyToDocument(aCopyRange, InsertDeleteFlags::ALL & ~InsertDeleteFlags::NOTE, false, rDoc);
        }
    }
    else
    {
        if ( bWithInsert )
        {
            rDoc.FitBlock( aOldRange, aNewRange );
            rDoc.DeleteAreaTab( aNewRange, InsertDeleteFlags::ALL & ~InsertDeleteFlags::NOTE );
            xRedoDoc->CopyToDocument(aNewRange, InsertDeleteFlags::ALL & ~InsertDeleteFlags::NOTE, false, rDoc);
        }
        else
        {
            ScRange aCopyRange( aOldRange.aStart, ScAddress(nEndX,nEndY,nEndZ) );
            rDoc.DeleteAreaTab( aCopyRange, InsertDeleteFlags::ALL & ~InsertDeleteFlags::NOTE );
            xRedoDoc->CopyToDocument(aCopyRange, InsertDeleteFlags::ALL & ~InsertDeleteFlags::NOTE, false, rDoc);
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
        pDocShell->PostPaint( aWorkRange, PaintPartFlags::Grid );

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
