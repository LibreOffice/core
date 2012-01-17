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


#ifndef SD_TASKPANE_CONTROLS_ANIMATION_SCHEMES_PANEL_HXX
#define SD_TASKPANE_CONTROLS_ANIMATION_SCHEMES_PANEL_HXX

#include "taskpane/SubToolPanel.hxx"

namespace sd {
class ViewShellBase;
}

namespace sd { namespace toolpanel {
class TreeNode;
} }

namespace sd { namespace toolpanel { namespace controls {

class AnimationSchemesPanel
    : public SubToolPanel
{
public:
    AnimationSchemesPanel (
        TreeNode* pParent,
        ViewShellBase& rBase);
    virtual ~AnimationSchemesPanel (void);

    virtual Size GetPreferredSize (void);
    virtual sal_Int32 GetPreferredWidth (sal_Int32 nHeigh);
    virtual sal_Int32 GetPreferredHeight (sal_Int32 nWidth);
    virtual ::Window* GetWindow (void);
    virtual bool IsResizable (void);
    virtual bool IsExpandable (void) const;

    using Window::GetWindow;

private:
    Size maPreferredSize;
    ::Window* mpWrappedControl;
};

} } } // end of namespace ::sd::toolpanel::controls

#endif
#ifndef SD_TASKPANE_CONTROLS_ANIMATION_SCHEMES_PANEL_HXX
#define SD_TASKPANE_CONTROLS_ANIMATION_SCHEMES_PANEL_HXX

#include "taskpane/SubToolPanel.hxx"

namespace sd {
class ViewShellBase;
}

namespace sd { namespace toolpanel {
class TreeNode;
} }

namespace sd { namespace toolpanel { namespace controls {

class AnimationSchemesPanel
    : public SubToolPanel
{
public:
    AnimationSchemesPanel (
        TreeNode* pParent,
        ViewShellBase& rBase);
    virtual ~AnimationSchemesPanel (void);

    virtual Size GetPreferredSize (void);
    virtual sal_Int32 GetPreferredWidth (sal_Int32 nHeigh);
    virtual sal_Int32 GetPreferredHeight (sal_Int32 nWidth);
    virtual ::Window* GetWindow (void);
    virtual bool IsResizable (void);
    virtual bool IsExpandable (void) const;

    using Window::GetWindow;

private:
    Size maPreferredSize;
    ::Window* mpWrappedControl;
};

} } } // end of namespace ::sd::toolpanel::controls

#endif
