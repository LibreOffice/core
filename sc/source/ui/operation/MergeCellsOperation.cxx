/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/MergeCellsOperation.hxx>

#include <detfunc.hxx>
#include <docfunc.hxx>
#include <docsh.hxx>
#include <document.hxx>
#include <drwlayer.hxx>
#include <editable.hxx>
#include <globstr.hrc>
#include <sc.hrc>
#include <undoblk.hxx>

#include <editeng/justifyitem.hxx>
#include <sfx2/bindings.hxx>
#include <svx/svxids.hrc>

namespace sc
{
MergeCellsOperation::MergeCellsOperation(ScDocShell& rDocShell, const ScCellMergeOption& rOption,
                                         bool bContents, bool bRecord, bool bApi,
                                         bool bEmptyMergedCells)
    : Operation(OperationType::MergeCells, bRecord, bApi)
    , mrDocShell(rDocShell)
    , maOption(rOption)
    , mbContents(bContents)
    , mbEmptyMergedCells(bEmptyMergedCells)
{
}

bool MergeCellsOperation::runImplementation()
{
    ScDocShellModificator aModificator(mrDocShell);

    ScCellMergeOption aOption(maOption);
    {
        std::set<SCTAB> aConvertedTabs;
        for (SCTAB nTab : aOption.maTabs)
        {
            ScAddress aConverted = convertAddress(ScAddress(0, 0, nTab));
            aConvertedTabs.insert(aConverted.Tab());
        }
        aOption.maTabs = aConvertedTabs;
    }

    SCCOL nStartCol = aOption.mnStartCol;
    SCROW nStartRow = aOption.mnStartRow;
    SCCOL nEndCol = aOption.mnEndCol;
    SCROW nEndRow = aOption.mnEndRow;
    if ((nStartCol == nEndCol && nStartRow == nEndRow) || aOption.maTabs.empty())
    {
        // Nothing to do.  Bail out quickly
        return true;
    }

    ScDocument& rDoc = mrDocShell.GetDocument();
    SCTAB nTab1 = *aOption.maTabs.begin(), nTab2 = *aOption.maTabs.rbegin();

    if (mbRecord && !rDoc.IsUndoEnabled())
        mbRecord = false;

    for (const auto& rTab : aOption.maTabs)
    {
        ScEditableTester aTester = ScEditableTester::CreateAndTestBlock(
            rDoc, rTab, nStartCol, nStartRow, nEndCol, nEndRow);
        if (!aTester.IsEditable())
        {
            if (!mbApi)
                mrDocShell.ErrorMessage(aTester.GetMessageId());
            return false;
        }

        if (rDoc.HasAttrib(nStartCol, nStartRow, rTab, nEndCol, nEndRow, rTab,
                           HasAttrFlags::Merged | HasAttrFlags::Overlapped))
        {
            // "Merge of already merged cells not possible"
            if (!mbApi)
                mrDocShell.ErrorMessage(STR_MSSG_MERGECELLS_0);
            return false;
        }
    }

    ScDocumentUniquePtr pUndoDoc;
    bool bNeedContentsUndo = false;
    for (const SCTAB nTab : aOption.maTabs)
    {
        bool bIsBlockEmpty
            = (nStartRow == nEndRow)
                  ? rDoc.IsEmptyData(nStartCol + 1, nStartRow, nEndCol, nEndRow, nTab)
                  : rDoc.IsEmptyData(nStartCol, nStartRow + 1, nStartCol, nEndRow, nTab)
                        && rDoc.IsEmptyData(nStartCol + 1, nStartRow, nEndCol, nEndRow, nTab);
        bool bNeedContents = mbContents && !bIsBlockEmpty;
        bool bNeedEmpty = mbEmptyMergedCells && !bIsBlockEmpty
                          && !bNeedContents; // if DoMergeContents then cells are emptied

        if (mbRecord)
        {
            // test if the range contains other notes which also implies that we need an undo document
            bool bHasNotes = rDoc.HasNote(nTab, nStartCol, nStartRow, nEndCol, nEndRow);
            if (!pUndoDoc)
            {
                pUndoDoc.reset(new ScDocument(SCDOCMODE_UNDO));
                pUndoDoc->InitUndo(rDoc, nTab1, nTab2);
            }
            // note captions are collected by drawing undo
            rDoc.CopyToDocument(nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab,
                                InsertDeleteFlags::ALL | InsertDeleteFlags::NOCAPTIONS, false,
                                *pUndoDoc);
            if (bHasNotes)
                rDoc.BeginDrawUndo();
        }

        if (bNeedContents)
            rDoc.DoMergeContents(nStartCol, nStartRow, nEndCol, nEndRow, nTab);
        else if (bNeedEmpty)
            rDoc.DoEmptyBlock(nStartCol, nStartRow, nEndCol, nEndRow, nTab);
        rDoc.DoMerge(nStartCol, nStartRow, nEndCol, nEndRow, nTab);

        if (aOption.mbCenter)
        {
            rDoc.ApplyAttr(nStartCol, nStartRow, nTab,
                           SvxHorJustifyItem(SvxCellHorJustify::Center, ATTR_HOR_JUSTIFY));
            rDoc.ApplyAttr(nStartCol, nStartRow, nTab,
                           SvxVerJustifyItem(SvxCellVerJustify::Center, ATTR_VER_JUSTIFY));
        }

        if (!mrDocShell.GetDocFunc().AdjustRowHeight(
                ScRange(0, nStartRow, nTab, rDoc.MaxCol(), nEndRow, nTab), true, mbApi))
            mrDocShell.PostPaint(nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab,
                                 PaintPartFlags::Grid);
        if (bNeedContents || aOption.mbCenter)
        {
            ScRange aRange(nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab);
            rDoc.SetDirty(aRange, true);
        }

        bool bDone = ScDetectiveFunc(rDoc, nTab).DeleteAll(ScDetectiveDelete::Circles);
        if (bDone)
            mrDocShell.GetDocFunc().DetectiveMarkInvalid(nTab);

        bNeedContentsUndo |= bNeedContents;
    }

    if (pUndoDoc)
    {
        std::unique_ptr<SdrUndoGroup> pDrawUndo
            = rDoc.GetDrawLayer() ? rDoc.GetDrawLayer()->GetCalcUndo() : nullptr;
        mrDocShell.GetUndoManager()->AddUndoAction(std::make_unique<ScUndoMerge>(
            &mrDocShell, aOption, bNeedContentsUndo, std::move(pUndoDoc), std::move(pDrawUndo)));
    }

    syncSheetViews();

    aModificator.SetDocumentModified();

    SfxBindings* pBindings = mrDocShell.GetViewBindings();
    if (pBindings)
    {
        pBindings->Invalidate(FID_MERGE_ON);
        pBindings->Invalidate(FID_MERGE_OFF);
        pBindings->Invalidate(FID_MERGE_TOGGLE);
    }

    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
