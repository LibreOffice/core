/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/AutoFormatOperation.hxx>

#include <autoform.hxx>
#include <columnspanset.hxx>
#include <docfunc.hxx>
#include <docsh.hxx>
#include <editable.hxx>
#include <globstr.hrc>
#include <markdata.hxx>
#include <scresid.hxx>
#include <undoblk.hxx>

#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

#include <memory>

#define AUTOFORMAT_WARN_SIZE 0x10ffffUL

namespace sc
{
AutoFormatOperation::AutoFormatOperation(ScDocFunc& rDocFunc, ScDocShell& rDocShell,
                                         const ScRange& rRange, const ScMarkData* pTabMark,
                                         sal_uInt16 nFormatNo, bool bApi)
    : Operation(OperationType::AutoFormat, true, bApi)
    , mrDocFunc(rDocFunc)
    , mrDocShell(rDocShell)
    , maRange(rRange)
    , mpTabMark(pTabMark)
    , mnFormatNo(nFormatNo)
{
}

bool AutoFormatOperation::canRunTheOperation() const
{
    return !isInputOnSheetViewAutoFilter(maRange);
}

bool AutoFormatOperation::runImplementation()
{
    ScDocShellModificator aModificator(mrDocShell);

    ScDocument& rDoc = mrDocShell.GetDocument();

    ScRange aRange = convertRange(maRange);
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

    ScAutoFormat* pAutoFormat = ScGlobal::GetOrCreateAutoFormat();

    ScEditableTester aTester = ScEditableTester::CreateAndTestSelectedBlock(
        rDoc, nStartCol, nStartRow, nEndCol, nEndRow, aMark);
    if (mnFormatNo < pAutoFormat->size() && aTester.IsEditable())
    {
        weld::WaitObject aWait(ScDocShell::GetActiveDialogParent());

        bool bSize = pAutoFormat->findByIndex(mnFormatNo)->GetIncludeWidthHeight();
        if (sal_uInt64(nEndCol - nStartCol + 1) * sal_uInt64(nEndRow - nStartRow + 1)
            > AUTOFORMAT_WARN_SIZE)
        {
            std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(
                ScDocShell::GetActiveDialogParent(), VclMessageType::Warning, VclButtonsType::YesNo,
                ScResId(STR_AUTOFORMAT_WAIT_WARNING)));
            xQueryBox->set_default_response(RET_NO);
            if (xQueryBox->run() != RET_YES)
                return false;
        }

        SCTAB nTabCount = rDoc.GetTableCount();
        ScDocumentUniquePtr pUndoDoc;
        if (mbRecord)
        {
            pUndoDoc.reset(new ScDocument(SCDOCMODE_UNDO));
            pUndoDoc->InitUndo(rDoc, nStartTab, nStartTab, bSize, bSize);
            for (const auto& rTab : aMark)
            {
                if (rTab >= nTabCount)
                    break;

                if (rTab != nStartTab)
                    pUndoDoc->AddUndoTab(rTab, rTab, bSize, bSize);
            }

            ScRange aCopyRange = aRange;
            aCopyRange.aStart.SetTab(0);
            aCopyRange.aStart.SetTab(nTabCount - 1);
            rDoc.CopyToDocument(aCopyRange, InsertDeleteFlags::ATTRIB, false, *pUndoDoc, &aMark);
            if (bSize)
            {
                rDoc.CopyToDocument(nStartCol, 0, 0, nEndCol, rDoc.MaxRow(), nTabCount - 1,
                                    InsertDeleteFlags::NONE, false, *pUndoDoc, &aMark);
                rDoc.CopyToDocument(0, nStartRow, 0, rDoc.MaxCol(), nEndRow, nTabCount - 1,
                                    InsertDeleteFlags::NONE, false, *pUndoDoc, &aMark);
            }
            rDoc.BeginDrawUndo();
        }

        rDoc.AutoFormat(nStartCol, nStartRow, nEndCol, nEndRow, mnFormatNo, aMark);

        if (bSize)
        {
            std::vector<sc::ColRowSpan> aCols(1, sc::ColRowSpan(nStartCol, nEndCol));
            std::vector<sc::ColRowSpan> aRows(1, sc::ColRowSpan(nStartRow, nEndRow));

            for (const auto& rTab : aMark)
            {
                if (rTab >= nTabCount)
                    break;

                mrDocFunc.SetWidthOrHeight(true, aCols, rTab, SC_SIZE_VISOPT, STD_EXTRA_WIDTH,
                                           false, true);
                mrDocFunc.SetWidthOrHeight(false, aRows, rTab, SC_SIZE_VISOPT, 0, false, false);
                mrDocShell.PostPaint(0, 0, rTab, rDoc.MaxCol(), rDoc.MaxRow(), rTab,
                                     PaintPartFlags::Grid | PaintPartFlags::Left
                                         | PaintPartFlags::Top);
            }
        }
        else
        {
            for (const auto& rTab : aMark)
            {
                if (rTab >= nTabCount)
                    break;

                bool bAdj = mrDocFunc.AdjustRowHeight(
                    ScRange(nStartCol, nStartRow, rTab, nEndCol, nEndRow, rTab), false, mbApi);
                if (bAdj)
                    mrDocShell.PostPaint(0, nStartRow, rTab, rDoc.MaxCol(), rDoc.MaxRow(), rTab,
                                         PaintPartFlags::Grid | PaintPartFlags::Left);
                else
                    mrDocShell.PostPaint(nStartCol, nStartRow, rTab, nEndCol, nEndRow, rTab,
                                         PaintPartFlags::Grid);
            }
        }

        if (mbRecord) // only now is Draw-Undo available
        {
            mrDocShell.GetUndoManager()->AddUndoAction(std::make_unique<ScUndoAutoFormat>(
                &mrDocShell, aRange, std::move(pUndoDoc), aMark, bSize, mnFormatNo));
        }

        syncSheetViews();

        aModificator.SetDocumentModified();
        return true;
    }
    else if (!mbApi)
        mrDocShell.ErrorMessage(aTester.GetMessageId());

    return false;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
