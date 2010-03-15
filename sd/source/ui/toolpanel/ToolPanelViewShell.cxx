/*************************************************************************
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "taskpane/ToolPanelViewShell.hxx"

#include "TaskPaneShellManager.hxx"
#include "TaskPaneFocusManager.hxx"
#include "controls/MasterPagesPanel.hxx"
#include "LayoutMenu.hxx"
#include "controls/TableDesignPanel.hxx"
#include "controls/CustomAnimationPanel.hxx"
#include "controls/SlideTransitionPanel.hxx"
#include "controls/MasterPagesSelector.hxx"
#include "PaneDockingWindow.hxx"
#include "FrameView.hxx"
#include "Window.hxx"
#include "PaneDockingWindow.hxx"
#include "ToolPanelDeck.hxx"
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
#include <sfx2/imagemgr.hxx>
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
#include <vcl/toolbox.hxx>
#include <tools/diagnose_ex.h>

#include <vector>

using namespace ::sd::toolpanel;

#define ToolPanelViewShell
#include "sdslots.hxx"

/** === begin UNO using === **/
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::UNO_SET_THROW;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::makeAny;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Type;
using ::com::sun::star::accessibility::XAccessible;
using ::com::sun::star::drawing::XDrawSubController;
using ::com::sun::star::frame::XFrame;
/** === end UNO using === **/

using ::sd::framework::FrameworkHelper;

namespace sd { namespace toolpanel {

// =====================================================================================================================
// = ToolPanelViewShell::Implementation - declaration
// =====================================================================================================================
/** Inner implementation class of ToolPanelViewShell.
*/
class ToolPanelViewShell::Implementation
{
public:
    static const size_t mnInvalidId = static_cast< size_t >( -1 );

    Implementation();
    ~Implementation();

    /** Here the panels are created that are shown in the task pane.
    */
    void Setup( ToolPanelViewShell& i_rViewShell, ToolPanelDeck& i_rPanelDeck );

    /** Return the public id for the given internal one.
        @return
            When the given public id is not known then PID_UNKNOWN is
            returned.
    */
    PanelId GetPublicId (size_t nInternalId) const;

    /** Return the internal id for the given public one.
        @return
            When the given public id is not known then mnInvalidId is
            returned.
    */
    size_t GetInternalId (PanelId nPublicId) const;

private:
    /** Make a new panel known to the translation table that translates
        between internal indices as returned by
        ControlContainer::AddControl() and public indices defined by
        ToolPanelViewShell::PanelId.
    */
    void AddPanel (size_t nInternalId, PanelId nPublicId);

    /** This map translates between internal indices returned by
        ControlContainer::AddControl() and public indices defined by
        ToolPanelViewShell::PanelId.
    */
    typedef ::std::vector<PanelId> InternalIdToPanelIdMap;
    InternalIdToPanelIdMap maIndexMap;
};

// =====================================================================================================================
// = helpers
// =====================================================================================================================
// ---------------------------------------------------------------------------------------------------------------------
namespace {

enum MenuId {
    MID_UNLOCK_TASK_PANEL = 1,
    MID_LOCK_TASK_PANEL = 2,
    MID_FIRST_CONTROL = 3
};

} // end of anonymouse namespace

// =====================================================================================================================
// = ToolPanelViewShell
// =====================================================================================================================
// ---------------------------------------------------------------------------------------------------------------------
SFX_IMPL_INTERFACE(ToolPanelViewShell, SfxShell, SdResId(STR_TASKPANEVIEWSHELL))
{
}

// ---------------------------------------------------------------------------------------------------------------------
TYPEINIT1(ToolPanelViewShell, ViewShell);

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell::Implementation::Setup( ToolPanelViewShell& i_rViewShell, ToolPanelDeck& i_rPanelDeck )
{
    typedef std::auto_ptr<ControlFactory> (*ControlFactoryFactory)( ToolPanelViewShell& i_rToolPanelShell );

    struct PanelDescriptor
    {
        ControlFactoryFactory   pFactory;
        const sal_Char*         pImageCommandName;
        USHORT                  nTitleResourceID;
        ULONG                   nHelpID;
        PanelId                 nPanelID;
    };

    PanelDescriptor aPanels[] = {
        // "Master Pages"
        { &controls::MasterPagesPanel::CreateControlFactory, "PresentationLayout", STR_TASKPANEL_MASTER_PAGE_TITLE, HID_SD_SLIDE_DESIGNS, PID_MASTER_PAGES },
        // "Layout"
        { &LayoutMenu::CreateControlFactory, "ModifyPage", STR_TASKPANEL_LAYOUT_MENU_TITLE, HID_SD_SLIDE_LAYOUTS, PID_LAYOUT },
        // "Tables Design"
        { &controls::TableDesignPanel::CreateControlFactory, "InsertTable", DLG_TABLEDESIGNPANE, HID_SD_TABLE_DESIGN, PID_TABLE_DESIGN },
        // "Custom Animation"
        { &controls::CustomAnimationPanel::CreateControlFactory, "CustomAnimation", STR_CUSTOMANIMATIONPANE, HID_SD_CUSTOM_ANIMATIONS, PID_CUSTOM_ANIMATION },
        // "Slide Transition"
        { &controls::SlideTransitionPanel::CreateControlFactory, "RehearseTimings", STR_SLIDE_TRANSITION_PANE, HID_SD_SLIDE_TRANSITIONS, PID_SLIDE_TRANSITION }
    };

    Reference< XFrame > xFrame( i_rViewShell.GetViewShellBase().GetViewFrame()->GetFrame()->GetFrameInterface() );
    for ( size_t i=0; i < sizeof( aPanels ) / sizeof( aPanels[0] ); ++i )
    {
        ::rtl::OUStringBuffer aCommandName;
        aCommandName.appendAscii( ".uno:" );
        aCommandName.appendAscii( aPanels[i].pImageCommandName );

        size_t nPanelPos = i_rPanelDeck.CreateAndInsertPanel(
            (*aPanels[i].pFactory)( i_rViewShell ),
            GetImage( xFrame, aCommandName.makeStringAndClear(), FALSE, FALSE ),
            aPanels[i].nTitleResourceID,
            aPanels[i].nHelpID
        );
        AddPanel( nPanelPos, aPanels[i].nPanelID );
    }

    // activate default panel
    i_rPanelDeck.ActivatePanel( GetInternalId( PID_LAYOUT ) );
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell::Initialize()
{
    if ( !mbIsInitialized )
    {
        mbIsInitialized = true;
        mpImpl->Setup( *this, *mpPanelDeck );
        mpPanelDeck->Show();
    }
}

// ---------------------------------------------------------------------------------------------------------------------
ToolPanelViewShell::ToolPanelViewShell( SfxViewFrame* pFrame, ViewShellBase& rViewShellBase, ::Window* pParentWindow,
        FrameView* pFrameViewArgument )
    :ViewShell(pFrame, pParentWindow, rViewShellBase)
    ,mpImpl( new Implementation )
    ,mpPanelDeck( new ToolPanelDeck( *mpContentWindow.get(), *this ) )
    ,mbIsInitialized(false)
    ,mpSubShellManager()
    ,mnMenuId(0)
{
    meShellType = ST_TOOL_PANEL;

    mpContentWindow->SetCenterAllowed( false );
    pParentWindow->SetStyle( pParentWindow->GetStyle() | WB_DIALOGCONTROL );

    GetParentWindow()->SetBackground( Wallpaper() );
    mpContentWindow->SetBackground( Wallpaper() );

    GetParentWindow()->SetHelpId(HID_SD_TASK_PANE);

    ConnectToDockingWindow();

    SetPool( &GetDoc()->GetPool() );

    if ( pFrameViewArgument )
        mpFrameView = pFrameViewArgument;
    else
        mpFrameView = new FrameView( GetDoc() );
    GetFrameView()->Connect();

    // Hide or delete unused controls that we have inherited from the
    // ViewShell base class.
    mpHorizontalScrollBar.reset();
    mpVerticalScrollBar.reset();
    mpScrollBarBox.reset();
    mpHorizontalRuler.reset();
    mpVerticalRuler.reset();

    SetName( String( RTL_CONSTASCII_USTRINGPARAM( "ToolPanelViewShell" ) ) );

    // For accessibility we have to shortly hide the content window.  This
    // triggers the construction of a new accessibility object for the new
    // view shell.  (One is created earlier while the construtor of the base
    // class is executed.  At that time the correct accessibility object can
    // not be constructed.)
    if ( mpContentWindow.get() )
    {
        mpContentWindow->Hide();
        mpContentWindow->Show();
    }

    // Register the shell manager as factory at the ViewShellManager.
    mpSubShellManager.reset( new TaskPaneShellManager(
        GetViewShellBase().GetViewShellManager(),
        *this
    ) );
    GetViewShellBase().GetViewShellManager()->AddSubShellFactory( this, mpSubShellManager );
}

// ---------------------------------------------------------------------------------------------------------------------
ToolPanelViewShell::~ToolPanelViewShell()
{
    mpPanelDeck.reset();
    GetViewShellBase().GetViewShellManager()->RemoveSubShellFactory(this, mpSubShellManager);
}

// ---------------------------------------------------------------------------------------------------------------------
// static
void ToolPanelViewShell::RegisterControls()
{
    SfxModule* pModule = SD_MOD();
    controls::MasterPagesSelector::RegisterInterface( pModule );
    LayoutMenu::RegisterInterface( pModule );
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell::ArrangeGUIElements()
{
    ViewShell::ArrangeGUIElements();

    Initialize();

    mpPanelDeck->SetPosSizePixel( Point(), maViewSize );
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell::GetFocus()
{
    Invalidate();
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell::LoseFocus()
{
    Invalidate();
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell::KeyInput( const KeyEvent& i_rKeyEvent )
{
    const KeyCode nCode = i_rKeyEvent.GetKeyCode();
    if ( nCode == KEY_RETURN )
    {
        if ( !mpPanelDeck->HasChildPathFocus() )
            mpPanelDeck->GrabFocus();
    }
    else
        ViewShell::KeyInput( i_rKeyEvent, NULL );
}

// ---------------------------------------------------------------------------------------------------------------------
IMPL_LINK( ToolPanelViewShell, DockingChanged, PaneDockingWindow*, i_pDockingWindow )
{
    ENSURE_OR_RETURN( i_pDockingWindow == dynamic_cast< PaneDockingWindow* >( GetDockingWindow() ), "unknown source", 0 );
    switch ( i_pDockingWindow->GetAlignment() )
    {
    case SFX_ALIGN_LEFT:
        mpPanelDeck->SetTabsLayout( ::svt::TABS_LEFT, ::svt::TABITEM_IMAGE_ONLY );
        break;
    case SFX_ALIGN_RIGHT:
    case SFX_ALIGN_NOALIGNMENT:
        mpPanelDeck->SetTabsLayout( ::svt::TABS_RIGHT, ::svt::TABITEM_IMAGE_ONLY );
        break;
    case SFX_ALIGN_TOP:
        mpPanelDeck->SetTabsLayout( ::svt::TABS_TOP, ::svt::TABITEM_IMAGE_ONLY );
        break;
    case SFX_ALIGN_BOTTOM:
        mpPanelDeck->SetTabsLayout( ::svt::TABS_BOTTOM, ::svt::TABITEM_IMAGE_ONLY );
        break;
    default:
        OSL_ENSURE( false, "ToolPanelViewShell::DockingChanged: unexpected alignment!" );
        break;
    }
    return 0L;
}

// ---------------------------------------------------------------------------------------------------------------------
IMPL_LINK(ToolPanelViewShell, ToolboxClickHandler, ToolBox*, pToolBox)
{
    if (pToolBox->GetCurItemId() == mnMenuId)
    {
        pToolBox->EndSelection();

        DockingWindow* pDockingWindow = GetDockingWindow();
        ::std::auto_ptr<PopupMenu> pMenu = CreatePopupMenu (
            pDockingWindow!=NULL && !pDockingWindow->IsFloatingMode());
        pMenu->SetSelectHdl (
            LINK(this, ToolPanelViewShell, MenuSelectHandler));

        // pass toolbox button rect so the menu can stay open on button up
        Rectangle aRect = pToolBox->GetItemRect(mnMenuId);
        aRect.SetPos(pToolBox->GetPosPixel() );
        pMenu->Execute (pDockingWindow, aRect, POPUPMENU_EXECUTE_DOWN);
    }

    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
IMPL_LINK(ToolPanelViewShell, MenuSelectHandler, Menu*, pMenu)
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

            default:
//            {
//                sal_uInt32 nIndex (pMenu->GetUserValue(pMenu->GetCurItemId()));
//                mpTaskPane->GetControlContainer().SetVisibilityState (
//                    nIndex,
//                    ControlContainer::VS_TOGGLE);
//                //                mpTaskPane->Resize();
//                //                mpTaskPane->Invalidate();
//            }
            break;
        }
    }

    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
::std::auto_ptr<PopupMenu> ToolPanelViewShell::CreatePopupMenu (
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

    // Add one entry for every tool panel element to individually make
    // them visible or hide them.
//    USHORT nIndex = MID_FIRST_CONTROL;
//    sal_uInt32 nControlIndex;
//    ControlContainer& rContainer (mpTaskPane->GetControlContainer());
//    for (nControlIndex=0;
//         nControlIndex<rContainer.GetControlCount();
//         nControlIndex=rContainer.GetNextIndex(nControlIndex,true,false))
//    {
//        TreeNode* pChild = rContainer.GetControl(nControlIndex);
//        TitledControl* pControl
//            = static_cast<TitledControl*>(pChild->GetWindow());
//        pMenu->InsertItem (nIndex,
//            pControl->GetTitle(),
//            MIB_CHECKABLE);
//        pMenu->SetUserValue (nIndex, nControlIndex);
//        if (pControl->IsVisible())
//            pMenu->CheckItem (nIndex, TRUE);
//        nIndex++;
//    }

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

// ---------------------------------------------------------------------------------------------------------------------
SdPage* ToolPanelViewShell::GetActualPage()
{
    return NULL;
}

// ---------------------------------------------------------------------------------------------------------------------
SdPage* ToolPanelViewShell::getCurrentPage() const
{
    return NULL;
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell::Execute( SfxRequest& )
{
    OSL_ENSURE( false, "ToolPanelViewShell::Execute: not to be called! (right?)" );
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell::GetState( SfxItemSet& )
{
    OSL_ENSURE( false, "ToolPanelViewShell::GetState: not to be called! (right?)" );
}

// ---------------------------------------------------------------------------------------------------------------------
TaskPaneShellManager& ToolPanelViewShell::GetSubShellManager() const
{
    return *mpSubShellManager.get();
}

// ---------------------------------------------------------------------------------------------------------------------
DockingWindow* ToolPanelViewShell::GetDockingWindow()
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

// ---------------------------------------------------------------------------------------------------------------------
Reference< XAccessible > ToolPanelViewShell::CreateAccessibleDocumentView( ::sd::Window* pWindow )
{
    ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible> xAccessible;

    OSL_ENSURE( false, "ToolPanelViewShell::CreateAccessibleDocumentView: missing implementation!" );
//    if (mpTaskPane.get()!=NULL && pWindow!=NULL)
//    {
//        // We have to call CreateAccessible directly so that we can specify
//        // the correct accessible parent.
//        ::Window* pParentWindow = pWindow->GetAccessibleParentWindow();
//        if (pParentWindow != NULL)
//            xAccessible = mpTaskPane->CreateAccessibleObject(
//                pParentWindow->GetAccessible());
//    }

    return xAccessible;
}

// ---------------------------------------------------------------------------------------------------------------------
Reference< XDrawSubController > ToolPanelViewShell::CreateSubController()
{
    // This view shell is not designed to be the main view shell and thus
    // does not support a UNO sub controller.
    return Reference< XDrawSubController >();
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell::ConnectToDockingWindow()
{
    PaneDockingWindow* pDockingWindow = dynamic_cast<PaneDockingWindow*>(GetDockingWindow());
    if (pDockingWindow != NULL)
    {
        pDockingWindow->InitializeTitleToolBox();
        mnMenuId = pDockingWindow->AddMenu (
            String(SdResId(STR_TASKPANEL_MASTER_PAGE_MENU_TITLE)),
            HID_SD_TASK_PANE_VIEW_MENU,
            LINK(this, ToolPanelViewShell, ToolboxClickHandler));

        // be notified when the docking position of the window changes
        pDockingWindow->SetEndDockingHdl( LINK( this, ToolPanelViewShell, DockingChanged ) );
    }

    // Tell the focus manager that we want to pass the focus to our
    // child.
    FocusManager::Instance().RegisterDownLink( GetParentWindow(), mpPanelDeck.get() );
}

// ---------------------------------------------------------------------------------------------------------------------
bool ToolPanelViewShell::RelocateToParentWindow( ::Window* pParentWindow )
{
    ::Window* pOldParentWindow = GetParentWindow();
    FocusManager::Instance().RemoveLinks( pOldParentWindow, mpPanelDeck.get() );
    FocusManager::Instance().RemoveLinks( mpPanelDeck.get(), pOldParentWindow );

    PaneDockingWindow* pDockingWindow = dynamic_cast< PaneDockingWindow* >( GetDockingWindow() );
    if ( pDockingWindow != NULL )
    {
        pDockingWindow->SetEndDockingHdl( Link() );
    }

    ViewShell::RelocateToParentWindow(pParentWindow);

    ConnectToDockingWindow();

    Resize();

    return true;
}




// =====================================================================================================================
// = ToolPanelViewShell:Implementation - implementation
// =====================================================================================================================
// ---------------------------------------------------------------------------------------------------------------------
ToolPanelViewShell::Implementation::Implementation()
    :maIndexMap( (InternalIdToPanelIdMap::size_type)PID__END, PID_UNKNOWN )
{
}

// ---------------------------------------------------------------------------------------------------------------------
ToolPanelViewShell::Implementation::~Implementation()
{
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell::Implementation::AddPanel( size_t nInternalId, PanelId nPublicId )
{
    maIndexMap[nInternalId] = nPublicId;
}

// ---------------------------------------------------------------------------------------------------------------------
ToolPanelViewShell::PanelId ToolPanelViewShell::Implementation::GetPublicId ( size_t nInternalId ) const
{
    if ( nInternalId < maIndexMap.size() )
        return maIndexMap[nInternalId];
    else
        return PID_UNKNOWN;
}

// ---------------------------------------------------------------------------------------------------------------------
size_t ToolPanelViewShell::Implementation::GetInternalId( ToolPanelViewShell::PanelId nPublicId ) const
{
    for ( size_t i = 0; i < maIndexMap.size(); ++i )
    {
        if ( maIndexMap[i] == nPublicId )
        {
            return i;
        }
    }

    return mnInvalidId;
}

} } // end of namespace ::sd::toolpanel
