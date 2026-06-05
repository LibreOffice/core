/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/UnmergeCellsOperation.hxx>

#include <scitems.hxx>
#include <attrib.hxx>
#include <detfunc.hxx>
#include <docfunc.hxx>
#include <docpool.hxx>
#include <docsh.hxx>
#include <document.hxx>
#include <patattr.hxx>
#include <rangelst.hxx>
#include <undoblk.hxx>

#include <memory>

namespace sc
{
UnmergeCellsOperation::UnmergeCellsOperation(ScDocFunc& rDocFunc, ScDocShell& rDocShell,
                                             ScCellMergeOption const& rOption, bool bRecord,
                                             ScUndoRemoveMerge* pUndoRemoveMerge)
    : Operation(OperationType::UnmergeCells, bRecord, true)
    , mrDocFunc(rDocFunc)
    , mrDocShell(rDocShell)
    , maOption(rOption)
    , mpUndoRemoveMerge(pUndoRemoveMerge)
{
}

bool UnmergeCellsOperation::runImplementation()
{
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

    if (aOption.maTabs.empty())
        // Nothing to unmerge.
        return true;

    ScDocShellModificator aModificator(mrDocShell);
    ScDocument& rDoc = mrDocShell.GetDocument();

    bool bRecord = mbRecord;
    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;

    ScDocument* pUndoDoc = (mpUndoRemoveMerge ? mpUndoRemoveMerge->GetUndoDoc() : nullptr);
    assert(pUndoDoc || !mpUndoRemoveMerge);
    for (const SCTAB nTab : aOption.maTabs)
    {
        ScRange aRange = aOption.getSingleRange(nTab);
        if (!rDoc.HasAttrib(aRange, HasAttrFlags::Merged))
            continue;

        ScRange aExtended = aRange;
        rDoc.ExtendMerge(aExtended);
        ScRange aRefresh = aExtended;
        rDoc.ExtendOverlapped(aRefresh);

        if (bRecord)
        {
            if (!pUndoDoc)
            {
                pUndoDoc = new ScDocument(SCDOCMODE_UNDO);
                pUndoDoc->InitUndo(rDoc, *aOption.maTabs.begin(), *aOption.maTabs.rbegin());
            }
            rDoc.CopyToDocument(aExtended, InsertDeleteFlags::ATTRIB, false, *pUndoDoc);
        }

        const SfxPoolItem& rDefAttr = rDoc.GetPool()->GetUserOrPoolDefaultItem(ATTR_MERGE);
        ScPatternAttr aPattern(rDoc.getCellAttributeHelper());
        aPattern.ItemSetPut(rDefAttr);
        rDoc.ApplyPatternAreaTab(aRange.aStart.Col(), aRange.aStart.Row(), aRange.aEnd.Col(),
                                 aRange.aEnd.Row(), nTab, aPattern);

        rDoc.RemoveFlagsTab(aExtended.aStart.Col(), aExtended.aStart.Row(), aExtended.aEnd.Col(),
                            aExtended.aEnd.Row(), nTab, ScMF::Hor | ScMF::Ver);

        rDoc.ExtendMerge(aRefresh, true);

        if (!mrDocFunc.AdjustRowHeight(aExtended, true, true))
            mrDocShell.PostPaint(aExtended, PaintPartFlags::Grid);

        bool bDone = ScDetectiveFunc(rDoc, nTab).DeleteAll(ScDetectiveDelete::Circles);
        if (bDone)
            mrDocFunc.DetectiveMarkInvalid(nTab);
    }

    if (bRecord)
    {
        if (mpUndoRemoveMerge)
        {
            // If mpUndoRemoveMerge was passed, the caller is responsible for
            // adding it to Undo. Just add the current option.
            mpUndoRemoveMerge->AddCellMergeOption(aOption);
        }
        else
        {
            mrDocShell.GetUndoManager()->AddUndoAction(std::make_unique<ScUndoRemoveMerge>(
                mrDocShell, aOption, ScDocumentUniquePtr(pUndoDoc)));
        }
    }

    syncSheetViews();

    aModificator.SetDocumentModified();

    return true;
}
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
