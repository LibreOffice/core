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

#include <uielement/menubarmanager.hxx>

#include <cppuhelper/implbase.hxx>
#include <svtools/popupmenucontrollerbase.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <sal/log.hxx>

#include <com/sun/star/embed/VerbAttributes.hpp>
#include <com/sun/star/embed/VerbDescriptor.hpp>
#include <com/sun/star/frame/Desktop.hpp>
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
    css::uno::Reference< css::frame::XDispatchProvider > m_xDispatchProvider;
    css::uno::Reference< css::container::XIndexAccess > m_xMenuContainer;
    css::uno::Reference< css::ui::XUIConfigurationManager > m_xConfigManager, m_xModuleConfigManager;
    void addVerbs( const css::uno::Sequence< css::embed::VerbDescriptor >& rVerbs );
    virtual void SAL_CALL disposing() override;

protected:
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
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
            else if ( aPropValue.Name == "ResourceURL" )
                aPropValue.Value >>= m_aMenuURL;
            else if ( aPropValue.Name == "Frame" )
                aPropValue.Value >>= m_xFrame;
            else if ( aPropValue.Name == "ModuleIdentifier" )
                aPropValue.Value >>= m_aModuleName;
            else if ( aPropValue.Name == "DispatchProvider" )
                aPropValue.Value >>= m_xDispatchProvider;
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
    framework::MenuBarManager::FillMenu( m_nNewMenuId, comphelper::getFromUnoTunnel<VCLXMenu>( m_xPopupMenu )->GetMenu(), m_aModuleName, m_xMenuContainer, m_xDispatchProvider );

    // For context menus, add object verbs.
    if ( !m_bContextMenu )
        return;

    css::util::URL aObjectMenuURL;
    aObjectMenuURL.Complete = ".uno:ObjectMenue";
    m_xURLTransformer->parseStrict( aObjectMenuURL );
    css::uno::Reference< css::frame::XDispatchProvider > xDispatchProvider( m_xFrame, css::uno::UNO_QUERY );
    css::uno::Reference< css::frame::XDispatch > xDispatch( xDispatchProvider->queryDispatch( aObjectMenuURL, OUString(), 0 ) );
    if ( xDispatch.is() )
    {
        xDispatch->addStatusListener( this, aObjectMenuURL );
        xDispatch->removeStatusListener( this, aObjectMenuURL );
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
    VCLXMenu* pAwtMenu = comphelper::getFromUnoTunnel<VCLXMenu>( m_xPopupMenu );
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
        VCLXMenu* pAwtMenu = comphelper::getFromUnoTunnel<VCLXMenu>( m_xPopupMenu );
        m_xMenuBarManager.set( new framework::MenuBarManager(
            m_xContext, m_xFrame, m_xURLTransformer, m_xDispatchProvider, m_aModuleName, pAwtMenu->GetMenu(), false, !m_bContextMenu && !m_bInToolbar ) );
        m_xFrame->addFrameActionListener( m_xMenuBarManager );
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
            if (m_xFrame.is())
                m_xFrame->removeFrameActionListener( m_xMenuBarManager );

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
    m_xDispatchProvider.clear();
    if ( m_xMenuBarManager.is() )
    {
        if (m_xFrame.is())
            m_xFrame->removeFrameActionListener( m_xMenuBarManager );

        m_xMenuBarManager->dispose();
        m_xMenuBarManager.clear();
    }

    svt::PopupMenuControllerBase::disposing();
}

OUString ResourceMenuController::getImplementationName()
{
    if ( m_bToolbarContainer )
        return "com.sun.star.comp.framework.ToolbarAsMenuController";

    return "com.sun.star.comp.framework.ResourceMenuController";
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

void InsertItem(const css::uno::Reference<css::awt::XPopupMenu>& rPopupMenu,
                const OUString& rCommand)
{
    sal_uInt16 nItemId = rPopupMenu->getItemCount() + 1;
    rPopupMenu->insertItem(nItemId, OUString(), 0, -1);
    rPopupMenu->setCommand(nItemId, rCommand);
}

void SaveAsMenuController::impl_setPopupMenu()
{
    SolarMutexGuard aGuard;

    InsertItem(m_xPopupMenu, ".uno:SaveAs");
    InsertItem(m_xPopupMenu, ".uno:ExportTo");
    InsertItem(m_xPopupMenu, ".uno:SaveACopy");
    InsertItem(m_xPopupMenu, ".uno:SaveAsTemplate");
    m_xPopupMenu->insertSeparator(-1);
    InsertItem(m_xPopupMenu, ".uno:SaveAsRemote");
}

OUString SaveAsMenuController::getImplementationName()
{
    return "com.sun.star.comp.framework.SaveAsMenuController";
}

class WindowListMenuController : public ResourceMenuController
{
public:
    WindowListMenuController( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                              const css::uno::Sequence< css::uno::Any >& rxArgs )
        : ResourceMenuController(rxContext, rxArgs, false) {}

    // XMenuListener
    void SAL_CALL itemActivated( const css::awt::MenuEvent& rEvent ) override;
    void SAL_CALL itemSelected( const css::awt::MenuEvent& rEvent ) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName() override;

private:
    void impl_setPopupMenu() override;
};

constexpr sal_uInt16 START_ITEMID_WINDOWLIST    = 4600;
constexpr sal_uInt16 END_ITEMID_WINDOWLIST      = 4699;

void WindowListMenuController::itemActivated( const css::awt::MenuEvent& rEvent )
{
    ResourceMenuController::itemActivated( rEvent );

    // update window list
    ::std::vector< OUString > aNewWindowListVector;

    css::uno::Reference< css::frame::XDesktop2 > xDesktop = css::frame::Desktop::create( m_xContext );

    sal_uInt16  nActiveItemId = 0;
    sal_uInt16  nItemId = START_ITEMID_WINDOWLIST;

    css::uno::Reference< css::frame::XFrame > xCurrentFrame = xDesktop->getCurrentFrame();
    css::uno::Reference< css::container::XIndexAccess > xList = xDesktop->getFrames();
    sal_Int32 nFrameCount = xList->getCount();
    aNewWindowListVector.reserve(nFrameCount);
    for (sal_Int32 i=0; i<nFrameCount; ++i )
    {
        css::uno::Reference< css::frame::XFrame > xFrame;
        xList->getByIndex(i) >>= xFrame;

        if (xFrame.is())
        {
            if ( xFrame == xCurrentFrame )
                nActiveItemId = nItemId;

            VclPtr<vcl::Window> pWin = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
            OUString sWindowTitle;
            if ( pWin && pWin->IsVisible() )
                sWindowTitle = pWin->GetText();

            // tdf#101658 In case the frame is embedded somewhere, LO has no control over it.
            // So we just skip it.
            if ( sWindowTitle.isEmpty() )
                continue;

            aNewWindowListVector.push_back( sWindowTitle );
            ++nItemId;
        }
    }

    {
        SolarMutexGuard g;

        VCLXMenu* pAwtMenu = comphelper::getFromUnoTunnel<VCLXMenu>( m_xPopupMenu );
        Menu* pVCLMenu = pAwtMenu->GetMenu();
        int nItemCount = pVCLMenu->GetItemCount();

        if ( nItemCount > 0 )
        {
            // remove all old window list entries from menu
            sal_uInt16 nPos = pVCLMenu->GetItemPos( START_ITEMID_WINDOWLIST );
            for ( sal_uInt16 n = nPos; n < pVCLMenu->GetItemCount(); )
                pVCLMenu->RemoveItem( n );

            if ( pVCLMenu->GetItemType( pVCLMenu->GetItemCount()-1 ) == MenuItemType::SEPARATOR )
                pVCLMenu->RemoveItem( pVCLMenu->GetItemCount()-1 );
        }

        if ( !aNewWindowListVector.empty() )
        {
            // append new window list entries to menu
            pVCLMenu->InsertSeparator();
            nItemId = START_ITEMID_WINDOWLIST;
            const sal_uInt32 nCount = aNewWindowListVector.size();
            for ( sal_uInt32 i = 0; i < nCount; i++ )
            {
                pVCLMenu->InsertItem( nItemId, aNewWindowListVector.at( i ), MenuItemBits::RADIOCHECK );
                if ( nItemId == nActiveItemId )
                    pVCLMenu->CheckItem( nItemId );
                ++nItemId;
            }
        }
    }
}

void WindowListMenuController::itemSelected( const css::awt::MenuEvent& rEvent )
{
    if ( rEvent.MenuId < START_ITEMID_WINDOWLIST || rEvent.MenuId > END_ITEMID_WINDOWLIST )
        return;

    // window list menu item selected
    css::uno::Reference< css::frame::XDesktop2 > xDesktop = css::frame::Desktop::create( m_xContext );

    sal_uInt16 nTaskId = START_ITEMID_WINDOWLIST;
    css::uno::Reference< css::container::XIndexAccess > xList = xDesktop->getFrames();
    sal_Int32 nCount = xList->getCount();
    for ( sal_Int32 i=0; i<nCount; ++i )
    {
        css::uno::Reference< css::frame::XFrame > xFrame;
        xList->getByIndex(i) >>= xFrame;
        if ( xFrame.is() && nTaskId == rEvent.MenuId )
        {
            VclPtr<vcl::Window> pWin = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
            pWin->GrabFocus();
            pWin->ToTop( ToTopFlags::RestoreWhenMin );
            break;
        }

        nTaskId++;
    }
}

void WindowListMenuController::impl_setPopupMenu()
{
    // Make this controller work also with initially empty
    // menu, which PopupMenu::ImplExecute doesn't allow.
    if (m_xPopupMenu.is() && !m_xPopupMenu->getItemCount())
        m_xPopupMenu->insertSeparator(0);
}

OUString WindowListMenuController::getImplementationName()
{
    return "com.sun.star.comp.framework.WindowListMenuController";
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
com_sun_star_comp_framework_WindowListMenuController_get_implementation(
    css::uno::XComponentContext* context,
    css::uno::Sequence< css::uno::Any > const & args )
{
    return cppu::acquire( new WindowListMenuController( context, args ) );
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_framework_SaveAsMenuController_get_implementation(
    css::uno::XComponentContext* context,
    css::uno::Sequence< css::uno::Any > const & args )
{
    return cppu::acquire( new SaveAsMenuController( context, args ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
