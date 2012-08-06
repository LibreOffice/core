/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include <hintids.hxx>
#include <svx/svdview.hxx>
#include <svx/svdobj.hxx>
#include <svl/ptitem.hxx>
#include <editeng/sizeitem.hxx>
#include <sfx2/request.hxx>
#include <sfx2/bindings.hxx>
#include <fmtclds.hxx>
#include <frmfmt.hxx>
#include "cmdid.h"
#include "basesh.hxx"
#include "view.hxx"
#include "wrtsh.hxx"
#include "drawbase.hxx"
#include "edtwin.hxx"
#include "caption.hxx"
#include "swundo.hxx"
#include <SwRewriter.hxx>
#include "comcore.hrc"

using namespace ::com::sun::star;

extern sal_Bool bNoInterrupt;       // in mainwn.cxx

#define MINMOVE ((sal_uInt16)m_pSh->GetOut()->PixelToLogic(Size(m_pSh->GetDrawView()->GetMarkHdlSizePixel()/2,0)).Width())


/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/


SwDrawBase::SwDrawBase(SwWrtShell* pSwWrtShell, SwEditWin* pWindow, SwView* pSwView) :
    m_pView(pSwView),
    m_pSh(pSwWrtShell),
    m_pWin(pWindow),
    m_nSlotId(USHRT_MAX),
    m_bCreateObj(sal_True),
    m_bInsForm(sal_False)
{
    if ( !m_pSh->HasDrawView() )
        m_pSh->MakeDrawView();
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

SwDrawBase::~SwDrawBase()
{
    if (m_pView->GetWrtShellPtr()) // Im view-Dtor koennte die wrtsh bereits geloescht worden sein...
        m_pSh->GetDrawView()->SetEditMode(sal_True);
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/


sal_Bool SwDrawBase::MouseButtonDown(const MouseEvent& rMEvt)
{
    sal_Bool bReturn = sal_False;

    SdrView *pSdrView = m_pSh->GetDrawView();

    // #i33136#
    pSdrView->SetOrtho(doConstructOrthogonal() ? !rMEvt.IsShift() : rMEvt.IsShift());
    pSdrView->SetAngleSnapEnabled(rMEvt.IsShift());

    if (rMEvt.IsMod2())
    {
        pSdrView->SetCreate1stPointAsCenter(sal_True);
        pSdrView->SetResizeAtCenter(sal_True);
    }
    else
    {
        pSdrView->SetCreate1stPointAsCenter(sal_False);
        pSdrView->SetResizeAtCenter(sal_False);
    }

    SdrViewEvent aVEvt;
    SdrHitKind eHit = pSdrView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

    // Nur neues Objekt, wenn nicht im Basismode (bzw reinem Selektionsmode)
    if (rMEvt.IsLeft() && !m_pWin->IsDrawAction())
    {
        if (IsCreateObj() && (eHit == SDRHIT_UNMARKEDOBJECT || eHit == SDRHIT_NONE || m_pSh->IsDrawCreate()))
        {
            bNoInterrupt = sal_True;
            m_pWin->CaptureMouse();

            m_aStartPos = m_pWin->PixelToLogic(rMEvt.GetPosPixel());

            bReturn = m_pSh->BeginCreate( static_cast< sal_uInt16 >(m_pWin->GetSdrDrawMode()), m_aStartPos);

            SetDrawPointer();

            if ( bReturn )
                m_pWin->SetDrawAction(sal_True);
        }
        else if (!pSdrView->IsAction())
        {
            /**********************************************************************
            * BEZIER-EDITOR
            **********************************************************************/
            m_pWin->CaptureMouse();
            m_aStartPos = m_pWin->PixelToLogic(rMEvt.GetPosPixel());
            sal_uInt16 nEditMode = m_pWin->GetBezierMode();

            if (eHit == SDRHIT_HANDLE && aVEvt.pHdl->GetKind() == HDL_BWGT)
            {
                /******************************************************************
                * Handle draggen
                ******************************************************************/
                bNoInterrupt = sal_True;
                bReturn = pSdrView->BegDragObj(m_aStartPos, (OutputDevice*) NULL, aVEvt.pHdl);
                m_pWin->SetDrawAction(sal_True);
            }
            else if (eHit == SDRHIT_MARKEDOBJECT && nEditMode == SID_BEZIER_INSERT)
            {
                /******************************************************************
                * Klebepunkt einfuegen
                ******************************************************************/
                bNoInterrupt = sal_True;
                bReturn = pSdrView->BegInsObjPoint(m_aStartPos, rMEvt.IsMod1());
                m_pWin->SetDrawAction(sal_True);
            }
            else if (eHit == SDRHIT_MARKEDOBJECT && rMEvt.IsMod1())
            {
                /******************************************************************
                * Klebepunkt selektieren
                ******************************************************************/
                if (!rMEvt.IsShift())
                    pSdrView->UnmarkAllPoints();

                bReturn = pSdrView->BegMarkPoints(m_aStartPos);
                m_pWin->SetDrawAction(sal_True);
            }
            else if (eHit == SDRHIT_MARKEDOBJECT && !rMEvt.IsShift() && !rMEvt.IsMod2())
            {
                /******************************************************************
                * Objekt verschieben
                ******************************************************************/
                return sal_False;
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
                        pHdl = pSdrView->PickHandle(m_aStartPos);
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
                            pHdl = pSdrView->PickHandle(m_aStartPos);
                        }
                    }

                    if (pHdl)
                    {
                        bNoInterrupt = sal_True;
                        pSdrView->MarkPoint(*pHdl);
                    }
                }
            }
            else
            {
                /******************************************************************
                * Objekt selektieren oder draggen
                ******************************************************************/
                if (m_pSh->IsObjSelectable(m_aStartPos) && eHit == SDRHIT_UNMARKEDOBJECT)
                {
                    if (pSdrView->HasMarkablePoints())
                        pSdrView->UnmarkAllPoints();

                    bNoInterrupt = sal_False;
                    // Drag im edtwin verwenden
                    return sal_False;
                }

                bNoInterrupt = sal_True;

                if (m_pSh->IsObjSelected())
                {
                    if (!rMEvt.IsShift())
                    {
                        if (!pSdrView->HasMarkablePoints())
                        {
                            sal_Bool bUnlockView = !m_pSh->IsViewLocked();
                            m_pSh->LockView( sal_True ); //lock visible section
                            m_pSh->SelectObj(Point(LONG_MAX, LONG_MAX)); // deselect all
                            if( bUnlockView )
                                m_pSh->LockView( sal_False );
                        }
                        else
                            pSdrView->UnmarkAllPoints();
                    }
                }
                if (!m_pSh->IsSelFrmMode())
                    m_pSh->EnterSelFrmMode(NULL);

                if( 0 != (bReturn = m_pSh->BeginMark(m_aStartPos)) )
                    m_pWin->SetDrawAction(sal_True);

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


sal_Bool SwDrawBase::MouseMove(const MouseEvent& rMEvt)
{
    SdrView *pSdrView = m_pSh->GetDrawView();
    Point aPnt(m_pWin->PixelToLogic(rMEvt.GetPosPixel()));
    sal_Bool bRet = sal_False;

    if (IsCreateObj() && !m_pWin->IsDrawSelMode() && pSdrView->IsCreateObj())
    {
        // #i33136#
        pSdrView->SetOrtho(doConstructOrthogonal() ? !rMEvt.IsShift() : rMEvt.IsShift());
        pSdrView->SetAngleSnapEnabled(rMEvt.IsShift());

        m_pSh->MoveCreate(aPnt);
        bRet = sal_True;
    }
    else if (pSdrView->IsAction() || pSdrView->IsInsObjPoint() || pSdrView->IsMarkPoints())
    {
        m_pSh->MoveMark(aPnt);
        bRet = sal_True;
    }

    return (bRet);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/


sal_Bool SwDrawBase::MouseButtonUp(const MouseEvent& rMEvt)
{
    sal_Bool bReturn = sal_False;
    sal_Bool bCheckShell = sal_False;
    sal_Bool bAutoCap = sal_False;

    Point aPnt(m_pWin->PixelToLogic(rMEvt.GetPosPixel()));

    if (IsCreateObj() && m_pSh->IsDrawCreate() && !m_pWin->IsDrawSelMode())
    {
        const SdrObjKind nDrawMode = m_pWin->GetSdrDrawMode();
        //objects with multiple point may end at the start position
        sal_Bool bMultiPoint = OBJ_PLIN == nDrawMode ||
                                OBJ_PATHLINE == nDrawMode ||
                                OBJ_FREELINE == nDrawMode;
        if(rMEvt.IsRight() || (aPnt == m_aStartPos && !bMultiPoint))
        {
            m_pSh->BreakCreate();
            m_pView->LeaveDrawCreate();
        }
        else
        {
            if (OBJ_NONE == nDrawMode)
            {
                SwRewriter aRewriter;

                aRewriter.AddRule(UndoArg1, SW_RES(STR_FRAME));
                m_pSh->StartUndo(UNDO_INSERT, &aRewriter);
            }

            m_pSh->EndCreate(SDRCREATE_FORCEEND);
            if (OBJ_NONE == nDrawMode)   // Textrahmen eingefuegt
            {
               uno::Reference< frame::XDispatchRecorder > xRecorder =
                    m_pSh->GetView().GetViewFrame()->GetBindings().GetRecorder();
                if ( xRecorder.is() )
                {
                    SfxRequest aReq(m_pSh->GetView().GetViewFrame(),FN_INSERT_FRAME);
                        aReq.AppendItem(SfxUInt16Item( FN_INSERT_FRAME,
                                static_cast<sal_uInt16>(FLY_AT_PARA) ));
                        aReq.AppendItem(SfxPointItem( FN_PARAM_1, m_pSh->GetAnchorObjDiff()));
                        aReq.AppendItem(SvxSizeItem( FN_PARAM_2, m_pSh->GetObjSize()));
                    aReq.Done();
                }
                bAutoCap = sal_True;
                if(m_pWin->GetFrmColCount() > 1)
                {
                    SfxItemSet aSet(m_pView->GetPool(),RES_COL,RES_COL);
                    SwFmtCol aCol((const SwFmtCol&)aSet.Get(RES_COL));
                    aCol.Init(m_pWin->GetFrmColCount(), aCol.GetGutterWidth(), aCol.GetWishWidth());
                    aSet.Put(aCol);
                    // Vorlagen-AutoUpdate
                    SwFrmFmt* pFmt = m_pSh->GetCurFrmFmt();
                    if(pFmt && pFmt->IsAutoUpdateFmt())
                        m_pSh->AutoUpdateFrame(pFmt, aSet);
                    else
                        m_pSh->SetFlyFrmAttr( aSet );
                }
            }
            if (m_pWin->GetSdrDrawMode() == OBJ_NONE)
            {
                m_pSh->EndUndo();
            }
        }

        bReturn = sal_True;

        EnterSelectMode(rMEvt);
    }
    else
    {
        SdrView *pSdrView = m_pSh->GetDrawView();

        if (!pSdrView->HasMarkablePoints())
        {
            /**********************************************************************
            * KEIN BEZIER_EDITOR
            **********************************************************************/
            if ((m_pSh->GetDrawView()->IsMarkObj() || m_pSh->GetDrawView()->IsMarkPoints())
                 && rMEvt.IsLeft())
            {
                bReturn = m_pSh->EndMark();

                m_pWin->SetDrawAction(sal_False);

                if (aPnt == m_aStartPos && m_pSh->IsObjSelectable(aPnt))
                {
                    m_pSh->SelectObj(aPnt, ( rMEvt.IsShift() &&
                                   m_pSh->IsSelFrmMode()) ? SW_ADD_SELECT : 0);

                    if (!m_pSh->IsObjSelected())
                    {
                        m_pView->LeaveDrawCreate();    // In Selektionsmode wechseln

                        m_pSh->GetView().GetViewFrame()->GetBindings().Invalidate(SID_INSERT_DRAW);

                        if (m_pSh->IsSelFrmMode())
                            m_pSh->LeaveSelFrmMode();
                    }
                    m_pView->NoRotate();

                    bCheckShell = sal_True; // ggf BezierShell anwerfen
                }
                else if (!m_pSh->IsObjSelected() && !m_pWin->IsDrawAction())
                {
                    if (m_pSh->IsObjSelectable(aPnt))
                        m_pSh->SelectObj(aPnt, ( rMEvt.IsShift() &&
                            m_pSh->IsSelFrmMode() ) ? SW_ADD_SELECT : 0 );
                    else
                    {
                        m_pView->LeaveDrawCreate();
                        if (m_pSh->IsSelFrmMode())
                            m_pSh->LeaveSelFrmMode();
                    }
                    m_pView->NoRotate();

                    bReturn = sal_True;
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
                    bReturn = sal_True;
                }
                m_pWin->SetDrawAction(sal_False);

                if (aPnt == m_aStartPos)
                {
                    if (!m_pSh->IsObjSelectable(aPnt))
                        m_pSh->SelectObj(Point(LONG_MAX, LONG_MAX));
                    else if (!bReturn)
                    {
                        if (!rMEvt.IsShift())
                            pSdrView->UnmarkAllPoints();
                        m_pSh->SelectObj(aPnt, (rMEvt.IsShift() &&
                                       m_pSh->IsSelFrmMode()) ? SW_ADD_SELECT :0);
                    }

                    if (!m_pSh->IsObjSelected())
                    {
                        m_pView->LeaveDrawCreate();    // In Selektionsmode wechseln

                        m_pSh->GetView().GetViewFrame()->GetBindings().Invalidate(SID_INSERT_DRAW);

                        if (m_pSh->IsSelFrmMode())
                            m_pSh->LeaveSelFrmMode();
                    }
                    m_pView->NoRotate();

                    bCheckShell = sal_True; // ggf BezierShell anwerfen
                }
            }

            SetDrawPointer();

            if (!m_pSh->IsObjSelected() && !m_pWin->IsDrawAction())
            {
                m_pView->LeaveDrawCreate();
                if (m_pSh->IsSelFrmMode())
                    m_pSh->LeaveSelFrmMode();

                m_pView->NoRotate();
                bReturn = sal_True;
            }
        }
    }

    if (bCheckShell)
        m_pView->AttrChangedNotify( m_pSh ); // ggf BezierShell anwerfen

    //!!!!!!!!!! Achtung Suizid !!!!!!!!!!! Sollte alles mal erneuert werden
    if ( bAutoCap )
        m_pView->AutoCaption(FRAME_CAP);   //Kann derzeit nur FRAME sein, sonst auf
                                        //enums umstellen
    return (bReturn);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/


void SwDrawBase::Activate(const sal_uInt16 nSlot)
{
    SetSlotId(nSlot);
    SdrView *pSdrView = m_pSh->GetDrawView();

    pSdrView->SetCurrentObj( static_cast< sal_uInt16 >(m_pWin->GetSdrDrawMode()) );
    pSdrView->SetEditMode(sal_False);

    SetDrawPointer();
    m_pSh->NoEdit();
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/


void SwDrawBase::Deactivate()
{
    SdrView *pSdrView = m_pSh->GetDrawView();
    pSdrView->SetOrtho(sal_False);
    pSdrView->SetAngleSnapEnabled(sal_False);

    if (m_pWin->IsDrawAction() && m_pSh->IsDrawCreate())
        m_pSh->BreakCreate();

    m_pWin->SetDrawAction(sal_False);

    m_pWin->ReleaseMouse();
    bNoInterrupt = sal_False;

    if(m_pWin->GetApplyTemplate())
        m_pWin->SetApplyTemplate(SwApplyTemplate());
    m_pSh->GetView().GetViewFrame()->GetBindings().Invalidate(SID_INSERT_DRAW);
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert sal_True, andernfalls
|* sal_False.
|*
\************************************************************************/


sal_Bool SwDrawBase::KeyInput(const KeyEvent& rKEvt)
{
    sal_Bool bReturn = sal_False;
    sal_uInt16 nCode = rKEvt.GetKeyCode().GetCode();

    switch (nCode)
    {
        case KEY_ESCAPE:
        {
            if (m_pWin->IsDrawAction())
            {
                BreakCreate();
                m_pView->LeaveDrawCreate();
            }

            bReturn = sal_True;
        }
        break;

        case KEY_DELETE:
        {
            m_pSh->DelSelectedObj();
            bReturn = sal_True;
        }
        break;

        case KEY_UP:
        case KEY_DOWN:
        case KEY_LEFT:
        case KEY_RIGHT:
        {
            SdrView *pSdrView = m_pSh->GetDrawView();

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

                bReturn = sal_True;
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
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert sal_True, andernfalls
|* sal_False.
|*
\************************************************************************/


void SwDrawBase::BreakCreate()
{
    m_pSh->BreakCreate();
    m_pWin->SetDrawAction(sal_False);
    m_pWin->ReleaseMouse();

    Deactivate();
}

/*************************************************************************
|*
|* Mauspointer umschalten
|*
\************************************************************************/


void SwDrawBase::SetDrawPointer()
{
    SdrView *pSdrView = m_pSh->GetDrawView();
        Point aPnt(m_pWin->OutputToScreenPixel(m_pWin->GetPointerPosPixel()));
    aPnt = m_pWin->PixelToLogic(m_pWin->ScreenToOutputPixel(aPnt));
    const Pointer aPointTyp = pSdrView->GetPreferedPointer(aPnt, m_pSh->GetOut());
    const Pointer aDrawPt(aPointTyp);
    m_pWin->SetPointer(aDrawPt);
}

/*************************************************************************
|*
|* Ggf in Selektionsmode wechseln
|*
\************************************************************************/

void SwDrawBase::EnterSelectMode(const MouseEvent& rMEvt)
{
    m_pWin->SetDrawAction(sal_False);

    if (!m_pSh->IsObjSelected() && !m_pWin->IsDrawAction())
    {
        Point aPnt(m_pWin->PixelToLogic(rMEvt.GetPosPixel()));

        if (m_pSh->IsObjSelectable(aPnt))
        {
            m_pSh->SelectObj(aPnt);
            if (rMEvt.GetModifier() == KEY_SHIFT || !m_pSh->IsObjSelected())
            {
                m_pView->LeaveDrawCreate();    // In Selektionsmode wechseln

                m_pSh->GetView().GetViewFrame()->GetBindings().Invalidate(SID_INSERT_DRAW);
            }
        }
        else
        {
            m_pView->LeaveDrawCreate();
            if (m_pSh->IsSelFrmMode())
                m_pSh->LeaveSelFrmMode();
        }
        m_pView->NoRotate();
    }
}

void SwDrawBase::CreateDefaultObject()
{
    Point aStartPos = GetDefaultCenterPos();
    Point aEndPos(aStartPos);
    aStartPos.X() -= 8 * MM50;
    aStartPos.Y() -= 4 * MM50;
    aEndPos.X() += 8 * MM50;
    aEndPos.Y() += 4 * MM50;
    Rectangle aRect(aStartPos, aEndPos);
    m_pSh->CreateDefaultShape( static_cast< sal_uInt16 >(m_pWin->GetSdrDrawMode()), aRect, m_nSlotId);
}

Point  SwDrawBase::GetDefaultCenterPos()
{
    Size aDocSz(m_pSh->GetDocSize());
    const SwRect& rVisArea = m_pSh->VisArea();
    Point aStartPos = rVisArea.Center();
    if(rVisArea.Width() > aDocSz.Width())
        aStartPos.X() = aDocSz.Width() / 2 + rVisArea.Left();
    if(rVisArea.Height() > aDocSz.Height())
        aStartPos.Y() = aDocSz.Height() / 2 + rVisArea.Top();
    return aStartPos;
}

// #i33136#
bool SwDrawBase::doConstructOrthogonal() const
{
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
