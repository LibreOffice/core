/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/FillAutoOperation.hxx>

#include <docfunc.hxx>
#include <docsh.hxx>
#include <editable.hxx>
#include <globstr.hrc>
#include <markdata.hxx>
#include <progress.hxx>
#include <scresid.hxx>
#include <undoblk.hxx>
#include <viewdata.hxx>

#include <osl/diagnose.h>
#include <vcl/weld.hxx>

#include <memory>

namespace sc
{
FillAutoOperation::FillAutoOperation(ScDocFunc& rDocFunc, ScDocShell& rDocShell, ScRange& rRange,
                                     const ScMarkData* pTabMark, FillDir eDir, FillCmd eCmd,
                                     FillDateCmd eDateCmd, sal_uLong nCount, double fStep,
                                     double fMax, bool bRecord, bool bApi)
    : Operation(OperationType::FillAuto, bRecord, bApi)
    , mrDocFunc(rDocFunc)
    , mrDocShell(rDocShell)
    , mrRange(rRange)
    , mpTabMark(pTabMark)
    , meDir(eDir)
    , meCmd(eCmd)
    , meDateCmd(eDateCmd)
    , mnCount(nCount)
    , mfStep(fStep)
    , mfMax(fMax)
{
}

bool FillAutoOperation::canRunTheOperation() const
{
    return !isInputOnSheetViewAutoFilter(mrRange);
}

bool FillAutoOperation::runImplementation()
{
    ScDocShellModificator aModificator(mrDocShell);

    ScDocument& rDoc = mrDocShell.GetDocument();

    ScRange aRange = convertRange(mrRange);
    SCCOL nStartCol = aRange.aStart.Col();
    SCROW nStartRow = aRange.aStart.Row();
    SCTAB nStartTab = aRange.aStart.Tab();
    SCCOL nEndCol = aRange.aEnd.Col();
    SCROW nEndRow = aRange.aEnd.Row();
    SCTAB nEndTab = aRange.aEnd.Tab();

    if (mbRecord && !rDoc.IsUndoEnabled())
        mbRecord = false;

    ScMarkData aMark(rDoc.GetSheetLimits());
    if (mpTabMark)
        aMark = *mpTabMark;
    else
    {
        for (SCTAB nTab = nStartTab; nTab <= nEndTab; nTab++)
            aMark.SelectTable(nTab, true);
    }

    ScRange aSourceArea = aRange;
    ScRange aDestArea = aRange;

    switch (meDir)
    {
        case FILL_TO_BOTTOM:
            aDestArea.aEnd.SetRow(sal::static_int_cast<SCROW>(aSourceArea.aEnd.Row() + mnCount));
            break;
        case FILL_TO_TOP:
            if (mnCount > sal::static_int_cast<sal_uLong>(aSourceArea.aStart.Row()))
            {
                OSL_FAIL("FillAuto: Row < 0");
                mnCount = aSourceArea.aStart.Row();
            }
            aDestArea.aStart.SetRow(
                sal::static_int_cast<SCROW>(aSourceArea.aStart.Row() - mnCount));
            break;
        case FILL_TO_RIGHT:
            aDestArea.aEnd.SetCol(sal::static_int_cast<SCCOL>(aSourceArea.aEnd.Col() + mnCount));
            break;
        case FILL_TO_LEFT:
            if (mnCount > sal::static_int_cast<sal_uLong>(aSourceArea.aStart.Col()))
            {
                OSL_FAIL("FillAuto: Col < 0");
                mnCount = aSourceArea.aStart.Col();
            }
            aDestArea.aStart.SetCol(
                sal::static_int_cast<SCCOL>(aSourceArea.aStart.Col() - mnCount));
            break;
        default:
            OSL_FAIL("Wrong direction with FillAuto");
            break;
    }

    //      Test for cell protection
    //!     Source range can be protected !!!
    //!     but can't contain matrix fragments !!!

    ScEditableTester aTester
        = ScEditableTester::CreateAndTestRange(rDoc, aDestArea, sc::EditAction::Unknown);
    if (!aTester.IsEditable())
    {
        if (!mbApi)
            mrDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    if (rDoc.HasSelectedBlockMatrixFragment(nStartCol, nStartRow, nEndCol, nEndRow, aMark))
    {
        if (!mbApi)
            mrDocShell.ErrorMessage(STR_MATRIXFRAGMENTERR);
        return false;
    }

    // FID_FILL_... slots should already had been disabled, check here for API
    // calls, no message.
    if (ScViewData::SelectionFillDOOM(aDestArea))
        return false;

    weld::WaitObject aWait(ScDocShell::GetActiveDialogParent());

    ScDocumentUniquePtr pUndoDoc;
    if (mbRecord)
    {
        SCTAB nTabCount = rDoc.GetTableCount();
        SCTAB nDestStartTab = aDestArea.aStart.Tab();

        pUndoDoc.reset(new ScDocument(SCDOCMODE_UNDO));
        pUndoDoc->InitUndo(rDoc, nDestStartTab, nDestStartTab);
        for (const auto& rTab : aMark)
        {
            if (rTab >= nTabCount)
                break;

            if (rTab != nDestStartTab)
                pUndoDoc->AddUndoTab(rTab, rTab);
        }

        // do not clone note captions in undo document
        rDoc.CopyToDocument(aDestArea.aStart.Col(), aDestArea.aStart.Row(), 0, aDestArea.aEnd.Col(),
                            aDestArea.aEnd.Row(), nTabCount - 1, InsertDeleteFlags::AUTOFILL, false,
                            *pUndoDoc, &aMark);
    }

    sal_uLong nProgCount;
    if (meDir == FILL_TO_BOTTOM || meDir == FILL_TO_TOP)
        nProgCount = aSourceArea.aEnd.Col() - aSourceArea.aStart.Col() + 1;
    else
        nProgCount = aSourceArea.aEnd.Row() - aSourceArea.aStart.Row() + 1;
    nProgCount *= mnCount;
    ScProgress aProgress(rDoc.GetDocumentShell(), ScResId(STR_FILL_SERIES_PROGRESS), nProgCount,
                         true);

    rDoc.Fill(aSourceArea.aStart.Col(), aSourceArea.aStart.Row(), aSourceArea.aEnd.Col(),
              aSourceArea.aEnd.Row(), &aProgress, aMark, mnCount, meDir, meCmd, meDateCmd, mfStep,
              mfMax);

    mrDocFunc.AdjustRowHeight(aDestArea, true, mbApi);

    syncSheetViews();

    if (mbRecord) // only now is Draw-Undo available
    {
        mrDocShell.GetUndoManager()->AddUndoAction(std::make_unique<ScUndoAutoFill>(
            &mrDocShell, aDestArea, aSourceArea, std::move(pUndoDoc), aMark, meDir, meCmd,
            meDateCmd, MAXDOUBLE, mfStep, mfMax));
    }

    mrDocShell.PostPaintGridAll();
    aModificator.SetDocumentModified();

    mrRange = aDestArea; // return destination range (for marking)
    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
