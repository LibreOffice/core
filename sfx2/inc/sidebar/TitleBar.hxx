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
#pragma once

#include <sidebar/SidebarToolBox.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/weldutils.hxx>
#include <vcl/InterimItemWindow.hxx>

namespace sfx2::sidebar {

class TitleBar : public InterimItemWindow
{
public:
    TitleBar(vcl::Window* pParentWindow,
             const OUString& rUIXMLDescription, const OString& rID,
             Theme::ThemeItem eThemeItem);
    virtual void dispose() override;
    virtual ~TitleBar() override;

    virtual void SetTitle (const OUString& rsTitle) = 0;
    virtual OUString GetTitle() const = 0;

    void SetIcon(const css::uno::Reference<css::graphic::XGraphic>& rIcon);

    virtual void DataChanged (const DataChangedEvent& rEvent) override;

    weld::Toolbar& GetToolBox()
    {
        return *mxToolBox;
    }
    const weld::Toolbar& GetToolBox() const
    {
        return *mxToolBox;
    }

protected:
    std::unique_ptr<weld::Image> mxAddonImage;
    std::unique_ptr<weld::Toolbar> mxToolBox;
    std::unique_ptr<ToolbarUnoDispatcher> mxToolBoxController;
    Theme::ThemeItem meThemeItem;

    virtual void HandleToolBoxItemClick() = 0;

    DECL_LINK(SelectionHandler, const OString&, void);
};

} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
