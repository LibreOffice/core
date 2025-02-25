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

#include <utility>
#include <view/SlsPageObjectLayouter.hxx>
#include <view/SlsTheme.hxx>
#include <view/SlsLayouter.hxx>
#include <model/SlideSorterModel.hxx>
#include <model/SlsPageDescriptor.hxx>
#include <Window.hxx>
#include <osl/diagnose.h>

namespace sd::slidesorter::view {

class Layouter::Implementation
{
public:
    VclPtr<sd::Window> mpWindow;
    static const sal_Int32 mnRequestedLeftBorder = 5;
    static const sal_Int32 mnRequestedRightBorder = 5;
    static const sal_Int32 mnRequestedTopBorder = 5;
    static const sal_Int32 mnRequestedBottomBorder = 5;
    sal_Int32 mnLeftBorder;
    sal_Int32 mnRightBorder;
    sal_Int32 mnTopBorder;
    sal_Int32 mnBottomBorder;
    static const sal_Int32 gnVerticalGap = 10 - 2*Theme_FocusIndicatorWidth;
    static const sal_Int32 gnHorizontalGap = 10 - 2*Theme_FocusIndicatorWidth;
    Size      maMinimalSize;
    Size      maPreferredSize;
    Size      maMaximalSize;
    sal_Int32 mnMinimalColumnCount;
    sal_Int32 mnMaximalColumnCount;
    sal_Int32 mnPageCount;
    sal_Int32 mnColumnCount;
    sal_Int32 mnRowCount;
    /// The maximum number of columns.  Can only be larger than the current
    /// number of columns when there are not enough pages to fill all
    /// available columns.
    sal_Int32 mnMaxColumnCount;
    /// The maximum number of rows.  Can only be larger than the current
    /// number of rows when there are not enough pages to fill all available
    /// rows.
    sal_Int32 mnMaxRowCount;
    Size maPageObjectSize;
    std::shared_ptr<PageObjectLayouter> mpPageObjectLayouter;
    std::shared_ptr<view::Theme> mpTheme;

    /** Specify how the gap between two page objects is associated with the
      page objects.
    */
    enum GapMembership {
        GM_NONE,       // Gap is not associated with any page object.
        GM_PREVIOUS,   // The whole gap is associated with the previous page
                       // object (left or above the gap.)
        GM_BOTH,       // Half of the gap is associated with previous, half
                       // with the next page object.
        GM_NEXT,       // The whole gap is associated with the next page
                       // object (right or below the gap.)
        GM_PAGE_BORDER
    };

    static Implementation* Create (
        const Implementation& rImplementation,
        const Layouter::Orientation eOrientation);

    virtual Layouter::Orientation GetOrientation() const = 0;

    bool Rearrange (
        const Size& rWindowSize,
        const Size& rPreviewModelSize,
        const sal_uInt32 nPageCount);

    /** Calculate the row that the point with the given vertical coordinate
        is over.  The horizontal component is ignored.
        @param nYPosition
            Vertical position in model coordinates.
        @param bIncludeBordersAndGaps
            When this flag is <TRUE/> then the area of borders and gaps are
            interpreted as belonging to one of the rows.
        @param eGapMembership
            Specifies to what row the gap areas belong.  Here GM_NONE
            corresponds to bIncludeBordersAndGaps being <FALSE/>.  When
            GM_BOTH is given then the upper half is associated to the row
            above and the lower half to the row below.  Values of
            GM_PREVIOUS and GM_NEXT associate the whole gap area with the
            row above or below respectively.
    */
    sal_Int32 GetRowAtPosition (
        sal_Int32 nYPosition,
        bool bIncludeBordersAndGaps,
        GapMembership eGapMembership) const;

    /** Calculate the column that the point with the given horizontal
        coordinate is over.  The vertical component is ignored.
        @param nXPosition
            Horizontal position in model coordinates.
        @param bIncludeBordersAndGaps
            When this flag is <TRUE/> then the area of borders and gaps are
            interpreted as belonging to one of the columns.
        @param eGapMembership
            Specifies to what column the gap areas belong.
    */
    sal_Int32 GetColumnAtPosition (
        sal_Int32 nXPosition,
        bool bIncludeBordersAndGaps,
        GapMembership eGapMembership) const;

    /** This method is typically called from GetRowAtPosition() and
        GetColumnAtPosition() to handle a position that lies inside the gap
        between two adjacent rows or columns.
        @param nDistanceIntoGap
            Vertical distance from the bottom of the upper row down into the
            gap or horizontal distance from the right edge right into the
            gap.
        @param eGapMemberhship
            This value decides what areas in the gap belong to which (or no)
            row or column.
        @param nIndex
            The row index of the upper row or the column index of the left
            column.
        @param nGap
             Width or height of the gap in model coordinates between the
             page borders.
        @return
           Returns either the index of the upper row (as given as nRow), the
           index of the lower row (nRow+1) or -1 to indicate that the
           position belongs to no row.
        */
    static sal_Int32 ResolvePositionInGap (
        sal_Int32 nDistanceIntoGap,
        GapMembership eGapMembership,
        sal_Int32 nIndex,
        sal_Int32 nGap);

    /** Calculate the logical part of the insert position, i.e. the page
        after which to insert.
    */
    virtual void CalculateLogicalInsertPosition (
        const Point& rModelPosition,
        InsertPosition& rPosition) const = 0;

    /** Calculate the geometrical part of the insert position, i.e. the
        location of where to display the insertion indicator and the
        distances about which the leading and trailing pages have to be
        moved to make room for the indicator.
    */
    void CalculateGeometricPosition (
        InsertPosition& rPosition,
        const Size& rIndicatorSize,
        const bool bIsVertical,
        model::SlideSorterModel const & rModel) const;

    /** Return the bounding box of the preview or, when selected, of the page
        object.  Thus, it returns something like a visual bounding box.
    */
    ::tools::Rectangle GetInnerBoundingBox (
        model::SlideSorterModel const & rModel,
        const sal_Int32 nIndex) const;

    Range GetValidHorizontalSizeRange() const;
    Range GetValidVerticalSizeRange() const;

    Range GetRangeOfVisiblePageObjects (const ::tools::Rectangle& aVisibleArea) const;
    sal_Int32 GetIndex (
        const sal_Int32 nRow,
        const sal_Int32 nColumn,
        const bool bClampToValidRange) const;

    ::tools::Rectangle GetPageObjectBox (
        const sal_Int32 nIndex,
        const bool bIncludeBorderAndGap = false) const;

    ::tools::Rectangle GetPageObjectBox (
        const sal_Int32 nRow,
        const sal_Int32 nColumn) const;

    ::tools::Rectangle AddBorderAndGap (
        const ::tools::Rectangle& rBoundingBox,
        const sal_Int32 nRow,
        const sal_Int32 nColumn) const;

    ::tools::Rectangle GetTotalBoundingBox() const;

    virtual ~Implementation();

protected:
    Implementation (
        sd::Window *pWindow,
        std::shared_ptr<view::Theme> pTheme);
    explicit Implementation (const Implementation& rImplementation);

    virtual void CalculateRowAndColumnCount (const Size& rWindowSize) = 0;
    virtual void CalculateMaxRowAndColumnCount (const Size& rWindowSize) = 0;
    virtual Size CalculateTargetSize (
        const Size& rWindowSize) const = 0;
    Size GetTargetSize (
        const Size& rWindowSize,
        const bool bCalculateWidth,
        const bool bCalculateHeight) const;
    void CalculateVerticalLogicalInsertPosition (
        const Point& rModelPosition,
        InsertPosition& rPosition) const;
};

namespace {

/** The vertical layouter has one column and as many rows as there are
    pages.
*/
class VerticalImplementation : public Layouter::Implementation
{
public:
    explicit VerticalImplementation (const Implementation& rImplementation);

    virtual Layouter::Orientation GetOrientation() const override;

    void CalculateLogicalInsertPosition (
        const Point& rModelPosition,
        InsertPosition& rPosition) const override;

protected:
    virtual void CalculateRowAndColumnCount (const Size& rWindowSize) override;
    virtual void CalculateMaxRowAndColumnCount (const Size& rWindowSize) override;
    virtual Size CalculateTargetSize (
        const Size& rWindowSize) const override;
};

/** The horizontal layouter has one row and as many columns as there are
    pages.
*/
class HorizontalImplementation : public Layouter::Implementation
{
public:
    explicit HorizontalImplementation(const Implementation& rImplementation);

    virtual Layouter::Orientation GetOrientation() const override;

    void CalculateLogicalInsertPosition (
        const Point& rModelPosition,
        InsertPosition& rPosition) const override;

protected:
    virtual void CalculateRowAndColumnCount (const Size& rWindowSize) override;
    virtual void CalculateMaxRowAndColumnCount (const Size& rWindowSize) override;
    virtual Size CalculateTargetSize (
        const Size& rWindowSize) const override;
};

/** The number of columns of the grid layouter is defined via a control in
    the slide sorter tool bar.  The number of rows is calculated from the
    number of columns and the number of pages.
*/
class GridImplementation : public Layouter::Implementation
{
public:
    GridImplementation (
        sd::Window *pWindow,
        const std::shared_ptr<view::Theme>& rpTheme);
    explicit GridImplementation(const Implementation& rImplementation);

    virtual Layouter::Orientation GetOrientation() const override;

    void CalculateLogicalInsertPosition (
        const Point& rModelPosition,
        InsertPosition& rPosition) const override;

protected:
    virtual void CalculateRowAndColumnCount (const Size& rWindowSize) override;
    virtual void CalculateMaxRowAndColumnCount (const Size& rWindowSize) override;
    virtual Size CalculateTargetSize (
        const Size& rWindowSize) const override;
};

}

//===== Layouter ==============================================================

Layouter::Layouter (
    sd::Window *pWindow,
    const std::shared_ptr<Theme>& rpTheme)
    : mpImplementation(new GridImplementation(pWindow, rpTheme)),
      mpWindow(pWindow)
{
}

Layouter::~Layouter()
{
}

std::shared_ptr<PageObjectLayouter> const & Layouter::GetPageObjectLayouter() const
{
    return mpImplementation->mpPageObjectLayouter;
}

void Layouter::SetColumnCount (
    sal_Int32 nMinimalColumnCount,
        sal_Int32 nMaximalColumnCount)
{
    if (nMinimalColumnCount <= nMaximalColumnCount)
    {
        mpImplementation->mnMinimalColumnCount = nMinimalColumnCount;
        mpImplementation->mnMaximalColumnCount = nMaximalColumnCount;
    }
}

bool Layouter::Rearrange (
    const Orientation eOrientation,
    const Size& rWindowSize,
    const Size& rPageSize,
    const sal_uInt32 nPageCount)
{
    OSL_ASSERT(mpWindow);

    if (eOrientation != mpImplementation->GetOrientation())
        mpImplementation.reset(Implementation::Create(*mpImplementation, eOrientation));

    return mpImplementation->Rearrange(rWindowSize, rPageSize, nPageCount);
}

sal_Int32 Layouter::GetColumnCount() const
{
    return mpImplementation->mnColumnCount;
}

sal_Int32 Layouter::GetIndex (const sal_Int32 nRow, const sal_Int32 nColumn) const
{
    return mpImplementation->GetIndex(nRow,nColumn,true);
}

Size const & Layouter::GetPageObjectSize() const
{
    return mpImplementation->maPageObjectSize;
}

Size Layouter::AddGap(const Size & rObjectSize)
{
    Size newSize = rObjectSize;
    newSize.AdjustWidth(Implementation::gnHorizontalGap);
    newSize.AdjustHeight(Implementation::gnVerticalGap);
    return newSize;
}

::tools::Rectangle Layouter::GetPageObjectBox (
    const sal_Int32 nIndex,
    const bool bIncludeBorderAndGap) const
{
    return mpImplementation->GetPageObjectBox(nIndex, bIncludeBorderAndGap);
}

::tools::Rectangle Layouter::GetTotalBoundingBox() const
{
    return mpImplementation->GetTotalBoundingBox();
}

InsertPosition Layouter::GetInsertPosition (
    const Point& rModelPosition,
    const Size& rIndicatorSize,
    model::SlideSorterModel const & rModel) const
{
    InsertPosition aPosition;
    mpImplementation->CalculateLogicalInsertPosition(
        rModelPosition,
        aPosition);
    mpImplementation->CalculateGeometricPosition(
        aPosition,
        rIndicatorSize,
        GetColumnCount()==1,
        rModel);
    return aPosition;
}

Range Layouter::GetValidHorizontalSizeRange() const
{
    return mpImplementation->GetValidHorizontalSizeRange();
}

Range Layouter::GetValidVerticalSizeRange() const
{
    return mpImplementation->GetValidVerticalSizeRange();
}

Range Layouter::GetRangeOfVisiblePageObjects (const ::tools::Rectangle& aVisibleArea) const
{
    return mpImplementation->GetRangeOfVisiblePageObjects(aVisibleArea);
}

sal_Int32 Layouter::GetIndexAtPoint(const Point& rPosition) const
{
    const sal_Int32 nRow (
        mpImplementation->GetRowAtPosition (
            rPosition.Y(),
            /*bIncludePageBorders*/false,
            Implementation::GM_NONE));
    const sal_Int32 nColumn (
        mpImplementation->GetColumnAtPosition (
            rPosition.X(),
            /*bIncludePageBorders*/false,
            Implementation::GM_NONE));

    return mpImplementation->GetIndex(nRow,nColumn, /*bClampToValidRange*/ false);
}

//===== Layouter::Implementation ==============================================

Layouter::Implementation* Layouter::Implementation::Create (
    const Implementation& rImplementation,
    const Layouter::Orientation eOrientation)
{
    switch (eOrientation)
    {
        case HORIZONTAL: return new HorizontalImplementation(rImplementation);
        case VERTICAL: return new VerticalImplementation(rImplementation);
        case GRID:
        default: return new GridImplementation(rImplementation);
    }
}

Layouter::Implementation::Implementation (
    sd::Window *pWindow,
    std::shared_ptr<view::Theme> pTheme)
    : mpWindow(pWindow),
      mnLeftBorder(5),
      mnRightBorder(5),
      mnTopBorder(5),
      mnBottomBorder(5),
      maMinimalSize(132,98),
      maPreferredSize(200,150),
      maMaximalSize(600,400),
      mnMinimalColumnCount(1),
      mnMaximalColumnCount(15),
      mnPageCount(0),
      mnColumnCount(1),
      mnRowCount(0),
      mnMaxColumnCount(0),
      mnMaxRowCount(0),
      maPageObjectSize(1,1),
      mpTheme(std::move(pTheme))
{
}

Layouter::Implementation::Implementation (const Implementation& rImplementation)
    : mpWindow(rImplementation.mpWindow),
      mnLeftBorder(rImplementation.mnLeftBorder),
      mnRightBorder(rImplementation.mnRightBorder),
      mnTopBorder(rImplementation.mnTopBorder),
      mnBottomBorder(rImplementation.mnBottomBorder),
      maMinimalSize(rImplementation.maMinimalSize),
      maPreferredSize(rImplementation.maPreferredSize),
      maMaximalSize(rImplementation.maMaximalSize),
      mnMinimalColumnCount(rImplementation.mnMinimalColumnCount),
      mnMaximalColumnCount(rImplementation.mnMaximalColumnCount),
      mnPageCount(rImplementation.mnPageCount),
      mnColumnCount(rImplementation.mnColumnCount),
      mnRowCount(rImplementation.mnRowCount),
      mnMaxColumnCount(rImplementation.mnMaxColumnCount),
      mnMaxRowCount(rImplementation.mnMaxRowCount),
      maPageObjectSize(rImplementation.maPageObjectSize),
      mpTheme(rImplementation.mpTheme)
{
}

Layouter::Implementation::~Implementation()
{
}

bool Layouter::Implementation::Rearrange (
    const Size& rWindowSize,
    const Size& rPreviewModelSize,
    const sal_uInt32 nPageCount)
{
    mnPageCount = nPageCount;

    // Return early when the window or the model have not yet been initialized.
    if (rWindowSize.IsEmpty())
        return false;
    if (rPreviewModelSize.IsEmpty())
        return false;

    CalculateRowAndColumnCount(rWindowSize);

    // Update the border values.
    mnLeftBorder = mnRequestedLeftBorder;
    mnTopBorder = mnRequestedTopBorder;
    mnRightBorder = mnRequestedRightBorder;
    mnBottomBorder = mnRequestedBottomBorder;
    if (mnColumnCount > 1)
    {
        int nMinimumBorderWidth = gnHorizontalGap/2;
        if (mnLeftBorder < nMinimumBorderWidth)
            mnLeftBorder = nMinimumBorderWidth;
        if (mnRightBorder < nMinimumBorderWidth)
            mnRightBorder = nMinimumBorderWidth;
    }
    else
    {
        int nMinimumBorderHeight = gnVerticalGap/2;
        if (mnTopBorder < nMinimumBorderHeight)
            mnTopBorder = nMinimumBorderHeight;
        if (mnBottomBorder < nMinimumBorderHeight)
            mnBottomBorder = nMinimumBorderHeight;
    }

    mpPageObjectLayouter =
        std::make_shared<PageObjectLayouter>(
            CalculateTargetSize(rWindowSize),
            rPreviewModelSize,
            mpWindow,
            mnPageCount);

    maPageObjectSize = mpPageObjectLayouter->GetGridMaxSize();

    CalculateMaxRowAndColumnCount(rWindowSize);

    return true;
}

sal_Int32 Layouter::Implementation::GetRowAtPosition (
    sal_Int32 nYPosition,
    bool bIncludeBordersAndGaps,
    GapMembership eGapMembership) const
{
    sal_Int32 nRow = -1;

    const sal_Int32 nY = nYPosition - mnTopBorder;
    if (nY >= 0)
    {
        // Vertical distance from one row to the next.
        const sal_Int32 nRowOffset (maPageObjectSize.Height() + gnVerticalGap);

        // Calculate row consisting of page objects and gap below.
        nRow = nY / nRowOffset;

        const sal_Int32 nDistanceIntoGap ((nY - nRow*nRowOffset) - maPageObjectSize.Height());
        // When inside the gap below then nYPosition is not over a page
        // object.
        if (nDistanceIntoGap > 0)
        {
            sal_Int32 nResolvedRow = ResolvePositionInGap(
                nDistanceIntoGap,
                eGapMembership,
                nRow,
                gnVerticalGap);
            if (!bIncludeBordersAndGaps || nResolvedRow != -1)
                nRow = nResolvedRow;
        }
    }
    else if (bIncludeBordersAndGaps)
    {
        // We are in the top border area.  Set nRow to the first row when
        // the top border shall be considered to belong to the first row.
        nRow = 0;
    }

    return nRow;
}

sal_Int32 Layouter::Implementation::GetColumnAtPosition (
    sal_Int32 nXPosition,
    bool bIncludeBordersAndGaps,
    GapMembership eGapMembership) const
{
    sal_Int32 nColumn = -1;

    sal_Int32 nX = nXPosition - mnLeftBorder;
    if (nX >= 0)
    {
        // Horizontal distance from one column to the next.
        const sal_Int32 nColumnOffset (maPageObjectSize.Width() + gnHorizontalGap);

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
        {
            sal_Int32 nResolvedColumn = ResolvePositionInGap(
                nDistanceIntoGap,
                eGapMembership,
                nColumn,
                gnHorizontalGap);
            if (!bIncludeBordersAndGaps || nResolvedColumn != -1)
                nColumn = nResolvedColumn;
        }
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

sal_Int32 Layouter::Implementation::ResolvePositionInGap (
    sal_Int32 nDistanceIntoGap,
    GapMembership eGapMembership,
    sal_Int32 nIndex,
    sal_Int32 nGap)
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

void Layouter::Implementation::CalculateGeometricPosition (
    InsertPosition& rPosition,
    const Size& rIndicatorSize,
    const bool bIsVertical,
    model::SlideSorterModel const & rModel) const
{
    // 1. Determine right/bottom of the leading page and the left/top of the
    // trailing page object and how to distribute the missing space.
    sal_Int32 nLeadingLocation (0);
    sal_Int32 nTrailingLocation (0);
    bool bIsLeadingFixed (false);
    bool bIsTrailingFixed (false);
    sal_Int32 nSecondaryLocation (0);
    const sal_Int32 nIndex (rPosition.GetIndex());

    if (rPosition.IsAtRunStart())
    {
        // Place indicator at the top of the column.
        const ::tools::Rectangle aOuterBox (GetPageObjectBox(nIndex));
        const ::tools::Rectangle aInnerBox (GetInnerBoundingBox(rModel, nIndex));
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
    else if (rPosition.IsAtRunEnd())
    {
        // Place indicator at the bottom/right of the column/row.

        const ::tools::Rectangle aOuterBox (GetPageObjectBox(nIndex-1));
        const ::tools::Rectangle aInnerBox (GetInnerBoundingBox(rModel, nIndex-1));
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
        if ( ! rPosition.IsExtraSpaceNeeded())
            bIsLeadingFixed = true;
    }
    else
    {
        // Place indicator between two rows/columns.
        const ::tools::Rectangle aBox1 (GetInnerBoundingBox(rModel, nIndex-1));
        const ::tools::Rectangle aBox2 (GetInnerBoundingBox(rModel, nIndex));
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
        rPosition.SetGeometricalPosition(
            Point(nSecondaryLocation, nPrimaryLocation),
            Point(0, nLeadingOffset),
            Point(0, nTrailingOffset));
    }
    else
    {
        rPosition.SetGeometricalPosition(
            Point(nPrimaryLocation, nSecondaryLocation),
            Point(nLeadingOffset, 0),
            Point(nTrailingOffset, 0));
    }
}

::tools::Rectangle Layouter::Implementation::GetInnerBoundingBox (
    model::SlideSorterModel const & rModel,
    const sal_Int32 nIndex) const
{
    model::SharedPageDescriptor pDescriptor (rModel.GetPageDescriptor(nIndex));
    if ( ! pDescriptor)
        return ::tools::Rectangle();

    PageObjectLayouter::Part ePart = PageObjectLayouter::Part::Preview;

    if (pDescriptor->HasState(model::PageDescriptor::ST_Selected))
        ePart = PageObjectLayouter::Part::PageObject;

    return mpPageObjectLayouter->GetBoundingBox(
            pDescriptor, ePart,
            PageObjectLayouter::ModelCoordinateSystem, true);
}

Range Layouter::Implementation::GetValidHorizontalSizeRange() const
{
    return Range(
        mnLeftBorder + maMinimalSize.Width() + mnRightBorder,
        mnLeftBorder + maMaximalSize.Width() + mnRightBorder);
}

Range Layouter::Implementation::GetValidVerticalSizeRange() const
{
    return Range(
        mnTopBorder + maMinimalSize.Height() + mnBottomBorder,
        mnTopBorder + maMaximalSize.Height() + mnBottomBorder);
}

Range Layouter::Implementation::GetRangeOfVisiblePageObjects (const ::tools::Rectangle& aVisibleArea) const
{
    // technically that's not empty, but it's the default, so...
    if (aVisibleArea.IsEmpty())
        return Range(-1, -1);

    const sal_Int32 nRow0 (GetRowAtPosition(aVisibleArea.Top(), true, GM_NEXT));
    const sal_Int32 nCol0 (GetColumnAtPosition(aVisibleArea.Left(),true, GM_NEXT));
    const sal_Int32 nRow1 (GetRowAtPosition(aVisibleArea.Bottom(), true, GM_PREVIOUS));
    const sal_Int32 nCol1 (GetColumnAtPosition(aVisibleArea.Right(), true, GM_PREVIOUS));

    // When start and end lie in different rows then the range may include
    // slides outside (left or right of) the given area.
    return Range(GetIndex(nRow0,nCol0,true), GetIndex(nRow1,nCol1,true));
}

Size Layouter::Implementation::GetTargetSize (
    const Size& rWindowSize,
    const bool bCalculateWidth,
    const bool bCalculateHeight) const
{
    if (mnColumnCount<=0 || mnRowCount<=0)
        return maPreferredSize;
    if ( ! (bCalculateWidth || bCalculateHeight))
    {
        OSL_ASSERT(bCalculateWidth || bCalculateHeight);
        return maPreferredSize;
    }

    // Calculate the width of each page object.
    Size aTargetSize (0,0);
    if (bCalculateWidth)
        aTargetSize.setWidth(
            (rWindowSize.Width() - mnLeftBorder - mnRightBorder
                - (mnColumnCount-1) * gnHorizontalGap)
                    / mnColumnCount);
    else if (bCalculateHeight)
        aTargetSize.setHeight(
            (rWindowSize.Height() - mnTopBorder - mnBottomBorder
                - (mnRowCount-1) * gnVerticalGap)
                    / mnRowCount);

    if (bCalculateWidth)
    {
        if (aTargetSize.Width() < maMinimalSize.Width())
            aTargetSize.setWidth(maMinimalSize.Width());
        else if (aTargetSize.Width() > maMaximalSize.Width())
            aTargetSize.setWidth(maMaximalSize.Width());
    }
    else if (bCalculateHeight)
    {
        if (aTargetSize.Height() < maMinimalSize.Height())
            aTargetSize.setHeight(maMinimalSize.Height());
        else if (aTargetSize.Height() > maMaximalSize.Height())
            aTargetSize.setHeight(maMaximalSize.Height());
    }

    return aTargetSize;
}

sal_Int32 Layouter::Implementation::GetIndex (
    const sal_Int32 nRow,
    const sal_Int32 nColumn,
    const bool bClampToValidRange) const
{
    if (nRow >= 0 && nColumn >= 0)
    {
        const sal_Int32 nIndex (nRow * mnColumnCount + nColumn);
        if (nIndex >= mnPageCount)
            if (bClampToValidRange)
                return mnPageCount-1;
            else
                return -1;
        else
            return nIndex;
    }
    else if (bClampToValidRange)
        return 0;
    else
        return -1;
}

::tools::Rectangle Layouter::Implementation::GetPageObjectBox (
    const sal_Int32 nIndex,
    const bool bIncludeBorderAndGap) const
{
    const sal_Int32 nRow (nIndex / mnColumnCount);
    const sal_Int32 nColumn (nIndex % mnColumnCount);

    const ::tools::Rectangle aBoundingBox (GetPageObjectBox(nRow,nColumn));
    if (bIncludeBorderAndGap)
        return AddBorderAndGap(aBoundingBox, nRow, nColumn);
    else
        return aBoundingBox;
}

::tools::Rectangle Layouter::Implementation::GetPageObjectBox (
    const sal_Int32 nRow,
    const sal_Int32 nColumn) const
{
    return ::tools::Rectangle(
        Point (mnLeftBorder
            + nColumn * maPageObjectSize.Width()
            + std::max<sal_Int32>(nColumn,0) * gnHorizontalGap,
            mnTopBorder
            + nRow * maPageObjectSize.Height()
            + std::max<sal_Int32>(nRow,0) * gnVerticalGap),
        maPageObjectSize);
}

::tools::Rectangle Layouter::Implementation::AddBorderAndGap (
    const ::tools::Rectangle& rBoundingBox,
    const sal_Int32 nRow,
    const sal_Int32 nColumn) const
{
    ::tools::Rectangle aBoundingBox (rBoundingBox);

    if (nColumn == 0)
        aBoundingBox.SetLeft( 0 );
    else
        aBoundingBox.AdjustLeft( -(gnHorizontalGap/2) );
    if (nColumn == mnColumnCount-1)
        aBoundingBox.AdjustRight(mnRightBorder );
    else
        aBoundingBox.AdjustRight(gnHorizontalGap/2 );
    if (nRow == 0)
        aBoundingBox.SetTop( 0 );
    else
        aBoundingBox.AdjustTop( -(gnVerticalGap/2) );
    if (nRow == mnRowCount-1)
        aBoundingBox.AdjustBottom(mnBottomBorder );
    else
        aBoundingBox.AdjustBottom(gnVerticalGap/2 );
    return aBoundingBox;
}

::tools::Rectangle Layouter::Implementation::GetTotalBoundingBox() const
{
    sal_Int32 nHorizontalSize = 0;
    sal_Int32 nVerticalSize = 0;
    if (mnColumnCount > 0)
    {
        sal_Int32 nRowCount = (mnPageCount+mnColumnCount-1) / mnColumnCount;
        nHorizontalSize =
            mnLeftBorder
            + mnRightBorder
            + mnColumnCount * maPageObjectSize.Width();
        if (mnColumnCount > 1)
            nHorizontalSize +=  (mnColumnCount-1) * gnHorizontalGap;
        nVerticalSize =
            mnTopBorder
            + mnBottomBorder
            + nRowCount * maPageObjectSize.Height();
        if (nRowCount > 1)
            nVerticalSize += (nRowCount-1) * gnVerticalGap;
    }

    return ::tools::Rectangle (
        Point(0,0),
        Size (nHorizontalSize, nVerticalSize)
        );
}

void Layouter::Implementation::CalculateVerticalLogicalInsertPosition (
    const Point& rModelPosition,
    InsertPosition& rPosition) const
{
    const sal_Int32 nY = rModelPosition.Y() - mnTopBorder + maPageObjectSize.Height()/2;
    const sal_Int32 nRowHeight (maPageObjectSize.Height() + gnVerticalGap);
    const sal_Int32 nRow (::std::min(mnPageCount, nY / nRowHeight));
    rPosition.SetLogicalPosition (
        nRow,
        0,
        nRow,
        (nRow == 0),
        (nRow == mnRowCount),
        (nRow >= mnMaxRowCount));
}

//===== HorizontalImplementation ================================================

HorizontalImplementation::HorizontalImplementation (const Implementation& rImplementation)
    : Implementation(rImplementation)
{
}

Layouter::Orientation HorizontalImplementation::GetOrientation() const
{
    return Layouter::HORIZONTAL;
}

void HorizontalImplementation::CalculateRowAndColumnCount (const Size&)
{
    // Row and column count are fixed (for a given page count.)
    mnColumnCount = mnPageCount;
    mnRowCount = 1;
}

void HorizontalImplementation::CalculateMaxRowAndColumnCount (const Size& rWindowSize)
{
    mnMaxColumnCount = (rWindowSize.Width() - mnLeftBorder - mnRightBorder)
        / (maPageObjectSize.Width()  + gnHorizontalGap);
    mnMaxRowCount = 1;
}

Size HorizontalImplementation::CalculateTargetSize (
    const Size& rWindowSize) const
{
    return Implementation::GetTargetSize(rWindowSize, false, true);
}

void HorizontalImplementation::CalculateLogicalInsertPosition (
    const Point& rModelPosition,
    InsertPosition& rPosition) const
{
    const sal_Int32 nX = rModelPosition.X() - mnLeftBorder + maPageObjectSize.Width()/2;
    const sal_Int32 nColumnWidth (maPageObjectSize.Width() + gnHorizontalGap);
    const sal_Int32 nColumn (::std::min(mnPageCount, nX / nColumnWidth));
    rPosition.SetLogicalPosition (
        0,
        nColumn,
        nColumn,
        (nColumn == 0),
        (nColumn == mnColumnCount),
        (nColumn >= mnMaxColumnCount));
}

//===== VerticalImplementation ================================================

VerticalImplementation::VerticalImplementation (const Implementation& rImplementation)
    : Implementation(rImplementation)
{
}

Layouter::Orientation VerticalImplementation::GetOrientation() const
{
    return Layouter::VERTICAL;
}

void VerticalImplementation::CalculateRowAndColumnCount (const Size&)
{
    // Row and column count are fixed (for a given page count.)
    mnRowCount = mnPageCount;
    mnColumnCount = 1;

}

void VerticalImplementation::CalculateMaxRowAndColumnCount (const Size& rWindowSize)
{
    mnMaxRowCount = (rWindowSize.Height() - mnTopBorder - mnBottomBorder)
        / (maPageObjectSize.Height()  + gnVerticalGap);
    mnMaxColumnCount = 1;
}

Size VerticalImplementation::CalculateTargetSize (
    const Size& rWindowSize) const
{
    return Implementation::GetTargetSize(rWindowSize, true, false);
}

void VerticalImplementation::CalculateLogicalInsertPosition (
    const Point& rModelPosition,
    InsertPosition& rPosition) const
{
    return CalculateVerticalLogicalInsertPosition(rModelPosition, rPosition);
}

//===== GridImplementation ================================================

GridImplementation::GridImplementation (
    sd::Window *pWindow,
    const std::shared_ptr<view::Theme>& rpTheme)
    : Implementation(pWindow, rpTheme)
{
}

GridImplementation::GridImplementation (const Implementation& rImplementation)
    : Implementation(rImplementation)
{
}

Layouter::Orientation GridImplementation::GetOrientation() const
{
    return Layouter::GRID;
}

void GridImplementation::CalculateRowAndColumnCount (const Size& rWindowSize)
{
    // Calculate the column count.
    mnColumnCount
        = (rWindowSize.Width() - mnRequestedLeftBorder - mnRequestedRightBorder)
        / (maPreferredSize.Width()  + gnHorizontalGap);
    if (mnColumnCount < mnMinimalColumnCount)
        mnColumnCount = mnMinimalColumnCount;
    if (mnColumnCount > mnMaximalColumnCount)
        mnColumnCount = mnMaximalColumnCount;
    mnRowCount = (mnPageCount + mnColumnCount-1)/mnColumnCount;
}

void GridImplementation::CalculateMaxRowAndColumnCount (const Size& rWindowSize)
{
    mnMaxColumnCount = (rWindowSize.Width() - mnLeftBorder - mnRightBorder)
        / (maPageObjectSize.Width()  + gnHorizontalGap);
    mnMaxRowCount = (rWindowSize.Height() - mnTopBorder - mnBottomBorder)
        / (maPageObjectSize.Height()  + gnVerticalGap);
}

Size GridImplementation::CalculateTargetSize (
    const Size& rWindowSize) const
{
    return Implementation::GetTargetSize(rWindowSize, true, true);
}

void GridImplementation::CalculateLogicalInsertPosition (
    const Point& rModelPosition,
    InsertPosition& rPosition) const
{
    if (mnColumnCount == 1)
    {
        CalculateVerticalLogicalInsertPosition(rModelPosition, rPosition);
    }
    else
    {
        // Handle the general case of more than one column.
        sal_Int32 nRow (::std::min(
            mnRowCount-1,
            GetRowAtPosition (rModelPosition.Y(), true, GM_BOTH)));
        const sal_Int32 nX = rModelPosition.X() - mnLeftBorder + maPageObjectSize.Width()/2;
        const sal_Int32 nColumnWidth (maPageObjectSize.Width() + gnHorizontalGap);
        sal_Int32 nColumn (::std::min(mnColumnCount, nX / nColumnWidth));
        sal_Int32 nIndex (nRow * mnColumnCount + nColumn);
        bool bIsAtRunEnd (nColumn == mnColumnCount);

        if (nIndex >= mnPageCount)
        {
            nIndex = mnPageCount;
            nRow = mnRowCount-1;
            nColumn = ::std::min(::std::min(mnPageCount, mnColumnCount), nColumn);
            bIsAtRunEnd = true;
        }

        rPosition.SetLogicalPosition (
            nRow,
            nColumn,
            nIndex,
            (nColumn == 0),
            bIsAtRunEnd,
            (nColumn >= mnMaxColumnCount));
    }
}

//===== InsertPosition ========================================================

InsertPosition::InsertPosition()
    : mnRow(-1),
      mnColumn(-1),
      mnIndex(-1),
      mbIsAtRunStart(false),
      mbIsAtRunEnd(false),
      mbIsExtraSpaceNeeded(false),
      maLocation(0,0),
      maLeadingOffset(0,0),
      maTrailingOffset(0,0)
{
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

void InsertPosition::SetLogicalPosition (
    const sal_Int32 nRow,
    const sal_Int32 nColumn,
    const sal_Int32 nIndex,
    const bool bIsAtRunStart,
    const bool bIsAtRunEnd,
    const bool bIsExtraSpaceNeeded)
{
    mnRow = nRow;
    mnColumn = nColumn;
    mnIndex = nIndex;
    mbIsAtRunStart = bIsAtRunStart;
    mbIsAtRunEnd = bIsAtRunEnd;
    mbIsExtraSpaceNeeded = bIsExtraSpaceNeeded;
}

void InsertPosition::SetGeometricalPosition(
    const Point& rLocation,
    const Point& rLeadingOffset,
    const Point& rTrailingOffset)
{
    maLocation = rLocation;
    maLeadingOffset = rLeadingOffset;
    maTrailingOffset = rTrailingOffset;
}

} // end of namespace ::sd::slidesorter::namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
