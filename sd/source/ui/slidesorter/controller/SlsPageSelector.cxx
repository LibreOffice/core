/*************************************************************************
 *
 *  $RCSfile: SlsPageSelector.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:15:13 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "controller/SlsPageSelector.hxx"

#include "SlideSorterViewShell.hxx"
#include "controller/SlideSorterController.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "model/SlsPageEnumeration.hxx"
#include "model/SlideSorterModel.hxx"
#include "view/SlideSorterView.hxx"

#include "sdpage.hxx"
#include "ViewShell.hxx"
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
      mpMostRecentlySelectedPage(NULL)
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
    mpMostRecentlySelectedPage = NULL;
}




void PageSelector::UpdateAllPages (void)
{
    mnSelectedPageCount = 0;
    model::SlideSorterModel::Enumeration aAllPages (
        mrModel.GetAllPagesEnumeration());
    while (aAllPages.HasMoreElements())
    {
        model::PageDescriptor& rDescriptor (aAllPages.GetNextElement());
        if (rDescriptor.UpdateSelection())
        {
            mrController.GetView().RequestRepaint (rDescriptor);
            if (mnBroadcastDisableLevel > 0)
                mbSelectionChangeBroadcastPending = true;
            else
                mrController.SelectionHasChanged();
        }

        if (rDescriptor.IsSelected())
            mnSelectedPageCount++;
    }
}




void PageSelector::SelectPage (int nPageIndex)
{
    PageDescriptor* pDescriptor = mrModel.GetPageDescriptor(nPageIndex);
    if (pDescriptor != NULL)
        SelectPage (*pDescriptor);
}




void PageSelector::SelectPage (const SdPage* pPage)
{
    int nPageIndex = pPage->GetPageNum() / 2;
    PageDescriptor* pDescriptor = mrModel.GetPageDescriptor(nPageIndex);
    if (pDescriptor!=NULL && pDescriptor->GetPage()==pPage)
        SelectPage (*pDescriptor);
}




void PageSelector::SelectPage (PageDescriptor& rDescriptor)
{
    if (rDescriptor.Select())
    {
        mnSelectedPageCount ++;
        mrController.GetView().RequestRepaint (rDescriptor);
        mpMostRecentlySelectedPage = &rDescriptor;
        if (mnBroadcastDisableLevel > 0)
            mbSelectionChangeBroadcastPending = true;
        else
            mrController.SelectionHasChanged();
    }
}




void PageSelector::DeselectPage (int nPageIndex)
{
    model::PageDescriptor* pDescriptor (
        mrModel.GetPageDescriptor(nPageIndex));
    if (pDescriptor != NULL)
        DeselectPage (*pDescriptor);
}




void PageSelector::DeselectPage (const SdPage* pPage)
{
    int nPageIndex = pPage->GetPageNum() / 2;
    PageDescriptor* pDescriptor = mrModel.GetPageDescriptor(nPageIndex);
    if (pDescriptor!=NULL && pDescriptor->GetPage()==pPage)
        DeselectPage (*pDescriptor);
}




void PageSelector::DeselectPage (PageDescriptor& rDescriptor)
{
    if (rDescriptor.Deselect())
    {
        mnSelectedPageCount --;
        mrController.GetView().RequestRepaint (rDescriptor);
        if (mpMostRecentlySelectedPage == &rDescriptor)
            mpMostRecentlySelectedPage = NULL;
        if (mnBroadcastDisableLevel > 0)
            mbSelectionChangeBroadcastPending = true;
        else
            mrController.SelectionHasChanged();
    }
}




bool PageSelector::IsPageSelected (int nPageIndex)
{
    PageDescriptor* pDescriptor = mrModel.GetPageDescriptor(nPageIndex);
    if (pDescriptor != NULL)
        return pDescriptor->IsSelected();
    else
        return false;
}




void PageSelector::SetCurrentPage (PageDescriptor& rDescriptor)
{
    // Set current page.
    try
    {
        do
        {
            Reference<beans::XPropertySet> xSet (
                mrController.GetViewShell().GetViewShellBase().GetController(),
                UNO_QUERY);
            if ( ! xSet.is())
                break;

            Any aPage;
            aPage <<= rDescriptor.GetPage()->getUnoPage();
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




void PageSelector::SetCurrentPage (int nPageIndex)
{
    PageDescriptor* pDescriptor = mrModel.GetPageDescriptor(nPageIndex);
    if (pDescriptor != NULL)
        SetCurrentPage (*pDescriptor);
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




PageDescriptor* PageSelector::GetMostRecentlySelectedPage (void) const
{
    return mpMostRecentlySelectedPage;
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



} } } // end of namespace ::sd::slidesorter::controller
