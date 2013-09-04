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

#include <vcl/wrkwin.hxx>
#include <sot/factory.hxx>
#include <tools/shl.hxx>
#include <tools/helpers.hxx>
#include <vcl/msgbox.hxx>
#include <svl/eitem.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svtools/miscopt.hxx>
#include <unotools/localedatawrapper.hxx>
#include <comphelper/processfactory.hxx>

#include <svx/dialmgr.hxx>
#include "svx/xoutbmp.hxx"
#include <svx/dialogs.hrc>
#include <svx/svxids.hrc>
#include <svx/contdlg.hxx>
#include "contimp.hxx"
#include "contdlg.hrc"
#include "contwnd.hxx"
#include <svx/svdtrans.hxx>
#include <svx/svdopath.hxx>
#include "svx/dlgutil.hxx"
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include "dlgunit.hxx"

SFX_IMPL_FLOATINGWINDOW_WITHID( SvxContourDlgChildWindow, SID_CONTOUR_DLG );


/******************************************************************************/

SvxContourDlgItem::SvxContourDlgItem( sal_uInt16 _nId, SvxSuperContourDlg& rContourDlg, SfxBindings& rBindings ) :
            SfxControllerItem   ( _nId, rBindings ),
            rDlg                ( rContourDlg )
{
}

void SvxContourDlgItem::StateChanged( sal_uInt16 nSID, SfxItemState /*eState*/, const SfxPoolItem* pItem )
{
    if ( pItem && ( SID_CONTOUR_EXEC == nSID ) )
    {
        const SfxBoolItem* pStateItem = PTR_CAST( SfxBoolItem, pItem );

        DBG_ASSERT( pStateItem || pItem == 0, "SfxBoolItem expected ");

        rDlg.SetExecState( !pStateItem->GetValue() );
    }
}


/******************************************************************************/

SvxContourDlgChildWindow::SvxContourDlgChildWindow( Window* _pParent, sal_uInt16 nId,
                                                    SfxBindings* pBindings, SfxChildWinInfo* pInfo ) :
            SfxChildWindow( _pParent, nId )
{
    SvxSuperContourDlg* pDlg = new SvxSuperContourDlg( pBindings, this, _pParent, CONT_RESID( RID_SVXDLG_CONTOUR ) );
    pWindow = pDlg;

    if ( pInfo->nFlags & SFX_CHILDWIN_ZOOMIN )
        pDlg->RollUp();

    eChildAlignment = SFX_ALIGN_NOALIGNMENT;

    pDlg->Initialize( pInfo );
}

SvxContourDlg::SvxContourDlg( SfxBindings* _pBindings, SfxChildWindow* pCW,
                              Window* _pParent, const ResId& rResId ) :

    SfxFloatingWindow   ( _pBindings, pCW, _pParent, rResId )

{
}

SvxContourDlg::~SvxContourDlg()
{
}

PolyPolygon SvxContourDlg::CreateAutoContour( const Graphic& rGraphic,
                                              const Rectangle* pRect,
                                              const sal_uIntPtr nFlags )
{
    Bitmap  aBmp;
    sal_uIntPtr nContourFlags = XOUTBMP_CONTOUR_HORZ;

    if ( rGraphic.GetType() == GRAPHIC_BITMAP )
    {
        if( rGraphic.IsAnimated() )
        {
            VirtualDevice       aVDev;
            MapMode             aTransMap;
            const Animation     aAnim( rGraphic.GetAnimation() );
            const Size&         rSizePix = aAnim.GetDisplaySizePixel();
            const sal_uInt16        nCount = aAnim.Count();

            if ( aVDev.SetOutputSizePixel( rSizePix ) )
            {
                aVDev.SetLineColor( Color( COL_BLACK ) );
                aVDev.SetFillColor( Color( COL_BLACK ) );

                for( sal_uInt16 i = 0; i < nCount; i++ )
                {
                    const AnimationBitmap& rStepBmp = aAnim.Get( i );

                    // Push Polygon output to the right place; this is the
                    // offset of the sub-image within the total animation
                    aTransMap.SetOrigin( Point( rStepBmp.aPosPix.X(), rStepBmp.aPosPix.Y() ) );
                    aVDev.SetMapMode( aTransMap );
                    aVDev.DrawPolyPolygon( CreateAutoContour( rStepBmp.aBmpEx, pRect, nFlags ) );
                }

                aTransMap.SetOrigin( Point() );
                aVDev.SetMapMode( aTransMap );
                aBmp = aVDev.GetBitmap( Point(), rSizePix );
                aBmp.Convert( BMP_CONVERSION_1BIT_THRESHOLD );
            }
        }
        else if( rGraphic.IsTransparent() )
            aBmp = rGraphic.GetBitmapEx().GetMask();
        else
        {
            aBmp = rGraphic.GetBitmap();
            nContourFlags |= XOUTBMP_CONTOUR_EDGEDETECT;
        }
    }
    else if( rGraphic.GetType() != GRAPHIC_NONE )
    {
        const Graphic   aTmpGrf( rGraphic.GetGDIMetaFile().GetMonochromeMtf( Color( COL_BLACK ) ) );
        VirtualDevice   aVDev;
        Size            aSizePix( aVDev.LogicToPixel( aTmpGrf.GetPrefSize(), aTmpGrf.GetPrefMapMode() ) );

        if( aSizePix.Width() && aSizePix.Height() && ( aSizePix.Width() > 512 || aSizePix.Height() > 512 ) )
        {
            double fWH = (double) aSizePix.Width() / aSizePix.Height();

            if( fWH <= 1.0 )
                aSizePix.Width() = FRound( ( aSizePix.Height() = 512 ) * fWH );
            else
                aSizePix.Height() = FRound( ( aSizePix.Width() = 512 ) / fWH );
        }

        if( aVDev.SetOutputSizePixel( aSizePix ) )
        {
            const Point aPt;
            aTmpGrf.Draw( &aVDev, aPt, aSizePix );
            aBmp = aVDev.GetBitmap( aPt, aSizePix );
        }

        nContourFlags |= XOUTBMP_CONTOUR_EDGEDETECT;
    }

    aBmp.SetPrefSize( rGraphic.GetPrefSize() );
    aBmp.SetPrefMapMode( rGraphic.GetPrefMapMode() );

    return PolyPolygon( XOutBitmap::GetCountour( aBmp, nContourFlags, 128, pRect ) );
}

// Loop through to super class, no virtual Methods to not become incompatible
// due to IF changes

const Graphic& SvxContourDlg::GetGraphic() const
{
    return pSuperClass->GetGraphic();
}

sal_Bool SvxContourDlg::IsGraphicChanged() const
{
    return pSuperClass->IsGraphicChanged();
}

PolyPolygon SvxContourDlg::GetPolyPolygon()
{
    return pSuperClass->GetPolyPolygon( true );
}

const void* SvxContourDlg::GetEditingObject() const
{
    return pSuperClass->GetEditingObject();
}

void SvxContourDlg::Update( const Graphic& rGraphic, sal_Bool bGraphicLinked,
                            const PolyPolygon* pPolyPoly, void* pEditingObj )
{
    pSuperClass->UpdateGraphic( rGraphic, bGraphicLinked, pPolyPoly, pEditingObj );
}

SvxSuperContourDlg::SvxSuperContourDlg( SfxBindings *_pBindings, SfxChildWindow *pCW,
                                        Window* _pParent, const ResId& rResId ) :
        SvxContourDlg       ( _pBindings, pCW, _pParent, rResId ),
        pCheckObj           ( NULL ),
        aContourItem        ( SID_CONTOUR_EXEC, *this, *_pBindings ),
        aTbx1               ( this, ResId( TBX1, *rResId.GetResMgr() ) ),
        aMtfTolerance       ( this, ResId( MTF_TOLERANCE, *rResId.GetResMgr() ) ),
        aContourWnd         ( this, ResId( CTL_CONTOUR, *rResId.GetResMgr() ) ),
        aStbStatus          ( this, WB_BORDER | WB_3DLOOK | WB_LEFT ),
        nGrfChanged         ( 0UL ),
        bExecState          ( sal_False ),
        bGraphicLinked      ( sal_False ),
        maImageList         ( SVX_RES( CD_IMAPDLG ) )
{
    ApplyImageList();

    FreeResource();

    SvxContourDlg::SetSuperClass( *this );

    aContourWnd.SetMousePosLink( LINK( this, SvxSuperContourDlg, MousePosHdl ) );
    aContourWnd.SetGraphSizeLink( LINK( this, SvxSuperContourDlg, GraphSizeHdl ) );
    aContourWnd.SetUpdateLink( LINK( this, SvxSuperContourDlg, StateHdl ) );
    aContourWnd.SetPipetteHdl( LINK( this, SvxSuperContourDlg, PipetteHdl ) );
    aContourWnd.SetPipetteClickHdl( LINK( this, SvxSuperContourDlg, PipetteClickHdl ) );
    aContourWnd.SetWorkplaceClickHdl( LINK( this, SvxSuperContourDlg, WorkplaceClickHdl ) );

    const Size      aTbxSize( aTbx1.CalcWindowSizePixel() );
    Point           aPos( aTbx1.GetPosPixel() );
      SvtMiscOptions  aMiscOptions;

    aMiscOptions.AddListenerLink( LINK( this, SvxSuperContourDlg, MiscHdl ) );

    aTbx1.SetOutStyle( aMiscOptions.GetToolboxStyle() );
    aTbx1.SetSizePixel( aTbxSize );
    aTbx1.SetSelectHdl( LINK( this, SvxSuperContourDlg, Tbx1ClickHdl ) );

    aPos.X() += aTbxSize.Width() + LogicToPixel( Size( 3, 0 ), MapMode( MAP_APPFONT ) ).Width();
    aMtfTolerance.SetPosPixel( aPos );
    aMtfTolerance.SetValue( 10L );

    SetMinOutputSizePixel( aLastSize = GetOutputSizePixel() );

    aStbStatus.InsertItem( 1, 130, SIB_LEFT | SIB_IN | SIB_AUTOSIZE );
    aStbStatus.InsertItem( 2, 10 + GetTextWidth( OUString(" 9999,99 cm / 9999,99 cm ") ), SIB_CENTER | SIB_IN );
    aStbStatus.InsertItem( 3, 10 + GetTextWidth( OUString(" 9999,99 cm x 9999,99 cm ") ), SIB_CENTER | SIB_IN );
    aStbStatus.InsertItem( 4, 20, SIB_CENTER | SIB_IN );

    Resize();

    aUpdateTimer.SetTimeout( 100 );
    aUpdateTimer.SetTimeoutHdl( LINK( this, SvxSuperContourDlg, UpdateHdl ) );

    aCreateTimer.SetTimeout( 50 );
    aCreateTimer.SetTimeoutHdl( LINK( this, SvxSuperContourDlg, CreateHdl ) );
}

SvxSuperContourDlg::~SvxSuperContourDlg()
{
    SvtMiscOptions aMiscOptions;
    aMiscOptions.RemoveListenerLink( LINK(this, SvxSuperContourDlg, MiscHdl) );
}

// Resize methods

void SvxSuperContourDlg::Resize()
{
    SfxFloatingWindow::Resize();

    Size aMinSize( GetMinOutputSizePixel() );
    Size aNewSize( GetOutputSizePixel() );

    if ( aNewSize.Height() >= aMinSize.Height() )
    {
        Size    _aSize( aStbStatus.GetSizePixel() );
        Point   aPoint( 0, aNewSize.Height() - _aSize.Height() );

        // Position the StatusBar
        aStbStatus.SetPosSizePixel( aPoint, Size( aNewSize.Width(), _aSize.Height() ) );
        aStbStatus.Show();

        // Position the EditWindow
        _aSize.Width() = aNewSize.Width() - 18;
        _aSize.Height() = aPoint.Y() - aContourWnd.GetPosPixel().Y() - 6;
        aContourWnd.SetSizePixel( _aSize );

        aLastSize = aNewSize;
    }
}

// Close methods

sal_Bool SvxSuperContourDlg::Close()
{
    sal_Bool bRet = sal_True;

    if ( aTbx1.IsItemEnabled( TBI_APPLY ) )
    {
        QueryBox    aQBox( this, WB_YES_NO_CANCEL | WB_DEF_YES,
                           CONT_RESID(STR_CONTOURDLG_MODIFY).toString() );
        const long  nRet = aQBox.Execute();

        if ( nRet == RET_YES )
        {
            SfxBoolItem aBoolItem( SID_CONTOUR_EXEC, sal_True );
            GetBindings().GetDispatcher()->Execute(
                SID_CONTOUR_EXEC, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD, &aBoolItem, 0L );
        }
        else if ( nRet == RET_CANCEL )
            bRet = sal_False;
    }

    return( bRet ? SfxFloatingWindow::Close() : sal_False );
}

// Enabled or disabled all Controls

void SvxSuperContourDlg::SetExecState( sal_Bool bEnable )
{
    bExecState = bEnable;
}

void SvxSuperContourDlg::SetGraphic( const Graphic& rGraphic )
{
    aUndoGraphic = aRedoGraphic = Graphic();
    aGraphic = rGraphic;
    nGrfChanged = 0UL;
    aContourWnd.SetGraphic( aGraphic );
}

void SvxSuperContourDlg::SetPolyPolygon( const PolyPolygon& rPolyPoly )
{
    DBG_ASSERT(  aContourWnd.GetGraphic().GetType() != GRAPHIC_NONE, "Graphic must've been set first!" );

    PolyPolygon     aPolyPoly( rPolyPoly );
    const MapMode   aMap100( MAP_100TH_MM );
    const MapMode   aGrfMap( aGraphic.GetPrefMapMode() );
    OutputDevice*   pOutDev = Application::GetDefaultDevice();
    sal_Bool            bPixelMap = aGrfMap.GetMapUnit() == MAP_PIXEL;

    for ( sal_uInt16 j = 0, nPolyCount = aPolyPoly.Count(); j < nPolyCount; j++ )
    {
        Polygon& rPoly = aPolyPoly[ j ];

        for ( sal_uInt16 i = 0, nCount = rPoly.GetSize(); i < nCount; i++ )
        {
            Point& rPt = rPoly[ i ];

            if ( !bPixelMap )
                rPt = pOutDev->LogicToPixel( rPt, aGrfMap );

            rPt = pOutDev->PixelToLogic( rPt, aMap100 );
        }
    }

    aContourWnd.SetPolyPolygon( aPolyPoly );
    aContourWnd.GetSdrModel()->SetChanged( sal_True );
}

PolyPolygon SvxSuperContourDlg::GetPolyPolygon( bool bRescaleToGraphic )
{
    PolyPolygon aRetPolyPoly( aContourWnd.GetPolyPolygon() );

    if ( bRescaleToGraphic )
    {
        const MapMode   aMap100( MAP_100TH_MM );
        const MapMode   aGrfMap( aGraphic.GetPrefMapMode() );
        OutputDevice*   pOutDev = Application::GetDefaultDevice();
        sal_Bool            bPixelMap = aGrfMap.GetMapUnit() == MAP_PIXEL;

        for ( sal_uInt16 j = 0, nPolyCount = aRetPolyPoly.Count(); j < nPolyCount; j++ )
        {
            Polygon& rPoly = aRetPolyPoly[ j ];

            for ( sal_uInt16 i = 0, nCount = rPoly.GetSize(); i < nCount; i++ )
            {
                Point& rPt = rPoly[ i ];

                rPt = pOutDev->LogicToPixel( rPt, aMap100  );

                if ( !bPixelMap )
                    rPt = pOutDev->PixelToLogic( rPt, aGrfMap  );
            }
        }
    }

    return aRetPolyPoly;
}

void SvxSuperContourDlg::UpdateGraphic( const Graphic& rGraphic, sal_Bool _bGraphicLinked,
                                 const PolyPolygon* pPolyPoly, void* pEditingObj )
{
    aUpdateGraphic = rGraphic;
    bUpdateGraphicLinked = _bGraphicLinked;
    pUpdateEditingObject = pEditingObj;

    if ( pPolyPoly )
        aUpdatePolyPoly = *pPolyPoly;
    else
        aUpdatePolyPoly = PolyPolygon();

    aUpdateTimer.Start();
}

bool SvxSuperContourDlg::IsUndoPossible() const
{
    return aUndoGraphic.GetType() != GRAPHIC_NONE;
}

bool SvxSuperContourDlg::IsRedoPossible() const
{
    return aRedoGraphic.GetType() != GRAPHIC_NONE;
}

// Click handler for ToolBox

IMPL_LINK( SvxSuperContourDlg, Tbx1ClickHdl, ToolBox*, pTbx )
{
    sal_uInt16 nNewItemId = pTbx->GetCurItemId();

    switch( pTbx->GetCurItemId() )
    {
        case( TBI_APPLY ):
        {
            SfxBoolItem aBoolItem( SID_CONTOUR_EXEC, sal_True );
            GetBindings().GetDispatcher()->Execute(
                SID_CONTOUR_EXEC, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aBoolItem, 0L );
        }
        break;

        case( TBI_WORKPLACE ):
        {
            if ( aTbx1.IsItemChecked( TBI_WORKPLACE ) )
            {
                QueryBox aQBox( this, WB_YES_NO | WB_DEF_NO, CONT_RESID(STR_CONTOURDLG_WORKPLACE).toString() );

                if ( !aContourWnd.IsContourChanged() || ( aQBox.Execute() == RET_YES ) )
                    aContourWnd.SetWorkplaceMode( sal_True );
                else
                    aTbx1.CheckItem( TBI_WORKPLACE, sal_False );
            }
            else
                aContourWnd.SetWorkplaceMode( sal_False );
        }
        break;

        case( TBI_SELECT ):
        {
            pTbx->CheckItem( nNewItemId, sal_True );
            aContourWnd.SetEditMode( sal_True );
        }
        break;

        case( TBI_RECT ):
        {
            pTbx->CheckItem( nNewItemId, sal_True );
            aContourWnd.SetObjKind( OBJ_RECT );
        }
        break;

        case( TBI_CIRCLE ):
        {
            pTbx->CheckItem( nNewItemId, sal_True );
            aContourWnd.SetObjKind( OBJ_CIRC );

        }
        break;

        case( TBI_POLY ):
        {
            pTbx->CheckItem( nNewItemId, sal_True );
            aContourWnd.SetObjKind( OBJ_POLY );
        }
        break;

        case( TBI_FREEPOLY ):
        {
            pTbx->CheckItem( nNewItemId, sal_True );
            aContourWnd.SetObjKind( OBJ_FREEFILL );
        }
        break;

        case( TBI_POLYEDIT ):
            aContourWnd.SetPolyEditMode( pTbx->IsItemChecked( TBI_POLYEDIT ) ? SID_BEZIER_MOVE : 0 );
        break;

        case( TBI_POLYMOVE ):
            aContourWnd.SetPolyEditMode( SID_BEZIER_MOVE );
        break;

        case( TBI_POLYINSERT ):
            aContourWnd.SetPolyEditMode( SID_BEZIER_INSERT );
        break;

        case( TBI_POLYDELETE ):
            aContourWnd.GetSdrView()->DeleteMarkedPoints();
        break;

        case( TBI_UNDO ):
        {
            nGrfChanged = nGrfChanged ? nGrfChanged - 1 : 0UL;
            aRedoGraphic = aGraphic;
            aGraphic = aUndoGraphic;
            aUndoGraphic = Graphic();
            aContourWnd.SetGraphic( aGraphic, sal_False );
        }
        break;

        case( TBI_REDO ):
        {
            nGrfChanged++;
            aUndoGraphic = aGraphic;
            aGraphic = aRedoGraphic;
            aRedoGraphic = Graphic();
            aContourWnd.SetGraphic( aGraphic, sal_False );
        }
        break;

        case( TBI_AUTOCONTOUR ):
            aCreateTimer.Start();
        break;

        case( TBI_PIPETTE ):
        {
            sal_Bool bPipette = aTbx1.IsItemChecked( TBI_PIPETTE );

            if ( !bPipette )
                aStbStatus.Invalidate();
            else if ( bGraphicLinked )
            {
                QueryBox aQBox( this, WB_YES_NO | WB_DEF_YES, CONT_RESID(STR_CONTOURDLG_LINKED).toString() );

                if ( aQBox.Execute() != RET_YES )
                {
                    aTbx1.CheckItem( TBI_PIPETTE, bPipette = sal_False );
                    aStbStatus.Invalidate();
                }
            }

            aContourWnd.SetPipetteMode( bPipette );
        }
        break;

        default:
        break;
    }

    return 0L;
}

IMPL_LINK( SvxSuperContourDlg, MousePosHdl, ContourWindow*, pWnd )
{
    OUString aStr;
    const FieldUnit eFieldUnit = GetBindings().GetDispatcher()->GetModule()->GetFieldUnit();
    const Point& rMousePos = pWnd->GetMousePos();
    const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
    const sal_Unicode cSep = rLocaleWrapper.getNumDecimalSep()[0];

    aStr = GetUnitString( rMousePos.X(), eFieldUnit, cSep );
    aStr += " / ";
    aStr += GetUnitString( rMousePos.Y(), eFieldUnit, cSep );

    aStbStatus.SetItemText( 2, aStr );

    return 0L;
}

IMPL_LINK( SvxSuperContourDlg, GraphSizeHdl, ContourWindow*, pWnd )
{
    OUString aStr;
    const FieldUnit eFieldUnit = GetBindings().GetDispatcher()->GetModule()->GetFieldUnit();
    const Size& rSize = pWnd->GetGraphicSize();
    const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
    const sal_Unicode cSep = rLocaleWrapper.getNumDecimalSep()[0];

    aStr = GetUnitString( rSize.Width(), eFieldUnit, cSep );
    aStr += " x ";
    aStr += GetUnitString( rSize.Height(), eFieldUnit, cSep );

    aStbStatus.SetItemText( 3, aStr );

    return 0L;
}

IMPL_LINK_NOARG(SvxSuperContourDlg, UpdateHdl)
{
    aUpdateTimer.Stop();

    if ( pUpdateEditingObject != pCheckObj )
    {
        if( !GetEditingObject() )
            aContourWnd.GrabFocus();

        SetGraphic( aUpdateGraphic );
        SetPolyPolygon( aUpdatePolyPoly );
        SetEditingObject( pUpdateEditingObject );
        bGraphicLinked = bUpdateGraphicLinked;

        aUpdateGraphic = Graphic();
        aUpdatePolyPoly = PolyPolygon();
        bUpdateGraphicLinked = sal_False;

        aContourWnd.GetSdrModel()->SetChanged( sal_False );
    }

    GetBindings().Invalidate( SID_CONTOUR_EXEC );

    return 0L;
}

IMPL_LINK_NOARG(SvxSuperContourDlg, CreateHdl)
{
    aCreateTimer.Stop();

    const Rectangle aWorkRect = aContourWnd.LogicToPixel( aContourWnd.GetWorkRect(), MapMode( MAP_100TH_MM ) );
    const Graphic&  rGraphic = aContourWnd.GetGraphic();
    const sal_Bool      bValid = aWorkRect.Left() != aWorkRect.Right() && aWorkRect.Top() != aWorkRect.Bottom();

    EnterWait();
    SetPolyPolygon( CreateAutoContour( rGraphic, bValid ? &aWorkRect : NULL ) );
    LeaveWait();

    return 0L;
}

IMPL_LINK( SvxSuperContourDlg, StateHdl, ContourWindow*, pWnd )
{
    const SdrObject*    pObj = pWnd->GetSelectedSdrObject();
    const SdrView*      pView = pWnd->GetSdrView();
    const sal_Bool          bPolyEdit = ( pObj != NULL ) && pObj->ISA( SdrPathObj );
    const sal_Bool          bDrawEnabled = !( bPolyEdit && aTbx1.IsItemChecked( TBI_POLYEDIT ) );
    const sal_Bool          bPipette = aTbx1.IsItemChecked( TBI_PIPETTE );
    const sal_Bool          bWorkplace = aTbx1.IsItemChecked( TBI_WORKPLACE );
    const sal_Bool          bDontHide = !( bPipette || bWorkplace );
    const sal_Bool          bBitmap = pWnd->GetGraphic().GetType() == GRAPHIC_BITMAP;

    aTbx1.EnableItem( TBI_APPLY, bDontHide && bExecState && pWnd->IsChanged() );

    aTbx1.EnableItem( TBI_WORKPLACE, !bPipette && bDrawEnabled );

    aTbx1.EnableItem( TBI_SELECT, bDontHide && bDrawEnabled );
    aTbx1.EnableItem( TBI_RECT, bDontHide && bDrawEnabled );
    aTbx1.EnableItem( TBI_CIRCLE, bDontHide && bDrawEnabled );
    aTbx1.EnableItem( TBI_POLY, bDontHide && bDrawEnabled );
    aTbx1.EnableItem( TBI_FREEPOLY, bDontHide && bDrawEnabled );

    aTbx1.EnableItem( TBI_POLYEDIT, bDontHide && bPolyEdit );
    aTbx1.EnableItem( TBI_POLYMOVE, bDontHide && !bDrawEnabled );
    aTbx1.EnableItem( TBI_POLYINSERT, bDontHide && !bDrawEnabled );
    aTbx1.EnableItem( TBI_POLYDELETE, bDontHide && !bDrawEnabled && pView->IsDeleteMarkedPointsPossible() );

    aTbx1.EnableItem( TBI_AUTOCONTOUR, bDontHide && bDrawEnabled );
    aTbx1.EnableItem( TBI_PIPETTE, !bWorkplace && bDrawEnabled && bBitmap );

    aTbx1.EnableItem( TBI_UNDO, bDontHide && IsUndoPossible() );
    aTbx1.EnableItem( TBI_REDO, bDontHide && IsRedoPossible() );

    if ( bPolyEdit )
    {
        sal_uInt16 nId = 0;

        switch( pWnd->GetPolyEditMode() )
        {
            case( SID_BEZIER_MOVE ): nId = TBI_POLYMOVE; break;
            case( SID_BEZIER_INSERT ): nId = TBI_POLYINSERT; break;

            default:
            break;
        }

        aTbx1.CheckItem( nId, sal_True );
    }
    else
    {
        aTbx1.CheckItem( TBI_POLYEDIT, sal_False );
        aTbx1.CheckItem( TBI_POLYMOVE, sal_True );
        aTbx1.CheckItem( TBI_POLYINSERT, sal_False );
        pWnd->SetPolyEditMode( 0 );
    }

    return 0L;
}

IMPL_LINK( SvxSuperContourDlg, PipetteHdl, ContourWindow*, pWnd )
{
    const Color& rOldLineColor = aStbStatus.GetLineColor();
    const Color& rOldFillColor = aStbStatus.GetFillColor();

    Rectangle       aRect( aStbStatus.GetItemRect( 4 ) );
    const Color&    rColor = pWnd->GetPipetteColor();

    aStbStatus.SetLineColor( rColor );
    aStbStatus.SetFillColor( rColor );

    aRect.Left() += 4;
    aRect.Top() += 4;
    aRect.Right() -= 4;
    aRect.Bottom() -= 4;

    aStbStatus.DrawRect( aRect );

    aStbStatus.SetLineColor( rOldLineColor );
    aStbStatus.SetFillColor( rOldFillColor );

    return 0L;
}

IMPL_LINK( SvxSuperContourDlg, PipetteClickHdl, ContourWindow*, pWnd )
{
    if ( pWnd->IsClickValid() )
    {
        Bitmap          aMask;
        const Color&    rColor = pWnd->GetPipetteColor();

        EnterWait();

        if( aGraphic.GetType() == GRAPHIC_BITMAP )
        {
            Bitmap      aBmp( aGraphic.GetBitmap() );
            const long  nTol = static_cast<long>(aMtfTolerance.GetValue() * 255L / 100L);

            aMask = aBmp.CreateMask( rColor, nTol );

            if( aGraphic.IsTransparent() )
                aMask.CombineSimple( aGraphic.GetBitmapEx().GetMask(), BMP_COMBINE_OR );

            if( !!aMask )
            {
                QueryBox    aQBox( this, WB_YES_NO | WB_DEF_YES, CONT_RESID(STR_CONTOURDLG_NEWPIPETTE).toString() );
                sal_Bool        bNewContour;

                aRedoGraphic = Graphic();
                aUndoGraphic = aGraphic;
                aGraphic = Graphic( BitmapEx( aBmp, aMask ) );
                nGrfChanged++;

                bNewContour = ( aQBox.Execute() == RET_YES );
                pWnd->SetGraphic( aGraphic, bNewContour );

                if( bNewContour )
                    aCreateTimer.Start();
            }
        }

        LeaveWait();
    }

    aTbx1.CheckItem( TBI_PIPETTE, sal_False );
    pWnd->SetPipetteMode( sal_False );
    aStbStatus.Invalidate();

    return 0L;
}

IMPL_LINK( SvxSuperContourDlg, WorkplaceClickHdl, ContourWindow*, pWnd )
{
    aTbx1.CheckItem( TBI_WORKPLACE, sal_False );
    aTbx1.CheckItem( TBI_SELECT, sal_True );
    pWnd->SetWorkplaceMode( sal_False );

    return 0L;
}

void SvxSuperContourDlg::ApplyImageList()
{
    ImageList& rImgLst = maImageList;

    aTbx1.SetImageList( rImgLst );
}

void SvxSuperContourDlg::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxFloatingWindow::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
            ApplyImageList();
}

IMPL_LINK_NOARG(SvxSuperContourDlg, MiscHdl)
{
       SvtMiscOptions aMiscOptions;
    aTbx1.SetOutStyle( aMiscOptions.GetToolboxStyle() );

    return 0L;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
