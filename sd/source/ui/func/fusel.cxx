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

#include "fusel.hxx"
#include <vos/process.hxx>
#include <basic/sbstar.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdogrp.hxx>
#include "drawview.hxx"
#include <svtools/imapobj.hxx>
#include <svl/urihelper.hxx>
#include <unotools/localfilehelper.hxx>
#include <svx/svxids.hrc>
#include <svx/xfillit0.hxx>
#include <sfx2/app.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/aeitem.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <sfx2/dispatch.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/docfile.hxx>
#include <tools/debug.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>
#include <svx/svdotable.hxx>
#include <svx/svdtrans.hxx>
#include <svx/globl3d.hxx>
#include <svx/svdundo.hxx>
#include <avmedia/mediawindow.hxx>
#include <svx/sdrhittesthelper.hxx>
#include "app.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "GraphicDocShell.hxx"
#include "app.hxx"
#include "DrawDocShell.hxx"
#include "stlpool.hxx"
#include "anmdef.hxx"
#include "anminfo.hxx"
#include "fudraw.hxx"
#include "ViewShell.hxx"
#include "ViewShellBase.hxx"
#ifndef SD_FRAMW_VIEW_HXX
#include "FrameView.hxx"
#endif
#include "View.hxx"
#ifndef SD_WINDOW_SHELL_HXX
#include "Window.hxx"
#endif
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "DrawViewShell.hxx"
#include "ToolBarManager.hxx"
#include "pgjump.hxx"
#include "Client.hxx"
#include "slideshow.hxx"
#include <svx/scene3d.hxx>

using namespace ::com::sun::star;

namespace sd {

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuSelection::FuSelection (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
:   FuDraw(pViewSh, pWin, pView, pDoc, rReq),
    bTempRotation(false),
    bSelectionChanged(false),
    bHideAndAnimate(false),
    pHdl(NULL),
    bSuppressChangesOfSelection(false),
    bMirrorSide0(false),
    nEditMode(SID_BEZIER_MOVE),
    pWaterCanCandidate(NULL),
//IAccessibility2 Implementation 2009-----
  //Solution: Add Shift+UP/DOWN/LEFT/RIGHT key to move the position of insert point,
  //and SHIFT+ENTER key to decide the postion and draw the new insert point
    maOldPoint(0, 0),
    mbBeginInsertPoint(false),
    mbMovedToCenterPoint(false)
//-----IAccessibility2 Implementation 2009
{
}

FunctionReference FuSelection::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuSelection( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuSelection::DoExecute( SfxRequest& rReq )
{
    FuDraw::DoExecute( rReq );

    // Objektbar auswaehlen
    SelectionHasChanged();
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuSelection::~FuSelection()
{
    mpView->MarkPoints(0, true); // unmarkall
    mpView->ResetCreationActive();

    if ( mpView->GetDragMode() != SDRDRAG_MOVE )
    {
        mpView->SetDragMode(SDRDRAG_MOVE);
    }
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

bool FuSelection::MouseButtonDown(const MouseEvent& rMEvt)
{
    // Hack fuer #?????#
    bHideAndAnimate = false;

    pHdl = NULL;
    bool bReturn = FuDraw::MouseButtonDown(rMEvt);
    bool bWaterCan = SD_MOD()->GetWaterCan();
    const bool bReadOnly = mpDocSh->IsReadOnly();
    // When the right mouse button is pressed then only select objects
    // (and deselect others) as a preparation for showing the context
    // menu.
    const bool bSelectionOnly = rMEvt.IsRight();

    bMBDown = true;
    bSelectionChanged = false;

    if ( mpView->IsAction() )
    {
        if ( rMEvt.IsRight() )
            mpView->BckAction();
        return true;
    }

    // The following code is executed for right clicks as well as for left
    // clicks in order to modify the selection for the right button as a
    // preparation for the context menu.  The functions BegMarkObject() and
    // BegDragObject(), however, are not called for right clicks because a)
    // it makes no sense and b) to have IsAction() return false when called
    // from Command() which is a prerequisite for the context menu.
    if ((rMEvt.IsLeft() || rMEvt.IsRight())
        && !mpView->IsAction()
        && (mpView->IsFrameHandles() || !mpView->HasMarkablePoints()))
    {
        /******************************************************************
        * KEIN BEZIER_EDITOR
        ******************************************************************/
        mpWindow->CaptureMouse();
        pHdl = mpView->PickHandle(aMDPos);
        SdrObject* pObj;

        const basegfx::B2DVector aDelta(aMDPos - mpView->GetRef1());
        long nAngle0  = GetAngle(Point(basegfx::fround(aDelta.getX()), basegfx::fround(aDelta.getY())));
        nAngle0 -= 27000;
        nAngle0 = NormAngle360(nAngle0);
        bMirrorSide0 = (nAngle0 < 18000L);

        if (!pHdl && mpView->Is3DRotationCreationActive())
        {
            /******************************************************************
            * Wenn 3D-Rotationskoerper erstellt werden sollen, jetzt
            * die Erstellung beenden
            ******************************************************************/
            bSuppressChangesOfSelection = true;
            if(mpWindow)
                mpWindow->EnterWait();
            mpView->End3DCreation();
            bSuppressChangesOfSelection = false;
            mpView->ResetCreationActive();
            if(mpWindow)
                mpWindow->LeaveWait();
        }

        bool bTextEdit = false;
        SdrViewEvent aVEvt;
        SdrHitKind eHit = mpView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

        if ( eHit == SDRHIT_TEXTEDITOBJ && ( mpViewShell->GetFrameView()->IsQuickEdit() || dynamic_cast< sdr::table::SdrTableObj* >( aVEvt.mpObj ) != NULL ) )
        {
            bTextEdit = true;
        }

        if(!bTextEdit
            && !mpDocSh->IsReadOnly()
            && ((mpView->IsMarkedObjHit(aMDPos, mpView->getHitTolLog()) && !rMEvt.IsShift() && !rMEvt.IsMod2()) || pHdl != NULL)
            && (rMEvt.GetClicks() != 2)
            )
        {
            if (!pHdl && mpView->Is3DRotationCreationActive())
            {
                // Wechsel Rotationskoerper -> Selektion
                mpView->ResetCreationActive();
            }
            else if (bWaterCan)
            {
                // Remember the selected object for proper handling in
                // MouseButtonUp().
                pWaterCanCandidate = pickObject (aMDPos);
            }
            else
            {
                // Handle oder markiertes Objekt getroffen
                bFirstMouseMove = true;
                aDragTimer.Start();
            }

            if ( ! rMEvt.IsRight())
                mpView->BegDragObj(aMDPos, pHdl, mpView->getMinMovLog());
            bReturn = true;
        }
        else
        {
            if (!rMEvt.IsMod2() && mpView->PickObj(aMDPos, mpView->getHitTolLog(), pObj, SDRSEARCH_PICKMACRO))
            {
                mpView->BegMacroObj(aMDPos, mpView->getHitTolLog(), pObj, mpWindow);
                bReturn = true;
            }
            else if ( bTextEdit )
            {
                sal_uInt16 nSdrObjKind = aVEvt.mpObj->GetObjIdentifier();

                if (aVEvt.mpObj->GetObjInventor() == SdrInventor &&
                    (nSdrObjKind == OBJ_TEXT ||
                     nSdrObjKind == OBJ_TITLETEXT ||
                     nSdrObjKind == OBJ_OUTLINETEXT ||
                     !aVEvt.mpObj->IsEmptyPresObj()))
                {
                    // Seamless Editing: Verzweigen zur Texteingabe
                    if (!rMEvt.IsShift())
                        mpView->UnmarkAll();

                    SfxUInt16Item aItem(SID_TEXTEDIT, 1);
                    mpViewShell->GetViewFrame()->GetDispatcher()->
                    Execute(SID_TEXTEDIT, SFX_CALLMODE_SYNCHRON |
                            SFX_CALLMODE_RECORD, &aItem, 0L);
                    return bReturn; // VORSICHT, durch den synchronen Slot ist das objekt jetzt geloescht
                }
            }
            else if ( !rMEvt.IsMod2() && rMEvt.GetClicks() == 1 &&
                      aVEvt.meEvent == SDREVENT_EXECUTEURL )
             {
                mpWindow->ReleaseMouse();
                SfxStringItem aStrItem(SID_FILE_NAME, aVEvt.maURLField);
                SfxStringItem aReferer(SID_REFERER, mpDocSh->GetMedium()->GetName());
                SfxBoolItem aBrowseItem( SID_BROWSE, true );
                SfxViewFrame* pFrame = mpViewShell->GetViewFrame();
                mpWindow->ReleaseMouse();

                if (rMEvt.IsMod1())
                {
                    // Im neuen Frame oeffnen
                    pFrame->GetDispatcher()->Execute(SID_OPENDOC, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                                &aStrItem, &aBrowseItem, &aReferer, 0L);
                }
                else
                {
                    // Im aktuellen Frame oeffnen
                    SfxFrameItem aFrameItem(SID_DOCFRAME, pFrame);
                    pFrame->GetDispatcher()->Execute(SID_OPENDOC, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                                &aStrItem, &aFrameItem, &aBrowseItem, &aReferer, 0L);
                }

                bReturn = true;
            }
            else if(!rMEvt.IsMod2() && dynamic_cast< DrawViewShell* >(mpViewShell) )
            {
                if(mpView->PickObj(aMDPos, mpView->getHitTolLog(), pObj, SDRSEARCH_ALSOONMASTER))
                {
                    // Animate object when not just selecting.
                    if ( ! bSelectionOnly)
                        bReturn = AnimateObj(pObj, aMDPos);

                    if (!bReturn && (dynamic_cast< SdrObjGroup* >(pObj) || dynamic_cast< E3dScene* >(pObj)))
                    {
                        if(rMEvt.GetClicks() == 1)
                        {
                            // In die Gruppe hineinschauen
                            if (mpView->PickObj(aMDPos, mpView->getHitTolLog(), pObj, SDRSEARCH_ALSOONMASTER | SDRSEARCH_DEEP))
                                bReturn = AnimateObj(pObj, aMDPos);
                        }
                        else if( !bReadOnly && rMEvt.GetClicks() == 2)
                        {
                            // Neu: Doppelklick auf selektiertes Gruppenobjekt
                            // Gruppe betreten
                            if ( ! bSelectionOnly && pObj)
                            {
                                SdrPageView* pSdrPageView = mpView->GetSdrPageView();

                                if(pSdrPageView && pObj->getSdrPageFromSdrObject() == &pSdrPageView->getSdrPageFromSdrPageView())
                                {
                                    bReturn = pSdrPageView->EnterGroup(pObj);
                                }
                            }
                        }
                    }
                }

                // #i71727# replaced else here with two possibilities, once the original else (!pObj)
                // and also ignoring the found object when it's on a masterpage
                if(!pObj || (pObj->getSdrPageFromSdrObject() && pObj->getSdrPageFromSdrObject()->IsMasterPage()))
                {
                    if(mpView->IsGroupEntered() && 2 == rMEvt.GetClicks())
                    {
                        // New: double click on empty space/on obj on MasterPage, leave group
                        mpView->LeaveOneGroup();
                        bReturn = true;
                    }
                }
            }

            if (!bReturn)
            {
                if (bWaterCan)
                {
                    if ( ! (rMEvt.IsShift() || rMEvt.IsMod2()))
                    {
                        // Find the object under the current mouse position
                        // and store it for the MouseButtonUp() method to
                        // evaluate.
                        pWaterCanCandidate = pickObject (aMDPos);
                    }
                }
                else
                {
                    bReturn = true;
                    bool bDeactivateOLE = false;

                    if ( !rMEvt.IsShift() && !rMEvt.IsMod2() )
                    {
                        OSL_ASSERT (mpViewShell->GetViewShell()!=NULL);
                        Client* pIPClient = static_cast<Client*>(
                            mpViewShell->GetViewShell()->GetIPClient());

                        if (pIPClient && pIPClient->IsObjectInPlaceActive())
                        {
                            // OLE-Objekt wird im nachfolgenden UnmarkAll() deaktiviert
                            bDeactivateOLE = true;
                        }

                        mpView->UnmarkAll();
                    }

                    bool bMarked = false;

                    if ( !rMEvt.IsMod1() && !bDeactivateOLE)
                    {
                        if ( rMEvt.IsMod2() )
                        {
                            bMarked = mpView->MarkNextObj(aMDPos, mpView->getHitTolLog(), rMEvt.IsShift() );
                        }
                        else
                        {
                            bool bToggle = false;

                            if (rMEvt.IsShift() && mpView->getSelectedSdrObjectCount() > 1)
                            {
                                // Bei Einfachselektion kein Toggle
                                bToggle = true;
                            }

                            bMarked = mpView->MarkObj(aMDPos, mpView->getHitTolLog(), bToggle, false);
                        }
                    }

                    if( !bDeactivateOLE )
                    {
                        if ( !bReadOnly &&
                             bMarked                                                   &&
                             (!rMEvt.IsShift() || mpView->IsMarkedObjHit(aMDPos, mpView->getHitTolLog())))
                        {
                            /**********************************************************
                             * Objekt verschieben
                             **********************************************************/
                            aDragTimer.Start();

                            pHdl=mpView->PickHandle(aMDPos);
                            if ( ! rMEvt.IsRight())
                                mpView->BegDragObj(aMDPos, pHdl, mpView->getMinMovLog());
                        }
                        else
                        {
                            /**********************************************************
                             * Objekt selektieren
                             **********************************************************/
                            if ( ! rMEvt.IsRight())
                                mpView->BegMarkObj(aMDPos);
                        }
                    }

                    if( bMarked && bTempRotation && (nSlotId == SID_OBJECT_ROTATE) && !rMEvt.IsShift() && (rMEvt.GetClicks() != 2) )
                    {
                        nSlotId = SID_OBJECT_SELECT;
                        Activate();
                    }
                }
            }
        }
    }
    else if ( !bReadOnly
              && (rMEvt.IsLeft() || rMEvt.IsRight())
              && !mpView->IsAction())
    {
        /**********************************************************************
        * BEZIER-EDITOR
        **********************************************************************/
        mpWindow->CaptureMouse();
        SdrViewEvent aVEvt;
        SdrHitKind eHit = mpView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

        if (eHit == SDRHIT_HANDLE && aVEvt.mpHdl->GetKind() == HDL_BWGT)
        {
            /******************************************************************
            * Handle draggen
            ******************************************************************/
            if ( ! rMEvt.IsRight())
                mpView->BegDragObj(aMDPos, aVEvt.mpHdl, mpView->getMinMovLog());
        }
        else if (eHit == SDRHIT_MARKEDOBJECT && nEditMode == SID_BEZIER_INSERT)
        {
            /******************************************************************
            * Klebepunkt einfuegen
            ******************************************************************/
            mpView->BegInsObjPoint(aMDPos, rMEvt.IsMod1());
        }
        else if (eHit == SDRHIT_MARKEDOBJECT && rMEvt.IsMod1())
        {
            /******************************************************************
            * Klebepunkt selektieren
            ******************************************************************/
            if (!rMEvt.IsShift())
            {
                mpView->MarkPoints(0, true); // unmarkall
            }

            if ( ! rMEvt.IsRight())
            {
                mpView->BegMarkPoints(aMDPos);
            }
        }
        else if (eHit == SDRHIT_MARKEDOBJECT && !rMEvt.IsShift() && !rMEvt.IsMod2())
        {
            /******************************************************************
            * Objekt verschieben
            ******************************************************************/
            if ( ! rMEvt.IsRight())
                mpView->BegDragObj(aMDPos, NULL, mpView->getMinMovLog());
        }
        else if (eHit == SDRHIT_HANDLE)
        {
            /******************************************************************
            * Klebepunkt selektieren
            ******************************************************************/
            if (!mpView->IsPointMarked(*aVEvt.mpHdl) || rMEvt.IsShift())
            {
                if (!rMEvt.IsShift())
                {
                    // unmarkall
                    mpView->MarkPoints(0, true);

                    // update all SdrHdl before getting one
                    // mpView->forceSelectionChange(); TTTT: Should be done in PickHandle

                    // get one
                    pHdl = mpView->PickHandle(aMDPos);
                }
                else
                {
                    if (mpView->IsPointMarked(*aVEvt.mpHdl))
                    {
                        mpView->MarkPoint(const_cast< SdrHdl& >(*aVEvt.mpHdl), true); // unmark
                        pHdl = NULL;
                    }
                    else
                    {
                        pHdl = mpView->PickHandle(aMDPos);
                    }
                }

                if (pHdl)
                {
                    // mark this point. If it was not yet marked, this will change
                    // the selection (seel below)
                    mpView->MarkPoint(*pHdl);

                    if ( ! rMEvt.IsRight())
                    {
                        //if(mpView->isSelectionChangePending())
                        //{
                            // trigger evtl. outstanding selection change
                            // to get a valid SdrHdl with the next call
                            // mpView->forceSelectionChange(); TTTT: Should be done in PickHandle

                            pHdl = mpView->PickHandle(aMDPos);
                        //}

                        if(pHdl)
                        {
                            mpView->BegDragObj(aMDPos, pHdl, mpView->getMinMovLog());
                        }
                        else
                        {
                            OSL_ENSURE(false, "OOps, got no new SdrHdl after after selection change (!)");
                        }
                    }
                }
            }
            else
            {
                // #90239# point IS marked and NO shift is pressed. Start
                // dragging of selected point(s)
                pHdl = mpView->PickHandle(aMDPos);
                if(pHdl)
                    if ( ! rMEvt.IsRight())
                        mpView->BegDragObj(aMDPos, pHdl, mpView->getMinMovLog());
            }
        }
        else
        {
            /******************************************************************
            * Objekt selektieren oder draggen
            ******************************************************************/
            if (!rMEvt.IsShift() && !rMEvt.IsMod2() && eHit == SDRHIT_UNMARKEDOBJECT)
            {
               mpView->UnmarkAllObj();
            }

            bool bMarked = false;

            if (!rMEvt.IsMod1())
            {
                if (rMEvt.IsMod2())
                {
                    bMarked = mpView->MarkNextObj(aMDPos, mpView->getHitTolLog(), rMEvt.IsShift());
                }
                else
                {
                    bMarked = mpView->MarkObj(aMDPos, mpView->getHitTolLog(), rMEvt.IsShift(), false);
                }
            }

            if (bMarked &&
                (!rMEvt.IsShift() || eHit == SDRHIT_MARKEDOBJECT))
            {
                // Objekt verschieben
                if ( ! rMEvt.IsRight())
                    mpView->BegDragObj(aMDPos, aVEvt.mpHdl, mpView->getMinMovLog());
            }
            else if (mpView->areSdrObjectsSelected())
            {
                /**************************************************************
                * Klebepunkt selektieren
                **************************************************************/
                if (!rMEvt.IsShift())
                {
                    mpView->MarkPoints(0, true); // unmarkall
                }

                if ( ! rMEvt.IsRight())
                {
                    mpView->BegMarkPoints(aMDPos);
                }
            }
            else
            {
                /**************************************************************
                * Objekt selektieren
                **************************************************************/
                if ( ! rMEvt.IsRight())
                {
                    mpView->BegMarkObj(aMDPos);
                }
            }

            ForcePointer(&rMEvt);
        }
    }

    if (!bIsInDragMode)
    {
        ForcePointer(&rMEvt);
    }

    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

bool FuSelection::MouseMove(const MouseEvent& rMEvt)
{
    bool bReturn = FuDraw::MouseMove(rMEvt);

    if (aDragTimer.IsActive())
    {
        if(bFirstMouseMove)
        {
            bFirstMouseMove = false;
        }
        else
        {
            aDragTimer.Stop();
        }
    }

    if (mpView->IsAction())
    {
        const basegfx::B2DPoint aPix(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
        const basegfx::B2DPoint aLogicPos(mpWindow->GetInverseViewTransformation() * aPix);

        ForceScroll(aPix);

        if (mpView->IsInsObjPoint())
        {
            mpView->MovInsObjPoint(aLogicPos);
        }
        else
        {
            mpView->MovAction(aLogicPos);
        }
    }

    ForcePointer(&rMEvt);

    return (bReturn);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

bool FuSelection::MouseButtonUp(const MouseEvent& rMEvt)
{
    bool bReturn = false;
    // When the right mouse button is pressed then only select objects
    // (and deselect others) as a preparation for showing the context
    // menu.
    const bool bSelectionOnly = rMEvt.IsRight();

    if (bHideAndAnimate)
    {
        // Animation laeuft noch -> sofort returnieren
        bHideAndAnimate = false;
        pHdl = NULL;
        mpWindow->ReleaseMouse();
        return(true);
    }

    if (aDragTimer.IsActive() )
    {
        aDragTimer.Stop();
        bIsInDragMode = false;
    }

    if( !mpView )
        return false;

    // force trigger selection change to get bSelectionChanged
    // set in FuSelection::SelectionHasChanged() call eventually
    // mpView->forceSelectionChange(); TTTT: Should be done in HasMarkablePoints
    const basegfx::B2DPoint aPixelPos(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
    const basegfx::B2DPoint aLogicPos(mpWindow->GetInverseViewTransformation() * aPixelPos);

    if (mpView->IsFrameHandles() || !mpView->HasMarkablePoints())
    {
        /**********************************************************************
        * KEIN BEZIER_EDITOR
        **********************************************************************/
        if ( mpView->IsDragObj() )
        {
            /******************************************************************
            * Objekt wurde verschoben
            ******************************************************************/
            FrameView* pFrameView = mpViewShell->GetFrameView();
            bool bDragWithCopy = (rMEvt.IsMod1() && pFrameView->IsDragWithCopy());

            if (bDragWithCopy)
            {
                bDragWithCopy = !mpView->IsPresObjSelected(false, true);
            }

            mpView->SetDragWithCopy(bDragWithCopy);
            mpView->EndDragObj( mpView->IsDragWithCopy() );

            if (!rMEvt.IsShift() && !rMEvt.IsMod1() && !rMEvt.IsMod2() &&
                !bSelectionChanged                   &&
                fabs(aLogicPos.getX() - aMDPos.getX()) < mpView->getMinMovLog() &&
                fabs(aLogicPos.getY() - aMDPos.getY()) < mpView->getMinMovLog())
            {
                /**************************************************************
                * Toggle zw. Selektion und Rotation
                **************************************************************/
                SdrObject* pSingleObj = mpView->getSelectedIfSingle();

                if (nSlotId == SID_OBJECT_SELECT
                    && mpView->IsRotateAllowed()

                    && (rMEvt.GetClicks() != 2)
                    && (mpViewShell->GetFrameView()->IsClickChangeRotation()
                        || (pSingleObj
                            && pSingleObj->GetObjInventor()==E3dInventor))
                    && ! bSelectionOnly)

                {
                    bTempRotation = true;
                    nSlotId = SID_OBJECT_ROTATE;
                    Activate();
                }
                else if (nSlotId == SID_OBJECT_ROTATE)
                {
                    nSlotId = SID_OBJECT_SELECT;
                    Activate();
                }
            }
            else if (nSlotId == SID_CONVERT_TO_3D_LATHE)
            {
                if (!pHdl)
                {
                    bSuppressChangesOfSelection = true;
                    mpView->Start3DCreation();
                    bSuppressChangesOfSelection = false;
                }
                else if (pHdl->GetKind() != HDL_MIRX &&
                         pHdl->GetKind() != HDL_REF1 &&
                         pHdl->GetKind() != HDL_REF2 && mpView->Is3DRotationCreationActive())
                {
                    /*********************************************************
                    * Wenn 3D-Rotationskoerper erstellt werden sollen, jetzt
                    * die Erstellung beenden
                    **********************************************************/
                    const basegfx::B2DVector aDelta(aLogicPos - mpView->GetRef1());
                    long nAngle1  = GetAngle(Point(basegfx::fround(aDelta.getX()), basegfx::fround(aDelta.getY())));
                    nAngle1 -= 27000;
                    nAngle1 = NormAngle360(nAngle1);
                    bool bMirrorSide1 = bool (nAngle1 < 18000L);

                     if (bMirrorSide0 != bMirrorSide1)
                     {
                        bSuppressChangesOfSelection = true;
                        if(mpWindow)
                            mpWindow->EnterWait();
                         mpView->End3DCreation();
                        bSuppressChangesOfSelection = false;
                         nSlotId = SID_OBJECT_SELECT;
                        if(mpWindow)
                            mpWindow->LeaveWait();
                         Activate();
                    }
                }
            }
        }
        else if (rMEvt.IsMod1()
            && !rMEvt.IsMod2()
            && fabs(aLogicPos.getX() - aMDPos.getX()) < mpView->getMinMovLog()
            && fabs(aLogicPos.getY() - aMDPos.getY()) < mpView->getMinMovLog())
        {
            // Gruppe betreten
            mpView->MarkObj(aLogicPos, mpView->getHitTolLog(), rMEvt.IsShift(), rMEvt.IsMod1());
        }

        if (mpView->IsAction() )
        {
            mpView->EndAction();
        }

        if( SD_MOD()->GetWaterCan() )
        {
            if( rMEvt.IsRight() )
            {
                // Bei rechter Maustaste wird im Giesskannenmodus ein Undo ausgefuehrt
                mpViewShell->GetViewFrame()->GetDispatcher()->Execute( SID_UNDO, SFX_CALLMODE_ASYNCHRON );
            }
            else if (pWaterCanCandidate != NULL)
            {
                // Is the candiate object still under the mouse?
                if (pickObject (aLogicPos) == pWaterCanCandidate)
                {
                    SdStyleSheetPool* pPool = static_cast<SdStyleSheetPool*>(
                        mpDocSh->GetStyleSheetPool());
                    if (pPool != NULL)
                    {
                        SfxStyleSheet* pStyleSheet = static_cast<SfxStyleSheet*>(
                            pPool->GetActualStyleSheet());
                        if (pStyleSheet != NULL && mpView->IsUndoEnabled() )
                        {
                            // #108981#
                            // Added UNDOs for the WaterCan mode. This was never done in
                            // the past, thus it was missing all the time.
                            SdrUndoAction* pUndoAttr = mpDoc->GetSdrUndoFactory().CreateUndoAttrObject(*pWaterCanCandidate, sal_True, sal_True);
                            mpView->BegUndo(pUndoAttr->GetComment());
                            mpView->AddUndo(mpDoc->GetSdrUndoFactory().CreateUndoGeoObject(*pWaterCanCandidate));
                            mpView->AddUndo(pUndoAttr);

                            pWaterCanCandidate->SetStyleSheet (pStyleSheet, false);

                            // #108981#
                            mpView->EndUndo();
                        }
                    }
                }
            }
            // else when there has been no object under the mouse when the
            // button was pressed then nothing happens even when there is
            // one now.
        }

        sal_uInt16 nClicks = rMEvt.GetClicks();

        if (nClicks == 2 && rMEvt.IsLeft() && bMBDown &&
            !rMEvt.IsMod1() && !rMEvt.IsShift() )
        {
            DoubleClick(rMEvt);
        }

        bMBDown = false;

        ForcePointer(&rMEvt);
        pHdl = NULL;
        mpWindow->ReleaseMouse();
        SdrObject* pSingleObj = mpView->getSelectedIfSingle();

        if ( (nSlotId != SID_OBJECT_SELECT && !mpView->areSdrObjectsSelected()) ||
             ( mpView->GetDragMode() == SDRDRAG_CROOK &&
              !mpView->IsCrookAllowed( mpView->IsCrookNoContortion() ) ) ||
             ( mpView->GetDragMode() == SDRDRAG_SHEAR &&
              !mpView->IsShearAllowed() && !mpView->IsDistortAllowed() ) ||
             ( nSlotId==SID_CONVERT_TO_3D_LATHE && pSingleObj &&
              (pSingleObj->GetObjInventor() != SdrInventor         ||
               pSingleObj->GetObjIdentifier() == OBJ_MEASURE) ) )
        {
            bReturn = true;
            ForcePointer(&rMEvt);
            pHdl = NULL;
            mpWindow->ReleaseMouse();
            FuDraw::MouseButtonUp(rMEvt);
            mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_SYNCHRON);
            return bReturn; // VORSICHT, durch den synchronen Slot ist das objekt jetzt geloescht
        }

        FuDraw::MouseButtonUp(rMEvt);
    }
    else
    {
        /**********************************************************************
        * BEZIER_EDITOR
        **********************************************************************/
        if ( mpView->IsAction() )
        {
            if ( mpView->IsInsObjPoint() )
            {
                mpView->EndInsObjPoint(SDRCREATE_FORCEEND);
            }
            else if ( mpView->IsDragObj() )
            {
                FrameView* pFrameView = mpViewShell->GetFrameView();
                bool bDragWithCopy = (rMEvt.IsMod1() && pFrameView->IsDragWithCopy());

                if (bDragWithCopy)
                {
                    bDragWithCopy = !mpView->IsPresObjSelected(false, true);
                }

                mpView->SetDragWithCopy(bDragWithCopy);
                mpView->EndDragObj( mpView->IsDragWithCopy() );
            }
            else
            {
                mpView->EndAction();
                const basegfx::B2DVector aDelta(aMDPos - aLogicPos);

                if(fabs(aDelta.getX()) < mpView->getMinMovLog()
                    && fabs(aDelta.getY()) < mpView->getMinMovLog()
                    && !rMEvt.IsShift()
                    && !rMEvt.IsMod2())
                {
                    SdrViewEvent aVEvt;
                    SdrHitKind eHit = mpView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

                    if (eHit == SDRHIT_NONE)
                    {
                        // Klick auf der Stelle: deselektieren
                        mpView->UnmarkAllObj();
                    }
                }
            }
        }
        else if (!rMEvt.IsShift() && rMEvt.IsMod1() && !rMEvt.IsMod2() &&
                 fabs(aLogicPos.getX() - aMDPos.getX()) < mpView->getMinMovLog() &&
                 fabs(aLogicPos.getY() - aMDPos.getY()) < mpView->getMinMovLog())
        {
            // Gruppe betreten
            mpView->MarkObj(aLogicPos, mpView->getHitTolLog(), false, rMEvt.IsMod1());
        }


        ForcePointer(&rMEvt);
        pHdl = NULL;
        mpWindow->ReleaseMouse();

        FuDraw::MouseButtonUp(rMEvt);
    }

    return (bReturn);
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert true, andernfalls
|* false.
|*
\************************************************************************/

bool FuSelection::KeyInput(const KeyEvent& rKEvt)
{
    bool bReturn = false;

    switch (rKEvt.GetKeyCode().GetCode())
    {
        case KEY_ESCAPE:
        {
            bReturn = FuSelection::cancel();
        }
        break;
//IAccessibility2 Implementation 2009-----
  //Solution: add keyboard operation for insert points in drawing curve
        case KEY_UP:
        case KEY_DOWN:
        case KEY_LEFT:
        case KEY_RIGHT:
        {
            if(rKEvt.GetKeyCode().IsShift()&&(nEditMode == SID_BEZIER_INSERT))
            {
                long nX = 0;
                long nY = 0;
                sal_uInt16  nCode = rKEvt.GetKeyCode().GetCode();
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

                const basegfx::B2DPoint aSelectCenterPixel(
                    mpWindow->GetViewTransformation() * mpView->getMarkedObjectSnapRange().getCenter());
                const Point aPoint(mbMovedToCenterPoint
                    ? maOldPoint
                    : Point(basegfx::fround(aSelectCenterPixel.getX()), basegfx::fround(aSelectCenterPixel.getY())));
                const Point ePoint(aPoint + Point(nX, nY));

                mpWindow->SetPointerPosPixel(ePoint);

                //simulate mouse move action
                MouseEvent eMevt(ePoint, 1, 2, MOUSE_LEFT, 0);
                MouseMove(eMevt);

                maOldPoint = ePoint;
                mbMovedToCenterPoint = true;
                bReturn = true;
            }
        }
        break;
        case KEY_RETURN:
            if(rKEvt.GetKeyCode().IsShift()&&(nEditMode == SID_BEZIER_INSERT))
            {
                if(!mbBeginInsertPoint)
                {
                    //simulate mouse button down action
                    MouseEvent aMevt(maOldPoint, 1, 3, MOUSE_LEFT, KEY_SHIFT);
                    MouseButtonDown(aMevt);
                    mpWindow->CaptureMouse();
                    mbBeginInsertPoint = true;
                }
                else
                {
                    //simulate mouse button up action
                    MouseEvent rMEvt(maOldPoint, 1, 17, MOUSE_LEFT, KEY_SHIFT);
                    MouseButtonUp(rMEvt);
                    mbBeginInsertPoint = false;
                }
                bReturn= sal_True;
            }
            break;
    }
//-----IAccessibility2 Implementation 2009
    if (!bReturn)
    {
        bReturn = FuDraw::KeyInput(rKEvt);

        if(!mpView->areSdrObjectsSelected())
        {
            mpView->ResetCreationActive();

            mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
        }
    }

    return(bReturn);

}


//IAccessibility2 Implementation 2009-----
void FuSelection::ForcePointer(const MouseEvent* pMEvt)
{
    if(mbMovedToCenterPoint && !mbBeginInsertPoint && pMEvt)
    {
        MouseEvent aMEvt(pMEvt->GetPosPixel(), pMEvt->GetClicks(),
            pMEvt->GetMode(), pMEvt->GetButtons(), pMEvt->GetModifier() & ~KEY_SHIFT);
        FuDraw::ForcePointer(&aMEvt);
    }
    else
    {
        FuDraw::ForcePointer(pMEvt);
    }
}
//-----IAccessibility2 Implementation 2009
/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuSelection::Activate()
{
    SdrDragMode eMode;
    mpView->ResetCreationActive();
    mpView->SetViewEditMode(SDREDITMODE_EDIT);

    switch( nSlotId )
    {
        case SID_OBJECT_ROTATE:
        {
            // (gemapter) Slot wird explizit auf Rotate gesetzt #31052#
            DrawViewShell* pDrawViewShell = dynamic_cast< DrawViewShell* >(mpViewShell);

            if( pDrawViewShell )
            {
                sal_uInt16* pSlotArray = pDrawViewShell->GetSlotArray();
                pSlotArray[ 1 ] = SID_OBJECT_ROTATE;
            }

            eMode = SDRDRAG_ROTATE;

            if ( mpView->GetDragMode() != eMode )
                mpView->SetDragMode(eMode);
        }
        break;

        case SID_OBJECT_MIRROR:
        {
            eMode = SDRDRAG_MIRROR;

            if ( mpView->GetDragMode() != eMode )
                mpView->SetDragMode(eMode);
        }
        break;

        case SID_OBJECT_CROP:
        {
            eMode = SDRDRAG_CROP;

            if ( mpView->GetDragMode() != eMode )
                mpView->SetDragMode(eMode);
        }
        break;

        case SID_OBJECT_TRANSPARENCE:
        {
            eMode = SDRDRAG_TRANSPARENCE;

            if ( mpView->GetDragMode() != eMode )
                mpView->SetDragMode(eMode);
        }
        break;

        case SID_OBJECT_GRADIENT:
        {
            eMode = SDRDRAG_GRADIENT;

            if ( mpView->GetDragMode() != eMode )
                mpView->SetDragMode(eMode);
        }
        break;

        case SID_OBJECT_SHEAR:
        {
            eMode = SDRDRAG_SHEAR;

            if ( mpView->GetDragMode() != eMode )
                mpView->SetDragMode(eMode);
        }
        break;

        case SID_OBJECT_CROOK_ROTATE:
        {
            eMode = SDRDRAG_CROOK;

            if ( mpView->GetDragMode() != eMode )
            {
                mpView->SetDragMode(eMode);
                mpView->SetCrookMode(SDRCROOK_ROTATE);
            }
        }
        break;

        case SID_OBJECT_CROOK_SLANT:
        {
            eMode = SDRDRAG_CROOK;

            if ( mpView->GetDragMode() != eMode )
            {
                mpView->SetDragMode(eMode);
                mpView->SetCrookMode(SDRCROOK_SLANT);
            }
        }
        break;

        case SID_OBJECT_CROOK_STRETCH:
        {
            eMode = SDRDRAG_CROOK;

            if ( mpView->GetDragMode() != eMode )
            {
                mpView->SetDragMode(eMode);
                mpView->SetCrookMode(SDRCROOK_STRETCH);
            }
        }
        break;

        case SID_CONVERT_TO_3D_LATHE:
        {
            eMode = SDRDRAG_MIRROR;
            bSuppressChangesOfSelection = true;

            if ( mpView->GetDragMode() != eMode )
                mpView->SetDragMode(eMode);

            if (!mpView->Is3DRotationCreationActive())
                mpView->Start3DCreation();

            bSuppressChangesOfSelection = false;
        }
        break;

        default:
        {
            eMode = SDRDRAG_MOVE;

            if ( mpView->GetDragMode() != eMode )
                mpView->SetDragMode(eMode);
        }
        break;
    }

    if (nSlotId != SID_OBJECT_ROTATE)
    {
        bTempRotation = false;
    }

    FuDraw::Activate();
}



/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuSelection::Deactivate()
{
    FuDraw::Deactivate();
}


/*************************************************************************
|*
|* Selektion hat sich geaendert
|*
\************************************************************************/

void FuSelection::SelectionHasChanged()
{
    bSelectionChanged = true;

    FuDraw::SelectionHasChanged();

    if ((mpView->Is3DRotationCreationActive() && !bSuppressChangesOfSelection))
    {
        // Wechsel Rotationskoerper -> Selektion
        mpView->ResetCreationActive();
        nSlotId = SID_OBJECT_SELECT;
        Activate();
    }

    // Activate the right tool bar for the current context of the view.
    mpViewShell->GetViewShellBase().GetToolBarManager()->SelectionHasChanged(*mpViewShell, *mpView);
}


/*************************************************************************
|*
|* Aktuellen Bezier-Editmodus setzen
|*
\************************************************************************/

void FuSelection::SetEditMode(sal_uInt16 nMode)
{
    nEditMode = nMode;

    if (nEditMode == SID_BEZIER_INSERT)
    {
        mpView->SetInsObjPointMode(true);
    }
    else
    {
        mpView->SetInsObjPointMode(false);
    }

    ForcePointer();

    SfxBindings& rBindings = mpViewShell->GetViewFrame()->GetBindings();
    rBindings.Invalidate(SID_BEZIER_MOVE);
    rBindings.Invalidate(SID_BEZIER_INSERT);
}

/*************************************************************************
|*
|* Animation oder Interaktion ausfuehren
|*
\************************************************************************/

bool FuSelection::AnimateObj(SdrObject* pObj, const basegfx::B2DPoint& rPos)
    {
    bool bAnimated(false);
    const bool bClosed(pObj->IsClosedObj());
    const bool bFilled(bClosed? pObj->HasFillStyle() : false);
    double f2HitLog(mpView->getHitTolLog() * 2);

    const basegfx::B2DPoint aHitPosR(rPos.getX() + f2HitLog, rPos.getY());
    const basegfx::B2DPoint aHitPosL(rPos.getX() - f2HitLog, rPos.getY());
    const basegfx::B2DPoint aHitPosT(rPos.getX(), rPos.getY() + f2HitLog);
    const basegfx::B2DPoint aHitPosB(rPos.getX(), rPos.getY() - f2HitLog);

    if ( !bClosed                                      ||
         !bFilled                                      ||
         (SdrObjectPrimitiveHit(*pObj, aHitPosR, mpView->getHitTolLog(), *mpView, false, 0) &&
          SdrObjectPrimitiveHit(*pObj, aHitPosL, mpView->getHitTolLog(), *mpView, false, 0) &&
          SdrObjectPrimitiveHit(*pObj, aHitPosT, mpView->getHitTolLog(), *mpView, false, 0) &&
          SdrObjectPrimitiveHit(*pObj, aHitPosB, mpView->getHitTolLog(), *mpView, false, 0) ) )
    {
        if ( mpDoc->GetIMapInfo( pObj ) )
        {
            const IMapObject* pIMapObj = mpDoc->GetHitIMapObject( pObj, rPos, *mpWindow );

            if ( pIMapObj && pIMapObj->GetURL().Len() )
            {
                // Sprung zu Dokument
                mpWindow->ReleaseMouse();
                SfxStringItem aStrItem(SID_FILE_NAME, pIMapObj->GetURL());
                SfxStringItem aReferer(SID_REFERER, mpDocSh->GetMedium()->GetName());
                SfxViewFrame* pFrame = mpViewShell->GetViewFrame();
                SfxFrameItem aFrameItem(SID_DOCFRAME, pFrame);
                SfxBoolItem aBrowseItem( SID_BROWSE, true );
                mpWindow->ReleaseMouse();
                pFrame->GetDispatcher()->
                    Execute(SID_OPENDOC, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                            &aStrItem, &aFrameItem, &aBrowseItem, &aReferer, 0L);

                bAnimated = true;
            }
        }
        else if(!dynamic_cast< GraphicDocShell* >(mpDocSh)
            && dynamic_cast< DrawView* >(mpView)
            && mpDoc->GetAnimationInfo(pObj))
        {
            /**********************************************************
            * Animations-Objekt in der Mitte getroffen -> Interaktion
            **********************************************************/
            SdAnimationInfo* pInfo = mpDoc->GetAnimationInfo(pObj);
            DrawViewShell* pDrViewSh = static_cast<DrawViewShell*>(mpViewShell);
            mpWindow->ReleaseMouse();

            switch (pInfo->meClickAction)
            {
                case presentation::ClickAction_BOOKMARK:
                {
                     // Sprung zu Bookmark (Seite oder Objekt)
                    SfxStringItem aItem(SID_NAVIGATOR_OBJECT, pInfo->GetBookmark());
                    mpViewShell->GetViewFrame()->GetDispatcher()->
                    Execute(SID_NAVIGATOR_OBJECT, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD, &aItem, 0L);
                    bAnimated = true;
                }
                break;

                case presentation::ClickAction_DOCUMENT:
                {
                    String sBookmark( pInfo->GetBookmark() );
                    // Sprung zu Dokument
                    if (sBookmark.Len())
                    {
                        SfxStringItem aReferer(SID_REFERER, mpDocSh->GetMedium()->GetName());
                        SfxStringItem aStrItem(SID_FILE_NAME, sBookmark);
                        SfxViewFrame* pFrame = mpViewShell->GetViewFrame();
                        SfxFrameItem aFrameItem(SID_DOCFRAME, pFrame);
                        SfxBoolItem aBrowseItem( SID_BROWSE, true );
                        pFrame->GetDispatcher()->
                        Execute(SID_OPENDOC, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                                &aStrItem, &aFrameItem, &aBrowseItem, &aReferer, 0L);
                    }

                    bAnimated = true;
                }
                break;

                case presentation::ClickAction_PREVPAGE:
                {
                    // Sprung zur vorherigen Seite
                    SfxUInt16Item aItem(SID_NAVIGATOR_PAGE, PAGE_PREVIOUS);
                    mpViewShell->GetViewFrame()->GetDispatcher()->
                    Execute(SID_NAVIGATOR_PAGE, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD,
                            &aItem, 0L);
                    bAnimated = true;
                }
                break;

                case presentation::ClickAction_NEXTPAGE:
                {
                    // Sprung zur naechsten Seite
                    SfxUInt16Item aItem(SID_NAVIGATOR_PAGE, PAGE_NEXT);
                    mpViewShell->GetViewFrame()->GetDispatcher()->
                    Execute(SID_NAVIGATOR_PAGE, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD,
                            &aItem, 0L);
                    bAnimated = true;
                }
                break;

                case presentation::ClickAction_FIRSTPAGE:
                {
                    // Sprung zu erster Seite
                    SfxUInt16Item aItem(SID_NAVIGATOR_PAGE, PAGE_FIRST);
                    mpViewShell->GetViewFrame()->GetDispatcher()->
                    Execute(SID_NAVIGATOR_PAGE, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD,
                            &aItem, 0L);
                    bAnimated = true;
                }
                break;

                case presentation::ClickAction_LASTPAGE:
                {
                    // Sprung zu letzter Seite
                    SfxUInt16Item aItem(SID_NAVIGATOR_PAGE, PAGE_LAST);
                    mpViewShell->GetViewFrame()->GetDispatcher()->
                    Execute(SID_NAVIGATOR_PAGE, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD,
                            &aItem, 0L);
                    bAnimated = true;
                }
                break;

                case presentation::ClickAction_SOUND:
                {
                        try
                        {
                            mxPlayer.set( avmedia::MediaWindow::createPlayer( pInfo->GetBookmark()), uno::UNO_QUERY_THROW );
                            mxPlayer->start();
                        }
                        catch( uno::Exception& e )
                        {
                            (void)e;
                        }
                    bAnimated = true;
                }
                break;

                case presentation::ClickAction_VERB:
                {
                    // Verb zuweisen
                    mpView->UnmarkAll();
                    mpView->MarkObj(*pObj, false );
                    pDrViewSh->DoVerb((sal_Int16)pInfo->mnVerb);
                    bAnimated = true;
                }
                break;

                case presentation::ClickAction_PROGRAM:
                {
                   String aBaseURL = GetDocSh()->GetMedium()->GetBaseURL();
                   INetURLObject aURL( ::URIHelper::SmartRel2Abs( INetURLObject(aBaseURL), pInfo->GetBookmark(),
                                                URIHelper::GetMaybeFileHdl(), true, false,
                                                INetURLObject::WAS_ENCODED, INetURLObject::DECODE_UNAMBIGUOUS ) );

                   if( INET_PROT_FILE == aURL.GetProtocol() )
                   {
                        SfxStringItem aUrl( SID_FILE_NAME, aURL.GetMainURL( INetURLObject::NO_DECODE ) );
                        SfxBoolItem aBrowsing( SID_BROWSE, true );

                        SfxViewFrame* pViewFrm = SfxViewFrame::Current();
                        if (pViewFrm)
                            pViewFrm->GetDispatcher()->Execute( SID_OPENDOC,
                                                          SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                                                        &aUrl,
                                                        &aBrowsing,
                                                        0L );
                   }

                    bAnimated = true;
                }
                break;

                case presentation::ClickAction_MACRO:
                {
                    // Execute makro
                    String aMacro = pInfo->GetBookmark();

                    if ( SfxApplication::IsXScriptURL( aMacro ) )
                    {
                        uno::Any aRet;
                        uno::Sequence< sal_Int16 > aOutArgsIndex;
                        uno::Sequence< uno::Any > aOutArgs;
                        uno::Sequence< uno::Any >* pInArgs =
                            new uno::Sequence< uno::Any >(0);

                        ErrCode eErr = mpDocSh->CallXScript( aMacro,
                            *pInArgs, aRet, aOutArgsIndex, aOutArgs);

                        // Check the return value from the script
                        bool bTmp = false;
                        if ( eErr == ERRCODE_NONE &&
                             aRet.getValueType() == getCppuBooleanType() &&
                             sal_True == ( aRet >>= bTmp ) &&
                             bTmp == true )
                        {
                            bAnimated = true;
                        }
                        else
                        {
                            bAnimated = false;
                        }
                    }
                    else
                    {
                        // aMacro has got following format:
                        // "Macroname.Modulname.Libname.Documentname" or
                        // "Macroname.Modulname.Libname.Applicationsname"
                        String aMacroName =
                            aMacro.GetToken(0, sal_Unicode('.'));
                        String aModulName =
                             aMacro.GetToken(1, sal_Unicode('.'));
                        String aLibName   =
                             aMacro.GetToken(2, sal_Unicode('.'));
                        String aDocName   =
                             aMacro.GetToken(3, sal_Unicode('.'));

                        // In this moment the Call-method only
                        // resolves modulename+macroname
                        String aExecMacro(aModulName);
                        aExecMacro.Append( sal_Unicode('.') );
                        aExecMacro.Append( aMacroName );
                        bAnimated = mpDocSh->GetBasic()->Call(aExecMacro);
                    }
                }
                break;

                default:
                {
                    bAnimated = false;
                }
                break;
            }
        }

        if (!bAnimated
            && dynamic_cast< DrawView* >(mpView)
            && !dynamic_cast< GraphicDocShell* >(mpDocSh)
            && SlideShow::IsRunning( mpViewShell->GetViewShellBase() )
            && mpDoc->GetAnimationInfo(pObj))
        {
            /**********************************************************
            * Effekt-Objekt in der Mitte getroffen -> Effekt abspielen
            **********************************************************/
            SdAnimationInfo* pInfo = mpDoc->GetAnimationInfo(pObj);

            switch (pInfo->meClickAction)
            {
                case presentation::ClickAction_VANISH:
                case presentation::ClickAction_INVISIBLE:
                    break;

                default:
                    bAnimated = false;
                break;
            }
        }
    }

    return bAnimated;
}



/** is called when the currenct function should be aborted. <p>
    This is used when a function gets a KEY_ESCAPE but can also
    be called directly.

    @returns true if a active function was aborted
*/
bool FuSelection::cancel()
{
    if (mpView->Is3DRotationCreationActive())
    {
        mpView->ResetCreationActive();
        mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
        return true;
    }
    else
    {
        return false;
    }
}




SdrObject* FuSelection::pickObject (const basegfx::B2DPoint& rTestPoint)
{
    SdrObject* pObject = 0;

    mpView->PickObj(rTestPoint, mpView->getHitTolLog(), pObject, SDRSEARCH_PICKMARKABLE);

    return pObject;
}
} // end of namespace sd
