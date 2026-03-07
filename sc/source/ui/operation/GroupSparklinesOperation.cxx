/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/GroupSparklinesOperation.hxx>

#include <docsh.hxx>
#include <SparklineGroup.hxx>
#include <undo/UndoGroupSparklines.hxx>

namespace sc
{
GroupSparklinesOperation::GroupSparklinesOperation(ScDocShell& rDocShell, ScRange const& rRange,
                                                   std::shared_ptr<SparklineGroup> const& rpGroup)
    : Operation(OperationType::SparklineGroup, false, false)
    , mrDocShell(rDocShell)
    , maRange(rRange)
    , mpGroup(rpGroup)
{
}

bool GroupSparklinesOperation::runImplementation()
{
    if (!checkSheetViewProtection())
        return false;

    auto pUndo = std::make_unique<UndoGroupSparklines>(mrDocShell, maRange, mpGroup);
    // group sparklines by "redoing"
    pUndo->Redo();
    mrDocShell.GetUndoManager()->AddUndoAction(std::move(pUndo));
    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
