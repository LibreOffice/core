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

#include <vcl/scrbar.hxx>
#include <svx/svdview.hxx>
#include "dlgedfunc.hxx"
#include "dlged.hxx"
#include "dlgedview.hxx"
#include <vcl/seleng.hxx>

namespace basctl
{

//----------------------------------------------------------------------------

IMPL_LINK_INLINE_START( DlgEdFunc, ScrollTimeout, Timer *, pTimer )
{
    (void)pTimer;
    Window& rWindow = rParent.GetWindow();
    Point aPos = rWindow.ScreenToOutputPixel( rWindow.GetPointerPosPixel() );
    aPos = rWindow.PixelToLogic( aPos );
    ForceScroll( aPos );
    return 0;
}
IMPL_LINK_INLINE_END( DlgEdFunc, ScrollTimeout, Timer *, pTimer )

//----------------------------------------------------------------------------

void DlgEdFunc::ForceScroll( const Point& rPos )
{
    aScrollTimer.Stop();

    Window& rWindow  = rParent.GetWindow();

    static Point aDefPoint;
    Rectangle aOutRect( aDefPoint, rWindow.GetOutputSizePixel() );
    aOutRect = rWindow.PixelToLogic( aOutRect );

    ScrollBar* pHScroll = rParent.GetHScroll();
    ScrollBar* pVScroll = rParent.GetVScroll();
    long nDeltaX = pHScroll->GetLineSize();
    long nDeltaY = pVScroll->GetLineSize();

    if( !aOutRect.IsInside( rPos ) )
    {
        if( rPos.X() < aOutRect.Left() )
            nDeltaX = -nDeltaX;
        else
        if( rPos.X() <= aOutRect.Right() )
            nDeltaX = 0;

        if( rPos.Y() < aOutRect.Top() )
            nDeltaY = -nDeltaY;
        else
        if( rPos.Y() <= aOutRect.Bottom() )
            nDeltaY = 0;

        if( nDeltaX )
            pHScroll->SetThumbPos( pHScroll->GetThumbPos() + nDeltaX );
        if( nDeltaY )
            pVScroll->SetThumbPos( pVScroll->GetThumbPos() + nDeltaY );

        if( nDeltaX )
            rParent.DoScroll( pHScroll );
        if( nDeltaY )
            rParent.DoScroll( pVScroll );
    }

    aScrollTimer.Start();
}

//----------------------------------------------------------------------------

DlgEdFunc::DlgEdFunc (DlgEditor& rParent_) :
    rParent(rParent_)
{
    aScrollTimer.SetTimeoutHdl( LINK( this, DlgEdFunc, ScrollTimeout ) );
    aScrollTimer.SetTimeout( SELENG_AUTOREPEAT_INTERVAL );
}

//----------------------------------------------------------------------------

DlgEdFunc::~DlgEdFunc()
{
}

//----------------------------------------------------------------------------

bool DlgEdFunc::MouseButtonDown( const MouseEvent& )
{
    return true;
}

//----------------------------------------------------------------------------

bool DlgEdFunc::MouseButtonUp( const MouseEvent& )
{
    aScrollTimer.Stop();
    return true;
}

//----------------------------------------------------------------------------

bool DlgEdFunc::MouseMove( const MouseEvent& )
{
    return true;
}

//----------------------------------------------------------------------------

bool DlgEdFunc::KeyInput( const KeyEvent& rKEvt )
{
    bool bReturn = false;

    SdrView& rView = rParent.GetView();
    Window& rWindow = rParent.GetWindow();

    KeyCode aCode = rKEvt.GetKeyCode();
    sal_uInt16 nCode = aCode.GetCode();

    switch ( nCode )
    {
        case KEY_ESCAPE:
        {
            if ( rView.IsAction() )
            {
                rView.BrkAction();
                bReturn = true;
            }
            else if ( rView.AreObjectsMarked() )
            {
                const SdrHdlList& rHdlList = rView.GetHdlList();
                SdrHdl* pHdl = rHdlList.GetFocusHdl();
                if ( pHdl )
                    const_cast<SdrHdlList&>(rHdlList).ResetFocusHdl();
                else
                    rView.UnmarkAll();

                bReturn = true;
            }
        }
        break;
        case KEY_TAB:
        {
            if ( !aCode.IsMod1() && !aCode.IsMod2() )
            {
                // mark next object
                if ( !rView.MarkNextObj( !aCode.IsShift() ) )
                {
                    // if no next object, mark first/last
                    rView.UnmarkAllObj();
                    rView.MarkNextObj( !aCode.IsShift() );
                }

                if ( rView.AreObjectsMarked() )
                    rView.MakeVisible( rView.GetAllMarkedRect(), rWindow );

                bReturn = true;
            }
            else if ( aCode.IsMod1() )
            {
                // selected handle
                const SdrHdlList& rHdlList = rView.GetHdlList();
                const_cast<SdrHdlList&>(rHdlList).TravelFocusHdl( !aCode.IsShift() );

                // guarantee visibility of focused handle
                if (SdrHdl* pHdl = rHdlList.GetFocusHdl())
                {
                    Point aHdlPosition( pHdl->GetPos() );
                    Rectangle aVisRect( aHdlPosition - Point( 100, 100 ), Size( 200, 200 ) );
                    rView.MakeVisible( aVisRect, rWindow );
                }

                bReturn = true;
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

            if ( nCode == KEY_UP )
            {
                // scroll up
                nX =  0;
                nY = -1;
            }
            else if ( nCode == KEY_DOWN )
            {
                // scroll down
                nX =  0;
                nY =  1;
            }
            else if ( nCode == KEY_LEFT )
            {
                // scroll left
                nX = -1;
                nY =  0;
            }
            else if ( nCode == KEY_RIGHT )
            {
                // scroll right
                nX =  1;
                nY =  0;
            }

            if ( rView.AreObjectsMarked() && !aCode.IsMod1() )
            {
                if ( aCode.IsMod2() )
                {
                    // move in 1 pixel distance
                    Size aPixelSize = rWindow.PixelToLogic(Size(1, 1));
                    nX *= aPixelSize.Width();
                    nY *= aPixelSize.Height();
                }
                else
                {
                    // move in 1 mm distance
                    nX *= 100;
                    nY *= 100;
                }

                const SdrHdlList& rHdlList = rView.GetHdlList();
                SdrHdl* pHdl = rHdlList.GetFocusHdl();

                if ( pHdl == 0 )
                {
                    // no handle selected
                    if ( rView.IsMoveAllowed() )
                    {
                        // restrict movement to work area
                        const Rectangle& rWorkArea = rView.GetWorkArea();

                        if ( !rWorkArea.IsEmpty() )
                        {
                            Rectangle aMarkRect( rView.GetMarkedObjRect() );
                            aMarkRect.Move( nX, nY );

                            if ( !rWorkArea.IsInside( aMarkRect ) )
                            {
                                if ( aMarkRect.Left() < rWorkArea.Left() )
                                    nX += rWorkArea.Left() - aMarkRect.Left();

                                if ( aMarkRect.Right() > rWorkArea.Right() )
                                    nX -= aMarkRect.Right() - rWorkArea.Right();

                                if ( aMarkRect.Top() < rWorkArea.Top() )
                                    nY += rWorkArea.Top() - aMarkRect.Top();

                                if ( aMarkRect.Bottom() > rWorkArea.Bottom() )
                                    nY -= aMarkRect.Bottom() - rWorkArea.Bottom();
                            }
                        }

                        if ( nX != 0 || nY != 0 )
                        {
                            rView.MoveAllMarked( Size( nX, nY ) );
                            rView.MakeVisible( rView.GetAllMarkedRect(), rWindow );
                        }
                    }
                }
                else
                {
                    // move the handle
                    if ( pHdl && ( nX || nY ) )
                    {
                        Point aStartPoint( pHdl->GetPos() );
                        Point aEndPoint( pHdl->GetPos() + Point( nX, nY ) );
                        const SdrDragStat& rDragStat = rView.GetDragStat();

                        // start dragging
                        rView.BegDragObj( aStartPoint, 0, pHdl, 0 );

                        if ( rView.IsDragObj() )
                        {
                            bool const bWasNoSnap = rDragStat.IsNoSnap();
                            bool const bWasSnapEnabled = rView.IsSnapEnabled();

                            // switch snapping off
                            if ( !bWasNoSnap )
                                const_cast<SdrDragStat&>(rDragStat).SetNoSnap(true);
                            if ( bWasSnapEnabled )
                                rView.SetSnapEnabled(false);

                            rView.MovAction( aEndPoint );
                            rView.EndDragObj();

                            // restore snap
                            if ( !bWasNoSnap )
                                const_cast<SdrDragStat&>(rDragStat).SetNoSnap( bWasNoSnap );
                            if ( bWasSnapEnabled )
                                rView.SetSnapEnabled( bWasSnapEnabled );
                        }

                        // make moved handle visible
                        Rectangle aVisRect( aEndPoint - Point( 100, 100 ), Size( 200, 200 ) );
                        rView.MakeVisible( aVisRect, rWindow );
                    }
                }
            }
            else
            {
                // scroll page
                ScrollBar* pScrollBar = ( nX != 0 ) ? rParent.GetHScroll() : rParent.GetVScroll();
                if ( pScrollBar )
                {
                    long nRangeMin = pScrollBar->GetRangeMin();
                    long nRangeMax = pScrollBar->GetRangeMax();
                    long nThumbPos = pScrollBar->GetThumbPos() + ( ( nX != 0 ) ? nX : nY ) * pScrollBar->GetLineSize();
                    if ( nThumbPos < nRangeMin )
                        nThumbPos = nRangeMin;
                    if ( nThumbPos > nRangeMax )
                        nThumbPos = nRangeMax;
                    pScrollBar->SetThumbPos( nThumbPos );
                    rParent.DoScroll( pScrollBar );
                }
            }

            bReturn = true;
        }
        break;
        default:
        {
        }
        break;
    }

    if ( bReturn )
        rWindow.ReleaseMouse();

    return bReturn;
}

//----------------------------------------------------------------------------

DlgEdFuncInsert::DlgEdFuncInsert (DlgEditor& rParent_) :
    DlgEdFunc(rParent_)
{
    rParent.GetView().SetCreateMode(true);
}

//----------------------------------------------------------------------------

DlgEdFuncInsert::~DlgEdFuncInsert()
{
    rParent.GetView().SetEditMode( true );
}

//----------------------------------------------------------------------------

bool DlgEdFuncInsert::MouseButtonDown( const MouseEvent& rMEvt )
{
    if( !rMEvt.IsLeft() )
        return true;

    SdrView& rView  = rParent.GetView();
    Window& rWindow = rParent.GetWindow();
    rView.SetActualWin(&rWindow);

    Point aPos = rWindow.PixelToLogic( rMEvt.GetPosPixel() );
    sal_uInt16 nHitLog = static_cast<sal_uInt16>(rWindow.PixelToLogic(Size(3, 0)).Width());
    sal_uInt16 nDrgLog = static_cast<sal_uInt16>(rWindow.PixelToLogic(Size(3, 0)).Width());

    rWindow.CaptureMouse();

    if ( rMEvt.IsLeft() && rMEvt.GetClicks() == 1 )
    {
        SdrHdl* pHdl = rView.PickHandle(aPos);

        // if selected object was hit, drag object
        if ( pHdl!=NULL || rView.IsMarkedHit(aPos, nHitLog) )
            rView.BegDragObj(aPos, (OutputDevice*) NULL, pHdl, nDrgLog);
        else if ( rView.AreObjectsMarked() )
            rView.UnmarkAll();

        // if no action, create object
        if ( !rView.IsAction() )
            rView.BegCreateObj(aPos);
    }
    else if ( rMEvt.IsLeft() && rMEvt.GetClicks() == 2 )
    {
        // if object was hit, show property browser
        if ( rView.IsMarkedHit(aPos, nHitLog) && rParent.GetMode() != DlgEditor::READONLY )
            rParent.ShowProperties();
    }

    return true;
}

//----------------------------------------------------------------------------

bool DlgEdFuncInsert::MouseButtonUp( const MouseEvent& rMEvt )
{
    DlgEdFunc::MouseButtonUp( rMEvt );

    SdrView& rView  = rParent.GetView();
    Window& rWindow = rParent.GetWindow();
    rView.SetActualWin(&rWindow);

    rWindow.ReleaseMouse();

    // object creation active?
    if ( rView.IsCreateObj() )
    {
        rView.EndCreateObj(SDRCREATE_FORCEEND);

        if ( !rView.AreObjectsMarked() )
        {
            sal_uInt16 nHitLog = static_cast<sal_uInt16>(rWindow.PixelToLogic(Size(3, 0)).Width());
            Point aPos( rWindow.PixelToLogic( rMEvt.GetPosPixel() ) );
            rView.MarkObj(aPos, nHitLog);
        }

        return rView.AreObjectsMarked();
    }
    else
    {
        if ( rView.IsDragObj() )
            rView.EndDragObj( rMEvt.IsMod1() );
        return true;
    }
}

//----------------------------------------------------------------------------

bool DlgEdFuncInsert::MouseMove( const MouseEvent& rMEvt )
{
    SdrView& rView  = rParent.GetView();
    Window& rWindow = rParent.GetWindow();
    rView.SetActualWin(&rWindow);

    Point aPos = rWindow.PixelToLogic(rMEvt.GetPosPixel());
    sal_uInt16 nHitLog = static_cast<sal_uInt16>(rWindow.PixelToLogic(Size(3, 0)).Width());

    if (rView.IsAction())
    {
        ForceScroll(aPos);
        rView.MovAction(aPos);
    }

    rWindow.SetPointer( rView.GetPreferedPointer( aPos, &rWindow, nHitLog ) );

    return true;
}

//----------------------------------------------------------------------------

DlgEdFuncSelect::DlgEdFuncSelect (DlgEditor& rParent_) :
    DlgEdFunc(rParent_),
    bMarkAction(false)
{
}

//----------------------------------------------------------------------------

DlgEdFuncSelect::~DlgEdFuncSelect()
{
}

//----------------------------------------------------------------------------

bool DlgEdFuncSelect::MouseButtonDown( const MouseEvent& rMEvt )
{
    // get view from parent
    SdrView& rView  = rParent.GetView();
    Window& rWindow = rParent.GetWindow();
    rView.SetActualWin(&rWindow);

    sal_uInt16 nDrgLog = static_cast<sal_uInt16>(rWindow.PixelToLogic(Size(3, 0)).Width());
    sal_uInt16 nHitLog = static_cast<sal_uInt16>(rWindow.PixelToLogic(Size(3, 0)).Width());
    Point aMDPos = rWindow.PixelToLogic(rMEvt.GetPosPixel());

    if ( rMEvt.IsLeft() && rMEvt.GetClicks() == 1 )
    {
        SdrHdl* pHdl = rView.PickHandle(aMDPos);
        SdrObject* pObj;
        SdrPageView* pPV;

        // hit selected object?
        if ( pHdl!=NULL || rView.IsMarkedHit(aMDPos, nHitLog) )
        {
            rView.BegDragObj(aMDPos, (OutputDevice*) NULL, pHdl, nDrgLog);
        }
        else
        {
            // if not multi selection, unmark all
            if ( !rMEvt.IsShift() )
                rView.UnmarkAll();
            else
            {
                if( rView.PickObj( aMDPos, nHitLog, pObj, pPV ) )
                {
                    //if (dynamic_cast<DlgEdForm*>(pObj))
                    //  rView.UnmarkAll();
                    //else
                    //  rParent.UnmarkDialog();
                }
            }

            if ( rView.MarkObj(aMDPos, nHitLog) )
            {
                // drag object
                pHdl = rView.PickHandle(aMDPos);
                rView.BegDragObj(aMDPos, (OutputDevice*) NULL, pHdl, nDrgLog);
            }
            else
            {
                // select object
                rView.BegMarkObj(aMDPos);
                bMarkAction = true;
            }
        }
    }
    else if ( rMEvt.IsLeft() && rMEvt.GetClicks() == 2 )
    {
        // if object was hit, show property browser
        if ( rView.IsMarkedHit(aMDPos, nHitLog) && rParent.GetMode() != DlgEditor::READONLY )
            rParent.ShowProperties();
    }

    return true;
}

//----------------------------------------------------------------------------

bool DlgEdFuncSelect::MouseButtonUp( const MouseEvent& rMEvt )
{
    DlgEdFunc::MouseButtonUp( rMEvt );

    // get view from parent
    SdrView& rView  = rParent.GetView();
    Window& rWindow = rParent.GetWindow();
    rView.SetActualWin(&rWindow);

    Point aPnt = rWindow.PixelToLogic(rMEvt.GetPosPixel());
    sal_uInt16 nHitLog = static_cast<sal_uInt16>(rWindow.PixelToLogic(Size(3, 0)).Width());

    if ( rMEvt.IsLeft() )
    {
        if (rView.IsDragObj())
        {
            // object was dragged
            rView.EndDragObj( rMEvt.IsMod1() );
            rView.ForceMarkedToAnotherPage();
        }
        else if (rView.IsAction())
        {
            rView.EndAction();
        }
    }

    bMarkAction = false;

    rWindow.SetPointer( rView.GetPreferedPointer( aPnt, &rWindow, nHitLog ) );
    rWindow.ReleaseMouse();

    return true;
}

//----------------------------------------------------------------------------

bool DlgEdFuncSelect::MouseMove( const MouseEvent& rMEvt )
{
    SdrView& rView  = rParent.GetView();
    Window& rWindow = rParent.GetWindow();
    rView.SetActualWin(&rWindow);

    Point aPnt = rWindow.PixelToLogic(rMEvt.GetPosPixel());
    sal_uInt16 nHitLog = static_cast<sal_uInt16>(rWindow.PixelToLogic(Size(3, 0)).Width());

    if ( rView.IsAction() )
    {
        Point aPix = rMEvt.GetPosPixel();
        Point aPnt_ = rWindow.PixelToLogic(aPix);

        ForceScroll(aPnt_);
        rView.MovAction(aPnt_);
    }

    rWindow.SetPointer( rView.GetPreferedPointer( aPnt, &rWindow, nHitLog ) );

    return true;
}

//----------------------------------------------------------------------------

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
