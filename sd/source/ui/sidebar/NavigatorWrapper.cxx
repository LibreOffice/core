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
#include "navigatr.hrc"
#include "ViewShellBase.hxx"

#include <sfx2/sidebar/Theme.hxx>

#include <boost/bind.hpp>


namespace sd { namespace sidebar {

NavigatorWrapper::NavigatorWrapper (
    ::Window* pParent,
    sd::ViewShellBase& rViewShellBase,
    SfxBindings* pBindings)
    : Control(pParent, 0),
      mrViewShellBase(rViewShellBase),
      maNavigator(
        this,
        NULL,
        SdResId(FLT_NAVIGATOR),
        pBindings,
        ::boost::bind(&NavigatorWrapper::UpdateNavigator, this))
{
    maNavigator.SetPosSizePixel(
        Point(0,0),
        GetSizePixel());
    maNavigator.SetBackground(sfx2::sidebar::Theme::GetWallpaper(sfx2::sidebar::Theme::Paint_PanelBackground));
    maNavigator.Show();
}




NavigatorWrapper::~NavigatorWrapper (void)
{
}




void NavigatorWrapper::Resize (void)
{
    maNavigator.SetSizePixel(GetSizePixel());
}




css::ui::LayoutSize NavigatorWrapper::GetHeightForWidth (const sal_Int32 nWidth)
{
    (void)nWidth;

    return css::ui::LayoutSize(-1,-1,-1);
}




void NavigatorWrapper::UpdateNavigator (void)
{
    maNavigator.InitTreeLB(mrViewShellBase.GetDocument());
}




void NavigatorWrapper::GetFocus (void)
{
    maNavigator.GrabFocus();
}



} } // end of namespace sd::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
