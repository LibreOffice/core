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

#include <fuconarc.hxx>
#include <tabvwsh.hxx>
#include <drawview.hxx>

// Create default drawing objects via keyboard
#include <svx/svdocirc.hxx>
#include <svx/sxciaitm.hxx>

FuConstArc::FuConstArc(ScTabViewShell& rViewSh, vcl::Window* pWin, ScDrawView* pViewP,
                       SdrModel* pDoc, const SfxRequest& rReq)
    : FuConstruct(rViewSh, pWin, pViewP, pDoc, rReq)
{
}

FuConstArc::~FuConstArc()
{
}

bool FuConstArc::MouseButtonDown( const MouseEvent& rMEvt )
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    bool bReturn = FuConstruct::MouseButtonDown( rMEvt );

    if ( rMEvt.IsLeft() && !pView->IsAction() )
    {
        Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
        pWindow->CaptureMouse();
        pView->BegCreateObj( aPnt );
        bReturn = true;
    }
    return bReturn;
}

bool FuConstArc::MouseButtonUp( const MouseEvent& rMEvt )
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    bool bReturn = false;

    if ( pView->IsCreateObj() && rMEvt.IsLeft() )
    {
        pView->EndCreateObj( SdrCreateCmd::NextPoint );
        bReturn = true;
    }
    return (FuConstruct::MouseButtonUp(rMEvt) || bReturn);
}

void FuConstArc::Activate()
{
    SdrObjKind aObjKind;

    switch (aSfxRequest.GetSlot() )
    {
        case SID_DRAW_ARC:
            aNewPointer = PointerStyle::DrawArc;
            aObjKind = OBJ_CARC;
            break;

        case SID_DRAW_PIE:
            aNewPointer = PointerStyle::DrawPie;
            aObjKind = OBJ_SECT;
            break;

        case SID_DRAW_CIRCLECUT:
            aNewPointer = PointerStyle::DrawCircleCut;
            aObjKind = OBJ_CCUT;
            break;

        default:
            aNewPointer = PointerStyle::Cross;
            aObjKind = OBJ_CARC;
            break;
    }

    pView->SetCurrentObj( sal::static_int_cast<sal_uInt16>( aObjKind ) );

    aOldPointer = pWindow->GetPointer();
    rViewShell.SetActivePointer( aNewPointer );

    FuDraw::Activate();
}

void FuConstArc::Deactivate()
{
    FuDraw::Deactivate();
    rViewShell.SetActivePointer( aOldPointer );
}

// Create default drawing objects via keyboard
SdrObjectUniquePtr FuConstArc::CreateDefaultObject(const sal_uInt16 nID, const tools::Rectangle& rRectangle)
{
    // case SID_DRAW_ARC:
    // case SID_DRAW_PIE:
    // case SID_DRAW_CIRCLECUT:

    SdrObjectUniquePtr pObj(SdrObjFactory::MakeNewObject(
        *pDrDoc,
        pView->GetCurrentObjInventor(),
        pView->GetCurrentObjIdentifier()));

    if(pObj)
    {
        if(dynamic_cast<const SdrCircObj*>( pObj.get() ) !=  nullptr)
        {
            tools::Rectangle aRect(rRectangle);

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
