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

#include "precompiled_sfx2.hxx"

#include "sfx2/taskpane.hxx"
#include "sfx2/sfxsids.hrc"
#include "sfx2/bindings.hxx"
#include "sfx2/dispatch.hxx"
#include "sfxresid.hxx"
#include "helpid.hrc"

/** === begin UNO includes === **/
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/ui/XToolPanel.hpp>
#include <com/sun/star/ui/XUIElementFactory.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
/** === end UNO includes === **/

#include <comphelper/componentcontext.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/types.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/diagnose_ex.h>
#include <svtools/toolpanel/toolpaneldeck.hxx>
#include <svtools/toolpanel/tablayouter.hxx>
#include <svtools/toolpanel/drawerlayouter.hxx>
#include <unotools/confignode.hxx>

#if OSL_DEBUG_LEVEL > 0
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <comphelper/accimplaccess.hxx>
#endif

#include <boost/noncopyable.hpp>

//......................................................................................................................
namespace sfx2
{
//......................................................................................................................

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
    using ::com::sun::star::frame::XModuleManager;
    using ::com::sun::star::container::XNameAccess;
    using ::com::sun::star::ui::XToolPanel;
    using ::com::sun::star::ui::XUIElementFactory;
    using ::com::sun::star::ui::XUIElement;
    using ::com::sun::star::awt::XWindow;
    using ::com::sun::star::frame::XModuleManager;
    using ::com::sun::star::frame::XFrame;
    using ::com::sun::star::lang::XComponent;
    using ::com::sun::star::graphic::XGraphicProvider;
    using ::com::sun::star::graphic::XGraphic;
    using ::com::sun::star::accessibility::XAccessible;
    /** === end UNO using === **/
    namespace PosSize = ::com::sun::star::awt::PosSize;

    //==================================================================================================================
    //= helpers
    //==================================================================================================================
    namespace
    {
        //--------------------------------------------------------------------------------------------------------------
        ::utl::OConfigurationTreeRoot lcl_getModuleUIElementStatesConfig( const ::rtl::OUString& i_rModuleIdentifier,
            const ::rtl::OUString& i_rResourceURL = ::rtl::OUString() )
        {
            const ::comphelper::ComponentContext aContext( ::comphelper::getProcessServiceFactory() );
            ::rtl::OUStringBuffer aPathComposer;
            try
            {
                const Reference< XNameAccess > xModuleAccess( aContext.createComponent( "com.sun.star.frame.ModuleManager" ), UNO_QUERY_THROW );
                const ::comphelper::NamedValueCollection aModuleProps( xModuleAccess->getByName( i_rModuleIdentifier ) );

                const ::rtl::OUString sWindowStateRef( aModuleProps.getOrDefault( "ooSetupFactoryWindowStateConfigRef", ::rtl::OUString() ) );

                aPathComposer.appendAscii( "org.openoffice.Office.UI." );
                aPathComposer.append( sWindowStateRef );
                aPathComposer.appendAscii( "/UIElements/States" );
                if ( i_rResourceURL.getLength() )
                {
                    aPathComposer.appendAscii( "/" );
                    aPathComposer.append( i_rResourceURL );
                }
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            return ::utl::OConfigurationTreeRoot( aContext, aPathComposer.makeStringAndClear(), false );
        }

        //--------------------------------------------------------------------------------------------------------------
        ::rtl::OUString lcl_identifyModule( const Reference< XFrame >& i_rDocumentFrame )
        {
            ::rtl::OUString sModuleName;
            try
            {
                const ::comphelper::ComponentContext aContext( ::comphelper::getProcessServiceFactory() );
                const Reference< XModuleManager > xModuleManager( aContext.createComponent( "com.sun.star.frame.ModuleManager" ), UNO_QUERY_THROW );
                sModuleName = xModuleManager->identify( i_rDocumentFrame );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            return sModuleName;
        }

        //--------------------------------------------------------------------------------------------------------------
        Reference< XFrame > lcl_getFrame( const SfxBindings* i_pBindings )
        {
            const SfxViewFrame* pViewFrame = i_pBindings->GetDispatcher()->GetFrame();
            const SfxFrame* pFrame = pViewFrame->GetFrame();
            const Reference< XFrame > xFrame( pFrame->GetFrameInterface() );
            return xFrame;
        }

        //--------------------------------------------------------------------------------------------------------------
        Image lcl_getPanelImage( const ::utl::OConfigurationNode& i_rPanelConfigNode )
        {
            const ::rtl::OUString sImageURL( ::comphelper::getString( i_rPanelConfigNode.getNodeValue( "ImageURL" ) ) );
            if ( sImageURL.getLength() )
            {
                try
                {
                    const ::comphelper::ComponentContext aContext( ::comphelper::getProcessServiceFactory() );
                    const Reference< XGraphicProvider > xGraphicProvider( aContext.createComponent( "com.sun.star.graphic.GraphicProvider" ), UNO_QUERY_THROW );

                    ::comphelper::NamedValueCollection aMediaProperties;
                    aMediaProperties.put( "URL", sImageURL );

                    const Reference< XGraphic > xGraphic( xGraphicProvider->queryGraphic( aMediaProperties.getPropertyValues() ), UNO_SET_THROW );
                    return Image( xGraphic );
                }
                catch( const Exception& )
                {
                    DBG_UNHANDLED_EXCEPTION();
                }
            }
            return Image();
        }
    }

    //==================================================================================================================
    //= TaskPaneDockingWindow
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    TaskPaneDockingWindow::TaskPaneDockingWindow( SfxBindings* i_pBindings, TaskPaneWrapper& i_rWrapper, Window* i_pParent, WinBits i_nBits )
        :TitledDockingWindow( i_pBindings, &i_rWrapper, i_pParent, i_nBits )
        ,m_aTaskPane( GetContentWindow(), lcl_getFrame( i_pBindings ) )
    {
        m_aTaskPane.Show();
        SetText( String( SfxResId( SID_TASKPANE ) ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    void TaskPaneDockingWindow::GetFocus()
    {
        TitledDockingWindow::GetFocus();
        m_aTaskPane.GrabFocus();
    }

    //------------------------------------------------------------------------------------------------------------------
    void TaskPaneDockingWindow::onLayoutDone()
    {
        m_aTaskPane.SetPosSizePixel( Point(), GetContentWindow().GetOutputSizePixel() );
    }

    //------------------------------------------------------------------------------------------------------------------
    long TaskPaneDockingWindow::Notify( NotifyEvent& i_rNotifyEvent )
    {
        // in case this is a MouseButtonDown event, consume it - VCL's DockingWindow would otherwise
        // start undocking, no matter which window this event was intended for
        if ( i_rNotifyEvent.GetType() == EVENT_MOUSEBUTTONDOWN )
        {
            const MouseEvent& rMouseEvent = *i_rNotifyEvent.GetMouseEvent();
            if ( rMouseEvent.IsLeft() && ( rMouseEvent.GetClicks() == 1 ) && !rMouseEvent.IsMod1() )
            {
                if ( m_aTaskPane.IsWindowOrChild( i_rNotifyEvent.GetWindow() ) )
                    return TRUE;
            }
        }
        return TitledDockingWindow::Notify( i_rNotifyEvent );
    }

    //==================================================================================================================
    //= TaskPaneWrapper
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    SFX_IMPL_DOCKINGWINDOW( TaskPaneWrapper, SID_TASKPANE );

    //------------------------------------------------------------------------------------------------------------------
    TaskPaneWrapper::TaskPaneWrapper( Window* i_pParent, USHORT i_nId, SfxBindings* i_pBindings, SfxChildWinInfo* i_pInfo )
        :SfxChildWindow( i_pParent, i_nId )
    {
        pWindow = new TaskPaneDockingWindow( i_pBindings, *this, i_pParent,
            WB_STDDOCKWIN | WB_CLIPCHILDREN | WB_SIZEABLE | WB_3DLOOK | WB_ROLLABLE);
        eChildAlignment = SFX_ALIGN_RIGHT;

        pWindow->SetHelpId( HID_TASKPANE_WINDOW );
        pWindow->SetOutputSizePixel( Size( 300, 450 ) );
        pWindow->Show();

        dynamic_cast< SfxDockingWindow* >( pWindow )->Initialize( i_pInfo );
        SetHideNotDelete( TRUE );
    }

    //==================================================================================================================
    //= CustomPanelUIElement
    //==================================================================================================================
    class CustomPanelUIElement
    {
    public:
        CustomPanelUIElement()
            :m_xUIElement()
            ,m_xToolPanel()
            ,m_xPanelWindow()
        {
        }

        CustomPanelUIElement( const Reference< XUIElement >& i_rUIElement )
            :m_xUIElement( i_rUIElement, UNO_SET_THROW )
            ,m_xToolPanel( i_rUIElement->getRealInterface(), UNO_QUERY_THROW )
            ,m_xPanelWindow( m_xToolPanel->getWindow(), UNO_SET_THROW )
        {
        }

        bool is() const { return m_xPanelWindow.is(); }

        const Reference< XUIElement >&  getUIElement() const { return m_xUIElement; }
        const Reference< XToolPanel >&  getToolPanel() const { return m_xToolPanel; }
        const Reference< XWindow >&     getPanelWindow() const { return m_xPanelWindow; }

    private:
        Reference< XUIElement > m_xUIElement;
        Reference< XToolPanel > m_xToolPanel;
        Reference< XWindow >    m_xPanelWindow;
    };

    //==================================================================================================================
    //= CustomToolPanel
    //==================================================================================================================
    class CustomToolPanel : public ::svt::ToolPanelBase
    {
    public:
        CustomToolPanel( const ::utl::OConfigurationNode& i_rPanelWindowState, const Reference< XFrame >& i_rFrame );

        virtual ::rtl::OUString GetDisplayName() const;
        virtual Image GetImage() const;
        virtual void Activate( Window& i_rParentWindow );
        virtual void Deactivate();
        virtual void SetSizePixel( const Size& i_rPanelWindowSize );
        virtual void GrabFocus();
        virtual void Dispose();
        virtual Reference< XAccessible >
                    CreatePanelAccessible( const Reference< XAccessible >& i_rParentAccessible );

        const ::rtl::OUString&
                    GetResourceURL() const { return m_sResourceURL; }

    protected:
        ~CustomToolPanel();

    private:
        bool    impl_ensureToolPanelWindow( Window& i_rPanelParentWindow );
        void    impl_updatePanelConfig( const bool i_bVisible ) const;

    private:
        const ::rtl::OUString   m_sUIName;
        const Image             m_aPanelImage;
        const ::rtl::OUString   m_sResourceURL;
        const ::rtl::OUString   m_sPanelConfigPath;
        Reference< XFrame >     m_xFrame;
        CustomPanelUIElement    m_aCustomPanel;
        bool                    m_bAttemptedCreation;
    };

    //------------------------------------------------------------------------------------------------------------------
    CustomToolPanel::CustomToolPanel( const ::utl::OConfigurationNode& i_rPanelWindowState, const Reference< XFrame >& i_rFrame )
        :m_sUIName( ::comphelper::getString( i_rPanelWindowState.getNodeValue( "UIName" ) ) )
        ,m_aPanelImage( lcl_getPanelImage( i_rPanelWindowState ) )
        ,m_sResourceURL( i_rPanelWindowState.getLocalName() )
        ,m_sPanelConfigPath( i_rPanelWindowState.getNodePath() )
        ,m_xFrame( i_rFrame )
        ,m_aCustomPanel()
        ,m_bAttemptedCreation( false )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    CustomToolPanel::~CustomToolPanel()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    bool CustomToolPanel::impl_ensureToolPanelWindow( Window& i_rPanelParentWindow )
    {
        if ( m_bAttemptedCreation )
            return m_aCustomPanel.is();

        m_bAttemptedCreation = true;
        try
        {
            const ::comphelper::ComponentContext aContext( ::comphelper::getProcessServiceFactory() );
            const Reference< XUIElementFactory > xFactory( aContext.createComponent( "com.sun.star.ui.UIElementFactoryManager" ), UNO_QUERY_THROW );

            ::comphelper::NamedValueCollection aCreationArgs;
            aCreationArgs.put( "Frame", makeAny( m_xFrame ) );
            aCreationArgs.put( "ParentWindow", makeAny( i_rPanelParentWindow.GetComponentInterface() ) );

            const Reference< XUIElement > xElement(
                xFactory->createUIElement( m_sResourceURL, aCreationArgs.getPropertyValues() ),
                UNO_SET_THROW );

            m_aCustomPanel = CustomPanelUIElement( xElement );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return m_aCustomPanel.is();
    }

    //------------------------------------------------------------------------------------------------------------------
    void CustomToolPanel::impl_updatePanelConfig( const bool i_bVisible ) const
    {
        ::comphelper::ComponentContext aContext( ::comphelper::getProcessServiceFactory() );
        ::utl::OConfigurationTreeRoot aConfig( aContext, m_sPanelConfigPath, true );

        aConfig.setNodeValue( "Visible", makeAny( i_bVisible ) );
        aConfig.commit();
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString CustomToolPanel::GetDisplayName() const
    {
        return m_sUIName;
    }

    //------------------------------------------------------------------------------------------------------------------
    Image CustomToolPanel::GetImage() const
    {
        return m_aPanelImage;
    }

    //------------------------------------------------------------------------------------------------------------------
    void CustomToolPanel::Activate( Window& i_rParentWindow )
    {
        ENSURE_OR_RETURN_VOID( impl_ensureToolPanelWindow( i_rParentWindow ), "no panel to activate!" );

        // TODO: we might need a mechanism to decide whether the panel should be destroyed/re-created, or (as it is
        // done now) hidden/shown
        m_aCustomPanel.getPanelWindow()->setVisible( sal_True );

        // update the panel's configuration
        impl_updatePanelConfig( true );
    }

    //------------------------------------------------------------------------------------------------------------------
    void CustomToolPanel::Deactivate()
    {
        ENSURE_OR_RETURN_VOID( m_aCustomPanel.is(), "no panel to deactivate!" );

        m_aCustomPanel.getPanelWindow()->setVisible( sal_False );

        // update the panel's configuration
        impl_updatePanelConfig( false );
    }

    //------------------------------------------------------------------------------------------------------------------
    void CustomToolPanel::SetSizePixel( const Size& i_rPanelWindowSize )
    {
        ENSURE_OR_RETURN_VOID( m_aCustomPanel.is(), "no panel/window to position!" );

        try
        {
            m_aCustomPanel.getPanelWindow()->setPosSize( 0, 0, i_rPanelWindowSize.Width(), i_rPanelWindowSize.Height(),
                PosSize::POSSIZE );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void CustomToolPanel::GrabFocus()
    {
        ENSURE_OR_RETURN_VOID( m_aCustomPanel.is(), "no panel/window to focus!" );

        m_aCustomPanel.getPanelWindow()->setFocus();
    }

    //------------------------------------------------------------------------------------------------------------------
    void CustomToolPanel::Dispose()
    {
        if ( !m_bAttemptedCreation )
            // nothing to dispose
            return;

        ENSURE_OR_RETURN_VOID( m_aCustomPanel.is(), "no panel to destroy!" );
        try
        {
            Reference< XComponent > xUIElementComponent( m_aCustomPanel.getUIElement(), UNO_QUERY_THROW );
            xUIElementComponent->dispose();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XAccessible > CustomToolPanel::CreatePanelAccessible( const Reference< XAccessible >& i_rParentAccessible )
    {
        ENSURE_OR_RETURN( m_aCustomPanel.is(), "no panel to ask!", NULL );

        Reference< XAccessible > xPanelAccessible;
        try
        {
            xPanelAccessible.set( m_aCustomPanel.getToolPanel()->createAccessible( i_rParentAccessible ), UNO_SET_THROW );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return xPanelAccessible;
    }

    //==================================================================================================================
    //= ModuleTaskPane_Impl
    //==================================================================================================================
    class ModuleTaskPane_Impl : public ::boost::noncopyable
    {
    public:
        ModuleTaskPane_Impl( ModuleTaskPane& i_rAntiImpl, const Reference< XFrame >& i_rDocumentFrame )
            :m_rAntiImpl( i_rAntiImpl )
            ,m_sModuleIdentifier( lcl_identifyModule( i_rDocumentFrame ) )
            ,m_xFrame( i_rDocumentFrame )
            ,m_aPanels( i_rAntiImpl )
        {
            m_aPanels.Show();
            OnResize();
            impl_initFromConfiguration();
        }

        ~ModuleTaskPane_Impl()
        {
        }

        void    OnResize();
        void    OnGetFocus();

        static bool ModuleHasToolPanels( const ::rtl::OUString& i_rModuleIdentifier );

              ::svt::ToolPanelDeck& GetPanelDeck()          { return m_aPanels; }
        const ::svt::ToolPanelDeck& GetPanelDeck() const    { return m_aPanels; }

        ::boost::optional< size_t >
                    GetPanelPos( const ::rtl::OUString& i_rResourceURL );

        void        SetDrawersLayout();
        void        SetTabsLayout( const ::svt::TabAlignment i_eTabAlignment, const ::svt::TabItemContent i_eTabContent );

    private:
        void    impl_initFromConfiguration();

        static bool
                impl_isToolPanelResource( const ::rtl::OUString& i_rResourceURL );

        DECL_LINK( OnActivatePanel, void* );

    private:
        ModuleTaskPane&             m_rAntiImpl;
        const ::rtl::OUString       m_sModuleIdentifier;
        const Reference< XFrame >   m_xFrame;
        ::svt::ToolPanelDeck        m_aPanels;
    };

    //------------------------------------------------------------------------------------------------------------------
    void ModuleTaskPane_Impl::OnResize()
    {
        m_aPanels.SetPosSizePixel( Point(), m_rAntiImpl.GetOutputSizePixel() );
    }

    //------------------------------------------------------------------------------------------------------------------
    void ModuleTaskPane_Impl::OnGetFocus()
    {
        m_aPanels.GrabFocus();
    }

    //------------------------------------------------------------------------------------------------------------------
    IMPL_LINK( ModuleTaskPane_Impl, OnActivatePanel, void*, i_pArg )
    {
        m_aPanels.ActivatePanel( reinterpret_cast< size_t >( i_pArg ) );
        return 1L;
    }

    //------------------------------------------------------------------------------------------------------------------
    bool ModuleTaskPane_Impl::impl_isToolPanelResource( const ::rtl::OUString& i_rResourceURL )
    {
        return i_rResourceURL.matchAsciiL( RTL_CONSTASCII_STRINGPARAM( "private:resource/toolpanel/" ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    void ModuleTaskPane_Impl::impl_initFromConfiguration()
    {
        const ::utl::OConfigurationTreeRoot aWindowStateConfig( lcl_getModuleUIElementStatesConfig( m_sModuleIdentifier ) );
        if ( !aWindowStateConfig.isValid() )
            return;

        size_t nFirstVisiblePanel = size_t( -1 );

        const Sequence< ::rtl::OUString > aUIElements( aWindowStateConfig.getNodeNames() );
        for (   const ::rtl::OUString* resource = aUIElements.getConstArray();
                resource != aUIElements.getConstArray() + aUIElements.getLength();
                ++resource
            )
        {
            if ( !impl_isToolPanelResource( *resource ) )
                continue;

            ::utl::OConfigurationNode aResourceNode( aWindowStateConfig.openNode( *resource ) );
            ::svt::PToolPanel pCustomPanel( new CustomToolPanel( aResourceNode, m_xFrame ) );
            size_t nPanelPos = m_aPanels.InsertPanel( pCustomPanel, m_aPanels.GetPanelCount() );

            if ( ::comphelper::getBOOL( aResourceNode.getNodeValue( "Visible" ) ) )
                nFirstVisiblePanel = nPanelPos;
        }

        if ( nFirstVisiblePanel != size_t( -1 ) )
        {
            m_rAntiImpl.PostUserEvent( LINK( this, ModuleTaskPane_Impl, OnActivatePanel ), reinterpret_cast< void* >( nFirstVisiblePanel ) );
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    bool ModuleTaskPane_Impl::ModuleHasToolPanels( const ::rtl::OUString& i_rModuleIdentifier )
    {
        const ::utl::OConfigurationTreeRoot aWindowStateConfig( lcl_getModuleUIElementStatesConfig( i_rModuleIdentifier ) );
        if ( !aWindowStateConfig.isValid() )
            return false;

        const Sequence< ::rtl::OUString > aUIElements( aWindowStateConfig.getNodeNames() );
        for (   const ::rtl::OUString* resource = aUIElements.getConstArray();
                resource != aUIElements.getConstArray() + aUIElements.getLength();
                ++resource
            )
        {
            if ( impl_isToolPanelResource( *resource ) )
                return true;
        }
        return false;
    }

    //------------------------------------------------------------------------------------------------------------------
    ::boost::optional< size_t > ModuleTaskPane_Impl::GetPanelPos( const ::rtl::OUString& i_rResourceURL )
    {
        ::boost::optional< size_t > aPanelPos;
        for ( size_t i = 0; i < m_aPanels.GetPanelCount(); ++i )
        {
            const ::svt::PToolPanel pPanel( m_aPanels.GetPanel( i ) );
            const CustomToolPanel* pCustomPanel = dynamic_cast< const CustomToolPanel* >( pPanel.get() );
            ENSURE_OR_CONTINUE( pCustomPanel != NULL, "ModuleTaskPane_Impl::GetPanelPos: illegal panel implementation!" );
            if ( pCustomPanel->GetResourceURL() == i_rResourceURL )
            {
                aPanelPos = i;
                break;
            }
        }
        return aPanelPos;
    }

    //------------------------------------------------------------------------------------------------------------------
    void ModuleTaskPane_Impl::SetDrawersLayout()
    {
        const ::svt::PDeckLayouter pLayouter( m_aPanels.GetLayouter() );
        const ::svt::DrawerDeckLayouter* pDrawerLayouter = dynamic_cast< const ::svt::DrawerDeckLayouter* >( pLayouter.get() );
        if ( pDrawerLayouter != NULL )
            // already have the proper layout
            return;
        m_aPanels.SetLayouter( new ::svt::DrawerDeckLayouter( m_aPanels, m_aPanels ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    void ModuleTaskPane_Impl::SetTabsLayout( const ::svt::TabAlignment i_eTabAlignment, const ::svt::TabItemContent i_eTabContent )
    {
        ::svt::PDeckLayouter pLayouter( m_aPanels.GetLayouter() );
        ::svt::TabDeckLayouter* pTabLayouter = dynamic_cast< ::svt::TabDeckLayouter* >( pLayouter.get() );
        if  (   ( pTabLayouter != NULL )
            &&  ( pTabLayouter->GetTabAlignment() == i_eTabAlignment )
            &&  ( pTabLayouter->GetTabItemContent() == i_eTabContent )
            )
            // already have the requested layout
            return;

        if ( pTabLayouter && ( pTabLayouter->GetTabAlignment() == i_eTabAlignment ) )
        {
            // changing only the item content does not require a new layouter instance
            pTabLayouter->SetTabItemContent( i_eTabContent );
            return;
        }

        m_aPanels.SetLayouter( new ::svt::TabDeckLayouter( m_aPanels, m_aPanels, i_eTabAlignment, i_eTabContent ) );
    }

    //==================================================================================================================
    //= ModuleTaskPane
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    ModuleTaskPane::ModuleTaskPane( Window& i_rParentWindow, const Reference< XFrame >& i_rDocumentFrame )
        :Window( &i_rParentWindow, 0 )
        ,m_pImpl( new ModuleTaskPane_Impl( *this, i_rDocumentFrame ) )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    ModuleTaskPane::~ModuleTaskPane()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    bool ModuleTaskPane::ModuleHasToolPanels( const ::rtl::OUString& i_rModuleIdentifier )
    {
        return ModuleTaskPane_Impl::ModuleHasToolPanels( i_rModuleIdentifier );
    }

    //------------------------------------------------------------------------------------------------------------------
    bool ModuleTaskPane::ModuleHasToolPanels( const Reference< XFrame >& i_rDocumentFrame )
    {
        return ModuleTaskPane_Impl::ModuleHasToolPanels( lcl_identifyModule( i_rDocumentFrame ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    void ModuleTaskPane::Resize()
    {
        Window::Resize();
        m_pImpl->OnResize();
    }

    //------------------------------------------------------------------------------------------------------------------
    void ModuleTaskPane::GetFocus()
    {
        Window::GetFocus();
        m_pImpl->OnGetFocus();
    }

    //------------------------------------------------------------------------------------------------------------------
    ::svt::ToolPanelDeck& ModuleTaskPane::GetPanelDeck()
    {
        return m_pImpl->GetPanelDeck();
    }

    //------------------------------------------------------------------------------------------------------------------
    const ::svt::ToolPanelDeck& ModuleTaskPane::GetPanelDeck() const
    {
        return m_pImpl->GetPanelDeck();
    }

    //------------------------------------------------------------------------------------------------------------------
    ::boost::optional< size_t > ModuleTaskPane::GetPanelPos( const ::rtl::OUString& i_rResourceURL )
    {
        return m_pImpl->GetPanelPos( i_rResourceURL );
    }

    //------------------------------------------------------------------------------------------------------------------
    void ModuleTaskPane::SetDrawersLayout()
    {
        m_pImpl->SetDrawersLayout();
    }

    //------------------------------------------------------------------------------------------------------------------
    void ModuleTaskPane::SetTabsLayout( const ::svt::TabAlignment i_eTabAlignment, const ::svt::TabItemContent i_eTabContent )
    {
        m_pImpl->SetTabsLayout( i_eTabAlignment, i_eTabContent );
    }

//......................................................................................................................
} // namespace sfx2
//......................................................................................................................
