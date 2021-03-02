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

namespace sfx2::sidebar {

TitleBarBase::TitleBarBase(weld::Builder& rBuilder, Theme::ThemeItem eThemeItem)
    : mrBuilder(rBuilder)
    , mxAddonImage(rBuilder.weld_image("addonimage"))
    , mxToolBox(rBuilder.weld_toolbar("toolbar"))
    , meThemeItem(eThemeItem)
{
    mxToolBox->set_background(Theme::GetColor(meThemeItem));

    mxToolBox->connect_clicked(LINK(this, TitleBarBase, SelectionHandler));
}

TitleBarBase::~TitleBarBase()
{
}

void TitleBarBase::reset()
{
    mxToolBox.reset();
    mxAddonImage.reset();
}

void TitleBarBase::SetIcon(const css::uno::Reference<css::graphic::XGraphic>& rIcon)
{
    mxAddonImage->set_image(rIcon);
    mxAddonImage->set_visible(rIcon.is());
}

IMPL_LINK_NOARG(TitleBarBase, SelectionHandler, const OString&, void)
{
    HandleToolBoxItemClick();
}

TitleBar::TitleBar(vcl::Window* pParentWindow,
                   const OUString& rUIXMLDescription, const OString& rID,
                   Theme::ThemeItem eThemeItem)
    : InterimItemWindow(pParentWindow, rUIXMLDescription, rID)
    , TitleBarBase(*m_xBuilder, eThemeItem)
{
    m_xContainer->set_background(Theme::GetColor(meThemeItem));
}

TitleBar::~TitleBar()
{
    disposeOnce();
}

void TitleBar::dispose()
{
    reset();
    InterimItemWindow::dispose();
}

void TitleBar::DataChanged (const DataChangedEvent& /*rEvent*/)
{
    m_xContainer->set_background(Theme::GetColor(meThemeItem));
}

} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
