/*************************************************************************
 *
 *  $RCSfile: fupoor.cxx,v $
 *
 *  $Revision: 1.33 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:05:15 $
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

#pragma hdrstop

#include "fupoor.hxx"

#include <svx/svxids.hrc>

#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _SVDOOLE2_HXX
#include <svx/svdoole2.hxx>
#endif
#ifndef _SVDOGRAF_HXX
#include <svx/svdograf.hxx>
#endif
#ifndef _SV_SELENG_HXX //autogen
#include <vcl/seleng.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _SFXSTBMGR_HXX //autogen
#include <sfx2/stbmgr.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XLAYER_HPP_
#include <com/sun/star/drawing/XLayer.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XLAYERMANAGER_HPP_
#include <com/sun/star/drawing/XLayerManager.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif

#ifndef SD_FRAMW_VIEW_HXX
#include "FrameView.hxx"
#endif
#include "app.hrc"
#ifndef SD_FU_SELECTION_HXX
#include "fusel.hxx"
#endif
#include "sdpage.hxx"
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#ifndef SD_WINDOW_SHELL_HXX
#include "Window.hxx"
#endif
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "zoomlist.hxx"
#ifndef SD_CLIENT_HXX
#include "Client.hxx"
#endif
#ifndef SD_FU_SLIDE_SHOW_HXX
#include "fuslshow.hxx"
#endif

// #97016# IV
#ifndef _SVDITER_HXX
#include <svx/svditer.hxx>
#endif

// #98533#
#ifndef _MyEDITENG_HXX
#include <svx/editeng.hxx>
#endif

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;

namespace sd {

TYPEINIT0( FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuPoor::FuPoor (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDrDoc,
    SfxRequest& rReq)
    : pViewShell(pViewSh),
      pWindow(pWin),
      pView(pView),
      pDoc(pDrDoc),
      pDocSh( pDrDoc->GetDocSh() ),
      nSlotId( rReq.GetSlot() ),
      pDialog(NULL),
      bIsInDragMode(FALSE),
      bScrollable (FALSE),
      bDelayActive (FALSE),
      bNoScrollUntilInside (TRUE),
      nSlotValue(0),
      // #95491# remember MouseButton state
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

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuPoor::~FuPoor()
{
    aDragTimer.Stop();
    aScrollTimer.Stop();
    aDelayToScrollTimer.Stop ();

    if (pDialog)
        delete pDialog;
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuPoor::Activate()
{
    if (pDialog)
    {
        pDialog->Show();
    }
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuPoor::Deactivate()
{
    aDragTimer.Stop();
    aScrollTimer.Stop();
    aDelayToScrollTimer.Stop ();
        bScrollable  =
        bDelayActive = FALSE;

    if (pDialog)
    {
        pDialog->Hide();
    }

    if (pWindow) pWindow->ReleaseMouse ();
}

/*************************************************************************
|*
|* Scrollen bei Erreichen des Fensterrandes; wird von
|* MouseMove aufgerufen
|*
\************************************************************************/

void FuPoor::ForceScroll(const Point& aPixPos)
{
    aScrollTimer.Stop();

    if ( !pView->IsDragHelpLine() && !pView->IsSetPageOrg() &&
         !pViewShell->GetSlideShow() )
    {
/*      Size aSize = pWindow->GetSizePixel();
        short dx = 0, dy = 0;

        if ( aPixPos.X() <= 0              ) dx = -1;
        if ( aPixPos.X() >= aSize.Width()  ) dx =  1;
        if ( aPixPos.Y() <= 0              ) dy = -1;
        if ( aPixPos.Y() >= aSize.Height() ) dy =  1;
*/
        Point aPos = pWindow->OutputToScreenPixel(aPixPos);
        const Rectangle& rRect = pViewShell->GetAllWindowRect();

        if ( bNoScrollUntilInside )
        {
            if ( rRect.IsInside(aPos) )
                bNoScrollUntilInside = FALSE;
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
                    // Scrollaktion in abgeleiteter Klasse
                    ScrollStart();
                    pViewShell->ScrollLines(dx, dy);
                    ScrollEnd();
                    aScrollTimer.Start();
                }
                else if (! bDelayActive) StartDelayToScrollTimer ();
            }
        }
    }
}

/*************************************************************************
|*
|* Timer-Handler fuer Fensterscrolling
|*
\************************************************************************/

IMPL_LINK_INLINE_START( FuPoor, ScrollHdl, Timer *, pTimer )
{
    Point aPnt(pWindow->GetPointerPosPixel());

    // #95491# use remembered MouseButton state to create correct
    // MouseEvents for this artifical MouseMove.
    MouseMove(MouseEvent(aPnt, 1, 0, GetMouseButtonCode()));

    return 0;
}
IMPL_LINK_INLINE_END( FuPoor, ScrollHdl, Timer *, pTimer )

/*************************************************************************
|*
|* String in Applikations-Statuszeile ausgeben
|*
\************************************************************************/

void FuPoor::WriteStatus(const String& aStr)
{
/*
//  SFX_APP()->SetHelpText(aStr);
    SfxStatusBarManager* pStatBarMan = SFX_APP()->GetStatusBarManager();
    pStatBarMan->ShowHelpText(aStr);
*/
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert TRUE, andernfalls
|* FALSE.
|*
\************************************************************************/

BOOL FuPoor::KeyInput(const KeyEvent& rKEvt)
{
    USHORT          nCode = rKEvt.GetKeyCode().GetCode();
    BOOL            bReturn = FALSE;
    BOOL            bSlideShow = FALSE;
     FuSlideShow*    pFuSlideShow = pViewShell->GetSlideShow();

    if( pFuSlideShow )
        bSlideShow = TRUE;

    switch (nCode)
    {
        // #97016# IV
        case KEY_RETURN:
        {
            if(rKEvt.GetKeyCode().IsMod1())
            {
                if(pViewShell && pViewShell->ISA(DrawViewShell))
                {
                    DrawViewShell* pDrawViewShell =
                        static_cast<DrawViewShell*>(pViewShell);
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
                        pView->UnMarkAll();
                        pView->MarkObj(pCandidate, pView->GetPageViewPvNum(0));

                        pViewShell->GetViewFrame()->GetDispatcher()->Execute(
                            SID_ATTR_CHAR, SFX_CALLMODE_ASYNCHRON);
                    }
                    else
                    {
                        // insert a new page with the same page layout
                        pViewShell->GetViewFrame()->GetDispatcher()->Execute(
                            SID_INSERTPAGE_QUICK, SFX_CALLMODE_ASYNCHRON);
                    }

                    // consumed
                    bReturn = TRUE;
                }
            }
            else
            {
                // #98255# activate OLE object on RETURN for selected object
                // #98198# activate text edit on RETURN for selected object
                const SdrMarkList& rMarkList = pView->GetMarkedObjectList();

                if( !pView->IsTextEdit() && 1 == rMarkList.GetMarkCount() )
                {
                    SdrObject* pObj = rMarkList.GetMark( 0 )->GetObj();

                    if( pObj && pObj->ISA( SdrOle2Obj ) && !pDocSh->IsUIActive() )
                    {
                        pView->HideMarkHdl(NULL);
                        pViewShell->ActivateObject( static_cast< SdrOle2Obj* >( pObj ), 0 );
                    }
                    else if( pObj && pObj->IsEmptyPresObj() && pObj->ISA( SdrGrafObj ) )
                    {
                        pViewShell->GetViewFrame()->GetDispatcher()->Execute( SID_INSERT_GRAPHIC, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
                    }
                    else
                    {
                        pViewShell->GetViewFrame()->GetDispatcher()->Execute( SID_ATTR_CHAR, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
                    }

                    // consumed
                    bReturn = TRUE;
                }
            }
        }
        break;

        // #97016# II
        case KEY_TAB:
        {
            // #98994# handle Mod1 and Mod2 to get travelling running on different systems
            if(rKEvt.GetKeyCode().IsMod1() || rKEvt.GetKeyCode().IsMod2())
            {
                // #97016# II do something with a selected handle?
                const SdrHdlList& rHdlList = pView->GetHdlList();
                sal_Bool bForward(!rKEvt.GetKeyCode().IsShift());

                ((SdrHdlList&)rHdlList).TravelFocusHdl(bForward);

                // guarantee visibility of focused handle
                SdrHdl* pHdl = rHdlList.GetFocusHdl();

                if(pHdl)
                {
                    Point aHdlPosition(pHdl->GetPos());
                    Rectangle aVisRect(aHdlPosition - Point(100, 100), Size(200, 200));
                    pView->MakeVisible(aVisRect, *pWindow);
                }

                // consumed
                bReturn = TRUE;
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
            if (!pView->IsTextEdit() && !bSlideShow && !pDocSh->IsUIActive())
            {
                // Zoom vergroessern
                pViewShell->SetZoom(pWindow->GetZoom() * 3 / 2);

                if (pViewShell->ISA(DrawViewShell))
                    static_cast<DrawViewShell*>(pViewShell)
                        ->SetZoomOnPage(FALSE);

                bReturn = TRUE;
            }
        }
        break;

        case KEY_SUBTRACT:
        {
            if (!pView->IsTextEdit() && !bSlideShow && !pDocSh->IsUIActive())
            {
                // Zoom verringern
                pViewShell->SetZoom(pWindow->GetZoom() * 2 / 3);

                if (pViewShell->ISA(DrawViewShell))
                    static_cast<DrawViewShell*>(pViewShell)
                        ->SetZoomOnPage(FALSE);

                bReturn = TRUE;
            }
        }
        break;

        case KEY_MULTIPLY:
        {
            if (!pView->IsTextEdit() && !bSlideShow)
            {
                // Zoom auf Seite
                pViewShell->GetViewFrame()->GetDispatcher()->
                Execute(SID_SIZE_PAGE, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
                bReturn = TRUE;
            }
        }
        break;

        case KEY_DIVIDE:
        {
            if (!pView->IsTextEdit() && !bSlideShow)
            {
                // Zoom auf selektierte Objekte
                pViewShell->GetViewFrame()->GetDispatcher()->
                Execute(SID_SIZE_OPTIMAL, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
                bReturn = TRUE;
            }
        }
        break;

        case KEY_POINT:
        {
            ZoomList* pZoomList = pViewShell->GetZoomList();

            if (!pView->IsTextEdit() && pZoomList->IsNextPossible() && !bSlideShow && !pDocSh->IsUIActive())
            {
                // Naechstes ZoomRect einstellen
                pViewShell->SetZoomRect(pZoomList->GetNextZoomRect());
                bReturn = TRUE;
            }
        }
        break;

        case KEY_COMMA:
        {
            ZoomList* pZoomList = pViewShell->GetZoomList();

            if (!pView->IsTextEdit() && pZoomList->IsPreviousPossible() && !bSlideShow && !pDocSh->IsUIActive())
            {
                // Vorheriges ZoomRect einstellen
                pViewShell->SetZoomRect(pZoomList->GetPreviousZoomRect());
                bReturn = TRUE;
            }
        }
        break;

        case KEY_HOME:
        {
            if (!pView->IsTextEdit()
                && pViewShell->ISA(DrawViewShell)
                && !bSlideShow)
            {
               // Sprung zu erster Seite
               static_cast<DrawViewShell*>(pViewShell)->SwitchPage(0);
               bReturn = TRUE;
            }
        }
        break;

        case KEY_END:
        {
            if (!pView->IsTextEdit()
                && pViewShell->ISA(DrawViewShell)
                && !bSlideShow)
            {
                // Sprung zu letzter Seite
                SdPage* pPage =
                    static_cast<DrawViewShell*>(pViewShell)->GetActualPage();
                static_cast<DrawViewShell*>(pViewShell)
                    ->SwitchPage(pDoc->GetSdPageCount(
                        pPage->GetPageKind()) - 1);
                bReturn = TRUE;
            }
        }
        break;

        case KEY_PAGEUP:
        {
            if(pViewShell->ISA(DrawViewShell) && !bSlideShow)
            {
                // The page-up key works either with no or with the CTRL
                // modifier.  The reaction in either case is the same.
                if ( ! rKEvt.GetKeyCode().GetAllModifier()
                    || rKEvt.GetKeyCode().IsMod1())
                {
                    // The type of reaction depends on whether the layer
                    // mode is active.
                    if (static_cast<DrawViewShell*>(pViewShell)->GetLayerMode())
                    {
                        // With the layer mode active pressing page-up
                        // moves to the previous layer.
                        SwitchLayer (-1);
                    }
                    else
                    {
                        // When not in layer mode the page-up key moves to
                        // the previous slide.

                        pView->EndTextEdit();

                        // Previous page.
                        bReturn = TRUE;
                        SdPage* pPage = static_cast<DrawViewShell*>(pViewShell)
                            ->GetActualPage();
                        USHORT nSdPage = (pPage->GetPageNum() - 1) / 2;

                        if (nSdPage > 0)
                        {
                            // Switch the page and send events
                            // regarding deactivation the old page and
                            // activating the new one.
                            TabControl* pPageTabControl =
                                static_cast<DrawViewShell*>(pViewShell)
                                ->GetPageTabControl();
                            if (pPageTabControl->IsReallyShown())
                                pPageTabControl->SendDeactivatePageEvent ();
                            static_cast<DrawViewShell*>(pViewShell)
                                ->SwitchPage(nSdPage - 1);
                            if (pPageTabControl->IsReallyShown())
                                pPageTabControl->SendActivatePageEvent ();
                        }
                    }
                }
            }
        }
        break;

        case KEY_PAGEDOWN:
        {
            if(pViewShell->ISA(DrawViewShell) && !bSlideShow)
            {
                // The page-down key works either with no or with the CTRL
                // modifier.  The reaction in either case is the same.
                if ( ! rKEvt.GetKeyCode().GetAllModifier()
                    || rKEvt.GetKeyCode().IsMod1())
                {
                    // The type of reaction depends on whether the layer
                    // mode is active.
                    if (static_cast<DrawViewShell*>(pViewShell)
                        ->GetLayerMode())
                    {
                        // With the layer mode active pressing page-down
                        // moves to the next layer.
                        SwitchLayer (+1);
                    }
                    else
                    {
                        // When not in layer mode the page-down key moves to
                        // the next slide.

                        pView->EndTextEdit();

                        // Next page.
                        bReturn = TRUE;
                        SdPage* pPage = static_cast<DrawViewShell*>(pViewShell)
                            ->GetActualPage();
                        USHORT nSdPage = (pPage->GetPageNum() - 1) / 2;

                        if (nSdPage < pDoc->GetSdPageCount(pPage->GetPageKind()) - 1)
                        {
                            // Switch the page and send events
                            // regarding deactivation the old page and
                            // activating the new one.
                            TabControl* pPageTabControl =
                                static_cast<DrawViewShell*>(pViewShell)
                                ->GetPageTabControl();
                            if (pPageTabControl->IsReallyShown())
                                pPageTabControl->SendDeactivatePageEvent ();
                            static_cast<DrawViewShell*>(pViewShell)
                                ->SwitchPage(nSdPage + 1);
                            if (pPageTabControl->IsReallyShown())
                                pPageTabControl->SendActivatePageEvent ();
                        }
                    }
                }
            }
        }
        break;

        // #97016# II change select state when focus is on poly point
        case KEY_SPACE:
        {
            const SdrHdlList& rHdlList = pView->GetHdlList();
            SdrHdl* pHdl = rHdlList.GetFocusHdl();

            if(pHdl)
            {
                if(pHdl->GetKind() == HDL_POLY)
                {
                    // rescue ID of point with focus
                    sal_uInt16 nPol(pHdl->GetPolyNum());
                    sal_uInt16 nPnt(pHdl->GetPointNum());

                    if(pView->IsPointMarked(*pHdl))
                    {
                        if(rKEvt.GetKeyCode().IsShift())
                        {
                            pView->UnmarkPoint(*pHdl);
                        }
                    }
                    else
                    {
                        if(!rKEvt.GetKeyCode().IsShift())
                        {
                            pView->UnmarkAllPoints();
                        }

                        pView->MarkPoint(*pHdl);
                    }

                    if(0L == rHdlList.GetFocusHdl())
                    {
                        // restore point with focus
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

                    bReturn = TRUE;
                }
            }
        }
        break;

        case KEY_UP:
        case KEY_DOWN:
        case KEY_LEFT:
        case KEY_RIGHT:
        {
            if (!pView->IsTextEdit() && !bSlideShow)
            {
                long nX = 0;
                long nY = 0;

                if (nCode == KEY_UP)
                {
                    // Scroll nach oben
                    nX = 0;
                    nY =-1;
                }
                else if (nCode == KEY_DOWN)
                {
                    // Scroll nach unten
                    nX = 0;
                    nY = 1;
                }
                else if (nCode == KEY_LEFT)
                {
                    // Scroll nach links
                    nX =-1;
                    nY = 0;
                }
                else if (nCode == KEY_RIGHT)
                {
                    // Scroll nach rechts
                    nX = 1;
                    nY = 0;
                }

                if (pView->AreObjectsMarked() && !rKEvt.GetKeyCode().IsMod1() &&
                    !pDocSh->IsReadOnly())
                {
                    // #97016# II
                    const SdrHdlList& rHdlList = pView->GetHdlList();
                    SdrHdl* pHdl = rHdlList.GetFocusHdl();

                    // #109007#
                    sal_Bool bIsMoveOfConnectedHandle(sal_False);
                    sal_Bool bOldSuppress;
                    SdrEdgeObj* pEdgeObj = 0L;

                    if(pHdl && pHdl->GetObj() && pHdl->GetObj()->ISA(SdrEdgeObj) && 0 == pHdl->GetPolyNum())
                    {
                        pEdgeObj = (SdrEdgeObj*)pHdl->GetObj();

                        if(0 == pHdl->GetPointNum())
                        {
                            if(pEdgeObj->GetConnection(sal_True).GetObject())
                            {
                                bIsMoveOfConnectedHandle = sal_True;
                            }
                        }
                        if(1 == pHdl->GetPointNum())
                        {
                            if(pEdgeObj->GetConnection(sal_False).GetObject())
                            {
                                bIsMoveOfConnectedHandle = sal_True;
                            }
                        }
                    }

                    // #109007#
                    if(pEdgeObj)
                    {
                        // Suppress default connects to inside object and object center
                        bOldSuppress = pEdgeObj->GetSuppressDefaultConnect();
                        pEdgeObj->SetSuppressDefaultConnect(sal_True);
                    }

                    // #109007#
                    if(bIsMoveOfConnectedHandle)
                    {
                        sal_uInt16 nMarkHdSiz(pView->GetMarkHdlSizePixel());
                        Size aHalfConSiz(nMarkHdSiz + 1, nMarkHdSiz + 1);
                        aHalfConSiz = pWindow->PixelToLogic(aHalfConSiz);

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
                        // #97016# move in 1 pixel distance
                        Size aLogicSizeOnePixel = (pWindow) ? pWindow->PixelToLogic(Size(1,1)) : Size(100, 100);
                        nX *= aLogicSizeOnePixel.Width();
                        nY *= aLogicSizeOnePixel.Height();
                    }
                    else
                    {
                        // old, fixed move distance
                        nX *= 100;
                        nY *= 100;
                    }

                    if(0L == pHdl)
                    {
                        // #67368# only take action when move is allowed
                        if(pView->IsMoveAllowed())
                        {
                            // #90129# restrict movement to WorkArea
                            const Rectangle& rWorkArea = pView->GetWorkArea();

                            if(!rWorkArea.IsEmpty())
                            {
                                Rectangle aMarkRect(pView->GetMarkedObjRect());
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
                                pView->MoveAllMarked(Size(nX, nY));

                                // #97016# II
                                pView->MakeVisible(pView->GetAllMarkedRect(), *pWindow);
                            }
                        }
                    }
                    else
                    {
                        // move handle with index nHandleIndex
                        if(pHdl && (nX || nY))
                        {
                            // now move the Handle (nX, nY)
                            Point aStartPoint(pHdl->GetPos());
                            Point aEndPoint(pHdl->GetPos() + Point(nX, nY));
                            const SdrDragStat& rDragStat = pView->GetDragStat();

                            // start dragging
                            pView->BegDragObj(aStartPoint, 0, pHdl, 0);

                            if(pView->IsDragObj())
                            {
                                FASTBOOL bWasNoSnap = rDragStat.IsNoSnap();
                                BOOL bWasSnapEnabled = pView->IsSnapEnabled();

                                // switch snapping off
                                if(!bWasNoSnap)
                                    ((SdrDragStat&)rDragStat).SetNoSnap(TRUE);
                                if(bWasSnapEnabled)
                                    pView->SetSnapEnabled(FALSE);

                                pView->MovAction(aEndPoint);
                                pView->EndDragObj();

                                // restore snap
                                if(!bWasNoSnap)
                                    ((SdrDragStat&)rDragStat).SetNoSnap(bWasNoSnap);
                                if(bWasSnapEnabled)
                                    pView->SetSnapEnabled(bWasSnapEnabled);
                            }

                            // make moved handle visible
                            Rectangle aVisRect(aEndPoint - Point(100, 100), Size(200, 200));
                            pView->MakeVisible(aVisRect, *pWindow);
                        }
                    }

                    // #109007#
                    if(pEdgeObj)
                    {
                        // Restore original suppress value
                        pEdgeObj->SetSuppressDefaultConnect(bOldSuppress);
                    }
                }
                else
                {
                    // Seite scrollen
                    ScrollStart();
                    pViewShell->ScrollLines(nX, nY);
                    ScrollEnd();
                }

                bReturn = TRUE;
            }
        }
        break;
    }

    if (bReturn)
    {
        pWindow->ReleaseMouse();
    }

    // #98198# when a text-editable object is selected and the
    // input character is printable, activate text edit on that object
    // and feed character to object
    if(!bReturn && !pDocSh->IsReadOnly())
    {
        if(!pView->IsTextEdit() && pViewShell)
        {
            const SdrMarkList& rMarkList = pView->GetMarkedObjectList();

            if(1 == rMarkList.GetMarkCount())
            {
                SdrObject* pObj = rMarkList.GetMark(0)->GetObj();

                if(pObj->ISA(SdrTextObj) && pObj->HasTextEdit() && !pObj->ISA(SdrOle2Obj))
                {
                    // #98533# use common IsSimpleCharInput from
                    // the EditEngine.
                    sal_Bool bPrintable(EditEngine::IsSimpleCharInput(rKEvt));

                    if(bPrintable)
                    {
                        // try to activate textedit mode for the selected object
                        SfxStringItem aInputString(SID_ATTR_CHAR, String(rKEvt.GetCharCode()));

                        pViewShell->GetViewFrame()->GetDispatcher()->Execute(
                            SID_ATTR_CHAR,
                            SFX_CALLMODE_ASYNCHRON,
                            &aInputString,
                            0L);

                        // consumed
                        bReturn = TRUE;
                    }
                }
            }
            else
            {
                // #99039# test if there is a title object there. If yes, try to
                // set it to edit mode and start typing...
                if(pViewShell->ISA(DrawViewShell)
                    && EditEngine::IsSimpleCharInput(rKEvt))
                {
                    DrawViewShell* pDrawViewShell =
                        static_cast<DrawViewShell*>(pViewShell);
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

                    // when candidate found and candidate is untouched, start editing text...
                    if(pCandidate && pCandidate->IsEmptyPresObj())
                    {
                        pView->UnMarkAll();
                        pView->MarkObj(pCandidate, pView->GetPageViewPvNum(0));
                        SfxStringItem aInputString(SID_ATTR_CHAR, String(rKEvt.GetCharCode()));

                        pViewShell->GetViewFrame()->GetDispatcher()->Execute(
                            SID_ATTR_CHAR,
                            SFX_CALLMODE_ASYNCHRON,
                            &aInputString,
                            0L);

                        // consumed
                        bReturn = TRUE;
                    }
                }
            }
        }
    }

    return(bReturn);
}

// #97016# II
void FuPoor::SelectionHasChanged()
{
    const SdrHdlList& rHdlList = pView->GetHdlList();
    ((SdrHdlList&)rHdlList).ResetFocusHdl();
}

/*************************************************************************
|*
|* Cut object to clipboard
|*
\************************************************************************/

void FuPoor::DoCut()
{
    if (pView)
    {
        pView->DoCut(pWindow);
    }
}

/*************************************************************************
|*
|* Copy object to clipboard
|*
\************************************************************************/

void FuPoor::DoCopy()
{
    if (pView)
    {
        pView->DoCopy(pWindow);
    }
}

/*************************************************************************
|*
|* Paste object from clipboard
|*
\************************************************************************/

void FuPoor::DoPaste()
{
    if (pView)
    {
        pView->DoPaste(pWindow);
    }
}

/*************************************************************************
|*
|* Timer-Handler fuer Drag&Drop
|*
\************************************************************************/

IMPL_LINK( FuPoor, DragHdl, Timer *, pTimer )
{
    USHORT nHitLog = USHORT ( pWindow->PixelToLogic(Size(HITPIX,0)).Width() );
    SdrHdl* pHdl = pView->HitHandle(aMDPos, *pWindow);

    if ( pHdl==NULL && pView->IsMarkedHit(aMDPos, nHitLog)
         && !pView->IsPresObjSelected(FALSE, TRUE) )
    {
        pWindow->ReleaseMouse();
        bIsInDragMode = TRUE;
        pView->StartDrag( aMDPos, pWindow );
    }
    return 0;
}

/*************************************************************************
|*
|* Command-event
|*
\************************************************************************/

BOOL FuPoor::Command(const CommandEvent& rCEvt)
{
    return( pView->Command(rCEvt,pWindow) );
}

/*************************************************************************
|*
|* Timer-Handler fuer Fensterscrolling
|*
\************************************************************************/

IMPL_LINK_INLINE_START( FuPoor, DelayHdl, Timer *, pTimer )
{
    aDelayToScrollTimer.Stop ();
    bScrollable = TRUE;

    Point aPnt(pWindow->GetPointerPosPixel());

    // #95491# use remembered MouseButton state to create correct
    // MouseEvents for this artifical MouseMove.
    MouseMove(MouseEvent(aPnt, 1, 0, GetMouseButtonCode()));

    return 0;
}
IMPL_LINK_INLINE_END( FuPoor, DelayHdl, Timer *, pTimer )

/*************************************************************************
|*
|* Handler fuer Maustaste
|*
\************************************************************************/

BOOL FuPoor::MouseButtonUp (const MouseEvent& rMEvt)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    aDelayToScrollTimer.Stop ();
    return bScrollable  =
        bDelayActive = FALSE;
}

BOOL FuPoor::MouseButtonDown(const MouseEvent& rMEvt)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    return FALSE;
}

/*************************************************************************
|*
|* Handler fuer Maustaste
|*
\************************************************************************/

void FuPoor::StartDelayToScrollTimer ()
{
    bDelayActive = TRUE;
    aDelayToScrollTimer.Start ();
}

/*************************************************************************
|*
|* Handler fuer Maustaste
|*
\************************************************************************/

long FuPoor::diffPoint (long pos1, long pos2)
{
    return (pos1 > pos2) ? pos1 - pos2
                         : pos2 - pos1;
}

/*************************************************************************
|*
|* Help-event
|*
\************************************************************************/

BOOL FuPoor::RequestHelp(const HelpEvent& rHEvt)
{
    BOOL bReturn = FALSE;

    SdrPageView* pPV = pView->GetPageViewPvNum(0);

    if (pPV)
    {
        SdPage* pPage = (SdPage*) pPV->GetPage();

        if (pPage)
        {
            bReturn = pPage->RequestHelp(pWindow, pView, rHEvt);
        }
    }

    return(bReturn);
}

/*************************************************************************
|*
|* Request verarbeiten
|*
\************************************************************************/

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

/*************************************************************************
|*
|* #97016#
|*
\************************************************************************/

SdrObject* FuPoor::CreateDefaultObject(const sal_uInt16 nID, const Rectangle& rRectangle)
{
    // empty base implementation
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
    if(pViewShell && pViewShell->ISA(DrawViewShell))
    {
        DrawViewShell* pDrawViewShell =
            static_cast<DrawViewShell*>(pViewShell);

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
                static_cast<DrawViewShell*>(pViewShell)->GetLayerTabControl();
            pLayerTabControl->SendDeactivatePageEvent ();

            pDrawViewShell->SetActiveTabLayerIndex (nIndex);

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
        pViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);
        return true;
    }

    return false;
}

} // end of namespace sd
