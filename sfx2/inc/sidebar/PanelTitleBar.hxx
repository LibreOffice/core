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

#include <sidebar/TitleBar.hxx>

namespace com::sun::star::frame { class XController; }
namespace com::sun::star::frame { class XFrame; }

namespace sfx2::sidebar {

class Panel;

class PanelTitleBar final
    : public TitleBar
{
public:
    PanelTitleBar(const OUString& rsTitle, vcl::Window* pParentWindow, Panel* pPanel);
    virtual ~PanelTitleBar() override;
    virtual void dispose() override;

    void SetMoreOptionsCommand(const OUString& rsCommandName,
                               const css::uno::Reference<css::frame::XFrame>& rxFrame,
                               const css::uno::Reference<css::frame::XController>& rxController);

    virtual void DataChanged(const DataChangedEvent& rEvent) override;
    virtual void MouseButtonDown(const MouseEvent& rMouseEvent) override;
    virtual void MouseButtonUp(const MouseEvent& rMouseEvent) override;

private:
    virtual tools::Rectangle GetTitleArea(const tools::Rectangle& rTitleBarBox) override;
    virtual void PaintDecoration(vcl::RenderContext& rRenderContext) override;
    virtual Color GetBackgroundPaintColor() override;
    virtual void HandleToolBoxItemClick (const sal_uInt16 nItemIndex) override;
    virtual css::uno::Reference<css::accessibility::XAccessible> CreateAccessible() override;

    bool mbIsLeftButtonDown;
    VclPtr<Panel> mpPanel;
    static const sal_uInt16 mnMenuItemIndex = 1;
    css::uno::Reference<css::frame::XFrame> mxFrame;
    OUString msMoreOptionsCommand;
};

} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
