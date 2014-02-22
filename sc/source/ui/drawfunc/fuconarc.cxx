/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "fuconarc.hxx"
#include "sc.hrc"
#include "tabvwsh.hxx"
#include "drawview.hxx"


#include <svx/svdocirc.hxx>
#include <svx/sxciaitm.hxx>

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuConstArc::FuConstArc( ScTabViewShell* pViewSh, Window* pWin, ScDrawView* pViewP,
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
    
    SetMouseButtonCode(rMEvt.GetButtons());

    bool bReturn = FuConstruct::MouseButtonDown( rMEvt );

    if ( rMEvt.IsLeft() && !pView->IsAction() )
    {
        Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
        
        
        
        
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
            aNewPointer = Pointer( POINTER_DRAW_ARC );
            aObjKind = OBJ_CARC;
            break;

        case SID_DRAW_PIE:
            aNewPointer = Pointer( POINTER_DRAW_PIE );
            aObjKind = OBJ_SECT;
            break;

        case SID_DRAW_CIRCLECUT:
            aNewPointer = Pointer( POINTER_DRAW_CIRCLECUT );
            aObjKind = OBJ_CCUT;
            break;

        default:
            aNewPointer = Pointer( POINTER_CROSS );
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


SdrObject* FuConstArc::CreateDefaultObject(const sal_uInt16 nID, const Rectangle& rRectangle)
{
    
    
    

    SdrObject* pObj = SdrObjFactory::MakeNewObject(
        pView->GetCurrentObjInventor(), pView->GetCurrentObjIdentifier(),
        0L, pDrDoc);

    if(pObj)
    {
        if(pObj->ISA(SdrCircObj))
        {
            Rectangle aRect(rRectangle);

            if(SID_DRAW_ARC == nID || SID_DRAW_CIRCLECUT == nID)
            {
                
                ImpForceQuadratic(aRect);
            }

            pObj->SetLogicRect(aRect);

            SfxItemSet aAttr(pDrDoc->GetItemPool());
            aAttr.Put(SdrCircStartAngleItem(9000));
            aAttr.Put(SdrCircEndAngleItem(0));

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
