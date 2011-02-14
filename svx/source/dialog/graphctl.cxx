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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <svl/itempool.hxx>
#include <vcl/dialog.hxx>
#include <vcl/wrkwin.hxx>
#include <unotools/syslocale.hxx>
#include <rtl/math.hxx>
#include <unotools/localedatawrapper.hxx>
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>

#include <svx/graphctl.hxx>
#include "GraphCtlAccessibleContext.hxx"
#include "svx/xoutbmp.hxx"
#include <svx/svxids.hrc>
#include <svx/svdpage.hxx>

// #i72889#
#include "svx/sdrpaintwindow.hxx"

/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrlUserCall::Changed( const SdrObject& rObj, SdrUserCallType eType, const Rectangle& /*rOldBoundRect*/ )
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
            eObjKind        ( OBJ_NONE ),
            nPolyEdit       ( 0 ),
            bEditMode       ( sal_False ),
            bSdrMode        ( sal_False ),
            mpAccContext    ( NULL ),
            pModel          ( NULL ),
            pView           ( NULL )
{
    pUserCall = new GraphCtrlUserCall( *this );
    aUpdateTimer.SetTimeout( 200 );
    aUpdateTimer.SetTimeoutHdl( LINK( this, GraphCtrl, UpdateHdl ) );
    aUpdateTimer.Start();

    SetWinStyle( nWinBits );

    EnableRTL( sal_False );
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

GraphCtrl::GraphCtrl( Window* pParent, const ResId& rResId ) :
            Control         ( pParent, rResId ),
            aMap100         ( MAP_100TH_MM ),
            nWinStyle       ( 0 ),
            eObjKind        ( OBJ_NONE ),
            nPolyEdit       ( 0 ),
            bEditMode       ( sal_False ),
            bSdrMode        ( sal_False ),
            bAnim           ( sal_False ),
            mpAccContext    ( NULL ),
            pModel          ( NULL ),
            pView           ( NULL )
{
    pUserCall = new GraphCtrlUserCall( *this );
    aUpdateTimer.SetTimeout( 500 );
    aUpdateTimer.SetTimeoutHdl( LINK( this, GraphCtrl, UpdateHdl ) );
    aUpdateTimer.Start();
    EnableRTL( sal_False );
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
    ::vos::OGuard aGuard (Application::GetSolarMutex());

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
    pModel->SetChanged( sal_False );

    // View anlegen
    pView = new GraphCtrlView( pModel, this );
    pView->SetWorkArea( Rectangle( Point(), aGraphSize ) );
    pView->EnableExtendedMouseEventDispatcher( sal_True );
    pView->ShowSdrPage(pView->GetModel()->GetPage(0));
//  pView->ShowSdrPage(pView->GetModel()->GetPage(0));
    pView->SetFrameDragSingles( sal_True );
    pView->SetMarkedPointsSmooth( SDRPATHSMOOTH_SYMMETRIC );
    pView->SetEditMode( sal_True );

    // #i72889# set neeeded flags
    pView->SetPagePaintingAllowed(false);
    pView->SetBufferedOutputAllowed(true);
    pView->SetBufferedOverlayAllowed(true);

    // Tell the accessibility object about the changes.
    if (mpAccContext != NULL)
        mpAccContext->setModelAndView (pModel, pView);
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrl::SetGraphic( const Graphic& rGraphic, sal_Bool bNewModel )
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
    // #i72889# used splitted repaint to be able to paint an own background
    // even to the buffered view
    const bool bGraphicValid(GRAPHIC_NONE != aGraphic.GetType());

    if(bSdrMode)
    {
        SdrPaintWindow* pPaintWindow = pView->BeginCompleteRedraw(this);

        if(bGraphicValid)
        {
            OutputDevice& rTarget = pPaintWindow->GetTargetOutputDevice();

            rTarget.SetBackground(GetBackground());
            rTarget.Erase();

            aGraphic.Draw(&rTarget, Point(), aGraphSize);
        }

        const Region aRepaintRegion(rRect);
        pView->DoCompleteRedraw(*pPaintWindow, aRepaintRegion);
        pView->EndCompleteRedraw(*pPaintWindow, true);
    }
    else
    {
        // #i73381# in non-SdrMode, paint to local directly
        if(bGraphicValid)
        {
            aGraphic.Draw(this, Point(), aGraphSize);
        }
    }
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrl::SdrObjChanged( const SdrObject&  )
{
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrl::SdrObjCreated( const SdrObject& )
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
    sal_Bool    bProc = sal_False;

    switch ( aCode.GetCode() )
    {
        case KEY_DELETE:
        case KEY_BACKSPACE:
        {
            if ( bSdrMode )
            {
                pView->DeleteMarked();
                bProc = sal_True;
                if( !pView->AreObjectsMarked() )
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
                else if ( pView->AreObjectsMarked() )
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
                bProc = sal_True;
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
                    bProc = sal_True;
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
                pView->MarkNextObj(sal_False);

                bProc = true;
            }
        }
        break;

        case KEY_HOME:
        {
            if ( aCode.IsMod1() )
            {
                pView->UnmarkAllObj();
                pView->MarkNextObj(sal_True);

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

            if (pView->AreObjectsMarked() && !aCode.IsMod1() )
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
                            sal_Bool bWasSnapEnabled = pView->IsSnapEnabled();

                            // switch snapping off
                            if(!bWasNoSnap)
                                ((SdrDragStat&)rDragStat).SetNoSnap(sal_True);
                            if(bWasSnapEnabled)
                                pView->SetSnapEnabled(sal_False);

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
                    sal_uInt32 nPol(pHdl->GetPolyNum());
                    sal_uInt32 nPnt(pHdl->GetPointNum());

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

                    bProc = sal_True;
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
                    pView->BegInsObjPoint( aLogPt, rMEvt.IsMod1());
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
            !pView->PickHandle( aLogPos ) &&
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
        const SdrMarkList&  rMarkList = pView->GetMarkedObjectList();

        if ( rMarkList.GetMarkCount() == 1 )
            pSdrObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();
    }

    return pSdrObj;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrl::SetEditMode( const sal_Bool _bEditMode )
{
    if ( bSdrMode )
    {
        bEditMode = _bEditMode;
        pView->SetEditMode( bEditMode );
        eObjKind = OBJ_NONE;
        pView->SetCurrentObj( sal::static_int_cast< sal_uInt16 >( eObjKind ) );
    }
    else
        bEditMode = sal_False;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrl::SetPolyEditMode( const sal_uInt16 _nPolyEdit )
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
        bEditMode = sal_False;
        pView->SetEditMode( bEditMode );
        eObjKind = _eObjKind;
        pView->SetCurrentObj( sal::static_int_cast< sal_uInt16 >( eObjKind ) );
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


::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > GraphCtrl::CreateAccessible()
{
    if( mpAccContext == NULL )
    {
        Window* pParent = GetParent();

        DBG_ASSERT( pParent, "-GraphCtrl::CreateAccessible(): No Parent!" );

        if( pParent )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > xAccParent( pParent->GetAccessible() );

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
