/*************************************************************************
 *
 *  $RCSfile: dlgedfunc.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: tbe $ $Date: 2001-03-07 18:08:43 $
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

#ifndef _BASCTL_DLGED_HXX
#include "dlged.hxx"
#endif

#ifndef _BASCTL_DLGEDFUNC_HXX
#include "dlgedfunc.hxx"
#endif

#ifndef _BASCTL_DLGEDOBJ_HXX
#include "dlgedobj.hxx"
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

DlgEdFunc::DlgEdFunc( VCDlgEditor* pParent )
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
    if( rKEvt.GetKeyCode().GetCode() == KEY_ESCAPE )
    {
        if( pParent->GetView() )
        {
            pParent->GetView()->BrkAction();
            return TRUE;
        }
    }
    return FALSE;
}

//----------------------------------------------------------------------------

DlgEdFuncInsert::DlgEdFuncInsert( VCDlgEditor* pParent ) :
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

    SdrHdl* pHdl = pView->HitHandle(aPos, *pWindow);

    // Selektiertes Object getroffen ?
    // Wenn ja, draggen
    if ( pHdl!=NULL || pView->IsMarkedHit(aPos, nHitLog) )
        pView->BegDragObj(aPos, (OutputDevice*) NULL, pHdl, nDrgLog);
    else
    if ( pView->HasMarkedObj() )
        pView->UnmarkAll();

    // Keine Aktion im Gange ?
    // Dann Object erzeugen
    if ( rMEvt.IsLeft() && !pView->IsAction() )
        pView->BegCreateObj(aPos);

    return TRUE;

    /*
    if( !rMEvt.IsLeft() )
        return TRUE;

    SdrView* pView  = pParent->GetView();
    Window*  pWindow= pParent->GetWindow();
    pView->SetActualWin( pWindow );

    //pView->MouseButtonDown( rMEvt, pWindow );

    const Point aLogPt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

    // Focus anziehen fuer Key-Inputs
    pWindow->GrabFocus();

    pView->MouseButtonDown( rMEvt, pWindow );


    SdrObject* pCreateObj = pView->GetCreateObj();

    // Wir wollen das Inserten mitbekommen
    //if ( pCreateObj && !pCreateObj->GetUserCall() )
    //  pCreateObj->SetUserCall( pUserCall );

    pWindow->SetPointer( pView->GetPreferedPointer( aLogPt, pWindow ) );

    return TRUE; // delete this
    */

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

    // Object erzeugen aktiv ?
    if ( pView->IsCreateObj() )
    {
        pView->EndCreateObj(SDRCREATE_FORCEEND);

        const SdrMarkList& rMarkList = pView->GetMarkList();
        if(rMarkList.GetMarkCount() == 1)
        {
            SdrMark* pMark = rMarkList.GetMark(0);
            SdrObject* pObj = pMark->GetObj();

            DlgEdObj* pDlgEdObj = PTR_CAST(DlgEdObj, pObj);
            if ( pDlgEdObj && !pDlgEdObj->ISA(DlgEdForm) )
            {
                sal_Int32 nCurStep = pDlgEdObj->GetDlgEdForm()->GetStep();
                pDlgEdObj->SetStep( nCurStep );
                pDlgEdObj->UpdateStep();
            }
        }

        if ( !pView->HasMarkedObj() )
        {
            USHORT nHitLog = USHORT ( pWindow->PixelToLogic(Size(3,0)).Width() );
            pView->MarkObj(aPos, nHitLog);
        }

        if( pView->HasMarkedObj() )
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


    /*
    SdrView* pView  = pParent->GetView();
    Window*  pWindow= pParent->GetWindow();
    pView->SetActualWin( pWindow );

    //pView->MouseButtonUp( rMEvt, pWindow );


    //if ( bSdrMode )
    //{
        if ( pView->IsInsObjPoint() )
            pView->EndInsObjPoint( SDRCREATE_FORCEEND );
        else
            pView->MouseButtonUp( rMEvt, pWindow );

        pWindow->ReleaseMouse();
        pWindow->SetPointer( pView->GetPreferedPointer( pWindow->PixelToLogic( rMEvt.GetPosPixel() ), pWindow ) );
    //}
    //else
    //  Control::MouseButtonUp( rMEvt );


    return TRUE; // delete this
    */

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

    /*
    SdrView* pView  = pParent->GetView();
    Window*  pWindow= pParent->GetWindow();
    pView->SetActualWin( pWindow );

    //pView->MouseMove( rMEvt, pWindow );


    const Point aLogPos( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

    //if ( bSdrMode )
    //{
        pView->MouseMove( rMEvt, pWindow );

        //if( ( SID_BEZIER_INSERT == nPolyEdit ) &&
        //  !pView->HitHandle( aLogPos, *this ) &&
        //  !pView->IsInsObjPoint() )
        //{
        //  SetPointer( POINTER_CROSS );
        //}
        //else
            pWindow->SetPointer( pView->GetPreferedPointer( aLogPos, pWindow ) );
    //}
    //else
    //  Control::MouseButtonUp( rMEvt );

    //if ( aMousePosLink.IsSet() )
    //{
    //  if ( Rectangle( Point(), aGraphSize ).IsInside( aLogPos ) )
    //      aMousePos = aLogPos;
    //  else
    //      aMousePos = Point();

    //  aMousePosLink.Call( this );
    //}

    return TRUE; // delete this
    */

}

//----------------------------------------------------------------------------

DlgEdFuncSelect::DlgEdFuncSelect( VCDlgEditor* pParent ) :
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
    // View vom Parent holen
    SdrView* pView   = pParent->GetView();
    Window*  pWindow = pParent->GetWindow();
    pView->SetActualWin( pWindow );

    USHORT nDrgLog = USHORT ( pWindow->PixelToLogic(Size(3,0)).Width() );
    USHORT nHitLog = USHORT ( pWindow->PixelToLogic(Size(3,0)).Width() );
    Point  aMDPos = pWindow->PixelToLogic( rMEvt.GetPosPixel() );

    if ( rMEvt.IsLeft() )
    {
#ifdef MAC
        pWindow->GrabFocus();
#endif
        SdrHdl* pHdl = pView->HitHandle(aMDPos, *pWindow );
        SdrObject* pObj;
        SdrPageView* pPV;

        // Selektiertes Object getroffen ?
        if ( pHdl!=NULL || pView->IsMarkedHit(aMDPos, nHitLog) )
        {
            pView->BegDragObj(aMDPos, (OutputDevice*) NULL, pHdl, nDrgLog);
        }
        else
        {
            // Wenn nicht Multi-Select, dann alles deselektieren
            if ( !rMEvt.IsShift() )
                pView->UnmarkAll();
            else
            {
                if( pView->PickObj( aMDPos, nHitLog, pObj, pPV ) )
                {

                    if( pObj->ISA( DlgEdForm ) )
                        pView->UnmarkAll();
                    else
                        pParent->UnmarkDialog();

                    /*
                    if( pObj->IsA( TYPE( VCSbxDialogObject ) ) )
                        pView->UnmarkAll();
                    else
                        pParent->UnmarkDialog();
                    */
                }
            }

            if ( pView->MarkObj(aMDPos, nHitLog) )
            {
                /**********************************************************
                * Objekt verschieben
                **********************************************************/
                pHdl=pView->HitHandle(aMDPos, *pWindow);
                pView->BegDragObj(aMDPos, (OutputDevice*) NULL, pHdl, nDrgLog);
            }
            else
            {
                /**********************************************************
                * Objekt selektieren
                **********************************************************/
                pView->BegMarkObj(aMDPos, (OutputDevice*) NULL);
                bMarkAction = TRUE;
            }
        }
    }

    return TRUE;
}

//----------------------------------------------------------------------------

BOOL DlgEdFuncSelect::MouseButtonUp( const MouseEvent& rMEvt )
{
    DlgEdFunc::MouseButtonUp( rMEvt );

    // View vom Parent holen
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
            /******************************************************************
            * Objekt wurde verschoben
            ******************************************************************/
            pView->EndDragObj( rMEvt.IsMod1() );
            pView->ForceMarkedToAnotherPage();
        }
        else
        if (pView->IsAction() )
        {
            pView->EndAction();
            if( bMarkAction )
                pParent->UnmarkDialog();
        }
    }


    USHORT nClicks = rMEvt.GetClicks();

//  if (nClicks == 2)
//  {
//      if ( pView->HasMarkedObj() )
//      {
//          const SdrMarkList& rMarkList = pView->GetMarkList();
//
//          if (rMarkList.GetMarkCount() == 1)
//          {
//              SdrMark* pMark = rMarkList.GetMark(0);
//              SdrObject* pObj = pMark->GetObj();
//
//              // Hier koennen Objecte editiert werden
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

