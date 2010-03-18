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

#ifndef SD_PANE_DOCKING_WINDOW_HXX
#define SD_PANE_DOCKING_WINDOW_HXX

#include <sfx2/dockwin.hxx>
#include <sfx2/viewfrm.hxx>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

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
        const ::rtl::OUString& rsPaneURL,
        const ::rtl::OUString& rsTitle);

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

    ::Window* GetContentWindow (void);

    ::boost::shared_ptr<ToolBox> GetTitleToolBox (void) const;

private:
    /** The pane which is represented by the docking window.
    */
    ::rtl::OUString msPaneURL;

    /** Title that is shown at the top of the docking window.
    */
    ::rtl::OUString msTitle;

    /** The tool box that is displayed in the window title area contains
        menus and the closer button.
    */
    ::boost::shared_ptr<ToolBox> mpTitleToolBox;

    /** The border that is painted arround the inner window.  The bevel
        shadow lines are part of the border, so where the border is 0 no
        such line is painted.
    */
    SvBorder maBorder;

    sal_uInt16 mnChildWindowId;

    ::boost::scoped_ptr< ::Window> mpContentWindow;

    /** Remember that a layout is pending, i.e. Resize() has been called
        since the last Paint().
    */
    bool mbIsLayoutPending;

    DECL_LINK(ToolboxSelectHandler, ToolBox*);

    /** This does the actual placing and sizing of the title bar and the
        content window after the size of the docking window has changed.
        This method is called from withing the Paint() method when since its
        last invocation the size of the docking window has changed.
    */
    void Layout (void);
};

} // end of namespace ::sd

#endif
