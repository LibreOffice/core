/*************************************************************************
 *
 *  $RCSfile: graphctl.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-24 14:46:11 $
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

#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif

#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif

#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_SYSLOCALE_HXX
#include <svtools/syslocale.hxx>
#endif
#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif

#include "graphctl.hxx"
#include "GraphCtlAccessibleContext.hxx"
#include "xoutbmp.hxx"
#include "svxids.hrc"
#include "svdpage.hxx"

/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrlUserCall::Changed( const SdrObject& rObj, SdrUserCallType eType, const Rectangle& rOldBoundRect )
{
    switch( eType )
    {
        case( SDRUSERCALL_MOVEONLY ):
        case( SDRUSERCALL_RESIZE ):
            rWin.SdrObjChanged( rObj );
        break;

        case( SDRUSERCALL_INSERTED ):
            rWin.SdrObjCreated( rObj );
        break;

        default:
        break;
    }
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

GraphCtrl::GraphCtrl( Window* pParent, const WinBits nWinBits ) :
            Control         ( pParent, nWinBits ),
            aMap100         ( MAP_100TH_MM ),
            pModel          ( NULL ),
            pView           ( NULL ),
            eObjKind        ( OBJ_NONE ),
            nPolyEdit       ( 0 ),
            bEditMode       ( FALSE ),
            bSdrMode        ( FALSE ),
            mpAccContext    ( NULL )
{
    pUserCall = new GraphCtrlUserCall( *this );
    aUpdateTimer.SetTimeout( 200 );
    aUpdateTimer.SetTimeoutHdl( LINK( this, GraphCtrl, UpdateHdl ) );
    aUpdateTimer.Start();

    SetWinStyle( nWinBits );
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

GraphCtrl::GraphCtrl( Window* pParent, const ResId& rResId ) :
            Control         ( pParent, rResId ),
            aMap100         ( MAP_100TH_MM ),
            pModel          ( NULL ),
            pView           ( NULL ),
            eObjKind        ( OBJ_NONE ),
            nPolyEdit       ( 0 ),
            bEditMode       ( FALSE ),
            bSdrMode        ( FALSE ),
            bAnim           ( FALSE ),
            nWinStyle       ( 0 ),
            mpAccContext    ( NULL )
{
    pUserCall = new GraphCtrlUserCall( *this );
    aUpdateTimer.SetTimeout( 500 );
    aUpdateTimer.SetTimeoutHdl( LINK( this, GraphCtrl, UpdateHdl ) );
    aUpdateTimer.Start();
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

GraphCtrl::~GraphCtrl()
{
    if( mpAccContext )
    {
        mpAccContext->disposing();
        mpAccContext->release();
    }
    delete pView;
    delete pModel;
    delete pUserCall;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrl::SetWinStyle( WinBits nWinBits )
{
    nWinStyle = nWinBits;
    bAnim = ( nWinStyle & WB_ANIMATION ) == WB_ANIMATION;
    bSdrMode = ( nWinStyle & WB_SDRMODE ) == WB_SDRMODE;

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    SetBackground( Wallpaper( rStyleSettings.GetWindowColor() ) );
    SetMapMode( aMap100 );

    delete pView;
    pView = NULL;

    delete pModel;
    pModel = NULL;

    if ( bSdrMode )
        InitSdrModel();
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrl::InitSdrModel()
{
    SdrPage* pPage;

    // alten Kram zerstoeren
    delete pView;
    delete pModel;

    // Model anlegen
    pModel = new SdrModel;
    pModel->GetItemPool().FreezeIdRanges();
    pModel->SetScaleUnit( aMap100.GetMapUnit() );
    pModel->SetScaleFraction( Fraction( 1, 1 ) );
    pModel->SetDefaultFontHeight( 500 );

    pPage = new SdrPage( *pModel );

    pPage->SetSize( aGraphSize );
    pPage->SetBorder( 0, 0, 0, 0 );
    pModel->InsertPage( pPage );
    pModel->SetChanged( FALSE );

    // View anlegen
    pView = new GraphCtrlView( pModel, this );
    pView->SetWorkArea( Rectangle( Point(), aGraphSize ) );
    pView->SetHlplVisible( FALSE );
    pView->SetGridVisible( FALSE );
    pView->SetBordVisible( FALSE );
    pView->SetPageVisible( FALSE );
    pView->EnableExtendedMouseEventDispatcher( TRUE );
    pView->ShowPagePgNum( 0, Point() );
    pView->SetFrameDragSingles( TRUE );
    pView->SetMarkedPointsSmooth( SDRPATHSMOOTH_SYMMETRIC );
    pView->SetEditMode( TRUE );
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrl::SetGraphic( const Graphic& rGraphic, BOOL bNewModel )
{
    // Bitmaps dithern wir ggf. fuer die Anzeige
    if ( !bAnim && ( rGraphic.GetType() == GRAPHIC_BITMAP )  )
    {
        if ( rGraphic.IsTransparent() )
        {
            Bitmap  aBmp( rGraphic.GetBitmap() );

            DitherBitmap( aBmp );
            aGraphic = Graphic( BitmapEx( aBmp, rGraphic.GetBitmapEx().GetMask() ) );
        }
        else
        {
            Bitmap aBmp( rGraphic.GetBitmap() );
            DitherBitmap( aBmp );
            aGraphic = aBmp;
        }
    }
    else
        aGraphic = rGraphic;

    if ( aGraphic.GetPrefMapMode().GetMapUnit() == MAP_PIXEL )
        aGraphSize = Application::GetDefaultDevice()->PixelToLogic( aGraphic.GetPrefSize(), aMap100 );
    else
        aGraphSize = OutputDevice::LogicToLogic( aGraphic.GetPrefSize(), aGraphic.GetPrefMapMode(), aMap100 );

    if ( bSdrMode && bNewModel )
        InitSdrModel();

    if ( aGraphSizeLink.IsSet() )
        aGraphSizeLink.Call( this );

    Resize();
    Invalidate();
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrl::Resize()
{
    Control::Resize();

    if ( aGraphSize.Width() && aGraphSize.Height() )
    {
        MapMode         aDisplayMap( aMap100 );
        Point           aNewPos;
        Size            aNewSize;
        const Size      aWinSize = PixelToLogic( GetOutputSizePixel(), aDisplayMap );
        const long      nWidth = aWinSize.Width();
        const long      nHeight = aWinSize.Height();
        double          fGrfWH = (double) aGraphSize.Width() / aGraphSize.Height();
        double          fWinWH = (double) nWidth / nHeight;

        // Bitmap an Thumbgroesse anpassen
        if ( fGrfWH < fWinWH)
        {
            aNewSize.Width() = (long) ( (double) nHeight * fGrfWH );
            aNewSize.Height()= nHeight;
        }
        else
        {
            aNewSize.Width() = nWidth;
            aNewSize.Height()= (long) ( (double) nWidth / fGrfWH );
        }

        aNewPos.X() = ( nWidth - aNewSize.Width() )  >> 1;
        aNewPos.Y() = ( nHeight - aNewSize.Height() ) >> 1;

        // MapMode fuer Engine umsetzen
        aDisplayMap.SetScaleX( Fraction( aNewSize.Width(), aGraphSize.Width() ) );
        aDisplayMap.SetScaleY( Fraction( aNewSize.Height(), aGraphSize.Height() ) );

        aDisplayMap.SetOrigin( LogicToLogic( aNewPos, aMap100, aDisplayMap ) );
        SetMapMode( aDisplayMap );
    }

    Invalidate();
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrl::Paint( const Rectangle& rRect )
{
    if ( aGraphic.GetType() != GRAPHIC_NONE )
        aGraphic.Draw( this, Point(), aGraphSize );

    if ( bSdrMode )
        pView->InitRedraw( this , Region( rRect ) );
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrl::SdrObjChanged( const SdrObject& rObj )
{
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrl::SdrObjCreated( const SdrObject& rObj )
{
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrl::MarkListHasChanged()
{
    if ( aMarkObjLink.IsSet() )
        aMarkObjLink.Call( this );
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrl::KeyInput( const KeyEvent& rKEvt )
{
    KeyCode aCode( rKEvt.GetKeyCode() );
    BOOL    bProc = FALSE;

    switch ( aCode.GetCode() )
    {
        case KEY_DELETE:
        case KEY_BACKSPACE:
        {
            if ( bSdrMode )
            {
                pView->DeleteMarked();
                bProc = TRUE;
                if( !pView->HasMarkedObj() )
                    ((Dialog*)GetParent())->GrabFocusToFirstControl();
            }
        }
        break;

        case KEY_ESCAPE:
        {
            if ( bSdrMode )
            {
                if ( pView->IsAction() )
                {
                    pView->BrkAction();
                }
                else if ( pView->HasMarkedObj() )
                {
                    const SdrHdlList& rHdlList = pView->GetHdlList();
                    SdrHdl* pHdl = rHdlList.GetFocusHdl();

                    if(pHdl)
                    {
                        ((SdrHdlList&)rHdlList).ResetFocusHdl();
                    }
                    else
                    {
                        ((Dialog*)GetParent())->GrabFocusToFirstControl();
                    }
                }
                else
                {
                    ((Dialog*)GetParent())->GrabFocusToFirstControl();
                }
                bProc = TRUE;
            }
        }
        break;

        case KEY_F11:
        case KEY_TAB:
        {
            if( bSdrMode )
            {
                if( !aCode.IsMod1() && !aCode.IsMod2() )
                {
                    bool bForward = !aCode.IsShift();
                    // select next object
                    if ( ! pView->MarkNextObj( bForward ))
                    {
                        // At first or last object.  Cycle to the other end
                        // of the list.
                        pView->UnmarkAllObj();
                        pView->MarkNextObj (bForward);
                    }
                    bProc = TRUE;
                }
                else if(aCode.IsMod1())
                {
                    // select next handle
                    const SdrHdlList& rHdlList = pView->GetHdlList();
                    sal_Bool bForward(!aCode.IsShift());

                    ((SdrHdlList&)rHdlList).TravelFocusHdl(bForward);

                    bProc = true;
                }
            }
        }
        break;

        case KEY_END:
        {

            if ( aCode.IsMod1() )
            {
                // #97016# mark last object
                pView->UnmarkAllObj();
                pView->MarkNextObj(FALSE);

                bProc = true;
            }
        }
        break;

        case KEY_HOME:
        {
            if ( aCode.IsMod1() )
            {
                pView->UnmarkAllObj();
                pView->MarkNextObj(TRUE);

                bProc = true;
            }
        }
        break;

        case KEY_UP:
        case KEY_DOWN:
        case KEY_LEFT:
        case KEY_RIGHT:
        {
            long nX = 0;
            long nY = 0;

            if (aCode.GetCode() == KEY_UP)
            {
                // Scroll nach oben
                nX = 0;
                nY =-1;
            }
            else if (aCode.GetCode() == KEY_DOWN)
            {
                // Scroll nach unten
                nX = 0;
                nY = 1;
            }
            else if (aCode.GetCode() == KEY_LEFT)
            {
                // Scroll nach links
                nX =-1;
                nY = 0;
            }
            else if (aCode.GetCode() == KEY_RIGHT)
            {
                // Scroll nach rechts
                nX = 1;
                nY = 0;
            }

            if (pView->HasMarkedObj() && !aCode.IsMod1() )
            {
                if(aCode.IsMod2())
                {
                    // #97016# move in 1 pixel distance
                    Size aLogicSizeOnePixel = PixelToLogic(Size(1,1));
                    nX *= aLogicSizeOnePixel.Width();
                    nY *= aLogicSizeOnePixel.Height();
                }
                else
                {
                    // old, fixed move distance
                    nX *= 100;
                    nY *= 100;
                }

                // #97016# II
                const SdrHdlList& rHdlList = pView->GetHdlList();
                SdrHdl* pHdl = rHdlList.GetFocusHdl();

                if(0L == pHdl)
                {
                    // #90129# restrict movement to WorkArea
                    const Rectangle& rWorkArea = pView->GetWorkArea();

                    if(!rWorkArea.IsEmpty())
                    {
                        Rectangle aMarkRect(pView->GetMarkedObjRect());
                        aMarkRect.Move(nX, nY);

                        if(!aMarkRect.IsInside(rWorkArea))
                        {
                            if(aMarkRect.Left() < rWorkArea.Left())
                            {
                                nX += rWorkArea.Left() - aMarkRect.Left();
                            }

                            if(aMarkRect.Right() > rWorkArea.Right())
                            {
                                nX -= aMarkRect.Right() - rWorkArea.Right();
                            }

                            if(aMarkRect.Top() < rWorkArea.Top())
                            {
                                nY += rWorkArea.Top() - aMarkRect.Top();
                            }

                            if(aMarkRect.Bottom() > rWorkArea.Bottom())
                            {
                                nY -= aMarkRect.Bottom() - rWorkArea.Bottom();
                            }
                        }
                    }

                    // no handle selected
                    if(0 != nX || 0 != nY)
                    {
                        pView->MoveAllMarked(Size(nX, nY));
                    }
                }
                else
                {
                    // move handle with index nHandleIndex
                    if(pHdl && (nX || nY))
                    {
                        // now move the Handle (nX, nY)
                        Point aStartPoint(pHdl->GetPos());
                        Point aEndPoint(pHdl->GetPos() + Point(nX, nY));
                        const SdrDragStat& rDragStat = pView->GetDragStat();

                        // start dragging
                        pView->BegDragObj(aStartPoint, 0, pHdl, 0);

                        if(pView->IsDragObj())
                        {
                            FASTBOOL bWasNoSnap = rDragStat.IsNoSnap();
                            BOOL bWasSnapEnabled = pView->IsSnapEnabled();

                            // switch snapping off
                            if(!bWasNoSnap)
                                ((SdrDragStat&)rDragStat).SetNoSnap(TRUE);
                            if(bWasSnapEnabled)
                                pView->SetSnapEnabled(FALSE);

                            pView->MovAction(aEndPoint);
                            pView->EndDragObj();

                            // restore snap
                            if(!bWasNoSnap)
                                ((SdrDragStat&)rDragStat).SetNoSnap(bWasNoSnap);
                            if(bWasSnapEnabled)
                                pView->SetSnapEnabled(bWasSnapEnabled);
                        }
                    }
                }

                bProc = true;
            }
        }
        break;

        case KEY_SPACE:
        {
            const SdrHdlList& rHdlList = pView->GetHdlList();
            SdrHdl* pHdl = rHdlList.GetFocusHdl();

            if(pHdl)
            {
                if(pHdl->GetKind() == HDL_POLY)
                {
                    // rescue ID of point with focus
                    sal_uInt16 nPol(pHdl->GetPolyNum());
                    sal_uInt16 nPnt(pHdl->GetPointNum());

                    if(pView->IsPointMarked(*pHdl))
                    {
                        if(rKEvt.GetKeyCode().IsShift())
                        {
                            pView->UnmarkPoint(*pHdl);
                        }
                    }
                    else
                    {
                        if(!rKEvt.GetKeyCode().IsShift())
                        {
                            pView->UnmarkAllPoints();
                        }

                        pView->MarkPoint(*pHdl);
                    }

                    if(0L == rHdlList.GetFocusHdl())
                    {
                        // restore point with focus
                        SdrHdl* pNewOne = 0L;

                        for(sal_uInt32 a(0); !pNewOne && a < rHdlList.GetHdlCount(); a++)
                        {
                            SdrHdl* pAct = rHdlList.GetHdl(a);

                            if(pAct
                                && pAct->GetKind() == HDL_POLY
                                && pAct->GetPolyNum() == nPol
                                && pAct->GetPointNum() == nPnt)
                            {
                                pNewOne = pAct;
                            }
                        }

                        if(pNewOne)
                        {
                            ((SdrHdlList&)rHdlList).SetFocusHdl(pNewOne);
                        }
                    }

                    bProc = TRUE;
                }
            }
        }
        break;

        default:
        break;
    }

    if ( !bProc )
        Control::KeyInput( rKEvt );
    else
        ReleaseMouse();
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrl::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( bSdrMode && ( rMEvt.GetClicks() < 2 ) )
    {
        const Point aLogPt( PixelToLogic( rMEvt.GetPosPixel() ) );

        if ( !Rectangle( Point(), aGraphSize ).IsInside( aLogPt ) && !pView->IsEditMode() )
            Control::MouseButtonDown( rMEvt );
        else
        {
            // Focus anziehen fuer Key-Inputs
            GrabFocus();

            if ( nPolyEdit )
            {
                SdrViewEvent    aVEvt;
                SdrHitKind      eHit = pView->PickAnything( rMEvt, SDRMOUSEBUTTONDOWN, aVEvt );

                if ( nPolyEdit == SID_BEZIER_INSERT && eHit == SDRHIT_MARKEDOBJECT )
                    pView->BegInsObjPoint( aLogPt, rMEvt.IsMod1(), NULL, 0 );
                else
                    pView->MouseButtonDown( rMEvt, this );
            }
            else
                pView->MouseButtonDown( rMEvt, this );
        }

        SdrObject* pCreateObj = pView->GetCreateObj();

        // Wir wollen das Inserten mitbekommen
        if ( pCreateObj && !pCreateObj->GetUserCall() )
            pCreateObj->SetUserCall( pUserCall );

        SetPointer( pView->GetPreferedPointer( aLogPt, this ) );
    }
    else
        Control::MouseButtonDown( rMEvt );
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrl::MouseMove(const MouseEvent& rMEvt)
{
    const Point aLogPos( PixelToLogic( rMEvt.GetPosPixel() ) );

    if ( bSdrMode )
    {
        pView->MouseMove( rMEvt, this );

        if( ( SID_BEZIER_INSERT == nPolyEdit ) &&
            !pView->HitHandle( aLogPos, *this ) &&
            !pView->IsInsObjPoint() )
        {
            SetPointer( POINTER_CROSS );
        }
        else
            SetPointer( pView->GetPreferedPointer( aLogPos, this ) );
    }
    else
        Control::MouseButtonUp( rMEvt );

    if ( aMousePosLink.IsSet() )
    {
        if ( Rectangle( Point(), aGraphSize ).IsInside( aLogPos ) )
            aMousePos = aLogPos;
        else
            aMousePos = Point();

        aMousePosLink.Call( this );
    }
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrl::MouseButtonUp(const MouseEvent& rMEvt)
{
    if ( bSdrMode )
    {
        if ( pView->IsInsObjPoint() )
            pView->EndInsObjPoint( SDRCREATE_FORCEEND );
        else
            pView->MouseButtonUp( rMEvt, this );

        ReleaseMouse();
        SetPointer( pView->GetPreferedPointer( PixelToLogic( rMEvt.GetPosPixel() ), this ) );
    }
    else
        Control::MouseButtonUp( rMEvt );
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

SdrObject* GraphCtrl::GetSelectedSdrObject() const
{
    SdrObject* pSdrObj = NULL;

    if ( bSdrMode )
    {
        const SdrMarkList&  rMarkList = pView->GetMarkList();

        if ( rMarkList.GetMarkCount() == 1 )
            pSdrObj = rMarkList.GetMark( 0 )->GetObj();
    }

    return pSdrObj;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrl::SetEditMode( const BOOL _bEditMode )
{
    if ( bSdrMode )
    {
        pView->SetEditMode( bEditMode = _bEditMode );
        pView->SetCurrentObj( eObjKind = OBJ_NONE );
    }
    else
        bEditMode = FALSE;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrl::SetPolyEditMode( const USHORT _nPolyEdit )
{
    if ( bSdrMode && ( _nPolyEdit != nPolyEdit ) )
    {
        nPolyEdit = _nPolyEdit;
        pView->SetFrameDragSingles( nPolyEdit == 0 );
    }
    else
        nPolyEdit = 0;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrl::SetObjKind( const SdrObjKind _eObjKind )
{
    if ( bSdrMode )
    {
        pView->SetEditMode( bEditMode = FALSE );
        pView->SetCurrentObj( eObjKind = _eObjKind );
    }
    else
        eObjKind = OBJ_NONE;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

String GraphCtrl::GetStringFromDouble( const double& rDouble )
{
    sal_Unicode cSep =
        SvtSysLocale().GetLocaleData().getNumDecimalSep().GetChar(0);
    String aStr( ::rtl::math::doubleToUString( rDouble,
                rtl_math_StringFormat_F, 2, cSep ));
    return aStr;
}


/*************************************************************************
www|*
|*
|*
\************************************************************************/

IMPL_LINK( GraphCtrl, UpdateHdl, Timer*, pTimer )
{
    if ( aUpdateLink.IsSet() )
        aUpdateLink.Call( this );

    pTimer->Start();

    return 0L;
}


::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > GraphCtrl::CreateAccessible()
{
    if( mpAccContext == NULL )
    {
        Window* pParent = GetParent();

        DBG_ASSERT( pParent, "-GraphCtrl::CreateAccessible(): No Parent!" );

        if( pParent )
        {
            ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > xAccParent( pParent->GetAccessible() );

            // #103856# Disable accessibility if no model/view data available
            if( pView &&
                pModel &&
                xAccParent.is() )
            {
                mpAccContext = new SvxGraphCtrlAccessibleContext( xAccParent, *this );
                mpAccContext->acquire();
            }
        }
    }

    return mpAccContext;
}
