/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/ApplyStyleOperation.hxx>

#include <docfunc.hxx>
#include <docsh.hxx>
#include <document.hxx>
#include <globstr.hrc>
#include <markdata.hxx>
#include <rangelst.hxx>
#include <stlpool.hxx>
#include <stlsheet.hxx>
#include <undoblk.hxx>

#include <memory>

namespace sc
{
ApplyStyleOperation::ApplyStyleOperation(ScDocFunc& rDocFunc, ScDocShell& rDocShell,
                                         ScMarkData const& rMark, OUString const& rStyleName,
                                         bool bApi)
    : Operation(OperationType::ApplyStyle, true, bApi)
    , mrDocFunc(rDocFunc)
    , mrDocShell(rDocShell)
    , mrMark(rMark)
    , mrStyleName(rStyleName)
{
}

bool ApplyStyleOperation::runImplementation()
{
    ScMarkData aMark = convertMark(mrMark);
    ScDocument& rDoc = mrDocShell.GetDocument();
    bool bRecord = true;
    if (!rDoc.IsUndoEnabled())
        bRecord = false;

    bool bImportingXML = rDoc.IsImportingXML();
    // Cell formats can still be set if the range isn't editable only because of matrix formulas.
    // #i62483# When loading XML, the check can be skipped altogether.
    bool bOnlyNotBecauseOfMatrix;
    if (!bImportingXML && !rDoc.IsSelectionEditable(aMark, &bOnlyNotBecauseOfMatrix)
        && !bOnlyNotBecauseOfMatrix)
    {
        if (!mbApi)
            mrDocShell.ErrorMessage(STR_PROTECTIONERR);
        return false;
    }

    ScStyleSheet* pStyleSheet = static_cast<ScStyleSheet*>(
        rDoc.GetStyleSheetPool()->Find(mrStyleName, SfxStyleFamily::Para));
    if (!pStyleSheet)
        return false;

    ScDocShellModificator aModificator(mrDocShell);

    ScRange aMultiRange;
    bool bMulti = aMark.IsMultiMarked();
    if (bMulti)
        aMultiRange = aMark.GetMultiMarkArea();
    else
        aMultiRange = aMark.GetMarkArea();

    if (bRecord)
    {
        ScDocumentUniquePtr pUndoDoc(new ScDocument(SCDOCMODE_UNDO));
        SCTAB nStartTab = aMultiRange.aStart.Tab();
        SCTAB nTabCount = rDoc.GetTableCount();
        pUndoDoc->InitUndo(rDoc, nStartTab, nStartTab);
        for (const auto& rTab : aMark)
        {
            if (rTab >= nTabCount)
                break;

            if (rTab != nStartTab)
                pUndoDoc->AddUndoTab(rTab, rTab);
        }

        ScRange aCopyRange = aMultiRange;
        aCopyRange.aStart.SetTab(0);
        aCopyRange.aEnd.SetTab(nTabCount - 1);
        rDoc.CopyToDocument(aCopyRange, InsertDeleteFlags::ATTRIB, bMulti, *pUndoDoc, &aMark);

        mrDocShell.GetUndoManager()->AddUndoAction(std::make_unique<ScUndoSelectionStyle>(
            mrDocShell, aMark, aMultiRange, mrStyleName, std::move(pUndoDoc)));
    }

    rDoc.ApplySelectionStyle(*pStyleSheet, aMark);

    syncSheetViews();

    if (!mrDocFunc.AdjustRowHeight(aMultiRange, true, mbApi))
        mrDocShell.PostPaint(aMultiRange, PaintPartFlags::Grid);

    aModificator.SetDocumentModified();

    return true;
}
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
