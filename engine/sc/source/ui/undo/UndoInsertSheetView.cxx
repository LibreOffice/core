/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <undo/UndoInsertSheetView.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <document.hxx>
#include <docsh.hxx>
#include <operation/InsertSheetViewOperation.hxx>
#include <SheetView.hxx>
#include <SheetViewManager.hxx>
#include <tabvwsh.hxx>
#include <undoolk.hxx>

namespace sc
{
UndoInsertSheetView::UndoInsertSheetView(ScDocShell& rNewDocShell, SCTAB nTab, SCTAB nSheetViewTab,
                                         SheetViewID nSheetViewID)
    : ScSimpleUndo(rNewDocShell)
    , mnTab(nTab)
    , mnSheetViewTab(nSheetViewTab)
    , mnSheetViewID(nSheetViewID)
{
    // Snapshot the sheet view's identity now. The SheetView is destroyed
    // when this insert is undone, so its name and GUIDs cannot be read
    // later.
    auto pManager = rNewDocShell.GetDocument().GetSheetViewManager(mnTab);
    if (pManager)
    {
        auto pSheetView = pManager->get(mnSheetViewID);
        if (pSheetView)
        {
            maName = pSheetView->GetName();
            maGUID = pSheetView->GetGUID();
            maFilterGUID = pSheetView->GetFilterGUID();
        }
    }
    mpDrawUndo = GetSdrUndoAction(&rNewDocShell.GetDocument());
}

UndoInsertSheetView::~UndoInsertSheetView() = default;

OUString UndoInsertSheetView::GetComment() const { return ScResId(STR_UNDO_INSERT_SHEET_VIEW); }

void UndoInsertSheetView::Undo()
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (!pViewShell)
        return;

    // Switch back to the default tab first
    pViewShell->SetTabNo(mnTab);

    rDocShell.SetInUndo(true);
    bDrawIsInUndo = true;
    pViewShell->DeleteTable(mnSheetViewTab, false);
    bDrawIsInUndo = false;
    rDocShell.SetInUndo(false);

    DoSdrUndoAction(mpDrawUndo.get(), &rDocShell.GetDocument());

    rDocShell.Broadcast(SfxHint(SfxHintId::ScForceSetTab));
}

void UndoInsertSheetView::Redo()
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (!pViewShell)
        return;

    RedoSdrUndoAction(mpDrawUndo.get());

    rDocShell.SetInUndo(true);
    bDrawIsInUndo = true;

    InsertSheetViewOperation aOperation(rDocShell, mnTab, false);
    aOperation.setRestoreIdentity(mnSheetViewID, maName, maGUID, maFilterGUID);
    if (aOperation.run())
    {
        mnSheetViewID = aOperation.getSheetViewID();
        mnSheetViewTab = aOperation.getSheetViewTab();
        pViewShell->SetTabNo(mnSheetViewTab);
    }

    bDrawIsInUndo = false;
    rDocShell.SetInUndo(false);
}

void UndoInsertSheetView::Repeat(SfxRepeatTarget& /*rTarget*/) {}

bool UndoInsertSheetView::CanRepeat(SfxRepeatTarget& /*rTarget*/) const { return false; }

} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
