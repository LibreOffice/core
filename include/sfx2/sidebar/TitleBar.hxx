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
#ifndef INCLUDED_SFX2_SOURCE_SIDEBAR_TITLEBAR_HXX
#define INCLUDED_SFX2_SOURCE_SIDEBAR_TITLEBAR_HXX

#include <sfx2/sidebar/Paint.hxx>

#include <vcl/fixed.hxx>
#include <sfx2/sidebar/SidebarToolBox.hxx>

namespace sfx2 { namespace sidebar {

class TitleBar : public vcl::Window
{
public:
    TitleBar (const OUString& rsTitle,
              vcl::Window* pParentWindow,
              const sidebar::Paint& rInitialBackgroundPaint);
    virtual ~TitleBar() override;
    virtual void dispose() override;

    void SetTitle (const OUString& rsTitle);
    const OUString& GetTitle() {return msTitle; }

    void SetIcon (const Image& rIcon);

    virtual void ApplySettings(vcl::RenderContext& rRenderContext) override;
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rUpdateArea) override;
    virtual void DataChanged (const DataChangedEvent& rEvent) override;
    virtual void setPosSizePixel (long nX, long nY, long nWidth, long nHeight, PosSizeFlags nFlags = PosSizeFlags::All) override;

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
    virtual void PaintDecoration (vcl::RenderContext& rRenderContext, const tools::Rectangle& rTitleBarBox) = 0;
    void PaintFocus(vcl::RenderContext& rRenderContext, const tools::Rectangle& rFocusBox);
    virtual sidebar::Paint GetBackgroundPaint() = 0;
    virtual void HandleToolBoxItemClick (const sal_uInt16 nItemIndex);
    virtual css::uno::Reference<css::accessibility::XAccessible> CreateAccessible() override;

private:
    Image maIcon;
    sidebar::Paint maBackgroundPaint;

    void PaintTitle(vcl::RenderContext& rRenderContext, const tools::Rectangle& rTitleBox);
    DECL_LINK(SelectionHandler, ToolBox*, void);
};

} } // end of namespace sfx2::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
