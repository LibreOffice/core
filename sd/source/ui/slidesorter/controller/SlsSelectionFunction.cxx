/*************************************************************************
 *
 *  $RCSfile: SlsSelectionFunction.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: kz $ $Date: 2005-07-14 10:16:28 $
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

#include "controller/SlsSelectionFunction.hxx"

#ifndef SD_SLIDESORTER_SLIDE_SORTER_VIEW_SHELL_HXX
#include "SlideSorterViewShell.hxx"
#endif
#include "controller/SlideSorterController.hxx"
#include "controller/SlsPageSelector.hxx"
#include "controller/SlsFocusManager.hxx"
#include "controller/SlsScrollBarManager.hxx"
#include "controller/SlsClipboard.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "model/SlsPageEnumeration.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsViewOverlay.hxx"
#include "view/SlsLayouter.hxx"
#include "view/SlsPageObjectViewObjectContact.hxx"
#ifndef SD_SHOW_VIEW_HXX
#include "showview.hxx"
#endif
#ifndef SD_PANE_MANAGER_HXX
#include "PaneManager.hxx"
#endif
#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif
#include "DrawController.hxx"
#ifndef _SV_SOUND_HXX
#include <vcl/sound.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#include <svx/svdpagv.hxx>
#include <vcl/msgbox.hxx>
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#ifndef _SDPAGE_HXX
#include "sdpage.hxx"
#endif
#ifndef _DRAWDOC_HXX
#include "drawdoc.hxx"
#endif
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#ifndef SD_FRAME_VIEW_HXX
#include "FrameView.hxx"
#endif
#include "app.hrc"
#ifndef SD_RESID_HXX
#include "sdresid.hxx"
#endif
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

void TraceEventCode (int nCode)
{
    if ((nCode & KEY_EVENT) != 0)
        OSL_TRACE("Event code is %x: key %d, %s",
            nCode,
            nCode & 0x0ffff,
            (nCode&KEY_SHIFT)?"shift":"--");
    else
        OSL_TRACE("Event code is %x: %s, %s, %s, %s, %s, %s, %s",
            nCode,
            (nCode&SINGLE_CLICK)?"Single":((nCode&DOUBLE_CLICK)?"Double":"No"),
            (nCode&RIGHT_BUTTON)
                ?"Right"
                :((nCode&MIDDLE_BUTTON)?"Middle":((nCode&LEFT_BUTTON)?"Left":"No")),
            (nCode&BUTTON_DOWN)
                ?"Down"
                :((nCode&BUTTON_UP)?"Up":((nCode&MOUSE_MOTION)?"Motion":"--")),
            (nCode&OVER_SELECTED_PAGE)
                ?"Selected page"
                :((nCode&OVER_UNSELECTED_PAGE)
                    ?"Unselected page"
                    :((nCode&OVER_FADE_INDICATOR)?"fade indicator":"no page")),
            (nCode&SHIFT_MODIFIER)?"Shift":((nCode&CONTROL_MODIFIER)?"Control":"--"),
            (nCode&SUBSTITUTION_VISIBLE)?"substitution":"--",
            (nCode&RECTANGLE_VISIBLE)?"rectangle":"--");
}
#ifdef DEBUG
#define TRACE_EVENT_CODE(code) TraceEventCode(code)
#else
#define TRACE_EVENT_CODE(code) {}
#endif

} // end of anonymous namespace


namespace sd { namespace slidesorter { namespace controller {

class SelectionFunction::SubstitutionHandler
{
public:
    SubstitutionHandler (SlideSorterController& rController);
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
    SlideSorterController& mrController;

    bool mbHasBeenMoved;

    /** Determine whether there is a) a substitution and b) its insertion at
        the current position of the insertion marker would alter the
        document.   This would be the case when the substitution has been
        moved or is not consecutive.
    */
    bool IsSubstitutionInsertionNonTrivial (void) const;
};


class SelectionFunction::InsertionIndicatorHandler
{
public:
    InsertionIndicatorHandler (SlideSorterController& rController);
    ~InsertionIndicatorHandler (void);

    /** Show the insertion marker at the given coordinates.
    */
    void Start (const Point& rMouseModelPosition);

    void UpdatePosition (const Point& rMouseModelPosition);

    /** Hide the insertion marker.
    */
    void End (void);

private:
    SlideSorterController& mrController;
};

class SelectionFunction::EventDescriptor
{
public:

    Point maMousePosition;
    Point maMouseModelPosition;
    model::PageDescriptor* mpHitDescriptor;
    SdrPage* mpHitPage;
    sal_uInt32 mnEventCode;

    EventDescriptor (
        sal_uInt32 nEventType,
        const MouseEvent& rEvent,
        SlideSorterController& rController);
    EventDescriptor (
        const KeyEvent& rEvent,
        SlideSorterController& rController);
};


TYPEINIT1(SelectionFunction, FuPoor);


SelectionFunction::SelectionFunction (
    SlideSorterController& rController,
    SfxRequest& rRequest)
    : SlideFunction (
        rController,
        rRequest),
      mrController (rController),
      mbDragSelection(false),
      maInsertionMarkerBox(),
      mbProcessingMouseButtonDown(false),
      mpSubstitutionHandler(new SubstitutionHandler(rController)),
      mpInsertionIndicatorHandler(new InsertionIndicatorHandler(rController))
{
    //af    aDelayToScrollTimer.SetTimeout(50);
    aDragTimer.SetTimeoutHdl( LINK( this, SelectionFunction, DragSlideHdl ) );
}




SelectionFunction::~SelectionFunction (void)
{
    aDragTimer.Stop();
}




BOOL SelectionFunction::MouseButtonDown (const MouseEvent& rEvent)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode (rEvent.GetButtons());
    mbProcessingMouseButtonDown = true;

    pWindow->CaptureMouse();

    ProcessMouseEvent(BUTTON_DOWN, rEvent);

    return TRUE;
}




BOOL SelectionFunction::MouseMove (const MouseEvent& rEvent)
{
    Point aMousePosition (rEvent.GetPosPixel());

    // Determine page under mouse and show the mouse over effect.
    model::PageDescriptor* pHitDescriptor = mrController.GetPageAt (aMousePosition);
    view::ViewOverlay& rOverlay (mrController.GetView().GetOverlay());
    rOverlay.GetMouseOverIndicatorOverlay().SetSlideUnderMouse(
        rEvent.IsLeaveWindow() ? NULL : pHitDescriptor);
    if (pHitDescriptor != NULL)
        rOverlay.GetMouseOverIndicatorOverlay().Show();
    else
        rOverlay.GetMouseOverIndicatorOverlay().Hide();

    // Allow one mouse move before the drag timer is disabled.
    if (aDragTimer.IsActive())
    {
        if (bFirstMouseMove)
            bFirstMouseMove = FALSE;
        else
            aDragTimer.Stop();
    }

    Rectangle aRectangle (Point(0,0),pWindow->GetOutputSizePixel());
    if ( ! aRectangle.IsInside(aMousePosition)
        && rOverlay.GetSubstitutionOverlay().IsShowing())
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

    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode (rEvent.GetButtons());

    if (aDragTimer.IsActive())
        aDragTimer.Stop();

    ProcessMouseEvent(BUTTON_UP, rEvent);

    mbProcessingMouseButtonDown = false;
    pWindow->ReleaseMouse();

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
                model::PageDescriptor* pDescriptor = rFocusManager.GetFocusedPageDescriptor();
                if (pDescriptor != NULL)
                {
                    SetCurrentPage(*pDescriptor);
                    SwitchView(*pDescriptor);
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
            rFocusManager.HideFocus();
            mrController.GetViewShell().GetParentWindow()->GrabFocus();
            bResult = TRUE;
            break;

        case KEY_SPACE:
        {
            // Toggle the selection state.
            model::PageDescriptor* pDescriptor =
                rFocusManager.GetFocusedPageDescriptor();
            if (pDescriptor != NULL)
            {
                // Doing a multi selection by default.  Can we ask the event
                // for the state of the shift key?
                if (pDescriptor->IsSelected())
                    mrController.GetPageSelector().DeselectPage (*pDescriptor);
                else
                    mrController.GetPageSelector().SelectPage (*pDescriptor);
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

        case KEY_ADD:
            // Scale up by 3/2 of the current zoom scale.  Round so that
            // even with the smallest zoom scale of 1 the scale is
            // increased.
            pViewShell->SetZoom ((pWindow->GetZoom() * 3 + 1) / 2);
            bResult = TRUE;
            break;

        case KEY_SUBTRACT:
            pViewShell->SetZoom(pWindow->GetZoom() * 2 / 3);
            bResult = TRUE;
            break;

        case KEY_DELETE:
        {
            int nSelectedPagesCount = 0;
            int nPageCount = mrController.GetModel().GetPageCount();

            // Count the selected pages and look if there any objects on any
            // of the selected pages so that we can warn the user and
            // prevent an accidental deletion.
            model::SlideSorterModel::Enumeration aSelectedPages (
                mrController.GetModel().GetSelectedPagesEnumeration());
            while (aSelectedPages.HasMoreElements())
            {
                nSelectedPagesCount++;
                aSelectedPages.GetNextElement();
            }

            if (nSelectedPagesCount > 0)
                mrController.DeleteSelectedPages();

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
    // About to start scrolling (i.e. the whole window content changes
    // without complete redraws).  Hide the overlays to avoid conflicts.
    mrController.GetView().GetOverlay().HideAndSave();
}




void SelectionFunction::ScrollEnd (void)
{
    // Scrolling has ended so we can turn on the overlays again.
    mrController.GetView().GetOverlay().Restore();
}




void SelectionFunction::DoCut (void)
{
    mrController.GetClipboard().DoCut();
}




void SelectionFunction::DoCopy (void)
{
    mrController.GetClipboard().DoCopy();
}




void SelectionFunction::DoPaste (void)
{
    mrController.GetClipboard().DoPaste();
}




void SelectionFunction::Paint (const Rectangle& rRect, SdWindow* pWin)
{
    mrController.GetView().GetOverlay().Paint ();
}




IMPL_LINK( SelectionFunction, DragSlideHdl, Timer*, pTimer )
{
    StartDrag();
    return 0;
}




void SelectionFunction::StartDrag (void)
{
    if (mbPageHit)
    {
        view::ViewOverlay& rOverlay (mrController.GetView().GetOverlay());
        mpSubstitutionHandler->Start(rOverlay.GetSubstitutionOverlay().GetPosition());
        mbPageHit = false;
        pWindow->ReleaseMouse();

        mrController.GetViewShell().StartDrag (
            rOverlay.GetSubstitutionOverlay().GetPosition(),
            pWindow);
    }
}




bool SelectionFunction::cancel (void)
{
    mrController.GetFocusManager().ToggleFocus();
    return true;
}




void SelectionFunction::SelectHitPage (model::PageDescriptor& rDescriptor)
{
    mrController.GetPageSelector().SelectPage(rDescriptor);
}




void SelectionFunction::DeselectHitPage (model::PageDescriptor& rDescriptor)
{
    mrController.GetPageSelector().DeselectPage (rDescriptor);
}




void SelectionFunction::DeselectAllPages (void)
{
    mrController.GetPageSelector().DeselectAllPages();
}




void SelectionFunction::StartRectangleSelection (const Point& rMouseModelPosition)
{
    mrController.GetView().GetOverlay().GetSelectionRectangleOverlay().Start(rMouseModelPosition);
}




void SelectionFunction::UpdateRectangleSelection (const Point& rMouseModelPosition)
{
    mrController.GetView().GetOverlay().GetSelectionRectangleOverlay().Update(rMouseModelPosition);
}




void SelectionFunction::ProcessRectangleSelection (bool bToggleSelection)
{
    view::ViewOverlay& rOverlay (mrController.GetView().GetOverlay());
    if (rOverlay.GetSelectionRectangleOverlay().IsShowing())
    {
        PageSelector& rSelector (mrController.GetPageSelector());

        rOverlay.GetSelectionRectangleOverlay().Hide();

        // Select all pages whose page object lies completly inside the drag
        // rectangle.
        const Rectangle& rSelectionRectangle (
            rOverlay.GetSelectionRectangleOverlay().GetSelectionRectangle());
        model::SlideSorterModel::Enumeration aPages (
            mrController.GetModel().GetAllPagesEnumeration());
        bool bFirstPage = true;
        while (aPages.HasMoreElements())
        {
            model::PageDescriptor& rDescriptor (aPages.GetNextElement());
            Rectangle aPageBox (mrController.GetView().GetPageBoundingBox(
                rDescriptor,
                view::SlideSorterView::CS_MODEL,
                view::SlideSorterView::BBT_SHAPE));
            if (rSelectionRectangle.IsOver(aPageBox))
            {
                // When we are extending the selection (shift key is
                // pressed) then toggle the selection state of the page.
                // Otherwise select it: this results in the previously
                // selected pages becoming deslected.
                if (bToggleSelection && rDescriptor.IsSelected())
                    rSelector.DeselectPage (rDescriptor);
                else
                    rSelector.SelectPage (rDescriptor);
            }
        }
    }
}




void SelectionFunction::PrepareMouseMotion (const Point& aMouseModelPosition)
{
    bFirstMouseMove = TRUE;
    aDragTimer.Start();
}




void SelectionFunction::RangeSelect (model::PageDescriptor& rDescriptor)
{
    PageSelector& rSelector (mrController.GetPageSelector());

    model::PageDescriptor* pAnchor = rSelector.GetSelectionAnchor();
    DeselectAllPages();

    if (pAnchor != NULL)
    {
        // Select all pages between the anchor and the given one, including
        // the two.
        USHORT nAnchorIndex ((pAnchor->GetPage()->GetPageNum()-1) / 2);
        USHORT nOtherIndex ((rDescriptor.GetPage()->GetPageNum()-1) / 2);

        // Iterate over all pages in the range.  Start with the anchor
        // page.  This way the PageSelector will recognize it again as
        // anchor (the first selected page after a DeselectAllPages()
        // becomes the anchor.)
        int nStep = (nAnchorIndex < nOtherIndex) ? +1 : -1;
        USHORT nIndex = nAnchorIndex;
        while (true)
        {
            rSelector.SelectPage(nIndex);
            if (nIndex == nOtherIndex)
                break;
            nIndex += nStep;
        }
    }
}




void SelectionFunction::GotoNextPage (int nOffset)
{
    SdPage* pPage = mrController.GetActualPage();
    if (pPage != NULL)
    {
        sal_Int32 nIndex = (pPage->GetPageNum()-1) / 2;
        nIndex += nOffset;
        USHORT nPageCount = mrController.GetModel().GetPageCount();

        if (nIndex >= nPageCount)
            nIndex = nPageCount - 1;
        if (nIndex < 0)
            nIndex = 0;

        mrController.GetFocusManager().SetFocusedPage(nIndex);
        mrController.GetPageSelector().DeselectAllPages();
        mrController.GetPageSelector().SelectPage(nIndex);
        mrController.GetPageSelector().SetCurrentPage(nIndex);
    }
}




void SelectionFunction::ClearOverlays (void)
{
    view::ViewOverlay& rOverlay (mrController.GetView().GetOverlay());

    rOverlay.GetSubstitutionOverlay().Hide();
    rOverlay.GetSubstitutionOverlay().Clear();

    mpInsertionIndicatorHandler->End();
    rOverlay.GetMouseOverIndicatorOverlay().SetSlideUnderMouse (NULL);
}




void SelectionFunction::ProcessMouseEvent (sal_uInt32 nEventType, const MouseEvent& rEvent)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode (rEvent.GetButtons());

    // 1. Compute some frequently used values relating to the event.
    ::std::auto_ptr<EventDescriptor> pEventDescriptor (
        new EventDescriptor(nEventType, rEvent, mrController));

    // 2. Compute a numerical code that describes the event and that is used
    // for fast look-up of the associated reaction.
    pEventDescriptor->mnEventCode = EncodeMouseEvent(*pEventDescriptor, rEvent);
    OSL_TRACE ("EventCode is %x", pEventDescriptor->mnEventCode);
    TRACE_EVENT_CODE(pEventDescriptor->mnEventCode);

    // 3. Process the event.
    EventPreprocessing(*pEventDescriptor);
    if ( ! EventProcessing(*pEventDescriptor))
        OSL_TRACE ("can not handle event code %x", pEventDescriptor->mnEventCode);
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
    if (rDescriptor.mpHitPage != NULL)
        if (rDescriptor.mpHitDescriptor->IsSelected())
            nEventCode |= OVER_SELECTED_PAGE;
        else
            nEventCode |= OVER_UNSELECTED_PAGE;

    // Detect pressed modifier keys.
    if (rEvent.IsShift())
        nEventCode |= SHIFT_MODIFIER;
    if (rEvent.IsMod1())
        nEventCode |= CONTROL_MODIFIER;

    // Detect whether we are dragging pages or dragging a selection rectangle.
    view::ViewOverlay& rOverlay (mrController.GetView().GetOverlay());
    if (rOverlay.GetSubstitutionOverlay().IsShowing())
        nEventCode |= SUBSTITUTION_VISIBLE;
    if (rOverlay.GetSelectionRectangleOverlay().IsShowing())
        nEventCode |= RECTANGLE_VISIBLE;

    return nEventCode;
}




void SelectionFunction::ProcessKeyEvent (const KeyEvent& rEvent)
{
    // 1. Compute some frequently used values relating to the event.
    ::std::auto_ptr<EventDescriptor> pEventDescriptor (
        new EventDescriptor(rEvent, mrController));

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
    if (rDescriptor.mpHitPage != NULL)
        if (rDescriptor.mpHitDescriptor->IsSelected())
            nEventCode |= OVER_SELECTED_PAGE;
        else
            nEventCode |= OVER_UNSELECTED_PAGE;

    // Detect whether we are dragging pages or dragging a selection rectangle.
    view::ViewOverlay& rOverlay (mrController.GetView().GetOverlay());
    if (rOverlay.GetSubstitutionOverlay().IsShowing())
        nEventCode |= SUBSTITUTION_VISIBLE;
    if (rOverlay.GetSelectionRectangleOverlay().IsShowing())
        nEventCode |= RECTANGLE_VISIBLE;

    return nEventCode;
}



void SelectionFunction::EventPreprocessing (const EventDescriptor& rDescriptor)
{
    // Some general processing that is not specific to the exact event code.
    bool bMakeSelectionVisible = true;
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

    switch (rDescriptor.mnEventCode)
    {
        case BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | OVER_UNSELECTED_PAGE:
            SetCurrentPage(*rDescriptor.mpHitDescriptor);
            PrepareMouseMotion(pWindow->PixelToLogic(rDescriptor.maMousePosition));
            mpSubstitutionHandler->Start(rDescriptor.maMouseModelPosition);
            break;

        case BUTTON_UP | LEFT_BUTTON | SINGLE_CLICK | OVER_SELECTED_PAGE:
            SetCurrentPage(*rDescriptor.mpHitDescriptor);
            mpSubstitutionHandler->End();
            break;

        case BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | OVER_SELECTED_PAGE:
            PrepareMouseMotion(pWindow->PixelToLogic(rDescriptor.maMousePosition));
            mpSubstitutionHandler->Start(rDescriptor.maMouseModelPosition);
            break;

        case BUTTON_DOWN | LEFT_BUTTON | DOUBLE_CLICK | OVER_SELECTED_PAGE:
        case BUTTON_DOWN | LEFT_BUTTON | DOUBLE_CLICK | OVER_UNSELECTED_PAGE:
            // A double click allways shows the selected slide in the center
            // pane in an edit view.
            SetCurrentPage(*rDescriptor.mpHitDescriptor);
            SwitchView(*rDescriptor.mpHitDescriptor);
            break;

        // Multi selection with the control modifier.
        case BUTTON_UP | LEFT_BUTTON | SINGLE_CLICK | OVER_SELECTED_PAGE | CONTROL_MODIFIER:
            DeselectHitPage(*rDescriptor.mpHitDescriptor);
            PrepareMouseMotion(pWindow->PixelToLogic(rDescriptor.maMousePosition));
            break;

        case BUTTON_UP | LEFT_BUTTON | SINGLE_CLICK | OVER_UNSELECTED_PAGE | CONTROL_MODIFIER:
            SelectHitPage(*rDescriptor.mpHitDescriptor);
            PrepareMouseMotion(pWindow->PixelToLogic(rDescriptor.maMousePosition));
            break;

        // Range selection with the shift modifier.
        case BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | OVER_SELECTED_PAGE | SHIFT_MODIFIER:
        case BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | OVER_UNSELECTED_PAGE | SHIFT_MODIFIER:
            RangeSelect(*rDescriptor.mpHitDescriptor);
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
            SelectHitPage(*rDescriptor.mpHitDescriptor);
            SetCurrentPage(*rDescriptor.mpHitDescriptor);
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
                mpSubstitutionHandler->Process();
            else
                if (rDescriptor.mpHitDescriptor != NULL)
                    SetCurrentPage(*rDescriptor.mpHitDescriptor);
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
        view::ViewOverlay& rOverlay (mrController.GetView().GetOverlay());

        pWindow->ReleaseMouse();

        // The overlays should not be visible anymore.  Warn when one of
        // them still is.
        DBG_ASSERT(
            !rOverlay.GetInsertionIndicatorOverlay().IsShowing(),
            "slidesorter::SelectionFunction: insertion indicator still visible");
        DBG_ASSERT(
            !rOverlay.GetSubstitutionOverlay().IsShowing(),
            "slidesorter::SelectionFunction: substitution still visible");
        DBG_ASSERT(
            !rOverlay.GetSelectionRectangleOverlay().IsShowing(),
            "slidesorter::SelectionFunction: selection rectangle still visible");

        // Now turn them off.
        rOverlay.GetInsertionIndicatorOverlay().Hide();
        rOverlay.GetSubstitutionOverlay().Hide();
        rOverlay.GetSelectionRectangleOverlay().Hide();
    }
}




void SelectionFunction::SetCurrentPage (model::PageDescriptor& rDescriptor)
{
    PageSelector& rSelector (mrController.GetPageSelector());

    rSelector.DeselectAllPages ();
    rSelector.SelectPage (rDescriptor);
    rSelector.SetCurrentPage (rDescriptor);
}




void SelectionFunction::SwitchView (model::PageDescriptor& rDescriptor)
{
    // Switch to the draw view.  This is done only when the current
    // view is the main view.
    if (mrController.GetViewShell().IsMainViewShell())
    {
        mrController.GetModel().GetDocument()->SetSelected(rDescriptor.GetPage(), TRUE);
        pViewShell->GetFrameView()->SetSelectedPage((rDescriptor.GetPage()->GetPageNum()-1)/2);
        mrController.GetViewShell().GetViewShellBase().GetPaneManager().RequestMainViewShellChange(
            ViewShell::ST_IMPRESS);
    }
}




//===== EventDescriptor =======================================================

SelectionFunction::EventDescriptor::EventDescriptor (
    sal_uInt32 nEventType,
    const MouseEvent& rEvent,
    SlideSorterController& rController)
    : maMousePosition(),
      maMouseModelPosition(),
      mpHitDescriptor(NULL),
      mpHitPage(NULL),
      mnEventCode(nEventType)
{
    ::Window* pWindow = rController.GetViewShell().GetActiveWindow();

    maMousePosition = rEvent.GetPosPixel();
    maMouseModelPosition = pWindow->PixelToLogic(maMousePosition);
    mpHitDescriptor = rController.GetPageAt(maMousePosition);
    if (mpHitDescriptor != NULL)
        mpHitPage = mpHitDescriptor->GetPage();
}





SelectionFunction::EventDescriptor::EventDescriptor (
    const KeyEvent& rEvent,
    SlideSorterController& rController)
    : maMousePosition(),
      maMouseModelPosition(),
      mpHitDescriptor(NULL),
      mpHitPage(NULL),
      mnEventCode(0)
{
    ::Window* pWindow = rController.GetViewShell().GetActiveWindow();
    mpHitDescriptor = rController.GetFocusManager().GetFocusedPageDescriptor();
    if (mpHitDescriptor != NULL)
        mpHitPage = mpHitDescriptor->GetPage();
}





//===== SubstitutionHandler ===================================================

SelectionFunction::SubstitutionHandler::SubstitutionHandler (SlideSorterController& rController)
    : mrController(rController),
      mbHasBeenMoved(false)
{
}




SelectionFunction::SubstitutionHandler::~SubstitutionHandler (void)
{
    view::ViewOverlay& rOverlay (mrController.GetView().GetOverlay());
    rOverlay.GetSubstitutionOverlay().Hide();
    rOverlay.GetSubstitutionOverlay().Clear();
}




void SelectionFunction::SubstitutionHandler::Start (const Point& rMouseModelPosition)
{
    // No Drag-and-Drop for master pages.
    if (mrController.GetModel().GetEditMode() == EM_PAGE)
    {
        view::ViewOverlay& rOverlay (mrController.GetView().GetOverlay());

        if ( ! rOverlay.GetSubstitutionOverlay().IsShowing())
        {
            // Show a new substitution for the selected page objects.
            model::PageEnumeration aSelectedPages(
                mrController.GetModel().GetSelectedPagesEnumeration());
            rOverlay.GetSubstitutionOverlay().Create(aSelectedPages, rMouseModelPosition);
            rOverlay.GetSubstitutionOverlay().Show();
            mbHasBeenMoved = false;
        }
        else
            UpdatePosition(rMouseModelPosition);
    }
}




void SelectionFunction::SubstitutionHandler::UpdatePosition (const Point& rMouseModelPosition)
{
    view::ViewOverlay& rOverlay (mrController.GetView().GetOverlay());

    // Move the existing substitution to the new position.
    rOverlay.GetSubstitutionOverlay().SetPosition(rMouseModelPosition);

    rOverlay.GetInsertionIndicatorOverlay().SetPosition(rMouseModelPosition);
    rOverlay.GetInsertionIndicatorOverlay().Show();

    mbHasBeenMoved = true;
}




void SelectionFunction::SubstitutionHandler::Process (void)
{
    view::ViewOverlay& rOverlay (mrController.GetView().GetOverlay());

    if (IsSubstitutionInsertionNonTrivial())
    {
        // Tell the model to move the selected pages behind the one with the
        // index mnInsertionIndex which first has to transformed into an index
        // understandable by the document.
        sal_Int32 nInsertionIndex = rOverlay.GetInsertionIndicatorOverlay().GetInsertionPageIndex();
        if (nInsertionIndex >= 0)
        {
            USHORT nDocumentIndex = (USHORT)nInsertionIndex-1;
            model::SlideSorterModel& rModel (mrController.GetModel());
            mrController.MoveSelectedPages(nDocumentIndex);
        }

        mrController.GetViewShell().GetViewFrame()->GetBindings().Invalidate(SID_STATUS_PAGE);
    }
}




void SelectionFunction::SubstitutionHandler::End (void)
{
    view::ViewOverlay& rOverlay (mrController.GetView().GetOverlay());
    rOverlay.GetSubstitutionOverlay().Hide();
    rOverlay.GetSubstitutionOverlay().Clear();
    rOverlay.GetInsertionIndicatorOverlay().Hide();
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
        view::ViewOverlay& rOverlay (mrController.GetView().GetOverlay());

        // Make sure that the substitution and the insertion indicator are visible.
        if ( ! rOverlay.GetSubstitutionOverlay().IsShowing())
            break;
        if ( ! rOverlay.GetInsertionIndicatorOverlay().IsShowing())
            break;

        // Iterate over all selected pages and check whether there are
        // holes.  While we do this we remember the indices of the first and
        // last selected page as preparation for the next step.
        sal_Int32 nCurrentIndex = -1;
        sal_Int32 nFirstIndex = -1;
        sal_Int32 nLastIndex = -1;
        model::PageEnumeration aSelectedPages(mrController.GetModel().GetSelectedPagesEnumeration());
        while (aSelectedPages.HasMoreElements() && ! bIsNonTrivial)
        {
            model::PageDescriptor& rDescriptor (aSelectedPages.GetNextElement());
            SdPage* pPage = rDescriptor.GetPage();
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




//===== InsertionIndicatorHandler =============================================

SelectionFunction::InsertionIndicatorHandler::InsertionIndicatorHandler (
    SlideSorterController& rController)
    : mrController(rController)
{
}




SelectionFunction::InsertionIndicatorHandler::~InsertionIndicatorHandler (void)
{
}




void SelectionFunction::InsertionIndicatorHandler::Start (const Point& rMouseModelPosition)
{
    view::ViewOverlay& rOverlay (mrController.GetView().GetOverlay());
    rOverlay.GetInsertionIndicatorOverlay().SetPosition(rMouseModelPosition);
    rOverlay.GetInsertionIndicatorOverlay().Show();
}




void SelectionFunction::InsertionIndicatorHandler::UpdatePosition (const Point& rMouseModelPosition)
{
    view::ViewOverlay& rOverlay (mrController.GetView().GetOverlay());
    rOverlay.GetInsertionIndicatorOverlay().SetPosition(rMouseModelPosition);
}




void SelectionFunction::InsertionIndicatorHandler::End (void)
{
    mrController.GetView().GetOverlay().GetInsertionIndicatorOverlay().Hide();
}

} } } // end of namespace ::sd::slidesorter::controller
