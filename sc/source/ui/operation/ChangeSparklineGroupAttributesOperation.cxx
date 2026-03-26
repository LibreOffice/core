/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/ChangeSparklineGroupAttributesOperation.hxx>

#include <docsh.hxx>
#include <SparklineGroup.hxx>
#include <undo/UndoEditSparklineGroup.hxx>

namespace sc
{
ChangeSparklineGroupAttributesOperation::ChangeSparklineGroupAttributesOperation(
    ScDocShell& rDocShell, std::shared_ptr<SparklineGroup> const& pExistingSparklineGroup,
    SparklineAttributes const& rNewAttributes)
    : Operation(OperationType::SparklineGroupChange, false, false)
    , mrDocShell(rDocShell)
    , mpExistingSparklineGroup(pExistingSparklineGroup)
    , maNewAttributes(rNewAttributes)
{
}

bool ChangeSparklineGroupAttributesOperation::runImplementation()
{
    auto pUndo = std::make_unique<UndoEditSparklneGroup>(mrDocShell, mpExistingSparklineGroup,
                                                         maNewAttributes);
    // change sparkline group attributes by "redoing"
    pUndo->Redo();
    mrDocShell.GetUndoManager()->AddUndoAction(std::move(pUndo));

    syncSheetViews();

    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
