/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <iostream>

#include "gridwin.hxx"
#include <svx/svdpage.hxx>
#include <libxml/xmlwriter.h>
#include <viewdata.hxx>
#include "document.hxx"
#include "patattr.hxx"
#include <svl/poolitem.hxx>
#include "userdat.hxx"
#include <dpobject.hxx>

namespace {

std::ostream& operator<<(std::ostream& rStrm, const ScAddress& rAddr)
{
    rStrm << "Col: " << rAddr.Col() << ", Row: " << rAddr.Row() << ", Tab: " << rAddr.Tab();
    return rStrm;
}

void dumpScDrawObjData(const ScGridWindow& rWindow, ScDrawObjData& rData, MapUnit eMapUnit)
{
    const Point& rStartOffset = rData.maStartOffset;
    Point aStartOffsetPixel = rWindow.LogicToPixel(rStartOffset, MapMode(eMapUnit));
    std::cout << "  Start: " << rData.maStart << ", Offset: " << aStartOffsetPixel << std::endl;

    const Point& rEndOffset = rData.maEndOffset;
    Point aEndOffsetPixel = rWindow.LogicToPixel(rEndOffset, MapMode(eMapUnit));
    std::cout << "  End: : " << rData.maEnd << ", Offset: " << aEndOffsetPixel << std::endl;
}

}

void ScGridWindow::dumpColumnInformationPixel()
{
    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB nTab = pViewData->GetTabNo();
    for (SCCOL nCol = 0; nCol <= 20; ++nCol)
    {
        sal_uInt16 nWidth = pDoc->GetColWidth(nCol, nTab);
        long nPixel = LogicToPixel(Point(nWidth, 0), MapMode(MapUnit::MapTwip)).getX();
        std::cout << "Column: " << nCol << ", Width: " << nPixel << "px" << std::endl;
    }
}

void ScGridWindow::dumpColumnInformationHmm()
{
    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB nTab = pViewData->GetTabNo();
    for (SCCOL nCol = 0; nCol <= 20; ++nCol)
    {
        sal_uInt16 nWidth = pDoc->GetColWidth(nCol, nTab);
        long nPixel = LogicToLogic(Point(nWidth, 0), MapUnit::MapTwip, MapUnit::Map100thMM).getX();
        std::cout << "Column: " << nCol << ", Width: " << nPixel << "hmm" << std::endl;
    }
}

void ScGridWindow::dumpCellProperties()
{
    ScDocument* pDoc = pViewData->GetDocument();
    const ScMarkData& rMark = pViewData->GetMarkData();
    SCTAB nTab = pViewData->GetTabNo();

    ScRangeList aList;
    if (rMark.IsMultiMarked())
    {
        aList = rMark.GetMarkedRangesForTab(nTab);
    }
    else if (rMark.IsMarked())
    {
        ScRange aRange;
        rMark.GetMarkArea(aRange);
        aList.Join(aRange);
    }
    else
    {
        SCCOL nCol = pViewData->GetCurX();
        SCROW nRow = pViewData->GetCurY();

        ScRange aRange(nCol, nRow, nTab);
        aList.Join(aRange, false);
    }

    xmlTextWriterPtr writer = xmlNewTextWriterFilename( "dump.xml", 0 );
    xmlTextWriterSetIndent(writer,1);
    xmlTextWriterSetIndentString(writer, BAD_CAST("    "));

    xmlTextWriterStartDocument( writer, nullptr, nullptr, nullptr );

    xmlTextWriterStartElement(writer, BAD_CAST("selection"));

    for (size_t i = 0, n = aList.size(); i < n; ++i)
    {
        ScRange* pRange = aList[i];
        if (!pRange)
            continue;

        for (SCCOL nCol = pRange->aStart.Col(); nCol <= pRange->aEnd.Col(); ++nCol)
        {
            for (SCROW nRow = pRange->aStart.Row(); nRow <= pRange->aEnd.Row(); ++nRow)
            {
                const ScPatternAttr* pPatternAttr = pDoc->GetPattern(nCol, nRow, nTab);
                xmlTextWriterStartElement(writer, BAD_CAST("cell"));
                xmlTextWriterWriteAttribute(writer, BAD_CAST("column"), BAD_CAST(OString::number(nCol).getStr()));
                xmlTextWriterWriteAttribute(writer, BAD_CAST("row"), BAD_CAST(OString::number(nRow).getStr()));
                xmlTextWriterWriteAttribute(writer, BAD_CAST("tab"), BAD_CAST(OString::number(nTab).getStr()));

                pPatternAttr->GetItemSet().dumpAsXml(writer);

                xmlTextWriterEndElement(writer);
            }
        }
    }

    xmlTextWriterEndElement(writer);

    xmlTextWriterEndDocument( writer );
    xmlFreeTextWriter (writer);
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
            size_t nObjCount = pPage->GetObjCount();
            for (size_t nObj = 0; nObj < nObjCount; ++nObj)
            {
                SdrObject* pObj = pPage->GetObj(nObj);
                std::cout << "Graphic Object" << std::endl;
                ScDrawObjData* pObjData = ScDrawLayer::GetObjData(pObj);
                if (pObjData)
                    dumpScDrawObjData(*this, *pObjData, pDrawLayer->GetScaleUnit());

                const tools::Rectangle& rRect = pObj->GetSnapRect();
                tools::Rectangle aRect = LogicToPixel(rRect, MapMode(pDrawLayer->GetScaleUnit()));
                std::cout << "Snap Rectangle (in pixel): " << aRect << std::endl;
            }
        }
    }
}

void ScGridWindow::dumpColumnCellStorage()
{
    // Get the current cursor position.
    ScAddress aCurPos = pViewData->GetCurPos();

    ScDocument* pDoc = pViewData->GetDocument();
    const ScDPObject* pDP = pDoc->GetDPAtCursor(aCurPos.Col(), aCurPos.Row(), aCurPos.Tab());
    if (pDP)
    {
        // Dump the pivot table info if the cursor is over a pivot table.
        pDP->Dump();
        pDP->DumpCache();
        return;
    }

    // Dump the column cell storage info.
    pDoc->DumpColumnStorage(aCurPos.Tab(), aCurPos.Col());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
