/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AllMasterPagesSelector.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 09:21:56 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "AllMasterPagesSelector.hxx"
#include "PreviewValueSet.hxx"

#include "MasterPageContainer.hxx"
#include "MasterPageDescriptor.hxx"
#include <tools/link.hxx>
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
    SetName (String(RTL_CONSTASCII_USTRINGPARAM("AllMasterPagesSelector")));
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




} } } // end of namespace ::sd::toolpanel::controls
