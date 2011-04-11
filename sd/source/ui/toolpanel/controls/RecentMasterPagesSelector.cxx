/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_sd.hxx"

#include "RecentMasterPagesSelector.hxx"
#include "PreviewValueSet.hxx"

#include "RecentlyUsedMasterPages.hxx"
#include "MasterPageContainerProviders.hxx"
#include "MasterPageObserver.hxx"
#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "app.hrc"
#include <vcl/bitmap.hxx>
#include <tools/color.hxx>

namespace sd { namespace toolpanel { namespace controls {


RecentMasterPagesSelector::RecentMasterPagesSelector (
    TreeNode* pParent,
    SdDrawDocument& rDocument,
    ViewShellBase& rBase,
    const ::boost::shared_ptr<MasterPageContainer>& rpContainer)
    : MasterPagesSelector (pParent, rDocument, rBase, rpContainer)
{
    SetName (String(RTL_CONSTASCII_USTRINGPARAM("RecentMasterPagesSelector")));
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




IMPL_LINK(RecentMasterPagesSelector,MasterPageListListener, void*, EMPTYARG)
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
    MasterPageObserver::MasterPageNameSet::iterator aI;

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
