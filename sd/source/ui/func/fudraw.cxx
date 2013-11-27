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

#include <sot/object.hxx>
#include <editeng/eeitem.hxx>
#include <vcl/waitobj.hxx>
#include <editeng/flditem.hxx>
#include <svx/svdogrp.hxx>
#include <tools/urlobj.hxx>
#include <vcl/help.hxx>
#include <svx/bmpmask.hxx>
#include <svx/svdotext.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <svx/svdpagv.hxx>
#include <svtools/imapobj.hxx>
#include <svx/svxids.hrc>
#include <svx/obj3d.hxx>
#include <svx/svdpagv.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/svdlegacy.hxx>
#include "anminfo.hxx"
#include "anmdef.hxx"
#include "imapinfo.hxx"
#include "app.hrc"
#include "glob.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "app.hxx"
#include "GraphicDocShell.hxx"
#include "fudraw.hxx"
#include "ViewShell.hxx"
#include "FrameView.hxx"
#include "View.hxx"
#ifndef SD_WINDOW_SHELL_HXX
#include "Window.hxx"
#endif
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "Client.hxx"
#include "sdresid.hxx"
#include "drawview.hxx"
#include "fusel.hxx"
#include <svl/aeitem.hxx>
#include <vcl/msgbox.hxx>
#include "slideshow.hxx"
#include <svx/sdrhittesthelper.hxx>
#include <svx/scene3d.hxx>

using namespace ::com::sun::star;

namespace sd {

/*************************************************************************
|*
|* Base-class for all drawmodul-specific functions
|*
\************************************************************************/

FuDraw::FuDraw(ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView,
               SdDrawDocument* pDoc, SfxRequest& rReq) :
    FuPoor(pViewSh, pWin, pView, pDoc, rReq),
    bMBDown(false),
    bDragHelpLine(false),
    bPermanent(false)
{
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuDraw::~FuDraw()
{
    mpView->BrkAction();
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

bool FuDraw::MouseButtonDown(const MouseEvent& rMEvt)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());
    bool bReturn = false;
    bDragHelpLine = false;

    const basegfx::B2DPoint aPixelPos(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
    aMDPos = mpWindow->GetInverseViewTransformation() * aPixelPos;

    if ( rMEvt.IsLeft() )
    {
        FrameView* pFrameView = mpViewShell->GetFrameView();
        bool bOrtho = false;
        bool bRestricted = true;

        if (mpView->IsDragObj())
        {
            // object is dragged (move, resize,...)
            const SdrHdl* pHdl = mpView->GetDragStat().GetActiveHdl();

            if (!pHdl || (!pHdl->IsCornerHdl() && !pHdl->IsVertexHdl()))
            {
                // Move
                bRestricted = false;
            }
        }

        // #i33136#
        if(bRestricted && doConstructOrthogonal())
        {
            // Restrict movement:
            // rectangle->quadrat, ellipse->circle etc.
            bOrtho = !rMEvt.IsShift();
        }
        else
        {
            bOrtho = (bool)rMEvt.IsShift() != pFrameView->IsOrthogonal();
        }

        if (!mpView->IsSnapEnabled())
            mpView->SetSnapEnabled(true);

        const bool bSnapModPressed = rMEvt.IsMod1();
        bool bGridSnap = pFrameView->IsGridSnap();
        bGridSnap = (bSnapModPressed != bGridSnap);

        if (mpView->IsGridSnap() != bGridSnap)
            mpView->SetGridSnap(bGridSnap);

        bool bBordSnap = pFrameView->IsBorderSnap();
        bBordSnap = (bSnapModPressed != bBordSnap);

        if (mpView->IsBorderSnap() != bBordSnap)
            mpView->SetBorderSnap(bBordSnap);

        bool bHlplSnap = pFrameView->IsHelplineSnap();
        bHlplSnap = (bSnapModPressed != bHlplSnap);

        if (mpView->IsHelplineSnap() != bHlplSnap)
            mpView->SetHelplineSnap(bHlplSnap);

        bool bOFrmSnap = pFrameView->IsOFrameSnap();
        bOFrmSnap = (bSnapModPressed != bOFrmSnap);

        if (mpView->IsOFrameSnap() != bOFrmSnap)
            mpView->SetOFrameSnap(bOFrmSnap);

        bool bOPntSnap = pFrameView->IsOPointSnap();
        bOPntSnap = (bSnapModPressed != bOPntSnap);

        if (mpView->IsOPointSnap() != bOPntSnap)
            mpView->SetOPointSnap(bOPntSnap);

        bool bOConSnap = pFrameView->IsOConnectorSnap();
        bOConSnap = (bSnapModPressed != bOConSnap);

        if (mpView->IsOConnectorSnap() != bOConSnap)
            mpView->SetOConnectorSnap(bOConSnap);

        bool bAngleSnap = rMEvt.IsShift() == !pFrameView->IsAngleSnapEnabled();

        if (mpView->IsAngleSnapEnabled() != bAngleSnap)
            mpView->SetAngleSnapEnabled(bAngleSnap);

        if (mpView->IsOrthogonal() != bOrtho)
            mpView->SetOrthogonal(bOrtho);

        bool bCenter = rMEvt.IsMod2();

        if ( mpView->IsCreate1stPointAsCenter() != bCenter ||
             mpView->IsResizeAtCenter() != bCenter )
        {
            mpView->SetCreate1stPointAsCenter(bCenter);
            mpView->SetResizeAtCenter(bCenter);
        }

        // #76572# look only for HelpLines when they are visible (!)
        bool bHelpLine(false);
        if(mpView->IsHlplVisible())
            bHelpLine = mpView->PickHelpLine(aMDPos, mpView->getHitTolLog(), nHelpLine);
        bool bHitHdl = (mpView->PickHandle(aMDPos) != NULL);

        if ( bHelpLine
            && !mpView->GetCreateObj()
            && ((SDREDITMODE_EDIT == mpView->GetViewEditMode()  && !bHitHdl) || (rMEvt.IsShift() && bSnapModPressed)) )
        {
            mpWindow->CaptureMouse();
            mpView->BegDragHelpLine(nHelpLine);
            bDragHelpLine = mpView->IsDragHelpLine();
            bReturn = true;
        }
    }
    ForcePointer(&rMEvt);

    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

bool FuDraw::MouseMove(const MouseEvent& rMEvt)
{
    FrameView* pFrameView = mpViewShell->GetFrameView();
    const basegfx::B2DPoint aPixelPos(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
    const basegfx::B2DPoint aLogicPos(mpWindow->GetInverseViewTransformation() * aPixelPos);
    bool bOrtho = false;
    bool bRestricted = true;

    if (mpView->IsDragObj())
    {
        // object is dragged (move, resize, ...)
        const SdrHdl* pHdl = mpView->GetDragStat().GetActiveHdl();

        if (!pHdl || (!pHdl->IsCornerHdl() && !pHdl->IsVertexHdl()))
        {
            // Move
            bRestricted = false;
        }
    }

    if (mpView->IsAction())
    {
        // #i33136#
        if(bRestricted && doConstructOrthogonal())
        {
            // Restrict movement:
            // rectangle->quadrat, ellipse->circle etc.
            bOrtho = !rMEvt.IsShift();
        }
        else
        {
            bOrtho = (bool)rMEvt.IsShift() != pFrameView->IsOrthogonal();
        }

        const bool bSnapModPressed = rMEvt.IsMod2();
        mpView->SetDragWithCopy(rMEvt.IsMod1() && pFrameView->IsDragWithCopy());

        bool bGridSnap = pFrameView->IsGridSnap();
        bGridSnap = (bSnapModPressed != bGridSnap);

        if (mpView->IsGridSnap() != bGridSnap)
            mpView->SetGridSnap(bGridSnap);

        bool bBordSnap = pFrameView->IsBorderSnap();
        bBordSnap = (bSnapModPressed != bBordSnap);

        if (mpView->IsBorderSnap() != bBordSnap)
            mpView->SetBorderSnap(bBordSnap);

        bool bHlplSnap = pFrameView->IsHelplineSnap();
        bHlplSnap = (bSnapModPressed != bHlplSnap);

        if (mpView->IsHelplineSnap() != bHlplSnap)
            mpView->SetHelplineSnap(bHlplSnap);

        bool bOFrmSnap = pFrameView->IsOFrameSnap();
        bOFrmSnap = (bSnapModPressed != bOFrmSnap);

        if (mpView->IsOFrameSnap() != bOFrmSnap)
            mpView->SetOFrameSnap(bOFrmSnap);

        bool bOPntSnap = pFrameView->IsOPointSnap();
        bOPntSnap = (bSnapModPressed != bOPntSnap);

        if (mpView->IsOPointSnap() != bOPntSnap)
            mpView->SetOPointSnap(bOPntSnap);

        bool bOConSnap = pFrameView->IsOConnectorSnap();
        bOConSnap = (bSnapModPressed != bOConSnap);

        if (mpView->IsOConnectorSnap() != bOConSnap)
            mpView->SetOConnectorSnap(bOConSnap);

        bool bAngleSnap = rMEvt.IsShift() == !pFrameView->IsAngleSnapEnabled();

        if (mpView->IsAngleSnapEnabled() != bAngleSnap)
            mpView->SetAngleSnapEnabled(bAngleSnap);

        if (mpView->IsOrthogonal() != bOrtho)
            mpView->SetOrthogonal(bOrtho);

        bool bCenter = rMEvt.IsMod2();

        if ( mpView->IsCreate1stPointAsCenter() != bCenter ||
             mpView->IsResizeAtCenter() != bCenter )
        {
            mpView->SetCreate1stPointAsCenter(bCenter);
            mpView->SetResizeAtCenter(bCenter);
        }

        if ( mpView->IsDragHelpLine() )
            mpView->MovDragHelpLine(aLogicPos);
    }

    bool bReturn = mpView->MouseMove(rMEvt, mpWindow);

    if (mpView->IsAction())
    {
        // Because the flag set back if necessary in MouseMove
        if (mpView->IsOrthogonal() != bOrtho)
            mpView->SetOrthogonal(bOrtho);
    }

    ForcePointer(&rMEvt);

    return bReturn;
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

bool FuDraw::MouseButtonUp(const MouseEvent& rMEvt)
{
    if ( mpView->IsDragHelpLine() )
        mpView->EndDragHelpLine();

    if ( bDragHelpLine )
    {
        Rectangle aOutputArea(Point(0,0), mpWindow->GetOutputSizePixel());

        if ( !aOutputArea.IsInside(rMEvt.GetPosPixel()) && mpView->GetSdrPageView())
        {
            mpView->GetSdrPageView()->DeleteHelpLine(nHelpLine);
        }

        mpWindow->ReleaseMouse();
    }

    FrameView* pFrameView = mpViewShell->GetFrameView();
    mpView->SetOrthogonal( pFrameView->IsOrthogonal() );
    mpView->SetAngleSnapEnabled( pFrameView->IsAngleSnapEnabled() );
    mpView->SetSnapEnabled(true);
    mpView->SetCreate1stPointAsCenter(false);
    mpView->SetResizeAtCenter(false);
    mpView->SetDragWithCopy(pFrameView->IsDragWithCopy());
    mpView->SetGridSnap(pFrameView->IsGridSnap());
    mpView->SetBorderSnap(pFrameView->IsBorderSnap());
    mpView->SetHelplineSnap(pFrameView->IsHelplineSnap());
    mpView->SetOFrameSnap(pFrameView->IsOFrameSnap());
    mpView->SetOPointSnap(pFrameView->IsOPointSnap());
    mpView->SetOConnectorSnap(pFrameView->IsOConnectorSnap());

    bIsInDragMode = false;
    ForcePointer(&rMEvt);
    FuPoor::MouseButtonUp(rMEvt);

    return false;
}

/*************************************************************************
|*
|* Process keyboard-events
|*
|* When processing a KeyEvent the returnvalue is true, otherwise false.
|*
\************************************************************************/

bool FuDraw::KeyInput(const KeyEvent& rKEvt)
{
    bool bReturn = false;

    switch ( rKEvt.GetKeyCode().GetCode() )
    {
        case KEY_ESCAPE:
        {
            bReturn = FuDraw::cancel();
        }
        break;

        case KEY_DELETE:
        case KEY_BACKSPACE:
        {
            if (!mpDocSh->IsReadOnly())
            {
                if ( mpView && mpView->IsPresObjSelected(false, true, false, true) )
                {
                    InfoBox(mpWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
                }
                else
                {
                    // Falls IP-Client aktiv, werden die Pointer
                    // auf das OLE- und das alte Graphic-Object
                    // am SdClient zurueckgesetzt, damit bei
                    // ::SelectionHasChanged nach dem Loeschen
                    // nicht mehr versucht wird, ein Grafik-Objekt
                    // zu restaurieren, das gar nicht mehr existiert.
                    // Alle anderen OLE-Objekte sind davon nicht
                    // betroffen (KA 06.10.95)
                    OSL_ASSERT (mpViewShell->GetViewShell()!=NULL);
                    Client* pIPClient = static_cast<Client*>(
                        mpViewShell->GetViewShell()->GetIPClient());
                    if (pIPClient && pIPClient->IsObjectInPlaceActive())
                        pIPClient->SetSdrGrafObj(NULL);

                    // wait-mousepointer while deleting object
                    WaitObject aWait( (Window*)mpViewShell->GetActiveWindow() );
                    // delete object
                    mpView->DeleteMarked();
                }
            }
            bReturn = true;
        }
        break;

        case KEY_TAB:
        {
            KeyCode aCode = rKEvt.GetKeyCode();

            if ( !aCode.IsMod1() && !aCode.IsMod2() )
            {
                // #105336# Moved next line which was a bugfix itself into
                // the scope which really does the object selection travel
                // and thus is allowed to call SelectionHasChanged().

                // Switch to FuSelect.
                mpViewShell->GetViewFrame()->GetDispatcher()->Execute(
                    SID_OBJECT_SELECT,
                    SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);

                // changeover to the next object
                if(!mpView->MarkNextObj( !aCode.IsShift() ))
                {
                    //IAccessibility2 Implementation 2009-----
                    //If there is only one object, don't do the UnmarkAlllObj() & MarkNextObj().
                    if ( mpView->getSelectedSdrObjectCount()  > 1 )
                    {
                        // #97016# No next object: go over open end and
                        // get first from the other side
                        mpView->UnmarkAllObj();
                        mpView->MarkNextObj(!aCode.IsShift());
                    }
                    //-----IAccessibility2 Implementation 2009
                }

                // #97016# II
                if(mpView->areSdrObjectsSelected())
                {
                    mpView->MakeVisibleAtView(mpView->getMarkedObjectSnapRange(), *mpWindow);
                }

                bReturn = true;
            }
        }
        break;

        case KEY_END:
        {
            KeyCode aCode = rKEvt.GetKeyCode();

            if ( aCode.IsMod1() )
            {
                // #97016# mark last object
                mpView->UnmarkAllObj();
                mpView->MarkNextObj(false);

                // #97016# II
                if(mpView->areSdrObjectsSelected())
                {
                    mpView->MakeVisibleAtView(mpView->getMarkedObjectSnapRange(), *mpWindow);
                }

                bReturn = true;
            }
        }
        break;

        case KEY_HOME:
        {
            KeyCode aCode = rKEvt.GetKeyCode();

            if ( aCode.IsMod1() )
            {
                // #97016# mark first object
                mpView->UnmarkAllObj();
                mpView->MarkNextObj(true);

                // #97016# II
                if(mpView->areSdrObjectsSelected())
                {
                    mpView->MakeVisibleAtView(mpView->getMarkedObjectSnapRange(), *mpWindow);
                }

                bReturn = true;
            }
        }
        break;

        default:
        break;
    }

    if (!bReturn)
    {
        bReturn = FuPoor::KeyInput(rKEvt);
    }
    else
    {
        mpWindow->ReleaseMouse();
    }

    return (bReturn);
}

/*************************************************************************
|*
|* Fade out the selection-presentation before scrolling
|*
\************************************************************************/

void FuDraw::ScrollStart()
{
}

/*************************************************************************
|*
|* After scrolling show the selection-presentation again
|*
\************************************************************************/

void FuDraw::ScrollEnd()
{
}

/*************************************************************************
|*
|* Aktivate function
|*
\************************************************************************/

void FuDraw::Activate()
{
    FuPoor::Activate();
    ForcePointer();
}

/*************************************************************************
|*
|* Deaktivate function
|*
\************************************************************************/

void FuDraw::Deactivate()
{
    FuPoor::Deactivate();
}


/*************************************************************************
|*
|* Toggle mouse-pointer
|*
\************************************************************************/

void FuDraw::ForcePointer(const MouseEvent* pMEvt)
{
    basegfx::B2DPoint aPnt;
    sal_uInt16 nModifier = 0;
    bool bLeftDown = false;
    bool bDefPointer = true;

    if (pMEvt)
    {
        const basegfx::B2DPoint aPixelPos(pMEvt->GetPosPixel().X(), pMEvt->GetPosPixel().Y());
        aPnt = mpWindow->GetInverseViewTransformation() * aPixelPos;
        nModifier = pMEvt->GetModifier();
        bLeftDown = pMEvt->IsLeft();
    }
    else
    {
        const basegfx::B2DPoint aPixelPos(mpWindow->GetPointerPosPixel().X(), mpWindow->GetPointerPosPixel().Y());
        aPnt = mpWindow->GetInverseViewTransformation() * aPixelPos;
    }

    if (mpView->IsDragObj())
    {
        if (SD_MOD()->GetWaterCan() && !mpView->PickHandle(aPnt))
        {
            /******************************************************************
            * Giesskannenmodus
            ******************************************************************/
            bDefPointer = false;
            mpWindow->SetPointer(Pointer(POINTER_FILL));
        }
    }
    else
    {
        SdrHdl* pHdl = mpView->PickHandle(aPnt);

        if (SD_MOD()->GetWaterCan() && !pHdl)
        {
            /******************************************************************
            * Giesskannenmodus
            ******************************************************************/
            bDefPointer = false;
            mpWindow->SetPointer(Pointer(POINTER_FILL));
        }
        else if (!pHdl &&
                 mpViewShell->GetViewFrame()->HasChildWindow(SvxBmpMaskChildWindow::GetChildWindowId()))
        {
            /******************************************************************
            * Pipettenmodus
            ******************************************************************/
            SvxBmpMask* pMask = (SvxBmpMask*) mpViewShell->GetViewFrame()->GetChildWindow(SvxBmpMaskChildWindow::GetChildWindowId())->GetWindow();

            if (pMask && pMask->IsEyedropping())
            {
                bDefPointer = false;
                mpWindow->SetPointer(Pointer(POINTER_REFHAND));
            }
        }
        else if (!mpView->IsAction())
        {
            SdrObject* pObj = NULL;
            SdrViewEvent aVEvt;
            SdrHitKind eHit = SDRHIT_NONE;
            const SdrDragMode eDragMode(mpView->GetDragMode());

            if (pMEvt)
            {
                eHit = mpView->PickAnything(*pMEvt, SDRMOUSEMOVE, aVEvt);
            }

            if ((SDRDRAG_ROTATE == eDragMode) && (SDRHIT_MARKEDOBJECT == eHit))
            {
                // The goal of this request is show always the rotation-arrow for 3D-objects at rotation-modus
                // Independent of the settings at Extras->Optionen->Grafik "Objekte immer verschieben"
                // 2D-objects acquit in an other way. Otherwise, the rotation of 3d-objects around any axises
                // wouldn't be possible per default.
                E3dObject* pSelected = dynamic_cast< E3dObject* >(mpView->getSelectedIfSingle());

                if(pSelected)
                {
                    mpWindow->SetPointer(Pointer(POINTER_ROTATE));
                    bDefPointer = false;     // Otherwise it'll be calles Joes routine and the mousepointer will reconfigurate again
                }
            }

            if (eHit == SDRHIT_NONE)
            {
                // found nothing -> look after at the masterpage
                mpView->PickObj(aPnt, mpView->getHitTolLog(), pObj, SDRSEARCH_ALSOONMASTER);
            }
            else if (eHit == SDRHIT_UNMARKEDOBJECT)
            {
                pObj = aVEvt.mpObj;
            }
            else if(SDRHIT_TEXTEDITOBJ == eHit && dynamic_cast< FuSelection* >(this))
            {
                sal_uInt16 nSdrObjKind = aVEvt.mpObj->GetObjIdentifier();

                if ( nSdrObjKind != OBJ_TEXT        &&
                     nSdrObjKind != OBJ_TITLETEXT   &&
                     nSdrObjKind != OBJ_OUTLINETEXT &&
                     aVEvt.mpObj->IsEmptyPresObj() )
                {
                    pObj = NULL;
                    bDefPointer = false;
                    mpWindow->SetPointer(Pointer(POINTER_ARROW));
                }
            }

            if (pObj && pMEvt && !pMEvt->IsMod2() && dynamic_cast< FuSelection* >(this))
            {
                // Auf Animation oder ImageMap pruefen
                bDefPointer = !SetPointer(pObj, aPnt);

                if (bDefPointer && (dynamic_cast< SdrObjGroup* >(pObj) || dynamic_cast< E3dScene* >(pObj)))
                {
                    // In die Gruppe hineinschauen
                    if (mpView->PickObj(aPnt, mpView->getHitTolLog(), pObj, SDRSEARCH_ALSOONMASTER | SDRSEARCH_DEEP))
                        bDefPointer = !SetPointer(pObj, aPnt);
                }
            }
        }
    }

    if (bDefPointer)
    {
        mpWindow->SetPointer(mpView->GetPreferedPointer(
                            aPnt, mpWindow, nModifier, bLeftDown));
    }
}

/*************************************************************************
|*
|* Set cursor for animaton or imagemap
|*
\************************************************************************/

bool FuDraw::SetPointer(SdrObject* pObj, const basegfx::B2DPoint& rPos)
{
    bool bSet = false;
    bool bAnimationInfo = (!dynamic_cast< GraphicDocShell* >(mpDocSh) && mpDoc->GetAnimationInfo(pObj)) ? true : false;
    bool bImageMapInfo = false;

    if (!bAnimationInfo)
        bImageMapInfo = mpDoc->GetIMapInfo(pObj) ? true : false;

    if (bAnimationInfo || bImageMapInfo)
    {
        const double fHitLog(mpView->getHitTolLog());
        const double f2HitLog(fHitLog * 2.0);
        const basegfx::B2DPoint aHitPosR(rPos.getX() + f2HitLog, rPos.getY());
        const basegfx::B2DPoint aHitPosL(rPos.getX() - f2HitLog, rPos.getY());
        const basegfx::B2DPoint aHitPosT(rPos.getX(), rPos.getY() + f2HitLog);
        const basegfx::B2DPoint aHitPosB(rPos.getX(), rPos.getY() - f2HitLog);

        if ( !pObj->IsClosedObj() ||
            ( SdrObjectPrimitiveHit(*pObj, aHitPosR, fHitLog, *mpView, false, 0) &&
              SdrObjectPrimitiveHit(*pObj, aHitPosL, fHitLog, *mpView, false, 0) &&
              SdrObjectPrimitiveHit(*pObj, aHitPosT, fHitLog, *mpView, false, 0) &&
              SdrObjectPrimitiveHit(*pObj, aHitPosB, fHitLog, *mpView, false, 0)))
        {
            /**********************************************************
            * hit inside the object (without margin) or open object
            ********************************************************/

            if (bAnimationInfo)
            {
                /******************************************************
                * Click-Action
                ******************************************************/
                SdAnimationInfo* pInfo = mpDoc->GetAnimationInfo(pObj);
                DrawView* pDrawView = dynamic_cast< DrawView* >(mpView);

                if ((pDrawView &&
                      (pInfo->meClickAction == presentation::ClickAction_BOOKMARK  ||
                       pInfo->meClickAction == presentation::ClickAction_DOCUMENT  ||
                       pInfo->meClickAction == presentation::ClickAction_PREVPAGE  ||
                       pInfo->meClickAction == presentation::ClickAction_NEXTPAGE  ||
                       pInfo->meClickAction == presentation::ClickAction_FIRSTPAGE ||
                       pInfo->meClickAction == presentation::ClickAction_LASTPAGE  ||
                       pInfo->meClickAction == presentation::ClickAction_VERB      ||
                       pInfo->meClickAction == presentation::ClickAction_PROGRAM   ||
                       pInfo->meClickAction == presentation::ClickAction_MACRO     ||
                       pInfo->meClickAction == presentation::ClickAction_SOUND))
                                                                    ||
                    (pDrawView &&
                        SlideShow::IsRunning( mpViewShell->GetViewShellBase() )   &&
                         (pInfo->meClickAction == presentation::ClickAction_VANISH            ||
                          pInfo->meClickAction == presentation::ClickAction_INVISIBLE         ||
                          pInfo->meClickAction == presentation::ClickAction_STOPPRESENTATION ||
                         (pInfo->mbActive &&
                          ( pInfo->meEffect != presentation::AnimationEffect_NONE ||
                            pInfo->meTextEffect != presentation::AnimationEffect_NONE )))))
                    {
                        // Animations-Objekt
                        bSet = true;
                        mpWindow->SetPointer(Pointer(POINTER_REFHAND));
                    }
            }
            else if (bImageMapInfo && mpDoc->GetHitIMapObject(pObj, rPos, *mpWindow))
            {
                /******************************************************
                * ImageMap
                ******************************************************/
                bSet = true;
                mpWindow->SetPointer(Pointer(POINTER_REFHAND));
            }
        }
    }

    return bSet;
}



/*************************************************************************
|*
|* Response of doubleclick
|*
\************************************************************************/

void FuDraw::DoubleClick(const MouseEvent& rMEvt)
{
    if ( mpView->areSdrObjectsSelected() )
    {
        const SdrObject* pSelected = mpView->getSelectedIfSingle();

        if (pSelected)
        {
            const sal_uInt32 nInv = pSelected->GetObjInventor();
            const sal_uInt16 nSdrObjKind = pSelected->GetObjIdentifier();

            if (nInv == SdrInventor && nSdrObjKind == OBJ_OLE2)
            {
                DrawDocShell* pDocSh = mpDoc->GetDocSh();

                if ( !pDocSh->IsUIActive() )
                {
                    /**********************************************************
                    * aktivate OLE-object
                    **********************************************************/
                    mpViewShell->ActivateObject( (SdrOle2Obj*) pSelected, 0);
                }
            }
            else if (nInv == SdrInventor &&  nSdrObjKind == OBJ_GRAF && pSelected->IsEmptyPresObj() )
            {
                mpViewShell->GetViewFrame()->
                    GetDispatcher()->Execute( SID_INSERT_GRAPHIC,
                                              SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
            }
            else if ( ( dynamic_cast< const SdrTextObj* >(pSelected) || dynamic_cast< const SdrObjGroup* >(pSelected) ) &&
                      !SD_MOD()->GetWaterCan()                            &&
                      mpViewShell->GetFrameView()->IsDoubleClickTextEdit() &&
                      !mpDocSh->IsReadOnly())
            {
                SfxUInt16Item aItem(SID_TEXTEDIT, 2);
                mpViewShell->GetViewFrame()->GetDispatcher()->
                                 Execute(SID_TEXTEDIT, SFX_CALLMODE_ASYNCHRON |
                                         SFX_CALLMODE_RECORD, &aItem, 0L);
            }
            else if (nInv == SdrInventor &&  nSdrObjKind == OBJ_GRUP)
            {
                // hit group -> select subobject
                mpView->UnmarkAllObj();
                mpView->MarkObj(aMDPos, mpView->getHitTolLog(), rMEvt.IsShift(), true);
            }
        }
    }
    else
    {
        mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
    }
}

/*************************************************************************
|*
|* Help-event
|*
\************************************************************************/

bool FuDraw::RequestHelp(const HelpEvent& rHEvt)
{
    bool bReturn = false;

    if (Help::IsBalloonHelpEnabled() || Help::IsQuickHelpEnabled())
    {
        SdrViewEvent aVEvt;

        MouseEvent aMEvt(mpWindow->GetPointerPosPixel(), 1, 0, MOUSE_LEFT);

        SdrHitKind eHit = mpView->PickAnything(aMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

        SdrObject* pObj = aVEvt.mpObj;

        if (eHit != SDRHIT_NONE && pObj != NULL)
        {
            Point aPosPixel = rHEvt.GetMousePosPixel();

            bReturn = SetHelpText(pObj, aPosPixel, aVEvt);

            if (!bReturn && (dynamic_cast< SdrObjGroup* >(pObj) || dynamic_cast< E3dScene* >(pObj)))
            {
                // In die Gruppe hineinschauen
                const Point aOutputPixel(mpWindow->ScreenToOutputPixel(aPosPixel));
                const basegfx::B2DPoint aPixelPos(aOutputPixel.X(), aOutputPixel.Y());
                const basegfx::B2DPoint aLogicPos(mpWindow->GetInverseViewTransformation() * aPixelPos);

                if (mpView->PickObj(aLogicPos, mpView->getHitTolLog(), pObj, SDRSEARCH_ALSOONMASTER | SDRSEARCH_DEEP))
                    bReturn = SetHelpText(pObj, aPosPixel, aVEvt);
            }
        }
    }

    if (!bReturn)
    {
        bReturn = FuPoor::RequestHelp(rHEvt);
    }

    return(bReturn);
}



/*************************************************************************
|*
|* Command-event
|*
\************************************************************************/

bool FuDraw::SetHelpText(SdrObject* pObj, const Point& rPosPixel, const SdrViewEvent& rVEvt)
{
    bool bSet = false;
    String aHelpText;
    const basegfx::B2DPoint aPixelPos(rPosPixel.X(), rPosPixel.Y());
    const basegfx::B2DPoint aLogicPos(mpWindow->GetInverseViewTransformation() * aPixelPos);

    // URL fuer IMapObject unter Pointer ist Hilfetext
    if ( mpDoc->GetIMapInfo(pObj) )
    {
        IMapObject* pIMapObj = mpDoc->GetHitIMapObject(pObj, aLogicPos, *mpWindow );

        if ( pIMapObj )
        {
            // show name
            aHelpText = pIMapObj->GetAltText();

            if (aHelpText.Len() == 0)
            {
                // show url if no name is available
                aHelpText = INetURLObject::decode( pIMapObj->GetURL(), '%', INetURLObject::DECODE_WITH_CHARSET );
            }
        }
    }
    else if (!dynamic_cast< GraphicDocShell* >(mpDocSh) && mpDoc->GetAnimationInfo(pObj))
    {
        SdAnimationInfo* pInfo = mpDoc->GetAnimationInfo(pObj);

        switch (pInfo->meClickAction)
        {
            case presentation::ClickAction_PREVPAGE:
            {
                // jump to the prior page
                aHelpText = String(SdResId(STR_CLICK_ACTION_PREVPAGE));
            }
            break;

            case presentation::ClickAction_NEXTPAGE:
            {
                // jump to the next page
                aHelpText = String(SdResId(STR_CLICK_ACTION_NEXTPAGE));
            }
            break;

            case presentation::ClickAction_FIRSTPAGE:
            {
                // jump to the first page
                aHelpText = String(SdResId(STR_CLICK_ACTION_FIRSTPAGE));
            }
            break;

            case presentation::ClickAction_LASTPAGE:
            {
                // jump to the last page
                aHelpText = String(SdResId(STR_CLICK_ACTION_LASTPAGE));
            }
            break;

            case presentation::ClickAction_BOOKMARK:
            {
                // jump to object/page
                aHelpText = String(SdResId(STR_CLICK_ACTION_BOOKMARK));
                aHelpText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );
                aHelpText.Append( String(INetURLObject::decode( pInfo->GetBookmark(), '%', INetURLObject::DECODE_WITH_CHARSET ) ));
            }
            break;

            case presentation::ClickAction_DOCUMENT:
            {
                // jump to document (object/page)
                aHelpText = String(SdResId(STR_CLICK_ACTION_DOCUMENT));
                aHelpText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );
                aHelpText.Append( String(INetURLObject::decode( pInfo->GetBookmark(), '%', INetURLObject::DECODE_WITH_CHARSET ) ));
            }
            break;

            case presentation::ClickAction_PROGRAM:
            {
                // execute program
                aHelpText = String(SdResId(STR_CLICK_ACTION_PROGRAM));
                aHelpText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );
                aHelpText.Append( String(INetURLObject::decode( pInfo->GetBookmark(), '%', INetURLObject::DECODE_WITH_CHARSET ) ));
            }
            break;

            case presentation::ClickAction_MACRO:
            {
                // execute program
                aHelpText = String(SdResId(STR_CLICK_ACTION_MACRO));
                aHelpText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );

                if ( SfxApplication::IsXScriptURL( pInfo->GetBookmark() ) )
                {
                    aHelpText.Append( pInfo->GetBookmark() );
                }
                else
                {
                    String sBookmark( pInfo->GetBookmark() );
                    sal_Unicode cToken = '.';
                    aHelpText.Append( sBookmark.GetToken( 2, cToken ) );
                    aHelpText.Append( cToken );
                    aHelpText.Append( sBookmark.GetToken( 1, cToken ) );
                    aHelpText.Append( cToken );
                    aHelpText.Append( sBookmark.GetToken( 0, cToken ) );
                }
            }
            break;

            case presentation::ClickAction_SOUND:
            {
                // play-back sound
                aHelpText = String(SdResId(STR_CLICK_ACTION_SOUND));
            }
            break;

            case presentation::ClickAction_VERB:
            {
                // execute OLE-verb
                aHelpText = String(SdResId(STR_CLICK_ACTION_VERB));
            }
            break;

            case presentation::ClickAction_STOPPRESENTATION:
            {
                // quit presentation
                aHelpText = String(SdResId(STR_CLICK_ACTION_STOPPRESENTATION));
            }
            break;
            default:
                break;
        }
    }
    else if (rVEvt.maURLField.Len())
    {
        /**************************************************************
        * URL-Field
        **************************************************************/
        aHelpText = INetURLObject::decode( rVEvt.maURLField, '%', INetURLObject::DECODE_WITH_CHARSET );
    }

    if (aHelpText.Len())
    {
        bSet = true;
        Rectangle aLogicPix = mpWindow->LogicToPixel(sdr::legacy::GetLogicRect(*pObj));
        Rectangle aScreenRect(mpWindow->OutputToScreenPixel(aLogicPix.TopLeft()),
                              mpWindow->OutputToScreenPixel(aLogicPix.BottomRight()));

        if (Help::IsBalloonHelpEnabled())
            Help::ShowBalloon( (Window*)mpWindow, rPosPixel, aScreenRect, aHelpText);
        else if (Help::IsQuickHelpEnabled())
            Help::ShowQuickHelp( (Window*)mpWindow, aScreenRect, aHelpText);
    }

    return bSet;
}


/** is called when the currenct function should be aborted. <p>
    This is used when a function gets a KEY_ESCAPE but can also
    be called directly.

    @returns true if a active function was aborted
*/
bool FuDraw::cancel()
{
    bool bReturn = false;

    if ( mpView->IsAction() )
    {
        mpView->BrkAction();
        bReturn = true;
    }
    else if ( mpView->IsTextEdit() )
    {
        mpView->SdrEndTextEdit();
        bReturn = true;

        SfxBindings& rBindings = mpViewShell->GetViewFrame()->GetBindings();
        rBindings.Invalidate( SID_PARASPACE_INCREASE );
        rBindings.Invalidate( SID_PARASPACE_DECREASE );
    }
    else if ( mpView->areSdrObjectsSelected() )
    {
        // #97016# II
        const SdrHdlList& rHdlList = mpView->GetHdlList();
        SdrHdl* pHdl = rHdlList.GetFocusHdl();

        if(pHdl)
        {
            ((SdrHdlList&)rHdlList).ResetFocusHdl();
        }
        else
        {
            mpView->UnmarkAll();
        }

        // Switch to FuSelect.
        mpViewShell->GetViewFrame()->GetDispatcher()->Execute(
            SID_OBJECT_SELECT,
            SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);

        bReturn = true;
    }

    return bReturn;
}

} // end of namespace sd
