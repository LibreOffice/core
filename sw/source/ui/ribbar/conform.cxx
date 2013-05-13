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

#include <svx/fmglob.hxx>
#include <svx/svdview.hxx>
#include <svx/fmshell.hxx>

#include "view.hxx"
#include "edtwin.hxx"
#include "wrtsh.hxx"
#include "drawbase.hxx"
#include "conform.hxx"

extern bool bNoInterrupt;       // in mainwn.cxx

ConstFormControl::ConstFormControl(SwWrtShell* pWrtShell, SwEditWin* pEditWin, SwView* pSwView) :
    SwDrawBase(pWrtShell, pEditWin, pSwView)
{
    m_bInsForm = true;
}

sal_Bool ConstFormControl::MouseButtonDown(const MouseEvent& rMEvt)
{
    sal_Bool bReturn = sal_False;

    SdrView *pSdrView = m_pSh->GetDrawView();

    pSdrView->SetOrtho(rMEvt.IsShift());
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

    // Only new object; if not in base mode (or pure selection mode)
    if (rMEvt.IsLeft() && !m_pWin->IsDrawAction() &&
        (eHit == SDRHIT_UNMARKEDOBJECT || eHit == SDRHIT_NONE || m_pSh->IsDrawCreate()))
    {
        bNoInterrupt = true;
        m_pWin->CaptureMouse();

        m_pWin->SetPointer(Pointer(POINTER_DRAW_RECT));

        m_aStartPos = m_pWin->PixelToLogic(rMEvt.GetPosPixel());
        bReturn = m_pSh->BeginCreate( static_cast< sal_uInt16 >(m_pWin->GetSdrDrawMode()), FmFormInventor, m_aStartPos);

        if (bReturn)
            m_pWin->SetDrawAction(sal_True);
    }
    else
        bReturn = SwDrawBase::MouseButtonDown(rMEvt);

    return (bReturn);
}

void ConstFormControl::Activate(const sal_uInt16 nSlotId)
{
    m_pWin->SetSdrDrawMode( static_cast<SdrObjKind>(nSlotId) );
    SwDrawBase::Activate(nSlotId);
    m_pSh->GetDrawView()->SetCurrentObj(nSlotId);

    m_pWin->SetPointer(Pointer(POINTER_DRAW_RECT));
}

void ConstFormControl::CreateDefaultObject()
{
    Point aStartPos(GetDefaultCenterPos());
    Point aEndPos(aStartPos);
    aStartPos.X() -= 2 * MM50;
    aStartPos.Y() -= MM50;
    aEndPos.X() += 2 * MM50;
    aEndPos.Y() += MM50;

    if(!m_pSh->HasDrawView())
        m_pSh->MakeDrawView();

    SdrView *pSdrView = m_pSh->GetDrawView();
    pSdrView->SetDesignMode(sal_True);
    m_pSh->BeginCreate( static_cast< sal_uInt16 >(m_pWin->GetSdrDrawMode()), FmFormInventor, aStartPos);
    m_pSh->MoveCreate(aEndPos);
    m_pSh->EndCreate(SDRCREATE_FORCEEND);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
