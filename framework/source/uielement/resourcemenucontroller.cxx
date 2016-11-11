/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <uielement/menubarmanager.hxx>

#include <cppuhelper/implbase.hxx>
#include <svtools/popupmenucontrollerbase.hxx>

#include <com/sun/star/embed/VerbAttributes.hpp>
#include <com/sun/star/embed/VerbDescriptor.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/ui/ItemType.hpp>
#include <com/sun/star/ui/theModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/util/URL.hpp>

namespace {

class ResourceMenuController : public cppu::ImplInheritanceHelper< svt::PopupMenuControllerBase, css::ui::XUIConfigurationListener >
{
public:
    ResourceMenuController( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                            const css::uno::Sequence< css::uno::Any >& rxArgs, bool bToolbarContainer );
    virtual ~ResourceMenuController();

    // XPopupMenuController
    virtual void SAL_CALL updatePopupMenu() throw ( css::uno::RuntimeException, std::exception ) override;

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent ) throw ( css::uno::RuntimeException, std::exception ) override;

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& rEvent ) throw ( css::uno::RuntimeException, std::exception ) override;

    // XUIConfigurationListener
    virtual void SAL_CALL elementInserted( const css::ui::ConfigurationEvent& rEvent ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL elementRemoved( const css::ui::ConfigurationEvent& rEvent ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL elementReplaced( const css::ui::ConfigurationEvent& rEvent ) throw ( css::uno::RuntimeException, std::exception ) override;

    // XMenuListener
    virtual void SAL_CALL itemActivated( const css::awt::MenuEvent& rEvent ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL itemSelected( const css::awt::MenuEvent& rEvent ) throw ( css::uno::RuntimeException, std::exception ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw ( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw ( css::uno::RuntimeException, std::exception ) override;

private:
    OUString m_aMenuURL;
    bool m_bContextMenu;
    bool m_bInToolbar;
    bool m_bToolbarContainer;
    sal_uInt16 m_nNewMenuId;
    rtl::Reference< framework::MenuBarManager > m_xMenuBarManager;
    css::uno::Reference< css::container::XIndexAccess > m_xMenuContainer;
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::ui::XUIConfigurationManager > m_xConfigManager, m_xModuleConfigManager;
    void addVerbs( const css::uno::Sequence< css::embed::VerbDescriptor >& rVerbs );
    void fillToolbarData();
    virtual void SAL_CALL disposing() override;
};

ResourceMenuController::ResourceMenuController( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                                                const css::uno::Sequence< css::uno::Any >& rxArgs, bool bToolbarContainer ) :
    ImplInheritanceHelper( rxContext ),
    m_bContextMenu( false ),
    m_bInToolbar( false ),
    m_bToolbarContainer( bToolbarContainer ),
    m_nNewMenuId( 1 ),
    m_xContext( rxContext )
{
    for ( const auto& arg: rxArgs )
    {
        css::beans::PropertyValue aPropValue;
        if ( arg >>= aPropValue )
        {
            if ( aPropValue.Name == "Value" )
            {
                OUString aMenuName;
                aPropValue.Value >>= aMenuName;
                if ( m_bToolbarContainer )
                    m_aMenuURL = "private:resource/toolbar/" + aMenuName;
                else
                    m_aMenuURL = "private:resource/popupmenu/" + aMenuName;
            }
            else if ( aPropValue.Name == "Frame" )
                aPropValue.Value >>= m_xFrame;
            else if ( aPropValue.Name == "ModuleIdentifier" )
                aPropValue.Value >>= m_aModuleName;
            else if ( aPropValue.Name == "IsContextMenu" )
                aPropValue.Value >>= m_bContextMenu;
            else if ( aPropValue.Name == "InToolbar" )
                aPropValue.Value >>= m_bInToolbar;
        }
    }
    if ( m_xFrame.is() )
        // No need to initialize again through initialize method.
        m_bInitialized = true;
}

ResourceMenuController::~ResourceMenuController()
{
}

void ResourceMenuController::updatePopupMenu()
    throw ( css::uno::RuntimeException, std::exception )
{
    if ( m_xMenuContainer.is() && !m_bContextMenu )
        // Container is still valid, no need to do anything on our side.
        return;

    if ( m_aModuleName.isEmpty() )
    {
        try
        {
            css::uno::Reference< css::frame::XModuleManager > xModuleManager( css::frame::ModuleManager::create( m_xContext ) );
            m_aModuleName = xModuleManager->identify( m_xFrame );
        }
        catch( const css::uno::Exception& )
        {}
    }

    if ( !m_xConfigManager.is() )
    {
        try
        {
            css::uno::Reference< css::frame::XController > xController( m_xFrame->getController() );
            css::uno::Reference< css::frame::XModel > xModel( xController->getModel() );
            css::uno::Reference< css::ui::XUIConfigurationManagerSupplier > xSupplier( xModel, css::uno::UNO_QUERY_THROW );
            m_xConfigManager.set( xSupplier->getUIConfigurationManager() );
            css::uno::Reference< css::ui::XUIConfiguration > xConfig( m_xConfigManager, css::uno::UNO_QUERY_THROW );
            xConfig->addConfigurationListener( this );
        }
        catch( const css::uno::RuntimeException& )
        {}
    }

    if ( !m_xModuleConfigManager.is() )
    {
        try
        {
            css::uno::Reference< css::ui::XModuleUIConfigurationManagerSupplier > xModuleCfgMgrSupplier(
                css::ui::theModuleUIConfigurationManagerSupplier::get( m_xContext ) );
            m_xModuleConfigManager.set( xModuleCfgMgrSupplier->getUIConfigurationManager( m_aModuleName ) );
            css::uno::Reference< css::ui::XUIConfiguration > xConfig( m_xModuleConfigManager, css::uno::UNO_QUERY_THROW );
            xConfig->addConfigurationListener( this );
        }
        catch ( const css::container::NoSuchElementException& )
        {
            SAL_WARN( "fwk.uielement", "Invalid module identifier: " << m_aModuleName );
        }
        catch( const css::uno::RuntimeException& )
        {}
    }

    if ( !m_xMenuContainer.is() )
    {
        try
        {
            if ( m_xConfigManager.is() && m_xConfigManager->hasSettings( m_aMenuURL ) )
                m_xMenuContainer.set( m_xConfigManager->getSettings( m_aMenuURL, false ) );
            else if ( m_xModuleConfigManager.is() && m_xModuleConfigManager->hasSettings( m_aMenuURL ) )
                m_xMenuContainer.set( m_xModuleConfigManager->getSettings( m_aMenuURL, false ) );
        }
        catch ( const css::container::NoSuchElementException& )
        {
            SAL_WARN( "fwk.uielement", "Can not find settings for " << m_aMenuURL );
            return;
        }
        catch ( const css::lang::IllegalArgumentException& )
        {
            SAL_WARN( "fwk.uielement", "The given URL is not valid: " << m_aMenuURL );
            return;
        }
    }

    if ( !m_xMenuContainer.is() )
        return;

    // Clear previous content.
    if ( m_xMenuBarManager.is() )
    {
        m_xMenuBarManager->dispose();
        m_xMenuBarManager.clear();
    }
    resetPopupMenu( m_xPopupMenu );
    m_nNewMenuId = 1;

    // Now fill the menu with the configuration data.
    css::uno::Reference< css::frame::XDispatchProvider > xDispatchProvider( m_xFrame, css::uno::UNO_QUERY );
    if ( m_bToolbarContainer )
        fillToolbarData();
    else
        framework::MenuBarManager::FillMenu( m_nNewMenuId, VCLXMenu::GetImplementation( m_xPopupMenu )->GetMenu(), m_aModuleName, m_xMenuContainer, xDispatchProvider );

    // For context menus, add object verbs.
    if ( m_bContextMenu )
    {
        css::util::URL aObjectMenuURL;
        aObjectMenuURL.Complete = ".uno:ObjectMenue";
        m_xURLTransformer->parseStrict( aObjectMenuURL );
        css::uno::Reference< css::frame::XDispatch > xDispatch( xDispatchProvider->queryDispatch( aObjectMenuURL, OUString(), 0 ) );
        if ( xDispatch.is() )
        {
            xDispatch->addStatusListener( this, aObjectMenuURL );
            xDispatch->removeStatusListener( this, aObjectMenuURL );
        }
    }
}

void ResourceMenuController::statusChanged( const css::frame::FeatureStateEvent& rEvent )
    throw ( css::uno::RuntimeException, std::exception )
{
    css::uno::Sequence< css::embed::VerbDescriptor > aVerbs;
    if ( rEvent.IsEnabled && ( rEvent.State >>= aVerbs ) )
        addVerbs( aVerbs );
}

void ResourceMenuController::addVerbs( const css::uno::Sequence< css::embed::VerbDescriptor >& rVerbs )
{
    // Check if the document is read-only.
    css::uno::Reference< css::frame::XController > xController( m_xFrame->getController() );
    css::uno::Reference< css::frame::XStorable > xStorable;
    if ( xController.is() )
        xStorable.set( xController->getModel(), css::uno::UNO_QUERY );

    bool bReadOnly = xStorable.is() && xStorable->isReadonly();
    VCLXMenu* pAwtMenu = VCLXMenu::GetImplementation( m_xPopupMenu );
    Menu* pVCLMenu = pAwtMenu->GetMenu();

    for ( const auto& rVerb : rVerbs )
    {
        if ( !( rVerb.VerbAttributes & css::embed::VerbAttributes::MS_VERBATTR_ONCONTAINERMENU ) ||
            ( bReadOnly && !( rVerb.VerbAttributes & css::embed::VerbAttributes::MS_VERBATTR_NEVERDIRTIES ) ) )
            continue;

        pVCLMenu->InsertItem( m_nNewMenuId, rVerb.VerbName );
        pVCLMenu->SetItemCommand( m_nNewMenuId, ".uno:ObjectMenue?VerbID:short=" + OUString::number( rVerb.VerbID ) );
        ++m_nNewMenuId;
    }
}

void ResourceMenuController::fillToolbarData()
{
    VCLXMenu* pAwtMenu = VCLXMenu::GetImplementation( m_xPopupMenu );
    Menu* pVCLMenu = pAwtMenu->GetMenu();

    css::uno::Sequence< css::beans::PropertyValue > aPropSequence;
    for ( sal_Int32 i = 0; i < m_xMenuContainer->getCount(); ++i )
    {
        try
        {
            if ( m_xMenuContainer->getByIndex( i ) >>= aPropSequence )
            {
                OUString aCommandURL;
                OUString aLabel;
                sal_uInt16 nType = css::ui::ItemType::DEFAULT;
                bool bVisible = true;

                for ( const auto& aProp: aPropSequence )
                {
                    if ( aProp.Name == "CommandURL" )
                        aProp.Value >>= aCommandURL;
                    else if ( aProp.Name == "Label" )
                        aProp.Value >>= aLabel;
                    else if ( aProp.Name == "Type" )
                        aProp.Value >>= nType;
                    else if ( aProp.Name == "IsVisible" )
                        aProp.Value >>= bVisible;
                }

                switch ( nType )
                {
                case css::ui::ItemType::DEFAULT:
                    if ( bVisible )
                    {
                        pVCLMenu->InsertItem( m_nNewMenuId, aLabel );
                        pVCLMenu->SetItemCommand( m_nNewMenuId++, aCommandURL );
                    }
                    break;
                case css::ui::ItemType::SEPARATOR_LINE:
                case css::ui::ItemType::SEPARATOR_LINEBREAK:
                    if ( bVisible && pVCLMenu->GetItemId( pVCLMenu->GetItemCount() - 1 ) != 0 )
                        pVCLMenu->InsertSeparator();
                    break;
                default: ;
                }
            }
        }
        catch ( const css::uno::Exception& )
        {
            break;
        }
    }
}

void ResourceMenuController::itemActivated( const css::awt::MenuEvent& /*rEvent*/ )
    throw ( css::uno::RuntimeException, std::exception )
{
    // Must initialize MenuBarManager here, because we want to let the app do context menu interception before.
    if ( !m_xMenuBarManager.is() )
    {
        VCLXMenu* pAwtMenu = VCLXMenu::GetImplementation( m_xPopupMenu );
        css::uno::Reference< css::frame::XDispatchProvider > xDispatchProvider( m_xFrame, css::uno::UNO_QUERY );
        m_xMenuBarManager.set( new framework::MenuBarManager(
            m_xContext, m_xFrame, m_xURLTransformer, xDispatchProvider, m_aModuleName, pAwtMenu->GetMenu(), false, true, !m_bContextMenu && !m_bInToolbar ) );
    }
}

void ResourceMenuController::itemSelected( const css::awt::MenuEvent& /*rEvent*/ )
    throw ( css::uno::RuntimeException, std::exception )
{
    // Must override this, because we are managed by MenuBarManager, so don't want the handler found in the base class.
}

void ResourceMenuController::elementInserted( const css::ui::ConfigurationEvent& rEvent )
    throw ( css::uno::RuntimeException, std::exception )
{
    if ( rEvent.ResourceURL == m_aMenuURL )
        m_xMenuContainer.clear();
}

void ResourceMenuController::elementRemoved( const css::ui::ConfigurationEvent& rEvent )
    throw ( css::uno::RuntimeException, std::exception )
{
    elementInserted( rEvent );
}

void ResourceMenuController::elementReplaced( const css::ui::ConfigurationEvent& rEvent )
    throw ( css::uno::RuntimeException, std::exception )
{
    elementInserted( rEvent );
}

void ResourceMenuController::disposing( const css::lang::EventObject& rEvent )
    throw ( css::uno::RuntimeException, std::exception )
{
    if ( rEvent.Source == m_xConfigManager )
        m_xConfigManager.clear();
    else if ( rEvent.Source == m_xModuleConfigManager )
        m_xModuleConfigManager.clear();
    else
    {
        if ( m_xMenuBarManager.is() )
        {
            m_xMenuBarManager->dispose();
            m_xMenuBarManager.clear();
        }
        svt::PopupMenuControllerBase::disposing( rEvent );
    }
}

void ResourceMenuController::disposing()
{
    css::uno::Reference< css::ui::XUIConfiguration > xConfig( m_xConfigManager, css::uno::UNO_QUERY );
    if ( xConfig.is() )
        xConfig->removeConfigurationListener( this );

    css::uno::Reference< css::ui::XUIConfiguration > xModuleConfig( m_xModuleConfigManager, css::uno::UNO_QUERY );
    if ( xModuleConfig.is() )
        xModuleConfig->removeConfigurationListener( this );

    m_xConfigManager.clear();
    m_xModuleConfigManager.clear();
    m_xMenuContainer.clear();
    if ( m_xMenuBarManager.is() )
    {
        m_xMenuBarManager->dispose();
        m_xMenuBarManager.clear();
    }

    svt::PopupMenuControllerBase::disposing();
}

OUString ResourceMenuController::getImplementationName()
    throw ( css::uno::RuntimeException, std::exception )
{
    if ( m_bToolbarContainer )
        return OUString( "com.sun.star.comp.framework.ToolbarAsMenuController" );

    return OUString( "com.sun.star.comp.framework.ResourceMenuController" );
}

css::uno::Sequence< OUString > ResourceMenuController::getSupportedServiceNames()
    throw ( css::uno::RuntimeException, std::exception )
{
    return { "com.sun.star.frame.PopupMenuController" };
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_framework_ResourceMenuController_get_implementation(
    css::uno::XComponentContext* context,
    css::uno::Sequence< css::uno::Any > const & args )
{
    return cppu::acquire( new ResourceMenuController( context, args, false ) );
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_framework_ToolbarAsMenuController_get_implementation(
    css::uno::XComponentContext* context,
    css::uno::Sequence< css::uno::Any > const & args )
{
    return cppu::acquire( new ResourceMenuController( context, args, true ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
