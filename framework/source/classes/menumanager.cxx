/*************************************************************************
 *
 *  $RCSfile: menumanager.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: cd $ $Date: 2001-09-19 08:07:46 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_CLASSES_MENUMANAGER_HXX_
#include <classes/menumanager.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_MENUCONFIGURATION_HXX_
#include <classes/menuconfiguration.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_BMKMENU_HXX
#include <classes/bmkmenu.hxx>
#endif

#ifndef __FRAMEWORK_HELPER_IMAGEPRODUCER_HXX_
#include <helper/imageproducer.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XTASKSSUPPLIER_HPP_
#include <com/sun/star/frame/XTasksSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/container/XEnumeration.hpp>
#endif
#if SUPD > 631
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHINFORMATIONPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchInformationProvider.hpp>
#endif
#endif // SUPD

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#include <comphelper/extract.hxx>

#ifndef INCLUDED_SVTOOLS_MENUOPTIONS_HXX
#include <svtools/menuoptions.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_HISTORYOPTIONS_HXX
#include <svtools/historyoptions.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif

#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vos/mutex.hxx>

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

using namespace ::cppu;
using namespace ::vos;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;


namespace framework
{

// special menu ids/command ids for dynamic popup menus
#define SID_SFX_START           5000
#define SID_NEWDOCDIRECT        (SID_SFX_START + 537)
#define SID_AUTOPILOTMENU       (SID_SFX_START + 1381)
#define SID_PICKLIST            (SID_SFX_START + 510)
#define SID_MDIWINDOWLIST       (SID_SFX_START + 610)

#define SFX_REFERER_USER        "private:user"
#define DESKTOP_SERVICE         ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" ))

const ::rtl::OUString aSlotString( RTL_CONSTASCII_USTRINGPARAM( "slot:" ));
const ::rtl::OUString aSlotNewDocDirect( RTL_CONSTASCII_USTRINGPARAM( "slot:5537" ));
const ::rtl::OUString aSlotAutoPilot( RTL_CONSTASCII_USTRINGPARAM( "slot:6381" ));

const ::rtl::OUString aSpecialFileMenu( RTL_CONSTASCII_USTRINGPARAM( "file" ));
const ::rtl::OUString aSpecialWindowMenu( RTL_CONSTASCII_USTRINGPARAM( "window" ));
const ::rtl::OUString aSlotSpecialFileMenu( RTL_CONSTASCII_USTRINGPARAM( "slot:5510" ));
const ::rtl::OUString aSlotSpecialWindowMenu( RTL_CONSTASCII_USTRINGPARAM( "slot:5610" ));

MenuManager::MenuManager( REFERENCE< XFRAME >& rFrame, Menu* pMenu, sal_Bool bDelete, sal_Bool bDeleteChildren ) :
    ThreadHelpBase( &Application::GetSolarMutex() ), OWeakObject()
{
    m_bActive           = sal_False;
    m_bDeleteMenu       = bDelete;
    m_bDeleteChildren   = bDeleteChildren;
    m_pVCLMenu          = pMenu;
    m_xFrame            = rFrame;
    m_bInitialized      = sal_False;
    m_bIsBookmarkMenu   = sal_False;
    SAL_STATIC_CAST( ::com::sun::star::uno::XInterface*, (OWeakObject*)this )->acquire();

    SvtMenuOptions aOptions;
    ::std::vector< USHORT > aQueryLabelItemIdVector;

    int nItemCount = pMenu->GetItemCount();
    for ( int i = 0; i < nItemCount; i++ )
    {
        USHORT nItemId = pMenu->GetItemId( i );

        ::rtl::OUString aItemCommand = pMenu->GetItemCommand( nItemId );
        if ( !aItemCommand.getLength() )
        {
            aItemCommand = aSlotString;
            aItemCommand += ::rtl::OUString::valueOf( (sal_Int32)nItemId );
            pMenu->SetItemCommand( nItemId, aItemCommand );
        }

        PopupMenu* pPopupMenu = pMenu->GetPopupMenu( nItemId );
        if ( pPopupMenu )
        {
            MenuManager* pSubMenuManager = new MenuManager( rFrame, pPopupMenu, bDeleteChildren, bDeleteChildren );

            // store menu item command as we later have to know which menu is active (see Activate handler)
            pSubMenuManager->m_aMenuItemCommand = aItemCommand;

            MenuItemHandler* pMenuItemHandler = new MenuItemHandler(
                                                        nItemId,
                                                        pSubMenuManager,
                                                        REFERENCE< XDISPATCH >() );
            m_aMenuItemHandlerVector.push_back( pMenuItemHandler );
            if ( pMenu->GetItemText( nItemId ).Len() == 0 )
                aQueryLabelItemIdVector.push_back( nItemId );
        }
        else
        {
            if ( nItemId == SID_NEWDOCDIRECT ||
                 aItemCommand == aSlotNewDocDirect )
            {
                MenuConfiguration aMenuCfg( ::comphelper::getProcessServiceFactory() );
                BmkMenu* pSubMenu = (BmkMenu*)aMenuCfg.CreateBookmarkMenu( rFrame, BOOKMARK_NEWMENU );
                pMenu->SetPopupMenu( nItemId, pSubMenu );
                MenuManager* pSubMenuManager = new MenuManager( rFrame, pSubMenu, sal_True, sal_False );
                MenuItemHandler* pMenuItemHandler = new MenuItemHandler(
                                                            nItemId,
                                                            pSubMenuManager,
                                                            REFERENCE< XDISPATCH >() );
                if ( pMenu->GetItemText( nItemId ).Len() == 0 )
                    aQueryLabelItemIdVector.push_back( nItemId );
                m_aMenuItemHandlerVector.push_back( pMenuItemHandler );
            }
            else if ( nItemId == SID_AUTOPILOTMENU ||
                      aItemCommand == aSlotAutoPilot )
            {
                MenuConfiguration aMenuCfg( ::comphelper::getProcessServiceFactory() );
                BmkMenu* pSubMenu = (BmkMenu*)aMenuCfg.CreateBookmarkMenu( rFrame, BOOKMARK_WIZARDMENU );
                pMenu->SetPopupMenu( nItemId, pSubMenu );
                MenuManager* pSubMenuManager = new MenuManager( rFrame, pSubMenu, sal_True, sal_False );
                MenuItemHandler* pMenuItemHandler = new MenuItemHandler(
                                                            nItemId,
                                                            pSubMenuManager,
                                                            REFERENCE< XDISPATCH >() );
                if ( pMenu->GetItemText( nItemId ).Len() == 0 )
                    aQueryLabelItemIdVector.push_back( nItemId );
                m_aMenuItemHandlerVector.push_back( pMenuItemHandler );
            }
            else if ( pMenu->GetItemType( i ) != MENUITEM_SEPARATOR )
            {
                if ( aOptions.IsMenuIconsEnabled() )
                {
                    Image aImage = GetImageFromURL( rFrame, aItemCommand, FALSE );
                    if ( !!aImage )
                        pMenu->SetItemImage( nItemId, aImage );
                }

                m_aMenuItemHandlerVector.push_back( new MenuItemHandler( nItemId, NULL, REFERENCE< XDISPATCH >() ));
                if ( pMenu->GetItemText( nItemId ).Len() == 0 )
                    aQueryLabelItemIdVector.push_back( nItemId );
            }
        }
    }


    // retrieve label information for all menu items without item text
#if 0
    if ( aQueryLabelItemIdVector.size() > 0 )
    {
        Sequence< ::rtl::OUString > aURLSequence( aQueryLabelItemIdVector.size() );
        Sequence< ::rtl::OUString > aLabelSequence( aQueryLabelItemIdVector.size() );

        sal_uInt32 nPos = 0;
        ::std::vector< USHORT >::iterator p;
        for ( p = aQueryLabelItemIdVector.begin(); p != aQueryLabelItemIdVector.end(); p++ )
            aURLSequence[nPos++] = pMenu->GetItemCommand( *p );

        Reference< XDispatchInformationProvider > xDIP( xFrame, UNO_QUERY );
        if ( xDIP.is() )
        {
            nPos = 0;
            xDIP->queryDispatchInformations( aURLSequence, aLabelSequence );
            for ( p = aQueryLabelItemIdVector.begin(); p != aQueryLabelItemIdVector.end(); p++ )
                pMenu->SetItemText( *p, aLabelSequence( nPos++ ));
        }
    }
#endif
    m_pVCLMenu->SetHighlightHdl( LINK( this, MenuManager, Highlight ));
    m_pVCLMenu->SetActivateHdl( LINK( this, MenuManager, Activate ));
    m_pVCLMenu->SetDeactivateHdl( LINK( this, MenuManager, Deactivate ));
    m_pVCLMenu->SetSelectHdl( LINK( this, MenuManager, Select ));
}


MenuManager::MenuManager( REFERENCE< XFRAME >& rFrame, BmkMenu* pBmkMenu, sal_Bool bDelete, sal_Bool bDeleteChildren ) :
    ThreadHelpBase( &Application::GetSolarMutex() ), OWeakObject()
{
    m_bActive           = sal_False;
    m_bDeleteMenu       = bDelete;
    m_bDeleteChildren   = bDeleteChildren;
    m_pVCLMenu          = pBmkMenu;
    m_xFrame            = rFrame;
    m_bInitialized      = sal_False;
    m_bIsBookmarkMenu   = sal_True;
    SAL_STATIC_CAST( ::com::sun::star::uno::XInterface*, (OWeakObject*)this )->acquire();

    int nItemCount = pBmkMenu->GetItemCount();
    for ( int i = 0; i < nItemCount; i++ )
    {
        USHORT nItemId = pBmkMenu->GetItemId( i );

        ::rtl::OUString aItemCommand = pBmkMenu->GetItemCommand( nItemId );
        if ( !aItemCommand.getLength() )
        {
            aItemCommand = aSlotString;
            aItemCommand += ::rtl::OUString::valueOf( (sal_Int32)nItemId );
            pBmkMenu->SetItemCommand( nItemId, aItemCommand );
        }

        PopupMenu* pPopupMenu = pBmkMenu->GetPopupMenu( nItemId );
        if ( pPopupMenu )
        {
            MenuManager* pSubMenuManager = new MenuManager( rFrame, pPopupMenu, bDeleteChildren, bDeleteChildren );

            // store menu item command as we later have to know which menu is active (see Acivate handler)
            pSubMenuManager->m_aMenuItemCommand = aItemCommand;

            MenuItemHandler* pMenuItemHandler = new MenuItemHandler(
                                                        nItemId,
                                                        pSubMenuManager,
                                                        REFERENCE< XDISPATCH >() );
            m_aMenuItemHandlerVector.push_back( pMenuItemHandler );
        }
        else
        {
            if ( pBmkMenu->GetItemType( i ) != MENUITEM_SEPARATOR )
            {
                MenuConfiguration::Attributes* pBmkAttributes = (MenuConfiguration::Attributes *)(pBmkMenu->GetUserValue( nItemId ));
                MenuItemHandler* pMenuItemHandler = new MenuItemHandler( nItemId, NULL, REFERENCE< XDISPATCH >() );

                if ( pBmkAttributes )
                {
                    // read additional attributes from attributes struct and delete it afterwards
                    pMenuItemHandler->aTargetFrame = pBmkAttributes->aTargetFrame;
                    delete pBmkAttributes;
                    pBmkMenu->SetUserValue( nItemId, 0 );
                }

                m_aMenuItemHandlerVector.push_back( pMenuItemHandler );
            }
        }
    }

    m_pVCLMenu->SetHighlightHdl( LINK( this, MenuManager, Highlight ));
    m_pVCLMenu->SetActivateHdl( LINK( this, MenuManager, Activate ));
    m_pVCLMenu->SetDeactivateHdl( LINK( this, MenuManager, Deactivate ));
    m_pVCLMenu->SetSelectHdl( LINK( this, MenuManager, Select ));
}


Any SAL_CALL MenuManager::queryInterface( const ::com::sun::star::uno::Type & rType ) throw ( RuntimeException )
{
    Any a = ::cppu::queryInterface(
                rType ,
                SAL_STATIC_CAST( XSTATUSLISTENER*, this ),
                SAL_STATIC_CAST( XEVENTLISTENER*, this ));
    if ( a.hasValue() )
        return a;

    return OWeakObject::queryInterface( rType );
}


MenuManager::~MenuManager()
{
    std::vector< MenuItemHandler* >::iterator p;
    for ( p = m_aMenuItemHandlerVector.begin(); p != m_aMenuItemHandlerVector.end(); p++ )
    {
        MenuItemHandler* pItemHandler = *p;
        pItemHandler->xMenuItemDispatch = REFERENCE< XDISPATCH >();
        if ( pItemHandler->pSubMenuManager )
            SAL_STATIC_CAST( ::com::sun::star::uno::XInterface*, (OWeakObject*)pItemHandler->pSubMenuManager )->release();
        delete pItemHandler;
    }

    if ( m_bDeleteMenu )
        delete m_pVCLMenu;
}


MenuManager::MenuItemHandler* MenuManager::GetMenuItemHandler( USHORT nItemId )
{
    ResetableGuard aGuard( m_aLock );

    std::vector< MenuItemHandler* >::iterator p;
    for ( p = m_aMenuItemHandlerVector.begin(); p != m_aMenuItemHandlerVector.end(); p++ )
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
        for ( p = m_aMenuItemHandlerVector.begin(); p != m_aMenuItemHandlerVector.end(); p++ )
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
        OGuard  aGuard( Application::GetSolarMutex() );
        {
            ResetableGuard aGuard( m_aLock );

            sal_Bool bCheckmark         = sal_False;
            sal_Bool bMenuItemEnabled   = m_pVCLMenu->IsItemEnabled( pStatusChangedMenu->nItemId );

            if ( Event.IsEnabled != bMenuItemEnabled )
            m_pVCLMenu->EnableItem( pStatusChangedMenu->nItemId, Event.IsEnabled );

            try
            {
                bCheckmark = ::cppu::any2bool(Event.State);
            }
            catch(const ::com::sun::star::lang::IllegalArgumentException&)
            {
            }

            m_pVCLMenu->CheckItem( pStatusChangedMenu->nItemId, bCheckmark );
        }

        if ( Event.Requery )
        {
            URL aTargetURL;
            aTargetURL.Complete = pStatusChangedMenu->aMenuItemURL;

            REFERENCE< XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))), UNO_QUERY );
            xTrans->parseStrict( aTargetURL );

            REFERENCE< XDISPATCHPROVIDER > xDispatchProvider( m_xFrame, UNO_QUERY );
            REFERENCE< XDISPATCH > xMenuItemDispatch = xDispatchProvider->queryDispatch(
                                                            aTargetURL, ::rtl::OUString(), 0 );

            if ( xMenuItemDispatch.is() )
            {
                pStatusChangedMenu->xMenuItemDispatch   = xMenuItemDispatch;
                pStatusChangedMenu->aMenuItemURL        = aTargetURL.Complete;
                xMenuItemDispatch->addStatusListener( SAL_STATIC_CAST( XSTATUSLISTENER*, this ), aTargetURL );
            }
        }
    }
}


void SAL_CALL MenuManager::disposing( const EVENTOBJECT& Source ) throw ( RUNTIMEEXCEPTION )
{
    if ( Source.Source == m_xFrame )
    {
        ResetableGuard aGuard( m_aLock );

        // disposing called from parent dispatcher
        // remove all listener to prepare shutdown
        REFERENCE< XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))), UNO_QUERY );

        std::vector< MenuItemHandler* >::iterator p;
        for ( p = m_aMenuItemHandlerVector.begin(); p != m_aMenuItemHandlerVector.end(); p++ )
        {
            MenuItemHandler* pItemHandler = *p;
            if ( pItemHandler->xMenuItemDispatch.is() )
            {
                URL aTargetURL;
                aTargetURL.Complete = pItemHandler->aMenuItemURL;
                xTrans->parseStrict( aTargetURL );

                pItemHandler->xMenuItemDispatch->removeStatusListener(
                    SAL_STATIC_CAST( XSTATUSLISTENER*, this ), aTargetURL );
            }

            pItemHandler->xMenuItemDispatch = REFERENCE< XDISPATCH >();
            if ( pItemHandler->pSubMenuManager )
                pItemHandler->pSubMenuManager->disposing( Source );
        }
    }
    else
    {
        // disposing called from menu item dispatcher, remove listener
        MenuItemHandler* pMenuItemDisposing = NULL;

        {
            ResetableGuard aGuard( m_aLock );

            std::vector< MenuItemHandler* >::iterator p;
            for ( p = m_aMenuItemHandlerVector.begin(); p != m_aMenuItemHandlerVector.end(); p++ )
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

                REFERENCE< XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))), UNO_QUERY );
                xTrans->parseStrict( aTargetURL );

                pMenuItemDisposing->xMenuItemDispatch->removeStatusListener(
                    SAL_STATIC_CAST( XSTATUSLISTENER*, this ), aTargetURL );
                pMenuItemDisposing->xMenuItemDispatch = REFERENCE< XDISPATCH >();
            }
        }
    }
}


void MenuManager::UpdateSpecialFileMenu( Menu* pMenu )
{
    // update picklist
    Sequence< Sequence< PropertyValue > > aHistoryList = SvtHistoryOptions().GetList( ePICKLIST );
    ::std::vector< MenuItemHandler* > aNewPickVector;

    USHORT  nPickItemId = START_ITEMID_PICKLIST;
    int     nPickListMenuItems = ( aHistoryList.getLength() > 9 ) ? 9 : aHistoryList.getLength();

    for ( int i = 0; i < nPickListMenuItems; i++ )
    {
        Sequence< PropertyValue > aPickListEntry = aHistoryList[i];

        MenuItemHandler* pNewMenuItemHandler = new MenuItemHandler(
                                                    nPickItemId++,
                                                    NULL,
                                                    REFERENCE< XDISPATCH >() );

        for ( int j = 0; j < aPickListEntry.getLength(); j++ )
        {
            Any a = aPickListEntry[j].Value;

            if ( aPickListEntry[j].Name == HISTORY_PROPERTYNAME_URL )
                a >>= pNewMenuItemHandler->aMenuItemURL;
            else if ( aPickListEntry[j].Name == HISTORY_PROPERTYNAME_FILTER )
                a >>= pNewMenuItemHandler->aFilter;
            else if ( aPickListEntry[j].Name == HISTORY_PROPERTYNAME_TITLE )
            {
                char menuShortCut[5] = "~n: ";

                if ( i == 10 )
                    menuShortCut[1] = '0';
                else
                    menuShortCut[1] = ( '1' + i );

                ::rtl::OUString aMenuShortCut( RTL_CONSTASCII_USTRINGPARAM( menuShortCut ));
                ::rtl::OUString aTitle;

                a >>= aTitle;
                pNewMenuItemHandler->aTitle = aMenuShortCut + aTitle;
            }
            else if ( aPickListEntry[j].Name == HISTORY_PROPERTYNAME_PASSWORD )
                a >>= pNewMenuItemHandler->aPassword;
        }

        aNewPickVector.push_back( pNewMenuItemHandler );
    }

    if ( aNewPickVector.size() > 0 )
    {
        URL aTargetURL;
        REFERENCE< XDISPATCHPROVIDER > xDispatchProvider( m_xFrame, UNO_QUERY );
        REFERENCE< XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))), UNO_QUERY );
        REFERENCE< XDISPATCH > xMenuItemDispatch;

        // query for dispatcher
        std::vector< MenuItemHandler* >::iterator p;
        for ( p = aNewPickVector.begin(); p != aNewPickVector.end(); p++ )
        {
            MenuItemHandler* pMenuItemHandler = *p;

            aTargetURL.Complete = pMenuItemHandler->aMenuItemURL;
            xTrans->parseStrict( aTargetURL );

            if ( !xMenuItemDispatch.is() )
            {
                // attention: this code assume that "_blank" can only be consumed by desktop service
                xMenuItemDispatch = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString::createFromAscii("_blank"), 0 );
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
            pMenu->InsertSeparator();
            for ( sal_uInt32 i = 0; i < aNewPickVector.size(); i++ )
            {
                MenuItemHandler* pMenuItemHandler = aNewPickVector.at( i );
                pMenu->InsertItem(
                    pMenuItemHandler->nItemId,
                    pMenuItemHandler->aTitle );
                m_aMenuItemHandlerVector.push_back( pMenuItemHandler );
            }
        }
    }
}


void MenuManager::UpdateSpecialWindowMenu( Menu* pMenu )
{
    // update window list
    ::std::vector< ::rtl::OUString > aNewWindowListVector;
    Reference< XDesktop > xDesktop( ::comphelper::getProcessServiceFactory()->createInstance(
                                    DESKTOP_SERVICE ), UNO_QUERY );

    USHORT  nActiveItemId = 0;
    USHORT  nItemId = START_ITEMID_WINDOWLIST;

    if ( xDesktop.is() )
    {
        Reference< XTasksSupplier > xTasksSupplier( xDesktop, UNO_QUERY );
        Reference< XFrame > xCurrentFrame = xDesktop->getCurrentFrame();
        Reference< XEnumeration > xList = xTasksSupplier->getTasks()->createEnumeration();
        while (( xList->hasMoreElements() == sal_True ))
        {
            Reference< XTask > xTask;
            xList->nextElement() >>= xTask;
            if ( xTask.is() )
            {
                Reference< XFrame > xFrame( xTask, UNO_QUERY );
                if ( xFrame == xCurrentFrame )
                    nActiveItemId = nItemId;

                Window* pWin = VCLUnoHelper::GetWindow( xTask->getContainerWindow() );
                if ( pWin && pWin->IsVisible() )
                {
                    aNewWindowListVector.push_back( pWin->GetText() );
                    ++nItemId;
                }
            }
        }
    }

    {
        ResetableGuard aGuard( m_aLock );

        int nRemoveItemCount = 0;
        int nItemCount       = pMenu->GetItemCount();

        if ( nItemCount > 0 )
        {
            // remove all old window list entries from menu
            sal_uInt16 nPos = pMenu->GetItemPos( START_ITEMID_WINDOWLIST );
            for ( sal_uInt16 n = nPos; n < pMenu->GetItemCount(); )
                pMenu->RemoveItem( n );

            if ( pMenu->GetItemType( pMenu->GetItemCount()-1 ) == MENUITEM_SEPARATOR )
                pMenu->RemoveItem( pMenu->GetItemCount()-1 );
        }

        if ( aNewWindowListVector.size() > 0 )
        {
            // append new window list entries to menu
            pMenu->InsertSeparator();
            nItemId = START_ITEMID_WINDOWLIST;
            for ( sal_uInt32 i = 0; i < aNewWindowListVector.size(); i++ )
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
    const int NUM_OF_PICKLIST_ARGS = 4;

    Any a;
    aArgsList.realloc( NUM_OF_PICKLIST_ARGS );

    aArgsList[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FileName" ));
    a <<= pMenuItemHandler->aMenuItemURL;
    aArgsList[0].Value = a;

    aArgsList[1].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Referer" ));
    a <<= ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SFX_REFERER_USER ));
    aArgsList[1].Value = a;

    ::rtl::OUString aFilter( pMenuItemHandler->aFilter );
    ::rtl::OUString aFilterOptions;

    sal_Int32 nPos = aFilter.indexOf( '|' );
    if( nPos >= 0 )
    {
        if ( nPos < ( aFilter.getLength() - 1 ))
            aFilterOptions = aFilter.copy( nPos+1 );
        aFilter = aFilter.copy( 0, nPos-1 );
    }

    aArgsList[2].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FilterOptions" ));
    a <<= aFilterOptions;
    aArgsList[2].Value = a;

    aArgsList[3].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FilterName" ));
    a <<= aFilter;
    aArgsList[3].Value = a;
}


//_________________________________________________________________________________________________________________
// vcl handler
//_________________________________________________________________________________________________________________

IMPL_LINK( MenuManager, Activate, Menu *, pMenu )
{
    if ( pMenu == m_pVCLMenu )
    {
        // set/unset hiding disabled menu entries
        sal_Bool bDontHide = SvtMenuOptions().IsEntryHidingEnabled();

        sal_uInt16 nFlag = pMenu->GetMenuFlags();
        if ( bDontHide )
            nFlag &= ~MENU_FLAG_HIDEDISABLEDENTRIES;
        else
            nFlag |= MENU_FLAG_HIDEDISABLEDENTRIES;
        pMenu->SetMenuFlags( nFlag );

        if ( m_bActive )
            return 0;

        m_bActive = TRUE;

        if ( m_aMenuItemCommand == aSpecialFileMenu ||
             m_aMenuItemCommand == aSlotSpecialFileMenu )
            UpdateSpecialFileMenu( pMenu );
        else if ( m_aMenuItemCommand == aSpecialWindowMenu ||
                  m_aMenuItemCommand == aSlotSpecialWindowMenu )
            UpdateSpecialWindowMenu( pMenu );

        if ( m_bInitialized )
            return 0;
        else
        {
            URL aTargetURL;
            REFERENCE< XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))), UNO_QUERY );

            ResetableGuard aGuard( m_aLock );

            REFERENCE< XDISPATCHPROVIDER > xDispatchProvider( m_xFrame, UNO_QUERY );
            if ( xDispatchProvider.is() )
            {
                std::vector< MenuItemHandler* >::iterator p;
                for ( p = m_aMenuItemHandlerVector.begin(); p != m_aMenuItemHandlerVector.end(); p++ )
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
                            if ( !aItemCommand.getLength() )
                            {
                                aItemCommand = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "slot:" ));
                                aItemCommand += ::rtl::OUString::valueOf( (sal_Int32)pMenuItemHandler->nItemId );
                                pMenu->SetItemCommand( pMenuItemHandler->nItemId, aItemCommand );
                            }

                            aTargetURL.Complete = aItemCommand;

                            xTrans->parseStrict( aTargetURL );

                            REFERENCE< XDISPATCH > xMenuItemDispatch;
                            if ( m_bIsBookmarkMenu )
                                xMenuItemDispatch = xDispatchProvider->queryDispatch( aTargetURL, pMenuItemHandler->aTargetFrame, 0 );
                            else
                                xMenuItemDispatch = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );

                            if ( xMenuItemDispatch.is() )
                            {
                                pMenuItemHandler->xMenuItemDispatch = xMenuItemDispatch;
                                pMenuItemHandler->aMenuItemURL      = aTargetURL.Complete;
                                xMenuItemDispatch->addStatusListener( SAL_STATIC_CAST( XSTATUSLISTENER*, this ), aTargetURL );
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

        USHORT nCurItemId = pMenu->GetCurItemId();
        if ( pMenu == m_pVCLMenu &&
             pMenu->GetItemType( nCurItemId ) != MENUITEM_SEPARATOR )
        {
            if ( nCurItemId >= START_ITEMID_WINDOWLIST &&
                 nCurItemId <= END_ITEMID_WINDOWLIST )
            {
                // window list menu item selected
                Reference< XTasksSupplier > xDesktop( ::comphelper::getProcessServiceFactory()->createInstance(
                                                DESKTOP_SERVICE ), UNO_QUERY );
                USHORT  nWindowItemId = START_ITEMID_WINDOWLIST;

                if ( xDesktop.is() )
                {
                    USHORT nTaskId = START_ITEMID_WINDOWLIST;
                    Reference< XEnumeration > xList = xDesktop->getTasks()->createEnumeration();
                    while (( xList->hasMoreElements() == sal_True ))
                    {
                        Reference< XTask > xTask;
                        xList->nextElement() >>= xTask;

                        if ( xTask.is() && nTaskId == nCurItemId )
                        {
                            Window* pWin = VCLUnoHelper::GetWindow( xTask->getContainerWindow() );
                            pWin->GrabFocus();
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
                    REFERENCE< XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))), UNO_QUERY );
                    aTargetURL.Complete = pMenuItemHandler->aMenuItemURL;
                    xTrans->parseStrict( aTargetURL );

                    if ( nCurItemId >= START_ITEMID_PICKLIST &&
                         nCurItemId <  START_ITEMID_WINDOWLIST )
                    {
                        // picklist menu item selected
                        CreatePicklistArguments( aArgs, pMenuItemHandler );
                    }
                    else if ( m_bIsBookmarkMenu )
                    {
                        // bookmark menu item selected
                        Any a;
                        aArgs.realloc( 1 );
                        aArgs[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Referer" ));
                        a <<= ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SFX_REFERER_USER ));
                        aArgs[0].Value = a;
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


IMPL_LINK( MenuManager, Highlight, Menu *, pMenu )
{
    return 0;
}

}
