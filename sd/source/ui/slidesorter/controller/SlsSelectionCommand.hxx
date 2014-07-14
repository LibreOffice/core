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



#ifndef SD_SLIDESORTER_SELECTION_COMMAND_HXX
#define SD_SLIDESORTER_SELECTION_COMMAND_HXX

#include "controller/SlsPageSelector.hxx"
#include "SlsCommand.hxx"
#include <tools/solar.h>
#include <com/sun/star/drawing/XDrawPage.hpp>

#include <boost/shared_ptr.hpp>
#include <vector>

namespace sd { namespace slidesorter { namespace model {
class SlideSorterModel;
} } }


namespace sd { namespace slidesorter { namespace controller {

class CurrentSlideManager;
class PageSelector;

/** The SelectionCommand stores a list of pages that it will select on its
    execution.  Furthermore it will make a page the current page.  Note that
    internally pages are stored with pointers because this command is designed
    to be executed after model changes where page indices may change but
    page object identities remain.
*/
class SelectionCommand
    : public Command
{
public:
    /** Create a new command object that will on its execution use the given
        PageSelector to select a set of pages.
    */
    SelectionCommand (
        PageSelector& rSelector,
        const ::boost::shared_ptr<controller::CurrentSlideManager>& rpCurrentSlideManager,
        const model::SlideSorterModel& rModel);

    /** Remember the specified page to be selected when this command is
        executed.
    */
    void AddSlide (sal_uInt16 nPageIndex);

    /** Execute the command and select the pages added by previous calls to
        AddPages() and AddPage().
    */
    virtual void operator() (void);

private:
    /// The page selector is used to select pages and set the current page.
    PageSelector& mrPageSelector;
    /// Used for setting the current slide.
    ::boost::shared_ptr<controller::CurrentSlideManager> mpCurrentSlideManager;
    /// The model is used to translate page indices into page pointers.
    const model::SlideSorterModel& mrModel;
    /// The list of pages to be selected when the command is executed.
    typedef ::std::vector<sal_Int32> PageList;
    PageList maPagesToSelect;
    /** The page that will be made the current page when the command is
        executed.
    */
    sal_Int32 mnCurrentPageIndex;
};

} } } // end of namespace sd::slidesorter::controller

#endif
