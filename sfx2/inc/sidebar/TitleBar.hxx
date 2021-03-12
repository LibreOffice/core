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

namespace sfx2::sidebar {

class TitleBar
{
public:
    TitleBar(weld::Builder& rBuilder, Theme::ThemeItem eThemeItem);
    virtual ~TitleBar();

    virtual void SetTitle (const OUString& rsTitle) = 0;
    virtual OUString GetTitle() const = 0;

    virtual void DataChanged();

    void Show(bool bShow);
    bool GetVisible() const;

    Size get_preferred_size() const;

    void SetIcon(const css::uno::Reference<css::graphic::XGraphic>& rIcon);

    weld::Toolbar& GetToolBox()
    {
        return *mxToolBox;
    }
    const weld::Toolbar& GetToolBox() const
    {
        return *mxToolBox;
    }

protected:
    weld::Builder& mrBuilder;
    std::unique_ptr<weld::Container> mxTitlebar;
    std::unique_ptr<weld::Image> mxAddonImage;
    std::unique_ptr<weld::Toolbar> mxToolBox;
    std::unique_ptr<ToolbarUnoDispatcher> mxToolBoxController;
    Theme::ThemeItem meThemeItem;

    virtual void HandleToolBoxItemClick() = 0;

    DECL_LINK(SelectionHandler, const OString&, void);

private:
    void SetBackground();
};

} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
