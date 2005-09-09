/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsSelectionCommand.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 06:16:00 $
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

#include "SlsSelectionCommand.hxx"

#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"

namespace sd { namespace slidesorter { namespace controller {



SelectionCommand::SelectionCommand (
    PageSelector& rSelector,
    const model::SlideSorterModel& rModel)
    : mrPageSelector(rSelector),
      mrModel(rModel),
      maPagesToSelect(),
      mpCurrentPage(NULL)
{
}




void SelectionCommand::AddPages (::std::auto_ptr<PageSelector::PageSelection> pSelection)
{
    PageSelector::PageSelection::iterator iPageIndex = pSelection->begin();
    PageSelector::PageSelection::iterator iEnd = pSelection->end();
    for (; iPageIndex!=iEnd; ++iPageIndex)
        AddPage(*iPageIndex);
}




void SelectionCommand::AddPage (USHORT nPageIndex)
{
    model::PageDescriptor* pDescriptor = mrModel.GetPageDescriptor(nPageIndex);
    if (pDescriptor != NULL)
    {
        maPagesToSelect.push_back(pDescriptor->GetPage());
        if (mpCurrentPage == NULL)
            mpCurrentPage = pDescriptor->GetPage();
    }
}




void SelectionCommand::SetCurrentPage (USHORT nPageIndex)
{
    model::PageDescriptor* pDescriptor = mrModel.GetPageDescriptor(nPageIndex);
    if (pDescriptor != NULL)
        mpCurrentPage = pDescriptor->GetPage();
}




void SelectionCommand::operator() (void)
{
    mrPageSelector.SetCurrentPage(mpCurrentPage);
    PageList::iterator iPage = maPagesToSelect.begin();
    PageList::iterator iEnd = maPagesToSelect.end();
    for (; iPage!=iEnd; ++iPage)
        mrPageSelector.SelectPage(*iPage);
}


} } } // end of namespace sd::slidesorter::controller
