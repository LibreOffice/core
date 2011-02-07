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

#include "precompiled_sd.hxx"

#include "controller/SlsSelectionFunction.hxx"

#include "SlideSorter.hxx"
#include "SlideSorterViewShell.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsPageSelector.hxx"
#include "controller/SlsFocusManager.hxx"
#include "controller/SlsScrollBarManager.hxx"
#include "controller/SlsClipboard.hxx"
#include "controller/SlsCurrentSlideManager.hxx"
#include "controller/SlsSelectionManager.hxx"
#include "controller/SlsProperties.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "model/SlsPageEnumerationProvider.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsViewOverlay.hxx"
#include "view/SlsLayouter.hxx"
#include "view/SlsPageObjectViewObjectContact.hxx"
#include "framework/FrameworkHelper.hxx"
#include "ViewShellBase.hxx"
#include "DrawController.hxx"
#include <vcl/sound.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svdpagv.hxx>
#include <vcl/msgbox.hxx>
#include "Window.hxx"
#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "ViewShell.hxx"
#include "ViewShellBase.hxx"
#include "FrameView.hxx"
#include "app.hrc"
#include "sdresid.hxx"
#include "strings.hrc"

namespace {
static const sal_uInt32 SINGLE_CLICK             (0x00000001);
static const sal_uInt32 DOUBLE_CLICK             (0x00000002);
static const sal_uInt32 LEFT_BUTTON              (0x00000010);
static const sal_uInt32 RIGHT_BUTTON             (0x00000020);
static const sal_uInt32 MIDDLE_BUTTON            (0x00000040);
static const sal_uInt32 BUTTON_DOWN              (0x00000100);
static const sal_uInt32 BUTTON_UP                (0x00000200);
static const sal_uInt32 MOUSE_MOTION             (0x00000400);
// The rest leaves the lower 16 bit untouched so that it can be used with
// key codes.
static const sal_uInt32 OVER_SELECTED_PAGE       (0x00010000);
static const sal_uInt32 OVER_UNSELECTED_PAGE     (0x00020000);
static const sal_uInt32 OVER_FADE_INDICATOR      (0x00040000);
static const sal_uInt32 SHIFT_MODIFIER           (0x00100000);
static const sal_uInt32 CONTROL_MODIFIER         (0x00200000);
static const sal_uInt32 SUBSTITUTION_VISIBLE     (0x01000000);
static const sal_uInt32 RECTANGLE_VISIBLE        (0x02000000);

static const sal_uInt32 KEY_EVENT                (0x10000000);

// Some absent events are defined so they can be expressed explicitly.
static const sal_uInt32 NO_MODIFIER              (0x00000000);
static const sal_uInt32 SUBSTITUTION_NOT_VISIBLE (0x00000000);
static const sal_uInt32 NOT_OVER_PAGE            (0x00000000);


} // end of anonymous namespace


namespace sd { namespace slidesorter { namespace controller {

class SelectionFunction::SubstitutionHandler
{
public:
    SubstitutionHandler (SlideSorter& rSlideSorter);
    ~SubstitutionHandler (void);

    /** Create a substitution display of the currently selected pages and
        use the given position as the anchor point.
    */
    void Start (const Point& rMouseModelPosition);

    /** Move the substitution display by the distance the mouse has
        travelled since the last call to this method or to
        CreateSubstitution().  The given point becomes the new anchor.
    */
    void UpdatePosition (const Point& rMouseModelPosition);

    /** Move the substitution display of the currently selected pages.
    */
    void Process (void);

    void End (void);

    bool HasBeenMoved (void) const;

private:
    SlideSorter& mrSlideSorter;

    bool mbHasBeenMoved;

    /** Determine whether there is a) a substitution and b) its insertion at
        the current position of the insertion marker would alter the
        document.   This would be the case when the substitution has been
        moved or is not consecutive.
    */
    bool IsSubstitutionInsertionNonTrivial (void) const;
};


class SelectionFunction::EventDescriptor
{
public:

    Point maMousePosition;
    Point maMouseModelPosition;
    ::boost::weak_ptr<model::PageDescriptor> mpHitDescriptor;
    SdrPage* mpHitPage;
    sal_uInt32 mnEventCode;

    EventDescriptor (
        sal_uInt32 nEventType,
        const MouseEvent& rEvent,
        SlideSorter& rSlideSorter);
    EventDescriptor (
        const KeyEvent& rEvent,
        SlideSorter& rSlideSorter);
};


TYPEINIT1(SelectionFunction, FuPoor);


SelectionFunction::SelectionFunction (
    SlideSorter& rSlideSorter,
    SfxRequest& rRequest)
    : SlideFunction (rSlideSorter, rRequest),
      mrSlideSorter(rSlideSorter),
      mrController(mrSlideSorter.GetController()),
      mbDragSelection(false),
      maInsertionMarkerBox(),
      mbProcessingMouseButtonDown(false),
      mpSubstitutionHandler(new SubstitutionHandler(mrSlideSorter))
{
    //af    aDelayToScrollTimer.SetTimeout(50);
    aDragTimer.SetTimeoutHdl( LINK( this, SelectionFunction, DragSlideHdl ) );
}

SelectionFunction::~SelectionFunction (void)
{
    aDragTimer.Stop();
}




FunctionReference SelectionFunction::Create(
    SlideSorter& rSlideSorter,
    SfxRequest& rRequest)
{
    FunctionReference xFunc( new SelectionFunction( rSlideSorter, rRequest ) );
    return xFunc;
}




BOOL SelectionFunction::MouseButtonDown (const MouseEvent& rEvent)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode (rEvent.GetButtons());
    mbProcessingMouseButtonDown = true;

    mpWindow->CaptureMouse();

    ProcessMouseEvent(BUTTON_DOWN, rEvent);

    return TRUE;
}




BOOL SelectionFunction::MouseMove (const MouseEvent& rEvent)
{
    Point aMousePosition (rEvent.GetPosPixel());

    // Determine page under mouse and show the mouse over effect.
    model::SharedPageDescriptor pHitDescriptor (mrController.GetPageAt(aMousePosition));
    view::ViewOverlay& rOverlay (mrSlideSorter.GetView().GetOverlay());
    rOverlay.GetMouseOverIndicatorOverlay().SetSlideUnderMouse(
        rEvent.IsLeaveWindow() ? model::SharedPageDescriptor() : pHitDescriptor);
    if (pHitDescriptor.get() != NULL)
        rOverlay.GetMouseOverIndicatorOverlay().setVisible(true);
    else
        rOverlay.GetMouseOverIndicatorOverlay().setVisible(false);

    // Allow one mouse move before the drag timer is disabled.
    if (aDragTimer.IsActive())
    {
        if (bFirstMouseMove)
            bFirstMouseMove = FALSE;
        else
            aDragTimer.Stop();
    }

    Rectangle aRectangle (Point(0,0),mpWindow->GetOutputSizePixel());
    if ( ! aRectangle.IsInside(aMousePosition)
        && rOverlay.GetSubstitutionOverlay().isVisible())
    {
        // Mouse left the window with pressed left button.  Make it a drag.
        StartDrag();
    }
    else
    {
        // Call ProcessMouseEvent() only when one of the buttons is
        // pressed. This prevents calling the method on every motion.
        if (rEvent.GetButtons() != 0
            && mbProcessingMouseButtonDown)
        {
            ProcessMouseEvent(MOUSE_MOTION, rEvent);
        }
    }

    return TRUE;
}




BOOL SelectionFunction::MouseButtonUp (const MouseEvent& rEvent)
{
    mrController.GetScrollBarManager().StopAutoScroll ();

    // remember button state for creation of own MouseEvents
    SetMouseButtonCode (rEvent.GetButtons());

    if (aDragTimer.IsActive())
        aDragTimer.Stop();

    ProcessMouseEvent(BUTTON_UP, rEvent);

    mbProcessingMouseButtonDown = false;
    mpWindow->ReleaseMouse();

    return TRUE;
}




BOOL SelectionFunction::KeyInput (const KeyEvent& rEvent)
{
    FocusManager& rFocusManager (mrController.GetFocusManager());
    BOOL bResult = FALSE;

    switch (rEvent.GetKeyCode().GetCode())
    {
        case KEY_RETURN:
            if (rFocusManager.HasFocus())
            {
                model::SharedPageDescriptor pDescriptor (rFocusManager.GetFocusedPageDescriptor());
                if (pDescriptor.get() != NULL)
                {
                    SetCurrentPage(pDescriptor);
                    SwitchView(pDescriptor);
                }
                bResult = TRUE;
            }
            break;

        case KEY_TAB:
            if ( ! rFocusManager.IsFocusShowing())
                rFocusManager.ShowFocus();
            else
                if (rEvent.GetKeyCode().IsShift())
                    rFocusManager.MoveFocus (FocusManager::FMD_LEFT);
                else
                    rFocusManager.MoveFocus (FocusManager::FMD_RIGHT);
            bResult = TRUE;
            break;

        case KEY_ESCAPE:
            rFocusManager.SetFocusToToolBox();
            bResult = TRUE;
            break;

        case KEY_SPACE:
        {
            // Toggle the selection state.
            model::SharedPageDescriptor pDescriptor (rFocusManager.GetFocusedPageDescriptor());
            if (pDescriptor.get() != NULL)
            {
                // Doing a multi selection by default.  Can we ask the event
                // for the state of the shift key?
                if (pDescriptor->IsSelected())
                    mrController.GetPageSelector().DeselectPage(pDescriptor);
                else
                    mrController.GetPageSelector().SelectPage(pDescriptor);
            }
            bResult = TRUE;
        }
        break;


        // Move the focus indicator left.
        case KEY_LEFT:
            rFocusManager.MoveFocus (FocusManager::FMD_LEFT);
            bResult = TRUE;
            break;

        // Move the focus indicator right.
        case KEY_RIGHT:
            rFocusManager.MoveFocus (FocusManager::FMD_RIGHT);
            bResult = TRUE;
            break;

        // Move the focus indicator up.
        case KEY_UP:
            rFocusManager.MoveFocus (FocusManager::FMD_UP);
            bResult = TRUE;
            break;

        // Move the focus indicator down.
        case KEY_DOWN:
            rFocusManager.MoveFocus (FocusManager::FMD_DOWN);
            bResult = TRUE;
            break;

        // Go to previous page.  No wrap around.
        case KEY_PAGEUP:
            GotoNextPage(-1);
            bResult = TRUE;
            break;

        // Go to next page.  No wrap around..
        case KEY_PAGEDOWN:
            GotoNextPage(+1);
            bResult = TRUE;
            break;

        case KEY_DELETE:
        case KEY_BACKSPACE:
        {
            if (mrController.GetProperties()->IsUIReadOnly())
                break;

            int nSelectedPagesCount = 0;

            // Count the selected pages and look if there any objects on any
            // of the selected pages so that we can warn the user and
            // prevent an accidental deletion.
            model::PageEnumeration aSelectedPages (
                model::PageEnumerationProvider::CreateSelectedPagesEnumeration(
                    mrSlideSorter.GetModel()));
            while (aSelectedPages.HasMoreElements())
            {
                nSelectedPagesCount++;
                aSelectedPages.GetNextElement();
            }

            if (nSelectedPagesCount > 0)
                 mrController.GetSelectionManager()->DeleteSelectedPages();

            bResult = TRUE;
        }
        break;

        case KEY_F10:
            if (rEvent.GetKeyCode().IsShift())
                ProcessKeyEvent(rEvent);
            break;

        default:
            break;
    }

    if ( ! bResult)
        bResult = SlideFunction::KeyInput (rEvent);

    return bResult;
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
    if ( ! mrController.GetProperties()->IsUIReadOnly())
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
    if ( ! mrController.GetProperties()->IsUIReadOnly())
    {
        mrController.GetClipboard().DoPaste();
    }
}




void SelectionFunction::Paint (const Rectangle&, ::sd::Window* )
{
}




IMPL_LINK( SelectionFunction, DragSlideHdl, Timer*, EMPTYARG )
{
    StartDrag();
    return 0;
}




void SelectionFunction::StartDrag (void)
{
    if (mbPageHit
        &&  ! mrController.GetProperties()->IsUIReadOnly())
    {
        view::ViewOverlay& rOverlay (mrSlideSorter.GetView().GetOverlay());
        mpSubstitutionHandler->Start(rOverlay.GetSubstitutionOverlay().GetPosition());
        mbPageHit = false;
        mpWindow->ReleaseMouse();

        if (mrSlideSorter.GetViewShell() != NULL)
        {
            SlideSorterViewShell* pSlideSorterViewShell
                = dynamic_cast<SlideSorterViewShell*>(mrSlideSorter.GetViewShell());
            pSlideSorterViewShell->StartDrag (
                rOverlay.GetSubstitutionOverlay().GetPosition(),
                mpWindow);
        }
    }
}




bool SelectionFunction::cancel (void)
{
    mrController.GetFocusManager().ToggleFocus();
    return true;
}




void SelectionFunction::SelectHitPage (const model::SharedPageDescriptor& rpDescriptor)
{
    mrController.GetPageSelector().SelectPage(rpDescriptor);
}




void SelectionFunction::DeselectHitPage (const model::SharedPageDescriptor& rpDescriptor)
{
    mrController.GetPageSelector().DeselectPage(rpDescriptor);
}




void SelectionFunction::DeselectAllPages (void)
{
    mrController.GetPageSelector().DeselectAllPages();
}




void SelectionFunction::StartRectangleSelection (const Point& rMouseModelPosition)
{
    if (mrController.GetProperties()->IsShowSelection())
    {
        mrSlideSorter.GetView().GetOverlay().GetSelectionRectangleOverlay().Start(
            rMouseModelPosition);
    }
}




void SelectionFunction::UpdateRectangleSelection (const Point& rMouseModelPosition)
{
    if (mrController.GetProperties()->IsShowSelection())
    {
        mrSlideSorter.GetView().GetOverlay().GetSelectionRectangleOverlay().Update(
            rMouseModelPosition);
    }
}




void SelectionFunction::ProcessRectangleSelection (bool bToggleSelection)
{
    if ( ! mrController.GetProperties()->IsShowSelection())
        return;

    view::ViewOverlay& rOverlay (mrSlideSorter.GetView().GetOverlay());
    if (rOverlay.GetSelectionRectangleOverlay().isVisible())
    {
        PageSelector& rSelector (mrController.GetPageSelector());

        rOverlay.GetSelectionRectangleOverlay().setVisible(false);

        // Select all pages whose page object lies completly inside the drag
        // rectangle.
        const Rectangle& rSelectionRectangle (
            rOverlay.GetSelectionRectangleOverlay().GetSelectionRectangle());
        model::PageEnumeration aPages (
            model::PageEnumerationProvider::CreateAllPagesEnumeration(
                mrSlideSorter.GetModel()));
        while (aPages.HasMoreElements())
        {
            model::SharedPageDescriptor pDescriptor (aPages.GetNextElement());
            Rectangle aPageBox (mrSlideSorter.GetView().GetPageBoundingBox(
                pDescriptor,
                view::SlideSorterView::CS_MODEL,
                view::SlideSorterView::BBT_SHAPE));
            if (rSelectionRectangle.IsOver(aPageBox))
            {
                // When we are extending the selection (shift key is
                // pressed) then toggle the selection state of the page.
                // Otherwise select it: this results in the previously
                // selected pages becoming deslected.
                if (bToggleSelection && pDescriptor->IsSelected())
                    rSelector.DeselectPage(pDescriptor);
                else
                    rSelector.SelectPage(pDescriptor);
            }
        }
    }
}




void SelectionFunction::PrepareMouseMotion (const Point& )
{
    if ( ! mrController.GetProperties()->IsUIReadOnly())
    {
        bFirstMouseMove = TRUE;
        aDragTimer.Start();
    }
}




void SelectionFunction::RangeSelect (const model::SharedPageDescriptor& rpDescriptor)
{
    PageSelector& rSelector (mrController.GetPageSelector());

    model::SharedPageDescriptor pAnchor (rSelector.GetSelectionAnchor());
    DeselectAllPages();

    if (pAnchor.get() != NULL)
    {
        // Select all pages between the anchor and the given one, including
        // the two.
        USHORT nAnchorIndex ((pAnchor->GetPage()->GetPageNum()-1) / 2);
        USHORT nOtherIndex ((rpDescriptor->GetPage()->GetPageNum()-1) / 2);

        // Iterate over all pages in the range.  Start with the anchor
        // page.  This way the PageSelector will recognize it again as
        // anchor (the first selected page after a DeselectAllPages()
        // becomes the anchor.)
        USHORT nStep = (nAnchorIndex < nOtherIndex) ? +1 : -1;
        USHORT nIndex = nAnchorIndex;
        while (true)
        {
            rSelector.SelectPage(nIndex);
            if (nIndex == nOtherIndex)
                break;
            nIndex = nIndex + nStep;
        }
    }
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
        nIndex += nOffset;
        USHORT nPageCount = (USHORT)mrSlideSorter.GetModel().GetPageCount();

        if (nIndex >= nPageCount)
            nIndex = nPageCount - 1;
        if (nIndex < 0)
            nIndex = 0;

        mrController.GetFocusManager().SetFocusedPage(nIndex);
        model::SharedPageDescriptor pNextPageDescriptor (
            mrSlideSorter.GetModel().GetPageDescriptor (nIndex));
        if (pNextPageDescriptor.get() != NULL)
            SetCurrentPage(pNextPageDescriptor);
        else
        {
            OSL_ASSERT(pNextPageDescriptor.get() != NULL);
        }
    }
}




void SelectionFunction::ProcessMouseEvent (sal_uInt32 nEventType, const MouseEvent& rEvent)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode (rEvent.GetButtons());

    // 1. Compute some frequently used values relating to the event.
    ::std::auto_ptr<EventDescriptor> pEventDescriptor (
        new EventDescriptor(nEventType, rEvent, mrSlideSorter));

    // 2. Compute a numerical code that describes the event and that is used
    // for fast look-up of the associated reaction.
    pEventDescriptor->mnEventCode = EncodeMouseEvent(*pEventDescriptor, rEvent);

    // 3. Process the event.
    EventPreprocessing(*pEventDescriptor);
    if ( ! EventProcessing(*pEventDescriptor))
    {
        OSL_TRACE ("can not handle event code %x", pEventDescriptor->mnEventCode);
    }
    EventPostprocessing(*pEventDescriptor);

    if (nEventType == BUTTON_UP)
        mbPageHit = false;
}




sal_uInt32 SelectionFunction::EncodeMouseEvent (
    const EventDescriptor& rDescriptor,
    const MouseEvent& rEvent) const
{
    // Initialize with the type of mouse event.
    sal_uInt32 nEventCode (rDescriptor.mnEventCode & (BUTTON_DOWN | BUTTON_UP | MOUSE_MOTION));

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

    // Detect whether the event has happened over a page object.
    if (rDescriptor.mpHitPage != NULL && ! rDescriptor.mpHitDescriptor.expired())
    {
        model::SharedPageDescriptor pHitDescriptor (rDescriptor.mpHitDescriptor);
        if (pHitDescriptor->IsSelected())
            nEventCode |= OVER_SELECTED_PAGE;
        else
            nEventCode |= OVER_UNSELECTED_PAGE;
    }

    // Detect pressed modifier keys.
    if (rEvent.IsShift())
        nEventCode |= SHIFT_MODIFIER;
    if (rEvent.IsMod1())
        nEventCode |= CONTROL_MODIFIER;

    // Detect whether we are dragging pages or dragging a selection rectangle.
    view::ViewOverlay& rOverlay (mrSlideSorter.GetView().GetOverlay());
    if (rOverlay.GetSubstitutionOverlay().isVisible())
        nEventCode |= SUBSTITUTION_VISIBLE;
    if (rOverlay.GetSelectionRectangleOverlay().isVisible())
        nEventCode |= RECTANGLE_VISIBLE;

    return nEventCode;
}




void SelectionFunction::ProcessKeyEvent (const KeyEvent& rEvent)
{
    // 1. Compute some frequently used values relating to the event.
    ::std::auto_ptr<EventDescriptor> pEventDescriptor (
        new EventDescriptor(rEvent, mrSlideSorter));

    // 2. Encode the event.
    pEventDescriptor->mnEventCode = EncodeKeyEvent(*pEventDescriptor, rEvent);

    // 3. Process the event.
    EventPreprocessing(*pEventDescriptor);
    if ( ! EventProcessing(*pEventDescriptor))
        OSL_TRACE ("can not handle event code %x", pEventDescriptor->mnEventCode);
    EventPostprocessing(*pEventDescriptor);
}




sal_uInt32 SelectionFunction::EncodeKeyEvent (
    const EventDescriptor& rDescriptor,
    const KeyEvent& rEvent) const
{
    // Initialize as key event.
    sal_uInt32 nEventCode (KEY_EVENT);

    // Add the actual key code in the lower 16 bit.
    nEventCode |= rEvent.GetKeyCode().GetCode();

    // Detect pressed modifier keys.
    if (rEvent.GetKeyCode().IsShift())
        nEventCode |= SHIFT_MODIFIER;
    if (rEvent.GetKeyCode().IsMod1())
        nEventCode |= CONTROL_MODIFIER;

    // Detect whether the event has happened over a page object.
    if (rDescriptor.mpHitPage != NULL && ! rDescriptor.mpHitDescriptor.expired())
    {
        model::SharedPageDescriptor pHitDescriptor (rDescriptor.mpHitDescriptor);
        if (pHitDescriptor->IsSelected())
            nEventCode |= OVER_SELECTED_PAGE;
        else
            nEventCode |= OVER_UNSELECTED_PAGE;
    }

    // Detect whether we are dragging pages or dragging a selection rectangle.
    view::ViewOverlay& rOverlay (mrSlideSorter.GetView().GetOverlay());
    if (rOverlay.GetSubstitutionOverlay().isVisible())
        nEventCode |= SUBSTITUTION_VISIBLE;
    if (rOverlay.GetSelectionRectangleOverlay().isVisible())
        nEventCode |= RECTANGLE_VISIBLE;

    return nEventCode;
}



void SelectionFunction::EventPreprocessing (const EventDescriptor& rDescriptor)
{
    // Some general processing that is not specific to the exact event code.
    if (rDescriptor.mnEventCode & BUTTON_DOWN)
        mbPageHit = (rDescriptor.mpHitPage!=NULL);

    // Set the focus to the slide under the mouse.
    if (rDescriptor.mpHitPage != NULL)
        mrController.GetFocusManager().FocusPage((rDescriptor.mpHitPage->GetPageNum()-1)/2);
}




bool SelectionFunction::EventProcessing (const EventDescriptor& rDescriptor)
{
    // Define some macros to make the following switch statement more readable.
#define ANY_MODIFIER(code)                      \
         code|NO_MODIFIER:                      \
    case code|SHIFT_MODIFIER:                   \
    case code|CONTROL_MODIFIER
#define ANY_PAGE(code)                          \
         code|NOT_OVER_PAGE:                    \
    case code|OVER_UNSELECTED_PAGE:             \
    case code|OVER_SELECTED_PAGE
#define ANY_PAGE_AND_MODIFIER(code)         \
         ANY_PAGE(code|NO_MODIFIER):        \
    case ANY_PAGE(code|SHIFT_MODIFIER):     \
    case ANY_PAGE(code|CONTROL_MODIFIER)


    bool bResult (true);
    bool bMakeSelectionVisible (true);

    mrController.GetPageSelector().DisableBroadcasting();

    // 2b. With the event code determine the type of operation with which to
    // react to the event.

    // Acquire a shared_ptr to the hit page descriptor.
    model::SharedPageDescriptor pHitDescriptor;
    if ( ! rDescriptor.mpHitDescriptor.expired())
        pHitDescriptor = model::SharedPageDescriptor(rDescriptor.mpHitDescriptor);

    switch (rDescriptor.mnEventCode)
    {
        case BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | OVER_UNSELECTED_PAGE:
            SetCurrentPage(pHitDescriptor);
            PrepareMouseMotion(mpWindow->PixelToLogic(rDescriptor.maMousePosition));
            mpSubstitutionHandler->Start(rDescriptor.maMouseModelPosition);
            break;

        case BUTTON_UP | LEFT_BUTTON | SINGLE_CLICK | OVER_SELECTED_PAGE:
            SetCurrentPage(pHitDescriptor);
            mpSubstitutionHandler->End();
            break;

        case BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | OVER_SELECTED_PAGE:
            PrepareMouseMotion(mpWindow->PixelToLogic(rDescriptor.maMousePosition));
            mpSubstitutionHandler->Start(rDescriptor.maMouseModelPosition);
            break;

        case BUTTON_DOWN | LEFT_BUTTON | DOUBLE_CLICK | OVER_SELECTED_PAGE:
        case BUTTON_DOWN | LEFT_BUTTON | DOUBLE_CLICK | OVER_UNSELECTED_PAGE:
            // A double click allways shows the selected slide in the center
            // pane in an edit view.
            SetCurrentPage(pHitDescriptor);
            SwitchView(pHitDescriptor);
            break;

        // Multi selection with the control modifier.
        case BUTTON_UP | LEFT_BUTTON | SINGLE_CLICK | OVER_SELECTED_PAGE | CONTROL_MODIFIER:
            DeselectHitPage(pHitDescriptor);
            PrepareMouseMotion(mpWindow->PixelToLogic(rDescriptor.maMousePosition));
            break;

        case BUTTON_UP | LEFT_BUTTON | SINGLE_CLICK | OVER_UNSELECTED_PAGE | CONTROL_MODIFIER:
            SelectHitPage(pHitDescriptor);
            PrepareMouseMotion(mpWindow->PixelToLogic(rDescriptor.maMousePosition));

            break;

        // Range selection with the shift modifier.
        case BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | OVER_SELECTED_PAGE | SHIFT_MODIFIER:
        case BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | OVER_UNSELECTED_PAGE | SHIFT_MODIFIER:
            RangeSelect(pHitDescriptor);
            break;

        // Was: Preview of the page transition effect.
        case BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | OVER_FADE_INDICATOR:
            // No action.
            break;

        // Right button for context menu.
        case BUTTON_DOWN | RIGHT_BUTTON | SINGLE_CLICK | OVER_UNSELECTED_PAGE:
        case KEY_EVENT | KEY_F10 | SHIFT_MODIFIER | OVER_UNSELECTED_PAGE:
            // Single right click and shift+F10 select as preparation to
            // show the context menu.  Change the selection only when the
            // page under the mouse is not selected.  In this case the
            // selection is set to this single page.  Otherwise the
            // selection is not modified.
            DeselectAllPages();
            SelectHitPage(pHitDescriptor);
            SetCurrentPage(pHitDescriptor);
            bMakeSelectionVisible = false;
            break;

        case BUTTON_DOWN | RIGHT_BUTTON | SINGLE_CLICK | OVER_SELECTED_PAGE:
        case KEY_EVENT | KEY_F10 | OVER_SELECTED_PAGE | SHIFT_MODIFIER:
            // Do not change the selection.  Just adjust the insertion indicator.
            bMakeSelectionVisible = false;
            break;

        case BUTTON_DOWN | RIGHT_BUTTON | SINGLE_CLICK | NOT_OVER_PAGE:
        // The Shift+F10 key event is here just for completeness. It should
        // not be possible to really receive this (not being over a page.)
        case KEY_EVENT | KEY_F10 | SHIFT_MODIFIER | NOT_OVER_PAGE:
            DeselectAllPages();
            bMakeSelectionVisible = false;
            break;

        // A mouse motion without visible substitution starts that.
        case ANY_MODIFIER(MOUSE_MOTION | LEFT_BUTTON | SINGLE_CLICK | OVER_SELECTED_PAGE):
            mrController.GetScrollBarManager().AutoScroll(rDescriptor.maMousePosition);
            mpSubstitutionHandler->Start(rDescriptor.maMouseModelPosition);
            break;

        // Move substitution.
        case ANY_PAGE_AND_MODIFIER(MOUSE_MOTION | LEFT_BUTTON | SINGLE_CLICK | SUBSTITUTION_VISIBLE):
            if ((rDescriptor.mnEventCode & CONTROL_MODIFIER) != 0)
                StartDrag();
            mrController.GetScrollBarManager().AutoScroll(rDescriptor.maMousePosition);
            mpSubstitutionHandler->UpdatePosition(rDescriptor.maMouseModelPosition);
            break;

        // Place substitution.
        case ANY_PAGE_AND_MODIFIER(BUTTON_UP | LEFT_BUTTON | SINGLE_CLICK | SUBSTITUTION_VISIBLE):
            // When the substitution has not been moved the button up event
            // is taken to be part of a single click.  The selected pages
            // are therefore not moved (which technically would be necessary
            // for unconsecutive multi selections).  Instead the page under
            // the mouse becomes the only selected page.
            if (mpSubstitutionHandler->HasBeenMoved())
            {
                // The following Process() call may lead to the desctruction
                // of pHitDescriptor so release our reference to it.
                pHitDescriptor.reset();
                mpSubstitutionHandler->Process();
            }
            else
                if (pHitDescriptor != NULL)
                    SetCurrentPage(pHitDescriptor);
            mpSubstitutionHandler->End();
            break;

        // Rectangle selection.
        case BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | NOT_OVER_PAGE | NO_MODIFIER:
            DeselectAllPages();
            StartRectangleSelection(rDescriptor.maMouseModelPosition);
            break;

        case BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | NOT_OVER_PAGE | SHIFT_MODIFIER:
        case BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | NOT_OVER_PAGE | CONTROL_MODIFIER:
            StartRectangleSelection(rDescriptor.maMouseModelPosition);
            break;

        case ANY_MODIFIER(MOUSE_MOTION | LEFT_BUTTON | SINGLE_CLICK | NOT_OVER_PAGE):
        case ANY_MODIFIER(MOUSE_MOTION | LEFT_BUTTON | SINGLE_CLICK | OVER_UNSELECTED_PAGE):
        case ANY_PAGE_AND_MODIFIER(MOUSE_MOTION | LEFT_BUTTON | SINGLE_CLICK | RECTANGLE_VISIBLE):
            mrController.GetScrollBarManager().AutoScroll(rDescriptor.maMousePosition);
            UpdateRectangleSelection(rDescriptor.maMouseModelPosition);
            break;

        case ANY_PAGE(BUTTON_UP | LEFT_BUTTON | SINGLE_CLICK | RECTANGLE_VISIBLE | NO_MODIFIER):
            ProcessRectangleSelection(false);
            break;

        case ANY_PAGE(BUTTON_UP | LEFT_BUTTON | SINGLE_CLICK | RECTANGLE_VISIBLE | SHIFT_MODIFIER):
        case ANY_PAGE(BUTTON_UP | LEFT_BUTTON | SINGLE_CLICK | RECTANGLE_VISIBLE | CONTROL_MODIFIER):
            ProcessRectangleSelection(true);
            break;

        default:
            bResult = false;
            break;
    }
    mrController.GetPageSelector().EnableBroadcasting(bMakeSelectionVisible);

    return bResult;
}




void SelectionFunction::EventPostprocessing (const EventDescriptor& rDescriptor)
{
    if (rDescriptor.mnEventCode & BUTTON_UP)
    {
        view::ViewOverlay& rOverlay (mrSlideSorter.GetView().GetOverlay());

        mpWindow->ReleaseMouse();

        // The overlays should not be visible anymore.  Warn when one of
        // them still is.  An exception is th insertion indicator in the
        // case that the context menu is visible.
        DBG_ASSERT(
            mrController.IsContextMenuOpen()
                || !rOverlay.GetInsertionIndicatorOverlay().isVisible(),
            "slidesorter::SelectionFunction: insertion indicator still visible");
        DBG_ASSERT(
            !rOverlay.GetSubstitutionOverlay().isVisible(),
            "slidesorter::SelectionFunction: substitution still visible");
        DBG_ASSERT(
            !rOverlay.GetSelectionRectangleOverlay().isVisible(),
            "slidesorter::SelectionFunction: selection rectangle still visible");

        // Now turn them off.
        if ( ! mrController.IsContextMenuOpen())
            rOverlay.GetInsertionIndicatorOverlay().setVisible(false);
        rOverlay.GetSubstitutionOverlay().setVisible(false);
        rOverlay.GetSelectionRectangleOverlay().setVisible(false);
    }
}




void SelectionFunction::SetCurrentPage (const model::SharedPageDescriptor& rpDescriptor)
{
    PageSelector& rSelector (mrController.GetPageSelector());
    rSelector.DeselectAllPages ();
    rSelector.SelectPage(rpDescriptor);

    mrController.GetCurrentSlideManager()->SwitchCurrentSlide(rpDescriptor);
}




void SelectionFunction::SwitchView (const model::SharedPageDescriptor& rpDescriptor)
{
    // Switch to the draw view.  This is done only when the current
    // view is the main view.
    ViewShell* pViewShell = mrSlideSorter.GetViewShell();
    if (pViewShell!=NULL && pViewShell->IsMainViewShell())
    {
        if (rpDescriptor.get()!=NULL && rpDescriptor->GetPage()!=NULL)
        {
            mrSlideSorter.GetModel().GetDocument()->SetSelected(rpDescriptor->GetPage(), TRUE);
            mpViewShell->GetFrameView()->SetSelectedPage(
                (rpDescriptor->GetPage()->GetPageNum()-1)/2);
        }
        if (mrSlideSorter.GetViewShellBase() != NULL)
        framework::FrameworkHelper::Instance(*mrSlideSorter.GetViewShellBase())->RequestView(
            framework::FrameworkHelper::msImpressViewURL,
            framework::FrameworkHelper::msCenterPaneURL);
    }
}




//===== EventDescriptor =======================================================

SelectionFunction::EventDescriptor::EventDescriptor (
    sal_uInt32 nEventType,
    const MouseEvent& rEvent,
    SlideSorter& rSlideSorter)
    : maMousePosition(),
      maMouseModelPosition(),
      mpHitDescriptor(),
      mpHitPage(),
      mnEventCode(nEventType)
{
    ::Window* pWindow = rSlideSorter.GetActiveWindow();

    maMousePosition = rEvent.GetPosPixel();
    maMouseModelPosition = pWindow->PixelToLogic(maMousePosition);
    model::SharedPageDescriptor pHitDescriptor (
        rSlideSorter.GetController().GetPageAt(maMousePosition));
    if (pHitDescriptor.get() != NULL)
    {
        mpHitDescriptor = pHitDescriptor;
        mpHitPage = pHitDescriptor->GetPage();
    }
}





SelectionFunction::EventDescriptor::EventDescriptor (
    const KeyEvent&,
    SlideSorter& rSlideSorter)
    : maMousePosition(),
      maMouseModelPosition(),
      mpHitDescriptor(),
      mpHitPage(),
      mnEventCode(0)
{
    mpHitDescriptor = rSlideSorter.GetController().GetFocusManager().GetFocusedPageDescriptor();
    model::SharedPageDescriptor pHitDescriptor (
        rSlideSorter.GetController().GetFocusManager().GetFocusedPageDescriptor());
    if (pHitDescriptor.get() != NULL)
    {
        mpHitPage = pHitDescriptor->GetPage();
        mpHitDescriptor = pHitDescriptor;
    }
}





//===== SubstitutionHandler ===================================================

SelectionFunction::SubstitutionHandler::SubstitutionHandler (SlideSorter& rSlideSorter)
    : mrSlideSorter(rSlideSorter),
      mbHasBeenMoved(false)
{
}




SelectionFunction::SubstitutionHandler::~SubstitutionHandler (void)
{
    if (mrSlideSorter.IsValid())
    {
        view::ViewOverlay& rOverlay (mrSlideSorter.GetView().GetOverlay());
        rOverlay.GetSubstitutionOverlay().setVisible(false);
        rOverlay.GetSubstitutionOverlay().Clear();
    }
}




void SelectionFunction::SubstitutionHandler::Start (const Point& rMouseModelPosition)
{
    // No Drag-and-Drop for master pages.
    if (mrSlideSorter.GetModel().GetEditMode() != EM_PAGE)
        return;

    if (mrSlideSorter.GetController().GetProperties()->IsUIReadOnly())
        return;

    view::ViewOverlay& rOverlay (mrSlideSorter.GetView().GetOverlay());

    if ( ! rOverlay.GetSubstitutionOverlay().isVisible())
    {
        // Show a new substitution for the selected page objects.
        model::PageEnumeration aSelectedPages(
            model::PageEnumerationProvider::CreateSelectedPagesEnumeration(
                mrSlideSorter.GetModel()));
        rOverlay.GetSubstitutionOverlay().Create(aSelectedPages, rMouseModelPosition);
        rOverlay.GetSubstitutionOverlay().setVisible(true);
        mbHasBeenMoved = false;
    }
    else
        UpdatePosition(rMouseModelPosition);
}




void SelectionFunction::SubstitutionHandler::UpdatePosition (const Point& rMouseModelPosition)
{
    if (mrSlideSorter.GetController().GetProperties()->IsUIReadOnly())
        return;

    view::ViewOverlay& rOverlay (mrSlideSorter.GetView().GetOverlay());

    // Move the existing substitution to the new position.
    rOverlay.GetSubstitutionOverlay().SetPosition(rMouseModelPosition);

    rOverlay.GetInsertionIndicatorOverlay().SetPosition(rMouseModelPosition);
    rOverlay.GetInsertionIndicatorOverlay().setVisible(true);

    mbHasBeenMoved = true;
}




void SelectionFunction::SubstitutionHandler::Process (void)
{
    if (mrSlideSorter.GetController().GetProperties()->IsUIReadOnly())
        return;

    view::ViewOverlay& rOverlay (mrSlideSorter.GetView().GetOverlay());

    if (IsSubstitutionInsertionNonTrivial())
    {
        // Tell the model to move the selected pages behind the one with the
        // index mnInsertionIndex which first has to transformed into an index
        // understandable by the document.
        sal_Int32 nInsertionIndex = rOverlay.GetInsertionIndicatorOverlay().GetInsertionPageIndex();
        if (nInsertionIndex >= 0)
        {
            USHORT nDocumentIndex = (USHORT)nInsertionIndex-1;
            mrSlideSorter.GetController().GetSelectionManager()->MoveSelectedPages(nDocumentIndex);
        }

        ViewShell* pViewShell = mrSlideSorter.GetViewShell();
        if (pViewShell != NULL)
            pViewShell->GetViewFrame()->GetBindings().Invalidate(SID_STATUS_PAGE);
    }
}




void SelectionFunction::SubstitutionHandler::End (void)
{
    view::ViewOverlay& rOverlay (mrSlideSorter.GetView().GetOverlay());
    rOverlay.GetSubstitutionOverlay().setVisible(false);
    rOverlay.GetSubstitutionOverlay().Clear();
    rOverlay.GetInsertionIndicatorOverlay().setVisible(false);
}




bool SelectionFunction::SubstitutionHandler::HasBeenMoved (void) const
{
    return mbHasBeenMoved;
}




bool SelectionFunction::SubstitutionHandler::IsSubstitutionInsertionNonTrivial (void) const
{
    bool bIsNonTrivial = false;

    do
    {
        view::ViewOverlay& rOverlay (mrSlideSorter.GetView().GetOverlay());

        // Make sure that the substitution and the insertion indicator are visible.
        if ( ! rOverlay.GetSubstitutionOverlay().isVisible())
            break;
        if ( ! rOverlay.GetInsertionIndicatorOverlay().isVisible())
            break;

        // Iterate over all selected pages and check whether there are
        // holes.  While we do this we remember the indices of the first and
        // last selected page as preparation for the next step.
        sal_Int32 nCurrentIndex = -1;
        sal_Int32 nFirstIndex = -1;
        sal_Int32 nLastIndex = -1;
        model::PageEnumeration aSelectedPages (
            model::PageEnumerationProvider::CreateSelectedPagesEnumeration(
                mrSlideSorter.GetModel()));
        while (aSelectedPages.HasMoreElements() && ! bIsNonTrivial)
        {
            model::SharedPageDescriptor pDescriptor (aSelectedPages.GetNextElement());
            SdPage* pPage = pDescriptor->GetPage();
            if (pPage != NULL)
            {
                // Get the page number and compare it to the last one.
                sal_Int32 nPageNumber = (pPage->GetPageNum()-1)/2;
                if (nCurrentIndex>=0 && nPageNumber>(nCurrentIndex+1))
                    bIsNonTrivial = true;
                else
                    nCurrentIndex = nPageNumber;

                // Remember indices of the first and last page of the selection.
                if (nFirstIndex == -1)
                    nFirstIndex = nPageNumber;
                nLastIndex = nPageNumber;
            }
        }
        if (bIsNonTrivial)
            break;

        // When we come here then the selection is consecutive.  We still
        // have to check that the insertion position is not directly in
        // front or directly behind the selection and thus moving the
        // selection there would not change the model.
        sal_Int32 nInsertionIndex = rOverlay.GetInsertionIndicatorOverlay().GetInsertionPageIndex();
        if (nInsertionIndex<nFirstIndex || nInsertionIndex>(nLastIndex+1))
            bIsNonTrivial = true;
    }
    while (false);

    return bIsNonTrivial;
}

} } } // end of namespace ::sd::slidesorter::controller

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
