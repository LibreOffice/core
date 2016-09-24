/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "RecentMasterPagesSelector.hxx"

#include "ViewShellBase.hxx"
#include "RecentlyUsedMasterPages.hxx"
#include "MasterPageContainerProviders.hxx"
#include "MasterPageObserver.hxx"
#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "app.hrc"
#include "helpids.h"

#include <vcl/bitmap.hxx>

namespace sd { namespace sidebar {

VclPtr<vcl::Window> RecentMasterPagesSelector::Create (
    vcl::Window* pParent,
    ViewShellBase& rViewShellBase,
    const css::uno::Reference<css::ui::XSidebar>& rxSidebar)
{
    SdDrawDocument* pDocument = rViewShellBase.GetDocument();
    if (pDocument == nullptr)
        return nullptr;

    std::shared_ptr<MasterPageContainer> pContainer (new MasterPageContainer());

    VclPtrInstance<RecentMasterPagesSelector> pSelector(
            pParent,
            *pDocument,
            rViewShellBase,
            pContainer,
            rxSidebar);
    pSelector->LateInit();
    pSelector->SetHelpId(HID_SD_TASK_PANE_PREVIEW_RECENT);

    return pSelector;
}

RecentMasterPagesSelector::RecentMasterPagesSelector (
    vcl::Window* pParent,
    SdDrawDocument& rDocument,
    ViewShellBase& rBase,
    const std::shared_ptr<MasterPageContainer>& rpContainer,
    const css::uno::Reference<css::ui::XSidebar>& rxSidebar)
    : MasterPagesSelector (pParent, rDocument, rBase, rpContainer, rxSidebar)
{
}

RecentMasterPagesSelector::~RecentMasterPagesSelector()
{
    disposeOnce();
}

void RecentMasterPagesSelector::dispose()
{
    RecentlyUsedMasterPages::Instance().RemoveEventListener (
        LINK(this,RecentMasterPagesSelector,MasterPageListListener));
    MasterPagesSelector::dispose();
}

void RecentMasterPagesSelector::LateInit()
{
    MasterPagesSelector::LateInit();

    MasterPagesSelector::Fill();
    RecentlyUsedMasterPages::Instance().AddEventListener (
        LINK(this,RecentMasterPagesSelector,MasterPageListListener));
}

IMPL_LINK_NOARG_TYPED(RecentMasterPagesSelector, MasterPageListListener, LinkParamNone*, void)
{
    MasterPagesSelector::Fill();
}

void RecentMasterPagesSelector::Fill (ItemList& rItemList)
{
    // Create a set of names of the master pages used by the document.
    MasterPageObserver::MasterPageNameSet aCurrentNames;
    sal_uInt16 nMasterPageCount = mrDocument.GetMasterSdPageCount(PK_STANDARD);
    sal_uInt16 nIndex;
    for (nIndex=0; nIndex<nMasterPageCount; nIndex++)
    {
        SdPage* pMasterPage = mrDocument.GetMasterSdPage (nIndex, PK_STANDARD);
        if (pMasterPage != nullptr)
            aCurrentNames.insert (pMasterPage->GetName());
    }

    // Insert the recently used master pages that are currently not used.
    RecentlyUsedMasterPages& rInstance (RecentlyUsedMasterPages::Instance());
    int nPageCount = rInstance.GetMasterPageCount();
    for (nIndex=0; nIndex<nPageCount; nIndex++)
    {
        // Add an entry when a) the page is already known to the
        // MasterPageContainer, b) the style name is empty, i.e. it has not yet
        // been loaded (and thus can not be in use) or otherwise c) the
        // style name is not currently in use.
        MasterPageContainer::Token aToken (rInstance.GetTokenForIndex(nIndex));
        if (aToken != MasterPageContainer::NIL_TOKEN)
        {
            OUString sStyleName (mpContainer->GetStyleNameForToken(aToken));
            if (sStyleName.isEmpty()
                || aCurrentNames.find(sStyleName) == aCurrentNames.end())
            {
                rItemList.push_back(aToken);
            }
        }
    }
}

void RecentMasterPagesSelector::AssignMasterPageToPageList (
    SdPage* pMasterPage,
    const std::shared_ptr<std::vector<SdPage*> >& rpPageList)
{
    sal_uInt16 nSelectedItemId = PreviewValueSet::GetSelectItemId();

    MasterPagesSelector::AssignMasterPageToPageList(pMasterPage, rpPageList);

    // Restore the selection.
    if (PreviewValueSet::GetItemCount() > 0)
    {
        if (PreviewValueSet::GetItemCount() >= nSelectedItemId)
            PreviewValueSet::SelectItem(nSelectedItemId);
        else
            PreviewValueSet::SelectItem(PreviewValueSet::GetItemCount());
    }
}

void RecentMasterPagesSelector::ProcessPopupMenu (Menu& rMenu)
{
    if (rMenu.GetItemPos(SID_TP_EDIT_MASTER) != MENU_ITEM_NOTFOUND)
        rMenu.EnableItem(SID_TP_EDIT_MASTER, false);
}

} } // end of namespace sd::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
