/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
