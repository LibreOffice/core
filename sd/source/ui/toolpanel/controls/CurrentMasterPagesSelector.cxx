/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CurrentMasterPagesSelector.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:48:04 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "CurrentMasterPagesSelector.hxx"
#include "PreviewValueSet.hxx"
#include "ViewShellBase.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "MasterPageContainer.hxx"
#include "MasterPageDescriptor.hxx"
#include "EventMultiplexer.hxx"
#include "app.hrc"

#include <vcl/image.hxx>
#include <svx/svdmodel.hxx>

#include <set>


using namespace ::com::sun::star;

namespace sd { namespace toolpanel { namespace controls {


CurrentMasterPagesSelector::CurrentMasterPagesSelector (
    TreeNode* pParent,
    SdDrawDocument& rDocument,
    ViewShellBase& rBase,
    const ::boost::shared_ptr<MasterPageContainer>& rpContainer)
    : MasterPagesSelector (pParent, rDocument, rBase, rpContainer)
{
    SetName(String(RTL_CONSTASCII_USTRINGPARAM("CurrentMasterPagesSelector")));

    // For this master page selector only we change the default action for
    // left clicks.
    mnDefaultClickAction = SID_TP_APPLY_TO_SELECTED_SLIDES;

    Link aLink (LINK(this,CurrentMasterPagesSelector,EventMultiplexerListener));
    rBase.GetEventMultiplexer()->AddEventListener(aLink,
        sd::tools::EventMultiplexerEvent::EID_CURRENT_PAGE
        | sd::tools::EventMultiplexerEvent::EID_EDIT_MODE
        | sd::tools::EventMultiplexerEvent::EID_PAGE_ORDER
        | sd::tools::EventMultiplexerEvent::EID_SHAPE_CHANGED
        | sd::tools::EventMultiplexerEvent::EID_SHAPE_INSERTED
        | sd::tools::EventMultiplexerEvent::EID_SHAPE_REMOVED);


}




CurrentMasterPagesSelector::~CurrentMasterPagesSelector (void)
{
    Link aLink (LINK(this,CurrentMasterPagesSelector,EventMultiplexerListener));
    mrBase.GetEventMultiplexer()->RemoveEventListener(aLink);
}




void CurrentMasterPagesSelector::LateInit (void)
{
    MasterPagesSelector::LateInit();
    MasterPagesSelector::Fill();
}




void CurrentMasterPagesSelector::Fill (ItemList& rItemList)
{
    USHORT nPageCount = mrDocument.GetMasterSdPageCount(PK_STANDARD);
    SdPage* pMasterPage;
    // Remember the names of the master pages that have been inserted to
    // avoid double insertion.
    ::std::set<String> aMasterPageNames;
    for (USHORT nIndex=0; nIndex<nPageCount; nIndex++)
    {
        pMasterPage = mrDocument.GetMasterSdPage (nIndex, PK_STANDARD);
        if (pMasterPage == NULL)
            continue;

        // Use the name of the master page to avoid duplicate entries.
        String sName (pMasterPage->GetName());
        if (aMasterPageNames.find(sName)!=aMasterPageNames.end())
            continue;
        aMasterPageNames.insert (sName);

        // Look up the master page in the container and, when it is not yet
        // in it, insert it.
        MasterPageContainer::Token aToken = mpContainer->GetTokenForPageObject(pMasterPage);
        if (aToken == MasterPageContainer::NIL_TOKEN)
        {
            SharedMasterPageDescriptor pDescriptor (new MasterPageDescriptor(
                MasterPageContainer::MASTERPAGE,
                nIndex,
                String(),
                pMasterPage->GetName(),
                String(),
                ::boost::shared_ptr<PageObjectProvider>(new ExistingPageProvider(pMasterPage)),
                ::boost::shared_ptr<PreviewProvider>(new PagePreviewProvider())));
            aToken = mpContainer->PutMasterPage(pDescriptor);
        }

        rItemList.push_back(aToken);
    }
}




SdPage* CurrentMasterPagesSelector::GetSelectedMasterPage (void)
{
    USHORT nIndex = mpPageSet->GetSelectItemId();
    UserData* pData = GetUserData(nIndex);
    if (pData != NULL)
        return mpContainer->GetPageObjectForToken(pData->second);
    else
        return NULL;
}




void CurrentMasterPagesSelector::UpdateSelection (void)
{
    // Iterate over all pages and for the selected ones put the name of
    // their master page into a set.
    USHORT nPageCount = mrDocument.GetSdPageCount(PK_STANDARD);
    SdPage* pPage;
    ::std::set<String> aNames;
    USHORT nIndex;
    bool bLoop (true);
    for (nIndex=0; nIndex<nPageCount && bLoop; nIndex++)
    {
        pPage = mrDocument.GetSdPage (nIndex, PK_STANDARD);
        if (pPage != NULL && pPage->IsSelected())
        {
            if ( ! pPage->TRG_HasMasterPage())
            {
                // One of the pages has no master page.  This is an
                // indicator for that this method is called in the middle of
                // a document change and that the model is not in a valid
                // state.  Therefore we stop update the selection and wait
                // for another call to UpdateSelection when the model is
                // valid again.
                bLoop = false;
            }
            else
            {
                SdrPage& rMasterPage (pPage->TRG_GetMasterPage());
                SdPage* pMasterPage = static_cast<SdPage*>(&rMasterPage);
                if (pMasterPage != NULL)
                    aNames.insert (pMasterPage->GetName());
            }
        }
    }

    // Find the items for the master pages in the set.
    USHORT nItemCount (mpPageSet->GetItemCount());
    for (nIndex=1; nIndex<=nItemCount && bLoop; nIndex++)
    {
        String sName (mpPageSet->GetItemText (nIndex));
        if (aNames.find(sName) != aNames.end())
        {
            mpPageSet->SelectItem (nIndex);
        }
    }
}




IMPL_LINK(CurrentMasterPagesSelector,EventMultiplexerListener,
    sd::tools::EventMultiplexerEvent*,pEvent)
{
    if (pEvent != NULL)
    {
        switch (pEvent->meEventId)
        {
            case sd::tools::EventMultiplexerEvent::EID_CURRENT_PAGE:
            case sd::tools::EventMultiplexerEvent::EID_EDIT_MODE:
            case sd::tools::EventMultiplexerEvent::EID_SLIDE_SORTER_SELECTION:
                UpdateSelection();
                break;

            case sd::tools::EventMultiplexerEvent::EID_PAGE_ORDER:
                // This is tricky.  If a master page is removed, moved, or
                // added we have to wait until both the notes master page
                // and the standard master page have been removed, moved,
                // or added.  We do this by looking at the number of master
                // pages which has to be odd in the consistent state (the
                // handout master page is always present).  If the number is
                // even we ignore the hint.
                if (mrBase.GetDocument()->GetMasterPageCount()%2 == 1)
                    MasterPagesSelector::Fill();
                break;

            case sd::tools::EventMultiplexerEvent::EID_SHAPE_CHANGED:
            case sd::tools::EventMultiplexerEvent::EID_SHAPE_INSERTED:
            case sd::tools::EventMultiplexerEvent::EID_SHAPE_REMOVED:
                InvalidatePreview((const SdPage*)pEvent->mpUserData);
                break;
        }
    }

    return 0;
}


} } } // end of namespace ::sd::toolpanel::controls
