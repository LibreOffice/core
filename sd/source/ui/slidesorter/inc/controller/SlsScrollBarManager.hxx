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

#pragma once

#include <tools/link.hxx>
#include <tools/gen.hxx>
#include <vcl/timer.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/vclptr.hxx>

#include <functional>

namespace sd { class Window; }

namespace sd::slidesorter { class SlideSorter; }

namespace sd::slidesorter::controller {

/** Manage the horizontal and vertical scroll bars.  Listen for events, set
    their sizes, place them in the window, determine their visibilities.

    <p>Handle auto scrolling, i.e. the scrolling of the window when the
    mouse comes near the window border while dragging a selection.</p>

    <p>In order to make the slide sorter be used in the task pane with its
    own vertical scrollbars the vertical scrollbar of the use of the slide
    sorter is optional.  When using it the available area in a window is
    used and the vertical scrollbar is displayed when that area is not large
    enough.  When the vertical scrollbar is not used then the available area
    is assumed to be modifiable.  In that case the PlaceScrollBars() method
    may return an area larger than the one given.<p>
*/
class ScrollBarManager
{
public:
    /** Create a new scroll bar manager that manages three controls: the
        horizontal scroll bar, the vertical scroll bar, and the little
        window that fills the gap at the bottom right corner that is left
        between the two scroll bars.  Call LateInitialization() after
        constructing a new object.
    */
    ScrollBarManager (SlideSorter& rSlideSorter);

    ~ScrollBarManager();

    /** Register listeners at the scroll bars.  This method is called after
        startup of a new slide sorter object or after a reactivation of a
        slide sorter that for example is taken from a cache.
    */
    void Connect();

    /** Remove listeners from the scroll bars.  This method is called when
        the slide sorter is destroyed or when it is suspended, e.g. put
        into a cache for later reuse.
    */
    void Disconnect();

    /** Set up the scroll bar, i.e. thumb size and position.  Call this
        method when the content of the browser window changed, i.e. pages
        were inserted or deleted, the layout or the zoom factor has
        changed.
        @param bScrollToCurrentPosition
            When <TRUE/> then scroll the window to the new offset that is
            defined by the scroll bars.  Otherwise the new offset is simply
            set and the whole window is repainted.
    */
    void UpdateScrollBars (
        bool bScrollToCurrentPosition);

    /** Place the scroll bars inside the given area.  When the available
        area is not large enough for the content to display the horizontal
        and/or vertical scroll bar is enabled.
        @param rAvailableArea
            The scroll bars will be placed inside this rectangle.  It is
            expected to be given in pixel relative to its parent.
        @param bIsHorizontalScrollBarAllowed
            Only when this flag is <TRUE/> the horizontal scroll may be
            displayed.
        @param bIsVerticalScrollBarAllowed
            Only when this flag is <TRUE/> the horizontal scroll may be
            displayed.
        @return
            Returns the space that remains after the scroll bars are
            placed.
    */
    ::tools::Rectangle PlaceScrollBars (
        const ::tools::Rectangle& rAvailableArea,
        const bool bIsHorizontalScrollBarAllowed,
        const bool bIsVerticalScrollBarAllowed);

    /** Update the vertical and horizontal scroll bars so that the visible
        area has the given top and left values.
    */
    void SetTopLeft (const Point& rNewTopLeft);

    /** Return the width of the vertical scroll bar, which--when
        shown--should be fixed in contrast to its height.
        @return
           Returns 0 when the vertical scroll bar is not shown or does not
           exist, otherwise its width in pixel is returned.
    */
    int GetVerticalScrollBarWidth() const;

    /** Return the height of the horizontal scroll bar, which--when
        shown--should be fixed in contrast to its width.
        @return
           Returns 0 when the vertical scroll bar is not shown or does not
           exist, otherwise its height in pixel is returned.
    */
    int GetHorizontalScrollBarHeight() const;

    /** Call this method to scroll a window while the mouse is in dragging a
        selection.  If the mouse is near the window border or is outside the
        window then scroll the window accordingly.
        @param rMouseWindowPosition
            The mouse position for which the scroll amount is calculated.
        @param rAutoScrollFunctor
            Every time when the window is scrolled then this functor is executed.
        @return
            When the window is scrolled then this method returns <TRUE/>.
            When the window is not changed then <FALSE/> is returned.
    */
    bool AutoScroll (
        const Point& rMouseWindowPosition,
        const ::std::function<void ()>& rAutoScrollFunctor);

    void StopAutoScroll();

    void clearAutoScrollFunctor();

    enum Orientation { Orientation_Horizontal, Orientation_Vertical };
    /** Scroll the slide sorter by setting the thumbs of the scroll bars and
        by moving the content of the content window.
        @param eOrientation
            Defines whether to scroll horizontally or vertically.
        @param nDistance
            distance in slides.
    */
    void Scroll(
        const Orientation eOrientation,
        const sal_Int32 nDistance);

private:
    SlideSorter& mrSlideSorter;

    /** The horizontal scroll bar.  Note that is used but not owned by
        objects of this class.  It is given to the constructor.
    */
    VclPtr<ScrollBar> mpHorizontalScrollBar;

    /** The vertical scroll bar.  Note that is used but not owned by
        objects of this class.  It is given to the constructor.
    */
    VclPtr<ScrollBar> mpVerticalScrollBar;

    /// Relative horizontal position of the visible area in the view.
    double mnHorizontalPosition;
    /// Relative vertical position of the visible area in the view.
    double mnVerticalPosition;
    /** The width and height of the border at the inside of the window which
        when entered while in drag mode leads to a scrolling of the window.
    */
    Size maScrollBorder;
    /** The only task of this little window is to paint the little square at
        the bottom right corner left by the two scroll bars (when both are
        visible).
    */
    VclPtr<ScrollBarBox> mpScrollBarFiller;

    /** The auto scroll timer is used for keep scrolling the window when the
        mouse reaches its border while dragging a selection.  When the mouse
        is not moved the timer issues events to keep scrolling.
    */
    Timer maAutoScrollTimer;
    Size maAutoScrollOffset;
    bool mbIsAutoScrollActive;

    /** The content window is the one whose view port is controlled by the
        scroll bars.
    */
    VclPtr<sd::Window> mpContentWindow;

    ::std::function<void ()> maAutoScrollFunctor;

    void SetWindowOrigin (
        double nHorizontalPosition,
        double nVerticalPosition);

    /** Determine the visibility of the scroll bars so that the window
        content is not clipped in any dimension without showing a scroll
        bar.
        @param rAvailableArea
            The area in which the scroll bars, the scroll bar filler, and
            the SlideSorterView will be placed.
        @return
            The area that is enclosed by the scroll bars is returned.  It
            will be filled with the SlideSorterView.
    */
    ::tools::Rectangle DetermineScrollBarVisibilities(
        const ::tools::Rectangle& rAvailableArea,
        const bool bIsHorizontalScrollBarAllowed,
        const bool bIsVerticalScrollBarAllowed);

    /** Typically called by DetermineScrollBarVisibilities() this method
        tests a specific configuration of the two scroll bars being visible
        or hidden.
        @return
            When the window content can be shown with only being clipped in
            an orientation where the scroll bar would be shown then <TRUE/>
            is returned.
    */
    bool TestScrollBarVisibilities (
        bool bHorizontalScrollBarVisible,
        bool bVerticalScrollBarVisible,
        const ::tools::Rectangle& rAvailableArea);

    void CalcAutoScrollOffset (const Point& rMouseWindowPosition);
    bool RepeatAutoScroll();

    DECL_LINK(HorizontalScrollBarHandler, ScrollBar*, void);
    DECL_LINK(VerticalScrollBarHandler, ScrollBar*, void);
    DECL_LINK(AutoScrollTimeoutHandler, Timer *, void);

    void PlaceHorizontalScrollBar (const ::tools::Rectangle& aArea);
    void PlaceVerticalScrollBar (const ::tools::Rectangle& aArea);
    void PlaceFiller (const ::tools::Rectangle& aArea);
};

} // end of namespace ::sd::slidesorter::controller

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
