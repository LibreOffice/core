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

#include <sal/config.h>

#include <utility>

#include "MasterPagesSelector.hxx"

#include "MasterPageContainer.hxx"
#include "DocumentHelper.hxx"
#include <pres.hxx>
#include <drawdoc.hxx>
#include <sdpage.hxx>
#include <app.hrc>

#include <DrawController.hxx>
#include <SlideSorterViewShell.hxx>
#include "PreviewValueSet.hxx"
#include <ViewShellBase.hxx>
#include <o3tl/safeint.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/image.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/weldutils.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <memory>

using namespace ::com::sun::star::text;

namespace sd::sidebar {

    /** menu entry that is executed as default action when the left mouse button is
        clicked over a master page.
    */
constexpr OStringLiteral gsDefaultClickAction = "applyselect";

MasterPagesSelector::MasterPagesSelector (
    vcl::Window* pParent,
    SdDrawDocument& rDocument,
    ViewShellBase& rBase,
    const std::shared_ptr<MasterPageContainer>& rpContainer,
    const css::uno::Reference<css::ui::XSidebar>& rxSidebar)
    : PanelLayout( pParent, "MasterPagePanel", "modules/simpress/ui/masterpagepanel.ui", nullptr ),
      maMutex(),
      mpContainer(rpContainer),
      mxPreviewValueSet(new PreviewValueSet),
      mxPreviewValueSetWin(new weld::CustomWeld(*m_xBuilder, "valueset", *mxPreviewValueSet)),
      mrDocument(rDocument),
      mrBase(rBase),
      maCurrentItemList(),
      maTokenToValueSetIndex(),
      maLockedMasterPages(),
      mxSidebar(rxSidebar)
{
    mxPreviewValueSet->SetSelectHdl (
        LINK(this, MasterPagesSelector, ClickHandler));
    mxPreviewValueSet->SetRightMouseClickHandler (
        LINK(this, MasterPagesSelector, RightClickHandler));
    mxPreviewValueSet->SetStyle(mxPreviewValueSet->GetStyle() | WB_NO_DIRECTSELECT);

    if ( GetDPIScaleFactor() > 1 )
        mpContainer->SetPreviewSize(MasterPageContainer::LARGE);

    mxPreviewValueSet->SetPreviewSize(mpContainer->GetPreviewSizePixel());
    mxPreviewValueSet->Show();

    mxPreviewValueSet->SetColor(sfx2::sidebar::Theme::GetColor(sfx2::sidebar::Theme::Color_PanelBackground));

    Link<MasterPageContainerChangeEvent&,void> aChangeListener (LINK(this,MasterPagesSelector,ContainerChangeListener));
    mpContainer->AddChangeListener(aChangeListener);

    m_pInitialFocusWidget = mxPreviewValueSet->GetDrawingArea();
}

MasterPagesSelector::~MasterPagesSelector()
{
    disposeOnce();
}

void MasterPagesSelector::dispose()
{
    Clear();
    UpdateLocks(ItemList());

    Link<MasterPageContainerChangeEvent&,void> aChangeListener (LINK(this,MasterPagesSelector,ContainerChangeListener));
    mpContainer->RemoveChangeListener(aChangeListener);
    mpContainer.reset();
    mxPreviewValueSetWin.reset();
    mxPreviewValueSet.reset();

    PanelLayout::dispose();
}

void MasterPagesSelector::LateInit()
{
}

sal_Int32 MasterPagesSelector::GetPreferredHeight (sal_Int32 nWidth)
{
    const ::osl::MutexGuard aGuard (maMutex);

    return mxPreviewValueSet->GetPreferredHeight (nWidth);
}

void MasterPagesSelector::UpdateLocks (const ItemList& rItemList)
{
    ItemList aNewLockList;

    // In here we first lock the master pages in the given list and then
    // release the locks acquired in a previous call to this method.  When
    // this were done the other way round the lock count of some master
    // pages might drop temporarily to 0 and would lead to unnecessary
    // deletion and re-creation of MasterPageDescriptor objects.

    // Lock the master pages in the given list.
    for (const auto& rItem : rItemList)
    {
        mpContainer->AcquireToken(rItem);
        aNewLockList.push_back(rItem);
    }

    // Release the previously locked master pages.
    for (const auto& rPage : maLockedMasterPages)
        mpContainer->ReleaseToken(rPage);

    maLockedMasterPages.swap(aNewLockList);
}

void MasterPagesSelector::Fill()
{
    ::std::unique_ptr<ItemList> pItemList (new ItemList);

    Fill(*pItemList);

    UpdateLocks(*pItemList);
    UpdateItemList(std::move(pItemList));
}

OUString MasterPagesSelector::GetContextMenuUIFile() const
{
    return "modules/simpress/ui/mastermenu.ui";
}

IMPL_LINK_NOARG(MasterPagesSelector, ClickHandler, ValueSet*, void)
{
    // We use the framework to assign the clicked-on master page because we
    // so use the same mechanism as the context menu does (where we do not
    // have the option to call the assignment method directly.)
    ExecuteCommand(gsDefaultClickAction);
}

IMPL_LINK(MasterPagesSelector, RightClickHandler, const MouseEvent&, rEvent, void)
{
    // Here we only prepare the display of the context menu: the item under
    // the mouse is selected.
    mxPreviewValueSet->GrabFocus ();
    mxPreviewValueSet->ReleaseMouse();
    SfxViewFrame* pViewFrame = mrBase.GetViewFrame();
    if (pViewFrame == nullptr)
        return;

    SfxDispatcher* pDispatcher = pViewFrame->GetDispatcher();
    if (pDispatcher != nullptr)
    {
        sal_uInt16 nIndex = mxPreviewValueSet->GetItemId (rEvent.GetPosPixel());
        if (nIndex > 0)
        {
            mxPreviewValueSet->SelectItem (nIndex);
            // Now do the actual display of the context menu
            ShowContextMenu(&rEvent.GetPosPixel());
        }
    }
}

void MasterPagesSelector::ShowContextMenu(const Point* pPos)
{
    // Use the currently selected item and show the popup menu in its
    // center.
    const sal_uInt16 nIndex = mxPreviewValueSet->GetSelectedItemId();
    if (nIndex <= 0)
        return;

    // The position of the upper left corner of the context menu is
    // taken either from the mouse position (when the command was sent
    // as reaction to a right click) or in the center of the selected
    // item (when the command was sent as reaction to Shift+F10.)
    Point aPosition;
    if (!pPos)
    {
        ::tools::Rectangle aBBox (mxPreviewValueSet->GetItemRect(nIndex));
        aPosition = aBBox.Center();
    }
    else
        aPosition = *pPos;

    // Setup the menu.
    std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(nullptr, GetContextMenuUIFile()));
    std::unique_ptr<weld::Menu> xMenu(xBuilder->weld_menu("menu"));
    ProcessPopupMenu(*xMenu);
    ::tools::Rectangle aRect(aPosition, Size(1,1));
    weld::Window* pParent = weld::GetPopupParent(*this, aRect);
    // Show the menu.
    ExecuteCommand(xMenu->popup_at_rect(pParent, aRect));
}

void MasterPagesSelector::Command (const CommandEvent& rEvent)
{
    if (rEvent.GetCommand() == CommandEventId::ContextMenu)
        ShowContextMenu(rEvent.IsMouseEvent() ? &rEvent.GetMousePosPixel() : nullptr);
}

void MasterPagesSelector::ProcessPopupMenu(weld::Menu& rMenu)
{
    // Disable some entries.
    if (mpContainer->GetPreviewSize() == MasterPageContainer::SMALL)
        rMenu.set_sensitive("small", false);
    else
        rMenu.set_sensitive("large", false);
}

void MasterPagesSelector::ExecuteCommand(const OString &rIdent)
{
    if (rIdent == "applyall")
    {
        mrBase.SetBusyState (true);
        AssignMasterPageToAllSlides (GetSelectedMasterPage());
        mrBase.SetBusyState (false);
    }
    else if (rIdent == "applyselect")
    {
        mrBase.SetBusyState (true);
        AssignMasterPageToSelectedSlides (GetSelectedMasterPage());
        mrBase.SetBusyState (false);
    }
    else if (rIdent == "large")
    {
        mrBase.SetBusyState (true);
        mpContainer->SetPreviewSize(MasterPageContainer::LARGE);
        mrBase.SetBusyState (false);
        if (mxSidebar.is())
            mxSidebar->requestLayout();
    }
    else if (rIdent == "small")
    {
        mrBase.SetBusyState (true);
        mpContainer->SetPreviewSize(MasterPageContainer::SMALL);
        mrBase.SetBusyState (false);
        if (mxSidebar.is())
            mxSidebar->requestLayout();
    }
    else if (rIdent == "edit")
    {
        using namespace ::com::sun::star;
        uno::Reference<drawing::XDrawPage> xSelectedMaster;
        SdPage* pMasterPage = GetSelectedMasterPage();
        assert(pMasterPage); //rhbz#902884
        if (pMasterPage)
            xSelectedMaster.set(pMasterPage->getUnoPage(), uno::UNO_QUERY);
        SfxViewFrame* pViewFrame = mrBase.GetViewFrame();
        if (pViewFrame != nullptr && xSelectedMaster.is())
        {
            SfxDispatcher* pDispatcher = pViewFrame->GetDispatcher();
            if (pDispatcher != nullptr)
            {
                sal_uInt16 nIndex = mxPreviewValueSet->GetSelectedItemId();
                pDispatcher->Execute(SID_MASTERPAGE, SfxCallMode::SYNCHRON);
                mxPreviewValueSet->SelectItem (nIndex);
                mrBase.GetDrawController().setCurrentPage(xSelectedMaster);
            }
        }
    }
}

IMPL_LINK(MasterPagesSelector, ContainerChangeListener, MasterPageContainerChangeEvent&, rEvent, void)
{
    NotifyContainerChangeEvent(rEvent);
}

SdPage* MasterPagesSelector::GetSelectedMasterPage()
{
    const ::osl::MutexGuard aGuard (maMutex);

    SdPage* pMasterPage = nullptr;
    sal_uInt16 nIndex = mxPreviewValueSet->GetSelectedItemId();
    UserData* pData = GetUserData(nIndex);
    if (pData != nullptr)
    {
        pMasterPage = mpContainer->GetPageObjectForToken(pData->second, true);
    }
    return pMasterPage;
}

/** Assemble a list of all slides of the document and pass it to
    AssignMasterPageToPageList().
*/
void MasterPagesSelector::AssignMasterPageToAllSlides (SdPage* pMasterPage)
{
    if (pMasterPage == nullptr)
        return;

    sal_uInt16 nPageCount = mrDocument.GetSdPageCount(PageKind::Standard);
    if (nPageCount == 0)
        return;

    // Get a list of all pages.  As a little optimization we only
    // include pages that do not already have the given master page
    // assigned.
    OUString sFullLayoutName(pMasterPage->GetLayoutName());
    ::sd::slidesorter::SharedPageSelection pPageList =
        std::make_shared<::sd::slidesorter::SlideSorterViewShell::PageSelection>();
    for (sal_uInt16 nPageIndex=0; nPageIndex<nPageCount; nPageIndex++)
    {
        SdPage* pPage = mrDocument.GetSdPage (nPageIndex, PageKind::Standard);
        if (pPage != nullptr && pPage->GetLayoutName() != sFullLayoutName)
        {
            pPageList->push_back (pPage);
        }
    }

    AssignMasterPageToPageList(pMasterPage, pPageList);
}

/** Assemble a list of the currently selected slides (selected in a visible
    slide sorter) and pass it to AssignMasterPageToPageList().
*/
void MasterPagesSelector::AssignMasterPageToSelectedSlides (
    SdPage* pMasterPage)
{
    using namespace ::sd::slidesorter;
    using namespace ::sd::slidesorter::controller;

    if (pMasterPage == nullptr)
        return;

    // Find a visible slide sorter.
    SlideSorterViewShell* pSlideSorter = SlideSorterViewShell::GetSlideSorter(mrBase);
    if (pSlideSorter == nullptr)
        return;

    // Get a list of selected pages.
    SharedPageSelection pPageSelection = pSlideSorter->GetPageSelection();
    if (pPageSelection->empty())
        return;

    AssignMasterPageToPageList(pMasterPage, pPageSelection);

    // Restore the previous selection.
    pSlideSorter->SetPageSelection(pPageSelection);
}

void MasterPagesSelector::AssignMasterPageToPageList (
    SdPage* pMasterPage,
    const std::shared_ptr<std::vector<SdPage*>>& rPageList)
{
    DocumentHelper::AssignMasterPageToPageList(mrDocument, pMasterPage, rPageList);
}

void MasterPagesSelector::NotifyContainerChangeEvent (const MasterPageContainerChangeEvent& rEvent)
{
    const ::osl::MutexGuard aGuard (maMutex);

    switch (rEvent.meEventType)
    {
        case MasterPageContainerChangeEvent::EventType::SIZE_CHANGED:
            mxPreviewValueSet->SetPreviewSize(mpContainer->GetPreviewSizePixel());
            UpdateAllPreviews();
            break;

        case MasterPageContainerChangeEvent::EventType::PREVIEW_CHANGED:
        {
            int nIndex (GetIndexForToken(rEvent.maChildToken));
            if (nIndex >= 0)
            {
                mxPreviewValueSet->SetItemImage (
                    static_cast<sal_uInt16>(nIndex),
                    mpContainer->GetPreviewForToken(rEvent.maChildToken));
                mxPreviewValueSet->Invalidate(mxPreviewValueSet->GetItemRect(static_cast<sal_uInt16>(nIndex)));
            }
        }
        break;

        case MasterPageContainerChangeEvent::EventType::DATA_CHANGED:
        {
            InvalidateItem(rEvent.maChildToken);
            Fill();
        }
        break;

        case MasterPageContainerChangeEvent::EventType::CHILD_REMOVED:
        {
            int nIndex (GetIndexForToken(rEvent.maChildToken));
            SetItem(nIndex, MasterPageContainer::NIL_TOKEN);
            break;
        }

        default:
            break;
   }
}

MasterPagesSelector::UserData* MasterPagesSelector::GetUserData (int nIndex) const
{
    const ::osl::MutexGuard aGuard (maMutex);

    if (nIndex>0 && o3tl::make_unsigned(nIndex)<=mxPreviewValueSet->GetItemCount())
        return static_cast<UserData*>(mxPreviewValueSet->GetItemData(static_cast<sal_uInt16>(nIndex)));
    else
        return nullptr;
}

void MasterPagesSelector::SetUserData (int nIndex, std::unique_ptr<UserData> pData)
{
    const ::osl::MutexGuard aGuard (maMutex);

    delete GetUserData(nIndex);
    mxPreviewValueSet->SetItemData(static_cast<sal_uInt16>(nIndex), pData.release());
}

void MasterPagesSelector::SetItem (
    sal_uInt16 nIndex,
    MasterPageContainer::Token aToken)
{
    const ::osl::MutexGuard aGuard (maMutex);

    RemoveTokenToIndexEntry(nIndex,aToken);

    if (nIndex <= 0)
        return;

    if (aToken != MasterPageContainer::NIL_TOKEN)
    {
        Image aPreview (mpContainer->GetPreviewForToken(aToken));
        MasterPageContainer::PreviewState eState (mpContainer->GetPreviewState(aToken));

        if (aPreview.GetSizePixel().Width()>0)
        {
            if (mxPreviewValueSet->GetItemPos(nIndex) != VALUESET_ITEM_NOTFOUND)
            {
                mxPreviewValueSet->SetItemImage(nIndex,aPreview);
                mxPreviewValueSet->SetItemText(nIndex, mpContainer->GetPageNameForToken(aToken));
            }
            else
            {
                mxPreviewValueSet->InsertItem (
                    nIndex,
                    aPreview,
                    mpContainer->GetPageNameForToken(aToken),
                    nIndex);
            }
            SetUserData(nIndex, std::make_unique<UserData>(nIndex,aToken));

            AddTokenToIndexEntry(nIndex,aToken);
        }

        if (eState == MasterPageContainer::PS_CREATABLE)
            mpContainer->RequestPreview(aToken);
    }
    else
    {
        mxPreviewValueSet->RemoveItem(nIndex);
    }

}

void MasterPagesSelector::AddTokenToIndexEntry (
    sal_uInt16 nIndex,
    MasterPageContainer::Token aToken)
{
    const ::osl::MutexGuard aGuard (maMutex);

    maTokenToValueSetIndex[aToken] = nIndex;
}

void MasterPagesSelector::RemoveTokenToIndexEntry (
    sal_uInt16 nIndex,
    MasterPageContainer::Token aNewToken)
{
    const ::osl::MutexGuard aGuard (maMutex);

    UserData* pData = GetUserData(nIndex);
    if (pData != nullptr)
    {
        // Get the token that the index pointed to previously.
        MasterPageContainer::Token aOldToken (pData->second);

        if (aNewToken != aOldToken
            && nIndex == GetIndexForToken(aOldToken))
        {
            maTokenToValueSetIndex[aOldToken] = 0;
        }
    }
}

void MasterPagesSelector::InvalidatePreview (const SdPage* pPage)
{
    const ::osl::MutexGuard aGuard (maMutex);

    for (size_t nIndex=1; nIndex<=mxPreviewValueSet->GetItemCount(); nIndex++)
    {
        UserData* pData = GetUserData(nIndex);
        if (pData != nullptr)
        {
            MasterPageContainer::Token aToken (pData->second);
            if (pPage == mpContainer->GetPageObjectForToken(aToken,false))
            {
                mpContainer->InvalidatePreview(aToken);
                mpContainer->RequestPreview(aToken);
                break;
            }
        }
    }
}

void MasterPagesSelector::UpdateAllPreviews()
{
    const ::osl::MutexGuard aGuard (maMutex);

    for (size_t nIndex=1; nIndex<=mxPreviewValueSet->GetItemCount(); nIndex++)
    {
        UserData* pData = GetUserData(nIndex);
        if (pData != nullptr)
        {
            MasterPageContainer::Token aToken (pData->second);
            mxPreviewValueSet->SetItemImage(
                nIndex,
                mpContainer->GetPreviewForToken(aToken));
            if (mpContainer->GetPreviewState(aToken) == MasterPageContainer::PS_CREATABLE)
                mpContainer->RequestPreview(aToken);
        }
    }
    mxPreviewValueSet->Rearrange();
}

void MasterPagesSelector::ClearPageSet()
{
    const ::osl::MutexGuard aGuard (maMutex);

    for (size_t nIndex=1; nIndex<=mxPreviewValueSet->GetItemCount(); nIndex++)
    {
        UserData* pData = GetUserData(nIndex);
        delete pData;
    }
    mxPreviewValueSet->Clear();
}

void MasterPagesSelector::SetHelpId( const OString& aId )
{
    const ::osl::MutexGuard aGuard (maMutex);

    mxPreviewValueSet->SetHelpId( aId );
}

sal_Int32 MasterPagesSelector::GetIndexForToken (MasterPageContainer::Token aToken) const
{
    const ::osl::MutexGuard aGuard (maMutex);

    TokenToValueSetIndex::const_iterator iIndex (maTokenToValueSetIndex.find(aToken));
    if (iIndex != maTokenToValueSetIndex.end())
        return iIndex->second;
    else
        return -1;
}

void MasterPagesSelector::Clear()
{
    const ::osl::MutexGuard aGuard (maMutex);

    ClearPageSet();
}

void MasterPagesSelector::InvalidateItem (MasterPageContainer::Token aToken)
{
    const ::osl::MutexGuard aGuard (maMutex);

    auto iItem = std::find(maCurrentItemList.begin(), maCurrentItemList.end(), aToken);
    if (iItem != maCurrentItemList.end())
        *iItem = MasterPageContainer::NIL_TOKEN;
}

void MasterPagesSelector::UpdateItemList (::std::unique_ptr<ItemList> && pNewItemList)
{
    const ::osl::MutexGuard aGuard (maMutex);

    ItemList::const_iterator iNewItem (pNewItemList->begin());
    ItemList::const_iterator iCurrentItem (maCurrentItemList.begin());
    ItemList::const_iterator iNewEnd (pNewItemList->end());
    ItemList::const_iterator iCurrentEnd (maCurrentItemList.end());
    sal_uInt16 nIndex (1);

    // Update existing items.
    for ( ; iNewItem!=iNewEnd && iCurrentItem!=iCurrentEnd; ++iNewItem, ++iCurrentItem,++nIndex)
    {
        if (*iNewItem != *iCurrentItem)
        {
            SetItem(nIndex,*iNewItem);
        }
    }

    // Append new items.
    for ( ; iNewItem!=iNewEnd; ++iNewItem,++nIndex)
    {
        SetItem(nIndex,*iNewItem);
    }

    // Remove trailing items.
    for ( ; iCurrentItem!=iCurrentEnd; ++iCurrentItem,++nIndex)
    {
        SetItem(nIndex,MasterPageContainer::NIL_TOKEN);
    }

    maCurrentItemList.swap(*pNewItemList);

    mxPreviewValueSet->Rearrange();
    if (mxSidebar.is())
        mxSidebar->requestLayout();
}

css::ui::LayoutSize MasterPagesSelector::GetHeightForWidth (const sal_Int32 nWidth)
{
    const sal_Int32 nHeight (GetPreferredHeight(nWidth));
    return css::ui::LayoutSize(nHeight,nHeight,nHeight);
}

} // end of namespace sd::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
