/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/SetValuesOperation.hxx>

#include <docfunc.hxx>
#include <docsh.hxx>
#include <address.hxx>
#include <document.hxx>
#include <undocell.hxx>

#include <memory>

namespace sc
{
SetValuesOperation::SetValuesOperation(ScDocFunc& rDocFunc, ScDocShell& rDocShell,
                                       const ScAddress& rPosition,
                                       std::vector<double> const& rValues, bool bApi)
    : Operation(OperationType::SetValues, true, bApi)
    , mrDocFunc(rDocFunc)
    , mrDocShell(rDocShell)
    , mrPosition(rPosition)
    , mrValues(rValues)
{
}

bool SetValuesOperation::runImplementation()
{
    ScDocument& rDoc = mrDocShell.GetDocument();

    SCROW nLastRow = mrPosition.Row() + mrValues.size() - 1;
    if (nLastRow > rDoc.MaxRow())
        return false;

    ScRange aRange(mrPosition);
    aRange.aEnd.SetRow(nLastRow);

    ScDocShellModificator aModificator(mrDocShell);

    if (rDoc.IsUndoEnabled())
    {
        auto pUndoObj = std::make_unique<sc::UndoSetCells>(mrDocShell, mrPosition);
        rDoc.TransferCellValuesTo(mrPosition, mrValues.size(), pUndoObj->GetOldValues());
        pUndoObj->SetNewValues(mrValues);
        mrDocShell.GetUndoManager()->AddUndoAction(std::move(pUndoObj));
    }

    rDoc.SetValues(mrPosition, mrValues);

    mrDocShell.PostPaint(aRange, PaintPartFlags::Grid);
    aModificator.SetDocumentModified();

    if (mbApi)
        mrDocFunc.NotifyInputHandler(mrPosition);

    return true;
}
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
