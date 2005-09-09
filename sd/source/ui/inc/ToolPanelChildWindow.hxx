/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ToolPanelChildWindow.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:18:09 $
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

#ifndef SD_TOOL_PANEL_CHILD_WINDOW_HXX
#define SD_TOOL_PANEL_CHILD_WINDOW_HXX

#ifndef _SFX_CHILDWIN_HXX
#include <sfx2/childwin.hxx>
#endif

#define TOOLPANEL_CHILD_WINDOW() (                                      \
    static_cast< ::sd::toolpanel::ToolPanelChildWindow*>(               \
        SfxViewFrame::Current()->GetChildWindow(                        \
            ::sd::toolpanel::ToolPanelChildWindow::GetChildWindowId()   \
            )->GetWindow()))


namespace sd { namespace toolpanel {

class ToolPanelChildWindow
    : public SfxChildWindow
{
public:
    ToolPanelChildWindow (::Window*, USHORT, SfxBindings*, SfxChildWinInfo*);
    virtual ~ToolPanelChildWindow (void);

    SFX_DECL_CHILDWINDOW (ToolPanelChildWindow);
};


} } // end of namespaces ::sd::toolpanel

#endif
