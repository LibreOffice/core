/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PaneDockingWindow.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2005-10-24 16:17:35 $
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

#ifndef SD_PANE_DOCKING_WINDOW_HXX
#define SD_PANE_DOCKING_WINDOW_HXX

#ifndef _SFXDOCKWIN_HXX
#include <sfx2/dockwin.hxx>
#endif
#include <sfx2/viewfrm.hxx>
#include "PaneManager.hxx"

class ToolBox;

namespace sd {

class PaneDockingWindow
    : public SfxDockingWindow
{
public:
    /** Create a new docking window that will be displayed in the specified
        pane.
        The constructor will determine the ViewShellBase via the given
        bindings and tell its PaneManager about the new window.  It will ask
        the PaneManager for a window title.
        @param pBindings
            Used, among others, to determine the ViewShellBase and
            PaneManager that manage the new docking window.
        @param pChildWindow
            This child window is the logical container for the new docking
            window.
        @param pParent
            The parent window of the new docking window.
        @param rResId
            The resource is used to determine initial size and attributes.
        @param ePane
            The pane in which to show the docking window.
    */
    PaneDockingWindow (
        SfxBindings *pBindings,
        SfxChildWindow *pChildWindow,
        ::Window* pParent,
        const ResId& rResId,
        PaneManager::PaneType ePane,
        const String& rsTitle);

    virtual ~PaneDockingWindow (void);

    virtual void Paint (const Rectangle& rRectangle);
    virtual void Resize (void);

    virtual long Notify( NotifyEvent& rNEvt );
    virtual void StateChanged( StateChangedType nType );
    virtual void DataChanged( const DataChangedEvent& rDCEvt );

    /** Initializing the title tool box either creates a new title tool box
        or clears all items from an existing one.  A closer is added as only
        item.
    */
    void InitializeTitleToolBox (void);

    /** Add a top down menu to the title bar or rather the top-level menu
        entry.  The given callback is called when the menu is clicked and it
        is the task of the callback to show the menu.
        @return
            The id of the new menu is returned.  It can be compared by the
            callback to the value of GetCurItemId() when called at the given
            tool box.
    */
    USHORT AddMenu (const String& rsMenuName, ULONG nHelpId, const Link& rCallback);

    /** Set the title of the docking window to the given string.  Use this
        method when the title is not yet known at the time of construction
        or can not be passed to the constructor.
    */
    void SetTitle (const String& rsTitle);

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
    DECL_LINK(WindowEventListener, VclSimpleEvent*);
};

} // end of namespace ::sd

#endif
