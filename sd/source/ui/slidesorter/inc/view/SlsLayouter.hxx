/*************************************************************************
 *
 *  $RCSfile: SlsLayouter.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:22:44 $
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

#ifndef SD_SLIDESORTER_VIEW_LAYOUTER_HXX
#define SD_SLIDESORTER_VIEW_LAYOUTER_HXX

#include <sal/types.h>
#include <tools/fract.hxx>
#include <vcl/mapmod.hxx>
#include <vector>
#include <utility>

class MapMode;
class Size;

namespace sd { namespace slidesorter { namespace view {

/** Calculate the size and position of page objects displayed by a slide
    sorter.  The layouter takes into account various input values:
    1.) Size of the window in which the slide sorter is displayed.
    2.) Desired and minimal and maximal widths of page objects.
    3.) Minimal and maximal number of columns.
    4.) Vertical and horizontal gaps between objects in adjacent columns.
    5.) Borders arround every page object.
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
    Layouter (void);
    ~Layouter (void);

    /** Set the minimal, the maximal, and the desired width of the page
        objects.  The three parameters have to fullfill the constraint
        nMinimalWidth <= nDesiredWidth <= nMaximalWidth or the call is
        ignored.
    */
    void SetObjectWidth (sal_Int32 nMinimalWidth, sal_Int32 nMaximalWidth,
        sal_Int32 nDesiredWidth);

    /** Set the horizontal and vertical borders in pixel coordinates between
        the enclosing window and page objects.  The borders may be painted
        larger then the given values when the space for the insertion marker
        is not sufficient.
        @param nLeftBorder
            Use a negative value to indicate that the left border size
            shall not be modified.  A value of 10 is the default.
        @param nRightBorder
            Use a negative value to indicate that the right border size
            shall not be modified.  A value of 10 is the default.
        @param nTopBorder
            Use a negative value to indicate that the top border size
            shall not be modified.  A value of 10 is the default.
        @param nBottomBorder
            Use a negative value to indicate that the bottom border size
            shall not be modified.  A value of 10 is the default.
    */
    void SetBorders (sal_Int32 nLeftBorder, sal_Int32 nRightBorder,
        sal_Int32 nTopBorder, sal_Int32 nBottomBorder);

    /** Set the borders arround every page object.
        @param nLeftBorder
            A negative value indicates that the left border shall not be
            modified.  A value of 0 is the default.
        @param nRightBorder
            A negative value indicates that the left border shall not be
            modified.  A value of 0 is the default.
        @param nTopBorder
            A negative value indicates that the left border shall not be
            modified.  A value of 0 is the default.
        @param nBottomBorder
            A negative value indicates that the left border shall not be
            modified.  A value of 0 is the default.
    */
    void SetPageBorders (sal_Int32 nLeftBorder, sal_Int32 nRightBorder,
        sal_Int32 nTopBorder, sal_Int32 nBottomBorder);

    /** Set the horizontal and vertical gaps between adjacent page objects.
        These gaps are only relevant when there is more than one column or
        more than one row.  Negative values indicate that the respective gap
        is not set.
    */
    void SetGaps (sal_Int32 nHorizontalGap, sal_Int32 nVerticalGap);

    /** Set the interval of valid column counts.  When nMinimalColumnCount
        <= nMaximalColumnCount is not fullfilled then the call is ignored.
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
        @param rWindowSize
            The size of the window in pixels that the slide sorter is
            displayed in.
        @param rPageObjectSize
            Size of each page in model coordinates.
        @param pDevice
            The map mode of this output device is adapted to the new layout
            of the page objects.
        @return
            The return value indicates whether the Get... methods can be
            used to obtain valid values (<TRUE/>).
    */
    bool Rearrange (
        const Size& rWindowSize,
        const Size& rPageObjectSize,
        OutputDevice* pDevice);

    /** Change the zoom factor.  This does not change the general layout
        (number of columns).
    */
    void SetZoom (double nZoomFactor, OutputDevice* pDevice);
    void SetZoom (Fraction nZoomFactor, OutputDevice* pDevice);

    /** Return the number of columns.
    */
    sal_Int32 GetColumnCount (void) const;

    /** Return the scale factor that can be set at the map mode of the
        output window.
    */
    Fraction GetScaleFactor (void) const;

    /** Return the bounding box in model coordinates of the nIndex-th page
        object.
    */
    Rectangle GetPageObjectBox (sal_Int32 nIndex) const;

    /** Return the bounding box in model coordinates of the page that
        contains the given amount of page objects.
    */
    Rectangle GetPageBox (sal_Int32 nObjectCount) const;

    /** Return the rectangle that bounds the insertion marker that is
        specified by the parameters.
        @param nIndex
            Index of the page object from which the position of the marker
            will be calculated.
        @param bVertical
            When <TRUE/> then the insertion marker will be calculated with a
            vertical orientation positioned to the left or right of the
            specified page object.  A horizontal orientation is indicated by
            <FALSE/>.  In this case the marker will be positioned above or
            below the page object.
        @param bLeftOrTop
            This flag indicates whether the insertion marker will be
            positioned above or to the left (<TRUE/>) the page object.  When
            <FALSE/> is given then the marker will be positioned below or to
            the right of the page object.
    */
    Rectangle GetInsertionMarkerBox (
        sal_Int32 nIndex,
        bool bVertical,
        bool bLeftOrTop) const;

    /** Return the index of the first fully or partially visible page
        object.  This takes into account only the vertical dimension.
    */
    sal_Int32 GetIndexOfFirstVisiblePageObject (
        const Rectangle& rVisibleArea) const;

    /** Return the index of the last fully or partially visible page
        object.  This takes into account only the vertical dimension.
        @return
            The returned index may be larger than the number of existing
            page objects.
    */
    sal_Int32 GetIndexOfLastVisiblePageObject (
        const Rectangle& rVisibleArea) const;

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
        @return
            The returned index may be larger than the number of existing
            page objects.
    */
    sal_Int32 GetIndexAtPoint (
        const Point& rModelPosition,
        bool bIncludePageBorders = false) const;

    /** Return the page index of where to do an insert operation when the
        user would release the the mouse button at the given position after
        a drag operation.
        @param rPosition
            The position in the model coordinate system for which to
            determine the insertion page index.  The position does not have
            to be over a page object to return a valid value.
        @param bAllowVerticalPosition
            When this flag is <TRUE/> then the vertical gaps between rows
            may be taken into account for calculating the insertion index as
            well as the horizontal gaps between columns.  This will happen
            only when there is only one column.
        (better name, anyone?)
        @return
            Returns the page index, as accepted by the slide sorter model,
            of the page after which an insertion would take place.  An index
            of 0 means that insertion will take place before the first page,
            An index equal to or greater than the page count means to insert
            after the last page.
            A value of -1 indicates that no valid insertion index exists for
            the given point.
    */
    sal_Int32 GetInsertionIndex (
        const Point& rModelPosition,
        bool bAllowVerticalPosition) const;

    typedef ::std::pair<double,double> DoublePoint;
    /** Transform a point given in model coordinates in to layouter
        coordinates.  Layouter coordinates are floating point numbers where
        the integer part denotes a row or a column and the part after the
        decimal point is a relative position in that row or column.
    */
    DoublePoint ConvertModelToLayouterCoordinates (
        const Point& rModelPoint) const;

    /** Transform a point given in layouter coordinates to model
        coordinates.  See ConvertModelToLayouterCoordinates for a
        description of layouter coordinates.
    */
    Point ConvertLayouterToModelCoordinates (
        const DoublePoint&rLayouterPoint) const;

    typedef ::std::vector<Rectangle> BackgroundRectangleList;
    const BackgroundRectangleList& GetBackgroundRectangleList (void) const;

private:
    class ScreenAndModelValue {public:
        sal_Int32 mnScreen,mnModel;
        explicit ScreenAndModelValue (sal_Int32 nScreen, sal_Int32 nModel = 0)
            : mnScreen(nScreen),mnModel(nModel) {}
    };
    ScreenAndModelValue mnRequestedLeftBorder;
    ScreenAndModelValue mnRequestedRightBorder;
    ScreenAndModelValue mnRequestedTopBorder;
    ScreenAndModelValue mnRequestedBottomBorder;
    ScreenAndModelValue mnLeftBorder;
    ScreenAndModelValue mnRightBorder;
    ScreenAndModelValue mnTopBorder;
    ScreenAndModelValue mnBottomBorder;
    ScreenAndModelValue mnLeftPageBorder;
    ScreenAndModelValue mnRightPageBorder;
    ScreenAndModelValue mnTopPageBorder;
    ScreenAndModelValue mnBottomPageBorder;
    ScreenAndModelValue mnVerticalGap;
    ScreenAndModelValue mnHorizontalGap;
    ScreenAndModelValue mnInsertionMarkerThickness;
    ScreenAndModelValue mnTotalVerticalGap;
    ScreenAndModelValue mnTotalHorizontalGap;
    sal_Int32 mnMinimalWidth;
    sal_Int32 mnDesiredWidth;
    sal_Int32 mnMaximalWidth;
    sal_Int32 mnMinimalColumnCount;
    sal_Int32 mnMaximalColumnCount;
    sal_Int32 mnColumnCount;
    Size maPageObjectModelSize;
    Size maPageObjectPixelSize;

    BackgroundRectangleList maBackgroundRectangleList;

    enum GapMembership { GM_NONE, GM_PREVIOUS, GM_BOTH, GM_NEXT,
                         GM_PAGE_BORDER};

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
        GapMembership eGapMembership = GM_NONE) const;

    /** Calculate the column that the point with the given horizontal
        coordinate is over.  The verical component is ignored.
        @param nXPosition
            Horizontal position in model coordinates.
        @param bIncludeBordersAndGaps
            When this flag is <TRUE/> then the area of borders and gaps are
            interpreted as belonging to one of the columns.
        @param eGapMembership
            Specifies to what column the gap areas belong.  Here GM_NONE
            corresponds to bIncludeBordersAndGaps being <FALSE/>.  When
            GM_BOTH is given then the left half is associated with the
            column at the left and the right half with the column to the
            right.  Values of GM_PREVIOUS and GM_NEXT associate the whole
            gap area with the column to the left or right respectively.
    */
    sal_Int32 GetColumnAtPosition (
        sal_Int32 nXPosition,
        bool bIncludeBordersAndGaps,
        GapMembership eGapMembership = GM_NONE) const;

    void BuildBackgroundRectangleList (void);

    /** This method is typically called from GetRowAtPosition() and
        GetColumnAtPosition() to handle a position that lies inside the gap
        between two adjacent rows or columns.
        @param nDistanceIntoGap
            Vertical distance from the bottom of the upper row down into the
            gap or or horizontal distance from the right edge right into the
            gap.
        @param eGapMemberhship
            This value decides what areas in the gap belong to which (or no)
            row or column.
        @param nIndex
            The row index of the upper row or the column index of the left
            column.
        @param nLeftOrTopPageBorder
            Width in model coordinates of the border the the right of or
            below a page.
        @param nGap
             Width or height of the gap in model coordiantes between the
             page borders.
        @return
           Returns either the index of the upper row (as given as nRow), the
           index of the lower row (nRow+1) or -1 to indicate that the
           position belongs to no row.
        */
    sal_Int32 ResolvePositionInGap (
        sal_Int32 nDistanceIntoGap,
        GapMembership eGapMembership,
        sal_Int32 nIndex,
        sal_Int32 nLeftOrTopPageBorder,
        sal_Int32 nGap) const;
};

} } } // end of namespace ::sd::slidesorter::view

#endif
