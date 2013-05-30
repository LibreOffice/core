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

#include "SlideTransitionPanel.hxx"

#include "ViewShellBase.hxx"


namespace sd {
    extern ::Window* createSlideTransitionPanel (::Window* pParent, ViewShellBase& rBase);
    extern sal_Int32 getSlideTransitionPanelMinimumHeight (::Window* pParent);
}




namespace sd { namespace sidebar {


SlideTransitionPanel::SlideTransitionPanel (
    ::Window* pParentWindow,
    ViewShellBase& rViewShellBase)
    : PanelBase(pParentWindow, rViewShellBase)
{
#ifdef DEBUG
    SetText(OUString("sd:SlideTransitionPanel"));
#endif
}




SlideTransitionPanel::~SlideTransitionPanel (void)
{
}




::Window* SlideTransitionPanel::CreateWrappedControl (
    ::Window* pParentWindow,
    ViewShellBase& rViewShellBase)
{
    return createSlideTransitionPanel(pParentWindow, rViewShellBase);
}




css::ui::LayoutSize SlideTransitionPanel::GetHeightForWidth (const sal_Int32 /*nWidth*/)
{
    const sal_Int32 nMinimumHeight(getSlideTransitionPanelMinimumHeight(mpWrappedControl.get()));
    return css::ui::LayoutSize(nMinimumHeight,-1, nMinimumHeight);
}

} } // end of namespace sd::sidebar
