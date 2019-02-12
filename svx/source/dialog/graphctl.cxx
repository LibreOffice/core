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
#include <vcl/ptrstyle.hxx>
#include <unotools/syslocale.hxx>
#include <rtl/math.hxx>
#include <unotools/localedatawrapper.hxx>
#include <vcl/svapp.hxx>

#include <svx/graphctl.hxx>
#include <GraphCtlAccessibleContext.hxx>
#include <svx/xoutbmp.hxx>
#include <svx/svxids.hrc>
#include <svx/svdpage.hxx>
#include <svx/sdrpaintwindow.hxx>

void GraphCtrlUserCall::Changed( const SdrObject& rObj, SdrUserCallType eType, const tools::Rectangle& /*rOldBoundRect*/ )
{
    switch( eType )
    {
        case SdrUserCallType::MoveOnly:
        case SdrUserCallType::Resize:
            rWin.SdrObjChanged( rObj );
        break;

        case SdrUserCallType::Inserted:
            rWin.SdrObjCreated( rObj );
        break;

        default:
        break;
    }
    rWin.QueueIdleUpdate();
}

GraphCtrl::GraphCtrl( vcl::Window* pParent, WinBits nStyle ) :
            Control         ( pParent, nStyle ),
            aUpdateIdle     ( "svx GraphCtrl Update" ),
            aMap100         ( MapUnit::Map100thMM ),
            eObjKind        ( OBJ_NONE ),
            nPolyEdit       ( 0 ),
            bEditMode       ( false ),
            mbSdrMode       ( false ),
            mbInIdleUpdate  ( false )
{
    pUserCall.reset(new GraphCtrlUserCall( *this ));
    aUpdateIdle.SetPriority( TaskPriority::LOWEST );
    aUpdateIdle.SetInvokeHandler( LINK( this, GraphCtrl, UpdateHdl ) );
    aUpdateIdle.Start();
    EnableRTL( false );
}

GraphCtrl::~GraphCtrl()
{
    disposeOnce();
}

void GraphCtrl::dispose()
{
    aUpdateIdle.Stop();

    if( mpAccContext.is() )
    {
        mpAccContext->disposing();
        mpAccContext.clear();
    }
    pView.reset();
    pModel.reset();
    pUserCall.reset();
    Control::dispose();
}

void GraphCtrl::SetSdrMode(bool bSdrMode)
{
    mbSdrMode = bSdrMode;

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    SetBackground( Wallpaper( rStyleSettings.GetWindowColor() ) );
    SetMapMode( aMap100 );

    pView.reset();
    pModel.reset();

    if ( mbSdrMode )
        InitSdrModel();

    QueueIdleUpdate();
}

void GraphCtrl::InitSdrModel()
{
    SolarMutexGuard aGuard;

    SdrPage* pPage;

    // destroy old junk
    pView.reset();
    pModel.reset();

    // Creating a Model
    pModel.reset(new SdrModel(nullptr, nullptr, true));
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
    pView.reset(new GraphCtrlView(*pModel, this));
    pView->SetWorkArea( tools::Rectangle( Point(), aGraphSize ) );
    pView->EnableExtendedMouseEventDispatcher( true );
    pView->ShowSdrPage(pView->GetModel()->GetPage(0));
    pView->SetFrameDragSingles();
    pView->SetMarkedPointsSmooth( SdrPathSmoothKind::Symmetric );
    pView->SetEditMode();

    // #i72889# set needed flags
    pView->SetPagePaintingAllowed(false);
    pView->SetBufferedOutputAllowed(true);
    pView->SetBufferedOverlayAllowed(true);

    // Tell the accessibility object about the changes.
    if (mpAccContext.is())
        mpAccContext->setModelAndView (pModel.get(), pView.get());
}

void GraphCtrl::SetGraphic( const Graphic& rGraphic, bool bNewModel )
{
    aGraphic = rGraphic;

    if ( aGraphic.GetPrefMapMode().GetMapUnit() == MapUnit::MapPixel )
        aGraphSize = Application::GetDefaultDevice()->PixelToLogic( aGraphic.GetPrefSize(), aMap100 );
    else
        aGraphSize = OutputDevice::LogicToLogic( aGraphic.GetPrefSize(), aGraphic.GetPrefMapMode(), aMap100 );

    if ( mbSdrMode && bNewModel )
        InitSdrModel();

    aGraphSizeLink.Call( this );

    Resize();
    Invalidate();
    QueueIdleUpdate();
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
        double          fGrfWH = static_cast<double>(aGraphSize.Width()) / aGraphSize.Height();
        double          fWinWH = static_cast<double>(nWidth) / nHeight;

        // Adapt Bitmap to Thumb size
        if ( fGrfWH < fWinWH)
        {
            aNewSize.setWidth( static_cast<long>( static_cast<double>(nHeight) * fGrfWH ) );
            aNewSize.setHeight( nHeight );
        }
        else
        {
            aNewSize.setWidth( nWidth );
            aNewSize.setHeight( static_cast<long>( static_cast<double>(nWidth) / fGrfWH ) );
        }

        aNewPos.setX( ( nWidth - aNewSize.Width() )  >> 1 );
        aNewPos.setY( ( nHeight - aNewSize.Height() ) >> 1 );

        // Implementing MapMode for Engine
        aDisplayMap.SetScaleX( Fraction( aNewSize.Width(), aGraphSize.Width() ) );
        aDisplayMap.SetScaleY( Fraction( aNewSize.Height(), aGraphSize.Height() ) );

        aDisplayMap.SetOrigin( LogicToLogic( aNewPos, aMap100, aDisplayMap ) );
        SetMapMode( aDisplayMap );
    }

    Invalidate();
}

void GraphCtrl::Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    // #i72889# used split repaint to be able to paint an own background
    // even to the buffered view
    const bool bGraphicValid(GraphicType::NONE != aGraphic.GetType());

    if (mbSdrMode)
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
    QueueIdleUpdate();
}

void GraphCtrl::SdrObjCreated( const SdrObject& )
{
    QueueIdleUpdate();
}

void GraphCtrl::MarkListHasChanged()
{
    QueueIdleUpdate();
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
            if ( mbSdrMode )
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
            if ( mbSdrMode )
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
                        const_cast<SdrHdlList&>(rHdlList).ResetFocusHdl();
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
            if( mbSdrMode )
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

                    const_cast<SdrHdlList&>(rHdlList).TravelFocusHdl(bForward);

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

                if(nullptr == pHdl)
                {
                    // restrict movement to WorkArea
                    const tools::Rectangle& rWorkArea = pView->GetWorkArea();

                    if(!rWorkArea.IsEmpty())
                    {
                        tools::Rectangle aMarkRect(pView->GetMarkedObjRect());
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
                    if (nX || nY)
                    {
                        // now move the Handle (nX, nY)
                        Point aStartPoint(pHdl->GetPos());
                        Point aEndPoint(pHdl->GetPos() + Point(nX, nY));
                        const SdrDragStat& rDragStat = pView->GetDragStat();

                        // start dragging
                        pView->BegDragObj(aStartPoint, nullptr, pHdl, 0);

                        if(pView->IsDragObj())
                        {
                            bool bWasNoSnap = rDragStat.IsNoSnap();
                            bool bWasSnapEnabled = pView->IsSnapEnabled();

                            // switch snapping off
                            if(!bWasNoSnap)
                                const_cast<SdrDragStat&>(rDragStat).SetNoSnap();
                            if(bWasSnapEnabled)
                                pView->SetSnapEnabled(false);

                            pView->MovAction(aEndPoint);
                            pView->EndDragObj();

                            // restore snap
                            if(!bWasNoSnap)
                                const_cast<SdrDragStat&>(rDragStat).SetNoSnap(bWasNoSnap);
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
                if(pHdl->GetKind() == SdrHdlKind::Poly)
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

                    if(nullptr == rHdlList.GetFocusHdl())
                    {
                        // restore point with focus
                        SdrHdl* pNewOne = nullptr;

                        for(size_t a = 0; !pNewOne && a < rHdlList.GetHdlCount(); ++a)
                        {
                            SdrHdl* pAct = rHdlList.GetHdl(a);

                            if(pAct
                                && pAct->GetKind() == SdrHdlKind::Poly
                                && pAct->GetPolyNum() == nPol
                                && pAct->GetPointNum() == nPnt)
                            {
                                pNewOne = pAct;
                            }
                        }

                        if(pNewOne)
                        {
                            const_cast<SdrHdlList&>(rHdlList).SetFocusHdl(pNewOne);
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

    QueueIdleUpdate();
}

void GraphCtrl::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( mbSdrMode && ( rMEvt.GetClicks() < 2 ) )
    {
        const Point aLogPt( PixelToLogic( rMEvt.GetPosPixel() ) );

        if ( !tools::Rectangle( Point(), aGraphSize ).IsInside( aLogPt ) && !pView->IsEditMode() )
            Control::MouseButtonDown( rMEvt );
        else
        {
            // Get Focus for key inputs
            GrabFocus();

            if ( nPolyEdit )
            {
                SdrViewEvent    aVEvt;
                SdrHitKind      eHit = pView->PickAnything( rMEvt, SdrMouseEventKind::BUTTONDOWN, aVEvt );

                if ( nPolyEdit == SID_BEZIER_INSERT && eHit == SdrHitKind::MarkedObject )
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
            pCreateObj->SetUserCall( pUserCall.get() );

        SetPointer( pView->GetPreferredPointer( aLogPt, this ) );
    }
    else
        Control::MouseButtonDown( rMEvt );

    QueueIdleUpdate();
}

void GraphCtrl::MouseMove(const MouseEvent& rMEvt)
{
    const Point aLogPos( PixelToLogic( rMEvt.GetPosPixel() ) );

    if ( mbSdrMode )
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
        if ( tools::Rectangle( Point(), aGraphSize ).IsInside( aLogPos ) )
            aMousePos = aLogPos;
        else
            aMousePos = Point();

        aMousePosLink.Call( this );
    }

    QueueIdleUpdate();
}

void GraphCtrl::MouseButtonUp(const MouseEvent& rMEvt)
{
    if ( mbSdrMode )
    {
        if ( pView->IsInsObjPoint() )
            pView->EndInsObjPoint( SdrCreateCmd::ForceEnd );
        else
            pView->MouseButtonUp( rMEvt, this );

        ReleaseMouse();
        SetPointer( pView->GetPreferredPointer( PixelToLogic( rMEvt.GetPosPixel() ), this ) );
    }
    else
        Control::MouseButtonUp( rMEvt );

    QueueIdleUpdate();
}

SdrObject* GraphCtrl::GetSelectedSdrObject() const
{
    SdrObject* pSdrObj = nullptr;

    if ( mbSdrMode )
    {
        const SdrMarkList&  rMarkList = pView->GetMarkedObjectList();

        if ( rMarkList.GetMarkCount() == 1 )
            pSdrObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();
    }

    return pSdrObj;
}

void GraphCtrl::SetEditMode( const bool _bEditMode )
{
    if ( mbSdrMode )
    {
        bEditMode = _bEditMode;
        pView->SetEditMode( bEditMode );
        eObjKind = OBJ_NONE;
        pView->SetCurrentObj( sal::static_int_cast< sal_uInt16 >( eObjKind ) );
    }
    else
        bEditMode = false;

    QueueIdleUpdate();
}

void GraphCtrl::SetPolyEditMode( const sal_uInt16 _nPolyEdit )
{
    if ( mbSdrMode && ( _nPolyEdit != nPolyEdit ) )
    {
        nPolyEdit = _nPolyEdit;
        pView->SetFrameDragSingles( nPolyEdit == 0 );
    }
    else
        nPolyEdit = 0;

    QueueIdleUpdate();
}

void GraphCtrl::SetObjKind( const SdrObjKind _eObjKind )
{
    if ( mbSdrMode )
    {
        bEditMode = false;
        pView->SetEditMode( bEditMode );
        eObjKind = _eObjKind;
        pView->SetCurrentObj( sal::static_int_cast< sal_uInt16 >( eObjKind ) );
    }
    else
        eObjKind = OBJ_NONE;

    QueueIdleUpdate();
}

IMPL_LINK( GraphCtrl, UpdateHdl, Timer *, , void )
{
    mbInIdleUpdate = true;
    aUpdateLink.Call( this );
    mbInIdleUpdate = false;
}

void GraphCtrl::QueueIdleUpdate()
{
    if (!mbInIdleUpdate)
        aUpdateIdle.Start();
}

css::uno::Reference< css::accessibility::XAccessible > GraphCtrl::CreateAccessible()
{
    if( mpAccContext == nullptr )
    {
        vcl::Window* pParent = GetParent();

        DBG_ASSERT( pParent, "-GraphCtrl::CreateAccessible(): No Parent!" );

        if( pParent )
        {
            css::uno::Reference< css::accessibility::XAccessible > xAccParent( pParent->GetAccessible() );

            // Disable accessibility if no model/view data available
            if( pView &&
                pModel &&
                xAccParent.is() )
            {
                mpAccContext = new SvxGraphCtrlAccessibleContext( xAccParent, *this );
            }
        }
    }

    return mpAccContext.get();
}

SvxGraphCtrl::SvxGraphCtrl()
    : aMap100(MapUnit::Map100thMM)
{
}

void SvxGraphCtrl::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    rRenderContext.Erase();
    const bool bGraphicValid(GraphicType::NONE != aGraphic.GetType());
    // #i73381# in non-SdrMode, paint to local directly
    if (bGraphicValid && aGraphSize.Width() && aGraphSize.Height())
    {
        MapMode         aDisplayMap( aMap100 );
        Point           aNewPos;
        Size            aNewSize;
        const Size      aWinSize = Application::GetDefaultDevice()->PixelToLogic(GetOutputSizePixel(), aMap100);
        const long      nWidth = aWinSize.Width();
        const long      nHeight = aWinSize.Height();
        double          fGrfWH = static_cast<double>(aGraphSize.Width()) / aGraphSize.Height();
        double          fWinWH = static_cast<double>(nWidth) / nHeight;

        // Adapt Bitmap to Thumb size
        if ( fGrfWH < fWinWH)
        {
            aNewSize.setWidth( static_cast<long>( static_cast<double>(nHeight) * fGrfWH ) );
            aNewSize.setHeight( nHeight );
        }
        else
        {
            aNewSize.setWidth( nWidth );
            aNewSize.setHeight( static_cast<long>( static_cast<double>(nWidth) / fGrfWH ) );
        }

        aNewPos.setX( ( nWidth - aNewSize.Width() )  >> 1 );
        aNewPos.setY( ( nHeight - aNewSize.Height() ) >> 1 );

        // Implementing MapMode for Engine
        aDisplayMap.SetScaleX( Fraction( aNewSize.Width(), aGraphSize.Width() ) );
        aDisplayMap.SetScaleY( Fraction( aNewSize.Height(), aGraphSize.Height() ) );

        aDisplayMap.SetOrigin(OutputDevice::LogicToLogic(aNewPos, aMap100, aDisplayMap));
        rRenderContext.SetMapMode(aDisplayMap);

        aGraphic.Draw(&rRenderContext, Point(), aGraphSize);
    }
}

SvxGraphCtrl::~SvxGraphCtrl()
{
}

void SvxGraphCtrl::SetGraphic(const Graphic& rGraphic)
{
    aGraphic = rGraphic;

    if ( aGraphic.GetPrefMapMode().GetMapUnit() == MapUnit::MapPixel )
        aGraphSize = Application::GetDefaultDevice()->PixelToLogic( aGraphic.GetPrefSize(), aMap100 );
    else
        aGraphSize = OutputDevice::LogicToLogic( aGraphic.GetPrefSize(), aGraphic.GetPrefMapMode(), aMap100 );

    Invalidate();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
