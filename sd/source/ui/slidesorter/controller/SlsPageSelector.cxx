/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsPageSelector.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:26:44 $
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

#include "precompiled_sd.hxx"

#include "controller/SlsPageSelector.hxx"

#include "SlideSorter.hxx"
#include "SlideSorterViewShell.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsSelectionManager.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "model/SlsPageEnumerationProvider.hxx"
#include "model/SlideSorterModel.hxx"
#include "view/SlideSorterView.hxx"

#include "sdpage.hxx"
#include "ViewShell.hxx"
#include "DrawViewShell.hxx"
#include "ViewShellBase.hxx"

#ifndef _COM_SUN_STAR_DRAWING_XDRAWVIEW_HPP_
#include <com/sun/star/drawing/XDrawView.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::sd::slidesorter::model;
using namespace ::sd::slidesorter::view;

namespace sd { namespace slidesorter { namespace controller {


PageSelector::PageSelector (SlideSorter& rSlideSorter)
    : mrModel(rSlideSorter.GetModel()),
      mrSlideSorter(rSlideSorter),
      mrController(mrSlideSorter.GetController()),
      mnSelectedPageCount(0),
      mnBroadcastDisableLevel(0),
      mbSelectionChangeBroadcastPending(false),
      mpMostRecentlySelectedPage(),
      mpSelectionAnchor()
{
    CountSelectedPages ();
}




void PageSelector::SelectAllPages (void)
{
    int nPageCount = mrModel.GetPageCount();
    for (int nPageIndex=0; nPageIndex<nPageCount; nPageIndex++)
        SelectPage (nPageIndex);
}




void PageSelector::DeselectAllPages (void)
{
    int nPageCount = mrModel.GetPageCount();
    for (int nPageIndex=0; nPageIndex<nPageCount; nPageIndex++)
        DeselectPage (nPageIndex);
    DBG_ASSERT (mnSelectedPageCount==0,
        "PageSelector::DeselectAllPages: the selected pages counter is not 0");
    mnSelectedPageCount = 0;
    mpMostRecentlySelectedPage.reset();
    mpSelectionAnchor.reset();
}




void PageSelector::UpdateAllPages (void)
{
    bool bSelectionHasChanged (true);
    mnSelectedPageCount = 0;
    model::PageEnumeration aAllPages (
        model::PageEnumerationProvider::CreateAllPagesEnumeration(mrModel));
    while (aAllPages.HasMoreElements())
    {
        model::SharedPageDescriptor pDescriptor (aAllPages.GetNextElement());
        if (pDescriptor->UpdateSelection())
        {
            mrSlideSorter.GetView().RequestRepaint(pDescriptor);
            bSelectionHasChanged = true;
        }

        if (pDescriptor->IsSelected())
            mnSelectedPageCount++;
    }

    if (bSelectionHasChanged)
        if (mnBroadcastDisableLevel > 0)
            mbSelectionChangeBroadcastPending = true;
        else
            mrController.GetSelectionManager()->SelectionHasChanged();
}




void PageSelector::SelectPage (int nPageIndex)
{
    SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(nPageIndex));
    if (pDescriptor.get() != NULL)
        SelectPage(pDescriptor);
}




void PageSelector::SelectPage (const SdPage* pPage)
{
    int nPageIndex = (pPage->GetPageNum()-1) / 2;
    SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(nPageIndex));
    if (pDescriptor.get()!=NULL && pDescriptor->GetPage()==pPage)
        SelectPage(pDescriptor);
}




void PageSelector::SelectPage (const SharedPageDescriptor& rpDescriptor)
{
    if (rpDescriptor.get()!=NULL && rpDescriptor->Select())
    {
        mnSelectedPageCount ++;
        mrSlideSorter.GetView().RequestRepaint(rpDescriptor);

        mpMostRecentlySelectedPage = rpDescriptor;
        if (mpSelectionAnchor == NULL)
            mpSelectionAnchor = rpDescriptor;

        if (mnBroadcastDisableLevel > 0)
            mbSelectionChangeBroadcastPending = true;
        else
            mrController.GetSelectionManager()->SelectionHasChanged();
    }
}




void PageSelector::DeselectPage (int nPageIndex)
{
    model::SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(nPageIndex));
    if (pDescriptor.get() != NULL)
        DeselectPage(pDescriptor);
}




void PageSelector::DeselectPage (const SdPage* pPage)
{
    int nPageIndex = (pPage->GetPageNum()-1) / 2;
    SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(nPageIndex));
    if (pDescriptor.get()!=NULL && pDescriptor->GetPage()==pPage)
        DeselectPage(pDescriptor);
}




void PageSelector::DeselectPage (const SharedPageDescriptor& rpDescriptor)
{
    if (rpDescriptor.get()!=NULL && rpDescriptor->Deselect())
    {
        mnSelectedPageCount --;
        mrSlideSorter.GetView().RequestRepaint(rpDescriptor);
        if (mpMostRecentlySelectedPage == rpDescriptor)
            mpMostRecentlySelectedPage.reset();
        if (mnBroadcastDisableLevel > 0)
            mbSelectionChangeBroadcastPending = true;
        else
            mrController.GetSelectionManager()->SelectionHasChanged();
    }
}




bool PageSelector::IsPageSelected (int nPageIndex)
{
    SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(nPageIndex));
    if (pDescriptor.get() != NULL)
        return pDescriptor->IsSelected();
    else
        return false;
}




int PageSelector::GetPageCount (void) const
{
    return mrModel.GetPageCount();
}




int PageSelector::GetSelectedPageCount (void) const
{
    return mnSelectedPageCount;
}




void PageSelector::PrepareModelChange (void)
{
    DeselectAllPages ();
}




void PageSelector::HandleModelChange (void)
{
    UpdateAllPages();
}




SharedPageDescriptor PageSelector::GetMostRecentlySelectedPage (void) const
{
    return mpMostRecentlySelectedPage;
}




SharedPageDescriptor PageSelector::GetSelectionAnchor (void) const
{
    return mpSelectionAnchor;
}




void PageSelector::CountSelectedPages (void)
{
    mnSelectedPageCount = 0;
    model::PageEnumeration aSelectedPages (
        model::PageEnumerationProvider::CreateSelectedPagesEnumeration(mrModel));
    while (aSelectedPages.HasMoreElements())
    {
        mnSelectedPageCount++;
        aSelectedPages.GetNextElement();
    }
}




void PageSelector::EnableBroadcasting (bool bMakeSelectionVisible)
{
    if (mnBroadcastDisableLevel > 0)
        mnBroadcastDisableLevel --;
    if (mnBroadcastDisableLevel==0 && mbSelectionChangeBroadcastPending)
    {
        mrController.GetSelectionManager()->SelectionHasChanged(bMakeSelectionVisible);
        mbSelectionChangeBroadcastPending = false;
    }
}




void PageSelector::DisableBroadcasting (void)
{
    mnBroadcastDisableLevel ++;
}




::boost::shared_ptr<PageSelector::PageSelection> PageSelector::GetPageSelection (void) const
{
    ::boost::shared_ptr<PageSelection> pSelection (new PageSelection());
    pSelection->reserve(GetSelectedPageCount());

    int nPageCount = GetPageCount();
    for (int nIndex=0; nIndex<nPageCount; nIndex++)
    {
        SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(nIndex));
        if (pDescriptor.get()!=NULL && pDescriptor->IsSelected())
            pSelection->push_back(pDescriptor->GetPage());
    }

    return pSelection;
}




void PageSelector::SetPageSelection (const ::boost::shared_ptr<PageSelection>& rpSelection)
{
    PageSelection::const_iterator iPage;
    for (iPage=rpSelection->begin(); iPage!=rpSelection->end(); ++iPage)
        SelectPage(*iPage);
}




} } } // end of namespace ::sd::slidesorter::controller
