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
#include "pres.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "sdpage.hxx"
#include "glob.hxx"
#include "glob.hrc"
#include "app.hrc"
#include "res_bmp.hrc"
#include "strings.hrc"
#include "DrawViewShell.hxx"
#include "DrawController.hxx"
#include "SlideSorterViewShell.hxx"
#include "PreviewValueSet.hxx"
#include "ViewShellBase.hxx"
#include <sfx2/objface.hxx>
#include "sdresid.hxx"
#include "drawview.hxx"
#include <vcl/image.hxx>
#include <vcl/floatwin.hxx>
#include <svl/languageoptions.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/mnumgr.hxx>
#include <svl/itemset.hxx>
#include <svl/eitem.hxx>
#include <svx/dlgutil.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svxids.hrc>
#include "FrameView.hxx"
#include "stlpool.hxx"
#include "unmovss.hxx"
#include <sfx2/request.hxx>
#include <svl/itempool.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <memory>

using namespace ::com::sun::star::text;

namespace sd { namespace sidebar {

MasterPagesSelector::MasterPagesSelector (
    vcl::Window* pParent,
    SdDrawDocument& rDocument,
    ViewShellBase& rBase,
    const std::shared_ptr<MasterPageContainer>& rpContainer,
    const css::uno::Reference<css::ui::XSidebar>& rxSidebar)
    : PreviewValueSet(pParent),
      maMutex(),
      mpContainer(rpContainer),
      mrDocument(rDocument),
      mrBase(rBase),
      mnDefaultClickAction(SID_TP_APPLY_TO_ALL_SLIDES),
      maCurrentItemList(),
      maTokenToValueSetIndex(),
      maLockedMasterPages(),
      mxSidebar(rxSidebar)
{
    PreviewValueSet::SetSelectHdl (
        LINK(this, MasterPagesSelector, ClickHandler));
    PreviewValueSet::SetRightMouseClickHandler (
        LINK(this, MasterPagesSelector, RightClickHandler));
    PreviewValueSet::SetStyle(PreviewValueSet::GetStyle() | WB_NO_DIRECTSELECT);

    if ( GetDPIScaleFactor() > 1 )
        mpContainer->SetPreviewSize(MasterPageContainer::LARGE);

    PreviewValueSet::SetPreviewSize(mpContainer->GetPreviewSizePixel());
    PreviewValueSet::Show();

    SetBackground(sfx2::sidebar::Theme::GetWallpaper(sfx2::sidebar::Theme::Paint_PanelBackground));
    SetColor(sfx2::sidebar::Theme::GetColor(sfx2::sidebar::Theme::Paint_PanelBackground));

    Link<MasterPageContainerChangeEvent&,void> aChangeListener (LINK(this,MasterPagesSelector,ContainerChangeListener));
    mpContainer->AddChangeListener(aChangeListener);
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
    PreviewValueSet::dispose();
}

void MasterPagesSelector::LateInit()
{
}

sal_Int32 MasterPagesSelector::GetPreferredHeight (sal_Int32 nWidth)
{
    const ::osl::MutexGuard aGuard (maMutex);

    return PreviewValueSet::GetPreferredHeight (nWidth);
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
    ItemList::const_iterator iItem;
    for (iItem=rItemList.begin(); iItem!=rItemList.end(); ++iItem)
    {
        mpContainer->AcquireToken(*iItem);
        aNewLockList.push_back(*iItem);
    }

    // Release the previously locked master pages.
    ItemList::const_iterator iPage;
    ItemList::const_iterator iEnd (maLockedMasterPages.end());
    for (iPage=maLockedMasterPages.begin(); iPage!=iEnd; ++iPage)
        mpContainer->ReleaseToken(*iPage);

    maLockedMasterPages.swap(aNewLockList);
}

void MasterPagesSelector::Fill()
{
    ::std::unique_ptr<ItemList> pItemList (new ItemList());

    Fill(*pItemList);

    UpdateLocks(*pItemList);
    UpdateItemList(std::move(pItemList));
}

ResId MasterPagesSelector::GetContextMenuResId() const
{
    return SdResId(RID_TASKPANE_MASTERPAGESSELECTOR_POPUP);
}

IMPL_LINK_NOARG_TYPED(MasterPagesSelector, ClickHandler, ValueSet*, void)
{
    // We use the framework to assign the clicked-on master page because we
    // so use the same mechanism as the context menu does (where we do not
    // have the option to call the assignment method directly.)
    ExecuteCommand(mnDefaultClickAction);
}

IMPL_LINK_TYPED(MasterPagesSelector, RightClickHandler, const MouseEvent&, rEvent, void)
{
    // Here we only prepare the display of the context menu: the item under
    // the mouse is selected.  The actual display of the context menu is
    // done in ContextMenuCallback which is called indirectly through
    // PreviewValueSet::Command().
    PreviewValueSet::GrabFocus ();
    PreviewValueSet::ReleaseMouse();
    SfxViewFrame* pViewFrame = mrBase.GetViewFrame();
    if (pViewFrame != nullptr)
    {
        SfxDispatcher* pDispatcher = pViewFrame->GetDispatcher();
        if (pDispatcher != nullptr)
        {
            sal_uInt16 nIndex = PreviewValueSet::GetItemId (rEvent.GetPosPixel());
            if (nIndex > 0)
                PreviewValueSet::SelectItem (nIndex);
        }
    }
}

void MasterPagesSelector::Command (const CommandEvent& rEvent)
{
    switch (rEvent.GetCommand())
    {
        case CommandEventId::ContextMenu:
        {
            // Use the currently selected item and show the popup menu in its
            // center.
            const sal_uInt16 nIndex = PreviewValueSet::GetSelectItemId();
            if (nIndex > 0)
            {
                // The position of the upper left corner of the context menu is
                // taken either from the mouse position (when the command was sent
                // as reaction to a right click) or in the center of the selected
                // item (when the command was sent as reaction to Shift+F10.)
                Point aPosition (rEvent.GetMousePosPixel());
                if ( ! rEvent.IsMouseEvent())
                {
                    Rectangle aBBox (PreviewValueSet::GetItemRect(nIndex));
                    aPosition = aBBox.Center();
                }

                // Setup the menu.
                std::unique_ptr<PopupMenu> pMenu (new PopupMenu(GetContextMenuResId()));
                FloatingWindow* pMenuWindow = dynamic_cast<FloatingWindow*>(pMenu->GetWindow());
                if (pMenuWindow != nullptr)
                    pMenuWindow->SetPopupModeFlags(
                        pMenuWindow->GetPopupModeFlags() | FloatWinPopupFlags::NoMouseUpClose);
                pMenu->SetSelectHdl(LINK(this, MasterPagesSelector, OnMenuItemSelected));

                ProcessPopupMenu(*pMenu);

                // Show the menu.
                pMenu->Execute(this, Rectangle(aPosition,Size(1,1)), PopupMenuFlags::ExecuteDown);
            }
        }
        break;
        default: break;
    }
}

void MasterPagesSelector::ProcessPopupMenu (Menu& rMenu)
{
    // Disable some entries.
    if (mpContainer->GetPreviewSize() == MasterPageContainer::SMALL)
        rMenu.EnableItem(SID_TP_SHOW_SMALL_PREVIEW, false);
    else
        rMenu.EnableItem(SID_TP_SHOW_LARGE_PREVIEW, false);
}

IMPL_LINK_TYPED(MasterPagesSelector, OnMenuItemSelected, Menu*, pMenu, bool)
{
    if (pMenu == nullptr)
    {
        OSL_ENSURE(pMenu!=nullptr, "MasterPagesSelector::OnMenuItemSelected: illegal menu!");
        return false;
    }

    pMenu->Deactivate();
    ExecuteCommand(pMenu->GetCurItemId());
    return false;
}

void MasterPagesSelector::ExecuteCommand (const sal_Int32 nCommandId)
{
    switch (nCommandId)
    {
        case SID_TP_APPLY_TO_ALL_SLIDES:
            mrBase.SetBusyState (true);
            AssignMasterPageToAllSlides (GetSelectedMasterPage());
            mrBase.SetBusyState (false);
            break;

        case SID_TP_APPLY_TO_SELECTED_SLIDES:
            mrBase.SetBusyState (true);
            AssignMasterPageToSelectedSlides (GetSelectedMasterPage());
            mrBase.SetBusyState (false);
            break;

        case SID_TP_USE_FOR_NEW_PRESENTATIONS:
            SAL_WARN ( "sd",
                "Using slides as default for new presentations"
                " is not yet implemented");
            break;

        case SID_TP_SHOW_SMALL_PREVIEW:
        case SID_TP_SHOW_LARGE_PREVIEW:
        {
            mrBase.SetBusyState (true);
            mpContainer->SetPreviewSize(
                nCommandId==SID_TP_SHOW_SMALL_PREVIEW
                ? MasterPageContainer::SMALL
                : MasterPageContainer::LARGE);
            mrBase.SetBusyState (false);
            if (mxSidebar.is())
                mxSidebar->requestLayout();
            break;
        }

        case SID_TP_EDIT_MASTER:
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
                    sal_uInt16 nIndex = PreviewValueSet::GetSelectItemId();
                    pDispatcher->Execute(SID_MASTERPAGE, SfxCallMode::SYNCHRON);
                    PreviewValueSet::SelectItem (nIndex);
                    mrBase.GetDrawController().setCurrentPage(xSelectedMaster);
                }
            }
            break;
        }

        case SID_CUT:
        case SID_COPY:
        case SID_PASTE:
            // Cut, copy, and paste are not supported and thus are ignored.
            break;
    }
}

IMPL_LINK_TYPED(MasterPagesSelector, ContainerChangeListener, MasterPageContainerChangeEvent&, rEvent, void)
{
    NotifyContainerChangeEvent(rEvent);
}

SdPage* MasterPagesSelector::GetSelectedMasterPage()
{
    const ::osl::MutexGuard aGuard (maMutex);

    SdPage* pMasterPage = nullptr;
    sal_uInt16 nIndex = PreviewValueSet::GetSelectItemId();
    UserData* pData = GetUserData(nIndex);
    if (pData != nullptr)
    {
        pMasterPage = mpContainer->GetPageObjectForToken(pData->second);
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

    sal_uInt16 nPageCount = mrDocument.GetSdPageCount(PK_STANDARD);
    if (nPageCount == 0)
        return;

    // Get a list of all pages.  As a little optimization we only
    // include pages that do not already have the given master page
    // assigned.
    OUString sFullLayoutName(pMasterPage->GetLayoutName());
    ::sd::slidesorter::SharedPageSelection pPageList (
        new ::sd::slidesorter::SlideSorterViewShell::PageSelection());
    for (sal_uInt16 nPageIndex=0; nPageIndex<nPageCount; nPageIndex++)
    {
        SdPage* pPage = mrDocument.GetSdPage (nPageIndex, PK_STANDARD);
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
    const ::sd::slidesorter::SharedPageSelection& rPageList)
{
    DocumentHelper::AssignMasterPageToPageList(mrDocument, pMasterPage, rPageList);
}

void MasterPagesSelector::NotifyContainerChangeEvent (const MasterPageContainerChangeEvent& rEvent)
{
    const ::osl::MutexGuard aGuard (maMutex);

    switch (rEvent.meEventType)
    {
        case MasterPageContainerChangeEvent::SIZE_CHANGED:
            PreviewValueSet::SetPreviewSize(mpContainer->GetPreviewSizePixel());
            UpdateAllPreviews();
            break;

        case MasterPageContainerChangeEvent::PREVIEW_CHANGED:
        {
            int nIndex (GetIndexForToken(rEvent.maChildToken));
            if (nIndex >= 0)
            {
                PreviewValueSet::SetItemImage (
                    (sal_uInt16)nIndex,
                    mpContainer->GetPreviewForToken(rEvent.maChildToken));
                PreviewValueSet::Invalidate(PreviewValueSet::GetItemRect((sal_uInt16)nIndex));
            }
        }
        break;

        case MasterPageContainerChangeEvent::DATA_CHANGED:
        {
            InvalidateItem(rEvent.maChildToken);
            Fill();
        }
        break;

        case MasterPageContainerChangeEvent::CHILD_REMOVED:
        {
            int nIndex (GetIndexForToken(rEvent.maChildToken));
            SetItem(nIndex, MasterPageContainer::NIL_TOKEN);
        }

        default:
            break;
   }
}

MasterPagesSelector::UserData* MasterPagesSelector::CreateUserData (
    int nIndex,
    MasterPageContainer::Token aToken)
{
    return new UserData(nIndex,aToken);
}

MasterPagesSelector::UserData* MasterPagesSelector::GetUserData (int nIndex) const
{
    const ::osl::MutexGuard aGuard (maMutex);

    if (nIndex>0 && static_cast<unsigned int>(nIndex)<=PreviewValueSet::GetItemCount())
        return static_cast<UserData*>(PreviewValueSet::GetItemData((sal_uInt16)nIndex));
    else
        return nullptr;
}

void MasterPagesSelector::SetUserData (int nIndex, UserData* pData)
{
    const ::osl::MutexGuard aGuard (maMutex);

    if (nIndex>0 && static_cast<unsigned int>(nIndex)<=PreviewValueSet::GetItemCount())
    {
        UserData* pOldData = GetUserData(nIndex);
        if (pOldData!=nullptr && pOldData!=pData)
            delete pOldData;
        PreviewValueSet::SetItemData((sal_uInt16)nIndex, pData);
    }
}

void MasterPagesSelector::UpdateSelection()
{
}

void MasterPagesSelector::SetItem (
    sal_uInt16 nIndex,
    MasterPageContainer::Token aToken)
{
    const ::osl::MutexGuard aGuard (maMutex);

    RemoveTokenToIndexEntry(nIndex,aToken);

    if (nIndex > 0)
    {
        if (aToken != MasterPageContainer::NIL_TOKEN)
        {
            Image aPreview (mpContainer->GetPreviewForToken(aToken));
            MasterPageContainer::PreviewState eState (mpContainer->GetPreviewState(aToken));

            if (aPreview.GetSizePixel().Width()>0)
            {
                if (PreviewValueSet::GetItemPos(nIndex) != VALUESET_ITEM_NOTFOUND)
                {
                    PreviewValueSet::SetItemImage(nIndex,aPreview);
                    PreviewValueSet::SetItemText(nIndex, mpContainer->GetPageNameForToken(aToken));
                }
                else
                {
                    PreviewValueSet::InsertItem (
                        nIndex,
                        aPreview,
                        mpContainer->GetPageNameForToken(aToken),
                        nIndex);
                }
                SetUserData(nIndex, CreateUserData(nIndex,aToken));

                AddTokenToIndexEntry(nIndex,aToken);
            }

            if (eState == MasterPageContainer::PS_CREATABLE)
                mpContainer->RequestPreview(aToken);
        }
        else
        {
            PreviewValueSet::RemoveItem(nIndex);
        }
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

    for (size_t nIndex=1; nIndex<=PreviewValueSet::GetItemCount(); nIndex++)
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

    for (size_t nIndex=1; nIndex<=PreviewValueSet::GetItemCount(); nIndex++)
    {
        UserData* pData = GetUserData(nIndex);
        if (pData != nullptr)
        {
            MasterPageContainer::Token aToken (pData->second);
            PreviewValueSet::SetItemImage(
                nIndex,
                mpContainer->GetPreviewForToken(aToken));
            if (mpContainer->GetPreviewState(aToken) == MasterPageContainer::PS_CREATABLE)
                mpContainer->RequestPreview(aToken);
        }
    }
    PreviewValueSet::Rearrange(true);
}

void MasterPagesSelector::ClearPageSet()
{
    const ::osl::MutexGuard aGuard (maMutex);

    for (size_t nIndex=1; nIndex<=PreviewValueSet::GetItemCount(); nIndex++)
    {
        UserData* pData = GetUserData(nIndex);
        if (pData != nullptr)
            delete pData;
    }
    PreviewValueSet::Clear();
}

void MasterPagesSelector::SetHelpId( const OString& aId )
{
    const ::osl::MutexGuard aGuard (maMutex);

    PreviewValueSet::SetHelpId( aId );
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

    ItemList::iterator iItem;
    for (iItem=maCurrentItemList.begin(); iItem!=maCurrentItemList.end(); ++iItem)
    {
        if (*iItem == aToken)
        {
            *iItem = MasterPageContainer::NIL_TOKEN;
            break;
        }
    }
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

    PreviewValueSet::Rearrange();
    if (mxSidebar.is())
        mxSidebar->requestLayout();
}

css::ui::LayoutSize MasterPagesSelector::GetHeightForWidth (const sal_Int32 nWidth)
{
    const sal_Int32 nHeight (GetPreferredHeight(nWidth));
    return css::ui::LayoutSize(nHeight,nHeight,nHeight);
}

} } // end of namespace sd::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
