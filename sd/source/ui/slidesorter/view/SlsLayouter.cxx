/*************************************************************************
 *
 *  $RCSfile: SlsLayouter.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:27:21 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "view/SlsLayouter.hxx"

#include <vcl/outdev.hxx>
#include <rtl/math.hxx>

namespace sd { namespace slidesorter { namespace view {

Layouter::Layouter (void)
    : mnRequestedLeftBorder(10),
      mnRequestedRightBorder(10),
      mnRequestedTopBorder(10),
      mnRequestedBottomBorder(10),
      mnLeftBorder(10),
      mnRightBorder(10),
      mnTopBorder(10),
      mnBottomBorder(10),
      mnLeftPageBorder(0),
      mnRightPageBorder(0),
      mnTopPageBorder(0),
      mnBottomPageBorder(0),
      mnVerticalGap (20),
      mnHorizontalGap (20),
      mnInsertionMarkerThickness (4),
      mnTotalVerticalGap(0),
      mnTotalHorizontalGap(0),
      mnMinimalWidth (100),
      mnDesiredWidth (200),
      mnMaximalWidth (300),
      mnMinimalColumnCount (1),
      mnMaximalColumnCount (5),
      mnColumnCount (1),
      maPageObjectModelSize (1,1),
      maPageObjectPixelSize (1,1)
{
}




Layouter::~Layouter (void)
{
}


void Layouter::SetObjectWidth (
    sal_Int32 nMinimalWidth,
    sal_Int32 nMaximalWidth,
    sal_Int32 nDesiredWidth)
{
    if (nMinimalWidth <= nDesiredWidth && nDesiredWidth <= nMaximalWidth)
    {
        mnMinimalWidth = nMinimalWidth;
        mnDesiredWidth = nMaximalWidth;
        mnMaximalWidth = nDesiredWidth;
    }
}




void Layouter::SetBorders (
    sal_Int32 nLeftBorder,
    sal_Int32 nRightBorder,
    sal_Int32 nTopBorder,
    sal_Int32 nBottomBorder)
{
    if (nLeftBorder >= 0)
        mnRequestedLeftBorder.mnScreen = nLeftBorder;
    if (nRightBorder >= 0)
        mnRequestedRightBorder.mnScreen = nRightBorder;
    if (nTopBorder >= 0)
        mnRequestedTopBorder.mnScreen = nTopBorder;
    if (nBottomBorder >= 0)
        mnRequestedBottomBorder.mnScreen = nBottomBorder;
}




void Layouter::SetPageBorders (
    sal_Int32 nLeftBorder,
    sal_Int32 nRightBorder,
    sal_Int32 nTopBorder,
    sal_Int32 nBottomBorder)
{
    if (nLeftBorder >= 0)
        mnLeftPageBorder.mnScreen = nLeftBorder;
    if (nRightBorder >= 0)
        mnRightPageBorder.mnScreen = nRightBorder;
    if (nTopBorder >= 0)
        mnTopPageBorder.mnScreen = nTopBorder;
    if (nBottomBorder >= 0)
        mnBottomPageBorder.mnScreen = nBottomBorder;
}




void Layouter::SetGaps (
    sal_Int32 nHorizontalGap,
    sal_Int32 nVerticalGap)
{
    if (nHorizontalGap >= 0)
        mnHorizontalGap.mnScreen = nHorizontalGap;
    if (nVerticalGap >= 0)
        mnVerticalGap.mnScreen = nVerticalGap;
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




bool Layouter::Rearrange (
    const Size& rWindowSize,
    const Size& rPageObjectSize,
    OutputDevice* pDevice)
{
    if (rWindowSize.Width() > 0
        && rWindowSize.Height() > 0
        && rPageObjectSize.Width() > 0
        && rPageObjectSize.Height() > 0)
    {
        mnTotalHorizontalGap.mnScreen = mnHorizontalGap.mnScreen
            + mnRightPageBorder.mnScreen + mnLeftPageBorder.mnScreen;
        mnTotalVerticalGap.mnScreen = mnVerticalGap.mnScreen
            + mnTopPageBorder.mnScreen + mnBottomPageBorder.mnScreen;

        // Calculate the column count.
        mnColumnCount = (rWindowSize.Width()
            - mnRequestedLeftBorder.mnScreen - mnRequestedRightBorder.mnScreen)
            / (mnDesiredWidth  + mnTotalHorizontalGap.mnScreen);
        if (mnColumnCount < mnMinimalColumnCount)
            mnColumnCount = mnMinimalColumnCount;
        if (mnColumnCount > mnMaximalColumnCount)
            mnColumnCount = mnMaximalColumnCount;

        // Update the border values.  The insertion marker has to have space.
        mnLeftBorder.mnScreen = mnRequestedLeftBorder.mnScreen;
        mnTopBorder.mnScreen = mnRequestedTopBorder.mnScreen;
        mnRightBorder.mnScreen = mnRequestedRightBorder.mnScreen;
        mnBottomBorder.mnScreen = mnRequestedBottomBorder.mnScreen;
        if (mnColumnCount > 1)
        {
            int nMinimumBorderWidth = mnInsertionMarkerThickness.mnScreen
                + mnHorizontalGap.mnScreen/2;
            if (mnLeftBorder.mnScreen < nMinimumBorderWidth)
                mnLeftBorder.mnScreen = nMinimumBorderWidth;
            if (mnRightBorder.mnScreen < nMinimumBorderWidth)
                mnRightBorder.mnScreen = nMinimumBorderWidth;
        }
        else
        {
            int nMinimumBorderHeight = mnInsertionMarkerThickness.mnScreen
                + mnVerticalGap.mnScreen/2;
            if (mnTopBorder.mnScreen < nMinimumBorderHeight)
                mnTopBorder.mnScreen = nMinimumBorderHeight;
            if (mnBottomBorder.mnScreen < nMinimumBorderHeight)
                mnBottomBorder.mnScreen = nMinimumBorderHeight;
        }

        // Calculate the width of each page object.
        sal_Int32 nTargetWidth = 0;
        if (mnColumnCount > 0)
            nTargetWidth = (rWindowSize.Width()
                - mnLeftBorder.mnScreen
                - mnRightBorder.mnScreen
                - mnColumnCount * (mnRightPageBorder.mnScreen
                    + mnLeftPageBorder.mnScreen)
                - (mnColumnCount-1) * mnTotalHorizontalGap.mnScreen
                )
            / mnColumnCount;
        if (nTargetWidth < mnMinimalWidth)
            nTargetWidth = mnMinimalWidth;
        if (nTargetWidth > mnMaximalWidth)
            nTargetWidth = mnMaximalWidth;

        // Calculate the resulting scale factor and the page object size in
        // pixels.
        maPageObjectModelSize = rPageObjectSize;
        MapMode aPixelMode (MAP_PIXEL);
        int nPagePixelWidth
            = pDevice->LogicToPixel (maPageObjectModelSize).Width();

        // Adapt the layout of the given output device to the new layout of
        // page objects.  The zoom factor is set so that the page objects in
        // one row fill the screen.
        Fraction aScaleFactor = Fraction (nTargetWidth, nPagePixelWidth);
        MapMode aMapMode (pDevice->GetMapMode());
        SetZoom (aMapMode.GetScaleX() * aScaleFactor, pDevice);

        return true;
    }
    else
        return false;
}




void Layouter::SetZoom (double nZoomFactor, OutputDevice* pDevice)
{
    SetZoom(Fraction(nZoomFactor), pDevice);
}



void Layouter::SetZoom (Fraction nZoomFactor, OutputDevice* pDevice)
{
    MapMode aMapMode (pDevice->GetMapMode());
    aMapMode.SetScaleX (nZoomFactor);
    aMapMode.SetScaleY (nZoomFactor);
    maPageObjectPixelSize = pDevice->LogicToPixel (maPageObjectModelSize);
    pDevice->SetMapMode (aMapMode);

    // Transform frequently used values from pixel to model coordinates.

    Size aTotalGap (pDevice->PixelToLogic (Size (
        mnTotalHorizontalGap.mnScreen,
        mnTotalVerticalGap.mnScreen)));
    mnTotalHorizontalGap.mnModel = aTotalGap.Width();
    mnTotalVerticalGap.mnModel = aTotalGap.Height();

    Size aGap (pDevice->PixelToLogic (Size (
        mnHorizontalGap.mnScreen,
        mnVerticalGap.mnScreen)));
    mnHorizontalGap.mnModel = aGap.Width();
    mnVerticalGap.mnModel = aGap.Height();

    Size aTopLeftBorder (pDevice->PixelToLogic (Size (
        mnLeftBorder.mnScreen,
        mnTopBorder.mnScreen)));
    mnLeftBorder.mnModel = aTopLeftBorder.Width();
    mnTopBorder.mnModel = aTopLeftBorder.Height();

    Size aBottomRightBorder (pDevice->PixelToLogic (Size (
        mnLeftBorder.mnScreen,
        mnTopBorder.mnScreen)));
    mnRightBorder.mnModel = aBottomRightBorder.Width();
    mnBottomBorder.mnModel = aBottomRightBorder.Height();

    Size aTopLeftPageBorder (pDevice->PixelToLogic (Size (
        mnLeftPageBorder.mnScreen,
        mnTopPageBorder.mnScreen)));
    mnLeftPageBorder.mnModel = aTopLeftPageBorder.Width();
    mnTopPageBorder.mnModel = aTopLeftPageBorder.Height();

    Size aBottomRightPageBorder (pDevice->PixelToLogic (Size (
        mnRightPageBorder.mnScreen,
        mnBottomPageBorder.mnScreen)));
    mnRightPageBorder.mnModel = aBottomRightPageBorder.Width();
    mnBottomPageBorder.mnModel = aBottomRightPageBorder.Height();

    mnInsertionMarkerThickness.mnModel = pDevice->PixelToLogic (
        Size(mnInsertionMarkerThickness.mnScreen,0)).Width();
}




sal_Int32 Layouter::GetColumnCount (void) const
{
    return mnColumnCount;
}




Rectangle Layouter::GetPageObjectBox (sal_Int32 nIndex) const
{
    int nColumn = nIndex % mnColumnCount;
    int nRow = nIndex / mnColumnCount;
    return Rectangle (
        Point (mnLeftBorder.mnModel
            + nColumn * maPageObjectModelSize.Width()
            + mnLeftPageBorder.mnModel
            + (nColumn>0 ? nColumn : 0) * mnTotalHorizontalGap.mnModel,
            mnTopBorder.mnModel
            + nRow * maPageObjectModelSize.Height()
            + mnTopPageBorder.mnModel
            + (nRow>0 ? nRow : 0) * mnTotalVerticalGap.mnModel),
        maPageObjectModelSize);
}




Rectangle Layouter::GetPageBox (sal_Int32 nObjectCount) const
{
    sal_Int32 nHorizontalSize = 0;
    sal_Int32 nVerticalSize = 0;
    if (mnColumnCount > 0)
    {
        sal_Int32 nRowCount = (nObjectCount+mnColumnCount-1) / mnColumnCount;
        nHorizontalSize =
            mnLeftBorder.mnModel
            + mnRightBorder.mnModel
            + mnColumnCount * maPageObjectModelSize.Width()
            + mnLeftPageBorder.mnModel + mnRightPageBorder.mnModel;
        if (mnColumnCount > 1)
            nHorizontalSize
                +=  (mnColumnCount-1) * mnTotalHorizontalGap.mnModel;
        nVerticalSize =
            mnTopBorder.mnModel
            + mnBottomBorder.mnModel
            + nRowCount * maPageObjectModelSize.Height()
            + mnTopPageBorder.mnModel + mnBottomPageBorder.mnModel;
        if (nRowCount > 1)
            nVerticalSize += (nRowCount-1) * mnTotalVerticalGap.mnModel;
    }

    return Rectangle (
        Point(0,0),
        Size (nHorizontalSize, nVerticalSize)
        );
}




Rectangle Layouter::GetInsertionMarkerBox (
    sal_Int32 nIndex,
    bool bVertical,
    bool bLeftOrTop) const
{
    Rectangle aBox (GetPageObjectBox (nIndex));

    if (bVertical)
    {
        sal_Int32 nHorizontalInsertionMarkerOffset
            = (mnHorizontalGap.mnModel-mnInsertionMarkerThickness.mnModel) / 2;
        if (bLeftOrTop)
        {
            // Left.
            aBox.Left() -= mnLeftPageBorder.mnModel
                + mnHorizontalGap.mnModel
                - nHorizontalInsertionMarkerOffset;
        }
        else
        {
            // Right.
            aBox.Left() = aBox.Right()
                + mnRightPageBorder.mnModel
                + nHorizontalInsertionMarkerOffset;
        }
        aBox.Right() = aBox.Left() + mnInsertionMarkerThickness.mnModel;
    }
    else
    {
        sal_Int32 nVerticalInsertionMarkerOffset
            = (mnVerticalGap.mnModel - mnInsertionMarkerThickness.mnModel) / 2;
        if (bLeftOrTop)
        {
            // Above.
            aBox.Top() -=  mnTopPageBorder.mnModel
                + mnVerticalGap.mnModel
                - nVerticalInsertionMarkerOffset;
        }
        else
        {
            // Below.
            aBox.Top() = aBox.Bottom()
                + mnBottomPageBorder.mnModel
                + nVerticalInsertionMarkerOffset;
        }
        aBox.Bottom() = aBox.Top() + mnInsertionMarkerThickness.mnModel;
    }

    return aBox;
}




sal_Int32 Layouter::GetIndexOfFirstVisiblePageObject (
    const Rectangle& aVisibleArea) const
{
    sal_Int32 nRow = GetRowAtPosition (aVisibleArea.Top(), true, GM_BOTH);
    return nRow * mnColumnCount;
}




sal_Int32 Layouter::GetIndexOfLastVisiblePageObject (
    const Rectangle& aVisibleArea) const
{
    sal_Int32 nRow = GetRowAtPosition (aVisibleArea.Bottom(),
        true, GM_BOTH);
    return (nRow+1) * mnColumnCount - 1;
}




sal_Int32 Layouter::GetIndexAtPoint (
    const Point& rPosition,
    bool bIncludePageBorders) const
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




Layouter::DoublePoint
    Layouter::ConvertModelToLayouterCoordinates (
        const Point& rModelPoint) const
{
    sal_Int32 nColumn = GetColumnAtPosition (rModelPoint.X(), true, GM_BOTH);
    sal_Int32 nColumnWidth
        = maPageObjectModelSize.Width() + mnTotalHorizontalGap.mnModel;
    sal_Int32 nDistanceIntoColumn =
        rModelPoint.X() - mnLeftBorder.mnModel - mnLeftPageBorder.mnModel
        - nColumn * nColumnWidth;

    sal_Int32 nRow = GetRowAtPosition (rModelPoint.Y(), true, GM_BOTH);
    sal_Int32 nRowHeight
        = maPageObjectModelSize.Height() + mnTotalVerticalGap.mnModel;
    sal_Int32 nDistanceIntoRow =
        rModelPoint.Y() - mnTopBorder.mnModel - mnTopPageBorder.mnModel
        - nRow * nRowHeight;

    return DoublePoint (
        nColumn + double(nDistanceIntoColumn) / double(nColumnWidth),
        nRow + double(nDistanceIntoRow) / double(nRowHeight));
}




Point Layouter::ConvertLayouterToModelCoordinates (
    const DoublePoint & rLayouterPoint) const
{
    sal_Int32 nColumn = (sal_Int32) ::rtl::math::round(rLayouterPoint.first,
        0,rtl_math_RoundingMode_Floor);
    sal_Int32 nColumnWidth
        = maPageObjectModelSize.Width() + mnTotalHorizontalGap.mnModel;
    sal_Int32 nDistanceIntoColumn
        = (sal_Int32)((rLayouterPoint.first - nColumn) * nColumnWidth);

    sal_Int32 nRow = (sal_Int32) ::rtl::math::round(rLayouterPoint.second,
        0,rtl_math_RoundingMode_Floor);
    sal_Int32 nRowHeight
        = maPageObjectModelSize.Height() + mnTotalVerticalGap.mnModel;
    sal_Int32 nDistanceIntoRow
        = (sal_Int32)((rLayouterPoint.second - nRow) * nRowHeight);

    return Point (
        mnLeftBorder.mnModel + mnLeftPageBorder.mnModel
        + nColumn * nColumnWidth + nDistanceIntoColumn,
        mnTopBorder.mnModel + mnTopPageBorder.mnModel
        + nRow * nRowHeight + nDistanceIntoRow);
}




sal_Int32 Layouter::GetRowAtPosition (
    sal_Int32 nYPosition,
    bool bIncludeBordersAndGaps,
    GapMembership eGapMembership) const
{
    sal_Int32 nRow = -1;

    const sal_Int32 nY = nYPosition
        - mnTopBorder.mnModel - mnTopPageBorder.mnModel;
    if (nY >= 0)
    {
        // Vertical distance from one row to the next.
        const sal_Int32 nRowOffset (
            maPageObjectModelSize.Height() + mnTotalVerticalGap.mnModel);

        // Calculate row consisting of page objects and gap below.
        nRow = nY / nRowOffset;

        const sal_Int32 nDistanceIntoGap (
            (nY - nRow*nRowOffset) - maPageObjectModelSize.Height());
        // When inside the gap below then nYPosition is not over a page
        // object.
        if (nDistanceIntoGap > 0)
            nRow = ResolvePositionInGap (
                nDistanceIntoGap,
                eGapMembership,
                nRow,
                mnBottomPageBorder.mnModel,
                mnVerticalGap.mnModel);
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

    sal_Int32 nX = nXPosition
        - mnLeftBorder.mnModel - mnLeftPageBorder.mnModel;
    if (nX >= 0)
    {
        // Horizontal distance from one column to the next.
        const sal_Int32 nColumnOffset (
            maPageObjectModelSize.Width() + mnTotalHorizontalGap.mnModel);

        // Calculate row consisting of page objects and gap below.
        nColumn = nX / nColumnOffset;

        const sal_Int32 nDistanceIntoGap (
            (nX - nColumn*nColumnOffset) - maPageObjectModelSize.Width());
        // When inside the gap at the right then nXPosition is not over a
        // page object.
        if (nDistanceIntoGap > 0)
            nColumn = ResolvePositionInGap (
                nDistanceIntoGap,
                eGapMembership,
                nColumn,
                mnRightPageBorder.mnModel,
                mnHorizontalGap.mnModel);
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
    sal_Int32 nLeftOrTopPageBorder,
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
            sal_Int32 nFirstHalfGapWidth = nLeftOrTopPageBorder + nGap / 2;
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
            if (nDistanceIntoGap > nLeftOrTopPageBorder)
            {
                if (nDistanceIntoGap > nLeftOrTopPageBorder + nGap)
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




const Layouter::BackgroundRectangleList&
    Layouter::GetBackgroundRectangleList (void) const
{
    return maBackgroundRectangleList;
}




void Layouter::BuildBackgroundRectangleList (void)
{
    maBackgroundRectangleList.clear();
    maBackgroundRectangleList.resize(mnColumnCount*5);
    maBackgroundRectangleList.push_back (
        GetPageObjectBox (10));
}




} } } // end of namespace ::sd::slidesorter::namespace
