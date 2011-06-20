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
#include "precompiled_sd.hxx"

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

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

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

        Activate();  // Setzt aObjKind
        SdrCircObj* pNewCircle =
        new SdrCircObj((SdrObjKind) mpView->GetCurrentObjIdentifier(),
                       aNewRectangle,
                       (long) (pPhiStart->GetValue () * 10.0),
                       (long) (pPhiEnd->GetValue () * 10.0));
        SdrPageView *pPV = mpView->GetSdrPageView();

        mpView->InsertObjectAtView(pNewCircle, *pPV, SDRINSERT_SETDEFLAYER);
    }
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

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

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

sal_Bool FuConstructArc::MouseMove( const MouseEvent& rMEvt )
{
    return FuConstruct::MouseMove(rMEvt);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

sal_Bool FuConstructArc::MouseButtonUp( const MouseEvent& rMEvt )
{
    sal_Bool bReturn = sal_False;
    sal_Bool bCreated = sal_False;

    if ( mpView->IsCreateObj() && rMEvt.IsLeft() )
    {
        Point aPnt( mpWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

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

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert sal_True, andernfalls
|* sal_False.
|*
\************************************************************************/

sal_Bool FuConstructArc::KeyInput(const KeyEvent& rKEvt)
{
    sal_Bool bReturn = FuConstruct::KeyInput(rKEvt);
    return(bReturn);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

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

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

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
