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

#include <uielement/newmenucontroller.hxx>
#include <menuconfiguration.hxx>

#include <services.h>

#include <com/sun/star/awt/MenuItemType.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/ui/theModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/GlobalAcceleratorConfiguration.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include <comphelper/propertyvalue.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/mutex.hxx>
#include <svtools/acceleratorexecute.hxx>
#include <svtools/imagemgr.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <tools/urlobj.hxx>
#include <unotools/dynamicmenuoptions.hxx>
#include <unotools/moduleoptions.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/graph.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>

//  Defines
constexpr OUStringLiteral aSlotNewDocDirect = u".uno:AddDirect";
constexpr OUStringLiteral aSlotAutoPilot = u".uno:AutoPilotMenu";

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace com::sun::star::container;
using namespace com::sun::star::ui;

namespace framework
{

OUString SAL_CALL NewMenuController::getImplementationName()
{
    return "com.sun.star.comp.framework.NewMenuController";
}

sal_Bool SAL_CALL NewMenuController::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

css::uno::Sequence< OUString > SAL_CALL NewMenuController::getSupportedServiceNames()
{
    return { SERVICENAME_POPUPMENUCONTROLLER };
}

void NewMenuController::setMenuImages(VCLXPopupMenu& rPopupMenu, bool bSetImages)
{
    sal_uInt16 nItemCount = rPopupMenu.getItemCount();
    Reference< XFrame > xFrame( m_xFrame );

    for ( sal_uInt16 i = 0; i < nItemCount; i++ )
    {
        sal_uInt16 nItemId = rPopupMenu.getItemId(i);
        if ( nItemId != 0 )
        {
            if ( bSetImages )
            {
                OUString aImageId;
                OUString aCmd(rPopupMenu.getCommand(nItemId));
                void* nAttributePtr = rPopupMenu.getUserValue(nItemId);
                MenuAttributes* pAttributes = static_cast<MenuAttributes *>(nAttributePtr);
                if (pAttributes)
                    aImageId = pAttributes->aImageId;

                INetURLObject aURLObj( aImageId.isEmpty() ? aCmd : aImageId );
                css::uno::Reference<css::graphic::XGraphic> xGraphic = Graphic(SvFileInformationManager::GetImageNoDefault(aURLObj)).GetXGraphic();
                if (!xGraphic.is())
                    xGraphic = vcl::CommandInfoProvider::GetXGraphicForCommand(aCmd, xFrame);

                rPopupMenu.setItemImage(nItemId, xGraphic, false);
            }
            else
                rPopupMenu.setItemImage(nItemId, nullptr, false);
        }
    }
}

void NewMenuController::determineAndSetNewDocAccel(const css::awt::KeyEvent& rKeyCode)
{
    sal_uInt16 nCount(m_xPopupMenu->getItemCount());
    sal_uInt16 nId( 0 );
    bool      bFound( false );
    OUString aCommand;

    if ( !m_aEmptyDocURL.isEmpty() )
    {
        // Search for the empty document URL

        for ( sal_uInt16 i = 0; i < nCount; i++ )
        {
            if (m_xPopupMenu->getItemType(i) != css::awt::MenuItemType_SEPARATOR)
            {
                nId = m_xPopupMenu->getItemId(i);
                aCommand = m_xPopupMenu->getCommand(nId);
                if ( aCommand.startsWith( m_aEmptyDocURL ) )
                {
                    m_xPopupMenu->setAcceleratorKeyEvent(nId, rKeyCode);
                    bFound = true;
                    break;
                }
            }
        }
    }

    if ( bFound )
        return;

    // Search for the default module name
    OUString aDefaultModuleName( SvtModuleOptions().GetDefaultModuleName() );
    if ( aDefaultModuleName.isEmpty() )
        return;

    for ( sal_uInt16 i = 0; i < nCount; i++ )
    {
        if (m_xPopupMenu->getItemType(i) != css::awt::MenuItemType_SEPARATOR)
        {
            nId = m_xPopupMenu->getItemId(i);
            aCommand = m_xPopupMenu->getCommand(nId);
            if ( aCommand.indexOf( aDefaultModuleName ) >= 0 )
            {
                m_xPopupMenu->setAcceleratorKeyEvent(nId, rKeyCode);
                break;
            }
        }
    }
}

void NewMenuController::setAccelerators()
{
    if ( !m_bModuleIdentified )
        return;

    Reference< XAcceleratorConfiguration > xDocAccelCfg( m_xDocAcceleratorManager );
    Reference< XAcceleratorConfiguration > xModuleAccelCfg( m_xModuleAcceleratorManager );
    Reference< XAcceleratorConfiguration > xGlobalAccelCfg( m_xGlobalAcceleratorManager );

    if ( !m_bAcceleratorCfg )
    {
        // Retrieve references on demand
        m_bAcceleratorCfg = true;
        if ( !xDocAccelCfg.is() )
        {
            Reference< XController > xController = m_xFrame->getController();
            Reference< XModel > xModel;
            if ( xController.is() )
            {
                xModel = xController->getModel();
                if ( xModel.is() )
                {
                    Reference< XUIConfigurationManagerSupplier > xSupplier( xModel, UNO_QUERY );
                    if ( xSupplier.is() )
                    {
                        Reference< XUIConfigurationManager > xDocUICfgMgr = xSupplier->getUIConfigurationManager();
                        if ( xDocUICfgMgr.is() )
                        {
                            xDocAccelCfg = xDocUICfgMgr->getShortCutManager();
                            m_xDocAcceleratorManager = xDocAccelCfg;
                        }
                    }
                }
            }
        }

        if ( !xModuleAccelCfg.is() )
        {
            Reference< XModuleUIConfigurationManagerSupplier > xModuleCfgMgrSupplier =
                theModuleUIConfigurationManagerSupplier::get( m_xContext );
            Reference< XUIConfigurationManager > xUICfgMgr = xModuleCfgMgrSupplier->getUIConfigurationManager( m_aModuleIdentifier );
            if ( xUICfgMgr.is() )
            {
                xModuleAccelCfg = xUICfgMgr->getShortCutManager();
                m_xModuleAcceleratorManager = xModuleAccelCfg;
            }
        }

        if ( !xGlobalAccelCfg.is() )
        {
            xGlobalAccelCfg = GlobalAcceleratorConfiguration::create( m_xContext );
            m_xGlobalAcceleratorManager = xGlobalAccelCfg;
        }
    }

    vcl::KeyCode                    aEmptyKeyCode;
    sal_uInt16                      nItemCount(m_xPopupMenu->getItemCount());
    std::vector< vcl::KeyCode >     aMenuShortCuts;
    std::vector< OUString >    aCmds;
    std::vector< sal_uInt16 >       aIds;
    for ( sal_uInt16 i = 0; i < nItemCount; i++ )
    {
        if (m_xPopupMenu->getItemType(i) != css::awt::MenuItemType_SEPARATOR)
        {
            sal_uInt16 nId(m_xPopupMenu->getItemId(i));
            aIds.push_back( nId );
            aMenuShortCuts.push_back( aEmptyKeyCode );
            aCmds.push_back(m_xPopupMenu->getCommand(nId));
        }
    }

    sal_uInt32 nSeqCount( aIds.size() );

    if ( m_bNewMenu )
        nSeqCount+=1;

    Sequence< OUString > aSeq( nSeqCount );
    auto aSeqRange = asNonConstRange(aSeq);

    // Add a special command for our "New" menu.
    if ( m_bNewMenu )
    {
        aSeqRange[nSeqCount-1] = m_aCommandURL;
        aMenuShortCuts.push_back( aEmptyKeyCode );
    }

    const sal_uInt32 nCount = aCmds.size();
    for ( sal_uInt32 i = 0; i < nCount; i++ )
        aSeqRange[i] = aCmds[i];

    if ( m_xGlobalAcceleratorManager.is() )
        retrieveShortcutsFromConfiguration( xGlobalAccelCfg, aSeq, aMenuShortCuts );
    if ( m_xModuleAcceleratorManager.is() )
        retrieveShortcutsFromConfiguration( xModuleAccelCfg, aSeq, aMenuShortCuts );
    if ( m_xDocAcceleratorManager.is() )
        retrieveShortcutsFromConfiguration( xDocAccelCfg, aSeq, aMenuShortCuts );

    const sal_uInt32 nCount2 = aIds.size();
    for ( sal_uInt32 i = 0; i < nCount2; i++ )
        m_xPopupMenu->setAcceleratorKeyEvent(aIds[i], svt::AcceleratorExecute::st_VCLKey2AWTKey(aMenuShortCuts[i]));

    // Special handling for "New" menu short-cut should be set at the
    // document which will be opened using it.
    if ( m_bNewMenu )
    {
        if ( aMenuShortCuts[nSeqCount-1] != aEmptyKeyCode )
            determineAndSetNewDocAccel(svt::AcceleratorExecute::st_VCLKey2AWTKey(aMenuShortCuts[nSeqCount-1]));
    }
}

void NewMenuController::retrieveShortcutsFromConfiguration(
    const Reference< XAcceleratorConfiguration >& rAccelCfg,
    const Sequence< OUString >& rCommands,
    std::vector< vcl::KeyCode >& aMenuShortCuts )
{
    if ( !rAccelCfg.is() )
        return;

    try
    {
        css::awt::KeyEvent aKeyEvent;
        Sequence< Any > aSeqKeyCode = rAccelCfg->getPreferredKeyEventsForCommandList( rCommands );
        for ( sal_Int32 i = 0; i < aSeqKeyCode.getLength(); i++ )
        {
            if ( aSeqKeyCode[i] >>= aKeyEvent )
                aMenuShortCuts[i] = svt::AcceleratorExecute::st_AWTKey2VCLKey( aKeyEvent );
        }
    }
    catch ( const IllegalArgumentException& )
    {
    }
}

NewMenuController::NewMenuController( const css::uno::Reference< css::uno::XComponentContext >& xContext ) :
    svt::PopupMenuControllerBase( xContext ),
    m_bShowImages( true ),
    m_bNewMenu( false ),
    m_bModuleIdentified( false ),
    m_bAcceleratorCfg( false ),
    m_aTargetFrame( "_default" ),
    m_xContext( xContext )
{
}

NewMenuController::~NewMenuController()
{
}

// private function
void NewMenuController::fillPopupMenu( Reference< css::awt::XPopupMenu > const & rPopupMenu )
{
    VCLXPopupMenu* pPopupMenu    = static_cast<VCLXPopupMenu *>(comphelper::getFromUnoTunnel<VCLXMenu>( rPopupMenu ));

    SolarMutexGuard aSolarMutexGuard;

    resetPopupMenu( rPopupMenu );

    if (!pPopupMenu)
        return;

    Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
    URL aTargetURL;
    aTargetURL.Complete = m_bNewMenu ? OUString(aSlotNewDocDirect) : OUString(aSlotAutoPilot);
    m_xURLTransformer->parseStrict( aTargetURL );
    Reference< XDispatch > xMenuItemDispatch = xDispatchProvider->queryDispatch( aTargetURL, OUString(), 0 );
    if(xMenuItemDispatch == nullptr)
        return;

    const std::vector< SvtDynMenuEntry > aDynamicMenuEntries =
        SvtDynamicMenuOptions::GetMenu( m_bNewMenu ? EDynamicMenuType::NewMenu : EDynamicMenuType::WizardMenu );

    sal_uInt16 nItemId = 1;

    for ( const auto& aDynamicMenuEntry : aDynamicMenuEntries )
    {
        if ( aDynamicMenuEntry.sTitle.isEmpty() && aDynamicMenuEntry.sURL.isEmpty() )
            continue;

        if ( aDynamicMenuEntry.sURL == "private:separator" )
            rPopupMenu->insertSeparator(-1);
        else
        {
            rPopupMenu->insertItem(nItemId, aDynamicMenuEntry.sTitle, 0, -1);
            rPopupMenu->setCommand(nItemId, aDynamicMenuEntry.sURL);

            void* nAttributePtr = MenuAttributes::CreateAttribute( aDynamicMenuEntry.sTargetName, aDynamicMenuEntry.sImageIdentifier );
            pPopupMenu->setUserValue(nItemId, nAttributePtr, MenuAttributes::ReleaseAttribute);

            nItemId++;
        }
    }

    if ( m_bShowImages )
        setMenuImages(*pPopupMenu, m_bShowImages);
}

// XEventListener
void SAL_CALL NewMenuController::disposing( const EventObject& )
{
    Reference< css::awt::XMenuListener > xHolder(this);

    osl::MutexGuard aLock( m_aMutex );
    m_xFrame.clear();
    m_xDispatch.clear();
    m_xContext.clear();

    if ( m_xPopupMenu.is() )
        m_xPopupMenu->removeMenuListener( Reference< css::awt::XMenuListener >(this) );
    m_xPopupMenu.clear();
}

// XStatusListener
void SAL_CALL NewMenuController::statusChanged( const FeatureStateEvent& )
{
}

// XMenuListener
void SAL_CALL NewMenuController::itemSelected( const css::awt::MenuEvent& rEvent )
{
    Reference< css::awt::XPopupMenu > xPopupMenu;
    Reference< XComponentContext >    xContext;

    {
        osl::MutexGuard aLock(m_aMutex);
        xPopupMenu = m_xPopupMenu;
        xContext = m_xContext;
    }

    if ( !xPopupMenu.is() )
        return;

    VCLXPopupMenu* pPopupMenu = static_cast<VCLXPopupMenu *>(comphelper::getFromUnoTunnel<VCLXMenu>( xPopupMenu ));
    if ( !pPopupMenu )
        return;

    OUString aURL;
    OUString aTargetFrame( m_aTargetFrame );

    {
        SolarMutexGuard aSolarMutexGuard;
        aURL = xPopupMenu->getCommand(rEvent.MenuId);
        void* nAttributePtr = pPopupMenu->getUserValue(rEvent.MenuId);
        MenuAttributes* pAttributes = static_cast<MenuAttributes *>(nAttributePtr);
        if (pAttributes)
            aTargetFrame = pAttributes->aTargetFrame;
    }

    Sequence< PropertyValue > aArgsList{ comphelper::makePropertyValue("Referer",
                                                                       OUString( "private:user" )) };

    dispatchCommand( aURL, aArgsList, aTargetFrame );
}

void SAL_CALL NewMenuController::itemActivated( const css::awt::MenuEvent& )
{
    SolarMutexGuard aSolarMutexGuard;
    if ( !(m_xFrame.is() && m_xPopupMenu.is()) )
        return;

    VCLXPopupMenu* pPopupMenu = static_cast<VCLXPopupMenu *>(comphelper::getFromUnoTunnel<VCLXMenu>( m_xPopupMenu ));
    if ( !pPopupMenu )
        return;

    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    bool bShowImages( rSettings.GetUseImagesInMenus() );
    OUString aIconTheme( rSettings.DetermineIconTheme() );

    if ( m_bShowImages != bShowImages || m_aIconTheme != aIconTheme )
    {
        m_bShowImages = bShowImages;
        m_aIconTheme = aIconTheme;
        setMenuImages(*pPopupMenu, m_bShowImages);
    }

    setAccelerators();
}

// XPopupMenuController
void NewMenuController::impl_setPopupMenu()
{

    if ( m_xPopupMenu.is() )
        fillPopupMenu( m_xPopupMenu );

    // Identify module that we are attach to. It's our context that we need to know.
    Reference< XModuleManager2 > xModuleManager = ModuleManager::create( m_xContext );
    try
    {
        m_aModuleIdentifier = xModuleManager->identify( m_xFrame );
        m_bModuleIdentified = true;

        if ( !m_aModuleIdentifier.isEmpty() )
        {
            Sequence< PropertyValue > aSeq;

            if ( xModuleManager->getByName( m_aModuleIdentifier ) >>= aSeq )
            {
                for ( PropertyValue const & prop : std::as_const(aSeq) )
                {
                    if ( prop.Name == "ooSetupFactoryEmptyDocumentURL" )
                    {
                        prop.Value >>= m_aEmptyDocURL;
                        break;
                    }
                }
            }
        }
    }
    catch ( const RuntimeException& )
    {
        throw;
    }
    catch ( const Exception& )
    {
    }
}

// XInitialization
void SAL_CALL NewMenuController::initialize( const Sequence< Any >& aArguments )
{
    osl::MutexGuard aLock( m_aMutex );

    bool bInitialized( m_bInitialized );
    if ( bInitialized )
        return;

    svt::PopupMenuControllerBase::initialize( aArguments );

    if ( m_bInitialized )
    {
        const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();

        m_bShowImages   = rSettings.GetUseImagesInMenus();
        m_aIconTheme    = rSettings.DetermineIconTheme();
        m_bNewMenu      = m_aCommandURL == aSlotNewDocDirect;
    }
}

}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
framework_NewMenuController_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(new framework::NewMenuController(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
