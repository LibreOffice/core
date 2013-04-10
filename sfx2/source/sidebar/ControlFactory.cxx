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

#include "precompiled_sfx2.hxx"

#include "sidebar/ControlFactory.hxx"

#include "MenuButton.hxx"
#include "TabItem.hxx"
#include "SidebarToolBox.hxx"
#include "ToolBoxBackground.hxx"
#include "CustomImageRadioButton.hxx"
#include <vcl/toolbox.hxx>


namespace sfx2 { namespace sidebar {


CheckBox* ControlFactory::CreateMenuButton (Window* pParentWindow)
{
    return new MenuButton(pParentWindow);
}




ImageRadioButton* ControlFactory::CreateTabItem (Window* pParentWindow)
{
    return new TabItem(pParentWindow);
}




ToolBox* ControlFactory::CreateToolBox (
    Window* pParentWindow,
    const ResId& rResId)
{
    SidebarToolBox* pToolBox = new SidebarToolBox(pParentWindow, rResId);
    pToolBox->SetBorderWindow(pParentWindow);

    pToolBox->Invalidate();

    return pToolBox;
}




Window* ControlFactory::CreateToolBoxBackground (
    Window* pParentWindow)
{
    ToolBoxBackground* pBorderWindow = new ToolBoxBackground(pParentWindow);
    pBorderWindow->Show();
    return pBorderWindow;
}




ImageRadioButton* ControlFactory::CreateCustomImageRadionButton(
    Window* pParentWindow,
    const ResId& rResId )
{
    return new CustomImageRadioButton(
        pParentWindow,
        rResId );
}

} } // end of namespace sfx2::sidebar
