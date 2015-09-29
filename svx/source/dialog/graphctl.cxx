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

#include <svl/itempool.hxx>
#include <vcl/dialog.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>
#include <unotools/syslocale.hxx>
#include <rtl/math.hxx>
#include <unotools/localedatawrapper.hxx>
#include <comphelper/processfactory.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>

#include <svx/graphctl.hxx>
#include "GraphCtlAccessibleContext.hxx"
#include "svx/xoutbmp.hxx"
#include <svx/svxids.hrc>
#include <svx/svdpage.hxx>
#include "svx/sdrpaintwindow.hxx"

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

GraphCtrl::GraphCtrl( vcl::Window* pParent, WinBits nStyle ) :
            Control         ( pParent, nStyle ),
            aMap100         ( MAP_100TH_MM ),
            nWinStyle       ( 0 ),
            eObjKind        ( OBJ_NONE ),
            nPolyEdit       ( 0 ),
            bEditMode       ( false ),
            bSdrMode        ( false ),
            bAnim           ( false ),
            mpAccContext    ( NULL ),
            pModel          ( NULL ),
            pView           ( NULL )
{
    pUserCall = new GraphCtrlUserCall( *this );
    aUpdateIdle.SetPriority( SchedulerPriority::LOWEST );
    aUpdateIdle.SetIdleHdl( LINK( this, GraphCtrl, UpdateHdl ) );
    aUpdateIdle.Start();
    EnableRTL( false );
}

VCL_BUILDER_DECL_FACTORY(GraphCtrl)
{
    WinBits nWinStyle = 0;
    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nWinStyle |= WB_BORDER;
    rRet = VclPtr<GraphCtrl>::Create(pParent, nWinStyle);
}

GraphCtrl::~GraphCtrl()
{
    disposeOnce();
}

void GraphCtrl::dispose()
{
    aUpdateIdle.Stop();

    if( mpAccContext )
    {
        mpAccContext->disposing();
        mpAccContext->release();
    }
    delete pView;
    pView = NULL;
    delete pModel;
    pModel = NULL;
    delete pUserCall;
    pUserCall = NULL;
    Control::dispose();
}

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

void GraphCtrl::InitSdrModel()
{
    SolarMutexGuard aGuard;

    SdrPage* pPage;

    // destroy old junk
    delete pView;
    delete pModel;

    // Creating a Model
    pModel = new SdrModel;
    pModel->GetItemPool().FreezeIdRanges();
    pModel->SetScaleUnit( aMap100.GetMapUnit() );
    pModel->SetScaleFraction( Fraction( 1, 1 ) );
    pModel->SetDefaultFontHeight( 500 );

    pPage = new SdrPage( *pModel );

    pPage->SetSize( aGraphSize );
    pPage->SetBorder( 0, 0, 0, 0 );
    pModel->InsertPage( pPage );
    pModel->SetChanged( false );

    // Creating a View
    pView = new GraphCtrlView( pModel, this );
    pView->SetWorkArea( Rectangle( Point(), aGraphSize ) );
    pView->EnableExtendedMouseEventDispatcher( true );
    pView->ShowSdrPage(pView->GetModel()->GetPage(0));
    pView->SetFrameDragSingles();
    pView->SetMarkedPointsSmooth( SDRPATHSMOOTH_SYMMETRIC );
    pView->SetEditMode();

    // #i72889# set needed flags
    pView->SetPagePaintingAllowed(false);
    pView->SetBufferedOutputAllowed(true);
    pView->SetBufferedOverlayAllowed(true);

    // Tell the accessibility object about the changes.
    if (mpAccContext != NULL)
        mpAccContext->setModelAndView (pModel, pView);
}

void GraphCtrl::SetGraphic( const Graphic& rGraphic, bool bNewModel )
{
    // If possible we dither bitmaps for the display
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

        // Adapt Bitmap to Thumb size
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

        // Implementing MapMode for Engine
        aDisplayMap.SetScaleX( Fraction( aNewSize.Width(), aGraphSize.Width() ) );
        aDisplayMap.SetScaleY( Fraction( aNewSize.Height(), aGraphSize.Height() ) );

        aDisplayMap.SetOrigin( LogicToLogic( aNewPos, aMap100, aDisplayMap ) );
        SetMapMode( aDisplayMap );
    }

    Invalidate();
}

void GraphCtrl::Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect)
{
    // #i72889# used splitted repaint to be able to paint an own background
    // even to the buffered view
    const bool bGraphicValid(GRAPHIC_NONE != aGraphic.GetType());

    if (bSdrMode)
    {
        SdrPaintWindow* pPaintWindow = pView->BeginCompleteRedraw(&rRenderContext);

        if (bGraphicValid)
        {
            vcl::RenderContext& rTarget = pPaintWindow->GetTargetOutputDevice();

            rTarget.SetBackground(GetBackground());
            rTarget.Erase();

            aGraphic.Draw(&rTarget, Point(), aGraphSize);
        }

        const vcl::Region aRepaintRegion(rRect);
        pView->DoCompleteRedraw(*pPaintWindow, aRepaintRegion);
        pView->EndCompleteRedraw(*pPaintWindow, true);
    }
    else
    {
        // #i73381# in non-SdrMode, paint to local directly
        if(bGraphicValid)
        {
            aGraphic.Draw(&rRenderContext, Point(), aGraphSize);
        }
    }
}

void GraphCtrl::SdrObjChanged( const SdrObject&  )
{
}

void GraphCtrl::SdrObjCreated( const SdrObject& )
{
}

void GraphCtrl::MarkListHasChanged()
{
}

void GraphCtrl::KeyInput( const KeyEvent& rKEvt )
{
    vcl::KeyCode aCode( rKEvt.GetKeyCode() );
    bool    bProc = false;

    Dialog* pDialog = GetParentDialog();

    switch ( aCode.GetCode() )
    {
        case KEY_DELETE:
        case KEY_BACKSPACE:
        {
            if ( bSdrMode )
            {
                pView->DeleteMarked();
                bProc = true;
                if (!pView->AreObjectsMarked() && pDialog)
                    pDialog->GrabFocusToFirstControl();
            }
        }
        break;

        case KEY_ESCAPE:
        {
            if ( bSdrMode )
            {
                bool bGrabFocusToFirstControl = true;
                if ( pView->IsAction() )
                {
                    pView->BrkAction();
                    bGrabFocusToFirstControl = false;
                }
                else if ( pView->AreObjectsMarked() )
                {
                    const SdrHdlList& rHdlList = pView->GetHdlList();
                    SdrHdl* pHdl = rHdlList.GetFocusHdl();

                    if(pHdl)
                    {
                        ((SdrHdlList&)rHdlList).ResetFocusHdl();
                        bGrabFocusToFirstControl = false;
                    }
                }
                if (bGrabFocusToFirstControl && pDialog)
                    pDialog->GrabFocusToFirstControl();
                bProc = true;
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
                    bProc = true;
                }
                else if(aCode.IsMod1())
                {
                    // select next handle
                    const SdrHdlList& rHdlList = pView->GetHdlList();
                    bool bForward(!aCode.IsShift());

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
                // mark last object
                pView->UnmarkAllObj();
                pView->MarkNextObj();

                bProc = true;
            }
        }
        break;

        case KEY_HOME:
        {
            if ( aCode.IsMod1() )
            {
                pView->UnmarkAllObj();
                pView->MarkNextObj(true);

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
                // Scroll up
                nX = 0;
                nY =-1;
            }
            else if (aCode.GetCode() == KEY_DOWN)
            {
                // Scroll down
                nX = 0;
                nY = 1;
            }
            else if (aCode.GetCode() == KEY_LEFT)
            {
                // Scroll left
                nX =-1;
                nY = 0;
            }
            else if (aCode.GetCode() == KEY_RIGHT)
            {
                // Scroll right
                nX = 1;
                nY = 0;
            }

            if (pView->AreObjectsMarked() && !aCode.IsMod1() )
            {
                if(aCode.IsMod2())
                {
                    // move in 1 pixel distance
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

                // II
                const SdrHdlList& rHdlList = pView->GetHdlList();
                SdrHdl* pHdl = rHdlList.GetFocusHdl();

                if(0L == pHdl)
                {
                    // restrict movement to WorkArea
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
                            bool bWasNoSnap = rDragStat.IsNoSnap();
                            bool bWasSnapEnabled = pView->IsSnapEnabled();

                            // switch snapping off
                            if(!bWasNoSnap)
                                ((SdrDragStat&)rDragStat).SetNoSnap();
                            if(bWasSnapEnabled)
                                pView->SetSnapEnabled(false);

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

                        for(size_t a = 0; !pNewOne && a < rHdlList.GetHdlCount(); ++a)
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

                    bProc = true;
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

void GraphCtrl::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( bSdrMode && ( rMEvt.GetClicks() < 2 ) )
    {
        const Point aLogPt( PixelToLogic( rMEvt.GetPosPixel() ) );

        if ( !Rectangle( Point(), aGraphSize ).IsInside( aLogPt ) && !pView->IsEditMode() )
            Control::MouseButtonDown( rMEvt );
        else
        {
            // Get Focus for key inputs
            GrabFocus();

            if ( nPolyEdit )
            {
                SdrViewEvent    aVEvt;
                SdrHitKind      eHit = pView->PickAnything( rMEvt, SdrMouseEventKind::BUTTONDOWN, aVEvt );

                if ( nPolyEdit == SID_BEZIER_INSERT && eHit == SDRHIT_MARKEDOBJECT )
                    pView->BegInsObjPoint( aLogPt, rMEvt.IsMod1());
                else
                    pView->MouseButtonDown( rMEvt, this );
            }
            else
                pView->MouseButtonDown( rMEvt, this );
        }

        SdrObject* pCreateObj = pView->GetCreateObj();

        // We want to realize the insert
        if ( pCreateObj && !pCreateObj->GetUserCall() )
            pCreateObj->SetUserCall( pUserCall );

        SetPointer( pView->GetPreferredPointer( aLogPt, this ) );
    }
    else
        Control::MouseButtonDown( rMEvt );
}

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
            SetPointer( PointerStyle::Cross );
        }
        else
            SetPointer( pView->GetPreferredPointer( aLogPos, this ) );
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

void GraphCtrl::MouseButtonUp(const MouseEvent& rMEvt)
{
    if ( bSdrMode )
    {
        if ( pView->IsInsObjPoint() )
            pView->EndInsObjPoint( SDRCREATE_FORCEEND );
        else
            pView->MouseButtonUp( rMEvt, this );

        ReleaseMouse();
        SetPointer( pView->GetPreferredPointer( PixelToLogic( rMEvt.GetPosPixel() ), this ) );
    }
    else
        Control::MouseButtonUp( rMEvt );
}

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

void GraphCtrl::SetEditMode( const bool _bEditMode )
{
    if ( bSdrMode )
    {
        bEditMode = _bEditMode;
        pView->SetEditMode( bEditMode );
        eObjKind = OBJ_NONE;
        pView->SetCurrentObj( sal::static_int_cast< sal_uInt16 >( eObjKind ) );
    }
    else
        bEditMode = false;
}

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

void GraphCtrl::SetObjKind( const SdrObjKind _eObjKind )
{
    if ( bSdrMode )
    {
        bEditMode = false;
        pView->SetEditMode( bEditMode );
        eObjKind = _eObjKind;
        pView->SetCurrentObj( sal::static_int_cast< sal_uInt16 >( eObjKind ) );
    }
    else
        eObjKind = OBJ_NONE;
}

IMPL_LINK_TYPED( GraphCtrl, UpdateHdl, Idle*, pTimer, void )
{
    if ( aUpdateLink.IsSet() )
        aUpdateLink.Call( this );

    pTimer->Start();
}


::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > GraphCtrl::CreateAccessible()
{
    if( mpAccContext == NULL )
    {
        vcl::Window* pParent = GetParent();

        DBG_ASSERT( pParent, "-GraphCtrl::CreateAccessible(): No Parent!" );

        if( pParent )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > xAccParent( pParent->GetAccessible() );

            // Disable accessibility if no model/view data available
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
