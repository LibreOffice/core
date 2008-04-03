/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsScrollBarManager.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:35:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SD_SLIDESORTER_SLIDE_SORTER_SCROLL_BAR_MANAGER_HXX
#define SD_SLIDESORTER_SLIDE_SORTER_SCROLL_BAR_MANAGER_HXX

#include <tools/link.hxx>
#include <tools/gen.hxx>
#include <vcl/timer.hxx>
#include <boost/shared_ptr.hpp>

class Point;
class Rectangle;
class ScrollBar;
class ScrollBarBox;
class Window;

namespace sd {
class Window;
}

namespace sd { namespace slidesorter {
    class SlideSorter;
} }


namespace sd { namespace slidesorter { namespace controller {

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

    ~ScrollBarManager (void);

    /** Call this method after constructing a new object of this class.
    */
    void LateInitialization (void);

    /** Register listeners at the scroll bars.  This method is called after
        startup of a new slide sorter object or after a reactivation of a
        slide sorter that for example is taken from a cache.
    */
    void Connect (void);

    /** Remove listeners from the scroll bars.  This method is called whent
        the slide sorter is destroyed or when it is suspended, e.g. put
        into a cache for later reuse.
    */
    void Disconnect (void);

    /** Set up the scroll bar, i.e. thumb size and position.  Call this
        method when the content of the browser window changed, i.e. pages
        were inserted or deleted, the layout or the zoom factor has
        changed.
        @param bResetThumbPosition
            When <TRUE/> then set the thumb position to position 0.  This is
            done when e.g. switching between master page mode and draw mode.
        @param bScrollToCurrentPosition
            When <TRUE/> then scroll the window to the new offset that is
            defined by the scroll bars.  Otherwise the new offset is simply
            set and the whole window is repainted.
    */
    void UpdateScrollBars (
        bool bResetThumbPosition = false,
        bool bScrollToCurrentPosition = true);

    /** Place the scroll bars inside the given area.  When the available
        area is not large enough for the content to display the resulting
        behaviour depends on the mbUseVerticalScrollBar flag.  When it is
        set to true then a vertical scroll bar is shown.  Otherwise the
        height of the returned area is enlarged so that the content fits
        into it.
        @param rAvailableArea
            The scroll bars will be placed inside this rectangle.  It is
            expected to be given in pixel relative to its parent.
        @return
            Returns the space that remains after the scroll bars are
            placed.  When the mbUseVerticalScrollBar flag is false then the
            returned rectangle may be larger than the given one.
    */
    Rectangle PlaceScrollBars (const Rectangle& rAvailableArea);

    /** Update the vertical scroll bar so that the visible area has the
        given top value.
    */
    void SetTop (const sal_Int32 nTop);

    /** Update the horizontal scroll bar so that the visible area has the
        given left value.
    */
    void SetLeft (const sal_Int32 nLeft);

    /** Return the width of the vertical scroll bar, which--when
        shown--should be fixed in contrast to its height.
        @return
           Returns 0 when the vertical scroll bar is not shown or does not
           exist, otherwise its width in pixel is returned.
    */
    int GetVerticalScrollBarWidth (void) const;

    /** Return the height of the horizontal scroll bar, which--when
        shown--should be fixed in contrast to its width.
        @return
           Returns 0 when the vertical scroll bar is not shown or does not
           exist, otherwise its height in pixel is returned.
    */
    int GetHorizontalScrollBarHeight (void) const;

    /** Call this method to scroll a window while the mouse is in dragging a
        selection.  If the mouse is near the window border or is outside the
        window then scroll the window accordingly.
        @return
            When the window is scrolled then this method returns <TRUE/>.
            When the window is not changed then <FALSE/> is returned.
    */
    bool AutoScroll (const Point& rMouseWindowPosition);

    void StopAutoScroll (void);

private:
    SlideSorter& mrSlideSorter;

    /** The horizontal scroll bar.  Note that is used but not owned by
        objects of this class.  It is given to the constructor.
    */
    ::boost::shared_ptr<ScrollBar> mpHorizontalScrollBar;

    /** The vertical scroll bar.  Note that is used but not owned by
        objects of this class.  It is given to the constructor.
    */
    ::boost::shared_ptr<ScrollBar> mpVerticalScrollBar;

    /// Relative horizontal position of the visible area in the view.
    double mnHorizontalPosition;
    /// Relative vertical position of the visible area in the view.
    double mnVerticalPosition;
    /** The width and height of the border at the inside of the window which
        when entered while in drag mode leads to a scrolling of the window.
    */
    Size maScrollBorder;
    double mnHorizontalScrollFactor;
    double mnVerticalScrollFactor;
    /** The only task of this little window is to paint the little square at
        the bottom right corner left by the two scroll bars (when both are
        visible).
    */
    ::boost::shared_ptr<ScrollBarBox> mpScrollBarFiller;

    /** The auto scroll timer is used for keep scrolling the window when the
        mouse reaches its border while dragging a selection.  When the mouse
        is not moved the timer issues events to keep scrolling.
    */
    Timer maAutoScrollTimer;
    Size maAutoScrollOffset;

    /** The content window is the one whose view port is controlled by the
        scroll bars.
    */
    ::boost::shared_ptr<sd::Window> mpContentWindow;

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
    Rectangle DetermineScrollBarVisibilities (const Rectangle& rAvailableArea);

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
        const Rectangle& rAvailableArea);

    void CalcAutoScrollOffset (const Point& rMouseWindowPosition);
    bool RepeatAutoScroll (void);

    DECL_LINK(HorizontalScrollBarHandler, ScrollBar*);
    DECL_LINK(VerticalScrollBarHandler, ScrollBar*);
    DECL_LINK(AutoScrollTimeoutHandler, Timer*);

    void PlaceHorizontalScrollBar (const Rectangle& aArea);
    void PlaceVerticalScrollBar (const Rectangle& aArea);
    void PlaceFiller (const Rectangle& aArea);

    /** Make the height of the content window larger or smaller, so that the

        content size fits exactly in.  This is achieved by changing the size
        of the parent window and rely on the resulting resize.
    */
    void AdaptWindowSize (const Rectangle& rArea);
};

} } } // end of namespace ::sd::slidesorter::controller

#endif
