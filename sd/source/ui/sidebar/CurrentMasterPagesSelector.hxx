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

#ifndef INCLUDED_SD_SOURCE_UI_SIDEBAR_CURRENTMASTERPAGESSELECTOR_HXX
#define INCLUDED_SD_SOURCE_UI_SIDEBAR_CURRENTMASTERPAGESSELECTOR_HXX

#include "MasterPagesSelector.hxx"


namespace sd { namespace tools { class EventMultiplexerEvent; } }

namespace sd { namespace sidebar {

/** Show the master pages currently used by a SdDrawDocument.
*/
class CurrentMasterPagesSelector
    : public MasterPagesSelector,
      public SfxListener
{
    friend class VclPtrInstance<CurrentMasterPagesSelector>;
public:
    static VclPtr<vcl::Window> Create (
        vcl::Window* pParent,
        ViewShellBase& rViewShellBase,
        const css::uno::Reference<css::ui::XSidebar>& rxSidebar);

    /** Set the selection so that the master page is selected that is
        used by the currently selected page of the document in the
        center pane.
    */
    void UpdateSelection();

    /** Copy all master pages that are to be shown into the given list.
    */
    virtual void Fill (ItemList& rItemList) override;

    using MasterPagesSelector::Fill;

protected:
    virtual OUString GetContextMenuUIFile() const override;

    virtual void ProcessPopupMenu (Menu& rMenu) override;
    virtual void ExecuteCommand(const OString &rIdent) override;

private:
    CurrentMasterPagesSelector (
        vcl::Window* pParent,
        SdDrawDocument& rDocument,
        ViewShellBase& rBase,
        const std::shared_ptr<MasterPageContainer>& rpContainer,
        const css::uno::Reference<css::ui::XSidebar>& rxSidebar);
    virtual ~CurrentMasterPagesSelector() override;
    virtual void dispose() override;

    virtual void LateInit() override;

    DECL_LINK(EventMultiplexerListener,sd::tools::EventMultiplexerEvent&, void);
};

} } // end of namespace sd::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
