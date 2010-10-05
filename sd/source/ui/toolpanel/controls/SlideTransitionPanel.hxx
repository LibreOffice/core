/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef SD_TOOLPANEL_CONTROLS_SLIDE_TRANSITION_PANEL_HXX
#define SD_TOOLPANEL_CONTROLS_SLIDE_TRANSITION_PANEL_HXX

#include "taskpane/SubToolPanel.hxx"

namespace sd {
class ViewShellBase;
}

namespace sd { namespace toolpanel {
class ControlFactory;
class TreeNode;
class ToolPanelViewShell;
} }

namespace sd { namespace toolpanel { namespace controls {

class SlideTransitionPanel
    : public SubToolPanel
{
public:
    SlideTransitionPanel (
        Window& i_rParentWindow,
        ToolPanelViewShell& i_rToolPanelShell);
    virtual ~SlideTransitionPanel (void);

    static std::auto_ptr<ControlFactory> CreateControlFactory (ToolPanelViewShell& i_rToolPanelShell);

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
