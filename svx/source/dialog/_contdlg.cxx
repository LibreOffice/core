/*************************************************************************
 *
 *  $RCSfile: _contdlg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pb $ $Date: 2000-10-09 11:38:45 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SOT_FACTORY_HXX
#include <sot/factory.hxx>
#endif
#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _SV_SALBTYPE_HXX
#include <vcl/salbtype.hxx>     // FRound
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#include <sfx2/viewfrm.hxx>
#pragma hdrstop

#define _SDR_NOITEMS
#define _SDR_NOTOUCH
#define _SDR_NOTRANSFORM
#define _SDR_NOOBJECTS
#define _SDR_NOVIEWMARKER
#define _SDR_NODRAGMETHODS
#define _SDR_NOUNDO
#define _SDR_NOXOUTDEV
#define _XOUTBMP_STATICS_ONLY

#include "dialmgr.hxx"
#include "xoutbmp.hxx"
#include "dialogs.hrc"
#include "svxids.hrc"
#include "contdlg.hxx"
#include "contimp.hxx"
#include "contdlg.hrc"
#include "contwnd.hxx"
#include "svdtrans.hxx"
#include "svdopath.hxx"
#include "dlgutil.hxx"

/******************************************************************************/

inline String GetUnitString( long nVal_100, FieldUnit eFieldUnit, sal_Unicode cSep )
{
    String aVal = UniString::CreateFromInt32( MetricField::ConvertValue( nVal_100, 2, MAP_100TH_MM, eFieldUnit ) );

    while( aVal.Len() < 3 )
        aVal.Insert( sal_Unicode('0'), 0 );

    aVal.Insert( cSep, aVal.Len() - 2 );
    aVal += sal_Unicode(' ');
    aVal += SdrFormatter::GetUnitStr( eFieldUnit );

    return aVal;
}


/******************************************************************************/


SFX_IMPL_FLOATINGWINDOW( SvxContourDlgChildWindow, SID_CONTOUR_DLG );


/******************************************************************************/


/*************************************************************************
|*
|* ControllerItem
|*
\************************************************************************/

SvxContourDlgItem::SvxContourDlgItem( USHORT nId, SvxSuperContourDlg& rContourDlg, SfxBindings& rBindings ) :
            SfxControllerItem   ( nId, rBindings ),
            rDlg                ( rContourDlg )
{
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void SvxContourDlgItem::StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pItem )
{
    if ( pItem && ( SID_CONTOUR_EXEC == nSID ) )
    {
        const SfxBoolItem* pStateItem = PTR_CAST( SfxBoolItem, pItem );

        DBG_ASSERT( pStateItem || pItem == 0, "SfxBoolItem erwartet");

        rDlg.SetExecState( !pStateItem->GetValue() );
    }
}


/******************************************************************************/


/*************************************************************************
|*
|* Contour-Float
|*
\************************************************************************/

SvxContourDlgChildWindow::SvxContourDlgChildWindow( Window* pParent, USHORT nId,
                                                    SfxBindings* pBindings, SfxChildWinInfo* pInfo ) :
            SfxChildWindow( pParent, nId )
{
    Point aPt;
    pWindow = new SvxSuperContourDlg( pBindings, this, pParent, CONT_RESID( RID_SVXDLG_CONTOUR ) );

    if ( pInfo->aSize.Width() && pInfo->aSize.Height() )
        pWindow->SetPosSizePixel( pInfo->aPos, pInfo->aSize );
    else
        pWindow->SetPosPixel( aPt );

    if ( pInfo->nFlags & SFX_CHILDWIN_ZOOMIN )
        ( (SvxSuperContourDlg*) pWindow )->RollUp();

    eChildAlignment = SFX_ALIGN_NOALIGNMENT;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

void SvxContourDlgChildWindow::UpdateContourDlg( const Graphic& rGraphic, BOOL bGraphicLinked,
                                                 const PolyPolygon* pPolyPoly, void* pEditingObj )
{
    if ( SfxViewFrame::Current() &&
         SfxViewFrame::Current()->HasChildWindow( SvxContourDlgChildWindow::GetChildWindowId() ) )
        SVXCONTOURDLG()->Update( rGraphic, bGraphicLinked, pPolyPoly, pEditingObj );
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

SvxContourDlg::SvxContourDlg( SfxBindings* pBindings, SfxChildWindow* pCW,
                              Window* pParent, const ResId& rResId ) :

    SfxFloatingWindow   ( pBindings, pCW, pParent, rResId )

{
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

SvxContourDlg::~SvxContourDlg()
{
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

PolyPolygon SvxContourDlg::CreateAutoContour( const Graphic& rGraphic,
                                              const Rectangle* pRect,
                                              const ULONG nFlags )
{
    Bitmap  aBmp;
    ULONG   nContourFlags = XOUTBMP_CONTOUR_HORZ;

    if ( rGraphic.GetType() == GRAPHIC_BITMAP )
    {
        if( rGraphic.IsAnimated() )
        {
            VirtualDevice       aVDev;
            MapMode             aTransMap;
            PolyPolygon         aAnimPolyPoly;
            const Animation     aAnim( rGraphic.GetAnimation() );
            const Size&         rSizePix = aAnim.GetDisplaySizePixel();
            const USHORT        nCount = aAnim.Count();

            if ( aVDev.SetOutputSizePixel( rSizePix ) )
            {
                aVDev.SetLineColor( Color( COL_BLACK ) );
                aVDev.SetFillColor( Color( COL_BLACK ) );

                for( USHORT i = 0; i < nCount; i++ )
                {
                    const AnimationBitmap& rStepBmp = aAnim.Get( i );

                    // Polygonausgabe an die richtige Stelle schieben;
                    // dies ist der Offset des Teilbildes innerhalb
                    // der Gesamtanimation
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
        VirtualDevice   aVDev;
        const MapMode   aMap( rGraphic.GetPrefMapMode() );
        const Size      aPrefSize( rGraphic.GetPrefSize() );
        const Point     aNullPt;

        if ( aVDev.SetOutputSizePixel( aVDev.LogicToPixel( Rectangle( aNullPt, aPrefSize ), aMap ).GetSize() ) )
        {
            rGraphic.Draw( &aVDev, aNullPt, aVDev.LogicToPixel( aPrefSize, aMap ) );
            aBmp = aVDev.GetBitmap( aNullPt, aVDev.GetOutputSizePixel() );
            nContourFlags |= XOUTBMP_CONTOUR_EDGEDETECT;
        }
    }

    aBmp.SetPrefSize( rGraphic.GetPrefSize() );
    aBmp.SetPrefMapMode( rGraphic.GetPrefMapMode() );

    return PolyPolygon( XOutBitmap::GetCountour( aBmp, nContourFlags, 128, pRect ) );
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void SvxContourDlg::ScaleContour( PolyPolygon& rContour, const Graphic& rGraphic,
                                  const MapUnit eUnit, const Size& rDisplaySize )
{
    DBG_ASSERT( rGraphic.GetType() != GRAPHIC_NONE, "Graphic is not valid!" );

    OutputDevice*   pOutDev = Application::GetDefaultDevice();
    const MapMode   aDispMap( eUnit );
    const MapMode   aGrfMap( rGraphic.GetPrefMapMode() );
    const Size      aGrfSize( rGraphic.GetPrefSize() );
    double          fScaleX;
    double          fScaleY;
    Size            aOrgSize;
    Point           aNewPoint;
    BOOL            bPixelMap = aGrfMap.GetMapUnit() == MAP_PIXEL;

    if ( bPixelMap )
        aOrgSize = pOutDev->PixelToLogic( aGrfSize, aDispMap );
    else
        aOrgSize = pOutDev->LogicToLogic( aGrfSize, aGrfMap, aDispMap );

    if ( aOrgSize.Width() && aOrgSize.Height() )
    {
        fScaleX = (double) rDisplaySize.Width() / aOrgSize.Width();
        fScaleY = (double) rDisplaySize.Height() / aOrgSize.Height();

        for ( USHORT j = 0, nPolyCount = rContour.Count(); j < nPolyCount; j++ )
        {
            Polygon& rPoly = rContour[ j ];

            for ( USHORT i = 0, nCount = rPoly.GetSize(); i < nCount; i++ )
            {
                if ( bPixelMap )
                    aNewPoint = pOutDev->PixelToLogic( rPoly[ i ], aDispMap  );
                else
                    aNewPoint = pOutDev->LogicToLogic( rPoly[ i ], aGrfMap, aDispMap  );

                rPoly[ i ] = Point( FRound( aNewPoint.X() * fScaleX ), FRound( aNewPoint.Y() * fScaleY ) );
            }
        }
    }
}


/*************************************************************************
|*
|* Durchschleifen an SuperClass; keine virt. Methoden, um
|* bei IF-Aenderungen nicht inkompatibel zu werden
|*
\************************************************************************/

void SvxContourDlg::SetExecState( BOOL bEnable )
{
    pSuperClass->SetExecState( bEnable );
}

void SvxContourDlg::SetGraphic( const Graphic& rGraphic )
{
    pSuperClass->SetGraphic( rGraphic );
}

void SvxContourDlg::SetGraphicLinked( BOOL bGraphicLinked )
{
    pSuperClass->SetGraphicLinked( bGraphicLinked );
}

const Graphic& SvxContourDlg::GetGraphic() const
{
    return pSuperClass->GetGraphic();
}

BOOL SvxContourDlg::IsGraphicChanged() const
{
    return pSuperClass->IsGraphicChanged();
}

void SvxContourDlg::SetPolyPolygon( const PolyPolygon& rPolyPoly )
{
    pSuperClass->SetPolyPolygon( rPolyPoly );
}

PolyPolygon SvxContourDlg::GetPolyPolygon()
{
    return pSuperClass->GetPolyPolygon( TRUE );
}

void SvxContourDlg::SetEditingObject( void* pObj )
{
    pSuperClass->SetEditingObject( pObj );
}

const void* SvxContourDlg::GetEditingObject() const
{
    return pSuperClass->GetEditingObject();
}

void SvxContourDlg::Update( const Graphic& rGraphic, BOOL bGraphicLinked,
                            const PolyPolygon* pPolyPoly, void* pEditingObj )
{
    pSuperClass->Update( rGraphic, bGraphicLinked, pPolyPoly, pEditingObj );
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

SvxSuperContourDlg::SvxSuperContourDlg( SfxBindings *pBindings, SfxChildWindow *pCW,
                                        Window* pParent, const ResId& rResId ) :
        SvxContourDlg       ( pBindings, pCW, pParent, rResId ),
        aContourItem        ( SID_CONTOUR_EXEC, *this, *pBindings ),
        aTbx1               ( this, ResId( TBX1 ) ),
        aMtfTolerance       ( this, ResId( MTF_TOLERANCE ) ),
        aContourWnd         ( this, ResId( CTL_CONTOUR) ),
        aStbStatus          ( this, WB_BORDER | WB_3DLOOK | WB_LEFT ),
        nGrfChanged         ( 0UL ),
        pCheckObj           ( NULL ),
        bExecState          ( FALSE ),
        bGraphicLinked      ( FALSE )
{
    FreeResource();

    SvxContourDlg::SetSuperClass( *this );

    aContourWnd.SetMousePosLink( LINK( this, SvxSuperContourDlg, MousePosHdl ) );
    aContourWnd.SetGraphSizeLink( LINK( this, SvxSuperContourDlg, GraphSizeHdl ) );
    aContourWnd.SetUpdateLink( LINK( this, SvxSuperContourDlg, StateHdl ) );
    aContourWnd.SetPipetteHdl( LINK( this, SvxSuperContourDlg, PipetteHdl ) );
    aContourWnd.SetPipetteClickHdl( LINK( this, SvxSuperContourDlg, PipetteClickHdl ) );
    aContourWnd.SetWorkplaceClickHdl( LINK( this, SvxSuperContourDlg, WorkplaceClickHdl ) );

    const Size  aTbxSize( aTbx1.CalcWindowSizePixel() );
    Point       aPos( aTbx1.GetPosPixel() );

    aTbx1.SetSizePixel( aTbxSize );
    aTbx1.SetSelectHdl( LINK( this, SvxSuperContourDlg, Tbx1ClickHdl ) );

    aPos.X() += aTbxSize.Width() + LogicToPixel( Size( 3, 0 ), MapMode( MAP_APPFONT ) ).Width();
    aMtfTolerance.SetPosPixel( aPos );
    aMtfTolerance.SetValue( 10L );

    SetMinOutputSizePixel( aLastSize = GetOutputSizePixel() );

    aStbStatus.InsertItem( 1, 130, SIB_LEFT | SIB_IN | SIB_AUTOSIZE );
    aStbStatus.InsertItem( 2, 10 + GetTextWidth( String::CreateFromAscii( " 9999,99 cm / 9999,99 cm " ) ), SIB_CENTER | SIB_IN );
    aStbStatus.InsertItem( 3, 10 + GetTextWidth( String::CreateFromAscii( " 9999,99 cm x 9999,99 cm " ) ), SIB_CENTER | SIB_IN );
    aStbStatus.InsertItem( 4, 20, SIB_CENTER | SIB_IN );

    Resize();

    EnableDrop(TRUE);

    aUpdateTimer.SetTimeout( 100 );
    aUpdateTimer.SetTimeoutHdl( LINK( this, SvxSuperContourDlg, UpdateHdl ) );

    aCreateTimer.SetTimeout( 50 );
    aCreateTimer.SetTimeoutHdl( LINK( this, SvxSuperContourDlg, CreateHdl ) );
}


/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/

SvxSuperContourDlg::~SvxSuperContourDlg()
{
}


/*************************************************************************
|*
|* Resize-Methode
|*
\************************************************************************/

void SvxSuperContourDlg::Resize()
{
    SfxFloatingWindow::Resize();

    Size aMinSize( GetMinOutputSizePixel() );
    Size aNewSize( GetOutputSizePixel() );

    if ( aNewSize.Height() >= aMinSize.Height() )
    {
        Size    aSize( aStbStatus.GetSizePixel() );
        Point   aPoint( 0, aNewSize.Height() - aSize.Height() );

        // StatusBar positionieren
        aStbStatus.SetPosSizePixel( aPoint, Size( aNewSize.Width(), aSize.Height() ) );
        aStbStatus.Show();

        // EditWindow positionieren
        aSize.Width() = aNewSize.Width() - 18;
        aSize.Height() = aPoint.Y() - aContourWnd.GetPosPixel().Y() - 6;
        aContourWnd.SetSizePixel( aSize );

        aLastSize = aNewSize;
    }
}


/*************************************************************************
|*
|* Close-Methode
|*
\************************************************************************/

BOOL SvxSuperContourDlg::Close()
{
    BOOL bRet = TRUE;

    if ( aTbx1.IsItemEnabled( TBI_APPLY ) )
    {
        QueryBox    aQBox( this, WB_YES_NO_CANCEL | WB_DEF_YES,
                           String( CONT_RESID( STR_CONTOURDLG_MODIFY ) ) );
        const long  nRet = aQBox.Execute();

        if ( nRet == RET_YES )
        {
            SfxBoolItem aBoolItem( SID_CONTOUR_EXEC, TRUE );
            GetBindings().GetDispatcher()->Execute(
                SID_CONTOUR_EXEC, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD, &aBoolItem, 0L );
        }
        else if ( nRet == RET_CANCEL )
            bRet = FALSE;
    }

    return( bRet ? SfxFloatingWindow::Close() : FALSE );
}


/*************************************************************************
|*
|* Enabled oder disabled alle Controls
|*
\************************************************************************/

void SvxSuperContourDlg::SetExecState( BOOL bEnable )
{
    bExecState = bEnable;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void SvxSuperContourDlg::SetGraphic( const Graphic& rGraphic )
{
    aUndoGraphic = aRedoGraphic = Graphic();
    aGraphic = rGraphic;
    nGrfChanged = 0UL;
    aContourWnd.SetGraphic( aGraphic );
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void SvxSuperContourDlg::SetPolyPolygon( const PolyPolygon& rPolyPoly )
{
    DBG_ASSERT(  aContourWnd.GetGraphic().GetType() != GRAPHIC_NONE, "Graphic must've been set first!" );

    PolyPolygon     aPolyPoly( rPolyPoly );
    const MapMode   aMap100( MAP_100TH_MM );
    const MapMode   aGrfMap( aGraphic.GetPrefMapMode() );
    OutputDevice*   pOutDev = Application::GetDefaultDevice();
    BOOL            bPixelMap = aGrfMap.GetMapUnit() == MAP_PIXEL;

    for ( USHORT j = 0, nPolyCount = aPolyPoly.Count(); j < nPolyCount; j++ )
    {
        Polygon& rPoly = aPolyPoly[ j ];

        for ( USHORT i = 0, nCount = rPoly.GetSize(); i < nCount; i++ )
        {
            Point& rPt = rPoly[ i ];

            if ( !bPixelMap )
                rPt = pOutDev->LogicToPixel( rPt, aGrfMap );

            rPt = pOutDev->PixelToLogic( rPt, aMap100 );
        }
    }

    aContourWnd.SetPolyPolygon( aPolyPoly );
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

PolyPolygon SvxSuperContourDlg::GetPolyPolygon( BOOL bRescaleToGraphic )
{
    PolyPolygon aRetPolyPoly( aContourWnd.GetPolyPolygon() );

    if ( bRescaleToGraphic )
    {
        const MapMode   aMap100( MAP_100TH_MM );
        const MapMode   aGrfMap( aGraphic.GetPrefMapMode() );
        OutputDevice*   pOutDev = Application::GetDefaultDevice();
        BOOL            bPixelMap = aGrfMap.GetMapUnit() == MAP_PIXEL;

        for ( USHORT j = 0, nPolyCount = aRetPolyPoly.Count(); j < nPolyCount; j++ )
        {
            Polygon& rPoly = aRetPolyPoly[ j ];

            for ( USHORT i = 0, nCount = rPoly.GetSize(); i < nCount; i++ )
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


/*************************************************************************
|*
|*
|*
\************************************************************************/

void SvxSuperContourDlg::Update( const Graphic& rGraphic, BOOL bGraphicLinked,
                                 const PolyPolygon* pPolyPoly, void* pEditingObj )
{
    aUpdateGraphic = rGraphic;
    bUpdateGraphicLinked = bGraphicLinked;
    pUpdateEditingObject = pEditingObj;

    if ( pPolyPoly )
        aUpdatePolyPoly = *pPolyPoly;
    else
        aUpdatePolyPoly = PolyPolygon();

    aUpdateTimer.Start();
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

BOOL SvxSuperContourDlg::IsUndoPossible() const
{
    return aUndoGraphic.GetType() != GRAPHIC_NONE;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

BOOL SvxSuperContourDlg::IsRedoPossible() const
{
    return aRedoGraphic.GetType() != GRAPHIC_NONE;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void SvxSuperContourDlg::DoAutoCreate()
{
    aCreateTimer.Start();
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void SvxSuperContourDlg::ReducePoints( const long nTol )
{
    PolyPolygon aPolyPoly( GetPolyPolygon( FALSE ) );

    if ( aPolyPoly.Count() )
    {
        const MapMode   aMapMode( MAP_100TH_MM );
        const long      nTol2 = nTol * nTol;
        Polygon&        rPoly = aPolyPoly[ 0 ];
        OutputDevice*   pOutDev = Application::GetDefaultDevice();
        Point           aPtPix;
        const USHORT    nSize = rPoly.GetSize();
        USHORT          nCounter = 0;

        if ( nSize )
            aPtPix = pOutDev->LogicToPixel( rPoly[ 0 ], aMapMode );

        for( USHORT i = 1; i < nSize; i++ )
        {
            const Point&    rNewPt = rPoly[ i ];
            const Point     aNewPtPix( pOutDev->LogicToPixel( rNewPt, aMapMode ) );

            const long nDistX = aNewPtPix.X() - aPtPix.X();
            const long nDistY = aNewPtPix.Y() - aPtPix.Y();

            if( ( nDistX * nDistX + nDistY * nDistY ) >= nTol2 )
            {
                rPoly[ ++nCounter ] = rNewPt;
                aPtPix = aNewPtPix;
            }
        }

        rPoly.SetSize( nCounter );
        aContourWnd.SetPolyPolygon( aPolyPoly );
    }
}


/*************************************************************************
|*
|* Click-Hdl fuer ToolBox
|*
\************************************************************************/

IMPL_LINK( SvxSuperContourDlg, Tbx1ClickHdl, ToolBox*, pTbx )
{
    USHORT nNewItemId = pTbx->GetCurItemId();

    switch( pTbx->GetCurItemId() )
    {
        case( TBI_APPLY ):
        {
            SfxBoolItem aBoolItem( SID_CONTOUR_EXEC, TRUE );
            GetBindings().GetDispatcher()->Execute(
                SID_CONTOUR_EXEC, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aBoolItem, 0L );
        }
        break;

        case( TBI_WORKPLACE ):
        {
            if ( aTbx1.IsItemChecked( TBI_WORKPLACE ) )
            {
                QueryBox aQBox( this, WB_YES_NO | WB_DEF_NO, String( CONT_RESID( STR_CONTOURDLG_WORKPLACE ) ) );

                if ( !aContourWnd.IsContourChanged() || ( aQBox.Execute() == RET_YES ) )
                    aContourWnd.SetWorkplaceMode( TRUE );
                else
                    aTbx1.CheckItem( TBI_WORKPLACE, FALSE );
            }
            else
                aContourWnd.SetWorkplaceMode( FALSE );
        }
        break;

        case( TBI_SELECT ):
        {
            pTbx->CheckItem( nNewItemId, TRUE );
            aContourWnd.SetEditMode( TRUE );
        }
        break;

        case( TBI_RECT ):
        {
            pTbx->CheckItem( nNewItemId, TRUE );
            aContourWnd.SetObjKind( OBJ_RECT );
        }
        break;

        case( TBI_CIRCLE ):
        {
            pTbx->CheckItem( nNewItemId, TRUE );
            aContourWnd.SetObjKind( OBJ_CIRC );
        }
        break;

        case( TBI_POLY ):
        {
            pTbx->CheckItem( nNewItemId, TRUE );
            aContourWnd.SetObjKind( OBJ_POLY );
        }
        break;

        case( TBI_FREEPOLY ):
        {
            pTbx->CheckItem( nNewItemId, TRUE );
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
            nGrfChanged = nGrfChanged ? nGrfChanged-- : 0UL;
            aRedoGraphic = aGraphic;
            aGraphic = aUndoGraphic;
            aUndoGraphic = Graphic();
            aContourWnd.SetGraphic( aGraphic, FALSE );
        }
        break;

        case( TBI_REDO ):
        {
            nGrfChanged++;
            aUndoGraphic = aGraphic;
            aGraphic = aRedoGraphic;
            aRedoGraphic = Graphic();
            aContourWnd.SetGraphic( aGraphic, FALSE );
        }
        break;

        case( TBI_AUTOCONTOUR ):
            aCreateTimer.Start();
        break;

        case( TBI_PIPETTE ):
        {
            BOOL bPipette = aTbx1.IsItemChecked( TBI_PIPETTE );

            if ( !bPipette )
                aStbStatus.Invalidate();
            else if ( bGraphicLinked )
            {
                QueryBox aQBox( this, WB_YES_NO | WB_DEF_YES, String( CONT_RESID( STR_CONTOURDLG_LINKED ) ) );

                if ( aQBox.Execute() != RET_YES )
                {
                    aTbx1.CheckItem( TBI_PIPETTE, bPipette = FALSE );
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


/*************************************************************************
|*
|*
|*
\************************************************************************/

IMPL_LINK( SvxSuperContourDlg, MousePosHdl, ContourWindow*, pWnd )
{
    String aStr;
    const FieldUnit eFieldUnit = GetModuleFieldUnit();
    const Point& rMousePos = pWnd->GetMousePos();
    const sal_Unicode cSep = International().GetNumDecimalSep();

    aStr.Assign( GetUnitString( rMousePos.X(), eFieldUnit, cSep ) );
    aStr.Append( String::CreateFromAscii( " / " ) );
    aStr.Append( GetUnitString( rMousePos.Y(), eFieldUnit, cSep ) );

    aStbStatus.SetItemText( 2, aStr );

    return 0L;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

IMPL_LINK( SvxSuperContourDlg, GraphSizeHdl, ContourWindow*, pWnd )
{
    String aStr;
    const FieldUnit eFieldUnit = GetModuleFieldUnit();
    const Size& rSize = pWnd->GetGraphicSize();
    const sal_Unicode cSep = International().GetNumDecimalSep();

    aStr.Assign( GetUnitString( rSize.Width(), eFieldUnit, cSep ) );
    aStr.Append( String::CreateFromAscii( " x " ) );
    aStr.Append( GetUnitString( rSize.Height(), eFieldUnit, cSep ) );

    aStbStatus.SetItemText( 3, aStr );

    return 0L;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

IMPL_LINK( SvxSuperContourDlg, UpdateHdl, Timer*, pTimer )
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
        bUpdateGraphicLinked = FALSE;

        aContourWnd.GetSdrModel()->SetChanged( FALSE );
    }

    GetBindings().Invalidate( SID_CONTOUR_EXEC );

    return 0L;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

IMPL_LINK( SvxSuperContourDlg, CreateHdl, Timer*, pTimer )
{
    aCreateTimer.Stop();

    const Rectangle aWorkRect = aContourWnd.LogicToPixel( aContourWnd.GetWorkRect(), MapMode( MAP_100TH_MM ) );
    const Graphic&  rGraphic = aContourWnd.GetGraphic();
    const BOOL      bValid = aWorkRect.Left() != aWorkRect.Right() && aWorkRect.Top() != aWorkRect.Bottom();

    EnterWait();
    SetPolyPolygon( CreateAutoContour( rGraphic, bValid ? &aWorkRect : NULL ) );
    LeaveWait();

    return 0L;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

IMPL_LINK( SvxSuperContourDlg, StateHdl, ContourWindow*, pWnd )
{
    const SdrObject*    pObj = pWnd->GetSelectedSdrObject();
    const SdrView*      pView = pWnd->GetSdrView();
    const BOOL          bPolyEdit = ( pObj != NULL ) && pObj->ISA( SdrPathObj );
    const BOOL          bDrawEnabled = !( bPolyEdit && aTbx1.IsItemChecked( TBI_POLYEDIT ) );
    const BOOL          bPipette = aTbx1.IsItemChecked( TBI_PIPETTE );
    const BOOL          bWorkplace = aTbx1.IsItemChecked( TBI_WORKPLACE );
    const BOOL          bDontHide = !( bPipette || bWorkplace );
    const BOOL          bBitmap = pWnd->GetGraphic().GetType() == GRAPHIC_BITMAP;

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
        USHORT nId = 0;

        switch( pWnd->GetPolyEditMode() )
        {
            case( SID_BEZIER_MOVE ): nId = TBI_POLYMOVE; break;
            case( SID_BEZIER_INSERT ): nId = TBI_POLYINSERT; break;

            default:
            break;
        }

        aTbx1.CheckItem( nId, TRUE );
    }
    else
    {
        aTbx1.CheckItem( TBI_POLYEDIT, FALSE );
        aTbx1.CheckItem( TBI_POLYMOVE, TRUE );
        aTbx1.CheckItem( TBI_POLYINSERT, FALSE );
        pWnd->SetPolyEditMode( 0 );
    }

    return 0L;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

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


/*************************************************************************
|*
|*
|*
\************************************************************************/

IMPL_LINK( SvxSuperContourDlg, PipetteClickHdl, ContourWindow*, pWnd )
{
    if ( pWnd->IsClickValid() )
    {
        Bitmap          aMask;
        const Color&    rColor = pWnd->GetPipetteColor();
        Bitmap*         pMask = NULL;

        EnterWait();

        if( aGraphic.GetType() == GRAPHIC_BITMAP )
        {
            Bitmap      aBmp( aGraphic.GetBitmap() );
            const long  nTol = aMtfTolerance.GetValue() * 255L / 100L;

            aMask = aBmp.CreateMask( rColor, nTol );

            if( aGraphic.IsTransparent() )
                aMask.CombineSimple( aGraphic.GetBitmapEx().GetMask(), BMP_COMBINE_OR );

            if( !!aMask )
            {
                QueryBox    aQBox( this, WB_YES_NO | WB_DEF_YES, String( CONT_RESID( STR_CONTOURDLG_NEWPIPETTE ) ) );
                BOOL        bNewContour;

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

    aTbx1.CheckItem( TBI_PIPETTE, FALSE );
    pWnd->SetPipetteMode( FALSE );
    aStbStatus.Invalidate();

    return 0L;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

IMPL_LINK( SvxSuperContourDlg, WorkplaceClickHdl, ContourWindow*, pWnd )
{
    aTbx1.CheckItem( TBI_WORKPLACE, FALSE );
    aTbx1.CheckItem( TBI_SELECT, TRUE );
    pWnd->SetWorkplaceMode( FALSE );

    return 0L;
}


