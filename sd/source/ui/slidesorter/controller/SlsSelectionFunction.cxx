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


#include "controller/SlsSelectionFunction.hxx"

#include "SlideSorter.hxx"
#include "SlideSorterViewShell.hxx"
#include "SlsDragAndDropContext.hxx"
#include "controller/SlsTransferable.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsPageSelector.hxx"
#include "controller/SlsFocusManager.hxx"
#include "controller/SlsScrollBarManager.hxx"
#include "controller/SlsClipboard.hxx"
#include "controller/SlsCurrentSlideManager.hxx"
#include "controller/SlsInsertionIndicatorHandler.hxx"
#include "controller/SlsSelectionManager.hxx"
#include "controller/SlsProperties.hxx"
#include "controller/SlsProperties.hxx"
#include "controller/SlsSlotManager.hxx"
#include "controller/SlsVisibleAreaManager.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "model/SlsPageEnumerationProvider.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsLayouter.hxx"
#include "view/SlsPageObjectLayouter.hxx"
#include "view/SlsButtonBar.hxx"
#include "framework/FrameworkHelper.hxx"
#include "ViewShellBase.hxx"
#include "DrawController.hxx"
#include "Window.hxx"
#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "sdxfer.hxx"
#include "ViewShell.hxx"
#include "ViewShellBase.hxx"
#include "FrameView.hxx"
#include "app.hrc"
#include "sdresid.hxx"
#include "strings.hrc"
#include <vcl/sound.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svdpagv.hxx>
#include <vcl/msgbox.hxx>
#include <svx/svxids.hrc>
#include <boost/bind.hpp>
#include <boost/optional.hpp>

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
static const sal_uInt32 OVER_FADE_INDICATOR      (0x00040000);
static const sal_uInt32 OVER_BUTTON_AREA         (0x00080000);
static const sal_uInt32 OVER_BUTTON              (0x00100000);
static const sal_uInt32 SHIFT_MODIFIER           (0x00200000);
static const sal_uInt32 CONTROL_MODIFIER         (0x00400000);

static const sal_uInt32 KEY_EVENT                (0x10000000);

// Some absent events are defined so they can be expressed explicitly.
static const sal_uInt32 NO_MODIFIER              (0x00000000);
static const sal_uInt32 NOT_OVER_PAGE            (0x00000000);

// Masks
static const sal_uInt32 MODIFIER_MASK            (SHIFT_MODIFIER | CONTROL_MODIFIER);
static const sal_uInt32 BUTTON_MASK              (LEFT_BUTTON | RIGHT_BUTTON | MIDDLE_BUTTON);

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
    Point maMousePosition;
    Point maMouseModelPosition;
    model::SharedPageDescriptor mpHitDescriptor;
    SdrPage* mpHitPage;
    sal_uInt32 mnEventCode;
    bool mbIsOverButton;
    InsertionIndicatorHandler::Mode meDragMode;
    bool mbMakeSelectionVisible;
    bool mbIsLeaving;

    EventDescriptor (
        sal_uInt32 nEventType,
        const MouseEvent& rEvent,
        SlideSorter& rSlideSorter);
    EventDescriptor (
        sal_uInt32 nEventType,
        const AcceptDropEvent& rEvent,
        const sal_Int8 nDragAction,
        SlideSorter& rSlideSorter);
    EventDescriptor (
        const KeyEvent& rEvent,
        SlideSorter& rSlideSorter);

private:
    /** Compute a numerical code that describes a mouse event and that can
        be used for fast look up of the appropriate reaction.
    */
    sal_uInt32 EncodeMouseEvent (const MouseEvent& rEvent) const;

    /** Compute a numerical code that describes a key event and that can
        be used for fast look up of the appropriate reaction.
    */
    sal_uInt32 EncodeKeyEvent (const KeyEvent& rEvent) const;

    /** Compute a numerical code that describes the current state like
        whether the selection rectangle is visible or whether the page under
        the mouse or the one that has the focus is selected.
    */
    sal_uInt32 EncodeState (void) const;
};




//===== SelectionFunction::ModeHandler ========================================

class SelectionFunction::ModeHandler
{
public:
    ModeHandler (
        SlideSorter& rSlideSorter,
        SelectionFunction& rSelectionFunction,
        const bool bIsMouseOverIndicatorAllowed);
    virtual ~ModeHandler (void);

    virtual Mode GetMode (void) const = 0;
    virtual void Abort (void) = 0;
    virtual void ProcessEvent (EventDescriptor& rDescriptor);

    /** Set the selection to exactly the specified page and also set it as
        the current page.
    */
    void SetCurrentPage (const model::SharedPageDescriptor& rpDescriptor);

    /// Deselect all pages.
    void DeselectAllPages (void);
    void SelectOnePage (const model::SharedPageDescriptor& rpDescriptor);

        /** When the view on which this selection function is working is the
        main view then the view is switched to the regular editing view.
    */
    void SwitchView (const model::SharedPageDescriptor& rpDescriptor);

    void StartDrag (
        const Point& rMousePosition,
        const InsertionIndicatorHandler::Mode eMode);

    bool IsMouseOverIndicatorAllowed (void) const;

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
    virtual ~NormalModeHandler (void);

    virtual SelectionFunction::Mode GetMode (void) const;
    virtual void Abort (void);

    void ResetButtonDownLocation (void);

protected:
    virtual bool ProcessButtonDownEvent (SelectionFunction::EventDescriptor& rDescriptor);
    virtual bool ProcessButtonUpEvent (SelectionFunction::EventDescriptor& rDescriptor);
    virtual bool ProcessMotionEvent (SelectionFunction::EventDescriptor& rDescriptor);
    virtual bool ProcessDragEvent (SelectionFunction::EventDescriptor& rDescriptor);

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
#ifndef MACOSX
        const Point& rMouseModelPosition);
#else
        const Point& rMouseModelPosition,
        const sal_uInt32 nEventCode);
#endif
    virtual ~MultiSelectionModeHandler (void);

#ifndef MACOSX
    void Initialize(const sal_uInt32 nEventCode);
#endif

    virtual SelectionFunction::Mode GetMode (void) const;
    virtual void Abort (void);
    virtual void ProcessEvent (SelectionFunction::EventDescriptor& rDescriptor);

    enum SelectionMode { SM_Normal, SM_Add, SM_Toggle };

    void SetSelectionMode (const SelectionMode eSelectionMode);
    void SetSelectionModeFromModifier (const sal_uInt32 nEventCode);

protected:
    virtual bool ProcessButtonUpEvent (SelectionFunction::EventDescriptor& rDescriptor);
    virtual bool ProcessMotionEvent (SelectionFunction::EventDescriptor& rDescriptor);
    virtual bool HandleUnprocessedEvent (SelectionFunction::EventDescriptor& rDescriptor);

private:
    SelectionMode meSelectionMode;
    Point maSecondCorner;
    Pointer maSavedPointer;
    sal_Int32 mnAnchorIndex;
    sal_Int32 mnSecondIndex;
    view::ButtonBar::Lock maButtonBarLock;

    virtual void UpdateModelPosition (const Point& rMouseModelPosition);
    virtual void UpdateSelection (void);

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
#ifndef MACOSX
        SelectionFunction& rSelectionFunction);
#else
        SelectionFunction& rSelectionFunction,
        const Point& rMousePosition,
        ::Window* pWindow);
#endif
    virtual ~DragAndDropModeHandler (void);

#ifndef MACOSX
    void Initialize(const Point& rMousePosition, ::Window* pWindow);
#endif

    virtual SelectionFunction::Mode GetMode (void) const;
    virtual void Abort (void);

protected:
    virtual bool ProcessButtonUpEvent (SelectionFunction::EventDescriptor& rDescriptor);
    virtual bool ProcessDragEvent (SelectionFunction::EventDescriptor& rDescriptor);

private:
    ::boost::scoped_ptr<DragAndDropContext> mpDragAndDropContext;
};


/** Handle events while the left mouse button is pressed over the button
    bar.
*/
class ButtonModeHandler : public SelectionFunction::ModeHandler
{
public:
    ButtonModeHandler (
        SlideSorter& rSlideSorter,
        SelectionFunction& rSelectionFunction);
    virtual ~ButtonModeHandler (void);
    virtual void Abort (void);

    virtual SelectionFunction::Mode GetMode (void) const;

protected:
    virtual bool ProcessButtonDownEvent (SelectionFunction::EventDescriptor& rDescriptor);
    virtual bool ProcessButtonUpEvent (SelectionFunction::EventDescriptor& rDescriptor);
    virtual bool ProcessMotionEvent (SelectionFunction::EventDescriptor& rDescriptor);
};




//===== SelectionFunction =====================================================

TYPEINIT1(SelectionFunction, FuPoor);


SelectionFunction::SelectionFunction (
    SlideSorter& rSlideSorter,
    SfxRequest& rRequest)
    : FuPoor (
        rSlideSorter.GetViewShell(),
        rSlideSorter.GetContentWindow().get(),
        &rSlideSorter.GetView(),
        rSlideSorter.GetModel().GetDocument(),
        rRequest),
      mrSlideSorter(rSlideSorter),
      mrController(mrSlideSorter.GetController()),
      mbDragSelection(false),
      maInsertionMarkerBox(),
      mbProcessingMouseButtonDown(false),
      mnShiftKeySelectionAnchor(-1),
      mpModeHandler(new NormalModeHandler(rSlideSorter, *this))
{
}




SelectionFunction::~SelectionFunction (void)
{
    mpModeHandler.reset();
}




FunctionReference SelectionFunction::Create(
    SlideSorter& rSlideSorter,
    SfxRequest& rRequest)
{
    FunctionReference xFunc( new SelectionFunction( rSlideSorter, rRequest ) );
    return xFunc;
}




sal_Bool SelectionFunction::MouseButtonDown (const MouseEvent& rEvent)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode (rEvent.GetButtons());
    aMDPos = rEvent.GetPosPixel();
    mbProcessingMouseButtonDown = true;

    //  mpWindow->CaptureMouse();

    ProcessMouseEvent(BUTTON_DOWN, rEvent);

    return sal_True;
}




sal_Bool SelectionFunction::MouseMove (const MouseEvent& rEvent)
{
    ProcessMouseEvent(MOUSE_MOTION, rEvent);
    return sal_True;
}




sal_Bool SelectionFunction::MouseButtonUp (const MouseEvent& rEvent)
{
    mrController.GetScrollBarManager().StopAutoScroll ();

    ProcessMouseEvent(BUTTON_UP, rEvent);

    mbProcessingMouseButtonDown = false;
//    mpWindow->ReleaseMouse();

    return sal_True;
}




void SelectionFunction::NotifyDragFinished (void)
{
    SwitchToNormalMode();
}




sal_Bool SelectionFunction::KeyInput (const KeyEvent& rEvent)
{
    view::SlideSorterView::DrawLock aDrawLock (mrSlideSorter);
    PageSelector::UpdateLock aLock (mrSlideSorter);
    FocusManager& rFocusManager (mrController.GetFocusManager());
    sal_Bool bResult = sal_False;

    const KeyCode& rCode (rEvent.GetKeyCode());
    switch (rCode.GetCode())
    {
        case KEY_RETURN:
        {
            model::SharedPageDescriptor pDescriptor (rFocusManager.GetFocusedPageDescriptor());
            ViewShell* pViewShell = mrSlideSorter.GetViewShell();
            if (rFocusManager.HasFocus() && pDescriptor && pViewShell!=NULL)
            {
                // The Return key triggers different functions depending on
                // whether the slide sorter is the main view or displayed in
                // the right pane.
                if (pViewShell->IsMainViewShell())
                {
                    mpModeHandler->SetCurrentPage(pDescriptor);
                    mpModeHandler->SwitchView(pDescriptor);
                }
                else
                {
                    pViewShell->GetDispatcher()->Execute(
                        SID_INSERTPAGE,
                        SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
                }
                bResult = sal_True;
            }
            break;
        }

        case KEY_TAB:
            if ( ! rFocusManager.IsFocusShowing())
            {
                rFocusManager.ShowFocus();
                bResult = sal_True;
            }
            break;

        case KEY_ESCAPE:
            // When there is an active multiselection or drag-and-drop
            // operation then stop that.
            mpModeHandler->Abort();
            SwitchToNormalMode();
            bResult = sal_True;
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
            bResult = sal_True;
        }
        break;


        // Move the focus indicator left.
        case KEY_LEFT:
            MoveFocus(FocusManager::FMD_LEFT, rCode.IsShift(), rCode.IsMod1());
            bResult = sal_True;
            break;

        // Move the focus indicator right.
        case KEY_RIGHT:
            MoveFocus(FocusManager::FMD_RIGHT, rCode.IsShift(), rCode.IsMod1());
            bResult = sal_True;
            break;

        // Move the focus indicator up.
        case KEY_UP:
            MoveFocus(FocusManager::FMD_UP, rCode.IsShift(), rCode.IsMod1());
            bResult = sal_True;
            break;

        // Move the focus indicator down.
        case KEY_DOWN:
            MoveFocus(FocusManager::FMD_DOWN, rCode.IsShift(), rCode.IsMod1());
            bResult = sal_True;
            break;

        // Go to previous page.  No wrap around.
        case KEY_PAGEUP:
            GotoNextPage(-1);
            bResult = sal_True;
            break;

        // Go to next page.  No wrap around..
        case KEY_PAGEDOWN:
            GotoNextPage(+1);
            bResult = sal_True;
            break;

        case KEY_HOME:
            GotoPage(0);
            bResult = sal_True;
            break;

        case KEY_END:
            GotoPage(mrSlideSorter.GetModel().GetPageCount()-1);
            bResult = sal_True;
            break;

        case KEY_DELETE:
        case KEY_BACKSPACE:
        {
            if (mrSlideSorter.GetProperties()->IsUIReadOnly())
                break;

            mrController.GetSelectionManager()->DeleteSelectedPages(rCode.GetCode()==KEY_DELETE);

            mnShiftKeySelectionAnchor = -1;
            bResult = sal_True;
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




void SelectionFunction::Activate()
{
    FuPoor::Activate();
}




void SelectionFunction::Deactivate()
{
    FuPoor::Deactivate();
}



void SelectionFunction::ScrollStart (void)
{
}




void SelectionFunction::ScrollEnd (void)
{
}




void SelectionFunction::DoCut (void)
{
    if ( ! mrSlideSorter.GetProperties()->IsUIReadOnly())
    {
        mrController.GetClipboard().DoCut();
    }
}




void SelectionFunction::DoCopy (void)
{
    mrController.GetClipboard().DoCopy();
}




void SelectionFunction::DoPaste (void)
{
    if ( ! mrSlideSorter.GetProperties()->IsUIReadOnly())
    {
        mrController.GetClipboard().DoPaste();
    }
}




bool SelectionFunction::cancel (void)
{
    mrController.GetFocusManager().ToggleFocus();
    return true;
}




void SelectionFunction::GotoNextPage (int nOffset)
{
    model::SharedPageDescriptor pDescriptor
        = mrController.GetCurrentSlideManager()->GetCurrentSlide();
    if (pDescriptor.get() != NULL)
    {
        SdPage* pPage = pDescriptor->GetPage();
        OSL_ASSERT(pPage!=NULL);
        sal_Int32 nIndex = (pPage->GetPageNum()-1) / 2;
        GotoPage(nIndex + nOffset);
    }
    ResetShiftKeySelectionAnchor();
}




void SelectionFunction::GotoPage (int nIndex)
{
    sal_uInt16 nPageCount = (sal_uInt16)mrSlideSorter.GetModel().GetPageCount();

    if (nIndex >= nPageCount)
        nIndex = nPageCount - 1;
    if (nIndex < 0)
        nIndex = 0;

    mrController.GetFocusManager().SetFocusedPage(nIndex);
    model::SharedPageDescriptor pNextPageDescriptor (
        mrSlideSorter.GetModel().GetPageDescriptor (nIndex));
    if (pNextPageDescriptor.get() != NULL)
        mpModeHandler->SetCurrentPage(pNextPageDescriptor);
    else
    {
        OSL_ASSERT(pNextPageDescriptor.get() != NULL);
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
    // Prevent the untimely destruction of the called handler  by aquiring a
    // temporary reference here.
    ::boost::shared_ptr<ModeHandler> pModeHandler (mpModeHandler);
    pModeHandler->ProcessEvent(rDescriptor);
}




bool Match (
    const sal_uInt32 nEventCode,
    const sal_uInt32 nPositivePattern)
{
    return (nEventCode & nPositivePattern)==nPositivePattern;
}




void SelectionFunction::SwitchToNormalMode (void)
{
    if (mpModeHandler->GetMode() != NormalMode)
        SwitchMode(::boost::shared_ptr<ModeHandler>(
            new NormalModeHandler(mrSlideSorter, *this)));
}




void SelectionFunction::SwitchToDragAndDropMode (const Point aMousePosition)
{
    if (mpModeHandler->GetMode() != DragAndDropMode)
    {
#ifndef MACOSX
        ::boost::shared_ptr<DragAndDropModeHandler> handler(
            new DragAndDropModeHandler(mrSlideSorter, *this));
        SwitchMode(handler);
        // Delayed initialization, only after mpModeHanler is set, otherwise DND initialization
        // could already trigger DND events, which would recursively trigger this code again,
        // and without mpModeHandler set it would again try to set a new handler.
        handler->Initialize(aMousePosition, mpWindow);
#else
        SwitchMode(::boost::shared_ptr<ModeHandler>(
            new DragAndDropModeHandler(mrSlideSorter, *this, aMousePosition, mpWindow)));
#endif
    }
}




void SelectionFunction::SwitchToMultiSelectionMode (
    const Point aMousePosition,
    const sal_uInt32 nEventCode)
{
    if (mpModeHandler->GetMode() != MultiSelectionMode)
#ifndef MACOSX
    {
        ::boost::shared_ptr<MultiSelectionModeHandler> handler(
            new MultiSelectionModeHandler(mrSlideSorter, *this, aMousePosition));
        SwitchMode(handler);
        // Delayed initialization, only after mpModeHanler is set, the handle ctor
        // is non-trivial, so it could possibly recurse just like the DND handler above.
        handler->Initialize(nEventCode);
    }
#else
        SwitchMode(::boost::shared_ptr<ModeHandler>(
            new MultiSelectionModeHandler(mrSlideSorter, *this, aMousePosition, nEventCode)));
#endif
}




bool SelectionFunction::SwitchToButtonMode (void)
{
    // Do not show the buttons for draw pages.
    ::boost::shared_ptr<ViewShell> pMainViewShell (mrSlideSorter.GetViewShellBase()->GetMainViewShell());
    if (pMainViewShell
        && pMainViewShell->GetShellType()!=ViewShell::ST_DRAW
        && mpModeHandler->GetMode() != ButtonMode)
    {
        SwitchMode(::boost::shared_ptr<ModeHandler>(new ButtonModeHandler(mrSlideSorter, *this)));
        return true;
    }
    else
        return false;
}




void SelectionFunction::SwitchMode (const ::boost::shared_ptr<ModeHandler>& rpHandler)
{
    // Not all modes allow mouse over indicator.
    if (mpModeHandler->IsMouseOverIndicatorAllowed() != rpHandler->IsMouseOverIndicatorAllowed())
    {
        if ( ! rpHandler->IsMouseOverIndicatorAllowed())
        {
            mrSlideSorter.GetView().SetPageUnderMouse(model::SharedPageDescriptor());
            mrSlideSorter.GetView().GetButtonBar().ResetPage();
        }
        else
            mrSlideSorter.GetView().UpdatePageUnderMouse(false);
    }

    mpModeHandler = rpHandler;
}




void SelectionFunction::ResetShiftKeySelectionAnchor (void)
{
    mnShiftKeySelectionAnchor = -1;
}




void SelectionFunction::ResetMouseAnchor (void)
{
    if (mpModeHandler && mpModeHandler->GetMode() == NormalMode)
    {
        ::boost::shared_ptr<NormalModeHandler> pHandler (
            ::boost::dynamic_pointer_cast<NormalModeHandler>(mpModeHandler));
        if (pHandler)
            pHandler->ResetButtonDownLocation();
    }
}




//===== EventDescriptor =======================================================

SelectionFunction::EventDescriptor::EventDescriptor (
    const sal_uInt32 nEventType,
    const MouseEvent& rEvent,
    SlideSorter& rSlideSorter)
    : maMousePosition(rEvent.GetPosPixel()),
      maMouseModelPosition(),
      mpHitDescriptor(),
      mpHitPage(),
      mnEventCode(nEventType),
      mbIsOverButton(rSlideSorter.GetView().GetButtonBar().IsMouseOverButton()),
      meDragMode(InsertionIndicatorHandler::MoveMode),
      mbMakeSelectionVisible(true),
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
        || ! Rectangle(Point(0,0),
             rSlideSorter.GetContentWindow()->GetOutputSizePixel()).IsInside(maMousePosition);
}




SelectionFunction::EventDescriptor::EventDescriptor (
    const sal_uInt32 nEventType,
    const AcceptDropEvent& rEvent,
    const sal_Int8 nDragAction,
    SlideSorter& rSlideSorter)
    : maMousePosition(rEvent.maPosPixel),
      maMouseModelPosition(),
      mpHitDescriptor(),
      mpHitPage(),
      mnEventCode(nEventType),
      mbIsOverButton(rSlideSorter.GetView().GetButtonBar().IsMouseOverButton()),
      meDragMode(InsertionIndicatorHandler::GetModeFromDndAction(nDragAction)),
      mbMakeSelectionVisible(true),
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
        || ! Rectangle(Point(0,0),
             rSlideSorter.GetContentWindow()->GetOutputSizePixel()).IsInside(maMousePosition);
}




SelectionFunction::EventDescriptor::EventDescriptor (
    const KeyEvent& rEvent,
    SlideSorter& rSlideSorter)
    : maMousePosition(),
      maMouseModelPosition(),
      mpHitDescriptor(),
      mpHitPage(),
      mnEventCode(KEY_EVENT),
      mbIsOverButton(rSlideSorter.GetView().GetButtonBar().IsMouseOverButton()),
      meDragMode(InsertionIndicatorHandler::MoveMode),
      mbMakeSelectionVisible(true),
      mbIsLeaving(false)
{
    model::SharedPageDescriptor pHitDescriptor (
        rSlideSorter.GetController().GetFocusManager().GetFocusedPageDescriptor());
    if (pHitDescriptor.get() != NULL)
    {
        mpHitPage = pHitDescriptor->GetPage();
        mpHitDescriptor = pHitDescriptor;
    }

    mnEventCode |= EncodeKeyEvent(rEvent) | EncodeState();
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

    // Detect whether the mouse is over one of the active elements inside a
    // page object.
    if (mbIsOverButton)
        nEventCode |= OVER_BUTTON;

    return nEventCode;
}




sal_uInt32 SelectionFunction::EventDescriptor::EncodeKeyEvent (const KeyEvent& rEvent) const
{
    // The key code in the lower 16 bit.
    sal_uInt32 nEventCode (rEvent.GetKeyCode().GetCode());

    // Detect pressed modifier keys.
    if (rEvent.GetKeyCode().IsShift())
        nEventCode |= SHIFT_MODIFIER;
    if (rEvent.GetKeyCode().IsMod1())
        nEventCode |= CONTROL_MODIFIER;

    return nEventCode;
}




sal_uInt32 SelectionFunction::EventDescriptor::EncodeState (void) const
{
    sal_uInt32 nEventCode (0);

    // Detect whether the event has happened over a page object.
    if (mpHitPage!=NULL && mpHitDescriptor)
    {
        if (mpHitDescriptor->HasState(model::PageDescriptor::ST_Selected))
            nEventCode |= OVER_SELECTED_PAGE;
        else
            nEventCode |= OVER_UNSELECTED_PAGE;

        // Detect whether the mouse is over one of the active elements
        // inside a page object.
        if (mbIsOverButton)
            nEventCode |= OVER_BUTTON;
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




SelectionFunction::ModeHandler::~ModeHandler (void)
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
        mrSlideSorter.GetView().UpdatePageUnderMouse(
            rDescriptor.maMousePosition,
            (rDescriptor.mnEventCode & LEFT_BUTTON) != 0,
            true);

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




void SelectionFunction::ModeHandler::DeselectAllPages (void)
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
    if (pViewShell!=NULL && pViewShell->IsMainViewShell())
    {
        if (rpDescriptor.get()!=NULL && rpDescriptor->GetPage()!=NULL)
        {
            mrSlideSorter.GetModel().GetDocument()->SetSelected(rpDescriptor->GetPage(), sal_True);
            pViewShell->GetFrameView()->SetSelectedPage(
                (rpDescriptor->GetPage()->GetPageNum()-1)/2);
        }
        if (mrSlideSorter.GetViewShellBase() != NULL)
        framework::FrameworkHelper::Instance(*mrSlideSorter.GetViewShellBase())->RequestView(
            framework::FrameworkHelper::msImpressViewURL,
            framework::FrameworkHelper::msCenterPaneURL);
    }
}




void SelectionFunction::ModeHandler::StartDrag (
    const Point& rMousePosition,
    const InsertionIndicatorHandler::Mode eMode)
{
    (void)eMode;
    // Do not start a drag-and-drop operation when one is already active.
    // (when dragging pages from one document into another, pressing a
    // modifier key can trigger a MouseMotion event in the originating
    // window (focus still in there).  Together with the mouse button pressed
    // (drag-and-drop is active) this triggers the start of drag-and-drop.)
    if (SD_MOD()->pTransferDrag != NULL)
        return;

    if ( ! mrSlideSorter.GetProperties()->IsUIReadOnly())
    {
        mrSelectionFunction.SwitchToDragAndDropMode(rMousePosition);
    }
}




bool SelectionFunction::ModeHandler::IsMouseOverIndicatorAllowed (void) const
{
    return mbIsMouseOverIndicatorAllowed;
}




//===== NormalModeHandler =====================================================

NormalModeHandler::NormalModeHandler (
    SlideSorter& rSlideSorter,
    SelectionFunction& rSelectionFunction)
    : ModeHandler(rSlideSorter, rSelectionFunction, true),
      maButtonDownLocation()
{
}




NormalModeHandler::~NormalModeHandler (void)
{
}




SelectionFunction::Mode NormalModeHandler::GetMode (void) const
{
    return SelectionFunction::NormalMode;
}




void NormalModeHandler::Abort (void)
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
            // A double click allways shows the selected slide in the center
            // pane in an edit view.
            SetCurrentPage(rDescriptor.mpHitDescriptor);
            SwitchView(rDescriptor.mpHitDescriptor);
            break;

        case BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | OVER_SELECTED_PAGE | SHIFT_MODIFIER:
        case BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | OVER_UNSELECTED_PAGE | SHIFT_MODIFIER:
            // Range selection with the shift modifier.
            RangeSelect(rDescriptor.mpHitDescriptor);
            break;

        case BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | OVER_UNSELECTED_PAGE | OVER_BUTTON:
        case BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | OVER_SELECTED_PAGE | OVER_BUTTON:
            OSL_ASSERT(mrSlideSorter.GetView().GetButtonBar().IsMouseOverButton());

            // Switch to button mode only when the buttons are visible
            // (or being faded in.)
            if (mrSlideSorter.GetView().GetButtonBar().IsVisible(rDescriptor.mpHitDescriptor))
            {
                if (mrSelectionFunction.SwitchToButtonMode())
                    ReprocessEvent(rDescriptor);
            }
            else
            {
                // When the buttons are not (yet) visible then behave like
                // the left button had been clicked over any other part of
                // the slide.
                SetCurrentPage(rDescriptor.mpHitDescriptor);
            }
            break;

            // Right button for context menu.
        case BUTTON_DOWN | RIGHT_BUTTON | SINGLE_CLICK | OVER_UNSELECTED_PAGE:
            // Single right click and shift+F10 select as preparation to
            // show the context menu.  Change the selection only when the
            // page under the mouse is not selected.  In this case the
            // selection is set to this single page.  Otherwise the
            // selection is not modified.
            SetCurrentPage(rDescriptor.mpHitDescriptor);
            rDescriptor.mbMakeSelectionVisible = false;
            break;

        case BUTTON_DOWN | RIGHT_BUTTON | SINGLE_CLICK | OVER_SELECTED_PAGE:
            // Do not change the selection.  Just adjust the insertion indicator.
            rDescriptor.mbMakeSelectionVisible = false;
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
            mrSlideSorter.GetView().UpdatePageUnderMouse(
                rDescriptor.mpHitDescriptor,
                rDescriptor.maMousePosition,
                false);
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
        case ANY_MODIFIER(MOUSE_MOTION | LEFT_BUTTON | SINGLE_CLICK | OVER_UNSELECTED_PAGE):
            //            SetCurrentPage(rDescriptor.mpHitDescriptor);
            // Fallthrough

        // A mouse motion without visible substitution starts that.
        case ANY_MODIFIER(MOUSE_MOTION | LEFT_BUTTON | SINGLE_CLICK | OVER_SELECTED_PAGE):
        {
            if (maButtonDownLocation)
            {
                const sal_Int32 nDistance (maButtonDownLocation
                    ? ::std::max (
                        abs(maButtonDownLocation->X() - rDescriptor.maMousePosition.X()),
                        abs(maButtonDownLocation->Y() - rDescriptor.maMousePosition.Y()))
                    : 0);
                if (nDistance > 3)
                    StartDrag(
                        rDescriptor.maMousePosition,
                        (rDescriptor.mnEventCode & CONTROL_MODIFIER) != 0
                            ? InsertionIndicatorHandler::CopyMode
                            : InsertionIndicatorHandler::MoveMode);
            }
        }
        break;

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

    if (pAnchor.get() != NULL)
    {
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
}




void NormalModeHandler::ResetButtonDownLocation (void)
{
    maButtonDownLocation = ::boost::optional<Point>();
}




//===== MultiSelectionModeHandler =============================================

MultiSelectionModeHandler::MultiSelectionModeHandler (
    SlideSorter& rSlideSorter,
    SelectionFunction& rSelectionFunction,
#ifndef MACOSX
    const Point& rMouseModelPosition)
#else
    const Point& rMouseModelPosition,
    const sal_uInt32 nEventCode)
#endif
    : ModeHandler(rSlideSorter, rSelectionFunction, false),
      meSelectionMode(SM_Normal),
      maSecondCorner(rMouseModelPosition),
      maSavedPointer(mrSlideSorter.GetContentWindow()->GetPointer()),
      mnAnchorIndex(-1),
      mnSecondIndex(-1),
      maButtonBarLock(rSlideSorter)
{
#ifndef MACOSX
}


void MultiSelectionModeHandler::Initialize(const sal_uInt32 nEventCode)
{
#endif
    const Pointer aSelectionPointer (POINTER_TEXT);
    mrSlideSorter.GetContentWindow()->SetPointer(aSelectionPointer);
    SetSelectionModeFromModifier(nEventCode);
}





MultiSelectionModeHandler::~MultiSelectionModeHandler (void)
{
    mrSlideSorter.GetContentWindow()->SetPointer(maSavedPointer);
}




SelectionFunction::Mode MultiSelectionModeHandler::GetMode (void) const
{
    return SelectionFunction::MultiSelectionMode;
}




void MultiSelectionModeHandler::Abort (void)
{
    mrSlideSorter.GetView().RequestRepaint(mrSlideSorter.GetModel().RestoreSelection());
}




void MultiSelectionModeHandler::ProcessEvent (
    SelectionFunction::EventDescriptor& rDescriptor)
{
    // During a multi selection we do not want sudden jumps of the
    // visible area caused by moving newly selected pages into view.
    // Therefore disable that temporarily.  The disabler object is
    // released at the end of the event processing, after the focus and
    // current slide have been updated.
    VisibleAreaManager::TemporaryDisabler aDisabler (mrSlideSorter);

    ModeHandler::ProcessEvent(rDescriptor);
}




bool MultiSelectionModeHandler::ProcessButtonUpEvent (
    SelectionFunction::EventDescriptor& rDescriptor)
{
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
        rDescriptor.mbMakeSelectionVisible = false;
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
    SharedSdWindow pWindow (mrSlideSorter.GetContentWindow());
    const Point aMouseModelPosition (pWindow->PixelToLogic(rMousePosition));

    if ( ! (bAllowAutoScroll && mrSlideSorter.GetController().GetScrollBarManager().AutoScroll(
        rMousePosition,
        ::boost::bind(
            &MultiSelectionModeHandler::UpdatePosition,
            this,
            rMousePosition,
            false))))
    {
        UpdateModelPosition(aMouseModelPosition);
    }
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




void MultiSelectionModeHandler::UpdateSelection (void)
{
    view::SlideSorterView::DrawLock aLock (mrSlideSorter);

    model::SlideSorterModel& rModel (mrSlideSorter.GetModel());
    const sal_Int32 nPageCount (rModel.GetPageCount());

    const sal_Int32 nIndexUnderMouse (
        mrSlideSorter.GetView().GetLayouter().GetIndexAtPoint (
            maSecondCorner,
            false,
            false));
    if (nIndexUnderMouse>=0 && nIndexUnderMouse<nPageCount)
    {
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
}




//===== DragAndDropModeHandler ================================================

DragAndDropModeHandler::DragAndDropModeHandler (
    SlideSorter& rSlideSorter,
#ifndef MACOSX
    SelectionFunction& rSelectionFunction)
#else
    SelectionFunction& rSelectionFunction,
    const Point& rMousePosition,
    ::Window* pWindow)
#endif
    : ModeHandler(rSlideSorter, rSelectionFunction, false)
{
#ifndef MACOSX
}


void DragAndDropModeHandler::Initialize(const Point& rMousePosition, ::Window* pWindow)
{
#endif
    SdTransferable* pDragTransferable = SD_MOD()->pTransferDrag;
    if (pDragTransferable==NULL && mrSlideSorter.GetViewShell() != NULL)
    {
        SlideSorterViewShell* pSlideSorterViewShell
            = dynamic_cast<SlideSorterViewShell*>(mrSlideSorter.GetViewShell());
        if (pSlideSorterViewShell != NULL)
            pSlideSorterViewShell->StartDrag(rMousePosition, pWindow);
        pDragTransferable = SD_MOD()->pTransferDrag;
    }

    mpDragAndDropContext.reset(new DragAndDropContext(mrSlideSorter));
    mrSlideSorter.GetController().GetInsertionIndicatorHandler()->Start(
        pDragTransferable != NULL
            && pDragTransferable->GetView()==&mrSlideSorter.GetView());
}




DragAndDropModeHandler::~DragAndDropModeHandler (void)
{
    if (mpDragAndDropContext)
    {
        // Disconnect the substitution handler from this selection function.
        mpDragAndDropContext->SetTargetSlideSorter();
        mpDragAndDropContext.reset();
    }
    mrSlideSorter.GetController().GetInsertionIndicatorHandler()->End(Animator::AM_Animated);
}




SelectionFunction::Mode DragAndDropModeHandler::GetMode (void) const
{
    return SelectionFunction::DragAndDropMode;
}




void DragAndDropModeHandler::Abort (void)
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
        // The following Process() call may lead to the desctruction
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
            rDescriptor.meDragMode);
    }

    return true;
}




//===== ButtonModeHandler =====================================================

ButtonModeHandler::ButtonModeHandler (
    SlideSorter& rSlideSorter,
    SelectionFunction& rSelectionFunction)
    : ModeHandler(rSlideSorter, rSelectionFunction, true)
{
}




ButtonModeHandler::~ButtonModeHandler (void)
{
}




SelectionFunction::Mode ButtonModeHandler::GetMode (void) const
{
    return SelectionFunction::ButtonMode;
}




void ButtonModeHandler::Abort (void)
{
}




bool ButtonModeHandler::ProcessButtonDownEvent (SelectionFunction::EventDescriptor& rDescriptor)
{
    switch (rDescriptor.mnEventCode)
    {
        case BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | OVER_UNSELECTED_PAGE | OVER_BUTTON:
        case BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | OVER_SELECTED_PAGE | OVER_BUTTON:
            // Remember page and button index.  When mouse button is
            // released over same page and button then invoke action of that
            // button.
            mrSlideSorter.GetView().GetButtonBar().ProcessButtonDownEvent(
                rDescriptor.mpHitDescriptor,
                rDescriptor.maMouseModelPosition);
            return true;

        default:
            return false;
    }
}




bool ButtonModeHandler::ProcessButtonUpEvent (SelectionFunction::EventDescriptor& rDescriptor)
{
    switch (rDescriptor.mnEventCode & BUTTON_MASK)
    {
        case LEFT_BUTTON:
            mrSlideSorter.GetView().GetButtonBar().ProcessButtonUpEvent(
                rDescriptor.mpHitDescriptor,
                rDescriptor.maMouseModelPosition);
            mrSelectionFunction.SwitchToNormalMode();
            return true;
    }

    return false;
}




bool ButtonModeHandler::ProcessMotionEvent (SelectionFunction::EventDescriptor& rDescriptor)
{
    switch (rDescriptor.mnEventCode & (MOUSE_MOTION | BUTTON_MASK))
    {
        case MOUSE_MOTION | LEFT_BUTTON:
            mrSlideSorter.GetView().GetButtonBar().ProcessMouseMotionEvent(
                rDescriptor.mpHitDescriptor,
                rDescriptor.maMouseModelPosition,
                true);
            return true;

        case MOUSE_MOTION:
            mrSlideSorter.GetView().GetButtonBar().ProcessMouseMotionEvent(
                rDescriptor.mpHitDescriptor,
                rDescriptor.maMouseModelPosition,
                false);
            return true;
    }

    return false;
}




} } } // end of namespace ::sd::slidesorter::controller

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
