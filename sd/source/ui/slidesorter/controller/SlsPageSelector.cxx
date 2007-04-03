/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsPageSelector.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-03 16:17:36 $
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

#include "controller/SlsPageSelector.hxx"

#include "SlideSorterViewShell.hxx"
#include "controller/SlideSorterController.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "model/SlsPageEnumeration.hxx"
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


PageSelector::PageSelector (
    model::SlideSorterModel& rModel,
    SlideSorterController& rController)
    : mrModel(rModel),
      mrController (rController),
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
    mnSelectedPageCount = 0;
    model::SlideSorterModel::Enumeration aAllPages (
        mrModel.GetAllPagesEnumeration());
    while (aAllPages.HasMoreElements())
    {
        model::SharedPageDescriptor pDescriptor (aAllPages.GetNextElement());
        if (pDescriptor->UpdateSelection())
        {
            mrController.GetView().RequestRepaint(pDescriptor);
            if (mnBroadcastDisableLevel > 0)
                mbSelectionChangeBroadcastPending = true;
            else
                mrController.SelectionHasChanged();
        }

        if (pDescriptor->IsSelected())
            mnSelectedPageCount++;
    }
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
        mrController.GetView().RequestRepaint(rpDescriptor);

        mpMostRecentlySelectedPage = rpDescriptor;
        if (mpSelectionAnchor == NULL)
            mpSelectionAnchor = rpDescriptor;

        if (mnBroadcastDisableLevel > 0)
            mbSelectionChangeBroadcastPending = true;
        else
            mrController.SelectionHasChanged();
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
        mrController.GetView().RequestRepaint(rpDescriptor);
        if (mpMostRecentlySelectedPage == rpDescriptor)
            mpMostRecentlySelectedPage.reset();
        if (mnBroadcastDisableLevel > 0)
            mbSelectionChangeBroadcastPending = true;
        else
            mrController.SelectionHasChanged();
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




void PageSelector::SetCurrentPage (const SharedPageDescriptor& rpDescriptor)
{
    // Set current page.  At the moment we have to do this in two different
    // ways.  The UNO way is the preferable one but, alas, it does not work
    // always correctly (after some kinds of model changes).  Therefore, we
    // call DrawViewShell::SwitchPage(), too.
    try
    {
        // First the traditional way.
        DrawViewShell* pDrawViewShell = dynamic_cast<DrawViewShell*>(
            mrController.GetViewShell().GetViewShellBase().GetMainViewShell().get());
        if (pDrawViewShell != NULL)
        {
            USHORT nPageNumber = (rpDescriptor->GetPage()->GetPageNum()-1)/2;
            pDrawViewShell->SwitchPage(nPageNumber);
            pDrawViewShell->GetPageTabControl()->SetCurPageId(nPageNumber+1);
        }

        // Now the UNO way.
        do
        {
            Reference<beans::XPropertySet> xSet (
                mrController.GetViewShell().GetViewShellBase().GetController(),
                UNO_QUERY);
            if ( ! xSet.is())
                break;

            Any aPage;
            aPage <<= rpDescriptor->GetPage()->getUnoPage();
            xSet->setPropertyValue (
                String::CreateFromAscii("CurrentPage"),
                aPage);
        }
        while (false);
    }
    catch (beans::UnknownPropertyException aException)
    {
        // We have not been able to set the current page at the main view.
        // This is sad but still leaves us in a valid state.  Therefore,
        // this exception is silently ignored.
    }
}




void PageSelector::SetCurrentPage (const SdPage* pPage)
{
    int nPageIndex = (pPage->GetPageNum()-1) / 2;
    SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(nPageIndex));
    if (pDescriptor.get()!=NULL && pDescriptor->GetPage()==pPage)
        SetCurrentPage(pDescriptor);
}




void PageSelector::SetCurrentPage (int nPageIndex)
{
    SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(nPageIndex));
    if (pDescriptor.get() != NULL)
        SetCurrentPage(pDescriptor);
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
    model::SlideSorterModel::Enumeration aSelectedPages (
        mrModel.GetSelectedPagesEnumeration());
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
        mrController.SelectionHasChanged(bMakeSelectionVisible);
        mbSelectionChangeBroadcastPending = false;
    }
}




void PageSelector::DisableBroadcasting (void)
{
    mnBroadcastDisableLevel ++;
}




::std::auto_ptr<PageSelector::PageSelection>
    PageSelector::GetPageSelection (void)
{
    ::std::auto_ptr<PageSelection> pSelection (new PageSelection());

    int nPageCount = GetPageCount();
    for (int nIndex=0; nIndex<nPageCount; nIndex++)
    {
        if (IsPageSelected(nIndex))
            pSelection->insert (nIndex);
    }

    return pSelection;
}




void PageSelector::SetPageSelection (const PageSelection& rSelection)
{
    PageSelection::const_iterator iIndex;
    for (iIndex=rSelection.begin(); iIndex!=rSelection.end(); ++iIndex)
        SelectPage (*iIndex);
}

} } } // end of namespace ::sd::slidesorter::controller
