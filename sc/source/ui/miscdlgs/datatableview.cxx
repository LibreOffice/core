/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <datatableview.hxx>

#include <document.hxx>
#include <utility>
#include <viewdata.hxx>
#include <output.hxx>
#include <fillinfo.hxx>
#include <table.hxx>

#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/seleng.hxx>
#include <sal/log.hxx>

constexpr double nPPTX = 0.06666;
constexpr double nPPTY = 0.06666;

constexpr sal_uInt16 nRowHeaderWidth = 100;
constexpr sal_uInt16 nColHeaderHeight = 20;

ScDataTableColView::ScDataTableColView(vcl::Window* pParent, ScDocument* pDoc, SelectionEngine* pSelectionEngine):
        ScHeaderControl(pParent, pSelectionEngine, pDoc->MaxCol()+1, false, nullptr),
        mpDoc(pDoc),
        mnCol(0)
{
}

void ScDataTableColView::SetPos(SCCOLROW nCol)
{
    mnCol = nCol;
}

SCCOLROW ScDataTableColView::GetPos() const
{
    return mnCol;
}

sal_uInt16 ScDataTableColView::GetEntrySize(SCCOLROW nPos) const
{
    return ScViewData::ToPixel(mpDoc->GetColWidth(nPos, 0), nPPTX);
}

OUString ScDataTableColView::GetEntryText(SCCOLROW nPos) const
{
    return "Col: " + OUString::number(nPos + 1);
}

bool ScDataTableColView::IsLayoutRTL() const
{
    return false;
}

void ScDataTableColView::SetEntrySize(SCCOLROW nPos, sal_uInt16 nColWidth)
{
    mpDoc->SetColWidthOnly(nPos, 0, nColWidth/nPPTX);
}

void ScDataTableColView::HideEntries(SCCOLROW nPos, SCCOLROW nEndPos)
{
    for (SCCOLROW nCol = nPos; nCol <= nEndPos; ++nCol)
    {
        mpDoc->ShowCol(nCol, 0, false);
    }
}


ScDataTableRowView::ScDataTableRowView(vcl::Window* pParent, ScDocument* pDoc, SelectionEngine* pSelectionEngine):
        ScHeaderControl(pParent, pSelectionEngine, pDoc->MaxRow()+1, true, nullptr),
        mpDoc(pDoc),
        mnRow(0)
{
}

void ScDataTableRowView::SetPos(SCCOLROW nRow)
{
    mnRow = nRow;
}

SCCOLROW ScDataTableRowView::GetPos() const
{
    return mnRow;
}

sal_uInt16 ScDataTableRowView::GetEntrySize(SCCOLROW nPos) const
{
    return ScViewData::ToPixel(mpDoc->GetRowHeight(nPos, SCTAB(0), true), nPPTX);
}

OUString ScDataTableRowView::GetEntryText(SCCOLROW nPos) const
{
    return OUString::number(nPos + 1);
}

bool ScDataTableRowView::IsLayoutRTL() const
{
    return false;
}

void ScDataTableRowView::SetEntrySize(SCCOLROW nPos, sal_uInt16 nColWidth)
{
    mpDoc->SetRowHeight(nPos, 0, nColWidth/nPPTX);
}

void ScDataTableRowView::HideEntries(SCCOLROW nPos, SCCOLROW nEndPos)
{
    for (SCCOLROW nCol = nPos; nCol <= nEndPos; ++nCol)
    {
        mpDoc->ShowRow(nCol, 0, false);
    }
}

ScDataTableView::ScDataTableView(const css::uno::Reference<css::awt::XWindow> &rParent, std::shared_ptr<ScDocument> pDoc) :
    Control(VCLUnoHelper::GetWindow(rParent)),
    mpDoc(std::move(pDoc)),
    mpSelectionEngine(new SelectionEngine(this)),
    mpColView(VclPtr<ScDataTableColView>::Create(this, mpDoc.get(), mpSelectionEngine.get())),
    mpRowView(VclPtr<ScDataTableRowView>::Create(this, mpDoc.get(), mpSelectionEngine.get())),
    mpVScroll(VclPtr<ScrollAdaptor>::Create(this, false)),
    mpHScroll(VclPtr<ScrollAdaptor>::Create(this, true)),
    mnScrollBarSize(mpVScroll->GetSizePixel().Width()),
    mnFirstVisibleRow(0),
    mnFirstVisibleCol(0)
{
    mpColView->setPosSizePixel(nRowHeaderWidth, 0, nRowHeaderWidth, nColHeaderHeight);
    mpRowView->setPosSizePixel(0, nColHeaderHeight, nRowHeaderWidth, nColHeaderHeight);

    mpVScroll->SetRangeMin(0);
    mpVScroll->SetRangeMax(100);
    mpVScroll->SetScrollHdl(LINK(this, ScDataTableView, VertScrollHdl));

    mpHScroll->SetRangeMin(0);
    mpHScroll->SetRangeMax(50);
    mpHScroll->SetScrollHdl(LINK(this, ScDataTableView, HorzScrollHdl));

    mpColView->Show();
    mpRowView->Show();
    mpVScroll->Show();
    mpHScroll->Show();
}

ScDataTableView::~ScDataTableView()
{
    disposeOnce();
}

void ScDataTableView::dispose()
{
    mpColView.disposeAndClear();
    mpRowView.disposeAndClear();
    mpVScroll.disposeAndClear();
    mpHScroll.disposeAndClear();
    Control::dispose();
}

void ScDataTableView::MouseButtonDown(const MouseEvent& rMEvt)
{
    if (!rMEvt.IsLeft())
        return;

    mpMouseEvent.reset(new MouseEvent(rMEvt));
}

namespace {

SCCOL findColFromPos(sal_uInt16 nPixelPos, const ScDocument* pDoc, SCCOL nStartCol = 0)
{
    nPixelPos -= nRowHeaderWidth;
    sal_uInt32 nPixelLength = 0;
    for (SCCOL nCol : pDoc->GetColumnsRange(0, nStartCol, pDoc->MaxCol()))
    {
        sal_uInt16 nColWidth = pDoc->GetColWidth(nCol, 0, true);
        sal_uInt32 nPixel = ScViewData::ToPixel(nColWidth, nPPTX);
        nPixelLength += nPixel;

        if (nPixelLength >= nPixelPos)
        {
            return nCol;
        }
    }

    SAL_WARN("sc", "Could not find the corresponding column");
    return pDoc->MaxCol();
}

SCROW findRowFromPos(sal_uInt16 nPixelPos, const ScDocument* pDoc, SCROW nStartRow = 0)
{
    nPixelPos -= nColHeaderHeight;
    sal_uInt32 nPixelLength = 0;
    for (SCROW nRow = nStartRow; nRow <= pDoc->MaxRow(); ++nRow)
    {
        sal_uInt16 nColWidth = pDoc->GetRowHeight(nRow, SCTAB(0), true);
        sal_uInt32 nPixel = ScViewData::ToPixel(nColWidth, nPPTX);
        nPixelLength += nPixel;

        if (nPixelLength >= nPixelPos)
        {
            return nRow;
        }
    }

    SAL_WARN("sc", "Could not find the corresponding row");
    return pDoc->MaxRow();
}

}

void ScDataTableView::MouseButtonUp(const MouseEvent& rMEvt)
{
    if (!rMEvt.IsLeft())
        return;
    if (!mpMouseEvent) // tdf#120528 The event originated in another window, like context menu
        return;

    SCCOL nStartCol = findColFromPos(mpMouseEvent->GetPosPixel().getX(), mpDoc.get());
    SCCOL nEndCol = findColFromPos(rMEvt.GetPosPixel().getX(), mpDoc.get());
    SCROW nStartRow = findRowFromPos(mpMouseEvent->GetPosPixel().getY(), mpDoc.get());
    SCROW nEndRow = findRowFromPos(rMEvt.GetPosPixel().getY(), mpDoc.get());
    PutInOrder(nStartCol, nEndCol);
    PutInOrder(nStartRow, nEndRow);
    mpColView->SetMark(true, nStartCol, nEndCol);
    mpRowView->SetMark(true, nStartRow, nEndRow);

    mpMouseEvent.reset();
}

void ScDataTableView::Resize()
{
    Size aSize = GetSizePixel();
    mpColView->setPosSizePixel(nRowHeaderWidth, 0, aSize.Width() - mnScrollBarSize, nColHeaderHeight);
    mpRowView->setPosSizePixel(0, nColHeaderHeight, nRowHeaderWidth, aSize.Height());

    mpVScroll->setPosSizePixel(aSize.Width() - mnScrollBarSize, nColHeaderHeight, mnScrollBarSize, aSize.Height() - nColHeaderHeight - mnScrollBarSize);
    mpHScroll->setPosSizePixel(nRowHeaderWidth, aSize.Height() - mnScrollBarSize, aSize.Width() - nRowHeaderWidth - mnScrollBarSize, mnScrollBarSize);
}

void ScDataTableView::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRectangle)
{
    Size aSize = GetSizePixel();
    SCCOL nMaxVisibleCol = findColFromPos(aSize.Width() - mnScrollBarSize, mpDoc.get(), mnFirstVisibleCol);
    SCROW nMaxVisibleRow = findRowFromPos(aSize.Height(), mpDoc.get(), mnFirstVisibleRow);

    ScTableInfo aTableInfo;
    mpDoc->FillInfo(aTableInfo, mnFirstVisibleCol, mnFirstVisibleRow, nMaxVisibleCol, nMaxVisibleRow, 0, 0.06666, 0.06666, false, false);
    ScOutputData aOutput(&rRenderContext, OUTTYPE_WINDOW, aTableInfo, mpDoc.get(), 0,
            nRowHeaderWidth, nColHeaderHeight, mnFirstVisibleCol, mnFirstVisibleRow, nMaxVisibleCol, nMaxVisibleRow, nPPTX, nPPTY);

    aOutput.SetGridColor(COL_BLACK);
    aOutput.SetSolidBackground(true);
    aOutput.DrawClear();
    aOutput.DrawDocumentBackground();
    aOutput.DrawGrid(rRenderContext, true, false);
    aOutput.DrawStrings();

    Color aFaceColor(rRenderContext.GetSettings().GetStyleSettings().GetFaceColor());
    rRenderContext.SetLineColor(aFaceColor);
    rRenderContext.SetFillColor(aFaceColor);
    rRenderContext.DrawRect(tools::Rectangle(Point(0, 0), Size(nRowHeaderWidth, nColHeaderHeight)));
    rRenderContext.DrawRect(tools::Rectangle(Point(aSize.Width() - mnScrollBarSize, aSize.Height() - mnScrollBarSize), Size(mnScrollBarSize, mnScrollBarSize)));

    Control::Paint(rRenderContext, rRectangle);
}

Size ScDataTableView::GetOptimalSize() const
{
    return Size(450, 400);
}

void ScDataTableView::getColRange(SCCOL& rStartCol, SCCOL& rEndCol) const
{
    SCCOLROW aStart = 0;
    SCCOLROW aEnd = 0;
    mpColView->GetMarkRange(aStart, aEnd);
    rStartCol = static_cast<SCCOL>(aStart);
    rEndCol = static_cast<SCCOL>(aEnd);
}

void ScDataTableView::getRowRange(SCROW& rStartCol, SCROW& rEndCol) const
{
    SCCOLROW aStart = 0;
    SCCOLROW aEnd = 0;
    mpRowView->GetMarkRange(aStart, aEnd);
    rStartCol = static_cast<SCROW>(aStart);
    rEndCol = static_cast<SCROW>(aEnd);
}

IMPL_LINK_NOARG(ScDataTableView, VertScrollHdl, weld::Scrollbar&, void)
{
    mnFirstVisibleRow = mpVScroll->GetThumbPos();
    mpVScroll->SetRangeMax(std::min(mpDoc->MaxRow(), static_cast<SCROW>(mnFirstVisibleRow + 100)));
    mpRowView->SetPos(mnFirstVisibleRow);
    Invalidate();
}

IMPL_LINK_NOARG(ScDataTableView, HorzScrollHdl, weld::Scrollbar&, void)
{
    mnFirstVisibleCol = mpHScroll->GetThumbPos();
    mpHScroll->SetRangeMax(std::min(mpDoc->MaxCol(), static_cast<SCCOL>(mnFirstVisibleCol + 50)));
    mpColView->SetPos(mnFirstVisibleCol);
    Invalidate();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
