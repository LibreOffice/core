/*************************************************************************
 *
 *  $RCSfile: SlsSelectionCommand.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2005-03-23 13:59:41 $
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

#ifndef SD_SLIDESORTER_SELECTION_COMMAND_HXX
#define SD_SLIDESORTER_SELECTION_COMMAND_HXX

#include "controller/SlsPageSelector.hxx"
#include "SlsCommand.hxx"
#include <tools/solar.h>

#include <memory>
#include <vector>

namespace sd { namespace slidesorter { namespace model {
class SlideSorterModel;
} } }


namespace sd { namespace slidesorter { namespace controller {

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
        const model::SlideSorterModel& rModel);

    /** Add the pages in the given list of selected pages to those that will
        be selected when this command is executed, i.e. its operator()
        method is called.
        The first page will be set as current page when the new current page
        has not been specified previously.
    */
    void AddPages (::std::auto_ptr<PageSelector::PageSelection> pSelection);

    /** Remember the specified page to be selected when this command is
        executed.
    */
    void AddPage (USHORT nPageIndex);

    /** Call this method to explicitly set the page that will be made the
        current page when this command is executed.
    */
    void SetCurrentPage (USHORT nPageIndex);

    /** Execute the command and select the pages added by previous calls to
        AddPages() and AddPage().
    */
    virtual void operator() (void);

private:
    /// The page selector is used to select pages and set the current page.
    PageSelector& mrPageSelector;
    /// The model is used to translate page indices into page pointers.
    const model::SlideSorterModel& mrModel;
    /// The list of pages to be selected when the command is executed.
    typedef ::std::vector<SdPage*> PageList;
    PageList maPagesToSelect;
    /// The page that will be made the current page when the command is executed.
    SdPage* mpCurrentPage;
};

} } } // end of namespace sd::slidesorter::controller

#endif
