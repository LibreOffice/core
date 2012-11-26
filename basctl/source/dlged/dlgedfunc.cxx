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
#include "precompiled_basctl.hxx"
#include <vcl/scrbar.hxx>
#include <svx/svdview.hxx>
#include "dlgedfunc.hxx"
#include "dlged.hxx"
#include "dlgedview.hxx"
#include <vcl/seleng.hxx>


//----------------------------------------------------------------------------

IMPL_LINK_INLINE_START( DlgEdFunc, ScrollTimeout, Timer *, pTimer )
{
    (void)pTimer;
    Window* pWindow = pParent->GetWindow();
    const basegfx::B2DPoint aPixelPos(pWindow->GetPointerPosPixel().X(), pWindow->GetPointerPosPixel().Y());
    const basegfx::B2DPoint aLogicPos(pWindow->GetInverseViewTransformation() * aPixelPos);

    ForceScroll( aLogicPos );

    return 0;
}
IMPL_LINK_INLINE_END( DlgEdFunc, ScrollTimeout, Timer *, pTimer )

//----------------------------------------------------------------------------

void DlgEdFunc::ForceScroll( const basegfx::B2DPoint& rPos )
{
    aScrollTimer.Stop();

    Window* pWindow  = pParent->GetWindow();
    const basegfx::B2DRange aOutRangeDiscrete(pWindow->GetDiscreteRange());
    const basegfx::B2DRange aOutRangeLogic(pWindow->GetLogicRange());
    ScrollBar* pHScroll = pParent->GetHScroll();
    ScrollBar* pVScroll = pParent->GetVScroll();
    long nDeltaX = pHScroll->GetLineSize();
    long nDeltaY = pVScroll->GetLineSize();

    if( !aOutRangeLogic.isInside( rPos ) )
    {
        if( rPos.getX() < aOutRangeLogic.getMinX() )
            nDeltaX = -nDeltaX;
        else if( rPos.getX() <= aOutRangeLogic.getMaxX() )
            nDeltaX = 0;

        if( rPos.getY() < aOutRangeLogic.getMinY() )
            nDeltaY = -nDeltaY;
        else if( rPos.getY() <= aOutRangeLogic.getMaxY() )
            nDeltaY = 0;

        if( nDeltaX )
            pHScroll->SetThumbPos( pHScroll->GetThumbPos() + nDeltaX );
        if( nDeltaY )
            pVScroll->SetThumbPos( pVScroll->GetThumbPos() + nDeltaY );

        if( nDeltaX )
            pParent->DoScroll( pHScroll );
        if( nDeltaY )
            pParent->DoScroll( pVScroll );
    }

    aScrollTimer.Start();
}

//----------------------------------------------------------------------------

DlgEdFunc::DlgEdFunc( DlgEditor* pParent_ )
{
    DlgEdFunc::pParent = pParent_;
    aScrollTimer.SetTimeoutHdl( LINK( this, DlgEdFunc, ScrollTimeout ) );
    aScrollTimer.SetTimeout( SELENG_AUTOREPEAT_INTERVAL );
}

//----------------------------------------------------------------------------

DlgEdFunc::~DlgEdFunc()
{
}

//----------------------------------------------------------------------------

sal_Bool DlgEdFunc::MouseButtonDown( const MouseEvent& )
{
    return sal_True;
}

//----------------------------------------------------------------------------

sal_Bool DlgEdFunc::MouseButtonUp( const MouseEvent& )
{
    aScrollTimer.Stop();
    return sal_True;
}

//----------------------------------------------------------------------------

sal_Bool DlgEdFunc::MouseMove( const MouseEvent& )
{
    return sal_True;
}

//----------------------------------------------------------------------------

sal_Bool DlgEdFunc::KeyInput( const KeyEvent& rKEvt )
{
    sal_Bool bReturn = sal_False;

    SdrView* pView = pParent->GetView();
    Window* pWindow = pParent->GetWindow();

    KeyCode aCode = rKEvt.GetKeyCode();
    sal_uInt16 nCode = aCode.GetCode();

    switch ( nCode )
    {
        case KEY_ESCAPE:
        {
            if ( pView->IsAction() )
            {
                pView->BrkAction();
                bReturn = sal_True;
            }
            else if ( pView->areSdrObjectsSelected() )
            {
                const SdrHdlList& rHdlList = pView->GetHdlList();
                SdrHdl* pHdl = rHdlList.GetFocusHdl();
                if ( pHdl )
                    ((SdrHdlList&)rHdlList).ResetFocusHdl();
                else
                    pView->UnmarkAll();

                bReturn = sal_True;
            }
        }
        break;
        case KEY_TAB:
        {
            if ( !aCode.IsMod1() && !aCode.IsMod2() )
            {
                // mark next object
                if ( !pView->MarkNextObj( !aCode.IsShift() ) )
                {
                    // if no next object, mark first/last
                    pView->UnmarkAllObj();
                    pView->MarkNextObj( !aCode.IsShift() );
                }

                if ( pView->areSdrObjectsSelected() )
                    pView->MakeVisibleAtView( pView->getMarkedObjectSnapRange(), *pWindow );

                bReturn = sal_True;
            }
            else if ( aCode.IsMod1() )
            {
                // selected handle
                const SdrHdlList& rHdlList = pView->GetHdlList();
                ((SdrHdlList&)rHdlList).TravelFocusHdl( !aCode.IsShift() );

                // guarantee visibility of focused handle
                SdrHdl* pHdl = rHdlList.GetFocusHdl();
                if ( pHdl )
                {
                    const basegfx::B2DRange aRange(
                        pHdl->getPosition() - basegfx::B2DPoint(100.0, 100.0),
                        pHdl->getPosition() + basegfx::B2DPoint(100.0, 100.0));

                    pView->MakeVisibleAtView( aRange, *pWindow );
                }

                bReturn = sal_True;
            }
        }
        break;
        case KEY_UP:
        case KEY_DOWN:
        case KEY_LEFT:
        case KEY_RIGHT:
        {
            basegfx::B2DVector aMove(0.0, 0.0);

            if ( nCode == KEY_UP )
            {
                // scroll up
                aMove = basegfx::B2DVector(0.0, -1.0);
            }
            else if ( nCode == KEY_DOWN )
            {
                // scroll down
                aMove = basegfx::B2DVector(0.0, 1.0);
            }
            else if ( nCode == KEY_LEFT )
            {
                // scroll left
                aMove = basegfx::B2DVector(-1.0, 0.0);
            }
            else if ( nCode == KEY_RIGHT )
            {
                // scroll right
                aMove = basegfx::B2DVector(1.0, 0.0);
            }

            if ( pView->areSdrObjectsSelected() && !aCode.IsMod1() )
            {
                if ( aCode.IsMod2() )
                {
                    // move in 1 pixel distance
                    aMove *= pWindow
                        ? pWindow->GetInverseViewTransformation() * basegfx::B2DVector(1.0, 1.0)
                        : basegfx::B2DVector(100.0, 100.0);
                }
                else
                {
                    // move in 1 mm distance
                    aMove *= 100.0;
                }

                const SdrHdlList& rHdlList = pView->GetHdlList();
                SdrHdl* pHdl = rHdlList.GetFocusHdl();

                if ( pHdl == 0 )
                {
                    // no handle selected
                    if ( pView->IsMoveAllowed() )
                    {
                        // restrict movement to work area
                        const basegfx::B2DRange& rWorkArea = pView->GetWorkArea();

                        if ( !rWorkArea.isEmpty() )
                        {
                            basegfx::B2DRange aMarkRange( pView->getMarkedObjectSnapRange() );
                            aMarkRange.transform(basegfx::tools::createTranslateB2DHomMatrix(aMove));

                            if ( !rWorkArea.isInside( aMarkRange ) )
                            {
                                if ( aMarkRange.getMinX() < rWorkArea.getMinX() )
                                {
                                    aMove.setX(aMove.getX() + rWorkArea.getMinX() - aMarkRange.getMinX());
                                }

                                if ( aMarkRange.getMaxX() > rWorkArea.getMaxX() )
                                {
                                    aMove.setX(aMove.getX() - aMarkRange.getMaxX() - rWorkArea.getMaxX());
                                }

                                if ( aMarkRange.getMinY() < rWorkArea.getMinY() )
                                {
                                    aMove.setY(aMove.getY() + rWorkArea.getMinY() - aMarkRange.getMinY());
                                }

                                if ( aMarkRange.getMaxY() > rWorkArea.getMaxY() )
                                {
                                    aMove.setY(aMove.getY() - aMarkRange.getMaxY() - rWorkArea.getMaxY());
                                }
                            }
                        }

                        if ( !aMove.equalZero() )
                        {
                            pView->MoveMarkedObj( aMove );
                            pView->MakeVisibleAtView( pView->getMarkedObjectSnapRange(), *pWindow );
                        }
                    }
                }
                else
                {
                    // move the handle
                    if ( pHdl && !aMove.equalZero() )
                    {
                        const basegfx::B2DPoint aStartPoint( pHdl->getPosition() );
                        const basegfx::B2DPoint aEndPoint( pHdl->getPosition() + aMove );
                        const SdrDragStat& rDragStat = pView->GetDragStat();

                        // start dragging
                        pView->BegDragObj( aStartPoint, pHdl, 0.0 );

                        if ( pView->IsDragObj() )
                        {
                            const bool bWasNoSnap(rDragStat.IsNoSnap());
                            const bool bWasSnapEnabled(pView->IsSnapEnabled());

                            // switch snapping off
                            if ( !bWasNoSnap )
                            {
                                ((SdrDragStat&)rDragStat).SetNoSnap(true);
                            }

                            if ( bWasSnapEnabled )
                            {
                                pView->SetSnapEnabled( false );
                            }

                            pView->MovAction( aEndPoint );
                            pView->EndDragObj();

                            // restore snap
                            if ( !bWasNoSnap )
                            {
                                ((SdrDragStat&)rDragStat).SetNoSnap( bWasNoSnap );
                            }

                            if ( bWasSnapEnabled )
                            {
                                pView->SetSnapEnabled( bWasSnapEnabled );
                            }
                        }

                        // make moved handle visible
                        const basegfx::B2DRange aVisRange(
                            aEndPoint - basegfx::B2DPoint(100.0, 100.0),
                            aEndPoint + basegfx::B2DPoint(100.0, 100.0));

                        pView->MakeVisibleAtView( aVisRange, *pWindow );
                    }
                }
            }
            else
            {
                // scroll page
                const bool bHorUsed(!basegfx::fTools::equalZero(aMove.getY()));
                ScrollBar* pScrollBar = bHorUsed ? pParent->GetHScroll() : pParent->GetVScroll();

                if ( pScrollBar )
                {
                    long nRangeMin = pScrollBar->GetRangeMin();
                    long nRangeMax = pScrollBar->GetRangeMax();
                    long nThumbPos = pScrollBar->GetThumbPos() +
                        basegfx::fround( bHorUsed ? aMove.getX() : aMove.getY() ) *
                        pScrollBar->GetLineSize();

                    if ( nThumbPos < nRangeMin )
                        nThumbPos = nRangeMin;
                    if ( nThumbPos > nRangeMax )
                        nThumbPos = nRangeMax;

                    pScrollBar->SetThumbPos( nThumbPos );
                    pParent->DoScroll( pScrollBar );
                }
            }

            bReturn = sal_True;
        }
        break;
        default:
        {
        }
        break;
    }

    if ( bReturn )
        pWindow->ReleaseMouse();

    return bReturn;
}

//----------------------------------------------------------------------------

DlgEdFuncInsert::DlgEdFuncInsert( DlgEditor* pParent_ ) :
    DlgEdFunc( pParent_ )
{
    pParent_->GetView()->SetViewEditMode(SDREDITMODE_CREATE);
}

//----------------------------------------------------------------------------

DlgEdFuncInsert::~DlgEdFuncInsert()
{
    pParent->GetView()->SetViewEditMode(SDREDITMODE_EDIT);
}

//----------------------------------------------------------------------------

sal_Bool DlgEdFuncInsert::MouseButtonDown( const MouseEvent& rMEvt )
{
    if( !rMEvt.IsLeft() )
        return sal_True;

    SdrView* pView  = pParent->GetView();
    Window*  pWindow= pParent->GetWindow();
    pView->SetActualOutDev( pWindow );

    const basegfx::B2DPoint aPixelPos(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
    const basegfx::B2DPoint aLogicPos(pWindow->GetInverseViewTransformation() * aPixelPos);
    const double fTolerance(basegfx::B2DVector(pWindow->GetInverseViewTransformation() *
        basegfx::B2DVector(3.0, 0.0)).getLength());

    pWindow->CaptureMouse();

    if ( rMEvt.IsLeft() && rMEvt.GetClicks() == 1 )
    {
        SdrHdl* pHdl = pView->PickHandle(aLogicPos);

        // if selected object was hit, drag object
        if ( pHdl!=NULL || pView->IsMarkedObjHit(aLogicPos, fTolerance) )
            pView->BegDragObj(aLogicPos, pHdl, fTolerance);
        else if ( pView->areSdrObjectsSelected() )
            pView->UnmarkAll();

        // if no action, create object
        if ( !pView->IsAction() )
            pView->BegCreateObj(aLogicPos);
    }
    else if ( rMEvt.IsLeft() && rMEvt.GetClicks() == 2 )
    {
        // if object was hit, show property browser
        if ( pView->IsMarkedObjHit(aLogicPos, fTolerance) && pParent->GetMode() != DLGED_READONLY )
            pParent->ShowProperties();
    }

    return sal_True;
}

//----------------------------------------------------------------------------

sal_Bool DlgEdFuncInsert::MouseButtonUp( const MouseEvent& rMEvt )
{
    DlgEdFunc::MouseButtonUp( rMEvt );

    SdrView* pView  = pParent->GetView();
    Window*  pWindow= pParent->GetWindow();
    pView->SetActualOutDev( pWindow );

    pWindow->ReleaseMouse();

    // object creation active?
    if ( pView->GetCreateObj() )
    {
        pView->EndCreateObj(SDRCREATE_FORCEEND);

        if ( !pView->areSdrObjectsSelected() )
        {
            const double fTolerance(basegfx::B2DVector(pWindow->GetInverseViewTransformation() *
                basegfx::B2DVector(3.0, 0.0)).getLength());
            const basegfx::B2DPoint aPixelPos(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
            const basegfx::B2DPoint aLogicPos(pWindow->GetInverseViewTransformation() * aPixelPos);

            pView->MarkObj(aLogicPos, fTolerance);
        }

        if( pView->areSdrObjectsSelected() )
            return sal_True;
        else
            return sal_False;
    }
    else
    {
        if ( pView->IsDragObj() )
             pView->EndDragObj( rMEvt.IsMod1() );
        return sal_True;
    }
}

//----------------------------------------------------------------------------

sal_Bool DlgEdFuncInsert::MouseMove( const MouseEvent& rMEvt )
{
    SdrView* pView  = pParent->GetView();
    Window*  pWindow= pParent->GetWindow();
    pView->SetActualOutDev( pWindow );

    const basegfx::B2DPoint aPixelPos(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
    const basegfx::B2DPoint aLogicPos(pWindow->GetInverseViewTransformation() * aPixelPos);

    if ( pView->IsAction() )
    {
        ForceScroll(aLogicPos);
        pView->MovAction(aLogicPos);
    }

    pWindow->SetPointer(pView->GetPreferedPointer(aLogicPos, pWindow, rMEvt.GetModifier()));

    return sal_True;
}

//----------------------------------------------------------------------------

DlgEdFuncSelect::DlgEdFuncSelect( DlgEditor* pParent_ ) :
    DlgEdFunc( pParent_ ),
    bMarkAction(sal_False)
{
}

//----------------------------------------------------------------------------

DlgEdFuncSelect::~DlgEdFuncSelect()
{
}

//----------------------------------------------------------------------------

sal_Bool DlgEdFuncSelect::MouseButtonDown( const MouseEvent& rMEvt )
{
    // get view from parent
    SdrView* pView   = pParent->GetView();
    Window*  pWindow = pParent->GetWindow();
    pView->SetActualOutDev( pWindow );

    const basegfx::B2DPoint aPixelPos(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
    const basegfx::B2DPoint aLogicPos(pWindow->GetInverseViewTransformation() * aPixelPos);
    const double fTolerance(basegfx::B2DVector(pWindow->GetInverseViewTransformation() *
        basegfx::B2DVector(3.0, 0.0)).getLength());

    if ( rMEvt.IsLeft() && rMEvt.GetClicks() == 1 )
    {
        SdrHdl* pHdl = pView->PickHandle(aLogicPos);
        SdrObject* pObj;

        // hit selected object?
        if ( pHdl!=NULL || pView->IsMarkedObjHit(aLogicPos, fTolerance) )
        {
            pView->BegDragObj(aLogicPos, pHdl, fTolerance);
        }
        else
        {
            // if not multi selection, unmark all
            if ( !rMEvt.IsShift() )
                pView->UnmarkAll();
            else
            {
                if( pView->PickObj( aLogicPos, fTolerance, pObj ) )
                {
                    //if( dynamic_cast< DlgEdForm* >(pObj) )
                    //  pView->UnmarkAll();
                    //else
                    //  pParent->UnmarkDialog();
                }
            }

            if ( pView->MarkObj(aLogicPos, fTolerance) )
            {
                // drag object
                pHdl=pView->PickHandle(aLogicPos);
                pView->BegDragObj(aLogicPos, pHdl, fTolerance);
            }
            else
            {
                // select object
                pView->BegMarkObj(aLogicPos);
                bMarkAction = sal_True;
            }
        }
    }
    else if ( rMEvt.IsLeft() && rMEvt.GetClicks() == 2 )
    {
        // if object was hit, show property browser
        if ( pView->IsMarkedObjHit(aLogicPos, fTolerance) && pParent->GetMode() != DLGED_READONLY )
            pParent->ShowProperties();
    }

    return sal_True;
}

//----------------------------------------------------------------------------

sal_Bool DlgEdFuncSelect::MouseButtonUp( const MouseEvent& rMEvt )
{
    DlgEdFunc::MouseButtonUp( rMEvt );

    // get view from parent
    SdrView* pView  = pParent->GetView();
    Window*  pWindow= pParent->GetWindow();
    pView->SetActualOutDev( pWindow );

    const basegfx::B2DPoint aPixelPos(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
    const basegfx::B2DPoint aLogicPos(pWindow->GetInverseViewTransformation() * aPixelPos);

    if ( rMEvt.IsLeft() )
    {
        if ( pView->IsDragObj() )
        {
            // object was dragged
            pView->EndDragObj( rMEvt.IsMod1() );
        }
        else if (pView->IsAction() )
        {
            pView->EndAction();
            //if( bMarkAction )
                //pParent->UnmarkDialog();
        }
    }

    bMarkAction = sal_False;
    pWindow->SetPointer(pView->GetPreferedPointer(aLogicPos, pWindow, rMEvt.GetModifier()));
    pWindow->ReleaseMouse();

    return sal_True;
}

//----------------------------------------------------------------------------

sal_Bool DlgEdFuncSelect::MouseMove( const MouseEvent& rMEvt )
{
    SdrView* pView  = pParent->GetView();
    Window*  pWindow= pParent->GetWindow();
    pView->SetActualOutDev( pWindow );

    const basegfx::B2DPoint aPixelPos(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
    const basegfx::B2DPoint aLogicPos(pWindow->GetInverseViewTransformation() * aPixelPos);

    if ( pView->IsAction() )
    {
        ForceScroll(aLogicPos);
        pView->MovAction(aLogicPos);
    }

    pWindow->SetPointer(pView->GetPreferedPointer(aLogicPos, pWindow, rMEvt.GetModifier()));

    return sal_True;
}

//----------------------------------------------------------------------------
