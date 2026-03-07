/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/ConvertFormulaToValueOperation.hxx>

#include <docsh.hxx>
#include <editable.hxx>
#include <undoconvert.hxx>

namespace sc
{
ConvertFormulaToValueOperation::ConvertFormulaToValueOperation(ScDocShell& rDocShell,
                                                               const ScRange& rRange,
                                                               bool bInteraction)
    : Operation(OperationType::ConvertFormulaToValue, true, !bInteraction)
    , mrDocShell(rDocShell)
    , maRange(rRange)
{
}

bool ConvertFormulaToValueOperation::runImplementation()
{
    ScDocShellModificator aModificator(mrDocShell);
    ScDocument& rDoc = mrDocShell.GetDocument();

    if (mbRecord && !rDoc.IsUndoEnabled())
        mbRecord = false;

    if (!checkSheetViewProtection())
        return false;

    ScEditableTester aTester
        = ScEditableTester::CreateAndTestRange(rDoc, maRange, sc::EditAction::Unknown);
    if (!aTester.IsEditable())
    {
        if (!mbApi)
            mrDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    sc::TableValues aUndoVals(maRange);
    sc::TableValues* pUndoVals = mbRecord ? &aUndoVals : nullptr;

    rDoc.ConvertFormulaToValue(maRange, pUndoVals);

    if (mbRecord && pUndoVals)
    {
        mrDocShell.GetUndoManager()->AddUndoAction(
            std::make_unique<sc::UndoFormulaToValue>(&mrDocShell, *pUndoVals));
    }

    mrDocShell.PostPaint(maRange, PaintPartFlags::Grid);
    mrDocShell.PostDataChanged();
    rDoc.BroadcastCells(maRange, SfxHintId::ScDataChanged);
    aModificator.SetDocumentModified();

    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
