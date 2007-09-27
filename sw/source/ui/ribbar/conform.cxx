/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: conform.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:24:23 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#ifndef _SVX_FMGLOB_HXX //autogen
#include <svx/fmglob.hxx>
#endif
#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
#endif
#ifndef _SVX_FMSHELL_HXX //autogen
#include <svx/fmshell.hxx>
#endif

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

