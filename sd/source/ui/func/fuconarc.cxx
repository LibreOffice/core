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

#include <app.hrc>
#include <Window.hxx>
#include <drawdoc.hxx>

#include <View.hxx>
#include <ViewShell.hxx>
#include <ViewShellBase.hxx>
#include <ToolBarManager.hxx>

#include <svx/sxciaitm.hxx>

using namespace com::sun::star;

namespace sd {


FuConstructArc::FuConstructArc (
    ViewShell*  pViewSh,
    ::sd::Window*       pWin,
    ::sd::View*         pView,
    SdDrawDocument* pDoc,
    SfxRequest&     rReq )
    : FuConstruct( pViewSh, pWin, pView, pDoc, rReq )
{
}

rtl::Reference<FuPoor> FuConstructArc::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq, bool bPermanent  )
{
    FuConstructArc* pFunc;
    rtl::Reference<FuPoor> xFunc( pFunc = new FuConstructArc( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    pFunc->SetPermanent(bPermanent);
    return xFunc;
}

void FuConstructArc::DoExecute( SfxRequest& rReq )
{
    FuConstruct::DoExecute( rReq );

    mpViewShell->GetViewShellBase().GetToolBarManager()->SetToolBar(
        ToolBarManager::ToolBarGroup::Function,
        ToolBarManager::msDrawingObjectToolBar);

    const SfxItemSet *pArgs = rReq.GetArgs ();

    if (!pArgs)
        return;

    const SfxUInt32Item* pCenterX = rReq.GetArg<SfxUInt32Item>(ID_VAL_CENTER_X);
    const SfxUInt32Item* pCenterY = rReq.GetArg<SfxUInt32Item>(ID_VAL_CENTER_Y);
    const SfxUInt32Item* pAxisX = rReq.GetArg<SfxUInt32Item>(ID_VAL_AXIS_X);
    const SfxUInt32Item* pAxisY = rReq.GetArg<SfxUInt32Item>(ID_VAL_AXIS_Y);
    const SfxUInt32Item* pPhiStart = rReq.GetArg<SfxUInt32Item>(ID_VAL_ANGLESTART);
    const SfxUInt32Item* pPhiEnd = rReq.GetArg<SfxUInt32Item>(ID_VAL_ANGLEEND);

    ::tools::Rectangle   aNewRectangle (pCenterX->GetValue () - pAxisX->GetValue () / 2,
                               pCenterY->GetValue () - pAxisY->GetValue () / 2,
                               pCenterX->GetValue () + pAxisX->GetValue () / 2,
                               pCenterY->GetValue () + pAxisY->GetValue () / 2);

    Activate();  // sets aObjKind
    SdrCircObj* pNewCircle =
    new SdrCircObj(
        mpView->getSdrModelFromSdrView(),
        static_cast<SdrObjKind>(mpView->GetCurrentObjIdentifier()),
        aNewRectangle,
        static_cast<long>(pPhiStart->GetValue () * 10.0),
        static_cast<long>(pPhiEnd->GetValue () * 10.0));
    SdrPageView *pPV = mpView->GetSdrPageView();

    mpView->InsertObjectAtView(pNewCircle, *pPV, SdrInsertFlags::SETDEFLAYER);
}

bool FuConstructArc::MouseButtonDown( const MouseEvent& rMEvt )
{
    bool bReturn = FuConstruct::MouseButtonDown( rMEvt );

    if ( rMEvt.IsLeft() && !mpView->IsAction() )
    {
        Point aPnt( mpWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
        mpWindow->CaptureMouse();
        sal_uInt16 nDrgLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );
        mpView->BegCreateObj(aPnt, nullptr, nDrgLog);

        SdrObject* pObj = mpView->GetCreateObj();

        if (pObj)
        {
            SfxItemSet aAttr(mpDoc->GetPool());
            SetStyleSheet(aAttr, pObj);

            pObj->SetMergedItemSet(aAttr);
        }

        bReturn = true;
    }
    return bReturn;
}

bool FuConstructArc::MouseButtonUp( const MouseEvent& rMEvt )
{
    bool bReturn = false;
    bool bCreated = false;

    if ( mpView->IsCreateObj() && rMEvt.IsLeft() )
    {
        const size_t nCount = mpView->GetSdrPageView()->GetObjList()->GetObjCount();

        if (mpView->EndCreateObj(SdrCreateCmd::NextPoint) )
        {
            if (nCount != mpView->GetSdrPageView()->GetObjList()->GetObjCount())
            {
                bCreated = true;
            }
        }

        bReturn = true;
    }

    bReturn = FuConstruct::MouseButtonUp (rMEvt) || bReturn;

    if (!bPermanent && bCreated)
        mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);

    return bReturn;
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

    mpView->SetCurrentObj(static_cast<sal_uInt16>(aObjKind));

    FuConstruct::Activate();
}

SdrObjectUniquePtr FuConstructArc::CreateDefaultObject(const sal_uInt16 nID, const ::tools::Rectangle& rRectangle)
{

    SdrObjectUniquePtr pObj(SdrObjFactory::MakeNewObject(
        mpView->getSdrModelFromSdrView(),
        mpView->GetCurrentObjInventor(),
        mpView->GetCurrentObjIdentifier()));

    if(pObj)
    {
        if( dynamic_cast< const SdrCircObj *>( pObj.get() ) !=  nullptr)
        {
            ::tools::Rectangle aRect(rRectangle);

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
            aAttr.Put(makeSdrCircStartAngleItem(9000));
            aAttr.Put(makeSdrCircEndAngleItem(0));

            if(SID_DRAW_PIE_NOFILL == nID ||
                SID_DRAW_CIRCLEPIE_NOFILL == nID ||
                SID_DRAW_ELLIPSECUT_NOFILL == nID ||
                SID_DRAW_CIRCLECUT_NOFILL == nID)
            {
                aAttr.Put(XFillStyleItem(drawing::FillStyle_NONE));
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
