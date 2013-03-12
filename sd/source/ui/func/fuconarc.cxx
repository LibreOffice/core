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
#include <svx/svdpagv.hxx>
#include <svx/svdocirc.hxx>
#include <sfx2/request.hxx>
#include <svl/intitem.hxx>
#include <svl/aeitem.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svdobj.hxx>
#include <sfx2/viewfrm.hxx>


#include <svx/svxids.hrc>
#include <math.h>

#include "app.hrc"
#include "Window.hxx"
#include "drawdoc.hxx"
#include "res_bmp.hrc"
#include "View.hxx"
#include "ViewShell.hxx"
#include "ViewShellBase.hxx"
#include "ToolBarManager.hxx"

#include <svx/sxciaitm.hxx>

namespace sd {

TYPEINIT1( FuConstructArc, FuConstruct );


FuConstructArc::FuConstructArc (
    ViewShell*  pViewSh,
    ::sd::Window*       pWin,
    ::sd::View*         pView,
    SdDrawDocument* pDoc,
    SfxRequest&     rReq )
    : FuConstruct( pViewSh, pWin, pView, pDoc, rReq )
{
}

FunctionReference FuConstructArc::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq, bool bPermanent  )
{
    FuConstructArc* pFunc;
    FunctionReference xFunc( pFunc = new FuConstructArc( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    pFunc->SetPermanent(bPermanent);
    return xFunc;
}

void FuConstructArc::DoExecute( SfxRequest& rReq )
{
    FuConstruct::DoExecute( rReq );

    mpViewShell->GetViewShellBase().GetToolBarManager()->SetToolBar(
        ToolBarManager::TBG_FUNCTION,
        ToolBarManager::msDrawingObjectToolBar);

    const SfxItemSet *pArgs = rReq.GetArgs ();

    if (pArgs)
    {
        SFX_REQUEST_ARG (rReq, pCenterX, SfxUInt32Item, ID_VAL_CENTER_X, sal_False);
        SFX_REQUEST_ARG (rReq, pCenterY, SfxUInt32Item, ID_VAL_CENTER_Y, sal_False);
        SFX_REQUEST_ARG (rReq, pAxisX, SfxUInt32Item, ID_VAL_AXIS_X, sal_False);
        SFX_REQUEST_ARG (rReq, pAxisY, SfxUInt32Item, ID_VAL_AXIS_Y, sal_False);
        SFX_REQUEST_ARG (rReq, pPhiStart, SfxUInt32Item, ID_VAL_ANGLESTART, sal_False);
        SFX_REQUEST_ARG (rReq, pPhiEnd, SfxUInt32Item, ID_VAL_ANGLEEND, sal_False);

        Rectangle   aNewRectangle (pCenterX->GetValue () - pAxisX->GetValue () / 2,
                                   pCenterY->GetValue () - pAxisY->GetValue () / 2,
                                   pCenterX->GetValue () + pAxisX->GetValue () / 2,
                                   pCenterY->GetValue () + pAxisY->GetValue () / 2);

        Activate();  // sets aObjKind
        SdrCircObj* pNewCircle =
        new SdrCircObj((SdrObjKind) mpView->GetCurrentObjIdentifier(),
                       aNewRectangle,
                       (long) (pPhiStart->GetValue () * 10.0),
                       (long) (pPhiEnd->GetValue () * 10.0));
        SdrPageView *pPV = mpView->GetSdrPageView();

        mpView->InsertObjectAtView(pNewCircle, *pPV, SDRINSERT_SETDEFLAYER);
    }
}


sal_Bool FuConstructArc::MouseButtonDown( const MouseEvent& rMEvt )
{
    sal_Bool bReturn = FuConstruct::MouseButtonDown( rMEvt );

    if ( rMEvt.IsLeft() && !mpView->IsAction() )
    {
        Point aPnt( mpWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
        mpWindow->CaptureMouse();
        sal_uInt16 nDrgLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );
        mpView->BegCreateObj(aPnt, (OutputDevice*) NULL, nDrgLog);

        SdrObject* pObj = mpView->GetCreateObj();

        if (pObj)
        {
            SfxItemSet aAttr(mpDoc->GetPool());
            SetStyleSheet(aAttr, pObj);

            pObj->SetMergedItemSet(aAttr);
        }

        bReturn = sal_True;
    }
    return bReturn;
}


sal_Bool FuConstructArc::MouseMove( const MouseEvent& rMEvt )
{
    return FuConstruct::MouseMove(rMEvt);
}


sal_Bool FuConstructArc::MouseButtonUp( const MouseEvent& rMEvt )
{
    sal_Bool bReturn = sal_False;
    sal_Bool bCreated = sal_False;

    if ( mpView->IsCreateObj() && rMEvt.IsLeft() )
    {
        sal_uLong nCount = mpView->GetSdrPageView()->GetObjList()->GetObjCount();

        if (mpView->EndCreateObj(SDRCREATE_NEXTPOINT) )
        {
            if (nCount != mpView->GetSdrPageView()->GetObjList()->GetObjCount())
            {
                bCreated = sal_True;
            }
        }

        bReturn = sal_True;
    }

    bReturn = FuConstruct::MouseButtonUp (rMEvt) || bReturn;

    if (!bPermanent && bCreated)
        mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);

    return bReturn;
}

/**
 * Process keyboard input
 * @returns sal_True if a KeyEvent is being processed, sal_False otherwise
 */
sal_Bool FuConstructArc::KeyInput(const KeyEvent& rKEvt)
{
    sal_Bool bReturn = FuConstruct::KeyInput(rKEvt);
    return(bReturn);
}


void FuConstructArc::Activate()
{
    SdrObjKind aObjKind;

    switch( nSlotId )
    {
        case SID_DRAW_ARC      :
        case SID_DRAW_CIRCLEARC:
        {
            aObjKind = OBJ_CARC;
        }
        break;

        case SID_DRAW_PIE             :
        case SID_DRAW_PIE_NOFILL      :
        case SID_DRAW_CIRCLEPIE       :
        case SID_DRAW_CIRCLEPIE_NOFILL:
        {
            aObjKind = OBJ_SECT;
        }
        break;

        case SID_DRAW_ELLIPSECUT       :
        case SID_DRAW_ELLIPSECUT_NOFILL:
        case SID_DRAW_CIRCLECUT        :
        case SID_DRAW_CIRCLECUT_NOFILL :
        {
            aObjKind = OBJ_CCUT;
        }
        break;

        default:
        {
            aObjKind = OBJ_CARC;
        }
        break;
    }

    mpView->SetCurrentObj((sal_uInt16)aObjKind);

    FuConstruct::Activate();
}


void FuConstructArc::Deactivate()
{
    FuConstruct::Deactivate();
}

SdrObject* FuConstructArc::CreateDefaultObject(const sal_uInt16 nID, const Rectangle& rRectangle)
{

    SdrObject* pObj = SdrObjFactory::MakeNewObject(
        mpView->GetCurrentObjInventor(), mpView->GetCurrentObjIdentifier(),
        0L, mpDoc);

    if(pObj)
    {
        if(pObj->ISA(SdrCircObj))
        {
            Rectangle aRect(rRectangle);

            if(SID_DRAW_ARC == nID ||
                SID_DRAW_CIRCLEARC == nID ||
                SID_DRAW_CIRCLEPIE == nID ||
                SID_DRAW_CIRCLEPIE_NOFILL == nID ||
                SID_DRAW_CIRCLECUT == nID ||
                SID_DRAW_CIRCLECUT_NOFILL == nID)
            {
                // force quadratic
                ImpForceQuadratic(aRect);
            }

            pObj->SetLogicRect(aRect);

            SfxItemSet aAttr(mpDoc->GetPool());
            aAttr.Put(SdrCircStartAngleItem(9000));
            aAttr.Put(SdrCircEndAngleItem(0));

            if(SID_DRAW_PIE_NOFILL == nID ||
                SID_DRAW_CIRCLEPIE_NOFILL == nID ||
                SID_DRAW_ELLIPSECUT_NOFILL == nID ||
                SID_DRAW_CIRCLECUT_NOFILL == nID)
            {
                aAttr.Put(XFillStyleItem(XFILL_NONE));
            }

            pObj->SetMergedItemSet(aAttr);
        }
        else
        {
            OSL_FAIL("Object is NO circle object");
        }
    }

    return pObj;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
