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

#include <memory>
#include <sal/config.h>

#include <cstdlib>

#include <controller/SlsSelectionFunction.hxx>

#include <SlideSorter.hxx>
#include <SlideSorterViewShell.hxx>
#include "SlsDragAndDropContext.hxx"
#include <controller/SlsTransferableData.hxx>
#include <controller/SlideSorterController.hxx>
#include <controller/SlsPageSelector.hxx>
#include <controller/SlsFocusManager.hxx>
#include <controller/SlsScrollBarManager.hxx>
#include <controller/SlsClipboard.hxx>
#include <controller/SlsCurrentSlideManager.hxx>
#include <controller/SlsInsertionIndicatorHandler.hxx>
#include <controller/SlsSelectionManager.hxx>
#include <controller/SlsProperties.hxx>
#include <controller/SlsSlotManager.hxx>
#include <controller/SlsVisibleAreaManager.hxx>
#include <model/SlideSorterModel.hxx>
#include <model/SlsPageDescriptor.hxx>
#include <model/SlsPageEnumerationProvider.hxx>
#include <view/SlideSorterView.hxx>
#include <view/SlsLayouter.hxx>
#include <view/SlsPageObjectLayouter.hxx>
#include <framework/FrameworkHelper.hxx>
#include <ViewShellBase.hxx>
#include <DrawController.hxx>
#include <Window.hxx>
#include <sdpage.hxx>
#include <drawdoc.hxx>
#include <DrawDocShell.hxx>
#include <sdxfer.hxx>
#include <ViewShell.hxx>
#include <FrameView.hxx>
#include <app.hrc>
#include <o3tl/deleter.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svxids.hrc>
#include <vcl/ptrstyle.hxx>
#include <boost/optional.hpp>
#include <sdmod.hxx>

namespace {
static const sal_uInt32 SINGLE_CLICK             (0x00000001);
static const sal_uInt32 DOUBLE_CLICK             (0x00000002);
static const sal_uInt32 LEFT_BUTTON              (0x00000010);
static const sal_uInt32 RIGHT_BUTTON             (0x00000020);
static const sal_uInt32 MIDDLE_BUTTON            (0x00000040);
static const sal_uInt32 BUTTON_DOWN              (0x00000100);
static const sal_uInt32 BUTTON_UP                (0x00000200);
static const sal_uInt32 MOUSE_MOTION             (0x00000400);
static const sal_uInt32 MOUSE_DRAG               (0x00000800);
// The rest leaves the lower 16 bit untouched so that it can be used with
// key codes.
static const sal_uInt32 OVER_SELECTED_PAGE       (0x00010000);
static const sal_uInt32 OVER_UNSELECTED_PAGE     (0x00020000);
static const sal_uInt32 SHIFT_MODIFIER           (0x00200000);
static const sal_uInt32 CONTROL_MODIFIER         (0x00400000);

// Some absent events are defined so they can be expressed explicitly.
static const sal_uInt32 NO_MODIFIER              (0x00000000);
static const sal_uInt32 NOT_OVER_PAGE            (0x00000000);

// Masks
static const sal_uInt32 MODIFIER_MASK            (SHIFT_MODIFIER | CONTROL_MODIFIER);

} // end of anonymous namespace

// Define some macros to make the following switch statement more readable.
#define ANY_MODIFIER(code)                  \
         code|NO_MODIFIER:                  \
    case code|SHIFT_MODIFIER:               \
    case code|CONTROL_MODIFIER

namespace sd { namespace slidesorter { namespace controller {

//===== SelectionFunction::EventDescriptor ====================================

class SelectionFunction::EventDescriptor
{
public:
    Point const maMousePosition;
    Point maMouseModelPosition;
    model::SharedPageDescriptor mpHitDescriptor;
    SdrPage* mpHitPage;
    sal_uInt32 mnEventCode;
    InsertionIndicatorHandler::Mode const meDragMode;
    bool mbIsLeaving;

    EventDescriptor (
        sal_uInt32 nEventType,
        const MouseEvent& rEvent,
        SlideSorter const & rSlideSorter);
    EventDescriptor (
        sal_uInt32 nEventType,
        const AcceptDropEvent& rEvent,
        const sal_Int8 nDragAction,
        SlideSorter const & rSlideSorter);

private:
    /** Compute a numerical code that describes a mouse event and that can
        be used for fast look up of the appropriate reaction.
    */
    sal_uInt32 EncodeMouseEvent (const MouseEvent& rEvent) const;

    /** Compute a numerical code that describes the current state like
        whether the selection rectangle is visible or whether the page under
        the mouse or the one that has the focus is selected.
    */
    sal_uInt32 EncodeState() const;
};

//===== SelectionFunction::ModeHandler ========================================

class SelectionFunction::ModeHandler
{
public:
    ModeHandler (
        SlideSorter& rSlideSorter,
        SelectionFunction& rSelectionFunction,
        const bool bIsMouseOverIndicatorAllowed);
    virtual ~ModeHandler() COVERITY_NOEXCEPT_FALSE;

    virtual Mode GetMode() const = 0;
    virtual void Abort() = 0;
    virtual void ProcessEvent (EventDescriptor& rDescriptor);

    /** Set the selection to exactly the specified page and also set it as
        the current page.
    */
    void SetCurrentPage (const model::SharedPageDescriptor& rpDescriptor);

    /// Deselect all pages.
    void DeselectAllPages();
    void SelectOnePage (const model::SharedPageDescriptor& rpDescriptor);

        /** When the view on which this selection function is working is the
        main view then the view is switched to the regular editing view.
    */
    void SwitchView (const model::SharedPageDescriptor& rpDescriptor);

    void StartDrag (
        const Point& rMousePosition);

    bool IsMouseOverIndicatorAllowed() const { return mbIsMouseOverIndicatorAllowed;}

protected:
    SlideSorter& mrSlideSorter;
    SelectionFunction& mrSelectionFunction;

    virtual bool ProcessButtonDownEvent (EventDescriptor& rDescriptor);
    virtual bool ProcessButtonUpEvent (EventDescriptor& rDescriptor);
    virtual bool ProcessMotionEvent (EventDescriptor& rDescriptor);
    virtual bool ProcessDragEvent (EventDescriptor& rDescriptor);
    virtual bool HandleUnprocessedEvent (EventDescriptor& rDescriptor);

    void ReprocessEvent (EventDescriptor& rDescriptor);

private:
    const bool mbIsMouseOverIndicatorAllowed;
};

/** This is the default handler for processing events.  It activates the
    multi selection or drag-and-drop when the right conditions are met.
*/
class NormalModeHandler : public SelectionFunction::ModeHandler
{
public:
    NormalModeHandler (
        SlideSorter& rSlideSorter,
        SelectionFunction& rSelectionFunction);

    virtual SelectionFunction::Mode GetMode() const override;
    virtual void Abort() override;

    void ResetButtonDownLocation();

protected:
    virtual bool ProcessButtonDownEvent (SelectionFunction::EventDescriptor& rDescriptor) override;
    virtual bool ProcessButtonUpEvent (SelectionFunction::EventDescriptor& rDescriptor) override;
    virtual bool ProcessMotionEvent (SelectionFunction::EventDescriptor& rDescriptor) override;
    virtual bool ProcessDragEvent (SelectionFunction::EventDescriptor& rDescriptor) override;

private:
    ::boost::optional<Point> maButtonDownLocation;

    /** Select all pages between and including the selection anchor and the
        specified page.
    */
    void RangeSelect (const model::SharedPageDescriptor& rpDescriptor);
};

/** Handle events during a multi selection, which typically is started by
    pressing the left mouse button when not over a page.
*/
class MultiSelectionModeHandler : public SelectionFunction::ModeHandler
{
public:
    /** Start a rectangle selection at the given position.
    */
    MultiSelectionModeHandler (
        SlideSorter& rSlideSorter,
        SelectionFunction& rSelectionFunction,
        const Point& rMouseModelPosition,
        const sal_uInt32 nEventCode);

    virtual ~MultiSelectionModeHandler() override;

    virtual SelectionFunction::Mode GetMode() const override;
    virtual void Abort() override;
    virtual void ProcessEvent (SelectionFunction::EventDescriptor& rDescriptor) override;

    enum SelectionMode { SM_Normal, SM_Add, SM_Toggle };

    void SetSelectionMode (const SelectionMode eSelectionMode);
    void SetSelectionModeFromModifier (const sal_uInt32 nEventCode);

protected:
    virtual bool ProcessButtonUpEvent (SelectionFunction::EventDescriptor& rDescriptor) override;
    virtual bool ProcessMotionEvent (SelectionFunction::EventDescriptor& rDescriptor) override;
    virtual bool HandleUnprocessedEvent (SelectionFunction::EventDescriptor& rDescriptor) override;

private:
    SelectionMode meSelectionMode;
    Point maSecondCorner;
    PointerStyle const maSavedPointer;
    bool mbAutoScrollInstalled;
    sal_Int32 mnAnchorIndex;
    sal_Int32 mnSecondIndex;

    void UpdateModelPosition (const Point& rMouseModelPosition);
    void UpdateSelection();

    /** Update the rectangle selection so that the given position becomes
        the new second point of the selection rectangle.
    */
    void UpdatePosition (
        const Point& rMousePosition,
        const bool bAllowAutoScroll);

    void UpdateSelectionState (
        const model::SharedPageDescriptor& rpDescriptor,
        const bool bIsInSelection) const;
};

/** Handle events during drag-and-drop.
*/
class DragAndDropModeHandler : public SelectionFunction::ModeHandler
{
public:
    DragAndDropModeHandler (
        SlideSorter& rSlideSorter,
        SelectionFunction& rSelectionFunction,
        const Point& rMousePosition,
        vcl::Window* pWindow);
    virtual ~DragAndDropModeHandler() override;

    virtual SelectionFunction::Mode GetMode() const override;
    virtual void Abort() override;

protected:
    virtual bool ProcessButtonUpEvent (SelectionFunction::EventDescriptor& rDescriptor) override;
    virtual bool ProcessDragEvent (SelectionFunction::EventDescriptor& rDescriptor) override;

private:
    std::unique_ptr<DragAndDropContext, o3tl::default_delete<DragAndDropContext>> mpDragAndDropContext;
};

//===== SelectionFunction =====================================================


SelectionFunction::SelectionFunction (
    SlideSorter& rSlideSorter,
    SfxRequest& rRequest)
    : FuPoor (
        rSlideSorter.GetViewShell(),
        rSlideSorter.GetContentWindow(),
        &rSlideSorter.GetView(),
        rSlideSorter.GetModel().GetDocument(),
        rRequest),
      mrSlideSorter(rSlideSorter),
      mrController(mrSlideSorter.GetController()),
      mnShiftKeySelectionAnchor(-1),
      mpModeHandler(new NormalModeHandler(rSlideSorter, *this))
{
}

SelectionFunction::~SelectionFunction()
{
    mpModeHandler.reset();
}

rtl::Reference<FuPoor> SelectionFunction::Create(
    SlideSorter& rSlideSorter,
    SfxRequest& rRequest)
{
    rtl::Reference<FuPoor> xFunc( new SelectionFunction( rSlideSorter, rRequest ) );
    return xFunc;
}

bool SelectionFunction::MouseButtonDown (const MouseEvent& rEvent)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode (rEvent.GetButtons());
    aMDPos = rEvent.GetPosPixel();

    //  mpWindow->CaptureMouse();

    ProcessMouseEvent(BUTTON_DOWN, rEvent);

    return true;
}

bool SelectionFunction::MouseMove (const MouseEvent& rEvent)
{
    ProcessMouseEvent(MOUSE_MOTION, rEvent);
    return true;
}

bool SelectionFunction::MouseButtonUp (const MouseEvent& rEvent)
{
    mrController.GetScrollBarManager().StopAutoScroll ();

    ProcessMouseEvent(BUTTON_UP, rEvent);

    return true;
}

void SelectionFunction::NotifyDragFinished()
{
    SwitchToNormalMode();
}

bool SelectionFunction::KeyInput (const KeyEvent& rEvent)
{
    view::SlideSorterView::DrawLock aDrawLock (mrSlideSorter);
    PageSelector::BroadcastLock aBroadcastLock (mrSlideSorter);
    PageSelector::UpdateLock aLock (mrSlideSorter);
    FocusManager& rFocusManager (mrController.GetFocusManager());
    bool bResult = false;

    const vcl::KeyCode& rCode (rEvent.GetKeyCode());
    switch (rCode.GetCode())
    {
        case KEY_RETURN:
        {
            model::SharedPageDescriptor pDescriptor (rFocusManager.GetFocusedPageDescriptor());
            ViewShell* pViewShell = mrSlideSorter.GetViewShell();
            if (rFocusManager.HasFocus() && pDescriptor && pViewShell!=nullptr)
            {
                // The Return key triggers different functions depending on
                // whether the slide sorter is the main view or displayed in
                // the right pane.
                if (pViewShell->IsMainViewShell())
                {
                    mpModeHandler->SetCurrentPage(pDescriptor);
                    mpModeHandler->SwitchView(pDescriptor);
                }
                else if (pViewShell->GetDispatcher() != nullptr)
                {
                    pViewShell->GetDispatcher()->Execute(
                        SID_INSERTPAGE,
                        SfxCallMode::ASYNCHRON | SfxCallMode::RECORD);
                }
                bResult = true;
            }
            break;
        }

        case KEY_TAB:
            if ( ! rFocusManager.IsFocusShowing())
            {
                rFocusManager.ShowFocus();
                bResult = true;
            }
            break;

        case KEY_ESCAPE:
            // When there is an active multiselection or drag-and-drop
            // operation then stop that.
            mpModeHandler->Abort();
            SwitchToNormalMode();
            bResult = true;
            break;

        case KEY_SPACE:
        {
            // Toggle the selection state.
            model::SharedPageDescriptor pDescriptor (rFocusManager.GetFocusedPageDescriptor());
            if (pDescriptor && rCode.IsMod1())
            {
                if (pDescriptor->HasState(model::PageDescriptor::ST_Selected))
                    mrController.GetPageSelector().DeselectPage(pDescriptor, false);
                else
                    mrController.GetPageSelector().SelectPage(pDescriptor);
            }
            bResult = true;
        }
        break;

        // Move the focus indicator left.
        case KEY_LEFT:
            MoveFocus(FocusManager::FocusMoveDirection::Left, rCode.IsShift(), rCode.IsMod1());
            bResult = true;
            break;

        // Move the focus indicator right.
        case KEY_RIGHT:
            MoveFocus(FocusManager::FocusMoveDirection::Right, rCode.IsShift(), rCode.IsMod1());
            bResult = true;
            break;

        // Move the focus indicator up.
        case KEY_UP:
            MoveFocus(FocusManager::FocusMoveDirection::Up, rCode.IsShift(), rCode.IsMod1());
            bResult = true;
            break;

        // Move the focus indicator down.
        case KEY_DOWN:
            MoveFocus(FocusManager::FocusMoveDirection::Down, rCode.IsShift(), rCode.IsMod1());
            bResult = true;
            break;

        // Go to previous page.  No wrap around.
        case KEY_PAGEUP:
            GotoNextPage(-1);
            bResult = true;
            break;

        // Go to next page.  No wrap around..
        case KEY_PAGEDOWN:
            GotoNextPage(+1);
            bResult = true;
            break;

        case KEY_HOME:
            GotoPage(0);
            bResult = true;
            break;

        case KEY_END:
            GotoPage(mrSlideSorter.GetModel().GetPageCount()-1);
            bResult = true;
            break;

        case KEY_DELETE:
        case KEY_BACKSPACE:
        {
            if (mrSlideSorter.GetProperties()->IsUIReadOnly())
                break;

            mrController.GetSelectionManager()->DeleteSelectedPages(rCode.GetCode()==KEY_DELETE);

            mnShiftKeySelectionAnchor = -1;
            bResult = true;
        }
        break;

        case KEY_F10:
            if (rCode.IsShift())
            {
                mpModeHandler->SelectOnePage(
                    mrSlideSorter.GetController().GetFocusManager().GetFocusedPageDescriptor());
            }
            break;

        default:
            break;
    }

    if ( ! bResult)
        bResult = FuPoor::KeyInput(rEvent);

    return bResult;
}

void SelectionFunction::MoveFocus (
    const FocusManager::FocusMoveDirection eDirection,
    const bool bIsShiftDown,
    const bool bIsControlDown)
{
    // Remember the anchor of shift key multi selection.
    if (bIsShiftDown)
    {
        if (mnShiftKeySelectionAnchor<0)
        {
            model::SharedPageDescriptor pFocusedDescriptor (
                mrController.GetFocusManager().GetFocusedPageDescriptor());
            mnShiftKeySelectionAnchor = pFocusedDescriptor->GetPageIndex();
        }
    }
    else if ( ! bIsControlDown)
        ResetShiftKeySelectionAnchor();

    mrController.GetFocusManager().MoveFocus(eDirection);

    PageSelector& rSelector (mrController.GetPageSelector());
    model::SharedPageDescriptor pFocusedDescriptor (
        mrController.GetFocusManager().GetFocusedPageDescriptor());
    if (bIsShiftDown)
    {
        // When shift is pressed then select all pages in the range between
        // the currently and the previously focused pages, including them.
        if (pFocusedDescriptor)
        {
            sal_Int32 nPageRangeEnd (pFocusedDescriptor->GetPageIndex());
            model::PageEnumeration aPages (
                model::PageEnumerationProvider::CreateAllPagesEnumeration(
                    mrSlideSorter.GetModel()));
            while (aPages.HasMoreElements())
            {
                model::SharedPageDescriptor pDescriptor (aPages.GetNextElement());
                if (pDescriptor)
                {
                    const sal_Int32 nPageIndex(pDescriptor->GetPageIndex());
                    if ((nPageIndex>=mnShiftKeySelectionAnchor && nPageIndex<=nPageRangeEnd)
                        || (nPageIndex<=mnShiftKeySelectionAnchor && nPageIndex>=nPageRangeEnd))
                    {
                        rSelector.SelectPage(pDescriptor);
                    }
                    else
                    {
                        rSelector.DeselectPage(pDescriptor);
                    }
                }
            }
        }
    }
    else if (bIsControlDown)
    {
        // When control is pressed then do not alter the selection or the
        // current page, just move the focus.
    }
    else
    {
        // Without shift just select the focused page.
        mpModeHandler->SelectOnePage(pFocusedDescriptor);
    }
}

void SelectionFunction::DoCut()
{
    if ( ! mrSlideSorter.GetProperties()->IsUIReadOnly())
    {
        mrController.GetClipboard().DoCut();
    }
}

void SelectionFunction::DoCopy()
{
    mrController.GetClipboard().DoCopy();
}

void SelectionFunction::DoPaste()
{
    if ( ! mrSlideSorter.GetProperties()->IsUIReadOnly())
    {
        mrController.GetClipboard().DoPaste();
    }
}

bool SelectionFunction::cancel()
{
    mrController.GetFocusManager().ToggleFocus();
    return true;
}

void SelectionFunction::GotoNextPage (int nOffset)
{
    model::SharedPageDescriptor pDescriptor
        = mrController.GetCurrentSlideManager()->GetCurrentSlide();
    if (pDescriptor.get() != nullptr)
    {
        SdPage* pPage = pDescriptor->GetPage();
        OSL_ASSERT(pPage!=nullptr);
        sal_Int32 nIndex = (pPage->GetPageNum()-1) / 2;
        GotoPage(nIndex + nOffset);
    }
    ResetShiftKeySelectionAnchor();
}

void SelectionFunction::GotoPage (int nIndex)
{
    sal_uInt16 nPageCount = static_cast<sal_uInt16>(mrSlideSorter.GetModel().GetPageCount());

    if (nIndex >= nPageCount)
        nIndex = nPageCount - 1;
    if (nIndex < 0)
        nIndex = 0;

    mrController.GetFocusManager().SetFocusedPage(nIndex);
    model::SharedPageDescriptor pNextPageDescriptor (
        mrSlideSorter.GetModel().GetPageDescriptor (nIndex));
    if (pNextPageDescriptor.get() != nullptr)
        mpModeHandler->SetCurrentPage(pNextPageDescriptor);
    else
    {
        OSL_ASSERT(pNextPageDescriptor.get() != nullptr);
    }
    ResetShiftKeySelectionAnchor();
}

void SelectionFunction::ProcessMouseEvent (sal_uInt32 nEventType, const MouseEvent& rEvent)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode (rEvent.GetButtons());

    EventDescriptor aEventDescriptor (nEventType, rEvent, mrSlideSorter);
    ProcessEvent(aEventDescriptor);
}

void SelectionFunction::MouseDragged (
    const AcceptDropEvent& rEvent,
    const sal_Int8 nDragAction)
{
    EventDescriptor aEventDescriptor (MOUSE_DRAG, rEvent, nDragAction, mrSlideSorter);
    ProcessEvent(aEventDescriptor);
}

void SelectionFunction::ProcessEvent (EventDescriptor& rDescriptor)
{
    // The call to ProcessEvent may switch to another mode handler.
    // Prevent the untimely destruction of the called handler  by acquiring a
    // temporary reference here.
    std::shared_ptr<ModeHandler> pModeHandler (mpModeHandler);
    pModeHandler->ProcessEvent(rDescriptor);
}

static bool Match (
    const sal_uInt32 nEventCode,
    const sal_uInt32 nPositivePattern)
{
    return (nEventCode & nPositivePattern)==nPositivePattern;
}

void SelectionFunction::SwitchToNormalMode()
{
    if (mpModeHandler->GetMode() != NormalMode)
        SwitchMode(std::shared_ptr<ModeHandler>(
            new NormalModeHandler(mrSlideSorter, *this)));
}

void SelectionFunction::SwitchToDragAndDropMode (const Point& rMousePosition)
{
    if (mpModeHandler->GetMode() == DragAndDropMode)
        return;

    SwitchMode(std::shared_ptr<ModeHandler>(
        new DragAndDropModeHandler(mrSlideSorter, *this, rMousePosition, mpWindow)));
}

void SelectionFunction::SwitchToMultiSelectionMode (
    const Point& rMousePosition,
    const sal_uInt32 nEventCode)
{
    if (mpModeHandler->GetMode() != MultiSelectionMode)
        SwitchMode(std::shared_ptr<ModeHandler>(
            new MultiSelectionModeHandler(mrSlideSorter, *this, rMousePosition, nEventCode)));
}

void SelectionFunction::SwitchMode (const std::shared_ptr<ModeHandler>& rpHandler)
{
    // Not all modes allow mouse over indicator.
    if (mpModeHandler->IsMouseOverIndicatorAllowed() != rpHandler->IsMouseOverIndicatorAllowed())
    {
        if ( ! rpHandler->IsMouseOverIndicatorAllowed())
        {
            mrSlideSorter.GetView().SetPageUnderMouse(model::SharedPageDescriptor());
        }
        else
            mrSlideSorter.GetView().UpdatePageUnderMouse();
    }

    mpModeHandler = rpHandler;
}

void SelectionFunction::ResetShiftKeySelectionAnchor()
{
    mnShiftKeySelectionAnchor = -1;
}

void SelectionFunction::ResetMouseAnchor()
{
    if (mpModeHandler && mpModeHandler->GetMode() == NormalMode)
    {
        std::shared_ptr<NormalModeHandler> pHandler (
            std::dynamic_pointer_cast<NormalModeHandler>(mpModeHandler));
        if (pHandler)
            pHandler->ResetButtonDownLocation();
    }
}

//===== EventDescriptor =======================================================

SelectionFunction::EventDescriptor::EventDescriptor (
    const sal_uInt32 nEventType,
    const MouseEvent& rEvent,
    SlideSorter const & rSlideSorter)
    : maMousePosition(rEvent.GetPosPixel()),
      maMouseModelPosition(),
      mpHitDescriptor(),
      mpHitPage(),
      mnEventCode(nEventType),
      meDragMode(InsertionIndicatorHandler::MoveMode),
      mbIsLeaving(false)
{
    maMouseModelPosition = rSlideSorter.GetContentWindow()->PixelToLogic(maMousePosition);
    mpHitDescriptor = rSlideSorter.GetController().GetPageAt(maMousePosition);
    if (mpHitDescriptor)
    {
        mpHitPage = mpHitDescriptor->GetPage();
    }

    mnEventCode |= EncodeMouseEvent(rEvent);
    mnEventCode |= EncodeState();

    // Detect the mouse leaving the window.  When not button is pressed then
    // we can call IsLeaveWindow at the event.  Otherwise we have to make an
    // explicit test.
    mbIsLeaving = rEvent.IsLeaveWindow()
        || ! ::tools::Rectangle(Point(0,0),
             rSlideSorter.GetContentWindow()->GetOutputSizePixel()).IsInside(maMousePosition);
}

SelectionFunction::EventDescriptor::EventDescriptor (
    const sal_uInt32 nEventType,
    const AcceptDropEvent& rEvent,
    const sal_Int8 nDragAction,
    SlideSorter const & rSlideSorter)
    : maMousePosition(rEvent.maPosPixel),
      maMouseModelPosition(),
      mpHitDescriptor(),
      mpHitPage(),
      mnEventCode(nEventType),
      meDragMode(InsertionIndicatorHandler::GetModeFromDndAction(nDragAction)),
      mbIsLeaving(false)
{
    maMouseModelPosition = rSlideSorter.GetContentWindow()->PixelToLogic(maMousePosition);
    mpHitDescriptor = rSlideSorter.GetController().GetPageAt(maMousePosition);
    if (mpHitDescriptor)
    {
        mpHitPage = mpHitDescriptor->GetPage();
    }

    mnEventCode |= EncodeState();

    // Detect the mouse leaving the window.  When not button is pressed then
    // we can call IsLeaveWindow at the event.  Otherwise we have to make an
    // explicit test.
    mbIsLeaving = rEvent.mbLeaving
        || ! ::tools::Rectangle(Point(0,0),
             rSlideSorter.GetContentWindow()->GetOutputSizePixel()).IsInside(maMousePosition);
}

sal_uInt32 SelectionFunction::EventDescriptor::EncodeMouseEvent (
    const MouseEvent& rEvent) const
{
    // Initialize with the type of mouse event.
    sal_uInt32 nEventCode (mnEventCode & (BUTTON_DOWN | BUTTON_UP | MOUSE_MOTION));

    // Detect the affected button.
    switch (rEvent.GetButtons())
    {
        case MOUSE_LEFT:   nEventCode |= LEFT_BUTTON; break;
        case MOUSE_RIGHT:  nEventCode |= RIGHT_BUTTON; break;
        case MOUSE_MIDDLE: nEventCode |= MIDDLE_BUTTON; break;
    }

    // Detect the number of clicks.
    switch (rEvent.GetClicks())
    {
        case 1: nEventCode |= SINGLE_CLICK; break;
        case 2: nEventCode |= DOUBLE_CLICK; break;
    }

    // Detect pressed modifier keys.
    if (rEvent.IsShift())
        nEventCode |= SHIFT_MODIFIER;
    if (rEvent.IsMod1())
        nEventCode |= CONTROL_MODIFIER;

    return nEventCode;
}

sal_uInt32 SelectionFunction::EventDescriptor::EncodeState() const
{
    sal_uInt32 nEventCode (0);

    // Detect whether the event has happened over a page object.
    if (mpHitPage!=nullptr && mpHitDescriptor)
    {
        if (mpHitDescriptor->HasState(model::PageDescriptor::ST_Selected))
            nEventCode |= OVER_SELECTED_PAGE;
        else
            nEventCode |= OVER_UNSELECTED_PAGE;
    }

    return nEventCode;
}

//===== SelectionFunction::ModeHandler ========================================

SelectionFunction::ModeHandler::ModeHandler (
    SlideSorter& rSlideSorter,
    SelectionFunction& rSelectionFunction,
    const bool bIsMouseOverIndicatorAllowed)
    : mrSlideSorter(rSlideSorter),
      mrSelectionFunction(rSelectionFunction),
      mbIsMouseOverIndicatorAllowed(bIsMouseOverIndicatorAllowed)
{
}

SelectionFunction::ModeHandler::~ModeHandler() COVERITY_NOEXCEPT_FALSE
{
}

void SelectionFunction::ModeHandler::ReprocessEvent (EventDescriptor& rDescriptor)
{
    mrSelectionFunction.ProcessEvent(rDescriptor);
}

void SelectionFunction::ModeHandler::ProcessEvent (
    SelectionFunction::EventDescriptor& rDescriptor)
{
    PageSelector::BroadcastLock aBroadcastLock (mrSlideSorter);
    PageSelector::UpdateLock aUpdateLock (mrSlideSorter);

    bool bIsProcessed (false);
    switch (rDescriptor.mnEventCode & (BUTTON_DOWN | BUTTON_UP | MOUSE_MOTION | MOUSE_DRAG))
    {
        case BUTTON_DOWN:
            bIsProcessed = ProcessButtonDownEvent(rDescriptor);
            break;

        case BUTTON_UP:
            bIsProcessed = ProcessButtonUpEvent(rDescriptor);
            break;

        case MOUSE_MOTION:
            bIsProcessed = ProcessMotionEvent(rDescriptor);
            break;

        case MOUSE_DRAG:
            bIsProcessed = ProcessDragEvent(rDescriptor);
            break;
    }

    if ( ! bIsProcessed)
        HandleUnprocessedEvent(rDescriptor);
}

bool SelectionFunction::ModeHandler::ProcessButtonDownEvent (EventDescriptor&)
{
    return false;
}

bool SelectionFunction::ModeHandler::ProcessButtonUpEvent (EventDescriptor&)
{
    mrSelectionFunction.SwitchToNormalMode();
    return false;
}

bool SelectionFunction::ModeHandler::ProcessMotionEvent (EventDescriptor& rDescriptor)
{
    if (mbIsMouseOverIndicatorAllowed)
        mrSlideSorter.GetView().UpdatePageUnderMouse(rDescriptor.maMousePosition);

    if (rDescriptor.mbIsLeaving)
    {
        mrSelectionFunction.SwitchToNormalMode();
        mrSlideSorter.GetView().SetPageUnderMouse(model::SharedPageDescriptor());

        return true;
    }
    else
        return false;
}

bool SelectionFunction::ModeHandler::ProcessDragEvent (EventDescriptor&)
{
    return false;
}

bool SelectionFunction::ModeHandler::HandleUnprocessedEvent (EventDescriptor&)
{
    return false;
}

void SelectionFunction::ModeHandler::SetCurrentPage (
    const model::SharedPageDescriptor& rpDescriptor)
{
    SelectOnePage(rpDescriptor);
    mrSlideSorter.GetController().GetCurrentSlideManager()->SwitchCurrentSlide(rpDescriptor);
}

void SelectionFunction::ModeHandler::DeselectAllPages()
{
    mrSlideSorter.GetController().GetPageSelector().DeselectAllPages();
    mrSelectionFunction.ResetShiftKeySelectionAnchor();
}

void SelectionFunction::ModeHandler::SelectOnePage (
    const model::SharedPageDescriptor& rpDescriptor)
{
    DeselectAllPages();
    mrSlideSorter.GetController().GetPageSelector().SelectPage(rpDescriptor);
}

void SelectionFunction::ModeHandler::SwitchView (const model::SharedPageDescriptor& rpDescriptor)
{
    // Switch to the draw view.  This is done only when the current
    // view is the main view.
    ViewShell* pViewShell = mrSlideSorter.GetViewShell();
    if (pViewShell==nullptr || !pViewShell->IsMainViewShell())
        return;

    if (rpDescriptor.get()!=nullptr && rpDescriptor->GetPage()!=nullptr)
    {
        mrSlideSorter.GetModel().GetDocument()->SetSelected(rpDescriptor->GetPage(), true);
        pViewShell->GetFrameView()->SetSelectedPage(
            (rpDescriptor->GetPage()->GetPageNum()-1)/2);
    }
    if (mrSlideSorter.GetViewShellBase() != nullptr)
    framework::FrameworkHelper::Instance(*mrSlideSorter.GetViewShellBase())->RequestView(
        framework::FrameworkHelper::msImpressViewURL,
        framework::FrameworkHelper::msCenterPaneURL);
}

void SelectionFunction::ModeHandler::StartDrag (
    const Point& rMousePosition)
{
    // Do not start a drag-and-drop operation when one is already active.
    // (when dragging pages from one document into another, pressing a
    // modifier key can trigger a MouseMotion event in the originating
    // window (focus still in there).  Together with the mouse button pressed
    // (drag-and-drop is active) this triggers the start of drag-and-drop.)
    if (SD_MOD()->pTransferDrag != nullptr)
        return;

    if ( ! mrSlideSorter.GetProperties()->IsUIReadOnly())
    {
        mrSelectionFunction.SwitchToDragAndDropMode(rMousePosition);
    }
}

//===== NormalModeHandler =====================================================

NormalModeHandler::NormalModeHandler (
    SlideSorter& rSlideSorter,
    SelectionFunction& rSelectionFunction)
    : ModeHandler(rSlideSorter, rSelectionFunction, true),
      maButtonDownLocation()
{
}

SelectionFunction::Mode NormalModeHandler::GetMode() const
{
    return SelectionFunction::NormalMode;
}

void NormalModeHandler::Abort()
{
}

bool NormalModeHandler::ProcessButtonDownEvent (
    SelectionFunction::EventDescriptor& rDescriptor)
{
    // Remember the location where the left button is pressed.  With
    // that we can filter away motion events that are caused by key
    // presses.  We also can tune the minimal motion distance that
    // triggers a drag-and-drop operation.
    if ((rDescriptor.mnEventCode & BUTTON_DOWN) != 0)
        maButtonDownLocation = rDescriptor.maMousePosition;

    switch (rDescriptor.mnEventCode)
    {
        case BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | OVER_UNSELECTED_PAGE:
            SetCurrentPage(rDescriptor.mpHitDescriptor);
            break;

        case BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | OVER_SELECTED_PAGE:
            break;

        case BUTTON_DOWN | LEFT_BUTTON | DOUBLE_CLICK | OVER_SELECTED_PAGE:
        case BUTTON_DOWN | LEFT_BUTTON | DOUBLE_CLICK | OVER_UNSELECTED_PAGE:
            // A double click always shows the selected slide in the center
            // pane in an edit view.
            SetCurrentPage(rDescriptor.mpHitDescriptor);
            SwitchView(rDescriptor.mpHitDescriptor);
            break;

        case BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | OVER_SELECTED_PAGE | SHIFT_MODIFIER:
        case BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | OVER_UNSELECTED_PAGE | SHIFT_MODIFIER:
            // Range selection with the shift modifier.
            RangeSelect(rDescriptor.mpHitDescriptor);
            break;

            // Right button for context menu.
        case BUTTON_DOWN | RIGHT_BUTTON | SINGLE_CLICK | OVER_UNSELECTED_PAGE:
            // Single right click and shift+F10 select as preparation to
            // show the context menu.  Change the selection only when the
            // page under the mouse is not selected.  In this case the
            // selection is set to this single page.  Otherwise the
            // selection is not modified.
            SetCurrentPage(rDescriptor.mpHitDescriptor);
            break;

        case BUTTON_DOWN | RIGHT_BUTTON | SINGLE_CLICK | OVER_SELECTED_PAGE:
            // Do not change the selection.  Just adjust the insertion indicator.
            break;

        case BUTTON_DOWN | RIGHT_BUTTON | SINGLE_CLICK | NOT_OVER_PAGE:
            // Remember the current selection so that when a multi selection
            // is started, we can restore the previous selection.
            mrSlideSorter.GetModel().SaveCurrentSelection();
            DeselectAllPages();
            break;

        case ANY_MODIFIER(BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | NOT_OVER_PAGE):
            // Remember the current selection so that when a multi selection
            // is started, we can restore the previous selection.
            mrSlideSorter.GetModel().SaveCurrentSelection();
            DeselectAllPages();
            break;

        case BUTTON_DOWN | LEFT_BUTTON | DOUBLE_CLICK | NOT_OVER_PAGE:
        {
            // Insert a new slide:
            // First of all we need to set the insertion indicator which sets the
            // position where the new slide will be inserted.
            std::shared_ptr<InsertionIndicatorHandler> pInsertionIndicatorHandler
                = mrSlideSorter.GetController().GetInsertionIndicatorHandler();

            pInsertionIndicatorHandler->Start(false);
            pInsertionIndicatorHandler->UpdatePosition(
                    rDescriptor.maMousePosition,
                    InsertionIndicatorHandler::MoveMode);

            mrSlideSorter.GetController().GetSelectionManager()->SetInsertionPosition(
                pInsertionIndicatorHandler->GetInsertionPageIndex());

            mrSlideSorter.GetViewShell()->GetDispatcher()->Execute(
                SID_INSERTPAGE,
                SfxCallMode::ASYNCHRON | SfxCallMode::RECORD);

            pInsertionIndicatorHandler->End(Animator::AM_Immediate);

            break;
        }

        default:
            return false;
    }
    return true;
}

bool NormalModeHandler::ProcessButtonUpEvent (
    SelectionFunction::EventDescriptor& rDescriptor)
{
    bool bIsProcessed (true);
    switch (rDescriptor.mnEventCode)
    {
        case BUTTON_UP | LEFT_BUTTON | SINGLE_CLICK | OVER_SELECTED_PAGE:
            SetCurrentPage(rDescriptor.mpHitDescriptor);
            break;

            // Multi selection with the control modifier.
        case BUTTON_UP | LEFT_BUTTON | SINGLE_CLICK | OVER_SELECTED_PAGE | CONTROL_MODIFIER:
            mrSlideSorter.GetController().GetPageSelector().DeselectPage(
                rDescriptor.mpHitDescriptor);
            break;

        case BUTTON_UP | LEFT_BUTTON | SINGLE_CLICK | OVER_UNSELECTED_PAGE | CONTROL_MODIFIER:
            mrSlideSorter.GetController().GetPageSelector().SelectPage(
                rDescriptor.mpHitDescriptor);
            mrSlideSorter.GetView().SetPageUnderMouse(rDescriptor.mpHitDescriptor);
            break;
        case BUTTON_UP | LEFT_BUTTON | SINGLE_CLICK | NOT_OVER_PAGE:
            break;

        default:
            bIsProcessed = false;
            break;
    }
    mrSelectionFunction.SwitchToNormalMode();
    return bIsProcessed;
}

bool NormalModeHandler::ProcessMotionEvent (
    SelectionFunction::EventDescriptor& rDescriptor)
{
    if (ModeHandler::ProcessMotionEvent(rDescriptor))
        return true;

    bool bIsProcessed (true);
    switch (rDescriptor.mnEventCode)
    {
        // A mouse motion without visible substitution starts that.
        case ANY_MODIFIER(MOUSE_MOTION | LEFT_BUTTON | SINGLE_CLICK | OVER_UNSELECTED_PAGE):
        case ANY_MODIFIER(MOUSE_MOTION | LEFT_BUTTON | SINGLE_CLICK | OVER_SELECTED_PAGE):
        {
            if (maButtonDownLocation)
            {
                const sal_Int32 nDistance(std::max(
                    std::abs(maButtonDownLocation->X() - rDescriptor.maMousePosition.X()),
                    std::abs(maButtonDownLocation->Y() - rDescriptor.maMousePosition.Y())));
                if (nDistance > 3)
                    StartDrag(rDescriptor.maMousePosition);
            }
            break;
        }

        // A mouse motion not over a page starts a rectangle selection.
        case ANY_MODIFIER(MOUSE_MOTION | LEFT_BUTTON | SINGLE_CLICK | NOT_OVER_PAGE):
            mrSelectionFunction.SwitchToMultiSelectionMode(
                rDescriptor.maMouseModelPosition,
                rDescriptor.mnEventCode);
            break;

        default:
            bIsProcessed = false;
            break;
    }
    return bIsProcessed;
}

bool NormalModeHandler::ProcessDragEvent (SelectionFunction::EventDescriptor& rDescriptor)
{
    mrSelectionFunction.SwitchToDragAndDropMode(rDescriptor.maMousePosition);
    ReprocessEvent(rDescriptor);
    return true;
}

void NormalModeHandler::RangeSelect (const model::SharedPageDescriptor& rpDescriptor)
{
    PageSelector::UpdateLock aLock (mrSlideSorter);
    PageSelector& rSelector (mrSlideSorter.GetController().GetPageSelector());

    model::SharedPageDescriptor pAnchor (rSelector.GetSelectionAnchor());
    DeselectAllPages();

    if (pAnchor.get() == nullptr)
        return;

    // Select all pages between the anchor and the given one, including
    // the two.
    const sal_uInt16 nAnchorIndex ((pAnchor->GetPage()->GetPageNum()-1) / 2);
    const sal_uInt16 nOtherIndex ((rpDescriptor->GetPage()->GetPageNum()-1) / 2);

    // Iterate over all pages in the range.  Start with the anchor
    // page.  This way the PageSelector will recognize it again as
    // anchor (the first selected page after a DeselectAllPages()
    // becomes the anchor.)
    const sal_uInt16 nStep ((nAnchorIndex < nOtherIndex) ? +1 : -1);
    sal_uInt16 nIndex (nAnchorIndex);
    while (true)
    {
        rSelector.SelectPage(nIndex);
        if (nIndex == nOtherIndex)
            break;
        nIndex = nIndex + nStep;
    }
}

void NormalModeHandler::ResetButtonDownLocation()
{
    maButtonDownLocation = ::boost::optional<Point>();
}

//===== MultiSelectionModeHandler =============================================

MultiSelectionModeHandler::MultiSelectionModeHandler (
    SlideSorter& rSlideSorter,
    SelectionFunction& rSelectionFunction,
    const Point& rMouseModelPosition,
    const sal_uInt32 nEventCode)
    : ModeHandler(rSlideSorter, rSelectionFunction, false),
      meSelectionMode(SM_Normal),
      maSecondCorner(rMouseModelPosition),
      maSavedPointer(mrSlideSorter.GetContentWindow()->GetPointer()),
      mbAutoScrollInstalled(false),
      mnAnchorIndex(-1),
      mnSecondIndex(-1)
{

    mrSlideSorter.GetContentWindow()->SetPointer(PointerStyle::Text);
    SetSelectionModeFromModifier(nEventCode);
}

MultiSelectionModeHandler::~MultiSelectionModeHandler()
{
    if (mbAutoScrollInstalled)
    {
        //a call to this handler's MultiSelectionModeHandler::UpdatePosition
        //may be still waiting to be called back
        mrSlideSorter.GetController().GetScrollBarManager().clearAutoScrollFunctor();
    }
    mrSlideSorter.GetContentWindow()->SetPointer(maSavedPointer);
}

SelectionFunction::Mode MultiSelectionModeHandler::GetMode() const
{
    return SelectionFunction::MultiSelectionMode;
}

void MultiSelectionModeHandler::Abort()
{
    mrSlideSorter.GetView().RequestRepaint(mrSlideSorter.GetModel().RestoreSelection());
}

void MultiSelectionModeHandler::ProcessEvent (
    SelectionFunction::EventDescriptor& rDescriptor)
{
    // During a multi selection we do not want sudden jumps of the
    // visible area caused by moving newly selected pages into view.
    // Therefore disable that temporarily.  The disabled object is
    // released at the end of the event processing, after the focus and
    // current slide have been updated.
    VisibleAreaManager::TemporaryDisabler aDisabler (mrSlideSorter);

    ModeHandler::ProcessEvent(rDescriptor);
}

bool MultiSelectionModeHandler::ProcessButtonUpEvent (
    SelectionFunction::EventDescriptor& rDescriptor)
{
    if (mbAutoScrollInstalled)
    {
        //a call to this handler's MultiSelectionModeHandler::UpdatePosition
        //may be still waiting to be called back
        mrSlideSorter.GetController().GetScrollBarManager().clearAutoScrollFunctor();
        mbAutoScrollInstalled = false;
    }

    if (Match(rDescriptor.mnEventCode, BUTTON_UP | LEFT_BUTTON | SINGLE_CLICK))
    {
        mrSelectionFunction.SwitchToNormalMode();
        return true;
    }
    else
        return false;
}

bool MultiSelectionModeHandler::ProcessMotionEvent (
    SelectionFunction::EventDescriptor& rDescriptor)
{
    // The selection rectangle is visible.  Handle events accordingly.
    if (Match(rDescriptor.mnEventCode, MOUSE_MOTION | LEFT_BUTTON | SINGLE_CLICK))
    {
        SetSelectionModeFromModifier(rDescriptor.mnEventCode);
        UpdatePosition(rDescriptor.maMousePosition, true);
        return true;
    }
    else
        return false;
}

bool MultiSelectionModeHandler::HandleUnprocessedEvent (
    SelectionFunction::EventDescriptor& rDescriptor)
{
    if ( ! ModeHandler::HandleUnprocessedEvent(rDescriptor))
    {
        // If the event has not been processed then stop multi selection.
        mrSelectionFunction.SwitchToNormalMode();
        ReprocessEvent(rDescriptor);
    }
    return true;
}

void MultiSelectionModeHandler::UpdatePosition (
    const Point& rMousePosition,
    const bool bAllowAutoScroll)
{
    VisibleAreaManager::TemporaryDisabler aDisabler (mrSlideSorter);

    // Convert window coordinates into model coordinates (we need the
    // window coordinates for auto-scrolling because that remains
    // constant while scrolling.)
    sd::Window *pWindow (mrSlideSorter.GetContentWindow().get());
    const Point aMouseModelPosition (pWindow->PixelToLogic(rMousePosition));

    bool bDoAutoScroll = bAllowAutoScroll && mrSlideSorter.GetController().GetScrollBarManager().AutoScroll(
        rMousePosition,
        [this, &rMousePosition] () { return this->UpdatePosition(rMousePosition, false); });

    if (!bDoAutoScroll)
        UpdateModelPosition(aMouseModelPosition);

    mbAutoScrollInstalled |= bDoAutoScroll;
}

void MultiSelectionModeHandler::SetSelectionModeFromModifier (
    const sal_uInt32 nEventCode)
{
    switch (nEventCode & MODIFIER_MASK)
    {
        case NO_MODIFIER:
            SetSelectionMode(SM_Normal);
            break;

        case SHIFT_MODIFIER:
            SetSelectionMode(SM_Add);
            break;

        case CONTROL_MODIFIER:
            SetSelectionMode(SM_Toggle);
            break;
    }
}

void MultiSelectionModeHandler::SetSelectionMode (const SelectionMode eSelectionMode)
{
    if (meSelectionMode != eSelectionMode)
    {
        meSelectionMode = eSelectionMode;
        UpdateSelection();
    }
}

void MultiSelectionModeHandler::UpdateSelectionState (
    const model::SharedPageDescriptor& rpDescriptor,
    const bool bIsInSelection) const
{
    // Determine whether the page was selected before the rectangle
    // selection was started.
    const bool bWasSelected (rpDescriptor->HasState(model::PageDescriptor::ST_WasSelected));

    // Combine the two selection states depending on the selection mode.
    bool bSelect (false);
    switch(meSelectionMode)
    {
        case SM_Normal:
            bSelect = bIsInSelection;
            break;

        case SM_Add:
            bSelect = bIsInSelection || bWasSelected;
            break;

        case SM_Toggle:
            if (bIsInSelection)
                bSelect = !bWasSelected;
            else
                bSelect = bWasSelected;
            break;
    }

    // Set the new selection state.
    if (bSelect)
        mrSlideSorter.GetController().GetPageSelector().SelectPage(rpDescriptor);
    else
        mrSlideSorter.GetController().GetPageSelector().DeselectPage(rpDescriptor);
}

void MultiSelectionModeHandler::UpdateModelPosition (const Point& rMouseModelPosition)
{
    maSecondCorner = rMouseModelPosition;
    UpdateSelection();
}

void MultiSelectionModeHandler::UpdateSelection()
{
    view::SlideSorterView::DrawLock aLock (mrSlideSorter);

    model::SlideSorterModel& rModel (mrSlideSorter.GetModel());
    const sal_Int32 nPageCount (rModel.GetPageCount());

    const sal_Int32 nIndexUnderMouse (
        mrSlideSorter.GetView().GetLayouter().GetIndexAtPoint (
            maSecondCorner,
            false,
            false));
    if (!(nIndexUnderMouse>=0 && nIndexUnderMouse<nPageCount))
        return;

    if (mnAnchorIndex < 0)
        mnAnchorIndex = nIndexUnderMouse;
    mnSecondIndex = nIndexUnderMouse;

    Range aRange (mnAnchorIndex, mnSecondIndex);
    aRange.Justify();

    for (sal_Int32 nIndex=0; nIndex<nPageCount; ++nIndex)
    {
        UpdateSelectionState(rModel.GetPageDescriptor(nIndex), aRange.IsInside(nIndex));
    }
}

//===== DragAndDropModeHandler ================================================

DragAndDropModeHandler::DragAndDropModeHandler (
    SlideSorter& rSlideSorter,
    SelectionFunction& rSelectionFunction,
    const Point& rMousePosition,
    vcl::Window* pWindow)
    : ModeHandler(rSlideSorter, rSelectionFunction, false)
{
    SdTransferable* pDragTransferable = SD_MOD()->pTransferDrag;
    if (pDragTransferable==nullptr && mrSlideSorter.GetViewShell() != nullptr)
    {
        SlideSorterViewShell* pSlideSorterViewShell
            = dynamic_cast<SlideSorterViewShell*>(mrSlideSorter.GetViewShell());
        if (pSlideSorterViewShell != nullptr)
            pSlideSorterViewShell->StartDrag(rMousePosition, pWindow);
        pDragTransferable = SD_MOD()->pTransferDrag;
    }

    mpDragAndDropContext.reset(new DragAndDropContext(mrSlideSorter));
    mrSlideSorter.GetController().GetInsertionIndicatorHandler()->Start(
        pDragTransferable != nullptr
            && pDragTransferable->GetView()==&mrSlideSorter.GetView());
}

DragAndDropModeHandler::~DragAndDropModeHandler()
{
    if (mpDragAndDropContext)
    {
        // Disconnect the substitution handler from this selection function.
        mpDragAndDropContext->SetTargetSlideSorter();
        mpDragAndDropContext.reset();
    }
    mrSlideSorter.GetController().GetInsertionIndicatorHandler()->End(Animator::AM_Animated);
}

SelectionFunction::Mode DragAndDropModeHandler::GetMode() const
{
    return SelectionFunction::DragAndDropMode;
}

void DragAndDropModeHandler::Abort()
{
    mrSlideSorter.GetController().GetClipboard().Abort();
    if (mpDragAndDropContext)
        mpDragAndDropContext->Dispose();
    //    mrSlideSorter.GetView().RequestRepaint(mrSlideSorter.GetModel().RestoreSelection());
}

bool DragAndDropModeHandler::ProcessButtonUpEvent (
    SelectionFunction::EventDescriptor& rDescriptor)
{
    if (Match(rDescriptor.mnEventCode, BUTTON_UP | LEFT_BUTTON))
    {
        // The following Process() call may lead to the destruction
        // of rDescriptor.mpHitDescriptor so release our reference to it.
        rDescriptor.mpHitDescriptor.reset();
        mrSelectionFunction.SwitchToNormalMode();
        return true;
    }
    else
        return false;
}

bool DragAndDropModeHandler::ProcessDragEvent (SelectionFunction::EventDescriptor& rDescriptor)
{
    OSL_ASSERT(mpDragAndDropContext);

    if (rDescriptor.mbIsLeaving)
    {
        mrSelectionFunction.SwitchToNormalMode();
    }
    else if (mpDragAndDropContext)
    {
        mpDragAndDropContext->UpdatePosition(
            rDescriptor.maMousePosition,
            rDescriptor.meDragMode, true);
    }

    return true;
}

} } } // end of namespace ::sd::slidesorter::controller

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
