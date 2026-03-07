/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/ChangeSparklineOperation.hxx>

#include <docsh.hxx>
#include <Sparkline.hxx>
#include <undo/UndoEditSparkline.hxx>

namespace sc
{
ChangeSparklineOperation::ChangeSparklineOperation(ScDocShell& rDocShell,
                                                   std::shared_ptr<Sparkline> const& rpSparkline,
                                                   SCTAB nTab, ScRangeList const& rDataRange)
    : Operation(OperationType::SparklineChange, false, false)
    , mrDocShell(rDocShell)
    , mpSparkline(rpSparkline)
    , mnTab(nTab)
    , maDataRange(rDataRange)
{
}

bool ChangeSparklineOperation::runImplementation()
{
    if (!checkSheetViewProtection())
        return false;

    auto pUndo = std::make_unique<UndoEditSparkline>(mrDocShell, mpSparkline, mnTab, maDataRange);
    // change sparkline by "redoing"
    pUndo->Redo();
    mrDocShell.GetUndoManager()->AddUndoAction(std::move(pUndo));
    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
