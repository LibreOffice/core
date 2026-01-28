/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/SortOperation.hxx>

#include <docfuncutil.hxx>
#include <docfunc.hxx>
#include <editable.hxx>
#include <markdata.hxx>
#include <SheetViewOperationsTester.hxx>
#include <editable.hxx>
#include <progress.hxx>
#include <undosort.hxx>
#include <scresid.hxx>
#include <scmod.hxx>
#include <dbdata.hxx>
#include <globstr.hrc>
#include <globalnames.hxx>
#include <tabvwsh.hxx>
#include <dociter.hxx>
#include <queryentry.hxx>
#include <inputopt.hxx>

#include <memory>

namespace sc
{
SortOperation::SortOperation(ScDocShell& rDocShell, SCTAB nTab, const ScSortParam& rSortParam,
                             bool bRecord, bool bPaint, bool bApi)
    : Operation(OperationType::Sort, bRecord, bApi)
    , mrDocShell(rDocShell)
    , mnTab(nTab)
    , mrSortParam(rSortParam)
    , mbPaint(bPaint)
{
}

bool SortOperation::runImplementation()
{
    ScDocShellModificator aModificator(mrDocShell);

    ScDocument& rDoc = mrDocShell.GetDocument();
    if (mbRecord && !rDoc.IsUndoEnabled())
        mbRecord = false;

    ScDBData* pDBData = rDoc.GetDBAtArea(mnTab, mrSortParam.nCol1, mrSortParam.nRow1,
                                         mrSortParam.nCol2, mrSortParam.nRow2);
    if (!pDBData)
    {
        SAL_WARN("sc", "Sort: no DBData");
        return false;
    }

    bool bCopy = !mrSortParam.bInplace;
    if (bCopy && mrSortParam.nDestCol == mrSortParam.nCol1
        && mrSortParam.nDestRow == mrSortParam.nRow1 && mrSortParam.nDestTab == mnTab)
        bCopy = false;

    ScSortParam aLocalParam(mrSortParam);
    if (bCopy)
    {
        // Copy the data range to the destination then move the sort range to it.
        ScRange aSrcRange(mrSortParam.nCol1, mrSortParam.nRow1, mnTab, mrSortParam.nCol2,
                          mrSortParam.nRow2, mnTab);
        ScAddress aDestPos(mrSortParam.nDestCol, mrSortParam.nDestRow, mrSortParam.nDestTab);

        ScDocFunc& rDocFunc = mrDocShell.GetDocFunc();
        bool bRet = rDocFunc.MoveBlock(aSrcRange, aDestPos, false, mbRecord, mbPaint, mbApi);

        if (!bRet)
            return false;

        aLocalParam.MoveToDest();
        mnTab = aLocalParam.nDestTab;
    }

    // tdf#119804: If there is a header row/column, it won't be affected by
    // sorting; so we can exclude it from the test.
    SCROW nStartingRowToEdit = aLocalParam.nRow1;
    SCCOL nStartingColToEdit = aLocalParam.nCol1;
    if (aLocalParam.bHasHeader)
    {
        if (aLocalParam.bByRow)
            nStartingRowToEdit++;
        else
            nStartingColToEdit++;
    }

    if (!checkSheetViewProtection())
        return false;

    ScEditableTester aTester = ScEditableTester::CreateAndTestBlock(
        rDoc, mnTab, nStartingColToEdit, nStartingRowToEdit, aLocalParam.nCol2, aLocalParam.nRow2,
        true /*bNoMatrixAtAll*/);
    if (!aTester.IsEditable())
    {
        if (!mbApi)
            mrDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    const ScInputOptions aInputOption = ScModule::get()->GetInputOptions();
    const bool bUpdateRefs = aInputOption.GetSortRefUpdate();

    // Adjust aLocalParam cols/rows to used data area. Keep sticky top row or
    // column (depending on direction) in any case, not just if it has headers,
    // so empty leading cells will be sorted to the end.
    // aLocalParam.nCol/Row will encompass data content only, extras in
    // aLocalParam.aDataAreaExtras.
    bool bShrunk = false;
    aLocalParam.aDataAreaExtras.resetArea();
    rDoc.ShrinkToUsedDataArea(
        bShrunk, mnTab, aLocalParam.nCol1, aLocalParam.nRow1, aLocalParam.nCol2, aLocalParam.nRow2,
        false, aLocalParam.bByRow, !aLocalParam.bByRow,
        (aLocalParam.aDataAreaExtras.anyExtrasWanted() ? &aLocalParam.aDataAreaExtras : nullptr));

    SCROW nStartRow = aLocalParam.nRow1;
    if (aLocalParam.bByRow && aLocalParam.bHasHeader && nStartRow < aLocalParam.nRow2)
        ++nStartRow;

    SCCOL nOverallCol1 = aLocalParam.nCol1;
    SCROW nOverallRow1 = aLocalParam.nRow1;
    SCCOL nOverallCol2 = aLocalParam.nCol2;
    SCROW nOverallRow2 = aLocalParam.nRow2;
    if (aLocalParam.aDataAreaExtras.anyExtrasWanted())
    {
        // Trailing empty excess columns/rows are excluded from being sorted,
        // they stick at the end. Clip them.
        const ScDataAreaExtras::Clip eClip
            = (aLocalParam.bByRow ? ScDataAreaExtras::Clip::Row : ScDataAreaExtras::Clip::Col);
        aLocalParam.aDataAreaExtras.GetOverallRange(nOverallCol1, nOverallRow1, nOverallCol2,
                                                    nOverallRow2, eClip);
        // Make it permanent.
        aLocalParam.aDataAreaExtras.SetOverallRange(nOverallCol1, nOverallRow1, nOverallCol2,
                                                    nOverallRow2);

        if (bUpdateRefs)
        {
            // With update references the entire range needs to be handled as
            // one entity for references pointing within to be moved along,
            // even when there's no data content. For huge ranges we may be
            // DOOMed then.
            aLocalParam.nCol1 = nOverallCol1;
            aLocalParam.nRow1 = nOverallRow1;
            aLocalParam.nCol2 = nOverallCol2;
            aLocalParam.nRow2 = nOverallRow2;
        }
    }

    if (aLocalParam.aDataAreaExtras.mbCellFormats
        && rDoc.HasAttrib(nOverallCol1, nStartRow, mnTab, nOverallCol2, nOverallRow2, mnTab,
                          HasAttrFlags::Merged | HasAttrFlags::Overlapped))
    {
        // Merge attributes would be mixed up during sorting.
        if (!mbApi)
            mrDocShell.ErrorMessage(STR_SORT_ERR_MERGED);
        return false;
    }

    //      execute

    weld::WaitObject aWait(ScDocShell::GetActiveDialogParent());

    // Calculate the script types for all cells in the sort range beforehand.
    // This will speed up the row height adjustment that takes place after the
    // sort.
    rDoc.UpdateScriptTypes(ScAddress(aLocalParam.nCol1, nStartRow, mnTab),
                           aLocalParam.nCol2 - aLocalParam.nCol1 + 1,
                           aLocalParam.nRow2 - nStartRow + 1);

    // No point adjusting row heights after the sort when all rows have the same height.
    bool bUniformRowHeight = rDoc.HasUniformRowHeight(mnTab, nStartRow, nOverallRow2);

    bool bRepeatQuery = false; // repeat existing filter?
    ScQueryParam aQueryParam;
    pDBData->GetQueryParam(aQueryParam);
    if (aQueryParam.GetEntry(0).bDoQuery)
        bRepeatQuery = true;

    sc::ReorderParam aUndoParam;

    // don't call ScDocument::Sort with an empty SortParam (may be empty here if bCopy is set)
    if (aLocalParam.GetSortKeyCount() && aLocalParam.maKeyState[0].bDoSort)
    {
        ScProgress aProgress(&mrDocShell, ScResId(STR_PROGRESS_SORTING), 0, true);
        if (!bRepeatQuery)
            bRepeatQuery = rDoc.HasHiddenRows(aLocalParam.nRow1, aLocalParam.nRow2, mnTab);
        rDoc.Sort(mnTab, aLocalParam, bRepeatQuery, bUpdateRefs, &aProgress, &aUndoParam);
    }

    if (mbRecord)
    {
        // Set up an undo object.
        mrDocShell.GetUndoManager()->AddUndoAction(
            std::make_unique<sc::UndoSort>(&mrDocShell, aUndoParam));
    }

    pDBData->SetSortParam(mrSortParam);
    // Remember additional settings on anonymous database ranges.
    if (pDBData == rDoc.GetAnonymousDBData(mnTab)
        || rDoc.GetDBCollection()->getAnonDBs().has(pDBData))
        pDBData->UpdateFromSortParam(mrSortParam);

    if (SfxViewShell* pKitSomeViewForThisDoc
        = comphelper::LibreOfficeKit::isActive() ? mrDocShell.GetBestViewShell(false) : nullptr)
    {
        SfxViewShell* pViewShell = SfxViewShell::GetFirst();
        while (pViewShell)
        {
            ScTabViewShell* pTabViewShell = dynamic_cast<ScTabViewShell*>(pViewShell);
            if (pTabViewShell && pTabViewShell->GetDocId() == pKitSomeViewForThisDoc->GetDocId())
            {
                if (ScPositionHelper* pPosHelper
                    = pTabViewShell->GetViewData().GetLOKHeightHelper(mnTab))
                    pPosHelper->invalidateByIndex(nStartRow);
            }
            pViewShell = SfxViewShell::GetNext(*pViewShell);
        }

        ScTabViewShell::notifyAllViewsSheetGeomInvalidation(
            pKitSomeViewForThisDoc, false /* bColumns */, true /* bRows */, true /* bSizes*/,
            true /* bHidden */, true /* bFiltered */, true /* bGroups */, mnTab);
    }

    if (nStartRow <= aLocalParam.nRow2)
    {
        ScRange aDirtyRange(aLocalParam.nCol1, nStartRow, mnTab, aLocalParam.nCol2,
                            aLocalParam.nRow2, mnTab);
        rDoc.SetDirty(aDirtyRange, true);
    }

    if (mbPaint)
    {
        PaintPartFlags nPaint = PaintPartFlags::Grid;
        SCCOL nStartX = nOverallCol1;
        SCROW nStartY = nOverallRow1;
        SCCOL nEndX = nOverallCol2;
        SCROW nEndY = nOverallRow2;
        if (bRepeatQuery)
        {
            nPaint |= PaintPartFlags::Left;
            nStartX = 0;
            nEndX = rDoc.MaxCol();
        }
        mrDocShell.PostPaint(ScRange(nStartX, nStartY, mnTab, nEndX, nEndY, mnTab), nPaint);
    }

    if (!bUniformRowHeight && nStartRow <= nOverallRow2)
        mrDocShell.AdjustRowHeight(nStartRow, nOverallRow2, mnTab);

    aModificator.SetDocumentModified();

    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
