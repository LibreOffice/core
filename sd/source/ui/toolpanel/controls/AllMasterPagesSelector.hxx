/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AllMasterPagesSelector.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 18:45:31 $
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

#ifndef SD_TOOLPANEL_CONTROLS_ALL_MASTER_PAGES_SELECTOR_HXX
#define SD_TOOLPANEL_CONTROLS_ALL_MASTER_PAGES_SELECTOR_HXX

#include "MasterPagesSelector.hxx"

#include <memory>

namespace sd { namespace toolpanel { namespace controls {


/** Show a list of all available master pages so that the user can assign
    them to the document.
*/
class AllMasterPagesSelector
    : public MasterPagesSelector
{
public:
    AllMasterPagesSelector (
        TreeNode* pParent,
        SdDrawDocument& rDocument,
        ViewShellBase& rBase,
        DrawViewShell& rViewShell,
        const ::boost::shared_ptr<MasterPageContainer>& rpContainer);
    virtual ~AllMasterPagesSelector (void);

    /** Scan the set of templates for the ones whose first master pages are
        shown by this control and store them in the MasterPageContainer.
    */
    virtual void Fill (ItemList& rItemList);

protected:
    virtual void NotifyContainerChangeEvent (const MasterPageContainerChangeEvent& rEvent);

private:
    DrawViewShell& mrViewShell;

    /** The list of master pages displayed by this class.
    */
    class SortedMasterPageDescriptorList;
    ::std::auto_ptr<SortedMasterPageDescriptorList> mpSortedMasterPages;

    void AddTemplate (const TemplateEntry& rEntry);

    /** This filter returns <TRUE/> when the master page specified by the
        given file name belongs to the set of Impress master pages.
    */
    bool FileFilter (const String& sFileName);

    void AddItem (MasterPageContainer::Token aToken);

    /** Add all items in the internal master page list into the given list.
    */
    void UpdatePageSet (ItemList& rItemList);

    /** Update the internal list of master pages that are to show in the
        control.
    */
    void UpdateMasterPageList (void);

    using sd::toolpanel::controls::MasterPagesSelector::Fill;
};

} } } // end of namespace ::sd::toolpanel::controls

#endif
