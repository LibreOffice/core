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

#ifndef INCLUDED_SD_SOURCE_UI_SIDEBAR_RECENTMASTERPAGESSELECTOR_HXX
#define INCLUDED_SD_SOURCE_UI_SIDEBAR_RECENTMASTERPAGESSELECTOR_HXX

#include "MasterPagesSelector.hxx"

namespace sd::sidebar {

/** Show the recently used master pages (that are not currently used).
*/
class RecentMasterPagesSelector final
    : public MasterPagesSelector
{
    friend class VclPtrInstance<RecentMasterPagesSelector>;
public:
    static VclPtr<PanelLayout> Create (
        vcl::Window* pParent,
        ViewShellBase& rViewShellBase,
        const css::uno::Reference<css::ui::XSidebar>& rxSidebar);

private:
    DECL_LINK(MasterPageListListener, LinkParamNone*, void);
    virtual void Fill (ItemList& rItemList) override;

    using sd::sidebar::MasterPagesSelector::Fill;

    /** Forward this call to the base class but save and restore the
        currently selected item.
        Assign the given master page to the list of pages.
        @param pMasterPage
            This master page will usually be a member of the list of all
            available master pages as provided by the MasterPageContainer.
        @param rPageList
            The pages to which to assign the master page.  These pages may
            be slides or master pages themselves.
    */
    virtual void AssignMasterPageToPageList (
        SdPage* pMasterPage,
        const std::shared_ptr<std::vector<SdPage*> >& rpPageList) override;

    RecentMasterPagesSelector (
        vcl::Window* pParent,
        SdDrawDocument& rDocument,
        ViewShellBase& rBase,
        const std::shared_ptr<MasterPageContainer>& rpContainer,
        const css::uno::Reference<css::ui::XSidebar>& rxSidebar);
    virtual ~RecentMasterPagesSelector() override;
    virtual void dispose() override;

    virtual void LateInit() override;
};

} // end of namespace sd::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
