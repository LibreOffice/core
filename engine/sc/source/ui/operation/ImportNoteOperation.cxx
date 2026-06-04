/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/ImportNoteOperation.hxx>

#include <docsh.hxx>
#include <document.hxx>
#include <postit.hxx>

#include <sal/log.hxx>

#include <memory>

namespace sc
{
ImportNoteOperation::ImportNoteOperation(ScDocShell& rDocShell, const ScAddress& rPos,
                                         std::unique_ptr<GenerateNoteCaption> xGenerator,
                                         const tools::Rectangle& rCaptionRect, bool bShown)
    : Operation(OperationType::ImportNote, false, true)
    , mrDocShell(rDocShell)
    , maPos(rPos)
    , mxGenerator(std::move(xGenerator))
    , maCaptionRect(rCaptionRect)
    , mbShown(bShown)
{
}

bool ImportNoteOperation::runImplementation()
{
    ScDocShellModificator aModificator(mrDocShell);
    ScDocument& rDoc = mrDocShell.GetDocument();

    std::unique_ptr<ScPostIt> pOldNote = rDoc.ReleaseNote(maPos);
    SAL_WARN_IF(pOldNote, "sc.ui", "imported data has >1 notes on same cell? at pos " << maPos);

    // create new note
    ScNoteUtil::CreateNoteFromGenerator(rDoc, maPos, std::move(mxGenerator), maCaptionRect,
                                        mbShown);

    rDoc.SetStreamValid(maPos.Tab(), false);

    aModificator.SetDocumentModified();
    return true;
}
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
