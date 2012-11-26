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
#include "precompiled_sc.hxx"

#include "fuconarc.hxx"
#include "sc.hrc"
#include "tabvwsh.hxx"
#include "drawview.hxx"
#include <svx/svdocirc.hxx>
#include <svx/svdlegacy.hxx>

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

sal_Bool __EXPORT FuConstArc::MouseButtonDown( const MouseEvent& rMEvt )
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    sal_Bool bReturn = FuConstruct::MouseButtonDown( rMEvt );

    if ( rMEvt.IsLeft() && !pView->IsAction() )
    {
        const basegfx::B2DPoint aPixelPos(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
        const basegfx::B2DPoint aLogicPos(pWindow->GetInverseViewTransformation() * aPixelPos);

        pWindow->CaptureMouse();
        pView->BegCreateObj( aLogicPos );
        bReturn = sal_True;
    }
    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

sal_Bool __EXPORT FuConstArc::MouseMove( const MouseEvent& rMEvt )
{
    return FuConstruct::MouseMove(rMEvt);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

sal_Bool __EXPORT FuConstArc::MouseButtonUp( const MouseEvent& rMEvt )
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    sal_Bool bReturn = sal_False;

    if ( pView->GetCreateObj() && rMEvt.IsLeft() )
    {
        // Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
        pView->EndCreateObj( SDRCREATE_NEXTPOINT );
        bReturn = sal_True;
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

sal_Bool __EXPORT FuConstArc::KeyInput(const KeyEvent& rKEvt)
{
    sal_Bool bReturn = FuConstruct::KeyInput(rKEvt);
    return(bReturn);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuConstArc::Activate()
{
    SdrCircleObjType aSdrCircleObjType(CircleType_Circle);

    switch (aSfxRequest.GetSlot() )
    {
        case SID_DRAW_ARC:
            aNewPointer = Pointer( POINTER_DRAW_ARC );
            aSdrCircleObjType = CircleType_Arc;
            break;

        case SID_DRAW_PIE:
            aNewPointer = Pointer( POINTER_DRAW_PIE );
            aSdrCircleObjType = CircleType_Sector;
            break;

        case SID_DRAW_CIRCLECUT:
            aNewPointer = Pointer( POINTER_DRAW_CIRCLECUT );
            aSdrCircleObjType = CircleType_Segment;
            break;

        default:
            aNewPointer = Pointer( POINTER_CROSS );
            aSdrCircleObjType = CircleType_Arc;
            break;
    }

    SdrObjectCreationInfo aSdrObjectCreationInfo(static_cast< sal_uInt16 >(OBJ_CIRC));

    aSdrObjectCreationInfo.setSdrCircleObjType(aSdrCircleObjType);
    pView->setSdrObjectCreationInfo(aSdrObjectCreationInfo);

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

// #98185# Create default drawing objects via keyboard
SdrObject* FuConstArc::CreateDefaultObject(const sal_uInt16 nID, const basegfx::B2DRange& rRange)
{
    // case SID_DRAW_ARC:
    // case SID_DRAW_PIE:
    // case SID_DRAW_CIRCLECUT:

    SdrObject* pObj = SdrObjFactory::MakeNewObject(
        pView->getSdrModelFromSdrView(),
        pView->getSdrObjectCreationInfo());

    if(pObj)
    {
        SdrCircObj* pSdrCircObj = dynamic_cast< SdrCircObj* >(pObj);

        if(pSdrCircObj)
        {
            basegfx::B2DRange aRange(rRange);

            if(SID_DRAW_ARC == nID || SID_DRAW_CIRCLECUT == nID)
            {
                // force quadratic
                ImpForceQuadratic(aRange);
            }

            sdr::legacy::SetLogicRange(*pSdrCircObj, aRange);
            pSdrCircObj->SetStartAngle(F_PI2 * 3.0); // TTTT formally 9000, needs check (mirroring?)
            pSdrCircObj->SetEndAngle(0.0);
        }
        else
        {
            DBG_ERROR("Object is NO circle object");
        }
    }

    return pObj;
}


