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

    if (!checkSheetViewProtection())
        return false;

    ScEditableTester aTester = ScEditableTester::CreateAndTestBlock(
        rDoc, maPos.Tab(), maPos.Col(), maPos.Row(), maPos.Col(), maPos.Row());
    if (!aTester.IsEditable())
    {
        if (!mbApi)
            mrDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    OUString aNewText = convertLineEnd(maText, GetSystemLineEnd()); //! is this necessary ???

    if (ScPostIt* pNote = (!aNewText.isEmpty()) ? rDoc.GetOrCreateNote(maPos) : rDoc.GetNote(maPos))
        pNote->SetText(maPos, aNewText);

    //! Undo !!!

    rDoc.SetStreamValid(maPos.Tab(), false);

    mrDocShell.PostPaintCell(maPos);
    aModificator.SetDocumentModified();

    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
