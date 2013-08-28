/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "sfx2/taskpane.hxx"
#include "imagemgr.hxx"
#include "sfx2/sfxsids.hrc"
#include "sfx2/bindings.hxx"
#include "sfx2/dispatch.hxx"
#include "sfxresid.hxx"
#include "sfxlocal.hrc"
#include "helpid.hrc"

#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/ui/XToolPanel.hpp>
#include <com/sun/star/ui/XUIElementFactory.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/ui/UIElementFactoryManager.hpp>

#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/types.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/diagnose_ex.h>
#include <svtools/miscopt.hxx>
#include <svtools/toolpanel/toolpaneldeck.hxx>
#include <svtools/toolpanel/tablayouter.hxx>
#include <svtools/toolpanel/drawerlayouter.hxx>
#include <unotools/confignode.hxx>
#include <vcl/menu.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/urlobj.hxx>
#include <boost/noncopyable.hpp>

//......................................................................................................................
namespace sfx2
{
//......................................................................................................................

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XComponentContext;
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
    using ::com::sun::star::frame::ModuleManager;
    using ::com::sun::star::frame::XModuleManager2;
    using ::com::sun::star::container::XNameAccess;
    using ::com::sun::star::ui::XToolPanel;
    using ::com::sun::star::ui::XUIElementFactory;
    using ::com::sun::star::ui::XUIElementFactoryManager;
    using ::com::sun::star::ui::UIElementFactoryManager;
    using ::com::sun::star::ui::XUIElement;
    using ::com::sun::star::awt::XWindow;
    using ::com::sun::star::frame::XFrame;
    using ::com::sun::star::lang::XComponent;
    using ::com::sun::star::graphic::XGraphicProvider;
    using ::com::sun::star::graphic::XGraphic;
    using ::com::sun::star::accessibility::XAccessible;
    using ::com::sun::star::awt::XControl;

    namespace PosSize = ::com::sun::star::awt::PosSize;

    //==================================================================================================================
    //= helpers
    //==================================================================================================================
    namespace
    {
        //--------------------------------------------------------------------------------------------------------------
        ::utl::OConfigurationTreeRoot lcl_getModuleUIElementStatesConfig( const OUString& i_rModuleIdentifier,
            const OUString& i_rResourceURL = OUString() )
        {
            const Reference<XComponentContext> xContext( ::comphelper::getProcessComponentContext() );
            OUStringBuffer aPathComposer;
            try
            {
                const Reference< XModuleManager2 > xModuleAccess( ModuleManager::create(xContext) );
                const ::comphelper::NamedValueCollection aModuleProps( xModuleAccess->getByName( i_rModuleIdentifier ) );

                const OUString sWindowStateRef( aModuleProps.getOrDefault( "ooSetupFactoryWindowStateConfigRef", OUString() ) );

                aPathComposer.appendAscii(RTL_CONSTASCII_STRINGPARAM(
                    "org.openoffice.Office.UI."));
                aPathComposer.append( sWindowStateRef );
                aPathComposer.appendAscii(RTL_CONSTASCII_STRINGPARAM("/UIElements/States"));
                if ( !i_rResourceURL.isEmpty() )
                {
                    aPathComposer.append('/').append( i_rResourceURL );
                }
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            return ::utl::OConfigurationTreeRoot( xContext, aPathComposer.makeStringAndClear(), false );
        }

        //--------------------------------------------------------------------------------------------------------------
        OUString lcl_identifyModule( const Reference< XFrame >& i_rDocumentFrame )
        {
            OUString sModuleName;
            try
            {
                const Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
                const Reference< XModuleManager2 > xModuleManager( ModuleManager::create(xContext) );
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
            const SfxFrame& rFrame = pViewFrame->GetFrame();
            const Reference< XFrame > xFrame( rFrame.GetFrameInterface() );
            return xFrame;
        }

        //--------------------------------------------------------------------------------------------------------------
        OUString lcl_getPanelHelpURL( const ::utl::OConfigurationNode& i_rPanelConfigNode )
        {
            const OUString sHelpURL( ::comphelper::getString( i_rPanelConfigNode.getNodeValue( "HelpURL" ) ) );
            return sHelpURL;
        }

        //--------------------------------------------------------------------------------------------------------------
        Image lcl_getPanelImage( const Reference< XFrame >& i_rDocFrame, const ::utl::OConfigurationNode& i_rPanelConfigNode )
        {
            const OUString sImageURL( ::comphelper::getString( i_rPanelConfigNode.getNodeValue( "ImageURL" ) ) );
            if ( !sImageURL.isEmpty() )
            {
                try
                {
                    ::comphelper::NamedValueCollection aMediaProperties;
                    aMediaProperties.put( "URL", sImageURL );

                    // special handling: if the ImageURL denotes a CommandName, then retrieve the image for that command
                    static const sal_Char pCommandImagePrefix[] = "private:commandimage/";
                    const sal_Int32 nCommandImagePrefixLen = strlen( pCommandImagePrefix );
                    if ( sImageURL.startsWith( pCommandImagePrefix ) )
                    {
                        OUStringBuffer aCommandName;
                        aCommandName.appendAscii(RTL_CONSTASCII_STRINGPARAM(".uno:"));
                        aCommandName.append( sImageURL.copy( nCommandImagePrefixLen ) );
                        const OUString sCommandName( aCommandName.makeStringAndClear() );

                        const Image aPanelImage( GetImage( i_rDocFrame, sCommandName, sal_False ) );
                        return aPanelImage.GetXGraphic();
                    }

                    // otherwise, delegate to the GraphicProvider
                    const Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
                    const Reference< XGraphicProvider > xGraphicProvider( com::sun::star::graphic::GraphicProvider::create(xContext) );

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
        ,m_aPaneController( m_aTaskPane, *this )
    {
        m_aTaskPane.Show();
        SetText( SfxResId( SID_TASKPANE ).toString() );
    }

    //------------------------------------------------------------------------------------------------------------------
    void TaskPaneDockingWindow::ActivateToolPanel( const OUString& i_rPanelURL )
    {
        m_aPaneController.ActivateToolPanel( i_rPanelURL );
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

    //==================================================================================================================
    //= TaskPaneWrapper
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    SFX_IMPL_DOCKINGWINDOW( TaskPaneWrapper, SID_TASKPANE );

    //------------------------------------------------------------------------------------------------------------------
    TaskPaneWrapper::TaskPaneWrapper( Window* i_pParent, sal_uInt16 i_nId, SfxBindings* i_pBindings, SfxChildWinInfo* i_pInfo )
        :SfxChildWindow( i_pParent, i_nId )
    {
        pWindow = new TaskPaneDockingWindow( i_pBindings, *this, i_pParent,
            WB_STDDOCKWIN | WB_CLIPCHILDREN | WB_SIZEABLE | WB_3DLOOK | WB_ROLLABLE);
        eChildAlignment = SFX_ALIGN_RIGHT;

        pWindow->SetHelpId( HID_TASKPANE_WINDOW );
        pWindow->SetOutputSizePixel( Size( 300, 450 ) );

        dynamic_cast< SfxDockingWindow* >( pWindow )->Initialize( i_pInfo );
        SetHideNotDelete( sal_True );

        pWindow->Show();
    }

    //------------------------------------------------------------------------------------------------------------------
    void TaskPaneWrapper::ActivateToolPanel( const OUString& i_rPanelURL )
    {
        TaskPaneDockingWindow* pDockingWindow = dynamic_cast< TaskPaneDockingWindow* >( GetWindow() );
        ENSURE_OR_RETURN_VOID( pDockingWindow, "TaskPaneWrapper::ActivateToolPanel: invalid docking window implementation!" );
        pDockingWindow->ActivateToolPanel( i_rPanelURL );
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
            :m_xUIElement( i_rUIElement )
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

        virtual OUString GetDisplayName() const;
        virtual Image GetImage() const;
        virtual OString GetHelpID() const;
        virtual void Activate( Window& i_rParentWindow );
        virtual void Deactivate();
        virtual void SetSizePixel( const Size& i_rPanelWindowSize );
        virtual void GrabFocus();
        virtual void Dispose();
        virtual Reference< XAccessible >
                    CreatePanelAccessible( const Reference< XAccessible >& i_rParentAccessible );

        const OUString&
                    GetResourceURL() const { return m_sResourceURL; }

    protected:
        ~CustomToolPanel();

    private:
        bool    impl_ensureToolPanelWindow( Window& i_rPanelParentWindow );
        void    impl_updatePanelConfig( const bool i_bVisible ) const;

    private:
        const OUString   m_sUIName;
        const Image             m_aPanelImage;
        const OUString   m_aPanelHelpURL;
        const OUString   m_sResourceURL;
        const OUString   m_sPanelConfigPath;
        Reference< XFrame >     m_xFrame;
        CustomPanelUIElement    m_aCustomPanel;
        bool                    m_bAttemptedCreation;
    };

    //------------------------------------------------------------------------------------------------------------------
    CustomToolPanel::CustomToolPanel( const ::utl::OConfigurationNode& i_rPanelWindowState, const Reference< XFrame >& i_rFrame )
        :m_sUIName( ::comphelper::getString( i_rPanelWindowState.getNodeValue( "UIName" ) ) )
        ,m_aPanelImage( lcl_getPanelImage( i_rFrame, i_rPanelWindowState ) )
        ,m_aPanelHelpURL( lcl_getPanelHelpURL( i_rPanelWindowState ) )
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
            const Reference< XUIElementFactoryManager > xFactory = UIElementFactoryManager::create( ::comphelper::getProcessComponentContext() );

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
        ::utl::OConfigurationTreeRoot aConfig( ::comphelper::getProcessComponentContext(), m_sPanelConfigPath, true );

        aConfig.setNodeValue( "Visible", makeAny( i_bVisible ) );
        aConfig.commit();
    }

    //------------------------------------------------------------------------------------------------------------------
    OUString CustomToolPanel::GetDisplayName() const
    {
        return m_sUIName;
    }

    //------------------------------------------------------------------------------------------------------------------
    Image CustomToolPanel::GetImage() const
    {
        return m_aPanelImage;
    }

    static OString lcl_getHelpId( const OUString& _rHelpURL )
    {
        INetURLObject aHID( _rHelpURL );
        if ( aHID.GetProtocol() == INET_PROT_HID )
            return OUStringToOString( aHID.GetURLPath(), RTL_TEXTENCODING_UTF8 );
        else
            return OUStringToOString( _rHelpURL, RTL_TEXTENCODING_UTF8 );
    }

    //------------------------------------------------------------------------------------------------------------------
    OString CustomToolPanel::GetHelpID() const
    {
        return lcl_getHelpId( m_aPanelHelpURL );
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
        ModuleTaskPane_Impl( ModuleTaskPane& i_rAntiImpl, const Reference< XFrame >& i_rDocumentFrame,
                const IToolPanelCompare* i_pPanelCompare )
            :m_rAntiImpl( i_rAntiImpl )
            ,m_sModuleIdentifier( lcl_identifyModule( i_rDocumentFrame ) )
            ,m_xFrame( i_rDocumentFrame )
            ,m_aPanelDeck( i_rAntiImpl )
        {
            m_aPanelDeck.Show();
            OnResize();
            impl_initFromConfiguration( i_pPanelCompare );
        }

        ~ModuleTaskPane_Impl()
        {
        }

        void    OnResize();
        void    OnGetFocus();

        static bool ModuleHasToolPanels( const OUString& i_rModuleIdentifier );

              ::svt::ToolPanelDeck& GetPanelDeck()          { return m_aPanelDeck; }
        const ::svt::ToolPanelDeck& GetPanelDeck() const    { return m_aPanelDeck; }

        ::boost::optional< size_t >
                    GetPanelPos( const OUString& i_rResourceURL );
        OUString
                    GetPanelResourceURL( const size_t i_nPanelPos ) const;

        void        SetDrawersLayout();
        void        SetTabsLayout( const ::svt::TabAlignment i_eTabAlignment, const ::svt::TabItemContent i_eTabContent );

    private:
        void    impl_initFromConfiguration( const IToolPanelCompare* i_pPanelCompare );

        static bool
                impl_isToolPanelResource( const OUString& i_rResourceURL );

        DECL_LINK( OnActivatePanel, void* );

    private:
        ModuleTaskPane&             m_rAntiImpl;
        const OUString       m_sModuleIdentifier;
        const Reference< XFrame >   m_xFrame;
        ::svt::ToolPanelDeck        m_aPanelDeck;
    };

    //------------------------------------------------------------------------------------------------------------------
    void ModuleTaskPane_Impl::OnResize()
    {
        m_aPanelDeck.SetPosSizePixel( Point(), m_rAntiImpl.GetOutputSizePixel() );
    }

    //------------------------------------------------------------------------------------------------------------------
    void ModuleTaskPane_Impl::OnGetFocus()
    {
        m_aPanelDeck.GrabFocus();
    }

    //------------------------------------------------------------------------------------------------------------------
    IMPL_LINK( ModuleTaskPane_Impl, OnActivatePanel, void*, i_pArg )
    {
        m_aPanelDeck.ActivatePanel( reinterpret_cast< size_t >( i_pArg ) );
        return 1L;
    }

    //------------------------------------------------------------------------------------------------------------------
    bool ModuleTaskPane_Impl::impl_isToolPanelResource( const OUString& i_rResourceURL )
    {
        return i_rResourceURL.matchAsciiL( RTL_CONSTASCII_STRINGPARAM( "private:resource/toolpanel/" ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    void ModuleTaskPane_Impl::impl_initFromConfiguration( const IToolPanelCompare* i_pPanelCompare )
    {
        const ::utl::OConfigurationTreeRoot aWindowStateConfig( lcl_getModuleUIElementStatesConfig( m_sModuleIdentifier ) );
        if ( !aWindowStateConfig.isValid() )
            return;

        OUString sFirstVisiblePanelResource;
        OUString sFirstPanelResource;

        const Sequence< OUString > aUIElements( aWindowStateConfig.getNodeNames() );
        for (   const OUString* resource = aUIElements.getConstArray();
                resource != aUIElements.getConstArray() + aUIElements.getLength();
                ++resource
            )
        {
            if ( !impl_isToolPanelResource( *resource ) )
                continue;

            sFirstPanelResource = *resource;

            ::utl::OConfigurationNode aResourceNode( aWindowStateConfig.openNode( *resource ) );
            ::svt::PToolPanel pCustomPanel( new CustomToolPanel( aResourceNode, m_xFrame ) );

            size_t nPanelPos = m_aPanelDeck.GetPanelCount();
            if ( i_pPanelCompare )
            {
                // assuming that nobody will insert hundreths of panels, a simple O(n) search should suffice here ...
                while ( nPanelPos > 0 )
                {
                    const short nCompare = i_pPanelCompare->compareToolPanelsURLs(
                        *resource,
                        GetPanelResourceURL( --nPanelPos )
                    );
                    if ( nCompare >= 0 )
                    {
                        ++nPanelPos;
                        break;
                    }
                }
            }
            nPanelPos = m_aPanelDeck.InsertPanel( pCustomPanel, nPanelPos );

            if ( ::comphelper::getBOOL( aResourceNode.getNodeValue( "Visible" ) ) )
                sFirstVisiblePanelResource = *resource;
        }

        if ( sFirstVisiblePanelResource.isEmpty() )
            sFirstVisiblePanelResource = sFirstPanelResource;

        if ( !sFirstVisiblePanelResource.isEmpty() )
        {
            ::boost::optional< size_t > aPanelPos( GetPanelPos( sFirstVisiblePanelResource ) );
            OSL_ENSURE( !!aPanelPos, "ModuleTaskPane_Impl::impl_isToolPanelResource: just inserted it, and it's not there?!" );
            if ( !!aPanelPos )
                m_rAntiImpl.PostUserEvent( LINK( this, ModuleTaskPane_Impl, OnActivatePanel ), reinterpret_cast< void* >( *aPanelPos ) );
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    bool ModuleTaskPane_Impl::ModuleHasToolPanels( const OUString& i_rModuleIdentifier )
    {
        const ::utl::OConfigurationTreeRoot aWindowStateConfig( lcl_getModuleUIElementStatesConfig( i_rModuleIdentifier ) );
        if ( !aWindowStateConfig.isValid() )
            return false;

        const Sequence< OUString > aUIElements( aWindowStateConfig.getNodeNames() );
        for (   const OUString* resource = aUIElements.getConstArray();
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
    ::boost::optional< size_t > ModuleTaskPane_Impl::GetPanelPos( const OUString& i_rResourceURL )
    {
        ::boost::optional< size_t > aPanelPos;
        for ( size_t i = 0; i < m_aPanelDeck.GetPanelCount(); ++i )
        {
            const ::svt::PToolPanel pPanel( m_aPanelDeck.GetPanel( i ) );
            const CustomToolPanel* pCustomPanel = dynamic_cast< const CustomToolPanel* >( pPanel.get() );
            if ( !pCustomPanel )
            {
                SAL_WARN( "sfx.dialog", "ModuleTaskPane_Impl::GetPanelPos: illegal panel implementation!" );
                continue;
            }

            if ( pCustomPanel->GetResourceURL() == i_rResourceURL )
            {
                aPanelPos = i;
                break;
            }
        }
        return aPanelPos;
    }

    //------------------------------------------------------------------------------------------------------------------
    OUString ModuleTaskPane_Impl::GetPanelResourceURL( const size_t i_nPanelPos ) const
    {
        ENSURE_OR_RETURN( i_nPanelPos < m_aPanelDeck.GetPanelCount(), "ModuleTaskPane_Impl::GetPanelResourceURL: illegal panel position!", OUString() );
        const ::svt::PToolPanel pPanel( m_aPanelDeck.GetPanel( i_nPanelPos ) );
        const CustomToolPanel* pCustomPanel = dynamic_cast< const CustomToolPanel* >( pPanel.get() );
        ENSURE_OR_RETURN( pCustomPanel != NULL, "ModuleTaskPane_Impl::GetPanelPos: illegal panel implementation!", OUString() );
        return pCustomPanel->GetResourceURL();
    }

    //------------------------------------------------------------------------------------------------------------------
    void ModuleTaskPane_Impl::SetDrawersLayout()
    {
        const ::svt::PDeckLayouter pLayouter( m_aPanelDeck.GetLayouter() );
        const ::svt::DrawerDeckLayouter* pDrawerLayouter = dynamic_cast< const ::svt::DrawerDeckLayouter* >( pLayouter.get() );
        if ( pDrawerLayouter != NULL )
            // already have the proper layout
            return;
        m_aPanelDeck.SetLayouter( new ::svt::DrawerDeckLayouter( m_aPanelDeck, m_aPanelDeck ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    void ModuleTaskPane_Impl::SetTabsLayout( const ::svt::TabAlignment i_eTabAlignment, const ::svt::TabItemContent i_eTabContent )
    {
        ::svt::PDeckLayouter pLayouter( m_aPanelDeck.GetLayouter() );
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

        m_aPanelDeck.SetLayouter( new ::svt::TabDeckLayouter( m_aPanelDeck, m_aPanelDeck, i_eTabAlignment, i_eTabContent ) );
    }

    //==================================================================================================================
    //= ModuleTaskPane
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    ModuleTaskPane::ModuleTaskPane( Window& i_rParentWindow, const Reference< XFrame >& i_rDocumentFrame )
        :Window( &i_rParentWindow, WB_DIALOGCONTROL )
        ,m_pImpl( new ModuleTaskPane_Impl( *this, i_rDocumentFrame, NULL ) )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    ModuleTaskPane::ModuleTaskPane( Window& i_rParentWindow, const Reference< XFrame >& i_rDocumentFrame,
            const IToolPanelCompare& i_rCompare )
        :Window( &i_rParentWindow, WB_DIALOGCONTROL )
        ,m_pImpl( new ModuleTaskPane_Impl( *this, i_rDocumentFrame, &i_rCompare ) )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    ModuleTaskPane::~ModuleTaskPane()
    {
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
    ::boost::optional< size_t > ModuleTaskPane::GetPanelPos( const OUString& i_rResourceURL )
    {
        return m_pImpl->GetPanelPos( i_rResourceURL );
    }

    //------------------------------------------------------------------------------------------------------------------
    OUString ModuleTaskPane::GetPanelResourceURL( const size_t i_nPanelPos ) const
    {
        return m_pImpl->GetPanelResourceURL( i_nPanelPos );
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

    //==================================================================================================================
    //= helper
    //==================================================================================================================
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

    // =====================================================================================================================
    // = PanelDescriptor
    // =====================================================================================================================
    /** is a helper class for TaskPaneController_Impl, holding the details about a single panel which is not
        contained in the IToolPanel implementation itself.
    */
    struct PanelDescriptor
    {
        ::svt::PToolPanel   pPanel;
        bool                bHidden;

        PanelDescriptor()
            :pPanel()
            ,bHidden( false )
        {
        }

        PanelDescriptor( const ::svt::PToolPanel& i_rPanel )
            :pPanel( i_rPanel )
            ,bHidden( false )
        {
        }
    };

    //==================================================================================================================
    //= TaskPaneController_Impl
    //==================================================================================================================
    class TaskPaneController_Impl   :public ::boost::noncopyable
                                    ,public ::svt::IToolPanelDeckListener
    {
    public:
        TaskPaneController_Impl(
            ModuleTaskPane& i_rTaskPane,
            TitledDockingWindow& i_rDockingWindow
        );
        virtual ~TaskPaneController_Impl();

        void    SetDefaultTitle( const OUString& i_rTitle );
        void    ActivateToolPanel( const OUString& i_rPanelURL );

    protected:
        // IToolPanelDeckListener overridables
        virtual void PanelInserted( const ::svt::PToolPanel& i_pPanel, const size_t i_nPosition );
        virtual void PanelRemoved( const size_t i_nPosition );
        virtual void ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive );
        virtual void LayouterChanged( const ::svt::PDeckLayouter& i_rNewLayouter );
        virtual void Dying();

    private:
        DECL_LINK( OnToolboxClicked, ToolBox* );
        DECL_LINK( OnMenuItemSelected, Menu* );
        DECL_LINK( DockingChanged, TitledDockingWindow* );
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr< PopupMenu > impl_createPopupMenu() const;
        SAL_WNODEPRECATED_DECLARATIONS_POP

        /// sets the given layout for the panel selector
        void    impl_setLayout( const PanelSelectorLayout i_eLayout, const bool i_bForce = false );

        /// returns the current layout of the panel selector
        PanelSelectorLayout
                impl_getLayout() const { return m_eCurrentLayout; }

        void    impl_updateDockingWindowTitle();
        void    impl_togglePanelVisibility( const size_t i_nLogicalPanelIndex );
        size_t  impl_getLogicalPanelIndex( const size_t i_nVisibleIndex );

    private:
        enum MenuId
        {
            MID_UNLOCK_TASK_PANEL = 1,
            MID_LOCK_TASK_PANEL = 2,
            MID_LAYOUT_TABS = 3,
            MID_LAYOUT_DRAWERS = 4,
            MID_FIRST_PANEL = 5
        };

    private:
        typedef ::std::vector< PanelDescriptor >    PanelDescriptors;

        ModuleTaskPane&         m_rTaskPane;
        TitledDockingWindow&    m_rDockingWindow;
        sal_uInt16                  m_nViewMenuID;
        PanelSelectorLayout     m_eCurrentLayout;
        PanelDescriptors        m_aPanelRepository;
        bool                    m_bTogglingPanelVisibility;
        OUString         m_sDefaultTitle;
    };

    //------------------------------------------------------------------------------------------------------------------
    TaskPaneController_Impl::TaskPaneController_Impl( ModuleTaskPane& i_rTaskPane, TitledDockingWindow& i_rDockingWindow )
        :m_rTaskPane( i_rTaskPane )
        ,m_rDockingWindow( i_rDockingWindow )
        ,m_nViewMenuID( 0 )
        ,m_eCurrentLayout( LAYOUT_DRAWERS )
        ,m_aPanelRepository()
        ,m_bTogglingPanelVisibility( false )
        ,m_sDefaultTitle()
    {
        m_rDockingWindow.ResetToolBox();
        m_nViewMenuID = m_rDockingWindow.AddDropDownToolBoxItem(
            SfxResId( STR_SFX_TASK_PANE_VIEW ).toString(),
            HID_TASKPANE_VIEW_MENU,
            LINK( this, TaskPaneController_Impl, OnToolboxClicked )
        );
        m_rDockingWindow.SetEndDockingHdl( LINK( this, TaskPaneController_Impl, DockingChanged ) );
        impl_setLayout(LAYOUT_DRAWERS, true);

        m_rTaskPane.GetPanelDeck().AddListener( *this );

        // initialize the panel repository
        for ( size_t i = 0; i < m_rTaskPane.GetPanelDeck().GetPanelCount(); ++i )
        {
            ::svt::PToolPanel pPanel( m_rTaskPane.GetPanelDeck().GetPanel( i ) );
            m_aPanelRepository.push_back( PanelDescriptor( pPanel ) );
        }

        SetDefaultTitle( SfxResId( STR_SFX_TASKS ).toString() );
    }

    //------------------------------------------------------------------------------------------------------------------
    TaskPaneController_Impl::~TaskPaneController_Impl()
    {
        m_rTaskPane.GetPanelDeck().RemoveListener( *this );
        int i = 0;

        // remove the panels which are not under the control of the panel deck currently
        for (   PanelDescriptors::iterator panelPos = m_aPanelRepository.begin();
                panelPos != m_aPanelRepository.end();
                ++panelPos, ++i
            )
        {
            if ( panelPos->bHidden )
                impl_togglePanelVisibility( i );
        }
        m_aPanelRepository.clear();
    }

    // -----------------------------------------------------------------------------------------------------------------
    void TaskPaneController_Impl::SetDefaultTitle( const OUString& i_rTitle )
    {
        m_sDefaultTitle = i_rTitle;
        impl_updateDockingWindowTitle();
    }

    //------------------------------------------------------------------------------------------------------------------
    void TaskPaneController_Impl::ActivateToolPanel( const OUString& i_rPanelURL )
    {
        ::boost::optional< size_t > aPanelPos( m_rTaskPane.GetPanelPos( i_rPanelURL ) );
        ENSURE_OR_RETURN_VOID( !!aPanelPos, "TaskPaneController_Impl::ActivateToolPanel: no such panel!" );

        if ( aPanelPos == m_rTaskPane.GetPanelDeck().GetActivePanel() )
        {
            ::svt::PToolPanel pPanel( m_rTaskPane.GetPanelDeck().GetPanel( *aPanelPos ) );
            pPanel->GrabFocus();
        }
        else
        {
            m_rTaskPane.GetPanelDeck().ActivatePanel( aPanelPos );
        }
    }

    // -----------------------------------------------------------------------------------------------------------------
    IMPL_LINK( TaskPaneController_Impl, DockingChanged, TitledDockingWindow*, i_pDockingWindow )
    {
        ENSURE_OR_RETURN( i_pDockingWindow && &m_rDockingWindow, "TaskPaneController_Impl::DockingChanged: where does this come from?", 0L );

        if ( impl_getLayout() == LAYOUT_DRAWERS )
            return 0L;

        impl_setLayout( lcl_getTabLayoutFromAlignment( i_pDockingWindow->GetAlignment() ) );
        return 1L;
    }

    // -----------------------------------------------------------------------------------------------------------------
    IMPL_LINK( TaskPaneController_Impl, OnToolboxClicked, ToolBox*, i_pToolBox )
    {
        if ( i_pToolBox->GetCurItemId() == m_nViewMenuID )
        {
            i_pToolBox->EndSelection();

            SAL_WNODEPRECATED_DECLARATIONS_PUSH
            ::std::auto_ptr< PopupMenu > pMenu = impl_createPopupMenu();
            SAL_WNODEPRECATED_DECLARATIONS_POP
            pMenu->SetSelectHdl( LINK( this, TaskPaneController_Impl, OnMenuItemSelected ) );

            // pass toolbox button rect so the menu can stay open on button up
            Rectangle aMenuRect( i_pToolBox->GetItemRect( m_nViewMenuID ) );
            aMenuRect.SetPos( i_pToolBox->GetPosPixel() );
            pMenu->Execute( &m_rDockingWindow, aMenuRect, POPUPMENU_EXECUTE_DOWN );
        }

        return 0;
    }

    // ---------------------------------------------------------------------------------------------------------------------
    IMPL_LINK( TaskPaneController_Impl, OnMenuItemSelected, Menu*, i_pMenu )
    {
        ENSURE_OR_RETURN( i_pMenu, "TaskPaneController_Impl::OnMenuItemSelected: illegal menu!", 0L );

        i_pMenu->Deactivate();
        switch ( i_pMenu->GetCurItemId() )
        {
            case MID_UNLOCK_TASK_PANEL:
                m_rDockingWindow.SetFloatingMode( sal_True );
                break;

            case MID_LOCK_TASK_PANEL:
                m_rDockingWindow.SetFloatingMode( sal_False );
                break;

            case MID_LAYOUT_DRAWERS:
                impl_setLayout( LAYOUT_DRAWERS );
                break;

            case MID_LAYOUT_TABS:
                impl_setLayout( lcl_getTabLayoutFromAlignment( m_rDockingWindow.GetAlignment() ) );
                break;

            default:
            {
                size_t nPanelIndex = size_t( i_pMenu->GetCurItemId() - MID_FIRST_PANEL );
                impl_togglePanelVisibility( nPanelIndex );
            }
            break;
        }

        return 1L;
    }

    // ---------------------------------------------------------------------------------------------------------------------
    size_t TaskPaneController_Impl::impl_getLogicalPanelIndex( const size_t i_nVisibleIndex )
    {
        size_t nLogicalIndex = 0;
        size_t nVisibleIndex( i_nVisibleIndex );
        for ( size_t i=0; i < m_aPanelRepository.size(); ++i )
        {
            if ( !m_aPanelRepository[i].bHidden )
            {
                if ( !nVisibleIndex )
                    break;
                --nVisibleIndex;
            }
            ++nLogicalIndex;
        }
        return nLogicalIndex;
    }

    // ---------------------------------------------------------------------------------------------------------------------
    void TaskPaneController_Impl::PanelInserted( const ::svt::PToolPanel& i_pPanel, const size_t i_nPosition )
    {
        if ( m_bTogglingPanelVisibility )
            return;

        const size_t nLogicalIndex( impl_getLogicalPanelIndex( i_nPosition ) );
        m_aPanelRepository.insert( m_aPanelRepository.begin() + nLogicalIndex, PanelDescriptor( i_pPanel ) );
    }

    // ---------------------------------------------------------------------------------------------------------------------
    void TaskPaneController_Impl::PanelRemoved( const size_t i_nPosition )
    {
        if ( m_bTogglingPanelVisibility )
            return;

        const size_t nLogicalIndex( impl_getLogicalPanelIndex( i_nPosition ) );
        m_aPanelRepository.erase( m_aPanelRepository.begin() + nLogicalIndex );
    }

    // ---------------------------------------------------------------------------------------------------------------------
    void TaskPaneController_Impl::ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive )
    {
        if ( impl_getLayout() == LAYOUT_DRAWERS )
            // no adjustment of the title when we use the classical "drawers" layout
            return;

        impl_updateDockingWindowTitle( );
        (void)i_rOldActive;
        (void)i_rNewActive;
    }

    // ---------------------------------------------------------------------------------------------------------------------
    void TaskPaneController_Impl::LayouterChanged( const ::svt::PDeckLayouter& i_rNewLayouter )
    {
        // not interested in
        (void)i_rNewLayouter;
    }

    // ---------------------------------------------------------------------------------------------------------------------
    void TaskPaneController_Impl::Dying()
    {
        OSL_FAIL( "TaskPaneController_Impl::Dying: unexpected call!" );
        // We are expected to live longer than the ToolPanelDeck we work with. Since we remove ourself, in our dtor,
        // as listener from the panel deck, this method here should never be called.
    }

    // ---------------------------------------------------------------------------------------------------------------------
    void TaskPaneController_Impl::impl_togglePanelVisibility( const size_t i_nLogicalPanelIndex )
    {
        ENSURE_OR_RETURN_VOID( i_nLogicalPanelIndex < m_aPanelRepository.size(), "illegal index" );

        // get the actual panel index, within the deck
        size_t nActualPanelIndex(0);
        for ( size_t i=0; i < i_nLogicalPanelIndex; ++i )
        {
            if ( !m_aPanelRepository[i].bHidden )
                ++nActualPanelIndex;
        }

        ::boost::optional< size_t > aActivatePanel;

        m_bTogglingPanelVisibility = true;
        if ( m_aPanelRepository[ i_nLogicalPanelIndex ].bHidden )
        {
            OSL_VERIFY( m_rTaskPane.GetPanelDeck().InsertPanel( m_aPanelRepository[ i_nLogicalPanelIndex ].pPanel, nActualPanelIndex ) == nActualPanelIndex );
            // if there has not been an active panel before, activate the newly inserted one
            ::boost::optional< size_t > aActivePanel( m_rTaskPane.GetPanelDeck().GetActivePanel() );
            if ( !aActivePanel )
                aActivatePanel = nActualPanelIndex;
        }
        else
        {
            OSL_VERIFY( m_rTaskPane.GetPanelDeck().RemovePanel( nActualPanelIndex ).get() == m_aPanelRepository[ i_nLogicalPanelIndex ].pPanel.get() );
        }
        m_bTogglingPanelVisibility = false;
        m_aPanelRepository[ i_nLogicalPanelIndex ].bHidden = !m_aPanelRepository[ i_nLogicalPanelIndex ].bHidden;

        if ( !!aActivatePanel )
            m_rTaskPane.GetPanelDeck().ActivatePanel( *aActivatePanel );
    }

    // ---------------------------------------------------------------------------------------------------------------------
    void TaskPaneController_Impl::impl_setLayout( const PanelSelectorLayout i_eLayout, const bool i_bForce )
    {
        if ( !i_bForce && ( m_eCurrentLayout == i_eLayout ) )
            return;

        switch ( i_eLayout )
        {
        case LAYOUT_DRAWERS:
            m_rTaskPane.SetDrawersLayout();
            break;
        case LAYOUT_TABS_TOP:
            m_rTaskPane.SetTabsLayout( ::svt::TABS_TOP, ::svt::TABITEM_IMAGE_ONLY );
            break;
        case LAYOUT_TABS_BOTTOM:
            m_rTaskPane.SetTabsLayout( ::svt::TABS_BOTTOM, ::svt::TABITEM_IMAGE_ONLY );
            break;
        case LAYOUT_TABS_LEFT:
            m_rTaskPane.SetTabsLayout( ::svt::TABS_LEFT, ::svt::TABITEM_IMAGE_ONLY );
            break;
        case LAYOUT_TABS_RIGHT:
            m_rTaskPane.SetTabsLayout( ::svt::TABS_RIGHT, ::svt::TABITEM_IMAGE_ONLY );
            break;
        }
        m_eCurrentLayout = i_eLayout;

        impl_updateDockingWindowTitle();
    }

    // ---------------------------------------------------------------------------------------------------------------------
    void TaskPaneController_Impl::impl_updateDockingWindowTitle()
    {
        ::boost::optional< size_t > aActivePanel( m_rTaskPane.GetPanelDeck().GetActivePanel() );
        if ( !aActivePanel || ( impl_getLayout() == LAYOUT_DRAWERS ) )
            m_rDockingWindow.SetTitle( m_sDefaultTitle );
        else
        {
            size_t nNewActive( *aActivePanel );
            for ( size_t i=0; i < m_aPanelRepository.size(); ++i )
            {
                if ( m_aPanelRepository[i].bHidden )
                    continue;

                if ( !nNewActive )
                {
                    m_rDockingWindow.SetTitle( m_aPanelRepository[i].pPanel->GetDisplayName() );
                    break;
                }
                --nNewActive;
            }
        }
    }

    // ---------------------------------------------------------------------------------------------------------------------
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr< PopupMenu > TaskPaneController_Impl::impl_createPopupMenu() const
    {
        ::std::auto_ptr<PopupMenu> pMenu( new PopupMenu );
        FloatingWindow* pMenuWindow = static_cast< FloatingWindow* >( pMenu->GetWindow() );
        if ( pMenuWindow != NULL )
        {
            pMenuWindow->SetPopupModeFlags ( pMenuWindow->GetPopupModeFlags() | FLOATWIN_POPUPMODE_NOMOUSEUPCLOSE );
        }

        // Add one entry for every tool panel element to individually make
        // them visible or hide them.
        sal_uInt16 nIndex = MID_FIRST_PANEL;
        for ( size_t i=0; i<m_aPanelRepository.size(); ++i, ++nIndex )
        {
            const PanelDescriptor& rPanelDesc( m_aPanelRepository[i] );
            pMenu->InsertItem( nIndex, rPanelDesc.pPanel->GetDisplayName(), MIB_CHECKABLE );
            pMenu->CheckItem( nIndex, !rPanelDesc.bHidden );
        }
        pMenu->InsertSeparator();

#if OSL_DEBUG_LEVEL > 0
        if (SvtMiscOptions().IsExperimentalMode())
        {
            pMenu->InsertItem( MID_LAYOUT_TABS, OUString("Tab-Layout (exp.)"), MIB_CHECKABLE );
            pMenu->CheckItem( MID_LAYOUT_TABS, impl_getLayout() != LAYOUT_DRAWERS );
            pMenu->InsertItem( MID_LAYOUT_DRAWERS, OUString("Drawer-Layout"), MIB_CHECKABLE );
            pMenu->CheckItem( MID_LAYOUT_DRAWERS, impl_getLayout() == LAYOUT_DRAWERS );

            pMenu->InsertSeparator();
        }
#endif

        // Add entry for docking or un-docking the tool panel.
        if ( m_rDockingWindow.IsFloatingMode() )
            pMenu->InsertItem(
                MID_LOCK_TASK_PANEL,
                SfxResId( STR_SFX_DOCK ).toString()
            );
        else
            pMenu->InsertItem(
                MID_UNLOCK_TASK_PANEL,
                SfxResId( STR_SFX_UNDOCK ).toString()
            );

        pMenu->RemoveDisabledEntries( sal_False, sal_False );

        return pMenu;
    }
    SAL_WNODEPRECATED_DECLARATIONS_POP

    //==================================================================================================================
    //= TaskPaneController
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    TaskPaneController::TaskPaneController( ModuleTaskPane& i_rTaskPane, TitledDockingWindow& i_rDockingWindow )
        :m_pImpl( new TaskPaneController_Impl( i_rTaskPane, i_rDockingWindow ) )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    TaskPaneController::~TaskPaneController()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    void TaskPaneController::ActivateToolPanel( const OUString& i_rPanelURL )
    {
        m_pImpl->ActivateToolPanel( i_rPanelURL );
    }

//......................................................................................................................
} // namespace sfx2
//......................................................................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
