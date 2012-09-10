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
#include "../TaskPaneShellManager.hxx"
#include "taskpane/TitledControl.hxx"
#include "taskpane/ControlContainer.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsPageSelector.hxx"
#include <sfx2/objface.hxx>
#include "sdresid.hxx"
#include "TemplateScanner.hxx"
#include "drawview.hxx"
#include <vcl/image.hxx>
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

using namespace ::sd::toolpanel::controls;
#define MasterPagesSelector
#include "sdslots.hxx"

using namespace ::com::sun::star::text;



namespace sd { namespace toolpanel { namespace controls {


SFX_IMPL_INTERFACE(MasterPagesSelector, SfxShell,
    SdResId(STR_MASTERPAGESSELECTOR))
{
    SFX_POPUPMENU_REGISTRATION( SdResId(RID_TASKPANE_MASTERPAGESSELECTOR_POPUP) );
}

TYPEINIT1(MasterPagesSelector, SfxShell);



MasterPagesSelector::MasterPagesSelector (
    TreeNode* pParent,
    SdDrawDocument& rDocument,
    ViewShellBase& rBase,
    const ::boost::shared_ptr<MasterPageContainer>& rpContainer)
    : TreeNode (pParent),
      SfxShell(),
      maMutex(),
      mpContainer(rpContainer),
      mrDocument(rDocument),
      mpPageSet (new PreviewValueSet(pParent)),
      mrBase(rBase),
      mnDefaultClickAction(SID_TP_APPLY_TO_ALL_SLIDES),
      maPreviewUpdateQueue(),
      maCurrentItemList(),
      maTokenToValueSetIndex(),
      maLockedMasterPages()
{
    SetPool (&rDocument.GetPool());

    mpPageSet->SetSelectHdl (
        LINK(this, MasterPagesSelector, ClickHandler));
    mpPageSet->SetRightMouseClickHandler (
        LINK(this, MasterPagesSelector, RightClickHandler));
    mpPageSet->SetContextMenuCallback (
        LINK(this, MasterPagesSelector, ContextMenuCallback));
    mpPageSet->SetStyle(mpPageSet->GetStyle() | WB_NO_DIRECTSELECT);
    mpPageSet->SetPreviewSize(mpContainer->GetPreviewSizePixel());
    mpPageSet->Show();

    Link aChangeListener (LINK(this,MasterPagesSelector,ContainerChangeListener));
    mpContainer->AddChangeListener(aChangeListener);
}




MasterPagesSelector::~MasterPagesSelector (void)
{
    Clear();
    mpPageSet.reset();
    UpdateLocks(ItemList());

    if (GetShellManager() != NULL)
        GetShellManager()->RemoveSubShell (this);

    Link aChangeListener (LINK(this,MasterPagesSelector,ContainerChangeListener));
    mpContainer->RemoveChangeListener(aChangeListener);
}




void MasterPagesSelector::LateInit (void)
{
}




sal_Int32 MasterPagesSelector::GetPreferredWidth (sal_Int32 nHeight)
{
    const ::osl::MutexGuard aGuard (maMutex);

    return mpPageSet->GetPreferredWidth (nHeight);
}




sal_Int32 MasterPagesSelector::GetPreferredHeight (sal_Int32 nWidth)
{
    const ::osl::MutexGuard aGuard (maMutex);

    return mpPageSet->GetPreferredHeight (nWidth);
}




Size MasterPagesSelector::GetPreferredSize (void)
{
    int nPreferredWidth = GetPreferredWidth(
        mpPageSet->GetOutputSizePixel().Height());
    int nPreferredHeight = GetPreferredHeight(nPreferredWidth);
    return Size (nPreferredWidth, nPreferredHeight);

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




void MasterPagesSelector::Fill (void)
{
    ::std::auto_ptr<ItemList> pItemList (new ItemList());

    Fill(*pItemList);

    UpdateLocks(*pItemList);
    UpdateItemList(pItemList);
}




ResId MasterPagesSelector::GetContextMenuResId (void) const
{
    return SdResId(RID_TASKPANE_MASTERPAGESSELECTOR_POPUP);
}




IMPL_LINK_NOARG(MasterPagesSelector, ClickHandler)
{
    // We use the framework to assign the clicked-on master page because we
    // so use the same mechanism as the context menu does (where we do not
    // have the option to call the assignment method directly.)
    if (GetShellManager() != NULL)
        GetShellManager()->MoveToTop (this);

    SfxViewFrame* pViewFrame = mrBase.GetViewFrame();
    if (pViewFrame != NULL)
    {
        SfxDispatcher* pDispatcher = pViewFrame->GetDispatcher();
        if (pDispatcher != NULL)
            pDispatcher->Execute(mnDefaultClickAction);
    }

    return 0;
}




IMPL_LINK(MasterPagesSelector, RightClickHandler, MouseEvent*, pEvent)
{
    // Here we only prepare the display of the context menu: the item under
    // the mouse is selected.  The actual display of the context menu is
    // done in ContextMenuCallback which is called indirectly through
    // PreviewValueSet::Command().
    mpPageSet->GrabFocus ();
    mpPageSet->ReleaseMouse();
    if (GetDispatcher() != NULL &&  pEvent != NULL)
    {
        sal_uInt16 nIndex = mpPageSet->GetItemId (pEvent->GetPosPixel());
        if (nIndex > 0)
            mpPageSet->SelectItem (nIndex);
    }
    return 0;
}




IMPL_LINK(MasterPagesSelector, ContextMenuCallback, CommandEvent*, pEvent)
{
    // Use the currently selected item and show the popup menu in its
    // center.
    if (GetShellManager() != NULL)
        GetShellManager()->MoveToTop (this);
    const sal_uInt16 nIndex = mpPageSet->GetSelectItemId();
    if (nIndex > 0 && pEvent!=NULL)
    {
        // The position of the upper left corner of the context menu is
        // taken either from the mouse position (when the command was sent
        // as reaction to a right click) or in the center of the selected
        // item (when the command was sent as reaction to Shift+F10.)
        Point aPosition (pEvent->GetMousePosPixel());
        if ( ! pEvent->IsMouseEvent())
        {
            Rectangle aBBox (mpPageSet->GetItemRect(nIndex));
            aPosition = aBBox.Center();
        }

        const ResId aPopupResId (GetContextMenuResId());
        mrBase.GetViewFrame()->GetDispatcher()->ExecutePopup(
            aPopupResId,
            mpPageSet.get(),
            &aPosition);
    }

    return 0;
}




IMPL_LINK(MasterPagesSelector, ContainerChangeListener, MasterPageContainerChangeEvent*, pEvent)
{
    if (pEvent)
        NotifyContainerChangeEvent(*pEvent);
    return 0;
}




SdPage* MasterPagesSelector::GetSelectedMasterPage (void)
{
    const ::osl::MutexGuard aGuard (maMutex);

    SdPage* pMasterPage = NULL;
    sal_uInt16 nIndex = mpPageSet->GetSelectItemId();
    UserData* pData = GetUserData(nIndex);
    if (pData != NULL)
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
    if (pMasterPage == NULL)
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
        if (pPage != NULL && pPage->GetLayoutName() != sFullLayoutName)
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

    if (pMasterPage == NULL)
        return;

    // Find a visible slide sorter.
    SlideSorterViewShell* pSlideSorter = SlideSorterViewShell::GetSlideSorter(mrBase);
    if (pSlideSorter == NULL)
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
            mpPageSet->SetPreviewSize(mpContainer->GetPreviewSizePixel());
            UpdateAllPreviews();
            break;

        case MasterPageContainerChangeEvent::PREVIEW_CHANGED:
        {
            int nIndex (GetIndexForToken(rEvent.maChildToken));
            if (nIndex >= 0)
            {
                mpPageSet->SetItemImage (
                    (sal_uInt16)nIndex,
                    mpContainer->GetPreviewForToken(rEvent.maChildToken));
                mpPageSet->Invalidate(mpPageSet->GetItemRect((sal_uInt16)nIndex));
            }
        }
        break;

        case MasterPageContainerChangeEvent::DATA_CHANGED:
        {
            InvalidateItem(rEvent.maChildToken);
            Fill();
        }
        break;

        default:
            break;
   }
}




MasterPagesSelector::UserData* MasterPagesSelector::CreateUserData (
    int nIndex,
    MasterPageContainer::Token aToken) const
{
    return new UserData(nIndex,aToken);
}




MasterPagesSelector::UserData* MasterPagesSelector::GetUserData (int nIndex) const
{
    const ::osl::MutexGuard aGuard (maMutex);

    if (nIndex>0 && static_cast<unsigned int>(nIndex)<=mpPageSet->GetItemCount())
        return reinterpret_cast<UserData*>(mpPageSet->GetItemData((sal_uInt16)nIndex));
    else
        return NULL;
}




void MasterPagesSelector::SetUserData (int nIndex, UserData* pData)
{
    const ::osl::MutexGuard aGuard (maMutex);

    if (nIndex>0 && static_cast<unsigned int>(nIndex)<=mpPageSet->GetItemCount())
    {
        UserData* pOldData = GetUserData(nIndex);
        if (pOldData!=NULL && pOldData!=pData)
            delete pOldData;
        mpPageSet->SetItemData((sal_uInt16)nIndex, pData);
    }
}




bool MasterPagesSelector::IsResizable (void)
{
    return false;
}




::Window* MasterPagesSelector::GetWindow (void)
{
    return mpPageSet.get();
}




sal_Int32 MasterPagesSelector::GetMinimumWidth (void)
{
    return mpContainer->GetPreviewSizePixel().Width() + 2*3;
}




void MasterPagesSelector::UpdateSelection (void)
{
}




void MasterPagesSelector::Execute (SfxRequest& rRequest)
{
    switch (rRequest.GetSlot())
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
            DBG_ASSERT (false,
                "Using slides as default for new presentations"
                " is not yet implemented");
            break;

        case SID_TP_SHOW_SMALL_PREVIEW:
        case SID_TP_SHOW_LARGE_PREVIEW:
        {
            mrBase.SetBusyState (true);
            mpContainer->SetPreviewSize(
                rRequest.GetSlot()==SID_TP_SHOW_SMALL_PREVIEW
                ? MasterPageContainer::SMALL
                : MasterPageContainer::LARGE);
            mrBase.SetBusyState (false);
            break;
        }

        case SID_TP_EDIT_MASTER:
        {
            using namespace ::com::sun::star;
            uno::Reference<drawing::XDrawPage> xSelectedMaster (
                GetSelectedMasterPage()->getUnoPage(), uno::UNO_QUERY);
            SfxViewFrame* pViewFrame = mrBase.GetViewFrame();
            if (pViewFrame != NULL && xSelectedMaster.is())
            {
                SfxDispatcher* pDispatcher = pViewFrame->GetDispatcher();
                if (pDispatcher != NULL)
                {
                    sal_uInt16 nIndex = mpPageSet->GetSelectItemId();
                    pDispatcher->Execute(SID_MASTERPAGE, SFX_CALLMODE_SYNCHRON);
                    mpPageSet->SelectItem (nIndex);
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




void MasterPagesSelector::GetState (SfxItemSet& rItemSet)
{
    if (mpContainer->GetPreviewSize() == MasterPageContainer::SMALL)
        rItemSet.DisableItem (SID_TP_SHOW_SMALL_PREVIEW);
    else
        rItemSet.DisableItem (SID_TP_SHOW_LARGE_PREVIEW);

    // Cut and paste is not supported so do not show the menu entries.
    rItemSet.DisableItem (SID_CUT);
    rItemSet.DisableItem (SID_COPY);
    rItemSet.DisableItem (SID_PASTE);
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
                if (mpPageSet->GetItemPos(nIndex) != VALUESET_ITEM_NOTFOUND)
                {
                    mpPageSet->SetItemImage(nIndex,aPreview);
                    mpPageSet->SetItemText(nIndex, mpContainer->GetPageNameForToken(aToken));
                }
                else
                {
                    mpPageSet->InsertItem (
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
            mpPageSet->RemoveItem(nIndex);
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
    if (pData != NULL)
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

    for (sal_uInt16 nIndex=1; nIndex<=mpPageSet->GetItemCount(); nIndex++)
    {
        UserData* pData = GetUserData(nIndex);
        if (pData != NULL)
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

void MasterPagesSelector::UpdateAllPreviews (void)
{
    const ::osl::MutexGuard aGuard (maMutex);

    for (sal_uInt16 nIndex=1; nIndex<=mpPageSet->GetItemCount(); nIndex++)
    {
        UserData* pData = GetUserData(nIndex);
        if (pData != NULL)
        {
            MasterPageContainer::Token aToken (pData->second);
            mpPageSet->SetItemImage(
                nIndex,
                mpContainer->GetPreviewForToken(aToken));
            if (mpContainer->GetPreviewState(aToken) == MasterPageContainer::PS_CREATABLE)
                mpContainer->RequestPreview(aToken);
        }
    }
    mpPageSet->Rearrange(true);
}




void MasterPagesSelector::ClearPageSet (void)
{
    const ::osl::MutexGuard aGuard (maMutex);

    for (sal_uInt16 nIndex=1; nIndex<=mpPageSet->GetItemCount(); nIndex++)
    {
        UserData* pData = GetUserData(nIndex);
        if (pData != NULL)
            delete pData;
    }
    mpPageSet->Clear();
}




void MasterPagesSelector::SetHelpId( const rtl::OString& aId )
{
    const ::osl::MutexGuard aGuard (maMutex);

    mpPageSet->SetHelpId( aId );
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




void MasterPagesSelector::Clear (void)
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



SAL_WNODEPRECATED_DECLARATIONS_PUSH
void MasterPagesSelector::UpdateItemList (::std::auto_ptr<ItemList> pNewItemList)
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

    mpPageSet->Rearrange();
    if (GetParentNode() != NULL)
        GetParentNode()->RequestResize();
}
SAL_WNODEPRECATED_DECLARATIONS_POP




} } } // end of namespace ::sd::toolpanel::controls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
