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

#include "TableDesignPanel.hxx"

#include "ViewShellBase.hxx"


namespace sd {
    extern ::Window * createTableDesignPanel (::Window* pParent, ViewShellBase& rBase);
}


namespace sd { namespace sidebar {


TableDesignPanel::TableDesignPanel (
    ::Window* pParentWindow,
    ViewShellBase& rViewShellBase)
    : PanelBase(pParentWindow, rViewShellBase)
{
#ifdef DEBUG
    SetText(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sd:TableDesignPanel")));
#endif
}




TableDesignPanel::~TableDesignPanel (void)
{
}




::Window* TableDesignPanel::CreateWrappedControl (
    ::Window* pParentWindow,
    ViewShellBase& rViewShellBase)
{
    return createTableDesignPanel(pParentWindow, rViewShellBase);
}




css::ui::LayoutSize TableDesignPanel::GetHeightForWidth (const sal_Int32 nWidth)
{
    //TODO: make the sizes depend on the font size.
    return css::ui::LayoutSize(350,-1, 400);
}

} } // end of namespace sd::sidebar
