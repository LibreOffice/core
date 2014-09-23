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

#include "CurrentMasterPagesSelector.hxx"
#include "PreviewValueSet.hxx"
#include "ViewShellBase.hxx"
#include "DrawViewShell.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "MasterPageContainer.hxx"
#include "MasterPageDescriptor.hxx"
#include "EventMultiplexer.hxx"
#include "app.hrc"
#include "DrawDocShell.hxx"
#include "res_bmp.hrc"
#include "sdresid.hxx"
#include "helpids.h"

#include <vcl/image.hxx>
#include <svx/svdmodel.hxx>
#include <sfx2/request.hxx>

#include <set>

using namespace ::com::sun::star;

namespace sd { namespace sidebar {

MasterPagesSelector* CurrentMasterPagesSelector::Create (
    vcl::Window* pParent,
    ViewShellBase& rViewShellBase,
    const css::uno::Reference<css::ui::XSidebar>& rxSidebar)
{
    SdDrawDocument* pDocument = rViewShellBase.GetDocument();
    if (pDocument == NULL)
        return NULL;

    ::boost::shared_ptr<MasterPageContainer> pContainer (new MasterPageContainer());

    MasterPagesSelector* pSelector(
        new CurrentMasterPagesSelector (
            pParent,
            *pDocument,
            rViewShellBase,
            pContainer,
            rxSidebar));
    pSelector->LateInit();
    pSelector->SetHelpId( HID_SD_TASK_PANE_PREVIEW_CURRENT );

    return pSelector;
}

CurrentMasterPagesSelector::CurrentMasterPagesSelector (
    vcl::Window* pParent,
    SdDrawDocument& rDocument,
    ViewShellBase& rBase,
    const ::boost::shared_ptr<MasterPageContainer>& rpContainer,
    const css::uno::Reference<css::ui::XSidebar>& rxSidebar)
    : MasterPagesSelector (pParent, rDocument, rBase, rpContainer, rxSidebar)
{
    // For this master page selector only we change the default action for
    // left clicks.
    mnDefaultClickAction = SID_TP_APPLY_TO_SELECTED_SLIDES;

    Link aLink (LINK(this,CurrentMasterPagesSelector,EventMultiplexerListener));
    rBase.GetEventMultiplexer()->AddEventListener(aLink,
        sd::tools::EventMultiplexerEvent::EID_CURRENT_PAGE
        | sd::tools::EventMultiplexerEvent::EID_EDIT_MODE_NORMAL
        | sd::tools::EventMultiplexerEvent::EID_EDIT_MODE_MASTER
        | sd::tools::EventMultiplexerEvent::EID_PAGE_ORDER
        | sd::tools::EventMultiplexerEvent::EID_SHAPE_CHANGED
        | sd::tools::EventMultiplexerEvent::EID_SHAPE_INSERTED
        | sd::tools::EventMultiplexerEvent::EID_SHAPE_REMOVED);
}

CurrentMasterPagesSelector::~CurrentMasterPagesSelector (void)
{
    if (mrDocument.GetDocSh() != NULL)
    {
        EndListening(*mrDocument.GetDocSh());
    }
    else
    {
        OSL_ASSERT(mrDocument.GetDocSh() != NULL);
    }

    Link aLink (LINK(this,CurrentMasterPagesSelector,EventMultiplexerListener));
    mrBase.GetEventMultiplexer()->RemoveEventListener(aLink);
}

void CurrentMasterPagesSelector::LateInit (void)
{
    MasterPagesSelector::LateInit();
    MasterPagesSelector::Fill();
    if (mrDocument.GetDocSh() != NULL)
    {
        StartListening(*mrDocument.GetDocSh());
    }
    else
    {
        OSL_ASSERT(mrDocument.GetDocSh() != NULL);
    }
}

void CurrentMasterPagesSelector::Fill (ItemList& rItemList)
{
    sal_uInt16 nPageCount = mrDocument.GetMasterSdPageCount(PK_STANDARD);
    SdPage* pMasterPage;
    // Remember the names of the master pages that have been inserted to
    // avoid double insertion.
    ::std::set<OUString> aMasterPageNames;
    for (sal_uInt16 nIndex=0; nIndex<nPageCount; nIndex++)
    {
        pMasterPage = mrDocument.GetMasterSdPage (nIndex, PK_STANDARD);
        if (pMasterPage == NULL)
            continue;

        // Use the name of the master page to avoid duplicate entries.
        OUString sName (pMasterPage->GetName());
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
                OUString(),
                pMasterPage->GetName(),
                OUString(),
                pMasterPage->IsPrecious(),
                ::boost::shared_ptr<PageObjectProvider>(new ExistingPageProvider(pMasterPage)),
                ::boost::shared_ptr<PreviewProvider>(new PagePreviewProvider())));
            aToken = mpContainer->PutMasterPage(pDescriptor);
        }

        rItemList.push_back(aToken);
    }
}

ResId CurrentMasterPagesSelector::GetContextMenuResId (void) const
{
    return SdResId(RID_TASKPANE_CURRENT_MASTERPAGESSELECTOR_POPUP);
}

void CurrentMasterPagesSelector::UpdateSelection (void)
{
    // Iterate over all pages and for the selected ones put the name of
    // their master page into a set.
    sal_uInt16 nPageCount = mrDocument.GetSdPageCount(PK_STANDARD);
    SdPage* pPage;
    ::std::set<OUString> aNames;
    sal_uInt16 nIndex;
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
    sal_uInt16 nItemCount (PreviewValueSet::GetItemCount());
    for (nIndex=1; nIndex<=nItemCount && bLoop; nIndex++)
    {
        OUString sName (PreviewValueSet::GetItemText (nIndex));
        if (aNames.find(sName) != aNames.end())
        {
            PreviewValueSet::SelectItem (nIndex);
        }
    }
}

void CurrentMasterPagesSelector::ExecuteCommand (const sal_Int32 nCommandId)
{
    if (nCommandId == SID_DELETE_MASTER_PAGE)
    {
        // Check once again that the master page can safely be deleted,
        // i.e. is not used.
        SdPage* pMasterPage = GetSelectedMasterPage();
        if (pMasterPage != NULL
            && mrDocument.GetMasterPageUserCount(pMasterPage) == 0)
        {
            // Removing the precious flag so that the following call to
            // RemoveUnnessesaryMasterPages() will remove this master page.
            pMasterPage->SetPrecious(false);
            mrDocument.RemoveUnnecessaryMasterPages(pMasterPage, false, true);
        }
    }
    else
        MasterPagesSelector::ExecuteCommand(nCommandId);
}

void CurrentMasterPagesSelector::ProcessPopupMenu (Menu& rMenu)
{
    // Disable the SID_DELTE_MASTER slot when there is only one master page.
    if (mrDocument.GetMasterPageUserCount(GetSelectedMasterPage()) > 0)
    {
        if (rMenu.GetItemPos(SID_DELETE_MASTER_PAGE) != MENU_ITEM_NOTFOUND)
            rMenu.EnableItem(SID_DELETE_MASTER_PAGE, false);
    }

    ::boost::shared_ptr<DrawViewShell> pDrawViewShell (
        ::boost::dynamic_pointer_cast<DrawViewShell>(mrBase.GetMainViewShell()));
    if (pDrawViewShell
        && pDrawViewShell->GetEditMode() == EM_MASTERPAGE)
    {
        if (rMenu.GetItemPos(SID_TP_EDIT_MASTER) != MENU_ITEM_NOTFOUND)
            rMenu.EnableItem(SID_TP_EDIT_MASTER, false);
    }

    MasterPagesSelector::ProcessPopupMenu(rMenu);
}

IMPL_LINK(CurrentMasterPagesSelector,EventMultiplexerListener,
    sd::tools::EventMultiplexerEvent*,pEvent)
{
    if (pEvent != NULL)
    {
        switch (pEvent->meEventId)
        {
            case sd::tools::EventMultiplexerEvent::EID_CURRENT_PAGE:
            case sd::tools::EventMultiplexerEvent::EID_EDIT_MODE_NORMAL:
            case sd::tools::EventMultiplexerEvent::EID_EDIT_MODE_MASTER:
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

} } // end of namespace sd::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
