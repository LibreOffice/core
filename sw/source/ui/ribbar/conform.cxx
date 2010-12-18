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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <svx/fmglob.hxx>
#include <svx/svdview.hxx>
#include <svx/fmshell.hxx>

#include "view.hxx"
#include "edtwin.hxx"
#include "wrtsh.hxx"
#include "drawbase.hxx"
#include "conform.hxx"

extern BOOL bNoInterrupt;       // in mainwn.cxx

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/


ConstFormControl::ConstFormControl(SwWrtShell* pWrtShell, SwEditWin* pEditWin, SwView* pSwView) :
    SwDrawBase(pWrtShell, pEditWin, pSwView)
{
    m_bInsForm = TRUE;
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/


BOOL ConstFormControl::MouseButtonDown(const MouseEvent& rMEvt)
{
    BOOL bReturn = FALSE;

    SdrView *pSdrView = m_pSh->GetDrawView();

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
    if (rMEvt.IsLeft() && !m_pWin->IsDrawAction() &&
        (eHit == SDRHIT_UNMARKEDOBJECT || eHit == SDRHIT_NONE || m_pSh->IsDrawCreate()))
    {
        bNoInterrupt = TRUE;
        m_pWin->CaptureMouse();

        m_pWin->SetPointer(Pointer(POINTER_DRAW_RECT));

        m_aStartPos = m_pWin->PixelToLogic(rMEvt.GetPosPixel());
        bReturn = m_pSh->BeginCreate( static_cast< UINT16 >(m_pWin->GetSdrDrawMode()), FmFormInventor, m_aStartPos);

        if (bReturn)
            m_pWin->SetDrawAction(TRUE);
    }
    else
        bReturn = SwDrawBase::MouseButtonDown(rMEvt);

    return (bReturn);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/


void ConstFormControl::Activate(const USHORT nSlotId)
{
    m_pWin->SetSdrDrawMode( static_cast<SdrObjKind>(nSlotId) );
    SwDrawBase::Activate(nSlotId);
    m_pSh->GetDrawView()->SetCurrentObj(nSlotId);

    m_pWin->SetPointer(Pointer(POINTER_DRAW_RECT));
}
/* -----------------------------19.04.2002 12:42------------------------------

 ---------------------------------------------------------------------------*/
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
    pSdrView->SetDesignMode(TRUE);
    m_pSh->BeginCreate( static_cast< UINT16 >(m_pWin->GetSdrDrawMode()), FmFormInventor, aStartPos);
    m_pSh->MoveCreate(aEndPos);
    m_pSh->EndCreate(SDRCREATE_FORCEEND);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
