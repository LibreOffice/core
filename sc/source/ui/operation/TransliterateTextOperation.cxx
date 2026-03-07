/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/TransliterateTextOperation.hxx>

#include <docfunc.hxx>
#include <docsh.hxx>
#include <editable.hxx>
#include <markdata.hxx>
#include <undoblk.hxx>

#include <memory>

namespace sc
{
TransliterateTextOperation::TransliterateTextOperation(ScDocFunc& rDocFunc, ScDocShell& rDocShell,
                                                       const ScMarkData& rMark,
                                                       TransliterationFlags nType, bool bApi)
    : Operation(OperationType::TransliterateText, true, bApi)
    , mrDocFunc(rDocFunc)
    , mrDocShell(rDocShell)
    , mrMark(rMark)
    , mnType(nType)
{
}

bool TransliterateTextOperation::runImplementation()
{
    ScDocShellModificator aModificator(mrDocShell);

    ScDocument& rDoc = mrDocShell.GetDocument();

    if (mbRecord && !rDoc.IsUndoEnabled())
        mbRecord = false;

    ScMarkData aMultiMark = convertMark(mrMark);
    aMultiMark.SetMarking(false); // for MarkToMulti
    aMultiMark.MarkToMulti();
    const ScRange& aMarkRange = aMultiMark.GetMultiMarkArea();

    ScEditableTester aTester = ScEditableTester::CreateAndTestSelection(rDoc, aMultiMark);
    if (!aTester.IsEditable())
    {
        if (!mbApi)
            mrDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    if (mbRecord)
    {
        SCTAB nStartTab = aMarkRange.aStart.Tab();
        SCTAB nTabCount = rDoc.GetTableCount();

        ScDocumentUniquePtr pUndoDoc(new ScDocument(SCDOCMODE_UNDO));
        pUndoDoc->InitUndo(rDoc, nStartTab, nStartTab);
        for (const auto& rTab : mrMark)
        {
            if (rTab >= nTabCount)
                break;

            if (rTab != nStartTab)
                pUndoDoc->AddUndoTab(rTab, rTab);
        }

        ScRange aCopyRange = aMarkRange;
        aCopyRange.aStart.SetTab(0);
        aCopyRange.aEnd.SetTab(nTabCount - 1);
        rDoc.CopyToDocument(aCopyRange, InsertDeleteFlags::CONTENTS, true, *pUndoDoc, &aMultiMark);

        mrDocShell.GetUndoManager()->AddUndoAction(std::make_unique<ScUndoTransliterate>(
            &mrDocShell, aMultiMark, std::move(pUndoDoc), mnType));
    }

    rDoc.TransliterateText(aMultiMark, mnType);

    syncSheetViews();

    if (!mrDocFunc.AdjustRowHeight(aMarkRange, true, true))
        mrDocShell.PostPaint(aMarkRange, PaintPartFlags::Grid);

    aModificator.SetDocumentModified();

    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
