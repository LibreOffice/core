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

#ifndef SD_SIDEBAR_PANELS_TABLE_DESIGN_PANEL_HXX
#define SD_SIDEBAR_PANELS_TABLE_DESIGN_PANEL_HXX

#include "PanelBase.hxx"

namespace sd { namespace sidebar {

class TableDesignPanel
    : public PanelBase
{
public:
    TableDesignPanel (
        ::Window* pParentWindow,
        ViewShellBase& rViewShellBase);
    virtual ~TableDesignPanel (void);

    // ILayoutableWindow
    virtual css::ui::LayoutSize GetHeightForWidth (const sal_Int32 nWidth);

protected:
    virtual ::Window* CreateWrappedControl (
        ::Window* pParentWindow,
        ViewShellBase& rViewShellBase);
};

} } // end of namespace sd::sidebar

#endif
