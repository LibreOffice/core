/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/SubTotalsOperation.hxx>

#include <osl/diagnose.h>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

#include <sortparam.hxx>
#include <dbdocfun.hxx>
#include <dbdata.hxx>
#include <docsh.hxx>
#include <document.hxx>
#include <editable.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <olinetab.hxx>
#include <rangenam.hxx>
#include <undodat.hxx>

namespace sc
{
SubTotalsOperation::SubTotalsOperation(ScDocShell& rDocShell, SCTAB nTab,
                                       const ScSubTotalParam& rParam, bool bRecord, bool bApi)
    : Operation(OperationType::SubTotals, bRecord, bApi)
    , mrDocShell(rDocShell)
    , mnTab(nTab)
    , maParam(rParam)
{
}

bool SubTotalsOperation::runImplementation()
{
    //! use also for ScDBFunc::DoSubTotals !
    //  then stays outside:
    //  - mark new range (from DBData)
    //  - SelectionChanged (?)

    SCTAB nTab = convertTab(mnTab);

    bool bDo = !maParam.bRemoveOnly; // false = only delete

    ScDocument& rDoc = mrDocShell.GetDocument();
    if (mbRecord && !rDoc.IsUndoEnabled())
        mbRecord = false;
    ScDBData* pDBData
        = rDoc.GetDBAtArea(nTab, maParam.nCol1, maParam.nRow1, maParam.nCol2, maParam.nRow2);
    if (!pDBData)
    {
        OSL_FAIL("SubTotals: no DBData");
        return false;
    }

    ScEditableTester aTester = ScEditableTester::CreateAndTestBlock(
        rDoc, nTab, 0, maParam.nRow1 + 1, rDoc.MaxCol(), rDoc.MaxRow());
    if (!aTester.IsEditable())
    {
        if (!mbApi)
            mrDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    if (rDoc.HasAttrib(maParam.nCol1, maParam.nRow1 + 1, nTab, maParam.nCol2, maParam.nRow2, nTab,
                       HasAttrFlags::Merged | HasAttrFlags::Overlapped))
    {
        if (!mbApi)
            mrDocShell.ErrorMessage(STR_MSSG_INSERTCELLS_0); // don't insert into merged
        return false;
    }

    bool bOk = true;
    if (maParam.bReplace)
    {
        if (rDoc.TestRemoveSubTotals(nTab, maParam))
        {
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(
                ScDocShell::GetActiveDialogParent(), VclMessageType::Question,
                VclButtonsType::YesNo, ScResId(STR_MSSG_DOSUBTOTALS_1))); // "Delete Data?"
            xBox->set_title(ScResId(STR_MSSG_DOSUBTOTALS_0)); // "StarCalc"
            bOk = xBox->run() == RET_YES;
        }
    }

    if (!bOk)
        return false;

    weld::WaitObject aWait(ScDocShell::GetActiveDialogParent());
    ScDocShellModificator aModificator(mrDocShell);

    ScSubTotalParam aNewParam(maParam); // end of range is being changed
    ScDocumentUniquePtr pUndoDoc;
    std::unique_ptr<ScOutlineTable> pUndoTab;
    std::unique_ptr<ScRangeName> pUndoRange;
    std::unique_ptr<ScDBCollection> pUndoDB;

    if (mbRecord) // secure old data
    {
        bool bOldFilter = bDo && maParam.bDoSort;

        SCTAB nTabCount = rDoc.GetTableCount();
        pUndoDoc.reset(new ScDocument(SCDOCMODE_UNDO));
        ScOutlineTable* pTable = rDoc.GetOutlineTable(nTab);
        if (pTable)
        {
            pUndoTab.reset(new ScOutlineTable(*pTable));

            // column/row state
            SCCOLROW nOutStartCol, nOutEndCol;
            SCCOLROW nOutStartRow, nOutEndRow;
            pTable->GetColArray().GetRange(nOutStartCol, nOutEndCol);
            pTable->GetRowArray().GetRange(nOutStartRow, nOutEndRow);

            pUndoDoc->InitUndo(rDoc, nTab, nTab, true, true);
            rDoc.CopyToDocument(static_cast<SCCOL>(nOutStartCol), 0, nTab,
                                static_cast<SCCOL>(nOutEndCol), rDoc.MaxRow(), nTab,
                                InsertDeleteFlags::NONE, false, *pUndoDoc);
            rDoc.CopyToDocument(0, nOutStartRow, nTab, rDoc.MaxCol(), nOutEndRow, nTab,
                                InsertDeleteFlags::NONE, false, *pUndoDoc);
        }
        else
            pUndoDoc->InitUndo(rDoc, nTab, nTab, false, bOldFilter);

        //  secure data range - incl. filtering result
        rDoc.CopyToDocument(0, maParam.nRow1 + 1, nTab, rDoc.MaxCol(), maParam.nRow2, nTab,
                            InsertDeleteFlags::ALL, false, *pUndoDoc);

        //  all formulas because of references
        rDoc.CopyToDocument(0, 0, 0, rDoc.MaxCol(), rDoc.MaxRow(), nTabCount - 1,
                            InsertDeleteFlags::FORMULA, false, *pUndoDoc);

        //  ranges of DB and other
        ScRangeName* pDocRange = rDoc.GetRangeName();
        if (!pDocRange->empty())
            pUndoRange.reset(new ScRangeName(*pDocRange));
        ScDBCollection* pDocDB = rDoc.GetDBCollection();
        if (!pDocDB->empty())
            pUndoDB.reset(new ScDBCollection(*pDocDB));
    }

    //      rDoc.SetOutlineTable( nTab, NULL );
    ScOutlineTable* pOut = rDoc.GetOutlineTable(nTab);
    if (pOut)
        pOut->GetRowArray().RemoveAll(); // only delete row outlines

    if (maParam.bReplace)
        rDoc.RemoveSubTotals(nTab, aNewParam);
    bool bSuccess = true;
    if (bDo)
    {
        // sort
        if (maParam.bDoSort)
        {
            pDBData->SetArea(nTab, aNewParam.nCol1, aNewParam.nRow1, aNewParam.nCol2,
                             aNewParam.nRow2);

            //  set partial result field to before the sorting
            //  (Duplicates are omitted, so can be called again)

            ScSortParam aOldSort;
            pDBData->GetSortParam(aOldSort);
            ScSortParam aSortParam(aNewParam, aOldSort);
            ScDBDocFunc aDBDocFunc(mrDocShell);
            aDBDocFunc.SortTab(nTab, aSortParam, false, false, mbApi);
        }

        bSuccess = rDoc.DoSubTotals(nTab, aNewParam);
        rDoc.SetDrawPageSize(nTab);
    }
    ScRange aDirtyRange(aNewParam.nCol1, aNewParam.nRow1, nTab, aNewParam.nCol2, aNewParam.nRow2,
                        nTab);
    rDoc.SetDirty(aDirtyRange, true);

    if (mbRecord)
    {
        //          ScDBData* pUndoDBData = pDBData ? new ScDBData( *pDBData ) : NULL;
        mrDocShell.GetUndoManager()->AddUndoAction(std::make_unique<ScUndoSubTotals>(
            &mrDocShell, nTab, maParam, aNewParam.nRow2, std::move(pUndoDoc),
            std::move(pUndoTab), // pUndoDBData,
            std::move(pUndoRange), std::move(pUndoDB)));
    }

    if (!bSuccess)
    {
        // "Cannot insert rows"
        if (!mbApi)
            mrDocShell.ErrorMessage(STR_MSSG_DOSUBTOTALS_2);
    }

    // memorize
    pDBData->SetSubTotalParam(aNewParam);
    pDBData->SetArea(nTab, aNewParam.nCol1, aNewParam.nRow1, aNewParam.nCol2, aNewParam.nRow2);
    rDoc.CompileDBFormula();

    mrDocShell.PostPaint(ScRange(0, 0, nTab, rDoc.MaxCol(), rDoc.MaxRow(), nTab),
                         PaintPartFlags::Grid | PaintPartFlags::Left | PaintPartFlags::Top
                             | PaintPartFlags::Size);

    syncSheetViews();

    aModificator.SetDocumentModified();

    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
