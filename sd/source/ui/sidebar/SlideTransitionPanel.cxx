/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#include "precompiled_sd.hxx"

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
    SetText(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sd:SlideTransitionPanel")));
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




css::ui::LayoutSize SlideTransitionPanel::GetHeightForWidth (const sal_Int32 nWidth)
{
    const sal_Int32 nMinimumHeight(getSlideTransitionPanelMinimumHeight(mpWrappedControl.get()));
    return css::ui::LayoutSize(nMinimumHeight,-1, nMinimumHeight);
}

} } // end of namespace sd::sidebar
