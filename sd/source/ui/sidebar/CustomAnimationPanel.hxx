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
#ifndef INCLUDED_SD_SOURCE_UI_SIDEBAR_CUSTOMANIMATIONPANEL_HXX
#define INCLUDED_SD_SOURCE_UI_SIDEBAR_CUSTOMANIMATIONPANEL_HXX

#include "PanelBase.hxx"

namespace com { namespace sun { namespace star { namespace frame { class XFrame; } } } }

namespace sd { namespace sidebar {

class CustomAnimationPanel
    : public PanelBase
{
public:
    CustomAnimationPanel (
        vcl::Window* pParentWindow,
        ViewShellBase& rViewShellBase,
        const css::uno::Reference<css::frame::XFrame>& rxFrame);
    virtual ~CustomAnimationPanel() override;

    // ILayoutableWindow
    virtual css::ui::LayoutSize GetHeightForWidth (const sal_Int32 nWidth) override;

protected:
    virtual vcl::Window* CreateWrappedControl (
        vcl::Window* pParentWindow,
        ViewShellBase& rViewShellBase) override;

private:
    css::uno::Reference<css::frame::XFrame>     mxFrame;
};

} } // end of namespace sd::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
