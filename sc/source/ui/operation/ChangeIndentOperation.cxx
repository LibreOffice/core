/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/ChangeIndentOperation.hxx>

#include <docsh.hxx>
#include <editable.hxx>
#include <markdata.hxx>
#include <undoblk.hxx>

#include <editeng/editids.hrc>
#include <sc.hrc>
#include <sfx2/bindings.hxx>
#include <svx/svxids.hrc>

#include <memory>

namespace sc
{
ChangeIndentOperation::ChangeIndentOperation(ScDocShell& rDocShell, const ScMarkData& rMark,
                                             bool bIncrement, bool bApi)
    : Operation(OperationType::ChangeIndent, false, bApi)
    , mrDocShell(rDocShell)
    , mrMark(rMark)
    , mbIncrement(bIncrement)
{
}

bool ChangeIndentOperation::runImplementation()
{
    ScDocShellModificator aModificator(mrDocShell);

    ScDocument& rDoc = mrDocShell.GetDocument();
    bool bUndo(rDoc.IsUndoEnabled());

    ScMarkData aMark = convertMark(mrMark);

    ScEditableTester aTester = ScEditableTester::CreateAndTestSelection(rDoc, aMark);
    if (!aTester.IsEditable())
    {
        if (!mbApi)
            mrDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    const ScRange& aMarkRange = aMark.GetMultiMarkArea();

    if (bUndo)
    {
        SCTAB nStartTab = aMarkRange.aStart.Tab();
        SCTAB nTabCount = rDoc.GetTableCount();

        ScDocumentUniquePtr pUndoDoc(new ScDocument(SCDOCMODE_UNDO));
        pUndoDoc->InitUndo(rDoc, nStartTab, nStartTab);
        for (const auto& rTab : aMark)
        {
            if (rTab >= nTabCount)
                break;

            if (rTab != nStartTab)
                pUndoDoc->AddUndoTab(rTab, rTab);
        }

        ScRange aCopyRange = aMarkRange;
        aCopyRange.aStart.SetTab(0);
        aCopyRange.aEnd.SetTab(nTabCount - 1);
        rDoc.CopyToDocument(aCopyRange, InsertDeleteFlags::ATTRIB, true, *pUndoDoc, &aMark);

        mrDocShell.GetUndoManager()->AddUndoAction(
            std::make_unique<ScUndoIndent>(&mrDocShell, aMark, std::move(pUndoDoc), mbIncrement));
    }

    rDoc.ChangeSelectionIndent(mbIncrement, aMark);

    syncSheetViews();

    mrDocShell.PostPaint(aMarkRange, PaintPartFlags::Grid, SC_PF_LINES | SC_PF_TESTMERGE);
    aModificator.SetDocumentModified();

    SfxBindings* pBindings = mrDocShell.GetViewBindings();
    if (pBindings)
    {
        pBindings->Invalidate(SID_ALIGNLEFT);
        pBindings->Invalidate(SID_ALIGNRIGHT);
        pBindings->Invalidate(SID_ALIGNBLOCK);
        pBindings->Invalidate(SID_ALIGNCENTERHOR);
        pBindings->Invalidate(SID_ATTR_LRSPACE);
        pBindings->Invalidate(SID_ATTR_PARA_ADJUST_LEFT);
        pBindings->Invalidate(SID_ATTR_PARA_ADJUST_RIGHT);
        pBindings->Invalidate(SID_ATTR_PARA_ADJUST_BLOCK);
        pBindings->Invalidate(SID_ATTR_PARA_ADJUST_CENTER);
        // pseudo slots for Format menu
        pBindings->Invalidate(SID_ALIGN_ANY_HDEFAULT);
        pBindings->Invalidate(SID_ALIGN_ANY_LEFT);
        pBindings->Invalidate(SID_ALIGN_ANY_HCENTER);
        pBindings->Invalidate(SID_ALIGN_ANY_RIGHT);
        pBindings->Invalidate(SID_ALIGN_ANY_JUSTIFIED);
    }

    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
