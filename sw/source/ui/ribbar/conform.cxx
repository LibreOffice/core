/*************************************************************************
 *
 *  $RCSfile: conform.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: os $ $Date: 2002-10-25 13:09:48 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop


#ifndef _SVX_FMGLOB_HXX //autogen
#include <svx/fmglob.hxx>
#endif
#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
#endif
#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
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
    bInsForm = TRUE;
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/


BOOL ConstFormControl::MouseButtonDown(const MouseEvent& rMEvt)
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
    if (rMEvt.IsLeft() && !pWin->IsDrawAction() &&
        (eHit == SDRHIT_UNMARKEDOBJECT || eHit == SDRHIT_NONE || pSh->IsDrawCreate()))
    {
        bNoInterrupt = TRUE;
        pWin->CaptureMouse();

        pWin->SetPointer(Pointer(POINTER_DRAW_RECT));

        aStartPos = pWin->PixelToLogic(rMEvt.GetPosPixel());
        bReturn = pSh->BeginCreate(pWin->GetDrawMode(), FmFormInventor, aStartPos);

        if (bReturn)
            pWin->SetDrawAction(TRUE);
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
    pWin->SetDrawMode(nSlotId);
    SwDrawBase::Activate(nSlotId);
    pSh->GetDrawView()->SetCurrentObj(nSlotId);

    pWin->SetPointer(Pointer(POINTER_DRAW_RECT));
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

    if(!pSh->HasDrawView())
        pSh->MakeDrawView();

    SdrView *pSdrView = pSh->GetDrawView();
    pSdrView->SetDesignMode(TRUE);
    pSh->BeginCreate(pWin->GetDrawMode(), FmFormInventor, aStartPos);
    pSh->MoveCreate(aEndPos);
    pSh->EndCreate(SDRCREATE_FORCEEND);
}

