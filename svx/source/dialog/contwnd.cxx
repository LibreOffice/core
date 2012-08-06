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

#include <svx/xoutbmp.hxx>
#include <svx/dialogs.hrc>
#include <svx/svxids.hrc>
#include <contdlg.hrc>
#include <contwnd.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdopath.hxx>
#include <svx/xfltrit.hxx>
#include <svx/xfillit.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>

// #i75482#
#include "svx/sdrpaintwindow.hxx"

#define TRANSCOL Color( COL_WHITE )

ContourWindow::ContourWindow( Window* pParent, const ResId& rResId ) :
            GraphCtrl       ( pParent, rResId ),
            aWorkRect       ( 0, 0, 0, 0 ),
            bPipetteMode    ( sal_False ),
            bWorkplaceMode  ( sal_False ),
            bClickValid     ( sal_False )
{
    SetWinStyle( WB_SDRMODE );
}

ContourWindow::~ContourWindow()
{
}

void ContourWindow::SetPolyPolygon( const PolyPolygon& rPolyPoly )
{
    SdrPage*        pPage = (SdrPage*) pModel->GetPage( 0 );
    const sal_uInt16    nPolyCount = rPolyPoly.Count();

    // First delete all drawing objects
    aPolyPoly = rPolyPoly;

    // To avoid to have destroyed objects which are still selected, it is necessary to deselect
    // them first (!)
    pView->UnmarkAllObj();

    pPage->Clear();

    for ( sal_uInt16 i = 0; i < nPolyCount; i++ )
    {
        basegfx::B2DPolyPolygon aPolyPolygon;
        aPolyPolygon.append(aPolyPoly[ i ].getB2DPolygon());
        SdrPathObj* pPathObj = new SdrPathObj( OBJ_PATHFILL, aPolyPolygon );

        if ( pPathObj )
        {
            SfxItemSet aSet( pModel->GetItemPool() );

            aSet.Put( XFillStyleItem( XFILL_SOLID ) );
            aSet.Put( XFillColorItem( String(), TRANSCOL ) );
            aSet.Put( XFillTransparenceItem( 50 ) );

            pPathObj->SetMergedItemSetAndBroadcast(aSet);

            pPage->InsertObject( pPathObj );
        }
    }

    if ( nPolyCount )
    {
        pView->MarkAll();
        pView->CombineMarkedObjects( sal_False );
    }

    pModel->SetChanged( sal_False );
}

const PolyPolygon& ContourWindow::GetPolyPolygon()
{
    if ( pModel->IsChanged() )
    {
        SdrPage* pPage = (SdrPage*) pModel->GetPage( 0 );

        aPolyPoly = PolyPolygon();

        if ( pPage && pPage->GetObjCount() )
        {
            SdrPathObj* pPathObj = (SdrPathObj*)pPage->GetObj(0L);
            // Not sure if subdivision is needed for ContourWindow, but maybe it cannot handle
            // curves at all. Keeping subdivision here for security
            const basegfx::B2DPolyPolygon aB2DPolyPolygon(basegfx::tools::adaptiveSubdivideByAngle(pPathObj->GetPathPoly()));
            aPolyPoly = PolyPolygon(aB2DPolyPolygon);
        }

        pModel->SetChanged( sal_False );
    }

    return aPolyPoly;
}

void ContourWindow::InitSdrModel()
{
    GraphCtrl::InitSdrModel();

    SfxItemSet aSet( pModel->GetItemPool() );

    aSet.Put( XFillColorItem( String(), TRANSCOL ) );
    aSet.Put( XFillTransparenceItem( 50 ) );
    pView->SetAttributes( aSet );
    pView->SetFrameDragSingles( sal_True );
}

void ContourWindow::SdrObjCreated( const SdrObject&  )
{
    pView->MarkAll();
    pView->CombineMarkedObjects( sal_False );
}

sal_Bool ContourWindow::IsContourChanged() const
{
    SdrPage*    pPage = (SdrPage*) pModel->GetPage( 0 );
    sal_Bool        bRet = sal_False;

    if ( pPage && pPage->GetObjCount() )
        bRet = ( (SdrPathObj*) pPage->GetObj( 0 ) )->GetPathPoly().count() && pModel->IsChanged();

    return bRet;
}

void ContourWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( bWorkplaceMode )
    {
        const Point aLogPt( PixelToLogic( rMEvt.GetPosPixel() ) );

        SetPolyPolygon( PolyPolygon() );
        aWorkRect = Rectangle( aLogPt, aLogPt );
        Paint( Rectangle( Point(), GetGraphicSize() ) );
        SetEditMode( sal_True );
    }

    if ( !bPipetteMode )
        GraphCtrl::MouseButtonDown( rMEvt );
}

void ContourWindow::MouseMove( const MouseEvent& rMEvt )
{
    bClickValid = sal_False;

    if ( bPipetteMode )
    {
        const Point aLogPt( PixelToLogic( rMEvt.GetPosPixel() ) );

        aPipetteColor = GetPixel( aLogPt );
        Control::MouseMove( rMEvt );

        if ( aPipetteLink.IsSet() && Rectangle( Point(), GetGraphicSize() ).IsInside( aLogPt ) )
        {
            SetPointer( POINTER_REFHAND );
            aPipetteLink.Call( this );
        }
    }
    else
        GraphCtrl::MouseMove( rMEvt );
}

void ContourWindow::MouseButtonUp(const MouseEvent& rMEvt)
{
    Point aTmpPoint;
    const Rectangle aGraphRect( aTmpPoint, GetGraphicSize() );
    const Point     aLogPt( PixelToLogic( rMEvt.GetPosPixel() ) );

    bClickValid = aGraphRect.IsInside( aLogPt );
    ReleaseMouse();

    if ( bPipetteMode )
    {
        Control::MouseButtonUp( rMEvt );

        if ( aPipetteClickLink.IsSet() )
            aPipetteClickLink.Call( this );
    }
    else if ( bWorkplaceMode )
    {
        GraphCtrl::MouseButtonUp( rMEvt );

        aWorkRect.Right() = aLogPt.X();
        aWorkRect.Bottom() = aLogPt.Y();
        aWorkRect.Intersection( aGraphRect );
        aWorkRect.Justify();

        if ( aWorkRect.Left() != aWorkRect.Right() && aWorkRect.Top() != aWorkRect.Bottom() )
        {
            PolyPolygon _aPolyPoly( GetPolyPolygon() );

            _aPolyPoly.Clip( aWorkRect );
            SetPolyPolygon( _aPolyPoly );
            pView->SetWorkArea( aWorkRect );
        }
        else
            pView->SetWorkArea( aGraphRect );

        Invalidate( aGraphRect );

        if ( aWorkplaceClickLink.IsSet() )
            aWorkplaceClickLink.Call( this );
    }
    else
        GraphCtrl::MouseButtonUp( rMEvt );
}

void ContourWindow::Paint( const Rectangle& rRect )
{
    // #i75482#
    // encapsulate the redraw using Begin/End and use the returned
    // data to get the target output device (e.g. when pre-rendering)
    SdrPaintWindow* pPaintWindow = pView->BeginCompleteRedraw(this);
    OutputDevice& rTarget = pPaintWindow->GetTargetOutputDevice();

    const Graphic& rGraphic = GetGraphic();
    const Color& rOldLineColor = GetLineColor();
    const Color& rOldFillColor = GetFillColor();

    rTarget.SetLineColor( Color( COL_BLACK ) );
    rTarget.SetFillColor( Color( COL_WHITE ) );

    rTarget.DrawRect( Rectangle( Point(), GetGraphicSize() ) );

    rTarget.SetLineColor( rOldLineColor );
    rTarget.SetFillColor( rOldFillColor );

    if ( rGraphic.GetType() != GRAPHIC_NONE )
        rGraphic.Draw( &rTarget, Point(), GetGraphicSize() );

    if ( aWorkRect.Left() != aWorkRect.Right() && aWorkRect.Top() != aWorkRect.Bottom() )
    {
        PolyPolygon _aPolyPoly( 2, 2 );
        const Color aOldFillColor( GetFillColor() );

        _aPolyPoly.Insert( Rectangle( Point(), GetGraphicSize() ) );
        _aPolyPoly.Insert( aWorkRect );

        rTarget.SetFillColor( COL_LIGHTRED );
        rTarget.DrawTransparent( _aPolyPoly, 50 );
        rTarget.SetFillColor( aOldFillColor );
    }

    // #i75482#
    const Region aRepaintRegion(rRect);
    pView->DoCompleteRedraw(*pPaintWindow, aRepaintRegion);
    pView->EndCompleteRedraw(*pPaintWindow, true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
