/*************************************************************************
 *
 *  $RCSfile: dselect.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:46 $
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

#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
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

/*************************************************************************

      Source Code Control System - History

      $Log: not supported by cvs2svn $
      Revision 1.44  2000/09/18 16:06:01  willem.vandorp
      OpenOffice header added.

      Revision 1.43  2000/09/07 15:59:27  os
      change: SFX_DISPATCHER/SFX_BINDINGS removed

      Revision 1.42  1998/06/12 11:33:44  OM
      Wieder SID_OBJECT_SELECT statt SID_DRAW_SELECT


      Rev 1.41   12 Jun 1998 13:33:44   OM
   Wieder SID_OBJECT_SELECT statt SID_DRAW_SELECT

      Rev 1.40   09 Jun 1998 15:34:00   OM
   VC-Controls entfernt

      Rev 1.39   28 Nov 1997 20:15:30   MA
   includes

      Rev 1.38   24 Nov 1997 14:53:58   MA
   includes

      Rev 1.37   03 Nov 1997 13:24:22   MA
   precomp entfernt

      Rev 1.36   05 Sep 1997 12:00:44   MH
   chg: header

      Rev 1.35   13 Aug 1997 12:18:56   OM
   #42383# Selektion von Punkten im Konstruktionsmode

      Rev 1.34   11 Jul 1997 15:26:24   OM
   Redraw-Bug behoben

      Rev 1.33   07 Apr 1997 13:39:48   MH
   chg: header

      Rev 1.32   20 Mar 1997 11:34:16   OM
   Insert und CreateMode merken

      Rev 1.31   14 Mar 1997 16:36:40   OM
   #37682# Bemassungsobjekt deselektieren

      Rev 1.30   04 Feb 1997 15:20:32   OM
   Bugs in Selektionsmodi behoben

      Rev 1.29   31 Jan 1997 10:58:26   OM
   Statusupdate

      Rev 1.28   30 Jan 1997 11:48:10   OM
   Freihandzeichnen

      Rev 1.27   22 Jan 1997 11:19:50   OM
   Neue Shells: DrawBaseShell und DrawControlShell

      Rev 1.26   14 Jan 1997 15:37:06   OM
   Bezierpunkte bearbeiten

      Rev 1.25   13 Jan 1997 17:23:28   OM
   Bezier-Punkte einfuegen

      Rev 1.24   13 Jan 1997 15:50:18   OM
   Bezier-Punkte loeschen

      Rev 1.23   13 Jan 1997 10:11:54   OM
   Selektionsmode fuer Bezier-Objekte

      Rev 1.22   08 Jan 1997 12:44:26   OM
   Aenderungen fuer Bezier-Shell

      Rev 1.21   28 Aug 1996 13:39:24   OS
   includes

      Rev 1.20   16 Aug 1996 15:22:32   OM
   #30046#Haenger bei Selektion beseitigt

      Rev 1.19   14 Aug 1996 11:31:00   JP
   svdraw.hxx entfernt

      Rev 1.18   07 Mar 1996 14:27:32   HJS
   2 defines zu viel

      Rev 1.17   24 Nov 1995 16:58:06   OM
   PCH->PRECOMPILED

      Rev 1.16   15 Nov 1995 18:35:28   OM
   Virtuelle Fkt rausgeworfen

      Rev 1.15   14 Nov 1995 17:36:56   OM
   Cast entfernt

      Rev 1.14   08 Nov 1995 15:29:48   OM
   Invalidate-Reihenfolge geaendert

      Rev 1.13   08 Nov 1995 14:15:50   OS
   Change => Set

      Rev 1.12   08 Nov 1995 10:35:42   OM
   Invalidate optimiert

      Rev 1.11   19 Oct 1995 18:30:44   OM
   Acceleratorproblem behoben

*************************************************************************/

