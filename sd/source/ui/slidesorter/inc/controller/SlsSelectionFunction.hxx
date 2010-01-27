/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SlsSelectionFunction.hxx,v $
 * $Revision: 1.14 $
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

#ifndef SD_SLIDESORTER_SELECTION_FUNCTION_HXX
#define SD_SLIDESORTER_SELECTION_FUNCTION_HXX

#include "model/SlsSharedPageDescriptor.hxx"
#include "controller/SlsFocusManager.hxx"
#include "fupoor.hxx"
#include <svtools/transfer.hxx>
//#include <memory>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

class SdSlideViewShell;
class SdWindow;
class SdSlideView;
class SdDrawDocument;
class Sound;

namespace sd { namespace slidesorter {
class SlideSorter;
} }

namespace sd { namespace slidesorter { namespace controller {

class SlideSorterController;

class SelectionFunction
    : public FuPoor,
      private ::boost::noncopyable
{
public:
    TYPEINFO();

    static FunctionReference Create( SlideSorter& rSlideSorter, SfxRequest& rRequest );

    // Mouse- & Key-Events
    virtual BOOL KeyInput(const KeyEvent& rKEvt);
    virtual BOOL MouseMove(const MouseEvent& rMEvt);
    virtual BOOL MouseButtonUp(const MouseEvent& rMEvt);
    virtual BOOL MouseButtonDown(const MouseEvent& rMEvt);

    virtual void Activate();
    virtual void Deactivate();

    virtual void ScrollStart();
    virtual void ScrollEnd();

    /// Forward to the clipboard manager.
    virtual void DoCut (void);

    /// Forward to the clipboard manager.
    virtual void DoCopy (void);

    /// Forward to the clipboard manager.
    virtual void DoPaste (void);

    /** is called when the current function should be aborted. <p>
        This is used when a function gets a KEY_ESCAPE but can also
        be called directly.

        @returns
            true if a active function was aborted
    */
    virtual bool cancel();

    void MouseDragged (const AcceptDropEvent& rEvent);

protected:
    SlideSorter& mrSlideSorter;
    SlideSorterController& mrController;

    SelectionFunction (
        SlideSorter& rSlideSorter,
        SfxRequest& rRequest);

    virtual ~SelectionFunction();

private:
    class SubstitutionHandler;
    class RectangleSelector;
    class EventDescriptor;

    /// Set in MouseButtonDown this flag indicates that a page has been hit.
    bool mbPageHit;

    /// The rectangle of the mouse drag selection.
    Rectangle maDragSelectionRectangle;
    bool mbDragSelection;

    /// Box of the insert marker in model coordinates.
    Rectangle maInsertionMarkerBox;

    /** We use this flag to filter out the cases where MouseMotion() is called
        with a pressed mouse button but without a prior MouseButtonDown()
        call.  This is an indication that the mouse button was pressed over
        another control, e.g. the view tab bar, and that a re-layout of the
        controls moved the slide sorter under the mouse.
    */
    bool mbProcessingMouseButtonDown;

    ::boost::scoped_ptr<SubstitutionHandler> mpSubstitutionHandler;
    ::boost::scoped_ptr<RectangleSelector> mpRectangleSelector;

    /** Remember where the left mouse button was pressed.
    */
    sal_Int32 mnButtonDownPageIndex;
    sal_Int32 mnButtonDownButtonIndex;

    bool mbIsDeselectionPending;

    /** Remember the slide where the shift key was pressed and started a
        multiselection via keyboard.
    */
    sal_Int32 mnShiftKeySelectionAnchor;

    DECL_LINK( DragSlideHdl, Timer* );
    void StartDrag (const Point& rMousePosition);

    /** Set the selection to exactly the specified page and also set it as
        the current page.
    */
    void SetCurrentPage (const model::SharedPageDescriptor& rpDescriptor);

    /** When the view on which this selection function is working is the
        main view then the view is switched to the regular editing view.
    */
    void SwitchView (const model::SharedPageDescriptor& rpDescriptor);

    /** Make the slide nOffset slides away of the current one the new
        current slide.  When the new index is outside the range of valid
        page numbers it is clipped to that range.
        @param nOffset
            When nOffset is negative then go back.  When nOffset if positive go
            forward.  When it is zero then ignore the call.
    */
    void GotoNextPage (int nOffset);

    void ProcessMouseEvent (sal_uInt32 nEventType, const MouseEvent& rEvent);
    void ProcessKeyEvent (const KeyEvent& rEvent);

    // What follows are a couple of helper methods that are used by
    // ProcessMouseEvent().

    /// Deselect all pages.
    void DeselectAllPages (void);

    /** For a possibly following mouse motion by starting the drag timer
        that after a short time of pressed but un-moved mouse starts a drag
        operation.
    */
    void StartDragTimer (void);

    /** Select all pages between and including the selection anchor and the
        specified page.
    */
    void RangeSelect (const model::SharedPageDescriptor& rpDescriptor);

    /** Hide and clear the insertion indiciator, substitution display and
        selection rectangle.
    */
    void ClearOverlays (void);

    /** Compute a numerical code that describes a mouse event and that can
        be used for fast look up of the appropriate reaction.
    */
    sal_uInt32 EncodeMouseEvent (
        const EventDescriptor& rDescriptor,
        const MouseEvent& rEvent) const;

    /** Compute a numerical code that describes a key event and that can
        be used for fast look up of the appropriate reaction.
    */
    sal_uInt32 EncodeKeyEvent (
        const EventDescriptor& rDescriptor,
        const KeyEvent& rEvent) const;

    /** Compute a numerical code that describes the current state like
        whether the selection rectangle is visible or whether the page under
        the mouse or the one that has the focus is selected.
    */
    sal_uInt32 EncodeState (const EventDescriptor& rDescriptor) const;

    void EventPreprocessing (const EventDescriptor& rEvent);
    bool EventProcessing (const EventDescriptor& rEvent);
    void EventPostprocessing (const EventDescriptor& rEvent);

    void UpdatePageUnderMouse (
        const Point& rMousePosition,
        const bool bIsMouseButtonDown);

    void ProcessButtonClick (
        const model::SharedPageDescriptor& rpDescriptor,
        const sal_Int32 nButtonIndex);

    void MoveFocus (
        const FocusManager::FocusMoveDirection eDirection,
        const bool bIsShiftDown);
};

} } } // end of namespace ::sd::slidesorter::controller

#endif

