/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SlsLayouter.cxx,v $
 * $Revision: 1.11 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "precompiled_sd.hxx"

#include "view/SlsLayouter.hxx"

#include "Window.hxx"
#include <basegfx/numeric/ftools.hxx>


namespace sd { namespace slidesorter { namespace view {

Layouter::Layouter (const SharedSdWindow& rpWindow)
    : mpWindow(rpWindow),
      mnRequestedLeftBorder(35),
      mnRequestedRightBorder(35),
      mnRequestedTopBorder(10),
      mnRequestedBottomBorder(10),
      mnLeftBorder(30),
      mnRightBorder(30),
      mnTopBorder(10),
      mnBottomBorder(10),
      mnVerticalGap (20),
      mnHorizontalGap (20),
      mnMinimalWidth (100),
      mnPreferredWidth (200),
      mnMaximalWidth (300),
      mnMinimalColumnCount (1),
      mnMaximalColumnCount (5),
      mnPageCount(0),
      mnColumnCount(1),
      mnRowCount(0),
      maPageObjectSize(1,1),
      mbIsVertical(true)
{
}




Layouter::~Layouter (void)
{
}




::boost::shared_ptr<PageObjectLayouter> Layouter::GetPageObjectLayouter (void) const
{
    return mpPageObjectLayouter;
}




void Layouter::SetObjectWidth (
    sal_Int32 nMinimalWidth,
    sal_Int32 nMaximalWidth,
    sal_Int32 nPreferredWidth)
{
    if (nMinimalWidth <= nPreferredWidth && nPreferredWidth <= nMaximalWidth)
    {
        mnMinimalWidth = nMinimalWidth;
        mnPreferredWidth = nMaximalWidth;
        mnMaximalWidth = nPreferredWidth;
    }
}




void Layouter::SetBorders (
    sal_Int32 nLeftBorder,
    sal_Int32 nRightBorder,
    sal_Int32 nTopBorder,
    sal_Int32 nBottomBorder)
{
    if (nLeftBorder >= 0)
        mnRequestedLeftBorder = nLeftBorder;
    if (nRightBorder >= 0)
        mnRequestedRightBorder = nRightBorder;
    if (nTopBorder >= 0)
        mnRequestedTopBorder = nTopBorder;
    if (nBottomBorder >= 0)
        mnRequestedBottomBorder = nBottomBorder;
}




void Layouter::SetGaps (
    sal_Int32 nHorizontalGap,
    sal_Int32 nVerticalGap)
{
    if (nHorizontalGap >= 0)
        mnHorizontalGap = nHorizontalGap;
    if (nVerticalGap >= 0)
        mnVerticalGap = nVerticalGap;
}





void Layouter::SetColumnCount (
    sal_Int32 nMinimalColumnCount,
        sal_Int32 nMaximalColumnCount)
{
    if (nMinimalColumnCount <= nMaximalColumnCount)
    {
        mnMinimalColumnCount = nMinimalColumnCount;
        mnMaximalColumnCount = nMaximalColumnCount;
    }
}




bool Layouter::RearrangeHorizontal (
    const Size& rWindowSize,
    const Size& rPageSize,
    const sal_uInt32 nPageCount)
{
    OSL_ASSERT(mpWindow);

    mbIsVertical = false;
    mnPageCount = nPageCount;

    if (rWindowSize.Width() > 0
        && rWindowSize.Height() > 0
        && rPageSize.Width() > 0
        && rPageSize.Height() > 0)
    {
        // Calculate the column count.
        mnColumnCount = nPageCount;
        mnRowCount = 1;

        // Update the border values.
        mnLeftBorder = mnRequestedLeftBorder;
        mnTopBorder = mnRequestedTopBorder;
        mnRightBorder = mnRequestedRightBorder;
        mnBottomBorder = mnRequestedBottomBorder;
        if (mnColumnCount > 1)
        {
            int nMinimumBorderWidth = mnHorizontalGap/2;
            if (mnLeftBorder < nMinimumBorderWidth)
                mnLeftBorder = nMinimumBorderWidth;
            if (mnRightBorder < nMinimumBorderWidth)
                mnRightBorder = nMinimumBorderWidth;
        }
        else
        {
            int nMinimumBorderHeight = mnVerticalGap/2;
            if (mnTopBorder < nMinimumBorderHeight)
                mnTopBorder = nMinimumBorderHeight;
            if (mnBottomBorder < nMinimumBorderHeight)
                mnBottomBorder = nMinimumBorderHeight;
        }

        // Calculate the width of each page object.
        sal_uInt32 nTargetHeight = 0;
        sal_uInt32 nRowCount = 1;
        if (mnColumnCount > 0)
            nTargetHeight = (rWindowSize.Height()
                - mnTopBorder
                - mnBottomBorder
                - (nRowCount-1) * mnVerticalGap
                )
            / nRowCount;
        sal_uInt32 nMinimalHeight (mnMinimalWidth * rPageSize.Height() / rPageSize.Width());
        sal_uInt32 nMaximalHeight (mnMaximalWidth * rPageSize.Height() / rPageSize.Width());
        if (nTargetHeight < nMinimalHeight)
            nTargetHeight = nMinimalHeight;
        if (nTargetHeight > nMaximalHeight)
            nTargetHeight = nMaximalHeight;

        // Setup the page object layouter and ask it for the page object size.
        mpPageObjectLayouter.reset(
            new PageObjectLayouter(
                Size(0, nTargetHeight),
                rPageSize,
                mpWindow,
                nPageCount));
        maPageObjectSize = mpPageObjectLayouter->GetPageObjectSize();

        return true;
    }
    else
        return false;
}




bool Layouter::RearrangeVertical (
    const Size& rWindowSize,
    const Size& rPreviewModelSize,
    const sal_uInt32 nPageCount)
{
    OSL_ASSERT(mpWindow);

    mbIsVertical = true;
    mnPageCount = nPageCount;

    if (rWindowSize.Width() > 0
        && rWindowSize.Height() > 0
        && rPreviewModelSize.Width() > 0
        && rPreviewModelSize.Height() > 0)
    {
        // Calculate the column count.
        mnColumnCount = (rWindowSize.Width() - mnRequestedLeftBorder - mnRequestedRightBorder)
            / (mnPreferredWidth  + mnHorizontalGap);
        if (mnColumnCount < mnMinimalColumnCount)
            mnColumnCount = mnMinimalColumnCount;
        if (mnColumnCount > mnMaximalColumnCount)
            mnColumnCount = mnMaximalColumnCount;
        mnRowCount = (nPageCount + mnColumnCount-1)/mnColumnCount;

        // Update the border values.
        mnLeftBorder = mnRequestedLeftBorder;
        mnTopBorder = mnRequestedTopBorder;
        mnRightBorder = mnRequestedRightBorder;
        mnBottomBorder = mnRequestedBottomBorder;
        if (mnColumnCount > 1)
        {
            int nMinimumBorderWidth = mnHorizontalGap/2;
            if (mnLeftBorder < nMinimumBorderWidth)
                mnLeftBorder = nMinimumBorderWidth;
            if (mnRightBorder < nMinimumBorderWidth)
                mnRightBorder = nMinimumBorderWidth;
        }
        else
        {
            int nMinimumBorderHeight = mnVerticalGap/2;
            if (mnTopBorder < nMinimumBorderHeight)
                mnTopBorder = nMinimumBorderHeight;
            if (mnBottomBorder < nMinimumBorderHeight)
                mnBottomBorder = nMinimumBorderHeight;
        }

        // Calculate the width of each page object.
        sal_Int32 nTargetWidth = 0;
        if (mnColumnCount > 0)
            nTargetWidth = (rWindowSize.Width()
                - mnLeftBorder
                - mnRightBorder
                - (mnColumnCount-1) * mnHorizontalGap
                )
                / mnColumnCount;
        if (nTargetWidth < mnMinimalWidth)
            nTargetWidth = mnMinimalWidth;
        if (nTargetWidth > mnMaximalWidth)
            nTargetWidth = mnMaximalWidth;

        // Setup the page object layouter and ask it for the page object size.
        mpPageObjectLayouter.reset(
            new PageObjectLayouter(
                Size(nTargetWidth, 0),
                rPreviewModelSize,
                mpWindow,
                nPageCount));
        maPageObjectSize = mpPageObjectLayouter->GetPageObjectSize();

        return true;
    }
    else
        return false;
}




void Layouter::SetZoom (double nZoomFactor)
{
    SetZoom(Fraction(nZoomFactor));
}




void Layouter::SetZoom (Fraction nZoomFactor)
{
    OSL_ASSERT(mpWindow);

    MapMode aMapMode (mpWindow->GetMapMode());
    aMapMode.SetScaleX (nZoomFactor);
    aMapMode.SetScaleY (nZoomFactor);
    //    maPageObjectPixelSize = mpWindow->LogicToPixel (maPageObjectModelSize);
    mpWindow->SetMapMode (aMapMode);
}




sal_Int32 Layouter::GetColumnCount (void) const
{
    return mnColumnCount;
}




sal_Int32 Layouter::GetRowCount (void) const
{
    return mnRowCount;
}




sal_Int32 Layouter::GetRow (const sal_Int32 nIndex) const
{
    return nIndex / mnColumnCount;
}




sal_Int32 Layouter::GetColumn (const sal_Int32 nIndex) const
{
    return nIndex % mnColumnCount;
}




sal_Int32 Layouter::GetIndex (const sal_Int32 nRow, const sal_Int32 nColumn) const
{
    const sal_Int32 nIndex (nRow * mnColumnCount + nColumn);
    OSL_ASSERT(nIndex>=0);
    return ::std::min(nIndex, mnPageCount-1);
}




bool Layouter::IsColumnCountFixed (void) const
{
    return mnMinimalColumnCount == mnMaximalColumnCount;
}




Size Layouter::GetPageObjectSize (void) const
{
    return maPageObjectSize;
}




Rectangle Layouter::GetPageObjectBox (
    const sal_Int32 nIndex,
    const bool bIncludeBorderAndGap) const
{
    const sal_Int32 nRow (GetRow(nIndex));
    const sal_Int32 nColumn (GetColumn(nIndex));
    Rectangle aBoundingBox(
        Point (mnLeftBorder
            + nColumn * maPageObjectSize.Width()
            + (nColumn>0 ? nColumn : 0) * mnHorizontalGap,
            mnTopBorder
            + nRow * maPageObjectSize.Height()
            + (nRow>0 ? nRow : 0) * mnVerticalGap),
        maPageObjectSize);
    if (bIncludeBorderAndGap)
    {
        if (nColumn == 0)
            aBoundingBox.Left() = 0;
        else
            aBoundingBox.Left() -= mnHorizontalGap/2;
        if (nColumn == mnColumnCount-1)
            aBoundingBox.Right() += mnRightBorder;
        else
            aBoundingBox.Right() += mnHorizontalGap/2;
        if (nRow == 0)
            aBoundingBox.Top() = 0;
        else
            aBoundingBox.Top() -= mnVerticalGap/2;
        if (nRow == mnRowCount-1)
            aBoundingBox.Bottom() += mnBottomBorder;
        else
            aBoundingBox.Bottom() += mnVerticalGap/2;
    }
    return aBoundingBox;
}




Rectangle Layouter::GetPageBox (const sal_Int32 nObjectCount) const
{
    sal_Int32 nCount (nObjectCount);
    if (nCount < 0)
        nCount = mnPageCount;

    sal_Int32 nHorizontalSize = 0;
    sal_Int32 nVerticalSize = 0;
    if (mnColumnCount > 0)
    {
        sal_Int32 nRowCount = (nCount+mnColumnCount-1) / mnColumnCount;
        nHorizontalSize =
            mnLeftBorder
            + mnRightBorder
            + mnColumnCount * maPageObjectSize.Width();
        if (mnColumnCount > 1)
            nHorizontalSize +=  (mnColumnCount-1) * mnHorizontalGap;
        nVerticalSize =
            mnTopBorder
            + mnBottomBorder
            + nRowCount * maPageObjectSize.Height();
        if (nRowCount > 1)
            nVerticalSize += (nRowCount-1) * mnVerticalGap;
    }

    return Rectangle (
        Point(0,0),
        Size (nHorizontalSize, nVerticalSize)
        );
}




Point Layouter::GetInsertionMarkerLocation (
    sal_Int32 nIndex,
    bool bVertical,
    bool bLeftOrTop) const
{
    Rectangle aBox (GetPageObjectBox (nIndex));
    Point aLocation = aBox.Center();

    if (bVertical)
    {
        if (bLeftOrTop)
        {
            // Left.
            aLocation.setX(aBox.Left() - (mnHorizontalGap+1)/2 - 1);
        }
        else
        {
            // Right.
            aLocation.setX(aBox.Right() + mnHorizontalGap/2);
        }
    }
    else
    {
        if (bLeftOrTop)
        {
            // Above.
            aLocation.setY(aBox.Top() - mnVerticalGap/2);
        }
        else
        {
            // Below.
            aLocation.setY(aBox.Bottom() + mnVerticalGap/2);
        }
    }

    return aLocation;
}




Range Layouter::GetRangeOfVisiblePageObjects (const Rectangle& aVisibleArea) const
{
    sal_Int32 nRow0 = GetRowAtPosition (aVisibleArea.Top(), true, GM_BOTH);
    sal_Int32 nRow1 = GetRowAtPosition (aVisibleArea.Bottom(),
        true, GM_BOTH);
    return Range(
        ::std::min(nRow0 * mnColumnCount, mnPageCount-1),
        ::std::min((nRow1+1) * mnColumnCount - 1, mnPageCount-1));
}




Range Layouter::GetSelectionRange (
    const Point& rAnchor,
    const Point& rOther,
    const Range& rCurrentSelectionRange) const
{
    (void)rCurrentSelectionRange;
    sal_Int32 nIndexA (GetInsertionIndex(rAnchor, true));
    sal_Int32 nIndexB (GetInsertionIndex(rOther, true));

    // When either of the locations does not lie over a page then we may
    // have to adapt the start or end index.
    Rectangle aBoxA (GetPageObjectBox(nIndexA));
    Rectangle aBoxB (GetPageObjectBox(nIndexB));
    if (nIndexA > nIndexB)
    {
        ::std::swap(nIndexA,nIndexB);
        ::std::swap(aBoxA, aBoxB);
    }

    if (mnColumnCount == 1)
    {
        // Vertical arrangement of pages.
        if (nIndexA <= nIndexB)
        {
            if (rOther.Y() < aBoxB.Top())
                --nIndexB;
        }
        else
        {
            if (rOther.Y() > aBoxB.Bottom())
                --nIndexA;
        }
    }
    else
    {
        // Horizontal arrangement of pages.
        if (nIndexA <= nIndexB)
        {
            if (rOther.X() < aBoxB.Left())
                --nIndexB;
        }
        else
        {
            if (rOther.X() > aBoxB.Right())
                --nIndexA;
        }
    }

    return Range(
        ::std::min(nIndexA, nIndexB),
        ::std::min(mnPageCount-1, ::std::max(nIndexA, nIndexB)));
}




sal_Int32 Layouter::GetIndexAtPoint (
    const Point& rPosition,
    const bool bIncludePageBorders) const
{
    sal_Int32 nRow = GetRowAtPosition (rPosition.Y(),
        bIncludePageBorders,
        bIncludePageBorders ? GM_PAGE_BORDER : GM_NONE);
    sal_Int32 nColumn = GetColumnAtPosition (rPosition.X(),
        bIncludePageBorders,
        bIncludePageBorders ? GM_PAGE_BORDER : GM_NONE);

    if (nRow >= 0 && nColumn >= 0)
        return nRow * mnColumnCount + nColumn;
    else
        return -1;
}




/** Calculation of the insertion index:
    1. Determine the row.  rPoint has to be in the row between upper and
    lower border.  If it is in a horizontal gap or border an invalid
    insertion index (-1, which is a valid return value) will be returned.
    2. Determine the column.  Here both vertical borders and vertical gaps
    will yield valid return values.  The horizontal positions between the
    center of page objects in column i and the center of page objects in
    column i+1 will return column i+1 as insertion index.

    When there is only one column and bAllowVerticalPosition is true than
    take the vertical areas between rows into account as well.
*/
sal_Int32 Layouter::GetInsertionIndex (
    const Point& rPosition,
    bool bAllowVerticalPosition) const
{
    sal_Int32 nIndex = -1;

    sal_Int32 nRow = GetRowAtPosition (rPosition.Y(), true,
        (mnColumnCount==1 && bAllowVerticalPosition) ? GM_BOTH : GM_BOTH);
    sal_Int32 nColumn = GetColumnAtPosition (rPosition.X(), true, GM_BOTH);

    if (nRow >= 0 && nColumn >= 0)
        nIndex = nRow * mnColumnCount + nColumn;

    return nIndex;
}




bool Layouter::IsVertical (void) const
{
    return mbIsVertical;
}




sal_Int32 Layouter::GetRowAtPosition (
    sal_Int32 nYPosition,
    bool bIncludeBordersAndGaps,
    GapMembership eGapMembership) const
{
    sal_Int32 nRow = -1;

    const sal_Int32 nY = nYPosition - mnTopBorder;
    if (nY >= 0)
    {
        // Vertical distance from one row to the next.
        const sal_Int32 nRowOffset (maPageObjectSize.Height() + mnVerticalGap);

        // Calculate row consisting of page objects and gap below.
        nRow = nY / nRowOffset;

        const sal_Int32 nDistanceIntoGap ((nY - nRow*nRowOffset) - maPageObjectSize.Height());
        // When inside the gap below then nYPosition is not over a page
        // object.
        if (nDistanceIntoGap > 0)
            nRow = ResolvePositionInGap (
                nDistanceIntoGap,
                eGapMembership,
                nRow,
                mnVerticalGap);
    }
    else if (bIncludeBordersAndGaps)
    {
        // We are in the top border area.  Set nRow to the first row when
        // the top border shall be considered to belong to the first row.
        nRow = 0;
    }

    return nRow;
}




sal_Int32 Layouter::GetColumnAtPosition (
    sal_Int32 nXPosition,
    bool bIncludeBordersAndGaps,
    GapMembership eGapMembership) const
{
    sal_Int32 nColumn = -1;

    sal_Int32 nX = nXPosition - mnLeftBorder;
    if (nX >= 0)
    {
        // Horizontal distance from one column to the next.
        const sal_Int32 nColumnOffset (maPageObjectSize.Width() + mnHorizontalGap);

        // Calculate row consisting of page objects and gap below.
        nColumn = nX / nColumnOffset;
        if (nColumn < 0)
            nColumn = 0;
        else if (nColumn >= mnColumnCount)
            nColumn = mnColumnCount-1;

        const sal_Int32 nDistanceIntoGap ((nX - nColumn*nColumnOffset) - maPageObjectSize.Width());
        // When inside the gap at the right then nXPosition is not over a
        // page object.
        if (nDistanceIntoGap > 0)
            nColumn = ResolvePositionInGap (
                nDistanceIntoGap,
                eGapMembership,
                nColumn,
                mnHorizontalGap);
    }
    else if (bIncludeBordersAndGaps)
    {
        // We are in the left border area.  Set nColumn to the first column
        // when the left border shall be considered to belong to the first
        // column.
        nColumn = 0;
    }
    return nColumn;
}




sal_Int32 Layouter::ResolvePositionInGap (
    sal_Int32 nDistanceIntoGap,
    GapMembership eGapMembership,
    sal_Int32 nIndex,
    sal_Int32 nGap) const
{
    switch (eGapMembership)
    {
        case GM_NONE:
            // The gap is no man's land.
            nIndex = -1;
            break;

        case GM_BOTH:
        {
            // The lower half of the gap belongs to the next row or column.
            sal_Int32 nFirstHalfGapWidth = nGap / 2;
            if (nDistanceIntoGap > nFirstHalfGapWidth)
                nIndex ++;
            break;
        }

        case GM_PREVIOUS:
            // Row or column already at correct value.
            break;

        case GM_NEXT:
            // The complete gap belongs to the next row or column.
            nIndex ++;
            break;

        case GM_PAGE_BORDER:
            if (nDistanceIntoGap > 0)
            {
                if (nDistanceIntoGap > nGap)
                {
                    // Inside the border of the next row or column.
                    nIndex ++;
                }
                else
                {
                    // Inside the gap between the page borders.
                    nIndex = -1;
                }
            }
            break;

        default:
            nIndex = -1;
    }

    return nIndex;
}




} } } // end of namespace ::sd::slidesorter::namespace
