/*************************************************************************
 *
 *  $RCSfile: SlsViewOverlay.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:23:54 $
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

#ifndef SD_SLIDESORTER_VIEW_OVERLAY_HXX
#define SD_SLIDESORTER_VIEW_OVERLAY_HXX

#include <osl/mutex.hxx>
#include <tools/gen.hxx>
#include <vector>

class OutputDevice;
class Region;

namespace sd { namespace slidesorter {
class SlideSorterViewShell;
} }

namespace sd { namespace slidesorter { namespace model {
class PageEnumeration;
} } }

namespace sd { namespace slidesorter { namespace controller {
class SlideSorterController;
} } }

namespace sd { namespace slidesorter { namespace view {


class ViewOverlay;
class SelectionRectangleOverlay;
class InsertionIndicatorOverlay;
class SubstitutionOverlay;

/** This base class of overlay graphics keeps track of the visibility of
    graphical objects that possibly are drawn in XOR paint mode.  This makes
    it possibly to switch such an overlay on or off without knowing wether
    it is visible.
*/
class OverlayBase
{
public:
    OverlayBase (ViewOverlay& rViewOverlay);
    virtual ~OverlayBase (void);

    virtual void Paint (void);

    virtual void Show (void);
    virtual void Hide (void);
    void Toggle (void);
    bool IsShowing (void);

protected:
    ::osl::Mutex maMutex;

    ViewOverlay& mrViewOverlay;

    bool mbIsShowing;
};




/** This class manages the substitution display of the page objects.  This
    subsitution is used to visualize the selected page objects during a
    mouse drag operation.
*/
class SubstitutionOverlay
    : public OverlayBase
{
public:
    SubstitutionOverlay (ViewOverlay& rViewOverlay);
    virtual ~SubstitutionOverlay (void);

    virtual void Paint (void);

    /** Setup the substitution display of the given set of selected pages.
        The given mouse position is remembered so that it later can be
        returned by GetPosition(). This is a convenience feature.
    */
    void Create (
        model::PageEnumeration& rSelection,
        const Point& rPosition);

    /** Clear the substitution display.  Until the next call of Create() no
        substution is painted.
    */
    void Clear (void);

    /** Move the substitution display by the given amount of pixels.
    */
    void Move (const Point& rOffset);
    void SetPosition (const Point& rPosition);
    const Point& GetPosition (void) const;

private:
    /// List of page object substitution displays.
    typedef ::std::vector<Rectangle> SubstitutionShapeList;
    SubstitutionShapeList maShapes;
    Point maPosition;
};




class SelectionRectangleOverlay
    : public OverlayBase
{
public:
    SelectionRectangleOverlay (ViewOverlay& rViewOverlay);

    virtual void Paint (void);

    virtual void Hide (void);

    const Rectangle& GetSelectionRectangle (void);

private:
    Rectangle maSelectionRectangle;
};




class InsertionIndicatorOverlay
    : public OverlayBase
{
public:
    InsertionIndicatorOverlay (ViewOverlay& rViewOverlay);

    void SetPositionAndSize (const Rectangle& rBoundingBox);

    virtual void Paint (void);

    /** Given a position in model coordinates this method calculates the
        insertion marker both as an index in the document and as a rectangle
        used for drawing the insertion indicator.
    */
    void SetPosition (const Point& rPosition);

    sal_Int32 GetInsertionPageIndex (void) const;

private:
    Rectangle maBoundingBox;
    sal_Int32 mnInsertionIndex;
};




/** The view overlay manages and paints some indicators that are painted on
    top of the regular view content (the page objects).  It is separated
    from the view to allow the indicators to be altered in position and size
    without to repaint the whole view content (inside that the bounding box
    of the indicator).  One technique to achive this is to use XOR-painting.

    The view overlay itself simply provides the more specialized classes
    that handle individual indicators.

*/
class ViewOverlay
{
public:
    ViewOverlay (SlideSorterViewShell& rViewShell);
    ~ViewOverlay (void);

    SelectionRectangleOverlay& GetSelectionRectangleOverlay (void);
    InsertionIndicatorOverlay& GetInsertionIndicatorOverlay (void);
    SubstitutionOverlay& GetSubstitutionOverlay (void);

    void Paint (void);

    /** The overlay type describes how an overlay is painted.  That can be
        either by using XOR operation or by doing a regular paint.
    */
    enum OverlayPaintType { OPT_ALL, OPT_XOR, OPT_PAINT };

    /** As a preparation for a scrolling--or some other kind of action that
        changes the map mode of a window--this method saves the current
        state of all overlays so that the next call to Restore() can restore
        them.  After that it hides the overlays so they do not corrupt the
        window during the scrolling does.
    */
    void HideAndSave (OverlayPaintType eType = OPT_ALL);

    /** Restore the state of the overlays that has been saved in an earlier
        call of HideAndSave().
    */
    void Restore (void);

    controller::SlideSorterController& GetController (void);
    SlideSorterViewShell& GetViewShell (void);

private:
    SlideSorterViewShell& mrViewShell;
    SelectionRectangleOverlay maSelectionRectangleOverlay;
    InsertionIndicatorOverlay maInsertionIndicatorOverlay;
    SubstitutionOverlay maSubstitutionOverlay;

    /** Remember whether HideAndSave() was called since construction of the
        last Restore() call to save the then current state of the overlays.
    */
    bool mbHasSavedState;
    OverlayPaintType meSavedStateType;

    bool mbSelectionRectangleWasVisible;
    bool mbInsertionIndicatorWasVisible;
    bool mbSubstitutionDisplayWasVisible;
};



} } } // end of namespace ::sd::slidesorter::view

#endif
