/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <sal/config.h>

#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include "StylesPropertyPanel.hxx"

using namespace css;
using namespace css::uno;

namespace svx::sidebar {

std::unique_ptr<PanelLayout> StylesPropertyPanel::Create (
    weld::Widget* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to StylesPropertyPanel::Create", nullptr, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to StylesPropertyPanel::Create", nullptr, 1);

    return std::make_unique<StylesPropertyPanel>(pParent, rxFrame);
}

StylesPropertyPanel::StylesPropertyPanel(weld::Widget* pParent, const css::uno::Reference<css::frame::XFrame>& rxFrame)
    : PanelLayout(pParent, "SidebarStylesPanel", "svx/ui/sidebarstylespanel.ui")
    , m_xFontStyle(m_xBuilder->weld_toolbar("fontstyletoolbox"))
    , m_xFontStyleDispatch(new ToolbarUnoDispatcher(*m_xFontStyle, *m_xBuilder, rxFrame))
    , m_xStyle(m_xBuilder->weld_toolbar("style"))
    , m_xStyleDispatch(new ToolbarUnoDispatcher(*m_xStyle, *m_xBuilder, rxFrame))
{
}

StylesPropertyPanel::~StylesPropertyPanel()
{
    m_xStyleDispatch.reset();
    m_xStyle.reset();
    m_xFontStyleDispatch.reset();
    m_xFontStyle.reset();
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
