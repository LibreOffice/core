/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/DeleteContentOperation.hxx>

#include <docfuncutil.hxx>
#include <docfunc.hxx>
#include <editable.hxx>
#include <markdata.hxx>
#include <SheetViewOperationsTester.hxx>

#include <memory>

namespace sc
{
DeleteContentOperation::DeleteContentOperation(ScDocFunc& rDocFunc, ScDocShell& rDocShell,
                                               const ScMarkData& rMark, InsertDeleteFlags nFlags,
                                               bool bRecord, bool bApi)
    : Operation(OperationType::DeleteContent, bRecord, bApi)
    , mrDocFunc(rDocFunc)
    , mrDocShell(rDocShell)
    , mrMark(rMark)
    , mnFlags(nFlags)
{
}

bool DeleteContentOperation::runImplementation()
{
    ScDocShellModificator aModificator(mrDocShell);

    if (!mrMark.IsMarked() && !mrMark.IsMultiMarked())
    {
        SAL_WARN("sc", "Operation DeleteContents without markings.");
        return false;
    }

    ScDocument& rDoc = mrDocShell.GetDocument();

    if (mbRecord && !rDoc.IsUndoEnabled())
        mbRecord = false;

    if (!checkSheetViewProtection())
        return false;

    ScEditableTester aTester = ScEditableTester::CreateAndTestSelection(rDoc, mrMark);
    if (!aTester.IsEditable())
    {
        if (!mbApi)
            mrDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    ScMarkData aMultiMark = mrMark;
    aMultiMark.SetMarking(false); // for MarkToMulti

    ScDocumentUniquePtr pUndoDoc;
    bool bMulti = aMultiMark.IsMultiMarked();
    aMultiMark.MarkToMulti();
    const ScRange& aMarkRange = aMultiMark.GetMultiMarkArea();
    ScRange aExtendedRange(aMarkRange);
    if (rDoc.ExtendMerge(aExtendedRange, true))
        bMulti = false;

    // no objects on protected tabs
    bool bObjects
        = (mnFlags & InsertDeleteFlags::OBJECTS) && !sc::DocFuncUtil::hasProtectedTab(rDoc, mrMark);

    sal_uInt16 nExtFlags = 0; // extra flags are needed only if attributes are deleted
    if (mnFlags & InsertDeleteFlags::ATTRIB)
        mrDocShell.UpdatePaintExt(nExtFlags, aMarkRange);

    //  order of operations:
    //  1) BeginDrawUndo
    //  2) Delete objects (DrawUndo will be filled)
    //  3) Copy content for undo and set up undo actions
    //  4) Delete content

    bool bDrawUndo = bObjects || (mnFlags & InsertDeleteFlags::NOTE);
    if (mbRecord && bDrawUndo)
        rDoc.BeginDrawUndo();

    if (bObjects)
    {
        if (bMulti)
            rDoc.DeleteObjectsInSelection(aMultiMark);
        else
            rDoc.DeleteObjectsInArea(aMarkRange.aStart.Col(), aMarkRange.aStart.Row(),
                                     aMarkRange.aEnd.Col(), aMarkRange.aEnd.Row(), aMultiMark);
    }

    // To keep track of all non-empty cells within the deleted area.
    std::shared_ptr<ScSimpleUndo::DataSpansType> pDataSpans;

    if (mbRecord)
    {
        pUndoDoc = sc::DocFuncUtil::createDeleteContentsUndoDoc(rDoc, aMultiMark, aMarkRange,
                                                                mnFlags, bMulti);
        pDataSpans = sc::DocFuncUtil::getNonEmptyCellSpans(rDoc, aMultiMark, aMarkRange);
    }

    rDoc.DeleteSelection(mnFlags, aMultiMark);

    // add undo action after drawing undo is complete (objects and note captions)
    if (mbRecord)
    {
        sc::DocFuncUtil::addDeleteContentsUndo(mrDocShell.GetUndoManager(), &mrDocShell, aMultiMark,
                                               aExtendedRange, std::move(pUndoDoc), mnFlags,
                                               pDataSpans, bMulti, bDrawUndo);
    }

    if (!mrDocFunc.AdjustRowHeight(aExtendedRange, true, mbApi))
        mrDocShell.PostPaint(aExtendedRange, PaintPartFlags::Grid, nExtFlags);
    else if (nExtFlags & SC_PF_LINES)
        ScDocFunc::PaintAbove(mrDocShell, aExtendedRange); // for lines above the range

    aModificator.SetDocumentModified();

    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
