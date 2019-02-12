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
#include <vcl/svapp.hxx>
#include <sal/log.hxx>

#include <com/sun/star/embed/VerbAttributes.hpp>
#include <com/sun/star/embed/VerbDescriptor.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/ui/theModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/util/URL.hpp>

namespace {

class ResourceMenuController : public cppu::ImplInheritanceHelper< svt::PopupMenuControllerBase, css::ui::XUIConfigurationListener >
{
public:
    ResourceMenuController( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                            const css::uno::Sequence< css::uno::Any >& rxArgs, bool bToolbarContainer );

    // XPopupMenuController
    virtual void SAL_CALL updatePopupMenu() override;

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent ) override;

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& rEvent ) override;

    // XUIConfigurationListener
    virtual void SAL_CALL elementInserted( const css::ui::ConfigurationEvent& rEvent ) override;
    virtual void SAL_CALL elementRemoved( const css::ui::ConfigurationEvent& rEvent ) override;
    virtual void SAL_CALL elementReplaced( const css::ui::ConfigurationEvent& rEvent ) override;

    // XMenuListener
    virtual void SAL_CALL itemActivated( const css::awt::MenuEvent& rEvent ) override;
    virtual void SAL_CALL itemSelected( const css::awt::MenuEvent& rEvent ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

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
                if ( aMenuName.isEmpty() )
                    continue;

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

void ResourceMenuController::updatePopupMenu()
{
    if ( ( m_xMenuContainer.is() && !m_bContextMenu ) || m_aMenuURL.isEmpty() )
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

    if ( !m_xMenuContainer.is() && m_xConfigManager.is() )
    {
        try
        {
            m_xMenuContainer.set( m_xConfigManager->getSettings( m_aMenuURL, false ) );
        }
        catch ( const css::container::NoSuchElementException& )
        {
            // Not an error - element may exist only in the module.
        }
        catch ( const css::lang::IllegalArgumentException& )
        {
            SAL_WARN( "fwk.uielement", "The given URL is not valid: " << m_aMenuURL );
            return;
        }
    }

    if ( !m_xMenuContainer.is() && m_xModuleConfigManager.is() )
    {
        try
        {
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

void ResourceMenuController::itemActivated( const css::awt::MenuEvent& /*rEvent*/ )
{
    // Must initialize MenuBarManager here, because we want to let the app do context menu interception before.
    if ( !m_xMenuBarManager.is() )
    {
        VCLXMenu* pAwtMenu = VCLXMenu::GetImplementation( m_xPopupMenu );
        css::uno::Reference< css::frame::XDispatchProvider > xDispatchProvider( m_xFrame, css::uno::UNO_QUERY );
        m_xMenuBarManager.set( new framework::MenuBarManager(
            m_xContext, m_xFrame, m_xURLTransformer, xDispatchProvider, m_aModuleName, pAwtMenu->GetMenu(), false, !m_bContextMenu && !m_bInToolbar ) );
        m_xFrame->addFrameActionListener( m_xMenuBarManager.get() );
    }
}

void ResourceMenuController::itemSelected( const css::awt::MenuEvent& /*rEvent*/ )
{
    // Must override this, because we are managed by MenuBarManager, so don't want the handler found in the base class.
}

void ResourceMenuController::elementInserted( const css::ui::ConfigurationEvent& rEvent )
{
    if ( rEvent.ResourceURL == m_aMenuURL )
        m_xMenuContainer.clear();
}

void ResourceMenuController::elementRemoved( const css::ui::ConfigurationEvent& rEvent )
{
    elementInserted( rEvent );
}

void ResourceMenuController::elementReplaced( const css::ui::ConfigurationEvent& rEvent )
{
    elementInserted( rEvent );
}

void ResourceMenuController::disposing( const css::lang::EventObject& rEvent )
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
{
    if ( m_bToolbarContainer )
        return OUString( "com.sun.star.comp.framework.ToolbarAsMenuController" );

    return OUString( "com.sun.star.comp.framework.ResourceMenuController" );
}

css::uno::Sequence< OUString > ResourceMenuController::getSupportedServiceNames()
{
    return { "com.sun.star.frame.PopupMenuController" };
}

class SaveAsMenuController : public ResourceMenuController
{
public:
    SaveAsMenuController( const css::uno::Reference< css::uno::XComponentContext >& rContext,
                          const css::uno::Sequence< css::uno::Any >& rArgs );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;

private:
    virtual void impl_setPopupMenu() override;
};

SaveAsMenuController::SaveAsMenuController( const css::uno::Reference< css::uno::XComponentContext >& rContext,
                                            const css::uno::Sequence< css::uno::Any >& rArgs )
    : ResourceMenuController( rContext, rArgs, false )
{
}

void SaveAsMenuController::impl_setPopupMenu()
{
    VCLXMenu* pPopupMenu    = VCLXMenu::GetImplementation( m_xPopupMenu );
    Menu*     pVCLPopupMenu = nullptr;

    SolarMutexGuard aGuard;

    if ( pPopupMenu )
        pVCLPopupMenu = pPopupMenu->GetMenu();

    if ( !pVCLPopupMenu )
        return;

    pVCLPopupMenu->InsertItem( ".uno:SaveAs", nullptr );
    pVCLPopupMenu->InsertItem( ".uno:ExportTo", nullptr );
    pVCLPopupMenu->InsertItem( ".uno:SaveAsTemplate", nullptr );
    pVCLPopupMenu->InsertSeparator();
    pVCLPopupMenu->InsertItem( ".uno:SaveAsRemote", nullptr );
}

OUString SaveAsMenuController::getImplementationName()
{
    return OUString( "com.sun.star.comp.framework.SaveAsMenuController" );
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_framework_ResourceMenuController_get_implementation(
    css::uno::XComponentContext* context,
    css::uno::Sequence< css::uno::Any > const & args )
{
    return cppu::acquire( new ResourceMenuController( context, args, false ) );
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_framework_ToolbarAsMenuController_get_implementation(
    css::uno::XComponentContext* context,
    css::uno::Sequence< css::uno::Any > const & args )
{
    return cppu::acquire( new ResourceMenuController( context, args, true ) );
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_framework_SaveAsMenuController_get_implementation(
    css::uno::XComponentContext* context,
    css::uno::Sequence< css::uno::Any > const & args )
{
    return cppu::acquire( new SaveAsMenuController( context, args ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
