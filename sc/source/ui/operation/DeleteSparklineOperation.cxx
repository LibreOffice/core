/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/DeleteSparklineOperation.hxx>

#include <docsh.hxx>
#include <undo/UndoDeleteSparkline.hxx>

namespace sc
{
DeleteSparklineOperation::DeleteSparklineOperation(ScDocShell& rDocShell, ScAddress const& rAddress)
    : Operation(OperationType::SparklineDelete, false, false)
    , mrDocShell(rDocShell)
    , maAddress(rAddress)
{
}

bool DeleteSparklineOperation::runImplementation()
{
    auto& rDocument = mrDocShell.GetDocument();

    if (!rDocument.HasSparkline(maAddress))
        return false;

    if (!checkSheetViewProtection())
        return false;

    auto pUndoDeleteSparkline = std::make_unique<UndoDeleteSparkline>(mrDocShell, maAddress);
    // delete sparkline by "redoing"
    pUndoDeleteSparkline->Redo();
    mrDocShell.GetUndoManager()->AddUndoAction(std::move(pUndoDeleteSparkline));

    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
