/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MasterPagesPanel.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 06:40:54 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
