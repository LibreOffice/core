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
#include "taskpane/TaskPaneControlFactory.hxx"
#include "controls/MasterPagesPanel.hxx"
#include "LayoutMenu.hxx"
#include "controls/TableDesignPanel.hxx"
#include "controls/CustomAnimationPanel.hxx"
#include "controls/SlideTransitionPanel.hxx"
#include "controls/MasterPagesSelector.hxx"
#include "ToolPanel.hxx"
#include "ToolPanelUIElement.hxx"
#include "PaneDockingWindow.hxx"
#include "FrameView.hxx"
#include "Window.hxx"
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
#include <com/sun/star/drawing/framework/AnchorBindingMode.hpp>
#include <com/sun/star/drawing/framework/ResourceActivationMode.hpp>
#include <com/sun/star/drawing/XDrawSubController.hpp>
/** === end UNO includes === **/

#include <svtools/toolpanel/toolpanel.hxx>
#include <svtools/toolpanel/toolpaneldeck.hxx>
#include <svx/dlgctrl.hxx>
#include <sfx2/taskpane.hxx>
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
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/basemutex.hxx>

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
using ::com::sun::star::drawing::framework::XConfigurationChangeListener;
using ::com::sun::star::drawing::framework::ConfigurationChangeEvent;
using ::com::sun::star::lang::EventObject;
using ::com::sun::star::lang::DisposedException;
using ::com::sun::star::drawing::framework::XConfigurationControllerBroadcaster;
using ::com::sun::star::drawing::framework::XConfigurationController;
using ::com::sun::star::drawing::framework::XConfiguration;
using ::com::sun::star::drawing::framework::AnchorBindingMode_DIRECT;
using ::com::sun::star::ui::XUIElement;
using ::com::sun::star::ui::XToolPanel;
using ::com::sun::star::drawing::framework::ResourceActivationMode_REPLACE;
/** === end UNO using === **/

using ::sd::framework::FrameworkHelper;

namespace sd { namespace toolpanel {

// =====================================================================================================================
// = PanelDescriptor
// =====================================================================================================================
/** is a helper class for ToolPanelViewShell_Impl, holding the details about a single panel which is not
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
// = ConfigurationListener - declaration
// =====================================================================================================================
typedef ::cppu::WeakImplHelper1 <   XConfigurationChangeListener
                                >   ConfigurationListener_Base;

class ConfigurationListener :public ::cppu::BaseMutex
                            ,public ConfigurationListener_Base
{
public:
    ConfigurationListener( ToolPanelViewShell_Impl& i_rShellImpl );

    // XConfigurationChangeListener
    virtual void SAL_CALL notifyConfigurationChange( const ConfigurationChangeEvent& aEvent ) throw (RuntimeException);

    // XEventListener
    virtual void SAL_CALL disposing( const EventObject& Source ) throw (RuntimeException);

    // XComponent equivalents (not available per UNO interface)
    void    dispose();

protected:
    ~ConfigurationListener();

    void impl_checkDisposed_throw()
    {
        if ( !m_pShellImpl )
            throw DisposedException( ::rtl::OUString(), *this );
    }

private:
    ToolPanelViewShell_Impl* m_pShellImpl;
};

// =====================================================================================================================
// = ToolPanelViewShell_Impl - declaration
// =====================================================================================================================
/** Inner implementation class of ToolPanelViewShell.
*/
class ToolPanelViewShell_Impl : public ::svt::IToolPanelDeckListener
{
public:
    static const size_t mnInvalidId = static_cast< size_t >( -1 );

    ToolPanelViewShell_Impl( ToolPanelViewShell& i_rPanelViewShell, ::Window& i_rPanelDeckParent );
    ~ToolPanelViewShell_Impl();

    ToolPanelViewShell& GetAntiImpl() { return m_rPanelViewShell; }

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

    /** activates the panel which has the given resource URL
    */
    void    ActivatePanelByResource( const ::rtl::OUString& i_rPanelResourceURL );

    /** de-activates the panel given by its resource URL, bypassing the configuration controller

        If the panel is not active currently, nothing happens.
    */
    void    DeactivatePanelByResource( const ::rtl::OUString& i_rPanelResourceURL );

    /** provides access to the the VCL window of the panel deck
    */
          ::sfx2::ModuleTaskPane& GetTaskPane()       { return *m_pTaskPane; }
    const ::sfx2::ModuleTaskPane& GetTaskPane() const { return *m_pTaskPane; }

          ::svt::ToolPanelDeck& GetToolPanelDeck()       { return GetTaskPane().GetPanelDeck(); }
    const ::svt::ToolPanelDeck& GetToolPanelDeck() const { return GetTaskPane().GetPanelDeck(); }

    void    TogglePanelVisibility( const size_t i_nLogicalPanelIndex );

    Reference< XAccessible >
            CreateAccessible( ::sd::Window& i_rWindow );

protected:
    // IToolPanelDeckListener overridables
    virtual void PanelInserted( const ::svt::PToolPanel& i_pPanel, const size_t i_nPosition );
    virtual void PanelRemoved( const size_t i_nPosition );
    virtual void ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive );
    virtual void LayouterChanged( const ::svt::PDeckLayouter& i_rNewLayouter );
    virtual void Dying();

private:
    void UpdateDockingWindowTitle();

    struct InitialPanel
    {
        ::rtl::OUString sPanelResourceURL;
        bool            bActivateDirectly;
        InitialPanel()
            :sPanelResourceURL()
            ,bActivateDirectly( true )
        {
        }
    };
    InitialPanel DetermineInitialPanel();

    typedef ::std::vector< PanelDescriptor >        PanelDescriptors;
    ToolPanelViewShell&                             m_rPanelViewShell;
    ::boost::scoped_ptr< ::sfx2::ModuleTaskPane >   m_pTaskPane;
    //::boost::scoped_ptr< ToolPanelDeck >            m_pPanelDeck;
    ::rtl::Reference< ConfigurationListener >       m_pConfigListener;
    PanelSelectorLayout                             m_eCurrentLayout;
    bool                                            m_bInitialized;
};

// =====================================================================================================================
// = ConfigurationListener - implementation
// =====================================================================================================================
// ---------------------------------------------------------------------------------------------------------------------
ConfigurationListener::ConfigurationListener( ToolPanelViewShell_Impl& i_rShellImpl )
    :m_pShellImpl( &i_rShellImpl )
{
    ::boost::shared_ptr< FrameworkHelper > pFrameworkHelper( FrameworkHelper::Instance( i_rShellImpl.GetAntiImpl().GetViewShellBase() ) );
    Reference< XConfigurationControllerBroadcaster > xBroadcaster;
    if ( pFrameworkHelper.get() )
        xBroadcaster.set( pFrameworkHelper->GetConfigurationController().get() );
    ENSURE_OR_THROW( pFrameworkHelper.get(), "no access to the config controller" );

    osl_incrementInterlockedCount( &m_refCount );
    {
        xBroadcaster->addConfigurationChangeListener( this, ::rtl::OUString(), Any() );
    }
    osl_decrementInterlockedCount( &m_refCount );
}

// ---------------------------------------------------------------------------------------------------------------------
ConfigurationListener::~ConfigurationListener()
{
}

// ---------------------------------------------------------------------------------------------------------------------
void SAL_CALL ConfigurationListener::notifyConfigurationChange( const ConfigurationChangeEvent& i_rEvent ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    // is this an event we're interested in?
    if ( i_rEvent.Type != FrameworkHelper::msResourceActivationEvent )
        return;

    // is this a resource we're interested in? Must be anchored in the task pane ...
    Reference< XResourceId > xAnchorId;
    if ( i_rEvent.ResourceId.is() )
        xAnchorId = i_rEvent.ResourceId->getAnchor();
    if ( !xAnchorId.is() )
        return;
    const ::rtl::OUString sAnchorURL( xAnchorId->getResourceURL() );
    if ( sAnchorURL != FrameworkHelper::msTaskPaneURL )
        return;

    m_pShellImpl->ActivatePanelByResource( i_rEvent.ResourceId->getResourceURL() );
}

// ---------------------------------------------------------------------------------------------------------------------
void SAL_CALL ConfigurationListener::disposing( const EventObject& i_rEvent ) throw (RuntimeException)
{
    (void)i_rEvent;
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        impl_checkDisposed_throw();
    }

    dispose();
}

// ---------------------------------------------------------------------------------------------------------------------
void ConfigurationListener::dispose()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    m_pShellImpl = NULL;
}

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
ToolPanelViewShell_Impl::InitialPanel ToolPanelViewShell_Impl::DetermineInitialPanel()
{
    // Default to Layout panel, but check whether the requested configuration already contains a tool panel, in this case,
    // use that one.
    InitialPanel aPanelToActivate;
    aPanelToActivate.sPanelResourceURL = FrameworkHelper::msLayoutTaskPanelURL;
    aPanelToActivate.bActivateDirectly = false;
    try
    {
        ::boost::shared_ptr< FrameworkHelper > pFrameworkHelper( FrameworkHelper::Instance( m_rPanelViewShell.GetViewShellBase() ) );
        const Reference< XResourceId > xToolPanelId( pFrameworkHelper->CreateResourceId( FrameworkHelper::msTaskPaneURL, FrameworkHelper::msRightPaneURL ) );
        Reference< XConfigurationController > xCC( pFrameworkHelper->GetConfigurationController(), UNO_QUERY_THROW );
        Reference< XConfiguration > xConfiguration( xCC->getRequestedConfiguration(), UNO_QUERY_THROW );
        Sequence< Reference< XResourceId > > aViewIds( xConfiguration->getResources(
            FrameworkHelper::CreateResourceId( FrameworkHelper::msTaskPaneURL, FrameworkHelper::msRightPaneURL ),
            FrameworkHelper::msTaskPanelURLPrefix, AnchorBindingMode_DIRECT ) );

        if ( aViewIds.getLength() > 0 )
        {
            const ::rtl::OUString sResourceURL( aViewIds[0]->getResourceURL() );
            PanelId nRequestedPanel = GetStandardPanelId( sResourceURL );
            if ( nRequestedPanel != PID_UNKNOWN )
            {
                aPanelToActivate.sPanelResourceURL = sResourceURL;
                aPanelToActivate.bActivateDirectly = true;
            }
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return aPanelToActivate;
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell_Impl::Setup()
{
    if ( m_bInitialized )
        return;
    m_bInitialized = true;

    // initially activate a panel
    const InitialPanel aInitialPanel = DetermineInitialPanel();
    if ( aInitialPanel.sPanelResourceURL.getLength() )
    {
        if ( aInitialPanel.bActivateDirectly )
        {
            ActivatePanelByResource( aInitialPanel.sPanelResourceURL );
        }
        else
        {
            ::boost::shared_ptr< FrameworkHelper > pFrameworkHelper( FrameworkHelper::Instance( GetAntiImpl().GetViewShellBase() ) );
            const Reference< XResourceId > xToolPanelId( pFrameworkHelper->CreateResourceId(
                FrameworkHelper::msTaskPaneURL, FrameworkHelper::msRightPaneURL, aInitialPanel.sPanelResourceURL ) );
            pFrameworkHelper->GetConfigurationController()->requestResourceActivation(
                xToolPanelId, ResourceActivationMode_REPLACE );
        }
    }

//    // add as listener to the panel deck
//    m_pPanelDeck->AddListener( *this );

    // initialize panel selector
    SetLayout( LAYOUT_DRAWERS, true );

    // listen at the configuration
    m_pConfigListener.set( new ConfigurationListener( *this ) );

    m_pTaskPane->Show();
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell_Impl::SetLayout( const PanelSelectorLayout i_eLayout, const bool i_bForce )
{
    if ( !i_bForce && ( m_eCurrentLayout == i_eLayout ) )
        return;

//    switch ( i_eLayout )
//    {
//    case LAYOUT_DRAWERS:
//        m_pPanelDeck->SetDrawersLayout();
//        break;
//    case LAYOUT_TABS_TOP:
//        m_pPanelDeck->SetTabsLayout( ::svt::TABS_TOP, ::svt::TABITEM_IMAGE_ONLY );
//        break;
//    case LAYOUT_TABS_BOTTOM:
//        m_pPanelDeck->SetTabsLayout( ::svt::TABS_BOTTOM, ::svt::TABITEM_IMAGE_ONLY );
//        break;
//    case LAYOUT_TABS_LEFT:
//        m_pPanelDeck->SetTabsLayout( ::svt::TABS_LEFT, ::svt::TABITEM_IMAGE_ONLY );
//        break;
//    case LAYOUT_TABS_RIGHT:
//        m_pPanelDeck->SetTabsLayout( ::svt::TABS_RIGHT, ::svt::TABITEM_IMAGE_ONLY );
//        break;
//    }
    m_eCurrentLayout = i_eLayout;
    UpdateDockingWindowTitle();
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell_Impl::Cleanup()
{
    if ( m_bInitialized )
    {
//        m_pPanelDeck->RemoveListener( *this );
//        // remove the panels which are not under the control of the panel deck currently
//        for (   PanelDescriptors::iterator panelPos = m_aPanels.begin();
//                panelPos != m_aPanels.end();
//                ++panelPos
//            )
//        {
//            if ( panelPos->bHidden )
//                panelPos->pPanel->Dispose();
//        }
//        m_aPanels.clear();
//
//        if ( m_pConfigListener.is() )
//            m_pConfigListener->dispose();
    }
//    m_pPanelDeck.reset();
    m_pTaskPane.reset();
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell_Impl::ActivatePanelByResource( const ::rtl::OUString& i_rResourceURL )
{
    // determine position of the requested panel
    ::boost::optional< size_t > aPanelPos = GetTaskPane().GetPanelPos( i_rResourceURL );
    OSL_ENSURE( !!aPanelPos, "ToolPanelViewShell_Impl::ActivatePanelByResource: illegal panel resource, or illegal panel deck setup!" );
    if ( !!aPanelPos )
        GetToolPanelDeck().ActivatePanel( *aPanelPos );
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell_Impl::DeactivatePanelByResource( const ::rtl::OUString& i_rPanelResourceURL )
{
    // determine position of the requested panel
    ::boost::optional< size_t > aPanelPos = GetTaskPane().GetPanelPos( i_rPanelResourceURL );
    OSL_ENSURE( !!aPanelPos, "ToolPanelViewShell_Impl::DeactivatePanelByResource: illegal panel resource, or illegal panel deck setup!" );
    if ( !!aPanelPos )
    {
        if ( GetToolPanelDeck().GetActivePanel() == *aPanelPos )
            GetToolPanelDeck().ActivatePanel( ::boost::optional< size_t >() );
    }
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
    ,mpImpl( new ToolPanelViewShell_Impl( *this, *mpContentWindow.get() ) )
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

    mpImpl->GetTaskPane().SetPosSizePixel( Point(), maViewSize );
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
        if ( !mpImpl->GetTaskPane().HasChildPathFocus() )
            mpImpl->GetTaskPane().GrabFocus();
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
//    for ( size_t i=0; i<mpImpl->GetPanelCount(); ++i, ++nIndex )
//    {
//        const PanelDescriptor& rPanelDesc( mpImpl->GetPanel(i) );
//        pMenu->InsertItem( nIndex, rPanelDesc.pPanel->GetDisplayName(), MIB_CHECKABLE );
//        pMenu->SetUserValue( nIndex, rPanelDesc.nId );
//        pMenu->CheckItem( nIndex, !rPanelDesc.bHidden );
//    }
//    pMenu->InsertSeparator();

#if OSL_DEBUG_LEVEL > 0
    pMenu->InsertItem( MID_LAYOUT_TABS, String::CreateFromAscii( "Tab-Layout (exp.)" ), MIB_CHECKABLE );
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
Reference< XAccessible > ToolPanelViewShell::CreateAccessibleDocumentView( ::sd::Window* i_pWindow )
{
    ENSURE_OR_RETURN( i_pWindow, "ToolPanelViewShell::CreateAccessibleDocumentView: illegal window!", NULL );
    return mpImpl->CreateAccessible( *i_pWindow );
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
        pDockingWindow->ResetToolBox();
        mnMenuId = pDockingWindow->AddDropDownToolBoxItem (
            String(SdResId(STR_TASKPANEL_MASTER_PAGE_MENU_TITLE)),
            HID_SD_TASK_PANE_VIEW_MENU,
            LINK(this, ToolPanelViewShell, ToolboxClickHandler));

        // be notified when the docking position of the window changes
        pDockingWindow->SetEndDockingHdl( LINK( this, ToolPanelViewShell, DockingChanged ) );
    }

    // Tell the focus manager that we want to pass the focus to our
    // child.
    FocusManager::Instance().RegisterDownLink( GetParentWindow(), &mpImpl->GetTaskPane() );
}

// ---------------------------------------------------------------------------------------------------------------------
bool ToolPanelViewShell::RelocateToParentWindow( ::Window* pParentWindow )
{
    ::Window* pOldParentWindow = GetParentWindow();
    FocusManager::Instance().RemoveLinks( pOldParentWindow, &mpImpl->GetTaskPane() );
    FocusManager::Instance().RemoveLinks( &mpImpl->GetTaskPane(), pOldParentWindow );

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

//---------------------------------------------------------------------------------------------------------------------
bool ToolPanelViewShell::IsPanelAnchorWindow( const ::Window& i_rWindow ) const
{
    return &mpImpl->GetToolPanelDeck().GetPanelWindowAnchor() == &i_rWindow;
}

//---------------------------------------------------------------------------------------------------------------------
namespace
{
    typedef std::auto_ptr< ControlFactory > (*ControlFactoryFactory)( ToolPanelViewShell& i_rToolPanelShell );

    struct PanelFactory
    {
        ControlFactoryFactory   pFactory;
        ULONG                   nHelpID;
        PanelFactory( const ControlFactoryFactory i_pFactory, const ULONG i_nHelpID )
            :pFactory( i_pFactory )
            ,nHelpID( i_nHelpID )
        {
        }
    };

    const PanelFactory lcl_describePanel( const PanelId i_ePanelId )
    {
        switch ( i_ePanelId )
        {
        case PID_MASTER_PAGES:
            return PanelFactory( &controls::MasterPagesPanel::CreateControlFactory, HID_SD_SLIDE_DESIGNS );
        case PID_LAYOUT:
            return PanelFactory( &LayoutMenu::CreateControlFactory, HID_SD_SLIDE_LAYOUTS );
        case PID_TABLE_DESIGN:
            return PanelFactory( &controls::TableDesignPanel::CreateControlFactory, HID_SD_TABLE_DESIGN );
        case PID_CUSTOM_ANIMATION:
            return PanelFactory( &controls::CustomAnimationPanel::CreateControlFactory, HID_SD_CUSTOM_ANIMATIONS );
        case PID_SLIDE_TRANSITION:
            return PanelFactory( &controls::SlideTransitionPanel::CreateControlFactory, HID_SD_SLIDE_TRANSITIONS );
        default:
            break;
        }
        throw RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "illegal panel ID" ) ), NULL );
    }
}

//---------------------------------------------------------------------------------------------------------------------
Reference< XUIElement > ToolPanelViewShell::CreatePanelUIElement( const Reference< XFrame >& i_rDocFrame, const ::rtl::OUString& i_rPanelResourceURL )
{
    const PanelId ePanelId( GetStandardPanelId( i_rPanelResourceURL ) );
    ENSURE_OR_RETURN( ePanelId != PID_UNKNOWN, "ToolPanelViewShell::CreatePanelUIElement: illegal panel URL!", NULL );

    // a TreeNode which will resemble the panel
    const PanelFactory aPanelFactory( lcl_describePanel( ePanelId ) );
    ::std::auto_ptr< ControlFactory > pControlFactory( (*aPanelFactory.pFactory)( *this ) );
    ::std::auto_ptr< TreeNode > pNode( pControlFactory->CreateControl( mpImpl->GetToolPanelDeck().GetPanelWindowAnchor() ) );
    ENSURE_OR_THROW( ( pNode.get() != NULL ) && ( pNode->GetWindow() != NULL ),
        "illegal node returned by the control factory" );
    pNode->GetWindow()->SetHelpId( aPanelFactory.nHelpID );

    // create an XToolPanel
    Reference< XToolPanel > xPanel( new ToolPanel( pNode ) );

    // create an XUIElement providing this panel
    const Reference< XUIElement > xUIElement( new ToolPanelUIElement( i_rDocFrame, i_rPanelResourceURL, xPanel ) );

    return xUIElement;
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell::DeactivatePanel( const ::rtl::OUString& i_rPanelResourceURL )
{
    mpImpl->DeactivatePanelByResource( i_rPanelResourceURL );
}

// =====================================================================================================================
// = ToolPanelViewShell_Impl - implementation
// =====================================================================================================================
// ---------------------------------------------------------------------------------------------------------------------
ToolPanelViewShell_Impl::ToolPanelViewShell_Impl( ToolPanelViewShell& i_rPanelViewShell, ::Window& i_rPanelDeckParent )
    :m_rPanelViewShell( i_rPanelViewShell )
    ,m_pTaskPane( new ::sfx2::ModuleTaskPane( i_rPanelDeckParent, i_rPanelViewShell.GetViewShellBase().GetViewFrame()->GetFrame()->GetFrameInterface() ) )
    ,m_eCurrentLayout( LAYOUT_DRAWERS )
    ,m_bInitialized( false )
{
    const String sPaneTitle( SdResId( STR_RIGHT_PANE_TITLE ) );
    GetToolPanelDeck().SetAccessibleName( sPaneTitle );
    GetToolPanelDeck().SetAccessibleDescription( sPaneTitle );
}

// ---------------------------------------------------------------------------------------------------------------------
ToolPanelViewShell_Impl::~ToolPanelViewShell_Impl()
{
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell_Impl::TogglePanelVisibility( const size_t i_nLogicalPanelIndex )
{
    OSL_ENSURE( false, "ToolPanelViewShell_Impl::TogglePanelVisibility: not yet re-implemented!" );
//    ENSURE_OR_RETURN_VOID( i_nLogicalPanelIndex < m_aPanels.size(), "illegal index" );
//
//    // get the actual panel index, within the deck
//    size_t nActualPanelIndex(0);
//    for ( size_t i=0; i < i_nLogicalPanelIndex; ++i )
//    {
//        if ( !m_aPanels[i].bHidden )
//            ++nActualPanelIndex;
//    }
//    if ( m_aPanels[ i_nLogicalPanelIndex ].bHidden )
//    {
//        OSL_VERIFY( m_pPanelDeck->InsertPanel( m_aPanels[ i_nLogicalPanelIndex ].pPanel, nActualPanelIndex ) == nActualPanelIndex );
//        // if there has not been an active panel before, activate the newly inserted one
//        ::boost::optional< size_t > aActivePanel( m_pPanelDeck->GetActivePanel() );
//        if ( !aActivePanel )
//            m_pPanelDeck->ActivatePanelResource( nActualPanelIndex );
//    }
//    else
//    {
//        OSL_VERIFY( m_pPanelDeck->RemovePanel( nActualPanelIndex ).get() == m_aPanels[ i_nLogicalPanelIndex ].pPanel.get() );
//    }
//    m_aPanels[ i_nLogicalPanelIndex ].bHidden = !m_aPanels[ i_nLogicalPanelIndex ].bHidden;
    (void)i_nLogicalPanelIndex;
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell_Impl::PanelInserted( const ::svt::PToolPanel& i_pPanel, const size_t i_nPosition )
{
    (void)i_pPanel;
    (void)i_nPosition;
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell_Impl::PanelRemoved( const size_t i_nPosition )
{
    (void)i_nPosition;
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell_Impl::UpdateDockingWindowTitle()
{
//    PaneDockingWindow* pDockingWindow = dynamic_cast< PaneDockingWindow* >( m_rPanelViewShell.GetDockingWindow() );
//    ENSURE_OR_RETURN_VOID( pDockingWindow, "ToolPanelViewShell_Impl::UpdateDockingWindowTitle: no PaneDockingWindow!?" );
//
//    ::boost::optional< size_t > aActivePanel( m_pPanelDeck->GetActivePanel() );
//    if ( !aActivePanel || ( GetLayout() == LAYOUT_DRAWERS ) )
//        pDockingWindow->SetTitle( String( SdResId( STR_RIGHT_PANE_TITLE ) ) );
//    else
//    {
//        size_t nNewActive( *aActivePanel );
//        for ( size_t i=0; i < m_aPanels.size(); ++i )
//        {
//            if ( m_aPanels[i].bHidden )
//                continue;
//
//            if ( !nNewActive )
//            {
//                pDockingWindow->SetTitle( m_aPanels[i].pPanel->GetDisplayName() );
//                break;
//            }
//            --nNewActive;
//        }
//    }
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell_Impl::ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive )
{
    if ( GetLayout() == LAYOUT_DRAWERS )
        // no adjustment of the title when we use the classical "drawers" layout
        return;

    UpdateDockingWindowTitle( );
    (void)i_rOldActive;
    (void)i_rNewActive;
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell_Impl::LayouterChanged( const ::svt::PDeckLayouter& i_rNewLayouter )
{
    // not interested in
    (void)i_rNewLayouter;
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell_Impl::Dying()
{
}

// ---------------------------------------------------------------------------------------------------------------------
Reference< XAccessible > ToolPanelViewShell_Impl::CreateAccessible( ::sd::Window& i_rWindow )
{
//    ::Window* pAccessibleParent = i_rWindow.GetAccessibleParentWindow();
//    m_pPanelDeck->SetAccessibleParentWindow( pAccessibleParent );
//    return m_pPanelDeck->GetAccessible();
    (void)i_rWindow;
    return Reference< XAccessible >();
}

// ---------------------------------------------------------------------------------------------------------------------
PanelId GetStandardPanelId( const ::rtl::OUString& i_rTaskPanelResourceURL )
{
    PanelId ePanelId( PID_UNKNOWN );

    if ( i_rTaskPanelResourceURL.equals( FrameworkHelper::msMasterPagesTaskPanelURL ) )
    {
        ePanelId = PID_MASTER_PAGES;
    }
    else if ( i_rTaskPanelResourceURL.equals( FrameworkHelper::msLayoutTaskPanelURL ) )
    {
        ePanelId = PID_LAYOUT;
    }
    else if ( i_rTaskPanelResourceURL.equals( FrameworkHelper::msTableDesignPanelURL ) )
    {
        ePanelId = PID_TABLE_DESIGN;
    }
    else if ( i_rTaskPanelResourceURL.equals( FrameworkHelper::msCustomAnimationTaskPanelURL ) )
    {
        ePanelId = PID_CUSTOM_ANIMATION;
    }
    else if ( i_rTaskPanelResourceURL.equals( FrameworkHelper::msSlideTransitionTaskPanelURL ) )
    {
        ePanelId = PID_SLIDE_TRANSITION;
    }
    else
    {
        OSL_ENSURE( false, "GetStandardPanelId: cannot translate the given resource URL!" );
    }

    return ePanelId;
}

} } // end of namespace ::sd::toolpanel
