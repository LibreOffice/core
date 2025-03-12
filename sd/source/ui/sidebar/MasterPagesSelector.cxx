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
#include <vcl/vclptr.hxx>
#include <ViewShellBase.hxx>
#include <o3tl/safeint.hxx>
#include <vcl/image.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <memory>
#include <vcl/virdev.hxx>

namespace sd::sidebar {

    /** menu entry that is executed as default action when the left mouse button is
        clicked over a master page.
    */
constexpr OUStringLiteral gsDefaultClickAction = u"applyselect";

MasterPagesSelector::MasterPagesSelector(weld::Widget* pParent, SdDrawDocument& rDocument,
                                         ViewShellBase& rBase,
                                         std::shared_ptr<MasterPageContainer> pContainer,
                                         css::uno::Reference<css::ui::XSidebar> xSidebar,
                                         const OUString& rUIFileName, const OUString& rIconViewId)
    : PanelLayout(pParent, u"MasterPagePanel"_ustr, rUIFileName)
    , mpContainer(std::move(pContainer))
    , mxPreviewIconView(m_xBuilder->weld_icon_view(rIconViewId))
    , mrDocument(rDocument)
    , mrBase(rBase)
    , mxSidebar(std::move(xSidebar))
    , maIconViewId(rIconViewId)
{
    mxPreviewIconView->connect_item_activated(LINK(this, MasterPagesSelector, MasterPageSelected));
    mxPreviewIconView->connect_mouse_press(LINK(this, MasterPagesSelector, MousePressHdl));
    mxPreviewIconView->connect_query_tooltip(LINK(this, MasterPagesSelector, QueryTooltipHdl));

    Link<MasterPageContainerChangeEvent&,void> aChangeListener (LINK(this,MasterPagesSelector,ContainerChangeListener));
    mpContainer->AddChangeListener(aChangeListener);
}

MasterPagesSelector::~MasterPagesSelector()
{
    Clear();
    UpdateLocks(ItemList());

    Link<MasterPageContainerChangeEvent&,void> aChangeListener (LINK(this,MasterPagesSelector,ContainerChangeListener));
    mpContainer->RemoveChangeListener(aChangeListener);
    mpContainer.reset();
    mxPreviewIconView.reset();
}

void MasterPagesSelector::LateInit()
{
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
    return u"modules/simpress/ui/mastermenu.ui"_ustr;
}

IMPL_LINK_NOARG(MasterPagesSelector, MasterPageSelected, weld::IconView&, bool)
{
    ExecuteCommand(gsDefaultClickAction);
    return true;
}

IMPL_LINK(MasterPagesSelector, MousePressHdl, const MouseEvent&, rMEvet, bool)
{
    if (!rMEvet.IsRight())
        return false;

    const Point& pPos = rMEvet.GetPosPixel();
    for (int i = 0; i < mxPreviewIconView->n_children(); i++)
    {
        const ::tools::Rectangle aRect = mxPreviewIconView->get_rect(i);
        if (aRect.Contains(pPos))
        {
            mxPreviewIconView->select(i);
            ShowContextMenu(pPos);
            break;
        }
    }
    return false;
}

IMPL_LINK(MasterPagesSelector, QueryTooltipHdl, const weld::TreeIter&, iter, OUString)
{
    const OUString sId = mxPreviewIconView->get_id(iter);
    if (!sId.isEmpty())
        return mpContainer->GetPageNameForToken(sId.toUInt32());

    return OUString();
}

void MasterPagesSelector::ShowContextMenu(const Point& pPos)
{
    // Setup the menu.
    ::tools::Rectangle aRect(pPos, Size(1, 1));
    std::unique_ptr<weld::Builder> xBuilder(
        Application::CreateBuilder(mxPreviewIconView.get(), GetContextMenuUIFile()));
    std::unique_ptr<weld::Menu> xMenu(xBuilder->weld_menu(u"menu"_ustr));
    ProcessPopupMenu(*xMenu);
    // Show the menu.
    ExecuteCommand(xMenu->popup_at_rect(mxPreviewIconView.get(), aRect));
}

void MasterPagesSelector::ProcessPopupMenu(weld::Menu& rMenu)
{
    // Disable some entries.
    if (mpContainer->GetPreviewSize() == MasterPageContainer::SMALL)
        rMenu.set_sensitive(u"small"_ustr, false);
    else
        rMenu.set_sensitive(u"large"_ustr, false);
}

void MasterPagesSelector::ExecuteCommand(const OUString &rIdent)
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
        SfxViewFrame& rViewFrame = mrBase.GetViewFrame();
        if (xSelectedMaster.is())
        {
            SfxDispatcher* pDispatcher = rViewFrame.GetDispatcher();
            if (pDispatcher != nullptr)
            {
                pDispatcher->Execute(SID_MASTERPAGE, SfxCallMode::SYNCHRON);
                mrBase.GetDrawController()->setCurrentPage(xSelectedMaster);
            }
        }
    }
    mxPreviewIconView->unselect_all();
}

IMPL_LINK(MasterPagesSelector, ContainerChangeListener, MasterPageContainerChangeEvent&, rEvent, void)
{
    NotifyContainerChangeEvent(rEvent);
}

SdPage* MasterPagesSelector::GetSelectedMasterPage()
{
    const ::osl::MutexGuard aGuard (maMutex);

    SdPage* pMasterPage = nullptr;
    OUString sSelectedId = mxPreviewIconView->get_selected_id();

    if (!sSelectedId.isEmpty())
    {
        const MasterPageContainer::Token aToken
            = static_cast<MasterPageContainer::Token>(sSelectedId.toInt32());
        pMasterPage = mpContainer->GetPageObjectForToken(aToken, true);
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
            UpdateAllPreviews();
            break;

        case MasterPageContainerChangeEvent::EventType::PREVIEW_CHANGED:
        {
            int nIndex (GetIndexForToken(rEvent.maChildToken));
            if (nIndex >= 0)
            {
                Image aPreview(mpContainer->GetPreviewForToken(rEvent.maChildToken));
                if (aPreview.GetSizePixel().Width() > 0)
                {
                    VclPtr<VirtualDevice> aDev = GetVirtualDevice(aPreview);
                    mxPreviewIconView->set_image(nIndex, aDev);
                }
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

std::unique_ptr<MasterPagesSelector::UserData> MasterPagesSelector::GetUserData(int nIndex) const
{
    const ::osl::MutexGuard aGuard(maMutex);

    if (nIndex >= 0 && nIndex < mxPreviewIconView->n_children())
    {
        const MasterPageContainer::Token aToken
            = static_cast<MasterPageContainer::Token>(mxPreviewIconView->get_id(nIndex).toInt32());
        return std::make_unique<UserData>(std::make_pair(nIndex, aToken));
    }
    else
        return nullptr;
}

void MasterPagesSelector::SetItem (
    sal_uInt16 nIndex,
    MasterPageContainer::Token aToken)
{
    const ::osl::MutexGuard aGuard (maMutex);

    RemoveTokenToIndexEntry(nIndex, aToken);

    mxPreviewIconView->freeze();

    if (aToken != MasterPageContainer::NIL_TOKEN)
    {
        Image aPreview (mpContainer->GetPreviewForToken(aToken));
        MasterPageContainer::PreviewState eState (mpContainer->GetPreviewState(aToken));

        if (aPreview.GetSizePixel().Width() > 0)
        {
            VclPtr<VirtualDevice> aVDev = GetVirtualDevice(aPreview);
            if (!mxPreviewIconView->get_id(nIndex).isEmpty())
            {
                mxPreviewIconView->set_image(nIndex, aVDev);
                mxPreviewIconView->set_id(nIndex, OUString::number(aToken));
            }
            else
            {
                OUString sId = OUString::number(aToken);
                mxPreviewIconView->insert(nIndex, nullptr, &sId, aVDev, nullptr);
            }

            AddTokenToIndexEntry(nIndex, aToken);
        }

        if (eState == MasterPageContainer::PS_CREATABLE)
            mpContainer->RequestPreview(aToken);
    }
    else
    {
        mxPreviewIconView->remove(nIndex);
    }
    mxPreviewIconView->thaw();
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

    std::unique_ptr<UserData> pData = GetUserData(nIndex);
    if (pData != nullptr)
    {
        // Get the token that the index pointed to previously.
        MasterPageContainer::Token aOldToken (pData->second);

        if (aNewToken != aOldToken
            && nIndex == GetIndexForToken(aOldToken))
        {
            maTokenToValueSetIndex[aOldToken] = -1;
        }
    }
}

void MasterPagesSelector::InvalidatePreview (const SdPage* pPage)
{
    const ::osl::MutexGuard aGuard (maMutex);

    for (int nIndex = 0; nIndex < mxPreviewIconView->n_children(); nIndex++)
    {
        std::unique_ptr<UserData> pData = GetUserData(nIndex);
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

VclPtr<VirtualDevice> MasterPagesSelector::GetVirtualDevice(Image pImage)
{
    BitmapEx aPreviewBitmap = pImage.GetBitmapEx();
    VclPtr<VirtualDevice> pVDev = VclPtr<VirtualDevice>::Create();
    const Point aNull(0, 0);
    if (pVDev->GetDPIScaleFactor() > 1)
        aPreviewBitmap.Scale(pVDev->GetDPIScaleFactor(), pVDev->GetDPIScaleFactor());
    const Size aSize(aPreviewBitmap.GetSizePixel());
    pVDev->SetOutputSizePixel(aSize);
    pVDev->DrawBitmapEx(aNull, aPreviewBitmap);

    return pVDev;
}

void MasterPagesSelector::UpdateAllPreviews()
{
    const ::osl::MutexGuard aGuard (maMutex);

    mxPreviewIconView->freeze();
    for (int aIndex = 0; aIndex < mxPreviewIconView->n_children(); aIndex++)
    {
        const MasterPageContainer::Token aToken
            = static_cast<MasterPageContainer::Token>(mxPreviewIconView->get_id(aIndex).toInt32());

        Image aPreview(mpContainer->GetPreviewForToken(aToken));
        if (aPreview.GetSizePixel().Width() > 0)
        {
            VclPtr<VirtualDevice> pVDev = GetVirtualDevice(aPreview);
            mxPreviewIconView->set_image(aIndex, pVDev);
        }
        else if (mpContainer->GetPreviewState(aToken) == MasterPageContainer::PS_CREATABLE)
        {
            mpContainer->RequestPreview(aToken);
        }
    }
    mxPreviewIconView->thaw();
}

void MasterPagesSelector::ClearPageSet()
{
    const ::osl::MutexGuard aGuard (maMutex);
    mxPreviewIconView->clear();
}

void MasterPagesSelector::SetHelpId( const OUString& aId )
{
    const ::osl::MutexGuard aGuard (maMutex);
    mxPreviewIconView->set_help_id(aId);
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
    sal_uInt16 nIndex(0);

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

    if (mxSidebar.is())
        mxSidebar->requestLayout();
}

css::ui::LayoutSize MasterPagesSelector::GetHeightForWidth(const sal_Int32 nWidth)
{
    // there is no way to get margin of item programatically, we use value provided in ui file.
    const int nMargin = 6;
    const Size& previewSize = mpContainer->GetPreviewSizePixel();
    sal_Int32 nColumnCount = nWidth / (previewSize.Width() + (2 * nMargin));
    if (nColumnCount < 1)
        nColumnCount = 1;

    sal_Int32 nTotalItems = mxPreviewIconView->n_children();
    sal_Int32 nRowCount = (nTotalItems + nColumnCount - 1) / nColumnCount;
    if (nRowCount < 1)
        nRowCount = 1;

    sal_Int32 nPreferedHeight = nRowCount * (previewSize.getHeight() + (4 * nMargin));
    return css::ui::LayoutSize(nPreferedHeight, nPreferedHeight, nPreferedHeight);
}

} // end of namespace sd::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
