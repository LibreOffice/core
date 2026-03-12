/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/ReplaceNoteTextOperation.hxx>

#include <svx/svdocapt.hxx>

#include <docsh.hxx>
#include <drwlayer.hxx>
#include <editable.hxx>
#include <postit.hxx>
#include <undocell.hxx>

namespace sc
{
ReplaceNoteTextOperation::ReplaceNoteTextOperation(ScDocShell& rDocShell, const ScAddress& rPos,
                                                   const OUString& rNoteText,
                                                   const OUString* pAuthor, const OUString* pDate,
                                                   bool bApi)
    : Operation(OperationType::ReplaceNoteText, false, bApi)
    , mrDocShell(rDocShell)
    , maPos(rPos)
    , maText(rNoteText)
    , moAuthor(pAuthor ? std::optional<OUString>(*pAuthor) : std::nullopt)
    , moDate(pDate ? std::optional<OUString>(*pDate) : std::nullopt)
{
}

bool ReplaceNoteTextOperation::runImplementation()
{
    ScDocShellModificator aModificator(mrDocShell);
    ScDocument& rDoc = mrDocShell.GetDocument();

    ScAddress aPos = convertAddress(maPos);

    ScEditableTester aTester = ScEditableTester::CreateAndTestBlock(
        rDoc, aPos.Tab(), aPos.Col(), aPos.Row(), aPos.Col(), aPos.Row());
    if (aTester.IsEditable())
    {
        ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
        SfxUndoManager* pUndoMgr
            = (pDrawLayer && rDoc.IsUndoEnabled()) ? mrDocShell.GetUndoManager() : nullptr;

        ScNoteData aOldData;
        std::unique_ptr<ScPostIt> pOldNote = rDoc.ReleaseNote(aPos);
        sal_uInt32 nNoteId = 0;
        if (pOldNote)
        {
            nNoteId = pOldNote->GetId();
            // ensure existing caption object before draw undo tracking starts
            pOldNote->GetOrCreateCaption(aPos);
            // rescue note data for undo
            aOldData = pOldNote->GetNoteData();
        }

        // collect drawing undo actions for deleting/inserting caption objects
        if (pUndoMgr)
            pDrawLayer->BeginCalcUndo(false);

        // delete the note (creates drawing undo action for the caption object)
        bool hadOldNote(pOldNote);
        pOldNote.reset();

        // create new note (creates drawing undo action for the new caption object)
        ScNoteData aNewData;
        ScPostIt* pNewNote = nullptr;
        if ((pNewNote = ScNoteUtil::CreateNoteFromString(rDoc, aPos, maText, false, true, nNoteId)))
        {
            if (moAuthor)
                pNewNote->SetAuthor(*moAuthor);
            if (moDate)
                pNewNote->SetDate(*moDate);

            // rescue note data for undo
            aNewData = pNewNote->GetNoteData();
        }

        // create the undo action
        if (pUndoMgr && (aOldData.mxCaption || aNewData.mxCaption))
            pUndoMgr->AddUndoAction(std::make_unique<ScUndoReplaceNote>(
                mrDocShell, aPos, aOldData, aNewData, pDrawLayer->GetCalcUndo()));

        // repaint cell (to make note marker visible)
        mrDocShell.PostPaintCell(aPos);

        rDoc.SetStreamValid(aPos.Tab(), false);

        syncSheetViews();

        aModificator.SetDocumentModified();

        // Let our LOK clients know about the new/modified note
        if (pNewNote)
        {
            ScDocShell::LOKCommentNotify(hadOldNote ? LOKCommentNotificationType::Modify
                                                    : LOKCommentNotificationType::Add,
                                         rDoc, aPos, pNewNote);
        }

        return true;
    }
    else if (!mbApi)
    {
        mrDocShell.ErrorMessage(aTester.GetMessageId());
    }

    return false;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
