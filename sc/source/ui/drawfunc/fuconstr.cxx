/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include <editeng/outliner.hxx>
#include <editeng/outlobj.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdouno.hxx>
#include <sfx2/dispatch.hxx>

#include "fuconstr.hxx"
#include "fudraw.hxx"
#include "tabvwsh.hxx"
#include "futext.hxx"
#include "sc.hrc"
#include "drawview.hxx"

//  Maximal erlaubte Mausbewegung um noch Drag&Drop zu starten
//! fusel,fuconstr,futext - zusammenfassen!
#define SC_MAXDRAGMOVE  3

//------------------------------------------------------------------------

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuConstruct::FuConstruct(ScTabViewShell* pViewSh, Window* pWin, ScDrawView* pViewP,
                   SdrModel* pDoc, SfxRequest& rReq) :
    FuDraw(pViewSh, pWin, pViewP, pDoc, rReq)
{
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuConstruct::~FuConstruct()
{
}

BYTE FuConstruct::Command(const CommandEvent& rCEvt)
{
    //  special code for non-VCL OS2/UNX removed

    return FuDraw::Command( rCEvt );
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

BOOL FuConstruct::MouseButtonDown(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    BOOL bReturn = FuDraw::MouseButtonDown(rMEvt);

    if ( pView->IsAction() )
    {
        if ( rMEvt.IsRight() )
            pView->BckAction();
        return TRUE;
    }

    aDragTimer.Start();

    aMDPos = pWindow->PixelToLogic( rMEvt.GetPosPixel() );

    if ( rMEvt.IsLeft() )
    {
        pWindow->CaptureMouse();

        SdrHdl* pHdl = pView->PickHandle(aMDPos);

        if ( pHdl != NULL || pView->IsMarkedHit(aMDPos) )
        {
            pView->BegDragObj(aMDPos, (OutputDevice*) NULL, pHdl, 1);
            bReturn = TRUE;
        }
        else if ( pView->AreObjectsMarked() )
        {
            pView->UnmarkAll();
            bReturn = TRUE;
        }
    }

    bIsInDragMode = FALSE;

    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

BOOL FuConstruct::MouseMove(const MouseEvent& rMEvt)
{
    FuDraw::MouseMove(rMEvt);

    if (aDragTimer.IsActive() )
    {
        Point aOldPixel = pWindow->LogicToPixel( aMDPos );
        Point aNewPixel = rMEvt.GetPosPixel();
        if ( Abs( aOldPixel.X() - aNewPixel.X() ) > SC_MAXDRAGMOVE ||
             Abs( aOldPixel.Y() - aNewPixel.Y() ) > SC_MAXDRAGMOVE )
            aDragTimer.Stop();
    }

    Point aPix(rMEvt.GetPosPixel());
    Point aPnt( pWindow->PixelToLogic(aPix) );

    if ( pView->IsAction() )
    {
        ForceScroll(aPix);
        pView->MovAction(aPnt);
    }
    else
    {
        SdrHdl* pHdl=pView->PickHandle(aPnt);

        if ( pHdl != NULL )
        {
            pViewShell->SetActivePointer(pHdl->GetPointer());
        }
        else if ( pView->IsMarkedHit(aPnt) )
        {
            pViewShell->SetActivePointer(Pointer(POINTER_MOVE));
        }
        else
        {
            pViewShell->SetActivePointer( aNewPointer );
        }
    }
    return TRUE;
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

BOOL FuConstruct::MouseButtonUp(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    BOOL bReturn = SimpleMouseButtonUp( rMEvt );

    //      Doppelklick auf Textobjekt? (->fusel)

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

                //  bei Uno-Controls nicht in Textmodus
                if ( pObj->ISA(SdrTextObj) && !pObj->ISA(SdrUnoObj) )
                {
                    OutlinerParaObject* pOPO = pObj->GetOutlinerParaObject();
                    BOOL bVertical = ( pOPO && pOPO->IsVertical() );
                    USHORT nTextSlotId = bVertical ? SID_DRAW_TEXT_VERTICAL : SID_DRAW_TEXT;

                    pViewShell->GetViewData()->GetDispatcher().
                        Execute(nTextSlotId, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);

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

    FuDraw::MouseButtonUp(rMEvt);

    return bReturn;
}

//      SimpleMouseButtonUp - ohne Test auf Doppelklick

BOOL FuConstruct::SimpleMouseButtonUp(const MouseEvent& rMEvt)
{
    BOOL    bReturn = TRUE;

    if (aDragTimer.IsActive() )
    {
        aDragTimer.Stop();
    }

    Point   aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

    if ( pView->IsDragObj() )
         pView->EndDragObj( rMEvt.IsMod1() );

    else if ( pView->IsMarkObj() )
        pView->EndMarkObj();

    else bReturn = FALSE;

    if ( !pView->IsAction() )
    {
        pWindow->ReleaseMouse();

        if ( !pView->AreObjectsMarked() && rMEvt.GetClicks() < 2 )
        {
            pView->MarkObj(aPnt, -2, FALSE, rMEvt.IsMod1());

            SfxDispatcher& rDisp = pViewShell->GetViewData()->GetDispatcher();
            if ( pView->AreObjectsMarked() )
                rDisp.Execute(SID_OBJECT_SELECT, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
            else
                rDisp.Execute(aSfxRequest.GetSlot(), SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
        }
    }

    return bReturn;
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert TRUE, andernfalls
|* FALSE.
|*
\************************************************************************/

BOOL FuConstruct::KeyInput(const KeyEvent& rKEvt)
{
    BOOL bReturn = FALSE;

    switch ( rKEvt.GetKeyCode().GetCode() )
    {
        case KEY_ESCAPE:
            if ( pView->IsAction() )
            {
                pView->BrkAction();
                pWindow->ReleaseMouse();
                bReturn = TRUE;
            }
            else                            // Zeichenmodus beenden
            {
                pViewShell->GetViewData()->GetDispatcher().
                    Execute(aSfxRequest.GetSlot(), SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
            }
            break;

        case KEY_DELETE:
            pView->DeleteMarked();
            bReturn = TRUE;
            break;
    }

    if ( !bReturn )
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

void FuConstruct::Activate()
{
    FuDraw::Activate();
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuConstruct::Deactivate()
{
    FuDraw::Deactivate();
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
