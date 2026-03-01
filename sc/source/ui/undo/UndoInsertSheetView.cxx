/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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
#include <tabvwsh.hxx>
#include <undoolk.hxx>

namespace sc
{
UndoInsertSheetView::UndoInsertSheetView(ScDocShell& rDocShell, SCTAB nTab, SCTAB nSheetViewTab,
                                         SheetViewID nSheetViewID)
    : ScSimpleUndo(&rDocShell)
    , mnTab(nTab)
    , mnSheetViewTab(nSheetViewTab)
    , mnSheetViewID(nSheetViewID)
{
    mpDrawUndo = GetSdrUndoAction(&pDocShell->GetDocument());
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

    pDocShell->SetInUndo(true);
    bDrawIsInUndo = true;
    pViewShell->DeleteTable(mnSheetViewTab, false);
    bDrawIsInUndo = false;
    pDocShell->SetInUndo(false);

    DoSdrUndoAction(mpDrawUndo.get(), &pDocShell->GetDocument());

    pDocShell->Broadcast(SfxHint(SfxHintId::ScForceSetTab));
}

void UndoInsertSheetView::Redo()
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (!pViewShell)
        return;

    RedoSdrUndoAction(mpDrawUndo.get());

    pDocShell->SetInUndo(true);
    bDrawIsInUndo = true;

    ScDocument& rDoc = pDocShell->GetDocument();
    auto[nNewID, nNewTab] = rDoc.CreateNewSheetView(mnTab);
    if (nNewID != InvalidSheetViewID)
    {
        mnSheetViewID = nNewID;
        mnSheetViewTab = nNewTab;
        pViewShell->SetTabNo(mnSheetViewTab);
    }

    bDrawIsInUndo = false;
    pDocShell->SetInUndo(false);
}

void UndoInsertSheetView::Repeat(SfxRepeatTarget& /*rTarget*/) {}

bool UndoInsertSheetView::CanRepeat(SfxRepeatTarget& /*rTarget*/) const { return false; }

} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
