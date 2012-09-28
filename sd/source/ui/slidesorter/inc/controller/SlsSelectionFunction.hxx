/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "controller/SlsInsertionIndicatorHandler.hxx"
#include "fupoor.hxx"
#include <svtools/transfer.hxx>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

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
    virtual sal_Bool KeyInput(const KeyEvent& rKEvt);
    virtual sal_Bool MouseMove(const MouseEvent& rMEvt);
    virtual sal_Bool MouseButtonUp(const MouseEvent& rMEvt);
    virtual sal_Bool MouseButtonDown(const MouseEvent& rMEvt);

    virtual void Activate();
    virtual void Deactivate();

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

    void MouseDragged (
        const AcceptDropEvent& rEvent,
        const sal_Int8 nDragAction);

    /** Turn of substitution display and insertion indicator.
    */
    void NotifyDragFinished (void);

    /** Call when drag-and-drop or multi selection is started or stopped in
        order to update permission of mouse over indication.
    */
    void UpdateMouseOverIndicationPermission (void);

    class EventDescriptor;
    class ModeHandler;
    friend class ModeHandler;
    enum Mode
    {
        NormalMode,
        MultiSelectionMode,
        DragAndDropMode,
        ButtonMode
    };
    void SwitchToNormalMode (void);
    void SwitchToDragAndDropMode(const Point aMousePosition);
    void SwitchToMultiSelectionMode (const Point aMousePosition, const sal_uInt32 nEventCode);

    void ResetShiftKeySelectionAnchor (void);
    /** Special case handling for when the context menu is hidden.  This
        method will reinitialize the current mouse position to prevent the
        mouse motion during the time the context menu is displayed from
        being interpreted as drag-and-drop start.
    */
    void ResetMouseAnchor (void);

protected:
    SlideSorter& mrSlideSorter;
    SlideSorterController& mrController;

    SelectionFunction (
        SlideSorter& rSlideSorter,
        SfxRequest& rRequest);

    virtual ~SelectionFunction();

private:

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

    bool mbIsDeselectionPending;

    /** Remember the slide where the shift key was pressed and started a
        multiselection via keyboard.
    */
    sal_Int32 mnShiftKeySelectionAnchor;

    /** The selection function can be in one of several mutually
        exclusive modes.
    */
    ::boost::shared_ptr<ModeHandler> mpModeHandler;

    /** Make the slide nOffset slides away of the current one the new
        current slide.  When the new index is outside the range of valid
        page numbers it is clipped to that range.
        @param nOffset
            When nOffset is negative then go back.  When nOffset if positive go
            forward.  When it is zero then ignore the call.
    */
    void GotoNextPage (int nOffset);

    /** Make the slide with the given index the new current slide.
        @param nIndex
            Index of the new current slide.  When the new index is outside
            the range of valid page numbers it is clipped to that range.
    */
    void GotoPage (int nIndex);

    void ProcessMouseEvent (sal_uInt32 nEventType, const MouseEvent& rEvent);

    // What follows are a couple of helper methods that are used by
    // ProcessMouseEvent().

    void ProcessEvent (EventDescriptor& rEvent);

    void MoveFocus (
        const FocusManager::FocusMoveDirection eDirection,
        const bool bIsShiftDown,
        const bool bIsControlDown);

    void StopDragAndDrop (void);

    void SwitchMode (const ::boost::shared_ptr<ModeHandler>& rpHandler);
};

} } } // end of namespace ::sd::slidesorter::controller

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
