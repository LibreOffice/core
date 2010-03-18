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
#include "StandardToolPanel.hxx"
#include "CustomToolPanel.hxx"
#include "controls/MasterPagesPanel.hxx"
#include "LayoutMenu.hxx"
#include "controls/TableDesignPanel.hxx"
#include "controls/CustomAnimationPanel.hxx"
#include "controls/SlideTransitionPanel.hxx"
#include "controls/MasterPagesSelector.hxx"
#include "PaneDockingWindow.hxx"
#include "FrameView.hxx"
#include "Window.hxx"
#include "ToolPanelDeck.hxx"
#include "sdmod.hxx"
#include "app.hrc"
#include "glob.hrc"
#include "res_bmp.hrc"
#include "helpids.h"
#include "strings.hrc"
#include "sdresid.hxx"
#include "framework/FrameworkHelper.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/drawing/framework/XResourceId.hpp>
#include <com/sun/star/drawing/framework/ResourceActivationMode.hpp>
#include <com/sun/star/drawing/XDrawSubController.hpp>
/** === end UNO includes === **/

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
#include <unotools/confignode.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/componentcontext.hxx>

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
using ::com::sun::star::drawing::framework::XResourceId;
/** === end UNO using === **/

using ::sd::framework::FrameworkHelper;

namespace sd { namespace toolpanel {

// =====================================================================================================================
// = PanelDescriptor
// =====================================================================================================================
/** is a helper class for ToolPanelViewShell::Implementation, holding the details about a single panel which is not
    contained in the IToolPanel implementation itself.
*/
struct PanelDescriptor
{
    PanelId             nId;
    ::svt::PToolPanel   pPanel;
    bool                bHidden;

    PanelDescriptor()
        :nId( PID_UNKNOWN )
        ,pPanel()
        ,bHidden( false )
    {
    }

    PanelDescriptor( const PanelId i_nPanelId, const ::svt::PToolPanel& i_rPanel )
        :nId( i_nPanelId )
        ,pPanel( i_rPanel )
        ,bHidden( false )
    {
    }
};

// =====================================================================================================================
// = PanelSelectorLayout
// =====================================================================================================================
enum PanelSelectorLayout
{
    LAYOUT_DRAWERS,
    LAYOUT_TABS_RIGHT,
    LAYOUT_TABS_LEFT,
    LAYOUT_TABS_TOP,
    LAYOUT_TABS_BOTTOM
};

// =====================================================================================================================
// = ToolPanelViewShell::Implementation - declaration
// =====================================================================================================================
/** Inner implementation class of ToolPanelViewShell.
*/
class ToolPanelViewShell::Implementation : public ::svt::IToolPanelDeckListener
{
public:
    static const size_t mnInvalidId = static_cast< size_t >( -1 );

    Implementation( ToolPanelViewShell& i_rPanelViewShell, ::Window& i_rPanelDeckParent );
    ~Implementation();

    /** Here the panels are created that are shown in the task pane.
    */
    void Setup();

    /** clean up the instance
    */
    void Cleanup();

    /** sets the given layout for the panel selector
    */
    void    SetLayout( const PanelSelectorLayout i_eLayout, const bool i_bForce = false );

    /** returns the current layout
    */
    PanelSelectorLayout
            GetLayout() const { return m_eCurrentLayout; }

    /** provides access to the the VCL window of the panel deck
    */
          ::Window& GetPanelDeck()       { return *m_pPanelDeck.get(); }
    const ::Window& GetPanelDeck() const { return *m_pPanelDeck.get(); }

    /** returns the logical number of panels. This is greater than or equal to the number of panels displayed in the
        panel deck
    */
    size_t GetPanelCount() const
    {
        return m_aPanels.size();
    }

    const PanelDescriptor& GetPanel( const size_t i_nLogicalPanelIndex ) const
    {
        return m_aPanels[ i_nLogicalPanelIndex ];
    }

    void    TogglePanelVisibility( const size_t i_nLogicalPanelIndex );

    /** ensures the panel with the given ID is visible, and directly activates it, bypassing the configuration controller
    */
    void    ActivatePanelDirectly( const PanelId i_nPanelId );

    /** de-activates the panel given by ID, bypassing the configuration controller

        If the panel is not active currently, nothing happens.
    */
    void    DeactivatePanelDirectly( const PanelId i_nPanelId );

protected:
    // IToolPanelDeckListener overridables
    virtual void PanelInserted( const ::svt::PToolPanel& i_pPanel, const size_t i_nPosition );
    virtual void PanelRemoved( const size_t i_nPosition );
    virtual void ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive );
    virtual void Dying();

private:
    void RegisterPanel( size_t i_nPosition, PanelId i_nPanelId, const ::svt::PToolPanel& i_rPanel );
    void UpdateDockingWindowTitle();

    /** creates the built-in panels, returns the position of the panel to activate initially
    */
    size_t SetupDefaultPanels();

    /** creates the custom panels
    */
    void SetupCustomPanels();

    typedef ::std::vector< PanelDescriptor >    PanelDescriptors;
    PanelDescriptors                            m_aPanels;
    ToolPanelViewShell&                         m_rPanelViewShell;
    ::boost::scoped_ptr< ToolPanelDeck >        m_pPanelDeck;
    PanelSelectorLayout                         m_eCurrentLayout;
    bool                                        m_bInitialized;
};

// =====================================================================================================================
// = helpers
// =====================================================================================================================
// ---------------------------------------------------------------------------------------------------------------------
namespace {

enum MenuId {
    MID_UNLOCK_TASK_PANEL = 1,
    MID_LOCK_TASK_PANEL = 2,
    MID_LAYOUT_TABS = 3,
    MID_LAYOUT_DRAWERS = 4,
    MID_FIRST_PANEL = 5
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
size_t ToolPanelViewShell::Implementation::SetupDefaultPanels()
{
    typedef std::auto_ptr<ControlFactory> (*ControlFactoryFactory)( ToolPanelViewShell& i_rToolPanelShell );

    struct PanelDescriptor
    {
        ControlFactoryFactory   pFactory;
        const sal_Char*         pImageCommandName;
        USHORT                  nTitleResourceID;
        ULONG                   nHelpID;
        PanelId                 nPanelID;
        ::rtl::OUString         sResourceURL;
    };

    PanelDescriptor aPanels[] =
    {
        // "Master Pages"
        {   &controls::MasterPagesPanel::CreateControlFactory,
            "PresentationLayout",
            STR_TASKPANEL_MASTER_PAGE_TITLE,
            HID_SD_SLIDE_DESIGNS,
            PID_MASTER_PAGES,
            FrameworkHelper::msMasterPagesTaskPanelURL
        },
        // "Layout"
        {   &LayoutMenu::CreateControlFactory,
            "ModifyPage",
            STR_TASKPANEL_LAYOUT_MENU_TITLE,
            HID_SD_SLIDE_LAYOUTS,
            PID_LAYOUT,
            FrameworkHelper::msLayoutTaskPanelURL
        },
        // "Tables Design"
        {   &controls::TableDesignPanel::CreateControlFactory,
            "InsertTable",
            DLG_TABLEDESIGNPANE,
            HID_SD_TABLE_DESIGN,
            PID_TABLE_DESIGN,
            FrameworkHelper::msTableDesignPanelURL
        },
        // "Custom Animation"
        {   &controls::CustomAnimationPanel::CreateControlFactory,
            "CustomAnimation",
            STR_CUSTOMANIMATIONPANE,
            HID_SD_CUSTOM_ANIMATIONS,
            PID_CUSTOM_ANIMATION,
            FrameworkHelper::msCustomAnimationTaskPanelURL
        },
        // "Slide Transition"
        {   &controls::SlideTransitionPanel::CreateControlFactory,
            "RehearseTimings",
            STR_SLIDE_TRANSITION_PANE,
            HID_SD_SLIDE_TRANSITIONS,
            PID_SLIDE_TRANSITION,
            FrameworkHelper::msSlideTransitionTaskPanelURL
        }
    };

    // compose the resource ID for the ToolPanel view
    ::boost::shared_ptr< FrameworkHelper > pFrameworkHelper( FrameworkHelper::Instance( m_rPanelViewShell.GetViewShellBase() ) );
    const Reference< XResourceId > xToolPanelId( pFrameworkHelper->CreateResourceId( FrameworkHelper::msTaskPaneURL, FrameworkHelper::msRightPaneURL ) );

    // want to activate the "Layout" panel later on, need to translate its PanelId to an actual position
    const PanelId nPanelIdToActivate = PID_LAYOUT;
    size_t nPanelPosToActivate = size_t( -1 );

    // create the panels
    Reference< XFrame > xFrame( m_rPanelViewShell.GetViewShellBase().GetViewFrame()->GetFrame()->GetFrameInterface() );
    const BOOL bHiContrast( m_pPanelDeck->GetSettings().GetStyleSettings().GetHighContrastMode() );
    for ( size_t i=0; i < sizeof( aPanels ) / sizeof( aPanels[0] ); ++i )
    {
        // compose the command name, and obtain the image for it
        ::rtl::OUStringBuffer aCommandName;
        aCommandName.appendAscii( ".uno:" );
        aCommandName.appendAscii( aPanels[i].pImageCommandName );
        const Image aPanelImage( GetImage( xFrame, aCommandName.makeStringAndClear(), FALSE, bHiContrast ) );

        // compose the resource ID of the panel
        const Reference< XResourceId > xPanelId( pFrameworkHelper->CreateResourceId( aPanels[i].sResourceURL, xToolPanelId ) );

        // create and insert the panel
        ::svt::PToolPanel pNewPanel( new StandardToolPanel(
            *m_pPanelDeck,
            (*aPanels[i].pFactory)( m_rPanelViewShell ),
            aPanels[i].nTitleResourceID,
            aPanelImage,
            aPanels[i].nHelpID,
            xPanelId
        ) );
        const size_t nPanelPos = m_pPanelDeck->InsertPanel( pNewPanel, m_pPanelDeck->GetPanelCount() );

        // remember it
        RegisterPanel( nPanelPos, aPanels[i].nPanelID, pNewPanel );

        if ( nPanelIdToActivate == aPanels[i].nPanelID )
            nPanelPosToActivate = nPanelPos;
    }

    return nPanelPosToActivate;
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell::Implementation::SetupCustomPanels()
{
    // compose the resource ID for the ToolPanel view
    ::boost::shared_ptr< FrameworkHelper > pFrameworkHelper( FrameworkHelper::Instance( m_rPanelViewShell.GetViewShellBase() ) );
    const Reference< XResourceId > xToolPanelId( pFrameworkHelper->CreateResourceId( FrameworkHelper::msTaskPaneURL, FrameworkHelper::msRightPaneURL ) );

    // get the config node holding the custom tool panel descriptions
    ::utl::OConfigurationTreeRoot aConfig( ::comphelper::ComponentContext( ::comphelper::getProcessServiceFactory() ),
        "/org.openoffice.Office.Impress/MultiPaneGUI/ToolPanel/CustomPanels", false );
    const Sequence< ::rtl::OUString > aCustomPanelDescs( aConfig.getNodeNames() );

    // create panels
    size_t nCustomPanelNo = 0;
    for (   const ::rtl::OUString* panelNodeName = aCustomPanelDescs.getConstArray();
            panelNodeName != aCustomPanelDescs.getConstArray() + aCustomPanelDescs.getLength();
            ++panelNodeName
        )
    {
        ::utl::OConfigurationNode aPanelDesc( aConfig.openNode( *panelNodeName ) );

        // create and insert the panel
        ::svt::PToolPanel pNewPanel( new CustomToolPanel( *m_pPanelDeck, aPanelDesc, xToolPanelId, pFrameworkHelper ) );
        const size_t nPanelPos = m_pPanelDeck->InsertPanel( pNewPanel, m_pPanelDeck->GetPanelCount() );

        // remember it
        RegisterPanel( nPanelPos, PanelId( PID_FIRST_CUSTOM_PANEL + nCustomPanelNo ), pNewPanel );
        ++nCustomPanelNo;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell::Implementation::Setup()
{
    if ( m_bInitialized )
        return;
    m_bInitialized = true;

    // standard (built-in) panels
    const size_t nPanelPosToActivate = SetupDefaultPanels();

    // custom panels
    SetupCustomPanels();

    // activate default panel
    m_pPanelDeck->ActivatePanelResource( nPanelPosToActivate );

    // add as listener to the panel deck
    m_pPanelDeck->AddListener( *this );

    // initialize panel selector
    SetLayout( LAYOUT_DRAWERS, true );

    m_pPanelDeck->Show();
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell::Implementation::SetLayout( const PanelSelectorLayout i_eLayout, const bool i_bForce )
{
    if ( !i_bForce && ( m_eCurrentLayout == i_eLayout ) )
        return;

    switch ( i_eLayout )
    {
    case LAYOUT_DRAWERS:
        m_pPanelDeck->SetDrawersLayout();
        break;
    case LAYOUT_TABS_TOP:
        m_pPanelDeck->SetTabsLayout( ::svt::TABS_TOP, ::svt::TABITEM_IMAGE_ONLY );
        break;
    case LAYOUT_TABS_BOTTOM:
        m_pPanelDeck->SetTabsLayout( ::svt::TABS_BOTTOM, ::svt::TABITEM_IMAGE_ONLY );
        break;
    case LAYOUT_TABS_LEFT:
        m_pPanelDeck->SetTabsLayout( ::svt::TABS_LEFT, ::svt::TABITEM_IMAGE_ONLY );
        break;
    case LAYOUT_TABS_RIGHT:
        m_pPanelDeck->SetTabsLayout( ::svt::TABS_RIGHT, ::svt::TABITEM_IMAGE_ONLY );
        break;
    }
    m_eCurrentLayout = i_eLayout;
    UpdateDockingWindowTitle();
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell::Implementation::Cleanup()
{
    if ( !m_bInitialized )
    {
        m_pPanelDeck->RemoveListener( *this );
        // remove the panels which are not under the control of the panel deck currently
        for (   PanelDescriptors::iterator panelPos = m_aPanels.begin();
                panelPos != m_aPanels.end();
                ++panelPos
            )
        {
            if ( panelPos->bHidden )
                panelPos->pPanel->Dispose();
        }
        m_aPanels.clear();
    }
    m_pPanelDeck.reset();
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell::Initialize()
{
    mpImpl->Setup();
}

// ---------------------------------------------------------------------------------------------------------------------
ToolPanelViewShell::ToolPanelViewShell( SfxViewFrame* pFrame, ViewShellBase& rViewShellBase, ::Window* pParentWindow,
        FrameView* pFrameViewArgument )
    :ViewShell(pFrame, pParentWindow, rViewShellBase)
    ,mpImpl( new Implementation( *this, *mpContentWindow.get() ) )
    ,mpSubShellManager()
    ,mnMenuId(0)
{
    meShellType = ST_TASK_PANE;

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
    mpImpl->Cleanup();

    // reset our impl before destroying the panel deck, to ensure the hidden panels are properly
    // disposed/destroyed, too
    mpImpl.reset();
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

    mpImpl->GetPanelDeck().SetPosSizePixel( Point(), maViewSize );
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
        if ( !mpImpl->GetPanelDeck().HasChildPathFocus() )
            mpImpl->GetPanelDeck().GrabFocus();
    }
    else
        ViewShell::KeyInput( i_rKeyEvent, NULL );
}

// ---------------------------------------------------------------------------------------------------------------------
namespace
{
    PanelSelectorLayout lcl_getTabLayoutFromAlignment( const SfxChildAlignment i_eAlignment )
    {
        switch ( i_eAlignment )
        {
        case SFX_ALIGN_LEFT:
            return LAYOUT_TABS_LEFT;
        case SFX_ALIGN_TOP:
            return LAYOUT_TABS_TOP;
        case SFX_ALIGN_BOTTOM:
            return LAYOUT_TABS_BOTTOM;
        default:
            return LAYOUT_TABS_RIGHT;
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
IMPL_LINK( ToolPanelViewShell, DockingChanged, PaneDockingWindow*, i_pDockingWindow )
{
    ENSURE_OR_RETURN( i_pDockingWindow == dynamic_cast< PaneDockingWindow* >( GetDockingWindow() ), "unknown source", 0 );

    if ( mpImpl->GetLayout() == LAYOUT_DRAWERS )
        return 0L;

    mpImpl->SetLayout( lcl_getTabLayoutFromAlignment( i_pDockingWindow->GetAlignment() ) );
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

            case MID_LAYOUT_DRAWERS:
                mpImpl->SetLayout( LAYOUT_DRAWERS );
                break;

            case MID_LAYOUT_TABS:
            {
                PaneDockingWindow* pDockingWindow = dynamic_cast< PaneDockingWindow* >( GetDockingWindow() );
                OSL_ENSURE( pDockingWindow != NULL, "ToolPanelViewShell::MenuSelectHandler: unknown docking window type!" );
                if ( pDockingWindow )
                    mpImpl->SetLayout( lcl_getTabLayoutFromAlignment( pDockingWindow->GetAlignment() ) );
            }
            break;

            default:
            {
                size_t nPanelIndex = size_t( pMenu->GetCurItemId() - MID_FIRST_PANEL );
                mpImpl->TogglePanelVisibility( nPanelIndex );
            }
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
    USHORT nIndex = MID_FIRST_PANEL;
    for ( size_t i=0; i<mpImpl->GetPanelCount(); ++i, ++nIndex )
    {
        const PanelDescriptor& rPanelDesc( mpImpl->GetPanel(i) );
        pMenu->InsertItem( nIndex, rPanelDesc.pPanel->GetDisplayName(), MIB_CHECKABLE );
        pMenu->SetUserValue( nIndex, rPanelDesc.nId );
        pMenu->CheckItem( nIndex, !rPanelDesc.bHidden );
    }
    pMenu->InsertSeparator();

#if OSL_DEBUG_LEVEL > 0
    pMenu->InsertItem( MID_LAYOUT_TABS, String::CreateFromAscii( "Tab-Layout" ), MIB_CHECKABLE );
    pMenu->CheckItem( MID_LAYOUT_TABS, mpImpl->GetLayout() != LAYOUT_DRAWERS );
    pMenu->InsertItem( MID_LAYOUT_DRAWERS, String::CreateFromAscii( "Drawer-Layout" ), MIB_CHECKABLE );
    pMenu->CheckItem( MID_LAYOUT_DRAWERS, mpImpl->GetLayout() == LAYOUT_DRAWERS );

    pMenu->InsertSeparator();
#endif

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
    FocusManager::Instance().RegisterDownLink( GetParentWindow(), &mpImpl->GetPanelDeck() );
}

// ---------------------------------------------------------------------------------------------------------------------
bool ToolPanelViewShell::RelocateToParentWindow( ::Window* pParentWindow )
{
    ::Window* pOldParentWindow = GetParentWindow();
    FocusManager::Instance().RemoveLinks( pOldParentWindow, &mpImpl->GetPanelDeck() );
    FocusManager::Instance().RemoveLinks( &mpImpl->GetPanelDeck(), pOldParentWindow );

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

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell::DeactivatePanel( const PanelId i_ePanelId )
{
    mpImpl->DeactivatePanelDirectly( i_ePanelId );
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell::ActivatePanel( const PanelId i_ePanelId )
{
    mpImpl->ActivatePanelDirectly( i_ePanelId );
}


// =====================================================================================================================
// = ToolPanelViewShell:Implementation - implementation
// =====================================================================================================================
// ---------------------------------------------------------------------------------------------------------------------
ToolPanelViewShell::Implementation::Implementation( ToolPanelViewShell& i_rPanelViewShell, ::Window& i_rPanelDeckParent )
    :m_aPanels()
    ,m_rPanelViewShell( i_rPanelViewShell )
    ,m_pPanelDeck( new ToolPanelDeck( i_rPanelDeckParent, i_rPanelViewShell ) )
    ,m_eCurrentLayout( LAYOUT_DRAWERS )
    ,m_bInitialized( false )
{
}

// ---------------------------------------------------------------------------------------------------------------------
ToolPanelViewShell::Implementation::~Implementation()
{
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell::Implementation::TogglePanelVisibility( const size_t i_nLogicalPanelIndex )
{
    ENSURE_OR_RETURN_VOID( i_nLogicalPanelIndex < m_aPanels.size(), "illegal index" );

    // get the actual panel index, within the deck
    size_t nActualPanelIndex(0);
    for ( size_t i=0; i < i_nLogicalPanelIndex; ++i )
    {
        if ( !m_aPanels[i].bHidden )
            ++nActualPanelIndex;
    }
    if ( m_aPanels[ i_nLogicalPanelIndex ].bHidden )
    {
        OSL_VERIFY( m_pPanelDeck->InsertPanel( m_aPanels[ i_nLogicalPanelIndex ].pPanel, nActualPanelIndex ) == nActualPanelIndex );
        // if there has not been an active panel before, activate the newly inserted one
        ::boost::optional< size_t > aActivePanel( m_pPanelDeck->GetActivePanel() );
        if ( !aActivePanel )
            m_pPanelDeck->ActivatePanelResource( nActualPanelIndex );
    }
    else
    {
        OSL_VERIFY( m_pPanelDeck->RemovePanel( nActualPanelIndex ).get() == m_aPanels[ i_nLogicalPanelIndex ].pPanel.get() );
    }
    m_aPanels[ i_nLogicalPanelIndex ].bHidden = !m_aPanels[ i_nLogicalPanelIndex ].bHidden;
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell::Implementation::DeactivatePanelDirectly( const PanelId i_nPanelId )
{
    for ( size_t i=0; i<m_aPanels.size(); ++i )
    {
        if ( m_aPanels[i].nId == i_nPanelId )
        {
            if ( m_pPanelDeck->GetActivePanel() == i )
                m_pPanelDeck->ActivatePanelDirectly( ::boost::optional< size_t >() );
            return;
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell::Implementation::ActivatePanelDirectly( const PanelId i_nPanelId )
{
    size_t nActualPanelIndex(0);
    for ( size_t i=0; i<m_aPanels.size(); ++i )
    {
        if ( m_aPanels[i].nId == i_nPanelId )
        {
            if ( m_aPanels[i].bHidden )
                TogglePanelVisibility( i );
            m_pPanelDeck->ActivatePanelDirectly( nActualPanelIndex );
            return;
        }
        if ( !m_aPanels[i].bHidden )
            ++nActualPanelIndex;
    }
    OSL_ENSURE( false, "ToolPanelViewShell::ActivatePanelDirectly: don't have a panel with the given ID!" );
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell::Implementation::RegisterPanel( size_t i_nPosition, PanelId i_nPanelId, const ::svt::PToolPanel& i_rPanel )
{
    if ( i_nPosition >= m_aPanels.size() )
        m_aPanels.resize( i_nPosition + 1 );

    OSL_PRECOND( m_aPanels[ i_nPosition ].nId == PID_UNKNOWN, "ToolPanelViewShell::Implementation::RegisterPanel: "
        "already registered a panel for this ID!" );
    m_aPanels[ i_nPosition ] = PanelDescriptor( i_nPanelId, i_rPanel );
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell::Implementation::PanelInserted( const ::svt::PToolPanel& i_pPanel, const size_t i_nPosition )
{
    (void)i_pPanel;
    (void)i_nPosition;
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell::Implementation::PanelRemoved( const size_t i_nPosition )
{
    (void)i_nPosition;
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell::Implementation::UpdateDockingWindowTitle()
{
    PaneDockingWindow* pDockingWindow = dynamic_cast< PaneDockingWindow* >( m_rPanelViewShell.GetDockingWindow() );
    ENSURE_OR_RETURN_VOID( pDockingWindow, "ToolPanelViewShell::Implementation::UpdateDockingWindowTitle: no PaneDockingWindow!?" );

    ::boost::optional< size_t > aActivePanel( m_pPanelDeck->GetActivePanel() );
    if ( !aActivePanel || ( GetLayout() == LAYOUT_DRAWERS ) )
        pDockingWindow->SetTitle( String( SdResId( STR_RIGHT_PANE_TITLE ) ) );
    else
    {
        size_t nNewActive( *aActivePanel );
        for ( size_t i=0; i < m_aPanels.size(); ++i )
        {
            if ( m_aPanels[i].bHidden )
                continue;

            if ( !nNewActive )
            {
                pDockingWindow->SetTitle( m_aPanels[i].pPanel->GetDisplayName() );
                break;
            }
            --nNewActive;
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell::Implementation::ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive )
{
    if ( GetLayout() == LAYOUT_DRAWERS )
        // no adjustment of the title when we use the classical "drawers" layout
        return;

    UpdateDockingWindowTitle( );
    (void)i_rOldActive;
    (void)i_rNewActive;
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell::Implementation::Dying()
{
}

} } // end of namespace ::sd::toolpanel
