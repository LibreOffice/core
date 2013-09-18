/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef SD_SIDEBAR_PANELS_ALL_MASTER_PAGES_SELECTOR_HXX
#define SD_SIDEBAR_PANELS_ALL_MASTER_PAGES_SELECTOR_HXX

#include "MasterPagesSelector.hxx"

#include <memory>

namespace sd {
class TemplateEntry;
}

namespace sd { namespace sidebar {


/** Show a list of all available master pages so that the user can assign
    them to the document.
*/
class AllMasterPagesSelector
    : public MasterPagesSelector
{
public:
    static MasterPagesSelector* Create (
        ::Window* pParent,
        ViewShellBase& rViewShellBase,
        const cssu::Reference<css::ui::XSidebar>& rxSidebar);

    /** Scan the set of templates for the ones whose first master pages are
        shown by this control and store them in the MasterPageContainer.
    */
    virtual void Fill (ItemList& rItemList);

    virtual void GetState (SfxItemSet& rItemSet);

protected:
    virtual void NotifyContainerChangeEvent (const MasterPageContainerChangeEvent& rEvent);

private:
    /** The list of master pages displayed by this class.
    */
    class SortedMasterPageDescriptorList;
    ::std::auto_ptr<SortedMasterPageDescriptorList> mpSortedMasterPages;

    AllMasterPagesSelector (
        ::Window* pParent,
        SdDrawDocument& rDocument,
        ViewShellBase& rBase,
        const ::boost::shared_ptr<MasterPageContainer>& rpContainer,
        const cssu::Reference<css::ui::XSidebar>& rxSidebar);
    virtual ~AllMasterPagesSelector (void);

    void AddTemplate (const TemplateEntry& rEntry);

    void AddItem (MasterPageContainer::Token aToken);

    /** Add all items in the internal master page list into the given list.
    */
    void UpdatePageSet (ItemList& rItemList);

    /** Update the internal list of master pages that are to show in the
        control.
    */
    void UpdateMasterPageList (void);

    using MasterPagesSelector::Fill;
};

} } // end of namespace sd::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
