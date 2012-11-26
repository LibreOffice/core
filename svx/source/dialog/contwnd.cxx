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
#include "precompiled_svx.hxx"
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

/*************************************************************************
|*
|*
|*
\************************************************************************/

ContourWindow::ContourWindow( Window* pParent, const ResId& rResId ) :
            GraphCtrl       ( pParent, rResId ),
            aWorkRect       ( 0, 0, 0, 0 ),
            bPipetteMode    ( sal_False ),
            bWorkplaceMode  ( sal_False ),
            bClickValid     ( sal_False )
{
    SetWinStyle( WB_SDRMODE );
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

ContourWindow::~ContourWindow()
{
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void ContourWindow::SetPolyPolygon( const PolyPolygon& rPolyPoly )
{
    SdrPage*        pPage = (SdrPage*) pModel->GetPage( 0 );
    const sal_uInt16    nPolyCount = rPolyPoly.Count();

    // zuerst alle Zeichenobjekte loeschen
    aPolyPoly = rPolyPoly;

    // #117412#
    // To avoid to have destroyed objects which are still selected, it is necessary to deselect
    // them first (!)
    pView->UnmarkAllObj();

    if(pPage->GetObjCount())
    {
        pPage->ClearSdrObjList();
        pPage->getSdrModelFromSdrObjList().SetChanged();
    }

    for ( sal_uInt16 i = 0; i < nPolyCount; i++ )
    {
        basegfx::B2DPolyPolygon aPolyPolygon;
        aPolyPolygon.append(aPolyPoly[ i ].getB2DPolygon());
        SdrPathObj* pPathObj = new SdrPathObj( *pModel, aPolyPolygon );

        if ( pPathObj )
        {
            SfxItemSet aSet( pPathObj->GetObjectItemPool() );

            aSet.Put( XFillStyleItem( XFILL_SOLID ) );
            aSet.Put( XFillColorItem( String(), TRANSCOL ) );
            aSet.Put( XFillTransparenceItem( 50 ) );
            pPathObj->SetMergedItemSetAndBroadcast(aSet);
            pPage->InsertObjectToSdrObjList(*pPathObj);
        }
    }

    if ( nPolyCount )
    {
        pView->MarkAll();
        pView->CombineMarkedObjects( sal_False );
    }

    pModel->SetChanged( sal_False );
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

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
            const basegfx::B2DPolyPolygon aB2DPolyPolygon(basegfx::tools::adaptiveSubdivideByAngle(pPathObj->getB2DPolyPolygonInObjectCoordinates()));
            aPolyPoly = PolyPolygon(aB2DPolyPolygon);
        }

        pModel->SetChanged( sal_False );
    }

    return aPolyPoly;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void ContourWindow::InitSdrModel()
{
    GraphCtrl::InitSdrModel();

    SfxItemSet aSet( pModel->GetItemPool() );

    aSet.Put( XFillColorItem( String(), TRANSCOL ) );
    aSet.Put( XFillTransparenceItem( 50 ) );
    pView->SetAttributes( aSet );
    pView->SetFrameHandles(true);
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void ContourWindow::SdrObjCreated( const SdrObject&  )
{
    pView->MarkAll();
    pView->CombineMarkedObjects( sal_False );
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

sal_Bool ContourWindow::IsContourChanged() const
{
    SdrPage*    pPage = (SdrPage*) pModel->GetPage( 0 );
    sal_Bool        bRet = sal_False;

    if ( pPage && pPage->GetObjCount() )
        bRet = ( (SdrPathObj*) pPage->GetObj( 0 ) )->getB2DPolyPolygonInObjectCoordinates().count() && pModel->IsChanged();

    return bRet;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

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


/*************************************************************************
|*
|*
|*
\************************************************************************/

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


/*************************************************************************
|*
|*
|*
\************************************************************************/

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
            pView->SetWorkArea( basegfx::B2DRange( aWorkRect.Left(), aWorkRect.Top(), aWorkRect.Right(), aWorkRect.Bottom() ) );
        }
        else
            pView->SetWorkArea( basegfx::B2DRange( aGraphRect.Left(), aGraphRect.Top(), aGraphRect.Right(), aGraphRect.Bottom() ) );

        Invalidate( aGraphRect );

        if ( aWorkplaceClickLink.IsSet() )
            aWorkplaceClickLink.Call( this );
    }
    else
        GraphCtrl::MouseButtonUp( rMEvt );
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

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

// eof
