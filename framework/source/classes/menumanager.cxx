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

#include <classes/menumanager.hxx>
#include <framework/menuconfiguration.hxx>
#include <framework/bmkmenu.hxx>
#include <framework/addonmenu.hxx>
#include <framework/imageproducer.hxx>
#include <threadhelp/resetableguard.hxx>
#include "framework/addonsoptions.hxx"
#include <classes/fwkresid.hxx>
#include <services.h>
#include "classes/resource.hrc"

#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/util/XStringWidth.hpp>

#include <comphelper/processfactory.hxx>

#include <comphelper/extract.hxx>
#include <svtools/menuoptions.hxx>
#include <unotools/historyoptions.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/localfilehelper.hxx>

#include <toolkit/helper/vclunohelper.hxx>
#include <tools/urlobj.hxx>

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <osl/mutex.hxx>
#include <osl/file.hxx>
#include <cppuhelper/implbase1.hxx>

using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;

class StringLength : public ::cppu::WeakImplHelper1< XStringWidth >
{
    public:
        StringLength() {}
        virtual ~StringLength() {}

        // XStringWidth
        sal_Int32 SAL_CALL queryStringWidth( const OUString& aString )
            throw (RuntimeException)
        {
            return aString.getLength();
        }
};

namespace framework
{

// special menu ids/command ids for dynamic popup menus
#define SID_SFX_START           5000
#define SID_NEWDOCDIRECT        (SID_SFX_START + 537)
#define SID_AUTOPILOTMENU       (SID_SFX_START + 1381)
#define SID_ADDONLIST           (SID_SFX_START + 1677)

#define SFX_REFERER_USER        "private:user"

#define aSlotNewDocDirect "slot:5537"
#define aSlotAutoPilot "slot:6381"

#define aSpecialFileMenu "file"
#define aSpecialWindowMenu "window"
#define aSlotSpecialFileMenu "slot:5510"
#define aSlotSpecialWindowMenu "slot:5610"
#define aSlotSpecialToolsMenu "slot:6677"

// special uno commands for picklist and window list
#define aSpecialFileCommand "PickList"
#define aSpecialWindowCommand "WindowList"

const char UNO_COMMAND[] = ".uno:";

MenuManager::MenuManager(
    const Reference< XComponentContext >& rxContext,
    Reference< XFrame >& rFrame, Menu* pMenu, sal_Bool bDelete, sal_Bool bDeleteChildren )
:
    ThreadHelpBase( &Application::GetSolarMutex() ),
    m_xContext(rxContext)
{
    m_bActive           = sal_False;
    m_bDeleteMenu       = bDelete;
    m_bDeleteChildren   = bDeleteChildren;
    m_pVCLMenu          = pMenu;
    m_xFrame            = rFrame;
    m_bInitialized      = sal_False;
    m_bIsBookmarkMenu   = sal_False;
    acquire();
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    m_bShowMenuImages   = rSettings.GetUseImagesInMenus();

    sal_Int32 nAddonsURLPrefixLength = ADDONSPOPUPMENU_URL_PREFIX.getLength();


    sal_uInt16 nItemCount = pMenu->GetItemCount();
    m_aMenuItemHandlerVector.reserve(nItemCount);
    OUString aItemCommand;
    for ( sal_uInt16 i = 0; i < nItemCount; i++ )
    {
        sal_uInt16 nItemId = FillItemCommand(aItemCommand,pMenu, i );
        bool bShowMenuImages( m_bShowMenuImages );

        // overwrite the show icons on menu option?
        if (!bShowMenuImages)
        {
            MenuItemBits nBits = pMenu->GetItemBits( nItemId );
            bShowMenuImages = ( ( nBits & MIB_ICON ) == MIB_ICON );
        }

        PopupMenu* pPopupMenu = pMenu->GetPopupMenu( nItemId );
        if ( pPopupMenu )
        {
            AddMenu(pPopupMenu,aItemCommand,nItemId,bDeleteChildren,bDeleteChildren);
            if (! (( aItemCommand.getLength() > nAddonsURLPrefixLength ) &&
                ( aItemCommand.indexOf( ADDONSPOPUPMENU_URL_PREFIX ) == 0 )) )
            {

                // Create addon popup menu if there exist elements and this is the tools popup menu
                if ( ( nItemId == SID_ADDONLIST || aItemCommand == aSlotSpecialToolsMenu )
                    && AddonMenuManager::HasAddonMenuElements() )
                {
                    AddonMenu*  pSubMenu = AddonMenuManager::CreateAddonMenu( rFrame );
                    if ( pSubMenu && ( pSubMenu->GetItemCount() > 0 ))
                    {
                        sal_uInt16 nCount = 0;
                        if ( pPopupMenu->GetItemType( nCount-1 ) != MENUITEM_SEPARATOR )
                            pPopupMenu->InsertSeparator();

                        // Use resource to load popup menu title
                        OUString aAddonsStrRes(FWK_RESSTR(STR_MENU_ADDONS));
                        pPopupMenu->InsertItem( ITEMID_ADDONLIST, aAddonsStrRes );
                        pPopupMenu->SetPopupMenu( ITEMID_ADDONLIST, pSubMenu );

                        // Set item command for popup menu to enable it for GetImageFromURL
                        const static OUString aSlotString( "slot:" );
                        aItemCommand = aSlotString;
                        aItemCommand += OUString::valueOf( (sal_Int32)ITEMID_ADDONLIST );
                        pPopupMenu->SetItemCommand( ITEMID_ADDONLIST, aItemCommand );

                        AddMenu(pSubMenu,OUString(),nItemId,sal_True,sal_False);
                        // Set image for the addon popup menu item
                        if ( bShowMenuImages && !pPopupMenu->GetItemImage( ITEMID_ADDONLIST ))
                        {
                            Image aImage = GetImageFromURL( rFrame, aItemCommand, false );
                            if ( !!aImage )
                                   pPopupMenu->SetItemImage( ITEMID_ADDONLIST, aImage );
                        }
                    }
                    else
                        delete pSubMenu;
                }
            }
        }
        else
        {
            if ( nItemId == SID_NEWDOCDIRECT || aItemCommand == aSlotNewDocDirect )
            {
                MenuConfiguration aMenuCfg( m_xContext );
                BmkMenu* pSubMenu = (BmkMenu*)aMenuCfg.CreateBookmarkMenu( rFrame, BOOKMARK_NEWMENU );
                pMenu->SetPopupMenu( nItemId, pSubMenu );

                AddMenu(pSubMenu,OUString(),nItemId,sal_True,sal_False);
                if ( bShowMenuImages && !pMenu->GetItemImage( nItemId ))
                {
                    Image aImage = GetImageFromURL( rFrame, aItemCommand, false );
                    if ( !!aImage )
                           pMenu->SetItemImage( nItemId, aImage );
                }
            }
            else if ( nItemId == SID_AUTOPILOTMENU || aItemCommand == aSlotAutoPilot )
            {
                MenuConfiguration aMenuCfg( m_xContext );
                BmkMenu* pSubMenu = (BmkMenu*)aMenuCfg.CreateBookmarkMenu( rFrame, BOOKMARK_WIZARDMENU );
                pMenu->SetPopupMenu( nItemId, pSubMenu );

                AddMenu(pSubMenu,OUString(),nItemId,sal_True,sal_False);


                if ( bShowMenuImages && !pMenu->GetItemImage( nItemId ))
                {
                    Image aImage = GetImageFromURL( rFrame, aItemCommand, false );
                    if ( !!aImage )
                           pMenu->SetItemImage( nItemId, aImage );
                }
            }
            else if ( pMenu->GetItemType( i ) != MENUITEM_SEPARATOR )
            {
                if ( bShowMenuImages )
                {
                    if ( AddonMenuManager::IsAddonMenuId( nItemId ))
                    {
                        // Add-Ons uses a images from different places
                        Image           aImage;
                        OUString   aImageId;

                        MenuConfiguration::Attributes* pMenuAttributes =
                            (MenuConfiguration::Attributes*)pMenu->GetUserValue( nItemId );

                        if ( pMenuAttributes && !pMenuAttributes->aImageId.isEmpty() )
                        {
                            // Retrieve image id from menu attributes
                            aImage = GetImageFromURL( rFrame, aImageId, false );
                        }

                        if ( !aImage )
                        {
                            aImage = GetImageFromURL( rFrame, aItemCommand, false );
                            if ( !aImage )
                                aImage = AddonsOptions().GetImageFromURL( aItemCommand, false );
                        }

                        if ( !!aImage )
                            pMenu->SetItemImage( nItemId, aImage );
                    }
                    else if ( !pMenu->GetItemImage( nItemId ))
                    {
                        Image aImage = GetImageFromURL( rFrame, aItemCommand, false );
                        if ( !!aImage )
                               pMenu->SetItemImage( nItemId, aImage );
                    }
                }

                Reference< XDispatch > aXDispatchRef;
                m_aMenuItemHandlerVector.push_back( new MenuItemHandler( nItemId, NULL, aXDispatchRef ));

            }
        }
    }

    // retrieve label information for all menu items without item text

    SetHdl();
}


void MenuManager::SetHdl()
{
    m_pVCLMenu->SetHighlightHdl( LINK( this, MenuManager, Highlight ));
    m_pVCLMenu->SetActivateHdl( LINK( this, MenuManager, Activate ));
    m_pVCLMenu->SetDeactivateHdl( LINK( this, MenuManager, Deactivate ));
    m_pVCLMenu->SetSelectHdl( LINK( this, MenuManager, Select ));

    if ( m_xContext.is() )
        m_xURLTransformer.set( URLTransformer::create( m_xContext ) );
}

MenuManager::~MenuManager()
{
    std::vector< MenuItemHandler* >::iterator p;
    for ( p = m_aMenuItemHandlerVector.begin(); p != m_aMenuItemHandlerVector.end(); ++p )
    {
        MenuItemHandler* pItemHandler = *p;
        pItemHandler->xMenuItemDispatch.clear();
        if ( pItemHandler->pSubMenuManager )
            (static_cast< XInterface* >((OWeakObject*)pItemHandler->pSubMenuManager))->release();
        delete pItemHandler;
    }

    if ( m_bDeleteMenu )
        delete m_pVCLMenu;
}


MenuManager::MenuItemHandler* MenuManager::GetMenuItemHandler( sal_uInt16 nItemId )
{
    ResetableGuard aGuard( m_aLock );

    std::vector< MenuItemHandler* >::iterator p;
    for ( p = m_aMenuItemHandlerVector.begin(); p != m_aMenuItemHandlerVector.end(); ++p )
    {
        MenuItemHandler* pItemHandler = *p;
        if ( pItemHandler->nItemId == nItemId )
            return pItemHandler;
    }

    return 0;
}


void SAL_CALL MenuManager::statusChanged( const FeatureStateEvent& Event )
throw ( RuntimeException )
{
    OUString aFeatureURL = Event.FeatureURL.Complete;
    MenuItemHandler* pStatusChangedMenu = NULL;

    {
        ResetableGuard aGuard( m_aLock );

        std::vector< MenuItemHandler* >::iterator p;
        for ( p = m_aMenuItemHandlerVector.begin(); p != m_aMenuItemHandlerVector.end(); ++p )
        {
            MenuItemHandler* pMenuItemHandler = *p;
            if ( pMenuItemHandler->aMenuItemURL == aFeatureURL )
            {
                pStatusChangedMenu = pMenuItemHandler;
                break;
            }
        }
    }

    if ( pStatusChangedMenu )
    {
        SolarMutexGuard aSolarGuard;
        {
            ResetableGuard aGuard( m_aLock );

            sal_Bool bSetCheckmark      = sal_False;
            sal_Bool bCheckmark         = sal_False;
            sal_Bool bMenuItemEnabled   = m_pVCLMenu->IsItemEnabled( pStatusChangedMenu->nItemId );

            if ( Event.IsEnabled != bMenuItemEnabled )
                m_pVCLMenu->EnableItem( pStatusChangedMenu->nItemId, Event.IsEnabled );

            if ( Event.State >>= bCheckmark )
                 bSetCheckmark = sal_True;

            if ( bSetCheckmark )
                m_pVCLMenu->CheckItem( pStatusChangedMenu->nItemId, bCheckmark );
        }

        if ( Event.Requery )
        {
            URL aTargetURL;
            aTargetURL.Complete = pStatusChangedMenu->aMenuItemURL;

            m_xURLTransformer->parseStrict( aTargetURL );

            Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
            Reference< XDispatch > xMenuItemDispatch = xDispatchProvider->queryDispatch(
                                                            aTargetURL, OUString(), 0 );

            if ( xMenuItemDispatch.is() )
            {
                pStatusChangedMenu->xMenuItemDispatch   = xMenuItemDispatch;
                pStatusChangedMenu->aMenuItemURL        = aTargetURL.Complete;
                xMenuItemDispatch->addStatusListener( (static_cast< XStatusListener* >(this)), aTargetURL );
            }
        }
    }
}


void MenuManager::RemoveListener()
{
    ResetableGuard aGuard( m_aLock );
    ClearMenuDispatch();
}

void MenuManager::ClearMenuDispatch(const EventObject& Source,bool _bRemoveOnly)
{
    // disposing called from parent dispatcher
    // remove all listener to prepare shutdown

    std::vector< MenuItemHandler* >::iterator p;
    for ( p = m_aMenuItemHandlerVector.begin(); p != m_aMenuItemHandlerVector.end(); ++p )
    {
        MenuItemHandler* pItemHandler = *p;
        if ( pItemHandler->xMenuItemDispatch.is() )
        {
            URL aTargetURL;
            aTargetURL.Complete = pItemHandler->aMenuItemURL;
            m_xURLTransformer->parseStrict( aTargetURL );

            pItemHandler->xMenuItemDispatch->removeStatusListener(
                (static_cast< XStatusListener* >(this)), aTargetURL );
        }

        pItemHandler->xMenuItemDispatch.clear();
        if ( pItemHandler->pSubMenuManager )
        {
            if ( _bRemoveOnly )
                pItemHandler->pSubMenuManager->RemoveListener();
            else
                pItemHandler->pSubMenuManager->disposing( Source );
        }
    }
}


void SAL_CALL MenuManager::disposing( const EventObject& Source ) throw ( RuntimeException )
{
    if ( Source.Source == m_xFrame )
    {
        ResetableGuard aGuard( m_aLock );
        ClearMenuDispatch(Source,false);
    }
    else
    {
        // disposing called from menu item dispatcher, remove listener
        MenuItemHandler* pMenuItemDisposing = NULL;

        {
            ResetableGuard aGuard( m_aLock );

            std::vector< MenuItemHandler* >::iterator p;
            for ( p = m_aMenuItemHandlerVector.begin(); p != m_aMenuItemHandlerVector.end(); ++p )
            {
                MenuItemHandler* pMenuItemHandler = *p;
                if ( pMenuItemHandler->xMenuItemDispatch == Source.Source )
                {
                    pMenuItemDisposing = pMenuItemHandler;
                    break;
                }
            }

            if ( pMenuItemDisposing )
            {
                URL aTargetURL;
                aTargetURL.Complete = pMenuItemDisposing->aMenuItemURL;

                m_xURLTransformer->parseStrict( aTargetURL );

                pMenuItemDisposing->xMenuItemDispatch->removeStatusListener((static_cast< XStatusListener* >(this)), aTargetURL );
                pMenuItemDisposing->xMenuItemDispatch.clear();
            }
        }
    }
}


void MenuManager::UpdateSpecialFileMenu( Menu* pMenu )
{
    // update picklist
    Sequence< Sequence< PropertyValue > > aHistoryList = SvtHistoryOptions().GetList( ePICKLIST );
    ::std::vector< MenuItemHandler* > aNewPickVector;
    Reference< XStringWidth > xStringLength( new StringLength );

    sal_uInt16  nPickItemId = START_ITEMID_PICKLIST;
    int     nPickListMenuItems = ( aHistoryList.getLength() > 99 ) ? 99 : aHistoryList.getLength();

    aNewPickVector.reserve(nPickListMenuItems);
    for ( int i = 0; i < nPickListMenuItems; i++ )
    {
        Sequence< PropertyValue > aPickListEntry = aHistoryList[i];

        Reference< XDispatch > aXDispatchRef;
        MenuItemHandler* pNewMenuItemHandler = new MenuItemHandler(
                                                    nPickItemId++,
                                                    NULL,
                                                    aXDispatchRef );

        for ( int j = 0; j < aPickListEntry.getLength(); j++ )
        {
            Any a = aPickListEntry[j].Value;

            if ( aPickListEntry[j].Name == HISTORY_PROPERTYNAME_URL )
                a >>= pNewMenuItemHandler->aMenuItemURL;
            else if ( aPickListEntry[j].Name == HISTORY_PROPERTYNAME_FILTER )
                a >>= pNewMenuItemHandler->aFilter;
            else if ( aPickListEntry[j].Name == HISTORY_PROPERTYNAME_TITLE )
                a >>= pNewMenuItemHandler->aTitle;
            else if ( aPickListEntry[j].Name == HISTORY_PROPERTYNAME_PASSWORD )
                a >>= pNewMenuItemHandler->aPassword;
        }

        aNewPickVector.push_back( pNewMenuItemHandler );
    }

    if ( !aNewPickVector.empty() )
    {
        URL aTargetURL;
        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );

        Reference< XDispatch > xMenuItemDispatch;

        static const OUString s_sDefault("_default");
        // query for dispatcher
        std::vector< MenuItemHandler* >::iterator p;
        for ( p = aNewPickVector.begin(); p != aNewPickVector.end(); ++p )
        {
            MenuItemHandler* pMenuItemHandler = *p;

            aTargetURL.Complete = pMenuItemHandler->aMenuItemURL;
            m_xURLTransformer->parseStrict( aTargetURL );

            if ( !xMenuItemDispatch.is() )
            {
                // attention: this code assume that "_blank" can only be consumed by desktop service
                xMenuItemDispatch = xDispatchProvider->queryDispatch( aTargetURL, s_sDefault, 0 );
            }

            if ( xMenuItemDispatch.is() )
            {
                pMenuItemHandler->xMenuItemDispatch = xMenuItemDispatch;
                pMenuItemHandler->aMenuItemURL      = aTargetURL.Complete;
            }
        }

        {
            ResetableGuard aGuard( m_aLock );

            int nRemoveItemCount = 0;
            int nItemCount       = pMenu->GetItemCount();

            if ( nItemCount > 0 )
            {
                // remove all old picklist entries from menu
                sal_uInt16 nPos = pMenu->GetItemPos( START_ITEMID_PICKLIST );
                for ( sal_uInt16 n = nPos; n < pMenu->GetItemCount(); )
                {
                    pMenu->RemoveItem( n );
                    ++nRemoveItemCount;
                }

                if ( pMenu->GetItemType( pMenu->GetItemCount()-1 ) == MENUITEM_SEPARATOR )
                    pMenu->RemoveItem( pMenu->GetItemCount()-1 );

                // remove all old picklist entries from menu handler
                if ( nRemoveItemCount > 0 )
                {
                    for( sal_uInt32 nIndex = m_aMenuItemHandlerVector.size() - nRemoveItemCount;
                         nIndex < m_aMenuItemHandlerVector.size();  )
                    {
                        delete m_aMenuItemHandlerVector.at( nIndex );
                        m_aMenuItemHandlerVector.erase( m_aMenuItemHandlerVector.begin() + nIndex );
                    }
                }
            }

            // append new picklist menu entries
            aNewPickVector.reserve(aNewPickVector.size());
            pMenu->InsertSeparator();
            const sal_uInt32 nCount = aNewPickVector.size();
            for ( sal_uInt32 i = 0; i < nCount; i++ )
            {
                char menuShortCut[5] = "~n: ";

                OUString aMenuShortCut;
                if ( i <= 9 )
                {
                    if ( i == 9 )
                        aMenuShortCut = OUString( "1~0: " );
                    else
                    {
                        menuShortCut[1] = (char)( '1' + i );
                        aMenuShortCut = OUString::createFromAscii( menuShortCut );
                    }
                }
                else
                {
                    aMenuShortCut = OUString::valueOf((sal_Int32)( i + 1 ));
                    aMenuShortCut += ": ";
                }

                // Abbreviate URL
                OUString   aURLString( aNewPickVector.at( i )->aMenuItemURL );
                OUString   aTipHelpText;
                OUString   aMenuTitle;
                INetURLObject   aURL( aURLString );

                if ( aURL.GetProtocol() == INET_PROT_FILE )
                {
                    // Do handle file URL differently => convert it to a system
                    // path and abbreviate it with a special function:
                    String aFileSystemPath( aURL.getFSysPath( INetURLObject::FSYS_DETECT ) );

                    OUString aSystemPath( aFileSystemPath );
                    OUString aCompactedSystemPath;

                    aTipHelpText = aSystemPath;
                    oslFileError nError = osl_abbreviateSystemPath( aSystemPath.pData, &aCompactedSystemPath.pData, 46, NULL );
                    if ( !nError )
                        aMenuTitle = String( aCompactedSystemPath );
                    else
                        aMenuTitle = aSystemPath;
                }
                else
                {
                    // Use INetURLObject to abbreviate all other URLs
                    String  aShortURL;
                    aShortURL = aURL.getAbbreviated( xStringLength, 46, INetURLObject::DECODE_UNAMBIGUOUS );
                    aMenuTitle += aShortURL;
                    aTipHelpText = aURLString;
                }

                OUString aTitle( aMenuShortCut + aMenuTitle );

                MenuItemHandler* pMenuItemHandler = aNewPickVector.at( i );
                pMenu->InsertItem( pMenuItemHandler->nItemId, aTitle );
                pMenu->SetTipHelpText( pMenuItemHandler->nItemId, aTipHelpText );
                m_aMenuItemHandlerVector.push_back( pMenuItemHandler );
            }
        }
    }
}

void MenuManager::UpdateSpecialWindowMenu( Menu* pMenu,const Reference< XComponentContext >& xContext,framework::IMutex& _rMutex )
{
    // update window list
    ::std::vector< OUString > aNewWindowListVector;

    Reference< XDesktop2 > xDesktop = Desktop::create( xContext );

    sal_uInt16  nActiveItemId = 0;
    sal_uInt16  nItemId = START_ITEMID_WINDOWLIST;

    Reference< XFrame > xCurrentFrame = xDesktop->getCurrentFrame();
    Reference< XIndexAccess > xList( xDesktop->getFrames(), UNO_QUERY );
    sal_Int32 nFrameCount = xList->getCount();
    aNewWindowListVector.reserve(nFrameCount);
    for (sal_Int32 i=0; i<nFrameCount; ++i )
    {
        Reference< XFrame > xFrame;
        xList->getByIndex(i) >>= xFrame;

        if (xFrame.is())
        {
            if ( xFrame == xCurrentFrame )
                nActiveItemId = nItemId;

            Window* pWin = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
            if ( pWin && pWin->IsVisible() )
            {
                aNewWindowListVector.push_back( pWin->GetText() );
                ++nItemId;
            }
        }
    }

    {
        ResetableGuard aGuard( _rMutex );

        int nItemCount = pMenu->GetItemCount();

        if ( nItemCount > 0 )
        {
            // remove all old window list entries from menu
            sal_uInt16 nPos = pMenu->GetItemPos( START_ITEMID_WINDOWLIST );
            for ( sal_uInt16 n = nPos; n < pMenu->GetItemCount(); )
                pMenu->RemoveItem( n );

            if ( pMenu->GetItemType( pMenu->GetItemCount()-1 ) == MENUITEM_SEPARATOR )
                pMenu->RemoveItem( pMenu->GetItemCount()-1 );
        }

        if ( !aNewWindowListVector.empty() )
        {
            // append new window list entries to menu
            pMenu->InsertSeparator();
            nItemId = START_ITEMID_WINDOWLIST;
            const sal_uInt32 nCount = aNewWindowListVector.size();
            for ( sal_uInt32 i = 0; i < nCount; i++ )
            {
                pMenu->InsertItem( nItemId, aNewWindowListVector.at( i ), MIB_RADIOCHECK );
                if ( nItemId == nActiveItemId )
                    pMenu->CheckItem( nItemId );
                ++nItemId;
            }
        }
    }
}


void MenuManager::CreatePicklistArguments( Sequence< PropertyValue >& aArgsList, const MenuItemHandler* pMenuItemHandler )
{
    int NUM_OF_PICKLIST_ARGS = 3;

    Any a;
    aArgsList.realloc( NUM_OF_PICKLIST_ARGS );

    aArgsList[0].Name = OUString( "FileName" );
    a <<= pMenuItemHandler->aMenuItemURL;
    aArgsList[0].Value = a;

    aArgsList[1].Name = OUString( "Referer" );
    a <<= OUString( SFX_REFERER_USER );
    aArgsList[1].Value = a;

    OUString aFilter( pMenuItemHandler->aFilter );

    sal_Int32 nPos = aFilter.indexOf( '|' );
    if ( nPos >= 0 )
    {
        OUString aFilterOptions;

        if ( nPos < ( aFilter.getLength() - 1 ) )
            aFilterOptions = aFilter.copy( nPos+1 );

        aArgsList[2].Name = OUString( "FilterOptions" );
        a <<= aFilterOptions;
        aArgsList[2].Value = a;

        aFilter = aFilter.copy( 0, nPos-1 );
        aArgsList.realloc( ++NUM_OF_PICKLIST_ARGS );
    }

    aArgsList[NUM_OF_PICKLIST_ARGS-1].Name = OUString( "FilterName" );
    a <<= aFilter;
    aArgsList[NUM_OF_PICKLIST_ARGS-1].Value = a;
}


//_________________________________________________________________________________________________________________
// vcl handler
//_________________________________________________________________________________________________________________

IMPL_LINK( MenuManager, Activate, Menu *, pMenu )
{
    if ( pMenu == m_pVCLMenu )
    {
        // set/unset hiding disabled menu entries
        sal_Bool bDontHide          = SvtMenuOptions().IsEntryHidingEnabled();
        const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
        sal_Bool bShowMenuImages    = rSettings.GetUseImagesInMenus();

        sal_uInt16 nFlag = pMenu->GetMenuFlags();
        if ( bDontHide )
            nFlag &= ~MENU_FLAG_HIDEDISABLEDENTRIES;
        else
            nFlag |= MENU_FLAG_HIDEDISABLEDENTRIES;
        pMenu->SetMenuFlags( nFlag );

        if ( m_bActive )
            return 0;

        m_bActive = sal_True;

        OUString aCommand( m_aMenuItemCommand );
        if (m_aMenuItemCommand.matchIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM(UNO_COMMAND)))
        {
            // Remove protocol part from command so we can use an easier comparision method
            aCommand = aCommand.copy(RTL_CONSTASCII_LENGTH(UNO_COMMAND));
        }

        if ( m_aMenuItemCommand == aSpecialFileMenu || m_aMenuItemCommand == aSlotSpecialFileMenu || aCommand == aSpecialFileCommand )
            UpdateSpecialFileMenu( pMenu );
        else if ( m_aMenuItemCommand == aSpecialWindowMenu || m_aMenuItemCommand == aSlotSpecialWindowMenu || aCommand == aSpecialWindowCommand )
            UpdateSpecialWindowMenu( pMenu, m_xContext, m_aLock );

        // Check if some modes have changed so we have to update our menu images
        if ( bShowMenuImages != m_bShowMenuImages )
        {
            // The mode changed so we have to replace all images
            m_bShowMenuImages   = bShowMenuImages;
            FillMenuImages( m_xFrame, pMenu, bShowMenuImages );
        }

        if ( m_bInitialized )
            return 0;
        else
        {
            URL aTargetURL;

            ResetableGuard aGuard( m_aLock );

            Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
            if ( xDispatchProvider.is() )
            {
                std::vector< MenuItemHandler* >::iterator p;
                for ( p = m_aMenuItemHandlerVector.begin(); p != m_aMenuItemHandlerVector.end(); ++p )
                {
                    MenuItemHandler* pMenuItemHandler = *p;
                    if ( pMenuItemHandler &&
                         pMenuItemHandler->pSubMenuManager == 0 &&
                         !pMenuItemHandler->xMenuItemDispatch.is() )
                    {
                        // There is no dispatch mechanism for the special window list menu items,
                        // because they are handled directly through XFrame->activate!!!
                        if ( pMenuItemHandler->nItemId < START_ITEMID_WINDOWLIST ||
                             pMenuItemHandler->nItemId > END_ITEMID_WINDOWLIST )
                        {
                            OUString aItemCommand = pMenu->GetItemCommand( pMenuItemHandler->nItemId );
                            if ( aItemCommand.isEmpty() )
                            {
                                const static OUString aSlotString( "slot:" );
                                aItemCommand = aSlotString;
                                aItemCommand += OUString::valueOf( (sal_Int32)pMenuItemHandler->nItemId );
                                pMenu->SetItemCommand( pMenuItemHandler->nItemId, aItemCommand );
                            }

                            aTargetURL.Complete = aItemCommand;

                            m_xURLTransformer->parseStrict( aTargetURL );

                            Reference< XDispatch > xMenuItemDispatch;
                            if ( m_bIsBookmarkMenu )
                                xMenuItemDispatch = xDispatchProvider->queryDispatch( aTargetURL, pMenuItemHandler->aTargetFrame, 0 );
                            else
                                xMenuItemDispatch = xDispatchProvider->queryDispatch( aTargetURL, OUString(), 0 );

                            if ( xMenuItemDispatch.is() )
                            {
                                pMenuItemHandler->xMenuItemDispatch = xMenuItemDispatch;
                                pMenuItemHandler->aMenuItemURL      = aTargetURL.Complete;
                                xMenuItemDispatch->addStatusListener( (static_cast< XStatusListener* >(this)), aTargetURL );
                            }
                            else
                                pMenu->EnableItem( pMenuItemHandler->nItemId, sal_False );
                        }
                    }
                }
            }
        }
    }

    return 1;
}


IMPL_LINK( MenuManager, Deactivate, Menu *, pMenu )
{
    if ( pMenu == m_pVCLMenu )
        m_bActive = sal_False;

    return 1;
}


IMPL_LINK( MenuManager, Select, Menu *, pMenu )
{
    URL                     aTargetURL;
    Sequence<PropertyValue> aArgs;
    Reference< XDispatch >  xDispatch;

    {
        ResetableGuard aGuard( m_aLock );

        sal_uInt16 nCurItemId = pMenu->GetCurItemId();
        if ( pMenu == m_pVCLMenu &&
             pMenu->GetItemType( nCurItemId ) != MENUITEM_SEPARATOR )
        {
            if ( nCurItemId >= START_ITEMID_WINDOWLIST &&
                 nCurItemId <= END_ITEMID_WINDOWLIST )
            {
                // window list menu item selected

                Reference< XDesktop2 > xDesktop = Desktop::create( m_xContext );

                sal_uInt16 nTaskId = START_ITEMID_WINDOWLIST;
                Reference< XIndexAccess > xList( xDesktop->getFrames(), UNO_QUERY );
                sal_Int32 nCount = xList->getCount();
                for ( sal_Int32 i=0; i<nCount; ++i )
                {
                    Reference< XFrame > xFrame;
                    xList->getByIndex(i) >>= xFrame;

                    if ( xFrame.is() && nTaskId == nCurItemId )
                    {
                        Window* pWin = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
                        pWin->GrabFocus();
                        pWin->ToTop( TOTOP_RESTOREWHENMIN );
                        break;
                    }

                    nTaskId++;
                }
            }
            else
            {
                MenuItemHandler* pMenuItemHandler = GetMenuItemHandler( nCurItemId );
                if ( pMenuItemHandler && pMenuItemHandler->xMenuItemDispatch.is() )
                {
                    aTargetURL.Complete = pMenuItemHandler->aMenuItemURL;
                    m_xURLTransformer->parseStrict( aTargetURL );

                    if ( nCurItemId >= START_ITEMID_PICKLIST &&
                         nCurItemId <  START_ITEMID_WINDOWLIST )
                    {
                        // picklist menu item selected
                        CreatePicklistArguments( aArgs, pMenuItemHandler );
                    }
                    else if ( m_bIsBookmarkMenu )
                    {
                        // bookmark menu item selected
                        aArgs.realloc( 1 );
                        aArgs[0].Name = OUString( "Referer" );
                        aArgs[0].Value <<= OUString( SFX_REFERER_USER );
                    }

                    xDispatch = pMenuItemHandler->xMenuItemDispatch;
                }
            }
        }
    }

    if ( xDispatch.is() )
        xDispatch->dispatch( aTargetURL, aArgs );

    return 1;
}


IMPL_LINK_NOARG(MenuManager, Highlight)
{
    return 0;
}

const Reference< XComponentContext >& MenuManager::getContext()
{
    return m_xContext;
}

void MenuManager::AddMenu(PopupMenu* _pPopupMenu,const OUString& _sItemCommand,sal_uInt16 _nItemId,sal_Bool _bDelete,sal_Bool _bDeleteChildren)
{
    MenuManager* pSubMenuManager = new MenuManager( m_xContext, m_xFrame, _pPopupMenu, _bDelete, _bDeleteChildren );

    // store menu item command as we later have to know which menu is active (see Activate handler)
    pSubMenuManager->m_aMenuItemCommand = _sItemCommand;

    Reference< XDispatch > aXDispatchRef;
    MenuItemHandler* pMenuItemHandler = new MenuItemHandler(
                                                _nItemId,
                                                pSubMenuManager,
                                                aXDispatchRef );
    m_aMenuItemHandlerVector.push_back( pMenuItemHandler );
}

sal_uInt16 MenuManager::FillItemCommand(OUString& _rItemCommand, Menu* _pMenu,sal_uInt16 _nIndex) const
{
    sal_uInt16 nItemId = _pMenu->GetItemId( _nIndex );

    _rItemCommand = _pMenu->GetItemCommand( nItemId );
    if ( _rItemCommand.isEmpty() )
    {
        const static OUString aSlotString( "slot:" );
        _rItemCommand = aSlotString;
        _rItemCommand += OUString::valueOf( (sal_Int32)nItemId );
        _pMenu->SetItemCommand( nItemId, _rItemCommand );
    }
    return nItemId;
}
void MenuManager::FillMenuImages(Reference< XFrame >& _xFrame, Menu* _pMenu,sal_Bool bShowMenuImages)
{
    AddonsOptions       aAddonOptions;

    for ( sal_uInt16 nPos = 0; nPos < _pMenu->GetItemCount(); nPos++ )
    {
        sal_uInt16 nId = _pMenu->GetItemId( nPos );
        if ( _pMenu->GetItemType( nPos ) != MENUITEM_SEPARATOR )
        {
            bool bTmpShowMenuImages( bShowMenuImages );
            // overwrite the show icons on menu option?
            if (!bTmpShowMenuImages)
            {
                MenuItemBits nBits =  _pMenu->GetItemBits( nId );
                bTmpShowMenuImages = ( ( nBits & MIB_ICON ) == MIB_ICON );
            }

            if ( bTmpShowMenuImages )
            {
                sal_Bool        bImageSet = sal_False;
                OUString aImageId;

                ::framework::MenuConfiguration::Attributes* pMenuAttributes =
                    (::framework::MenuConfiguration::Attributes*)_pMenu->GetUserValue( nId );

                if ( pMenuAttributes )
                    aImageId = pMenuAttributes->aImageId; // Retrieve image id from menu attributes

                if ( !aImageId.isEmpty() )
                {
                    Image aImage = GetImageFromURL( _xFrame, aImageId, false );
                    if ( !!aImage )
                    {
                        bImageSet = sal_True;
                        _pMenu->SetItemImage( nId, aImage );
                    }
                }

                if ( !bImageSet )
                {
                    OUString aMenuItemCommand = _pMenu->GetItemCommand( nId );
                    Image aImage = GetImageFromURL( _xFrame, aMenuItemCommand, false );
                    if ( !aImage )
                        aImage = aAddonOptions.GetImageFromURL( aMenuItemCommand, false );

                    _pMenu->SetItemImage( nId, aImage );
                }
            }
            else
                _pMenu->SetItemImage( nId, Image() );
        }
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
