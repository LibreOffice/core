/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/DeleteSparklineGroupOperation.hxx>

#include <docsh.hxx>
#include <SparklineGroup.hxx>
#include <undo/UndoDeleteSparklineGroup.hxx>

namespace sc
{
DeleteSparklineGroupOperation::DeleteSparklineGroupOperation(
    ScDocShell& rDocShell, std::shared_ptr<SparklineGroup> const& pSparklineGroup, SCTAB nTab)
    : Operation(OperationType::SparklineGroupDelete, false, false)
    , mrDocShell(rDocShell)
    , mpSparklineGroup(pSparklineGroup)
    , mnTab(nTab)
{
}

bool DeleteSparklineGroupOperation::runImplementation()
{
    if (!mpSparklineGroup)
        return false;

    SCTAB nTab = convertTab(mnTab);

    auto& rDocument = mrDocShell.GetDocument();

    if (!rDocument.HasTable(nTab))
        return false;

    auto pUndo = std::make_unique<UndoDeleteSparklineGroup>(mrDocShell, mpSparklineGroup, nTab);
    // delete sparkline group by "redoing"
    pUndo->Redo();
    mrDocShell.GetUndoManager()->AddUndoAction(std::move(pUndo));

    syncSheetViews();

    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
