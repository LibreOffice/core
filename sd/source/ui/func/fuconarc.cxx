/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
#include <svx/svdlegacy.hxx>
#include <math.h>

#include "app.hrc"
#include "Window.hxx"
#include "drawdoc.hxx"
#include "res_bmp.hrc"
#include "View.hxx"
#include "ViewShell.hxx"
#include "ViewShellBase.hxx"
#include "ToolBarManager.hxx"

namespace sd {

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
        SFX_REQUEST_ARG (rReq, pCenterX, SfxUInt32Item, ID_VAL_CENTER_X );
        SFX_REQUEST_ARG (rReq, pCenterY, SfxUInt32Item, ID_VAL_CENTER_Y );
        SFX_REQUEST_ARG (rReq, pAxisX, SfxUInt32Item, ID_VAL_AXIS_X );
        SFX_REQUEST_ARG (rReq, pAxisY, SfxUInt32Item, ID_VAL_AXIS_Y );
        SFX_REQUEST_ARG (rReq, pPhiStart, SfxUInt32Item, ID_VAL_ANGLESTART );
        SFX_REQUEST_ARG (rReq, pPhiEnd, SfxUInt32Item, ID_VAL_ANGLEEND );

        const sal_Int32 nWidth(pAxisX->GetValue());
        const sal_Int32 nHeight(pAxisY->GetValue());
        const basegfx::B2DHomMatrix aObjTrans(
            basegfx::tools::createScaleTranslateB2DHomMatrix(
                nWidth, nHeight,
                pCenterX->GetValue() - (nWidth / 2), pCenterY->GetValue() - (nHeight / 2)));

        // could not check, found to case to trigger this
        const double fStart((1800 - pPhiStart->GetValue()) * F_PI1800);
        const double fEnd((1800 - pPhiEnd->GetValue()) * F_PI1800);

        // sets aObjKind
        Activate();

        SdrCircObj* pNewCircle = new SdrCircObj(
            *GetDoc(),
            mpView->getSdrObjectCreationInfo().getSdrCircleObjType(),
            aObjTrans,
            fStart,
            fEnd);

        mpView->InsertObjectAtView(*pNewCircle, SDRINSERT_SETDEFLAYER);
    }
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

bool FuConstructArc::MouseButtonDown( const MouseEvent& rMEvt )
{
    bool bReturn = FuConstruct::MouseButtonDown( rMEvt );

    if ( rMEvt.IsLeft() && !mpView->IsAction() )
    {
        const basegfx::B2DPoint aPixelPos(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
        const basegfx::B2DPoint aLogicPos(mpWindow->GetInverseViewTransformation() * aPixelPos);

        mpWindow->CaptureMouse();
        const double fTolerance(basegfx::B2DVector(mpWindow->GetInverseViewTransformation() * basegfx::B2DVector(DRGPIX, 0.0)).getLength());
        mpView->BegCreateObj(aLogicPos, fTolerance);

        SdrObject* pObj = mpView->GetCreateObj();

        if (pObj)
        {
            SfxItemSet aAttr(mpDoc->GetItemPool());
            SetStyleSheet(aAttr, pObj);

            pObj->SetMergedItemSet(aAttr);
        }

        bReturn = true;
    }
    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

bool FuConstructArc::MouseMove( const MouseEvent& rMEvt )
{
    return FuConstruct::MouseMove(rMEvt);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

bool FuConstructArc::MouseButtonUp( const MouseEvent& rMEvt )
{
    bool bReturn = false;
    bool bCreated = false;

    if(mpView->GetCreateObj() && rMEvt.IsLeft() && mpView->GetSdrPageView())
    {
        Point aPnt( mpWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
        sal_uInt32 nCount = mpView->GetSdrPageView()->GetCurrentObjectList()->GetObjCount();

        if (mpView->EndCreateObj(SDRCREATE_NEXTPOINT) )
        {
            if (nCount != mpView->GetSdrPageView()->GetCurrentObjectList()->GetObjCount())
            {
                bCreated = true;
            }
        }

        bReturn = true;
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
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert true, andernfalls
|* false.
|*
\************************************************************************/

bool FuConstructArc::KeyInput(const KeyEvent& rKEvt)
{
    bool bReturn = FuConstruct::KeyInput(rKEvt);
    return(bReturn);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuConstructArc::Activate()
{
    SdrCircleObjType aSdrCircleObjType(CircleType_Circle);

    switch( nSlotId )
    {
        case SID_DRAW_ARC      :
        case SID_DRAW_CIRCLEARC:
        {
            aSdrCircleObjType = CircleType_Arc;
        }
        break;

        case SID_DRAW_PIE             :
        case SID_DRAW_PIE_NOFILL      :
        case SID_DRAW_CIRCLEPIE       :
        case SID_DRAW_CIRCLEPIE_NOFILL:
        {
            aSdrCircleObjType = CircleType_Sector;
        }
        break;

        case SID_DRAW_ELLIPSECUT       :
        case SID_DRAW_ELLIPSECUT_NOFILL:
        case SID_DRAW_CIRCLECUT        :
        case SID_DRAW_CIRCLECUT_NOFILL :
        {
            aSdrCircleObjType = CircleType_Segment;
        }
        break;

        default:
        {
            aSdrCircleObjType = CircleType_Arc;
        }
        break;
    }

    SdrObjectCreationInfo aSdrObjectCreationInfo(static_cast< sal_uInt16 >(OBJ_CIRC));

    aSdrObjectCreationInfo.setSdrCircleObjType(aSdrCircleObjType);
    mpView->setSdrObjectCreationInfo(aSdrObjectCreationInfo);

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
//  FuDraw::Deactivate();
}

// #97016#
SdrObject* FuConstructArc::CreateDefaultObject(const sal_uInt16 nID, const basegfx::B2DRange& rRange)
{
    // case SID_DRAW_ARC:
    // case SID_DRAW_CIRCLEARC:
    // case SID_DRAW_PIE:
    // case SID_DRAW_PIE_NOFILL:
    // case SID_DRAW_CIRCLEPIE:
    // case SID_DRAW_CIRCLEPIE_NOFILL:
    // case SID_DRAW_ELLIPSECUT:
    // case SID_DRAW_ELLIPSECUT_NOFILL:
    // case SID_DRAW_CIRCLECUT:
    // case SID_DRAW_CIRCLECUT_NOFILL:

    SdrObject* pObj = SdrObjFactory::MakeNewObject(
        mpView->getSdrModelFromSdrView(),
        mpView->getSdrObjectCreationInfo());

    if(pObj)
    {
        SdrCircObj* pSdrCircObj = dynamic_cast< SdrCircObj* >(pObj);

        if(pSdrCircObj)
        {
            basegfx::B2DRange aRange(rRange);

            if(SID_DRAW_ARC == nID ||
                SID_DRAW_CIRCLEARC == nID ||
                SID_DRAW_CIRCLEPIE == nID ||
                SID_DRAW_CIRCLEPIE_NOFILL == nID ||
                SID_DRAW_CIRCLECUT == nID ||
                SID_DRAW_CIRCLECUT_NOFILL == nID)
            {
                // force quadratic
                ImpForceQuadratic(aRange);
            }

            sdr::legacy::SetLogicRange(*pSdrCircObj, aRange);

            if(SID_DRAW_ARC != nID || SID_DRAW_CIRCLEARC != nID)
            {
                // no full circle, set angles to gat a 3/4 circle
                pSdrCircObj->SetStartAngle(M_PI + M_PI_2); // was: 9000
                pSdrCircObj->SetEndAngle(0.0); // was: 0
            }

            if(SID_DRAW_PIE_NOFILL == nID ||
                SID_DRAW_CIRCLEPIE_NOFILL == nID ||
                SID_DRAW_ELLIPSECUT_NOFILL == nID ||
                SID_DRAW_CIRCLECUT_NOFILL == nID)
            {
                pSdrCircObj->SetMergedItem(XFillStyleItem(XFILL_NONE));
            }
        }
        else
        {
            DBG_ERROR("Object is NO circle object");
        }
    }

    return pObj;
}

} // end of namespace sd
