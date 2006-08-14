/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dselect.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-08-14 17:52:19 $
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

#pragma hdrstop

#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#include "view.hxx"
#include "edtwin.hxx"
#include "wrtsh.hxx"
#include "cmdid.h"
#include "drawbase.hxx"
#include "dselect.hxx"

extern BOOL bNoInterrupt;       // in mainwn.cxx

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

DrawSelection::DrawSelection(SwWrtShell* pWrtShell, SwEditWin* pEditWin, SwView* pSwView) :
                SwDrawBase(pWrtShell, pEditWin, pSwView)
{
    bCreateObj = FALSE;
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert TRUE, andernfalls
|* FALSE.
|*
\************************************************************************/

BOOL DrawSelection::KeyInput(const KeyEvent& rKEvt)
{
    BOOL bReturn = FALSE;

    switch (rKEvt.GetKeyCode().GetCode())
    {
        case KEY_ESCAPE:
        {
            if (pWin->IsDrawAction())
            {
                pSh->BreakMark();
                pWin->ReleaseMouse();
            }
            bReturn = TRUE;
        }
        break;
    }

    if (!bReturn)
        bReturn = SwDrawBase::KeyInput(rKEvt);

    return (bReturn);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void DrawSelection::Activate(const USHORT nSlotId)
{
    pWin->SetDrawMode(SID_OBJECT_SELECT);
    SwDrawBase::Activate(nSlotId);

    pSh->GetView().GetViewFrame()->GetBindings().Invalidate(SID_INSERT_DRAW);
}


