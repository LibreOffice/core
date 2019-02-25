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

#include <editeng/outlobj.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdouno.hxx>
#include <sfx2/dispatch.hxx>

#include <fuconstr.hxx>
#include <fudraw.hxx>
#include <tabvwsh.hxx>
#include <futext.hxx>
#include <drawview.hxx>

//  maximal permitted mouse movement to start Drag&Drop
//! fusel,fuconstr,futext - combine them!
#define SC_MAXDRAGMOVE  3

FuConstruct::FuConstruct(ScTabViewShell& rViewSh, vcl::Window* pWin, ScDrawView* pViewP,
                         SdrModel* pDoc, const SfxRequest& rReq)
    : FuDraw(rViewSh, pWin, pViewP, pDoc, rReq)
{
}

FuConstruct::~FuConstruct()
{
}

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

        if ( pHdl != nullptr || pView->IsMarkedHit(aMDPos) )
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

    if ( pView->IsAction() )
    {
        ForceScroll(aPix);
        pView->MovAction(aPnt);
    }
    else
    {
        SdrHdl* pHdl=pView->PickHandle(aPnt);

        if ( pHdl != nullptr )
        {
            rViewShell.SetActivePointer(pHdl->GetPointer());
        }
        else if ( pView->IsMarkedHit(aPnt) )
        {
            rViewShell.SetActivePointer(PointerStyle::Move);
        }
        else
        {
            rViewShell.SetActivePointer( aNewPointer );
        }
    }
    return true;
}

bool FuConstruct::MouseButtonUp(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    bool bReturn = SimpleMouseButtonUp( rMEvt );

    //      Double-click on text object? (->fusel)

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

                //  if Uno-Controls no text mode
                if ( dynamic_cast<const SdrTextObj*>( pObj) != nullptr && dynamic_cast<const SdrUnoObj*>( pObj) ==  nullptr )
                {
                    OutlinerParaObject* pOPO = pObj->GetOutlinerParaObject();
                    bool bVertical = ( pOPO && pOPO->IsVertical() );
                    sal_uInt16 nTextSlotId = bVertical ? SID_DRAW_TEXT_VERTICAL : SID_DRAW_TEXT;

                    rViewShell.GetViewData().GetDispatcher().
                        Execute(nTextSlotId, SfxCallMode::SLOT | SfxCallMode::RECORD);

                    // Get the created FuText now and change into EditMode
                    FuPoor* pPoor = rViewShell.GetViewData().GetView()->GetDrawFuncPtr();
                    if ( pPoor && pPoor->GetSlotID() == nTextSlotId )    // has no RTTI
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

//      SimpleMouseButtonUp - no test on double-click

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

            SfxDispatcher& rDisp = rViewShell.GetViewData().GetDispatcher();
            if ( pView->AreObjectsMarked() )
                rDisp.Execute(SID_OBJECT_SELECT, SfxCallMode::SLOT | SfxCallMode::RECORD);
            else
                rDisp.Execute(aSfxRequest.GetSlot(), SfxCallMode::SLOT | SfxCallMode::RECORD);
        }
    }

    return bReturn;
}

// If we handle a KeyEvent, then the return value is sal_True else FALSE.
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
            else                            // end drawing mode
            {
                rViewShell.GetViewData().GetDispatcher().
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
