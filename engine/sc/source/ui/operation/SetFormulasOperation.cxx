/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/SetFormulasOperation.hxx>

#include <docfunc.hxx>
#include <docsh.hxx>
#include <address.hxx>
#include <document.hxx>
#include <formulacell.hxx>
#include <undocell.hxx>

#include <o3tl/safeint.hxx>

#include <memory>

namespace sc
{
SetFormulasOperation::SetFormulasOperation(ScDocFunc& rDocFunc, ScDocShell& rDocShell,
                                           const ScAddress& rPosition,
                                           std::vector<ScFormulaCell*>& rCells, bool bApi)
    : Operation(OperationType::SetFormulas, true, bApi)
    , mrDocFunc(rDocFunc)
    , mrDocShell(rDocShell)
    , mrPosition(rPosition)
    , mrCells(rCells)
{
}

bool SetFormulasOperation::runImplementation()
{
    ScAddress aPosition = convertAddress(mrPosition);
    ScDocument& rDoc = mrDocShell.GetDocument();

    const size_t nLength = mrCells.size();
    if (aPosition.Row() + nLength - 1 > o3tl::make_unsigned(rDoc.MaxRow()))
        return false;

    ScRange aRange(aPosition);
    aRange.aEnd.IncRow(nLength - 1);

    ScDocShellModificator aModificator(mrDocShell);
    bool bUndo = rDoc.IsUndoEnabled();

    std::unique_ptr<sc::UndoSetCells> pUndoObj;
    if (bUndo)
    {
        pUndoObj.reset(new sc::UndoSetCells(mrDocShell, aPosition));
        rDoc.TransferCellValuesTo(aPosition, nLength, pUndoObj->GetOldValues());
    }

    rDoc.SetFormulaCells(aPosition, mrCells);

    // For performance reasons API calls may disable calculation while
    // operating and recalculate once when done. If through user interaction
    // and AutoCalc is disabled, calculate the formula (without its
    // dependencies) once so the result matches the current document's content.
    if (!mbApi && !rDoc.GetAutoCalc())
    {
        for (auto* pCell : mrCells)
        {
            // calculate just the cell once and set Dirty again
            pCell->Interpret();
            pCell->SetDirtyVar();
            rDoc.PutInFormulaTree(pCell);
        }
    }

    if (bUndo)
    {
        pUndoObj->SetNewValues(mrCells);
        mrDocShell.GetUndoManager()->AddUndoAction(std::move(pUndoObj));
    }

    syncSheetViews();

    mrDocShell.PostPaint(aRange, PaintPartFlags::Grid);
    aModificator.SetDocumentModified();

    if (mbApi)
        mrDocFunc.NotifyInputHandler(aPosition);

    return true;
}
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
