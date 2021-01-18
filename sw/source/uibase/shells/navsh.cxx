/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cmdid.h>
#include <svx/svdview.hxx>
#include <svl/whiter.hxx>
#include <sfx2/request.hxx>
#include <sfx2/objface.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <navsh.hxx>
#define ShellClass_SwNavigationShell
#include <swslots.hxx>
#include <navmgr.hxx>

SFX_IMPL_INTERFACE(SwNavigationShell, SwBaseShell)

void SwNavigationShell::InitInterface_Impl() {}

SwNavigationShell::SwNavigationShell(SwView& _rView)
    : SwBaseShell(_rView)
{
    SetName("Navigation");
}

void SwNavigationShell::Execute(SfxRequest const& rReq)
{
    SwWrtShell* pSh = &GetShell();
    SdrView* pSdrView = pSh->GetDrawView();
    const SfxItemSet* pArgs = rReq.GetArgs();
    const sal_uInt16 nSlotId = rReq.GetSlot();
    bool bChanged = pSdrView->GetModel()->IsChanged();
    pSdrView->GetModel()->SetChanged(false);
    SwNavigationMgr& aSwNavigationMgr = pSh->GetNavigationMgr();
    const SfxPoolItem* pItem;
    if (pArgs)
        pArgs->GetItemState(nSlotId, false, &pItem);

    if (pSdrView->IsTextEdit())
        pSh->EndTextEdit();
    if (pSh->GetView().IsDrawMode())
        pSh->GetView().LeaveDrawCreate();
    pSh->EnterStdMode();

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

void SwNavigationShell::GetState(SfxItemSet& rSet)
{
    SwWrtShell* pSh = &GetShell();
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    SwNavigationMgr& aNavigationMgr = pSh->GetNavigationMgr();
    while (nWhich)
    {
        switch (nWhich)
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
