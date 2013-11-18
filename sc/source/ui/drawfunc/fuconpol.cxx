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

#include "fuconpol.hxx"
#include "tabvwsh.hxx"
#include "sc.hrc"
#include "drawview.hxx"
#include <svx/svdopath.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <svx/svdlegacy.hxx>
#include <svx/sdrobjecttools.hxx>

//  Pixelabstand zum Schliessen von Freihand-Zeichnungen
#ifndef CLOSE_PIXDIST
#define CLOSE_PIXDIST 5
#endif

//------------------------------------------------------------------------

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuConstPolygon::FuConstPolygon(ScTabViewShell* pViewSh, Window* pWin, ScDrawView* pViewP,
                   SdrModel* pDoc, SfxRequest& rReq)
    : FuConstruct(pViewSh, pWin, pViewP, pDoc, rReq)
{
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuConstPolygon::~FuConstPolygon()
{
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

sal_Bool __EXPORT FuConstPolygon::MouseButtonDown(const MouseEvent& rMEvt)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    sal_Bool bReturn = FuConstruct::MouseButtonDown(rMEvt);

    SdrViewEvent aVEvt;
    (void)pView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);
    if (aVEvt.meEvent == SDREVENT_BEGTEXTEDIT)
    {
        // Texteingabe hier nicht zulassen
        aVEvt.meEvent = SDREVENT_BEGDRAGOBJ;
        pView->EnableExtendedMouseEventDispatcher(sal_False);
    }
    else
    {
        pView->EnableExtendedMouseEventDispatcher(sal_True);
    }

    if ( pView->MouseButtonDown(rMEvt, pWindow) )
        bReturn = sal_True;

    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

sal_Bool __EXPORT FuConstPolygon::MouseMove(const MouseEvent& rMEvt)
{
    pView->MouseMove(rMEvt, pWindow);
    sal_Bool bReturn = FuConstruct::MouseMove(rMEvt);
    return bReturn;
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

sal_Bool __EXPORT FuConstPolygon::MouseButtonUp(const MouseEvent& rMEvt)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    sal_Bool bReturn = sal_False;
    sal_Bool bSimple = sal_False;

    SdrViewEvent aVEvt;
    (void)pView->PickAnything(rMEvt, SDRMOUSEBUTTONUP, aVEvt);

    pView->MouseButtonUp(rMEvt, pWindow);

    if (aVEvt.meEvent == SDREVENT_ENDCREATE)
    {
        bReturn = sal_True;
        bSimple = sal_True;         // Doppelklick nicht weiterreichen
    }

    sal_Bool bParent;
    if (bSimple)
        bParent = FuConstruct::SimpleMouseButtonUp(rMEvt);
    else
        bParent = FuConstruct::MouseButtonUp(rMEvt);

    return (bParent || bReturn);
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert sal_True, andernfalls
|* FALSE.
|*
\************************************************************************/

sal_Bool __EXPORT FuConstPolygon::KeyInput(const KeyEvent& rKEvt)
{
    sal_Bool bReturn = FuConstruct::KeyInput(rKEvt);

    return(bReturn);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuConstPolygon::Activate()
{
    pView->EnableExtendedMouseEventDispatcher(sal_True);
    SdrObjectCreationInfo aSdrObjectCreationInfo;

    switch (GetSlotID())
    {
        case SID_DRAW_POLYGON_NOFILL:
        case SID_DRAW_XPOLYGON_NOFILL:
        {
            aSdrObjectCreationInfo.setSdrPathObjType(PathType_OpenPolygon);
        }
        break;

        case SID_DRAW_POLYGON:
        case SID_DRAW_XPOLYGON:
        {
            aSdrObjectCreationInfo.setSdrPathObjType(PathType_ClosedPolygon);
        }
        break;

        case SID_DRAW_BEZIER_NOFILL:
        {
            aSdrObjectCreationInfo.setSdrPathObjType(PathType_OpenBezier);
        }
        break;

        case SID_DRAW_BEZIER_FILL:
        {
            aSdrObjectCreationInfo.setSdrPathObjType(PathType_ClosedBezier);
        }
        break;

        case SID_DRAW_FREELINE_NOFILL:
        {
            aSdrObjectCreationInfo.setSdrPathObjType(PathType_OpenBezier);
            aSdrObjectCreationInfo.setFreehandMode(true);
        }
        break;

        case SID_DRAW_FREELINE:
        {
            aSdrObjectCreationInfo.setSdrPathObjType(PathType_ClosedBezier);
            aSdrObjectCreationInfo.setFreehandMode(true);
        }
        break;

        default:
        {
            aSdrObjectCreationInfo.setSdrPathObjType(PathType_OpenBezier);
        }
        break;
    }

    pView->setSdrObjectCreationInfo(aSdrObjectCreationInfo);
    pView->SetViewEditMode(SDREDITMODE_CREATE);
    FuConstruct::Activate();
    aNewPointer = Pointer( POINTER_DRAW_POLYGON );
    aOldPointer = pWindow->GetPointer();
    pViewShell->SetActivePointer( aNewPointer );
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuConstPolygon::Deactivate()
{
    pView->SetViewEditMode(SDREDITMODE_EDIT);

    pView->EnableExtendedMouseEventDispatcher(sal_False);

    FuConstruct::Deactivate();

    pViewShell->SetActivePointer( aOldPointer );
}

// #98185# Create default drawing objects via keyboard
SdrObject* FuConstPolygon::CreateDefaultObject(const sal_uInt16 nID, const basegfx::B2DRange& rRange)
{
    SdrPathObj* pRetval = dynamic_cast< SdrPathObj* >(
        SdrObjFactory::MakeNewObject(
            pView->getSdrModelFromSdrView(),
            pView->getSdrObjectCreationInfo()));

    if(pRetval)
    {
        DefaultSdrPathObjType eType(DefaultSdrPathObjType_Line);
        bool bClosed(false);

        switch(nID)
        {
            case SID_DRAW_BEZIER_NOFILL:
                eType = DefaultSdrPathObjType_Bezier;
                break;
            case SID_DRAW_FREELINE_NOFILL:
                eType = DefaultSdrPathObjType_Freeline;
                break;
            case SID_DRAW_POLYGON:
                eType = DefaultSdrPathObjType_Polygon;
                bClosed = true;
                break;
            case SID_DRAW_POLYGON_NOFILL:
                eType = DefaultSdrPathObjType_Polygon;
                break;
        }

        initializeDefaultSdrPathObjByObjectType(*pRetval, eType, rRange, bClosed);
    }
    else
    {
        OSL_ENSURE(false, "Could not create default path object (!)");
    }

    return pRetval;
}

// eof
