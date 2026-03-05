/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/FillSimpleOperation.hxx>

#include <docfunc.hxx>
#include <docsh.hxx>
#include <editable.hxx>
#include <globstr.hrc>
#include <markdata.hxx>
#include <progress.hxx>
#include <scresid.hxx>
#include <undoblk.hxx>

#include <vcl/weld.hxx>

#include <memory>

namespace
{
void adjustFillRangeForAdjacentCopy(const ScDocument& rDoc, ScRange& rRange, FillDir eDir)
{
    switch (eDir)
    {
        case FILL_TO_BOTTOM:
        {
            if (rRange.aStart.Row() == 0)
                return;

            if (rRange.aStart.Row() != rRange.aEnd.Row())
                return;

            // Include the above row.
            ScAddress& s = rRange.aStart;
            s.SetRow(s.Row() - 1);
        }
        break;
        case FILL_TO_TOP:
        {
            if (rRange.aStart.Row() == rDoc.MaxRow())
                return;

            if (rRange.aStart.Row() != rRange.aEnd.Row())
                return;

            // Include the row below.
            ScAddress& e = rRange.aEnd;
            e.SetRow(e.Row() + 1);
        }
        break;
        case FILL_TO_LEFT:
        {
            if (rRange.aStart.Col() == rDoc.MaxCol())
                return;

            if (rRange.aStart.Col() != rRange.aEnd.Col())
                return;

            // Include the column to the right.
            ScAddress& e = rRange.aEnd;
            e.SetCol(e.Col() + 1);
        }
        break;
        case FILL_TO_RIGHT:
        {
            if (rRange.aStart.Col() == 0)
                return;

            if (rRange.aStart.Col() != rRange.aEnd.Col())
                return;

            // Include the column to the left.
            ScAddress& s = rRange.aStart;
            s.SetCol(s.Col() - 1);
        }
        break;
        default:;
    }
}
}

namespace sc
{
FillSimpleOperation::FillSimpleOperation(ScDocFunc& rDocFunc, ScDocShell& rDocShell,
                                         const ScRange& rRange, const ScMarkData* pTabMark,
                                         FillDir eDir, bool bApi)
    : Operation(OperationType::FillSimple, true, bApi)
    , mrDocFunc(rDocFunc)
    , mrDocShell(rDocShell)
    , maRange(rRange)
    , mpTabMark(pTabMark)
    , meDir(eDir)
{
}

bool FillSimpleOperation::runImplementation()
{
    ScDocShellModificator aModificator(mrDocShell);
    ScDocument& rDoc = mrDocShell.GetDocument();

    bool bSuccess = false;
    ScRange aRange = maRange;
    adjustFillRangeForAdjacentCopy(rDoc, aRange, meDir);

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

    if (!checkSheetViewProtection())
        return false;

    ScEditableTester aTester = ScEditableTester::CreateAndTestSelectedBlock(
        rDoc, nStartCol, nStartRow, nEndCol, nEndRow, aMark);
    if (aTester.IsEditable())
    {
        weld::WaitObject aWait(ScDocShell::GetActiveDialogParent());

        ScRange aSourceArea = aRange;
        ScRange aDestArea = aRange;

        SCCOLROW nCount = 0;
        switch (meDir)
        {
            case FILL_TO_BOTTOM:
                nCount = aSourceArea.aEnd.Row() - aSourceArea.aStart.Row();
                aSourceArea.aEnd.SetRow(aSourceArea.aStart.Row());
                break;
            case FILL_TO_RIGHT:
                nCount = aSourceArea.aEnd.Col() - aSourceArea.aStart.Col();
                aSourceArea.aEnd.SetCol(aSourceArea.aStart.Col());
                break;
            case FILL_TO_TOP:
                nCount = aSourceArea.aEnd.Row() - aSourceArea.aStart.Row();
                aSourceArea.aStart.SetRow(aSourceArea.aEnd.Row());
                break;
            case FILL_TO_LEFT:
                nCount = aSourceArea.aEnd.Col() - aSourceArea.aStart.Col();
                aSourceArea.aStart.SetCol(aSourceArea.aEnd.Col());
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

            ScRange aCopyRange = aDestArea;
            aCopyRange.aStart.SetTab(0);
            aCopyRange.aEnd.SetTab(nTabCount - 1);
            rDoc.CopyToDocument(aCopyRange, InsertDeleteFlags::AUTOFILL, false, *pUndoDoc, &aMark);
        }

        sal_uLong nProgCount;
        if (meDir == FILL_TO_BOTTOM || meDir == FILL_TO_TOP)
            nProgCount = aSourceArea.aEnd.Col() - aSourceArea.aStart.Col() + 1;
        else
            nProgCount = aSourceArea.aEnd.Row() - aSourceArea.aStart.Row() + 1;
        nProgCount *= nCount;
        ScProgress aProgress(rDoc.GetDocumentShell(), ScResId(STR_FILL_SERIES_PROGRESS), nProgCount,
                             true);

        rDoc.Fill(aSourceArea.aStart.Col(), aSourceArea.aStart.Row(), aSourceArea.aEnd.Col(),
                  aSourceArea.aEnd.Row(), &aProgress, aMark, nCount, meDir, FILL_SIMPLE);
        mrDocFunc.AdjustRowHeight(aRange, true, mbApi);

        if (mbRecord) // only now is Draw-Undo available
        {
            mrDocShell.GetUndoManager()->AddUndoAction(std::make_unique<ScUndoAutoFill>(
                &mrDocShell, aDestArea, aSourceArea, std::move(pUndoDoc), aMark, meDir, FILL_SIMPLE,
                FILL_DAY, MAXDOUBLE, 1.0, 1e307));
        }

        mrDocShell.PostPaintGridAll();
        aModificator.SetDocumentModified();

        bSuccess = true;
    }
    else if (!mbApi)
        mrDocShell.ErrorMessage(aTester.GetMessageId());

    return bSuccess;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
