/*************************************************************************
 *
 *  $RCSfile: SlsSelectionFunction.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: kz $ $Date: 2004-11-27 14:32:21 $
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
#ifndef SD_FADER_HXX
#include "fader.hxx"
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
static const sal_uInt32 OVER_SELECTED_PAGE       (0x00001000);
static const sal_uInt32 OVER_UNSELECTED_PAGE     (0x00002000);
static const sal_uInt32 OVER_FADE_INDICATOR      (0x00004000);
static const sal_uInt32 SHIFT_MODIFIER           (0x00010000);
static const sal_uInt32 CONTROL_MODIFIER         (0x00020000);
static const sal_uInt32 SUBSTITUTION_VISIBLE     (0x00100000);
static const sal_uInt32 RECTANGLE_VISIBLE        (0x00200000);
static const sal_uInt32 PAGE_SELECTED            (0x01000000);

// Some absent events are defined so they can be expressed explicitly.
static const sal_uInt32 NO_MODIFIER              (0x00000000);
static const sal_uInt32 SUBSTITUTION_NOT_VISIBLE (0x00000000);
static const sal_uInt32 NOT_OVER_PAGE            (0x00000000);
}


namespace sd { namespace slidesorter { namespace controller {

class SelectionFunction::ShowingEffectInfo
{
public:
    ShowingEffectInfo (bool bShowing)
        : mbIsShowingEffect (bShowing),
          mbDisposed (false)
    {}
    /// true while we show a fade effect.
    bool mbIsShowingEffect;
    // true if the SelectionFunction object that initiated the showing of an
    // effect was deleted during fade effect.
    bool mbDisposed;
};


TYPEINIT1(SelectionFunction, FuPoor);


SelectionFunction::SelectionFunction (
    SlideSorterController& rController,
    SfxRequest& rRequest)
    : SlideFunction (
        rController,
        rRequest),
      mrController (rController),
      mbPageHit(false),
      mbRectangleSelection(false),
      mpRangeSelectionAnchor(NULL),
      mbDragSelection(false),
      maInsertionMarkerBox(),
      mpSound(new Sound),
      mpShowingEffectInfo(new ShowingEffectInfo (false))
{
    //af    aDelayToScrollTimer.SetTimeout(50);
    aDragTimer.SetTimeoutHdl( LINK( this, SelectionFunction, DragSlideHdl ) );
}




SelectionFunction::~SelectionFunction (void)
{
    aDragTimer.Stop();
    delete mpSound;

    if (mpShowingEffectInfo!=NULL
        && mpShowingEffectInfo->mbIsShowingEffect)
    {
        // we can not delete the info because its
        // used in ShowFade(), but mark it as
        // dispose so ShowFade() will clear it
        mpShowingEffectInfo->mbDisposed = TRUE;
    }
    else
    {
        delete mpShowingEffectInfo;
    }
}




BOOL SelectionFunction::MouseButtonDown (const MouseEvent& rEvent)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode (rEvent.GetButtons());

    pWindow->CaptureMouse();

    ProcessMouseEvent(BUTTON_DOWN, rEvent);

    return TRUE;
}




BOOL SelectionFunction::MouseMove (const MouseEvent& rEvent)
{
    Point aMousePosition (rEvent.GetPosPixel());

    // Determine page under mouse and show the mouse over effect.
    model::PageDescriptor* pHitDescriptor
        = mrController.GetPageAt (aMousePosition);
    view::ViewOverlay& rOverlay (mrController.GetView().GetOverlay());
    rOverlay.GetMouseOverIndicatorOverlay().SetSlideUnderMouse (
        pHitDescriptor==NULL ? NULL : pHitDescriptor);
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
        if (rEvent.GetButtons() != 0)
            ProcessMouseEvent (MOUSE_MOTION, rEvent);
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

    ProcessMouseEvent (BUTTON_UP, rEvent);

    mrController.GetView().GetOverlay().GetSelectionRectangleOverlay().Hide();
    //    mrController.GetView().EndEncirclement();

    mbPageHit = false;                // ab jetzt keine Seite mehr "am Haken"
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
                model::PageDescriptor* pDescriptor =
                    rFocusManager.GetFocusedPageDescriptor();
                if (pDescriptor != NULL)
                    SetCurrentPageAndSwitchView(*pDescriptor);
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
            bool bHasObjects = false;;
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
                model::PageDescriptor& rDescriptor (
                    aSelectedPages.GetNextElement());
                if (rDescriptor.GetPage()->GetObjCount() > 0)
                    bHasObjects = true;
            }

            if (nSelectedPagesCount > 0)
            {
                String aString (SdResId (
                    nSelectedPagesCount == 1
                    ? STR_WARN_DEL_SEL_PAGE
                    : STR_WARN_DEL_SEL_PAGES));

                if ( ! bHasObjects
                    || QueryBox (pWindow, WB_YES_NO, aString).Execute()
                       == RET_YES)
                {
                    mrController.DeleteSelectedPages();
                }
            }

            bResult = TRUE;
        }
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
    // Sound nicht mehr blockieren, damit er in der Show gespielt werden kann
    if (mpSound != NULL)
        mpSound->Stop();

    FuPoor::Deactivate();
}




void SelectionFunction::ShowEffect (model::PageDescriptor& rDescriptor)
{
    DBG_ERROR("not implemented");
/*
    ShowingEffectInfo* pInfo = mpShowingEffectInfo;

    if (pInfo != NULL)
        pInfo->mbIsShowingEffect = true;

    SdPage* pPage = rDescriptor.GetPage();
    Rectangle aPageBox (mrController.GetView().GetPageBoundingBox (
        rDescriptor,
        view::SlideSorterView::CS_MODEL,
        view::SlideSorterView::BBT_SHAPE));
    Size aPageSize (aPageBox.GetSize());
    Point aPagePos (aPageBox.TopLeft());

    // korrigierte Seitengroesse, sonst kommt die letzte Pixelreihe(spalte)
    // nicht mit
    Size aCPageSize(aPageSize);
    Size aPixelSize = pWindow->PixelToLogic(Size(1,1));
    aCPageSize.Width()  += aPixelSize.Width();
    aCPageSize.Height() += aPixelSize.Height();


    // virtuelle Devices anlegen
    MapMode aMapMode = pWindow->GetMapMode();
    aMapMode.SetOrigin(Point(0,0));

    VirtualDevice* pVDev = new VirtualDevice(*pWindow);
    pVDev->SetMapMode(aMapMode);
    pVDev->SetOutputSize(aCPageSize);

    VirtualDevice* pVDevOld = new VirtualDevice(*pWindow);
    pVDevOld->SetMapMode(aMapMode);
    pVDevOld->SetOutputSize(aCPageSize);

    // Views fuer die virtuellen Devices
    ShowView aShowView(pDoc, pVDev, pViewShell);
    aShowView.SetAllowMasterPageCaching(FALSE);
    aShowView.SetLineDraft(pView->IsLineDraft());
    aShowView.SetFillDraft(pView->IsFillDraft());
    aShowView.SetTextDraft(pView->IsTextDraft());
    aShowView.SetGrafDraft(pView->IsGrafDraft());

    ShowView aShowViewOld(pDoc, pVDevOld, pViewShell);
    aShowViewOld.SetAllowMasterPageCaching(FALSE);
    aShowViewOld.SetLineDraft(pView->IsLineDraft());
    aShowViewOld.SetFillDraft(pView->IsFillDraft());
    aShowViewOld.SetTextDraft(pView->IsTextDraft());
    aShowViewOld.SetGrafDraft(pView->IsGrafDraft());

    // ein Fader zum Ueberblenden
    Fader* pFader = new Fader(pWindow);
    pFader->SetEffect(pPage->GetFadeEffect());
    pFader->SetSpeed(pPage->GetFadeSpeed());
    pFader->SetSource(Rectangle(Point(), aPageSize));
    pFader->SetTarget(Rectangle(aPagePos, aPageSize));

    // Bitmaps in virtuellen Devices bereitstellen
    long nWinZoom = pWindow->GetZoom();
    long nZoom;

    if (pPage->GetPageNum() < 3)
    {
        Rectangle aRect(Point(0,0), aCPageSize);
        pVDevOld->SetFillColor( Color( COL_BLACK ) );
        pVDevOld->DrawRect(aRect);
    }
    else
    {
        // Vorgaengerdia bestimmen
        short nPrevPage = (pPage->GetPageNum()-1)/2 - 1;

        nZoom = nWinZoom;
        GraphicObject* pOld = NULL;//const_cast< GraphicObject* >(
            //pSlView->GetFromCache( pDoc->GetSdPage(nPrevPage, PK_STANDARD), nZoom, -10 ) );
        if( !pOld )
        {
            SdPage* pPrevPage = pDoc->GetSdPage(nPrevPage, PK_STANDARD);
            aShowViewOld.ShowPage(pPrevPage, Point(0,0));

            SdrPageView* pPageView = aShowViewOld.GetPageView(pPrevPage);
            FrameView* pFrameView = pViewShell->GetFrameView();
            if (pPageView != NULL)
            {
                pPageView->SetVisibleLayers( pFrameView->GetVisibleLayers() );
                pPageView->SetLockedLayers( pFrameView->GetLockedLayers() );
                pPageView->SetPrintableLayers( pFrameView->GetPrintableLayers() );
            }
            aShowViewOld.CompleteRedraw(pVDevOld, Region(Rectangle(Point(), aCPageSize)));
            // #111097# IsRedrawReady() always gives sal_True while (!aShowViewOld.IsRedrawReady()){}
        }
        else
            pOld->Draw( pVDevOld, Point(), aCPageSize );
    }

    nZoom = nWinZoom;
    GraphicObject* pNew = NULL;//const_cast< GraphicObject* >( pSlView->GetFromCache( pDoc->GetSdPage(nPageNo, PK_STANDARD), nZoom, -10 ) );

    int nPageNo = (pPage->GetPageNum()-1)/2;
    if( !pNew )
    {
        SdPage* pPage = pDoc->GetSdPage(nPageNo, PK_STANDARD);
        aShowView.ShowPage(pPage, Point(0,0));

        SdrPageView* pPageView = aShowView.GetPageView(pPage);
        FrameView* pFrameView = pViewShell->GetFrameView();
        if (pPageView != NULL)
        {
            pPageView->SetVisibleLayers( pFrameView->GetVisibleLayers() );
            pPageView->SetLockedLayers( pFrameView->GetLockedLayers() );
            pPageView->SetPrintableLayers( pFrameView->GetPrintableLayers() );
        }

        aShowView.CompleteRedraw(pVDev, Region(Rectangle(Point(0,0), aCPageSize)));
        // #111097# IsRedrawReady() always gives sal_True while (!aShowView.IsRedrawReady()){}
    }
    else
        pNew->Draw( pVDev, Point(), aCPageSize );

    // virtuelle Devices an Fader uebergeben
    pFader->SetOldVirtualDevice(pVDevOld);
    pFader->SetNewVirtualDevice(pVDev);

    // Sound beim Ueberblenden
    if (pPage->IsSoundOn())
    {
        String aSoundFile(pPage->GetSoundFile());
        if (aSoundFile.Len() > 0)
        {
            mpSound->Stop();
            mpSound->SetSoundName(aSoundFile);
            mpSound->Play();
        }
    }

    // ueberblenden
    pFader->Fade();

    delete pFader;
    delete pVDev;
    delete pVDevOld;

    // check IsShowingEffectInfo if we were disposed
    // inside pFader->Fade()
    if (pInfo && !pInfo->mbDisposed )
    {
        pInfo->mbIsShowingEffect = false;
    }
    else
    {
        // were already dead, so kill the info and quit silently
        delete pInfo;
    }
    */
}




bool SelectionFunction::IsShowingEffect (void) const
{
    return mpShowingEffectInfo!= NULL
        && mpShowingEffectInfo->mbIsShowingEffect;
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
        model::PageEnumeration aSelectedPages (
            mrController.GetModel().GetSelectedPagesEnumeration());
        rOverlay.GetSubstitutionOverlay().Create (
            aSelectedPages,
            rOverlay.GetSubstitutionOverlay().GetPosition());
        rOverlay.GetSubstitutionOverlay().Show ();
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




void SelectionFunction::SetCurrentPageAndSwitchView (
    model::PageDescriptor& rDescriptor)
{
    PageSelector& rSelector (mrController.GetPageSelector());

    rSelector.DeselectAllPages ();
    rSelector.SelectPage (rDescriptor);
    rSelector.SetCurrentPage (rDescriptor);

    // Switch to the draw view.  This is done only when the current
    // view is the main view.
    if (mrController.GetViewShell().IsMainViewShell())
    {
        mrController.GetModel().GetDocument()->SetSelected (
            rDescriptor.GetPage(), TRUE);
        pViewShell->GetFrameView()->SetSelectedPage(
            (rDescriptor.GetPage()->GetPageNum()-1)/2);
        mrController.GetViewShell().GetViewShellBase().GetPaneManager()
            .RequestMainViewShellChange (ViewShell::ST_IMPRESS);
    }
}

#define ANY_MODIFIER(code) code|NO_MODIFIER:case code|SHIFT_MODIFIER: case code|CONTROL_MODIFIER

/** This method consists of two parts:
    1. Create a numeric code that describes the relevant information of the
    event.
    2. Use this code in a switch statement to select the operation for
    handling the event.
*/
void SelectionFunction::ProcessMouseEvent (sal_uInt32 nEventType, const MouseEvent& rEvent)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode (rEvent.GetButtons());

    // Prepare some frequently used data.
    Point aMousePosition (rEvent.GetPosPixel());
    Point aMouseModelPosition (pWindow->PixelToLogic (aMousePosition));
    model::PageDescriptor* pHitDescriptor = mrController.GetPageAt (aMousePosition);
    SdPage* pHitPage = (pHitDescriptor != NULL)
        ? pHitDescriptor->GetPage()
        : NULL;

    // 1. Create a code from various types of information about the given mouse
    // event.
    sal_uInt32 nEventCode (nEventType);
    switch (rEvent.GetButtons())
    {
        case MOUSE_LEFT:   nEventCode |= LEFT_BUTTON; break;
        case MOUSE_RIGHT:  nEventCode |= RIGHT_BUTTON; break;
        case MOUSE_MIDDLE: nEventCode |= MIDDLE_BUTTON; break;
    }
    switch (rEvent.GetClicks())
    {
        case 1: nEventCode |= SINGLE_CLICK; break;
        case 2: nEventCode |= DOUBLE_CLICK; break;
    }
    if (pHitPage != NULL)
        if (pHitDescriptor->IsSelected())
            nEventCode |= OVER_SELECTED_PAGE;
        else
            nEventCode |= OVER_UNSELECTED_PAGE;
    if (rEvent.IsShift())
        nEventCode |= SHIFT_MODIFIER;
    if (rEvent.IsMod1())
        nEventCode |= CONTROL_MODIFIER;
    view::ViewOverlay& rOverlay (mrController.GetView().GetOverlay());
    if (rOverlay.GetSubstitutionOverlay().IsShowing())
        nEventCode |= SUBSTITUTION_VISIBLE;
    if (mbRectangleSelection)
        nEventCode |= RECTANGLE_VISIBLE;

    mrController.GetPageSelector().DisableBroadcasting();
    bool bMakeSelectionVisible = true;

    // 2. With the event code determine the type of operation with which to
    // react to the event.
    OSL_TRACE ("EventCode is %x", nEventCode);
    switch (nEventCode)
    {
        // Simple single selection.
        case BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | OVER_UNSELECTED_PAGE:
        case BUTTON_UP | LEFT_BUTTON | SINGLE_CLICK | OVER_SELECTED_PAGE:
            mbPageHit = true;
            // Without the shift key the selection is set to only the page
            // object under the mouse. The center pane is switched to the
            // edit view that shows the elected slide when the slide sorter
            // is not shown in the center pane.
            if ( ! mrController.GetViewShell().IsMainViewShell())
            {
                DeselectAllPages();
                SelectHitPage(*pHitDescriptor);
                SetCurrentPageAndSwitchView(*pHitDescriptor);
            }
            else
            {
                DeselectAllPages();
                SelectHitPage(*pHitDescriptor);
            }
            PrepareMouseMotion(pWindow->PixelToLogic(aMousePosition));
            break;

        case BUTTON_DOWN | LEFT_BUTTON | DOUBLE_CLICK | OVER_SELECTED_PAGE:
        case BUTTON_DOWN | LEFT_BUTTON | DOUBLE_CLICK | OVER_UNSELECTED_PAGE:
            // A double click allways shows the selected slide in the center
            // pane in an edit view.
            DeselectAllPages();
            SelectHitPage(*pHitDescriptor);
            SetCurrentPageAndSwitchView(*pHitDescriptor);
            break;

        // Multi selection with the control modifier.
        case BUTTON_UP | LEFT_BUTTON | SINGLE_CLICK | OVER_SELECTED_PAGE | CONTROL_MODIFIER:
            mbPageHit = true;
            DeselectHitPage(*pHitDescriptor);
            PrepareMouseMotion(pWindow->PixelToLogic(aMousePosition));
            break;

        case BUTTON_UP | LEFT_BUTTON | SINGLE_CLICK | OVER_UNSELECTED_PAGE | CONTROL_MODIFIER:
            mbPageHit = true;
            SelectHitPage(*pHitDescriptor);
            PrepareMouseMotion(pWindow->PixelToLogic(aMousePosition));
            break;

        // Range selection with the shift modifier.
        case BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | OVER_SELECTED_PAGE | SHIFT_MODIFIER:
        case BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | OVER_UNSELECTED_PAGE | SHIFT_MODIFIER:
            DeselectAllPages();
            RangeSelect(*pHitDescriptor);
            break;

        // Preview of the page transition effect.
        case BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | OVER_FADE_INDICATOR:
            ShowEffect(*pHitDescriptor);
            break;

        // Right button for context menu.
        case BUTTON_DOWN | RIGHT_BUTTON | SINGLE_CLICK | OVER_UNSELECTED_PAGE:
            // Single right click selects as preparation to show the context
            // menu.  Change the selection only when the page under the
            // mouse is not selected.  In this case the selection is set to
            // this single page.  Otherwise the selection is not modified.
            DeselectAllPages();
            SelectHitPage(*pHitDescriptor);
            SetCurrentPage(*pHitDescriptor);
            bMakeSelectionVisible = false;
            break;

        // Drag and drop.
        case ANY_MODIFIER(MOUSE_MOTION | LEFT_BUTTON | SINGLE_CLICK | OVER_SELECTED_PAGE):
            mrController.GetScrollBarManager().AutoScroll(aMousePosition);
            CreateSubstitution(aMouseModelPosition);
            break;

        case ANY_MODIFIER(MOUSE_MOTION | LEFT_BUTTON | SINGLE_CLICK | OVER_SELECTED_PAGE | SUBSTITUTION_VISIBLE):
        case ANY_MODIFIER(MOUSE_MOTION | LEFT_BUTTON | SINGLE_CLICK | OVER_UNSELECTED_PAGE | SUBSTITUTION_VISIBLE):
        case ANY_MODIFIER(MOUSE_MOTION | LEFT_BUTTON | SINGLE_CLICK | NOT_OVER_PAGE | SUBSTITUTION_VISIBLE):
            mrController.GetScrollBarManager().AutoScroll(aMousePosition);
            UpdateSubstitution(aMouseModelPosition);
            break;

        case ANY_MODIFIER(BUTTON_UP | LEFT_BUTTON | SINGLE_CLICK | NOT_OVER_PAGE | SUBSTITUTION_VISIBLE):
        case ANY_MODIFIER(BUTTON_UP | LEFT_BUTTON | SINGLE_CLICK | OVER_SELECTED_PAGE | SUBSTITUTION_VISIBLE):
        case ANY_MODIFIER(BUTTON_UP | LEFT_BUTTON | SINGLE_CLICK | OVER_UNSELECTED_PAGE | SUBSTITUTION_VISIBLE):
            MoveSubstitution();
            break;

        // Rectangle selection.
        case BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | NOT_OVER_PAGE | NO_MODIFIER:
            DeselectAllPages();
            OSL_TRACE("start rectangle selection");
            StartRectangleSelection(aMouseModelPosition);
            break;

        case BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | NOT_OVER_PAGE | SHIFT_MODIFIER:
        case BUTTON_DOWN | LEFT_BUTTON | SINGLE_CLICK | NOT_OVER_PAGE | CONTROL_MODIFIER:
            OSL_TRACE("start rectangle selection2");
            StartRectangleSelection(aMouseModelPosition);
            break;

        case ANY_MODIFIER(MOUSE_MOTION | LEFT_BUTTON | SINGLE_CLICK | RECTANGLE_VISIBLE | NOT_OVER_PAGE):
        case ANY_MODIFIER(MOUSE_MOTION | LEFT_BUTTON | SINGLE_CLICK | RECTANGLE_VISIBLE | OVER_SELECTED_PAGE):
        case ANY_MODIFIER(MOUSE_MOTION | LEFT_BUTTON | SINGLE_CLICK | RECTANGLE_VISIBLE | OVER_UNSELECTED_PAGE):
            OSL_TRACE("update rectangle selection");
            mrController.GetScrollBarManager().AutoScroll(aMousePosition);
            UpdateRectangleSelection(aMouseModelPosition);
            break;

        case BUTTON_UP | LEFT_BUTTON | SINGLE_CLICK | RECTANGLE_VISIBLE | NO_MODIFIER:
            ProcessRectangleSelection(false);
            break;

        case BUTTON_UP | LEFT_BUTTON | SINGLE_CLICK | RECTANGLE_VISIBLE | SHIFT_MODIFIER:
        case BUTTON_UP | LEFT_BUTTON | SINGLE_CLICK | RECTANGLE_VISIBLE | CONTROL_MODIFIER:
            ProcessRectangleSelection(true);
            break;

        default:
            OSL_TRACE ("can not handle event code %x", nEventCode);
            // Do nothing.
            break;
    }
    mrController.GetPageSelector().EnableBroadcasting(bMakeSelectionVisible);
}




void SelectionFunction::SelectHitPage (model::PageDescriptor& rDescriptor)
{
    mrController.GetPageSelector().SelectPage(rDescriptor);
    mpRangeSelectionAnchor = &rDescriptor;
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
    mbRectangleSelection = true;
    OSL_TRACE("starting  rectangle selection at %d %d",
        rMouseModelPosition.X(), rMouseModelPosition.Y());

    mrController.GetView().GetOverlay().GetSelectionRectangleOverlay().Start(rMouseModelPosition);
    //    mrController.GetView().BegEncirclement (aMouseModelPosition);
}




void SelectionFunction::UpdateRectangleSelection (const Point& rMouseModelPosition)
{
    mrController.GetView().GetOverlay().GetSelectionRectangleOverlay().Update(rMouseModelPosition);
    //    mrController.GetView().MovEncirclement(rMouseModelPosition);
}




void SelectionFunction::ProcessRectangleSelection (bool bToggleSelection)
{
    if (mbRectangleSelection)
    {
        mbRectangleSelection = false;
        view::ViewOverlay& rOverlay (mrController.GetView().GetOverlay());
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
            if (rSelectionRectangle.IsInside (aPageBox))
            {
                // The first page in the selection rectangle becomes the new
                // selection anchor.
                if (bFirstPage)
                {
                    mpRangeSelectionAnchor = &rDescriptor;
                    bFirstPage = false;
                }

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
    // Remember position and status of the substitution.
    //    mrController.GetView().GetOverlay().GetSubstitutionOverlay().Hide();
    //    mrController.GetView().GetOverlay().GetSubstitutionOverlay().SetPosition (aMouseModelPosition);

    bFirstMouseMove = TRUE;
    aDragTimer.Start();
}




void SelectionFunction::SetCurrentPage (model::PageDescriptor& rDescriptor)
{
    mrController.GetPageSelector().SetCurrentPage (rDescriptor);
}




void SelectionFunction::RangeSelect (model::PageDescriptor& rDescriptor)
{
    PageSelector& rSelector (mrController.GetPageSelector());

    if (mpRangeSelectionAnchor!=NULL)
    {
        // Select all pages between the anchor and the given one, including
        // the two.
        USHORT nAnchorIndex (
            (mpRangeSelectionAnchor->GetPage()->GetPageNum()-1) / 2);
        USHORT nOtherIndex (
            (rDescriptor.GetPage()->GetPageNum()-1) / 2);
        USHORT nMinIndex (::std::min(nAnchorIndex, nOtherIndex));
        USHORT nMaxIndex (::std::max(nAnchorIndex, nOtherIndex));
        for (USHORT nIndex=nMinIndex; nIndex<=nMaxIndex; nIndex++)
            rSelector.SelectPage (nIndex);
    }
}




void SelectionFunction::MoveSubstitution (void)
{
    PageSelector& rSelector (mrController.GetPageSelector());
    view::ViewOverlay& rOverlay (mrController.GetView().GetOverlay());

    // Clear the substitution display and insertion marker.
    rOverlay.GetSubstitutionOverlay().Hide();
    rOverlay.GetSubstitutionOverlay().Clear();
    rOverlay.GetInsertionIndicatorOverlay().Hide();
    rOverlay.GetMouseOverIndicatorOverlay().SetSlideUnderMouse (NULL);

    // Tell the model to move the selected pages behind the one with the
    // index mnInsertionIndex which first has to transformed into an index
    // understandable by the document.
    sal_Int32 nInsertionIndex = rOverlay.GetInsertionIndicatorOverlay().GetInsertionPageIndex();
    if (nInsertionIndex >= 0)
    {
        USHORT nDocumentIndex = (USHORT)nInsertionIndex-1;
        int nSelectedPageCount (rSelector.GetSelectedPageCount());
        model::SlideSorterModel& rModel (mrController.GetModel());
        rModel.SynchronizeDocumentSelection();
        if (mrController.MoveSelectedPages(nDocumentIndex))
        {
            // Pages where moved.  Update the selection and make the
            // first selected page the current page.
            rModel.SynchronizeModelSelection();
            rSelector.UpdateAllPages();
            for (int i=0; i<rSelector.GetPageCount(); i++)
                if (rSelector.IsPageSelected(i))
                {
                    rSelector.SetCurrentPage(i);
                    break;
                }
        }

        // The selection anchor may have become invalid.
        mpRangeSelectionAnchor = NULL;
    }

    pViewShell->GetViewFrame()->GetBindings().Invalidate(SID_STATUS_PAGE);
}




void SelectionFunction::CreateSubstitution (const Point& rMouseModelPosition)
{
    // No Drag-and-Drop for master pages.
    if (mrController.GetModel().GetEditMode() == EM_PAGE)
    {
        view::ViewOverlay& rOverlay (mrController.GetView().GetOverlay());

        // Show a new substitution for the selected page objects.
        model::PageEnumeration aSelectedPages(mrController.GetModel().GetSelectedPagesEnumeration());
        rOverlay.GetSubstitutionOverlay().Create(aSelectedPages, rMouseModelPosition);
        rOverlay.GetSubstitutionOverlay().Show ();

        // Show an indicator of where the selected pages would be moved to when
        // the left mouse button were released at the current position.
        rOverlay.GetInsertionIndicatorOverlay().SetPosition(rMouseModelPosition);
        rOverlay.GetInsertionIndicatorOverlay().Show();
    }
}




void SelectionFunction::UpdateSubstitution (const Point& rMouseModelPosition)
{
    view::ViewOverlay& rOverlay (mrController.GetView().GetOverlay());

    // Move the existing substitution to the new position.
    model::PageEnumeration aSelectedPages(mrController.GetModel().GetSelectedPagesEnumeration());
    rOverlay.GetSubstitutionOverlay().SetPosition (rMouseModelPosition);
    rOverlay.GetSubstitutionOverlay().Show ();

    // Show an indicator of where the selected pages would be moved to when
    // the left mouse button were released at the current position.
    rOverlay.GetInsertionIndicatorOverlay().SetPosition(rMouseModelPosition);
    rOverlay.GetInsertionIndicatorOverlay().Show();
}


void SelectionFunction::GotoNextPage (int nOffset)
{
    SdPage* pPage = mrController.GetViewShell().GetActualPage();
    sal_Int32 nIndex = (pPage->GetPageNum()-1) / 2;
    nIndex += nOffset;
    USHORT nPageCount = mrController.GetModel().GetPageCount();

    if (nIndex >= nPageCount)
        nIndex = nPageCount - 1;
    if (nIndex < 0)
        nIndex = 0;

    mrController.GetPageSelector().SetCurrentPage(nIndex);
}



} } } // end of namespace ::sd::slidesorter::controller
