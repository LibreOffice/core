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

namespace sfx2::sidebar {

class TitleBar : public vcl::Window
{
public:
    TitleBar (const OUString& rsTitle,
              vcl::Window* pParentWindow,
              const Color& rInitialBackgroundColor);
    virtual ~TitleBar() override;
    virtual void dispose() override;

    void SetTitle (const OUString& rsTitle);
    const OUString& GetTitle() const {return msTitle; }

    void SetIcon(const css::uno::Reference<css::graphic::XGraphic>& rIcon);

    virtual void ApplySettings(vcl::RenderContext& rRenderContext) override;
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rUpdateArea) override;
    virtual void DataChanged (const DataChangedEvent& rEvent) override;
    virtual void setPosSizePixel (tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight, PosSizeFlags nFlags = PosSizeFlags::All) override;

    ToolBox& GetToolBox()
    {
        return *maToolBox;
    }
    const ToolBox& GetToolBox() const
    {
        return *maToolBox;
    }

protected:
    VclPtr<SidebarToolBox> maToolBox;
    OUString msTitle;

    virtual tools::Rectangle GetTitleArea (const tools::Rectangle& rTitleBarBox) = 0;
    virtual void PaintDecoration (vcl::RenderContext& rRenderContext) = 0;
    void PaintFocus(vcl::RenderContext& rRenderContext, const tools::Rectangle& rFocusBox);
    virtual Color GetBackgroundPaintColor() = 0;
    virtual void HandleToolBoxItemClick (const sal_uInt16 nItemIndex);
    virtual css::uno::Reference<css::accessibility::XAccessible> CreateAccessible() override;

private:
    Image maIcon;
    Color maBackgroundColor;

    void PaintTitle(vcl::RenderContext& rRenderContext, const tools::Rectangle& rTitleBox);
    DECL_LINK(SelectionHandler, ToolBox*, void);
};

} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
