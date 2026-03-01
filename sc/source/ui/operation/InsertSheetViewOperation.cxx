/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/InsertSheetViewOperation.hxx>

#include <docsh.hxx>
#include <document.hxx>
#include <undo/UndoInsertSheetView.hxx>
#include <tabvwsh.hxx>
#include <uiitems.hxx>
#include <SheetViewTypes.hxx>

#include <sfx2/app.hxx>
#include <vcl/weld.hxx>

#include <memory>

namespace sc
{
InsertSheetViewOperation::InsertSheetViewOperation(ScDocShell& rDocShell, SCTAB nTab, bool bRecord)
    : Operation(OperationType::InsertSheetView, bRecord, false)
    , mrDocShell(rDocShell)
    , mnTab(nTab)
{
}

bool InsertSheetViewOperation::runImplementation()
{
    weld::WaitObject aWait(ScDocShell::GetActiveDialogParent());

    ScDocShellModificator aModificator(mrDocShell);

    ScDocument& rDoc = mrDocShell.GetDocument();
    if (mbRecord && !rDoc.IsUndoEnabled())
        mbRecord = false;

    if (mbRecord)
        rDoc.BeginDrawUndo();

    auto[nSheetViewID, nSheetViewTab] = rDoc.CreateNewSheetView(mnTab);
    if (nSheetViewID == InvalidSheetViewID)
        return false;

    mnSheetViewID = nSheetViewID;
    mnSheetViewTab = nSheetViewTab;

    if (mbRecord)
        mrDocShell.GetUndoManager()->AddUndoAction(
            std::make_unique<UndoInsertSheetView>(mrDocShell, mnTab, nSheetViewTab, nSheetViewID));

    mrDocShell.Broadcast(ScTablesHint(SC_TAB_INSERTED, nSheetViewTab));
    mrDocShell.PostPaintExtras();
    aModificator.SetDocumentModified();
    SfxGetpApp()->Broadcast(SfxHint(SfxHintId::ScTablesChanged));

    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
