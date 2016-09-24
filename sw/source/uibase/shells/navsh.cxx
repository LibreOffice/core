/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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

SFX_IMPL_INTERFACE(SwNavigationShell, SwBaseShell)

void SwNavigationShell::InitInterface_Impl()
{
}

SwNavigationShell::SwNavigationShell(SwView &_rView)
    : SwBaseShell( _rView )
{
    SetName("Navigation");
    SetHelpId(SW_NAVIGATIONSHELL);
}

void SwNavigationShell::Execute(SfxRequest &rReq)
{
    SwWrtShell *pSh = &GetShell();
    SdrView* pSdrView = pSh->GetDrawView();
    const SfxItemSet *pArgs = rReq.GetArgs();
    const sal_uInt16 nSlotId = rReq.GetSlot();
    bool bChanged = pSdrView->GetModel()->IsChanged();
    pSdrView->GetModel()->SetChanged(false);
    SwNavigationMgr& aSwNavigationMgr = pSh->GetNavigationMgr();
    const SfxPoolItem* pItem;
    if(pArgs)
        pArgs->GetItemState(nSlotId, false, &pItem);
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
        pSdrView->GetModel()->SetChanged();
}

// determine if the buttons should be enabled/disabled

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
                if (!aNavigationMgr.backEnabled())
                {
                    rSet.DisableItem(FN_NAVIGATION_BACK);
                }
                break;
            case FN_NAVIGATION_FORWARD:
                if (!aNavigationMgr.forwardEnabled())
                {
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
