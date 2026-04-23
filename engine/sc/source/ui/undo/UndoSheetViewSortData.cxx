/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <undo/UndoSheetViewSortData.hxx>
#include <dbdata.hxx>
#include <docsh.hxx>
#include <document.hxx>

namespace sc
{
void UndoSheetViewSortData::setSheetViewContext(SCTAB nDefaultViewTab, SheetViewID nSheetViewID,
                                                std::shared_ptr<SheetViewSortData> pBefore,
                                                std::shared_ptr<SheetViewSortData> pAfter)
{
    mnDefaultViewTab = nDefaultViewTab;
    mnSheetViewID = nSheetViewID;
    mpSheetViewSortDataBefore = std::move(pBefore);
    mpSheetViewSortDataAfter = std::move(pAfter);
}

void UndoSheetViewSortData::setDefaultViewContext(SCTAB nDefaultViewTab,
                                                  std::shared_ptr<DefaultViewSortData> pBefore,
                                                  std::shared_ptr<DefaultViewSortData> pAfter)
{
    mnDefaultViewTab = nDefaultViewTab;
    mpDefaultViewSortDataBefore = std::move(pBefore);
    mpDefaultViewSortDataAfter = std::move(pAfter);
}

void UndoSheetViewSortData::setAutoFilterRange(ScRange const& rBefore, ScRange const& rAfter)
{
    moAutoFilterRangeBefore = rBefore;
    moAutoFilterRangeAfter = rAfter;
}

void UndoSheetViewSortData::restore(ScDocShell& rDocShell, bool bUndo) const
{
    if (mnDefaultViewTab < 0)
        return;

    ScDocument& rDocument = rDocShell.GetDocument();

    // Restore sheet view sort data
    if (mnSheetViewID >= 0)
    {
        std::shared_ptr<SheetViewManager> pManager
            = rDocument.GetSheetViewManager(mnDefaultViewTab);
        if (pManager)
        {
            std::shared_ptr<SheetView> pSheetView = pManager->get(mnSheetViewID);
            if (pSheetView)
                pSheetView->restoreSortData(bUndo ? mpSheetViewSortDataBefore
                                                  : mpSheetViewSortDataAfter);
        }
    }

    // Restore manager (default view) sort data
    if (mpDefaultViewSortDataBefore || mpDefaultViewSortDataAfter)
    {
        std::shared_ptr<SheetViewManager> pManager
            = rDocument.GetSheetViewManager(mnDefaultViewTab);
        if (pManager)
            pManager->restoreSortData(bUndo ? mpDefaultViewSortDataBefore
                                            : mpDefaultViewSortDataAfter);
    }

    // Restore auto-filter DB range
    if (moAutoFilterRangeBefore || moAutoFilterRangeAfter)
    {
        ScDBData* pDBData = rDocument.GetAnonymousDBData(mnDefaultViewTab);
        if (pDBData)
        {
            ScRange const& rRange = bUndo ? *moAutoFilterRangeBefore : *moAutoFilterRangeAfter;
            pDBData->SetArea(rRange);
        }
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
