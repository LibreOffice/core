/*************************************************************************
 *
 *  $RCSfile: fusel.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: aw $ $Date: 2000-10-30 11:46:16 $
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

#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif
#ifndef _SB_SBSTAR_HXX //autogen
#include <basic/sbstar.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _SVDOGRP_HXX //autogen
#include <svx/svdogrp.hxx>
#endif
#ifndef _E3D_POLYSC3D_HXX //autogen
#include <svx/polysc3d.hxx>
#endif
#include "drawview.hxx"
#ifndef _GOODIES_IMAPOBJ_HXX //autogen
#include <svtools/imapobj.hxx>
#endif
#ifndef _SV_SOUND_HXX //autogen
#include <vcl/sound.hxx>
#endif

#include <svtools/urihelper.hxx>
#include <svx/svxids.hrc>
#include <svx/xfillit0.hxx>
//#include <basctl/idetemp.hxx>

#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif
#define ITEMID_FIELD    EE_FEATURE_FIELD
#include <svx/flditem.hxx>

#include "app.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"

#include <svx/svdtrans.hxx>
#include "grdocsh.hxx"
#include "app.hxx"
#include "docshell.hxx"
#include "stlpool.hxx"
#include "anmdef.hxx"
#include "anminfo.hxx"
#include "fudraw.hxx"
#include "fusel.hxx"
#include "viewshel.hxx"
#include "frmview.hxx"
#include "sdview.hxx"
#include "sdwindow.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "drviewsh.hxx"
#include "pgjump.hxx"
#include <svx/globl3d.hxx>
#include "sdclient.hxx"

using namespace ::com::sun::star;

extern USHORT SlotArray[];

TYPEINIT1( FuSelection, FuDraw );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuSelection::FuSelection(SdViewShell*     pViewSh,
                         SdWindow*        pWin,
                         SdView*          pView,
                         SdDrawDocument*  pDoc,
                         SfxRequest&      rReq) :
    FuDraw(pViewSh, pWin, pView, pDoc, rReq),
    bTempRotation(FALSE),
    bHideAndAnimate(FALSE),
    bSelectionChanged(FALSE),
    pHdl(NULL),
    bSuppressChangesOfSelection(FALSE),
    bMirrorSide0(FALSE),
    nEditMode(SID_BEZIER_MOVE),
    pSound(NULL)
{
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
    HPUX_DTOR_BUG;
    pView->UnmarkAllPoints();
    pView->ShowMirrored();
    pView->ResetCreationActive();
    delete pSound;
    pSound = NULL;

    if ( pView->GetDragMode() != SDRDRAG_MOVE )
    {
        pView->SetDragMode(SDRDRAG_MOVE);
    }
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

BOOL FuSelection::MouseButtonDown(const MouseEvent& rMEvt)
{
    // Hack fuer #?????#
    bHideAndAnimate = FALSE;

    pHdl = NULL;
    BOOL bReturn = FuDraw::MouseButtonDown(rMEvt);
    BOOL bWaterCan = SD_MOD()->GetWaterCan();

    bMBDown = TRUE;
    bSelectionChanged = FALSE;

    if ( pView->IsAction() )
    {
        if ( rMEvt.IsRight() )
            pView->BckAction();
        return TRUE;
    }

    USHORT nDrgLog = USHORT ( pWindow->PixelToLogic(Size(DRGPIX,0)).Width() );
    USHORT nHitLog = USHORT ( pWindow->PixelToLogic(Size(HITPIX,0)).Width() );

    if (rMEvt.IsLeft() && !pView->IsAction() &&
        (pView->IsFrameDragSingles() || !pView->HasMarkablePoints()))
    {
        /******************************************************************
        * KEIN BEZIER_EDITOR
        ******************************************************************/
        pWindow->CaptureMouse();
        pHdl = pView->HitHandle(aMDPos, *pWindow);
        SdrObject* pObj;
        SdrPageView* pPV;

        long nAngle0  = GetAngle(aMDPos - pView->GetRef1());
        nAngle0 -= 27000;
        nAngle0 = NormAngle360(nAngle0);
        bMirrorSide0 = BOOL (nAngle0 < 18000L);

        if (!pHdl && pView->IsCreationActive())
        {
            /******************************************************************
            * Wenn 3D-Rotationskoerper erstellt werden sollen, jetzt
            * die Erstellung beenden
            ******************************************************************/
            bSuppressChangesOfSelection = TRUE;
            if(pWindow)
                pWindow->EnterWait();
            pView->End3DCreation();
            bSuppressChangesOfSelection = FALSE;
            pView->ResetCreationActive();
            if(pWindow)
                pWindow->LeaveWait();
        }

        BOOL bTextEdit = FALSE;
        SdrViewEvent aVEvt;
        SdrHitKind eHit = pView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

        if ( pViewShell->GetFrameView()->IsQuickEdit() && eHit == SDRHIT_TEXTEDITOBJ )
        {
            bTextEdit = TRUE;
        }

        if(!bTextEdit
            && !pDocSh->IsReadOnly()
            && ((pView->IsMarkedHit(aMDPos, nHitLog) && !rMEvt.IsShift() && !rMEvt.IsMod2()) || pHdl != NULL)
            && (rMEvt.GetClicks() != 2)
            )
        {
            if (!pHdl && pView->IsCreationActive())
            {
                // Wechsel Rotationskoerper -> Selektion
                pView->ShowMirrored();
                pView->ResetCreationActive();
            }
            else
            {
                // Handle oder markiertes Objekt getroffen
                bFirstMouseMove = TRUE;
                aDragTimer.Start();
            }

            pView->BegDragObj(aMDPos, (OutputDevice*) NULL, pHdl, nDrgLog);
            bReturn = TRUE;
        }
        else
        {
            if (!rMEvt.IsMod2() && pView->PickObj(aMDPos, pObj, pPV, SDRSEARCH_PICKMACRO))
            {
                pView->BegMacroObj(aMDPos, nHitLog, pObj, pPV, pWindow);
                bReturn = TRUE;
            }
            else if ( bTextEdit )
            {
                UINT16 nSdrObjKind = aVEvt.pObj->GetObjIdentifier();

                if (aVEvt.pObj->GetObjInventor() == SdrInventor &&
                    (nSdrObjKind == OBJ_TEXT ||
                     nSdrObjKind == OBJ_TITLETEXT ||
                     nSdrObjKind == OBJ_OUTLINETEXT ||
                     !aVEvt.pObj->IsEmptyPresObj()))
                {
                    // Seamless Editing: Verzweigen zur Texteingabe
                    if (!rMEvt.IsShift())
                        pView->UnmarkAll();

                    SfxUInt16Item aItem(SID_TEXTEDIT, 1);
                    pViewShell->GetViewFrame()->GetDispatcher()->
                    Execute(SID_TEXTEDIT, SFX_CALLMODE_SYNCHRON |
                            SFX_CALLMODE_RECORD, &aItem, 0L);
                    return bReturn; // VORSICHT, durch den synchronen Slot ist das objekt jetzt geloescht
                }
            }
            else if ( !rMEvt.IsMod2() && rMEvt.GetClicks() == 1 &&
                      aVEvt.eEvent == SDREVENT_EXECUTEURL )
             {
                pWindow->ReleaseMouse();
                SfxStringItem aStrItem(SID_FILE_NAME, aVEvt.pURLField->GetURL());
                SfxStringItem aReferer(SID_REFERER, pDocSh->GetMedium()->GetName());
                SfxBoolItem aBrowseItem( SID_BROWSING, TRUE );
                SfxViewFrame* pFrame = pViewShell->GetViewFrame();
                pWindow->ReleaseMouse();

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

                bReturn = TRUE;
            }
            else if(!rMEvt.IsMod2()
                && pViewShell->ISA(SdDrawViewShell)
                )
            {
                if(pView->PickObj(aMDPos, pObj, pPV, SDRSEARCH_ALSOONMASTER))
                {
                    // Objekt animieren
                    bReturn = AnimateObj(pObj, aMDPos);

                    if (!bReturn && (pObj->ISA(SdrObjGroup) || pObj->ISA(E3dPolyScene)))
                    {
                        if(rMEvt.GetClicks() == 1)
                        {
                            // In die Gruppe hineinschauen
                            if (pView->PickObj(aMDPos, pObj, pPV, SDRSEARCH_ALSOONMASTER | SDRSEARCH_DEEP))
                                bReturn = AnimateObj(pObj, aMDPos);
                        }
                        else if(rMEvt.GetClicks() == 2)
                        {
                            // Neu: Doppelklick auf selektiertes Gruppenobjekt
                            // Gruppe betreten
                            if(pObj && pObj->GetPage() == pPV->GetPage())
                                bReturn = pPV->EnterGroup(pObj);
                        }
                    }
                }
                else
                {
                    if(rMEvt.GetClicks() == 2)
                    {
                        // Neu: Doppelklick ins Leere
                        // Gruppe verlassen
                        pPV = pView->GetPageViewPvNum(0);

                        if(pPV && pPV->GetAktGroup())
                        {
                            pPV->LeaveOneGroup();
                        }
                        bReturn = TRUE;
                    }
                }
            }

            if (!bReturn)
            {
                bReturn = TRUE;
                BOOL bDeactivateOLE = FALSE;

                if ( !rMEvt.IsShift() && !rMEvt.IsMod2() )
                {
                    SdClient* pIPClient = (SdClient*) pViewShell->GetIPClient();

                    if (pIPClient && pIPClient->IsInPlaceActive())
                    {
                        // OLE-Objekt wird im nachfolgenden UnmarkAll() deaktiviert
                        bDeactivateOLE = TRUE;
                    }

                    pView->UnmarkAll();
                }

                BOOL bMarked = FALSE;

                if (!rMEvt.IsMod1() && !bDeactivateOLE)
                {
                    if ( rMEvt.IsMod2() )
                    {
                        bMarked = pView->MarkNextObj(aMDPos, nHitLog, rMEvt.IsShift() );
                    }
                    else
                    {
                        BOOL bToggle = FALSE;

                        if (rMEvt.IsShift() && pView->GetMarkList().GetMarkCount() > 1)
                        {
                            // Bei Einfachselektion kein Toggle
                            bToggle = TRUE;
                        }

                        bMarked = pView->MarkObj(aMDPos, nHitLog, bToggle, FALSE);
                    }
                }

                if (bMarked                                                   &&
                    (!rMEvt.IsShift() || pView->IsMarkedHit(aMDPos, nHitLog)) &&
                    !pDocSh->IsReadOnly())
                {
                    /**********************************************************
                    * Objekt verschieben
                    **********************************************************/
                    aDragTimer.Start();

                    pHdl=pView->HitHandle(aMDPos, *pWindow);
                    pView->BegDragObj(aMDPos, (OutputDevice*) NULL, pHdl, nDrgLog);
                }
                else
                {
                    /**********************************************************
                    * Objekt selektieren
                    **********************************************************/
                    pView->BegMarkObj(aMDPos);
                }
            }
        }
    }
    else if (rMEvt.IsLeft() && !pView->IsAction())
    {
        /**********************************************************************
        * BEZIER-EDITOR
        **********************************************************************/
        pWindow->CaptureMouse();
        SdrViewEvent aVEvt;
        SdrHitKind eHit = pView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

        if (eHit == SDRHIT_HANDLE && aVEvt.pHdl->GetKind() == HDL_BWGT)
        {
            /******************************************************************
            * Handle draggen
            ******************************************************************/
            pView->BegDragObj(aMDPos, (OutputDevice*) NULL, aVEvt.pHdl, nDrgLog);
        }
        else if (eHit == SDRHIT_MARKEDOBJECT && nEditMode == SID_BEZIER_INSERT)
        {
            /******************************************************************
            * Klebepunkt einfuegen
            ******************************************************************/
            pView->BegInsObjPoint(aMDPos, rMEvt.IsMod1(), NULL, 0);
        }
        else if (eHit == SDRHIT_MARKEDOBJECT && rMEvt.IsMod1())
        {
            /******************************************************************
            * Klebepunkt selektieren
            ******************************************************************/
            if (!rMEvt.IsShift())
                pView->UnmarkAllPoints();

            pView->BegMarkPoints(aMDPos, (OutputDevice*) NULL);
        }
        else if (eHit == SDRHIT_MARKEDOBJECT && !rMEvt.IsShift() && !rMEvt.IsMod2())
        {
            /******************************************************************
            * Objekt verschieben
            ******************************************************************/
            pView->BegDragObj(aMDPos, (OutputDevice*) NULL, NULL, nDrgLog);
        }
        else if (eHit == SDRHIT_HANDLE)
        {
            /******************************************************************
            * Klebepunkt selektieren
            ******************************************************************/
            if (!pView->IsPointMarked(*aVEvt.pHdl) || rMEvt.IsShift())
            {
                if (!rMEvt.IsShift())
                {
                    pView->UnmarkAllPoints();
                    pHdl = pView->HitHandle(aMDPos, *pWindow);
                }
                else
                {
                    if (pView->IsPointMarked(*aVEvt.pHdl))
                    {
                        pView->UnmarkPoint(*aVEvt.pHdl);
                        pHdl = NULL;
                    }
                    else
                    {
                        pHdl = pView->HitHandle(aMDPos, *pWindow);
                    }
                }

                if (pHdl)
                {
                    pView->MarkPoint(*pHdl);
                    pView->BegDragObj(aMDPos, (OutputDevice*) NULL, pHdl, nDrgLog);
                }
            }
        }
        else
        {
            /******************************************************************
            * Objekt selektieren oder draggen
            ******************************************************************/
            if (!rMEvt.IsShift() && !rMEvt.IsMod2() && eHit == SDRHIT_UNMARKEDOBJECT)
            {
               pView->UnmarkAllObj();
            }

            BOOL bMarked = FALSE;

            if (!rMEvt.IsMod1())
            {
                if (rMEvt.IsMod2())
                {
                    bMarked = pView->MarkNextObj(aMDPos, nHitLog, rMEvt.IsShift());
                }
                else
                {
                    bMarked = pView->MarkObj(aMDPos, nHitLog, rMEvt.IsShift(), FALSE);
                }
            }

            if (bMarked &&
                (!rMEvt.IsShift() || eHit == SDRHIT_MARKEDOBJECT))
            {
                // Objekt verschieben
                pView->BegDragObj(aMDPos, (OutputDevice*) NULL, aVEvt.pHdl, nDrgLog);
            }
            else if (pView->HasMarkedObj())
            {
                /**************************************************************
                * Klebepunkt selektieren
                **************************************************************/
                if (!rMEvt.IsShift())
                    pView->UnmarkAllPoints();

                pView->BegMarkPoints(aMDPos, (OutputDevice*) NULL);
            }
            else
            {
                /**************************************************************
                * Objekt selektieren
                **************************************************************/
                pView->BegMarkObj(aMDPos);
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

BOOL FuSelection::MouseMove(const MouseEvent& rMEvt)
{
    BOOL bReturn = FuDraw::MouseMove(rMEvt);

    if (aDragTimer.IsActive())
    {
        if(bFirstMouseMove)
        {
            bFirstMouseMove = FALSE;
        }
        else
        {
            aDragTimer.Stop();
        }
    }

    if (pView->IsAction())
    {
        Point aPix(rMEvt.GetPosPixel());
        Point aPnt(pWindow->PixelToLogic(aPix));

        ForceScroll(aPix);

        if (pView->IsInsObjPoint())
        {
            pView->MovInsObjPoint(aPnt);
        }
        else
        {
            pView->MovAction(aPnt);
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

BOOL FuSelection::MouseButtonUp(const MouseEvent& rMEvt)
{
    BOOL bReturn = FALSE;

    if (bHideAndAnimate)
    {
        // Animation laeuft noch -> sofort returnieren
        bHideAndAnimate = FALSE;
        pHdl = NULL;
        pWindow->ReleaseMouse();
        return(TRUE);
    }

    if (aDragTimer.IsActive() )
    {
        aDragTimer.Stop();
        bIsInDragMode = FALSE;
    }

    Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
    USHORT nHitLog = USHORT ( pWindow->PixelToLogic(Size(HITPIX,0)).Width() );
    USHORT nDrgLog = USHORT ( pWindow->PixelToLogic(Size(DRGPIX,0)).Width() );

    if (pView->IsFrameDragSingles() || !pView->HasMarkablePoints())
    {
        /**********************************************************************
        * KEIN BEZIER_EDITOR
        **********************************************************************/
        if ( pView->IsDragObj() )
        {
            /******************************************************************
            * Objekt wurde verschoben
            ******************************************************************/
            FrameView* pFrameView = pViewShell->GetFrameView();
            BOOL bDragWithCopy = (rMEvt.IsMod1() && pFrameView->IsDragWithCopy());

            if (bDragWithCopy)
            {
                bDragWithCopy = !pView->IsPresObjSelected(FALSE, TRUE);
            }

            pView->SetDragWithCopy(bDragWithCopy);
            pView->EndDragObj( pView->IsDragWithCopy() );

            pView->ForceMarkedToAnotherPage();

            if (!rMEvt.IsShift() && !rMEvt.IsMod1() && !rMEvt.IsMod2() &&
                !bSelectionChanged                   &&
                Abs(aPnt.X() - aMDPos.X()) < nDrgLog &&
                Abs(aPnt.Y() - aMDPos.Y()) < nDrgLog)
            {
                /**************************************************************
                * Toggle zw. Selektion und Rotation
                **************************************************************/
                SdrObject* pSingleObj = NULL;
                ULONG nMarkCount = pView->GetMarkList().GetMarkCount();

                if (nMarkCount==1)
                {
                    pSingleObj = pView->GetMarkList().GetMark(0)->GetObj();
                }

                if (nSlotId == SID_OBJECT_SELECT && pView->IsRotateAllowed() &&
                     (pViewShell->GetFrameView()->IsClickChangeRotation() ||
                      (pSingleObj && pSingleObj->GetObjInventor()==E3dInventor)))

                {
                    bTempRotation = TRUE;
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
                    bSuppressChangesOfSelection = TRUE;
                    pView->Start3DCreation();
                    bSuppressChangesOfSelection = FALSE;
                }
                else if (pHdl->GetKind() != HDL_MIRX &&
                         pHdl->GetKind() != HDL_REF1 &&
                         pHdl->GetKind() != HDL_REF2 && pView->IsCreationActive())
                {
                    /*********************************************************
                    * Wenn 3D-Rotationskoerper erstellt werden sollen, jetzt
                    * die Erstellung beenden
                    **********************************************************/
                     long nAngle1  = GetAngle(aPnt - pView->GetRef1());
                     nAngle1 -= 27000;
                     nAngle1 = NormAngle360(nAngle1);
                     BOOL bMirrorSide1 = BOOL (nAngle1 < 18000L);

                     if (bMirrorSide0 != bMirrorSide1)
                     {
                         bSuppressChangesOfSelection = TRUE;
                        if(pWindow)
                            pWindow->EnterWait();
                         pView->End3DCreation();
                         bSuppressChangesOfSelection = FALSE;
                         nSlotId = SID_OBJECT_SELECT;
                        if(pWindow)
                            pWindow->LeaveWait();
                         Activate();
                    }
                }
            }
        }
        else if (rMEvt.IsMod1() && !rMEvt.IsMod2() &&
                 Abs(aPnt.X() - aMDPos.X()) < nDrgLog &&
                 Abs(aPnt.Y() - aMDPos.Y()) < nDrgLog)
        {
            // Gruppe betreten
            pView->MarkObj(aPnt, nHitLog, rMEvt.IsShift(), rMEvt.IsMod1());
        }

        if (pView->IsAction() )
        {
            pView->EndAction();
        }

        if( SD_MOD()->GetWaterCan() )
        {
            if( rMEvt.IsRight() )
            {
                // Bei rechter Maustaste wird im Giesskannenmodus ein Undo ausgefuehrt
                pViewShell->GetViewFrame()->GetDispatcher()->Execute( SID_UNDO, SFX_CALLMODE_ASYNCHRON );
            }
            else if( pView->HasMarkedObj() )
            // Wenn wir im Giesskannenmodus sind und Objekte markiert sind
            {
                SfxStyleSheetBasePool* pSSPool = pDocSh->GetStyleSheetPool();
                if( pSSPool )
                {
                    SfxStyleSheetBase* pStyleSheet = ( (SdStyleSheetPool*) pSSPool )->GetActualStyleSheet();
                    if( pStyleSheet )
                        pView->SetStyleSheetToMarked( (SfxStyleSheet*) pStyleSheet, FALSE );
                }
            }
        }

        USHORT nClicks = rMEvt.GetClicks();

        if (nClicks == 2 && rMEvt.IsLeft() && bMBDown &&
            !rMEvt.IsMod1() && !rMEvt.IsShift() )
        {
            DoubleClick(rMEvt);
        }

        bMBDown = FALSE;

        ForcePointer(&rMEvt);
        pHdl = NULL;
        pWindow->ReleaseMouse();
        SdrObject* pSingleObj = NULL;
        ULONG nMarkCount = pView->GetMarkList().GetMarkCount();

        if (nMarkCount==1)
        {
            pSingleObj = pView->GetMarkList().GetMark(0)->GetObj();
        }

        if ( nSlotId != SID_OBJECT_SELECT                            &&
             (/* bTempRotation && */ nMarkCount==0)                    ||
             ( pView->GetDragMode() == SDRDRAG_CROOK &&
              !pView->IsCrookAllowed( pView->IsCrookNoContortion() ) ) ||
             ( pView->GetDragMode() == SDRDRAG_SHEAR &&
              !pView->IsShearAllowed() && !pView->IsDistortAllowed() ) ||
             ( nSlotId==SID_CONVERT_TO_3D_LATHE && pSingleObj &&
              (pSingleObj->GetObjInventor() != SdrInventor         ||
               pSingleObj->GetObjIdentifier() == OBJ_MEASURE) ) )
        {
            bReturn = TRUE;
            ForcePointer(&rMEvt);
            pHdl = NULL;
            pWindow->ReleaseMouse();
            FuDraw::MouseButtonUp(rMEvt);
            pViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_SYNCHRON);
            return bReturn; // VORSICHT, durch den synchronen Slot ist das objekt jetzt geloescht
        }

        FuDraw::MouseButtonUp(rMEvt);
    }
    else
    {
        /**********************************************************************
        * BEZIER_EDITOR
        **********************************************************************/
        if ( pView->IsAction() )
        {
            if ( pView->IsInsObjPoint() )
            {
                pView->EndInsObjPoint(SDRCREATE_FORCEEND);
            }
            else if ( pView->IsDragObj() )
            {
                FrameView* pFrameView = pViewShell->GetFrameView();
                BOOL bDragWithCopy = (rMEvt.IsMod1() && pFrameView->IsDragWithCopy());

                if (bDragWithCopy)
                {
                    bDragWithCopy = !pView->IsPresObjSelected(FALSE, TRUE);
                }

                pView->SetDragWithCopy(bDragWithCopy);
                pView->EndDragObj( pView->IsDragWithCopy() );
            }
            else
            {
                pView->EndAction();

                USHORT nDrgLog = USHORT ( pWindow->PixelToLogic(Size(DRGPIX,0)).Width() );
                Point aPos = pWindow->PixelToLogic( rMEvt.GetPosPixel() );

                if (Abs(aMDPos.X() - aPos.X()) < nDrgLog &&
                    Abs(aMDPos.Y() - aPos.Y()) < nDrgLog &&
                    !rMEvt.IsShift() && !rMEvt.IsMod2())
                {
                    SdrViewEvent aVEvt;
                    SdrHitKind eHit = pView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

                    if (eHit == SDRHIT_NONE)
                    {
                        // Klick auf der Stelle: deselektieren
                        pView->UnmarkAllObj();
                    }
                }
            }
        }
        else if (!rMEvt.IsShift() && rMEvt.IsMod1() && !rMEvt.IsMod2() &&
                 Abs(aPnt.X() - aMDPos.X()) < nDrgLog &&
                 Abs(aPnt.Y() - aMDPos.Y()) < nDrgLog)
        {
            // Gruppe betreten
            pView->MarkObj(aPnt, nHitLog, FALSE, rMEvt.IsMod1());
        }


        ForcePointer(&rMEvt);
        pHdl = NULL;
        pWindow->ReleaseMouse();

        FuDraw::MouseButtonUp(rMEvt);
    }

    return (bReturn);
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert TRUE, andernfalls
|* FALSE.
|*
\************************************************************************/

BOOL FuSelection::KeyInput(const KeyEvent& rKEvt)
{
    BOOL bReturn = FALSE;
    BOOL bSwitchToSelection = FALSE;

    switch (rKEvt.GetKeyCode().GetCode())
    {
        case KEY_ESCAPE:
        {
            if (pView->IsCreationActive())
            {
                bSwitchToSelection = TRUE;
                bReturn = TRUE;
            }
        }
        break;
    }

    if (!bReturn)
    {
        bReturn = FuDraw::KeyInput(rKEvt);
    }

    if (bSwitchToSelection || pView->GetMarkList().GetMarkCount() == 0)
    {
        pView->ShowMirrored();
        pView->ResetCreationActive();

        pViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
    }

    return(bReturn);

}


/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuSelection::Activate()
{
    SdrDragMode eMode;
    pView->ResetCreationActive();
    pView->SetEditMode(SDREDITMODE_EDIT);

    switch( nSlotId )
    {
        case SID_OBJECT_ROTATE:
        {
            // (gemapter) Slot wird explizit auf Rotate gesetzt #31052#
            SlotArray[ 1 ] = SID_OBJECT_ROTATE;

            eMode = SDRDRAG_ROTATE;

            if ( pView->GetDragMode() != eMode )
                pView->SetDragMode(eMode);
        }
        break;

        case SID_OBJECT_MIRROR:
        {
            eMode = SDRDRAG_MIRROR;

            if ( pView->GetDragMode() != eMode )
                pView->SetDragMode(eMode);
        }
        break;

        case SID_OBJECT_TRANSPARENCE:
        {
            eMode = SDRDRAG_TRANSPARENCE;

            if ( pView->GetDragMode() != eMode )
                pView->SetDragMode(eMode);
        }
        break;

        case SID_OBJECT_GRADIENT:
        {
            eMode = SDRDRAG_GRADIENT;

            if ( pView->GetDragMode() != eMode )
                pView->SetDragMode(eMode);
        }
        break;

        case SID_OBJECT_SHEAR:
        {
            eMode = SDRDRAG_SHEAR;

            if ( pView->GetDragMode() != eMode )
                pView->SetDragMode(eMode);
        }
        break;

        case SID_OBJECT_CROOK_ROTATE:
        {
            eMode = SDRDRAG_CROOK;

            if ( pView->GetDragMode() != eMode )
            {
                pView->SetDragMode(eMode);
                pView->SetCrookMode(SDRCROOK_ROTATE);
            }
        }
        break;

        case SID_OBJECT_CROOK_SLANT:
        {
            eMode = SDRDRAG_CROOK;

            if ( pView->GetDragMode() != eMode )
            {
                pView->SetDragMode(eMode);
                pView->SetCrookMode(SDRCROOK_SLANT);
            }
        }
        break;

        case SID_OBJECT_CROOK_STRETCH:
        {
            eMode = SDRDRAG_CROOK;

            if ( pView->GetDragMode() != eMode )
            {
                pView->SetDragMode(eMode);
                pView->SetCrookMode(SDRCROOK_STRETCH);
            }
        }
        break;

        case SID_CONVERT_TO_3D_LATHE:
        {
            eMode = SDRDRAG_MIRROR;
            bSuppressChangesOfSelection = TRUE;

            if ( pView->GetDragMode() != eMode )
                pView->SetDragMode(eMode);

            if (!pView->IsCreationActive())
                pView->Start3DCreation();

            bSuppressChangesOfSelection = FALSE;
        }
        break;

        default:
        {
            eMode = SDRDRAG_MOVE;

            if ( pView->GetDragMode() != eMode )
                pView->SetDragMode(eMode);
        }
        break;
    }

    if (nSlotId != SID_OBJECT_ROTATE)
    {
        bTempRotation = FALSE;
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
    bSelectionChanged = TRUE;

    FuDraw::SelectionHasChanged();

    if ((pView->IsCreationActive() && !bSuppressChangesOfSelection))
    {
        // Wechsel Rotationskoerper -> Selektion
        pView->ShowMirrored();
        pView->ResetCreationActive();
        nSlotId = SID_OBJECT_SELECT;
        Activate();
    }

    /**************************************************************************
    * ObjectBar einschalten
    **************************************************************************/
    USHORT nObjBarId;

    if (pView->GetContext() == SDRCONTEXT_POINTEDIT)
        nObjBarId = RID_BEZIER_TOOLBOX;
    else if (pView->GetContext() == SDRCONTEXT_GRAPHIC)
        nObjBarId = RID_DRAW_GRAF_TOOLBOX;
    else
        nObjBarId = RID_DRAW_OBJ_TOOLBOX;

    pViewShell->SwitchObjectBar(nObjBarId);
}


/*************************************************************************
|*
|* Aktuellen Bezier-Editmodus setzen
|*
\************************************************************************/

void FuSelection::SetEditMode(USHORT nMode)
{
    nEditMode = nMode;

    if (nEditMode == SID_BEZIER_INSERT)
    {
        pView->SetInsObjPointMode(TRUE);
    }
    else
    {
        pView->SetInsObjPointMode(FALSE);
    }

    ForcePointer();

    SfxBindings& rBindings = pViewShell->GetViewFrame()->GetBindings();
    rBindings.Invalidate(SID_BEZIER_MOVE);
    rBindings.Invalidate(SID_BEZIER_INSERT);
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

IMPL_LINK( FuSelection, SoundHasStoppedHdl, void*, p )
{
    pSound->SetNotifyHdl( Link() );
    pSound->SetSoundName( String() );
    return 0L;
}


/*************************************************************************
|*
|* Animation oder Interaktion ausfuehren
|*
\************************************************************************/

BOOL FuSelection::AnimateObj(SdrObject* pObj, const Point& rPos)
{
    BOOL bAnimated = FALSE;
    BOOL bClosed = pObj->IsClosedObj();
    BOOL bFilled = FALSE;

    if (bClosed)
    {
        SfxItemSet aSet(pDoc->GetPool());

//-/        pObj->TakeAttributes(aSet, TRUE, FALSE);
        aSet.Put(pObj->GetItemSet());

        const XFillStyleItem& rFillStyle = (const XFillStyleItem&) aSet.Get(XATTR_FILLSTYLE);
        bFilled = rFillStyle.GetValue() != XFILL_NONE;
    }

    const SetOfByte* pVisiLayer = &pView->GetPageViewPvNum(0)->GetVisibleLayers();
    USHORT nHitLog = USHORT ( pWindow->PixelToLogic(Size(HITPIX,0)).Width() );
    const long  n2HitLog = nHitLog * 2;
    Point aHitPosR(rPos);
    Point aHitPosL(rPos);
    Point aHitPosT(rPos);
    Point aHitPosB(rPos);

    aHitPosR.X() += n2HitLog;
    aHitPosL.X() -= n2HitLog;
    aHitPosT.Y() += n2HitLog;
    aHitPosB.Y() -= n2HitLog;

    if ( !bClosed                                      ||
         !bFilled                                      ||
         (pObj->IsHit( aHitPosR, nHitLog, pVisiLayer ) &&
          pObj->IsHit( aHitPosL, nHitLog, pVisiLayer ) &&
          pObj->IsHit( aHitPosT, nHitLog, pVisiLayer ) &&
          pObj->IsHit( aHitPosB, nHitLog, pVisiLayer ) ) )
    {
        if ( pDoc->GetIMapInfo( pObj ) )
        {
            const IMapObject* pIMapObj = pDoc->GetHitIMapObject( pObj, rPos, *pWindow );

            if ( pIMapObj && pIMapObj->GetURL().Len() )
            {
                // Sprung zu Dokument
                pWindow->ReleaseMouse();
                SfxStringItem aStrItem(SID_FILE_NAME, pIMapObj->GetURL());
                SfxStringItem aReferer(SID_REFERER, pDocSh->GetMedium()->GetName());
                SfxViewFrame* pFrame = pViewShell->GetViewFrame();
                SfxFrameItem aFrameItem(SID_DOCFRAME, pFrame);
                SfxBoolItem aBrowseItem( SID_BROWSING, TRUE );
                pWindow->ReleaseMouse();
                pFrame->GetDispatcher()->
                    Execute(SID_OPENDOC, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                            &aStrItem, &aFrameItem, &aBrowseItem, &aReferer, 0L);

                bAnimated = TRUE;
            }
        }
        else if (!pDocSh->ISA(SdGraphicDocShell)        &&
                 pView->ISA(SdDrawView)                 &&
                 ((SdDrawView*) pView)->IsActionMode()  &&
                 pDoc->GetAnimationInfo(pObj))
        {
            /**********************************************************
            * Animations-Objekt in der Mitte getroffen -> Interaktion
            **********************************************************/
            SdAnimationInfo* pInfo = pDoc->GetAnimationInfo(pObj);
            SdDrawViewShell* pDrViewSh = (SdDrawViewShell*) pViewShell;
            pWindow->ReleaseMouse();

            switch (pInfo->eClickAction)
            {
                case presentation::ClickAction_BOOKMARK:
                {
                     // Sprung zu Bookmark (Seite oder Objekt)
                    SfxStringItem aItem(SID_NAVIGATOR_OBJECT, pInfo->aBookmark);
                    pViewShell->GetViewFrame()->GetDispatcher()->
                    Execute(SID_NAVIGATOR_OBJECT, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD, &aItem, 0L);
                    bAnimated = TRUE;
                }
                break;

                case presentation::ClickAction_DOCUMENT:
                {
                    // Sprung zu Dokument
                    if (pInfo->aBookmark.Len())
                    {
                        SfxStringItem aReferer(SID_REFERER, pDocSh->GetMedium()->GetName());
                        SfxStringItem aStrItem(SID_FILE_NAME, pInfo->aBookmark);
                        SfxViewFrame* pFrame = pViewShell->GetViewFrame();
                        SfxFrameItem aFrameItem(SID_DOCFRAME, pFrame);
                        SfxBoolItem aBrowseItem( SID_BROWSING, TRUE );
                        pFrame->GetDispatcher()->
                        Execute(SID_OPENDOC, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                                &aStrItem, &aFrameItem, &aBrowseItem, &aReferer, 0L);
                    }

                    bAnimated = TRUE;
                }
                break;

                case presentation::ClickAction_PREVPAGE:
                {
                    // Sprung zur vorherigen Seite
                    SfxUInt16Item aItem(SID_NAVIGATOR_PAGE, PAGE_PREVIOUS);
                    pViewShell->GetViewFrame()->GetDispatcher()->
                    Execute(SID_NAVIGATOR_PAGE, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD,
                            &aItem, 0L);
                    bAnimated = TRUE;
                }
                break;

                case presentation::ClickAction_NEXTPAGE:
                {
                    // Sprung zur naechsten Seite
                    SfxUInt16Item aItem(SID_NAVIGATOR_PAGE, PAGE_NEXT);
                    pViewShell->GetViewFrame()->GetDispatcher()->
                    Execute(SID_NAVIGATOR_PAGE, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD,
                            &aItem, 0L);
                    bAnimated = TRUE;
                }
                break;

                case presentation::ClickAction_FIRSTPAGE:
                {
                    // Sprung zu erster Seite
                    SfxUInt16Item aItem(SID_NAVIGATOR_PAGE, PAGE_FIRST);
                    pViewShell->GetViewFrame()->GetDispatcher()->
                    Execute(SID_NAVIGATOR_PAGE, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD,
                            &aItem, 0L);
                    bAnimated = TRUE;
                }
                break;

                case presentation::ClickAction_LASTPAGE:
                {
                    // Sprung zu letzter Seite
                    SfxUInt16Item aItem(SID_NAVIGATOR_PAGE, PAGE_LAST);
                    pViewShell->GetViewFrame()->GetDispatcher()->
                    Execute(SID_NAVIGATOR_PAGE, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD,
                            &aItem, 0L);
                    bAnimated = TRUE;
                }
                break;

                case presentation::ClickAction_SOUND:
                {
                    // Sound asynchron abspielen
                    if( !pSound )
                    {
                        pSound = new Sound();
                    }

                    pSound->SetNotifyHdl( LINK( this, FuSelection, SoundHasStoppedHdl ) );
                    pSound->SetSoundName( pInfo->aBookmark );
                    pSound->Play();
                    bAnimated = TRUE;
                }
                break;

                case presentation::ClickAction_VERB:
                {
                    // Verb zuweisen
                    pView->UnmarkAll();
                    pView->MarkObj(pObj, pView->GetPageViewPvNum(0), FALSE, FALSE);
                    pDrViewSh->DoVerb(pInfo->nVerb);
                    bAnimated = TRUE;
                }
                break;

                case presentation::ClickAction_PROGRAM:
                {
                    // Execute application
                    String aStrApplication = pInfo->aBookmark;
                    INetURLObject aURLObj( ::URIHelper::SmartRelToAbs( aStrApplication, FALSE,
                                                                       INetURLObject::WAS_ENCODED,
                                                                       INetURLObject::DECODE_UNAMBIGUOUS ) );
                    aStrApplication = aURLObj.PathToFileName();

                    NAMESPACE_VOS(OProcess) aApp( aStrApplication);
                    NAMESPACE_VOS(OArgumentList) aParameters;

                    aApp.execute( (NAMESPACE_VOS(OProcess)::TProcessOption)
                                  (NAMESPACE_VOS(OProcess)::TOption_SearchPath |
                                  NAMESPACE_VOS(OProcess)::TOption_Detached),
                                  aParameters );

                }
                break;

                case presentation::ClickAction_MACRO:
                {
                    // Execute makro
                    String aMacro = pInfo->aBookmark;

                    // aMacro has got following format:
                    // "Macroname.Modulname.Libname.Documentname" or
                    // "Macroname.Modulname.Libname.Applicationsname"
                    String aMacroName = aMacro.GetToken(0, sal_Unicode('.'));
                    String aModulName = aMacro.GetToken(1, sal_Unicode('.'));
                    String aLibName   = aMacro.GetToken(2, sal_Unicode('.'));
                    String aDocName   = aMacro.GetToken(3, sal_Unicode('.'));

                    // In this moment the Call-method only
                    // resolves modulename+macroname
                    String aExecMacro(aModulName);
                    aExecMacro.Append( sal_Unicode('.') );
                    aExecMacro.Append( aMacroName );
                    bAnimated = pDocSh->GetBasic()->Call(aExecMacro);
                }
                break;

                default:
                {
                    bAnimated = FALSE;
                }
                break;
            }
        }

        if (!bAnimated                               &&
            pView->ISA(SdDrawView)                 &&
            !pDocSh->ISA(SdGraphicDocShell)        &&
            ((SdDrawView*) pView)->GetSlideShow()  &&
            pDoc->GetAnimationInfo(pObj))
        {
            /**********************************************************
            * Effekt-Objekt in der Mitte getroffen -> Effekt abspielen
            **********************************************************/
            SdAnimationInfo* pInfo = pDoc->GetAnimationInfo(pObj);
            SdDrawViewShell* pDrViewSh = (SdDrawViewShell*) pViewShell;

            switch (pInfo->eClickAction)
            {
                case presentation::ClickAction_VANISH:
                case presentation::ClickAction_INVISIBLE:
                {
                    if (((SdDrawView*) pView)->GetSlideShow())
                    {
                        // Objekt ausblenden
                        bHideAndAnimate = TRUE;
                        pDrViewSh->LockInput();
                        ((SdDrawView*) pView)->HideAndAnimateObject(pObj);
                        pDrViewSh->UnlockInput();
                        pWindow->ReleaseMouse();
                        bAnimated = TRUE;
                    }
                }
                break;

                default:
                {
                    bAnimated = FALSE;
                }
                break;
            }

            if (!bAnimated && ((SdDrawView*) pView)->GetSlideShow() &&
                ( pInfo->bActive &&
                    ( pInfo->eEffect != presentation::AnimationEffect_NONE ||
                    pInfo->eTextEffect != presentation::AnimationEffect_NONE )))
            {
                // Objekt ausblenden
                bHideAndAnimate = TRUE;
                pDrViewSh->LockInput();
                ((SdDrawView*) pView)->HideAndAnimateObject(pObj);
                pDrViewSh->UnlockInput();
                pWindow->ReleaseMouse();
                bAnimated = TRUE;
            }
        }
    }

    return bAnimated;
}



