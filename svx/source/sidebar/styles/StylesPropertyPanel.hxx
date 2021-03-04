/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <sfx2/sidebar/PanelLayout.hxx>
#include <sfx2/weldutils.hxx>

namespace svx::sidebar{

class StylesPropertyPanel : public PanelLayout
{
private:
    std::unique_ptr<weld::Toolbar> m_xFontStyle;
    std::unique_ptr<ToolbarUnoDispatcher> m_xFontStyleDispatch;

    std::unique_ptr<weld::Toolbar> m_xStyle;
    std::unique_ptr<ToolbarUnoDispatcher> m_xStyleDispatch;

public:
    virtual ~StylesPropertyPanel() override;

    static std::unique_ptr<PanelLayout> Create (
        weld::Widget* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame);

    StylesPropertyPanel(
        weld::Widget* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame);
};

} //end of namespace svx::sidebar
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
