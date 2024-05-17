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

#include <ViewShellBase.hxx>
#include "RecentlyUsedMasterPages.hxx"
#include <MasterPageObserver.hxx>
#include <sdpage.hxx>
#include <drawdoc.hxx>
#include <helpids.h>

namespace sd::sidebar {

std::unique_ptr<PanelLayout> RecentMasterPagesSelector::Create (
    weld::Widget* pParent,
    ViewShellBase& rViewShellBase,
    const css::uno::Reference<css::ui::XSidebar>& rxSidebar)
{
    SdDrawDocument* pDocument = rViewShellBase.GetDocument();
    if (pDocument == nullptr)
        return nullptr;

    auto pContainer = std::make_shared<MasterPageContainer>();

    auto xSelector(std::make_unique<RecentMasterPagesSelector>(
            pParent,
            *pDocument,
            rViewShellBase,
            pContainer,
            rxSidebar));
    xSelector->LateInit();
    xSelector->SetHelpId(HID_SD_TASK_PANE_PREVIEW_RECENT);

    return xSelector;
}

RecentMasterPagesSelector::RecentMasterPagesSelector (
    weld::Widget* pParent,
    SdDrawDocument& rDocument,
    ViewShellBase& rBase,
    const std::shared_ptr<MasterPageContainer>& rpContainer,
    const css::uno::Reference<css::ui::XSidebar>& rxSidebar)
    : MasterPagesSelector (pParent, rDocument, rBase, rpContainer, rxSidebar, u"modules/simpress/ui/masterpagepanelrecent.ui"_ustr, u"recentvalueset"_ustr)
{
}

RecentMasterPagesSelector::~RecentMasterPagesSelector()
{
    RecentlyUsedMasterPages::Instance().RemoveEventListener (
        LINK(this,RecentMasterPagesSelector,MasterPageListListener));
}

void RecentMasterPagesSelector::LateInit()
{
    MasterPagesSelector::LateInit();

    MasterPagesSelector::Fill();
    RecentlyUsedMasterPages::Instance().AddEventListener (
        LINK(this,RecentMasterPagesSelector,MasterPageListListener));
}

IMPL_LINK_NOARG(RecentMasterPagesSelector, MasterPageListListener, LinkParamNone*, void)
{
    MasterPagesSelector::Fill();
}

void RecentMasterPagesSelector::Fill (ItemList& rItemList)
{
    // Create a set of names of the master pages used by the document.
    MasterPageObserver::MasterPageNameSet aCurrentNames;
    sal_uInt16 nMasterPageCount = mrDocument.GetMasterSdPageCount(PageKind::Standard);
    for (sal_uInt16 nIndex=0; nIndex<nMasterPageCount; nIndex++)
    {
        SdPage* pMasterPage = mrDocument.GetMasterSdPage (nIndex, PageKind::Standard);
        if (pMasterPage != nullptr)
            aCurrentNames.insert (pMasterPage->GetName());
    }

    // Insert the recently used master pages that are currently not used.
    RecentlyUsedMasterPages& rInstance (RecentlyUsedMasterPages::Instance());
    int nPageCount = rInstance.GetMasterPageCount();
    for (int nIndex=0; nIndex<nPageCount; nIndex++)
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
    sal_uInt16 nSelectedItemId = mxPreviewValueSet->GetSelectedItemId();

    MasterPagesSelector::AssignMasterPageToPageList(pMasterPage, rpPageList);

    // Restore the selection.
    if (mxPreviewValueSet->GetItemCount() > 0)
    {
        if (mxPreviewValueSet->GetItemCount() >= nSelectedItemId)
            mxPreviewValueSet->SelectItem(nSelectedItemId);
        else
            mxPreviewValueSet->SelectItem(mxPreviewValueSet->GetItemCount());
    }
}

} // end of namespace sd::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
