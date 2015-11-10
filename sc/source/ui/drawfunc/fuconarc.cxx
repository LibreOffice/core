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

#include "fuconarc.hxx"
#include "sc.hrc"
#include "tabvwsh.hxx"
#include "drawview.hxx"

// Create default drawing objects via keyboard
#include <svx/svdocirc.hxx>
#include <svx/sxciaitm.hxx>

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuConstArc::FuConstArc( ScTabViewShell* pViewSh, vcl::Window* pWin, ScDrawView* pViewP,
                   SdrModel* pDoc, SfxRequest& rReq )
    : FuConstruct( pViewSh, pWin, pViewP, pDoc, rReq )
{
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuConstArc::~FuConstArc()
{
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

bool FuConstArc::MouseButtonDown( const MouseEvent& rMEvt )
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    bool bReturn = FuConstruct::MouseButtonDown( rMEvt );

    if ( rMEvt.IsLeft() && !pView->IsAction() )
    {
        Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
        // Hack  to align object to nearest grid position where object
        // would be anchored ( if it were cell anchored )
        // Get grid offset for current position ( note: aPnt is
        // also adjusted )
        Point aGridOff = CurrentGridSyncOffsetAndPos( aPnt );
        pWindow->CaptureMouse();
        pView->BegCreateObj( aPnt );
        pView->GetCreateObj()->SetGridOffset( aGridOff );
        bReturn = true;
    }
    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

bool FuConstArc::MouseMove( const MouseEvent& rMEvt )
{
    return FuConstruct::MouseMove(rMEvt);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

bool FuConstArc::MouseButtonUp( const MouseEvent& rMEvt )
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    bool bReturn = false;

    if ( pView->IsCreateObj() && rMEvt.IsLeft() )
    {
        pView->EndCreateObj( SDRCREATE_NEXTPOINT );
        bReturn = true;
    }
    return (FuConstruct::MouseButtonUp(rMEvt) || bReturn);
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert sal_True, andernfalls
|* FALSE.
|*
\************************************************************************/

bool FuConstArc::KeyInput(const KeyEvent& rKEvt)
{
    return FuConstruct::KeyInput(rKEvt);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuConstArc::Activate()
{
    SdrObjKind aObjKind;

    switch (aSfxRequest.GetSlot() )
    {
        case SID_DRAW_ARC:
            aNewPointer = Pointer( PointerStyle::DrawArc );
            aObjKind = OBJ_CARC;
            break;

        case SID_DRAW_PIE:
            aNewPointer = Pointer( PointerStyle::DrawPie );
            aObjKind = OBJ_SECT;
            break;

        case SID_DRAW_CIRCLECUT:
            aNewPointer = Pointer( PointerStyle::DrawCircleCut );
            aObjKind = OBJ_CCUT;
            break;

        default:
            aNewPointer = Pointer( PointerStyle::Cross );
            aObjKind = OBJ_CARC;
            break;
    }

    pView->SetCurrentObj( sal::static_int_cast<sal_uInt16>( aObjKind ) );

    aOldPointer = pWindow->GetPointer();
    pViewShell->SetActivePointer( aNewPointer );

    FuDraw::Activate();
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuConstArc::Deactivate()
{
    FuDraw::Deactivate();
    pViewShell->SetActivePointer( aOldPointer );
}

// Create default drawing objects via keyboard
SdrObject* FuConstArc::CreateDefaultObject(const sal_uInt16 nID, const Rectangle& rRectangle)
{
    // case SID_DRAW_ARC:
    // case SID_DRAW_PIE:
    // case SID_DRAW_CIRCLECUT:

    SdrObject* pObj = SdrObjFactory::MakeNewObject(
        pView->GetCurrentObjInventor(), pView->GetCurrentObjIdentifier(),
        nullptr, pDrDoc);

    if(pObj)
    {
        if(dynamic_cast<const SdrCircObj*>( pObj) !=  nullptr)
        {
            Rectangle aRect(rRectangle);

            if(SID_DRAW_ARC == nID || SID_DRAW_CIRCLECUT == nID)
            {
                // force quadratic
                ImpForceQuadratic(aRect);
            }

            pObj->SetLogicRect(aRect);

            SfxItemSet aAttr(pDrDoc->GetItemPool());
            aAttr.Put(makeSdrCircStartAngleItem(9000));
            aAttr.Put(makeSdrCircEndAngleItem(0));

            pObj->SetMergedItemSet(aAttr);
        }
        else
        {
            OSL_FAIL("Object is NO circle object");
        }
    }

    return pObj;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
