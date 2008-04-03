/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsSelectionCommand.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:27:59 $
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

#include "SlsSelectionCommand.hxx"

#include "controller/SlsCurrentSlideManager.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"

#include "sdpage.hxx"

namespace sd { namespace slidesorter { namespace controller {



SelectionCommand::SelectionCommand (
    PageSelector& rSelector,
    const ::boost::shared_ptr<CurrentSlideManager>& rpCurrentSlideManager,
    const model::SlideSorterModel& rModel)
    : mrPageSelector(rSelector),
      mpCurrentSlideManager(rpCurrentSlideManager),
      mrModel(rModel),
      maPagesToSelect(),
      mnCurrentPageIndex(-1)
{
}




void SelectionCommand::AddSlides (
    const ::boost::shared_ptr<PageSelector::PageSelection>& rpSelection)
{
    PageSelector::PageSelection::iterator iPage = rpSelection->begin();
    PageSelector::PageSelection::iterator iEnd = rpSelection->end();
    for (; iPage!=iEnd; ++iPage)
        AddSlide(((*iPage)->GetPageNum()-1)/2);
}




void SelectionCommand::AddSlide (USHORT nPageIndex)
{
    maPagesToSelect.push_back(nPageIndex);
}




void SelectionCommand::SetCurrentSlide (USHORT nPageIndex)
{
    mnCurrentPageIndex = nPageIndex;
}




void SelectionCommand::operator() (void)
{
    OSL_ASSERT(mpCurrentSlideManager.get()!=NULL);

    mrPageSelector.DeselectAllPages();

    if (mnCurrentPageIndex >= 0)
        mpCurrentSlideManager->SwitchCurrentSlide(mnCurrentPageIndex);

    PageList::iterator iPage = maPagesToSelect.begin();
    PageList::iterator iEnd = maPagesToSelect.end();
    for (; iPage!=iEnd; ++iPage)
    {
        sal_Int32 nIndex (*iPage);
        if (nIndex >= 0)
            mrPageSelector.SelectPage(mrModel.GetPageDescriptor(nIndex));
    }
}


} } } // end of namespace sd::slidesorter::controller
