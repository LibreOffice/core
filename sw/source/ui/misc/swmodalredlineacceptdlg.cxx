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
#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif



#define _SVSTDARR_STRINGSSORTDTOR
#define _SVSTDARR_USHORTSSORT
#define _SVSTDARR_USHORTS


#include <redline.hxx>
#include <tools/datetime.hxx>
#include <vcl/msgbox.hxx>
#ifndef _SVSTDARR_HXX
#include <svl/svstdarr.hxx>
#endif
#include <svl/eitem.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/ctredlin.hxx>
#include <svx/postattr.hxx>
#include <swtypes.hxx>
#include <wrtsh.hxx>
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#include <swmodule.hxx>
#include <swwait.hxx>
#include <uitool.hxx>

#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _MISC_HRC
#include <misc.hrc>
#endif
#ifndef _REDLNDLG_HRC
#include <redlndlg.hrc>
#endif
#ifndef _SHELLS_HRC
#include <shells.hrc>
#endif

#include <vector>
#ifndef _REDLNDLG_HXX
#define _REDLNACCEPTDLG
#include <redlndlg.hxx>
#endif
#include "swmodalredlineacceptdlg.hxx"

#include <unomid.h>


SwModalRedlineAcceptDlg::SwModalRedlineAcceptDlg(Window *pParent) :
    SfxModalDialog(pParent, SW_RES(DLG_MOD_REDLINE_ACCEPT))
{
    pImplDlg = new SwRedlineAcceptDlg(this, sal_True);

    pImplDlg->Initialize(GetExtraData());
    pImplDlg->Activate();   // Zur Initialisierung der Daten

    FreeResource();
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

SwModalRedlineAcceptDlg::~SwModalRedlineAcceptDlg()
{
    AcceptAll(sal_False);   // Alles uebriggebliebene ablehnen
    pImplDlg->FillInfo(GetExtraData());

    delete pImplDlg;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwModalRedlineAcceptDlg::Activate()
{
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

void SwModalRedlineAcceptDlg::Resize()
{
    pImplDlg->Resize();
    SfxModalDialog::Resize();
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

void SwModalRedlineAcceptDlg::AcceptAll( sal_Bool bAccept )
{
    SvxTPFilter* pFilterTP = pImplDlg->GetChgCtrl()->GetFilterPage();

    if (pFilterTP->IsDate() || pFilterTP->IsAuthor() ||
        pFilterTP->IsRange() || pFilterTP->IsAction())
    {
        pFilterTP->CheckDate(sal_False);    // Alle Filter abschalten
        pFilterTP->CheckAuthor(sal_False);
        pFilterTP->CheckRange(sal_False);
        pFilterTP->CheckAction(sal_False);
        pImplDlg->FilterChangedHdl();
    }

    pImplDlg->CallAcceptReject( sal_False, bAccept );
}
