/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include "taskpane/ToolPanelViewShell.hxx"

#include "TaskPaneShellManager.hxx"
#include "TaskPaneFocusManager.hxx"
#include "taskpane/TaskPaneControlFactory.hxx"
#include "controls/MasterPagesPanel.hxx"
#include "LayoutMenu.hxx"
#include "DrawDocShell.hxx"
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

#include <com/sun/star/drawing/framework/XResourceId.hpp>
#include <com/sun/star/drawing/framework/ResourceActivationMode.hpp>
#include <com/sun/star/drawing/framework/AnchorBindingMode.hpp>
#include <com/sun/star/drawing/XDrawSubController.hpp>

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
// = misc helper
// =====================================================================================================================
// ---------------------------------------------------------------------------------------------------------------------
PanelId GetStandardPanelId( const ::rtl::OUString& i_rTaskPanelResourceURL, const bool i_bIgnoreUnknown )
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
        OSL_ENSURE( i_bIgnoreUnknown, "GetStandardPanelId: cannot translate the given resource URL!" );
        (void)i_bIgnoreUnknown;
    }

    return ePanelId;
}

// ---------------------------------------------------------------------------------------------------------------------
PanelId GetStandardPanelId( const ::rtl::OUString& i_rTaskPanelResourceURL )
{
    return GetStandardPanelId( i_rTaskPanelResourceURL, false );
}

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
class ToolPanelViewShell_Impl   :public ::boost::noncopyable
                                ,public ::svt::IToolPanelDeckListener
                                ,public ::sfx2::IToolPanelCompare
{
public:
    static const size_t mnInvalidId = static_cast< size_t >( -1 );

    ToolPanelViewShell_Impl( ToolPanelViewShell& i_rPanelViewShell, ::Window& i_rPanelDeckParent );
    virtual ~ToolPanelViewShell_Impl();

    ToolPanelViewShell& GetAntiImpl() { return m_rPanelViewShell; }

    /** Here the panels are created that are shown in the task pane.
    */
    void Setup();

    /** clean up the instance
    */
    void Cleanup();

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

    Reference< XAccessible >
            CreateAccessible( ::sd::Window& i_rWindow );

    void    ConnectToDockingWindow();

private:
    // IToolPanelDeckListener overridables
    virtual void PanelInserted( const ::svt::PToolPanel& i_pPanel, const size_t i_nPosition );
    virtual void PanelRemoved( const size_t i_nPosition );
    virtual void ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive );
    virtual void LayouterChanged( const ::svt::PDeckLayouter& i_rNewLayouter );
    virtual void Dying();

    // IToolPanelCompare overridables
    virtual short compareToolPanelsURLs( const ::rtl::OUString& i_rLHS, const ::rtl::OUString& i_rRHS ) const;

private:
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
    InitialPanel        impl_determineInitialPanel();
    ::rtl::OUString     impl_getPanelURL( const ::boost::optional< size_t >& i_rPanel );

private:
    ToolPanelViewShell&                             m_rPanelViewShell;
    ::boost::scoped_ptr< ::sfx2::ModuleTaskPane >   m_pTaskPane;
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr< ::sfx2::TaskPaneController >   m_pTaskPaneController;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    ::rtl::Reference< ConfigurationListener >       m_pConfigListener;
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

    osl_atomic_increment( &m_refCount );
    {
        xBroadcaster->addConfigurationChangeListener( this, ::rtl::OUString(), Any() );
    }
    osl_atomic_decrement( &m_refCount );
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
// = ToolPanelViewShell
// =====================================================================================================================
// ---------------------------------------------------------------------------------------------------------------------
SFX_IMPL_INTERFACE(ToolPanelViewShell, SfxShell, SdResId(STR_TASKPANEVIEWSHELL))
{
}

// ---------------------------------------------------------------------------------------------------------------------
TYPEINIT1(ToolPanelViewShell, ViewShell);

// ---------------------------------------------------------------------------------------------------------------------
ToolPanelViewShell_Impl::InitialPanel ToolPanelViewShell_Impl::impl_determineInitialPanel()
{
    InitialPanel aPanelToActivate;
    if ( GetAntiImpl().GetViewShellBase().GetDocShell()->GetDocumentType() == DOCUMENT_TYPE_DRAW )
        // for Draw, rely on SFX's default handling, which is to activate the previously active panel
        return aPanelToActivate;

    // Default to Layout panel, but check whether the requested configuration already contains a tool panel, in this case,
    // use that one.
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
    const InitialPanel aInitialPanel = impl_determineInitialPanel();
    if ( !aInitialPanel.sPanelResourceURL.isEmpty() )
    {
        if ( aInitialPanel.bActivateDirectly )
        {
            ActivatePanelByResource( aInitialPanel.sPanelResourceURL );
        }
        else
        {
            ::boost::shared_ptr< FrameworkHelper > pFrameworkHelper( FrameworkHelper::Instance( GetAntiImpl().GetViewShellBase() ) );
            pFrameworkHelper->RequestTaskPanel( aInitialPanel.sPanelResourceURL );
        }
    }

    // listen at the configuration
    m_pConfigListener.set( new ConfigurationListener( *this ) );

    m_pTaskPane->Show();
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell_Impl::Cleanup()
{
    if ( m_bInitialized )
    {
        if ( m_pConfigListener.is() )
            m_pConfigListener->dispose();
    }
    GetToolPanelDeck().RemoveListener( *this );
    m_pTaskPaneController.reset();
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
    doShow();

    meShellType = ST_TASK_PANE;

    mpContentWindow->SetCenterAllowed( false );
    pParentWindow->SetStyle( pParentWindow->GetStyle() | WB_DIALOGCONTROL );

    GetParentWindow()->SetBackground( Wallpaper() );
    mpContentWindow->SetBackground( Wallpaper() );

    GetParentWindow()->SetHelpId(HID_SD_TASK_PANE);

    mpImpl->ConnectToDockingWindow();

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

    SetName( rtl::OUString( "ToolPanelViewShell" ) );

    // enforce the creation of the Accessible object here.
    // In some not-always-to-reproduce situations, creating the accessible on demand only leads to some
    // cycliy parenthood references between the involved objects, which make some AT tools (accerciser, in particular)
    // loop (which is /not/ a bug in the tool, of course).
    // However, since those situations were not reproducible anymore, we deliberately leave the Accessible creation
    // (which originally was intended as a workaround) herein. Better to be safe ...
    // Note that this is not a performance problem: The implementation of the ToolPanelDeck's Accessible
    // is separated from the implementation of its AccessibleContext (which even is in a separate library) - we only
    // create the former here, the latter is still created on demand, when somebody requests it.
    // #i113671# / 2010-09-17 / frank.schoenheit@oracle.com
    if (mpContentWindow.get())
        mpContentWindow->GetAccessible( sal_True );

    // For accessibility we have to shortly hide the content window.  This
    // triggers the construction of a new accessibility object for the new
    // view shell.  (One is created earlier while the construtor of the base
    // class is executed.  At that time the correct accessibility object can
    // not be constructed.)
    if (mpContentWindow.get())
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
    GetFrameView()->Disconnect();
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

    mpImpl->ConnectToDockingWindow();

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
        rtl::OString            sHelpID;
        PanelFactory( const ControlFactoryFactory i_pFactory, const rtl::OString& i_nHelpID )
            :pFactory( i_pFactory )
            ,sHelpID( i_nHelpID )
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
        throw RuntimeException( "illegal panel ID" , NULL );
    }
}

//---------------------------------------------------------------------------------------------------------------------
Reference< XUIElement > ToolPanelViewShell::CreatePanelUIElement( const Reference< XFrame >& i_rDocFrame, const ::rtl::OUString& i_rPanelResourceURL )
{
    const PanelId ePanelId( GetStandardPanelId( i_rPanelResourceURL ) );
    ENSURE_OR_RETURN( ePanelId != PID_UNKNOWN, "ToolPanelViewShell::CreatePanelUIElement: illegal panel URL!", NULL );

    // a TreeNode which will resemble the panel
    const PanelFactory aPanelFactory( lcl_describePanel( ePanelId ) );
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr< ControlFactory > pControlFactory( (*aPanelFactory.pFactory)( *this ) );
    ::std::auto_ptr< TreeNode > pNode( pControlFactory->CreateControl( mpImpl->GetToolPanelDeck().GetPanelWindowAnchor() ) );
    SAL_WNODEPRECATED_DECLARATIONS_POP
    ENSURE_OR_THROW( ( pNode.get() != NULL ) && ( pNode->GetWindow() != NULL ),
        "illegal node returned by the control factory" );
    pNode->GetWindow()->SetHelpId( aPanelFactory.sHelpID );

    // create an XToolPanel
    Reference< XToolPanel > xPanel( new ToolPanel( pNode ) );

    // create an XUIElement providing this panel
    const Reference< XUIElement > xUIElement( new ToolPanelUIElement( i_rDocFrame, i_rPanelResourceURL, xPanel ) );

    return xUIElement;
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell::ActivatePanel( const ::rtl::OUString& i_rPanelResourceURL )
{
    OSL_ENSURE( i_rPanelResourceURL.indexOf( FrameworkHelper::msTaskPanelURLPrefix ) < 0,
        "ToolPanelViewShell::ActivatePanel: for drawing-framework-controller panels, please use FrameworkHelper::RequestTaskPanel!" );
    mpImpl->ActivatePanelByResource( i_rPanelResourceURL );
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
    ,m_pTaskPane( new ::sfx2::ModuleTaskPane( i_rPanelDeckParent, i_rPanelViewShell.GetViewShellBase().GetViewFrame()->GetFrame().GetFrameInterface(), *this ) )
    ,m_bInitialized( false )
{
    const String sPaneTitle( SdResId( STR_RIGHT_PANE_TITLE ) );
    GetToolPanelDeck().SetAccessibleName( sPaneTitle );
    GetToolPanelDeck().SetAccessibleDescription( sPaneTitle );

    GetToolPanelDeck().AddListener( *this );
}

// ---------------------------------------------------------------------------------------------------------------------
ToolPanelViewShell_Impl::~ToolPanelViewShell_Impl()
{
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell_Impl::PanelInserted( const ::svt::PToolPanel& i_pPanel, const size_t i_nPosition )
{
    // not interested in
    (void)i_pPanel;
    (void)i_nPosition;
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell_Impl::PanelRemoved( const size_t i_nPosition )
{
    // not interested in
    (void)i_nPosition;
}

// ---------------------------------------------------------------------------------------------------------------------
::rtl::OUString ToolPanelViewShell_Impl::impl_getPanelURL( const ::boost::optional< size_t >& i_rPanel )
{
    ::rtl::OUString sPanelURL;
    if ( !!i_rPanel )
    {
        sPanelURL = GetTaskPane().GetPanelResourceURL( *i_rPanel );
        const PanelId ePanelId( GetStandardPanelId( sPanelURL, true ) );
        if ( ePanelId == PID_UNKNOWN )
            sPanelURL = ::rtl::OUString();
    }
    return sPanelURL;
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell_Impl::ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive )
{
    // update the configuration controller, since this change in the active panel might have been triggered by means other
    // than the drawing framework, so it does not yet know about it.

    const ::rtl::OUString sOldPanelURL( impl_getPanelURL( i_rOldActive ) );
    const ::rtl::OUString sNewPanelURL( impl_getPanelURL( i_rNewActive ) );

    const ::boost::shared_ptr< FrameworkHelper > pFrameworkHelper( FrameworkHelper::Instance( GetAntiImpl().GetViewShellBase() ) );
    if ( !sNewPanelURL.isEmpty() )
    {
        // activate the resource belonging to the new panel. This will automatically de-activate the previously active
        // panel resource (since ResourceActivationMode_REPLACE is used)
        pFrameworkHelper->RequestTaskPanel( sNewPanelURL );
    }
    else if ( !sOldPanelURL.isEmpty() )
    {
        // there is no new active panel, or it is not one of our standard panels, i.e. it is not covered by the
        // resource framework. => Deactivate the old resource.
        try
        {
            Reference< XConfigurationController > xConfigController( pFrameworkHelper->GetConfigurationController(), UNO_QUERY_THROW );
            xConfigController->requestResourceDeactivation(
                pFrameworkHelper->CreateResourceId(
                    sOldPanelURL,
                    FrameworkHelper::msTaskPaneURL,
                    FrameworkHelper::msRightPaneURL
                )
            );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
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
    // not interested in
}

// ---------------------------------------------------------------------------------------------------------------------
short ToolPanelViewShell_Impl::compareToolPanelsURLs( const ::rtl::OUString& i_rLHS, const ::rtl::OUString& i_rRHS ) const
{
    const PanelId eLHS( GetStandardPanelId( i_rLHS, true ) );
    const PanelId eRHS( GetStandardPanelId( i_rRHS, true ) );
    if ( eLHS < eRHS )
        return -1;
    if ( eLHS == eRHS )
        return 0;
    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
void ToolPanelViewShell_Impl::ConnectToDockingWindow()
{
    m_pTaskPaneController.reset();
    DockingWindow* pDockingWindow( GetAntiImpl().GetDockingWindow() );
    if ( pDockingWindow )
    {
        PaneDockingWindow* pPaneDockingWindow = dynamic_cast< PaneDockingWindow* >( pDockingWindow );
        OSL_ENSURE( pPaneDockingWindow, "ToolPanelViewShell_Impl::ConnectToDockingWindow: unsupported docking window type!" );
        if ( pPaneDockingWindow != NULL )
            m_pTaskPaneController.reset( new ::sfx2::TaskPaneController( GetTaskPane(), *pPaneDockingWindow ) );
    }

    // Tell the focus manager that we want to pass the focus to our
    // child.
    FocusManager::Instance().RegisterDownLink( GetAntiImpl().GetParentWindow(), &GetTaskPane() );
}

// ---------------------------------------------------------------------------------------------------------------------
Reference< XAccessible > ToolPanelViewShell_Impl::CreateAccessible( ::sd::Window& i_rWindow )
{
    Reference< XAccessible > xAccessible( GetToolPanelDeck().GetAccessible( sal_False ) );
    if ( !xAccessible.is() )
    {
        // determine the XAccessible which is the parent of the to-be-created object
        ::Window* pAccessibleParent = i_rWindow.GetAccessibleParentWindow();
        OSL_ENSURE( pAccessibleParent, "ToolPanelViewShell_Impl::CreateAccessible: illegal accessible parent provided by the sd::Window!" );
        GetToolPanelDeck().SetAccessibleParentWindow( pAccessibleParent );

        xAccessible = GetToolPanelDeck().GetAccessible( sal_True );
        ENSURE_OR_RETURN( xAccessible.is(), "ToolPanelViewShell_Impl::CreateAccessible: illegal ToolPanelDeck accessible!", NULL );
        OSL_ENSURE( xAccessible->getAccessibleContext().is()
                &&  xAccessible->getAccessibleContext()->getAccessibleParent() == pAccessibleParent->GetAccessible(),
                "ToolPanelViewShell_Impl::CreateAccessible: illegal parenthood!" );
    }
    return xAccessible;
}

} } // end of namespace ::sd::toolpanel

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
