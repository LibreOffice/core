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

#ifndef INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_VIEW_SLSLAYOUTER_HXX
#define INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_VIEW_SLSLAYOUTER_HXX

#include "SlideSorter.hxx"
#include "view/SlsPageObjectLayouter.hxx"
#include "view/SlsTheme.hxx"
#include <sal/types.h>
#include <tools/fract.hxx>
#include <vcl/mapmod.hxx>
#include <vector>
#include <utility>

class Size;

namespace sd { namespace slidesorter { namespace view {

class InsertPosition;

/** Calculate the size and position of page objects displayed by a slide
    sorter.  The layouter takes into account various input values:
    1.) Size of the window in which the slide sorter is displayed.
    2.) Desired and minimal and maximal widths of page objects.
    3.) Minimal and maximal number of columns.
    4.) Vertical and horizontal gaps between objects in adjacent columns.
    5.) Borders around every page object.
    6.) Vertical and horizontal borders between enclosing page and outer
        page objects.
    From these it calculates various output values:
    1.) The width of page objects.
    2.) The number of columns.
    3.) The size of the enclosing page.

    <p>Sizes and lengths are all in pixel except where explicitly stated
    otherwise.</p>

    <p>The GetIndex... methods may return indices that are larger than or
    equal to (zero based) the number of pages.  This is so because the
    number of pages is not known to the class instances.  Indices are
    calculated with reference to the general grid layout of page
    objects.</p>
*/
class Layouter
{
public:
    enum Orientation { HORIZONTAL, VERTICAL, GRID };

    Layouter (
        sd::Window *rpWindow,
        const std::shared_ptr<Theme>& rpTheme);
    ~Layouter();

    std::shared_ptr<PageObjectLayouter> GetPageObjectLayouter() const;
    /** Set the interval of valid column counts.  When nMinimalColumnCount
        <= nMaximalColumnCount is not fulfilled then the call is ignored.
        @param nMinimalColumnCount
            The default value is 1.  The question whether higher values make
            any sense is left to the caller.
        @param nMaximalColumnCount
            The default value is 5.
    */
    void SetColumnCount (sal_Int32 nMinimalColumnCount,
        sal_Int32 nMaximalColumnCount);

    /** Central method of this class.  It takes the input values and
        calculates the output values.  Both given sizes must not be 0 in any
        dimension or the call is ignored.
        @param eOrientation
            This defines the generally layout and specifies whether there may
            be more than one row or more than one column.
        @param rWindowSize
            The size of the window in pixels that the slide sorter is
            displayed in.  This can differ from the size of mpWindow during
            detection of whether or not the scroll bars should be visible.
        @param rPreviewModelSize
            Size of each page in model coordinates.
        @param rpWindow
            The map mode of this window is adapted to the new layout of the
            page objects.
        @return
            The return value indicates whether the Get... methods can be
            used to obtain valid values (<TRUE/>).
    */
    bool Rearrange (
        const Orientation eOrientation,
        const Size& rWindowSize,
        const Size& rPreviewModelSize,
        const sal_uInt32 nPageCount);

    /** Return the number of columns.
    */
    sal_Int32 GetColumnCount() const;

    sal_Int32 GetIndex (const sal_Int32 nRow, const sal_Int32 nColumn) const;

    Size GetPageObjectSize() const;

    /** Return the bounding box in window coordinates of the nIndex-th page
        object.
    */
    Rectangle GetPageObjectBox (
        const sal_Int32 nIndex,
        const bool bIncludeBorderAndGap = false) const;

    /** Return the bounding box in model coordinates of the page that
        contains the given amount of page objects.
    */
    Rectangle GetTotalBoundingBox() const;

    /** Return the index of the first fully or partially visible page
        object.  This takes into account only the vertical dimension.
        @return
            The second index may be larger than the number of existing
            page objects.
    */
    Range GetRangeOfVisiblePageObjects (const Rectangle& rVisibleArea) const;

    /** Return the index of the page object that is rendered at the given
        point.
        @param rPosition
            The position is expected to be in model coordinates relative to
            the page origin.
        @param bIncludePageBorders
            When <TRUE/> then include the page borders into the calculation,
            i.e. when a point lies in the border of a page object but not on
            the actual page area the index of that page is returned;
            otherwise -1 would be returned to indicate that no page object
            has been hit.
        @param bClampToValidRange
            When <TRUE/> then values outside the valid range [0,mnPageCount)
            are mapped to 0 (when smaller than 0) or mnPageCount-1 when
            equal to or larger than mnPageCount.
            When <FALSE/> then -1 is returned for values outside the valid range.
        @return
            The returned index may be larger than the number of existing
            page objects.
    */
    sal_Int32 GetIndexAtPoint (
        const Point& rModelPosition,
        const bool bIncludePageBorders = false,
        const bool bClampToValidRange = true) const;

    /** Return an object that describes the logical and visual properties of
        where to do an insert operation when the user would release the
        mouse button at the given position after a drag operation and of
        where and how to display an insertion indicator.
        @param rModelPosition
            The position in the model coordinate system for which to
            determine the insertion page index.  The position does not have
            to be over a page object to return a valid value.
        @param rIndicatorSize
            The size of the insertion indicator.  This size is used to adapt
            the location when at the left or right of a row or at the top or
            bottom of a column.
        @param rModel
            The model is used to get access to the selection states of the
            pages.  This in turn is used to determine the visual bounding
            boxes.
    */
    InsertPosition GetInsertPosition (
        const Point& rModelPosition,
        const Size& rIndicatorSize,
        model::SlideSorterModel& rModel) const;

    Range GetValidHorizontalSizeRange() const;
    Range GetValidVerticalSizeRange() const;

    class Implementation;

private:
    ::boost::scoped_ptr<Implementation> mpImplementation;
    VclPtr<sd::Window> mpWindow;
};

/** Collect all values concerning the logical and visual properties of the
    insertion position that is used for drag-and-drop and copy-and-past.
*/
class InsertPosition
{
public:
    InsertPosition();
    InsertPosition& operator= (const InsertPosition& rInsertPosition);
    bool operator== (const InsertPosition& rInsertPosition) const;
    bool operator!= (const InsertPosition& rInsertPosition) const;

    void SetLogicalPosition (
        const sal_Int32 nRow,
        const sal_Int32 nColumn,
        const sal_Int32 nIndex,
        const bool bIsAtRunStart,
        const bool bIsAtRunEnd,
        const bool bIsExtraSpaceNeeded);
    void SetGeometricalPosition(
        const Point& rLocation,
        const Point& rLeadingOffset,
        const Point& rTrailingOffset);

    sal_Int32 GetRow() const { return mnRow; }
    sal_Int32 GetColumn() const { return mnColumn; }
    sal_Int32 GetIndex() const { return mnIndex; }
    Point GetLocation() const { return maLocation; }
    Point GetLeadingOffset() const { return maLeadingOffset; }
    Point GetTrailingOffset() const { return maTrailingOffset; }
    bool IsAtRunStart() const { return mbIsAtRunStart; }
    bool IsAtRunEnd() const { return mbIsAtRunEnd; }
    bool IsExtraSpaceNeeded() const { return mbIsExtraSpaceNeeded; }

private:
    sal_Int32 mnRow;
    sal_Int32 mnColumn;
    sal_Int32 mnIndex;
    bool mbIsAtRunStart : 1;
    bool mbIsAtRunEnd : 1;
    bool mbIsExtraSpaceNeeded : 1;
    Point maLocation;
    Point maLeadingOffset;
    Point maTrailingOffset;
};

} } } // end of namespace ::sd::slidesorter::view

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
