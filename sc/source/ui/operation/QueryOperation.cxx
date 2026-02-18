/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/QueryOperation.hxx>

#include <dbdata.hxx>
#include <docsh.hxx>
#include <editable.hxx>
#include <globstr.hrc>
#include <tabvwsh.hxx>
#include <patattr.hxx>
#include <queryentry.hxx>
#include <markdata.hxx>
#include <progress.hxx>
#include <undodat.hxx>
#include <scresid.hxx>
#include <globalnames.hxx>
#include <SheetViewOperationsTester.hxx>

#include <osl/diagnose.h>
#include <vcl/weld.hxx>

namespace sc
{
QueryOperation::QueryOperation(ScDocShell& rDocShell, SCTAB nTab, ScQueryParam const& rQueryParam,
                               ScRange const* pAdvSource, bool bRecord, bool bApi)
    : Operation(OperationType::Query, bRecord, bApi)
    , mrDocShell(rDocShell)
    , mnTab(nTab)
    , mrQueryParam(rQueryParam)
    , mpAdvSource(pAdvSource)
{
}

bool QueryOperation::runImplementation()
{
    ScDocShellModificator aModificator(mrDocShell);

    ScDocument& rDoc = mrDocShell.GetDocument();

    ScTabViewShell* pViewSh = mrDocShell.GetBestViewShell();
    if (pViewSh
        && ScTabViewShell::isAnyEditViewInRange(pViewSh, /*bColumns*/ false, mrQueryParam.nRow1,
                                                mrQueryParam.nRow2))
    {
        return false;
    }

    if (mbRecord && !rDoc.IsUndoEnabled())
        mbRecord = false;

    ScDBData* pDBData = rDoc.GetDBAtArea(mnTab, mrQueryParam.nCol1, mrQueryParam.nRow1,
                                         mrQueryParam.nCol2, mrQueryParam.nRow2);
    if (!pDBData)
    {
        OSL_FAIL("Query: no DBData");
        return false;
    }

    //  Change from Inplace to non-Inplace, only then cancel Inplace:
    //  (only if "Persistent"  is selected in the dialog)

    if (!mrQueryParam.bInplace && pDBData->HasQueryParam() && mrQueryParam.bDestPers)
    {
        ScQueryParam aOldQuery;
        pDBData->GetQueryParam(aOldQuery);
        if (aOldQuery.bInplace)
        {
            //  cancel old filtering

            SCSIZE nEC = aOldQuery.GetEntryCount();
            for (SCSIZE i = 0; i < nEC; i++)
                aOldQuery.GetEntry(i).bDoQuery = false;
            aOldQuery.bDuplicate = true;
            QueryOperation aOperation(mrDocShell, mnTab, aOldQuery, nullptr, mbRecord, mbApi);
            aOperation.run();
        }
    }

    ScQueryParam aLocalParam(mrQueryParam); // for Paint / destination range
    bool bCopy = !mrQueryParam.bInplace; // copied in Table::Query
    ScDBData* pDestData = nullptr; // range to be copied to
    bool bDoSize = false; // adjust destination size (insert/delete)
    SCCOL nFormulaCols = 0; // only at bDoSize
    bool bKeepFmt = false;
    ScRange aOldDest;
    ScRange aDestTotal;
    if (bCopy && mrQueryParam.nDestCol == mrQueryParam.nCol1
        && mrQueryParam.nDestRow == mrQueryParam.nRow1 && mrQueryParam.nDestTab == mnTab)
        bCopy = false;
    SCTAB nDestTab = mnTab;
    if (bCopy)
    {
        aLocalParam.MoveToDest();
        nDestTab = mrQueryParam.nDestTab;
        if (!rDoc.ValidColRow(aLocalParam.nCol2, aLocalParam.nRow2))
        {
            if (!mbApi)
                mrDocShell.ErrorMessage(STR_PASTE_FULL);
            return false;
        }

        if (!checkSheetViewProtection())
            return false;

        ScEditableTester aTester = ScEditableTester::CreateAndTestBlock(
            rDoc, nDestTab, aLocalParam.nCol1, aLocalParam.nRow1, aLocalParam.nCol2,
            aLocalParam.nRow2);
        if (!aTester.IsEditable())
        {
            if (!mbApi)
                mrDocShell.ErrorMessage(aTester.GetMessageId());
            return false;
        }

        pDestData = rDoc.GetDBAtCursor(mrQueryParam.nDestCol, mrQueryParam.nDestRow,
                                       mrQueryParam.nDestTab, ScDBDataPortion::TOP_LEFT);
        if (pDestData)
        {
            pDestData->GetArea(aOldDest);
            aDestTotal = ScRange(mrQueryParam.nDestCol, mrQueryParam.nDestRow, nDestTab,
                                 mrQueryParam.nDestCol + mrQueryParam.nCol2 - mrQueryParam.nCol1,
                                 mrQueryParam.nDestRow + mrQueryParam.nRow2 - mrQueryParam.nRow1,
                                 nDestTab);

            bDoSize = pDestData->IsDoSize();
            //  test if formulas need to be filled in (nFormulaCols):
            if (bDoSize && aOldDest.aEnd.Col() == aDestTotal.aEnd.Col())
            {
                SCCOL nTestCol = aOldDest.aEnd.Col() + 1; // next to the range
                SCROW nTestRow = mrQueryParam.nDestRow + (aLocalParam.bHasHeader ? 1 : 0);
                while (nTestCol <= rDoc.MaxCol()
                       && rDoc.GetCellType(ScAddress(nTestCol, nTestRow, mnTab))
                              == CELLTYPE_FORMULA)
                {
                    ++nTestCol;
                    ++nFormulaCols;
                }
            }

            bKeepFmt = pDestData->IsKeepFmt();
            if (bDoSize && !rDoc.CanFitBlock(aOldDest, aDestTotal))
            {
                if (!mbApi)
                    mrDocShell.ErrorMessage(STR_MSSG_DOSUBTOTALS_2); // cannot insert rows
                return false;
            }
        }
    }

    //      execute

    weld::WaitObject aWait(ScDocShell::GetActiveDialogParent());

    bool bKeepSub = false; // repeat existing partial results?
    bool bKeepTotals = false;
    if (mrQueryParam.GetEntry(0).bDoQuery) // not at cancellation
    {
        ScSubTotalParam aSubTotalParam;
        pDBData->GetSubTotalParam(aSubTotalParam); // partial results exist?

        if (aSubTotalParam.aGroups[0].bActive && !aSubTotalParam.bRemoveOnly)
            bKeepSub = true;

        if (pDBData->HasTotals() && pDBData->GetTableStyleInfo())
            bKeepTotals = true;
    }

    ScDocumentUniquePtr pUndoDoc;
    std::unique_ptr<ScDBCollection> pUndoDB;
    const ScRange* pOld = nullptr;

    if (mbRecord)
    {
        pUndoDoc.reset(new ScDocument(SCDOCMODE_UNDO));
        if (bCopy)
        {
            pUndoDoc->InitUndo(rDoc, nDestTab, nDestTab, false, true);
            rDoc.CopyToDocument(aLocalParam.nCol1, aLocalParam.nRow1, nDestTab, aLocalParam.nCol2,
                                aLocalParam.nRow2, nDestTab, InsertDeleteFlags::ALL, false,
                                *pUndoDoc);
            //  secure attributes in case they were copied along

            if (pDestData)
            {
                rDoc.CopyToDocument(aOldDest, InsertDeleteFlags::ALL, false, *pUndoDoc);
                pOld = &aOldDest;
            }
        }
        else
        {
            pUndoDoc->InitUndo(rDoc, mnTab, mnTab, false, true);
            rDoc.CopyToDocument(0, mrQueryParam.nRow1, mnTab, rDoc.MaxCol(), mrQueryParam.nRow2,
                                mnTab, InsertDeleteFlags::NONE, false, *pUndoDoc);
        }

        ScDBCollection* pDocDB = rDoc.GetDBCollection();
        if (!pDocDB->empty())
            pUndoDB.reset(new ScDBCollection(*pDocDB));

        rDoc.BeginDrawUndo();
    }

    std::unique_ptr<ScDocument> pAttribDoc;
    ScRange aAttribRange;
    if (pDestData) // delete destination range
    {
        if (bKeepFmt)
        {
            //  smaller of the end columns, header+1 row
            aAttribRange = aOldDest;
            if (aAttribRange.aEnd.Col() > aDestTotal.aEnd.Col())
                aAttribRange.aEnd.SetCol(aDestTotal.aEnd.Col());
            aAttribRange.aEnd.SetRow(aAttribRange.aStart.Row() + (aLocalParam.bHasHeader ? 1 : 0));

            //  also for filled-in formulas
            aAttribRange.aEnd.SetCol(aAttribRange.aEnd.Col() + nFormulaCols);

            pAttribDoc.reset(new ScDocument(SCDOCMODE_UNDO));
            pAttribDoc->InitUndo(rDoc, nDestTab, nDestTab, false, true);
            rDoc.CopyToDocument(aAttribRange, InsertDeleteFlags::ATTRIB, false, *pAttribDoc);
        }

        if (bDoSize)
            rDoc.FitBlock(aOldDest, aDestTotal);
        else
            rDoc.DeleteAreaTab(aOldDest, InsertDeleteFlags::ALL); // simply delete
    }

    //  execute filtering on the document
    SCSIZE nCount = rDoc.Query(mnTab, mrQueryParam, bKeepSub, bKeepTotals);
    pDBData->CalcSaveFilteredCount(nCount);
    if (bCopy)
    {
        aLocalParam.nRow2 = aLocalParam.nRow1 + nCount;
        if (!aLocalParam.bHasHeader && nCount > 0)
            --aLocalParam.nRow2;

        if (bDoSize)
        {
            //  adjust to the real result range
            //  (this here is always a reduction)

            ScRange aNewDest(aLocalParam.nCol1, aLocalParam.nRow1, nDestTab, aLocalParam.nCol2,
                             aLocalParam.nRow2, nDestTab);
            rDoc.FitBlock(aDestTotal, aNewDest, false); // sal_False - don't delete

            if (nFormulaCols > 0)
            {
                //  fill in formulas
                //! Undo (Query and Repeat) !!!

                ScRange aNewForm(aLocalParam.nCol2 + 1, aLocalParam.nRow1, nDestTab,
                                 aLocalParam.nCol2 + nFormulaCols, aLocalParam.nRow2, nDestTab);
                ScRange aOldForm = aNewForm;
                aOldForm.aEnd.SetRow(aOldDest.aEnd.Row());
                rDoc.FitBlock(aOldForm, aNewForm, false);

                ScMarkData aMark(rDoc.GetSheetLimits());
                aMark.SelectOneTable(nDestTab);
                SCROW nFStartY = aLocalParam.nRow1 + (aLocalParam.bHasHeader ? 1 : 0);

                sal_uLong nProgCount = nFormulaCols;
                nProgCount *= aLocalParam.nRow2 - nFStartY;
                ScProgress aProgress(rDoc.GetDocumentShell(), ScResId(STR_FILL_SERIES_PROGRESS),
                                     nProgCount, true);

                rDoc.Fill(aLocalParam.nCol2 + 1, nFStartY, aLocalParam.nCol2 + nFormulaCols,
                          nFStartY, &aProgress, aMark, aLocalParam.nRow2 - nFStartY, FILL_TO_BOTTOM,
                          FILL_SIMPLE);
            }
        }

        if (pAttribDoc) // copy back the memorized attributes
        {
            //  Header
            if (aLocalParam.bHasHeader)
            {
                ScRange aHdrRange = aAttribRange;
                aHdrRange.aEnd.SetRow(aHdrRange.aStart.Row());
                pAttribDoc->CopyToDocument(aHdrRange, InsertDeleteFlags::ATTRIB, false, rDoc);
            }

            //  Data
            SCCOL nAttrEndCol = aAttribRange.aEnd.Col();
            SCROW nAttrRow = aAttribRange.aStart.Row() + (aLocalParam.bHasHeader ? 1 : 0);
            for (SCCOL nCol = aAttribRange.aStart.Col(); nCol <= nAttrEndCol; nCol++)
            {
                const ScPatternAttr* pSrcPattern = pAttribDoc->GetPattern(nCol, nAttrRow, nDestTab);
                OSL_ENSURE(pSrcPattern, "Pattern is 0");
                if (pSrcPattern)
                {
                    rDoc.ApplyPatternAreaTab(nCol, nAttrRow, nCol, aLocalParam.nRow2, nDestTab,
                                             *pSrcPattern);
                    const ScStyleSheet* pStyle = pSrcPattern->GetStyleSheet();
                    if (pStyle)
                        rDoc.ApplyStyleAreaTab(nCol, nAttrRow, nCol, aLocalParam.nRow2, nDestTab,
                                               *pStyle);
                }
            }
        }
    }

    //  saving: Inplace always, otherwise depending on setting
    //          old Inplace-Filter may have already been removed

    bool bSave = mrQueryParam.bInplace || mrQueryParam.bDestPers;
    if (bSave) // memorize
    {
        pDBData->SetQueryParam(mrQueryParam);
        pDBData->SetHeader(mrQueryParam.bHasHeader); //! ???
        pDBData->SetAdvancedQuerySource(mpAdvSource); // after SetQueryParam
    }

    if (bCopy) // memorize new DB range
    {
        //  Selection is done afterwards from outside (dbfunc).
        //  Currently through the DB area at the destination position,
        //  so a range must be created there in any case.

        ScDBData* pNewData;
        if (pDestData)
            pNewData = pDestData; // range exists -> adjust (always!)
        else // create range
            pNewData = mrDocShell.GetDBData(ScRange(aLocalParam.nCol1, aLocalParam.nRow1, nDestTab,
                                                    aLocalParam.nCol2, aLocalParam.nRow2, nDestTab),
                                            SC_DB_MAKE, ScGetDBSelection::ForceMark);

        if (pNewData)
        {
            pNewData->SetArea(nDestTab, aLocalParam.nCol1, aLocalParam.nRow1, aLocalParam.nCol2,
                              aLocalParam.nRow2);

            //  query parameter is no longer set at the destination, only leads to confusion
            //  and mistakes with the query parameter at the source range (#37187#)
        }
        else
        {
            OSL_FAIL("Target are not available");
        }
    }

    if (!bCopy)
    {
        rDoc.InvalidatePageBreaks(mnTab);
        rDoc.UpdatePageBreaks(mnTab);
    }

    // #i23299# Subtotal functions depend on cell's filtered states.
    ScRange aDirtyRange(0, aLocalParam.nRow1, nDestTab, rDoc.MaxCol(), aLocalParam.nRow2, nDestTab);
    rDoc.SetSubTotalCellsDirty(aDirtyRange);

    if (mbRecord)
    {
        // create undo action after executing, because of drawing layer undo
        mrDocShell.GetUndoManager()->AddUndoAction(
            std::make_unique<ScUndoQuery>(&mrDocShell, mnTab, mrQueryParam, std::move(pUndoDoc),
                                          std::move(pUndoDB), pOld, bDoSize, mpAdvSource));
    }

    if (pViewSh)
    {
        // could there be horizontal autofilter ?
        // maybe it would be better to set bColumns to !rQueryParam.bByRow ?
        // anyway at the beginning the value of bByRow is 'false'
        // then after the first sort action it becomes 'true'
        pViewSh->OnLOKShowHideColRow(/*bColumns*/ false, mrQueryParam.nRow1 - 1);
    }

    if (bCopy)
    {
        SCCOL nEndX = aLocalParam.nCol2;
        SCROW nEndY = aLocalParam.nRow2;
        if (pDestData)
        {
            if (aOldDest.aEnd.Col() > nEndX)
                nEndX = aOldDest.aEnd.Col();
            if (aOldDest.aEnd.Row() > nEndY)
                nEndY = aOldDest.aEnd.Row();
        }
        if (bDoSize)
            nEndY = rDoc.MaxRow();

        // remove AutoFilter button flags
        mrDocShell.DBAreaDeleted(nDestTab, aLocalParam.nCol1, aLocalParam.nRow1, aLocalParam.nCol2);

        mrDocShell.PostPaint(
            ScRange(aLocalParam.nCol1, aLocalParam.nRow1, nDestTab, nEndX, nEndY, nDestTab),
            PaintPartFlags::Grid);
    }
    else
        mrDocShell.PostPaint(
            ScRange(0, mrQueryParam.nRow1, mnTab, rDoc.MaxCol(), rDoc.MaxRow(), mnTab),
            PaintPartFlags::Grid | PaintPartFlags::Left);
    aModificator.SetDocumentModified();

    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
