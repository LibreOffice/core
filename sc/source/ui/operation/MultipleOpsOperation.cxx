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

ScRefAddress MultipleOpsOperation::convertRefAddress(ScRefAddress const& rRefAddress)
{
    ScAddress aConverted = convertAddress(rRefAddress.GetAddress());
    ScRefAddress aResult;
    aResult.Set(aConverted, rRefAddress.IsRelCol(), rRefAddress.IsRelRow(), rRefAddress.IsRelTab());
    return aResult;
}

bool MultipleOpsOperation::runImplementation()
{
    ScDocShellModificator aModificator(mrDocShell);

    ScRange aRange = convertRange(maRange);
    ScTabOpParam aParam;
    aParam.aRefFormulaCell = convertRefAddress(maParam.aRefFormulaCell);
    aParam.aRefFormulaEnd = convertRefAddress(maParam.aRefFormulaEnd);
    aParam.aRefRowCell = convertRefAddress(maParam.aRefRowCell);
    aParam.aRefColCell = convertRefAddress(maParam.aRefColCell);
    aParam.meMode = maParam.meMode;

    bool bSuccess = false;
    ScDocument& rDoc = mrDocShell.GetDocument();
    SCCOL nStartCol = aRange.aStart.Col();
    SCROW nStartRow = aRange.aStart.Row();
    SCTAB nStartTab = aRange.aStart.Tab();
    SCCOL nEndCol = aRange.aEnd.Col();
    SCROW nEndRow = aRange.aEnd.Row();
    SCTAB nEndTab = aRange.aEnd.Tab();

    if (mbRecord && !rDoc.IsUndoEnabled())
        mbRecord = false;

    ScMarkData aMark(rDoc.GetSheetLimits());
    if (mpTabMark)
        aMark = convertMark(*mpTabMark);
    else
    {
        for (SCTAB nTab = nStartTab; nTab <= nEndTab; nTab++)
            aMark.SelectTable(nTab, true);
    }

    ScEditableTester aTester = ScEditableTester::CreateAndTestSelectedBlock(
        rDoc, nStartCol, nStartRow, nEndCol, nEndRow, aMark);
    if (aTester.IsEditable())
    {
        weld::WaitObject aWait(ScDocShell::GetActiveDialogParent());
        rDoc.SetDirty(aRange, false);
        if (mbRecord)
        {
            //! take selected sheets into account also when undoing
            ScDocumentUniquePtr pUndoDoc(new ScDocument(SCDOCMODE_UNDO));
            pUndoDoc->InitUndo(rDoc, nStartTab, nEndTab);
            rDoc.CopyToDocument(aRange, InsertDeleteFlags::ALL & ~InsertDeleteFlags::NOTE, false,
                                *pUndoDoc);

            mrDocShell.GetUndoManager()->AddUndoAction(std::make_unique<ScUndoTabOp>(
                &mrDocShell, nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab,
                std::move(pUndoDoc), aParam.aRefFormulaCell, aParam.aRefFormulaEnd,
                aParam.aRefRowCell, aParam.aRefColCell, aParam.meMode));
        }

        rDoc.InsertTableOp(aParam, nStartCol, nStartRow, nEndCol, nEndRow, aMark);

        syncSheetViews();
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
