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




void SelectionCommand::AddSlide (sal_uInt16 nPageIndex)
{
    maPagesToSelect.push_back(nPageIndex);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
