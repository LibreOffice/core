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
#ifndef INCLUDED_SFX2_SOURCE_SIDEBAR_PANELTITLEBAR_HXX
#define INCLUDED_SFX2_SOURCE_SIDEBAR_PANELTITLEBAR_HXX

#include <sfx2/sidebar/TitleBar.hxx>

#include <com/sun/star/frame/XFrame.hpp>
#include <boost/function.hpp>

namespace sfx2 { namespace sidebar {

class Panel;

class PanelTitleBar
    : public TitleBar
{
public:
    PanelTitleBar(const OUString& rsTitle, vcl::Window* pParentWindow, Panel* pPanel);
    virtual ~PanelTitleBar();
    virtual void dispose() SAL_OVERRIDE;

    void SetMoreOptionsCommand(const OUString& rsCommandName,
                               const css::uno::Reference<css::frame::XFrame>& rxFrame);

    virtual void DataChanged(const DataChangedEvent& rEvent) SAL_OVERRIDE;
    virtual void MouseButtonDown(const MouseEvent& rMouseEvent) SAL_OVERRIDE;
    virtual void MouseButtonUp(const MouseEvent& rMouseEvent) SAL_OVERRIDE;

protected:
    virtual Rectangle GetTitleArea(const Rectangle& rTitleBarBox) SAL_OVERRIDE;
    virtual void PaintDecoration(vcl::RenderContext& rRenderContext, const Rectangle& rTitleBarBox) SAL_OVERRIDE;
    virtual sidebar::Paint GetBackgroundPaint() SAL_OVERRIDE;
    virtual Color GetTextColor() SAL_OVERRIDE;
    virtual void HandleToolBoxItemClick (const sal_uInt16 nItemIndex) SAL_OVERRIDE;
    virtual css::uno::Reference<css::accessibility::XAccessible> CreateAccessible() SAL_OVERRIDE;

private:
    bool mbIsLeftButtonDown;
    VclPtr<Panel> mpPanel;
    const sal_uInt16 mnMenuItemIndex;
    css::uno::Reference<css::frame::XFrame> mxFrame;
    OUString msMoreOptionsCommand;
};

} } // end of namespace sfx2::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
