/*************************************************************************
 *
 *  $RCSfile: TaskPaneViewShell.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2005-07-14 10:13:26 $
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

#ifndef SD_TOOLPANEL_TASK_PANE_VIEW_SHELL_HXX
#define SD_TOOLPANEL_TASK_PANE_VIEW_SHELL_HXX

#include "ViewShell.hxx"
#include "glob.hxx"
#include <vcl/button.hxx>
#ifndef _SFX_SHELL_HXX
#include <sfx2/shell.hxx>
#endif
#ifndef _VIEWFAC_HXX
#include <sfx2/viewfac.hxx>
#endif

#ifndef _SFXDOCKWIN_HXX
#include <sfx2/dockwin.hxx>
#endif
#include <memory>

class PopupMenu;

namespace sd {  namespace toolpanel {

class TaskPaneShellManager;
class TitleToolBox;
class TitleBar;
class TitledControl;
class ToolPanel;

/** The tool panel is a view shell for some very specific reasons:
    - It fits better into the concept of panes being docking windows whose
    content, a view shell, can be exchanged on runtime.
    - A control in the tool panel that wants to show a context menu has to
    do that over the dispatcher of a shell.  These shells, usually
    implemented by the controls themselves, have to be managed by someone.
    If interpreted as object bars this can be handled by the
    ObjectBarManager of the ViewShell.
*/
class TaskPaneViewShell
    : public ViewShell
{
public:
    TYPEINFO();
    SFX_DECL_INTERFACE(SD_IF_SDTASKPANEVIEWSHELL);

    /** List of top level panels that can be shown in the task pane.
    */
    enum PanelId
    {
        PID__START = 0,
        PID_UNKNOWN = PID__START,
        PID_MASTER_PAGES,
        PID_LAYOUT,
        PID_ANIMATION_SCHEMES,
        PID_CUSTOM_ANIMATION,
        PID_SLIDE_TRANSITION,
        PID__END = PID_SLIDE_TRANSITION
    };

    TaskPaneViewShell (
        SfxViewFrame* pFrame,
        ViewShellBase& rViewShellBase,
        ::Window* pParentWindow,
        FrameView* pFrameView);
    virtual ~TaskPaneViewShell (void);

    /** Register the SFX interfaces so that (some of) the controls can be
        pushed as SFX shells on the shell stack and process slot calls and
        so on.
    */
    static void RegisterControls (void);

    virtual void GetFocus (void);
    virtual void LoseFocus (void);
    virtual void KeyInput (const KeyEvent& rEvent);

    virtual SdPage* GetActualPage (void);
    virtual SdPage* getCurrentPage (void) const;

    void Execute (SfxRequest& rRequest);
    void GetState (SfxItemSet& rItemSet);

    virtual void ArrangeGUIElements (void);

    virtual void GetLowerShellList (
        ::std::vector<SfxShell*>& rShellList) const;
    virtual void GetUpperShellList (
        ::std::vector<SfxShell*>& rShellList) const;

    TaskPaneShellManager& GetSubShellManager (void) const;

    /** Called when a mouse button has been pressed but not yet
        released, this handler is used to show the popup menu of the
        title bar.
    */
    DECL_LINK(ToolboxClickHandler, ToolBox*);
    DECL_LINK(MenuSelectHandler, Menu*);

    /** Make the specified panel visible.
        @param nId
            The id of the panel that is to be made visible.
    */
    void ShowPanel (PanelId nId);

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible>
        CreateAccessibleDocumentView (::sd::Window* pWindow);

private:
    class Implementation;
    ::std::auto_ptr<Implementation> mpImpl;

    ::std::auto_ptr<ToolPanel> mpTaskPane;

    // Control that displays the closer symbol in the title bar.
    ::std::auto_ptr<TitleToolBox> mpTitleToolBox;

    bool mbIsInitialized;

    mutable ::std::auto_ptr<TaskPaneShellManager> mpSubShellManager;

    /** The id of the menu in the menu bar/tool box of the parent docking
        window.
    */
    USHORT mnMenuId;

    /** Create a popup menu.  it contains three sections, one for
        docking or un-docking the tool panel, one for toggling the
        visibility state of the tool panel items, and one for bringing
        up a customization dialog.
        @param bIsDocking
            According to this flag one of the lock/unlock entries is
            made disabled.
    */
    ::std::auto_ptr<PopupMenu> CreatePopupMenu (bool bIsDocking);


    /** Make sure that as long as there is at least one visible
        control there is exactly one expanded control.
        If the currently expanded control is being hidden then try to
        expand the control after the hidden one or if that does not
        exist expand the one before.
    */
    void EnsureExpandedControl (TitledControl* pHiddenControl);

    /** Return a pointer to the docking window that is the parent or a
        predecessor of the content window.
        @return
            When the view shell is not placed in a docking window, e.g. when
            shown in the center pane, then <NULL?> is returned.
    */
    DockingWindow* GetDockingWindow (void);
};


} } // end of namespace ::sd::toolpanel

#endif
