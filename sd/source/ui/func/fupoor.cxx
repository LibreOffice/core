/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "fupoor.hxx"

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
#include <vcl/dialog.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XLayer.hpp>
#include <com/sun/star/drawing/XLayerManager.hpp>
#include <com/sun/star/container/XChild.hpp>

#include "FrameView.hxx"
#include "app.hrc"
#include "fusel.hxx"
#include "sdpage.hxx"
#include "drawview.hxx"
#include "DrawViewShell.hxx"
#include "Window.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "zoomlist.hxx"
#include "Client.hxx"
#include "slideshow.hxx"
#include "LayerTabBar.hxx"

#include <sfx2/viewfrm.hxx>

#include <svx/svditer.hxx>

#include <editeng/editeng.hxx>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;

namespace sd {

TYPEINIT0( FuPoor );


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
      nSlotValue(0),
      pDialog(NULL),
      bIsInDragMode(sal_False),
      bNoScrollUntilInside (sal_True),
      bScrollable (sal_False),
      bDelayActive (sal_False),
      bFirstMouseMove (sal_False),
      
      mnCode(0)
{
    ReceiveRequest(rReq);

    aScrollTimer.SetTimeoutHdl( LINK(this, FuPoor, ScrollHdl) );
    aScrollTimer.SetTimeout(SELENG_AUTOREPEAT_INTERVAL);

    aDragTimer.SetTimeoutHdl( LINK(this, FuPoor, DragHdl) );
    aDragTimer.SetTimeout(SELENG_DRAGDROP_TIMEOUT);

    aDelayToScrollTimer.SetTimeoutHdl( LINK(this, FuPoor, DelayHdl) );
    aDelayToScrollTimer.SetTimeout(2000);
}


FuPoor::~FuPoor()
{
    aDragTimer.Stop();
    aScrollTimer.Stop();
    aDelayToScrollTimer.Stop ();

    if (pDialog)
        delete pDialog;
}


void FuPoor::Activate()
{
    if (pDialog)
    {
        pDialog->Show();
    }
}


void FuPoor::Deactivate()
{
    aDragTimer.Stop();
    aScrollTimer.Stop();
    aDelayToScrollTimer.Stop ();
        bScrollable  =
        bDelayActive = sal_False;

    if (pDialog)
    {
        pDialog->Hide();
    }

    if (mpWindow) mpWindow->ReleaseMouse ();
}

/**
 * scroll when approached the border of the window; is called by MouseMove
 */
void FuPoor::ForceScroll(const Point& aPixPos)
{
    aScrollTimer.Stop();

    if ( !mpView->IsDragHelpLine() && !mpView->IsSetPageOrg() &&
            !SlideShow::IsRunning( mpViewShell->GetViewShellBase() ) )
    {
        Point aPos = mpWindow->OutputToScreenPixel(aPixPos);
        const Rectangle& rRect = mpViewShell->GetAllWindowRect();

        if ( bNoScrollUntilInside )
        {
            if ( rRect.IsInside(aPos) )
                bNoScrollUntilInside = sal_False;
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
                    
                    mpViewShell->ScrollLines(dx, dy);
                    aScrollTimer.Start();
                }
                else if (! bDelayActive) StartDelayToScrollTimer ();
            }
        }
    }
}

/**
 * timer handler for window scrolling
 */
IMPL_LINK_NOARG_INLINE_START(FuPoor, ScrollHdl)
{
    Point aPnt(mpWindow->GetPointerPosPixel());

    
    
    MouseMove(MouseEvent(aPnt, 1, 0, GetMouseButtonCode()));

    return 0;
}
IMPL_LINK_INLINE_END( FuPoor, ScrollHdl, Timer *, pTimer )

/**
 * handle keyboard events
 * @returns sal_True if the event was handled, sal_False otherwise
 */
sal_Bool FuPoor::KeyInput(const KeyEvent& rKEvt)
{
    sal_uInt16          nCode = rKEvt.GetKeyCode().GetCode();
    sal_Bool            bReturn = sal_False;
    sal_Bool            bSlideShow = SlideShow::IsRunning( mpViewShell->GetViewShellBase() );

    switch (nCode)
    {
        case KEY_RETURN:
        {
            if(rKEvt.GetKeyCode().IsMod1())
            {
                if(mpViewShell && mpViewShell->ISA(DrawViewShell))
                {
                    DrawViewShell* pDrawViewShell =
                        static_cast<DrawViewShell*>(mpViewShell);
                    SdPage* pActualPage = pDrawViewShell->GetActualPage();
                    SdrTextObj* pCandidate = 0L;

                    if(pActualPage)
                    {
                        SdrObjListIter aIter(*pActualPage, IM_DEEPNOGROUPS);

                        while(aIter.IsMore() && !pCandidate)
                        {
                            SdrObject* pObj = aIter.Next();

                            if(pObj && pObj->ISA(SdrTextObj))
                            {
                                sal_uInt32 nInv(pObj->GetObjInventor());
                                sal_uInt16 nKnd(pObj->GetObjIdentifier());

                                if(SdrInventor == nInv &&
                                    (OBJ_TITLETEXT == nKnd || OBJ_OUTLINETEXT == nKnd || OBJ_TEXT == nKnd))
                                {
                                    pCandidate = (SdrTextObj*)pObj;
                                }
                            }
                        }
                    }

                    if(pCandidate)
                    {
                        mpView->UnMarkAll();
                        mpView->MarkObj(pCandidate, mpView->GetSdrPageView());

                        mpViewShell->GetViewFrame()->GetDispatcher()->Execute(
                            SID_ATTR_CHAR, SFX_CALLMODE_ASYNCHRON);
                    }
                    else
                    {
                        
                        mpViewShell->GetViewFrame()->GetDispatcher()->Execute(
                            SID_INSERTPAGE_QUICK, SFX_CALLMODE_ASYNCHRON);
                    }

                    
                    bReturn = sal_True;
                }
            }
            else
            {
                
                
                const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();

                if( !mpView->IsTextEdit() && 1 == rMarkList.GetMarkCount() )
                {
                    SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

                    if( pObj && pObj->ISA( SdrOle2Obj ) && !mpDocSh->IsUIActive() )
                    {
                        
                        mpViewShell->ActivateObject( static_cast< SdrOle2Obj* >( pObj ), 0 );
                    }
                    else if( pObj && pObj->IsEmptyPresObj() && pObj->ISA( SdrGrafObj ) )
                    {
                        mpViewShell->GetViewFrame()->GetDispatcher()->Execute( SID_INSERT_GRAPHIC, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
                    }
                    else
                    {
                        mpViewShell->GetViewFrame()->GetDispatcher()->Execute( SID_ATTR_CHAR, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
                    }

                    
                    bReturn = sal_True;
                }
            }
        }
        break;

        case KEY_TAB:
        {
            
            if(rKEvt.GetKeyCode().IsMod1() || rKEvt.GetKeyCode().IsMod2())
            {
                
                const SdrHdlList& rHdlList = mpView->GetHdlList();
                sal_Bool bForward(!rKEvt.GetKeyCode().IsShift());

                ((SdrHdlList&)rHdlList).TravelFocusHdl(bForward);

                
                SdrHdl* pHdl = rHdlList.GetFocusHdl();

                if(pHdl)
                {
                    Point aHdlPosition(pHdl->GetPos());
                    Rectangle aVisRect(aHdlPosition - Point(100, 100), Size(200, 200));
                    mpView->MakeVisible(aVisRect, *mpWindow);
                }

                
                bReturn = sal_True;
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
                
                mpViewShell->SetZoom(mpWindow->GetZoom() * 3 / 2);

                if (mpViewShell->ISA(DrawViewShell))
                    static_cast<DrawViewShell*>(mpViewShell)
                        ->SetZoomOnPage(sal_False);

                bReturn = sal_True;
            }
        }
        break;

        case KEY_SUBTRACT:
        {
            if (!mpView->IsTextEdit() && !bSlideShow && !mpDocSh->IsUIActive())
            {
                
                mpViewShell->SetZoom(mpWindow->GetZoom() * 2 / 3);

                if (mpViewShell->ISA(DrawViewShell))
                    static_cast<DrawViewShell*>(mpViewShell)
                        ->SetZoomOnPage(sal_False);

                bReturn = sal_True;
            }
        }
        break;

        case KEY_MULTIPLY:
        {
            if (!mpView->IsTextEdit() && !bSlideShow)
            {
                
                mpViewShell->GetViewFrame()->GetDispatcher()->
                Execute(SID_SIZE_PAGE, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
                bReturn = sal_True;
            }
        }
        break;

        case KEY_DIVIDE:
        {
            if (!mpView->IsTextEdit() && !bSlideShow)
            {
                
                mpViewShell->GetViewFrame()->GetDispatcher()->
                Execute(SID_SIZE_OPTIMAL, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
                bReturn = sal_True;
            }
        }
        break;

        case KEY_POINT:
        {
            ZoomList* pZoomList = mpViewShell->GetZoomList();

            if (!mpView->IsTextEdit() && pZoomList->IsNextPossible() && !bSlideShow && !mpDocSh->IsUIActive())
            {
                
                mpViewShell->SetZoomRect(pZoomList->GetNextZoomRect());
                bReturn = sal_True;
            }
        }
        break;

        case KEY_COMMA:
        {
            ZoomList* pZoomList = mpViewShell->GetZoomList();

            if (!mpView->IsTextEdit() && pZoomList->IsPreviousPossible() && !bSlideShow && !mpDocSh->IsUIActive())
            {
                
                mpViewShell->SetZoomRect(pZoomList->GetPreviousZoomRect());
                bReturn = sal_True;
            }
        }
        break;

        case KEY_HOME:
        {
            if (!mpView->IsTextEdit()
                && mpViewShell->ISA(DrawViewShell)
                && !bSlideShow)
            {
               
               static_cast<DrawViewShell*>(mpViewShell)->SwitchPage(0);
               bReturn = sal_True;
            }
        }
        break;

        case KEY_END:
        {
            if (!mpView->IsTextEdit()
                && mpViewShell->ISA(DrawViewShell)
                && !bSlideShow)
            {
                
                SdPage* pPage =
                    static_cast<DrawViewShell*>(mpViewShell)->GetActualPage();
                static_cast<DrawViewShell*>(mpViewShell)
                    ->SwitchPage(mpDoc->GetSdPageCount(
                        pPage->GetPageKind()) - 1);
                bReturn = sal_True;
            }
        }
        break;

        case KEY_PAGEUP:
        {
            if( rKEvt.GetKeyCode().IsMod1() && rKEvt.GetKeyCode().IsMod2() )
                break;

            if(mpViewShell->ISA(DrawViewShell) && !bSlideShow)
            {
                
                
                if ( ! rKEvt.GetKeyCode().GetAllModifier())
                {
                    
                    
                    mpView->SdrEndTextEdit();

                    
                    bReturn = sal_True;
                    SdPage* pPage = static_cast<DrawViewShell*>(mpViewShell)->GetActualPage();
                    sal_uInt16 nSdPage = (pPage->GetPageNum() - 1) / 2;

                    if (nSdPage > 0)
                    {
                        
                        
                        
                        TabControl* pPageTabControl =
                            static_cast<DrawViewShell*>(mpViewShell)
                            ->GetPageTabControl();
                        if (pPageTabControl->IsReallyShown())
                            pPageTabControl->SendDeactivatePageEvent ();
                        static_cast<DrawViewShell*>(mpViewShell)->SwitchPage(nSdPage - 1);
                        if (pPageTabControl->IsReallyShown())
                            pPageTabControl->SendActivatePageEvent ();
                    }
                }
                else if (rKEvt.GetKeyCode().IsMod1())
                {
                    
                    if (static_cast<DrawViewShell*>(mpViewShell)->IsLayerModeActive())
                    {
                        
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
            if(mpViewShell->ISA(DrawViewShell) && !bSlideShow)
            {
                
                
                if ( ! rKEvt.GetKeyCode().GetAllModifier())
                {
                    
                    mpView->SdrEndTextEdit();

                    
                    bReturn = sal_True;
                    SdPage* pPage = static_cast<DrawViewShell*>(mpViewShell)->GetActualPage();
                    sal_uInt16 nSdPage = (pPage->GetPageNum() - 1) / 2;

                    if (nSdPage < mpDoc->GetSdPageCount(pPage->GetPageKind()) - 1)
                    {
                        
                        
                        
                        TabControl* pPageTabControl =
                            static_cast<DrawViewShell*>(mpViewShell)->GetPageTabControl();
                        if (pPageTabControl->IsReallyShown())
                            pPageTabControl->SendDeactivatePageEvent ();
                        static_cast<DrawViewShell*>(mpViewShell)->SwitchPage(nSdPage + 1);
                        if (pPageTabControl->IsReallyShown())
                            pPageTabControl->SendActivatePageEvent ();
                    }
                }
                else if (rKEvt.GetKeyCode().IsMod1())
                {
                    
                    if (static_cast<DrawViewShell*>(mpViewShell)->IsLayerModeActive())
                    {
                        
                        
                        SwitchLayer (+1);
                    }
                }
            }
        }
        break;

        
        case KEY_SPACE:
        {
            const SdrHdlList& rHdlList = mpView->GetHdlList();
            SdrHdl* pHdl = rHdlList.GetFocusHdl();

            if(pHdl)
            {
                if(pHdl->GetKind() == HDL_POLY)
                {
                    
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

                    if(0L == rHdlList.GetFocusHdl())
                    {
                        
                        SdrHdl* pNewOne = 0L;

                        for(sal_uInt32 a(0); !pNewOne && a < rHdlList.GetHdlCount(); a++)
                        {
                            SdrHdl* pAct = rHdlList.GetHdl(a);

                            if(pAct
                                && pAct->GetKind() == HDL_POLY
                                && pAct->GetPolyNum() == nPol
                                && pAct->GetPointNum() == nPnt)
                            {
                                pNewOne = pAct;
                            }
                        }

                        if(pNewOne)
                        {
                            ((SdrHdlList&)rHdlList).SetFocusHdl(pNewOne);
                        }
                    }

                    bReturn = sal_True;
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
                    
                    nX = 0;
                    nY =-1;
                }
                else if (nCode == KEY_DOWN)
                {
                    
                    nX = 0;
                    nY = 1;
                }
                else if (nCode == KEY_LEFT)
                {
                    
                    nX =-1;
                    nY = 0;
                }
                else if (nCode == KEY_RIGHT)
                {
                    
                    nX = 1;
                    nY = 0;
                }

                if (mpView->AreObjectsMarked() && !rKEvt.GetKeyCode().IsMod1() &&
                    !mpDocSh->IsReadOnly())
                {
                    const SdrHdlList& rHdlList = mpView->GetHdlList();
                    SdrHdl* pHdl = rHdlList.GetFocusHdl();

                    sal_Bool bIsMoveOfConnectedHandle(sal_False);
                    sal_Bool bOldSuppress = false;
                    SdrEdgeObj* pEdgeObj = 0L;

                    if(pHdl && pHdl->GetObj() && pHdl->GetObj()->ISA(SdrEdgeObj) && 0 == pHdl->GetPolyNum())
                    {
                        pEdgeObj = (SdrEdgeObj*)pHdl->GetObj();

                        if(0L == pHdl->GetPointNum())
                        {
                            if(pEdgeObj->GetConnection(true).GetObject())
                            {
                                bIsMoveOfConnectedHandle = sal_True;
                            }
                        }
                        if(1L == pHdl->GetPointNum())
                        {
                            if(pEdgeObj->GetConnection(false).GetObject())
                            {
                                bIsMoveOfConnectedHandle = sal_True;
                            }
                        }
                    }

                    if(pEdgeObj)
                    {
                        
                        bOldSuppress = pEdgeObj->GetSuppressDefaultConnect();
                        pEdgeObj->SetSuppressDefaultConnect(sal_True);
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
                        
                        Size aLogicSizeOnePixel = (mpWindow) ? mpWindow->PixelToLogic(Size(1,1)) : Size(100, 100);
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
                        
                        nX *= 100;
                        nY *= 100;
                    }

                    if(0L == pHdl)
                    {
                        
                        if(mpView->IsMoveAllowed())
                        {
                            
                            const Rectangle& rWorkArea = mpView->GetWorkArea();

                            if(!rWorkArea.IsEmpty())
                            {
                                Rectangle aMarkRect(mpView->GetMarkedObjRect());
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

                            
                            if(0 != nX || 0 != nY)
                            {
                                mpView->MoveAllMarked(Size(nX, nY));

                                mpView->MakeVisible(mpView->GetAllMarkedRect(), *mpWindow);
                            }
                        }
                    }
                    else
                    {
                        
                        if(pHdl && (nX || nY))
                        {
                            
                            Point aStartPoint(pHdl->GetPos());
                            Point aEndPoint(pHdl->GetPos() + Point(nX, nY));
                            const SdrDragStat& rDragStat = mpView->GetDragStat();

                            
                            mpView->BegDragObj(aStartPoint, 0, pHdl, 0);

                            if(mpView->IsDragObj())
                            {
                                bool bWasNoSnap = rDragStat.IsNoSnap();
                                sal_Bool bWasSnapEnabled = mpView->IsSnapEnabled();

                                
                                if(!bWasNoSnap)
                                    ((SdrDragStat&)rDragStat).SetNoSnap(true);
                                if(bWasSnapEnabled)
                                    mpView->SetSnapEnabled(sal_False);

                                mpView->MovAction(aEndPoint);
                                mpView->EndDragObj();

                                
                                if(!bWasNoSnap)
                                    ((SdrDragStat&)rDragStat).SetNoSnap(bWasNoSnap);
                                if(bWasSnapEnabled)
                                    mpView->SetSnapEnabled(bWasSnapEnabled);
                            }

                            
                            Rectangle aVisRect(aEndPoint - Point(100, 100), Size(200, 200));
                            mpView->MakeVisible(aVisRect, *mpWindow);
                        }
                    }

                    if(pEdgeObj)
                    {
                        
                        pEdgeObj->SetSuppressDefaultConnect(bOldSuppress);
                    }
                }
                else
                {
                    
                    mpViewShell->ScrollLines(nX, nY);
                }

                bReturn = sal_True;
            }
        }
        break;
    }

    if (bReturn)
    {
        mpWindow->ReleaseMouse();
    }

    
    
    
    if(!bReturn && !mpDocSh->IsReadOnly())
    {
        if(!mpView->IsTextEdit() && mpViewShell)
        {
            const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();

            if(1 == rMarkList.GetMarkCount())
            {
                SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();

                
                if(pObj->ISA(SdrTextObj) && pObj->HasTextEdit())
                {
                    
                    sal_Bool bPrintable(EditEngine::IsSimpleCharInput(rKEvt));

                    if(bPrintable)
                    {
                        
                        SfxStringItem aInputString(SID_ATTR_CHAR, OUString(rKEvt.GetCharCode()));

                        mpViewShell->GetViewFrame()->GetDispatcher()->Execute(
                            SID_ATTR_CHAR,
                            SFX_CALLMODE_ASYNCHRON,
                            &aInputString,
                            0L);

                        
                        bReturn = sal_True;
                    }
                }
            }
            else
            {
                
                
                if(mpViewShell->ISA(DrawViewShell)
                    && EditEngine::IsSimpleCharInput(rKEvt))
                {
                    DrawViewShell* pDrawViewShell =
                        static_cast<DrawViewShell*>(mpViewShell);
                    SdPage* pActualPage = pDrawViewShell->GetActualPage();
                    SdrTextObj* pCandidate = 0L;

                    if(pActualPage)
                    {
                        SdrObjListIter aIter(*pActualPage, IM_DEEPNOGROUPS);

                        while(aIter.IsMore() && !pCandidate)
                        {
                            SdrObject* pObj = aIter.Next();

                            if(pObj && pObj->ISA(SdrTextObj))
                            {
                                sal_uInt32 nInv(pObj->GetObjInventor());
                                sal_uInt16 nKnd(pObj->GetObjIdentifier());

                                if(SdrInventor == nInv && OBJ_TITLETEXT == nKnd)
                                {
                                    pCandidate = (SdrTextObj*)pObj;
                                }
                            }
                        }
                    }

                    
                    if(pCandidate && pCandidate->IsEmptyPresObj())
                    {
                        mpView->UnMarkAll();
                        mpView->MarkObj(pCandidate, mpView->GetSdrPageView());
                        SfxStringItem aInputString(SID_ATTR_CHAR, OUString(rKEvt.GetCharCode()));

                        mpViewShell->GetViewFrame()->GetDispatcher()->Execute(
                            SID_ATTR_CHAR,
                            SFX_CALLMODE_ASYNCHRON,
                            &aInputString,
                            0L);

                        
                        bReturn = sal_True;
                    }
                }
            }
        }
    }

    return(bReturn);
}

sal_Bool FuPoor::MouseMove(const MouseEvent& )
{
    return sal_False;
}

void FuPoor::SelectionHasChanged()
{
    const SdrHdlList& rHdlList = mpView->GetHdlList();
    ((SdrHdlList&)rHdlList).ResetFocusHdl();
}

/**
 * Cut object to clipboard
 */
void FuPoor::DoCut()
{
    if (mpView)
    {
        mpView->DoCut(mpWindow);
    }
}

/**
 * Copy object to clipboard
 */
void FuPoor::DoCopy()
{
    if (mpView)
    {
        mpView->DoCopy(mpWindow);
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
                                mpWindow->PixelToLogic( Rectangle( Point(), mpWindow->GetOutputSizePixel() ).Center() ),
                                nAction, sal_False, FORMAT_STRING);
        }
    }
}

/**
 * Timer handler for Drag&Drop
 */
IMPL_LINK_NOARG(FuPoor, DragHdl)
{
    if( mpView )
    {
        sal_uInt16 nHitLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(HITPIX,0)).Width() );
        SdrHdl* pHdl = mpView->PickHandle(aMDPos);

        if ( pHdl==NULL && mpView->IsMarkedHit(aMDPos, nHitLog)
             && !mpView->IsPresObjSelected(sal_False, sal_True) )
        {
            mpWindow->ReleaseMouse();
            bIsInDragMode = sal_True;
            mpView->StartDrag( aMDPos, mpWindow );
        }
    }
    return 0;
}

sal_Bool FuPoor::Command(const CommandEvent& rCEvt)
{
    return( mpView->Command(rCEvt,mpWindow) );
}

/**
 * Timer handler for window scrolling
 */
IMPL_LINK_NOARG_INLINE_START(FuPoor, DelayHdl)
{
    aDelayToScrollTimer.Stop ();
    bScrollable = sal_True;

    Point aPnt(mpWindow->GetPointerPosPixel());

    
    
    MouseMove(MouseEvent(aPnt, 1, 0, GetMouseButtonCode()));

    return 0;
}
IMPL_LINK_INLINE_END( FuPoor, DelayHdl, Timer *, pTimer )

sal_Bool FuPoor::MouseButtonUp (const MouseEvent& rMEvt)
{
    
    SetMouseButtonCode(rMEvt.GetButtons());

    aDelayToScrollTimer.Stop ();
    return bScrollable  =
        bDelayActive = sal_False;
}

sal_Bool FuPoor::MouseButtonDown(const MouseEvent& rMEvt)
{
    
    SetMouseButtonCode(rMEvt.GetButtons());

    return sal_False;
}

void FuPoor::StartDelayToScrollTimer ()
{
    bDelayActive = sal_True;
    aDelayToScrollTimer.Start ();
}

sal_Bool FuPoor::RequestHelp(const HelpEvent& rHEvt)
{
    sal_Bool bReturn = sal_False;

    SdrPageView* pPV = mpView->GetSdrPageView();

    if (pPV)
    {
        SdPage* pPage = (SdPage*) pPV->GetPage();

        if (pPage)
        {
            bReturn = pPage->RequestHelp(mpWindow, mpView, rHEvt);
        }
    }

    return(bReturn);
}

void FuPoor::Paint(const Rectangle&, ::sd::Window* )
{
}

void FuPoor::ReceiveRequest(SfxRequest& rReq)
{
    const SfxItemSet* pSet = rReq.GetArgs();

    if (pSet)
    {
        if( pSet->GetItemState( nSlotId ) == SFX_ITEM_SET )
        {
            const SfxPoolItem& rItem = pSet->Get( nSlotId );

            if( rItem.ISA( SfxAllEnumItem ) )
            {
                nSlotValue = ( ( const SfxAllEnumItem& ) rItem ).GetValue();
            }
        }
    }
}

SdrObject* FuPoor::CreateDefaultObject(const sal_uInt16, const Rectangle& )
{
    
    return 0L;
}

void FuPoor::ImpForceQuadratic(Rectangle& rRect)
{
    if(rRect.GetWidth() > rRect.GetHeight())
    {
        rRect = Rectangle(
            Point(rRect.Left() + ((rRect.GetWidth() - rRect.GetHeight()) / 2), rRect.Top()),
            Size(rRect.GetHeight(), rRect.GetHeight()));
    }
    else
    {
        rRect = Rectangle(
            Point(rRect.Left(), rRect.Top() + ((rRect.GetHeight() - rRect.GetWidth()) / 2)),
            Size(rRect.GetWidth(), rRect.GetWidth()));
    }
}




void FuPoor::SwitchLayer (sal_Int32 nOffset)
{
    if(mpViewShell && mpViewShell->ISA(DrawViewShell))
    {
        DrawViewShell* pDrawViewShell =
            static_cast<DrawViewShell*>(mpViewShell);

        
        sal_Int32 nIndex = pDrawViewShell->GetActiveTabLayerIndex() + nOffset;

        
        if (nIndex < 0)
            nIndex = 0;
        else if (nIndex >= pDrawViewShell->GetTabLayerCount ())
            nIndex = pDrawViewShell->GetTabLayerCount() - 1;

        
        if (nIndex != pDrawViewShell->GetActiveTabLayerIndex ())
        {
            LayerTabBar* pLayerTabControl =
                static_cast<DrawViewShell*>(mpViewShell)->GetLayerTabControl();
            if (pLayerTabControl != NULL)
                pLayerTabControl->SendDeactivatePageEvent ();

            pDrawViewShell->SetActiveTabLayerIndex (nIndex);

            if (pLayerTabControl != NULL)
                pLayerTabControl->SendActivatePageEvent ();
        }
    }
}

/** is called when the currenct function should be aborted. <p>
    This is used when a function gets a KEY_ESCAPE but can also
    be called directly.

    @returns true if a active function was aborted
*/
bool FuPoor::cancel()
{
    if ( !this->ISA(FuSelection) )
    {
        mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);
        return true;
    }

    return false;
}


bool FuPoor::doConstructOrthogonal() const
{
    return (
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

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
