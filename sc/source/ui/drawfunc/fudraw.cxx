/*************************************************************************
 *
 *  $RCSfile: fudraw.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-09-22 18:53:18 $
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

//------------------------------------------------------------------------

#include <svx/svdobj.hxx>
#include <svx/svdview.hxx>
#include <sfx2/dispatch.hxx>

#include "sc.hrc"
#include "fudraw.hxx"
#include "tabvwsh.hxx"
#include "drwlayer.hxx"
#include "scresid.hxx"


/*************************************************************************
|*
|* Basisklasse fuer alle Drawmodul-spezifischen Funktionen
|*
\************************************************************************/

FuDraw::FuDraw(ScTabViewShell* pViewSh, Window* pWin, SdrView* pView,
               SdrModel* pDoc, SfxRequest& rReq) :
    FuPoor      (pViewSh, pWin, pView, pDoc, rReq),
    aNewPointer ( POINTER_ARROW ),
    aOldPointer ( POINTER_ARROW )
{
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuDraw::~FuDraw()
{
}

/*************************************************************************
|*
|* Modifier-Tasten auswerten
|*
\************************************************************************/

void FuDraw::DoModifiers(const MouseEvent& rMEvt)
{
    //  Shift   = Ortho und AngleSnap
    //  Control = Snap (Toggle)
    //  Alt     = zentrisch

    BOOL bShift = rMEvt.IsShift();
    BOOL bCtrl  = rMEvt.IsMod1();
    BOOL bAlt   = rMEvt.IsMod2();

    ScViewData* pViewData = pViewShell->GetViewData();
    const ScViewOptions& rOpt = pViewData->GetOptions();
    const ScGridOptions& rGrid = rOpt.GetGridOptions();
    BOOL bGridOpt = rGrid.GetUseGridSnap();

    BOOL bOrtho     = bShift;
    BOOL bAngleSnap = bShift;
    BOOL bGridSnap  = ( bGridOpt != bCtrl );        // andere Snap's nicht unterstuetzt
    BOOL bCenter    = bAlt;

    if (pView->IsOrtho() != bOrtho)
        pView->SetOrtho(bOrtho);
    if (pView->IsAngleSnapEnabled() != bAngleSnap)
        pView->SetAngleSnapEnabled(bAngleSnap);

/*  Control fuer Snap beisst sich beim Verschieben mit "kopieren" !!!

    if (pView->IsGridSnap() != bGridSnap)
        pView->SetGridSnap(bGridSnap);
    if (pView->IsSnapEnabled() != bGridSnap)
        pView->SetSnapEnabled(bGridSnap);
*/
    if (pView->IsCreate1stPointAsCenter() != bCenter)
        pView->SetCreate1stPointAsCenter(bCenter);
    if (pView->IsResizeAtCenter() != bCenter)
        pView->SetResizeAtCenter(bCenter);
}

void FuDraw::ResetModifiers()
{
    ScViewData* pViewData = pViewShell->GetViewData();
    const ScViewOptions& rOpt = pViewData->GetOptions();
    const ScGridOptions& rGrid = rOpt.GetGridOptions();
    BOOL bGridOpt = rGrid.GetUseGridSnap();

    if (pView->IsOrtho())
        pView->SetOrtho(FALSE);
    if (pView->IsAngleSnapEnabled())
        pView->SetAngleSnapEnabled(FALSE);

    if (pView->IsGridSnap() != bGridOpt)
        pView->SetGridSnap(bGridOpt);
    if (pView->IsSnapEnabled() != bGridOpt)
        pView->SetSnapEnabled(bGridOpt);

    if (pView->IsCreate1stPointAsCenter())
        pView->SetCreate1stPointAsCenter(FALSE);
    if (pView->IsResizeAtCenter())
        pView->SetResizeAtCenter(FALSE);
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

BOOL __EXPORT FuDraw::MouseButtonDown(const MouseEvent& rMEvt)
{
    DoModifiers( rMEvt );
    return FALSE;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

BOOL __EXPORT FuDraw::MouseMove(const MouseEvent& rMEvt)
{
    DoModifiers( rMEvt );
    return FALSE;
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

BOOL __EXPORT FuDraw::MouseButtonUp(const MouseEvent& rMEvt)
{
    ResetModifiers();
    return FALSE;
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert TRUE, andernfalls
|* FALSE.
|*
\************************************************************************/

BOOL __EXPORT FuDraw::KeyInput(const KeyEvent& rKEvt)
{
    BOOL bReturn = FALSE;

    switch ( rKEvt.GetKeyCode().GetCode() )
    {
        case KEY_ESCAPE:

    /* 18.12.95: TextShell beibehalten nicht mehr gewuenscht...
     *
     *          if ( pView->IsAction() )
     *          {
     *              pView->BrkAction();
     *              pWindow->ReleaseMouse();
     *              bReturn = TRUE;
     *          }
     *          else if ( pView->IsTextEdit() )
     *          {
     *              pView->EndTextEdit();
     *              pView->SetCreateMode();
     *              pViewShell->GetScDrawView()->InvalidateDrawTextAttrs();
     *              bReturn = TRUE;
     *          }
     *          else
     */

            if ( pViewShell->IsDrawTextShell() || aSfxRequest.GetSlot() == SID_DRAW_NOTEEDIT )
            {
                // in normale Draw-Shell, wenn Objekt selektiert, sonst Zeichnen aus
                pViewShell->GetViewData()->GetDispatcher().
                    Execute(aSfxRequest.GetSlot(), SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
                bReturn = TRUE;
            }
            else if ( pViewShell->IsDrawSelMode() )
            {
                pView->UnmarkAll();
                pViewShell->GetViewData()->GetDispatcher().
                    Execute(SID_OBJECT_SELECT, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
                bReturn = TRUE;
            }
            else if ( pView->HasMarkedObj() )
            {
                pView->UnmarkAll();
                //  Beim Bezier-Editieren ist jetzt wieder das Objekt selektiert
                if (!pView->HasMarkedObj())
                    pViewShell->SetDrawShell( FALSE );
                bReturn = TRUE;
            }
            break;

        case KEY_DELETE:                    //! ueber Accelerator
            pView->DeleteMarked();
            bReturn = TRUE;
            break;
    }

    if (!bReturn)
    {
        bReturn = FuPoor::KeyInput(rKEvt);
    }

    return (bReturn);
}

/*************************************************************************
|*
|* Vor dem Scrollen Selektionsdarstellung ausblenden
|*
\************************************************************************/

void FuDraw::ScrollStart()
{
//      HideShownXor in Gridwin
}

/*************************************************************************
|*
|* Nach dem Scrollen Selektionsdarstellung wieder anzeigen
|*
\************************************************************************/

void FuDraw::ScrollEnd()
{
//      ShowShownXor in Gridwin
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuDraw::Activate()
{
    FuPoor::Activate();
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuDraw::Deactivate()
{
    FuPoor::Deactivate();
}

/*************************************************************************
|*
|* Maus-Pointer umschalten
|*
\************************************************************************/

BOOL lcl_UrlHit( SdrView* pView, const Point& rPosPixel, Window* pWindow )
{
    SdrViewEvent aVEvt;
    MouseEvent aMEvt( rPosPixel, 1, 0, MOUSE_LEFT );
    SdrHitKind eHit = pView->PickAnything( aMEvt, SDRMOUSEBUTTONDOWN, aVEvt );

    if ( eHit != SDRHIT_NONE && aVEvt.pObj != NULL )
    {
        if ( ScDrawLayer::GetIMapInfo( aVEvt.pObj ) && ScDrawLayer::GetHitIMapObject(
                                aVEvt.pObj, pWindow->PixelToLogic(rPosPixel), *pWindow ) )
            return TRUE;

        if ( aVEvt.eEvent == SDREVENT_EXECUTEURL )
            return TRUE;
    }

    return FALSE;
}

void FuDraw::ForcePointer(const MouseEvent* pMEvt)
{
    if ( !pView->IsAction() )
    {
#ifdef VCL
        Point aPosPixel = pWindow->GetPointerPosPixel();
#else
        Point aPosPixel = pWindow->ScreenToOutputPixel( Pointer::GetPosPixel() );
#endif
        BOOL bAlt       = pMEvt && pMEvt->IsMod2();
        Point aPnt      = pWindow->PixelToLogic( aPosPixel );
        SdrHdl* pHdl    = pView->HitHandle(aPnt, *pWindow);
        SdrObject* pObj;
        SdrPageView* pPV;

        if ( pView->IsTextEdit() )
        {
            pViewShell->SetActivePointer(Pointer(POINTER_TEXT));        // kann nicht sein ?
        }
        else if ( pHdl )
        {
            pViewShell->SetActivePointer(
                pView->GetPreferedPointer( aPnt, pWindow ) );
        }
        else if ( pView->IsMarkedHit(aPnt) )
        {
            pViewShell->SetActivePointer( Pointer(POINTER_MOVE) );
        }
        else if ( !bAlt && ( !pMEvt || !pMEvt->GetButtons() )
                        && lcl_UrlHit( pView, aPosPixel, pWindow ) )
        {
            //  kann mit ALT unterdrueckt werden
            pWindow->SetPointer( Pointer( POINTER_REFHAND ) );          // Text-URL / ImageMap
        }
        else if ( !bAlt && pView->PickObj(aPnt, pObj, pPV, SDRSEARCH_PICKMACRO) )
        {
            //  kann mit ALT unterdrueckt werden
            SdrObjMacroHitRec aHitRec;  //! muss da noch irgendwas gesetzt werden ????
            pViewShell->SetActivePointer( pObj->GetMacroPointer(aHitRec) );
        }
        else if ( IsDetectiveHit( aPnt ) )
#ifdef VCL
            pViewShell->SetActivePointer( Pointer( POINTER_DETECTIVE ) );
#else
            pViewShell->SetActivePointer( ScResId( RID_SCPTR_DETECTIVE ) );
#endif
        else
            pViewShell->SetActivePointer( aNewPointer );            //! in Gridwin?
    }
}





