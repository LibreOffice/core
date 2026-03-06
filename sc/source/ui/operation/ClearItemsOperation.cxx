/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/ClearItemsOperation.hxx>

#include <docsh.hxx>
#include <editable.hxx>
#include <markdata.hxx>
#include <undoblk.hxx>

#include <memory>

namespace sc
{
ClearItemsOperation::ClearItemsOperation(ScDocShell& rDocShell, const ScMarkData& rMark,
                                         const sal_uInt16* pWhich, bool bApi)
    : Operation(OperationType::ClearItems, false, bApi)
    , mrDocShell(rDocShell)
    , mrMark(rMark)
    , mpWhich(pWhich)
{
}

bool ClearItemsOperation::runImplementation()
{
    ScDocShellModificator aModificator(mrDocShell);

    ScDocument& rDoc = mrDocShell.GetDocument();
    bool bUndo(rDoc.IsUndoEnabled());

    ScEditableTester aTester = ScEditableTester::CreateAndTestSelection(rDoc, mrMark);
    if (!aTester.IsEditable())
    {
        if (!mbApi)
            mrDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    //  #i12940# ClearItems is called (from setPropertyToDefault) directly with uno object's cached
    //  MarkData (GetMarkData), so rMark must be changed to multi selection for ClearSelectionItems
    //  here.

    ScMarkData aMultiMark = convertMark(mrMark);
    aMultiMark.SetMarking(false); // for MarkToMulti
    aMultiMark.MarkToMulti();
    const ScRange& aMarkRange = aMultiMark.GetMultiMarkArea();

    if (bUndo)
    {
        SCTAB nStartTab = aMarkRange.aStart.Tab();
        SCTAB nEndTab = aMarkRange.aEnd.Tab();

        ScDocumentUniquePtr pUndoDoc(new ScDocument(SCDOCMODE_UNDO));
        pUndoDoc->InitUndo(rDoc, nStartTab, nEndTab);
        rDoc.CopyToDocument(aMarkRange, InsertDeleteFlags::ATTRIB, true, *pUndoDoc, &aMultiMark);

        mrDocShell.GetUndoManager()->AddUndoAction(std::make_unique<ScUndoClearItems>(
            &mrDocShell, aMultiMark, std::move(pUndoDoc), mpWhich));
    }

    rDoc.ClearSelectionItems(mpWhich, aMultiMark);

    syncSheetViews();

    mrDocShell.PostPaint(aMarkRange, PaintPartFlags::Grid, SC_PF_LINES | SC_PF_TESTMERGE);
    aModificator.SetDocumentModified();

    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
