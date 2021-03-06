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

#include <controller/SlsFocusManager.hxx>
#include <fupoor.hxx>
#include <memory>

namespace sd::slidesorter
{
class SlideSorter;
}

struct AcceptDropEvent;

namespace sd::slidesorter::controller
{
class SlideSorterController;

class SelectionFunction final : public FuPoor
{
public:
    SelectionFunction(const SelectionFunction&) = delete;
    SelectionFunction& operator=(const SelectionFunction&) = delete;

    static rtl::Reference<FuPoor> Create(SlideSorter& rSlideSorter, SfxRequest& rRequest);

    // Mouse- & Key-Events
    virtual bool KeyInput(const KeyEvent& rKEvt) override;
    virtual bool MouseMove(const MouseEvent& rMEvt) override;
    virtual bool MouseButtonUp(const MouseEvent& rMEvt) override;
    virtual bool MouseButtonDown(const MouseEvent& rMEvt) override;

    /// Forward to the clipboard manager.
    virtual void DoCut() override;

    /// Forward to the clipboard manager.
    virtual void DoCopy() override;

    /// Forward to the clipboard manager.
    virtual void DoPaste() override;

    /** is called when the current function should be aborted. <p>
        This is used when a function gets a KEY_ESCAPE but can also
        be called directly.

        @returns
            true if an active function was aborted
    */
    virtual bool cancel() override;

    void MouseDragged(const AcceptDropEvent& rEvent, const sal_Int8 nDragAction);

    /** Turn of substitution display and insertion indicator.
    */
    void NotifyDragFinished();

    class EventDescriptor;
    class ModeHandler;
    friend class ModeHandler;
    enum Mode
    {
        NormalMode,
        MultiSelectionMode,
        DragAndDropMode
    };
    void SwitchToNormalMode();
    void SwitchToDragAndDropMode(const Point& rMousePosition);
    void SwitchToMultiSelectionMode(const Point& rMousePosition, const sal_uInt32 nEventCode);

    void ResetShiftKeySelectionAnchor();
    /** Special case handling for when the context menu is hidden.  This
        method will reinitialize the current mouse position to prevent the
        mouse motion during the time the context menu is displayed from
        being interpreted as drag-and-drop start.
    */
    void ResetMouseAnchor();

private:
    SlideSorter& mrSlideSorter;
    SlideSorterController& mrController;

    SelectionFunction(SlideSorter& rSlideSorter, SfxRequest& rRequest);

    virtual ~SelectionFunction() override;

    /** Remember the slide where the shift key was pressed and started a
        multiselection via keyboard.
    */
    sal_Int32 mnShiftKeySelectionAnchor;

    /** The selection function can be in one of several mutually
        exclusive modes.
    */
    std::shared_ptr<ModeHandler> mpModeHandler;

    /** Make the slide nOffset slides away of the current one the new
        current slide.  When the new index is outside the range of valid
        page numbers it is clipped to that range.
        @param nOffset
            When nOffset is negative then go back.  When nOffset if positive go
            forward.  When it is zero then ignore the call.
    */
    void GotoNextPage(int nOffset);

    /** Make the slide with the given index the new current slide.
        @param nIndex
            Index of the new current slide.  When the new index is outside
            the range of valid page numbers it is clipped to that range.
    */
    void GotoPage(int nIndex);

    void ProcessMouseEvent(sal_uInt32 nEventType, const MouseEvent& rEvent);

    // What follows are a couple of helper methods that are used by
    // ProcessMouseEvent().

    void ProcessEvent(EventDescriptor& rEvent);

    void MoveFocus(const FocusManager::FocusMoveDirection eDirection, const bool bIsShiftDown,
                   const bool bIsControlDown);

    void SwitchMode(const std::shared_ptr<ModeHandler>& rpHandler);
};

} // end of namespace ::sd::slidesorter::controller

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
