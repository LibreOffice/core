/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: MasterPagesPanel.hxx,v $
 * $Revision: 1.6 $
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

#ifndef SD_TASKPANE_CONTROLS_MASTER_PAGES_PANEL_HXX
#define SD_TASKPANE_CONTROLS_MASTER_PAGES_PANEL_HXX

#include "taskpane/ScrollPanel.hxx"

namespace sd {
class ViewShellBase;
}

namespace sd { namespace toolpanel {
class ControlFactory;
class TreeNode;
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
        TreeNode* pParent,
        ViewShellBase& rBase);
    virtual ~MasterPagesPanel (void);

    static std::auto_ptr<ControlFactory> CreateControlFactory (ViewShellBase& rBase);
};

} } } // end of namespace ::sd::toolpanel::controls

#endif
