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



#ifndef SD_TASKPANE_CONTROLS_MASTER_PAGES_PANEL_HXX
#define SD_TASKPANE_CONTROLS_MASTER_PAGES_PANEL_HXX

#include "taskpane/ScrollPanel.hxx"

namespace sd {
class ViewShellBase;
}

namespace sd { namespace toolpanel {
class ControlFactory;
class TreeNode;
class ToolPanelViewShell;
} }

namespace sd { namespace toolpanel { namespace controls {

/** The master pages panel combines three master page related panels into
    one.  This has the benefit that creation of the task pane becomes a
    little bit simpler and that common scroll bars can be displayed.
*/
class MasterPagesPanel
    : public ScrollPanel
{
public:
    MasterPagesPanel (
        ::Window& i_rParentWindow,
        ToolPanelViewShell& i_rPanelViewShell);
    virtual ~MasterPagesPanel (void);

    // TreeNode overridables
    virtual TaskPaneShellManager* GetShellManager (void);

    static std::auto_ptr<ControlFactory> CreateControlFactory (ToolPanelViewShell& i_rToolPanelShell);

private:
    void    impl_construct( ViewShellBase& rBase );

private:
    ToolPanelViewShell* m_pPanelViewShell;
};

} } } // end of namespace ::sd::toolpanel::controls

#endif
