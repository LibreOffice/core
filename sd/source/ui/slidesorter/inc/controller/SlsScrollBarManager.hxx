/*************************************************************************
 *
 *  $RCSfile: SlsScrollBarManager.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:20:26 $
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

#ifndef SD_SLIDESORTER_SLIDE_SORTER_SCROLL_BAR_MANAGER_HXX
#define SD_SLIDESORTER_SLIDE_SORTER_SCROLL_BAR_MANAGER_HXX

#include <tools/link.hxx>
#include <tools/gen.hxx>
#include <vcl/timer.hxx>
#include <memory>

class Point;
class Rectangle;
class ScrollBar;
class ScrollBarBox;
class Window;


namespace sd { namespace slidesorter { namespace controller {

class SlideSorterController;


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
    /** Create a new scroll bar manager that manages three controls: the two
        given scroll bars and a little window it creates and ownes that
        fills the gap at the bottom right corner that is left between the
        two scroll bars.
        @param pParentWindow
            This window is used to create the little filler window.
        @param pContentWindow
            This window holds the content.  Its view area is controlled by
            the scroll bars.  The content window is used to transform
            coordinates and, if the vertical scroll bar is not in use, is
            resized to fully show its content.
        @param pHorizontalScrollBar
            The horizontal scroll bar that is controlled by the new
            manager.  Its ownership remains with the caller.
        @param pVerticalScrollBar
            The horizontal scroll bar that is controlled by the new
            manager.  Its ownership remains with the caller.
        @param pScrollBarFiller
            The task of this filler is to paint the little square enclosed
            by the two scroll bars when they are both visible.
    */
    ScrollBarManager (
        SlideSorterController& rController,
        ::Window* pParentWindow,
        ::Window* pContentWindow,
        ScrollBar* pHorizontalScrollBar,
        ScrollBar* pVerticalScrollBar,
        ScrollBarBox* pScrollBarFiller);

    ~ScrollBarManager (void);
    void LateInitialization (void);

    /** This method tells the scroll bar manager whether to use the vertical
        scroll bar on the next call to PlaceScrollBars().
    */
    void UseVerticalScrollBar (bool bUseVerticalScrollBar);

    /** Set up the scroll bar, i.e. thumb size and position.  Call this
        method when the content of the browser window changed, i.e. pages
        were inserted or deleted, the layout or the zoom factor has
        changed.
        @param bResetThumbPosition
            When <TRUE/> then set the thumb position to position 0.  This is
            done when e.g. switching between master page mode and draw mode.
    */
    void UpdateScrollBars (bool bResetThumbPosition = false);

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
    void SetTop (long nTop);

    /** Return the width of the vertical scroll bar (which should be fixed
        in contrast to its height).
        @return
           Returns 0 when the vertical scroll bar does not exist, otherwise
           its width in pixel is returned.
    */
    int GetVerticalScrollBarWidth (void) const;

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
    SlideSorterController& mrController;
    ScrollBar* mpHorizontalScrollBar;
    ScrollBar* mpVerticalScrollBar;
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
    ScrollBarBox* mpScrollBarFiller;

    /** The auto scroll timer is used for keep scrolling the window when the
        mouse reaches its border while dragging a selection.  When the mouse
        is not moved the timer issues events to keep scrolling.
    */
    Timer maAutoScrollTimer;
    Size maAutoScrollOffset;

    /** This flag indicates whether the vertical scroll bar is displayed when
        necessary or whether it is not ever shown and instead the
        PlaceScrollBars() method enlarges the given area.
    */
    bool mbUseVerticalScrollBar;

    /** The content window is the one whose view port is controlled by the
        scroll bars.
    */
    ::Window* mpContentWindow;

    void SetWindowOrigin (
        double nHorizontalPosition,
        double nVerticalPosition);

    bool IsHorizontalScrollBarVisible (void);

    void CalcAutoScrollOffset (const Point& rMouseWindowPosition);
    bool RepeatAutoScroll (void);

    DECL_LINK(HorizontalScrollBarHandler, ScrollBar*);
    DECL_LINK(VerticalScrollBarHandler, ScrollBar*);
    DECL_LINK(AutoScrollTimeoutHandler, Timer*);

    void PlaceHorizontalScrollBar (Rectangle& aArea);
    void PlaceVerticalScrollBar (Rectangle& aArea);
    void PlaceFiller (Rectangle& aArea);

    /** Make the height of the content window larger or smaller, so that the

        content size fits exactly in.  This is achieved by changing the size
        of the parent window and rely on the resulting resize.
    */
    void AdaptWindowSize (const Rectangle& rArea);
};

} } } // end of namespace ::sd::slidesorter::controller

#endif
