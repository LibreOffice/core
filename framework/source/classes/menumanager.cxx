/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
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
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/util/XStringWidth.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/componentcontext.hxx>

#include <comphelper/extract.hxx>
#include <svtools/menuoptions.hxx>
#include <unotools/historyoptions.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/localfilehelper.hxx>

#include <toolkit/unohlp.hxx>
#include <tools/urlobj.hxx>

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <osl/file.hxx>
#include <cppuhelper/implbase1.hxx>

using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;


class StringLength : public ::cppu::WeakImplHelper1< ::com::sun::star::util::XStringWidth >
{
    public:
        StringLength() {}
        virtual ~StringLength() {}

        // XStringWidth
        sal_Int32 SAL_CALL queryStringWidth( const ::rtl::OUString& aString )
            throw (::com::sun::star::uno::RuntimeException)
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
#define SID_PICKLIST            (SID_SFX_START + 510)
#define SID_MDIWINDOWLIST       (SID_SFX_START + 610)
#define SID_ADDONLIST           (SID_SFX_START + 1677)
#define SID_HELPMENU            (SID_SFX_START + 410)

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
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
    REFERENCE< XFRAME >& rFrame, Menu* pMenu, sal_Bool bDelete, sal_Bool bDeleteChildren )
:
    ThreadHelpBase( &Application::GetSolarMutex() ),
    mxServiceFactory(xServiceFactory)
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
    ::rtl::OUString aItemCommand;
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
                        String aAddonsStrRes = String( FwkResId( STR_MENU_ADDONS ));
                        pPopupMenu->InsertItem( ITEMID_ADDONLIST, aAddonsStrRes );
                        pPopupMenu->SetPopupMenu( ITEMID_ADDONLIST, pSubMenu );

                        // Set item command for popup menu to enable it for GetImageFromURL
                        const static ::rtl::OUString aSlotString( RTL_CONSTASCII_USTRINGPARAM( "slot:" ));
                        aItemCommand = aSlotString;
                        aItemCommand += ::rtl::OUString::valueOf( (sal_Int32)ITEMID_ADDONLIST );
                        pPopupMenu->SetItemCommand( ITEMID_ADDONLIST, aItemCommand );

                        AddMenu(pSubMenu,::rtl::OUString(),nItemId,sal_True,sal_False);
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
                MenuConfiguration aMenuCfg( getServiceFactory() );
                BmkMenu* pSubMenu = (BmkMenu*)aMenuCfg.CreateBookmarkMenu( rFrame, BOOKMARK_NEWMENU );
                pMenu->SetPopupMenu( nItemId, pSubMenu );

                AddMenu(pSubMenu,::rtl::OUString(),nItemId,sal_True,sal_False);
                if ( bShowMenuImages && !pMenu->GetItemImage( nItemId ))
                {
                    Image aImage = GetImageFromURL( rFrame, aItemCommand, false );
                    if ( !!aImage )
                           pMenu->SetItemImage( nItemId, aImage );
                }
            }
            else if ( nItemId == SID_AUTOPILOTMENU || aItemCommand == aSlotAutoPilot )
            {
                MenuConfiguration aMenuCfg( getServiceFactory() );
                BmkMenu* pSubMenu = (BmkMenu*)aMenuCfg.CreateBookmarkMenu( rFrame, BOOKMARK_WIZARDMENU );
                pMenu->SetPopupMenu( nItemId, pSubMenu );

                AddMenu(pSubMenu,::rtl::OUString(),nItemId,sal_True,sal_False);


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
                        rtl::OUString   aImageId;

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

                REFERENCE< XDISPATCH > aXDispatchRef;
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

    if ( mxServiceFactory.is() )
        m_xURLTransformer.set( URLTransformer::create(::comphelper::ComponentContext(mxServiceFactory).getUNOContext()) );
}

MenuManager::~MenuManager()
{
    std::vector< MenuItemHandler* >::iterator p;
    for ( p = m_aMenuItemHandlerVector.begin(); p != m_aMenuItemHandlerVector.end(); ++p )
    {
        MenuItemHandler* pItemHandler = *p;
        pItemHandler->xMenuItemDispatch.clear();
        if ( pItemHandler->pSubMenuManager )
            (static_cast< ::com::sun::star::uno::XInterface* >((OWeakObject*)pItemHandler->pSubMenuManager))->release();
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


void SAL_CALL MenuManager::statusChanged( const FEATURSTATEEVENT& Event )
throw ( RuntimeException )
{
    ::rtl::OUString aFeatureURL = Event.FeatureURL.Complete;
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

            REFERENCE< XDISPATCHPROVIDER > xDispatchProvider( m_xFrame, UNO_QUERY );
            REFERENCE< XDISPATCH > xMenuItemDispatch = xDispatchProvider->queryDispatch(
                                                            aTargetURL, ::rtl::OUString(), 0 );

            if ( xMenuItemDispatch.is() )
            {
                pStatusChangedMenu->xMenuItemDispatch   = xMenuItemDispatch;
                pStatusChangedMenu->aMenuItemURL        = aTargetURL.Complete;
                xMenuItemDispatch->addStatusListener( (static_cast< XSTATUSLISTENER* >(this)), aTargetURL );
            }
        }
    }
}


void MenuManager::RemoveListener()
{
    ResetableGuard aGuard( m_aLock );
    ClearMenuDispatch();
}

void MenuManager::ClearMenuDispatch(const EVENTOBJECT& Source,bool _bRemoveOnly)
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
                (static_cast< XSTATUSLISTENER* >(this)), aTargetURL );
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


void SAL_CALL MenuManager::disposing( const EVENTOBJECT& Source ) throw ( RUNTIMEEXCEPTION )
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

                pMenuItemDisposing->xMenuItemDispatch->removeStatusListener((static_cast< XSTATUSLISTENER* >(this)), aTargetURL );
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

        REFERENCE< XDISPATCH > aXDispatchRef;
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
        REFERENCE< XDISPATCHPROVIDER > xDispatchProvider( m_xFrame, UNO_QUERY );

        REFERENCE< XDISPATCH > xMenuItemDispatch;

        static const ::rtl::OUString s_sDefault(RTL_CONSTASCII_USTRINGPARAM("_default"));
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

                ::rtl::OUString aMenuShortCut;
                if ( i <= 9 )
                {
                    if ( i == 9 )
                        aMenuShortCut = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "1~0: " ));
                    else
                    {
                        menuShortCut[1] = (char)( '1' + i );
                        aMenuShortCut = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( menuShortCut ));
                    }
                }
                else
                {
                    aMenuShortCut = rtl::OUString::valueOf((sal_Int32)( i + 1 ));
                    aMenuShortCut += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ": " ));
                }

                // Abbreviate URL
                rtl::OUString   aURLString( aNewPickVector.at( i )->aMenuItemURL );
                rtl::OUString   aTipHelpText;
                rtl::OUString   aMenuTitle;
                INetURLObject   aURL( aURLString );

                if ( aURL.GetProtocol() == INET_PROT_FILE )
                {
                    // Do handle file URL differently => convert it to a system
                    // path and abbreviate it with a special function:
                    String aFileSystemPath( aURL.getFSysPath( INetURLObject::FSYS_DETECT ) );

                    ::rtl::OUString aSystemPath( aFileSystemPath );
                    ::rtl::OUString aCompactedSystemPath;

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

                ::rtl::OUString aTitle( aMenuShortCut + aMenuTitle );

                MenuItemHandler* pMenuItemHandler = aNewPickVector.at( i );
                pMenu->InsertItem( pMenuItemHandler->nItemId, aTitle );
                pMenu->SetTipHelpText( pMenuItemHandler->nItemId, aTipHelpText );
                m_aMenuItemHandlerVector.push_back( pMenuItemHandler );
            }
        }
    }
}

void MenuManager::UpdateSpecialWindowMenu( Menu* pMenu,const Reference< XMultiServiceFactory >& xServiceFactory,framework::IMutex& _rMutex )
{
    // update window list
    ::std::vector< ::rtl::OUString > aNewWindowListVector;

    Reference< XDesktop > xDesktop( xServiceFactory->createInstance( SERVICENAME_DESKTOP ), UNO_QUERY );

    sal_uInt16  nActiveItemId = 0;
    sal_uInt16  nItemId = START_ITEMID_WINDOWLIST;

    if ( xDesktop.is() )
    {
        Reference< XFramesSupplier > xTasksSupplier( xDesktop, UNO_QUERY );
        Reference< XFrame > xCurrentFrame = xDesktop->getCurrentFrame();
        Reference< XIndexAccess > xList( xTasksSupplier->getFrames(), UNO_QUERY );
        sal_Int32 nCount = xList->getCount();
        aNewWindowListVector.reserve(nCount);
        for (sal_Int32 i=0; i<nCount; ++i )
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

    aArgsList[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FileName" ));
    a <<= pMenuItemHandler->aMenuItemURL;
    aArgsList[0].Value = a;

    aArgsList[1].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Referer" ));
    a <<= ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SFX_REFERER_USER ));
    aArgsList[1].Value = a;

    ::rtl::OUString aFilter( pMenuItemHandler->aFilter );

    sal_Int32 nPos = aFilter.indexOf( '|' );
    if ( nPos >= 0 )
    {
        ::rtl::OUString aFilterOptions;

        if ( nPos < ( aFilter.getLength() - 1 ) )
            aFilterOptions = aFilter.copy( nPos+1 );

        aArgsList[2].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FilterOptions" ));
        a <<= aFilterOptions;
        aArgsList[2].Value = a;

        aFilter = aFilter.copy( 0, nPos-1 );
        aArgsList.realloc( ++NUM_OF_PICKLIST_ARGS );
    }

    aArgsList[NUM_OF_PICKLIST_ARGS-1].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FilterName" ));
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

        ::rtl::OUString aCommand( m_aMenuItemCommand );
        if (m_aMenuItemCommand.matchIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM(UNO_COMMAND)))
        {
            // Remove protocol part from command so we can use an easier comparision method
            aCommand = aCommand.copy(RTL_CONSTASCII_LENGTH(UNO_COMMAND));
        }

        if ( m_aMenuItemCommand == aSpecialFileMenu || m_aMenuItemCommand == aSlotSpecialFileMenu || aCommand == aSpecialFileCommand )
            UpdateSpecialFileMenu( pMenu );
        else if ( m_aMenuItemCommand == aSpecialWindowMenu || m_aMenuItemCommand == aSlotSpecialWindowMenu || aCommand == aSpecialWindowCommand )
            UpdateSpecialWindowMenu( pMenu, getServiceFactory(), m_aLock );

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

            REFERENCE< XDISPATCHPROVIDER > xDispatchProvider( m_xFrame, UNO_QUERY );
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
                            ::rtl::OUString aItemCommand = pMenu->GetItemCommand( pMenuItemHandler->nItemId );
                            if ( aItemCommand.isEmpty() )
                            {
                                const static ::rtl::OUString aSlotString( RTL_CONSTASCII_USTRINGPARAM( "slot:" ));
                                aItemCommand = aSlotString;
                                aItemCommand += ::rtl::OUString::valueOf( (sal_Int32)pMenuItemHandler->nItemId );
                                pMenu->SetItemCommand( pMenuItemHandler->nItemId, aItemCommand );
                            }

                            aTargetURL.Complete = aItemCommand;

                            m_xURLTransformer->parseStrict( aTargetURL );

                            REFERENCE< XDISPATCH > xMenuItemDispatch;
                            if ( m_bIsBookmarkMenu )
                                xMenuItemDispatch = xDispatchProvider->queryDispatch( aTargetURL, pMenuItemHandler->aTargetFrame, 0 );
                            else
                                xMenuItemDispatch = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );

                            if ( xMenuItemDispatch.is() )
                            {
                                pMenuItemHandler->xMenuItemDispatch = xMenuItemDispatch;
                                pMenuItemHandler->aMenuItemURL      = aTargetURL.Complete;
                                xMenuItemDispatch->addStatusListener( (static_cast< XSTATUSLISTENER* >(this)), aTargetURL );
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
    REFERENCE< XDISPATCH >  xDispatch;

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

                Reference< XFramesSupplier > xDesktop( getServiceFactory()->createInstance( SERVICENAME_DESKTOP ), UNO_QUERY );

                if ( xDesktop.is() )
                {
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
                        aArgs[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Referer" ));
                        aArgs[0].Value <<= ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SFX_REFERER_USER ));
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

const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& MenuManager::getServiceFactory()
{
    return mxServiceFactory;
}

void MenuManager::AddMenu(PopupMenu* _pPopupMenu,const ::rtl::OUString& _sItemCommand,sal_uInt16 _nItemId,sal_Bool _bDelete,sal_Bool _bDeleteChildren)
{
    MenuManager* pSubMenuManager = new MenuManager( getServiceFactory(), m_xFrame, _pPopupMenu, _bDelete, _bDeleteChildren );

    // store menu item command as we later have to know which menu is active (see Activate handler)
    pSubMenuManager->m_aMenuItemCommand = _sItemCommand;

    REFERENCE< XDISPATCH > aXDispatchRef;
    MenuItemHandler* pMenuItemHandler = new MenuItemHandler(
                                                _nItemId,
                                                pSubMenuManager,
                                                aXDispatchRef );
    m_aMenuItemHandlerVector.push_back( pMenuItemHandler );
}

sal_uInt16 MenuManager::FillItemCommand(::rtl::OUString& _rItemCommand,Menu* _pMenu,sal_uInt16 _nIndex) const
{
    sal_uInt16 nItemId = _pMenu->GetItemId( _nIndex );

    _rItemCommand = _pMenu->GetItemCommand( nItemId );
    if ( _rItemCommand.isEmpty() )
    {
        const static ::rtl::OUString aSlotString( RTL_CONSTASCII_USTRINGPARAM( "slot:" ));
        _rItemCommand = aSlotString;
        _rItemCommand += ::rtl::OUString::valueOf( (sal_Int32)nItemId );
        _pMenu->SetItemCommand( nItemId, _rItemCommand );
    }
    return nItemId;
}
void MenuManager::FillMenuImages(Reference< XFrame >& _xFrame,Menu* _pMenu,sal_Bool bShowMenuImages)
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
                ::rtl::OUString aImageId;

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
                    rtl::OUString aMenuItemCommand = _pMenu->GetItemCommand( nId );
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
