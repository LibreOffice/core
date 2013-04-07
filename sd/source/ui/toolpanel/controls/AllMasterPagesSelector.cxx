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


#include "AllMasterPagesSelector.hxx"
#include "PreviewValueSet.hxx"
#include "app.hrc"
#include "MasterPageContainer.hxx"
#include "MasterPageDescriptor.hxx"
#include <set>

namespace {

using namespace sd::toolpanel::controls;

int GetURLPriority (const SharedMasterPageDescriptor& rpDescriptor)
{
    int nPriority (0);
    switch (rpDescriptor->GetURLClassification())
    {
        case MasterPageDescriptor::URLCLASS_USER:         nPriority = 0; break;
        case MasterPageDescriptor::URLCLASS_LAYOUT:       nPriority = 1; break;
        case MasterPageDescriptor::URLCLASS_PRESENTATION: nPriority = 2; break;
        case MasterPageDescriptor::URLCLASS_OTHER:        nPriority = 3; break;
        case MasterPageDescriptor::URLCLASS_UNKNOWN:      nPriority = 4; break;
        default:
        case MasterPageDescriptor::URLCLASS_UNDETERMINED: nPriority = 5; break;
    }
    return nPriority;
}


class MasterPageDescriptorOrder
{
public:
    bool operator() (const sd::toolpanel::controls::SharedMasterPageDescriptor& rp1,
        const sd::toolpanel::controls::SharedMasterPageDescriptor& rp2)
    {
        if (rp1->meOrigin == MasterPageContainer::DEFAULT)
            return true;
        else if (rp2->meOrigin == MasterPageContainer::DEFAULT)
            return false;
        else if (rp1->GetURLClassification() == rp2->GetURLClassification())
            return rp1->mnTemplateIndex < rp2->mnTemplateIndex;
        else
            return GetURLPriority(rp1) < GetURLPriority(rp2);
    }
};

} // end of anonymous namespace



namespace sd { namespace toolpanel { namespace controls {

class AllMasterPagesSelector::SortedMasterPageDescriptorList
    : public ::std::set<SharedMasterPageDescriptor,MasterPageDescriptorOrder>
{
public:
    SortedMasterPageDescriptorList (void) {}
};




AllMasterPagesSelector::AllMasterPagesSelector (
    TreeNode* pParent,
    SdDrawDocument& rDocument,
    ViewShellBase& rBase,
    DrawViewShell& rViewShell,
    const ::boost::shared_ptr<MasterPageContainer>& rpContainer)
    : MasterPagesSelector(pParent, rDocument, rBase, rpContainer),
      mrViewShell(rViewShell),
      mpSortedMasterPages(new SortedMasterPageDescriptorList())
{
    SetName (OUString("AllMasterPagesSelector"));
    MasterPagesSelector::Fill();
}




AllMasterPagesSelector::~AllMasterPagesSelector (void)
{
}




void AllMasterPagesSelector::Fill (ItemList& rItemList)
{
    if (mpSortedMasterPages->empty())
        UpdateMasterPageList();
    UpdatePageSet(rItemList);
}




void AllMasterPagesSelector::NotifyContainerChangeEvent (
    const MasterPageContainerChangeEvent& rEvent)
{
    switch (rEvent.meEventType)
    {
        case MasterPageContainerChangeEvent::CHILD_ADDED:
            AddItem(rEvent.maChildToken);
            MasterPagesSelector::Fill();
            break;

        case MasterPageContainerChangeEvent::INDEX_CHANGED:
        case MasterPageContainerChangeEvent::INDEXES_CHANGED:
            mpSortedMasterPages->clear();
            MasterPagesSelector::Fill();
            break;

        default:
            MasterPagesSelector::NotifyContainerChangeEvent(rEvent);
            break;
    }
}




void AllMasterPagesSelector::UpdateMasterPageList (void)
{
    mpSortedMasterPages->clear();
    int nTokenCount = mpContainer->GetTokenCount();
    for (int i=0; i<nTokenCount; i++)
        AddItem(mpContainer->GetTokenForIndex(i));
}




void AllMasterPagesSelector::AddItem (MasterPageContainer::Token aToken)
{
    switch (mpContainer->GetOriginForToken(aToken))
    {
        case MasterPageContainer::DEFAULT:
        case MasterPageContainer::TEMPLATE:
            // Templates are added only when coming from the
            // MasterPageContainerFiller so that they have an id which
            // defines their place in the list.  Templates (pre) loaded from
            // RecentlyUsedMasterPages are ignored (they will be loaded
            // later by the MasterPageContainerFiller.)
            if (mpContainer->GetTemplateIndexForToken(aToken) >= 0)
                mpSortedMasterPages->insert(mpContainer->GetDescriptorForToken(aToken));
            break;

        default:
            break;
    }
}




void AllMasterPagesSelector::UpdatePageSet (ItemList& rItemList)
{
    SortedMasterPageDescriptorList::const_iterator iDescriptor;
    SortedMasterPageDescriptorList::const_iterator iEnd (mpSortedMasterPages->end());
    for (iDescriptor=mpSortedMasterPages->begin(); iDescriptor!=iEnd; ++iDescriptor)
        rItemList.push_back((*iDescriptor)->maToken);
}




void AllMasterPagesSelector::GetState (SfxItemSet& rItemSet)
{
    MasterPagesSelector::GetState(rItemSet);

    if (rItemSet.GetItemState(SID_TP_EDIT_MASTER) == SFX_ITEM_AVAILABLE)
        rItemSet.DisableItem(SID_TP_EDIT_MASTER);
}




} } } // end of namespace ::sd::toolpanel::control

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
