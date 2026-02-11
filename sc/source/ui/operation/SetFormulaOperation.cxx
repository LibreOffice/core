/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/SetFormulaOperation.hxx>

#include <docfuncutil.hxx>
#include <docfunc.hxx>
#include <address.hxx>
#include <editable.hxx>
#include <undocell.hxx>
#include <validat.hxx>
#include <detfunc.hxx>
#include <stringutil.hxx>
#include <formulacell.hxx>

#include <memory>

namespace sc
{
SetFormulaOperation::SetFormulaOperation(ScDocFunc& rDocFunc, ScDocShell& rDocShell,
                                         const ScAddress& rPosition, ScFormulaCell* pFormulaCell,
                                         bool bApi)
    : Operation(OperationType::SetFormula, true, bApi)
    , mrDocFunc(rDocFunc)
    , mrDocShell(rDocShell)
    , mrPosition(rPosition)
    , mpFormulaCell(pFormulaCell)
{
}

bool SetFormulaOperation::runImplementation()
{
    ScAddress aPosition = convertAddress(mrPosition);

    ScFormulaCell* pCell(mpFormulaCell);
    std::unique_ptr<ScFormulaCell> xCell(mpFormulaCell);

    sc::SheetViewOperationsTester aSheetViewTester(ScDocShell::GetViewData());

    ScDocShellModificator aModificator(mrDocShell);
    ScDocument& rDoc = mrDocShell.GetDocument();
    bool bUndo = rDoc.IsUndoEnabled();

    bool bHeight = rDoc.HasAttrib(ScRange(aPosition), HasAttrFlags::NeedHeight);

    ScCellValue aOldVal;
    if (bUndo)
        aOldVal.assign(rDoc, aPosition);

    pCell = rDoc.SetFormulaCell(aPosition, xCell.release());

    // For performance reasons API calls may disable calculation while
    // operating and recalculate once when done. If through user interaction
    // and AutoCalc is disabled, calculate the formula (without its
    // dependencies) once so the result matches the current document's content.
    if (!mbApi && !rDoc.GetAutoCalc() && pCell)
    {
        // calculate just the cell once and set Dirty again
        pCell->Interpret();
        pCell->SetDirtyVar();
        rDoc.PutInFormulaTree(pCell);
    }

    if (bUndo)
    {
        SfxUndoManager* pUndoMgr = mrDocShell.GetUndoManager();
        ScCellValue aNewVal;
        aNewVal.assign(rDoc, aPosition);
        pUndoMgr->AddUndoAction(
            std::make_unique<ScUndoSetCell>(&mrDocShell, aPosition, aOldVal, aNewVal));
    }

    aSheetViewTester.sync();

    if (bHeight)
        mrDocFunc.AdjustRowHeight(ScRange(aPosition), true, mbApi);

    mrDocShell.PostPaintCell(aPosition);
    aModificator.SetDocumentModified();

    // #103934#; notify editline and cell in edit mode
    if (mbApi)
        mrDocFunc.NotifyInputHandler(aPosition);

    return true;
}

} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
