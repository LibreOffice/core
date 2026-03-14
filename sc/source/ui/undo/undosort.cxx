/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <undosort.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <global.hxx>
#include <undoutil.hxx>
#include <SheetViewManager.hxx>
#include <utility>

namespace sc {

UndoSort::UndoSort( ScDocShell& rDocSh, ReorderParam aParam ) :
    ScSimpleUndo(rDocSh), maParam(std::move(aParam)) {}

void UndoSort::setSheetViewContext(SCTAB nDefaultViewTab, SheetViewID nSheetViewID,
                                   std::shared_ptr<SheetViewSortData> pSortDataBefore,
                                   std::shared_ptr<SheetViewSortData> pSortDataAfter)
{
    mnDefaultViewTab = nDefaultViewTab;
    mnSheetViewID = nSheetViewID;
    mpSortDataBefore = std::move(pSortDataBefore);
    mpSortDataAfter = std::move(pSortDataAfter);
}

void UndoSort::setDefaultViewContext(SCTAB nDefaultViewTab,
                                     std::shared_ptr<DefaultViewSortData> pDefaultViewSortDataBefore,
                                     std::shared_ptr<DefaultViewSortData> pDefaultViewSortDataAfter)
{
    mnDefaultViewTab = nDefaultViewTab;
    mbDefaultViewSort = true;
    mpDefaultViewSortDataBefore = std::move(pDefaultViewSortDataBefore);
    mpDefaultViewSortDataAfter = std::move(pDefaultViewSortDataAfter);
}

OUString UndoSort::GetComment() const
{
    return ScResId(maParam.mbShuffle ? STR_UNDO_SHUFFLE : STR_UNDO_SORT);
}

void UndoSort::Undo()
{
    BeginUndo();
    Execute(true);
    EndUndo();
}

void UndoSort::Redo()
{
    BeginRedo();
    Execute(false);
    EndRedo();
}

void UndoSort::Execute( bool bUndo )
{
    ScDocument& rDoc = rDocShell.GetDocument();
    sc::ReorderParam aParam = maParam;
    if (bUndo)
        aParam.reverse();
    rDoc.Reorder(aParam);

    // Restore sheet view sort data for sorts on a sheet view tab
    if (mnSheetViewID >= 0 && mnDefaultViewTab >= 0)
    {
        std::shared_ptr<SheetViewManager> pManager = rDoc.GetSheetViewManager(mnDefaultViewTab);
        if (pManager)
        {
            std::shared_ptr<SheetView> pSheetView = pManager->get(mnSheetViewID);
            if (pSheetView)
                pSheetView->restoreSortData(bUndo ? mpSortDataBefore : mpSortDataAfter);
        }
    }

    // Restore manager sort data for sorts on a default view tab
    if (mbDefaultViewSort && mnDefaultViewTab >= 0)
    {
        std::shared_ptr<SheetViewManager> pManager = rDoc.GetSheetViewManager(mnDefaultViewTab);
        if (pManager)
            pManager->restoreSortData(bUndo ? mpDefaultViewSortDataBefore : mpDefaultViewSortDataAfter);
    }

    ScRange aOverallRange( maParam.maSortRange);
    if (maParam.maDataAreaExtras.anyExtrasWanted())
    {
        aOverallRange.aStart.SetCol( maParam.maDataAreaExtras.mnStartCol);
        aOverallRange.aStart.SetRow( maParam.maDataAreaExtras.mnStartRow);
        aOverallRange.aEnd.SetCol( maParam.maDataAreaExtras.mnEndCol);
        aOverallRange.aEnd.SetRow( maParam.maDataAreaExtras.mnEndRow);
    }

    if (maParam.mbHasHeaders)
    {
        ScRange aMarkRange( aOverallRange);
        if (maParam.mbByRow)
        {
            if (aMarkRange.aStart.Row() > 0)
                aMarkRange.aStart.IncRow(-1);
        }
        else
        {
            if (aMarkRange.aStart.Col() > 0)
                aMarkRange.aStart.IncCol(-1);
        }
        ScUndoUtil::MarkSimpleBlock(rDocShell, aMarkRange);
    }
    else
    {
        ScUndoUtil::MarkSimpleBlock(rDocShell, aOverallRange);
    }

    rDoc.SetDirty(maParam.maSortRange, true);
    if (!aParam.mbUpdateRefs)
        rDoc.BroadcastCells(aParam.maSortRange, SfxHintId::ScDataChanged);

    rDocShell.PostPaint(aOverallRange, PaintPartFlags::Grid);
    rDocShell.PostDataChanged();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
