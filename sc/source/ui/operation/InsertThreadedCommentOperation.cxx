/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the LibreOffice contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/InsertThreadedCommentOperation.hxx>

#include <rtl/digest.h>
#include <rtl/string.hxx>
#include <svx/svdocapt.hxx>
#include <tools/Guid.hxx>
#include <tools/datetime.hxx>
#include <unotools/datetime.hxx>
#include <unotools/useroptions.hxx>

#include <docsh.hxx>
#include <document.hxx>
#include <drwlayer.hxx>
#include <editable.hxx>
#include <postit.hxx>
#include <scresid.hxx>
#include <strings.hrc>
#include <undocell.hxx>

namespace sc
{
InsertThreadedCommentOperation::InsertThreadedCommentOperation(ScDocShell& rDocShell,
                                                               const ScAddress& rPos,
                                                               const OUString& rText,
                                                               const OUString* pAuthor, bool bApi)
    : Operation(OperationType::InsertThreadedComment, false, bApi)
    , mrDocShell(rDocShell)
    , maPos(rPos)
    , maText(rText)
    , moAuthor(pAuthor ? std::optional<OUString>(*pAuthor) : std::nullopt)
{
}

bool InsertThreadedCommentOperation::runImplementation()
{
    ScDocShellModificator aModificator(mrDocShell);
    ScDocument& rDoc = mrDocShell.GetDocument();

    ScAddress aPos = convertAddress(maPos);

    ScEditableTester aTester = ScEditableTester::CreateAndTestBlock(
        rDoc, aPos.Tab(), aPos.Col(), aPos.Row(), aPos.Col(), aPos.Row());
    if (!aTester.IsEditable())
    {
        if (!mbApi)
            mrDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    SfxUndoManager* pUndoMgr
        = (pDrawLayer && rDoc.IsUndoEnabled()) ? mrDocShell.GetUndoManager() : nullptr;

    // Rescue the existing note (if any) for undo. Threaded data lives on ScPostIt
    // outside ScNoteData, so capture it separately.
    ScNoteData aOldData;
    std::unique_ptr<ScThreadedCommentData> pOldThreadedData;
    std::unique_ptr<ScPostIt> pOldNote = rDoc.ReleaseNote(aPos);
    sal_uInt32 nNoteId = 0;
    if (pOldNote)
    {
        nNoteId = pOldNote->GetId();
        // ensure existing caption object before draw undo tracking starts
        pOldNote->GetOrCreateCaption(aPos);
        aOldData = pOldNote->GetNoteData();
        if (const ScThreadedCommentData* pOldThreaded = pOldNote->GetThreadedCommentData())
            pOldThreadedData = std::make_unique<ScThreadedCommentData>(*pOldThreaded);
    }

    if (pUndoMgr)
        pDrawLayer->BeginCalcUndo(false);

    // delete the old note (creates drawing undo action for the caption object)
    bool hadOldNote(pOldNote);
    pOldNote.reset();

    // create new note (creates drawing undo action for the new caption object)
    ScNoteData aNewData;
    std::unique_ptr<ScThreadedCommentData> pNewThreadedData;
    ScPostIt* pNewNote = ScNoteUtil::CreateNoteFromString(rDoc, aPos, maText, false, true, nNoteId);
    if (pNewNote)
    {
        OUString aAuthor = moAuthor.value_or(OUString());
        if (aAuthor.isEmpty())
            aAuthor = SvtUserOptions().GetFullName();
        if (aAuthor.isEmpty())
            aAuthor = ScResId(STR_CHG_UNKNOWN_AUTHOR);

        // Resolve the person entry; create one with a stable UUIDv5 derived from
        // the author name so the same user gets the same person-id across documents.
        OUString aPersonId;
        for (const auto& rPerson : rDoc.GetPersonList())
        {
            if (rPerson.maDisplayName == aAuthor)
            {
                aPersonId = rPerson.maId;
                break;
            }
        }
        if (aPersonId.isEmpty())
        {
            // Namespace string is wire-stable; changing it remaps every existing
            // document's person-ids and breaks author matching on reopen.
            OString aInput = "LibreOffice:ThreadedCommentPerson:"
                             + OUStringToOString(aAuthor, RTL_TEXTENCODING_UTF8);
            sal_uInt8 aDigest[RTL_DIGEST_LENGTH_SHA1];
            rtl_digest_SHA1(aInput.getStr(), aInput.getLength(), aDigest, RTL_DIGEST_LENGTH_SHA1);
            aDigest[6] = (aDigest[6] & 0x0F) | 0x50; // version 5
            aDigest[8] = (aDigest[8] & 0x3F) | 0x80; // variant RFC 4122
            tools::Guid aPersonGuid(aDigest);

            ScPersonData aPerson;
            aPerson.maId = aPersonGuid.getOUString();
            aPerson.maDisplayName = aAuthor;
            aPerson.maUserId = aAuthor;
            aPerson.maProviderId = u"None"_ustr;
            aPersonId = aPerson.maId;
            rDoc.AddPerson(aPerson);
        }

        auto pThreaded = std::make_unique<ScThreadedCommentData>();
        pThreaded->maRoot.maGuid = tools::Guid(tools::Guid::Generate).getOUString();
        pThreaded->maRoot.maDateTime = utl::toISO8601(DateTime(DateTime::SYSTEM).GetUNODateTime());
        pThreaded->maRoot.maPersonId = aPersonId;
        pThreaded->maRoot.maText = maText;

        pNewNote->SetAuthor(aAuthor);
        pNewNote->SetThreadedCommentData(std::move(pThreaded));

        aNewData = pNewNote->GetNoteData();
        if (const ScThreadedCommentData* pNewThreaded = pNewNote->GetThreadedCommentData())
            pNewThreadedData = std::make_unique<ScThreadedCommentData>(*pNewThreaded);
    }

    if (pUndoMgr && (aOldData.mxCaption || aNewData.mxCaption))
        pUndoMgr->AddUndoAction(std::make_unique<ScUndoReplaceNote>(
            mrDocShell, aPos, std::move(aOldData), std::move(aNewData), pDrawLayer->GetCalcUndo(),
            std::move(pOldThreadedData), std::move(pNewThreadedData)));

    mrDocShell.PostPaintCell(aPos);

    rDoc.SetStreamValid(aPos.Tab(), false);

    syncSheetViews();

    aModificator.SetDocumentModified();

    if (pNewNote)
    {
        ScDocShell::LOKCommentNotify(hadOldNote ? LOKCommentNotificationType::Modify
                                                : LOKCommentNotificationType::Add,
                                     rDoc, aPos, pNewNote);
    }

    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
