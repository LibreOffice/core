/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CurrentMasterPagesSelector.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 06:38:36 $
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

#include "CurrentMasterPagesSelector.hxx"
#include "MasterPagesSelectorListener.hxx"
#include "PreviewValueSet.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "MasterPageContainer.hxx"
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
    DrawViewShell& rViewShell)
    : MasterPagesSelector (pParent, rDocument, rBase),
      mxListener(NULL)
{
    SetName(String(RTL_CONSTASCII_USTRINGPARAM("CurrentMasterPagesSelector")));

    // For this master page selector only we change the default action for
    // left clicks.
    mnDefaultClickAction = SID_TP_APPLY_TO_SELECTED_SLIDES;
}




CurrentMasterPagesSelector::~CurrentMasterPagesSelector (void)
{
    if (mxListener.is())
    {
        mxListener->dispose();
        mxListener = NULL;
    }
}




void CurrentMasterPagesSelector::LateInit (void)
{
    MasterPagesSelector::LateInit ();
    Fill ();
    // The selection listener is reference counted and thus takes care
    // of its own lifetime.
    mxListener = uno::Reference<lang::XComponent>(
        static_cast<uno::XWeak*>(
            new MasterPagesSelectorListener (mrBase, *this)),
        uno::UNO_QUERY);
}




void CurrentMasterPagesSelector::Fill (void)
{
    Clear();

    USHORT nPageCount = mrDocument.GetMasterSdPageCount(PK_STANDARD);
    SdPage* pMasterPage;
    // Remember the names of the master pages that have been inserted to
    // avoid double insertion.
    ::std::set<String> aMasterPageNames;
    for (USHORT nIndex=0; nIndex<nPageCount; nIndex++)
    {
        pMasterPage = mrDocument.GetMasterSdPage (nIndex, PK_STANDARD);
        if (pMasterPage != NULL)
        {
            String sName (pMasterPage->GetName());
            if (aMasterPageNames.find(sName)==aMasterPageNames.end())
            {
                aMasterPageNames.insert (sName);

                int nIndex = mpPageSet->GetItemCount() + 1;

                Image aPreview = MasterPageContainer::Instance().GetPreviewForPage (
                    pMasterPage,
                    mnPreviewWidth);
                mpPageSet->InsertItem (
                    nIndex,
                    aPreview,
                    pMasterPage->GetName());
                mpPageSet->SetItemData (
                    nIndex,
                    pMasterPage);
            }
        }
    }
    GetParentNode()->RequestResize();
}




void CurrentMasterPagesSelector::Clear (void)
{
    mpPageSet->Clear ();
}




void CurrentMasterPagesSelector::UpdateAllPreviews (void)
{
    SdPage* pMasterPage;
    for (USHORT nIndex=1; nIndex<=mpPageSet->GetItemCount(); nIndex++)
    {
        pMasterPage = reinterpret_cast<SdPage*>(mpPageSet->GetItemData(nIndex));
        if (pMasterPage != NULL)
            mpPageSet->SetItemImage (nIndex,
                MasterPageContainer::Instance().GetPreviewForPage (
                    pMasterPage,
                    mnPreviewWidth));
    }
    GetParentNode()->RequestResize();
}




void CurrentMasterPagesSelector::UpdatePreview (USHORT nIndex)
{
    SdPage* pMasterPage = reinterpret_cast<SdPage*>(mpPageSet->GetItemData(nIndex));
    if (pMasterPage != NULL)
        mpPageSet->SetItemImage (nIndex,
            MasterPageContainer::Instance().GetPreviewForPage (
                pMasterPage,
                mnPreviewWidth));
    GetParentNode()->RequestResize();
}




void CurrentMasterPagesSelector::InvalidatePreview (const SdPage* pPage)
{
    for (USHORT nIndex=1; nIndex<=mpPageSet->GetItemCount(); nIndex++)
    {
        if (pPage == reinterpret_cast<SdPage*>(mpPageSet->GetItemData(nIndex)))
        {
            MasterPagesSelector::InvalidatePreview(nIndex);
            break;
        }
    }
}




SdPage* CurrentMasterPagesSelector::GetSelectedMasterPage (void)
{
    USHORT nIndex = mpPageSet->GetSelectItemId();
    SdPage* pMasterPage = reinterpret_cast<SdPage*>(mpPageSet->GetItemData (nIndex));
    return pMasterPage;
}




void CurrentMasterPagesSelector::UpdateSelection (void)
{
    // Iterate over all pages and for the selected ones put the name of
    // their master page into a set.
    USHORT nPageCount = mrDocument.GetSdPageCount(PK_STANDARD);
    SdPage* pPage;
    ::std::set<String> aNames;
    USHORT nIndex;
    for (nIndex=0; nIndex<nPageCount; nIndex++)
    {
        pPage = mrDocument.GetSdPage (nIndex, PK_STANDARD);
        if (pPage != NULL && pPage->IsSelected())
        {
            SdrPage& rMasterPage (pPage->TRG_GetMasterPage());
            SdPage* pMasterPage = static_cast<SdPage*>(&rMasterPage);
            if (pMasterPage != NULL)
                aNames.insert (pMasterPage->GetName());
        }
    }

    // Find the items for the master pages in the set.
    USHORT nItemCount (mpPageSet->GetItemCount());
    for (nIndex=1; nIndex<=nItemCount; nIndex++)
    {
        String sName (mpPageSet->GetItemText (nIndex));
        if (aNames.find(sName) != aNames.end())
        {
            mpPageSet->SelectItem (nIndex);
        }
    }
}


} } } // end of namespace ::sd::toolpanel::controls
