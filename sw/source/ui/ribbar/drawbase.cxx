/*************************************************************************
 *
 *  $RCSfile: drawbase.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:50:21 $
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

#pragma hdrstop
#include <hintids.hxx>
#ifndef _UIPARAM_HXX
#include <uiparam.hxx>
#endif

#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
#endif
#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif
#ifndef _SFXPTITEM_HXX //autogen
#include <svtools/ptitem.hxx>
#endif
#ifndef _SVX_SIZEITEM_HXX //autogen
#include <svx/sizeitem.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _FMTCLDS_HXX
#include <fmtclds.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#include "cmdid.h"
#include "basesh.hxx"
#include "view.hxx"
#include "wrtsh.hxx"
#include "drawbase.hxx"
#include "edtwin.hxx"
#include "caption.hxx"
#include "swundo.hxx"

extern BOOL bNoInterrupt;       // in mainwn.cxx

#define MINMOVE ((USHORT)pSh->GetOut()->PixelToLogic(Size(pSh->GetDrawView()->GetMarkHdlSizePixel()/2,0)).Width())


/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/


SwDrawBase::SwDrawBase(SwWrtShell* pSwWrtShell, SwEditWin* pWindow, SwView* pSwView) :
    pSh(pSwWrtShell),
    pWin(pWindow),
    pView(pSwView),
    nSlotId(USHRT_MAX),
    bInsForm(FALSE),
    bCreateObj(TRUE)
{
    if ( !pSh->HasDrawView() )
        pSh->MakeDrawView();
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

__EXPORT SwDrawBase::~SwDrawBase()
{
    if (pView->GetWrtShellPtr())    // Im view-Dtor koennte die wrtsh bereits geloescht worden sein...
        pSh->GetDrawView()->SetEditMode(TRUE);
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/


BOOL SwDrawBase::MouseButtonDown(const MouseEvent& rMEvt)
{
    BOOL bReturn = FALSE;

    SdrView *pSdrView = pSh->GetDrawView();

    pSdrView->SetOrtho(rMEvt.IsShift());
    pSdrView->SetAngleSnapEnabled(rMEvt.IsShift());

    if (rMEvt.IsMod2())
    {
        pSdrView->SetCreate1stPointAsCenter(TRUE);
        pSdrView->SetResizeAtCenter(TRUE);
    }
    else
    {
        pSdrView->SetCreate1stPointAsCenter(FALSE);
        pSdrView->SetResizeAtCenter(FALSE);
    }

    SdrViewEvent aVEvt;
    SdrHitKind eHit = pSdrView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

    // Nur neues Objekt, wenn nicht im Basismode (bzw reinem Selektionsmode)
    if (rMEvt.IsLeft() && !pWin->IsDrawAction())
    {
        if (IsCreateObj() && (eHit == SDRHIT_UNMARKEDOBJECT || eHit == SDRHIT_NONE || pSh->IsDrawCreate()))
        {
            bNoInterrupt = TRUE;
            pWin->CaptureMouse();

            aStartPos = pWin->PixelToLogic(rMEvt.GetPosPixel());

            bReturn = pSh->BeginCreate(pWin->GetDrawMode(), aStartPos);

            SetDrawPointer();

            if ( bReturn )
                pWin->SetDrawAction(TRUE);
        }
        else if (!pSdrView->IsAction())
        {
            /**********************************************************************
            * BEZIER-EDITOR
            **********************************************************************/
            pWin->CaptureMouse();
            aStartPos = pWin->PixelToLogic(rMEvt.GetPosPixel());
            UINT16 nEditMode = pWin->GetBezierMode();

            if (eHit == SDRHIT_HANDLE && aVEvt.pHdl->GetKind() == HDL_BWGT)
            {
                /******************************************************************
                * Handle draggen
                ******************************************************************/
                bNoInterrupt = TRUE;
                bReturn = pSdrView->BegDragObj(aStartPos, (OutputDevice*) NULL, aVEvt.pHdl);
                pWin->SetDrawAction(TRUE);
            }
            else if (eHit == SDRHIT_MARKEDOBJECT && nEditMode == SID_BEZIER_INSERT)
            {
                /******************************************************************
                * Klebepunkt einfuegen
                ******************************************************************/
                bNoInterrupt = TRUE;
                bReturn = pSdrView->BegInsObjPoint(aStartPos, rMEvt.IsMod1(), NULL, 0);
                pWin->SetDrawAction(TRUE);
            }
            else if (eHit == SDRHIT_MARKEDOBJECT && rMEvt.IsMod1())
            {
                /******************************************************************
                * Klebepunkt selektieren
                ******************************************************************/
                if (!rMEvt.IsShift())
                    pSdrView->UnmarkAllPoints();

                bReturn = pSdrView->BegMarkPoints(aStartPos, (OutputDevice*) NULL);
                pWin->SetDrawAction(TRUE);
            }
            else if (eHit == SDRHIT_MARKEDOBJECT && !rMEvt.IsShift() && !rMEvt.IsMod2())
            {
                /******************************************************************
                * Objekt verschieben
                ******************************************************************/
                return FALSE;
            }
            else if (eHit == SDRHIT_HANDLE)
            {
                /******************************************************************
                * Klebepunkt selektieren
                ******************************************************************/
                if (pSdrView->HasMarkablePoints() && (!pSdrView->IsPointMarked(*aVEvt.pHdl) || rMEvt.IsShift()))
                {
                    SdrHdl* pHdl = NULL;

                    if (!rMEvt.IsShift())
                    {
                        pSdrView->UnmarkAllPoints();
                        pHdl = pSdrView->HitHandle(aStartPos, *pWin);
                    }
                    else
                    {
                        if (pSdrView->IsPointMarked(*aVEvt.pHdl))
                        {
                            bReturn = pSdrView->UnmarkPoint(*aVEvt.pHdl);
                            pHdl = NULL;
                        }
                        else
                        {
                            pHdl = pSdrView->HitHandle(aStartPos, *pWin);
                        }
                    }

                    if (pHdl)
                    {
                        bNoInterrupt = TRUE;
                        pSdrView->MarkPoint(*pHdl);
//                      bReturn = pSdrView->BegDragObj(aStartPos, (OutputDevice*) NULL, pHdl);
//                      pWin->SetDrawAction(TRUE);
                    }
                }
            }
            else
            {
                /******************************************************************
                * Objekt selektieren oder draggen
                ******************************************************************/
                if (pSh->IsObjSelectable(aStartPos) && eHit == SDRHIT_UNMARKEDOBJECT)
                {
                    if (pSdrView->HasMarkablePoints())
                        pSdrView->UnmarkAllPoints();

                    bNoInterrupt = FALSE;
                    // Drag im edtwin verwenden
                    return FALSE;
                }

                bNoInterrupt = TRUE;

                if (pSh->IsObjSelected())
                {
                    if (!rMEvt.IsShift())
                    {
                        if (!pSdrView->HasMarkablePoints())
                        {
                            //JP 10.10.2001: Bug 89619 - don't scroll the
                            //              cursor into the visible area
                            BOOL bUnlockView = !pSh->IsViewLocked();
                            pSh->LockView( TRUE );  //lock visible section
                            pSh->SelectObj(Point(LONG_MAX, LONG_MAX));  // Alles deselektieren
                            if( bUnlockView )
                                pSh->LockView( FALSE );
                        }
                        else
                            pSdrView->UnmarkAllPoints();
                    }
                }
                BOOL bMarked = FALSE;

                if (!pSh->IsSelFrmMode())
                    pSh->EnterSelFrmMode(NULL);

                if( 0 != (bReturn = pSh->BeginMark(aStartPos)) )
                    pWin->SetDrawAction(TRUE);

                SetDrawPointer();
            }
        }
    }
    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/


BOOL SwDrawBase::MouseMove(const MouseEvent& rMEvt)
{
    SdrView *pSdrView = pSh->GetDrawView();
    Point aPnt(pWin->PixelToLogic(rMEvt.GetPosPixel()));
    BOOL bRet = FALSE;

    if (IsCreateObj() && !pWin->IsDrawSelMode() && pSdrView->IsCreateObj())
    {
        pSdrView->SetOrtho(rMEvt.IsShift());
        pSdrView->SetAngleSnapEnabled(rMEvt.IsShift());

        pSh->MoveCreate(aPnt);
        bRet = TRUE;
    }
    else if (pSdrView->IsAction() || pSdrView->IsInsObjPoint() || pSdrView->IsMarkPoints())
    {
        pSh->MoveMark(aPnt);
        bRet = TRUE;
    }

    return (bRet);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/


BOOL SwDrawBase::MouseButtonUp(const MouseEvent& rMEvt)
{
    BOOL bReturn = FALSE;
    BOOL bCheckShell = FALSE;
    BOOL bAutoCap = FALSE;

    Point aPnt(pWin->PixelToLogic(rMEvt.GetPosPixel()));

    if (IsCreateObj() && pSh->IsDrawCreate() && !pWin->IsDrawSelMode())
    {
        const USHORT nDrawMode = pWin->GetDrawMode();
        //objects with multiple point may end at the start position
        BOOL bMultiPoint = OBJ_PLIN == nDrawMode ||
                                OBJ_PATHLINE == nDrawMode ||
                                OBJ_FREELINE == nDrawMode;
        if(rMEvt.IsRight() || (aPnt == aStartPos && !bMultiPoint))
        {
            pSh->BreakCreate();
            pView->LeaveDrawCreate();
        }
        else
        {
            if (OBJ_NONE == nDrawMode)
                pSh->StartUndo(UNDO_INSERT);

            pSh->EndCreate(SDRCREATE_FORCEEND);
            if (OBJ_NONE == nDrawMode)   // Textrahmen eingefuegt
            {
               com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder > xRecorder =
                    pSh->GetView().GetViewFrame()->GetBindings().GetRecorder();
                if ( xRecorder.is() )
                {
                    SfxRequest aReq(pSh->GetView().GetViewFrame(),FN_INSERT_FRAME);
                        aReq.AppendItem(SfxUInt16Item( FN_INSERT_FRAME, (USHORT)FLY_AT_CNTNT ));
                        aReq.AppendItem(SfxPointItem( FN_PARAM_1, pSh->GetAnchorObjDiff()));
                        aReq.AppendItem(SvxSizeItem( FN_PARAM_2, pSh->GetObjSize()));
                    aReq.Done();
                }
                bAutoCap = TRUE;
                if(pWin->GetFrmColCount() > 1)
                {
                    SfxItemSet aSet(pView->GetPool(),RES_COL,RES_COL);
                    SwFmtCol aCol((const SwFmtCol&)aSet.Get(RES_COL));
                    aCol.Init(pWin->GetFrmColCount(), aCol.GetGutterWidth(), aCol.GetWishWidth());
                    aSet.Put(aCol);
                    // Vorlagen-AutoUpdate
                    SwFrmFmt* pFmt = pSh->GetCurFrmFmt();
                    if(pFmt && pFmt->IsAutoUpdateFmt())
                        pSh->AutoUpdateFrame(pFmt, aSet);
                    else
                        pSh->SetFlyFrmAttr( aSet );
                }
            }
            if (pWin->GetDrawMode() == OBJ_NONE)
                pSh->EndUndo(UNDO_INSERT);
        }

        bReturn = TRUE;

        EnterSelectMode(rMEvt);
    }
    else
    {
        SdrView *pSdrView = pSh->GetDrawView();

        if (!pSdrView->HasMarkablePoints())
        {
            /**********************************************************************
            * KEIN BEZIER_EDITOR
            **********************************************************************/
            if ((pSh->GetDrawView()->IsMarkObj() || pSh->GetDrawView()->IsMarkPoints())
                 && rMEvt.IsLeft())
            {
                bReturn = pSh->EndMark();

                pWin->SetDrawAction(FALSE);

                if (aPnt == aStartPos && pSh->IsObjSelectable(aPnt))
                {
                    pSh->SelectObj(aPnt, ( rMEvt.IsShift() &&
                                   pSh->IsSelFrmMode()) ? SW_ADD_SELECT : 0);

                    if (!pSh->IsObjSelected())
                    {
                        pView->LeaveDrawCreate();   // In Selektionsmode wechseln

                        pSh->GetView().GetViewFrame()->GetBindings().Invalidate(SID_INSERT_DRAW);

                        if (pSh->IsSelFrmMode())
                            pSh->LeaveSelFrmMode();
                    }
                    pView->NoRotate();

                    bCheckShell = TRUE; // ggf BezierShell anwerfen
                }
                else if (!pSh->IsObjSelected() && !pWin->IsDrawAction())
                {
                    if (pSh->IsObjSelectable(aPnt))
                        pSh->SelectObj(aPnt, ( rMEvt.IsShift() &&
                            pSh->IsSelFrmMode() ) ? SW_ADD_SELECT : 0 );
                    else
                    {
                        pView->LeaveDrawCreate();
                        if (pSh->IsSelFrmMode())
                            pSh->LeaveSelFrmMode();
                    }
                    pView->NoRotate();

                    bReturn = TRUE;
                }
            }
        }
        else
        {
            /**********************************************************************
            * BEZIER_EDITOR
            **********************************************************************/
            if ( pSdrView->IsAction() )
            {
                if ( pSdrView->IsInsObjPoint() )
                    bReturn = pSdrView->EndInsObjPoint(SDRCREATE_FORCEEND);
                else if (pSdrView->IsMarkPoints() )
                    bReturn = pSdrView->EndMarkPoints();
                else
                {
                    pSdrView->EndAction();
                    bReturn = TRUE;
                }
                pWin->SetDrawAction(FALSE);

                if (aPnt == aStartPos)
                {
                    if (!pSh->IsObjSelectable(aPnt))
                        pSh->SelectObj(Point(LONG_MAX, LONG_MAX));
                    else if (!bReturn)
                    {
                        if (!rMEvt.IsShift())
                            pSdrView->UnmarkAllPoints();
                        pSh->SelectObj(aPnt, (rMEvt.IsShift() &&
                                       pSh->IsSelFrmMode()) ? SW_ADD_SELECT :0);
                    }

                    if (!pSh->IsObjSelected())
                    {
                        pView->LeaveDrawCreate();   // In Selektionsmode wechseln

                        pSh->GetView().GetViewFrame()->GetBindings().Invalidate(SID_INSERT_DRAW);

                        if (pSh->IsSelFrmMode())
                            pSh->LeaveSelFrmMode();
                    }
                    pView->NoRotate();

                    bCheckShell = TRUE; // ggf BezierShell anwerfen
                }
            }

            SetDrawPointer();

            if (!pSh->IsObjSelected() && !pWin->IsDrawAction())
            {
                pView->LeaveDrawCreate();
                if (pSh->IsSelFrmMode())
                    pSh->LeaveSelFrmMode();

                pView->NoRotate();
                bReturn = TRUE;
            }
        }
    }

    if (bCheckShell)
        pView->AttrChangedNotify( pSh );    // ggf BezierShell anwerfen

    //!!!!!!!!!! Achtung Suizid !!!!!!!!!!! Sollte alles mal erneuert werden
    if ( bAutoCap )
        pView->AutoCaption(FRAME_CAP);  //Kann derzeit nur FRAME sein, sonst auf
                                        //enums umstellen
    return (bReturn);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/


void SwDrawBase::Activate(const USHORT nSlot)
{
    SetSlotId(nSlot);
    SdrView *pSdrView = pSh->GetDrawView();

    pSdrView->SetCurrentObj(pWin->GetDrawMode());
    pSdrView->SetEditMode(FALSE);

    SetDrawPointer();
    pSh->NoEdit();
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/


void __EXPORT SwDrawBase::Deactivate()
{
    SdrView *pSdrView = pSh->GetDrawView();
    pSdrView->SetOrtho(FALSE);
    pSdrView->SetAngleSnapEnabled(FALSE);

    if (pWin->IsDrawAction() && pSh->IsDrawCreate())
        pSh->BreakCreate();

    pWin->SetDrawAction(FALSE);

    pWin->ReleaseMouse();
    bNoInterrupt = FALSE;

//  if(!pSh->IsObjSelected())
//      pSh->Edit();

    if(pWin->GetApplyTemplate())
        pWin->SetApplyTemplate(SwApplyTemplate());
    pSh->GetView().GetViewFrame()->GetBindings().Invalidate(SID_INSERT_DRAW);
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert TRUE, andernfalls
|* FALSE.
|*
\************************************************************************/


BOOL SwDrawBase::KeyInput(const KeyEvent& rKEvt)
{
    BOOL bReturn = FALSE;
    USHORT nCode = rKEvt.GetKeyCode().GetCode();

    switch (nCode)
    {
        case KEY_ESCAPE:
        {
            if (pWin->IsDrawAction())
            {
                BreakCreate();
                pView->LeaveDrawCreate();
            }

            bReturn = TRUE;
        }
        break;

        case KEY_DELETE:
        {
            pSh->DelSelectedObj();
            bReturn = TRUE;
        }
        break;

        case KEY_UP:
        case KEY_DOWN:
        case KEY_LEFT:
        case KEY_RIGHT:
        {
            SdrView *pSdrView = pSh->GetDrawView();

            if (!pSdrView->IsTextEdit())
            {
                long nX = 0;
                long nY = 0;

                if (nCode == KEY_UP)
                {
                    // Scroll nach oben
                    nX = 0;
                    nY =-1;
                }
                else if (nCode == KEY_DOWN)
                {
                    // Scroll nach unten
                    nX = 0;
                    nY = 1;
                }
                else if (nCode == KEY_LEFT)
                {
                    // Scroll nach links
                    nX =-1;
                    nY = 0;
                }
                else if (nCode == KEY_RIGHT)
                {
                    // Scroll nach rechts
                    nX = 1;
                    nY = 0;
                }

                if (pSdrView->AreObjectsMarked() && rKEvt.GetKeyCode().IsMod2())
                {
                    // Objekte verschieben
                    nX *= 100;
                    nY *= 100;
                    pSdrView->MoveAllMarked(Size(nX, nY));
                }

                bReturn = TRUE;
            }
        }
        break;
    }

    return (bReturn);
}


/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert TRUE, andernfalls
|* FALSE.
|*
\************************************************************************/


void SwDrawBase::BreakCreate()
{
    pSh->BreakCreate();
    pWin->SetDrawAction(FALSE);
    pWin->ReleaseMouse();

    Deactivate();
//  pView->LeaveDrawCreate();
}

/*************************************************************************
|*
|* Mauspointer umschalten
|*
\************************************************************************/


void SwDrawBase::SetDrawPointer()
{
    SdrView *pSdrView = pSh->GetDrawView();
        Point aPnt(pWin->OutputToScreenPixel(pWin->GetPointerPosPixel()));
    aPnt = pWin->PixelToLogic(pWin->ScreenToOutputPixel(aPnt));
    const Pointer aPointTyp = pSdrView->GetPreferedPointer(aPnt, pSh->GetOut());
    const Pointer aDrawPt(aPointTyp);
    pWin->SetPointer(aDrawPt);
}

/*************************************************************************
|*
|* Ggf in Selektionsmode wechseln
|*
\************************************************************************/

void SwDrawBase::EnterSelectMode(const MouseEvent& rMEvt)
{
    pWin->SetDrawAction(FALSE);

    if (!pSh->IsObjSelected() && !pWin->IsDrawAction())
    {
        Point aPnt(pWin->PixelToLogic(rMEvt.GetPosPixel()));

        if (pSh->IsObjSelectable(aPnt))
        {
            pSh->SelectObj(aPnt);
            if (rMEvt.GetModifier() == KEY_SHIFT || !pSh->IsObjSelected())
            {
                pView->LeaveDrawCreate();   // In Selektionsmode wechseln

                pSh->GetView().GetViewFrame()->GetBindings().Invalidate(SID_INSERT_DRAW);
            }
        }
        else
        {
            pView->LeaveDrawCreate();
            if (pSh->IsSelFrmMode())
                pSh->LeaveSelFrmMode();
        }
        pView->NoRotate();
    }
}
/* -----------------------------03.04.2002 10:52------------------------------

 ---------------------------------------------------------------------------*/
void SwDrawBase::CreateDefaultObject()
{
    Point aStartPos = GetDefaultCenterPos();
    Point aEndPos(aStartPos);
    aStartPos.X() -= 8 * MM50;
    aStartPos.Y() -= 4 * MM50;
    aEndPos.X() += 8 * MM50;
    aEndPos.Y() += 4 * MM50;
    Rectangle aRect(aStartPos, aEndPos);
    pSh->CreateDefaultShape(pWin->GetDrawMode(), aRect, nSlotId);
}
/* -----------------25.10.2002 14:14-----------------
 *
 * --------------------------------------------------*/
Point  SwDrawBase::GetDefaultCenterPos()
{
    Size aDocSz(pSh->GetDocSize());
    const SwRect& rVisArea = pSh->VisArea();
    Point aStartPos = rVisArea.Center();
    if(rVisArea.Width() > aDocSz.Width())
        aStartPos.X() = aDocSz.Width() / 2 + rVisArea.Left();
    if(rVisArea.Height() > aDocSz.Height())
        aStartPos.Y() = aDocSz.Height() / 2 + rVisArea.Top();
    return aStartPos;
}

