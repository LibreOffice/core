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



#ifndef SD_TASKPANE_TEST_PANEL_HXX
#define SD_TASKPANE_TEST_PANEL_HXX

#include "taskpane/SubToolPanel.hxx"


namespace sd { namespace toolpanel {

class ControlFactory;
class TreeNode;

#ifdef SHOW_TEST_PANEL

/** This panel demonstrates how to create a panel for the task pane.
*/
class TestPanel
    : public SubToolPanel
{
public:
    TestPanel (::Window& i_rParent);
    virtual ~TestPanel (void);

    static std::auto_ptr<ControlFactory> CreateControlFactory (void);
};

#endif

} } // end of namespace ::sd::toolpanel

#endif
