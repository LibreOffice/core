/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fudraw.cxx,v $
 *
 *  $Revision: 1.29 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 14:28:38 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include <sot/object.hxx>
#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif

#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif

#define ITEMID_FIELD    EE_FEATURE_FIELD
#include <svx/flditem.hxx>
#ifndef _SVDOGRP_HXX //autogen
#include <svx/svdogrp.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _SV_HELP_HXX //autogen
#include <vcl/help.hxx>
#endif
#ifndef _BMPMASK_HXX_ //autogen
#include <svx/bmpmask.hxx>
#endif
#ifndef _SVDOTEXT_HXX //autogen
#include <svx/svdotext.hxx>
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
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _GOODIES_IMAPOBJ_HXX //autogen
#include <svtools/imapobj.hxx>
#endif
#include <svx/svxids.hrc>
#ifndef _E3D_OBJ3D_HXX //autogen
#include <svx/obj3d.hxx>
#endif
#ifndef _E3D_POLYSC3D_HXX //autogen
#include <svx/polysc3d.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif

#include <sfx2/viewfrm.hxx>

#include "anminfo.hxx"
#include "anmdef.hxx"
#include "imapinfo.hxx"
#include "app.hrc"
#include "glob.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"

#include "app.hxx"
#ifndef SD_GRAPHIC_DOC_SHELL_HXX
#include "GraphicDocShell.hxx"
#endif
#ifndef SD_FU_DRAW_HXX
#include "fudraw.hxx"
#endif
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#ifndef SD_FRAME_VIEW_HXX
#include "FrameView.hxx"
#endif
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#ifndef SD_WINDOW_SHELL_HXX
#include "Window.hxx"
#endif
#include "drawdoc.hxx"
#ifndef SD_DRAW_DOC_SHELL_HXX
#include "DrawDocShell.hxx"
#endif
#ifndef SD_CLIENT_HXX
#include "Client.hxx"
#endif
#include "sdresid.hxx"
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif
#ifndef SD_FU_SELECTION_HXX
#include "fusel.hxx"
#endif
#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

using namespace ::com::sun::star;

namespace sd {

TYPEINIT1( FuDraw, FuPoor );

/*************************************************************************
|*
|* Base-class for all drawmodul-specific functions
|*
\************************************************************************/

FuDraw::FuDraw(ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView,
               SdDrawDocument* pDoc, SfxRequest& rReq) :
    FuPoor(pViewSh, pWin, pView, pDoc, rReq),
    bMBDown(FALSE),
    bPermanent(FALSE),
    bDragHelpLine(FALSE)
{
}

FunctionReference FuDraw::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq, bool bPermanent )
{
    FuDraw* pFunc;
    FunctionReference xFunc( pFunc = new FuDraw( pViewSh, pWin, pView, pDoc, rReq ) );
    pFunc->SetPermanent(bPermanent);
    return xFunc;
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuDraw::~FuDraw()
{
    pView->BrkAction();
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

BOOL FuDraw::MouseButtonDown(const MouseEvent& rMEvt)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    BOOL bReturn = FALSE;

    bDragHelpLine = FALSE;

    aMDPos = pWindow->PixelToLogic( rMEvt.GetPosPixel() );

    if ( rMEvt.IsLeft() )
    {
        FrameView* pFrameView = pViewShell->GetFrameView();

//        BOOL bOrtho = pView->IsOrthoDesired() || pFrameView->IsOrtho();
//        bOrtho = bOrtho != rMEvt.IsShift();
        BOOL bOrtho = FALSE;

        BOOL bRestricted = TRUE;

        if (pView->IsDragObj())
        {
            // object is dragged (move, resize,...)
            const SdrHdl* pHdl = pView->GetDragStat().GetHdl();

            if (!pHdl || (!pHdl->IsCornerHdl() && !pHdl->IsVertexHdl()))
            {
                // Move
                bRestricted = FALSE;
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
            bOrtho = rMEvt.IsShift() != pFrameView->IsOrtho();
        }

        if (!pView->IsSnapEnabled())
            pView->SetSnapEnabled(TRUE);
        BOOL bCntrl = rMEvt.IsMod1();

        BOOL bGridSnap = pFrameView->IsGridSnap();
        bGridSnap = (bCntrl != bGridSnap);

        if (pView->IsGridSnap() != bGridSnap)
            pView->SetGridSnap(bGridSnap);

        BOOL bBordSnap = pFrameView->IsBordSnap();
        bBordSnap = (bCntrl != bBordSnap);

        if (pView->IsBordSnap() != bBordSnap)
            pView->SetBordSnap(bBordSnap);

        BOOL bHlplSnap = pFrameView->IsHlplSnap();
        bHlplSnap = (bCntrl != bHlplSnap);

        if (pView->IsHlplSnap() != bHlplSnap)
            pView->SetHlplSnap(bHlplSnap);

        BOOL bOFrmSnap = pFrameView->IsOFrmSnap();
        bOFrmSnap = (bCntrl != bOFrmSnap);

        if (pView->IsOFrmSnap() != bOFrmSnap)
            pView->SetOFrmSnap(bOFrmSnap);

        BOOL bOPntSnap = pFrameView->IsOPntSnap();
        bOPntSnap = (bCntrl != bOPntSnap);

        if (pView->IsOPntSnap() != bOPntSnap)
            pView->SetOPntSnap(bOPntSnap);

        BOOL bOConSnap = pFrameView->IsOConSnap();
        bOConSnap = (bCntrl != bOConSnap);

        if (pView->IsOConSnap() != bOConSnap)
            pView->SetOConSnap(bOConSnap);

        BOOL bAngleSnap = rMEvt.IsShift() == !pFrameView->IsAngleSnapEnabled();

        if (pView->IsAngleSnapEnabled() != bAngleSnap)
            pView->SetAngleSnapEnabled(bAngleSnap);

        if (pView->IsOrtho() != bOrtho)
            pView->SetOrtho(bOrtho);

        BOOL bCenter = rMEvt.IsMod2();

        if ( pView->IsCreate1stPointAsCenter() != bCenter ||
             pView->IsResizeAtCenter() != bCenter )
        {
            pView->SetCreate1stPointAsCenter(bCenter);
            pView->SetResizeAtCenter(bCenter);
        }

        SdrPageView* pPV;
        USHORT nHitLog = USHORT ( pWindow->PixelToLogic(Size(HITPIX,0)).Width() );

        // #76572# look only for HelpLines when they are visible (!)
        BOOL bHelpLine(FALSE);
        if(pView->IsHlplVisible())
            bHelpLine = pView->PickHelpLine(aMDPos, nHitLog, *pWindow, nHelpLine, pPV);
        BOOL bHitHdl = (pView->PickHandle(aMDPos) != NULL);

        if ( bHelpLine
            && !pView->IsCreateObj()
            && ((pView->GetEditMode() == SDREDITMODE_EDIT && !bHitHdl) || (rMEvt.IsShift() && bCntrl)) )
        {
            pWindow->CaptureMouse();
            pView->BegDragHelpLine(nHelpLine, pPV);
            bDragHelpLine = pView->IsDragHelpLine();
            bReturn = TRUE;
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

BOOL FuDraw::MouseMove(const MouseEvent& rMEvt)
{
    FrameView* pFrameView = pViewShell->GetFrameView();
    Point aPos = pWindow->PixelToLogic( rMEvt.GetPosPixel() );

//    BOOL bOrtho = pView->IsOrthoDesired() || pFrameView->IsOrtho();
//    bOrtho = bOrtho != rMEvt.IsShift();
    BOOL bOrtho = FALSE;

    BOOL bRestricted = TRUE;

    if (pView->IsDragObj())
    {
        // object is dragged (move, resize, ...)
        const SdrHdl* pHdl = pView->GetDragStat().GetHdl();

        if (!pHdl || (!pHdl->IsCornerHdl() && !pHdl->IsVertexHdl()))
        {
            // Move
            bRestricted = FALSE;
        }
    }

    if (pView->IsAction())
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
            bOrtho = rMEvt.IsShift() != pFrameView->IsOrtho();
        }

        BOOL bCntrl = rMEvt.IsMod1();
        pView->SetDragWithCopy(rMEvt.IsMod1() && pFrameView->IsDragWithCopy());

        BOOL bGridSnap = pFrameView->IsGridSnap();
        bGridSnap = (bCntrl != bGridSnap);

        if (pView->IsGridSnap() != bGridSnap)
            pView->SetGridSnap(bGridSnap);

        BOOL bBordSnap = pFrameView->IsBordSnap();
        bBordSnap = (bCntrl != bBordSnap);

        if (pView->IsBordSnap() != bBordSnap)
            pView->SetBordSnap(bBordSnap);

        BOOL bHlplSnap = pFrameView->IsHlplSnap();
        bHlplSnap = (bCntrl != bHlplSnap);

        if (pView->IsHlplSnap() != bHlplSnap)
            pView->SetHlplSnap(bHlplSnap);

        BOOL bOFrmSnap = pFrameView->IsOFrmSnap();
        bOFrmSnap = (bCntrl != bOFrmSnap);

        if (pView->IsOFrmSnap() != bOFrmSnap)
            pView->SetOFrmSnap(bOFrmSnap);

        BOOL bOPntSnap = pFrameView->IsOPntSnap();
        bOPntSnap = (bCntrl != bOPntSnap);

        if (pView->IsOPntSnap() != bOPntSnap)
            pView->SetOPntSnap(bOPntSnap);

        BOOL bOConSnap = pFrameView->IsOConSnap();
        bOConSnap = (bCntrl != bOConSnap);

        if (pView->IsOConSnap() != bOConSnap)
            pView->SetOConSnap(bOConSnap);

        BOOL bAngleSnap = rMEvt.IsShift() == !pFrameView->IsAngleSnapEnabled();

        if (pView->IsAngleSnapEnabled() != bAngleSnap)
            pView->SetAngleSnapEnabled(bAngleSnap);

        if (pView->IsOrtho() != bOrtho)
            pView->SetOrtho(bOrtho);

        BOOL bCenter = rMEvt.IsMod2();

        if ( pView->IsCreate1stPointAsCenter() != bCenter ||
             pView->IsResizeAtCenter() != bCenter )
        {
            pView->SetCreate1stPointAsCenter(bCenter);
            pView->SetResizeAtCenter(bCenter);
        }

        if ( pView->IsDragHelpLine() )
            pView->MovDragHelpLine(aPos);
    }

    BOOL bReturn = pView->MouseMove(rMEvt, pWindow);

    if (pView->IsAction())
    {
        // Because the flag set back if necessary in MouseMove
        if (pView->IsOrtho() != bOrtho)
            pView->SetOrtho(bOrtho);
    }

    ForcePointer(&rMEvt);

    return bReturn;
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

BOOL FuDraw::MouseButtonUp(const MouseEvent& rMEvt)
{
    if ( pView->IsDragHelpLine() )
        pView->EndDragHelpLine();

    if ( bDragHelpLine )
    {
        Rectangle aOutputArea(Point(0,0), pWindow->GetOutputSizePixel());

        if ( !aOutputArea.IsInside(rMEvt.GetPosPixel()) )
            pView->GetSdrPageView()->DeleteHelpLine(nHelpLine);

        pWindow->ReleaseMouse();
    }

    FrameView* pFrameView = pViewShell->GetFrameView();
    pView->SetOrtho( pFrameView->IsOrtho() );
    pView->SetAngleSnapEnabled( pFrameView->IsAngleSnapEnabled() );
    pView->SetSnapEnabled(TRUE);
    pView->SetCreate1stPointAsCenter(FALSE);
    pView->SetResizeAtCenter(FALSE);
    pView->SetDragWithCopy(pFrameView->IsDragWithCopy());
    pView->SetGridSnap(pFrameView->IsGridSnap());
    pView->SetBordSnap(pFrameView->IsBordSnap());
    pView->SetHlplSnap(pFrameView->IsHlplSnap());
    pView->SetOFrmSnap(pFrameView->IsOFrmSnap());
    pView->SetOPntSnap(pFrameView->IsOPntSnap());
    pView->SetOConSnap(pFrameView->IsOConSnap());

    bIsInDragMode = FALSE;
    ForcePointer(&rMEvt);
    FuPoor::MouseButtonUp(rMEvt);

    return FALSE;
}

/*************************************************************************
|*
|* Process keyboard-events
|*
|* When processing a KeyEvent the returnvalue is TRUE, otherwise FALSE.
|*
\************************************************************************/

BOOL FuDraw::KeyInput(const KeyEvent& rKEvt)
{
    BOOL bReturn = FALSE;

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
            if (!pDocSh->IsReadOnly())
            {
                if ( pView && pView->IsPresObjSelected(FALSE, TRUE, FALSE, TRUE) )
                {
                    InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
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
                    OSL_ASSERT (pViewShell->GetViewShell()!=NULL);
                    Client* pIPClient = static_cast<Client*>(
                        pViewShell->GetViewShell()->GetIPClient());
                    if (pIPClient && pIPClient->IsObjectInPlaceActive())
                        pIPClient->SetSdrGrafObj(NULL);

                    // wait-mousepointer while deleting object
                    WaitObject aWait( (Window*)pViewShell->GetActiveWindow() );
                    // delete object
                    pView->DeleteMarked();
                }
            }
            bReturn = TRUE;
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
                pViewShell->GetViewFrame()->GetDispatcher()->Execute(
                    SID_OBJECT_SELECT,
                    SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);

                // changeover to the next object
                if(!pView->MarkNextObj( !aCode.IsShift() ))
                {
                    // #97016# No next object: go over open end and
                    // get first from the other side
                    pView->UnmarkAllObj();
                    pView->MarkNextObj(!aCode.IsShift());
                }

                // #97016# II
                if(pView->AreObjectsMarked())
                    pView->MakeVisible(pView->GetAllMarkedRect(), *pWindow);

                bReturn = TRUE;
            }
        }
        break;

        case KEY_END:
        {
            KeyCode aCode = rKEvt.GetKeyCode();

            if ( aCode.IsMod1() )
            {
                // #97016# mark last object
                pView->UnmarkAllObj();
                pView->MarkNextObj(FALSE);

                // #97016# II
                if(pView->AreObjectsMarked())
                    pView->MakeVisible(pView->GetAllMarkedRect(), *pWindow);

                bReturn = TRUE;
            }
        }
        break;

        case KEY_HOME:
        {
            KeyCode aCode = rKEvt.GetKeyCode();

            if ( aCode.IsMod1() )
            {
                // #97016# mark first object
                pView->UnmarkAllObj();
                pView->MarkNextObj(TRUE);

                // #97016# II
                if(pView->AreObjectsMarked())
                    pView->MakeVisible(pView->GetAllMarkedRect(), *pWindow);

                bReturn = TRUE;
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
        pWindow->ReleaseMouse();
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
    //if ( pView->IsShownXorVisible(pWindow) )
    //{
    //  pView->HideShownXor(pWindow);
    //}
}

/*************************************************************************
|*
|* After scrolling show the selection-presentation again
|*
\************************************************************************/

void FuDraw::ScrollEnd()
{
    //if ( !pView->IsShownXorVisible(pWindow) )
    //{
    //  pView->ShowShownXor(pWindow);
    //}
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
    Point aPnt;
    USHORT nModifier = 0;
    BOOL bLeftDown = FALSE;
    BOOL bDefPointer = TRUE;

    if (pMEvt)
    {
        aPnt = pWindow->PixelToLogic(pMEvt->GetPosPixel());
        nModifier = pMEvt->GetModifier();
        bLeftDown = pMEvt->IsLeft();
    }
    else
    {
        aPnt = pWindow->PixelToLogic(pWindow->GetPointerPosPixel());
    }

    BOOL bIsAction = pView->IsAction();

    if (pView->IsDragObj())
    {
        if (SD_MOD()->GetWaterCan() && !pView->PickHandle(aPnt))
        {
            /******************************************************************
            * Giesskannenmodus
            ******************************************************************/
            bDefPointer = FALSE;
            pWindow->SetPointer(Pointer(POINTER_FILL));
        }
    }
    else
    {
        SdrHdl* pHdl = pView->PickHandle(aPnt);

        if (SD_MOD()->GetWaterCan() && !pHdl)
        {
            /******************************************************************
            * Giesskannenmodus
            ******************************************************************/
            bDefPointer = FALSE;
            pWindow->SetPointer(Pointer(POINTER_FILL));
        }
        else if (!pHdl &&
                 pViewShell->GetViewFrame()->HasChildWindow(SvxBmpMaskChildWindow::GetChildWindowId()))
        {
            /******************************************************************
            * Pipettenmodus
            ******************************************************************/
            SvxBmpMask* pMask = (SvxBmpMask*) pViewShell->GetViewFrame()->GetChildWindow(SvxBmpMaskChildWindow::GetChildWindowId())->GetWindow();

            if (pMask && pMask->IsEyedropping())
            {
                bDefPointer = FALSE;
                pWindow->SetPointer(Pointer(POINTER_REFHAND));
            }
        }
        else if (!pView->IsAction())
        {
            SdrObject* pObj = NULL;
            SdrPageView* pPV = NULL;
            SdrViewEvent aVEvt;
            SdrHitKind eHit = SDRHIT_NONE;
            SdrDragMode eDragMode = pView->GetDragMode();
            USHORT nHitLog(USHORT (pWindow->PixelToLogic(Size(HITPIX,0)).Width()));

            if (pMEvt)
            {
                eHit = pView->PickAnything(*pMEvt, SDRMOUSEMOVE, aVEvt);
            }

            if ((eDragMode == SDRDRAG_ROTATE) && (eHit == SDRHIT_MARKEDOBJECT))
            {
                // The goal of this request is show always the rotation-arrow for 3D-objects at rotation-modus
                // Independent of the settings at Extras->Optionen->Grafik "Objekte immer verschieben"
                // 2D-objects acquit in an other way. Otherwise, the rotation of 3d-objects around any axises
                // wouldn't be possible per default.
                const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
                SdrObject* pObject = rMarkList.GetMark(0)->GetMarkedSdrObj();
                if ((pObject->ISA(E3dObject)) && (rMarkList.GetMarkCount() == 1))
                {
                    pWindow->SetPointer(Pointer(POINTER_ROTATE));
                    bDefPointer = FALSE;     // Otherwise it'll be calles Joes routine and the mousepointer will reconfigurate again
                }
            }

            if (eHit == SDRHIT_NONE)
            {
                // found nothing -> look after at the masterpage
                pView->PickObj(aPnt, pObj, pPV, SDRSEARCH_ALSOONMASTER);
            }
            else if (eHit == SDRHIT_UNMARKEDOBJECT)
            {
                pObj = aVEvt.pObj;
            }
            else if (eHit == SDRHIT_TEXTEDITOBJ && this->ISA(FuSelection))
            {
                UINT16 nSdrObjKind = aVEvt.pObj->GetObjIdentifier();

                if ( nSdrObjKind != OBJ_TEXT        &&
                     nSdrObjKind != OBJ_TITLETEXT   &&
                     nSdrObjKind != OBJ_OUTLINETEXT &&
                     aVEvt.pObj->IsEmptyPresObj() )
                {
                    pObj = NULL;
                    bDefPointer = FALSE;
                    pWindow->SetPointer(Pointer(POINTER_ARROW));
                }
            }

            if (pObj && pMEvt && !pMEvt->IsMod2() && this->ISA(FuSelection))
            {
                // Auf Animation oder ImageMap pruefen
                bDefPointer = !SetPointer(pObj, aPnt);

                if (bDefPointer && (pObj->ISA(SdrObjGroup) || pObj->ISA(E3dPolyScene)))
                {
                    // In die Gruppe hineinschauen
                    if (pView->PickObj(aPnt, pObj, pPV, SDRSEARCH_ALSOONMASTER | SDRSEARCH_DEEP))
                        bDefPointer = !SetPointer(pObj, aPnt);
                }
            }
        }
    }

    if (bDefPointer)
    {
        pWindow->SetPointer(pView->GetPreferedPointer(
                            aPnt, pWindow, nModifier, bLeftDown));
    }
}

/*************************************************************************
|*
|* Set cursor for animaton or imagemap
|*
\************************************************************************/

BOOL FuDraw::SetPointer(SdrObject* pObj, const Point& rPos)
{
    BOOL bSet = FALSE;

    BOOL bAnimationInfo = (!pDocSh->ISA(GraphicDocShell) &&
                          pDoc->GetAnimationInfo(pObj)) ? TRUE:FALSE;

    BOOL bImageMapInfo = FALSE;

    if (!bAnimationInfo)
        bImageMapInfo = pDoc->GetIMapInfo(pObj) ? TRUE:FALSE;

    if (bAnimationInfo || bImageMapInfo)
    {
        const SetOfByte* pVisiLayer = &pView->GetSdrPageView()->GetVisibleLayers();
        USHORT nHitLog(USHORT (pWindow->PixelToLogic(Size(HITPIX,0)).Width()));
        long  n2HitLog(nHitLog * 2);
        Point aHitPosR(rPos);
        Point aHitPosL(rPos);
        Point aHitPosT(rPos);
        Point aHitPosB(rPos);

        aHitPosR.X() += n2HitLog;
        aHitPosL.X() -= n2HitLog;
        aHitPosT.Y() += n2HitLog;
        aHitPosB.Y() -= n2HitLog;

        if ( !pObj->IsClosedObj() ||
            ( pObj->IsHit(aHitPosR, nHitLog, pVisiLayer) &&
              pObj->IsHit(aHitPosL, nHitLog, pVisiLayer) &&
              pObj->IsHit(aHitPosT, nHitLog, pVisiLayer) &&
              pObj->IsHit(aHitPosB, nHitLog, pVisiLayer) ) )
        {
            /**********************************************************
            * hit inside the object (without margin) or open object
            ********************************************************/

            if (bAnimationInfo)
            {
                /******************************************************
                * Click-Action
                ******************************************************/
                SdAnimationInfo* pInfo = pDoc->GetAnimationInfo(pObj);

                if ((pView->ISA(DrawView) &&
                      (pInfo->eClickAction == presentation::ClickAction_BOOKMARK  ||
                       pInfo->eClickAction == presentation::ClickAction_DOCUMENT  ||
                       pInfo->eClickAction == presentation::ClickAction_PREVPAGE  ||
                       pInfo->eClickAction == presentation::ClickAction_NEXTPAGE  ||
                       pInfo->eClickAction == presentation::ClickAction_FIRSTPAGE ||
                       pInfo->eClickAction == presentation::ClickAction_LASTPAGE  ||
                       pInfo->eClickAction == presentation::ClickAction_VERB      ||
                       pInfo->eClickAction == presentation::ClickAction_PROGRAM   ||
                       pInfo->eClickAction == presentation::ClickAction_MACRO     ||
                       pInfo->eClickAction == presentation::ClickAction_SOUND))
                                                                    ||
                      (pView->ISA(DrawView) &&
                       static_cast<DrawView*>(pView)->GetSlideShow()         &&
                         (pInfo->eClickAction == presentation::ClickAction_VANISH             ||
                          pInfo->eClickAction == presentation::ClickAction_INVISIBLE          ||
                          pInfo->eClickAction == presentation::ClickAction_STOPPRESENTATION ||
                         (pInfo->bActive &&
                          ( pInfo->eEffect != presentation::AnimationEffect_NONE ||
                            pInfo->eTextEffect != presentation::AnimationEffect_NONE )))))
                    {
                        // Animations-Objekt
                        bSet = TRUE;
                        pWindow->SetPointer(Pointer(POINTER_REFHAND));
                    }
            }
            else if (bImageMapInfo &&
                     pDoc->GetHitIMapObject(pObj, rPos, *pWindow))
            {
                /******************************************************
                * ImageMap
                ******************************************************/
                bSet = TRUE;
                pWindow->SetPointer(Pointer(POINTER_REFHAND));
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
    SdrObject* pObj = NULL;
    SdrPageView* pPV = NULL;
    USHORT nHitLog = USHORT ( pWindow->PixelToLogic(Size(HITPIX,0)).Width() );

    if ( pView->AreObjectsMarked() )
    {
        const SdrMarkList& rMarkList = pView->GetMarkedObjectList();

        if (rMarkList.GetMarkCount() == 1)
        {
            SdrMark* pMark = rMarkList.GetMark(0);
            SdrObject* pObj = pMark->GetMarkedSdrObj();

            UINT32 nInv = pObj->GetObjInventor();
            UINT16 nSdrObjKind = pObj->GetObjIdentifier();

            if (nInv == SdrInventor && nSdrObjKind == OBJ_OLE2)
            {
                DrawDocShell* pDocSh = pDoc->GetDocSh();

                if ( !pDocSh->IsUIActive() )
                {
                    /**********************************************************
                    * aktivate OLE-object
                    **********************************************************/
                    pView->HideMarkHdl();
                    pViewShell->ActivateObject( (SdrOle2Obj*) pObj, 0);
                }
            }
            else if (nInv == SdrInventor &&  nSdrObjKind == OBJ_GRAF && pObj->IsEmptyPresObj() )
            {
                pViewShell->GetViewFrame()->
                    GetDispatcher()->Execute( SID_INSERT_GRAPHIC,
                                              SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
            }
            else if ( ( pObj->ISA(SdrTextObj) || pObj->ISA(SdrObjGroup) ) &&
                      !SD_MOD()->GetWaterCan()                            &&
                      pViewShell->GetFrameView()->IsDoubleClickTextEdit() &&
                      !pDocSh->IsReadOnly())
            {
                SfxUInt16Item aItem(SID_TEXTEDIT, 2);
                pViewShell->GetViewFrame()->GetDispatcher()->
                                 Execute(SID_TEXTEDIT, SFX_CALLMODE_ASYNCHRON |
                                         SFX_CALLMODE_RECORD, &aItem, 0L);
            }
            else if (nInv == SdrInventor &&  nSdrObjKind == OBJ_GRUP)
            {
                // hit group -> select subobject
                pView->UnMarkAll();
                pView->MarkObj(aMDPos, nHitLog, rMEvt.IsShift(), TRUE);
            }
        }
    }
    else
        pViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
}

/*************************************************************************
|*
|* Help-event
|*
\************************************************************************/

BOOL FuDraw::RequestHelp(const HelpEvent& rHEvt)
{
    BOOL bReturn = FALSE;

    if (Help::IsBalloonHelpEnabled() || Help::IsQuickHelpEnabled())
    {
        SdrViewEvent aVEvt;

        MouseEvent aMEvt(pWindow->GetPointerPosPixel(), 1, 0, MOUSE_LEFT);

        SdrHitKind eHit = pView->PickAnything(aMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

        SdrObject* pObj = aVEvt.pObj;

        if (eHit != SDRHIT_NONE && pObj != NULL)
        {
            Point aPosPixel = rHEvt.GetMousePosPixel();

            bReturn = SetHelpText(pObj, aPosPixel, aVEvt);

            if (!bReturn && (pObj->ISA(SdrObjGroup) || pObj->ISA(E3dPolyScene)))
            {
                // In die Gruppe hineinschauen
                SdrPageView* pPV = NULL;

                Point aPos(pWindow->PixelToLogic(pWindow->ScreenToOutputPixel(aPosPixel)));

                if (pView->PickObj(aPos, pObj, pPV, SDRSEARCH_ALSOONMASTER | SDRSEARCH_DEEP))
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

BOOL FuDraw::SetHelpText(SdrObject* pObj, const Point& rPosPixel, const SdrViewEvent& rVEvt)
{
    BOOL bSet = FALSE;
    String aHelpText;
    Point aPos(pWindow->PixelToLogic(pWindow->ScreenToOutputPixel(rPosPixel)));

    // URL fuer IMapObject unter Pointer ist Hilfetext
    if ( pDoc->GetIMapInfo(pObj) )
    {
        IMapObject* pIMapObj = pDoc->GetHitIMapObject(pObj, aPos, *pWindow );

        if ( pIMapObj )
        {
            // show name
            aHelpText = pIMapObj->GetDescription();

            if (aHelpText.Len() == 0)
            {
                // show url if no name is available
                aHelpText = INetURLObject::decode( pIMapObj->GetURL(), '%', INetURLObject::DECODE_WITH_CHARSET );
            }
        }
    }
    else if (!pDocSh->ISA(GraphicDocShell) && pDoc->GetAnimationInfo(pObj))
    {
        SdAnimationInfo* pInfo = pDoc->GetAnimationInfo(pObj);

        switch (pInfo->eClickAction)
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
                aHelpText.Append( String(INetURLObject::decode( pInfo->aBookmark, '%', INetURLObject::DECODE_WITH_CHARSET ) ));
            }
            break;

            case presentation::ClickAction_DOCUMENT:
            {
                // jump to document (object/page)
                aHelpText = String(SdResId(STR_CLICK_ACTION_DOCUMENT));
                aHelpText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );
                aHelpText.Append( String(INetURLObject::decode( pInfo->aBookmark, '%', INetURLObject::DECODE_WITH_CHARSET ) ));
            }
            break;

            case presentation::ClickAction_PROGRAM:
            {
                // execute program
                aHelpText = String(SdResId(STR_CLICK_ACTION_PROGRAM));
                aHelpText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );
                aHelpText.Append( String(INetURLObject::decode( pInfo->aBookmark, '%', INetURLObject::DECODE_WITH_CHARSET ) ));
            }
            break;

            case presentation::ClickAction_MACRO:
            {
                // execute program
                aHelpText = String(SdResId(STR_CLICK_ACTION_MACRO));
                aHelpText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );

                if ( SfxApplication::IsXScriptURL( pInfo->aBookmark ) )
                {
                    aHelpText.Append( pInfo->aBookmark );
                }
                else
                {
                    sal_Unicode cToken = '.';
                    aHelpText.Append( pInfo->aBookmark.GetToken( 2, cToken ) );
                    aHelpText.Append( cToken );
                    aHelpText.Append( pInfo->aBookmark.GetToken( 1, cToken ) );
                    aHelpText.Append( cToken );
                    aHelpText.Append(  pInfo->aBookmark.GetToken( 0, cToken ) );
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
        }
    }
    else if (rVEvt.pURLField)
    {
        /**************************************************************
        * URL-Field
        **************************************************************/
        aHelpText = INetURLObject::decode( rVEvt.pURLField->GetURL(), '%', INetURLObject::DECODE_WITH_CHARSET );
    }

    if (aHelpText.Len())
    {
        bSet = TRUE;
        Rectangle aLogicPix = pWindow->LogicToPixel(pObj->GetLogicRect());
        Rectangle aScreenRect(pWindow->OutputToScreenPixel(aLogicPix.TopLeft()),
                              pWindow->OutputToScreenPixel(aLogicPix.BottomRight()));

        if (Help::IsBalloonHelpEnabled())
            Help::ShowBalloon( (Window*)pWindow, rPosPixel, aScreenRect, aHelpText);
        else if (Help::IsQuickHelpEnabled())
            Help::ShowQuickHelp( (Window*)pWindow, aScreenRect, aHelpText);
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

    if ( pView->IsAction() )
    {
        pView->BrkAction();
        bReturn = true;
    }
    else if ( pView->IsTextEdit() )
    {
        pView->SdrEndTextEdit();
        bReturn = true;

        SfxBindings& rBindings = pViewShell->GetViewFrame()->GetBindings();
        rBindings.Invalidate( SID_PARASPACE_INCREASE );
        rBindings.Invalidate( SID_PARASPACE_DECREASE );
    }
    else if ( pView->AreObjectsMarked() )
    {
        // #97016# II
        const SdrHdlList& rHdlList = pView->GetHdlList();
        SdrHdl* pHdl = rHdlList.GetFocusHdl();

        if(pHdl)
        {
            ((SdrHdlList&)rHdlList).ResetFocusHdl();
        }
        else
        {
            pView->UnmarkAll();
        }

        // Switch to FuSelect.
        pViewShell->GetViewFrame()->GetDispatcher()->Execute(
            SID_OBJECT_SELECT,
            SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);

        bReturn = true;
    }

    return bReturn;
}

} // end of namespace sd
