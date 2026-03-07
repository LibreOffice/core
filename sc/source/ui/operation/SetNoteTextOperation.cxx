/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/SetNoteTextOperation.hxx>

#include <docsh.hxx>
#include <editable.hxx>
#include <postit.hxx>

#include <osl/endian.h>
#include <tools/lineend.hxx>

namespace sc
{
SetNoteTextOperation::SetNoteTextOperation(ScDocShell& rDocShell, const ScAddress& rPos,
                                           const OUString& rText, bool bApi)
    : Operation(OperationType::SetNoteText, false, bApi)
    , mrDocShell(rDocShell)
    , maPos(rPos)
    , maText(rText)
{
}

bool SetNoteTextOperation::runImplementation()
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

    OUString aNewText = convertLineEnd(maText, GetSystemLineEnd()); //! is this necessary ???

    if (ScPostIt* pNote = (!aNewText.isEmpty()) ? rDoc.GetOrCreateNote(aPos) : rDoc.GetNote(aPos))
        pNote->SetText(aPos, aNewText);

    //! Undo !!!

    rDoc.SetStreamValid(aPos.Tab(), false);

    syncSheetViews();

    mrDocShell.PostPaintCell(aPos);
    aModificator.SetDocumentModified();

    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
