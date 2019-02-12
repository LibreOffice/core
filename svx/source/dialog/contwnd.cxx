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

#include <svx/xoutbmp.hxx>
#include <svx/svxids.hrc>
#include "contwnd.hxx"
#include <svx/svdpage.hxx>
#include <svx/svdopath.hxx>
#include <svx/xfltrit.hxx>
#include <svx/xfillit.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <vcl/ptrstyle.hxx>

using namespace css;

#define TRANSCOL COL_WHITE

ContourWindow::ContourWindow(vcl::Window* pParent, WinBits nBits)
    : GraphCtrl (pParent, nBits)
    , aWorkRect(0, 0, 0, 0)
    , bPipetteMode(false)
    , bWorkplaceMode(false)
    , bClickValid(false)
{
    SetSdrMode(true);
}

void ContourWindow::SetPolyPolygon(const tools::PolyPolygon& rPolyPoly)
{
    SdrPage* pPage = pModel->GetPage(0);
    const sal_uInt16 nPolyCount = rPolyPoly.Count();

    // First delete all drawing objects
    aPolyPoly = rPolyPoly;

    // To avoid to have destroyed objects which are still selected, it is necessary to deselect
    // them first (!)
    pView->UnmarkAllObj();

    // clear SdrObjects with broadcasting
    pPage->ClearSdrObjList();

    for (sal_uInt16 i = 0; i < nPolyCount; i++)
    {
        basegfx::B2DPolyPolygon aPolyPolygon;
        aPolyPolygon.append(aPolyPoly[ i ].getB2DPolygon());
        SdrPathObj* pPathObj = new SdrPathObj(
            *pModel,
            OBJ_PATHFILL,
            aPolyPolygon);

        SfxItemSet aSet(pModel->GetItemPool());

        aSet.Put(XFillStyleItem(drawing::FillStyle_SOLID));
        aSet.Put(XFillColorItem("", TRANSCOL));
        aSet.Put(XFillTransparenceItem(50) );

        pPathObj->SetMergedItemSetAndBroadcast(aSet);

        pPage->InsertObject( pPathObj );
    }

    if (nPolyCount)
    {
        pView->MarkAll();
        pView->CombineMarkedObjects(false);
    }

    pModel->SetChanged(false);
}

const tools::PolyPolygon& ContourWindow::GetPolyPolygon()
{
    if ( pModel->IsChanged() )
    {
        SdrPage* pPage = pModel->GetPage( 0 );

        aPolyPoly = tools::PolyPolygon();

        if ( pPage && pPage->GetObjCount() )
        {
            SdrPathObj* pPathObj = static_cast<SdrPathObj*>(pPage->GetObj(0));
            // Not sure if subdivision is needed for ContourWindow, but maybe it cannot handle
            // curves at all. Keeping subdivision here for security
            const basegfx::B2DPolyPolygon aB2DPolyPolygon(basegfx::utils::adaptiveSubdivideByAngle(pPathObj->GetPathPoly()));
            aPolyPoly = tools::PolyPolygon(aB2DPolyPolygon);
        }

        pModel->SetChanged( false );
    }

    return aPolyPoly;
}

void ContourWindow::InitSdrModel()
{
    GraphCtrl::InitSdrModel();

    SfxItemSet aSet( pModel->GetItemPool() );

    aSet.Put( XFillColorItem( "", TRANSCOL ) );
    aSet.Put( XFillTransparenceItem( 50 ) );
    pView->SetAttributes( aSet );
    pView->SetFrameDragSingles();
}

void ContourWindow::SdrObjCreated( const SdrObject&  )
{
    pView->MarkAll();
    pView->CombineMarkedObjects( false );
}

bool ContourWindow::IsContourChanged() const
{
    SdrPage*    pPage = pModel->GetPage( 0 );
    bool        bRet = false;

    if ( pPage && pPage->GetObjCount() )
        bRet = static_cast<SdrPathObj*>( pPage->GetObj( 0 ) )->GetPathPoly().count() && pModel->IsChanged();

    return bRet;
}

void ContourWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( bWorkplaceMode )
    {
        const Point aLogPt( PixelToLogic( rMEvt.GetPosPixel() ) );

        SetPolyPolygon( tools::PolyPolygon() );
        aWorkRect = tools::Rectangle( aLogPt, aLogPt );
        Invalidate(tools::Rectangle(Point(), GetGraphicSize()));
        SetEditMode( true );
    }

    if ( !bPipetteMode )
        GraphCtrl::MouseButtonDown( rMEvt );
}

void ContourWindow::MouseMove( const MouseEvent& rMEvt )
{
    bClickValid = false;

    if ( bPipetteMode )
    {
        const Point aLogPt( PixelToLogic( rMEvt.GetPosPixel() ) );

        aPipetteColor = GetPixel( aLogPt );
        Control::MouseMove( rMEvt );

        if ( aPipetteLink.IsSet() && tools::Rectangle( Point(), GetGraphicSize() ).IsInside( aLogPt ) )
        {
            SetPointer( PointerStyle::RefHand );
            aPipetteLink.Call( *this );
        }
    }
    else
        GraphCtrl::MouseMove( rMEvt );
}

void ContourWindow::MouseButtonUp(const MouseEvent& rMEvt)
{
    const tools::Rectangle aGraphRect( Point(), GetGraphicSize() );
    const Point     aLogPt( PixelToLogic( rMEvt.GetPosPixel() ) );

    bClickValid = aGraphRect.IsInside( aLogPt );
    ReleaseMouse();

    if ( bPipetteMode )
    {
        Control::MouseButtonUp( rMEvt );

        aPipetteClickLink.Call( *this );
    }
    else if ( bWorkplaceMode )
    {
        GraphCtrl::MouseButtonUp( rMEvt );

        aWorkRect.SetRight( aLogPt.X() );
        aWorkRect.SetBottom( aLogPt.Y() );
        aWorkRect.Intersection( aGraphRect );
        aWorkRect.Justify();

        if ( aWorkRect.Left() != aWorkRect.Right() && aWorkRect.Top() != aWorkRect.Bottom() )
        {
            tools::PolyPolygon _aPolyPoly( GetPolyPolygon() );

            _aPolyPoly.Clip( aWorkRect );
            SetPolyPolygon( _aPolyPoly );
            pView->SetWorkArea( aWorkRect );
        }
        else
            pView->SetWorkArea( aGraphRect );

        Invalidate( aGraphRect );

        aWorkplaceClickLink.Call( *this );
    }
    else
        GraphCtrl::MouseButtonUp( rMEvt );
}

void ContourWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    // #i75482#
    // encapsulate the redraw using Begin/End and use the returned
    // data to get the target output device (e.g. when pre-rendering)
    SdrPaintWindow* pPaintWindow = pView->BeginCompleteRedraw(&rRenderContext);
    OutputDevice& rTarget = pPaintWindow->GetTargetOutputDevice();

    const Graphic& rGraphic = GetGraphic();
    rTarget.Push(PushFlags::LINECOLOR |PushFlags::FILLCOLOR);
    rTarget.SetLineColor(COL_BLACK);
    rTarget.SetFillColor(COL_WHITE);
    rTarget.DrawRect( tools::Rectangle( Point(), GetGraphicSize() ) );
    rTarget.Pop();

    if (rGraphic.GetType() != GraphicType::NONE)
        rGraphic.Draw(&rTarget, Point(), GetGraphicSize());

    if (aWorkRect.Left() != aWorkRect.Right() && aWorkRect.Top() != aWorkRect.Bottom())
    {
        tools::PolyPolygon _aPolyPoly(2);
        rTarget.Push(PushFlags::FILLCOLOR);
        _aPolyPoly.Insert(tools::Rectangle(Point(), GetGraphicSize()));
        _aPolyPoly.Insert(aWorkRect);
        rTarget.SetFillColor(COL_LIGHTRED);
        rTarget.DrawTransparent(_aPolyPoly, 50);
        rTarget.Pop();
    }

    // #i75482#
    const vcl::Region aRepaintRegion(rRect);
    pView->DoCompleteRedraw(*pPaintWindow, aRepaintRegion);
    pView->EndCompleteRedraw(*pPaintWindow, true);
}

Size ContourWindow::GetOptimalSize() const
{
    return LogicToPixel(Size(270, 170), MapMode(MapUnit::MapAppFont));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
