/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/DeleteCellOperation.hxx>

#include <docfuncutil.hxx>
#include <docfunc.hxx>
#include <markdata.hxx>
#include <editable.hxx>
#include <address.hxx>

#include <memory>

namespace sc
{
DeleteCellOperation::DeleteCellOperation(ScDocFunc& rDocFunc, ScDocShell& rDocShell,
                                         ScAddress const& rPosition, const ScMarkData& rMark,
                                         InsertDeleteFlags nFlags, bool bRecord, bool bApi)
    : Operation(OperationType::DeleteCell, bRecord, bApi)
    , mrDocFunc(rDocFunc)
    , mrDocShell(rDocShell)
    , mrPosition(rPosition)
    , mrMark(rMark)
    , mnFlags(nFlags)
{
}

bool DeleteCellOperation::runImplementation()
{
    ScDocShellModificator aModificator(mrDocShell);

    ScAddress const& rPos = mrPosition;
    ScMarkData const& rMark = mrMark;

    ScDocument& rDoc = mrDocShell.GetDocument();

    if (mbRecord && !rDoc.IsUndoEnabled())
        mbRecord = false;

    sc::SheetViewOperationsTester aSheetViewTester(ScDocShell::GetViewData());
    if (!aSheetViewTester.check(meType))
        return false;

    ScEditableTester aTester = ScEditableTester::CreateAndTestSelectedBlock(
        rDoc, rPos.Col(), rPos.Row(), rPos.Col(), rPos.Row(), rMark);
    if (!aTester.IsEditable())
    {
        mrDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    // no objects on protected tabs
    bool bObjects
        = (mnFlags & InsertDeleteFlags::OBJECTS) && !sc::DocFuncUtil::hasProtectedTab(rDoc, rMark);

    sal_uInt16 nExtFlags = 0; // extra flags are needed only if attributes are deleted
    if (mnFlags & InsertDeleteFlags::ATTRIB)
        mrDocShell.UpdatePaintExt(nExtFlags, ScRange(rPos));

    //  order of operations:
    //  1) BeginDrawUndo
    //  2) delete objects (DrawUndo is filled)
    //  3) copy contents for undo
    //  4) delete contents
    //  5) add undo-action

    bool bDrawUndo = bObjects || (mnFlags & InsertDeleteFlags::NOTE); // needed for shown notes
    if (bDrawUndo && mbRecord)
        rDoc.BeginDrawUndo();

    if (bObjects)
        rDoc.DeleteObjectsInArea(rPos.Col(), rPos.Row(), rPos.Col(), rPos.Row(), rMark);

    // To keep track of all non-empty cells within the deleted area.
    std::shared_ptr<ScSimpleUndo::DataSpansType> pDataSpans;

    ScDocumentUniquePtr pUndoDoc;
    if (mbRecord)
    {
        pUndoDoc = sc::DocFuncUtil::createDeleteContentsUndoDoc(rDoc, rMark, ScRange(rPos), mnFlags,
                                                                false);
        pDataSpans = sc::DocFuncUtil::getNonEmptyCellSpans(rDoc, rMark, ScRange(rPos));
    }

    tools::Long nBefore(mrDocShell.GetTwipWidthHint(rPos));
    rDoc.DeleteArea(rPos.Col(), rPos.Row(), rPos.Col(), rPos.Row(), rMark, mnFlags);

    if (mbRecord)
    {
        sc::DocFuncUtil::addDeleteContentsUndo(mrDocShell.GetUndoManager(), &mrDocShell, rMark,
                                               ScRange(rPos), std::move(pUndoDoc), mnFlags,
                                               pDataSpans, false, bDrawUndo);
    }

    if (!mrDocFunc.AdjustRowHeight(ScRange(rPos), true, mbApi))
        mrDocShell.PostPaint(rPos.Col(), rPos.Row(), rPos.Tab(), rPos.Col(), rPos.Row(), rPos.Tab(),
                             PaintPartFlags::Grid, nExtFlags, nBefore);

    aModificator.SetDocumentModified();

    return true;
}
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
