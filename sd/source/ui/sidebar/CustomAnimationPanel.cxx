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

#include "CustomAnimationPanel.hxx"

#include "ViewShellBase.hxx"


namespace sd {
    extern ::Window * createCustomAnimationPanel (::Window* pParent, ViewShellBase& rBase);
    extern sal_Int32 getCustomAnimationPanelMinimumHeight (::Window* pParent);
}




namespace sd { namespace sidebar {


CustomAnimationPanel::CustomAnimationPanel (
    ::Window* pParentWindow,
    ViewShellBase& rViewShellBase)
    : PanelBase(
        pParentWindow,
        rViewShellBase)
{
#ifdef DEBUG
    SetText(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sd:CustomAnimationPanel")));
#endif
}




CustomAnimationPanel::~CustomAnimationPanel (void)
{
}




::Window* CustomAnimationPanel::CreateWrappedControl (
    ::Window* pParentWindow,
    ViewShellBase& rViewShellBase)
{
    return createCustomAnimationPanel(pParentWindow, rViewShellBase);
}




css::ui::LayoutSize CustomAnimationPanel::GetHeightForWidth (const sal_Int32 nWidth)
{
    const sal_Int32 nMinimumHeight(getCustomAnimationPanelMinimumHeight(mpWrappedControl.get()));
    return css::ui::LayoutSize(nMinimumHeight,-1, nMinimumHeight);
}


} } // end of namespace sd::sidebar
