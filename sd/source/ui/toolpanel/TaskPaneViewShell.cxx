/*************************************************************************
 *
 *  $RCSfile: TaskPaneViewShell.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:38:02 $
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

#include "TaskPaneViewShell.hxx"

#include "TaskPaneShellManager.hxx"
#include "ToolPanelChildWindow.hrc"
#include "ToolPanelChildWindow.hxx"
#include "ToolPanel.hxx"
#include "TitledControl.hxx"
#include "SubToolPanel.hxx"
#include "LayoutMenu.hxx"
#include "TaskPaneFocusManager.hxx"
#include "ScrollPanel.hxx"
#include "controls/MasterPagesPanel.hxx"
#include "controls/MasterPagesSelector.hxx"
#include "TitleToolBox.hxx"
#include "ControlContainer.hxx"
#include "FrameView.hxx"
#include "ObjectBarManager.hxx"
#include "Window.hxx"
#ifndef SD_PREVIEW_CHILD_WINDOW_HXX
#include "PreviewChildWindow.hxx"
#endif
#include "PaneDockingWindow.hxx"
#include "sdmod.hxx"
#include "app.hrc"
#include "glob.hrc"
#include "res_bmp.hrc"
#include "strings.hrc"
#include "sdresid.hxx"
#ifndef _SVX_DLG_CTRL_HXX
#include <svx/dlgctrl.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#include <sfx2/msg.hxx>
#include <sfx2/objface.hxx>
#ifndef _SVX_COLRCTRL_HXX
#include <svx/colrctrl.hxx>
#endif
#ifndef _XTABLE_HXX
#include <svx/xtable.hxx>
#endif
#include <vcl/dockwin.hxx>
#ifndef _SDTREELB_HXX
#include "sdtreelb.hxx"
#endif
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#include "drawdoc.hxx"
#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif
#ifndef _SVX_RULER_HXX //autogen
#include <svx/ruler.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

using namespace ::sd::toolpanel;
#define TaskPaneViewShell
#include "sdslots.hxx"



//#define SHOW_TEST_PANEL
#ifdef SHOW_TEST_PANEL
#include "TestPanel.hxx"
#endif
//#define SHOW_TEST_MENU
#ifdef SHOW_TEST_MENU
#include "TestMenu.hxx"
#endif

namespace sd { namespace toolpanel {

SFX_IMPL_INTERFACE(TaskPaneViewShell, SfxShell, SdResId(STR_TASKPANEVIEWSHELL))
{
}


TYPEINIT1(TaskPaneViewShell, ViewShell);


namespace {
    enum MenuId {
        MID_UNLOCK_TASK_PANEL = 1,
        MID_LOCK_TASK_PANEL = 2,
        MID_CUSTOMIZE = 3,
        MID_FIRST_CONTROL = 4,
    };





void TestSetup (ToolPanel* pToolPanel, ViewShellBase& rBase)
{
    SdDrawDocument* pDocument = rBase.GetDocument();

    // A sub tool panel with slide sorters.
    TreeNode* pSubPanel = new controls::MasterPagesPanel (pToolPanel, rBase);
    pToolPanel->AddControl (
        ::std::auto_ptr<TreeNode>(pSubPanel),
        SdResId(STR_TASKPANEL_MASTER_PAGE_TITLE));

    // Layout Menu.
    ScrollPanel* pScrollPanel = new ScrollPanel (pToolPanel);
    LayoutMenu* pMenu = new LayoutMenu (
        pScrollPanel,
        *pDocument->GetDocSh(),
        rBase,
        false);
    pMenu->Expand(true);
    pScrollPanel->AddControl (::std::auto_ptr<TreeNode>(pMenu));
    pToolPanel->AddControl (
        ::std::auto_ptr<TreeNode>(pScrollPanel),
        SdResId(STR_TASKPANEL_LAYOUT_MENU_TITLE));

#ifdef SHOW_COLOR_MENU
    // Test Menu.
    TestMenu* pTestMenu = new TestMenu (pToolPanel);
    pToolPanel->AddControl (
        ::std::auto_ptr<TreeNode>(pTestMenu),
        String::CreateFromAscii ("Color Test Menu"));
#endif

#ifdef SHOW_TEST_PANEL
    // Test Panel.
    TestPanel* pTestPanel = new TestPanel (pToolPanel);
    pToolPanel->AddControl (
        ::std::auto_ptr<TreeNode>(pTestPanel),
        String::CreateFromAscii ("Test Panel"));
#endif

    // Make the first control visible.
    pToolPanel->GetControlContainer().SetExpansionState (
        sal_uInt32(0),
        ControlContainer::ES_EXPAND);

    pToolPanel->GetWindow()->Show();
}



} // end of anonymouse namespace


TaskPaneViewShell::TaskPaneViewShell (
    SfxViewFrame* pFrame,
    ViewShellBase& rViewShellBase,
    ::Window* pParentWindow,
    FrameView* pFrameViewArgument)
    : ViewShell (pFrame, pParentWindow, rViewShellBase),
      mbIsInitialized (false),
      mpSubShellManager(NULL),
      mnMenuId(0)
{
    meShellType = ST_TASK_PANE;

    mpContentWindow->SetCenterAllowed (false);

    mpTaskPane = ::std::auto_ptr<ToolPanel>(new ToolPanel (
        mpContentWindow.get(), *this));

    GetParentWindow()->SetBackground(Wallpaper());
    mpContentWindow->SetBackground(Wallpaper());

    PaneDockingWindow* pDockingWindow = static_cast<PaneDockingWindow*>(
        pParentWindow);
    if (pDockingWindow != NULL)
    {
        mnMenuId = pDockingWindow->AddMenu (
            String(SdResId(STR_TASKPANEL_MASTER_PAGE_MENU_TITLE)),
            LINK(this, TaskPaneViewShell, ToolboxClickHandler));
    }

    // Tell the focus manager that we want to pass the focus to our
    // child.
    FocusManager::Instance().RegisterDownLink (
        pParentWindow,
        mpTaskPane.get());

    SetPool (&GetDoc()->GetPool());

    if (pFrameViewArgument != NULL)
        pFrameView = pFrameViewArgument;
    else
        pFrameView = new FrameView(GetDoc());
    GetFrameView()->Connect();

    // Hide or delete unused controls that we have inherited from the
    // ViewShell base class.
    mpHorizontalScrollBar.reset();
    mpVerticalScrollBar.reset();
    mpScrollBarBox.reset();
    mpHorizontalRuler.reset();
    mpVerticalRuler.reset();

    SetName (String (RTL_CONSTASCII_USTRINGPARAM("TaskPaneViewShell")));
}




TaskPaneViewShell::~TaskPaneViewShell (void)
{
}



// static
void TaskPaneViewShell::RegisterControls (void)
{
    SfxModule* pModule = SD_MOD();
    controls::MasterPagesSelector::RegisterInterface (pModule);
    LayoutMenu::RegisterInterface (pModule);
}




void TaskPaneViewShell::ArrangeGUIElements (void)
{
    ViewShell::ArrangeGUIElements();

    Point aOrigin (aViewPos);
    Size aSize (aViewSize);

    if ( ! mbIsInitialized)
    {
        mbIsInitialized = true;
        TestSetup (mpTaskPane.get(), GetViewShellBase());
    }

    // Place the task pane.
    if (mpTaskPane.get() != NULL)
        mpTaskPane->SetPosSizePixel (aOrigin, aSize);
}




void TaskPaneViewShell::GetFocus (void)
{
    Invalidate ();
}




void TaskPaneViewShell::LoseFocus (void)
{
    Invalidate ();
}




void TaskPaneViewShell::KeyInput (const KeyEvent& rEvent)
{
    KeyCode nCode = rEvent.GetKeyCode();
    if (nCode == KEY_RETURN)
    {
        mpTaskPane->GrabFocus();
    }
    else
        ViewShell::KeyInput (rEvent, NULL);
}




IMPL_LINK(TaskPaneViewShell, ToolboxClickHandler, ToolBox*, pToolBox)
{
    if (pToolBox->GetCurItemId() == mnMenuId)
    {
        pToolBox->EndSelection();

        DockingWindow* pDockingWindow = GetDockingWindow();
        ::std::auto_ptr<PopupMenu> pMenu = CreatePopupMenu (
            pDockingWindow!=NULL && !pDockingWindow->IsFloatingMode());
        pMenu->SetSelectHdl (
            LINK(this, TaskPaneViewShell, MenuSelectHandler));

        Point aMenuPosition = pToolBox->GetItemRect(mnMenuId).BottomLeft();
        aMenuPosition += pToolBox->GetPosPixel();
        pMenu->Execute (GetActiveWindow(), aMenuPosition);
    }

    return 0;
}




IMPL_LINK(TaskPaneViewShell, MenuSelectHandler, Menu*, pMenu)
{
    if (pMenu)
    {
        pMenu->Deactivate();
        switch (pMenu->GetCurItemId())
        {
            case MID_UNLOCK_TASK_PANEL:
            {
                DockingWindow* pDockingWindow = GetDockingWindow();
                if (pDockingWindow != NULL)
                    pDockingWindow->SetFloatingMode (TRUE);
            }
            break;

            case MID_LOCK_TASK_PANEL:
            {
                DockingWindow* pDockingWindow = GetDockingWindow();
                if (pDockingWindow != NULL)
                    pDockingWindow->SetFloatingMode (FALSE);
            }
            break;

            case MID_CUSTOMIZE:
                DBG_ASSERT (0, "Customization not yet implemented");
                break;

            default:
            {
                sal_uInt32 nIndex (pMenu->GetUserValue(pMenu->GetCurItemId()));
                mpTaskPane->GetControlContainer().SetVisibilityState (
                    nIndex,
                    ControlContainer::VS_TOGGLE);
                //                mpTaskPane->Resize();
                //                mpTaskPane->Invalidate();
            }
            break;
        }
    }

    return 0;
}




::std::auto_ptr<PopupMenu> TaskPaneViewShell::CreatePopupMenu (
    bool bIsDocking)
{
    ::std::auto_ptr<PopupMenu> pMenu (new PopupMenu ());
    FloatingWindow* pFloat = static_cast<FloatingWindow*>(pMenu->GetWindow());
    if (pFloat != NULL)
    {
        pFloat->SetPopupModeFlags (
            pFloat->GetPopupModeFlags()
            | FLOATWIN_POPUPMODE_NOMOUSEUPCLOSE);
    }
    pMenu->SetMenuFlags (
        pMenu->GetMenuFlags() | (USHORT)FLOATWIN_POPUPMODE_NOMOUSEUPCLOSE);

    // Add one entry for every tool panel element to individually make
    // them visible or hide them.
    USHORT nIndex = MID_FIRST_CONTROL;
    sal_uInt32 nControlIndex;
    ControlContainer& rContainer (mpTaskPane->GetControlContainer());
    for (nControlIndex=0;
         nControlIndex<rContainer.GetControlCount();
         nControlIndex=rContainer.GetNextIndex(nControlIndex,true,false))
    {
        TreeNode* pChild = rContainer.GetControl(nControlIndex);
        TitledControl* pControl
            = static_cast<TitledControl*>(pChild->GetWindow());
        pMenu->InsertItem (nIndex,
            pControl->GetTitle(),
            MIB_CHECKABLE);
        pMenu->SetUserValue (nIndex, nControlIndex);
        if (pControl->IsVisible())
            pMenu->CheckItem (nIndex, TRUE);
        nIndex++;
    }

    pMenu->InsertSeparator ();

    // Add entry for docking or un-docking the tool panel.
    if (bIsDocking)
        pMenu->InsertItem (
            MID_UNLOCK_TASK_PANEL,
            String(SdResId(STR_TASKPANEL_MASTER_PAGE_MENU_UNLOCK)));
    else
        pMenu->InsertItem (
            MID_LOCK_TASK_PANEL,
            String(SdResId(STR_TASKPANEL_MASTER_PAGE_MENU_LOCK)));
    pMenu->RemoveDisabledEntries (FALSE, FALSE);

    return pMenu;
}




SdPage* TaskPaneViewShell::GetActualPage (void)
{
    return NULL;
}




void TaskPaneViewShell::Execute (SfxRequest& rRequest)
{
}




void TaskPaneViewShell::GetState (SfxItemSet& rItemSet)
{
    (void)rItemSet;
}




void TaskPaneViewShell::GetLowerShellList (
    ::std::vector<SfxShell*>& rShellList) const
{
    GetObjectBarManager().GetLowerShellList (rShellList);
    GetSubShellManager().GetLowerShellList (rShellList);
}




void TaskPaneViewShell::GetUpperShellList (
    ::std::vector<SfxShell*>& rShellList) const
{
    GetObjectBarManager().GetUpperShellList (rShellList);
    GetSubShellManager().GetUpperShellList (rShellList);
}



TaskPaneShellManager& TaskPaneViewShell::GetSubShellManager (void) const
{
    if (mpSubShellManager.get() == NULL)
        mpSubShellManager.reset (new TaskPaneShellManager(
            GetViewShellBase().GetViewShellManager(),
            *this));
    return *mpSubShellManager.get();
}




DockingWindow* TaskPaneViewShell::GetDockingWindow (void)
{
    ::Window* pParentWindow = GetParentWindow();
    DockingWindow* pDockingWindow = NULL;
    while (pParentWindow!=NULL && pDockingWindow==NULL)
    {
        pDockingWindow = static_cast<DockingWindow*>(pParentWindow);
        pParentWindow = pParentWindow->GetParent();
    }
    return pDockingWindow;
}


} } // end of namespace ::sd::toolpanel
