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
#include "PreviewValueSet.hxx"

#include "RecentlyUsedMasterPages.hxx"
#include "MasterPageContainerProviders.hxx"
#include "MasterPageObserver.hxx"
#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "app.hrc"
#include <vcl/bitmap.hxx>

namespace sd { namespace toolpanel { namespace controls {


RecentMasterPagesSelector::RecentMasterPagesSelector (
    TreeNode* pParent,
    SdDrawDocument& rDocument,
    ViewShellBase& rBase,
    const ::boost::shared_ptr<MasterPageContainer>& rpContainer)
    : MasterPagesSelector (pParent, rDocument, rBase, rpContainer)
{
    SetName (OUString("RecentMasterPagesSelector"));
}




RecentMasterPagesSelector::~RecentMasterPagesSelector (void)
{
    RecentlyUsedMasterPages::Instance().RemoveEventListener (
        LINK(this,RecentMasterPagesSelector,MasterPageListListener));
}




void RecentMasterPagesSelector::LateInit (void)
{
    MasterPagesSelector::LateInit();

    MasterPagesSelector::Fill();
    RecentlyUsedMasterPages::Instance().AddEventListener (
        LINK(this,RecentMasterPagesSelector,MasterPageListListener));
}




IMPL_LINK_NOARG(RecentMasterPagesSelector, MasterPageListListener)
{
    MasterPagesSelector::Fill();
    return 0;
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
        if (pMasterPage != NULL)
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
            String sStyleName (mpContainer->GetStyleNameForToken(aToken));
            if (sStyleName.Len()==0
                || aCurrentNames.find(sStyleName) == aCurrentNames.end())
            {
                rItemList.push_back(aToken);
            }
        }
    }
}




void RecentMasterPagesSelector::AssignMasterPageToPageList (
    SdPage* pMasterPage,
    const ::boost::shared_ptr<std::vector<SdPage*> >& rpPageList)
{
    sal_uInt16 nSelectedItemId = mpPageSet->GetSelectItemId();

    MasterPagesSelector::AssignMasterPageToPageList(pMasterPage, rpPageList);

    // Restore the selection.
    if (mpPageSet->GetItemCount() > 0)
    {
        if (mpPageSet->GetItemCount() >= nSelectedItemId)
            mpPageSet->SelectItem(nSelectedItemId);
        else
            mpPageSet->SelectItem(mpPageSet->GetItemCount());
    }
}




void RecentMasterPagesSelector::GetState (SfxItemSet& rItemSet)
{
    MasterPagesSelector::GetState (rItemSet);
    if (rItemSet.GetItemState(SID_TP_EDIT_MASTER) == SFX_ITEM_AVAILABLE)
        rItemSet.DisableItem (SID_TP_EDIT_MASTER);
}




} } } // end of namespace ::sd::toolpanel::controls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
