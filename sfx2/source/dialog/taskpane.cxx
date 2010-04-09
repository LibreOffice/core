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
    /** === end UNO using === **/
    namespace PosSize = ::com::sun::star::awt::PosSize;

//#define USE_DUMMY_PANEL

#if OSL_DEBUG_LEVEL > 0
    //==================================================================================================================
    //= DummyPanel - declaration
    //==================================================================================================================
    class DummyPanel : public ::svt::ToolPanelBase
    {
    public:
        DummyPanel( Window& i_rParent );
        virtual ~DummyPanel();

        // IToolPanel
        virtual ::rtl::OUString GetDisplayName() const;
        virtual Image GetImage() const;
        virtual void Activate( Window& i_rParentWindow );
        virtual void Deactivate();
        virtual void SetSizePixel( const Size& i_rPanelWindowSize );
        virtual void GrabFocus();
        virtual void Dispose();
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                    CreatePanelAccessible(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& i_rParentAccessible
                    );

        class GreenWindow : public Window
        {
        public:
            GreenWindow( Window& i_rParent )
                :Window( &i_rParent, 0 )
            {
                SetLineColor();
                SetFillColor( COL_GREEN );
            }
            virtual void Paint( const Rectangle& i_rRect )
            {
                DrawRect( i_rRect );
            }
        };

    private:
        ::boost::scoped_ptr< Window >  m_pWindow;
    };

    //==================================================================================================================
    //= DummyPanel - implementation
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    DummyPanel::DummyPanel( Window& i_rParent )
        :m_pWindow( new GreenWindow( i_rParent ) )
    {
        m_pWindow->SetPosSizePixel( Point(), i_rParent.GetOutputSizePixel() );
    }

    //------------------------------------------------------------------------------------------------------------------
    DummyPanel::~DummyPanel()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString DummyPanel::GetDisplayName() const
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Soylent Green" ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    Image DummyPanel::GetImage() const
    {
        return Image();
    }

    //------------------------------------------------------------------------------------------------------------------
    void DummyPanel::Activate( Window& i_rParentWindow )
    {
        OSL_ENSURE( &i_rParentWindow == m_pWindow->GetParent(), "DummyPanel::Activate: reparenting not supported (and not expected to be needed)!" );
        (void)i_rParentWindow;

        m_pWindow->Show();
    }

    //------------------------------------------------------------------------------------------------------------------
    void DummyPanel::Deactivate()
    {
        m_pWindow->Hide();
    }

    //------------------------------------------------------------------------------------------------------------------
    void DummyPanel::SetSizePixel( const Size& i_rPanelWindowSize )
    {
        m_pWindow->SetPosSizePixel( Point(), i_rPanelWindowSize );
    }

    //------------------------------------------------------------------------------------------------------------------
    void DummyPanel::GrabFocus()
    {
        m_pWindow->GrabFocus();
    }

    //------------------------------------------------------------------------------------------------------------------
    void DummyPanel::Dispose()
    {
        m_pWindow.reset();
    }

    //------------------------------------------------------------------------------------------------------------------
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::accessibility::XAccessible;
    Reference< XAccessible > DummyPanel::CreatePanelAccessible( const Reference< XAccessible >& i_rParentAccessible )
    {
        Reference< XAccessible > xPanelAccessible( m_pWindow->GetAccessible( FALSE ) );
        if ( !xPanelAccessible.is() )
        {
            xPanelAccessible = m_pWindow->GetAccessible( TRUE );
            ::comphelper::OAccessibleImplementationAccess::setAccessibleParent( xPanelAccessible->getAccessibleContext(),
                i_rParentAccessible );
        }
        return xPanelAccessible;
    }

#endif

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
        ::rtl::OUString lcl_identifyModule( const SfxBindings* i_pBindings )
        {
            const SfxViewFrame* pViewFrame = i_pBindings->GetDispatcher()->GetFrame();
            const SfxFrame* pFrame = pViewFrame->GetFrame();
            const Reference< XFrame > xFrame( pFrame->GetFrameInterface() );
            return lcl_identifyModule( xFrame );
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
        ,m_aTaskPane( GetContentWindow(), lcl_identifyModule( i_pBindings ) )
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
        CustomToolPanel( const ::utl::OConfigurationNode& i_rPanelWindowState );

        virtual ::rtl::OUString GetDisplayName() const;
        virtual Image GetImage() const;
        virtual void Activate( Window& i_rParentWindow );
        virtual void Deactivate();
        virtual void SetSizePixel( const Size& i_rPanelWindowSize );
        virtual void GrabFocus();
        virtual void Dispose();
        virtual Reference< XAccessible >
                    CreatePanelAccessible( const Reference< XAccessible >& i_rParentAccessible );

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
        CustomPanelUIElement    m_aCustomPanel;
        bool                    m_bAttemptedCreation;
    };

    //------------------------------------------------------------------------------------------------------------------
    CustomToolPanel::CustomToolPanel( const ::utl::OConfigurationNode& i_rPanelWindowState )
        :m_sUIName( ::comphelper::getString( i_rPanelWindowState.getNodeValue( "UIName" ) ) )
        ,m_aPanelImage( lcl_getPanelImage( i_rPanelWindowState ) )
        ,m_sResourceURL( i_rPanelWindowState.getLocalName() )
        ,m_sPanelConfigPath( i_rPanelWindowState.getNodePath() )
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
        ModuleTaskPane_Impl( ModuleTaskPane& i_rAntiImpl, const ::rtl::OUString& i_rModuleIdentifier )
            :m_rAntiImpl( i_rAntiImpl )
            ,m_sModuleIdentifier( i_rModuleIdentifier )
            ,m_aPanels( i_rAntiImpl )
        {
            m_aPanels.Show();
        #if ( OSL_DEBUG_LEVEL > 0 ) && defined ( USE_DUMMY_PANEL )
            m_aPanels.InsertPanel( ::svt::PToolPanel( new DummyPanel( m_aPanels.GetPanelWindowAnchor() ) ), m_aPanels.GetPanelCount() );
        #endif
            OnResize();
            impl_initFromConfiguration();
        }

        ~ModuleTaskPane_Impl()
        {
        }

        void    OnResize();
        void    OnGetFocus();

        static bool ModuleHasToolPanels( const ::rtl::OUString& i_rModuleIdentifier );

    private:
        void    impl_initFromConfiguration();

        static bool
                impl_isToolPanelResource( const ::rtl::OUString& i_rResourceURL );

    private:
        ModuleTaskPane&         m_rAntiImpl;
        ::rtl::OUString         m_sModuleIdentifier;
        ::svt::ToolPanelDeck    m_aPanels;
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
            ::svt::PToolPanel pCustomPanel( new CustomToolPanel( aResourceNode ) );
            size_t nPanelPos = m_aPanels.InsertPanel( pCustomPanel, m_aPanels.GetPanelCount() );

            if ( ::comphelper::getBOOL( aResourceNode.getNodeValue( "Visible" ) ) )
                nFirstVisiblePanel = nPanelPos;
        }

        if ( nFirstVisiblePanel != size_t( -1 ) )
        {
            m_aPanels.ActivatePanel( nFirstVisiblePanel );
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

    //==================================================================================================================
    //= ModuleTaskPane
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    ModuleTaskPane::ModuleTaskPane( Window& i_rParentWindow, const ::rtl::OUString& i_rModuleIdentifier )
        :Window( &i_rParentWindow, 0 )
        ,m_pImpl( new ModuleTaskPane_Impl( *this, i_rModuleIdentifier ) )
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

//......................................................................................................................
} // namespace sfx2
//......................................................................................................................
