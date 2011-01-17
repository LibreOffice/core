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
    /** Create a new command object that will on its exection use the given
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
