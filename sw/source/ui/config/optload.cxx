/*************************************************************************
 *
 *  $RCSfile: optload.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:33 $
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

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

#ifndef _SWTYPES_HXX //autogen
#include <swtypes.hxx>
#endif

#ifndef _UIITEMS_HXX //autogen
#include <uiitems.hxx>
#endif

#include "swmodule.hxx"
#include "modcfg.hxx"
#include "wrtsh.hxx"

#include "globals.hrc"
#include "cmdid.h"

#include "optload.hrc"
#include "optload.hxx"

#ifndef _FLDUPDE_HXX
#include <fldupde.hxx>
#endif

/* -----------------22.10.98 15:12-------------------
 *
 * --------------------------------------------------*/

SwLoadOptPage::SwLoadOptPage( Window* pParent, const SfxItemSet& rSet ) :
    SfxTabPage(pParent, SW_RES(TP_OPTLOAD_PAGE), rSet),
    aAlwaysRB   (this, ResId(RB_ALWAYS  )),
    aRequestRB  (this, ResId(RB_REQUEST )),
    aNeverRB    (this, ResId(RB_NEVER   )),
    aLinkDocOnlyCB  (this, ResId(CB_LINK_DOCONLY )),
    aLinkGB     (this, ResId(GB_LINK    )),
    aAutoUpdateFields(this, ResId(CB_AUTO_UPDATE_FIELDS )),
    aAutoUpdateCharts(this, ResId(CB_AUTO_UPDATE_CHARTS )),
    aFldDocOnlyCB  (this, ResId(CB_FLD_DOCONLY )),
    aFldGB      (this, ResId(GB_FIELD   )),
    aMergeDistCB(this, ResId(CB_MERGE_PARA_DIST )),
    aMergeDistPageStartCB(this, ResId(CB_MERGE_PARA_DIST_PAGESTART  )),
    aCompatGB   (this, ResId(GB_COMPAT  )),

    pWrtShell   (0),
    nOldLinkMode(MANUAL)
{
    FreeResource();

    aAutoUpdateFields.SetClickHdl(LINK(this, SwLoadOptPage, UpdateHdl));
}

/*-----------------18.01.97 12.43-------------------

--------------------------------------------------*/

SwLoadOptPage::~SwLoadOptPage()
{
}
/* -----------------09.02.99 12:10-------------------
 *
 * --------------------------------------------------*/
IMPL_LINK(  SwLoadOptPage, UpdateHdl, CheckBox*, pBox )
{
    aAutoUpdateCharts.Enable(pBox->IsChecked());
    return 0;
}

/*-----------------18.01.97 12.43-------------------

--------------------------------------------------*/

SfxTabPage* __EXPORT SwLoadOptPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet )
{
    return new SwLoadOptPage(pParent, rAttrSet );
}

/*-----------------18.01.97 12.42-------------------

--------------------------------------------------*/

BOOL __EXPORT SwLoadOptPage::FillItemSet( SfxItemSet& rSet )
{
    BOOL bRet = FALSE;
    SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();
    BOOL bFldDocOnly = aFldDocOnlyCB.IsChecked();
    BOOL bLinkDocOnly = aLinkDocOnlyCB.IsChecked();

    USHORT nNewLinkMode;
    if (aNeverRB.IsChecked())
        nNewLinkMode = NEVER;
    else if (aRequestRB.IsChecked())
        nNewLinkMode = MANUAL;
    else if (aAlwaysRB.IsChecked())
        nNewLinkMode = AUTOMATIC;

    USHORT nFldFlags = aAutoUpdateFields.IsChecked() ?
        aAutoUpdateCharts.IsChecked() ? AUTOUPD_FIELD_AND_CHARTS : AUTOUPD_FIELD_ONLY : AUTOUPD_OFF;

    if(aAutoUpdateFields.IsChecked() != aAutoUpdateFields.GetSavedValue() ||
            aAutoUpdateCharts.IsChecked() != aAutoUpdateCharts.GetSavedValue())
    {
        if (!bFldDocOnly || !pWrtShell)
            pModOpt->SetFldUpdateFlags(nFldFlags);
        if(pWrtShell)
        {
            USHORT nSet = bFldDocOnly ? nFldFlags : AUTOUPD_GLOBALSETTING;
            pWrtShell->SetFldUpdateFlags(nSet);
            pWrtShell->SetModified();
        }
    }

    if (nNewLinkMode != nOldLinkMode)
    {
        if (!bLinkDocOnly || !pWrtShell)
            pModOpt->SetLinkMode(nNewLinkMode);

        if (pWrtShell)
        {
            USHORT nSet = bLinkDocOnly ? nNewLinkMode : (USHORT)GLOBALSETTING;
            pWrtShell->SetLinkUpdMode( nSet );
            pWrtShell->SetModified();
        }

        bRet = TRUE;
    }
    if (pWrtShell)
    {
        if(aMergeDistCB.IsChecked() != aMergeDistCB.GetSavedValue() ||
            aMergeDistPageStartCB.IsChecked() != aMergeDistPageStartCB.GetSavedValue())
        {
            pWrtShell->SetParaSpaceMax(aMergeDistCB.IsChecked(),
                                aMergeDistPageStartCB.IsChecked());
        }

    }
    return bRet;
}
/*-----------------18.01.97 12.42-------------------

--------------------------------------------------*/

void __EXPORT SwLoadOptPage::Reset( const SfxItemSet& rSet)
{
    const SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();
    const SfxPoolItem* pItem;

    if(SFX_ITEM_SET == rSet.GetItemState(FN_PARAM_WRTSHELL, FALSE, &pItem))
        pWrtShell = (SwWrtShell*)((const SwPtrItem*)pItem)->GetValue();

    USHORT nFldFlags = AUTOUPD_GLOBALSETTING;
    nOldLinkMode = GLOBALSETTING;
    BOOL bFldDocOnly = TRUE;
    BOOL bLinkDocOnly = TRUE;
    if (pWrtShell)
    {
        nFldFlags = pWrtShell->GetFldUpdateFlags(TRUE);
        nOldLinkMode = pWrtShell->GetLinkUpdMode(TRUE);
        aMergeDistCB.Check(pWrtShell->IsParaSpaceMax());
        aMergeDistPageStartCB.Check(pWrtShell->IsParaSpaceMaxAtPages());

        aMergeDistCB.SaveValue();
        aMergeDistPageStartCB.SaveValue();
    }
    if(nOldLinkMode == GLOBALSETTING)
    {
        nOldLinkMode = pModOpt->GetLinkMode();
        bLinkDocOnly = FALSE;
    }
    if(nFldFlags == AUTOUPD_GLOBALSETTING)
    {
        nFldFlags = pModOpt->GetFldUpdateFlags();
        bFldDocOnly = FALSE;
    }

    aAutoUpdateFields.Check(nFldFlags != AUTOUPD_OFF);
    aAutoUpdateCharts.Check(nFldFlags == AUTOUPD_FIELD_AND_CHARTS);
    aAutoUpdateCharts.Enable(nFldFlags != AUTOUPD_OFF);

    aLinkDocOnlyCB.Enable(pWrtShell != 0);
    aLinkDocOnlyCB.Check(bLinkDocOnly);
    aFldDocOnlyCB.Enable(pWrtShell != 0);
    aFldDocOnlyCB.Check(bFldDocOnly);
    aMergeDistCB.Enable(pWrtShell != 0);
    aMergeDistPageStartCB.Enable(pWrtShell != 0);
    aCompatGB.Enable(pWrtShell != 0);

    switch (nOldLinkMode)
    {
        case NEVER:     aNeverRB.Check();   break;
        case MANUAL:    aRequestRB.Check(); break;
        case AUTOMATIC: aAlwaysRB.Check();  break;
    }

    aAutoUpdateFields.SaveValue();
    aAutoUpdateCharts.SaveValue();
}

/*--------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.12  2000/09/18 16:05:17  willem.vandorp
    OpenOffice header added.

    Revision 1.11  1999/10/01 06:04:24  os
    enable new compatibility button

    Revision 1.10  1999/09/30 09:59:56  os
    new compatibility option

    Revision 1.9  1999/09/30 09:45:35  os
    new compatibility option

    Revision 1.8  1999/03/15 15:22:58  OS
    #63171# Fieldpdate und LinkMode in getrennten Gruppen


      Rev 1.7   15 Mar 1999 16:22:58   OS
   #63171# Fieldpdate und LinkMode in getrennten Gruppen

      Rev 1.6   11 Mar 1999 23:56:38   JP
   Task #63171#: Optionen fuer Feld-/LinkUpdate Doc oder Modul lokal

      Rev 1.5   09 Feb 1999 15:19:10   OS
   #61404# Optionen fuer Felder/Charts aktualisieren

      Rev 1.4   07 Jan 1999 15:04:24   OS
   #60625# Absatzabstaende addieren

      Rev 1.3   03 Dec 1998 11:24:54   OM
   #58216# Verknuepfungsoptionen nur fuer aktuelles Dok

      Rev 1.2   30 Nov 1998 17:30:12   OM
   #59770# Tabellenoptionen: Ueberschrift nur auf erster Seite

      Rev 1.1   23 Nov 1998 16:13:42   OM
   #58216# Verknuepfungsoptionen pro Dokument

      Rev 1.0   20 Nov 1998 17:03:42   OM
   Initial revision.

 --------------------------------------------------------------------*/
