/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fusel.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-01 18:22:33 $
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
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <svx/flditem.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdotext.hxx>
#include <sfx2/dispatch.hxx>
#include <svtools/imapobj.hxx>
#include <svx/svdview.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdomedia.hxx>
#include <svx/svdpagv.hxx>
#include <svx/outlobj.hxx>
#include <svx/svdocapt.hxx>


#ifndef _COM_SUN_STAR_EMBED_EMBEDSTATES_HPP_
#include <com/sun/star/embed/EmbedStates.hpp>
#endif


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

using namespace com::sun::star;

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
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

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
            // Determine if this is the tail of a SdrCaptionObj i.e.
            // we need to disable the drag option on the tail of a note
            // object. Also, disable the ability to use the circular
            // drag of a note object.
            bool bDrag = false;
            const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
            if( rMarkList.GetMarkCount() == 1 )
            {
                SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();
                if( pObj && pObj->ISA( SdrCaptionObj )&& pObj->GetLayer() == SC_LAYER_INTERN)
                {
                    // move using the valid caption handles for note text box.
                    if(pHdl && (pHdl->GetKind() != HDL_POLY && pHdl->GetKind() != HDL_CIRC))
                        bDrag = true;
                    // move the complete note box.
                    else if(!pHdl)
                        bDrag = true;
                }
                else
                    bDrag = true;   // different object
            }
            else
                bDrag = true;       // several objects

            if ( bDrag )
            {
                aDragTimer.Start();
                pView->BegDragObj(aMDPos, (OutputDevice*) NULL, pHdl);
                bReturn = TRUE;
            }
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

                //  Is another object being edited in this view?
                //  (Editing is ended in MarkListHasChanged - test before UnmarkAll)
                SfxInPlaceClient* pClient = pViewShell->GetIPClient();
                BOOL bWasOleActive = ( pClient && pClient->IsObjectInPlaceActive() );

                //  Markieren

                if ( !rMEvt.IsShift() )
                    pView->UnmarkAll();

                // if a comment: unlock the internal layer here.
                // re-lock in ScDrawView::MarkListHasChanged()
                TestComment( pView->GetPageViewPvNum(0), aMDPos );

                if ( pView->MarkObj(aMDPos, -2, FALSE, rMEvt.IsMod1()) )
                {
                    //*********************************************************
                    //Objekt verschieben
                    //********************************************************
                    if (pView->IsMarkedHit(aMDPos))
                    {
                        //  #95834# Don't start drag timer if inplace editing of an OLE object
                        //  was just ended with this mouse click - the view will be moved
                        //  (different tool bars) and the object that was clicked on would
                        //  be moved unintentionally.
                        if ( !bWasOleActive )
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
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    BOOL bReturn = FuDraw::MouseButtonUp(rMEvt);
//  BOOL bOle    = pViewShell->GetViewData()->IsOle();
    BOOL bOle = pViewShell->GetViewFrame()->GetFrame()->IsInPlace();

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

            const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
            if (rMarkList.GetMarkCount() == 1)
            {
                  SdrMark* pMark = rMarkList.GetMark(0);
                  SdrObject* pObj = pMark->GetMarkedSdrObj();
                  FuPoor* pPoor = pViewShell->GetViewData()->GetView()->GetDrawFuncPtr();
                  FuText* pText = static_cast<FuText*>(pPoor);
                pText->StopDragMode(pObj );
            }
            bReturn = TRUE;
        }
        else if (pView->IsAction() )
        {
            pView->EndAction();
            if ( pView->AreObjectsMarked() )
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
        if ( pIPClient->IsObjectInPlaceActive() )
            pIPClient->DeactivateObject();
    }

    USHORT nClicks = rMEvt.GetClicks();
    if ( nClicks == 2 && rMEvt.IsLeft() )
    {
        if ( pView->AreObjectsMarked() )
        {
            const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
            if (rMarkList.GetMarkCount() == 1)
            {
                SdrMark* pMark = rMarkList.GetMark(0);
                SdrObject* pObj = pMark->GetMarkedSdrObj();

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
                            if (((SdrOle2Obj*) pObj)->GetObjRef().is())
                            {
                                pView->HideMarkHdl(NULL);
                                pViewShell->ActivateObject( (SdrOle2Obj*) pObj, 0 );
                            }
                        }
                    }

                    //
                    //  Edit text
                    //  #49458# not in UNO controls
                    //  #i32352# not in media objects
                    //
                    else if ( pObj->ISA(SdrTextObj) && !pObj->ISA(SdrUnoObj) && !pObj->ISA(SdrMediaObj) )
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





