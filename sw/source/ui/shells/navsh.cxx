/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       [ Maja Djordjevic < ovcica@gmail.com > ]
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Cédric Bosdonnat <cbosdonnat@novell.com>
 *                 Caolan McNamara <caolanm@redhat.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include "cmdid.h"
#include <svx/svdview.hxx>
#include <svl/srchitem.hxx>
#include <svl/eitem.hxx>
#include <svl/whiter.hxx>
#include <svx/svdopath.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objface.hxx>

#include "wrtsh.hxx"
#include "view.hxx"
#include "edtwin.hxx"
#include "helpid.h"
#include "globals.hrc"

#include "navsh.hxx"


#include "popup.hrc"
#include "shells.hrc"
#define SwNavigationShell
#include "swslots.hxx"

#include <unomid.h>

#include "navmgr.hxx"


SFX_IMPL_INTERFACE(SwNavigationShell, SwBaseShell, SW_RES(STR_SHELLNAME_NAVIGATION))
{
}

SwNavigationShell::SwNavigationShell(SwView &_rView):
  SwBaseShell( _rView )

{
  SetName(C2S("Navigation"));
  SetHelpId(SW_NAVIGATIONSHELL);
}

void SwNavigationShell::Execute(SfxRequest &rReq)
{
    SwWrtShell *pSh = &GetShell();
    SdrView*    pSdrView = pSh->GetDrawView();
    const SfxItemSet *pArgs = rReq.GetArgs();
    sal_uInt16      nSlotId = rReq.GetSlot();
    sal_Bool        bChanged = pSdrView->GetModel()->IsChanged();
    pSdrView->GetModel()->SetChanged(sal_False);
    SwNavigationMgr& aSwNavigationMgr = pSh->GetNavigationMgr();
    const SfxPoolItem* pItem;
    if(pArgs)
        pArgs->GetItemState(nSlotId, sal_False, &pItem);
    switch (nSlotId)
        {
        case FN_NAVIGATION_BACK:
            aSwNavigationMgr.goBack();
            break;

        case FN_NAVIGATION_FORWARD:
            aSwNavigationMgr.goForward();
            break;
        default:
            break;
        }
    if (pSdrView->GetModel()->IsChanged())
        GetShell().SetModified();
    else if (bChanged)
        pSdrView->GetModel()->SetChanged(sal_True);
}

/*--------------------------------------------------------------------
  determine if the buttons should be enabled/disabled
  --------------------------------------------------------------------*/


void SwNavigationShell::GetState(SfxItemSet &rSet)
{
  SwWrtShell *pSh = &GetShell();
  SfxWhichIter aIter( rSet );
  sal_uInt16 nWhich = aIter.FirstWhich();
  SwNavigationMgr& aNavigationMgr = pSh->GetNavigationMgr();
  while( nWhich )
    {
      switch( nWhich )
    {
    case FN_NAVIGATION_BACK:
      {
        if (!aNavigationMgr.backEnabled()) {
          rSet.DisableItem(FN_NAVIGATION_BACK);
        }
      }
      break;
    case FN_NAVIGATION_FORWARD:
      {
        if (!aNavigationMgr.forwardEnabled())
          rSet.DisableItem(FN_NAVIGATION_FORWARD);
      }
      break;
    default:
      break;
    }
      nWhich = aIter.NextWhich();
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
