/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SD_SLIDESORTER_SELECTION_FUNCTION_HXX
#define SD_SLIDESORTER_SELECTION_FUNCTION_HXX

#include "model/SlsSharedPageDescriptor.hxx"
#include "controller/SlsFocusManager.hxx"
#include "controller/SlsInsertionIndicatorHandler.hxx"
#include "fupoor.hxx"
#include <svtools/transfer.hxx>
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
class DragAndDropContext;


class SelectionFunction
    : public FuPoor,
      private ::boost::noncopyable
{
public:
    static FunctionReference Create( SlideSorter& rSlideSorter, SfxRequest& rRequest );

    // Mouse- & Key-Events
    virtual bool KeyInput(const KeyEvent& rKEvt);
    virtual bool MouseMove(const MouseEvent& rMEvt);
    virtual bool MouseButtonUp(const MouseEvent& rMEvt);
    virtual bool MouseButtonDown(const MouseEvent& rMEvt);

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
    bool SwitchToButtonMode (void);

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
    void GotoPage (sal_uInt32 nIndex);

    void ProcessMouseEvent (sal_uInt32 nEventType, const MouseEvent& rEvent);
    void ProcessKeyEvent (const KeyEvent& rEvent);

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
