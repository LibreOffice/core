/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "gridwin.hxx"
#include <svx/svdpage.hxx>

#include "userdat.hxx"

namespace {

std::ostream& operator<<(std::ostream& rStrm, const ScAddress& rAddr)
{
    rStrm << "Col: " << rAddr.Col() << ", Row: " << rAddr.Row() << ", Tab: " << rAddr.Tab();
    return rStrm;
}

void dumpScDrawObjData(ScGridWindow& rWindow, ScDrawObjData& rData, MapUnit eMapUnit)
{
    const Point& rStartOffset = rData.maStartOffset;
    Point aStartOffsetPixel = rWindow.LogicToPixel(rStartOffset, MapMode(eMapUnit));
    std::cout << "  Start: " << rData.maStart << ", Offset: " << aStartOffsetPixel << std::endl;

    const Point& rEndOffset = rData.maEndOffset;
    Point aEndOffsetPixel = rWindow.LogicToPixel(rEndOffset, MapMode(eMapUnit));
    std::cout << "  End: : " << rData.maEnd << ", Offset: " << aEndOffsetPixel << std::endl;
}

}

void ScGridWindow::dumpColumnInformation()
{
    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB nTab = pViewData->GetTabNo();
    for (SCCOL nCol = 0; nCol <= 20; ++nCol)
    {
        sal_uInt16 nWidth = pDoc->GetColWidth(nCol, nTab, true);
        long nPixel = LogicToPixel(Point(nWidth, 0), MapMode(MAP_TWIP)).getX();
        std::cout << "Column: " << nCol << ", Width: " << nPixel << "px" << std::endl;
    }
}

void ScGridWindow::dumpGraphicInformation()
{
    ScDocument* pDoc = pViewData->GetDocument();
    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    if (pDrawLayer)
    {
        sal_uInt16 nPageCount = pDrawLayer->GetPageCount();
        for (sal_uInt16 nPage = 0; nPage < nPageCount; ++nPage)
        {
            SdrPage* pPage = pDrawLayer->GetPage(nPage);
            sal_uInt16 nObjCount = pPage->GetObjCount();
            for (sal_uInt16 nObj = 0; nObj < nObjCount; ++nObj)
            {
                SdrObject* pObj = pPage->GetObj(nObj);
                std::cout << "Graphic Object" << std::endl;
                ScDrawObjData* pObjData = ScDrawLayer::GetObjData(pObj);
                if (pObjData)
                    dumpScDrawObjData(*this, *pObjData, pDrawLayer->GetScaleUnit());

                const Rectangle& rRect = pObj->GetSnapRect();
                Rectangle aRect = LogicToPixel(rRect, MapMode(pDrawLayer->GetScaleUnit()));
                std::cout << "Snap Rectangle (in pixel): " << aRect << std::endl;
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
