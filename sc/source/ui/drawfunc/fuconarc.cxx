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

BOOL FuConstArc::MouseButtonDown( const MouseEvent& rMEvt )
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    BOOL bReturn = FuConstruct::MouseButtonDown( rMEvt );

    if ( rMEvt.IsLeft() && !pView->IsAction() )
    {
        Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
        pWindow->CaptureMouse();
        pView->BegCreateObj( aPnt );
        bReturn = TRUE;
    }
    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

BOOL FuConstArc::MouseMove( const MouseEvent& rMEvt )
{
    return FuConstruct::MouseMove(rMEvt);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

BOOL FuConstArc::MouseButtonUp( const MouseEvent& rMEvt )
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    BOOL bReturn = FALSE;

    if ( pView->IsCreateObj() && rMEvt.IsLeft() )
    {
        pView->EndCreateObj( SDRCREATE_NEXTPOINT );
        bReturn = TRUE;
    }
    return (FuConstruct::MouseButtonUp(rMEvt) || bReturn);
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert TRUE, andernfalls
|* FALSE.
|*
\************************************************************************/

BOOL FuConstArc::KeyInput(const KeyEvent& rKEvt)
{
    BOOL bReturn = FuConstruct::KeyInput(rKEvt);
    return(bReturn);
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

    pView->SetCurrentObj( sal::static_int_cast<UINT16>( aObjKind ) );

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
        0L, pDrDoc);

    if(pObj)
    {
        if(pObj->ISA(SdrCircObj))
        {
            Rectangle aRect(rRectangle);

            if(SID_DRAW_ARC == nID || SID_DRAW_CIRCLECUT == nID)
            {
                // force quadratic
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
