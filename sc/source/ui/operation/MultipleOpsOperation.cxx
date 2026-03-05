/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/MultipleOpsOperation.hxx>

#include <docsh.hxx>
#include <editable.hxx>
#include <markdata.hxx>
#include <undoblk.hxx>

#include <vcl/weld.hxx>

#include <memory>

namespace sc
{
MultipleOpsOperation::MultipleOpsOperation(ScDocShell& rDocShell, const ScRange& rRange,
                                           const ScMarkData* pTabMark, const ScTabOpParam& rParam,
                                           bool bRecord, bool bApi)
    : Operation(OperationType::MultipleOps, bRecord, bApi)
    , mrDocShell(rDocShell)
    , maRange(rRange)
    , mpTabMark(pTabMark)
    , maParam(rParam)
{
}

bool MultipleOpsOperation::runImplementation()
{
    ScDocShellModificator aModificator(mrDocShell);

    bool bSuccess = false;
    ScDocument& rDoc = mrDocShell.GetDocument();
    SCCOL nStartCol = maRange.aStart.Col();
    SCROW nStartRow = maRange.aStart.Row();
    SCTAB nStartTab = maRange.aStart.Tab();
    SCCOL nEndCol = maRange.aEnd.Col();
    SCROW nEndRow = maRange.aEnd.Row();
    SCTAB nEndTab = maRange.aEnd.Tab();

    if (mbRecord && !rDoc.IsUndoEnabled())
        mbRecord = false;

    ScMarkData aMark(rDoc.GetSheetLimits());
    if (mpTabMark)
        aMark = *mpTabMark;
    else
    {
        for (SCTAB nTab = nStartTab; nTab <= nEndTab; nTab++)
            aMark.SelectTable(nTab, true);
    }

    if (!checkSheetViewProtection())
        return false;

    ScEditableTester aTester = ScEditableTester::CreateAndTestSelectedBlock(
        rDoc, nStartCol, nStartRow, nEndCol, nEndRow, aMark);
    if (aTester.IsEditable())
    {
        weld::WaitObject aWait(ScDocShell::GetActiveDialogParent());
        rDoc.SetDirty(maRange, false);
        if (mbRecord)
        {
            //! take selected sheets into account also when undoing
            ScDocumentUniquePtr pUndoDoc(new ScDocument(SCDOCMODE_UNDO));
            pUndoDoc->InitUndo(rDoc, nStartTab, nEndTab);
            rDoc.CopyToDocument(maRange, InsertDeleteFlags::ALL & ~InsertDeleteFlags::NOTE, false,
                                *pUndoDoc);

            mrDocShell.GetUndoManager()->AddUndoAction(std::make_unique<ScUndoTabOp>(
                &mrDocShell, nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab,
                std::move(pUndoDoc), maParam.aRefFormulaCell, maParam.aRefFormulaEnd,
                maParam.aRefRowCell, maParam.aRefColCell, maParam.meMode));
        }

        rDoc.InsertTableOp(maParam, nStartCol, nStartRow, nEndCol, nEndRow, aMark);
        mrDocShell.PostPaintGridAll();
        aModificator.SetDocumentModified();
        bSuccess = true;
    }
    else if (!mbApi)
        mrDocShell.ErrorMessage(aTester.GetMessageId());

    return bSuccess;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
