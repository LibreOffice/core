/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/FillSeriesOperation.hxx>

#include <docfunc.hxx>
#include <docsh.hxx>
#include <editable.hxx>
#include <globstr.hrc>
#include <markdata.hxx>
#include <progress.hxx>
#include <scresid.hxx>
#include <undoblk.hxx>

#include <vcl/weld.hxx>

#include <cassert>
#include <memory>

namespace
{
ScDirection DirFromFillDir(FillDir eDir)
{
    if (eDir == FILL_TO_BOTTOM)
        return DIR_BOTTOM;
    else if (eDir == FILL_TO_RIGHT)
        return DIR_RIGHT;
    else if (eDir == FILL_TO_TOP)
        return DIR_TOP;
    else // if (eDir==FILL_TO_LEFT)
        return DIR_LEFT;
}
}

namespace sc
{
FillSeriesOperation::FillSeriesOperation(ScDocFunc& rDocFunc, ScDocShell& rDocShell,
                                         const ScRange& rRange, const ScMarkData* pTabMark,
                                         FillDir eDir, FillCmd eCmd, FillDateCmd eDateCmd,
                                         double fStart, double fStep, double fMax, bool bApi)
    : Operation(OperationType::FillSeries, true, bApi)
    , mrDocFunc(rDocFunc)
    , mrDocShell(rDocShell)
    , maRange(rRange)
    , mpTabMark(pTabMark)
    , meDir(eDir)
    , meCmd(eCmd)
    , meDateCmd(eDateCmd)
    , mfStart(fStart)
    , mfStep(fStep)
    , mfMax(fMax)
{
}

bool FillSeriesOperation::runImplementation()
{
    ScDocShellModificator aModificator(mrDocShell);

    bool bSuccess = false;
    ScDocument& rDoc = mrDocShell.GetDocument();
    SCCOL nStartCol = maRange.aStart.Col();
    SCROW nStartRow = maRange.aStart.Row();
    SCTAB nStartTab = maRange.aStart.Tab();
    SCCOL nEndCol = maRange.aEnd.Col();
    SCROW nEndRow = maRange.aEnd.Row();
    SCTAB nEndTab = maRange.aEnd.Tab();

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

    if (!checkSheetViewProtection())
        return false;

    ScEditableTester aTester = ScEditableTester::CreateAndTestSelectedBlock(
        rDoc, nStartCol, nStartRow, nEndCol, nEndRow, aMark);
    if (aTester.IsEditable())
    {
        weld::WaitObject aWait(ScDocShell::GetActiveDialogParent());

        ScRange aSourceArea = maRange;
        ScRange aDestArea = maRange;

        SCSIZE nCount = rDoc.GetEmptyLinesInBlock(
            aSourceArea.aStart.Col(), aSourceArea.aStart.Row(), aSourceArea.aStart.Tab(),
            aSourceArea.aEnd.Col(), aSourceArea.aEnd.Row(), aSourceArea.aEnd.Tab(),
            DirFromFillDir(meDir));

        //  keep at least one row/column as source range
        SCSIZE nTotLines
            = (meDir == FILL_TO_BOTTOM || meDir == FILL_TO_TOP)
                  ? static_cast<SCSIZE>(aSourceArea.aEnd.Row() - aSourceArea.aStart.Row() + 1)
                  : static_cast<SCSIZE>(aSourceArea.aEnd.Col() - aSourceArea.aStart.Col() + 1);
        if (nCount >= nTotLines)
        {
            assert(nTotLines > 0 && "coverity 2023.12.2");
            nCount = nTotLines - 1;
        }

        switch (meDir)
        {
            case FILL_TO_BOTTOM:
                aSourceArea.aEnd.SetRow(
                    sal::static_int_cast<SCROW>(aSourceArea.aEnd.Row() - nCount));
                break;
            case FILL_TO_RIGHT:
                aSourceArea.aEnd.SetCol(
                    sal::static_int_cast<SCCOL>(aSourceArea.aEnd.Col() - nCount));
                break;
            case FILL_TO_TOP:
                aSourceArea.aStart.SetRow(
                    sal::static_int_cast<SCROW>(aSourceArea.aStart.Row() + nCount));
                break;
            case FILL_TO_LEFT:
                aSourceArea.aStart.SetCol(
                    sal::static_int_cast<SCCOL>(aSourceArea.aStart.Col() + nCount));
                break;
        }

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

            rDoc.CopyToDocument(aDestArea.aStart.Col(), aDestArea.aStart.Row(), 0,
                                aDestArea.aEnd.Col(), aDestArea.aEnd.Row(), nTabCount - 1,
                                InsertDeleteFlags::AUTOFILL, false, *pUndoDoc, &aMark);
        }

        if (aDestArea.aStart.Col() <= aDestArea.aEnd.Col()
            && aDestArea.aStart.Row() <= aDestArea.aEnd.Row())
        {
            if (mfStart != MAXDOUBLE)
            {
                SCCOL nValX
                    = (meDir == FILL_TO_LEFT) ? aDestArea.aEnd.Col() : aDestArea.aStart.Col();
                SCROW nValY
                    = (meDir == FILL_TO_TOP) ? aDestArea.aEnd.Row() : aDestArea.aStart.Row();
                SCTAB nTab = aDestArea.aStart.Tab();
                rDoc.SetValue(nValX, nValY, nTab, mfStart);
            }

            sal_uLong nProgCount;
            if (meDir == FILL_TO_BOTTOM || meDir == FILL_TO_TOP)
                nProgCount = aSourceArea.aEnd.Col() - aSourceArea.aStart.Col() + 1;
            else
                nProgCount = aSourceArea.aEnd.Row() - aSourceArea.aStart.Row() + 1;
            nProgCount *= nCount;
            ScProgress aProgress(rDoc.GetDocumentShell(), ScResId(STR_FILL_SERIES_PROGRESS),
                                 nProgCount, true);

            rDoc.Fill(aSourceArea.aStart.Col(), aSourceArea.aStart.Row(), aSourceArea.aEnd.Col(),
                      aSourceArea.aEnd.Row(), &aProgress, aMark, nCount, meDir, meCmd, meDateCmd,
                      mfStep, mfMax);
            mrDocFunc.AdjustRowHeight(maRange, true, mbApi);

            mrDocShell.PostPaintGridAll();
            aModificator.SetDocumentModified();
        }

        if (mbRecord) // only now is Draw-Undo available
        {
            mrDocShell.GetUndoManager()->AddUndoAction(std::make_unique<ScUndoAutoFill>(
                &mrDocShell, aDestArea, aSourceArea, std::move(pUndoDoc), aMark, meDir, meCmd,
                meDateCmd, mfStart, mfStep, mfMax));
        }

        bSuccess = true;
    }
    else if (!mbApi)
        mrDocShell.ErrorMessage(aTester.GetMessageId());

    return bSuccess;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
