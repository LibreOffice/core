/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/ShowNoteOperation.hxx>

#include <docsh.hxx>
#include <document.hxx>
#include <drawview.hxx>
#include <postit.hxx>
#include <tabview.hxx>
#include <undocell.hxx>
#include <viewdata.hxx>

#include <comphelper/kit.hxx>
#include <svx/svdocapt.hxx>

#include <memory>

namespace sc
{
ShowNoteOperation::ShowNoteOperation(ScDocShell& rDocShell, const ScAddress& rPos, bool bShow)
    : Operation(OperationType::ShowNote, true, false)
    , mrDocShell(rDocShell)
    , maPos(rPos)
    , mbShow(bShow)
{
}

bool ShowNoteOperation::runImplementation()
{
    ScDocument& rDoc = mrDocShell.GetDocument();
    ScPostIt* pNote = rDoc.GetNote(maPos);
    if (!pNote || (mbShow == pNote->IsCaptionShown()) || comphelper::COKit::isActive())
        return false;

    // move the caption to internal or hidden layer and create undo action
    pNote->ShowCaption(maPos, mbShow);
    if (rDoc.IsUndoEnabled())
        mrDocShell.GetUndoManager()->AddUndoAction(
            std::make_unique<ScUndoShowHideNote>(mrDocShell, maPos, mbShow));

    rDoc.SetStreamValid(maPos.Tab(), false);

    ScTabView::OnKitNoteStateChanged(pNote);

    if (ScViewData* pViewData = ScDocShell::GetViewData())
    {
        if (ScDrawView* pDrawView = pViewData->GetScDrawView())
            pDrawView->SyncForGrid(pNote->GetCaption());
    }

    mrDocShell.SetDocumentModified();

    return true;
}
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
