/*************************************************************************
 *
 *  $RCSfile: SlsSelectionFunction.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-15 08:56:41 $
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

#include "SlideSorterViewShell.hxx"
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
#include "TextLogger.hxx"
#include "showview.hxx"
#include "fader.hxx"
#include "PaneManager.hxx"
#include "ViewShellBase.hxx"
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
#include "Window.hxx"
#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "ViewShell.hxx"
#include "FrameView.hxx"
#include "app.hrc"
#include "sdresid.hxx"
#include "strings.hrc"

#include "TextLogger.hxx"


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
      mbDragSelection(false)
{
    mpSound = new Sound;

    mpShowingEffectInfo = new ShowingEffectInfo (false);

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

    BOOL bResult = false;
    Point aMousePosition (rEvent.GetPosPixel());
    Point aMouseModelPosition = pWindow->PixelToLogic(aMousePosition);
    model::PageDescriptor* pHitDescriptor
        = mrController.GetPageAt (aMousePosition);
    SdPage* pHitPage = (pHitDescriptor != NULL)
        ? pHitDescriptor->GetPage()
        : NULL;
    model::PageDescriptor* pFadeDescriptor = mrController.GetFadePageAt (
        aMousePosition);
    SdPage* pFadePage = (pFadeDescriptor != NULL)
        ? pFadeDescriptor->GetPage()
        : NULL;

    // Remember whether the slide sorter is shown in the main view.
    // Depending on that we have a slightly different behaviour.
    bool bIsMainView = mrController.GetViewShell().IsMainViewShell();

    pWindow->CaptureMouse();

    // In these flags we remember what actions to take while analyzing the
    // event.
    bool bToggleSelection = false;
    bool bSetSingleSelection = false;
    bool bSetCurrentPage = false;
    bool bSwitchView = false;
    bool bPrepareMouseMotion = false;

    // 1. In a first step we analyze the event and determine what actions to
    // execute.
    if (rEvent.GetButtons() == MOUSE_LEFT)
    {
        if (rEvent.GetClicks() == 1)
        {
            if (pHitPage != NULL)
            {
                // Remember the page hit for MouseMove.
                mbPageHit = true;

                if (rEvent.IsShift())
                {
                    // When the shift key is pressed then the selection of
                    // the page object is toggled without changing the
                    // selection of other page objects.
                    bToggleSelection = true;
                }
                else
                {
                    // Without the shift key the selection is set to only
                    // the page object under the mouse. The center pane
                    // is switched to the edit view that shows the elected
                    // slide when the slide sorter is not shown in the
                    // center pane.
                    if ( ! bIsMainView)
                    {
                        bSwitchView = true;
                        bSetCurrentPage = true;
                    }
                    else
                        bSetSingleSelection = true;
                    bResult = true;
                }

                bPrepareMouseMotion = true;
            }
            else if (pFadePage != NULL)
            {
                // One of the slide change symbols has been hit.
                ShowEffect (*pFadeDescriptor);
                bResult = true;
            }
        }
        else if (rEvent.GetClicks() == 2
            && pHitPage != NULL)
        {
            // A double click allways shows the selected slide in the center
            // pane in an edit view.
            bSwitchView = true;
            bSetCurrentPage = true;
        }
    }
    else if (rEvent.GetButtons()==MOUSE_RIGHT && rEvent.GetClicks()== 1)
    {
        // Single right click selects as preparation to show the context
        // menu.  Change the selection only when the page under the mouse is
        // not selected.  In this case the selection is set to this single
        // page.  Otherwise the selection is not modified.
        if (pHitDescriptor!=NULL && ! pHitDescriptor->IsSelected())
        {
            bSetSingleSelection = true;
            bSetCurrentPage = true;
        }
    }

    // 2. Execute the actions that have been determined ealier.
    controller::PageSelector& rSelector = mrController.GetPageSelector();
    bool bMakeSelectionVisible = true;
    rSelector.DisableBroadcasting ();
    if (bToggleSelection)
    {
        if (pHitDescriptor->IsSelected())
            rSelector.DeselectPage (*pHitDescriptor);
        else
            rSelector.SelectPage (*pHitDescriptor);
    }
    if (bSetSingleSelection)
    {
        if (rSelector.GetSelectedPageCount() > 1
            || ! pHitDescriptor->IsSelected())
        {
            rSelector.DeselectAllPages ();
            rSelector.SelectPage (*pHitDescriptor);
        }
    }
    if (bPrepareMouseMotion)
    {
        // Remember position and status of the substitution.
        mrController.GetView().GetOverlay().GetSubstitutionOverlay().Hide();
        mrController.GetView().GetOverlay().GetSubstitutionOverlay()
            .SetPosition (aMouseModelPosition);

        bFirstMouseMove = TRUE;
        aDragTimer.Start();
    }
    if (bSetCurrentPage)
    {
        if (bSwitchView)
            SetCurrentPageAndSwitchView (*pHitDescriptor);
        else
            rSelector.SetCurrentPage (*pHitDescriptor);
    }


    // Show the selection rectangle.
    if (pHitPage == NULL && !bResult)
    {
        // When the shift key is not pressed we first clear the current
        // selection.  Otherwise the newly selected pages will be added to
        // the existing selection.
        if ( ! rEvent.IsShift())
        {
            rSelector.DeselectAllPages ();
            // No slide is selected, so nothing to make visible.
            // Furthermore we want to prevent the view from scrolling when a
            // right click deselects the slides: we have to determine the
            // insertion position.
            bMakeSelectionVisible = false;
        }

        mbDragSelection = true;
        mrController.GetView().BegEncirclement (aMouseModelPosition);
    }
    rSelector.EnableBroadcasting (bMakeSelectionVisible);

    return bResult;
}




BOOL SelectionFunction::MouseMove (const MouseEvent& rEvent)
{
    Point aMousePosition (rEvent.GetPosPixel());
    Point aMouseModelPosition (pWindow->PixelToLogic (aMousePosition));
    BOOL bResult = FALSE;
    view::ViewOverlay& rOverlay (mrController.GetView().GetOverlay());

    // Allow one mouse move before the drag timer is disabled.
    if (aDragTimer.IsActive())
    {
        if (bFirstMouseMove)
            bFirstMouseMove = FALSE;
        else
            aDragTimer.Stop();
    }

    if (rEvent.GetButtons() == MOUSE_LEFT)
    {
        mrController.GetScrollBarManager().AutoScroll (aMousePosition);
        //ForceScroll (aMousePosition);
        Rectangle aRectangle (Point(0,0),pWindow->GetOutputSizePixel());
        if ( ! aRectangle.IsInside(aMousePosition)
            && mbPageHit
            && rOverlay.GetSubstitutionOverlay().IsShowing())
        {
            // Mouse left the window with pressed left button.  Make it a
            // drag.
            StartDrag();
        }
        else if ((mbPageHit
                && ! rEvent.IsShift()
                || rOverlay.GetSubstitutionOverlay().IsShowing())
            && mrController.GetModel().GetPageCount() > 0)
        {
            // Show the substitution of the page objects at the current
            // mouse position.
            if (rOverlay.GetSubstitutionOverlay().IsShowing())
            {
                // Move the existing substitution to the new position.
                rOverlay.GetSubstitutionOverlay().SetPosition (
                    aMouseModelPosition);
                rOverlay.GetSubstitutionOverlay().Show ();
            }
            else
            {
                // Show a new substitution for the selected page objects.
                model::PageEnumeration aSelectedPages (
                    mrController.GetModel().GetSelectedPagesEnumeration());
                rOverlay.GetSubstitutionOverlay().Create (
                    aSelectedPages,
                    aMouseModelPosition);
                rOverlay.GetSubstitutionOverlay().Show ();
            }

            // Show an indicator of where the selected pages would be moved to
            // when the left mouse button were released at the current
            // position.
            rOverlay.GetInsertionIndicatorOverlay().SetPosition(
                aMouseModelPosition);
            rOverlay.GetInsertionIndicatorOverlay().Show();

            bResult = TRUE;
        }
        else if (mbDragSelection)
        {
            mrController.GetView().MovEncirclement (aMouseModelPosition);
        }
    }

    return bResult;
}




BOOL SelectionFunction::MouseButtonUp (const MouseEvent& rEvent)
{
    bool bResult = false;
    view::ViewOverlay& rOverlay (mrController.GetView().GetOverlay());
    PageSelector& rSelector (mrController.GetPageSelector());

    mrController.GetScrollBarManager().StopAutoScroll ();

    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode (rEvent.GetButtons());
    //  SdDrawDocument* pDoc = pSlView->GetDoc();

    if (aDragTimer.IsActive())
        aDragTimer.Stop();

    if (rEvent.GetButtons() == MOUSE_LEFT
        && rOverlay.GetSubstitutionOverlay().IsShowing())
    {
        Point aMouseModelPosition =
            pWindow->PixelToLogic(rEvent.GetPosPixel());

        // Clear the substitution display and insertion marker.
        rOverlay.GetSubstitutionOverlay().Hide();
        rOverlay.GetSubstitutionOverlay().Clear();
        rOverlay.GetInsertionIndicatorOverlay().Hide();

        // Tell the model to move the selected pages behind the one with the
        // index mnInsertionIndex which first has to transformed into an
        // index understandable by the document.
        sal_Int32 nInsertionIndex
            = rOverlay.GetInsertionIndicatorOverlay().GetInsertionPageIndex();
        if (nInsertionIndex >= 0)
        {
            USHORT nDocumentIndex = (USHORT)nInsertionIndex-1;
            int nSelectedPageCount (rSelector.GetSelectedPageCount());
            model::SlideSorterModel& rModel (mrController.GetModel());
            rModel.SynchronizeDocumentSelection();
            if (rModel.GetDocument()->MovePages (nDocumentIndex))
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
        }

        pViewShell->GetViewFrame()->GetBindings().Invalidate(SID_STATUS_PAGE);
        bResult = true;
    }
    else if (mbDragSelection)
    {
        rOverlay.GetSelectionRectangleOverlay().Hide();

        // Select all pages whose page object lies completly inside the drag
        // rectangle.
        const Rectangle& rSelectionRectangle (
            rOverlay.GetSelectionRectangleOverlay().GetSelectionRectangle());
        model::SlideSorterModel::Enumeration aPages (
            mrController.GetModel().GetAllPagesEnumeration());
        while (aPages.HasMoreElements())
        {
            model::PageDescriptor& rDescriptor (aPages.GetNextElement());
            Rectangle aPageBox (mrController.GetView().GetPageBoundingBox(
                rDescriptor,
                view::SlideSorterView::CS_MODEL,
                view::SlideSorterView::BBT_SHAPE));
            if (rSelectionRectangle.IsInside (aPageBox))
            {
                // When we are extending the selection (shift key is
                // pressed) then toggle the selection state of the page.
                // Otherwise select it: this results in the previously
                // selected pages becoming deslected.
                bool bSelect = true;
                if (rEvent.IsShift() && rDescriptor.IsSelected())
                    bSelect = false;
                if (bSelect)
                    rSelector.SelectPage (rDescriptor);
                else
                    rSelector.DeselectPage (rDescriptor);
            }
        }
    }
    mbPageHit = false;                // ab jetzt keine Seite mehr "am Haken"
    pWindow->ReleaseMouse();

    return bResult;
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

        // Scroll up but don't move the focus indicator.
        case KEY_PAGEUP:
            ScrollStart();
            pViewShell->ScrollLines (0, -10);
            ScrollEnd();
            bResult = TRUE;
            break;

        // Scroll down but don't move the focus indicator.
        case KEY_PAGEDOWN:
            ScrollStart();
            pViewShell->ScrollLines (0, +10);
            ScrollEnd();
            bResult = TRUE;
            break;

        case KEY_ADD:
            pViewShell->SetZoom (pWindow->GetZoom() * 3 / 2);
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
        GraphicObject* pOld = NULL;/*const_cast< GraphicObject* >(
            pSlView->GetFromCache( pDoc->GetSdPage(nPrevPage, PK_STANDARD), nZoom, -10 ) );  */
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
    GraphicObject* pNew = NULL;/*const_cast< GraphicObject* >( pSlView->GetFromCache( pDoc->GetSdPage(nPageNo, PK_STANDARD), nZoom, -10 ) );*/

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




} } } // end of namespace ::sd::slidesorter::controller
