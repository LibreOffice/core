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


#include "fuconpol.hxx"
#include "tabvwsh.hxx"
#include "sc.hrc"
#include "drawview.hxx"

// Create default drawing objects via keyboard
#include <svx/svdopath.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/point/b2dpoint.hxx>

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

sal_Bool FuConstPolygon::MouseButtonDown(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    sal_Bool bReturn = FuConstruct::MouseButtonDown(rMEvt);

    SdrViewEvent aVEvt;
    (void)pView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);
    if (aVEvt.eEvent == SDREVENT_BEGTEXTEDIT)
    {
        // Texteingabe hier nicht zulassen
        aVEvt.eEvent = SDREVENT_BEGDRAGOBJ;
        pView->EnableExtendedMouseEventDispatcher(false);
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

sal_Bool FuConstPolygon::MouseMove(const MouseEvent& rMEvt)
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

sal_Bool FuConstPolygon::MouseButtonUp(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    sal_Bool bReturn = false;
    sal_Bool bSimple = false;

    SdrViewEvent aVEvt;
    (void)pView->PickAnything(rMEvt, SDRMOUSEBUTTONUP, aVEvt);

    pView->MouseButtonUp(rMEvt, pWindow);

    if (aVEvt.eEvent == SDREVENT_ENDCREATE)
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

sal_Bool FuConstPolygon::KeyInput(const KeyEvent& rKEvt)
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

    SdrObjKind eKind;

    switch (GetSlotID())
    {
        case SID_DRAW_POLYGON_NOFILL:
        case SID_DRAW_XPOLYGON_NOFILL:
        {
            eKind = OBJ_PLIN;
        }
        break;

        case SID_DRAW_POLYGON:
        case SID_DRAW_XPOLYGON:
        {
            eKind = OBJ_POLY;
        }
        break;

        case SID_DRAW_BEZIER_NOFILL:
        {
            eKind = OBJ_PATHLINE;
        }
        break;

        case SID_DRAW_BEZIER_FILL:
        {
            eKind = OBJ_PATHFILL;
        }
        break;

        case SID_DRAW_FREELINE_NOFILL:
        {
            eKind = OBJ_FREELINE;
        }
        break;

        case SID_DRAW_FREELINE:
        {
            eKind = OBJ_FREEFILL;
        }
        break;

        default:
        {
            eKind = OBJ_PATHLINE;
        }
        break;
    }

    pView->SetCurrentObj(sal::static_int_cast<sal_uInt16>(eKind));

    pView->SetEditMode(SDREDITMODE_CREATE);

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
    pView->SetEditMode(SDREDITMODE_EDIT);

    pView->EnableExtendedMouseEventDispatcher(false);

    FuConstruct::Deactivate();

    pViewShell->SetActivePointer( aOldPointer );
}

// Create default drawing objects via keyboard
SdrObject* FuConstPolygon::CreateDefaultObject(const sal_uInt16 nID, const Rectangle& rRectangle)
{
    // case SID_DRAW_POLYGON:
    // case SID_DRAW_POLYGON_NOFILL:
    // case SID_DRAW_BEZIER_NOFILL:
    // case SID_DRAW_FREELINE_NOFILL:

    SdrObject* pObj = SdrObjFactory::MakeNewObject(
        pView->GetCurrentObjInventor(), pView->GetCurrentObjIdentifier(),
        0L, pDrDoc);

    if(pObj)
    {
        if(pObj->ISA(SdrPathObj))
        {
            basegfx::B2DPolyPolygon aPoly;

            switch(nID)
            {
                case SID_DRAW_BEZIER_NOFILL:
                {
                    basegfx::B2DPolygon aInnerPoly;

                    aInnerPoly.append(basegfx::B2DPoint(rRectangle.Left(), rRectangle.Bottom()));

                    const basegfx::B2DPoint aCenterBottom(rRectangle.Center().X(), rRectangle.Bottom());
                    aInnerPoly.appendBezierSegment(
                        aCenterBottom,
                        aCenterBottom,
                        basegfx::B2DPoint(rRectangle.Center().X(), rRectangle.Center().Y()));

                    const basegfx::B2DPoint aCenterTop(rRectangle.Center().X(), rRectangle.Top());
                    aInnerPoly.appendBezierSegment(
                        aCenterTop,
                        aCenterTop,
                        basegfx::B2DPoint(rRectangle.Right(), rRectangle.Top()));

                    aPoly.append(aInnerPoly);
                    break;
                }
                case SID_DRAW_FREELINE_NOFILL:
                {
                    basegfx::B2DPolygon aInnerPoly;

                    aInnerPoly.append(basegfx::B2DPoint(rRectangle.Left(), rRectangle.Bottom()));

                    aInnerPoly.appendBezierSegment(
                        basegfx::B2DPoint(rRectangle.Left(), rRectangle.Top()),
                        basegfx::B2DPoint(rRectangle.Center().X(), rRectangle.Top()),
                        basegfx::B2DPoint(rRectangle.Center().X(), rRectangle.Center().Y()));

                    aInnerPoly.appendBezierSegment(
                        basegfx::B2DPoint(rRectangle.Center().X(), rRectangle.Bottom()),
                        basegfx::B2DPoint(rRectangle.Right(), rRectangle.Bottom()),
                        basegfx::B2DPoint(rRectangle.Right(), rRectangle.Top()));

                    aPoly.append(aInnerPoly);
                    break;
                }
                case SID_DRAW_POLYGON:
                case SID_DRAW_POLYGON_NOFILL:
                {
                    basegfx::B2DPolygon aInnerPoly;
                    const sal_Int32 nWdt(rRectangle.GetWidth());
                    const sal_Int32 nHgt(rRectangle.GetHeight());

                    aInnerPoly.append(basegfx::B2DPoint(rRectangle.Left(), rRectangle.Bottom()));
                    aInnerPoly.append(basegfx::B2DPoint(rRectangle.Left() + (nWdt * 30) / 100, rRectangle.Top() + (nHgt * 70) / 100));
                    aInnerPoly.append(basegfx::B2DPoint(rRectangle.Left(), rRectangle.Top() + (nHgt * 15) / 100));
                    aInnerPoly.append(basegfx::B2DPoint(rRectangle.Left() + (nWdt * 65) / 100, rRectangle.Top()));
                    aInnerPoly.append(basegfx::B2DPoint(rRectangle.Left() + nWdt, rRectangle.Top() + (nHgt * 30) / 100));
                    aInnerPoly.append(basegfx::B2DPoint(rRectangle.Left() + (nWdt * 80) / 100, rRectangle.Top() + (nHgt * 50) / 100));
                    aInnerPoly.append(basegfx::B2DPoint(rRectangle.Left() + (nWdt * 80) / 100, rRectangle.Top() + (nHgt * 75) / 100));
                    aInnerPoly.append(basegfx::B2DPoint(rRectangle.Bottom(), rRectangle.Right()));

                    if(SID_DRAW_POLYGON_NOFILL == nID)
                    {
                        aInnerPoly.append(basegfx::B2DPoint(rRectangle.Center().X(), rRectangle.Bottom()));
                    }
                    else
                    {
                        aInnerPoly.setClosed(true);
                    }

                    aPoly.append(aInnerPoly);
                    break;
                }
            }

            ((SdrPathObj*)pObj)->SetPathPoly(aPoly);
        }
        else
        {
            OSL_FAIL("Object is NO path object");
        }

        pObj->SetLogicRect(rRectangle);
    }

    return pObj;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
