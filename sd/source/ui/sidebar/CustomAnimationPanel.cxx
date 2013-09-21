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

#include "CustomAnimationPanel.hxx"

#include "ViewShellBase.hxx"


namespace sd {
    extern ::Window * createCustomAnimationPanel (::Window* pParent, ViewShellBase& rBase, const cssu::Reference<css::frame::XFrame>& rxFrame);
}

namespace sd { namespace sidebar {


CustomAnimationPanel::CustomAnimationPanel (
    ::Window* pParentWindow,
    ViewShellBase& rViewShellBase,
    const cssu::Reference<css::frame::XFrame>& rxFrame
    )
    : PanelBase(
        pParentWindow,
        rViewShellBase),
      mxFrame( rxFrame )
{
#ifdef DEBUG
    SetText(OUString("sd:CustomAnimationPanel"));
#endif
}




CustomAnimationPanel::~CustomAnimationPanel (void)
{
}




::Window* CustomAnimationPanel::CreateWrappedControl (
    ::Window* pParentWindow,
    ViewShellBase& rViewShellBase)
{
    return createCustomAnimationPanel(pParentWindow, rViewShellBase, mxFrame);
}




css::ui::LayoutSize CustomAnimationPanel::GetHeightForWidth (const sal_Int32 /*nWidth*/)
{
    Window *pControl = mpWrappedControl.get();
    sal_Int32 nMinimumHeight = pControl ? pControl->get_preferred_size().Height() : 0;
    return css::ui::LayoutSize(nMinimumHeight,-1, nMinimumHeight);
}


} } // end of namespace sd::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
