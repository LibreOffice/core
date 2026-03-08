/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/InsertNameListOperation.hxx>

#include <docsh.hxx>
#include <docfunc.hxx>
#include <document.hxx>
#include <editable.hxx>
#include <rangenam.hxx>
#include <stringutil.hxx>
#include <undoblk.hxx>

#include <memory>

namespace sc
{
InsertNameListOperation::InsertNameListOperation(ScDocShell& rDocShell, const ScAddress& rStartPos,
                                                 bool bApi)
    : Operation(OperationType::InsertNameList, false, bApi)
    , mrDocShell(rDocShell)
    , maStartPos(rStartPos)
{
}

bool InsertNameListOperation::runImplementation()
{
    ScDocShellModificator aModificator(mrDocShell);

    ScAddress aStartPos = convertAddress(maStartPos);

    bool bDone = false;
    ScDocument& rDoc = mrDocShell.GetDocument();
    const bool bRecord = rDoc.IsUndoEnabled();
    SCTAB nTab = aStartPos.Tab();

    //local names have higher priority than global names
    ScRangeName* pLocalList = rDoc.GetRangeName(nTab);
    sal_uInt16 nValidCount = 0;
    for (const auto& rEntry : *pLocalList)
    {
        const ScRangeData& r = *rEntry.second;
        if (!r.HasType(ScRangeData::Type::Database))
            ++nValidCount;
    }
    ScRangeName* pList = rDoc.GetRangeName();
    for (const auto& rEntry : *pList)
    {
        const ScRangeData& r = *rEntry.second;
        if (!r.HasType(ScRangeData::Type::Database)
            && !pLocalList->findByUpperName(r.GetUpperName()))
            ++nValidCount;
    }

    if (nValidCount)
    {
        SCCOL nStartCol = aStartPos.Col();
        SCROW nStartRow = aStartPos.Row();
        SCCOL nEndCol = nStartCol + 1;
        SCROW nEndRow = nStartRow + static_cast<SCROW>(nValidCount) - 1;

        ScEditableTester aTester = ScEditableTester::CreateAndTestBlock(
            rDoc, nTab, nStartCol, nStartRow, nEndCol, nEndRow);
        if (aTester.IsEditable())
        {
            ScDocumentUniquePtr pUndoDoc;

            if (bRecord)
            {
                pUndoDoc.reset(new ScDocument(SCDOCMODE_UNDO));
                pUndoDoc->InitUndo(rDoc, nTab, nTab);
                rDoc.CopyToDocument(nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab,
                                    InsertDeleteFlags::ALL, false, *pUndoDoc);

                rDoc.BeginDrawUndo(); // because of adjusting heights
            }

            std::unique_ptr<ScRangeData* []> ppSortArray(new ScRangeData*[nValidCount]);
            sal_uInt16 j = 0;
            for (const auto& rEntry : *pLocalList)
            {
                ScRangeData& r = *rEntry.second;
                if (!r.HasType(ScRangeData::Type::Database))
                    ppSortArray[j++] = &r;
            }
            for (const auto & [ rName, rxData ] : *pList)
            {
                ScRangeData& r = *rxData;
                if (!r.HasType(ScRangeData::Type::Database) && !pLocalList->findByUpperName(rName))
                    ppSortArray[j++] = &r;
            }
            qsort(static_cast<void*>(ppSortArray.get()), nValidCount, sizeof(ScRangeData*),
                  &ScRangeData_QsortNameCompare);
            OUString aName;
            OUStringBuffer aContent;
            OUString aFormula;
            SCROW nOutRow = nStartRow;
            for (j = 0; j < nValidCount; j++)
            {
                ScRangeData* pData = ppSortArray[j];
                pData->GetName(aName);
                // adjust relative references to the left column in Excel-compliant way:
                pData->UpdateSymbol(aContent, ScAddress(nStartCol, nOutRow, nTab));
                aFormula = "=" + aContent;
                ScSetStringParam aParam;
                aParam.setTextInput();
                rDoc.SetString(ScAddress(nStartCol, nOutRow, nTab), aName, &aParam);
                rDoc.SetString(ScAddress(nEndCol, nOutRow, nTab), aFormula, &aParam);
                ++nOutRow;
            }

            ppSortArray.reset();

            if (bRecord)
            {
                ScDocumentUniquePtr pRedoDoc(new ScDocument(SCDOCMODE_UNDO));
                pRedoDoc->InitUndo(rDoc, nTab, nTab);
                rDoc.CopyToDocument(nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab,
                                    InsertDeleteFlags::ALL, false, *pRedoDoc);

                mrDocShell.GetUndoManager()->AddUndoAction(std::make_unique<ScUndoListNames>(
                    &mrDocShell, ScRange(nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab),
                    std::move(pUndoDoc), std::move(pRedoDoc)));
            }

            if (!mrDocShell.GetDocFunc().AdjustRowHeight(
                    ScRange(0, nStartRow, nTab, rDoc.MaxCol(), nEndRow, nTab), true, true))
                mrDocShell.PostPaint(nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab,
                                     PaintPartFlags::Grid);

            syncSheetViews();

            aModificator.SetDocumentModified();
            bDone = true;
        }
        else if (!mbApi)
            mrDocShell.ErrorMessage(aTester.GetMessageId());
    }
    return bDone;
}
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
