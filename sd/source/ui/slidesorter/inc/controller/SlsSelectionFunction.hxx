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

#ifndef INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_CONTROLLER_SLSSELECTIONFUNCTION_HXX
#define INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_CONTROLLER_SLSSELECTIONFUNCTION_HXX

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
    TYPEINFO_OVERRIDE();

    static rtl::Reference<FuPoor> Create( SlideSorter& rSlideSorter, SfxRequest& rRequest );

    // Mouse- & Key-Events
    virtual bool KeyInput(const KeyEvent& rKEvt) SAL_OVERRIDE;
    virtual bool MouseMove(const MouseEvent& rMEvt) SAL_OVERRIDE;
    virtual bool MouseButtonUp(const MouseEvent& rMEvt) SAL_OVERRIDE;
    virtual bool MouseButtonDown(const MouseEvent& rMEvt) SAL_OVERRIDE;

    virtual void Activate() SAL_OVERRIDE;
    virtual void Deactivate() SAL_OVERRIDE;

    /// Forward to the clipboard manager.
    virtual void DoCut() SAL_OVERRIDE;

    /// Forward to the clipboard manager.
    virtual void DoCopy() SAL_OVERRIDE;

    /// Forward to the clipboard manager.
    virtual void DoPaste() SAL_OVERRIDE;

    /** is called when the current function should be aborted. <p>
        This is used when a function gets a KEY_ESCAPE but can also
        be called directly.

        @returns
            true if a active function was aborted
    */
    virtual bool cancel() SAL_OVERRIDE;

    void MouseDragged (
        const AcceptDropEvent& rEvent,
        const sal_Int8 nDragAction);

    /** Turn of substitution display and insertion indicator.
    */
    void NotifyDragFinished();

    /** Call when drag-and-drop or multi selection is started or stopped in
        order to update permission of mouse over indication.
    */
    void UpdateMouseOverIndicationPermission();

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
    void SwitchToNormalMode();
    void SwitchToDragAndDropMode(const Point& rMousePosition);
    void SwitchToMultiSelectionMode (const Point& rMousePosition, const sal_uInt32 nEventCode);

    void ResetShiftKeySelectionAnchor();
    /** Special case handling for when the context menu is hidden.  This
        method will reinitialize the current mouse position to prevent the
        mouse motion during the time the context menu is displayed from
        being interpreted as drag-and-drop start.
    */
    void ResetMouseAnchor();

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

    void StopDragAndDrop();

    void SwitchMode (const ::boost::shared_ptr<ModeHandler>& rpHandler);
};

} } } // end of namespace ::sd::slidesorter::controller

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
