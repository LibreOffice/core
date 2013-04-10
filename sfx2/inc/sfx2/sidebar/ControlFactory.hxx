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

#ifndef SFX_SIDEBAR_CONTROL_FACTORY_HXX
#define SFX_SIDEBAR_CONTROL_FACTORY_HXX

#include <sfx2/dllapi.h>
#include <vcl/button.hxx>

class ToolBox;

namespace sfx2 { namespace sidebar {

class ToolBoxBackground;


class SFX2_DLLPUBLIC ControlFactory
{
public:
    static CheckBox* CreateMenuButton (Window* pParentWindow);
    static ImageRadioButton* CreateTabItem (Window* pParentWindow);
    static ToolBox* CreateToolBox (Window* pParentWindow, const ResId& rResId);
    static Window* CreateToolBoxBackground (Window* pParentWindow);
    static ImageRadioButton* CreateCustomImageRadionButton(
        Window* pParentWindow,
        const ResId& rResId );
};


} } // end of namespace sfx2::sidebar

#endif
