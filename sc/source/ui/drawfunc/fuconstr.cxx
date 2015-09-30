/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
#include "document.hxx"
#include "gridwin.hxx"

//  Maximal erlaubte Mausbewegung um noch Drag&Drop zu starten
//! fusel,fuconstr,futext - zusammenfassen!
#define SC_MAXDRAGMOVE  3

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuConstruct::FuConstruct(ScTabViewShell* pViewSh, vcl::Window* pWin, ScDrawView* pViewP,
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

sal_uInt8 FuConstruct::Command(const CommandEvent& rCEvt)
{
    return FuDraw::Command( rCEvt );
}

// Calculate and return offset at current zoom. rInOutPos is adjusted by
// the calculated offset. rInOutPos now points to the position than when
// scaled to 100% actually would be at the position you see at the current zoom
// ( relative to the grid ) note: units are expected to be in 100th mm
Point FuConstruct::CurrentGridSyncOffsetAndPos( Point& rInOutPos )
{
    Point aRetGridOff;
    ScViewData& rViewData = pViewShell->GetViewData();
    ScDocument* pDoc = rViewData.GetDocument();
    if ( pDoc )
    {
        // rInOutPos might not be where you think it is if there is zoom
        // involved. Lets calculate where aPos would be at 100% zoom
        // that's the actual correct position for the object (when you
        // restore the zoom.
        bool bNegative = pDoc->IsNegativePage(pView->GetTab());
        Rectangle aObjRect( rInOutPos, rInOutPos );
        ScRange aRange = pDoc->GetRange( pView->GetTab(), aObjRect );
        ScAddress aOldStt = aRange.aStart;
        Point aOldPos( pDoc->GetColOffset( aOldStt.Col(), aOldStt.Tab()  ), pDoc->GetRowOffset( aOldStt.Row(), aOldStt.Tab() ) );
        aOldPos.X() = sc::TwipsToHMM( aOldPos.X() );
        aOldPos.Y() = sc::TwipsToHMM( aOldPos.Y() );
        ScSplitPos eWhich = rViewData.GetActivePart();
        ScGridWindow* pGridWin = rViewData.GetActiveWin();
        // and equiv screen pos
        Point aScreenPos =  pViewShell->GetViewData().GetScrPos( aOldStt.Col(), aOldStt.Row(), eWhich, true );
        MapMode aDrawMode = pGridWin->GetDrawMapMode();
        Point aCurPosHmm = pGridWin->PixelToLogic(aScreenPos, aDrawMode );
        Point aOff = ( rInOutPos - aCurPosHmm );
        rInOutPos = aOldPos + aOff;
        aRetGridOff = aCurPosHmm - aOldPos;
        // fdo#64011 fix the X position when the sheet are RTL
        if ( bNegative )
        {
            aRetGridOff.setX( aCurPosHmm.getX() + aOldPos.getX() );
            rInOutPos.setX( aOff.getX() - aOldPos.getX() );
        }
    }
    return aRetGridOff;
}
/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

bool FuConstruct::MouseButtonDown(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    bool bReturn = FuDraw::MouseButtonDown(rMEvt);

    if ( pView->IsAction() )
    {
        if ( rMEvt.IsRight() )
            pView->BckAction();
        return true;
    }

    aDragTimer.Start();

    aMDPos = pWindow->PixelToLogic( rMEvt.GetPosPixel() );

    if ( rMEvt.IsLeft() )
    {
        pWindow->CaptureMouse();

        SdrHdl* pHdl = pView->PickHandle(aMDPos);

        if ( pHdl != NULL || pView->IsMarkedHit(aMDPos) )
        {
            pView->BegDragObj(aMDPos, nullptr, pHdl, 1);
            bReturn = true;
        }
        else if ( pView->AreObjectsMarked() )
        {
            pView->UnmarkAll();
            bReturn = true;
        }
    }

    bIsInDragMode = false;

    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

bool FuConstruct::MouseMove(const MouseEvent& rMEvt)
{
    FuDraw::MouseMove(rMEvt);

    if (aDragTimer.IsActive() )
    {
        Point aOldPixel = pWindow->LogicToPixel( aMDPos );
        Point aNewPixel = rMEvt.GetPosPixel();
        if ( std::abs( aOldPixel.X() - aNewPixel.X() ) > SC_MAXDRAGMOVE ||
             std::abs( aOldPixel.Y() - aNewPixel.Y() ) > SC_MAXDRAGMOVE )
            aDragTimer.Stop();
    }

    Point aPix(rMEvt.GetPosPixel());
    Point aPnt( pWindow->PixelToLogic(aPix) );

    // if object is being created then more than likely the mouse
    // position has been 'adjusted' for the current zoom, need to
    // restore the mouse position here to ensure resize works as expected
    if ( pView->GetCreateObj() )
        aPnt -= pView->GetCreateObj()->GetGridOffset();

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
            pViewShell->SetActivePointer(Pointer(PointerStyle::Move));
        }
        else
        {
            pViewShell->SetActivePointer( aNewPointer );
        }
    }
    return true;
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

bool FuConstruct::MouseButtonUp(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    bool bReturn = SimpleMouseButtonUp( rMEvt );

    //      Doppelklick auf Textobjekt? (->fusel)

    sal_uInt16 nClicks = rMEvt.GetClicks();
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
                if ( dynamic_cast<const SdrTextObj*>( pObj) != nullptr && dynamic_cast<const SdrUnoObj*>( pObj) ==  nullptr )
                {
                    OutlinerParaObject* pOPO = pObj->GetOutlinerParaObject();
                    bool bVertical = ( pOPO && pOPO->IsVertical() );
                    sal_uInt16 nTextSlotId = bVertical ? SID_DRAW_TEXT_VERTICAL : SID_DRAW_TEXT;

                    pViewShell->GetViewData().GetDispatcher().
                        Execute(nTextSlotId, SfxCallMode::SLOT | SfxCallMode::RECORD);

                    // jetzt den erzeugten FuText holen und in den EditModus setzen
                    FuPoor* pPoor = pViewShell->GetViewData().GetView()->GetDrawFuncPtr();
                    if ( pPoor && pPoor->GetSlotID() == nTextSlotId )    // hat keine RTTI
                    {
                        FuText* pText = static_cast<FuText*>(pPoor);
                        Point aMousePixel = rMEvt.GetPosPixel();
                        pText->SetInEditMode( pObj, &aMousePixel );
                    }
                    bReturn = true;
                }
            }
        }
    }

    FuDraw::MouseButtonUp(rMEvt);

    return bReturn;
}

//      SimpleMouseButtonUp - ohne Test auf Doppelklick

bool FuConstruct::SimpleMouseButtonUp(const MouseEvent& rMEvt)
{
    bool    bReturn = true;

    if (aDragTimer.IsActive() )
    {
        aDragTimer.Stop();
    }

    Point   aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

    if ( pView->IsDragObj() )
         pView->EndDragObj( rMEvt.IsMod1() );

    else if ( pView->IsMarkObj() )
        pView->EndMarkObj();

    else bReturn = false;

    if ( !pView->IsAction() )
    {
        pWindow->ReleaseMouse();

        if ( !pView->AreObjectsMarked() && rMEvt.GetClicks() < 2 )
        {
            pView->MarkObj(aPnt, -2, false, rMEvt.IsMod1());

            SfxDispatcher& rDisp = pViewShell->GetViewData().GetDispatcher();
            if ( pView->AreObjectsMarked() )
                rDisp.Execute(SID_OBJECT_SELECT, SfxCallMode::SLOT | SfxCallMode::RECORD);
            else
                rDisp.Execute(aSfxRequest.GetSlot(), SfxCallMode::SLOT | SfxCallMode::RECORD);
        }
    }

    return bReturn;
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert sal_True, andernfalls
|* FALSE.
|*
\************************************************************************/

bool FuConstruct::KeyInput(const KeyEvent& rKEvt)
{
    bool bReturn = false;

    switch ( rKEvt.GetKeyCode().GetCode() )
    {
        case KEY_ESCAPE:
            if ( pView->IsAction() )
            {
                pView->BrkAction();
                pWindow->ReleaseMouse();
                bReturn = true;
            }
            else                            // Zeichenmodus beenden
            {
                pViewShell->GetViewData().GetDispatcher().
                    Execute(aSfxRequest.GetSlot(), SfxCallMode::SLOT | SfxCallMode::RECORD);
            }
            break;

        case KEY_DELETE:
            pView->DeleteMarked();
            bReturn = true;
            break;
    }

    if ( !bReturn )
    {
        bReturn = FuDraw::KeyInput(rKEvt);
    }

    return bReturn;
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
