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

#include "NavigatorWrapper.hxx"
#include <ViewShellBase.hxx>

#include <sfx2/sidebar/Theme.hxx>
#include <navigatr.hxx>


namespace sd::sidebar {

NavigatorWrapper::NavigatorWrapper (
    vcl::Window* pParent,
    sd::ViewShellBase& rViewShellBase,
    SfxBindings* pBindings)
    : SdNavigatorWin(pParent, pBindings)
    , mrViewShellBase(rViewShellBase)
{
    SetUpdateRequestFunctor(
            [this] () { return this->UpdateNavigator(); });
    SetBackground(sfx2::sidebar::Theme::GetColor(sfx2::sidebar::Theme::Color_PanelBackground));
    Show();
}

css::ui::LayoutSize NavigatorWrapper::GetHeightForWidth (const sal_Int32)
{
    return css::ui::LayoutSize(-1,-1,-1);
}

void NavigatorWrapper::UpdateNavigator()
{
    InitTreeLB(mrViewShellBase.GetDocument());
}

} // end of namespace sd::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
