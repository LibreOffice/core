/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SlsSelectionFunction.cxx,v $
 * $Revision: 1.37 $
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
#include "controller/SlsInsertionIndicatorHandler.hxx"
#include "controller/SlsSelectionManager.hxx"
#include "controller/SlsProperties.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "model/SlsPageEnumerationProvider.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsViewOverlay.hxx"
#include "view/SlsLayouter.hxx"
#include "view/SlsPageObjectLayouter.hxx"
#include "framework/FrameworkHelper.hxx"
#include "showview.hxx"
#include "ViewShellBase.hxx"
#include "DrawController.hxx"
#include "Window.hxx"
#include "sdpage.hxx"
#include "drawdoc.hxx"
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
static const sal_uInt32 OVER_BUTTON              (0x00080000);
static const sal_uInt32 SHIFT_MODIFIER           (0x00100000);
static const sal_uInt32 CONTROL_MODIFIER         (0x00200000);
static const sal_uInt32 SUBSTITUTION_VISIBLE     (0x01000000);
static const sal_uInt32 RECTANGLE_VISIBLE        (0x02000000);

static const sal_uInt32 KEY_EVENT                (0x10000000);

// Some absent events are defined so they can be expressed explicitly.
static const sal_uInt32 NO_MODIFIER              (0x00000000);
static const sal_uInt32 SUBSTITUTION_NOT_VISIBLE (0x00000000);
static const sal_uInt32 NOT_OVER_PAGE            (0x00000000);

// Masks
static const sal_uInt32 MODIFIER_MASK            (SHIFT_MODIFIER | CONTROL_MODIFIER);


} // end of anonymous namespace


namespace sd { namespace slidesorter { namespace controller {

/** A SubstitutionHandler object handles the display of a number of selected
    slides at the mouse position and the insertion or (with or without
    removing the pages at their original position) when the object is
    destoyed.
*/
class SelectionFunction::SubstitutionHandler
{
public:
    /** Create a substitution display of the currently selected pages and
        use the given position as the anchor point.
    */
    SubstitutionHandler (
        SlideSorter& rSlideSorter,
        const model::SharedPageDescriptor& rpHitDescriptor,
        const Point& rMouseModelPosition);
    ~SubstitutionHandler (void);

    /** Call this method (for example as reaction to ESC key press) to avoid
        processing (ie moving or inserting) the substition when the called
        SubstitutionHandler object is destroyed.
    */
    void Dispose (void);

    /** Move the substitution display by the distance the mouse has
        travelled since the last call to this method or to
        CreateSubstitution().  The given point becomes the new anchor.
    */
    void UpdatePosition (
        const Point& rMousePosition,
        const bool bAllowAutoScroll = true);

private:
    SlideSorter& mrSlideSorter;
    model::SharedPageDescriptor mpHitDescriptor;
    sal_Int32 mnInsertionIndex;

    /** Move the substitution display of the currently selected pages.
    */
    void Process (void);
};




class SelectionFunction::MouseMultiSelector
{
public:
    MouseMultiSelector (
        SlideSorter& rSlideSorter,
        const Point& rMouseModelPosition);
    virtual ~MouseMultiSelector (void);

    void RestoreInitialSelection (void);

    /** Update the rectangle selection so that the given position becomes
        the new second point of the selection rectangle.
    */
    void UpdatePosition (
        const Point& rMousePosition,
        const bool bAllowAutoScroll = true);
    enum SelectionMode { SM_Normal, SM_Add, SM_Toggle };

    void SetSelectionMode (const SelectionMode eSelectionMode);
    void SetSelectionModeFromModifier (const sal_uInt32 nEventCode);

protected:
    SlideSorter& mrSlideSorter;
    SelectionMode meSelectionMode;
    ::std::set<sal_Int32> maInitialSelection;

    virtual void UpdateModelPosition (const Point& rMouseModelPosition) = 0;
    virtual void UpdateSelection (void) = 0;

    void UpdateSelectionState (
        const model::SharedPageDescriptor& rpDescriptor,
        const bool bIsInSelection) const;
};

namespace {

    class RectangleSelector
        : public SelectionFunction::MouseMultiSelector
    {
    public:
        /** Start a rectangle selection at the given position.
        */
        RectangleSelector (
            SlideSorter& rSlideSorter,
            const Point& rMouseModelPosition);
        virtual ~RectangleSelector (void);

    protected:
        virtual void UpdateModelPosition (const Point& rMouseModelPosition);

        /** Select all pages that lie completly in the selection rectangle.
        */
        virtual void UpdateSelection (void);
    };

    class RangeSelector
        : public SelectionFunction::MouseMultiSelector
    {
    public:
        /** Start a rectangle selection at the given position.
        */
        RangeSelector (
            SlideSorter& rSlideSorter,
            const Point& rMouseModelPosition);
        virtual ~RangeSelector (void);

    protected:
        virtual void UpdateModelPosition (const Point& rMouseModelPosition);

        /** Select all pages that lie completly in the selection rectangle.
        */
        virtual void UpdateSelection (void);

    private:
        Point maAnchor;
        Point maSecondCorner;
        Pointer maSavedPointer;
        sal_Int32 mnAnchorIndex;
        sal_Int32 mnSecondIndex;

        Rectangle GetBoundingBox (void) const;
    };

} // end of anonymous namespace


class SelectionFunction::EventDescriptor
{
public:

    Point maMousePosition;
    Point maMouseModelPosition;
    ::boost::weak_ptr<model::PageDescriptor> mpHitDescriptor;
    SdrPage* mpHitPage;
    sal_uInt32 mnEventCode;
    sal_Int32 mnButtonIndex;

    EventDescriptor (
        sal_uInt32 nEventType,
        const MouseEvent& rEvent,
        SlideSorter& rSlideSorter);
    EventDescriptor (
        const KeyEvent& rEvent,
        SlideSorter& rSlideSorter);
    EventDescriptor (
        const AcceptDropEvent& rEvent,
        SlideSorter& rSlideSorter);
    EventDescriptor (const EventDescriptor& rDescriptor);
};


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
      mpSubstitutionHandler(),
      mpMouseMultiSelector(),
      mnButtonDownPageIndex(-1),
      mnButtonDownButtonIndex(-1),
      mbIsDeselectionPending(false),
      mnShiftKeySelectionAnchor(-1)
{
    aDragTimer.SetTimeoutHdl(LINK(this, SelectionFunction, DragSlideHdl));
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
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode (rEvent.GetButtons());
    aMDPos = rEvent.GetPosPixel();
    mbProcessingMouseButtonDown = true;

    mpWindow->CaptureMouse();

    ProcessMouseEvent(BUTTON_DOWN, rEvent);

    return TRUE;
}




BOOL SelectionFunction::MouseMove (const MouseEvent& rEvent)
{
    Point aMousePosition (rEvent.GetPosPixel());

    // Allow one mouse move before the drag timer is disabled.
    if (aDragTimer.IsActive())
    {
        if (bFirstMouseMove)
            bFirstMouseMove = FALSE;
        else
            aDragTimer.Stop();
    }


    // In some modes (dragging, moving) the mouse over indicator is only
    // annoying.  Turn it off in these cases.
    if (mpSubstitutionHandler || mpMouseMultiSelector)
    {
        mrSlideSorter.GetView().SetPageUnderMouse(model::SharedPageDescriptor());
    }
    else
    {
        mrSlideSorter.GetView().UpdatePageUnderMouse(
            aMousePosition,
            (rEvent.GetButtons() & MOUSE_LEFT)!=0);
    }

    view::ViewOverlay& rOverlay (mrSlideSorter.GetView().GetOverlay());
    Rectangle aRectangle (Point(0,0),mpWindow->GetOutputSizePixel());
    if ( ! aRectangle.IsInside(aMousePosition)
        && mpSubstitutionHandler)
    {
        // Mouse left the window with pressed left button.  Make it a drag.
        StartDrag(aMousePosition);
        // Mouse motion events are not sent to us while the mouse is outside
        // the window and drag&drop is active.  Therefore hide the
        // substitution.
        //        mpSubstitutionHandler->Hide();
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
    mpWindow->ReleaseMouse();

    return TRUE;
}




void SelectionFunction::MouseDragged (const AcceptDropEvent& rEvent)
{
    // 1. Compute some frequently used values relating to the event.
    ::std::auto_ptr<EventDescriptor> pEventDescriptor (
        new EventDescriptor(rEvent, mrSlideSorter));

    // 2. Detect whether we are dragging pages or dragging a selection rectangle.
    view::ViewOverlay& rOverlay (mrSlideSorter.GetView().GetOverlay());
    if (rOverlay.GetSubstitutionOverlay()->IsVisible())
        pEventDescriptor->mnEventCode |= SUBSTITUTION_VISIBLE;
    if (mpMouseMultiSelector)
        pEventDescriptor->mnEventCode |= RECTANGLE_VISIBLE;

    // 3. Process the event.
    EventPreprocessing(*pEventDescriptor);
    if ( ! EventProcessing(*pEventDescriptor))
    {
        OSL_TRACE ("can not handle event code %x", pEventDescriptor->mnEventCode);
    }
    EventPostprocessing(*pEventDescriptor);
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
            if (mpSubstitutionHandler)
            {
                mpSubstitutionHandler->Dispose();
                mpSubstitutionHandler.reset();
            }
            if (mpMouseMultiSelector)
            {
                mpMouseMultiSelector->RestoreInitialSelection();
                mpMouseMultiSelector.reset();
            }
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
                if (pDescriptor->HasState(model::PageDescriptor::ST_Selected))
                    mrController.GetPageSelector().DeselectPage(pDescriptor);
                else
                    mrController.GetPageSelector().SelectPage(pDescriptor);
            }
            bResult = TRUE;
        }
        break;


        // Move the focus indicator left.
        case KEY_LEFT:
            MoveFocus(FocusManager::FMD_LEFT, rEvent.GetKeyCode().IsShift());
            bResult = TRUE;
            break;

        // Move the focus indicator right.
        case KEY_RIGHT:
            MoveFocus(FocusManager::FMD_RIGHT, rEvent.GetKeyCode().IsShift());
            bResult = TRUE;
            break;

        // Move the focus indicator up.
        case KEY_UP:
            MoveFocus(FocusManager::FMD_UP, rEvent.GetKeyCode().IsShift());
            bResult = TRUE;
            break;

        // Move the focus indicator down.
        case KEY_DOWN:
            MoveFocus(FocusManager::FMD_DOWN, rEvent.GetKeyCode().IsShift());
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
            if (mrSlideSorter.GetProperties()->IsUIReadOnly())
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

            mnShiftKeySelectionAnchor = -1;
            bResult = TRUE;
        }
        break;

        case KEY_F10:
            if (rEvent.GetKeyCode().IsShift())
            {
                DeselectAllPages();
                mrController.GetPageSelector().SelectPage(
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
    const bool bIsShiftDown)
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
    else
        mnShiftKeySelectionAnchor = -1;

    mrController.GetFocusManager().MoveFocus(eDirection);

    // When shift is pressed then select all pages in the range between the
    // currently and the previously focused pages, including them.
    if (bIsShiftDown)
    {
        model::SharedPageDescriptor pFocusedDescriptor (
            mrController.GetFocusManager().GetFocusedPageDescriptor());
        if (pFocusedDescriptor)
        {
            PageSelector& rSelector (mrController.GetPageSelector());
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




void SelectionFunction::StartDragTimer (void)
{
    if ( ! mrSlideSorter.GetProperties()->IsUIReadOnly())
    {
        bFirstMouseMove = TRUE;
        aDragTimer.Start();
    }
}




IMPL_LINK( SelectionFunction, DragSlideHdl, Timer*, EMPTYARG )
{
    StartDrag(aMDPos);
    return 0;
}




void SelectionFunction::StartDrag (const Point& rMousePosition)
{
    if (mbPageHit
        &&  ! mrSlideSorter.GetProperties()->IsUIReadOnly())
    {
        if ( ! mpSubstitutionHandler)
            mpSubstitutionHandler.reset(
                new SubstitutionHandler(
                    mrSlideSorter,
                    mrSlideSorter.GetController().GetPageAt(rMousePosition),
                    rMousePosition));
        else
            mpSubstitutionHandler->UpdatePosition(rMousePosition);
        mbPageHit = false;
        mpWindow->ReleaseMouse();

        if (mrSlideSorter.GetViewShell() != NULL)
        {
            SlideSorterViewShell* pSlideSorterViewShell
                = dynamic_cast<SlideSorterViewShell*>(mrSlideSorter.GetViewShell());
            pSlideSorterViewShell->StartDrag (rMousePosition, mpWindow);
        }
    }
}




bool SelectionFunction::cancel (void)
{
    mrController.GetFocusManager().ToggleFocus();
    return true;
}




void SelectionFunction::DeselectAllPages (void)
{
    mbIsDeselectionPending = false;
    mrController.GetPageSelector().DeselectAllPages();
    mnShiftKeySelectionAnchor = -1;
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
    mnShiftKeySelectionAnchor = -1;
}




void SelectionFunction::ClearOverlays (void)
{
    view::ViewOverlay& rOverlay (mrSlideSorter.GetView().GetOverlay());

    rOverlay.GetSubstitutionOverlay()->SetIsVisible(false);
    rOverlay.GetSubstitutionOverlay()->Clear();

    mrController.GetInsertionIndicatorHandler()->End();
}




void SelectionFunction::ProcessMouseEvent (sal_uInt32 nEventType, const MouseEvent& rEvent)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode (rEvent.GetButtons());

    // 1. Compute some frequently used values relating to the event.
    ::std::auto_ptr<EventDescriptor> pEventDescriptor (
        new EventDescriptor(nEventType, rEvent, mrSlideSorter));

    // 2. Compute a numerical code that describes the event and that is used
    // for fast look-up of the associated reaction.
    pEventDescriptor->mnEventCode
        = EncodeMouseEvent(*pEventDescriptor, rEvent) | EncodeState(*pEventDescriptor);

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

    // Detect pressed modifier keys.
    if (rEvent.IsShift())
        nEventCode |= SHIFT_MODIFIER;
    if (rEvent.IsMod1())
        nEventCode |= CONTROL_MODIFIER;

    // Detect whether the mouse is over one of the active elements inside a
    // page object.
    if (rDescriptor.mnButtonIndex >= 0)
        nEventCode |= OVER_BUTTON;

    return nEventCode;
}



sal_uInt32 SelectionFunction::EncodeKeyEvent (
    const EventDescriptor& rDescriptor,
    const KeyEvent& rEvent) const
{
    (void)rDescriptor;

    // Initialize as key event.
    sal_uInt32 nEventCode (KEY_EVENT);

    // Add the actual key code in the lower 16 bit.
    nEventCode |= rEvent.GetKeyCode().GetCode();

    // Detect pressed modifier keys.
    if (rEvent.GetKeyCode().IsShift())
        nEventCode |= SHIFT_MODIFIER;
    if (rEvent.GetKeyCode().IsMod1())
        nEventCode |= CONTROL_MODIFIER;

    return nEventCode;
}




sal_uInt32 SelectionFunction::EncodeState (
    const EventDescriptor& rDescriptor) const
{
    sal_uInt32 nEventCode (0);

    // Detect whether the event has happened over a page object.
    if (rDescriptor.mpHitPage != NULL && ! rDescriptor.mpHitDescriptor.expired())
    {
        model::SharedPageDescriptor pHitDescriptor (rDescriptor.mpHitDescriptor);
        if (pHitDescriptor->HasState(model::PageDescriptor::ST_Selected))
            nEventCode |= OVER_SELECTED_PAGE;
        else
            nEventCode |= OVER_UNSELECTED_PAGE;

        // Detect whether the mouse is over one of the active elements
        // inside a page object.
        if (rDescriptor.mnButtonIndex >= 0)
            nEventCode |= OVER_BUTTON;
    }

    // Detect whether we are dragging pages or dragging a selection rectangle.
    view::ViewOverlay& rOverlay (mrSlideSorter.GetView().GetOverlay());
    if (mpSubstitutionHandler)
        nEventCode |= SUBSTITUTION_VISIBLE;
    if (mpMouseMultiSelector)
        nEventCode |= RECTANGLE_VISIBLE;

    return nEventCode;
}




void SelectionFunction::ProcessKeyEvent (const KeyEvent& rEvent)
{
    // 1. Compute some frequently used values relating to the event.
    ::std::auto_ptr<EventDescriptor> pEventDescriptor (
        new EventDescriptor(rEvent, mrSlideSorter));

    // 2. Encode the event.
    pEventDescriptor->mnEventCode
        = EncodeKeyEvent(*pEventDescriptor, rEvent) | EncodeState(*pEventDescriptor);

    // 3. Process the event.
    EventPreprocessing(*pEventDescriptor);
    if ( ! EventProcessing(*pEventDescriptor))
        OSL_TRACE ("can not handle event code %x", pEventDescriptor->mnEventCode);
    EventPostprocessing(*pEventDescriptor);
}




void SelectionFunction::EventPreprocessing (const EventDescriptor& rDescriptor)
{
    // Some general processing that is not specific to the exact event code.
    if (rDescriptor.mnEventCode & BUTTON_DOWN)
        mbPageHit = (rDescriptor.mpHitPage!=NULL);

    // Set the focus to the slide under the mouse.
    //  if (rDescriptor.mpHitPage != NULL)
    //
    //      mrController.GetFocusManager().FocusPage((rDescriptor.mpHitPage->GetPageNum()-1)/2);
}




bool Match (
    const sal_uInt32 nEventCode,
    const sal_uInt32 nPositivePattern)
{
    return (nEventCode & nPositivePattern)==nPositivePattern;
}




bool SelectionFunction::EventProcessing (const EventDescriptor& rDescriptor)
{
    // Define some macros to make the following switch statement more readable.
#define ANY_MODIFIER(code)                  \
         code|NO_MODIFIER:                  \
    case code|SHIFT_MODIFIER:               \
    case code|CONTROL_MODIFIER
#define ANY_PAGE(code)                      \
         code|NOT_OVER_PAGE:                \
    case code|OVER_UNSELECTED_PAGE:         \
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

    switch (rDescriptor.mnEventCode & (SUBSTITUTION_VISIBLE | RECTANGLE_VISIBLE))
    {
        case SUBSTITUTION_VISIBLE:
            // The substitution is visible.  Handle events accordingly.
            if (Match(rDescriptor.mnEventCode, MOUSE_MOTION | LEFT_BUTTON | SINGLE_CLICK))
            {
                if ((rDescriptor.mnEventCode & CONTROL_MODIFIER) != 0)
                    StartDrag(rDescriptor.maMousePosition);
                mpSubstitutionHandler->UpdatePosition(rDescriptor.maMousePosition);
            }
            else if (Match(rDescriptor.mnEventCode, MOUSE_DRAG))
            {
                mpSubstitutionHandler->UpdatePosition(rDescriptor.maMousePosition);
            }
            else if (Match(rDescriptor.mnEventCode, BUTTON_UP | LEFT_BUTTON))
            {
                // The following Process() call may lead to the desctruction
                // of pHitDescriptor so release our reference to it.
                pHitDescriptor.reset();
                mpSubstitutionHandler.reset();
            }
            break;

        case RECTANGLE_VISIBLE:
            OSL_ASSERT(mpMouseMultiSelector);
            // The selection rectangle is visible.  Handle events accordingly.
            if (Match(rDescriptor.mnEventCode, MOUSE_MOTION | LEFT_BUTTON | SINGLE_CLICK))
            {
                if (mpMouseMultiSelector)
                {
                    mpMouseMultiSelector->SetSelectionModeFromModifier(rDescriptor.mnEventCode);
                    mpMouseMultiSelector->UpdatePosition(rDescriptor.maMousePosition);
                }
                bMakeSelectionVisible = false;
            }
            else if (Match(rDescriptor.mnEventCode, BUTTON_UP | LEFT_BUTTON | SINGLE_CLICK))
            {
                mpMouseMultiSelector.reset();
            }
            // Anything else stops the rectangle selection and the event is
            // processed again.
            else
            {
                mpMouseMultiSelector.reset();
                EventDescriptor aModifiedDescriptor (rDescriptor);
                aModifiedDescriptor.mnEventCode &= ~RECTANGLE_VISIBLE;
                EventProcessing(aModifiedDescriptor);
            }
            break;

        default:
            OSL_ASSERT(!mpMouseMultiSelector);
            switch (rDescriptor.mnEventCode & (BUTTON_DOWN | BUTTON_UP | MOUSE_MOTION))
            {
                case BUTTON_DOWN:
                    switch (rDescriptor.mnEventCode)
                    {
                        case BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | OVER_UNSELECTED_PAGE:
                            SetCurrentPage(pHitDescriptor);
                            StartDragTimer();
                            break;

                        case BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | OVER_SELECTED_PAGE:
                            StartDragTimer();
                            break;

                        case BUTTON_DOWN | LEFT_BUTTON | DOUBLE_CLICK | OVER_SELECTED_PAGE:
                        case BUTTON_DOWN | LEFT_BUTTON | DOUBLE_CLICK | OVER_UNSELECTED_PAGE:
                            // A double click allways shows the selected slide in the center
                            // pane in an edit view.
                            SetCurrentPage(pHitDescriptor);
                            SwitchView(pHitDescriptor);
                            break;

                        // Range selection with the shift modifier.
                        case BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | OVER_SELECTED_PAGE | SHIFT_MODIFIER:
                        case BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | OVER_UNSELECTED_PAGE | SHIFT_MODIFIER:
                            RangeSelect(pHitDescriptor);
                            break;

                        case BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | OVER_UNSELECTED_PAGE | OVER_BUTTON:
                        case BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | OVER_SELECTED_PAGE | OVER_BUTTON:
                            // Remember page and button index.  When mouse button is
                            // released over same page and button then invoke action of that
                            // button.
                            mnButtonDownButtonIndex = rDescriptor.mnButtonIndex;
                            OSL_ASSERT(pHitDescriptor);
                            mnButtonDownPageIndex = pHitDescriptor->GetPageIndex();
                            pHitDescriptor->GetVisualState().SetActiveButtonState(
                                mnButtonDownButtonIndex,
                                model::VisualState::BS_Pressed);
                            mrSlideSorter.GetView().RequestRepaint(pHitDescriptor);
                            break;

                            // Right button for context menu.
                        case BUTTON_DOWN | RIGHT_BUTTON | SINGLE_CLICK | OVER_UNSELECTED_PAGE:
                            // Single right click and shift+F10 select as preparation to
                            // show the context menu.  Change the selection only when the
                            // page under the mouse is not selected.  In this case the
                            // selection is set to this single page.  Otherwise the
                            // selection is not modified.
                            DeselectAllPages();
                            SetCurrentPage(pHitDescriptor);
                            bMakeSelectionVisible = false;
                            break;

                        case BUTTON_DOWN | RIGHT_BUTTON | SINGLE_CLICK | OVER_SELECTED_PAGE:
                            // Do not change the selection.  Just adjust the insertion indicator.
                            bMakeSelectionVisible = false;
                            break;
                        case BUTTON_DOWN | RIGHT_BUTTON | SINGLE_CLICK | NOT_OVER_PAGE:

                        // Rectangle selection.
                        case ANY_MODIFIER(BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | NOT_OVER_PAGE):
                            mbIsDeselectionPending = true;
                            OSL_ASSERT(!mpMouseMultiSelector);
                            break;
                    }
                    break;

                case BUTTON_UP:
                    switch (rDescriptor.mnEventCode)
                    {
                        case BUTTON_UP | LEFT_BUTTON | SINGLE_CLICK | OVER_SELECTED_PAGE:
                            SetCurrentPage(pHitDescriptor);
                            break;

                            // Multi selection with the control modifier.
                        case BUTTON_UP | LEFT_BUTTON | SINGLE_CLICK | OVER_SELECTED_PAGE | CONTROL_MODIFIER:
                            mrController.GetPageSelector().DeselectPage(pHitDescriptor);
                            break;

                        case BUTTON_UP | LEFT_BUTTON | SINGLE_CLICK | OVER_UNSELECTED_PAGE | CONTROL_MODIFIER:
                            mrController.GetPageSelector().SelectPage(pHitDescriptor);
                            // fallthrough
                        case BUTTON_UP | LEFT_BUTTON | SINGLE_CLICK | OVER_UNSELECTED_PAGE | OVER_BUTTON:
                        case BUTTON_UP | LEFT_BUTTON | SINGLE_CLICK | OVER_SELECTED_PAGE | OVER_BUTTON:
                            if (mnButtonDownButtonIndex == rDescriptor.mnButtonIndex
                                && mnButtonDownPageIndex == pHitDescriptor->GetPageIndex())
                            {
                                ProcessButtonClick(pHitDescriptor, mnButtonDownButtonIndex);
                            }
                            break;
                        case BUTTON_UP | LEFT_BUTTON | SINGLE_CLICK | NOT_OVER_PAGE:
                            if (mbIsDeselectionPending)
                                DeselectAllPages();
                            break;
                    }
                    break;

                case MOUSE_MOTION:
                    switch (rDescriptor.mnEventCode)
                    {
                        // A mouse motion without visible substitution starts that.
                        case ANY_MODIFIER(MOUSE_MOTION | LEFT_BUTTON | SINGLE_CLICK | OVER_SELECTED_PAGE):
                            mpSubstitutionHandler.reset(new SubstitutionHandler(
                                mrSlideSorter,
                                pHitDescriptor,
                                rDescriptor.maMouseModelPosition));
                            break;

                            // A mouse motion not over a page starts a rectangle selection.
                        case ANY_MODIFIER(MOUSE_MOTION | LEFT_BUTTON | SINGLE_CLICK | NOT_OVER_PAGE):
                            OSL_ASSERT(!mpMouseMultiSelector);
                            mpMouseMultiSelector.reset(
                                new RangeSelector(mrSlideSorter, rDescriptor.maMouseModelPosition));
                            mpMouseMultiSelector->SetSelectionModeFromModifier(rDescriptor.mnEventCode);
                            break;
                    }
                    break;
            }
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
                || !rOverlay.GetInsertionIndicatorOverlay()->IsVisible(),
            "slidesorter::SelectionFunction: insertion indicator still visible");
        DBG_ASSERT(
            !rOverlay.GetSubstitutionOverlay()->IsVisible(),
            "slidesorter::SelectionFunction: substitution still visible");
        DBG_ASSERT(
            !rOverlay.GetSelectionRectangleOverlay()->IsVisible(),
            "slidesorter::SelectionFunction: selection rectangle still visible");

        // Now turn them off.
        if ( ! mrController.IsContextMenuOpen())
            rOverlay.GetInsertionIndicatorOverlay()->SetIsVisible(false);
        rOverlay.GetSubstitutionOverlay()->SetIsVisible(false);
        rOverlay.GetSelectionRectangleOverlay()->SetIsVisible(false);

        mnButtonDownPageIndex = -1;
        mnButtonDownButtonIndex = -1;
    }
}




void SelectionFunction::SetCurrentPage (const model::SharedPageDescriptor& rpDescriptor)
{
    PageSelector& rSelector (mrController.GetPageSelector());
    rSelector.DeselectAllPages ();
    rSelector.SelectPage(rpDescriptor);

    mrController.GetCurrentSlideManager()->SwitchCurrentSlide(rpDescriptor);
    mrController.GetFocusManager().SetFocusedPage(rpDescriptor);

    mnShiftKeySelectionAnchor = -1;
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




void SelectionFunction::ProcessButtonClick (
    const model::SharedPageDescriptor& rpDescriptor,
    const sal_Int32 nButtonIndex)
{
    OSL_ASSERT(rpDescriptor);

    PageSelector& rSelector (mrSlideSorter.GetController().GetPageSelector());
    switch (nButtonIndex)
    {
        case 2:
            // Start slide show at current slide.
            mrSlideSorter.GetController().GetCurrentSlideManager()->SwitchCurrentSlide(
                rpDescriptor);
            if (mrSlideSorter.GetViewShell() != NULL)
                mrSlideSorter.GetViewShell()->GetDispatcher()->Execute(
                    SID_PRESENTATION,
                    SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
            break;

        case 1:
            // Toggle exclusion state.
            rSelector.DeselectAllPages();
            rSelector.SelectPage(rpDescriptor);
            if (mrSlideSorter.GetViewShell() != NULL)
                mrSlideSorter.GetViewShell()->GetDispatcher()->Execute(
                    rpDescriptor->HasState(model::PageDescriptor::ST_Excluded)
                        ? SID_SHOW_SLIDE
                        : SID_HIDE_SLIDE,
                    SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
            break;

        case 0:
            // Insert page after current page.
            rSelector.DeselectAllPages();
            rSelector.SelectPage(rpDescriptor);
            if (mrSlideSorter.GetViewShell() != NULL)
                mrSlideSorter.GetViewShell()->GetDispatcher()->Execute(
                    SID_INSERTPAGE,
                    SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
            break;
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
      mnEventCode(nEventType),
      mnButtonIndex(-1)
{
    SharedSdWindow pWindow (rSlideSorter.GetContentWindow());

    maMousePosition = rEvent.GetPosPixel();
    maMouseModelPosition = pWindow->PixelToLogic(maMousePosition);
    model::SharedPageDescriptor pHitDescriptor (
        rSlideSorter.GetController().GetPageAt(maMousePosition));
    if (pHitDescriptor.get() != NULL)
    {
        mpHitDescriptor = pHitDescriptor;
        mpHitPage = pHitDescriptor->GetPage();

        mnButtonIndex = rSlideSorter.GetView().GetLayouter()
            .GetPageObjectLayouter()->GetButtonIndexAt(
                pHitDescriptor,
                maMouseModelPosition);
    }
}





SelectionFunction::EventDescriptor::EventDescriptor (
    const KeyEvent&,
    SlideSorter& rSlideSorter)
    : maMousePosition(),
      maMouseModelPosition(),
      mpHitDescriptor(),
      mpHitPage(),
      mnEventCode(0),
      mnButtonIndex(-1)
{
    model::SharedPageDescriptor pHitDescriptor (
        rSlideSorter.GetController().GetFocusManager().GetFocusedPageDescriptor());
    if (pHitDescriptor.get() != NULL)
    {
        mpHitPage = pHitDescriptor->GetPage();
        mpHitDescriptor = pHitDescriptor;
    }
}





SelectionFunction::EventDescriptor::EventDescriptor (
    const AcceptDropEvent& rEvent,
    SlideSorter& rSlideSorter)
    : maMousePosition(rEvent.maPosPixel),
      maMouseModelPosition(),
      mpHitDescriptor(),
      mpHitPage(),
      mnEventCode(MOUSE_DRAG),
      mnButtonIndex(-1)
{
    SharedSdWindow pWindow (rSlideSorter.GetContentWindow());

    maMouseModelPosition = pWindow->PixelToLogic(maMousePosition);
    model::SharedPageDescriptor pHitDescriptor (
        rSlideSorter.GetController().GetPageAt(maMousePosition));
    if (pHitDescriptor.get() != NULL)
    {
        mpHitDescriptor = pHitDescriptor;
        mpHitPage = pHitDescriptor->GetPage();
    }
}




SelectionFunction::EventDescriptor::EventDescriptor (const EventDescriptor& rDescriptor)
    : maMousePosition(rDescriptor.maMousePosition),
      maMouseModelPosition(rDescriptor.maMouseModelPosition),
      mpHitDescriptor(rDescriptor.mpHitDescriptor),
      mpHitPage(rDescriptor.mpHitPage),
      mnEventCode(rDescriptor.mnEventCode),
      mnButtonIndex(rDescriptor.mnButtonIndex)
{
}




//===== SubstitutionHandler ===================================================

SelectionFunction::SubstitutionHandler::SubstitutionHandler (
    SlideSorter& rSlideSorter,
    const model::SharedPageDescriptor& rpHitDescriptor,
    const Point& rMouseModelPosition)
    : mrSlideSorter(rSlideSorter),
      mpHitDescriptor(rpHitDescriptor),
      mnInsertionIndex(-1)
{
    mnInsertionIndex = -1;

    // No Drag-and-Drop for master pages.
    if (mrSlideSorter.GetModel().GetEditMode() != EM_PAGE)
        return;

    if (mrSlideSorter.GetProperties()->IsUIReadOnly())
        return;

    view::ViewOverlay& rOverlay (mrSlideSorter.GetView().GetOverlay());

    if ( ! rOverlay.GetSubstitutionOverlay()->IsVisible())
    {
        // Show a new substitution for the selected page objects.
        model::PageEnumeration aSelectedPages(
            model::PageEnumerationProvider::CreateSelectedPagesEnumeration(
                mrSlideSorter.GetModel()));
        rOverlay.GetSubstitutionOverlay()->Create(
            aSelectedPages,
            rMouseModelPosition,
            mpHitDescriptor);
        rOverlay.GetSubstitutionOverlay()->SetIsVisible(true);
        mrSlideSorter.GetController().GetInsertionIndicatorHandler()->Start(rMouseModelPosition);
    }
}




SelectionFunction::SubstitutionHandler::~SubstitutionHandler (void)
{
    Process();

    view::ViewOverlay& rOverlay (mrSlideSorter.GetView().GetOverlay());
    rOverlay.GetSubstitutionOverlay()->SetIsVisible(false);
    rOverlay.GetSubstitutionOverlay()->Clear();
    mrSlideSorter.GetController().GetInsertionIndicatorHandler()->End();
    mpHitDescriptor.reset();
}




void SelectionFunction::SubstitutionHandler::Dispose (void)
{
    mnInsertionIndex = -1;
}




void SelectionFunction::SubstitutionHandler::UpdatePosition (
    const Point& rMousePosition,
    const bool bAllowAutoScroll)
{
    if (mrSlideSorter.GetProperties()->IsUIReadOnly())
        return;

    // Convert window coordinates into model coordinates (we need the
    // window coordinates for auto-scrolling because that remains
    // constant while scrolling.)
    SharedSdWindow pWindow (mrSlideSorter.GetContentWindow());
    const Point aMouseModelPosition (pWindow->PixelToLogic(rMousePosition));

    if ( ! (bAllowAutoScroll && mrSlideSorter.GetController().GetScrollBarManager().AutoScroll(
        rMousePosition,
        ::boost::bind(
            &SelectionFunction::SubstitutionHandler::UpdatePosition,
            this,
            rMousePosition,
            false))))
    {
        view::ViewOverlay& rOverlay (mrSlideSorter.GetView().GetOverlay());

        // Move the existing substitution to the new position.
        rOverlay.GetSubstitutionOverlay()->SetPosition(aMouseModelPosition);

        mrSlideSorter.GetController().GetInsertionIndicatorHandler()->UpdatePosition(
            aMouseModelPosition);

        // Remember the new insertion index.
        if (mrSlideSorter.GetController().GetInsertionIndicatorHandler()->IsInsertionTrivial())
            mnInsertionIndex = -1;
        else
            mnInsertionIndex = mrSlideSorter.GetController().GetInsertionIndicatorHandler()
                ->GetInsertionPageIndex();
    }
}




void SelectionFunction::SubstitutionHandler::Process (void)
{
    if (mrSlideSorter.GetProperties()->IsUIReadOnly())
        return;

    if (mnInsertionIndex >= 0)
    {
        // Tell the model to move the selected pages behind the one with the
        // index mnInsertionIndex which first has to transformed into an index
        // understandable by the document.
        USHORT nDocumentIndex = (USHORT)mnInsertionIndex-1;
        mrSlideSorter.GetController().GetSelectionManager()->MoveSelectedPages(nDocumentIndex);

        ViewShell* pViewShell = mrSlideSorter.GetViewShell();
        if (pViewShell != NULL)
            pViewShell->GetViewFrame()->GetBindings().Invalidate(SID_STATUS_PAGE);
    }
}




//===== SelectionFunction::MouseMultiSelector ======================================

SelectionFunction::MouseMultiSelector::MouseMultiSelector (
    SlideSorter& rSlideSorter,
    const Point& rMouseModelPosition)
    : mrSlideSorter(rSlideSorter),
      meSelectionMode(SM_Normal),
      maInitialSelection()
{
    // Remember the current selection.
    model::PageEnumeration aPages (
        model::PageEnumerationProvider::CreateAllPagesEnumeration(
            mrSlideSorter.GetModel()));
    while (aPages.HasMoreElements())
    {
        model::SharedPageDescriptor pDescriptor (aPages.GetNextElement());
        pDescriptor->SetState(
            model::PageDescriptor::ST_WasSelected,
            pDescriptor->HasState(model::PageDescriptor::ST_Selected));
    }
}




SelectionFunction::MouseMultiSelector::~MouseMultiSelector (void)
{
}




void SelectionFunction::MouseMultiSelector::RestoreInitialSelection (void)
{
    // Remember the current selection.
    model::PageEnumeration aPages (
        model::PageEnumerationProvider::CreateAllPagesEnumeration(
            mrSlideSorter.GetModel()));
    view::SlideSorterView& rView (mrSlideSorter.GetView());
    while (aPages.HasMoreElements())
    {
        model::SharedPageDescriptor pDescriptor (aPages.GetNextElement());
        pDescriptor->SetState(
            model::PageDescriptor::ST_Selected,
            pDescriptor->HasState(model::PageDescriptor::ST_WasSelected));
        rView.RequestRepaint(pDescriptor);
    }
}




void SelectionFunction::MouseMultiSelector::UpdatePosition (
    const Point& rMousePosition,
    const bool bAllowAutoScroll)
{
    // Convert window coordinates into model coordinates (we need the
    // window coordinates for auto-scrolling because that remains
    // constant while scrolling.)
    SharedSdWindow pWindow (mrSlideSorter.GetContentWindow());
    const Point aMouseModelPosition (pWindow->PixelToLogic(rMousePosition));

    if ( ! (bAllowAutoScroll && mrSlideSorter.GetController().GetScrollBarManager().AutoScroll(
        rMousePosition,
        ::boost::bind(
            &SelectionFunction::MouseMultiSelector::UpdatePosition,
            this,
            rMousePosition,
            false))))
    {
        UpdateModelPosition(aMouseModelPosition);
    }
}




void SelectionFunction::MouseMultiSelector::SetSelectionModeFromModifier (
    const sal_uInt32 nEventCode)
{
    switch (nEventCode & MODIFIER_MASK)
    {
        case NO_MODIFIER:
            SetSelectionMode(RectangleSelector::SM_Normal);
            break;

        case SHIFT_MODIFIER:
            SetSelectionMode(RectangleSelector::SM_Add);
            break;

        case CONTROL_MODIFIER:
            SetSelectionMode(RectangleSelector::SM_Toggle);
            break;
    }
}




void SelectionFunction::MouseMultiSelector::SetSelectionMode (const SelectionMode eSelectionMode)
{
    if (meSelectionMode != eSelectionMode)
    {
        meSelectionMode = eSelectionMode;
        UpdateSelection();
    }
}




void SelectionFunction::MouseMultiSelector::UpdateSelectionState (
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




//===== RectangleSelector =====================================================

namespace {

RectangleSelector::RectangleSelector (
    SlideSorter& rSlideSorter,
    const Point& rMouseModelPosition)
    : MouseMultiSelector(rSlideSorter,rMouseModelPosition)
{
    if (mrSlideSorter.GetProperties()->IsShowSelection())
    {
        mrSlideSorter.GetView().GetOverlay().GetSelectionRectangleOverlay()
            ->Start(rMouseModelPosition);
    }
}




RectangleSelector::~RectangleSelector (void)
{
    if (mrSlideSorter.GetProperties()->IsShowSelection())
    {
        mrSlideSorter.GetView().GetOverlay().GetSelectionRectangleOverlay()->SetIsVisible(false);
    }
}




void RectangleSelector::UpdateModelPosition (const Point& rMouseModelPosition)
{
    if (mrSlideSorter.GetProperties()->IsShowSelection())
    {
        mrSlideSorter.GetView().GetOverlay().GetSelectionRectangleOverlay()
            ->Update(rMouseModelPosition);
        UpdateSelection();
    }
}




void RectangleSelector::UpdateSelection (void)
{
    if ( ! mrSlideSorter.GetProperties()->IsShowSelection())
        return;

    view::ViewOverlay& rOverlay (mrSlideSorter.GetView().GetOverlay());
    if (rOverlay.GetSelectionRectangleOverlay()->IsVisible())
    {
        view::SlideSorterView::DrawLock aLock (mrSlideSorter);

        // Select all pages whose page object lies completly or partially
        // inside the selection rectangle.
        const Rectangle& rSelectionRectangle (
            rOverlay.GetSelectionRectangleOverlay()->GetSelectionRectangle());
        model::PageEnumeration aPages (
            model::PageEnumerationProvider::CreateAllPagesEnumeration(
                mrSlideSorter.GetModel()));
        while (aPages.HasMoreElements())
        {
            model::SharedPageDescriptor pDescriptor (aPages.GetNextElement());

            // Determine whether the page object is inside the selection rectangle.
            Rectangle aPageBox (
                mrSlideSorter.GetView().GetLayouter().GetPageObjectLayouter()->GetBoundingBox(
                    pDescriptor,
                    view::PageObjectLayouter::Preview,
                    view::PageObjectLayouter::WindowCoordinateSystem));
            const bool bIsPageInSelectionRectangle (rSelectionRectangle.IsOver(aPageBox));

            UpdateSelectionState(pDescriptor, bIsPageInSelectionRectangle);
        }

        // Rely on auto scrolling to make page objects visible.
        mrSlideSorter.GetController().GetSelectionManager()->ResetMakeSelectionVisiblePending();
    }
}

} // end of anonymous namespace




//===== RangeSelector =========================================================

namespace {

RangeSelector::RangeSelector (
    SlideSorter& rSlideSorter,
    const Point& rMouseModelPosition)
    : MouseMultiSelector(rSlideSorter,rMouseModelPosition),
      maAnchor(rMouseModelPosition),
      maSecondCorner(rMouseModelPosition),
      maSavedPointer(mrSlideSorter.GetContentWindow()->GetPointer()),
      mnAnchorIndex(-1)
{
    const Pointer aSelectionPointer (POINTER_TEXT);
    mrSlideSorter.GetContentWindow()->SetPointer(aSelectionPointer);
}




RangeSelector::~RangeSelector (void)
{
    mrSlideSorter.GetContentWindow()->SetPointer(maSavedPointer);
}




void RangeSelector::UpdateModelPosition (const Point& rMouseModelPosition)
{
    maSecondCorner = rMouseModelPosition;
    UpdateSelection();
}




void RangeSelector::UpdateSelection (void)
{
    view::SlideSorterView::DrawLock aLock (mrSlideSorter);

    PageSelector& rSelector (mrSlideSorter.GetController().GetPageSelector());

    const sal_Int32 nIndexUnderMouse (
        mrSlideSorter.GetView().GetLayouter().GetIndexAtPoint (
            maSecondCorner,
            false));
    if (nIndexUnderMouse >= 0)
    {
        if (mnAnchorIndex < 0)
            mnAnchorIndex = nIndexUnderMouse;
        mnSecondIndex = nIndexUnderMouse;

        Range aRange (mnAnchorIndex, mnSecondIndex);
        aRange.Justify();

        model::SlideSorterModel& rModel (mrSlideSorter.GetModel());
        for (sal_Int32 nIndex=0,nCount(rModel.GetPageCount()); nIndex<nCount; ++nIndex)
        {
            model::SharedPageDescriptor pDescriptor (rModel.GetPageDescriptor(nIndex));

            // Determine whether the page was selected before the rectangle
            // selection was started.
            const bool bWasSelected (pDescriptor->HasState(model::PageDescriptor::ST_WasSelected));

            UpdateSelectionState(pDescriptor, aRange.IsInside(nIndex));
        }

        // Rely on auto scrolling to make page objects visible.
        mrSlideSorter.GetController().GetSelectionManager()->ResetMakeSelectionVisiblePending();
    }
}




Rectangle RangeSelector::GetBoundingBox (void) const
{
    return Rectangle(
        ::std::min(maAnchor.X(), maSecondCorner.X()),
        ::std::min(maAnchor.Y(), maSecondCorner.Y()),
        ::std::max(maAnchor.X(), maSecondCorner.X()),
        ::std::max(maAnchor.Y(), maSecondCorner.Y()));
}


} // end of anonymous namespace


} } } // end of namespace ::sd::slidesorter::controller
