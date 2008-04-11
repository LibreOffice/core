/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TaskPaneViewShell.cxx,v $
 * $Revision: 1.20 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "TaskPaneViewShell.hxx"

#include "TaskPaneShellManager.hxx"
#include "ToolPanelChildWindow.hrc"
#include "ToolPanelChildWindow.hxx"
#include "taskpane/SlideSorterCacheDisplay.hxx"
#include "taskpane/ToolPanel.hxx"
#include "taskpane/TitledControl.hxx"
#include "LayoutMenu.hxx"
#include "TaskPaneFocusManager.hxx"
#include "taskpane/SubToolPanel.hxx"
#include "taskpane/ScrollPanel.hxx"
#include "taskpane/TaskPaneControlFactory.hxx"
#include "controls/MasterPagesPanel.hxx"
#include "controls/MasterPagesSelector.hxx"
#include "controls/TableDesignPanel.hxx"
#include "controls/CustomAnimationPanel.hxx"
#include "controls/SlideTransitionPanel.hxx"
#include "controls/AnimationSchemesPanel.hxx"
#include "TitleToolBox.hxx"
#include "taskpane/ControlContainer.hxx"
#include "FrameView.hxx"
#include "Window.hxx"
#include "PaneDockingWindow.hxx"
#include "AccessibleTaskPane.hxx"
#include "DrawSubController.hxx"
#include "sdmod.hxx"
#include "app.hrc"
#include "glob.hrc"
#include "res_bmp.hrc"
#include "helpids.h"
#include "strings.hrc"
#include "sdresid.hxx"
#include "framework/FrameworkHelper.hxx"
#include <com/sun/star/drawing/framework/XResourceId.hpp>
#include <com/sun/star/drawing/framework/ResourceActivationMode.hpp>
#include <com/sun/star/drawing/XDrawSubController.hpp>

#include <svx/dlgctrl.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/objface.hxx>
#include <svx/colrctrl.hxx>
#include <svx/xtable.hxx>
#include <vcl/dockwin.hxx>
#include "sdtreelb.hxx"
#include "DrawViewShell.hxx"
#include "drawdoc.hxx"
#include "ViewShellBase.hxx"
#include <svx/ruler.hxx>
#include <vcl/svapp.hxx>

//#define SHOW_TEST_PANEL
#ifdef SHOW_TEST_PANEL
#include "TestPanel.hxx"
#endif
//#define SHOW_COLOR_MENU
#ifdef SHOW_COLOR_MENU
#include "TestMenu.hxx"
#endif

#include <vector>
#include <boost/shared_ptr.hpp>

using namespace ::sd::toolpanel;

#define TaskPaneViewShell
#include "sdslots.hxx"


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::sd::framework::FrameworkHelper;


namespace sd { namespace toolpanel {

SFX_IMPL_INTERFACE(TaskPaneViewShell, SfxShell, SdResId(STR_TASKPANEVIEWSHELL))
{
}


TYPEINIT1(TaskPaneViewShell, ViewShell);


/** Inner implementation class of TaskPaneViewShell.
*/
class TaskPaneViewShell::Implementation
{
public:
    static const sal_uInt32 mnInvalidId = 0xffffffff;

    Implementation (void);
    ~Implementation (void);

    /** Here the panels are created that are shown in the task pane.
    */
    void Setup (ToolPanel* pToolPanel, ViewShellBase& rBase);

    /** Make a new panel known to the translation table that translates
        between internal indices as returned by
        ControlContainer::AddControl() and public indices defined by
        TaskPaneViewShell::PanelId.
    */
    void AddPanel (sal_uInt32 nInternalId, PanelId nPublicId);

    /** Return the public id for the given internal one.
        @return
            When the given public id is not known then PID_UNKNOWN is
            returned.
    */
    PanelId GetPublicId (sal_uInt32 nInternalId) const;

    /** Return the internal id for the given public one.
        @return
            When the given public id is not known then mnInvalidId is
            returned.
    */
    sal_uInt32 GetInternalId (PanelId nPublicId) const;

private:
    /** This map translates between internal indices returned by
        ControlContainer::AddControl() and public indices defined by
        TaskPaneViewShell::PanelId.
    */
    typedef ::std::vector<PanelId> InternalIdToPanelIdMap;
    InternalIdToPanelIdMap maIndexMap;
};




namespace {

enum MenuId {
    MID_UNLOCK_TASK_PANEL = 1,
    MID_LOCK_TASK_PANEL = 2,
    MID_CUSTOMIZE = 3,
    MID_FIRST_CONTROL = 4
};

/** This control is used for extracting the title string from the resource
    of some dialogs that are displayed in the task pane.  With this the
    actual controls can be created on demand.  This class only loads (a part
    of) their resource.
*/
class DummyControl : public Control
{
public:
    DummyControl (::Window* pParent, const ResId& rResId)
        : Control(pParent,rResId)
    {
        FreeResource();
    }
};

class ResourceActivationClickHandler
{
public:
    ResourceActivationClickHandler (
        const ::boost::shared_ptr<FrameworkHelper>& rpFrameworkHelper,
        const Reference<drawing::framework::XResourceId>& rxResourceId,
        ControlContainer& rControlContainer);
    void operator () (TitledControl& rTitledControl);

private:
    ::boost::shared_ptr<FrameworkHelper> mpFrameworkHelper;
    Reference<drawing::framework::XResourceId> mxResourceId;
    ControlContainer& mrControlContainer;
};

} // end of anonymouse namespace





void TaskPaneViewShell::Implementation::Setup (
    ToolPanel* pToolPanel,
    ViewShellBase& rBase)
{
    SdDrawDocument* pDocument = rBase.GetDocument();
    sal_uInt32 nId;
    sal_uInt32 nIdOfControlToExpand;

    ::boost::shared_ptr<FrameworkHelper> pFrameworkHelper (FrameworkHelper::Instance(rBase));
    Reference<drawing::framework::XResourceId> xTaskPaneId (pFrameworkHelper->CreateResourceId(
        FrameworkHelper::msTaskPaneURL, FrameworkHelper::msRightPaneURL));

    // The master page controls.
    nId = pToolPanel->AddControl (
        controls::MasterPagesPanel::CreateControlFactory(rBase),
        SdResId(STR_TASKPANEL_MASTER_PAGE_TITLE),
        HID_SD_SLIDE_DESIGNS,
        ResourceActivationClickHandler(
            pFrameworkHelper,
            pFrameworkHelper->CreateResourceId(
                FrameworkHelper::msMasterPagesTaskPanelURL, xTaskPaneId),
            pToolPanel->GetControlContainer()));
    AddPanel (nId, PID_MASTER_PAGES);

    // Layout Menu.
    nId = pToolPanel->AddControl (
        LayoutMenu::CreateControlFactory(rBase, *pDocument->GetDocSh()),
        SdResId(STR_TASKPANEL_LAYOUT_MENU_TITLE),
        HID_SD_SLIDE_LAYOUTS,
        ResourceActivationClickHandler(
            pFrameworkHelper,
            pFrameworkHelper->CreateResourceId(
                FrameworkHelper::msLayoutTaskPanelURL, xTaskPaneId),
            pToolPanel->GetControlContainer()));
    AddPanel (nId, PID_LAYOUT);
    nIdOfControlToExpand = nId;

    {
        DummyControl aControl (pToolPanel, SdResId(DLG_TABLEDESIGNPANE));

        // TableDesignPanel
        nId = pToolPanel->AddControl (
            controls::TableDesignPanel::CreateControlFactory(rBase),
            aControl.GetText(),
            HID_SD_TABLE_DESIGN,
            ResourceActivationClickHandler(
                pFrameworkHelper,
                    pFrameworkHelper->CreateResourceId(
                        FrameworkHelper::msTableDesignPanelURL, xTaskPaneId),
                pToolPanel->GetControlContainer()));
        AddPanel (nId, PID_TABLE_DESIGN);
    }

    {
        DummyControl aControl (pToolPanel, SdResId(RID_CUSTOMANIMATION_START+0));

        // CustomAnimationPanel
        nId = pToolPanel->AddControl (
            controls::CustomAnimationPanel::CreateControlFactory(rBase),
            aControl.GetText(),
            HID_SD_CUSTOM_ANIMATIONS,
            ResourceActivationClickHandler(
                pFrameworkHelper,
                    pFrameworkHelper->CreateResourceId(
                        FrameworkHelper::msCustomAnimationTaskPanelURL, xTaskPaneId),
                pToolPanel->GetControlContainer()));
        AddPanel (nId, PID_CUSTOM_ANIMATION);
    }

    // SlideTransitionPanel
    {
        DummyControl aControl (pToolPanel, SdResId(RID_CUSTOMANIMATION_START+3));

        nId = pToolPanel->AddControl (
            controls::SlideTransitionPanel::CreateControlFactory(rBase),
            aControl.GetText(),
            HID_SD_SLIDE_TRANSITIONS,
            ResourceActivationClickHandler(
                pFrameworkHelper,
                    pFrameworkHelper->CreateResourceId(
                        FrameworkHelper::msSlideTransitionTaskPanelURL, xTaskPaneId),
                pToolPanel->GetControlContainer()));
        AddPanel (nId, PID_SLIDE_TRANSITION);
    }

#ifdef SHOW_COLOR_MENU
    // Test Menu.
    pToolPanel->AddControl (
        ColorMenu::CreateControlFactory(),
        String::CreateFromAscii ("Color Test Menu"),
        0);
#endif

#ifdef SHOW_TEST_PANEL
    // Test Panel.
    pToolPanel->AddControl (
        TestPanel::CreateControlFactory(),
        String::CreateFromAscii ("Test Panel"),
        0);
#endif

#ifdef USE_SLIDE_SORTER_CACHE_DISPLAY
    pToolPanel->AddControl (
        SlideSorterCacheDisplay::CreateControlFactory(pDocument),
        String::CreateFromAscii("Slide Sorter Cache Status"),
        0);
#endif

    // Expand the layout menu.
    pToolPanel->GetControlContainer().SetExpansionState (
        nIdOfControlToExpand,
        ControlContainer::ES_EXPAND);

    pToolPanel->GetWindow()->Show();
}




void TaskPaneViewShell::Initialize (void)
{
    if ( ! mbIsInitialized)
    {
        mbIsInitialized = true;
        mpImpl->Setup (mpTaskPane.get(), GetViewShellBase());
    }
}




TaskPaneViewShell::TaskPaneViewShell (
    SfxViewFrame* pFrame,
    ViewShellBase& rViewShellBase,
    ::Window* pParentWindow,
    FrameView* pFrameViewArgument)
    : ViewShell (pFrame, pParentWindow, rViewShellBase),
      mpImpl(NULL),
      mpTaskPane(NULL),
      mbIsInitialized(false),
      mpSubShellManager(),
      mnMenuId(0)
{
    meShellType = ST_TASK_PANE;

    mpImpl.reset (new Implementation());

    mpContentWindow->SetCenterAllowed (false);

    mpTaskPane = ::std::auto_ptr<ToolPanel>(new ToolPanel (
        mpContentWindow.get(), *this));

    GetParentWindow()->SetBackground(Wallpaper());
    mpContentWindow->SetBackground(Wallpaper());

    GetParentWindow()->SetHelpId(HID_SD_TASK_PANE);

    PaneDockingWindow* pDockingWindow = dynamic_cast<PaneDockingWindow*>(GetDockingWindow());
    if (pDockingWindow != NULL)
    {
        pDockingWindow->InitializeTitleToolBox();
        mnMenuId = pDockingWindow->AddMenu (
            String(SdResId(STR_TASKPANEL_MASTER_PAGE_MENU_TITLE)),
            HID_SD_TASK_PANE_VIEW_MENU,
            LINK(this, TaskPaneViewShell, ToolboxClickHandler));
    }

    // Tell the focus manager that we want to pass the focus to our
    // child.
    FocusManager::Instance().RegisterDownLink(pParentWindow, mpTaskPane.get());

    SetPool (&GetDoc()->GetPool());

    if (pFrameViewArgument != NULL)
        mpFrameView = pFrameViewArgument;
    else
        mpFrameView = new FrameView(GetDoc());
    GetFrameView()->Connect();

    // Hide or delete unused controls that we have inherited from the
    // ViewShell base class.
    mpHorizontalScrollBar.reset();
    mpVerticalScrollBar.reset();
    mpScrollBarBox.reset();
    mpHorizontalRuler.reset();
    mpVerticalRuler.reset();

    SetName (String (RTL_CONSTASCII_USTRINGPARAM("TaskPaneViewShell")));

    // For accessibility we have to shortly hide the content window.  This
    // triggers the construction of a new accessibility object for the new
    // view shell.  (One is created earlier while the construtor of the base
    // class is executed.  At that time the correct accessibility object can
    // not be constructed.)
    if (mpContentWindow.get() !=NULL)
    {
        mpContentWindow->Hide();
        mpContentWindow->Show();
    }

    // Register the shell manager as factory at the ViewShellManager.
    mpSubShellManager.reset (new TaskPaneShellManager(
        GetViewShellBase().GetViewShellManager(),
        *this));
    GetViewShellBase().GetViewShellManager()->AddSubShellFactory(this, mpSubShellManager);
}




TaskPaneViewShell::~TaskPaneViewShell (void)
{
    GetViewShellBase().GetViewShellManager()->RemoveSubShellFactory(this, mpSubShellManager);
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

    Point aOrigin (maViewPos);
    Size aSize (maViewSize);

    Initialize();

    // Place the task pane.
    if (mpTaskPane.get() != NULL)
        mpTaskPane->SetPosSizePixel (Point(0,0), aSize);
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

        // pass toolbox button rect so the menu can stay open on button up
        Rectangle aRect = pToolBox->GetItemRect(mnMenuId);
        aRect.SetPos(pToolBox->GetPosPixel() );
        pMenu->Execute (pDockingWindow, aRect, POPUPMENU_EXECUTE_DOWN);
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

    // warning free code changes:
    // FLOATWIN_POPUPMODE_NOMOUSEUPCLOSE is 0x10000, so cast to USHORT is 0
    // so why was this used anyway?
    pMenu->SetMenuFlags (
        pMenu->GetMenuFlags() /* | (USHORT)FLOATWIN_POPUPMODE_NOMOUSEUPCLOSE */ );

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

SdPage* TaskPaneViewShell::getCurrentPage(void) const
{
    return NULL;
}



void TaskPaneViewShell::Execute (SfxRequest& )
{
}




void TaskPaneViewShell::GetState (SfxItemSet& rItemSet)
{
    (void)rItemSet;
}




TaskPaneShellManager& TaskPaneViewShell::GetSubShellManager (void) const
{
    return *mpSubShellManager.get();
}




DockingWindow* TaskPaneViewShell::GetDockingWindow (void)
{
    ::Window* pParentWindow = GetParentWindow();
    DockingWindow* pDockingWindow = NULL;
    while (pParentWindow!=NULL && pDockingWindow==NULL)
    {
        pDockingWindow = dynamic_cast<DockingWindow*>(pParentWindow);
        pParentWindow = pParentWindow->GetParent();
    }
    return pDockingWindow;
}




void TaskPaneViewShell::ShowPanel (const PanelId nPublicId)
{
    Initialize();
    sal_uInt32 nId (mpImpl->GetInternalId(nPublicId));
    if (nId != Implementation::mnInvalidId)
    {
        mpTaskPane->GetControlContainer().SetVisibilityState (
            nId,
            ControlContainer::VS_SHOW);
    }
}




void TaskPaneViewShell::HidePanel (const PanelId nPublicId)
{
    Initialize();
    sal_uInt32 nId (mpImpl->GetInternalId(nPublicId));
    if (nId != Implementation::mnInvalidId)
    {
        mpTaskPane->GetControlContainer().SetVisibilityState (
            nId,
            ControlContainer::VS_HIDE);
    }
}




void TaskPaneViewShell::ExpandPanel (const PanelId nPublicId)
{
   Initialize();
    sal_uInt32 nId (mpImpl->GetInternalId(nPublicId));
    if (nId != Implementation::mnInvalidId)
    {
        mpTaskPane->GetControlContainer().SetExpansionState (
            nId,
            ControlContainer::ES_EXPAND);
    }
}




void TaskPaneViewShell::CollapsePanel (const PanelId nPublicId)
{
    Initialize();
    sal_uInt32 nId (mpImpl->GetInternalId(nPublicId));
    if (nId != Implementation::mnInvalidId)
    {
        mpTaskPane->GetControlContainer().SetExpansionState (
            nId,
            ControlContainer::ES_COLLAPSE);
    }
}




::com::sun::star::uno::Reference<
    ::com::sun::star::accessibility::XAccessible>
    TaskPaneViewShell::CreateAccessibleDocumentView (::sd::Window* pWindow)
{
    ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible> xAccessible;

    if (mpTaskPane.get()!=NULL && pWindow!=NULL)
    {
        // We have to call CreateAccessible directly so that we can specify
        // the correct accessible parent.
        ::Window* pParentWindow = pWindow->GetAccessibleParentWindow();
        if (pParentWindow != NULL)
            xAccessible = mpTaskPane->CreateAccessibleObject(
                pParentWindow->GetAccessible());
    }

    return xAccessible;
}




Reference<drawing::XDrawSubController> TaskPaneViewShell::CreateSubController (void)
{
    // This view shell is not designed to be the main view shell and thus
    // does not support a UNO sub controller.
    return Reference<drawing::XDrawSubController>();
}




bool TaskPaneViewShell::RelocateToParentWindow (::Window* pParentWindow)
{
    ::Window* pOldParentWindow = GetParentWindow();
    FocusManager::Instance().RemoveLinks(pOldParentWindow, mpTaskPane.get());
    FocusManager::Instance().RemoveLinks(mpTaskPane.get(), pOldParentWindow);

    ViewShell::RelocateToParentWindow(pParentWindow);

    PaneDockingWindow* pDockingWindow = dynamic_cast<PaneDockingWindow*>(GetDockingWindow());
    if (pDockingWindow != NULL)
    {
        pDockingWindow->InitializeTitleToolBox();
        mnMenuId = pDockingWindow->AddMenu (
            String(SdResId(STR_TASKPANEL_MASTER_PAGE_MENU_TITLE)),
            HID_SD_TASK_PANE_VIEW_MENU,
            LINK(this, TaskPaneViewShell, ToolboxClickHandler));
    }
    FocusManager::Instance().RegisterDownLink(pParentWindow, mpTaskPane.get());

    Resize();

    return true;
}




//===== TaskPaneViewShell::Implementation =====================================

TaskPaneViewShell::Implementation::Implementation (void)
    : maIndexMap(
        (InternalIdToPanelIdMap::size_type)PID__END,
        PID_UNKNOWN)
{
}




TaskPaneViewShell::Implementation::~Implementation (void)
{
}




void TaskPaneViewShell::Implementation::AddPanel (
    sal_uInt32 nInternalId,
    PanelId nPublicId)
{
    maIndexMap[nInternalId] = nPublicId;
}




TaskPaneViewShell::PanelId
    TaskPaneViewShell::Implementation::GetPublicId (
        sal_uInt32 nInternalId) const
{
    if (nInternalId < maIndexMap.size())
        return maIndexMap[nInternalId];
    else
        return PID_UNKNOWN;
}




sal_uInt32
    TaskPaneViewShell::Implementation::GetInternalId (
        TaskPaneViewShell::PanelId nPublicId) const
{
    sal_uInt32 nId = mnInvalidId;
    for (sal_uInt32 nI=0; nI<maIndexMap.size(); nI++)
        if (maIndexMap[nI] == nPublicId)
        {
            nId = nI;
            break;
        }

    return nId;
}




//===== PanelActivation =======================================================

PanelActivation::PanelActivation (ViewShellBase& rBase, TaskPaneViewShell::PanelId nPanelId)
    : mrBase(rBase),
      mnPanelId(nPanelId)
{
}

void PanelActivation::operator() (bool)
{
    toolpanel::TaskPaneViewShell* pTaskPane
        = dynamic_cast<toolpanel::TaskPaneViewShell*>(
            framework::FrameworkHelper::Instance(mrBase)
            ->GetViewShell(framework::FrameworkHelper::msRightPaneURL).get());
    if (pTaskPane != NULL)
        pTaskPane->ShowPanel(mnPanelId);
}




//===== ResourceActivationClickHandler ========================================

ResourceActivationClickHandler::ResourceActivationClickHandler (
    const ::boost::shared_ptr<FrameworkHelper>& rpFrameworkHelper,
    const Reference<drawing::framework::XResourceId>& rxResourceId,
    ControlContainer& rControlContainer)
    : mpFrameworkHelper(rpFrameworkHelper),
      mxResourceId(rxResourceId),
      mrControlContainer(rControlContainer)
{
}




void ResourceActivationClickHandler::operator () (TitledControl& rTitledControl)
{
    mrControlContainer.SetExpansionState (
        &rTitledControl,
        ControlContainer::ES_EXPAND);
    mpFrameworkHelper->GetConfigurationController()->requestResourceActivation(
        mxResourceId, drawing::framework::ResourceActivationMode_REPLACE);
}


} } // end of namespace ::sd::toolpanel
