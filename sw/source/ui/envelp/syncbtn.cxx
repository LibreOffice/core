/*************************************************************************
 *
 *  $RCSfile: syncbtn.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:36 $
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

#include "uiparam.hxx"

#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif

#include "cmdid.h"
#include "swmodule.hxx"
#include "view.hxx"
#include "edtwin.hxx"
#include "label.hrc"

#define _SYNCDLG
#include "syncbtn.hxx"
#include "swtypes.hxx"

SFX_IMPL_FLOATINGWINDOW( SwSyncChildWin, FN_SYNC_LABELS )

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

SwSyncChildWin::SwSyncChildWin( Window* pParent,
                                USHORT nId,
                                SfxBindings* pBindings,
                                SfxChildWinInfo* pInfo ) :
                                SfxChildWindow( pParent, nId )
{
    pWindow = new SwSyncBtnDlg( pBindings, this, pParent);

    if (!pInfo->aSize.Width() || !pInfo->aSize.Height())
    {
        const SwEditWin &rEditWin = ::GetActiveView()->GetEditWin();
        pWindow->SetPosPixel(rEditWin.OutputToScreenPixel(Point(0, 0)));
        pInfo->aPos = pWindow->GetPosPixel();
        pInfo->aSize = pWindow->GetSizePixel();
    }

    ((SwSyncBtnDlg *)pWindow)->Initialize(pInfo);

    pWindow->Show();
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

SwSyncBtnDlg::SwSyncBtnDlg( SfxBindings* pBindings,
                            SfxChildWindow* pChild,
                            Window *pParent) :
    SfxFloatingWindow(pBindings, pChild, pParent, SW_RES(DLG_SYNC_BTN)),
    aSyncBtn        (this, SW_RES(BTN_SYNC ))
{
    FreeResource();
    aSyncBtn.SetClickHdl(LINK(this, SwSyncBtnDlg, BtnHdl));
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

__EXPORT SwSyncBtnDlg::~SwSyncBtnDlg()
{
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwSyncBtnDlg, BtnHdl, PushButton *, pBtn )
{
    SfxViewFrame::Current()->GetDispatcher()->Execute(FN_UPDATE_ALL_LINKS, SFX_CALLMODE_ASYNCHRON);
    return 0;
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.6  2000/09/18 16:05:27  willem.vandorp
    OpenOffice header added.

    Revision 1.5  2000/09/07 15:59:23  os
    change: SFX_DISPATCHER/SFX_BINDINGS removed

    Revision 1.4  1998/03/18 10:27:32  OM
    #48713# SynchronButton initial oben links positionieren


      Rev 1.3   18 Mar 1998 11:27:32   OM
   #48713# SynchronButton initial oben links positionieren

      Rev 1.2   16 Mar 1998 20:11:34   OM
   Initial in der View positionieren

      Rev 1.1   16 Mar 1998 16:17:40   OM
   Aktualisieren-Button kontextsensitiv

      Rev 1.0   15 Mar 1998 14:09:52   OM
   Initial revision.

------------------------------------------------------------------------*/
