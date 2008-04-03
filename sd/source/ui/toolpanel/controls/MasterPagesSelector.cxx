/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MasterPagesSelector.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:50:10 $
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
#include "showview.hxx"
#include "DrawViewShell.hxx"
#include "SlideSorterViewShell.hxx"
#include "PreviewValueSet.hxx"
#include "ViewShellBase.hxx"
#include "../TaskPaneShellManager.hxx"
#include "taskpane/TitledControl.hxx"
#include "taskpane/ControlContainer.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsPageSelector.hxx"

#ifndef _SFXOBJFACE_HXX
#include <sfx2/objface.hxx>
#endif
#ifndef _SDRESID_HXX
#include "sdresid.hxx"
#endif
#ifndef _TEMPLATE_SCANNER_HXX
#include "TemplateScanner.hxx"
#endif
#ifndef _SD_DRAWVIEW_HXX
#include "drawview.hxx"
#endif
#ifndef _SV_IMAGE_HXX
#include <vcl/image.hxx>
#endif
#ifndef _SVTOOLS_LANGUAGEOPTIONS_HXX
#include <svtools/languageoptions.hxx>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#include <sfx2/dispatch.hxx>
#include <sfx2/mnumgr.hxx>
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _SVX_DLGUTIL_HXX
#include <svx/dlgutil.hxx>
#endif
#ifndef _SVDPAGV_HXX
#include <svx/svdpagv.hxx>
#endif
#include <svx/svxids.hrc>
#include "FrameView.hxx"
#include "sdpage.hxx"
#include "stlpool.hxx"
#include "unmovss.hxx"
#include <sfx2/request.hxx>
#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif

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




IMPL_LINK(MasterPagesSelector, ClickHandler, PreviewValueSet*, EMPTYARG)
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
        USHORT nIndex = mpPageSet->GetItemId (pEvent->GetPosPixel());
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
    const USHORT nIndex = mpPageSet->GetSelectItemId();
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

        mrBase.GetViewFrame()->GetDispatcher()->ExecutePopup(
            SdResId(RID_TASKPANE_MASTERPAGESSELECTOR_POPUP),
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
    USHORT nIndex = mpPageSet->GetSelectItemId();
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
    do
    {
        if (pMasterPage == NULL)
            break;

        USHORT nPageCount = mrDocument.GetSdPageCount(PK_STANDARD);
        if (nPageCount == 0)
            break;

        // Get a list of all pages.  As a little optimization we only
        // include pages that do not already have the given master page
        // assigned.
        String sFullLayoutName (pMasterPage->GetLayoutName());
        ::sd::slidesorter::SharedPageSelection pPageList (
            new ::sd::slidesorter::SlideSorterViewShell::PageSelection());
        for (USHORT nPageIndex=0; nPageIndex<nPageCount; nPageIndex++)
        {
            SdPage* pPage = mrDocument.GetSdPage (nPageIndex, PK_STANDARD);
            if (pPage != NULL
                && pPage->GetLayoutName().CompareTo(sFullLayoutName)!=0)
            {
                pPageList->push_back (pPage);
            }
        }

        AssignMasterPageToPageList(pMasterPage, pPageList);
    }
    while (false);
}




/** Assemble a list of the currently selected slides (selected in a visible
    slide sorter) and pass it to AssignMasterPageToPageList().
*/
void MasterPagesSelector::AssignMasterPageToSelectedSlides (
    SdPage* pMasterPage)
{
    do
    {
        using namespace ::std;
        using namespace ::sd::slidesorter;
        using namespace ::sd::slidesorter::controller;

        if (pMasterPage == NULL)
            break;

        // Find a visible slide sorter.
        SlideSorterViewShell* pSlideSorter = SlideSorterViewShell::GetSlideSorter(mrBase);
        if (pSlideSorter == NULL)
            break;

        // Get a list of selected pages.
        ::sd::slidesorter::SharedPageSelection pPageSelection = pSlideSorter->GetPageSelection();
        if (pPageSelection->empty())
            break;

        AssignMasterPageToPageList(pMasterPage, pPageSelection);

        // Restore the previous selection.
        pSlideSorter->SetPageSelection(pPageSelection);
    }
    while (false);
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
                    (USHORT)nIndex,
                    mpContainer->GetPreviewForToken(rEvent.maChildToken));
                mpPageSet->Invalidate(mpPageSet->GetItemRect((USHORT)nIndex));
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

    if (nIndex>0 && nIndex<=mpPageSet->GetItemCount())
        return reinterpret_cast<UserData*>(mpPageSet->GetItemData((USHORT)nIndex));
    else
        return NULL;
}




void MasterPagesSelector::SetUserData (int nIndex, UserData* pData)
{
    const ::osl::MutexGuard aGuard (maMutex);

    if (nIndex>0 && nIndex<=mpPageSet->GetItemCount())
    {
        UserData* pOldData = GetUserData(nIndex);
        if (pOldData!=NULL && pOldData!=pData)
            delete pOldData;
        mpPageSet->SetItemData((USHORT)nIndex, pData);
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
    USHORT nIndex,
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
    USHORT nIndex,
    MasterPageContainer::Token aToken)
{
    const ::osl::MutexGuard aGuard (maMutex);

    maTokenToValueSetIndex[aToken] = nIndex;
}




void MasterPagesSelector::RemoveTokenToIndexEntry (
    USHORT nIndex,
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




void MasterPagesSelector::InvalidatePreview (MasterPageContainer::Token aToken)
{
    const ::osl::MutexGuard aGuard (maMutex);

    int nIndex (GetIndexForToken(aToken));
    if (nIndex > 0)
    {
        mpContainer->InvalidatePreview(aToken);
        mpContainer->RequestPreview(aToken);
    }
}




void MasterPagesSelector::InvalidatePreview (const SdPage* pPage)
{
    const ::osl::MutexGuard aGuard (maMutex);

    for (USHORT nIndex=1; nIndex<=mpPageSet->GetItemCount(); nIndex++)
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




void MasterPagesSelector::InvalidateAllPreviews (void)
{
    const ::osl::MutexGuard aGuard (maMutex);

    for (USHORT nIndex=1; nIndex<=mpPageSet->GetItemCount(); nIndex++)
        InvalidatePreview(nIndex);
}




void MasterPagesSelector::UpdateAllPreviews (void)
{
    const ::osl::MutexGuard aGuard (maMutex);

    for (USHORT nIndex=1; nIndex<=mpPageSet->GetItemCount(); nIndex++)
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

    for (USHORT nIndex=1; nIndex<=mpPageSet->GetItemCount(); nIndex++)
    {
        UserData* pData = GetUserData(nIndex);
        if (pData != NULL)
            delete pData;
    }
    mpPageSet->Clear();
}




void MasterPagesSelector::SetSmartHelpId( const SmartId& aId, SmartIdUpdateMode aMode )
{
    const ::osl::MutexGuard aGuard (maMutex);

    mpPageSet->SetSmartHelpId( aId, aMode );
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




void MasterPagesSelector::UpdateItemList (::std::auto_ptr<ItemList> pNewItemList)
{
    const ::osl::MutexGuard aGuard (maMutex);

    ItemList::const_iterator iNewItem (pNewItemList->begin());
    ItemList::const_iterator iCurrentItem (maCurrentItemList.begin());
    ItemList::const_iterator iNewEnd (pNewItemList->end());
    ItemList::const_iterator iCurrentEnd (maCurrentItemList.end());
    USHORT nIndex (1);

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





} } } // end of namespace ::sd::toolpanel::controls
