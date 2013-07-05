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
#include "precompiled_sfx2.hxx"
#include "sfx2/sidebar/GridLayouter.hxx"

#include <vcl/window.hxx>

namespace sfx2 { namespace sidebar {

typedef std::vector<CellDescriptor> CellData;
typedef std::vector<CellData> ColumnData;

class GridLayouter::Implementation
{
public:
    Implementation (Window& rParent);
    ~Implementation (void);

    CellDescriptor& GetCell (
        const sal_Int32 nRow,
        const sal_Int32 nColumn,
        const sal_Int32 nVariant);

    void Layout (void);
    void LayoutColumn(
        ColumnData& rColumn,
        const sal_Int32 nX,
        const sal_Int32 nColumnIndex);

    void DistributeWidth (const sal_Int32 nTotalWidth);
    sal_Int32 GetMinimumColumnWidth (
        ColumnData& rColumn,
        const ColumnDescriptor& rDescriptor) const;

    void Paint (void);

    Window& mrParent;
    ::std::vector<ColumnData> maColumns;
    ::std::vector<ColumnDescriptor> maColumnDescriptors;
};

#define ForAllColumnDescriptors(I)                              \
    for (::std::vector<ColumnDescriptor>::iterator              \
             I(maColumnDescriptors.begin()),                    \
             iEnd(maColumnDescriptors.end());                   \
         I!=iEnd;                                               \
         ++I)

#define ForAllColumns(I,N)                                      \
    sal_Int32 N (0);                                            \
    for (::std::vector<ColumnData>::iterator                    \
             I(maColumns.begin()),                              \
             iEnd(maColumns.end());                             \
         I!=iEnd;                                               \
         ++I,++N)

#define ForAllRows(ColumnData,I)                                \
    for (std::vector<CellData>::iterator                        \
             I((ColumnData).begin()),                           \
             iRowEnd((ColumnData).end());                       \
         I!=iRowEnd;                                            \
         ++I)

#define ForAllCells(CellData,I)                                 \
    for (::std::vector<CellDescriptor>::iterator                \
             I((CellData).begin()),                             \
             iCellEnd((CellData).end());                        \
         I!=iCellEnd;                                           \
         ++I)


//===== GridLayouter ==========================================================

GridLayouter::GridLayouter (Window& rParent)
    : mpImplementation(new Implementation(rParent))
{
}




GridLayouter::~GridLayouter (void)
{
}




CellDescriptor& GridLayouter::GetCell (
    const sal_Int32 nRow,
    const sal_Int32 nColumn,
    const sal_Int32 nVariant)
{
    return mpImplementation->GetCell(nRow, nColumn, nVariant);
}




ColumnDescriptor& GridLayouter::GetColumn (
    const sal_Int32 nColumn)
{
    // Make sure that the specified column exists.
    mpImplementation->GetCell(0, nColumn, 0);
    return mpImplementation->maColumnDescriptors[nColumn];
}




void GridLayouter::Layout (void)
{
    mpImplementation->Layout();
}




void GridLayouter::Paint (const Rectangle& rBox)
{
    (void)rBox;

    mpImplementation->Paint();
}




//===== CellDescriptor ========================================================

CellDescriptor::CellDescriptor (void)
    : mpControl(NULL),
      mnGridWidth(1),
      mnMinimumWidth(-1),
      mnMaximumWidth(-1),
      mnOffset(0)
{
}




CellDescriptor::~CellDescriptor (void)
{
}




CellDescriptor& CellDescriptor::SetGridWidth (const sal_Int32 nColumnCount)
{
    mnGridWidth = nColumnCount;
    return *this;
}




CellDescriptor& CellDescriptor::SetControl (Window& rControl)
{
    mpControl = &rControl;
    return *this;
}




CellDescriptor& CellDescriptor::SetFixedWidth (const sal_Int32 nWidth)
{
    mnMinimumWidth = nWidth;
    mnMaximumWidth = nWidth;
    return *this;
}



CellDescriptor& CellDescriptor::SetOffset (const sal_Int32 nOffset)
{
    mnOffset = nOffset;
    return *this;
}




CellDescriptor& CellDescriptor::SetFixedWidth (void)
{
    sal_Int32 nMaxControlWidth (0);
    if (mpControl != NULL)
    {
        const sal_Int32 nControlWidth (mpControl->GetSizePixel().Width());
        if (nControlWidth > nMaxControlWidth)
            nMaxControlWidth = nControlWidth;
    }
    mnMinimumWidth = nMaxControlWidth;
    mnMaximumWidth = nMaxControlWidth;

    return *this;
}




CellDescriptor& CellDescriptor::SetMinimumWidth (const sal_Int32 nWidth)
{
    mnMinimumWidth = nWidth;
    return *this;
}



sal_Int32 CellDescriptor::GetGridWidth (void) const
{
    return mnGridWidth;
}




Window* CellDescriptor::GetControl (void) const
{
    return mpControl;
}




sal_Int32 CellDescriptor::GetMinimumWidth (void) const
{
    return mnMinimumWidth + mnOffset;
}




sal_Int32 CellDescriptor::GetMaximumWidth (void) const
{
    return mnMaximumWidth;
}



sal_Int32 CellDescriptor::GetOffset (void) const
{
    return mnOffset;
}




//===== GridLayouter::Implementation ==========================================

GridLayouter::Implementation::Implementation (Window& rParent)
    : mrParent(rParent),
      maColumns(),
      maColumnDescriptors()
{
}




GridLayouter::Implementation::~Implementation (void)
{
}




CellDescriptor& GridLayouter::Implementation::GetCell (
    const sal_Int32 nRow,
    const sal_Int32 nColumn,
    const sal_Int32 nVariant)
{
    if (nColumn<0 || nRow<0 || nVariant<0)
    {
        OSL_ASSERT(nColumn>=0);
        OSL_ASSERT(nRow>=0);
        OSL_ASSERT(nVariant>=0);
        return GetCell(0,0,0);
    }

    // Provide missing columns.
    if (maColumns.size() <= static_cast<size_t>(nColumn))
    {
        maColumns.resize(nColumn+1);
        maColumnDescriptors.resize(nColumn+1);
    }

    // Provide missing rows.
    ColumnData& rColumn (maColumns[nColumn]);
    if (rColumn.size() <= static_cast<size_t>(nRow))
        rColumn.resize(nRow+1);

    // Provide missing variants.
    CellData& rCellData (rColumn[nRow]);
    if (rCellData.size() <= static_cast<size_t>(nVariant))
        rCellData.resize(nVariant+1);

    return rCellData[nVariant];
}




void GridLayouter::Implementation::Layout (void)
{
    if (maColumns.empty())
    {
        // There are no columns and therefore no controls => nothing
        // to do.
        return;
    }

    const Size aParentSize (mrParent.GetSizePixel());

    // Determine the total column weight.
    sal_Int32 nTotalColumnWeight (0);
    ForAllColumnDescriptors(iDescriptor)
        nTotalColumnWeight += iDescriptor->GetWeight();
    if (nTotalColumnWeight <= 0)
    {
        OSL_ASSERT(nTotalColumnWeight>0);
        return;
    }

    // Distribute the width of the parent window to the columns.
    DistributeWidth(aParentSize.Width());

    // Set the new positions and widths.
    sal_Int32 nX (0);
    ForAllColumns(iColumn,nColumnIndex)
    {
        LayoutColumn(
            *iColumn,
            nX,
            nColumnIndex);

        nX += maColumnDescriptors[nColumnIndex].GetWidth();
    }
}




void GridLayouter::Implementation::LayoutColumn(
    ColumnData& rColumn,
    const sal_Int32 nX,
    const sal_Int32 nColumnIndex)
{
    ColumnDescriptor& rDescriptor (maColumnDescriptors[nColumnIndex]);
    const sal_Int32 nLeft (nX + rDescriptor.GetLeftPadding());
    const sal_Int32 nWidth (rDescriptor.GetWidth() - rDescriptor.GetLeftPadding() - rDescriptor.GetRightPadding());

    sal_Int32 nRow (-1);
    ForAllRows(rColumn, iCell)
    {
        ++nRow;

        ForAllCells(*iCell, iCellDescriptor)
        {
            Window* pControl = iCellDescriptor->GetControl();
            if (pControl==NULL || ! pControl->IsVisible())
                continue;

            sal_Int32 nCellWidth (nWidth);
            const sal_Int32 nGridWidth (iCellDescriptor->GetGridWidth());
            if (nGridWidth < 0)
                continue;
            else if (nGridWidth > 1)
            {
                // Cell spans more than one column.  Sum all their
                // widths.
                for (sal_Int32 nOffset=1;
                     nOffset<nGridWidth && static_cast<size_t>(nColumnIndex+nOffset)<maColumnDescriptors.size();
                     ++nOffset)
                {
                    nCellWidth += maColumnDescriptors[nColumnIndex+nOffset].GetWidth();
                }
                nCellWidth -= maColumnDescriptors[nColumnIndex+nGridWidth-1].GetRightPadding();
            }

            // Check width against valid range of cell.
            if (iCellDescriptor->GetMinimumWidth() > 0)
                if (nCellWidth < iCellDescriptor->GetMinimumWidth())
                    nCellWidth = iCellDescriptor->GetMinimumWidth();
            if (iCellDescriptor->GetMaximumWidth() > 0)
                if (nCellWidth > iCellDescriptor->GetMaximumWidth())
                    nCellWidth = iCellDescriptor->GetMaximumWidth();

            pControl->SetPosSizePixel(
                nLeft + iCellDescriptor->GetOffset(),
                0,
                nCellWidth,
                0,
                WINDOW_POSSIZE_X | WINDOW_POSSIZE_WIDTH);
        }
    }
}




void GridLayouter::Implementation::DistributeWidth (const sal_Int32 nTotalWidth)
{
    // Prepare width distribution:
    // a) Setup minimum widths for all columns.
    // b) Sum up the width of columns that have zero weight.
    // c) Sum up the non-zero weights.
    sal_Int32 nZeroWeightWidth (0);
    sal_Int32 nTotalColumnWeight (0);
    for (sal_uInt32 nColumn=0; nColumn<maColumns.size(); ++nColumn)
    {
        ColumnDescriptor& rDescriptor (maColumnDescriptors[nColumn]);
        ColumnData& rColumn (maColumns[nColumn]);

        const sal_Int32 nWidth (GetMinimumColumnWidth(rColumn, rDescriptor));

        rDescriptor.SetWidth(nWidth);

        if (rDescriptor.GetWeight() <= 0)
            nZeroWeightWidth += nWidth;
        else
            nTotalColumnWeight += rDescriptor.GetWeight();
    }

    sal_Int32 nRemainingWidth (nTotalWidth - nZeroWeightWidth);
    if (nRemainingWidth < 0)
        nRemainingWidth = 0;


    // Distribute the remaining width between columns that have
    // non-zero width.
    const sal_Int32 nDistributableWidth (nRemainingWidth);
    for (sal_uInt32 nColumn=0; nColumn<maColumns.size(); ++nColumn)
    {
        ColumnDescriptor& rDescriptor (maColumnDescriptors[nColumn]);

        if (rDescriptor.GetWeight() > 0)
        {
            sal_Int32 nWidth (nDistributableWidth * rDescriptor.GetWeight() / nTotalColumnWeight);
            // Make sure the width lies inside the valid range of
            // column widths.
            if (nWidth < rDescriptor.GetWidth())
                nWidth = rDescriptor.GetWidth();
            if (rDescriptor.GetMaximumWidth()>0)
                if (nWidth > rDescriptor.GetTotalMaximumWidth())
                    nWidth = rDescriptor.GetTotalMaximumWidth();

            rDescriptor.SetWidth(nWidth);
            nRemainingWidth -= nWidth;
        }
    }

    // If there are some pixels left (due to rounding errors), then
    // give them to the first column that has non-zero weight.
    if (nRemainingWidth > 0)
        for (sal_uInt32 nColumn=0; nColumn<maColumns.size(); ++nColumn)
        {
            ColumnDescriptor& rDescriptor (maColumnDescriptors[nColumn]);
            if (rDescriptor.GetWeight() > 0)
            {
                rDescriptor.SetWidth(rDescriptor.GetWidth() + nRemainingWidth);
                break;
            }
        }
}




sal_Int32 GridLayouter::Implementation::GetMinimumColumnWidth (
    ColumnData& rColumn,
    const ColumnDescriptor& rDescriptor) const
{
    // Start with the minimum width of the whole column.
    sal_Int32 nMinimumWidth (rDescriptor.GetMinimumWidth());

    // Take also into account the minimum widths of all cells in the column.
    ForAllRows(rColumn, iCell)
        ForAllCells(*iCell, iCellDescriptor)
        {
            if (iCellDescriptor->GetGridWidth() != 1)
                continue;
            const sal_Int32 nMinimumCellWidth (iCellDescriptor->GetMinimumWidth());
            if (nMinimumCellWidth > nMinimumWidth)
                nMinimumWidth = nMinimumCellWidth;
        }

    // Make sure that the minimum width does not become larger than
    // the maximum width of the column.
    if (nMinimumWidth > rDescriptor.GetMaximumWidth() && rDescriptor.GetMaximumWidth()>0)
        nMinimumWidth = rDescriptor.GetMaximumWidth();

    // Add the horizontal padding.
    return  nMinimumWidth
        + rDescriptor.GetLeftPadding()
        + rDescriptor.GetRightPadding();
}




void GridLayouter::Implementation::Paint (void)
{
    const Size aParentSize (mrParent.GetSizePixel());

    static const Color aSeparatorColor (0x66cdaa);
    static const Color aLeftPaddingColor (0x98fb98);
    static const Color aRightPaddingColor (0xff69b4);
    static const Color aControlOverlayColor (0xffff00);

    sal_Int32 nX (0);
    mrParent.SetLineColor();
    mrParent.SetFillColor(aLeftPaddingColor);
    ForAllColumnDescriptors(iColumn)
    {
        if (iColumn->GetLeftPadding() > 0)
        {
            mrParent.DrawRect(Rectangle(
                    nX,0,
                    nX+iColumn->GetLeftPadding(),aParentSize.Height()));
        }

        nX += iColumn->GetWidth();
    }

    nX = 0;
    mrParent.SetFillColor(aRightPaddingColor);
    ForAllColumnDescriptors(iColumn)
    {
        if (iColumn->GetRightPadding() > 0)
        {
            const sal_Int32 nRight (nX + iColumn->GetWidth());
            const sal_Int32 nLeft (nRight - iColumn->GetRightPadding());
            mrParent.DrawRect(Rectangle(
                    nLeft,0,
                    nRight,aParentSize.Height()));
        }

        nX += iColumn->GetWidth();
    }

    nX = 0;
    mrParent.SetFillColor();
    mrParent.SetLineColor(aSeparatorColor);
    ForAllColumnDescriptors(iColumn)
    {
        mrParent.DrawLine(Point(nX,0), Point(nX,aParentSize.Height()));
        nX += iColumn->GetWidth();
    }

    mrParent.SetFillColor();
    mrParent.SetLineColor(aControlOverlayColor);
    ForAllColumns(iColumn,nColumnIndex)
        ForAllRows(*iColumn, iCell)
            ForAllCells(*iCell, iCellDescriptor)
            {
                Window* pControl (iCellDescriptor->GetControl());
                if (pControl!=NULL && pControl->IsVisible())
                {
                    Rectangle aBox (
                        pControl->GetPosPixel(),
                        pControl->GetSizePixel());
                    --aBox.Left();
                    --aBox.Top();
                    ++aBox.Right();
                    ++aBox.Bottom();
                    mrParent.DrawRect(aBox);
                }
            }
}




//===== ColumnDescriptor ======================================================

ColumnDescriptor::ColumnDescriptor (void)
    : mnWeight(1),
      mnMinimumWidth(0),
      mnMaximumWidth(-1),
      mnLeftPadding(0),
      mnRightPadding(0),
      mnWidth(0)
{
}




ColumnDescriptor::~ColumnDescriptor (void)
{
}




ColumnDescriptor& ColumnDescriptor::SetWeight (const sal_Int32 nWeight)
{
    mnWeight = nWeight;

    return *this;
}




ColumnDescriptor& ColumnDescriptor::SetMinimumWidth (const sal_Int32 nWidth)
{
    mnMinimumWidth = nWidth;

    return *this;
}



ColumnDescriptor& ColumnDescriptor::SetFixedWidth (const sal_Int32 nWidth)
{
    mnMinimumWidth = nWidth;
    mnMaximumWidth = nWidth;

    return *this;
}



ColumnDescriptor& ColumnDescriptor::SetLeftPadding (const sal_Int32 nPadding)
{
    mnLeftPadding = nPadding;

    return *this;
}




ColumnDescriptor& ColumnDescriptor::SetRightPadding (const sal_Int32 nPadding)
{
    mnRightPadding = nPadding;

    return *this;
}




sal_Int32 ColumnDescriptor::GetWeight (void) const
{
    return mnWeight;
}




sal_Int32 ColumnDescriptor::GetMinimumWidth (void) const
{
    return mnMinimumWidth;
}




sal_Int32 ColumnDescriptor::GetMaximumWidth (void) const
{
    return mnMaximumWidth;
}




sal_Int32 ColumnDescriptor::GetTotalMaximumWidth (void) const
{
    return mnMaximumWidth + mnLeftPadding + mnRightPadding;
}




sal_Int32 ColumnDescriptor::GetLeftPadding (void) const
{
    return mnLeftPadding;
}




sal_Int32 ColumnDescriptor::GetRightPadding (void) const
{
    return mnRightPadding;
}




void ColumnDescriptor::SetWidth (const sal_Int32 nWidth)
{
    mnWidth = nWidth;
}




sal_Int32 ColumnDescriptor::GetWidth (void) const
{
    return mnWidth;
}

} } // end of namespace sfx2::sidebar
