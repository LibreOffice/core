/*************************************************************************
 *
 *  $RCSfile: PaneDockingWindow.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 13:58:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SD_PANE_DOCKING_WINDOW_HXX
#define SD_PANE_DOCKING_WINDOW_HXX

#ifndef _SFXDOCKWIN_HXX
#include <sfx2/dockwin.hxx>
#endif
#include "PaneManager.hxx"

class ToolBox;

namespace sd {

class PaneDockingWindow
    : public SfxDockingWindow
{
public:
    PaneDockingWindow (
        SfxBindings *pBindings,
        SfxChildWindow *pChildWindow,
        ::Window* pParent,
        const ResId& rResId,
        PaneManager::PaneType eType,
        const String& rsTitle);

    virtual ~PaneDockingWindow (void);

    virtual void Paint (const Rectangle& rRectangle);
    virtual void Resize (void);

    virtual long    Notify( NotifyEvent& rNEvt );
    virtual void    StateChanged( StateChangedType nType );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    /** Add a top down menu to the title bar or rather the top-level menu
        entry.  The given callback is called when the menu is clicked and it
        is the task of the callback to show the menu.
        @return
            The id of the new menu is returned.  It can be compared by the
            callback to the value of GetCurItemId() when called at the given
            tool box.
    */
    USHORT AddMenu (const String& rsMenuName, const Link& rCallback);

private:
    /** The pane which is represented by the docking window.
    */
    PaneManager::PaneType mePane;

    /** Title that is shown at the top of the docking window.
    */
    String msTitle;

    /** The tool box that is displayed in the window title area contains
        menus and the closer button.
    */
    ::std::auto_ptr<ToolBox> mpTitleToolBox;

    /** The border that is painted arround the inner window.  The bevel
        shadow lines are part of the border, so where the border is 0 no
        such line is painted.
    */
    SvBorder maBorder;

    sal_uInt16 mnChildWindowId;

    DECL_LINK(ToolboxSelectHandler, ToolBox*);
};

} // end of namespace ::sd

#endif
