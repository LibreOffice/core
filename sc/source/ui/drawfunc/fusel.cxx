/*************************************************************************
 *
 *  $RCSfile: fusel.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: nn $ $Date: 2001-03-02 21:09:13 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop


// INCLUDE ---------------------------------------------------------------

#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <svx/flditem.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdotext.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/ipfrm.hxx>
#include <so3/ipobj.hxx>
#include <svtools/imapobj.hxx>
#include <svx/svdview.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdpagv.hxx>
#include <svx/outlobj.hxx>



#include "fusel.hxx"
#include "sc.hrc"
#include "fudraw.hxx"
#include "futext.hxx"
#include "tabvwsh.hxx"
#include "drawpage.hxx"
#include "globstr.hrc"
#include "drwlayer.hxx"

// -----------------------------------------------------------------------

//  Maximal erlaubte Mausbewegung um noch Drag&Drop zu starten
//! fusel,fuconstr,futext - zusammenfassen!
#define SC_MAXDRAGMOVE  3

// -----------------------------------------------------------------------

#ifdef WNT
#pragma optimize ( "", off )
#endif


/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuSelection::FuSelection(ScTabViewShell* pViewSh, Window* pWin, SdrView* pView,
               SdrModel* pDoc, SfxRequest& rReq ) :
    FuDraw(pViewSh, pWin, pView, pDoc, rReq),
    bVCAction(FALSE)
{
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuSelection::~FuSelection()
{
}

BYTE FuSelection::Command(const CommandEvent& rCEvt)
{
    //  special code for non-VCL OS2/UNX removed

    return FuDraw::Command( rCEvt );
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

BOOL __EXPORT FuSelection::MouseButtonDown(const MouseEvent& rMEvt)
{
    if ( pView->IsAction() )
    {
        if ( rMEvt.IsRight() )
            pView->BckAction();
        return TRUE;
    }

    bVCAction = FALSE;
    bIsInDragMode = FALSE;      //  irgendwo muss es ja zurueckgesetzt werden (#50033#)

    BOOL bReturn = FuDraw::MouseButtonDown(rMEvt);

    aMDPos = pWindow->PixelToLogic( rMEvt.GetPosPixel() );

    if ( rMEvt.IsLeft() )
    {
        SdrHdl* pHdl = pView->HitHandle(aMDPos, *pWindow);
        SdrObject* pObj;
        SdrPageView* pPV;

        if ( pHdl!=NULL || pView->IsMarkedHit(aMDPos) )
        {
            aDragTimer.Start();
            pView->BegDragObj(aMDPos, (OutputDevice*) NULL, pHdl);
            bReturn = TRUE;
        }
        else
        {
            BOOL bAlt = rMEvt.IsMod2();
            if ( !bAlt && pView->PickObj(aMDPos, pObj, pPV, SDRSEARCH_PICKMACRO) )
            {
                pView->BegMacroObj(aMDPos, pObj, pPV, pWindow);
                bReturn = TRUE;
            }
            else
            {
                //  URL / ImageMap

                SdrViewEvent aVEvt;
                if ( !bAlt &&
                    pView->PickAnything( rMEvt, SDRMOUSEBUTTONDOWN, aVEvt ) != SDRHIT_NONE &&
                    aVEvt.pObj != NULL )
                {
                    if ( ScDrawLayer::GetIMapInfo( aVEvt.pObj ) )       // ImageMap
                    {
                        const IMapObject* pIMapObj =
                                ScDrawLayer::GetHitIMapObject( aVEvt.pObj, aMDPos, *pWindow );
                        if ( pIMapObj && pIMapObj->GetURL().Len() )
                        {
                            ScGlobal::OpenURL( pIMapObj->GetURL(), pIMapObj->GetTarget() );

                            pViewShell->FakeButtonUp( pViewShell->GetViewData()->GetActivePart() );
                            return TRUE;        // kein CaptureMouse etc.
                        }
                    }
                    if ( aVEvt.eEvent == SDREVENT_EXECUTEURL && aVEvt.pURLField )   // URL
                    {
                        ScGlobal::OpenURL( aVEvt.pURLField->GetURL(),
                                            aVEvt.pURLField->GetTargetFrame() );

                        pViewShell->FakeButtonUp( pViewShell->GetViewData()->GetActivePart() );
                        return TRUE;        // kein CaptureMouse etc.
                    }
                }

                //  Markieren

                if ( !rMEvt.IsShift() )
                    pView->UnmarkAll();

                if ( pView->MarkObj(aMDPos, -2, FALSE, rMEvt.IsMod1()) )
                {
                    //*********************************************************
                    //Objekt verschieben
                    //********************************************************
                    if (pView->IsMarkedHit(aMDPos))
                    {
                        aDragTimer.Start();
                        pHdl=pView->HitHandle(aMDPos, *pWindow);
                        pView->BegDragObj(aMDPos, (OutputDevice*) NULL, pHdl);
                        bReturn = TRUE;
                    }
                    else                                    // Objekt am Rand getroffen
                        if (pViewShell->IsDrawSelMode())
                            bReturn = TRUE;
                }
                else
                {
                    //      nichts getroffen

                    if (pViewShell->IsDrawSelMode())
                    {
                        //*********************************************************
                        //Objekt selektieren
                        //********************************************************
                        pView->BegMarkObj(aMDPos, (OutputDevice*) NULL);
                        bReturn = TRUE;
                    }
                }
            }
        }

        if ( !bReturn )
            bReturn = TestComment( pView->GetPageViewPvNum(0), aMDPos );
    }

    if (!bIsInDragMode)
    {
        if (!bVCAction)                 // VC rufen selber CaptureMouse
            pWindow->CaptureMouse();
        ForcePointer(&rMEvt);
    }

    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

BOOL __EXPORT FuSelection::MouseMove(const MouseEvent& rMEvt)
{
    BOOL bReturn = FuDraw::MouseMove(rMEvt);

    if (aDragTimer.IsActive() )
    {
        Point aOldPixel = pWindow->LogicToPixel( aMDPos );
        Point aNewPixel = rMEvt.GetPosPixel();
        if ( Abs( aOldPixel.X() - aNewPixel.X() ) > SC_MAXDRAGMOVE ||
             Abs( aOldPixel.Y() - aNewPixel.Y() ) > SC_MAXDRAGMOVE )
            aDragTimer.Stop();
    }

    if ( pView->IsAction() )
    {
        Point aPix(rMEvt.GetPosPixel());
        Point aPnt(pWindow->PixelToLogic(aPix));

        ForceScroll(aPix);
        pView->MovAction(aPnt);
        bReturn = TRUE;
    }

    // Ein VCControl ist aktiv
    // Event an den Manager weiterleiten
    if( bVCAction )
    {
        //  GetSbxForm gibts nicht mehr - Basic-Controls sind tot
        //SdrPageView* pPgView = pView->GetPageViewPvNum(0);
        //ScDrawPage*  pPage     = (ScDrawPage*)pPgView->GetPage();
        //VCSbxForm* pForm = (VCSbxForm*)(SbxObject*)(pPage->GetSbxForm());
        //((VCManager*)(pForm->GetVCContainer()))->
        //    MouseMove( pWindow, rMEvt );
        bReturn = TRUE;
    }

    ForcePointer(&rMEvt);

    return (bReturn);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

BOOL __EXPORT FuSelection::MouseButtonUp(const MouseEvent& rMEvt)
{
    BOOL bReturn = FuDraw::MouseButtonUp(rMEvt);
//  BOOL bOle    = pViewShell->GetViewData()->IsOle();
    BOOL bOle    = pViewShell->GetViewFrame()->ISA(SfxInPlaceFrame);

    if (aDragTimer.IsActive() )
    {
        aDragTimer.Stop();
    }

    Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

    if ( rMEvt.IsLeft() )
    {
        if ( pView->IsDragObj() )
        {
            /******************************************************************
            * Objekt wurde verschoben
            ******************************************************************/
            pView->EndDragObj( rMEvt.IsMod1() );
            pView->ForceMarkedToAnotherPage();
            bReturn = TRUE;
        }
        else if (pView->IsAction() )
        {
            pView->EndAction();
            if ( pView->HasMarkedObj() )
                bReturn = TRUE;
        }
    }

/*
    if ( pView->IsObjEdit() )
    {
        BOOL bShowCursor = TRUE;
//!     pOutlinerView = pView->GetOutlinerView(pWindow, bShowCursor);
        bReturn = TRUE;
    }
*/
    /**************************************************************************
    * Ggf. OLE-Objekt beruecksichtigen
    **************************************************************************/
    SfxInPlaceClient* pIPClient = pViewShell->GetIPClient();

    if (pIPClient)
    {
        if ( pIPClient->IsInPlaceActive() )
        {
            pIPClient->GetProtocol().Reset2Open();
            SFX_APP()->SetViewFrame(pViewShell->GetViewFrame()); // 242.a: ???
        }
    }

    USHORT nClicks = rMEvt.GetClicks();
    if ( nClicks == 2 && rMEvt.IsLeft() )
    {
        if ( pView->HasMarkedObj() )
        {
            const SdrMarkList& rMarkList = pView->GetMarkList();
            if (rMarkList.GetMarkCount() == 1)
            {
                SdrMark* pMark = rMarkList.GetMark(0);
                SdrObject* pObj = pMark->GetObj();

                //  #43984# aktivieren nur, wenn die Maus auch (noch) ueber dem
                //  selektierten Objekt steht

                SdrViewEvent aVEvt;
                SdrHitKind eHit = pView->PickAnything( rMEvt, SDRMOUSEBUTTONDOWN, aVEvt );
                if ( eHit != SDRHIT_NONE && aVEvt.pObj == pObj )
                {
                    UINT16 nSdrObjKind = pObj->GetObjIdentifier();

                    //
                    //  OLE: aktivieren
                    //

                    if (nSdrObjKind == OBJ_OLE2)
                    {
                        if (!bOle)
                        {
                            SvInPlaceObjectRef aIPObj = ((SdrOle2Obj*) pObj)->GetObjRef();
                            if (aIPObj.Is())
                            {
                                pView->HideMarkHdl(NULL);
                                pViewShell->ActivateObject( (SdrOle2Obj*) pObj, 0 );
                            }
                        }
                    }

                    //
                    //  Text: editieren
                    //  #49458# nicht bei Uno-Controls
                    //
                    else if ( pObj->ISA(SdrTextObj) && !pObj->ISA(SdrUnoObj) )
                    {
                        OutlinerParaObject* pOPO = pObj->GetOutlinerParaObject();
                        BOOL bVertical = ( pOPO && pOPO->IsVertical() );
                        USHORT nTextSlotId = bVertical ? SID_DRAW_TEXT_VERTICAL : SID_DRAW_TEXT;

                        pViewShell->GetViewData()->GetDispatcher().
                            Execute(nTextSlotId, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD);

                        // jetzt den erzeugten FuText holen und in den EditModus setzen
                        FuPoor* pPoor = pViewShell->GetViewData()->GetView()->GetDrawFuncPtr();
                        if ( pPoor && pPoor->GetSlotID() == nTextSlotId )    // hat keine RTTI
                        {
                            FuText* pText = (FuText*)pPoor;
                            Point aMousePixel = rMEvt.GetPosPixel();
                            pText->SetInEditMode( pObj, &aMousePixel );
                        }
                        bReturn = TRUE;
                    }
                }
            }
        }
        else if ( TestDetective( pView->GetPageViewPvNum(0), aPnt ) )
            bReturn = TRUE;
    }

    // Ein VCControl ist aktiv
    // Event an den Manager weiterleiten
    if( bVCAction )
    {
        //  GetSbxForm gibts nicht mehr - Basic-Controls sind tot
        //SdrPageView* pPgView = pView->GetPageViewPvNum(0);
        //ScDrawPage*  pPage     = (ScDrawPage*)pPgView->GetPage();
        //VCSbxForm* pForm = (VCSbxForm*)(SbxObject*)(pPage->GetSbxForm());
        //((VCManager*)(pForm->GetVCContainer()))->
        //    MouseButtonUp( pWindow, rMEvt );
        pView->ShowMarkHdl( pWindow );
        bVCAction = FALSE;
        bReturn = TRUE;
    }

    ForcePointer(&rMEvt);

    pWindow->ReleaseMouse();

    //  Command-Handler fuer Kontext-Menue kommt erst nach MouseButtonUp,
    //  darum hier die harte IsLeft-Abfrage
    if ( !bReturn && rMEvt.IsLeft() )
        if (pViewShell->IsDrawSelMode())
            pViewShell->GetViewData()->GetDispatcher().
                Execute(SID_OBJECT_SELECT, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);

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

    if (!bReturn)
    {
        bReturn = FuDraw::KeyInput(rKEvt);
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
/*
    SdrDragMode eMode;
    switch (aSfxRequest.GetSlot() )
    {
        case SID_OBJECT_SELECT:
            eMode = SDRDRAG_MOVE;
            break;
        case SID_OBJECT_ROTATE:
            eMode = SDRDRAG_ROTATE;
            break;
        case SID_OBJECT_MIRROR:
            eMode = SDRDRAG_MIRROR;
            break;
    }
    pView->SetDragMode(eMode);
*/
    FuDraw::Activate();
}



/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuSelection::Deactivate()
{
    /**************************************************************************
    * Hide Cursor
    **************************************************************************/
    BOOL bShowCursor = FALSE;
//! pOutlinerView = pView->GetOutlinerView(pWindow, bShowCursor);

//  pView->SetDragMode(SDRDRAG_MOVE);
    FuDraw::Deactivate();
}


#ifdef WNT
#pragma optimize ( "", on )
#endif





