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

#include <fuconpol.hxx>
#include <tabvwsh.hxx>
#include <drawview.hxx>

// Create default drawing objects via keyboard
#include <svx/svdopath.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/point/b2dpoint.hxx>

FuConstPolygon::FuConstPolygon(ScTabViewShell& rViewSh, vcl::Window* pWin, ScDrawView* pViewP,
                               SdrModel* pDoc, const SfxRequest& rReq)
    : FuConstruct(rViewSh, pWin, pViewP, pDoc, rReq)
{
}

FuConstPolygon::~FuConstPolygon()
{
}

bool FuConstPolygon::MouseButtonDown(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    bool bReturn = FuConstruct::MouseButtonDown(rMEvt);

    SdrViewEvent aVEvt;
    (void)pView->PickAnything(rMEvt, SdrMouseEventKind::BUTTONDOWN, aVEvt);
    if (aVEvt.eEvent == SdrEventKind::BeginTextEdit)
    {
        // Text input not allowed here
        aVEvt.eEvent = SdrEventKind::BeginDragObj;
        pView->EnableExtendedMouseEventDispatcher(false);
    }
    else
    {
        pView->EnableExtendedMouseEventDispatcher(true);
    }

    if ( pView->MouseButtonDown(rMEvt, pWindow) )
        bReturn = true;

    return bReturn;
}

bool FuConstPolygon::MouseMove(const MouseEvent& rMEvt)
{
    pView->MouseMove(rMEvt, pWindow);
    return FuConstruct::MouseMove(rMEvt);
}

bool FuConstPolygon::MouseButtonUp(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    bool bReturn = false;
    bool bSimple = false;

    SdrViewEvent aVEvt;
    (void)pView->PickAnything(rMEvt, SdrMouseEventKind::BUTTONUP, aVEvt);

    pView->MouseButtonUp(rMEvt, pWindow);

    if (aVEvt.eEvent == SdrEventKind::EndCreate)
    {
        bReturn = true;
        bSimple = true;         // Do not pass on double-click
    }

    bool bParent;
    if (bSimple)
        bParent = FuConstruct::SimpleMouseButtonUp(rMEvt);
    else
        bParent = FuConstruct::MouseButtonUp(rMEvt);

    return (bParent || bReturn);
}

void FuConstPolygon::Activate()
{
    pView->EnableExtendedMouseEventDispatcher(true);

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

    pView->SetEditMode(SdrViewEditMode::Create);

    FuConstruct::Activate();

    aNewPointer = PointerStyle::DrawPolygon;
    aOldPointer = pWindow->GetPointer();
    rViewShell.SetActivePointer( aNewPointer );
}

void FuConstPolygon::Deactivate()
{
    pView->SetEditMode(SdrViewEditMode::Edit);

    pView->EnableExtendedMouseEventDispatcher(false);

    FuConstruct::Deactivate();

    rViewShell.SetActivePointer( aOldPointer );
}

// Create default drawing objects via keyboard
SdrObjectUniquePtr FuConstPolygon::CreateDefaultObject(const sal_uInt16 nID, const tools::Rectangle& rRectangle)
{
    // case SID_DRAW_XPOLYGON:
    // case SID_DRAW_XPOLYGON_NOFILL:
    // case SID_DRAW_POLYGON:
    // case SID_DRAW_POLYGON_NOFILL:
    // case SID_DRAW_BEZIER_FILL:
    // case SID_DRAW_BEZIER_NOFILL:
    // case SID_DRAW_FREELINE:
    // case SID_DRAW_FREELINE_NOFILL:

    SdrObjectUniquePtr pObj(SdrObjFactory::MakeNewObject(
        *pDrDoc,
        pView->GetCurrentObjInventor(),
        pView->GetCurrentObjIdentifier()));

    if(pObj)
    {
        if(dynamic_cast<const SdrPathObj*>( pObj.get() ) !=  nullptr)
        {
            basegfx::B2DPolyPolygon aPoly;

            switch(nID)
            {
                case SID_DRAW_BEZIER_FILL:
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
                case SID_DRAW_FREELINE:
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
                case SID_DRAW_XPOLYGON:
                case SID_DRAW_XPOLYGON_NOFILL:
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

            static_cast<SdrPathObj*>(pObj.get())->SetPathPoly(aPoly);
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
