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

    virtual void GetState (SfxItemSet& rItemSet);

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
