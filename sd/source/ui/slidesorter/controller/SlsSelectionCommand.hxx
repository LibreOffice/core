/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsSelectionCommand.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:28:20 $
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

    /** Add the pages in the given list of selected pages to those that will
        be selected when this command is executed, i.e. its operator()
        method is called.
        The first page will be set as current page when the new current page
        has not been specified previously.
    */
    void AddSlides (const ::boost::shared_ptr<PageSelector::PageSelection>& rpSelection);

    /** Remember the specified page to be selected when this command is
        executed.
    */
    void AddSlide (USHORT nPageIndex);

    /** Call this method to explicitly set the page that will be made the
        current page when this command is executed.
    */
    void SetCurrentSlide (USHORT nPageIndex);

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
