/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef SD_TOOLPANEL_CONTROLS_CUSTOM_ANIMATION_PANEL_HXX
#define SD_TOOLPANEL_CONTROLS_CUSTOM_ANIMATION_PANEL_HXX

#include "taskpane/SubToolPanel.hxx"

namespace sd { namespace toolpanel {
class TreeNode;
class ControlFactory;
class ToolPanelViewShell;
} }

namespace sd { namespace toolpanel { namespace controls {

class CustomAnimationPanel
    : public SubToolPanel
{
public:
    CustomAnimationPanel (
        Window& i_rParentWindow,
        ToolPanelViewShell& i_rPanelViewShell);
    virtual ~CustomAnimationPanel (void);

    static std::auto_ptr<ControlFactory> CreateControlFactory (ToolPanelViewShell& i_rPanelViewShell);

    // TreeNode overridables
    virtual TaskPaneShellManager* GetShellManager();

    // ILayoutableWindow overridables
    virtual Size GetPreferredSize (void);
    virtual sal_Int32 GetPreferredWidth (sal_Int32 nHeigh);
    virtual sal_Int32 GetPreferredHeight (sal_Int32 nWidth);
    virtual ::Window* GetWindow (void);
    virtual bool IsResizable (void);
    virtual bool IsExpandable (void) const;

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > CreateAccessibleObject (
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible>& rxParent);

    using Window::GetWindow;
private:
    Size maPreferredSize;
    ::Window* mpWrappedControl;
    ToolPanelViewShell* m_pPanelViewShell;
};

} } } // end of namespace ::sd::toolpanel::controls

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
