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
#include "cell.hxx"
#include "paramisc.hxx"
#include "postit.hxx"
#include "docuno.hxx"
#include "progress.hxx"

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
                ScDocument* pNewUndoDoc, sal_Bool bNewMulti,
                sal_uInt16 nNewFlags, sal_Bool bObjects )
    :   ScSimpleUndo( pNewDocShell ),
        aRange      ( rRange ),
        aMarkData   ( rMark ),
        pUndoDoc    ( pNewUndoDoc ),
        pDrawUndo   ( NULL ),
        nFlags      ( nNewFlags ),
        bMulti      ( bNewMulti )   // unnecessary
{
    if (bObjects)
        pDrawUndo = GetSdrUndoAction( pDocShell->GetDocument() );

    if ( !(aMarkData.IsMarked() || aMarkData.IsMultiMarked()) )     // if no cell is selected:
        aMarkData.SetMarkArea( aRange );                            // select cell under cursor

    SetChangeTrack();
}

ScUndoDeleteContents::~ScUndoDeleteContents()
{
    delete pUndoDoc;
    DeleteSdrUndoAction( pDrawUndo );
}

rtl::OUString ScUndoDeleteContents::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_DELETECONTENTS );    // "Delete"
}

void ScUndoDeleteContents::SetChangeTrack()
{
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
    if ( pChangeTrack && (nFlags & IDF_CONTENTS) )
        pChangeTrack->AppendContentRange( aRange, pUndoDoc,
            nStartChangeAction, nEndChangeAction );
    else
        nStartChangeAction = nEndChangeAction = 0;
}

void ScUndoDeleteContents::DoChange( const sal_Bool bUndo )
{
    ScDocument* pDoc = pDocShell->GetDocument();

    SetViewMarkData( aMarkData );

    sal_uInt16 nExtFlags = 0;

    if (bUndo)  // only Undo
    {
        sal_uInt16 nUndoFlags = IDF_NONE; // copy either all or none of the content
        if (nFlags & IDF_CONTENTS)        // (Only the correct ones have been copied into UndoDoc)
            nUndoFlags |= IDF_CONTENTS;
        if (nFlags & IDF_ATTRIB)
            nUndoFlags |= IDF_ATTRIB;
        if (nFlags & IDF_EDITATTR)          // Edit-Engine attribute
            nUndoFlags |= IDF_STRING;       // -> Cells will be changed
        // do not create clones of note captions, they will be restored via drawing undo
        nUndoFlags |= IDF_NOCAPTIONS;

        ScRange aCopyRange = aRange;
        SCTAB nTabCount = pDoc->GetTableCount();
        aCopyRange.aStart.SetTab(0);
        aCopyRange.aEnd.SetTab(nTabCount-1);

        pUndoDoc->CopyToDocument( aCopyRange, nUndoFlags, bMulti, pDoc, &aMarkData );

        DoSdrUndoAction( pDrawUndo, pDoc );

        ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
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
        sal_uInt16 nRedoFlags = (nFlags & ~IDF_OBJECTS) | IDF_NOCAPTIONS;
        pDoc->DeleteSelection( nRedoFlags, aMarkData );
        aMarkData.MarkToSimple();

        SetChangeTrack();
    }

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if ( !( (pViewShell) && pViewShell->AdjustRowHeight(
                                aRange.aStart.Row(), aRange.aEnd.Row() ) ) )
/*A*/   pDocShell->PostPaint( aRange, PAINT_GRID | PAINT_EXTRAS, nExtFlags );

    pDocShell->PostDataChanged();
    if (pViewShell)
        pViewShell->CellContentChanged();

    ShowTable( aRange );
}

void ScUndoDeleteContents::Undo()
{
    BeginUndo();
    DoChange( sal_True );
    EndUndo();

    // #i97876# Spreadsheet data changes are not notified
    ScModelObj* pModelObj = ScModelObj::getImplementation( pDocShell->GetModel() );
    if ( pModelObj && pModelObj->HasChangesListeners() )
    {
        ScRangeList aChangeRanges;
        aChangeRanges.Append( aRange );
        pModelObj->NotifyChanges( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "cell-change" ) ), aChangeRanges );
    }
}

void ScUndoDeleteContents::Redo()
{
    BeginRedo();
    DoChange( false );
    EndRedo();

    // #i97876# Spreadsheet data changes are not notified
    ScModelObj* pModelObj = ScModelObj::getImplementation( pDocShell->GetModel() );
    if ( pModelObj && pModelObj->HasChangesListeners() )
    {
        ScRangeList aChangeRanges;
        aChangeRanges.Append( aRange );
        pModelObj->NotifyChanges( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "cell-change" ) ), aChangeRanges );
    }
}

void ScUndoDeleteContents::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->DeleteContents( nFlags, sal_True );
}

sal_Bool ScUndoDeleteContents::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

ScUndoFillTable::ScUndoFillTable( ScDocShell* pNewDocShell,
                const ScMarkData& rMark,
                SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                SCCOL nEndX, SCROW nEndY, SCTAB nEndZ,
                ScDocument* pNewUndoDoc, sal_Bool bNewMulti, SCTAB nSrc,
                sal_uInt16 nFlg, sal_uInt16 nFunc, sal_Bool bSkip, sal_Bool bLink )
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

rtl::OUString ScUndoFillTable::GetComment() const
{
    return ScGlobal::GetRscString( STR_FILL_TAB );
}

void ScUndoFillTable::SetChangeTrack()
{
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
    if ( pChangeTrack )
    {
        SCTAB nTabCount = pDocShell->GetDocument()->GetTableCount();
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

void ScUndoFillTable::DoChange( const sal_Bool bUndo )
{
    ScDocument* pDoc = pDocShell->GetDocument();

    SetViewMarkData( aMarkData );

    if (bUndo)  // only Undo
    {
        SCTAB nTabCount = pDoc->GetTableCount();
        ScRange aWorkRange(aRange);
        ScMarkData::iterator itr = aMarkData.begin(), itrEnd = aMarkData.end();
        for (; itr != itrEnd && *itr < nTabCount; ++itr)
            if (*itr != nSrcTab)
            {
                aWorkRange.aStart.SetTab(*itr);
                aWorkRange.aEnd.SetTab(*itr);
                if (bMulti)
                    pDoc->DeleteSelectionTab( *itr, IDF_ALL, aMarkData );
                else
                    pDoc->DeleteAreaTab( aWorkRange, IDF_ALL );
                pUndoDoc->CopyToDocument( aWorkRange, IDF_ALL, bMulti, pDoc, &aMarkData );
            }

        ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
        if ( pChangeTrack )
            pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );
    }
    else        // only Redo
    {
        aMarkData.MarkToMulti();
        pDoc->FillTabMarked( nSrcTab, aMarkData, nFlags, nFunction, bSkipEmpty, bAsLink );
        aMarkData.MarkToSimple();
        SetChangeTrack();
    }

    pDocShell->PostPaint(0,0,0,MAXCOL,MAXROW,MAXTAB, PAINT_GRID|PAINT_EXTRAS);
    pDocShell->PostDataChanged();

    //  CellContentChanged comes with the selection

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        SCTAB nTab = pViewShell->GetViewData()->GetTabNo();
        if ( !aMarkData.GetTableSelect(nTab) )
            pViewShell->SetTabNo( nSrcTab );

        pViewShell->DoneBlockMode();    // causes problems otherwise since selection is on the wrong sheet.
    }
}

void ScUndoFillTable::Undo()
{
    BeginUndo();
    DoChange( sal_True );
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
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->FillTab( nFlags, nFunction, bSkipEmpty, bAsLink );
}

sal_Bool ScUndoFillTable::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

ScUndoSelectionAttr::ScUndoSelectionAttr( ScDocShell* pNewDocShell,
                const ScMarkData& rMark,
                SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                SCCOL nEndX, SCROW nEndY, SCTAB nEndZ,
                ScDocument* pNewUndoDoc, sal_Bool bNewMulti,
                const ScPatternAttr* pNewApply,
                const SvxBoxItem* pNewOuter, const SvxBoxInfoItem* pNewInner )
    :   ScSimpleUndo( pNewDocShell ),
        aMarkData   ( rMark ),
        aRange      ( nStartX, nStartY, nStartZ, nEndX, nEndY, nEndZ ),
        pUndoDoc    ( pNewUndoDoc ),
        bMulti      ( bNewMulti )
{
    ScDocumentPool* pPool = pDocShell->GetDocument()->GetPool();
    pApplyPattern = (ScPatternAttr*) &pPool->Put( *pNewApply );
    pLineOuter = pNewOuter ? (SvxBoxItem*) &pPool->Put( *pNewOuter ) : NULL;
    pLineInner = pNewInner ? (SvxBoxInfoItem*) &pPool->Put( *pNewInner ) : NULL;
}

ScUndoSelectionAttr::~ScUndoSelectionAttr()
{
    ScDocumentPool* pPool = pDocShell->GetDocument()->GetPool();
    pPool->Remove(*pApplyPattern);
    if (pLineOuter)
        pPool->Remove(*pLineOuter);
    if (pLineInner)
        pPool->Remove(*pLineInner);

    delete pUndoDoc;
}

rtl::OUString ScUndoSelectionAttr::GetComment() const
{
    //"Attribute" "/Lines"
    return ScGlobal::GetRscString( pLineOuter ? STR_UNDO_SELATTRLINES : STR_UNDO_SELATTR );
}

ScEditDataArray* ScUndoSelectionAttr::GetDataArray()
{
    return &aDataArray;
}

void ScUndoSelectionAttr::DoChange( const sal_Bool bUndo )
{
    ScDocument* pDoc = pDocShell->GetDocument();

    SetViewMarkData( aMarkData );

    ScRange aEffRange( aRange );
    if ( pDoc->HasAttrib( aEffRange, HASATTR_MERGED ) )         // merged cells?
        pDoc->ExtendMerge( aEffRange );

    sal_uInt16 nExtFlags = 0;
    pDocShell->UpdatePaintExt( nExtFlags, aEffRange );

    ChangeEditData(bUndo);

    if (bUndo)  // only for Undo
    {
        ScRange aCopyRange = aRange;
        SCTAB nTabCount = pDoc->GetTableCount();
        aCopyRange.aStart.SetTab(0);
        aCopyRange.aEnd.SetTab(nTabCount-1);
        pUndoDoc->CopyToDocument( aCopyRange, IDF_ATTRIB, bMulti, pDoc, &aMarkData );
    }
    else        // only for Redo
    {
        aMarkData.MarkToMulti();
        pDoc->ApplySelectionPattern( *pApplyPattern, aMarkData );
        aMarkData.MarkToSimple();

        if (pLineOuter)
            pDoc->ApplySelectionFrame( aMarkData, pLineOuter, pLineInner );
    }

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if ( !( (pViewShell) && pViewShell->AdjustBlockHeight() ) )
/*A*/   pDocShell->PostPaint( aEffRange, PAINT_GRID | PAINT_EXTRAS, nExtFlags );

    ShowTable( aRange );
}

void ScUndoSelectionAttr::ChangeEditData( const bool bUndo )
{
    ScDocument* pDoc = pDocShell->GetDocument();
    for (const ScEditDataArray::Item* pItem = aDataArray.First(); pItem; pItem = aDataArray.Next())
    {
        ScBaseCell* pCell;
        pDoc->GetCell(pItem->GetCol(), pItem->GetRow(), pItem->GetTab(), pCell);
        if (!pCell || pCell->GetCellType() != CELLTYPE_EDIT)
            continue;

        ScEditCell* pEditCell = static_cast<ScEditCell*>(pCell);
        if (bUndo)
            pEditCell->SetData(pItem->GetOldData(), NULL);
        else
            pEditCell->SetData(pItem->GetNewData(), NULL);
    }
}

void ScUndoSelectionAttr::Undo()
{
    BeginUndo();
    DoChange( sal_True );
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
    if (rTarget.ISA(ScTabViewTarget))
    {
        ScTabViewShell& rViewShell = *((ScTabViewTarget&)rTarget).GetViewShell();
        if (pLineOuter)
            rViewShell.ApplyPatternLines( *pApplyPattern, pLineOuter, pLineInner, sal_True );
        else
            rViewShell.ApplySelectionPattern( *pApplyPattern, sal_True );
    }
}

sal_Bool ScUndoSelectionAttr::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
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

rtl::OUString ScUndoAutoFill::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_AUTOFILL ); //"Fill"
}

void ScUndoAutoFill::SetChangeTrack()
{
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->AppendContentRange( aBlockRange, pUndoDoc,
            nStartChangeAction, nEndChangeAction );
    else
        nStartChangeAction = nEndChangeAction = 0;
}

void ScUndoAutoFill::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();

    SCTAB nTabCount = pDoc->GetTableCount();
    ScMarkData::iterator itr = aMarkData.begin(), itrEnd = aMarkData.end();
    for (; itr != itrEnd && *itr < nTabCount; ++itr)
    {
        ScRange aWorkRange = aBlockRange;
        aWorkRange.aStart.SetTab(*itr);
        aWorkRange.aEnd.SetTab(*itr);

        sal_uInt16 nExtFlags = 0;
        pDocShell->UpdatePaintExt( nExtFlags, aWorkRange );
        pDoc->DeleteAreaTab( aWorkRange, IDF_AUTOFILL );
        pUndoDoc->CopyToDocument( aWorkRange, IDF_AUTOFILL, false, pDoc );

        pDoc->ExtendMerge( aWorkRange, sal_True );
        pDocShell->PostPaint( aWorkRange, PAINT_GRID, nExtFlags );
    }
    pDocShell->PostDataChanged();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->CellContentChanged();

    ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
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

    ScDocument* pDoc = pDocShell->GetDocument();
    if ( fStartValue != MAXDOUBLE )
    {
        SCCOL nValX = (eFillDir == FILL_TO_LEFT) ? aSource.aEnd.Col() : aSource.aStart.Col();
        SCROW nValY = (eFillDir == FILL_TO_TOP ) ? aSource.aEnd.Row() : aSource.aStart.Row();
        SCTAB nTab = aSource.aStart.Tab();
        pDoc->SetValue( nValX, nValY, nTab, fStartValue );
    }
    sal_uLong nProgCount;
    if (eFillDir == FILL_TO_BOTTOM || eFillDir == FILL_TO_TOP)
        nProgCount = aSource.aEnd.Col() - aSource.aStart.Col() + 1;
    else
        nProgCount = aSource.aEnd.Row() - aSource.aStart.Row() + 1;
    nProgCount *= nCount;
    ScProgress aProgress( pDoc->GetDocumentShell(),
            ScGlobal::GetRscString(STR_FILL_SERIES_PROGRESS), nProgCount );

    pDoc->Fill( aSource.aStart.Col(), aSource.aStart.Row(),
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
    if (rTarget.ISA(ScTabViewTarget))
    {
        ScTabViewShell& rViewShell = *((ScTabViewTarget&)rTarget).GetViewShell();
        if (eFillCmd==FILL_SIMPLE)
            rViewShell.FillSimple( eFillDir, sal_True );
        else
            rViewShell.FillSeries( eFillDir, eFillCmd, eFillDateCmd,
                                   fStartValue, fStepValue, fMaxValue, sal_True );
    }
}

sal_Bool ScUndoAutoFill::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
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

rtl::OUString ScUndoMerge::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_MERGE );
}

void ScUndoMerge::DoChange( bool bUndo ) const
{
    using ::std::set;

    if (maOption.maTabs.empty())
        // Nothing to do.
        return;

    ScDocument* pDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    ScRange aCurRange = maOption.getSingleRange(pDocShell->GetCurTab());
    ScUndoUtil::MarkSimpleBlock(pDocShell, aCurRange);

    for (set<SCTAB>::const_iterator itr = maOption.maTabs.begin(), itrEnd = maOption.maTabs.end();
          itr != itrEnd; ++itr)
    {
        SCTAB nTab = *itr;
        ScRange aRange = maOption.getSingleRange(nTab);

        if (bUndo)
            // remove merge (contents are copied back below from undo document)
            pDoc->RemoveMerge( aRange.aStart.Col(), aRange.aStart.Row(), aRange.aStart.Tab() );
        else
        {
            // repeat merge, but do not remove note captions (will be done by drawing redo below)
            pDoc->DoMerge( aRange.aStart.Tab(),
                           aRange.aStart.Col(), aRange.aStart.Row(),
                           aRange.aEnd.Col(),   aRange.aEnd.Row(), false );

            if (maOption.mbCenter)
            {
                pDoc->ApplyAttr( aRange.aStart.Col(), aRange.aStart.Row(),
                                 aRange.aStart.Tab(),
                                 SvxHorJustifyItem( SVX_HOR_JUSTIFY_CENTER, ATTR_HOR_JUSTIFY ) );
                pDoc->ApplyAttr( aRange.aStart.Col(), aRange.aStart.Row(),
                                 aRange.aStart.Tab(),
                                 SvxVerJustifyItem( SVX_VER_JUSTIFY_CENTER, ATTR_VER_JUSTIFY ) );
            }
        }

        // undo -> copy back deleted contents
        if (bUndo && mpUndoDoc)
        {
            pDoc->DeleteAreaTab( aRange, IDF_CONTENTS|IDF_NOCAPTIONS );
            mpUndoDoc->CopyToDocument( aRange, IDF_ALL|IDF_NOCAPTIONS, false, pDoc );
        }

        // redo -> merge contents again
        else if (!bUndo && mbMergeContents)
        {
            pDoc->DoMergeContents( aRange.aStart.Tab(),
                                   aRange.aStart.Col(), aRange.aStart.Row(),
                                   aRange.aEnd.Col(), aRange.aEnd.Row() );
        }

        if (bUndo)
            DoSdrUndoAction( mpDrawUndo, pDoc );
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
    if (rTarget.ISA(ScTabViewTarget))
    {
        ScTabViewShell& rViewShell = *((ScTabViewTarget&)rTarget).GetViewShell();
        sal_Bool bCont = false;
        rViewShell.MergeCells( false, bCont, sal_True );
    }
}

sal_Bool ScUndoMerge::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

ScUndoAutoFormat::ScUndoAutoFormat( ScDocShell* pNewDocShell,
                        const ScRange& rRange, ScDocument* pNewUndoDoc,
                        const ScMarkData& rMark, sal_Bool bNewSize, sal_uInt16 nNewFormatNo )
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

rtl::OUString ScUndoAutoFormat::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_AUTOFORMAT );   //"Auto-Format"
}

void ScUndoAutoFormat::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();

    SCTAB nTabCount = pDoc->GetTableCount();
    pDoc->DeleteArea( aBlockRange.aStart.Col(), aBlockRange.aStart.Row(),
                      aBlockRange.aEnd.Col(), aBlockRange.aEnd.Row(),
                      aMarkData, IDF_ATTRIB );
    ScRange aCopyRange = aBlockRange;
    aCopyRange.aStart.SetTab(0);
    aCopyRange.aEnd.SetTab(nTabCount-1);
    pUndoDoc->CopyToDocument( aCopyRange, IDF_ATTRIB, false, pDoc, &aMarkData );

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
                                    IDF_NONE, false, pDoc, &aMarkData );
        pUndoDoc->CopyToDocument( 0, nStartY, 0, MAXCOL, nEndY, nTabCount-1,
                                    IDF_NONE, false, pDoc, &aMarkData );
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

    ScDocument* pDoc = pDocShell->GetDocument();

    SCCOL nStartX = aBlockRange.aStart.Col();
    SCROW nStartY = aBlockRange.aStart.Row();
    SCTAB nStartZ = aBlockRange.aStart.Tab();
    SCCOL nEndX = aBlockRange.aEnd.Col();
    SCROW nEndY = aBlockRange.aEnd.Row();
    SCTAB nEndZ = aBlockRange.aEnd.Tab();

    pDoc->AutoFormat( nStartX, nStartY, nEndX, nEndY, nFormatNo, aMarkData );

    if (bSize)
    {
        VirtualDevice aVirtDev;
        Fraction aZoomX(1,1);
        Fraction aZoomY = aZoomX;
        double nPPTX,nPPTY;
        ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
        if (pViewShell)
        {
            ScViewData* pData = pViewShell->GetViewData();
            nPPTX = pData->GetPPTX();
            nPPTY = pData->GetPPTY();
            aZoomX = pData->GetZoomX();
            aZoomY = pData->GetZoomY();
        }
        else
        {
            // Keep zoom at 100
            nPPTX = ScGlobal::nScreenPPTX;
            nPPTY = ScGlobal::nScreenPPTY;
        }

        sal_Bool bFormula = false;  // remember

        for (SCTAB nTab=nStartZ; nTab<=nEndZ; nTab++)
        {
            ScMarkData aDestMark;
            aDestMark.SelectOneTable( nTab );
            aDestMark.SetMarkArea( ScRange( nStartX, nStartY, nTab, nEndX, nEndY, nTab ) );
            aDestMark.MarkToMulti();

            // as SC_SIZE_VISOPT
            for (SCROW nRow=nStartY; nRow<=nEndY; nRow++)
            {
                sal_uInt8 nOld = pDoc->GetRowFlags(nRow,nTab);
                bool bHidden = pDoc->RowHidden(nRow, nTab);
                if ( !bHidden && ( nOld & CR_MANUALSIZE ) )
                    pDoc->SetRowFlags( nRow, nTab, nOld & ~CR_MANUALSIZE );
            }
            pDoc->SetOptimalHeight( nStartY, nEndY, nTab, 0, &aVirtDev,
                                        nPPTX, nPPTY, aZoomX, aZoomY, false );

            for (SCCOL nCol=nStartX; nCol<=nEndX; nCol++)
                if (!pDoc->ColHidden(nCol, nTab))
                {
                    sal_uInt16 nThisSize = STD_EXTRA_WIDTH + pDoc->GetOptimalColWidth( nCol, nTab,
                                                &aVirtDev, nPPTX, nPPTY, aZoomX, aZoomY, bFormula,
                                                &aDestMark );
                    pDoc->SetColWidth( nCol, nTab, nThisSize );
                    pDoc->ShowCol( nCol, nTab, sal_True );
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
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->AutoFormat( nFormatNo, sal_True );
}

sal_Bool ScUndoAutoFormat::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

ScUndoReplace::ScUndoReplace( ScDocShell* pNewDocShell, const ScMarkData& rMark,
                                    SCCOL nCurX, SCROW nCurY, SCTAB nCurZ,
                                    const String& rNewUndoStr, ScDocument* pNewUndoDoc,
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
    ScDocument* pDoc = pDocShell->GetDocument();
    ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
    if ( pChangeTrack )
    {
        if ( pUndoDoc )
        {   //! UndoDoc includes only teh changed cells,
            // that is why an Iterator can be used
            pChangeTrack->AppendContentsIfInRefDoc( pUndoDoc,
                nStartChangeAction, nEndChangeAction );
        }
        else
        {
            nStartChangeAction = pChangeTrack->GetActionMax() + 1;
            ScChangeActionContent* pContent = new ScChangeActionContent(
                ScRange( aCursorPos) );
            pContent->SetOldValue( aUndoStr, pDoc );
            pContent->SetNewValue( pDoc->GetCell( aCursorPos ), pDoc );
            pChangeTrack->Append( pContent );
            nEndChangeAction = pChangeTrack->GetActionMax();
        }
    }
    else
        nStartChangeAction = nEndChangeAction = 0;
}

rtl::OUString ScUndoReplace::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_REPLACE );  // "Replace"
}

void ScUndoReplace::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    ShowTable( aCursorPos.Tab() );

    if (pUndoDoc)       // only for ReplaceAll !!
    {
        OSL_ENSURE(pSearchItem->GetCommand() == SVX_SEARCHCMD_REPLACE_ALL,
                   "ScUndoReplace:: Wrong Mode");

        SetViewMarkData( aMarkData );

//! selected sheet
//! select range ?

        // Undo document has no row/column information, thus copy with
        // bColRowFlags = FALSE to not destroy Outline groups

        sal_uInt16 nUndoFlags = (pSearchItem->GetPattern()) ? IDF_ATTRIB : IDF_CONTENTS;
        pUndoDoc->CopyToDocument( 0,      0,      0,
                                  MAXCOL, MAXROW, MAXTAB,
                                  nUndoFlags, false, pDoc, NULL, false );   // without row flags
        pDocShell->PostPaintGridAll();
    }
    else if (pSearchItem->GetPattern() &&
             pSearchItem->GetCommand() == SVX_SEARCHCMD_REPLACE)
    {
        String aTempStr = pSearchItem->GetSearchString();       // toggle
        pSearchItem->SetSearchString(pSearchItem->GetReplaceString());
        pSearchItem->SetReplaceString(aTempStr);
        pDoc->ReplaceStyle( *pSearchItem,
                            aCursorPos.Col(), aCursorPos.Row(), aCursorPos.Tab(),
                            aMarkData, sal_True);
        pSearchItem->SetReplaceString(pSearchItem->GetSearchString());
        pSearchItem->SetSearchString(aTempStr);
        if (pViewShell)
            pViewShell->MoveCursorAbs( aCursorPos.Col(), aCursorPos.Row(),
                                       SC_FOLLOW_JUMP, false, false );
        pDocShell->PostPaintGridAll();
    }
    else if (pSearchItem->GetCellType() == SVX_SEARCHIN_NOTE)
    {
        ScPostIt* pNote = pDoc->GetNotes( aCursorPos.Tab() )->findByAddress( aCursorPos );
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
        if ( aUndoStr.Search('\n') != STRING_NOTFOUND )
            pDoc->PutCell( aCursorPos, new ScEditCell( aUndoStr, pDoc ) );
        else
            pDoc->SetString( aCursorPos.Col(), aCursorPos.Row(), aCursorPos.Tab(), aUndoStr );
        if (pViewShell)
            pViewShell->MoveCursorAbs( aCursorPos.Col(), aCursorPos.Row(),
                                       SC_FOLLOW_JUMP, false, false );
        pDocShell->PostPaintGridAll();
    }

    ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );

    EndUndo();
}

void ScUndoReplace::Redo()
{
    BeginRedo();

    ScDocument* pDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    if (pViewShell)
        pViewShell->MoveCursorAbs( aCursorPos.Col(), aCursorPos.Row(),
                                   SC_FOLLOW_JUMP, false, false );
    if (pUndoDoc)
    {
        if (pViewShell)
        {
            SetViewMarkData( aMarkData );

            pViewShell->SearchAndReplace( pSearchItem, false, sal_True );
        }
    }
    else if (pSearchItem->GetPattern() &&
             pSearchItem->GetCommand() == SVX_SEARCHCMD_REPLACE)
    {
        pDoc->ReplaceStyle( *pSearchItem,
                            aCursorPos.Col(), aCursorPos.Row(), aCursorPos.Tab(),
                            aMarkData, sal_True);
        pDocShell->PostPaintGridAll();
    }
    else
        if (pViewShell)
            pViewShell->SearchAndReplace( pSearchItem, false, sal_True );

    SetChangeTrack();

    EndRedo();
}

void ScUndoReplace::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->SearchAndReplace( pSearchItem, sal_True, false );
}

sal_Bool ScUndoReplace::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

// multi-operation (only simple blocks)
ScUndoTabOp::ScUndoTabOp( ScDocShell* pNewDocShell,
                SCCOL nStartX, SCROW nStartY, SCTAB nStartZ,
                SCCOL nEndX, SCROW nEndY, SCTAB nEndZ, ScDocument* pNewUndoDoc,
                const ScRefAddress& rFormulaCell,
                const ScRefAddress& rFormulaEnd,
                const ScRefAddress& rRowCell,
                const ScRefAddress& rColCell,
                sal_uInt8 nMd )
    :   ScSimpleUndo( pNewDocShell ),
        aRange          ( nStartX, nStartY, nStartZ, nEndX, nEndY, nEndZ ),
        pUndoDoc        ( pNewUndoDoc ),
        theFormulaCell  ( rFormulaCell ),
        theFormulaEnd   ( rFormulaEnd ),
        theRowCell      ( rRowCell ),
        theColCell      ( rColCell ),
        nMode           ( nMd )
{
}

ScUndoTabOp::~ScUndoTabOp()
{
    delete pUndoDoc;
}

rtl::OUString ScUndoTabOp::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_TABOP );    // "Multiple operation"
}

void ScUndoTabOp::Undo()
{
    BeginUndo();

    ScUndoUtil::MarkSimpleBlock( pDocShell, aRange );

    sal_uInt16 nExtFlags = 0;
    pDocShell->UpdatePaintExt( nExtFlags, aRange );

    ScDocument* pDoc = pDocShell->GetDocument();
    pDoc->DeleteAreaTab( aRange,IDF_ALL & ~IDF_NOTE );
    pUndoDoc->CopyToDocument( aRange, IDF_ALL & ~IDF_NOTE, false, pDoc );
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

    ScTabOpParam aParam( theFormulaCell, theFormulaEnd,
                         theRowCell,     theColCell,
                         nMode );

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->TabOp( aParam, false);

    EndRedo();
}

void ScUndoTabOp::Repeat(SfxRepeatTarget& /* rTarget */)
{
}

sal_Bool ScUndoTabOp::CanRepeat(SfxRepeatTarget& /* rTarget */) const
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
    ScDocument* pDoc = pDocShell->GetDocument();
    ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
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

rtl::OUString ScUndoConversion::GetComment() const
{
    rtl::OUString aText;
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
        ScDocument* pDoc = pDocShell->GetDocument();
        ShowTable( rCursorPos.Tab() );

        SetViewMarkData( aMarkData );

        SCTAB nTabCount = pDoc->GetTableCount();
        //  Undo/Redo-doc has only selected tables

        sal_Bool bMulti = aMarkData.IsMultiMarked();
        pRefDoc->CopyToDocument( 0,      0,      0,
                                 MAXCOL, MAXROW, nTabCount-1,
                                 IDF_CONTENTS, bMulti, pDoc, &aMarkData );
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
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
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
    if( rTarget.ISA( ScTabViewTarget ) )
        ((ScTabViewTarget&)rTarget).GetViewShell()->DoSheetConversion( maConvParam, sal_True );
}

sal_Bool ScUndoConversion::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return rTarget.ISA( ScTabViewTarget );
}

ScUndoRefConversion::ScUndoRefConversion( ScDocShell* pNewDocShell,
                                         const ScRange& aMarkRange, const ScMarkData& rMark,
                                         ScDocument* pNewUndoDoc, ScDocument* pNewRedoDoc, sal_Bool bNewMulti, sal_uInt16 nNewFlag) :
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

rtl::OUString ScUndoRefConversion::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_ENTERDATA ); // "Input"
}

void ScUndoRefConversion::SetChangeTrack()
{
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
    if ( pChangeTrack && (nFlags & IDF_FORMULA) )
        pChangeTrack->AppendContentsIfInRefDoc( pUndoDoc,
            nStartChangeAction, nEndChangeAction );
    else
        nStartChangeAction = nEndChangeAction = 0;
}

void ScUndoRefConversion::DoChange( ScDocument* pRefDoc)
{
    ScDocument* pDoc = pDocShell->GetDocument();

    ShowTable(aRange);

    SetViewMarkData( aMarkData );

    ScRange aCopyRange = aRange;
    SCTAB nTabCount = pDoc->GetTableCount();
    aCopyRange.aStart.SetTab(0);
    aCopyRange.aEnd.SetTab(nTabCount-1);
    pRefDoc->CopyToDocument( aCopyRange, nFlags, bMulti, pDoc, &aMarkData );
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
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
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
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->DoRefConversion();
}

sal_Bool ScUndoRefConversion::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
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

rtl::OUString ScUndoRefreshLink::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_UPDATELINK );
}

void ScUndoRefreshLink::Undo()
{
    BeginUndo();

    sal_Bool bMakeRedo = !pRedoDoc;
    if (bMakeRedo)
        pRedoDoc = new ScDocument( SCDOCMODE_UNDO );

    sal_Bool bFirst = sal_True;
    ScDocument* pDoc = pDocShell->GetDocument();
    SCTAB nCount = pDoc->GetTableCount();
    for (SCTAB nTab=0; nTab<nCount; nTab++)
        if (pUndoDoc->HasTable(nTab))
        {
            ScRange aRange(0,0,nTab,MAXCOL,MAXROW,nTab);
            if (bMakeRedo)
            {
                if (bFirst)
                    pRedoDoc->InitUndo( pDoc, nTab, nTab, sal_True, sal_True );
                else
                    pRedoDoc->AddUndoTab( nTab, nTab, true, true );
                bFirst = false;
                pDoc->CopyToDocument(aRange, IDF_ALL, false, pRedoDoc);
                pRedoDoc->SetLink( nTab,
                                   pDoc->GetLinkMode(nTab),
                                   pDoc->GetLinkDoc(nTab),
                                   pDoc->GetLinkFlt(nTab),
                                   pDoc->GetLinkOpt(nTab),
                                   pDoc->GetLinkTab(nTab),
                                   pDoc->GetLinkRefreshDelay(nTab) );
            }

            pDoc->DeleteAreaTab( aRange,IDF_ALL );
            pUndoDoc->CopyToDocument( aRange, IDF_ALL, false, pDoc );
            pDoc->SetLink( nTab, pUndoDoc->GetLinkMode(nTab), pUndoDoc->GetLinkDoc(nTab),
                                 pUndoDoc->GetLinkFlt(nTab),  pUndoDoc->GetLinkOpt(nTab),
                                 pUndoDoc->GetLinkTab(nTab),
                                 pUndoDoc->GetLinkRefreshDelay(nTab) );
        }

    pDocShell->PostPaintGridAll();

    EndUndo();
}

void ScUndoRefreshLink::Redo()
{
    OSL_ENSURE(pRedoDoc, "No RedoDoc for ScUndoRefreshLink::Redo");

    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    SCTAB nCount = pDoc->GetTableCount();
    for (SCTAB nTab=0; nTab<nCount; nTab++)
        if (pRedoDoc->HasTable(nTab))
        {
            ScRange aRange(0,0,nTab,MAXCOL,MAXROW,nTab);

            pDoc->DeleteAreaTab( aRange, IDF_ALL );
            pRedoDoc->CopyToDocument( aRange, IDF_ALL, false, pDoc );
            pDoc->SetLink( nTab,
                           pRedoDoc->GetLinkMode(nTab),
                           pRedoDoc->GetLinkDoc(nTab),
                           pRedoDoc->GetLinkFlt(nTab),
                           pRedoDoc->GetLinkOpt(nTab),
                           pRedoDoc->GetLinkTab(nTab),
                           pRedoDoc->GetLinkRefreshDelay(nTab) );
        }

    pDocShell->PostPaintGridAll();

    EndUndo();
}

void ScUndoRefreshLink::Repeat(SfxRepeatTarget& /* rTarget */)
{
    // makes no sense
}

sal_Bool ScUndoRefreshLink::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;
}

ScAreaLink* lcl_FindAreaLink( sfx2::LinkManager* pLinkManager, const String& rDoc,
                            const String& rFlt, const String& rOpt,
                            const String& rSrc, const ScRange& rDest )
{
    const ::sfx2::SvBaseLinks& rLinks = pLinkManager->GetLinks();
    sal_uInt16 nCount = pLinkManager->GetLinks().size();
    for (sal_uInt16 i=0; i<nCount; i++)
    {
        ::sfx2::SvBaseLink* pBase = *rLinks[i];
        if (pBase->ISA(ScAreaLink))
            if ( ((ScAreaLink*)pBase)->IsEqual( rDoc, rFlt, rOpt, rSrc, rDest ) )
                return (ScAreaLink*)pBase;
    }

    OSL_FAIL("ScAreaLink not found");
    return NULL;
}

ScUndoInsertAreaLink::ScUndoInsertAreaLink( ScDocShell* pShell,
                            const String& rDoc,
                            const String& rFlt, const String& rOpt,
                            const String& rArea, const ScRange& rDestRange,
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

rtl::OUString ScUndoInsertAreaLink::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_INSERTAREALINK );
}

void ScUndoInsertAreaLink::Undo()
{
    ScDocument* pDoc = pDocShell->GetDocument();
    sfx2::LinkManager* pLinkManager = pDoc->GetLinkManager();

    ScAreaLink* pLink = lcl_FindAreaLink( pLinkManager, aDocName, aFltName, aOptions,
                                            aAreaName, aRange );
    if (pLink)
        pLinkManager->Remove( pLink );

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );     // Navigator
}

void ScUndoInsertAreaLink::Redo()
{
    ScDocument* pDoc = pDocShell->GetDocument();
    sfx2::LinkManager* pLinkManager = pDoc->GetLinkManager();

    ScAreaLink* pLink = new ScAreaLink( pDocShell, aDocName, aFltName, aOptions,
                                            aAreaName, aRange.aStart, nRefreshDelay );
    pLink->SetInCreate( sal_True );
    pLink->SetDestArea( aRange );
    pLinkManager->InsertFileLink( *pLink, OBJECT_CLIENT_FILE, aDocName, &aFltName, &aAreaName );
    pLink->Update();
    pLink->SetInCreate( false );

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );     // Navigator
}

void ScUndoInsertAreaLink::Repeat(SfxRepeatTarget& /* rTarget */)
{
    // makes no sense
}

sal_Bool ScUndoInsertAreaLink::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;
}

ScUndoRemoveAreaLink::ScUndoRemoveAreaLink( ScDocShell* pShell,
                            const String& rDoc, const String& rFlt, const String& rOpt,
                            const String& rArea, const ScRange& rDestRange,
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

rtl::OUString ScUndoRemoveAreaLink::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_REMOVELINK );   //! eigener Text ??
}

void ScUndoRemoveAreaLink::Undo()
{
    ScDocument* pDoc = pDocShell->GetDocument();
    sfx2::LinkManager* pLinkManager = pDoc->GetLinkManager();

    ScAreaLink* pLink = new ScAreaLink( pDocShell, aDocName, aFltName, aOptions,
                                        aAreaName, aRange.aStart, nRefreshDelay );
    pLink->SetInCreate( sal_True );
    pLink->SetDestArea( aRange );
    pLinkManager->InsertFileLink( *pLink, OBJECT_CLIENT_FILE, aDocName, &aFltName, &aAreaName );
    pLink->Update();
    pLink->SetInCreate( false );

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );     // Navigator
}

void ScUndoRemoveAreaLink::Redo()
{
    ScDocument* pDoc = pDocShell->GetDocument();
    sfx2::LinkManager* pLinkManager = pDoc->GetLinkManager();

    ScAreaLink* pLink = lcl_FindAreaLink( pLinkManager, aDocName, aFltName, aOptions,
                                            aAreaName, aRange );
    if (pLink)
        pLinkManager->Remove( pLink );

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );     // Navigator
}

void ScUndoRemoveAreaLink::Repeat(SfxRepeatTarget& /* rTarget */)
{
    // makes no sense
}

sal_Bool ScUndoRemoveAreaLink::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;
}

ScUndoUpdateAreaLink::ScUndoUpdateAreaLink( ScDocShell* pShell,
                            const String& rOldD, const String& rOldF, const String& rOldO,
                            const String& rOldA, const ScRange& rOldR, sal_uLong nOldRD,
                            const String& rNewD, const String& rNewF, const String& rNewO,
                            const String& rNewA, const ScRange& rNewR, sal_uLong nNewRD,
                            ScDocument* pUndo, ScDocument* pRedo, sal_Bool bDoInsert )
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

rtl::OUString ScUndoUpdateAreaLink::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_UPDATELINK );   //! own text ??
}

void ScUndoUpdateAreaLink::DoChange( const sal_Bool bUndo ) const
{
    ScDocument* pDoc = pDocShell->GetDocument();

    SCCOL nEndX = Max( aOldRange.aEnd.Col(), aNewRange.aEnd.Col() );
    SCROW nEndY = Max( aOldRange.aEnd.Row(), aNewRange.aEnd.Row() );
    SCTAB nEndZ = Max( aOldRange.aEnd.Tab(), aNewRange.aEnd.Tab() );    //?

    if ( bUndo )
    {
        if ( bWithInsert )
        {
            pDoc->FitBlock( aNewRange, aOldRange );
            pDoc->DeleteAreaTab( aOldRange, IDF_ALL & ~IDF_NOTE );
            pUndoDoc->UndoToDocument( aOldRange, IDF_ALL & ~IDF_NOTE, false, pDoc );
        }
        else
        {
            ScRange aCopyRange( aOldRange.aStart, ScAddress(nEndX,nEndY,nEndZ) );
            pDoc->DeleteAreaTab( aCopyRange, IDF_ALL & ~IDF_NOTE );
            pUndoDoc->CopyToDocument( aCopyRange, IDF_ALL & ~IDF_NOTE, false, pDoc );
        }
    }
    else
    {
        if ( bWithInsert )
        {
            pDoc->FitBlock( aOldRange, aNewRange );
            pDoc->DeleteAreaTab( aNewRange, IDF_ALL & ~IDF_NOTE );
            pRedoDoc->CopyToDocument( aNewRange, IDF_ALL & ~IDF_NOTE, false, pDoc );
        }
        else
        {
            ScRange aCopyRange( aOldRange.aStart, ScAddress(nEndX,nEndY,nEndZ) );
            pDoc->DeleteAreaTab( aCopyRange, IDF_ALL & ~IDF_NOTE );
            pRedoDoc->CopyToDocument( aCopyRange, IDF_ALL & ~IDF_NOTE, false, pDoc );
        }
    }

    ScRange aWorkRange( aNewRange.aStart, ScAddress( nEndX, nEndY, nEndZ ) );
    pDoc->ExtendMerge( aWorkRange, sal_True );

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
    ScDocument* pDoc = pDocShell->GetDocument();
    sfx2::LinkManager* pLinkManager = pDoc->GetLinkManager();
    ScAreaLink* pLink = lcl_FindAreaLink( pLinkManager, aNewDoc, aNewFlt, aNewOpt,
                                            aNewArea, aNewRange );
    if (pLink)
    {
        pLink->SetSource( aOldDoc, aOldFlt, aOldOpt, aOldArea );        // old data in Link
        pLink->SetDestArea( aOldRange );
        pLink->SetRefreshDelay( nOldRefresh );
    }

    DoChange(sal_True);
}

void ScUndoUpdateAreaLink::Redo()
{
    ScDocument* pDoc = pDocShell->GetDocument();
    sfx2::LinkManager* pLinkManager = pDoc->GetLinkManager();
    ScAreaLink* pLink = lcl_FindAreaLink( pLinkManager, aOldDoc, aOldFlt, aOldOpt,
                                            aOldArea, aOldRange );
    if (pLink)
    {
        pLink->SetSource( aNewDoc, aNewFlt, aNewOpt, aNewArea );        // neue Werte im Link
        pLink->SetDestArea( aNewRange );
        pLink->SetRefreshDelay( nNewRefresh );
    }

    DoChange(false);
}

void ScUndoUpdateAreaLink::Repeat(SfxRepeatTarget& /* rTarget */)
{
    // makes no sense
}

sal_Bool ScUndoUpdateAreaLink::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
