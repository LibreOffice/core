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

#include <threadhelp/resetableguard.hxx>
#include "services.h"
#include <classes/resource.hrc>
#include <classes/fwkresid.hxx>
#include <framework/bmkmenu.hxx>
#include <framework/imageproducer.hxx>
#include <framework/menuconfiguration.hxx>

#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/awt/MenuItemStyle.hpp>
#include <com/sun/star/ui/ModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/GlobalAcceleratorConfiguration.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>

#include <vcl/svapp.hxx>
#include <vcl/i18nhelp.hxx>
#include <rtl/ustrbuf.hxx>
#include <cppuhelper/implbase1.hxx>
#include <osl/file.hxx>
#include <svtools/menuoptions.hxx>
#include <svtools/acceleratorexecute.hxx>
#include <unotools/moduleoptions.hxx>
#include <osl/mutex.hxx>

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace com::sun::star::container;
using namespace com::sun::star::ui;

static const char SFX_REFERER_USER[] = "private:user";

namespace framework
{

DEFINE_XSERVICEINFO_MULTISERVICE_2      (   NewMenuController                           ,
                                            OWeakObject                                 ,
                                            SERVICENAME_POPUPMENUCONTROLLER             ,
                                            IMPLEMENTATIONNAME_NEWMENUCONTROLLER
                                        )

DEFINE_INIT_SERVICE                     (   NewMenuController, {} )

void NewMenuController::setMenuImages( PopupMenu* pPopupMenu, sal_Bool bSetImages )
{
    sal_uInt16 nItemCount = pPopupMenu->GetItemCount();
    Image               aImage;
    Reference< XFrame > xFrame( m_xFrame );

    for ( sal_uInt16 i = 0; i < nItemCount; i++ )
    {
        sal_uInt16 nItemId = pPopupMenu->GetItemId( sal::static_int_cast<sal_uInt16>( i ));
        if ( nItemId != 0 )
        {
            if ( bSetImages )
            {
                sal_Bool        bImageSet( sal_False );
                OUString aImageId;

                AddInfoForId::const_iterator pInfo = m_aAddInfoForItem.find( nItemId );
                if ( pInfo != m_aAddInfoForItem.end() )
                    aImageId = pInfo->second.aImageId; // Retrieve image id for menu item

                if ( !aImageId.isEmpty() )
                {
                    aImage = GetImageFromURL( xFrame, aImageId, false );
                    if ( !!aImage )
                    {
                        bImageSet = sal_True;
                        pPopupMenu->SetItemImage( nItemId, aImage );
                    }
                }

                if ( !bImageSet )
                {
                    OUString aCmd( pPopupMenu->GetItemCommand( nItemId ) );
                    if ( !aCmd.isEmpty() )
                        aImage = GetImageFromURL( xFrame, aCmd, false );

                    if ( !!aImage )
                        pPopupMenu->SetItemImage( nItemId, aImage );
                }
            }
            else
                pPopupMenu->SetItemImage( nItemId, aImage );
        }
    }
}

void NewMenuController::determineAndSetNewDocAccel( PopupMenu* pPopupMenu, const KeyCode& rKeyCode )
{
    sal_uInt16        nCount( pPopupMenu->GetItemCount() );
    sal_uInt16        nId( 0 );
    sal_Bool      bFound( sal_False );
    OUString aCommand;

    if ( !m_aEmptyDocURL.isEmpty() )
    {
        // Search for the empty document URL

        for ( sal_uInt32 i = 0; i < sal_uInt32( nCount ); i++ )
        {
            nId = pPopupMenu->GetItemId( sal_uInt16( i ));
            if ( nId != 0 && pPopupMenu->GetItemType( nId ) != MENUITEM_SEPARATOR )
            {
                aCommand = pPopupMenu->GetItemCommand( nId );
                if ( aCommand.indexOf( m_aEmptyDocURL ) == 0 )
                {
                    pPopupMenu->SetAccelKey( nId, rKeyCode );
                    bFound = sal_True;
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
            for ( sal_uInt32 i = 0; i < sal_uInt32( nCount ); i++ )
            {
                nId = pPopupMenu->GetItemId( sal_uInt16( i ));
                if ( nId != 0 && pPopupMenu->GetItemType( nId ) != MENUITEM_SEPARATOR )
                {
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
            m_bAcceleratorCfg = sal_True;
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
                    ModuleUIConfigurationManagerSupplier::create( m_xContext );
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

        KeyCode                         aEmptyKeyCode;
        sal_uInt32                      nItemCount( pPopupMenu->GetItemCount() );
        std::vector< KeyCode >          aMenuShortCuts;
        std::vector< OUString >    aCmds;
        std::vector< sal_uInt32 >       aIds;
        for ( sal_uInt32 i = 0; i < nItemCount; i++ )
        {
            sal_uInt16 nId( pPopupMenu->GetItemId( sal_uInt16( i )));
            if ( nId & ( pPopupMenu->GetItemType( nId ) != MENUITEM_SEPARATOR ))
            {
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
            retrieveShortcutsFromConfiguration( xGlobalAccelCfg, aSeq, aMenuShortCuts );

        const sal_uInt32 nCount2 = aIds.size();
        for ( sal_uInt32 i = 0; i < nCount2; i++ )
            pPopupMenu->SetAccelKey( sal_uInt16( aIds[i] ), aMenuShortCuts[i] );

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
    std::vector< KeyCode >& aMenuShortCuts )
{
    if ( rAccelCfg.is() )
    {
        try
        {
            com::sun::star::awt::KeyEvent aKeyEvent;
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

NewMenuController::NewMenuController( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext ) :
    svt::PopupMenuControllerBase( xContext ),
    m_bShowImages( sal_True ),
    m_bNewMenu( sal_False ),
    m_bModuleIdentified( sal_False ),
    m_bAcceleratorCfg( sal_False ),
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
    VCLXPopupMenu* pPopupMenu    = (VCLXPopupMenu *)VCLXMenu::GetImplementation( rPopupMenu );
    PopupMenu*     pVCLPopupMenu = 0;

    SolarMutexGuard aSolarMutexGuard;

    resetPopupMenu( rPopupMenu );
    if ( pPopupMenu )
        pVCLPopupMenu = (PopupMenu *)pPopupMenu->GetMenu();

    if ( pVCLPopupMenu )
    {
        MenuConfiguration aMenuCfg( m_xContext );
        BmkMenu* pSubMenu( 0 );

        if ( m_bNewMenu )
            pSubMenu = (BmkMenu*)aMenuCfg.CreateBookmarkMenu( m_xFrame, BOOKMARK_NEWMENU );
        else
            pSubMenu = (BmkMenu*)aMenuCfg.CreateBookmarkMenu( m_xFrame, BOOKMARK_WIZARDMENU );

        // copy entries as we have to use the provided popup menu
        *pVCLPopupMenu = *pSubMenu;

        Image           aImage;
        AddInfo         aAddInfo;

        // retrieve additional parameters from bookmark menu and
        // store it in a boost::unordered_map.
        for ( sal_uInt16 i = 0; i < pSubMenu->GetItemCount(); i++ )
        {
            sal_uInt16 nItemId = pSubMenu->GetItemId( sal::static_int_cast<sal_uInt16>( i ) );
            if (( nItemId != 0 ) &&
                ( pSubMenu->GetItemType( nItemId ) != MENUITEM_SEPARATOR ))
            {
                MenuConfiguration::Attributes* pBmkAttributes = (MenuConfiguration::Attributes *)(pSubMenu->GetUserValue( nItemId ));
                if ( pBmkAttributes != 0 )
                {
                    aAddInfo.aTargetFrame = pBmkAttributes->aTargetFrame;
                    aAddInfo.aImageId     = pBmkAttributes->aImageId;

                    m_aAddInfoForItem.insert( AddInfoForId::value_type( nItemId, aAddInfo ));
                }
            }
        }

        if ( m_bShowImages )
            setMenuImages( pVCLPopupMenu, m_bShowImages );

        delete pSubMenu;
    }
}

// XEventListener
void SAL_CALL NewMenuController::disposing( const EventObject& ) throw ( RuntimeException )
{
    Reference< css::awt::XMenuListener > xHolder(( OWeakObject *)this, UNO_QUERY );

    osl::MutexGuard aLock( m_aMutex );
    m_xFrame.clear();
    m_xDispatch.clear();
    m_xContext.clear();

    if ( m_xPopupMenu.is() )
        m_xPopupMenu->removeMenuListener( Reference< css::awt::XMenuListener >(( OWeakObject *)this, UNO_QUERY ));
    m_xPopupMenu.clear();
}

// XStatusListener
void SAL_CALL NewMenuController::statusChanged( const FeatureStateEvent& ) throw ( RuntimeException )
{
}

// XMenuListener
void SAL_CALL NewMenuController::itemSelected( const css::awt::MenuEvent& rEvent ) throw (RuntimeException)
{
    Reference< css::awt::XPopupMenu > xPopupMenu;
    Reference< XDispatch >            xDispatch;
    Reference< XDispatchProvider >    xDispatchProvider;
    Reference< XComponentContext >    xContext;
    Reference< XURLTransformer >      xURLTransformer;

    osl::ClearableMutexGuard aLock( m_aMutex );
    xPopupMenu          = m_xPopupMenu;
    xDispatchProvider   = Reference< XDispatchProvider >( m_xFrame, UNO_QUERY );
    xContext     = m_xContext;
    xURLTransformer     = m_xURLTransformer;
    aLock.clear();

    css::util::URL aTargetURL;
    Sequence< PropertyValue > aArgsList( 1 );

    if ( xPopupMenu.is() && xDispatchProvider.is() )
    {
        VCLXPopupMenu* pPopupMenu = (VCLXPopupMenu *)VCLXPopupMenu::GetImplementation( xPopupMenu );
        if ( pPopupMenu )
        {
            {
                SolarMutexGuard aSolarMutexGuard;
                PopupMenu* pVCLPopupMenu = (PopupMenu *)pPopupMenu->GetMenu();
                aTargetURL.Complete = pVCLPopupMenu->GetItemCommand( rEvent.MenuId );
            }

            xURLTransformer->parseStrict( aTargetURL );

            aArgsList[0].Name = OUString( "Referer" );
            aArgsList[0].Value = makeAny( OUString(SFX_REFERER_USER ));

            OUString aTargetFrame( m_aTargetFrame );
            AddInfoForId::const_iterator pItem = m_aAddInfoForItem.find( rEvent.MenuId );
            if ( pItem != m_aAddInfoForItem.end() )
                aTargetFrame = pItem->second.aTargetFrame;

            xDispatch = xDispatchProvider->queryDispatch( aTargetURL, aTargetFrame, 0 );
        }
    }

    if ( xDispatch.is() )
    {
        // Call dispatch asychronously as we can be destroyed while dispatch is
        // executed. VCL is not able to survive this as it wants to call listeners
        // after select!!!
        NewDocument* pNewDocument = new NewDocument;
        pNewDocument->xDispatch  = xDispatch;
        pNewDocument->aTargetURL = aTargetURL;
        pNewDocument->aArgSeq    = aArgsList;
        Application::PostUserEvent( STATIC_LINK(0, NewMenuController, ExecuteHdl_Impl), pNewDocument );
    }
}

void SAL_CALL NewMenuController::itemActivated( const css::awt::MenuEvent& ) throw (RuntimeException)
{
    SolarMutexGuard aSolarMutexGuard;
    if ( m_xFrame.is() && m_xPopupMenu.is() )
    {
        VCLXPopupMenu* pPopupMenu = (VCLXPopupMenu *)VCLXPopupMenu::GetImplementation( m_xPopupMenu );
        if ( pPopupMenu )
        {
            const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
            sal_Bool bShowImages( rSettings.GetUseImagesInMenus() );

            PopupMenu* pVCLPopupMenu = (PopupMenu *)pPopupMenu->GetMenu();

            if ( m_bShowImages != bShowImages )
            {
                m_bShowImages = bShowImages;
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
        m_bModuleIdentified = sal_True;

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
void SAL_CALL NewMenuController::initialize( const Sequence< Any >& aArguments ) throw ( Exception, RuntimeException )
{
    osl::MutexGuard aLock( m_aMutex );

    sal_Bool bInitalized( m_bInitialized );
    if ( !bInitalized )
    {
        svt::PopupMenuControllerBase::initialize( aArguments );

        if ( m_bInitialized )
        {
            const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();

            m_bShowImages   = rSettings.GetUseImagesInMenus();
            m_bNewMenu      = m_aCommandURL.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ".uno:AddDirect" ) );
        }
    }
}

IMPL_STATIC_LINK_NOINSTANCE( NewMenuController, ExecuteHdl_Impl, NewDocument*, pNewDocument )
{
/*  i62706: Don't catch all exceptions. We hide all problems here and are not able
            to handle them on higher levels.
    try
    {
*/
        // Asynchronous execution as this can lead to our own destruction!
        // Framework can recycle our current frame and the layout manager disposes all user interface
        // elements if a component gets detached from its frame!
        pNewDocument->xDispatch->dispatch( pNewDocument->aTargetURL, pNewDocument->aArgSeq );
    delete pNewDocument;
    return 0;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
