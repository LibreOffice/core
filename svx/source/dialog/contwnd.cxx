/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: contwnd.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 13:15:05 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef _XOUTX_HXX //autogen
#include <xoutx.hxx>
#endif
#include <xoutbmp.hxx>
#include <dialogs.hrc>
#include <svxids.hrc>
#include <contdlg.hrc>
#include <contwnd.hxx>
#include "svdpage.hxx"
#include "svdopath.hxx"
#include "xfltrit.hxx"

#ifndef _SVX_FILLITEM_HXX //autogen
#include <xfillit.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#endif

#ifdef MAC
#define TRANSCOL Color( COL_LIGHTGRAY )
#else
#define TRANSCOL Color( COL_WHITE )
#endif

/*************************************************************************
|*
|*
|*
\************************************************************************/

ContourWindow::ContourWindow( Window* pParent, const ResId& rResId ) :
            GraphCtrl       ( pParent, rResId ),
            aWorkRect       ( 0, 0, 0, 0 ),
            bPipetteMode    ( FALSE ),
            bWorkplaceMode  ( FALSE ),
            bClickValid     ( FALSE )
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
    const USHORT    nPolyCount = rPolyPoly.Count();

    // zuerst alle Zeichenobjekte loeschen
    aPolyPoly = rPolyPoly;

    // #117412#
    // To avoid to have destroyed objects which are still selected, it is necessary to deselect
    // them first (!)
    pView->UnmarkAllObj();

    pPage->Clear();

    for ( USHORT i = 0; i < nPolyCount; i++ )
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

            //pPathObj->SetItemSetAndBroadcast(aSet);
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
            const basegfx::B2DPolyPolygon aB2DPolyPolygon(basegfx::tools::adaptiveSubdivideByAngle(pPathObj->GetPathPoly()));
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
    pView->SetFrameDragSingles( TRUE );
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

BOOL ContourWindow::IsContourChanged() const
{
    SdrPage*    pPage = (SdrPage*) pModel->GetPage( 0 );
    BOOL        bRet = FALSE;

    if ( pPage && pPage->GetObjCount() )
        bRet = ( (SdrPathObj*) pPage->GetObj( 0 ) )->GetPathPoly().count() && pModel->IsChanged();

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
        SetEditMode( TRUE );
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
    bClickValid = FALSE;

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


/*************************************************************************
|*
|*
|*
\************************************************************************/

void ContourWindow::Paint( const Rectangle& rRect )
{
    const Graphic& rGraphic = GetGraphic();

    const Color& rOldLineColor = GetLineColor();
    const Color& rOldFillColor = GetFillColor();

    SetLineColor( Color( COL_BLACK ) );
    SetFillColor( Color( COL_WHITE ) );

    DrawRect( Rectangle( Point(), GetGraphicSize() ) );

    SetLineColor( rOldLineColor );
    SetFillColor( rOldFillColor );

    if ( rGraphic.GetType() != GRAPHIC_NONE )
        rGraphic.Draw( this, Point(), GetGraphicSize() );

    if ( aWorkRect.Left() != aWorkRect.Right() && aWorkRect.Top() != aWorkRect.Bottom() )
    {
        PolyPolygon _aPolyPoly( 2, 2 );
        const Color aOldFillColor( GetFillColor() );

        _aPolyPoly.Insert( Rectangle( Point(), GetGraphicSize() ) );
        _aPolyPoly.Insert( aWorkRect );

        SetFillColor( COL_LIGHTRED );
        DrawTransparent( _aPolyPoly, 50 );
        SetFillColor( aOldFillColor );
    }

    pView->CompleteRedraw( this , rRect );
}


