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

#include <fupoor.hxx>

#include <svx/svxids.hrc>
#include <svl/aeitem.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdograf.hxx>
#include <vcl/seleng.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XLayer.hpp>
#include <com/sun/star/drawing/XLayerManager.hpp>
#include <com/sun/star/container/XChild.hpp>

#include <FrameView.hxx>
#include <app.hrc>
#include <fusel.hxx>
#include <sdpage.hxx>
#include <drawview.hxx>
#include <DrawViewShell.hxx>
#include <Window.hxx>
#include <drawdoc.hxx>
#include <DrawDocShell.hxx>
#include <zoomlist.hxx>
#include <Client.hxx>
#include <slideshow.hxx>
#include <LayerTabBar.hxx>

#include <sfx2/viewfrm.hxx>

#include <svx/svditer.hxx>

#include <editeng/editeng.hxx>

using namespace ::com::sun::star;

namespace sd {


FuPoor::FuPoor (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDrDoc,
    SfxRequest& rReq)
    : mpView(pView),
      mpViewShell(pViewSh),
      mpWindow(pWin),
      mpDocSh( pDrDoc->GetDocSh() ),
      mpDoc(pDrDoc),
      nSlotId( rReq.GetSlot() ),
      bIsInDragMode(false),
      bNoScrollUntilInside (true),
      bScrollable (false),
      bDelayActive (false),
      bFirstMouseMove (false),
      // remember MouseButton state
      mnCode(0)
{
    ReceiveRequest(rReq);

    aScrollTimer.SetInvokeHandler( LINK(this, FuPoor, ScrollHdl) );
    aScrollTimer.SetTimeout(SELENG_AUTOREPEAT_INTERVAL);

    aDragTimer.SetInvokeHandler( LINK(this, FuPoor, DragHdl) );
    aDragTimer.SetTimeout(SELENG_DRAGDROP_TIMEOUT);

    aDelayToScrollTimer.SetInvokeHandler( LINK(this, FuPoor, DelayHdl) );
    aDelayToScrollTimer.SetTimeout(2000);
}

FuPoor::~FuPoor()
{
    aDragTimer.Stop();
    aScrollTimer.Stop();
    aDelayToScrollTimer.Stop();
}

void FuPoor::Activate()
{
}

void FuPoor::Deactivate()
{
    aDragTimer.Stop();
    aScrollTimer.Stop();
    aDelayToScrollTimer.Stop ();
    bScrollable = bDelayActive = false;

    if (mpWindow && mpWindow->IsMouseCaptured())
        mpWindow->ReleaseMouse();
}

void FuPoor::SetWindow(::sd::Window* pWin)
{
    mpWindow = pWin;
}

/**
 * scroll when approached the border of the window; is called by MouseMove
 */
void FuPoor::ForceScroll(const Point& aPixPos)
{
    aScrollTimer.Stop();

    if ( mpView->IsDragHelpLine() || mpView->IsSetPageOrg() ||
         SlideShow::IsRunning( mpViewShell->GetViewShellBase() ) )
        return;

    Point aPos = mpWindow->OutputToScreenPixel(aPixPos);
    const ::tools::Rectangle& rRect = mpViewShell->GetAllWindowRect();

    if ( bNoScrollUntilInside )
    {
        if ( rRect.IsInside(aPos) )
            bNoScrollUntilInside = false;
    }
    else
    {
        short dx = 0, dy = 0;

        if ( aPos.X() <= rRect.Left()   ) dx = -1;
        if ( aPos.X() >= rRect.Right()  ) dx =  1;
        if ( aPos.Y() <= rRect.Top()    ) dy = -1;
        if ( aPos.Y() >= rRect.Bottom() ) dy =  1;

        if ( dx != 0 || dy != 0 )
        {
            if (bScrollable)
            {
                // scroll action in derived class
                mpViewShell->ScrollLines(dx, dy);
                aScrollTimer.Start();
            }
            else if (! bDelayActive) StartDelayToScrollTimer ();
        }
    }
}

/**
 * timer handler for window scrolling
 */
IMPL_LINK_NOARG(FuPoor, ScrollHdl, Timer *, void)
{
    Point aPnt(mpWindow->GetPointerPosPixel());

    // use remembered MouseButton state to create correct
    // MouseEvents for this artificial MouseMove.
    MouseMove(MouseEvent(aPnt, 1, MouseEventModifiers::NONE, GetMouseButtonCode()));
}

/**
 * handle keyboard events
 * @returns sal_True if the event was handled, sal_False otherwise
 */
bool FuPoor::KeyInput(const KeyEvent& rKEvt)
{
    sal_uInt16          nCode = rKEvt.GetKeyCode().GetCode();
    bool            bReturn = false;
    bool            bSlideShow = SlideShow::IsRunning( mpViewShell->GetViewShellBase() );

    switch (nCode)
    {
        case KEY_RETURN:
        {
            if(rKEvt.GetKeyCode().IsMod1())
            {
                if( dynamic_cast< const DrawViewShell *>( mpViewShell ) !=  nullptr)
                {
                    DrawViewShell* pDrawViewShell =
                        static_cast<DrawViewShell*>(mpViewShell);
                    SdPage* pActualPage = pDrawViewShell->GetActualPage();
                    SdrTextObj* pCandidate = nullptr;

                    if(pActualPage)
                    {
                        SdrObjListIter aIter(pActualPage, SdrIterMode::DeepNoGroups);

                        while(aIter.IsMore() && !pCandidate)
                        {
                            SdrObject* pObj = aIter.Next();

                            if(dynamic_cast< const SdrTextObj *>( pObj ))
                            {
                                SdrInventor nInv(pObj->GetObjInventor());
                                sal_uInt16 nKnd(pObj->GetObjIdentifier());

                                if(SdrInventor::Default == nInv &&
                                    (OBJ_TITLETEXT == nKnd || OBJ_OUTLINETEXT == nKnd || OBJ_TEXT == nKnd))
                                {
                                    pCandidate = static_cast<SdrTextObj*>(pObj);
                                }
                            }
                        }
                    }

                    if(pCandidate)
                    {
                        mpView->UnMarkAll();
                        mpView->MarkObj(pCandidate, mpView->GetSdrPageView());

                        mpViewShell->GetViewFrame()->GetDispatcher()->Execute(
                            SID_ATTR_CHAR, SfxCallMode::ASYNCHRON);
                    }
                    else
                    {
                        // insert a new page with the same page layout
                        mpViewShell->GetViewFrame()->GetDispatcher()->Execute(
                            SID_INSERTPAGE_QUICK, SfxCallMode::ASYNCHRON);
                    }

                    // consumed
                    bReturn = true;
                }
            }
            else
            {
                // activate OLE object on RETURN for selected object
                // activate text edit on RETURN for selected object
                const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();

                if( !mpView->IsTextEdit() && 1 == rMarkList.GetMarkCount() )
                {
                    SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

                    if( dynamic_cast< const SdrOle2Obj* >( pObj ) && !mpDocSh->IsUIActive() )
                    {
                        //HMHmpView->HideMarkHdl();
                        mpViewShell->ActivateObject( static_cast< SdrOle2Obj* >( pObj ), 0 );
                    }
                    else if( pObj && pObj->IsEmptyPresObj() && dynamic_cast< const SdrGrafObj *>( pObj ) !=  nullptr )
                    {
                        mpViewShell->GetViewFrame()->GetDispatcher()->Execute( SID_INSERT_GRAPHIC, SfxCallMode::ASYNCHRON | SfxCallMode::RECORD );
                    }
                    else
                    {
                        mpViewShell->GetViewFrame()->GetDispatcher()->Execute( SID_ATTR_CHAR, SfxCallMode::ASYNCHRON | SfxCallMode::RECORD );
                    }

                    // consumed
                    bReturn = true;
                }
            }
        }
        break;

        case KEY_TAB:
        {
            // handle Mod1 and Mod2 to get travelling running on different systems
            if(rKEvt.GetKeyCode().IsMod1() || rKEvt.GetKeyCode().IsMod2())
            {
                // do something with a selected handle?
                const SdrHdlList& rHdlList = mpView->GetHdlList();
                bool bForward(!rKEvt.GetKeyCode().IsShift());

                const_cast<SdrHdlList&>(rHdlList).TravelFocusHdl(bForward);

                // guarantee visibility of focused handle
                SdrHdl* pHdl = rHdlList.GetFocusHdl();

                if(pHdl)
                {
                    Point aHdlPosition(pHdl->GetPos());
                    ::tools::Rectangle aVisRect(aHdlPosition - Point(100, 100), Size(200, 200));
                    mpView->MakeVisible(aVisRect, *mpWindow);
                }

                // consumed
                bReturn = true;
            }
        }
        break;

        case KEY_ESCAPE:
        {
            bReturn = FuPoor::cancel();
        }
        break;

        case KEY_ADD:
        {
            if (!mpView->IsTextEdit() && !bSlideShow && !mpDocSh->IsUIActive())
            {
                // increase zoom
                mpViewShell->SetZoom(mpWindow->GetZoom() * 3 / 2);

                if( dynamic_cast< const DrawViewShell *>( mpViewShell ) !=  nullptr)
                    static_cast<DrawViewShell*>(mpViewShell)
                        ->SetZoomOnPage(false);

                bReturn = true;
            }
        }
        break;

        case KEY_SUBTRACT:
        {
            if (!mpView->IsTextEdit() && !bSlideShow && !mpDocSh->IsUIActive())
            {
                // decrease zoom
                mpViewShell->SetZoom(mpWindow->GetZoom() * 2 / 3);

                if( dynamic_cast< const DrawViewShell *>( mpViewShell ) !=  nullptr)
                    static_cast<DrawViewShell*>(mpViewShell)
                        ->SetZoomOnPage(false);

                bReturn = true;
            }
        }
        break;

        case KEY_MULTIPLY:
        {
            if (!mpView->IsTextEdit() && !bSlideShow)
            {
                // zoom to page
                mpViewShell->GetViewFrame()->GetDispatcher()->
                Execute(SID_SIZE_PAGE, SfxCallMode::ASYNCHRON | SfxCallMode::RECORD);
                bReturn = true;
            }
        }
        break;

        case KEY_DIVIDE:
        {
            if (!mpView->IsTextEdit() && !bSlideShow)
            {
                // zoom to selected objects
                mpViewShell->GetViewFrame()->GetDispatcher()->
                Execute(SID_SIZE_OPTIMAL, SfxCallMode::ASYNCHRON | SfxCallMode::RECORD);
                bReturn = true;
            }
        }
        break;

        case KEY_POINT:
        {
            ZoomList* pZoomList = mpViewShell->GetZoomList();

            if (!mpView->IsTextEdit() && pZoomList->IsNextPossible() && !bSlideShow && !mpDocSh->IsUIActive())
            {
                // use next ZoomRect
                mpViewShell->SetZoomRect(pZoomList->GetNextZoomRect());
                bReturn = true;
            }
        }
        break;

        case KEY_COMMA:
        {
            ZoomList* pZoomList = mpViewShell->GetZoomList();

            if (!mpView->IsTextEdit() && pZoomList->IsPreviousPossible() && !bSlideShow && !mpDocSh->IsUIActive())
            {
                // use previous ZoomRect
                mpViewShell->SetZoomRect(pZoomList->GetPreviousZoomRect());
                bReturn = true;
            }
        }
        break;

        case KEY_HOME:
        {
            if (!mpView->IsTextEdit()
                && dynamic_cast< const DrawViewShell *>( mpViewShell ) !=  nullptr
                && !bSlideShow)
            {
               // jump to first page
               static_cast<DrawViewShell*>(mpViewShell)->SwitchPage(0);
               bReturn = true;
            }
        }
        break;

        case KEY_END:
        {
            if (!mpView->IsTextEdit()
                && dynamic_cast< const DrawViewShell *>( mpViewShell ) !=  nullptr
                && !bSlideShow)
            {
                // jump to last page
                SdPage* pPage =
                    static_cast<DrawViewShell*>(mpViewShell)->GetActualPage();
                static_cast<DrawViewShell*>(mpViewShell)
                    ->SwitchPage(mpDoc->GetSdPageCount(
                        pPage->GetPageKind()) - 1);
                bReturn = true;
            }
        }
        break;

        case KEY_PAGEUP:
        {
            if( rKEvt.GetKeyCode().IsMod1() && rKEvt.GetKeyCode().IsMod2() )
                break;

            if( dynamic_cast< const DrawViewShell *>( mpViewShell ) !=  nullptr && !bSlideShow)
            {
                // The page-up key switches layers or pages depending on the
                // modifier key.
                if ( ! rKEvt.GetKeyCode().GetModifier())
                {
                    // With no modifier pressed we move to the previous
                    // slide.
                    mpView->SdrEndTextEdit();

                    // Previous page.
                    bReturn = true;
                    SdPage* pPage = static_cast<DrawViewShell*>(mpViewShell)->GetActualPage();
                    sal_uInt16 nSdPage = (pPage->GetPageNum() - 1) / 2;

                    if (nSdPage > 0)
                    {
                        // Switch the page and send events regarding
                        // deactivation the old page and activating the new
                        // one.
                        TabControl& rPageTabControl =
                            static_cast<DrawViewShell*>(mpViewShell)
                            ->GetPageTabControl();
                        if (rPageTabControl.IsReallyShown())
                            rPageTabControl.SendDeactivatePageEvent ();
                        static_cast<DrawViewShell*>(mpViewShell)->SwitchPage(nSdPage - 1);
                        if (rPageTabControl.IsReallyShown())
                            rPageTabControl.SendActivatePageEvent ();
                    }
                }
                else if (rKEvt.GetKeyCode().IsMod1())
                {
                    // With the CONTROL modifier we switch layers.
                    if (static_cast<DrawViewShell*>(mpViewShell)->IsLayerModeActive())
                    {
                        // Moves to the previous layer.
                        SwitchLayer (-1);
                    }
                }
            }
        }
        break;

        case KEY_PAGEDOWN:
        {
            if( rKEvt.GetKeyCode().IsMod1() && rKEvt.GetKeyCode().IsMod2() )
                break;
            if(dynamic_cast< const DrawViewShell *>( mpViewShell ) !=  nullptr && !bSlideShow)
            {
                // The page-down key switches layers or pages depending on the
                // modifier key.
                if ( ! rKEvt.GetKeyCode().GetModifier())
                {
                    // With no modifier pressed we move to the next slide.
                    mpView->SdrEndTextEdit();

                    // Next page.
                    bReturn = true;
                    SdPage* pPage = static_cast<DrawViewShell*>(mpViewShell)->GetActualPage();
                    sal_uInt16 nSdPage = (pPage->GetPageNum() - 1) / 2;

                    if (nSdPage < mpDoc->GetSdPageCount(pPage->GetPageKind()) - 1)
                    {
                        // Switch the page and send events regarding
                        // deactivation the old page and activating the new
                        // one.
                        TabControl& rPageTabControl =
                            static_cast<DrawViewShell*>(mpViewShell)->GetPageTabControl();
                        if (rPageTabControl.IsReallyShown())
                            rPageTabControl.SendDeactivatePageEvent ();
                        static_cast<DrawViewShell*>(mpViewShell)->SwitchPage(nSdPage + 1);
                        if (rPageTabControl.IsReallyShown())
                            rPageTabControl.SendActivatePageEvent ();
                    }
                }
                else if (rKEvt.GetKeyCode().IsMod1())
                {
                    // With the CONTROL modifier we switch layers.
                    if (static_cast<DrawViewShell*>(mpViewShell)->IsLayerModeActive())
                    {
                        // With the layer mode active pressing page-down
                        // moves to the next layer.
                        SwitchLayer (+1);
                    }
                }
            }
        }
        break;

        // change select state when focus is on poly point
        case KEY_SPACE:
        {
            const SdrHdlList& rHdlList = mpView->GetHdlList();
            SdrHdl* pHdl = rHdlList.GetFocusHdl();

            if(pHdl)
            {
                if(pHdl->GetKind() == SdrHdlKind::Poly)
                {
                    // rescue ID of point with focus
                    sal_uInt32 nPol(pHdl->GetPolyNum());
                    sal_uInt32 nPnt(pHdl->GetPointNum());

                    if(mpView->IsPointMarked(*pHdl))
                    {
                        if(rKEvt.GetKeyCode().IsShift())
                        {
                            mpView->UnmarkPoint(*pHdl);
                        }
                    }
                    else
                    {
                        if(!rKEvt.GetKeyCode().IsShift())
                        {
                            mpView->UnmarkAllPoints();
                        }

                        mpView->MarkPoint(*pHdl);
                    }

                    if(nullptr == rHdlList.GetFocusHdl())
                    {
                        // restore point with focus
                        SdrHdl* pNewOne = nullptr;

                        for(size_t a = 0; !pNewOne && a < rHdlList.GetHdlCount(); ++a)
                        {
                            SdrHdl* pAct = rHdlList.GetHdl(a);

                            if(pAct
                                && pAct->GetKind() == SdrHdlKind::Poly
                                && pAct->GetPolyNum() == nPol
                                && pAct->GetPointNum() == nPnt)
                            {
                                pNewOne = pAct;
                            }
                        }

                        if(pNewOne)
                        {
                            const_cast<SdrHdlList&>(rHdlList).SetFocusHdl(pNewOne);
                        }
                    }

                    bReturn = true;
                }
            }
        }
        break;

        case KEY_UP:
        case KEY_DOWN:
        case KEY_LEFT:
        case KEY_RIGHT:
        {
            if (!mpView->IsTextEdit() && !bSlideShow)
            {
                long nX = 0;
                long nY = 0;

                if (nCode == KEY_UP)
                {
                    // scroll up
                    nX = 0;
                    nY =-1;
                }
                else if (nCode == KEY_DOWN)
                {
                    // scroll down
                    nX = 0;
                    nY = 1;
                }
                else if (nCode == KEY_LEFT)
                {
                    // scroll left
                    nX =-1;
                    nY = 0;
                }
                else if (nCode == KEY_RIGHT)
                {
                    // scroll right
                    nX = 1;
                    nY = 0;
                }

                if (mpView->AreObjectsMarked() && !rKEvt.GetKeyCode().IsMod1() &&
                    !mpDocSh->IsReadOnly())
                {
                    const SdrHdlList& rHdlList = mpView->GetHdlList();
                    SdrHdl* pHdl = rHdlList.GetFocusHdl();

                    bool bIsMoveOfConnectedHandle(false);
                    bool bOldSuppress = false;
                    SdrEdgeObj* pEdgeObj = nullptr;

                    if(pHdl && dynamic_cast< const SdrEdgeObj *>( pHdl->GetObj() ) && 0 == pHdl->GetPolyNum())
                    {
                        pEdgeObj = static_cast<SdrEdgeObj*>(pHdl->GetObj());

                        if(0 == pHdl->GetPointNum())
                        {
                            if(pEdgeObj->GetConnection(true).GetObject())
                            {
                                bIsMoveOfConnectedHandle = true;
                            }
                        }
                        if(1 == pHdl->GetPointNum())
                        {
                            if(pEdgeObj->GetConnection(false).GetObject())
                            {
                                bIsMoveOfConnectedHandle = true;
                            }
                        }
                    }

                    if(pEdgeObj)
                    {
                        // Suppress default connects to inside object and object center
                        bOldSuppress = pEdgeObj->GetSuppressDefaultConnect();
                        pEdgeObj->SetSuppressDefaultConnect(true);
                    }

                    if(bIsMoveOfConnectedHandle)
                    {
                        sal_uInt16 nMarkHdSiz(mpView->GetMarkHdlSizePixel());
                        Size aHalfConSiz(nMarkHdSiz + 1, nMarkHdSiz + 1);
                        aHalfConSiz = mpWindow->PixelToLogic(aHalfConSiz);

                        if(100 < aHalfConSiz.Width())
                            nX *= aHalfConSiz.Width();
                        else
                            nX *= 100;

                        if(100 < aHalfConSiz.Height())
                            nY *= aHalfConSiz.Height();
                        else
                            nY *= 100;
                    }
                    else if(rKEvt.GetKeyCode().IsMod2())
                    {
                        // move in 1 pixel distance
                        Size aLogicSizeOnePixel = mpWindow->PixelToLogic(Size(1,1));
                        nX *= aLogicSizeOnePixel.Width();
                        nY *= aLogicSizeOnePixel.Height();
                    }
                    else if(rKEvt.GetKeyCode().IsShift())
                    {
                        nX *= 1000;
                        nY *= 1000;
                    }
                    else
                    {
                        // old, fixed move distance
                        nX *= 100;
                        nY *= 100;
                    }

                    if(nullptr == pHdl)
                    {
                        // only take action when move is allowed
                        if(mpView->IsMoveAllowed())
                        {
                            // restrict movement to WorkArea
                            const ::tools::Rectangle& rWorkArea = mpView->GetWorkArea();

                            if(!rWorkArea.IsEmpty())
                            {
                                ::tools::Rectangle aMarkRect(mpView->GetMarkedObjRect());
                                aMarkRect.Move(nX, nY);

                                if(!aMarkRect.IsInside(rWorkArea))
                                {
                                    if(aMarkRect.Left() < rWorkArea.Left())
                                    {
                                        nX += rWorkArea.Left() - aMarkRect.Left();
                                    }

                                    if(aMarkRect.Right() > rWorkArea.Right())
                                    {
                                        nX -= aMarkRect.Right() - rWorkArea.Right();
                                    }

                                    if(aMarkRect.Top() < rWorkArea.Top())
                                    {
                                        nY += rWorkArea.Top() - aMarkRect.Top();
                                    }

                                    if(aMarkRect.Bottom() > rWorkArea.Bottom())
                                    {
                                        nY -= aMarkRect.Bottom() - rWorkArea.Bottom();
                                    }
                                }
                            }

                            // no handle selected
                            if(0 != nX || 0 != nY)
                            {
                                mpView->MoveAllMarked(Size(nX, nY));

                                mpView->MakeVisible(mpView->GetAllMarkedRect(), *mpWindow);
                            }
                        }
                    }
                    else
                    {
                        // move handle with index nHandleIndex
                        if (nX || nY)
                        {
                            // now move the Handle (nX, nY)
                            Point aStartPoint(pHdl->GetPos());
                            Point aEndPoint(pHdl->GetPos() + Point(nX, nY));
                            const SdrDragStat& rDragStat = mpView->GetDragStat();

                            // start dragging
                            mpView->BegDragObj(aStartPoint, nullptr, pHdl, 0);

                            if(mpView->IsDragObj())
                            {
                                bool bWasNoSnap = rDragStat.IsNoSnap();
                                bool bWasSnapEnabled = mpView->IsSnapEnabled();

                                // switch snapping off
                                if(!bWasNoSnap)
                                    const_cast<SdrDragStat&>(rDragStat).SetNoSnap();
                                if(bWasSnapEnabled)
                                    mpView->SetSnapEnabled(false);

                                mpView->MovAction(aEndPoint);
                                mpView->EndDragObj();

                                // restore snap
                                if(!bWasNoSnap)
                                    const_cast<SdrDragStat&>(rDragStat).SetNoSnap(bWasNoSnap);
                                if(bWasSnapEnabled)
                                    mpView->SetSnapEnabled(bWasSnapEnabled);
                            }

                            // make moved handle visible
                            ::tools::Rectangle aVisRect(aEndPoint - Point(100, 100), Size(200, 200));
                            mpView->MakeVisible(aVisRect, *mpWindow);
                        }
                    }

                    if(pEdgeObj)
                    {
                        // Restore original suppress value
                        pEdgeObj->SetSuppressDefaultConnect(bOldSuppress);
                    }
                }
                else
                {
                    // scroll page
                    mpViewShell->ScrollLines(nX, nY);
                }

                bReturn = true;
            }
        }
        break;
    }

    if (bReturn)
    {
        mpWindow->ReleaseMouse();
    }

    // when a text-editable object is selected and the
    // input character is printable, activate text edit on that object
    // and feed character to object
    if(!bReturn && !mpDocSh->IsReadOnly())
    {
        if (!mpView->IsTextEdit())
        {
            const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();

            if(1 == rMarkList.GetMarkCount())
            {
                SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();

                // #i118485# allow TextInput for OLEs, too
                if( dynamic_cast< const SdrTextObj *>( pObj ) !=  nullptr && pObj->HasTextEdit())
                {
                    // use common IsSimpleCharInput from the EditEngine.
                    bool bPrintable(EditEngine::IsSimpleCharInput(rKEvt));

                    if(bPrintable)
                    {
                        // try to activate textedit mode for the selected object
                        SfxStringItem aInputString(SID_ATTR_CHAR, OUString(rKEvt.GetCharCode()));

                        mpViewShell->GetViewFrame()->GetDispatcher()->ExecuteList(
                            SID_ATTR_CHAR,
                            SfxCallMode::ASYNCHRON,
                            { &aInputString });

                        // consumed
                        bReturn = true;
                    }
                }
            }
            else
            {
                // test if there is a title object there. If yes, try to
                // set it to edit mode and start typing...
                DrawViewShell* pDrawViewShell = dynamic_cast<DrawViewShell*>(mpViewShell);
                if (pDrawViewShell && EditEngine::IsSimpleCharInput(rKEvt))
                {
                    SdPage* pActualPage = pDrawViewShell->GetActualPage();
                    SdrTextObj* pCandidate = nullptr;

                    if(pActualPage)
                    {
                        SdrObjListIter aIter(pActualPage, SdrIterMode::DeepNoGroups);

                        while(aIter.IsMore() && !pCandidate)
                        {
                            SdrObject* pObj = aIter.Next();

                            if(dynamic_cast< const SdrTextObj *>( pObj ))
                            {
                                SdrInventor nInv(pObj->GetObjInventor());
                                sal_uInt16 nKnd(pObj->GetObjIdentifier());

                                if(SdrInventor::Default == nInv && OBJ_TITLETEXT == nKnd)
                                {
                                    pCandidate = static_cast<SdrTextObj*>(pObj);
                                }
                            }
                        }
                    }

                    // when candidate found and candidate is untouched, start editing text...
                    if(pCandidate && pCandidate->IsEmptyPresObj())
                    {
                        mpView->UnMarkAll();
                        mpView->MarkObj(pCandidate, mpView->GetSdrPageView());
                        SfxStringItem aInputString(SID_ATTR_CHAR, OUString(rKEvt.GetCharCode()));

                        mpViewShell->GetViewFrame()->GetDispatcher()->ExecuteList(
                            SID_ATTR_CHAR,
                            SfxCallMode::ASYNCHRON,
                            { &aInputString });

                        // consumed
                        bReturn = true;
                    }
                }
            }
        }
    }

    return bReturn;
}

bool FuPoor::MouseMove(const MouseEvent& )
{
    return false;
}

void FuPoor::SelectionHasChanged()
{
    const SdrHdlList& rHdlList = mpView->GetHdlList();
    const_cast<SdrHdlList&>(rHdlList).ResetFocusHdl();
}

/**
 * Cut object to clipboard
 */
void FuPoor::DoCut()
{
    if (mpView)
    {
        mpView->DoCut();
    }
}

/**
 * Copy object to clipboard
 */
void FuPoor::DoCopy()
{
    if (mpView)
    {
        mpView->DoCopy();
    }
}

/**
 * Paste object from clipboard
 */
void FuPoor::DoPaste()
{
    if (mpView)
    {
        mpView->DoPaste(mpWindow);
    }
}

/**
 * Paste unformatted text from clipboard
 */
void FuPoor::DoPasteUnformatted()
{
    if (mpView)
    {
        sal_Int8 nAction = DND_ACTION_COPY;
        TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( mpViewShell->GetActiveWindow() ) );
        if (aDataHelper.GetTransferable().is())
        {
            mpView->InsertData( aDataHelper,
                                mpWindow->PixelToLogic( ::tools::Rectangle( Point(), mpWindow->GetOutputSizePixel() ).Center() ),
                                nAction, false, SotClipboardFormatId::STRING);
        }
    }
}

/**
 * Timer handler for Drag&Drop
 */
IMPL_LINK_NOARG(FuPoor, DragHdl, Timer *, void)
{
    if( !mpView )
        return;

    sal_uInt16 nHitLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(HITPIX,0)).Width() );
    SdrHdl* pHdl = mpView->PickHandle(aMDPos);

    if ( pHdl==nullptr && mpView->IsMarkedHit(aMDPos, nHitLog)
         && !mpView->IsPresObjSelected(false) )
    {
        mpWindow->ReleaseMouse();
        bIsInDragMode = true;
        mpView->StartDrag( aMDPos, mpWindow );
    }
}

bool FuPoor::Command(const CommandEvent& rCEvt)
{
    return mpView->Command(rCEvt,mpWindow);
}

/**
 * Timer handler for window scrolling
 */
IMPL_LINK_NOARG(FuPoor, DelayHdl, Timer *, void)
{
    aDelayToScrollTimer.Stop ();
    bScrollable = true;

    Point aPnt(mpWindow->GetPointerPosPixel());

    // use remembered MouseButton state to create correct
    // MouseEvents for this artificial MouseMove.
    MouseMove(MouseEvent(aPnt, 1, MouseEventModifiers::NONE, GetMouseButtonCode()));
}

bool FuPoor::MouseButtonUp (const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    aDelayToScrollTimer.Stop ();
    return bScrollable  =
        bDelayActive = false;
}

bool FuPoor::MouseButtonDown(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    return false;
}

void FuPoor::StartDelayToScrollTimer ()
{
    bDelayActive = true;
    aDelayToScrollTimer.Start ();
}

bool FuPoor::RequestHelp(const HelpEvent& rHEvt)
{
    bool bReturn = false;

    SdrPageView* pPV = mpView->GetSdrPageView();

    if (pPV)
    {
        SdPage* pPage = static_cast<SdPage*>( pPV->GetPage() );

        if (pPage)
        {
            bReturn = FmFormPage::RequestHelp(mpWindow, mpView, rHEvt);
        }
    }

    return bReturn;
}

void FuPoor::ReceiveRequest(SfxRequest& /*rReq*/)
{
}

SdrObjectUniquePtr FuPoor::CreateDefaultObject(const sal_uInt16, const ::tools::Rectangle& )
{
    // empty base implementation
    return nullptr;
}

void FuPoor::ImpForceQuadratic(::tools::Rectangle& rRect)
{
    if(rRect.GetWidth() > rRect.GetHeight())
    {
        rRect = ::tools::Rectangle(
            Point(rRect.Left() + ((rRect.GetWidth() - rRect.GetHeight()) / 2), rRect.Top()),
            Size(rRect.GetHeight(), rRect.GetHeight()));
    }
    else
    {
        rRect = ::tools::Rectangle(
            Point(rRect.Left(), rRect.Top() + ((rRect.GetHeight() - rRect.GetWidth()) / 2)),
            Size(rRect.GetWidth(), rRect.GetWidth()));
    }
}

void FuPoor::SwitchLayer (sal_Int32 nOffset)
{
    auto pDrawViewShell = dynamic_cast<DrawViewShell *>( mpViewShell );
    if(!pDrawViewShell)
        return;

    // Calculate the new index.
    sal_Int32 nIndex = pDrawViewShell->GetActiveTabLayerIndex() + nOffset;

    // Make sure the new index lies inside the range of valid indices.
    if (nIndex < 0)
        nIndex = 0;
    else if (nIndex >= pDrawViewShell->GetTabLayerCount ())
        nIndex = pDrawViewShell->GetTabLayerCount() - 1;

    // Set the new active layer.
    if (nIndex != pDrawViewShell->GetActiveTabLayerIndex ())
    {
        LayerTabBar* pLayerTabControl =
            static_cast<DrawViewShell*>(mpViewShell)->GetLayerTabControl();
        if (pLayerTabControl != nullptr)
            pLayerTabControl->SendDeactivatePageEvent ();

        pDrawViewShell->SetActiveTabLayerIndex (nIndex);

        if (pLayerTabControl != nullptr)
            pLayerTabControl->SendActivatePageEvent ();
    }
}

/** is called when the current function should be aborted. <p>
    This is used when a function gets a KEY_ESCAPE but can also
    be called directly.

    @returns true if a active function was aborted
*/
bool FuPoor::cancel()
{
    if ( dynamic_cast< const FuSelection *>( this ) ==  nullptr )
    {
        mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);
        return true;
    }

    return false;
}

// #i33136#
bool FuPoor::doConstructOrthogonal() const
{
    // Check whether a media object is selected
    bool bResizeKeepRatio = false;
    // tdf#89758 Avoid interactive crop preview from being proportionally scaled by default.
    if (mpView->AreObjectsMarked() && mpView->GetDragMode() != SdrDragMode::Crop)
    {
        const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
        if (rMarkList.GetMarkCount() == 1)
        {
            sal_uInt16 aObjIdentifier = rMarkList.GetMark(0)->GetMarkedSdrObj()->GetObjIdentifier();
            bResizeKeepRatio = aObjIdentifier == OBJ_GRAF ||
                               aObjIdentifier == OBJ_MEDIA ||
                               aObjIdentifier == OBJ_OLE2;
        }
    }
    SdrHdl* pHdl = mpView->PickHandle(aMDPos);
    // Resize proportionally when media is selected and the user drags on a corner
    if (pHdl)
        bResizeKeepRatio = bResizeKeepRatio && pHdl->IsCornerHdl();

    return (
        bResizeKeepRatio ||
        SID_DRAW_XLINE == nSlotId ||
        SID_DRAW_CIRCLEARC == nSlotId ||
        SID_DRAW_SQUARE == nSlotId ||
        SID_DRAW_SQUARE_NOFILL == nSlotId ||
        SID_DRAW_SQUARE_ROUND == nSlotId ||
        SID_DRAW_SQUARE_ROUND_NOFILL == nSlotId ||
        SID_DRAW_CIRCLE == nSlotId ||
        SID_DRAW_CIRCLE_NOFILL == nSlotId ||
        SID_DRAW_CIRCLEPIE == nSlotId ||
        SID_DRAW_CIRCLEPIE_NOFILL == nSlotId ||
        SID_DRAW_CIRCLECUT == nSlotId ||
        SID_DRAW_CIRCLECUT_NOFILL == nSlotId ||
        SID_DRAW_XPOLYGON == nSlotId ||
        SID_DRAW_XPOLYGON_NOFILL == nSlotId ||
        SID_3D_CUBE == nSlotId ||
        SID_3D_SPHERE == nSlotId ||
        SID_3D_SHELL == nSlotId ||
        SID_3D_HALF_SPHERE == nSlotId ||
        SID_3D_TORUS == nSlotId ||
        SID_3D_CYLINDER == nSlotId ||
        SID_3D_CONE == nSlotId ||
        SID_3D_PYRAMID == nSlotId);
}

void FuPoor::DoExecute( SfxRequest& )
{
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
