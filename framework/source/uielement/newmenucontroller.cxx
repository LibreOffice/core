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

#include "services.h"
#include <classes/resource.hrc>
#include <classes/fwkresid.hxx>

#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/awt/MenuItemStyle.hpp>
#include <com/sun/star/ui/theModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/GlobalAcceleratorConfiguration.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>

#include <vcl/svapp.hxx>
#include <vcl/i18nhelp.hxx>
#include <vcl/settings.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/file.hxx>
#include <svtools/menuoptions.hxx>
#include <svtools/acceleratorexecute.hxx>
#include <svtools/imagemgr.hxx>
#include <tools/urlobj.hxx>
#include <unotools/dynamicmenuoptions.hxx>
#include <unotools/moduleoptions.hxx>
#include <osl/mutex.hxx>
#include <memory>

//  Defines
#define aSlotNewDocDirect ".uno:AddDirect"
#define aSlotAutoPilot ".uno:AutoPilotMenu"

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace com::sun::star::container;
using namespace com::sun::star::ui;

namespace framework
{

DEFINE_XSERVICEINFO_MULTISERVICE_2      (   NewMenuController                           ,
                                            OWeakObject                                 ,
                                            SERVICENAME_POPUPMENUCONTROLLER             ,
                                            IMPLEMENTATIONNAME_NEWMENUCONTROLLER
                                        )

DEFINE_INIT_SERVICE                     (   NewMenuController, {} )

void NewMenuController::setMenuImages( PopupMenu* pPopupMenu, bool bSetImages )
{
    sal_uInt16 nItemCount = pPopupMenu->GetItemCount();
    Reference< XFrame > xFrame( m_xFrame );

    for ( sal_uInt16 i = 0; i < nItemCount; i++ )
    {
        sal_uInt16 nItemId = pPopupMenu->GetItemId( i );
        if ( nItemId != 0 )
        {
            if ( bSetImages )
            {
                OUString aImageId;
                OUString aCmd( pPopupMenu->GetItemCommand( nItemId ) );
                sal_uLong nAttributePtr = pPopupMenu->GetUserValue( nItemId );
                MenuAttributes* pAttributes = reinterpret_cast<MenuAttributes *>(nAttributePtr);
                if (pAttributes)
                    aImageId = pAttributes->aImageId;

                INetURLObject aURLObj( aImageId.isEmpty() ? aCmd : aImageId );
                Image aImage = SvFileInformationManager::GetImageNoDefault( aURLObj );
                if ( !aImage )
                    aImage = vcl::CommandInfoProvider::GetImageForCommand(aCmd, xFrame);

                if ( !!aImage )
                    pPopupMenu->SetItemImage( nItemId, aImage );
            }
            else
                pPopupMenu->SetItemImage( nItemId, Image() );
        }
    }
}

void NewMenuController::determineAndSetNewDocAccel( PopupMenu* pPopupMenu, const vcl::KeyCode& rKeyCode )
{
    sal_uInt16        nCount( pPopupMenu->GetItemCount() );
    sal_uInt16        nId( 0 );
    bool      bFound( false );
    OUString aCommand;

    if ( !m_aEmptyDocURL.isEmpty() )
    {
        // Search for the empty document URL

        for ( sal_uInt16 i = 0; i < nCount; i++ )
        {
            if ( pPopupMenu->GetItemType( i ) != MenuItemType::SEPARATOR )
            {
                nId = pPopupMenu->GetItemId( i );
                aCommand = pPopupMenu->GetItemCommand( nId );
                if ( aCommand.startsWith( m_aEmptyDocURL ) )
                {
                    pPopupMenu->SetAccelKey( nId, rKeyCode );
                    bFound = true;
                    break;
                }
            }
        }
    }

    if ( !bFound )
    {
        // Search for the default module name
        OUString aDefaultModuleName( SvtModuleOptions().GetDefaultModuleName() );
        if ( !aDefaultModuleName.isEmpty() )
        {
            for ( sal_uInt16 i = 0; i < nCount; i++ )
            {
                if ( pPopupMenu->GetItemType( i ) != MenuItemType::SEPARATOR )
                {
                    nId = pPopupMenu->GetItemId( i );
                    aCommand = pPopupMenu->GetItemCommand( nId );
                    if ( aCommand.indexOf( aDefaultModuleName ) >= 0 )
                    {
                        pPopupMenu->SetAccelKey( nId, rKeyCode );
                        break;
                    }
                }
            }
        }
    }
}

void NewMenuController::setAccelerators( PopupMenu* pPopupMenu )
{
    if ( m_bModuleIdentified )
    {
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
                            Reference< XUIConfigurationManager > xDocUICfgMgr( xSupplier->getUIConfigurationManager(), UNO_QUERY );
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
        sal_uInt16                      nItemCount( pPopupMenu->GetItemCount() );
        std::vector< vcl::KeyCode >     aMenuShortCuts;
        std::vector< OUString >    aCmds;
        std::vector< sal_uInt16 >       aIds;
        for ( sal_uInt16 i = 0; i < nItemCount; i++ )
        {
            if ( pPopupMenu->GetItemType( i ) != MenuItemType::SEPARATOR )
            {
                sal_uInt16 nId( pPopupMenu->GetItemId( i ));
                aIds.push_back( nId );
                aMenuShortCuts.push_back( aEmptyKeyCode );
                aCmds.push_back( pPopupMenu->GetItemCommand( nId ));
            }
        }

        sal_uInt32 nSeqCount( aIds.size() );

        if ( m_bNewMenu )
            nSeqCount+=1;

        Sequence< OUString > aSeq( nSeqCount );

        // Add a special command for our "New" menu.
        if ( m_bNewMenu )
        {
            aSeq[nSeqCount-1] = m_aCommandURL;
            aMenuShortCuts.push_back( aEmptyKeyCode );
        }

        const sal_uInt32 nCount = aCmds.size();
        for ( sal_uInt32 i = 0; i < nCount; i++ )
            aSeq[i] = aCmds[i];

        if ( m_xGlobalAcceleratorManager.is() )
            retrieveShortcutsFromConfiguration( xGlobalAccelCfg, aSeq, aMenuShortCuts );
        if ( m_xModuleAcceleratorManager.is() )
            retrieveShortcutsFromConfiguration( xModuleAccelCfg, aSeq, aMenuShortCuts );
        if ( m_xDocAcceleratorManager.is() )
            retrieveShortcutsFromConfiguration( xDocAccelCfg, aSeq, aMenuShortCuts );

        const sal_uInt32 nCount2 = aIds.size();
        for ( sal_uInt32 i = 0; i < nCount2; i++ )
            pPopupMenu->SetAccelKey( aIds[i], aMenuShortCuts[i] );

        // Special handling for "New" menu short-cut should be set at the
        // document which will be opened using it.
        if ( m_bNewMenu )
        {
            if ( aMenuShortCuts[nSeqCount-1] != aEmptyKeyCode )
                determineAndSetNewDocAccel( pPopupMenu, aMenuShortCuts[nSeqCount-1] );
        }
    }
}

void NewMenuController::retrieveShortcutsFromConfiguration(
    const Reference< XAcceleratorConfiguration >& rAccelCfg,
    const Sequence< OUString >& rCommands,
    std::vector< vcl::KeyCode >& aMenuShortCuts )
{
    if ( rAccelCfg.is() )
    {
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
void NewMenuController::fillPopupMenu( Reference< css::awt::XPopupMenu >& rPopupMenu )
{
    VCLXPopupMenu* pPopupMenu    = static_cast<VCLXPopupMenu *>(VCLXMenu::GetImplementation( rPopupMenu ));
    PopupMenu*     pVCLPopupMenu = nullptr;

    SolarMutexGuard aSolarMutexGuard;

    resetPopupMenu( rPopupMenu );
    if ( pPopupMenu )
        pVCLPopupMenu = static_cast<PopupMenu *>(pPopupMenu->GetMenu());

    if ( pVCLPopupMenu )
    {
        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
        URL aTargetURL;
        aTargetURL.Complete = rtl::OUString::createFromAscii(m_bNewMenu ? aSlotNewDocDirect : aSlotAutoPilot);
        m_xURLTransformer->parseStrict( aTargetURL );
        Reference< XDispatch > xMenuItemDispatch = xDispatchProvider->queryDispatch( aTargetURL, OUString(), 0 );
        if(xMenuItemDispatch == nullptr)
            return;

        css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > > aDynamicMenuEntries =
            SvtDynamicMenuOptions().GetMenu( m_bNewMenu ? E_NEWMENU : E_WIZARDMENU );

        OUString aTitle;
        OUString aURL;
        OUString aTargetFrame;
        OUString aImageId;
        sal_uInt16 nItemId = 1;

        for ( const auto& aDynamicMenuEntry : aDynamicMenuEntries )
        {
            for ( const auto& aProperty : aDynamicMenuEntry )
            {
                if ( aProperty.Name == DYNAMICMENU_PROPERTYNAME_URL )
                    aProperty.Value >>= aURL;
                else if ( aProperty.Name == DYNAMICMENU_PROPERTYNAME_TITLE )
                    aProperty.Value >>= aTitle;
                else if ( aProperty.Name == DYNAMICMENU_PROPERTYNAME_IMAGEIDENTIFIER )
                    aProperty.Value >>= aImageId;
                else if ( aProperty.Name == DYNAMICMENU_PROPERTYNAME_TARGETNAME )
                    aProperty.Value >>= aTargetFrame;
            }

            if ( aTitle.isEmpty() && aURL.isEmpty() )
                continue;

            if ( aURL == "private:separator" )
                pVCLPopupMenu->InsertSeparator();
            else
            {
                pVCLPopupMenu->InsertItem( nItemId, aTitle );
                pVCLPopupMenu->SetItemCommand( nItemId, aURL );

                sal_uIntPtr nAttributePtr = MenuAttributes::CreateAttribute( aTargetFrame, aImageId );
                pVCLPopupMenu->SetUserValue( nItemId, nAttributePtr, MenuAttributes::ReleaseAttribute );

                nItemId++;
            }
        }

        if ( m_bShowImages )
            setMenuImages( pVCLPopupMenu, m_bShowImages );
    }
}

// XEventListener
void SAL_CALL NewMenuController::disposing( const EventObject& )
{
    Reference< css::awt::XMenuListener > xHolder(static_cast<OWeakObject *>(this), UNO_QUERY );

    osl::MutexGuard aLock( m_aMutex );
    m_xFrame.clear();
    m_xDispatch.clear();
    m_xContext.clear();

    if ( m_xPopupMenu.is() )
        m_xPopupMenu->removeMenuListener( Reference< css::awt::XMenuListener >(static_cast<OWeakObject *>(this), UNO_QUERY ));
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

    osl::ClearableMutexGuard aLock( m_aMutex );
    xPopupMenu = m_xPopupMenu;
    xContext   = m_xContext;
    aLock.clear();

    if ( xPopupMenu.is() )
    {
        VCLXPopupMenu* pPopupMenu = static_cast<VCLXPopupMenu *>(VCLXPopupMenu::GetImplementation( xPopupMenu ));
        if ( pPopupMenu )
        {
            OUString aURL;
            OUString aTargetFrame( m_aTargetFrame );

            {
                SolarMutexGuard aSolarMutexGuard;
                PopupMenu* pVCLPopupMenu = static_cast<PopupMenu *>(pPopupMenu->GetMenu());
                aURL = pVCLPopupMenu->GetItemCommand(rEvent.MenuId);
                sal_uLong nAttributePtr = pVCLPopupMenu->GetUserValue(rEvent.MenuId);
                MenuAttributes* pAttributes = reinterpret_cast<MenuAttributes *>(nAttributePtr);
                if (pAttributes)
                    aTargetFrame = pAttributes->aTargetFrame;
            }

            Sequence< PropertyValue > aArgsList( 1 );
            aArgsList[0].Name = "Referer";
            aArgsList[0].Value = makeAny( OUString( "private:user" ));

            dispatchCommand( aURL, aArgsList, aTargetFrame );
        }
    }
}

void SAL_CALL NewMenuController::itemActivated( const css::awt::MenuEvent& )
{
    SolarMutexGuard aSolarMutexGuard;
    if ( m_xFrame.is() && m_xPopupMenu.is() )
    {
        VCLXPopupMenu* pPopupMenu = static_cast<VCLXPopupMenu *>(VCLXPopupMenu::GetImplementation( m_xPopupMenu ));
        if ( pPopupMenu )
        {
            const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
            bool bShowImages( rSettings.GetUseImagesInMenus() );
            OUString aIconTheme( rSettings.DetermineIconTheme() );

            PopupMenu* pVCLPopupMenu = static_cast<PopupMenu *>(pPopupMenu->GetMenu());

            if ( m_bShowImages != bShowImages || m_aIconTheme != aIconTheme )
            {
                m_bShowImages = bShowImages;
                m_aIconTheme = aIconTheme;
                setMenuImages( pVCLPopupMenu, m_bShowImages );
            }

            setAccelerators( pVCLPopupMenu );
        }
    }
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
                for ( sal_Int32 y = 0; y < aSeq.getLength(); y++ )
                {
                    if ( aSeq[y].Name == "ooSetupFactoryEmptyDocumentURL" )
                    {
                        aSeq[y].Value >>= m_aEmptyDocURL;
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

    bool bInitalized( m_bInitialized );
    if ( !bInitalized )
    {
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

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
