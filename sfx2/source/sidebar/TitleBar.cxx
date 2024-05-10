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

#include <sidebar/TitleBar.hxx>
#include <vcl/help.hxx>
#include <vcl/svapp.hxx>

namespace sfx2::sidebar {

TitleBar::TitleBar(weld::Builder& rBuilder, Theme::ThemeItem eThemeItem)
    : mrBuilder(rBuilder)
    , mxTitlebar(rBuilder.weld_box(u"titlebar"_ustr))
    , mxAddonImage(rBuilder.weld_image(u"addonimage"_ustr))
    , mxToolBox(rBuilder.weld_toolbar(u"toolbar"_ustr))
    , meThemeItem(eThemeItem)
    , msToolBoxRId(u""_ustr)
{
    SetBackground();

    mxToolBox->connect_clicked(LINK(this, TitleBar, SelectionHandler));
}

void TitleBar::SetBackground()
{
    Color aColor(Theme::GetColor(meThemeItem));
    mxTitlebar->set_background(aColor);
    mxToolBox->set_background(aColor);
}

void TitleBar::DataChanged()
{
    SetBackground();
}

TitleBar::~TitleBar()
{
}

Size TitleBar::get_preferred_size() const
{
    return mxTitlebar->get_preferred_size();
}

void TitleBar::Show(bool bShow)
{
    mxTitlebar->set_visible(bShow);
}

bool TitleBar::GetVisible() const
{
    return mxTitlebar->get_visible();
}

void TitleBar::SetIcon(const css::uno::Reference<css::graphic::XGraphic>& rIcon)
{
    mxAddonImage->set_image(rIcon);
    mxAddonImage->set_visible(rIcon.is());
}

void TitleBar::ShowHelp(const OUString& rHelpId)
{
    Help* pHelp = Application::GetHelp();
    if (pHelp)
        pHelp->Start(rHelpId);
}

IMPL_LINK(TitleBar, SelectionHandler, const OUString&, rId, void)
{
    msToolBoxRId = rId;
    HandleToolBoxItemClick();
}

} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
