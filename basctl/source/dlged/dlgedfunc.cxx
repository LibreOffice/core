/*************************************************************************
 *
 *  $RCSfile: dlgedfunc.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:54:53 $
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

#ifndef _SV_SCRBAR_HXX
#include <vcl/scrbar.hxx>
#endif
#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
#endif
#pragma hdrstop

#ifndef _BASCTL_DLGEDFUNC_HXX
#include "dlgedfunc.hxx"
#endif

#ifndef _BASCTL_DLGED_HXX
#include "dlged.hxx"
#endif

#ifndef _BASCTL_DLGEDVIEW_HXX
#include "dlgedview.hxx"
#endif

#ifndef _SV_SELENG_HXX //autogen
#include <vcl/seleng.hxx>
#endif


//----------------------------------------------------------------------------

IMPL_LINK_INLINE_START( DlgEdFunc, ScrollTimeout, Timer *, pTimer )
{
    Window* pWindow = pParent->GetWindow();
    Point aPos = pWindow->ScreenToOutputPixel( pWindow->GetPointerPosPixel() );
    aPos = pWindow->PixelToLogic( aPos );
    ForceScroll( aPos );
    return 0;
}
IMPL_LINK_INLINE_END( DlgEdFunc, ScrollTimeout, Timer *, pTimer )

//----------------------------------------------------------------------------

void DlgEdFunc::ForceScroll( const Point& rPos )
{
    aScrollTimer.Stop();

    Window* pWindow  = pParent->GetWindow();

    static Point aDefPoint;
    Rectangle aOutRect( aDefPoint, pWindow->GetOutputSizePixel() );
    aOutRect = pWindow->PixelToLogic( aOutRect );

    ScrollBar* pHScroll = pParent->GetHScroll();
    ScrollBar* pVScroll = pParent->GetVScroll();
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
            pParent->DoScroll( pHScroll );
        if( nDeltaY )
            pParent->DoScroll( pVScroll );
    }

    aScrollTimer.Start();
}

//----------------------------------------------------------------------------

DlgEdFunc::DlgEdFunc( DlgEditor* pParent )
{
    DlgEdFunc::pParent = pParent;
    aScrollTimer.SetTimeoutHdl( LINK( this, DlgEdFunc, ScrollTimeout ) );
    aScrollTimer.SetTimeout( SELENG_AUTOREPEAT_INTERVAL );
}

//----------------------------------------------------------------------------

DlgEdFunc::~DlgEdFunc()
{
}

//----------------------------------------------------------------------------

BOOL DlgEdFunc::MouseButtonDown( const MouseEvent& rMEvt )
{
#ifdef MAC
    pParent->GetWindow()->GrabFocus();
#endif
    return TRUE;
}

//----------------------------------------------------------------------------

BOOL DlgEdFunc::MouseButtonUp( const MouseEvent& rMEvt )
{
    aScrollTimer.Stop();
    return TRUE;
}

//----------------------------------------------------------------------------

BOOL DlgEdFunc::MouseMove( const MouseEvent& rMEvt )
{
    return TRUE;
}

//----------------------------------------------------------------------------

BOOL DlgEdFunc::KeyInput( const KeyEvent& rKEvt )
{
    BOOL bReturn = FALSE;

    SdrView* pView = pParent->GetView();
    Window* pWindow = pParent->GetWindow();

    KeyCode aCode = rKEvt.GetKeyCode();
    USHORT nCode = aCode.GetCode();

    switch ( nCode )
    {
        case KEY_ESCAPE:
        {
            if ( pView->IsAction() )
            {
                pView->BrkAction();
                bReturn = TRUE;
            }
            else if ( pView->AreObjectsMarked() )
            {
                const SdrHdlList& rHdlList = pView->GetHdlList();
                SdrHdl* pHdl = rHdlList.GetFocusHdl();
                if ( pHdl )
                    ((SdrHdlList&)rHdlList).ResetFocusHdl();
                else
                    pView->UnmarkAll();

                bReturn = TRUE;
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

                if ( pView->AreObjectsMarked() )
                    pView->MakeVisible( pView->GetAllMarkedRect(), *pWindow );

                bReturn = TRUE;
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
                    Point aHdlPosition( pHdl->GetPos() );
                    Rectangle aVisRect( aHdlPosition - Point( 100, 100 ), Size( 200, 200 ) );
                    pView->MakeVisible( aVisRect, *pWindow );
                }

                bReturn = TRUE;
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

            if ( pView->AreObjectsMarked() && !aCode.IsMod1() )
            {
                if ( aCode.IsMod2() )
                {
                    // move in 1 pixel distance
                    Size aPixelSize = pWindow ? pWindow->PixelToLogic( Size( 1, 1 ) ) : Size( 100, 100 );
                    nX *= aPixelSize.Width();
                    nY *= aPixelSize.Height();
                }
                else
                {
                    // move in 1 mm distance
                    nX *= 100;
                    nY *= 100;
                }

                const SdrHdlList& rHdlList = pView->GetHdlList();
                SdrHdl* pHdl = rHdlList.GetFocusHdl();

                if ( pHdl == 0 )
                {
                    // no handle selected
                    if ( pView->IsMoveAllowed() )
                    {
                        // restrict movement to work area
                        const Rectangle& rWorkArea = pView->GetWorkArea();

                        if ( !rWorkArea.IsEmpty() )
                        {
                            Rectangle aMarkRect( pView->GetMarkedObjRect() );
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
                            pView->MoveAllMarked( Size( nX, nY ) );
                            pView->MakeVisible( pView->GetAllMarkedRect(), *pWindow );
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
                        const SdrDragStat& rDragStat = pView->GetDragStat();

                        // start dragging
                        pView->BegDragObj( aStartPoint, 0, pHdl, 0 );

                        if ( pView->IsDragObj() )
                        {
                            FASTBOOL bWasNoSnap = rDragStat.IsNoSnap();
                            BOOL bWasSnapEnabled = pView->IsSnapEnabled();

                            // switch snapping off
                            if ( !bWasNoSnap )
                                ((SdrDragStat&)rDragStat).SetNoSnap( TRUE );
                            if ( bWasSnapEnabled )
                                pView->SetSnapEnabled( FALSE );

                            pView->MovAction( aEndPoint );
                            pView->EndDragObj();

                            // restore snap
                            if ( !bWasNoSnap )
                                ((SdrDragStat&)rDragStat).SetNoSnap( bWasNoSnap );
                            if ( bWasSnapEnabled )
                                pView->SetSnapEnabled( bWasSnapEnabled );
                        }

                        // make moved handle visible
                        Rectangle aVisRect( aEndPoint - Point( 100, 100 ), Size( 200, 200 ) );
                        pView->MakeVisible( aVisRect, *pWindow );
                    }
                }
            }
            else
            {
                // scroll page
                ScrollBar* pScrollBar = ( nX != 0 ) ? pParent->GetHScroll() : pParent->GetVScroll();
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
                    pParent->DoScroll( pScrollBar );
                }
            }

            bReturn = TRUE;
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

DlgEdFuncInsert::DlgEdFuncInsert( DlgEditor* pParent ) :
    DlgEdFunc( pParent )
{
    pParent->GetView()->SetCreateMode( TRUE );
}

//----------------------------------------------------------------------------

DlgEdFuncInsert::~DlgEdFuncInsert()
{
    pParent->GetView()->SetEditMode( TRUE );
}

//----------------------------------------------------------------------------

BOOL DlgEdFuncInsert::MouseButtonDown( const MouseEvent& rMEvt )
{
    if( !rMEvt.IsLeft() )
        return TRUE;

    SdrView* pView  = pParent->GetView();
    Window*  pWindow= pParent->GetWindow();
    pView->SetActualWin( pWindow );

    Point aPos = pWindow->PixelToLogic( rMEvt.GetPosPixel() );
    USHORT nHitLog = USHORT ( pWindow->PixelToLogic(Size(3,0)).Width() );
    USHORT nDrgLog = USHORT ( pWindow->PixelToLogic(Size(3,0)).Width() );

#ifdef MAC
    pWindow->GrabFocus();
#endif
    pWindow->CaptureMouse();

    if ( rMEvt.IsLeft() && rMEvt.GetClicks() == 1 )
    {
        SdrHdl* pHdl = pView->HitHandle(aPos, *pWindow);

        // if selected object was hit, drag object
        if ( pHdl!=NULL || pView->IsMarkedHit(aPos, nHitLog) )
            pView->BegDragObj(aPos, (OutputDevice*) NULL, pHdl, nDrgLog);
        else if ( pView->AreObjectsMarked() )
            pView->UnmarkAll();

        // if no action, create object
        if ( !pView->IsAction() )
            pView->BegCreateObj(aPos);
    }
    else if ( rMEvt.IsLeft() && rMEvt.GetClicks() == 2 )
    {
        // if object was hit, show property browser
        if ( pView->IsMarkedHit(aPos, nHitLog) && pParent->GetMode() != DLGED_READONLY )
            pParent->ShowProperties();
    }

    return TRUE;
}

//----------------------------------------------------------------------------

BOOL DlgEdFuncInsert::MouseButtonUp( const MouseEvent& rMEvt )
{
    DlgEdFunc::MouseButtonUp( rMEvt );

    SdrView* pView  = pParent->GetView();
    Window*  pWindow= pParent->GetWindow();
    pView->SetActualWin( pWindow );

    Point   aPos( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
    USHORT nHitLog = USHORT ( pWindow->PixelToLogic(Size(3,0)).Width() );

    pWindow->ReleaseMouse();

    // object creation active?
    if ( pView->IsCreateObj() )
    {
        pView->EndCreateObj(SDRCREATE_FORCEEND);

        const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
        if(rMarkList.GetMarkCount() == 1)
        {
            SdrMark* pMark = rMarkList.GetMark(0);
            SdrObject* pObj = pMark->GetObj();
        }

        if ( !pView->AreObjectsMarked() )
        {
            USHORT nHitLog = USHORT ( pWindow->PixelToLogic(Size(3,0)).Width() );
            pView->MarkObj(aPos, nHitLog);
        }

        if( pView->AreObjectsMarked() )
            return TRUE;
        else
            return FALSE;
    }
    else
    {
        if ( pView->IsDragObj() )
             pView->EndDragObj( rMEvt.IsMod1() );
        return TRUE;
    }
}

//----------------------------------------------------------------------------

BOOL DlgEdFuncInsert::MouseMove( const MouseEvent& rMEvt )
{
    SdrView* pView  = pParent->GetView();
    Window*  pWindow= pParent->GetWindow();
    pView->SetActualWin( pWindow );

    Point   aPos( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
    USHORT nHitLog = USHORT ( pWindow->PixelToLogic(Size(3,0)).Width() );

    if ( pView->IsAction() )
    {
        ForceScroll(aPos);
        pView->MovAction(aPos);
    }

    pWindow->SetPointer( pView->GetPreferedPointer( aPos, pWindow, nHitLog ) );

    return TRUE;
}

//----------------------------------------------------------------------------

DlgEdFuncSelect::DlgEdFuncSelect( DlgEditor* pParent ) :
    DlgEdFunc( pParent ),
    bMarkAction(FALSE)
{
}

//----------------------------------------------------------------------------

DlgEdFuncSelect::~DlgEdFuncSelect()
{
}

//----------------------------------------------------------------------------

BOOL DlgEdFuncSelect::MouseButtonDown( const MouseEvent& rMEvt )
{
    // get view from parent
    SdrView* pView   = pParent->GetView();
    Window*  pWindow = pParent->GetWindow();
    pView->SetActualWin( pWindow );

    USHORT nDrgLog = USHORT ( pWindow->PixelToLogic(Size(3,0)).Width() );
    USHORT nHitLog = USHORT ( pWindow->PixelToLogic(Size(3,0)).Width() );
    Point  aMDPos = pWindow->PixelToLogic( rMEvt.GetPosPixel() );

    if ( rMEvt.IsLeft() && rMEvt.GetClicks() == 1 )
    {
#ifdef MAC
        pWindow->GrabFocus();
#endif
        SdrHdl* pHdl = pView->HitHandle(aMDPos, *pWindow );
        SdrObject* pObj;
        SdrPageView* pPV;

        // hit selected object?
        if ( pHdl!=NULL || pView->IsMarkedHit(aMDPos, nHitLog) )
        {
            pView->BegDragObj(aMDPos, (OutputDevice*) NULL, pHdl, nDrgLog);
        }
        else
        {
            // if not multi selection, unmark all
            if ( !rMEvt.IsShift() )
                pView->UnmarkAll();
            else
            {
                if( pView->PickObj( aMDPos, nHitLog, pObj, pPV ) )
                {
                    //if( pObj->ISA( DlgEdForm ) )
                    //  pView->UnmarkAll();
                    //else
                    //  pParent->UnmarkDialog();
                }
            }

            if ( pView->MarkObj(aMDPos, nHitLog) )
            {
                // drag object
                pHdl=pView->HitHandle(aMDPos, *pWindow);
                pView->BegDragObj(aMDPos, (OutputDevice*) NULL, pHdl, nDrgLog);
            }
            else
            {
                // select object
                pView->BegMarkObj(aMDPos, (OutputDevice*) NULL);
                bMarkAction = TRUE;
            }
        }
    }
    else if ( rMEvt.IsLeft() && rMEvt.GetClicks() == 2 )
    {
        // if object was hit, show property browser
        if ( pView->IsMarkedHit(aMDPos, nHitLog) && pParent->GetMode() != DLGED_READONLY )
            pParent->ShowProperties();
    }

    return TRUE;
}

//----------------------------------------------------------------------------

BOOL DlgEdFuncSelect::MouseButtonUp( const MouseEvent& rMEvt )
{
    DlgEdFunc::MouseButtonUp( rMEvt );

    // get view from parent
    SdrView* pView  = pParent->GetView();
    Window*  pWindow= pParent->GetWindow();
    pView->SetActualWin( pWindow );

    Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
    USHORT nHitLog = USHORT ( pWindow->PixelToLogic(Size(3,0)).Width() );
    ULONG nView = 0;


    if ( rMEvt.IsLeft() )
    {
        if ( pView->IsDragObj() )
        {
            // object was dragged
            pView->EndDragObj( rMEvt.IsMod1() );
            pView->ForceMarkedToAnotherPage();
        }
        else
        if (pView->IsAction() )
        {
            pView->EndAction();
            //if( bMarkAction )
                //pParent->UnmarkDialog();
        }
    }


    USHORT nClicks = rMEvt.GetClicks();

//  if (nClicks == 2)
//  {
//      if ( pView->AreObjectsMarked() )
//      {
//          const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
//
//          if (rMarkList.GetMarkCount() == 1)
//          {
//              SdrMark* pMark = rMarkList.GetMark(0);
//              SdrObject* pObj = pMark->GetObj();
//
//              // edit objects here
//          }
//      }
//  }

    bMarkAction = FALSE;

    pWindow->SetPointer( pView->GetPreferedPointer( aPnt, pWindow, nHitLog ) );
    pWindow->ReleaseMouse();

    return TRUE;
}

//----------------------------------------------------------------------------

BOOL DlgEdFuncSelect::MouseMove( const MouseEvent& rMEvt )
{
    SdrView* pView  = pParent->GetView();
    Window*  pWindow= pParent->GetWindow();
    pView->SetActualWin( pWindow );

    Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
    USHORT nHitLog = USHORT ( pWindow->PixelToLogic(Size(3,0)).Width() );

    if ( pView->IsAction() )
    {
        Point aPix(rMEvt.GetPosPixel());
        Point aPnt(pWindow->PixelToLogic(aPix));

        ForceScroll(aPnt);
        pView->MovAction(aPnt);
    }

    pWindow->SetPointer( pView->GetPreferedPointer( aPnt, pWindow, nHitLog ) );

    return TRUE;
}

//----------------------------------------------------------------------------
