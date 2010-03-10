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
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"

#include "Window.hxx"
#include <basegfx/numeric/ftools.hxx>


namespace sd { namespace slidesorter { namespace view {

Layouter::Layouter (const SharedSdWindow& rpWindow)
    : mpWindow(rpWindow),
      mnRequestedLeftBorder(5),
      mnRequestedRightBorder(5),
      mnRequestedTopBorder(5),
      mnRequestedBottomBorder(5),
      mnLeftBorder(5),
      mnRightBorder(5),
      mnTopBorder(5),
      mnBottomBorder(5),
      mnVerticalGap (10),
      mnHorizontalGap(10),
      mnMinimalWidth(100),
      mnPreferredWidth(200),
      mnMaximalWidth(300),
      mnMinimalColumnCount(1),
      mnMaximalColumnCount(4),
      mnPageCount(0),
      mnColumnCount(1),
      mnRowCount(0),
      mnMaxColumnCount(0),
      mnMaxRowCount(0),
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

        mnMaxColumnCount = (rWindowSize.Width() - mnLeftBorder - mnRightBorder)
            / (maPageObjectSize.Width()  + mnHorizontalGap);
        mnMaxRowCount = 1;

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

        mnMaxColumnCount = (rWindowSize.Width() - mnLeftBorder - mnRightBorder)
            / (maPageObjectSize.Width()  + mnHorizontalGap);
        mnMaxRowCount = (rWindowSize.Height() - mnTopBorder - mnBottomBorder)
            / (maPageObjectSize.Height()  + mnVerticalGap);

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




InsertPosition Layouter::GetInsertPosition (
    const Point& rModelPosition,
    const Size& rIndicatorSize,
    model::SlideSorterModel& rModel) const
{
    InsertPosition aPosition;
    CalculateLogicalInsertPosition(rModelPosition, aPosition);
    CalculateGeometricPosition(aPosition, rIndicatorSize, GetColumnCount()==1, rModel);
    return aPosition;
}




void Layouter::CalculateLogicalInsertPosition (
    const Point& rModelPosition,
    InsertPosition& rPosition) const
{
    // Determine logical position.
    if (mnColumnCount == 1)
    {
        const sal_Int32 nY = rModelPosition.Y() - mnTopBorder + maPageObjectSize.Height()/2;
        const sal_Int32 nRowHeight (maPageObjectSize.Height() + mnVerticalGap);
        rPosition.mnRow = ::std::min(mnPageCount, nY / nRowHeight);
        rPosition.mnColumn = 0;
        rPosition.mnIndex = rPosition.mnRow;
        rPosition.mbIsAtRunStart = (rPosition.mnRow == 0);
        rPosition.mbIsAtRunEnd = (rPosition.mnRow == mnRowCount);
        rPosition.mbIsExtraSpaceNeeded = (rPosition.mnRow >= mnMaxRowCount);
    }
    else
    {
        rPosition.mnRow = ::std::min(
            mnRowCount-1,
            GetRowAtPosition (rModelPosition.Y(), true, GM_BOTH));

        const sal_Int32 nX = rModelPosition.X() - mnLeftBorder + maPageObjectSize.Width()/2;
        const sal_Int32 nRowWidth (maPageObjectSize.Width() + mnHorizontalGap);
        rPosition.mnColumn = ::std::min(mnColumnCount, nX / nRowWidth);
        rPosition.mnIndex = rPosition.mnRow * mnColumnCount + rPosition.mnColumn;

        rPosition.mbIsAtRunStart = (rPosition.mnColumn == 0);
        rPosition.mbIsAtRunEnd = (rPosition.mnColumn == mnColumnCount);

        if (rPosition.mnIndex >= mnPageCount)
        {
            rPosition.mnIndex = mnPageCount;
            rPosition.mnRow = mnRowCount-1;
            rPosition.mnColumn = mnPageCount%mnColumnCount;
            rPosition.mbIsAtRunEnd = true;
        }
        rPosition.mbIsExtraSpaceNeeded = (rPosition.mnColumn >= mnMaxColumnCount);
    }
}




void Layouter::CalculateGeometricPosition (
    InsertPosition& rPosition,
    const Size& rIndicatorSize,
    const bool bIsVertical,
    model::SlideSorterModel& rModel) const
{
    // 1. Determine right/bottom of the leading page and the left/top of the
    // trailing page object and how to distribute the missing space.
    sal_Int32 nLeadingLocation (0);
    sal_Int32 nTrailingLocation (0);
    bool bIsLeadingFixed (false);
    bool bIsTrailingFixed (false);
    sal_Int32 nSecondaryLocation (0);

    if (rPosition.mbIsAtRunStart)
    {
        // Place indicator at the top of the column.
        const Rectangle aOuterBox (GetPageObjectBox(rPosition.mnIndex));
        const Rectangle aInnerBox (GetInnerBoundingBox(rModel, rPosition.mnIndex));
        if (bIsVertical)
        {
            nLeadingLocation = aOuterBox.Top();
            nTrailingLocation = aInnerBox.Top();
            nSecondaryLocation = aInnerBox.Center().X();
        }
        else
        {
            nLeadingLocation = aOuterBox.Left();
            nTrailingLocation = aInnerBox.Left();
            nSecondaryLocation = aInnerBox.Center().Y();
        }
        bIsLeadingFixed = true;
    }
    else if (rPosition.mbIsAtRunEnd)
    {
        // Place indicator at the bottom/right of the column/row.

        const Rectangle aOuterBox (GetPageObjectBox(rPosition.mnIndex-1));
        const Rectangle aInnerBox (GetInnerBoundingBox(rModel, rPosition.mnIndex-1));
        if (bIsVertical)
        {
            nLeadingLocation = aInnerBox.Bottom();
            nTrailingLocation = aOuterBox.Bottom();
            nSecondaryLocation = aInnerBox.Center().X();
        }
        else
        {
            nLeadingLocation = aInnerBox.Right();
            nTrailingLocation = aOuterBox.Right();
            nSecondaryLocation = aInnerBox.Center().Y();
        }
        bIsTrailingFixed = true;
        if ( ! rPosition.mbIsExtraSpaceNeeded)
            bIsLeadingFixed = true;
    }
    else
    {
        // Place indicator between two rows/columns.
        const Rectangle aBox1 (GetInnerBoundingBox(rModel, rPosition.mnIndex-1));
        const Rectangle aBox2 (GetInnerBoundingBox(rModel, rPosition.mnIndex));
        if (bIsVertical)
        {
            nLeadingLocation = aBox1.Bottom();
            nTrailingLocation = aBox2.Top();
            nSecondaryLocation = (aBox1.Center().X() + aBox2.Center().X()) / 2;
        }
        else
        {
            nLeadingLocation = aBox1.Right();
            nTrailingLocation = aBox2.Left();
            nSecondaryLocation = (aBox1.Center().Y() + aBox2.Center().Y()) / 2;
        }
    }

    // 2. Calculate the location of the insert indicator and the offsets of
    // leading and trailing pages.
    const sal_Int32 nAvailableSpace (nTrailingLocation - nLeadingLocation);
    const sal_Int32 nRequiredSpace (bIsVertical ? rIndicatorSize.Height():rIndicatorSize.Width());
    const sal_Int32 nMissingSpace (::std::max(sal_Int32(0), nRequiredSpace - nAvailableSpace));
    sal_Int32 nPrimaryLocation (0);
    sal_Int32 nLeadingOffset (0);
    sal_Int32 nTrailingOffset (0);
    if (bIsLeadingFixed)
    {
        nPrimaryLocation = nLeadingLocation + nRequiredSpace/2;
        if ( ! bIsTrailingFixed)
            nTrailingOffset = nMissingSpace;
    }
    else if (bIsTrailingFixed)
    {
        nPrimaryLocation = nTrailingLocation - nRequiredSpace/2;
        nLeadingOffset = -nMissingSpace;
    }
    else
    {
        nPrimaryLocation = (nLeadingLocation + nTrailingLocation) /2;
        nLeadingOffset = -nMissingSpace/2;
        nTrailingOffset = nMissingSpace + nLeadingOffset;
    }

    if (bIsVertical)
    {
        rPosition.maLocation = Point(nSecondaryLocation, nPrimaryLocation);
        rPosition.maLeadingOffset = Point(0, nLeadingOffset);
        rPosition.maTrailingOffset = Point(0, nTrailingOffset);
    }
    else
    {
        rPosition.maLocation = Point(nPrimaryLocation, nSecondaryLocation);
        rPosition.maLeadingOffset = Point(nLeadingOffset, 0);
        rPosition.maTrailingOffset = Point(nTrailingOffset, 0);
    }
}




Rectangle Layouter::GetInnerBoundingBox (
    model::SlideSorterModel& rModel,
    const sal_Int32 nIndex) const
{
    model::SharedPageDescriptor pDescriptor (rModel.GetPageDescriptor(nIndex));
    if ( ! pDescriptor)
        return Rectangle();

    const Point aLocation (pDescriptor->GetLocation(true));
    if (pDescriptor->HasState(model::PageDescriptor::ST_Selected))
        return GetPageObjectLayouter()->GetBoundingBox(
            aLocation,
            PageObjectLayouter::PageObject,
            PageObjectLayouter::ModelCoordinateSystem);
    else
        return GetPageObjectLayouter()->GetBoundingBox(
            aLocation,
            PageObjectLayouter::Preview,
            PageObjectLayouter::ModelCoordinateSystem);
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




//===== InsertPosition ========================================================

InsertPosition::InsertPosition (void)
    : mnRow(-1),
      mnColumn(-1),
      mnIndex(-1),
      maLocation(0,0),
      maLeadingOffset(0,0),
      maTrailingOffset(0,0),
      mbIsAtRunStart(false),
      mbIsAtRunEnd(false),
      mbIsExtraSpaceNeeded(false)
{
}




InsertPosition& InsertPosition::operator= (const InsertPosition& rInsertPosition)
{
    if (this != &rInsertPosition)
    {
        mnRow = rInsertPosition.mnRow;
        mnColumn = rInsertPosition.mnColumn;
        mnIndex = rInsertPosition.mnIndex;
        maLocation = rInsertPosition.maLocation;
        maLeadingOffset = rInsertPosition.maLeadingOffset;
        maTrailingOffset = rInsertPosition.maTrailingOffset;
        mbIsAtRunStart = rInsertPosition.mbIsAtRunStart;
        mbIsAtRunEnd = rInsertPosition.mbIsAtRunEnd;
        mbIsExtraSpaceNeeded = rInsertPosition.mbIsExtraSpaceNeeded;
    }
    return *this;
}




bool InsertPosition::operator== (const InsertPosition& rInsertPosition) const
{
    // Do not compare the geometrical information (maLocation).
    return mnRow==rInsertPosition.mnRow
        && mnColumn==rInsertPosition.mnColumn
        && mnIndex==rInsertPosition.mnIndex
        && mbIsAtRunStart==rInsertPosition.mbIsAtRunStart
        && mbIsAtRunEnd==rInsertPosition.mbIsAtRunEnd
        && mbIsExtraSpaceNeeded==rInsertPosition.mbIsExtraSpaceNeeded;
}




bool InsertPosition::operator!= (const InsertPosition& rInsertPosition) const
{
    return !operator==(rInsertPosition);
}



} } } // end of namespace ::sd::slidesorter::namespace
