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

extern bool g_bNoInterrupt;       // in swmodule.cxx

SwDrawBase::SwDrawBase(SwWrtShell* pSwWrtShell, SwEditWin* pWindow, SwView* pSwView) :
    m_pView(pSwView),
    m_pSh(pSwWrtShell),
    m_pWin(pWindow),
    m_nSlotId(USHRT_MAX),
    m_bCreateObj(true),
    m_bInsForm(false)
{
    if ( !m_pSh->HasDrawView() )
        m_pSh->MakeDrawView();
}

SwDrawBase::~SwDrawBase()
{
    if (m_pView->GetWrtShellPtr()) // In the view-dtor could the wrtsh already been deleted...
        m_pSh->GetDrawView()->SetEditMode();
}

bool SwDrawBase::MouseButtonDown(const MouseEvent& rMEvt)
{
    bool bReturn = false;

    SdrView *pSdrView = m_pSh->GetDrawView();

    // #i33136#
    pSdrView->SetOrtho(doConstructOrthogonal() ? !rMEvt.IsShift() : rMEvt.IsShift());
    pSdrView->SetAngleSnapEnabled(rMEvt.IsShift());

    if (rMEvt.IsMod2())
    {
        pSdrView->SetCreate1stPointAsCenter(true);
        pSdrView->SetResizeAtCenter(true);
    }
    else
    {
        pSdrView->SetCreate1stPointAsCenter(false);
        pSdrView->SetResizeAtCenter(false);
    }

    SdrViewEvent aVEvt;
    SdrHitKind eHit = pSdrView->PickAnything(rMEvt, SdrMouseEventKind::BUTTONDOWN, aVEvt);

    // Only new object, if not in the basic mode (or pure selection mode).
    if (rMEvt.IsLeft() && !m_pWin->IsDrawAction())
    {
        if (IsCreateObj() && (eHit == SDRHIT_UNMARKEDOBJECT || eHit == SDRHIT_NONE || m_pSh->IsDrawCreate()))
        {
            g_bNoInterrupt = true;
            m_pWin->CaptureMouse();

            m_aStartPos = m_pWin->PixelToLogic(rMEvt.GetPosPixel());

            bReturn = m_pSh->BeginCreate( static_cast< sal_uInt16 >(m_pWin->GetSdrDrawMode()), m_aStartPos);

            SetDrawPointer();

            if ( bReturn )
                m_pWin->SetDrawAction(true);
        }
        else if (!pSdrView->IsAction())
        {
            // BEZIER-EDITOR
            m_pWin->CaptureMouse();
            m_aStartPos = m_pWin->PixelToLogic(rMEvt.GetPosPixel());
            sal_uInt16 nEditMode = m_pWin->GetBezierMode();

            if (eHit == SDRHIT_HANDLE && aVEvt.pHdl->GetKind() == HDL_BWGT)
            {
                // Drag handle
                g_bNoInterrupt = true;
                bReturn = pSdrView->BegDragObj(m_aStartPos, nullptr, aVEvt.pHdl);
                m_pWin->SetDrawAction(true);
            }
            else if (eHit == SDRHIT_MARKEDOBJECT && nEditMode == SID_BEZIER_INSERT)
            {
                // Insert gluepoint
                g_bNoInterrupt = true;
                bReturn = pSdrView->BegInsObjPoint(m_aStartPos, rMEvt.IsMod1());
                m_pWin->SetDrawAction(true);
            }
            else if (eHit == SDRHIT_MARKEDOBJECT && rMEvt.IsMod1())
            {
                // Select gluepoint
                if (!rMEvt.IsShift())
                    pSdrView->UnmarkAllPoints();

                bReturn = pSdrView->BegMarkPoints(m_aStartPos);
                m_pWin->SetDrawAction(true);
            }
            else if (eHit == SDRHIT_MARKEDOBJECT && !rMEvt.IsShift() && !rMEvt.IsMod2())
            {
                // Move objekt
                return false;
            }
            else if (eHit == SDRHIT_HANDLE)
            {
                // Select gluepoint
                if (pSdrView->HasMarkablePoints() && (!pSdrView->IsPointMarked(*aVEvt.pHdl) || rMEvt.IsShift()))
                {
                    SdrHdl* pHdl = nullptr;

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
                            pHdl = nullptr;
                        }
                        else
                        {
                            pHdl = pSdrView->PickHandle(m_aStartPos);
                        }
                    }

                    if (pHdl)
                    {
                        g_bNoInterrupt = true;
                        pSdrView->MarkPoint(*pHdl);
                    }
                }
            }
            else
            {
                // Select or drag object
                if (m_pSh->IsObjSelectable(m_aStartPos) && eHit == SDRHIT_UNMARKEDOBJECT)
                {
                    if (pSdrView->HasMarkablePoints())
                        pSdrView->UnmarkAllPoints();

                    g_bNoInterrupt = false;
                    // Use drag in edtwin
                    return false;
                }

                g_bNoInterrupt = true;

                if (m_pSh->IsObjSelected())
                {
                    if (!rMEvt.IsShift())
                    {
                        if (!pSdrView->HasMarkablePoints())
                        {
                            bool bUnlockView = !m_pSh->IsViewLocked();
                            m_pSh->LockView( true ); //lock visible section
                            m_pSh->SelectObj(Point(LONG_MAX, LONG_MAX)); // deselect all
                            if( bUnlockView )
                                m_pSh->LockView( false );
                        }
                        else
                            pSdrView->UnmarkAllPoints();
                    }
                }
                if (!m_pSh->IsSelFrameMode())
                    m_pSh->EnterSelFrameMode();

                if( (bReturn = m_pSh->BeginMark(m_aStartPos)) )
                    m_pWin->SetDrawAction(true);

                SetDrawPointer();
            }
        }
    }
    return bReturn;
}

bool SwDrawBase::MouseMove(const MouseEvent& rMEvt)
{
    SdrView *pSdrView = m_pSh->GetDrawView();
    Point aPnt(m_pWin->PixelToLogic(rMEvt.GetPosPixel()));
    bool bRet = false;

    if (IsCreateObj() && !m_pWin->IsDrawSelMode() && pSdrView->IsCreateObj())
    {
        // #i33136#
        pSdrView->SetOrtho(doConstructOrthogonal() ? !rMEvt.IsShift() : rMEvt.IsShift());
        pSdrView->SetAngleSnapEnabled(rMEvt.IsShift());

        m_pSh->MoveCreate(aPnt);
        bRet = true;
    }
    else if (pSdrView->IsAction() || pSdrView->IsInsObjPoint() || pSdrView->IsMarkPoints())
    {
        m_pSh->MoveMark(aPnt);
        bRet = true;
    }

    return bRet;
}

bool SwDrawBase::MouseButtonUp(const MouseEvent& rMEvt)
{
    bool bReturn = false;
    bool bCheckShell = false;
    bool bAutoCap = false;

    Point aPnt(m_pWin->PixelToLogic(rMEvt.GetPosPixel()));

    if (IsCreateObj() && m_pSh->IsDrawCreate() && !m_pWin->IsDrawSelMode())
    {
        const SdrObjKind nDrawMode = m_pWin->GetSdrDrawMode();
        //objects with multiple point may end at the start position
        bool bMultiPoint = OBJ_PLIN == nDrawMode ||
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
            if (OBJ_NONE == nDrawMode)   // Text border inserted
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
                bAutoCap = true;
                if(m_pWin->GetFrameColCount() > 1)
                {
                    SfxItemSet aSet(m_pView->GetPool(),RES_COL,RES_COL);
                    SwFormatCol aCol(static_cast<const SwFormatCol&>(aSet.Get(RES_COL)));
                    aCol.Init(m_pWin->GetFrameColCount(), aCol.GetGutterWidth(), aCol.GetWishWidth());
                    aSet.Put(aCol);
                    // Template AutoUpdate
                    SwFrameFormat* pFormat = m_pSh->GetSelectedFrameFormat();
                    if(pFormat && pFormat->IsAutoUpdateFormat())
                        m_pSh->AutoUpdateFrame(pFormat, aSet);
                    else
                        m_pSh->SetFlyFrameAttr( aSet );
                }
            }
            if (m_pWin->GetSdrDrawMode() == OBJ_NONE)
            {
                m_pSh->EndUndo();
            }
        }

        bReturn = true;

        EnterSelectMode(rMEvt);
    }
    else
    {
        SdrView *pSdrView = m_pSh->GetDrawView();

        if (!pSdrView->HasMarkablePoints())
        {
            // NO BEZIER_EDITOR
            if ((m_pSh->GetDrawView()->IsMarkObj() || m_pSh->GetDrawView()->IsMarkPoints())
                 && rMEvt.IsLeft())
            {
                bReturn = m_pSh->EndMark();

                m_pWin->SetDrawAction(false);

                if (aPnt == m_aStartPos && m_pSh->IsObjSelectable(aPnt))
                {
                    m_pSh->SelectObj(aPnt, ( rMEvt.IsShift() &&
                                   m_pSh->IsSelFrameMode()) ? SW_ADD_SELECT : 0);

                    if (!m_pSh->IsObjSelected())
                    {
                        m_pView->LeaveDrawCreate();    // Switch to selection mode

                        m_pSh->GetView().GetViewFrame()->GetBindings().Invalidate(SID_INSERT_DRAW);

                        if (m_pSh->IsSelFrameMode())
                            m_pSh->LeaveSelFrameMode();
                    }
                    m_pView->NoRotate();

                    bCheckShell = true; // if necessary turn on BezierShell
                }
                else if (!m_pSh->IsObjSelected() && !m_pWin->IsDrawAction())
                {
                    if (m_pSh->IsObjSelectable(aPnt))
                        m_pSh->SelectObj(aPnt, ( rMEvt.IsShift() &&
                            m_pSh->IsSelFrameMode() ) ? SW_ADD_SELECT : 0 );
                    else
                    {
                        m_pView->LeaveDrawCreate();
                        if (m_pSh->IsSelFrameMode())
                            m_pSh->LeaveSelFrameMode();
                    }
                    m_pView->NoRotate();

                    bReturn = true;
                }
            }
        }
        else
        {
            // BEZIER_EDITOR
            if ( pSdrView->IsAction() )
            {
                if ( pSdrView->IsInsObjPoint() )
                    bReturn = pSdrView->EndInsObjPoint(SDRCREATE_FORCEEND);
                else if (pSdrView->IsMarkPoints() )
                    bReturn = pSdrView->EndMarkPoints();
                else
                {
                    pSdrView->EndAction();
                    bReturn = true;
                }
                m_pWin->SetDrawAction(false);

                if (aPnt == m_aStartPos)
                {
                    if (!m_pSh->IsObjSelectable(aPnt))
                        m_pSh->SelectObj(Point(LONG_MAX, LONG_MAX));
                    else if (!bReturn)
                    {
                        if (!rMEvt.IsShift())
                            pSdrView->UnmarkAllPoints();
                        m_pSh->SelectObj(aPnt, (rMEvt.IsShift() &&
                                       m_pSh->IsSelFrameMode()) ? SW_ADD_SELECT :0);
                    }

                    if (!m_pSh->IsObjSelected())
                    {
                        m_pView->LeaveDrawCreate();    // Switch to selection mode

                        m_pSh->GetView().GetViewFrame()->GetBindings().Invalidate(SID_INSERT_DRAW);

                        if (m_pSh->IsSelFrameMode())
                            m_pSh->LeaveSelFrameMode();
                    }
                    m_pView->NoRotate();

                    bCheckShell = true; // if necessary turn on BezierShell
                }
            }

            SetDrawPointer();

            if (!m_pSh->IsObjSelected() && !m_pWin->IsDrawAction())
            {
                m_pView->LeaveDrawCreate();
                if (m_pSh->IsSelFrameMode())
                    m_pSh->LeaveSelFrameMode();

                m_pView->NoRotate();
                bReturn = true;
            }
        }
    }

    if (bCheckShell)
        m_pView->AttrChangedNotify( m_pSh ); // if necessary turn on BezierShell

    //!!!!!!!!!! Attention suicide !!!!!!!!!!! Everything should be renewed once
    if ( bAutoCap )
        m_pView->AutoCaption(FRAME_CAP);   //Can currently only be FRAME, otherwise convert
                                           // to enums
    return bReturn;
}

void SwDrawBase::Activate(const sal_uInt16 nSlot)
{
    SetSlotId(nSlot);
    SdrView *pSdrView = m_pSh->GetDrawView();

    pSdrView->SetCurrentObj( static_cast< sal_uInt16 >(m_pWin->GetSdrDrawMode()) );
    pSdrView->SetEditMode(false);

    SetDrawPointer();
    m_pSh->NoEdit();
}

void SwDrawBase::Deactivate()
{
    SdrView *pSdrView = m_pSh->GetDrawView();
    pSdrView->SetOrtho(false);
    pSdrView->SetAngleSnapEnabled(false);

    if (m_pWin->IsDrawAction() && m_pSh->IsDrawCreate())
        m_pSh->BreakCreate();

    m_pWin->SetDrawAction(false);

    m_pWin->ReleaseMouse();
    g_bNoInterrupt = false;

    if(m_pWin->GetApplyTemplate())
        m_pWin->SetApplyTemplate(SwApplyTemplate());
    m_pSh->GetView().GetViewFrame()->GetBindings().Invalidate(SID_INSERT_DRAW);
}

// Process keyboard events

// If a KeyEvent is processed then the return value is true, otherwise
// false.

bool SwDrawBase::KeyInput(const KeyEvent& rKEvt)
{
    bool bReturn = false;
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

            bReturn = true;
        }
        break;

        case KEY_DELETE:
        {
            m_pSh->DelSelectedObj();
            bReturn = true;
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
                    // Scroll to top
                    nX = 0;
                    nY =-1;
                }
                else if (nCode == KEY_DOWN)
                {
                    // Scroll down
                    nX = 0;
                    nY = 1;
                }
                else if (nCode == KEY_LEFT)
                {
                    // Scroll left
                    nX =-1;
                    nY = 0;
                }
                else if (nCode == KEY_RIGHT)
                {
                    // Scroll right
                    nX = 1;
                    nY = 0;
                }

                if (pSdrView->AreObjectsMarked() && rKEvt.GetKeyCode().IsMod2())
                {
                    // Move objects
                    nX *= 100;
                    nY *= 100;
                    pSdrView->MoveAllMarked(Size(nX, nY));
                }

                bReturn = true;
            }
        }
        break;
    }

    return bReturn;
}

// Process keyboard events

// If a KeyEvent is processed then the return value is true, otherwise
// false.

void SwDrawBase::BreakCreate()
{
    m_pSh->BreakCreate();
    m_pWin->SetDrawAction(false);
    m_pWin->ReleaseMouse();

    Deactivate();
}

void SwDrawBase::SetDrawPointer()
{
    SdrView *pSdrView = m_pSh->GetDrawView();
        Point aPnt(m_pWin->OutputToScreenPixel(m_pWin->GetPointerPosPixel()));
    aPnt = m_pWin->PixelToLogic(m_pWin->ScreenToOutputPixel(aPnt));
    const Pointer aPointTyp = pSdrView->GetPreferredPointer(aPnt, m_pSh->GetOut());
    const Pointer aDrawPt(aPointTyp);
    m_pWin->SetPointer(aDrawPt);
}

// If necessary switch into selection mode

void SwDrawBase::EnterSelectMode(const MouseEvent& rMEvt)
{
    m_pWin->SetDrawAction(false);

    if (!m_pSh->IsObjSelected() && !m_pWin->IsDrawAction())
    {
        Point aPnt(m_pWin->PixelToLogic(rMEvt.GetPosPixel()));

        if (m_pSh->IsObjSelectable(aPnt))
        {
            m_pSh->SelectObj(aPnt);
            if (rMEvt.GetModifier() == KEY_SHIFT || !m_pSh->IsObjSelected())
            {
                m_pView->LeaveDrawCreate();    // Switch to selection mode

                m_pSh->GetView().GetViewFrame()->GetBindings().Invalidate(SID_INSERT_DRAW);
            }
        }
        else
        {
            m_pView->LeaveDrawCreate();
            if (m_pSh->IsSelFrameMode())
                m_pSh->LeaveSelFrameMode();
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
