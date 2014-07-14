/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"


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

#ifndef SD_FRAMW_VIEW_HXX
#include "FrameView.hxx"
#endif
#include "app.hrc"
#include "fusel.hxx"
#include "sdpage.hxx"
#include "drawview.hxx"
#include "DrawViewShell.hxx"
#ifndef SD_WINDOW_SHELL_HXX
#include "Window.hxx"
#endif
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "zoomlist.hxx"
#include "Client.hxx"
#include "slideshow.hxx"
#include "LayerTabBar.hxx"

#include <sfx2/viewfrm.hxx>

// #97016# IV
#include <svx/svditer.hxx>

// #98533#
#include <editeng/editeng.hxx>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;

namespace sd {

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
    : mpView(pView),
      mpViewShell(pViewSh),
      mpWindow(pWin),
      mpDocSh( pDrDoc->GetDocSh() ),
      mpDoc(pDrDoc),
      nSlotId( rReq.GetSlot() ),
      nSlotValue(0),
      pDialog(NULL),
      bIsInDragMode(false),
      bNoScrollUntilInside (true),
      bScrollable (false),
      bDelayActive (false),
      bFirstMouseMove (false),
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
        bDelayActive = false;

    if (pDialog)
    {
        pDialog->Hide();
    }

    if (mpWindow) mpWindow->ReleaseMouse ();
}

/*************************************************************************
|*
|* Scrollen bei Erreichen des Fensterrandes; wird von
|* MouseMove aufgerufen
|*
\************************************************************************/

void FuPoor::ForceScroll(const basegfx::B2DPoint& aPixPos)
{
    aScrollTimer.Stop();

    if ( !mpView->IsDragHelpLine() && !mpView->IsSetPageOrg() &&
            !SlideShow::IsRunning( mpViewShell->GetViewShellBase() ) )
    {
/*      Size aSize = mpWindow->GetSizePixel();
        short dx = 0, dy = 0;

        if ( aPixPos.X() <= 0              ) dx = -1;
        if ( aPixPos.X() >= aSize.Width()  ) dx =  1;
        if ( aPixPos.Y() <= 0              ) dy = -1;
        if ( aPixPos.Y() >= aSize.Height() ) dy =  1;
*/
        const Point aPos = mpWindow->OutputToScreenPixel(Point(basegfx::fround(aPixPos.getX()), basegfx::fround(aPixPos.getY())));
        const Rectangle& rRect = mpViewShell->GetAllWindowRect();

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
                    // Scrollaktion in abgeleiteter Klasse
                    ScrollStart();
                    mpViewShell->ScrollLines(basegfx::B2DVector(dx, dy));
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

IMPL_LINK_INLINE_START( FuPoor, ScrollHdl, Timer *, EMPTYARG )
{
    Point aPnt(mpWindow->GetPointerPosPixel());

    // #95491# use remembered MouseButton state to create correct
    // MouseEvents for this artificial MouseMove.
    MouseMove(MouseEvent(aPnt, 1, 0, GetMouseButtonCode()));

    return 0;
}
IMPL_LINK_INLINE_END( FuPoor, ScrollHdl, Timer *, pTimer )

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert true, andernfalls
|* false.
|*
\************************************************************************/

bool FuPoor::KeyInput(const KeyEvent& rKEvt)
{
    sal_uInt16          nCode = rKEvt.GetKeyCode().GetCode();
    bool            bReturn = false;
    bool            bSlideShow = SlideShow::IsRunning( mpViewShell->GetViewShellBase() );

    switch (nCode)
    {
        // #97016# IV
        case KEY_RETURN:
        {
            if(rKEvt.GetKeyCode().IsMod1())
            {
                DrawViewShell* pDrawViewShell = dynamic_cast< DrawViewShell* >(mpViewShell);

                if(pDrawViewShell)
                {
                    SdPage* pActualPage = pDrawViewShell->GetActualPage();
                    SdrTextObj* pCandidate = 0L;

                    if(pActualPage)
                    {
                        SdrObjListIter aIter(*pActualPage, IM_DEEPNOGROUPS);

                        while(aIter.IsMore() && !pCandidate)
                        {
                            SdrTextObj* pObj = dynamic_cast< SdrTextObj* >(aIter.Next());

                            if(pObj)
                            {
                                sal_uInt32 nInv(pObj->GetObjInventor());
                                sal_uInt16 nKnd(pObj->GetObjIdentifier());

                                if(SdrInventor == nInv && (OBJ_TITLETEXT == nKnd || OBJ_OUTLINETEXT == nKnd || OBJ_TEXT == nKnd))
                                {
                                    pCandidate = pObj;
                                }
                            }
                        }
                    }

                    if(pCandidate)
                    {
                        mpView->UnmarkAllObj();
                        mpView->MarkObj(*pCandidate);

                        mpViewShell->GetViewFrame()->GetDispatcher()->Execute(
                            SID_ATTR_CHAR, SFX_CALLMODE_ASYNCHRON);
                    }
                    else
                    {
                        // insert a new page with the same page layout
                        mpViewShell->GetViewFrame()->GetDispatcher()->Execute(
                            SID_INSERTPAGE_QUICK, SFX_CALLMODE_ASYNCHRON);
                    }

                    // consumed
                    bReturn = true;
                }
            }
            else
            {
                // #98255# activate OLE object on RETURN for selected object
                // #98198# activate text edit on RETURN for selected object
                if( !mpView->IsTextEdit() )
                {
                    SdrOle2Obj* pSdrOle2Obj = dynamic_cast< SdrOle2Obj* >(mpView->getSelectedIfSingle());

                    if( pSdrOle2Obj && !mpDocSh->IsUIActive() )
                    {
                        mpViewShell->ActivateObject( pSdrOle2Obj, 0 );
                    }
                    else
                    {
                        SdrGrafObj* pSdrGrafObj = dynamic_cast< SdrGrafObj* >(mpView->getSelectedIfSingle());

                        if( pSdrGrafObj && pSdrGrafObj->IsEmptyPresObj() )
                        {
                            mpViewShell->GetViewFrame()->GetDispatcher()->Execute( SID_INSERT_GRAPHIC, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
                        }
                        else
                        {
                            mpViewShell->GetViewFrame()->GetDispatcher()->Execute( SID_ATTR_CHAR, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
                        }
                    }

                    // consumed
                    bReturn = true;
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
                const SdrHdlList& rHdlList = mpView->GetHdlList();
                sal_Bool bForward(!rKEvt.GetKeyCode().IsShift());

                ((SdrHdlList&)rHdlList).TravelFocusHdl(bForward);

                // guarantee visibility of focused handle
                SdrHdl* pHdl = rHdlList.GetFocusHdl();

                if(pHdl)
                {
                    const basegfx::B2DRange aRange(
                        pHdl->getPosition() - basegfx::B2DPoint(100.0, 100.0),
                        pHdl->getPosition() + basegfx::B2DPoint(100.0, 100.0));

                    mpView->MakeVisibleAtView(aRange, *mpWindow);
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
                // Zoom vergroessern
                mpViewShell->SetZoom(basegfx::fround(mpWindow->GetZoom() * (3.0 / 2.0)));

                DrawViewShell* pDrawViewShell = dynamic_cast< DrawViewShell* >(mpViewShell);

                if(pDrawViewShell)
                {
                    pDrawViewShell->SetZoomOnPage(false);
                }

                bReturn = true;
            }
        }
        break;

        case KEY_SUBTRACT:
        {
            if (!mpView->IsTextEdit() && !bSlideShow && !mpDocSh->IsUIActive())
            {
                // Zoom verringern
                mpViewShell->SetZoom(basegfx::fround(mpWindow->GetZoom() * (2.0 / 3.0)));

                DrawViewShell* pDrawViewShell = dynamic_cast< DrawViewShell* >(mpViewShell);

                if(pDrawViewShell)
                {
                    pDrawViewShell->SetZoomOnPage(false);
                }

                bReturn = true;
            }
        }
        break;

        case KEY_MULTIPLY:
        {
            if (!mpView->IsTextEdit() && !bSlideShow)
            {
                // Zoom auf Seite
                mpViewShell->GetViewFrame()->GetDispatcher()->
                Execute(SID_SIZE_PAGE, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
                bReturn = true;
            }
        }
        break;

        case KEY_DIVIDE:
        {
            if (!mpView->IsTextEdit() && !bSlideShow)
            {
                // Zoom auf selektierte Objekte
                mpViewShell->GetViewFrame()->GetDispatcher()->
                Execute(SID_SIZE_OPTIMAL, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
                bReturn = true;
            }
        }
        break;

        case KEY_POINT:
        {
            ZoomList* pZoomList = mpViewShell->GetZoomList();

            if (!mpView->IsTextEdit() && pZoomList->IsNextPossible() && !bSlideShow && !mpDocSh->IsUIActive())
            {
                // Naechstes ZoomRect einstellen
                mpViewShell->SetZoomRange(pZoomList->GetNextZoomRange());
                bReturn = true;
            }
        }
        break;

        case KEY_COMMA:
        {
            ZoomList* pZoomList = mpViewShell->GetZoomList();

            if (!mpView->IsTextEdit() && pZoomList->IsPreviousPossible() && !bSlideShow && !mpDocSh->IsUIActive())
            {
                // Vorheriges ZoomRect einstellen
                mpViewShell->SetZoomRange(pZoomList->GetPreviousZoomRange());
                bReturn = true;
            }
        }
        break;

        case KEY_HOME:
        {
            if (!mpView->IsTextEdit() && !bSlideShow)
            {
                DrawViewShell* pDrawViewShell = dynamic_cast< DrawViewShell* >(mpViewShell);

                if(pDrawViewShell)
                {
                   // Sprung zu erster Seite
                   pDrawViewShell->SwitchPage(0);
                   bReturn = true;
                }
            }
        }
        break;

        case KEY_END:
        {
            if(!mpView->IsTextEdit() && !bSlideShow)
            {
                DrawViewShell* pDrawViewShell = dynamic_cast< DrawViewShell* >(mpViewShell);

                if(pDrawViewShell)
                {
                    // Sprung zu letzter Seite
                    SdPage* pPage = pDrawViewShell->GetActualPage();
                    pDrawViewShell->SwitchPage(mpDoc->GetSdPageCount(pPage->GetPageKind()) - 1);
                    bReturn = true;
                }
            }
        }
        break;

        case KEY_PAGEUP:
        {
            if( rKEvt.GetKeyCode().IsMod1() && rKEvt.GetKeyCode().IsMod2() )
                break;

            if(!bSlideShow)
            {
                DrawViewShell* pDrawViewShell = dynamic_cast< DrawViewShell* >(mpViewShell);

                if(pDrawViewShell)
                {
                    // The page-up key switches layers or pages depending on the
                    // modifier key.
                    if ( ! rKEvt.GetKeyCode().GetAllModifier())
                    {
                        // With no modifier pressed we move to the previous
                        // slide.
                        mpView->SdrEndTextEdit();

                        // Previous page.
                        bReturn = true;
                        SdPage* pPage = pDrawViewShell->GetActualPage();
                        sal_uInt32 nSdPage = (pPage->GetPageNumber() - 1) / 2;

                        if (nSdPage > 0)
                        {
                            // Switch the page and send events regarding
                            // deactivation the old page and activating the new
                            // one.
                                TabControl* pPageTabControl = pDrawViewShell->GetPageTabControl();
                            if (pPageTabControl->IsReallyShown())
                                pPageTabControl->SendDeactivatePageEvent ();
                                pDrawViewShell->SwitchPage(nSdPage - 1);
                            if (pPageTabControl->IsReallyShown())
                                pPageTabControl->SendActivatePageEvent ();
                        }
                    }
                    else if (rKEvt.GetKeyCode().IsMod1())
                    {
                        // With the CONTROL modifier we switch layers.
                        if (pDrawViewShell->IsLayerModeActive())
                        {
                            // Moves to the previous layer.
                            SwitchLayer (-1);
                        }
                    }
                }
            }
        }
        break;

        case KEY_PAGEDOWN:
        {
            if( rKEvt.GetKeyCode().IsMod1() && rKEvt.GetKeyCode().IsMod2() )
                break;

            if(!bSlideShow)
            {
                DrawViewShell* pDrawViewShell = dynamic_cast< DrawViewShell* >(mpViewShell);

                if(pDrawViewShell)
                {
                    // The page-down key switches layers or pages depending on the
                    // modifier key.
                    if ( ! rKEvt.GetKeyCode().GetAllModifier())
                    {
                        // With no modifier pressed we move to the next slide.
                        mpView->SdrEndTextEdit();

                        // Next page.
                        bReturn = true;
                        SdPage* pPage = pDrawViewShell->GetActualPage();
                        sal_uInt32 nSdPage = (pPage->GetPageNumber() - 1) / 2;

                        if (nSdPage < mpDoc->GetSdPageCount(pPage->GetPageKind()) - 1)
                        {
                            // Switch the page and send events regarding
                            // deactivation the old page and activating the new
                            // one.
                                TabControl* pPageTabControl = pDrawViewShell->GetPageTabControl();
                            if (pPageTabControl->IsReallyShown())
                                pPageTabControl->SendDeactivatePageEvent ();
                                pDrawViewShell->SwitchPage(nSdPage + 1);
                            if (pPageTabControl->IsReallyShown())
                                pPageTabControl->SendActivatePageEvent ();
                        }
                    }
                    else if (rKEvt.GetKeyCode().IsMod1())
                    {
                        // With the CONTROL modifier we switch layers.
                        if (pDrawViewShell->IsLayerModeActive())
                        {
                            // With the layer mode active pressing page-down
                            // moves to the next layer.
                            SwitchLayer (+1);
                        }
                    }
                }
            }
        }
        break;

        // #97016# II change select state when focus is on poly point
        case KEY_SPACE:
        {
            const SdrHdlList& rHdlList = mpView->GetHdlList();
            SdrHdl* pHdl = rHdlList.GetFocusHdl();

            if(pHdl)
            {
                if(pHdl->GetKind() == HDL_POLY)
                {
                    // rescue ID of point with focus
                    sal_uInt32 nPol(pHdl->GetPolyNum());
                    sal_uInt32 nPnt(pHdl->GetPointNum());

                    if(mpView->IsPointMarked(*pHdl))
                    {
                        if(rKEvt.GetKeyCode().IsShift())
                        {
                            mpView->MarkPoint(*pHdl, true); // unmark
                        }
                    }
                    else
                    {
                        if(!rKEvt.GetKeyCode().IsShift())
                        {
                            mpView->MarkPoints(0, true); // unmarkall
                        }

                        mpView->MarkPoint(*pHdl);
                    }

                    if(0L == rHdlList.GetFocusHdl())
                    {
                        // restore point with focus
                        SdrHdl* pNewOne = 0L;

                        for(sal_uInt32 a(0); !pNewOne && a < rHdlList.GetHdlCount(); a++)
                        {
                            SdrHdl* pAct = rHdlList.GetHdlByIndex(a);

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
                basegfx::B2DVector aMove(0.0, 0.0);

                if (nCode == KEY_UP)
                {
                    // Scroll nach oben
                    aMove = basegfx::B2DPoint(0.0, -1.0);
                }
                else if (nCode == KEY_DOWN)
                {
                    // Scroll nach unten
                    aMove = basegfx::B2DPoint(0.0, 1.0);
                }
                else if (nCode == KEY_LEFT)
                {
                    // Scroll nach links
                    aMove = basegfx::B2DPoint(-1.0, 0.0);
                }
                else if (nCode == KEY_RIGHT)
                {
                    // Scroll nach rechts
                    aMove = basegfx::B2DPoint(1.0, 0.0);
                }

                if (mpView->areSdrObjectsSelected() && !rKEvt.GetKeyCode().IsMod1() &&
                    !mpDocSh->IsReadOnly())
                {
                    // #97016# II
                    const SdrHdlList& rHdlList = mpView->GetHdlList();
                    SdrHdl* pHdl = rHdlList.GetFocusHdl();

                    // #109007#
                    bool bIsMoveOfConnectedHandle(false);
                    bool bOldSuppress(false);
                    SdrEdgeObj* pEdgeObj = dynamic_cast< SdrEdgeObj* >(pHdl ? const_cast< SdrObject* >(pHdl->GetObj()) : 0);

                    if(pEdgeObj && 0 == pHdl->GetPolyNum())
                    {
                        if(0 == pHdl->GetPointNum())
                        {
                            if(pEdgeObj->GetSdrObjectConnection(true))
                            {
                                bIsMoveOfConnectedHandle = true;
                            }
                        }

                        if(1 == pHdl->GetPointNum())
                        {
                            if(pEdgeObj->GetSdrObjectConnection(false))
                            {
                                bIsMoveOfConnectedHandle = true;
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
                        basegfx::B2DVector aDist(
                            mpWindow->GetInverseViewTransformation() *
                            basegfx::B2DVector(mpView->GetMarkHdlSizePixel(), mpView->GetMarkHdlSizePixel()));

                        aDist.setX(std::max(100.0, aDist.getX()));
                        aDist.setY(std::max(100.0, aDist.getY()));

                        aMove *= aDist;
                    }
                    else if(rKEvt.GetKeyCode().IsMod2())
                    {
                        // #97016# move in 1 pixel distance
                        const basegfx::B2DVector aDist(mpWindow
                            ? mpWindow->GetInverseViewTransformation() * basegfx::B2DVector(1.0, 1.0)
                            : basegfx::B2DVector(100.0, 100.0));

                        aMove *= aDist;
                    }
                    else if(rKEvt.GetKeyCode().IsShift())
                    {
                        aMove *= 1000.0;
                    }
                    else
                    {
                        // old, fixed move distance
                        aMove *= 100;
                    }

                    if(!pHdl)
                    {
                        // #67368# only take action when move is allowed
                        if(mpView->IsMoveAllowed())
                        {
                            // #90129# restrict movement to WorkArea
                            const basegfx::B2DRange& rWorkArea = mpView->GetWorkArea();

                            if(!rWorkArea.isEmpty())
                            {
                                basegfx::B2DRange aMarkRange(mpView->getMarkedObjectSnapRange());
                                aMarkRange.transform(basegfx::tools::createTranslateB2DHomMatrix(aMove));

                                if(!aMarkRange.isInside(rWorkArea))
                                {
                                    if(aMarkRange.getMinX() < rWorkArea.getMinX())
                                    {
                                        aMove.setX(aMove.getX() + rWorkArea.getMinX() - aMarkRange.getMinX());
                                    }

                                    if(aMarkRange.getMaxX() > rWorkArea.getMaxX())
                                    {
                                        aMove.setX(aMove.getX() - aMarkRange.getMaxX() - rWorkArea.getMaxX());
                                    }

                                    if(aMarkRange.getMinY() < rWorkArea.getMinY())
                                    {
                                        aMove.setY(aMove.getY() + rWorkArea.getMinY() - aMarkRange.getMinY());
                                    }

                                    if(aMarkRange.getMaxY() > rWorkArea.getMaxY())
                                    {
                                        aMove.setY(aMove.getY() - aMarkRange.getMaxY() - rWorkArea.getMaxY());
                                    }
                                }
                            }

                            // no handle selected
                            if(!aMove.equalZero())
                            {
                                mpView->MoveMarkedObj(aMove);

                                // #97016# II
                                mpView->MakeVisibleAtView(mpView->getMarkedObjectSnapRange(), *mpWindow);
                            }
                        }
                    }
                    else
                    {
                        // move handle with index nHandleIndex
                        mpView->MoveHandleByVector(*pHdl, aMove, mpWindow, 0);
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
                    mpViewShell->ScrollLines(aMove);
                    ScrollEnd();
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

    // #98198# when a text-editable object is selected and the
    // input character is printable, activate text edit on that object
    // and feed character to object
    if(!bReturn && !mpDocSh->IsReadOnly() && !mpView->IsTextEdit() && mpViewShell)
    {
        SdrObject* pSelected = mpView->getSelectedIfSingle();

        if(pSelected)
        {
            if(dynamic_cast< SdrTextObj* >(pSelected) && pSelected->HasTextEdit() && !dynamic_cast< SdrOle2Obj* >(pSelected))
            {
                // #98533# use common IsSimpleCharInput from
                // the EditEngine.
                bool bPrintable(EditEngine::IsSimpleCharInput(rKEvt));

                if(bPrintable)
                {
                    // try to activate textedit mode for the selected object
                    SfxStringItem aInputString(SID_ATTR_CHAR, String(rKEvt.GetCharCode()));

                    mpViewShell->GetViewFrame()->GetDispatcher()->Execute(
                        SID_ATTR_CHAR,
                        SFX_CALLMODE_ASYNCHRON,
                        &aInputString,
                        0L);

                    // consumed
                    bReturn = true;
                }
            }
        }
        else
        {
            // #99039# test if there is a title object there. If yes, try to
            // set it to edit mode and start typing...
            DrawViewShell* pDrawViewShell = dynamic_cast< DrawViewShell* >(mpViewShell);

            if(pDrawViewShell && EditEngine::IsSimpleCharInput(rKEvt))
            {
                SdPage* pActualPage = pDrawViewShell->GetActualPage();
                SdrTextObj* pCandidate = 0L;

                if(pActualPage)
                {
                    SdrObjListIter aIter(*pActualPage, IM_DEEPNOGROUPS);

                    while(aIter.IsMore() && !pCandidate)
                    {
                        SdrTextObj* pObj = dynamic_cast< SdrTextObj* >(aIter.Next());

                        if(pObj)
                        {
                            sal_uInt32 nInv(pObj->GetObjInventor());
                            sal_uInt16 nKnd(pObj->GetObjIdentifier());

                            if(SdrInventor == nInv && OBJ_TITLETEXT == nKnd)
                            {
                                pCandidate = pObj;
                            }
                        }
                    }
                }

                // when candidate found and candidate is untouched, start editing text...
                if(pCandidate && pCandidate->IsEmptyPresObj())
                {
                    mpView->UnmarkAllObj();
                    mpView->MarkObj(*pCandidate);
                    SfxStringItem aInputString(SID_ATTR_CHAR, String(rKEvt.GetCharCode()));

                    mpViewShell->GetViewFrame()->GetDispatcher()->Execute(
                        SID_ATTR_CHAR,
                        SFX_CALLMODE_ASYNCHRON,
                        &aInputString,
                        0L);

                        // consumed
                    bReturn = true;
                }
            }
        }
    }

    return(bReturn);
}

bool FuPoor::MouseMove(const MouseEvent& )
{
    return false;
}

void FuPoor::SelectionHasChanged()
{
}

/*************************************************************************
|*
|* Cut object to clipboard
|*
\************************************************************************/

void FuPoor::DoCut()
{
    if (mpView)
    {
        mpView->DoCut(mpWindow);
    }
}

/*************************************************************************
|*
|* Copy object to clipboard
|*
\************************************************************************/

void FuPoor::DoCopy()
{
    if (mpView)
    {
        mpView->DoCopy(mpWindow);
    }
}

/*************************************************************************
|*
|* Paste object from clipboard
|*
\************************************************************************/

void FuPoor::DoPaste()
{
    if (mpView)
    {
        mpView->DoPaste(mpWindow);
    }
}

/*************************************************************************
|*
|* Timer-Handler fuer Drag&Drop
|*
\************************************************************************/

IMPL_LINK( FuPoor, DragHdl, Timer *, EMPTYARG )
{
    if( mpView )
    {
        SdrHdl* pHdl = mpView->PickHandle(aMDPos);

        if ( pHdl==NULL && mpView->IsMarkedObjHit(aMDPos, mpView->getHitTolLog())
             && !mpView->IsPresObjSelected(false, true) )
        {
            mpWindow->ReleaseMouse();
            bIsInDragMode = true;
            const Point aPoint(basegfx::fround(aMDPos.getX()), basegfx::fround(aMDPos.getY()));
            mpView->StartDrag( aPoint, mpWindow );
        }
    }
    return 0;
}

/*************************************************************************
|*
|* Command-event
|*
\************************************************************************/

bool FuPoor::Command(const CommandEvent& rCEvt)
{
    return( mpView->Command(rCEvt,mpWindow) );
}

/*************************************************************************
|*
|* Timer-Handler fuer Fensterscrolling
|*
\************************************************************************/

IMPL_LINK_INLINE_START( FuPoor, DelayHdl, Timer *, EMPTYARG )
{
    aDelayToScrollTimer.Stop ();
    bScrollable = true;

    Point aPnt(mpWindow->GetPointerPosPixel());

    // #95491# use remembered MouseButton state to create correct
    // MouseEvents for this artificial MouseMove.
    MouseMove(MouseEvent(aPnt, 1, 0, GetMouseButtonCode()));

    return 0;
}
IMPL_LINK_INLINE_END( FuPoor, DelayHdl, Timer *, pTimer )

/*************************************************************************
|*
|* Handler fuer Maustaste
|*
\************************************************************************/

bool FuPoor::MouseButtonUp (const MouseEvent& rMEvt)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    aDelayToScrollTimer.Stop ();
    return bScrollable  =
        bDelayActive = false;
}

bool FuPoor::MouseButtonDown(const MouseEvent& rMEvt)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    return false;
}

/*************************************************************************
|*
|* Handler fuer Maustaste
|*
\************************************************************************/

void FuPoor::StartDelayToScrollTimer ()
{
    bDelayActive = true;
    aDelayToScrollTimer.Start ();
}

/*************************************************************************
|*
|* Help-event
|*
\************************************************************************/

bool FuPoor::RequestHelp(const HelpEvent& rHEvt)
{
    bool bReturn = false;

    SdrPageView* pPV = mpView->GetSdrPageView();

    if (pPV)
    {
        SdPage& rPage = (SdPage&) pPV->getSdrPageFromSdrPageView();

        bReturn = rPage.RequestHelp(mpWindow, mpView, rHEvt);
    }

    return(bReturn);
}

void FuPoor::Paint(const Rectangle&, ::sd::Window* )
{
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
            const SfxAllEnumItem* pItem = dynamic_cast< const SfxAllEnumItem* >(&pSet->Get( nSlotId ));

            if( pItem )
            {
                nSlotValue = pItem->GetValue();
            }
        }
    }
}

/*************************************************************************
|*
|* #97016#
|*
\************************************************************************/

SdrObject* FuPoor::CreateDefaultObject(const sal_uInt16, const basegfx::B2DRange& )
{
    // empty base implementation
    return 0L;
}

void FuPoor::ImpForceQuadratic(basegfx::B2DRange& rRange)
{
    basegfx::B2DPoint aNewTopLeft(0.0, 0.0);

    if(rRange.getWidth() > rRange.getHeight())
    {
        aNewTopLeft = basegfx::B2DPoint(rRange.getMinX() + ((rRange.getWidth() - rRange.getHeight()) * 0.5), rRange.getMinY());
    }
    else
    {
        aNewTopLeft = basegfx::B2DPoint(rRange.getMinX(), rRange.getMinY() + ((rRange.getHeight() - rRange.getWidth()) * 0.5));
    }

    rRange = basegfx::B2DRange(aNewTopLeft, aNewTopLeft + rRange.getRange());
}

void FuPoor::SwitchLayer (sal_Int32 nOffset)
{
    DrawViewShell* pDrawViewShell = dynamic_cast< DrawViewShell* >(mpViewShell);

    if(pDrawViewShell)
    {
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
            LayerTabBar* pLayerTabControl = pDrawViewShell->GetLayerTabControl();
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
    if ( !dynamic_cast< FuSelection* >(this) )
    {
        mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);
        return true;
    }

    return false;
}

// #i33136#
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

} // end of namespace sd
