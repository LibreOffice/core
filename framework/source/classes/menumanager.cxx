/*************************************************************************
 *
 *  $RCSfile: menumanager.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: cd $ $Date: 2001-05-02 05:38:28 $
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

#include <classes/menumanager.hxx>
#include <classes/bmkmenu.hxx>

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

#define DESKTOP_SERVICE         ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" ))

const USHORT START_ITEMID_PICKLIST      = 4500;
const USHORT START_ITEMID_WINDOWLIST    = 4600;
const USHORT END_ITEMID_WINDOWLIST      = 4699;

const ::rtl::OUString aSlotString( RTL_CONSTASCII_USTRINGPARAM( "slot:" ));
const ::rtl::OUString aSlotNewDocDirect( RTL_CONSTASCII_USTRINGPARAM( "slot:5537" ));
const ::rtl::OUString aSlotAutoPilot( RTL_CONSTASCII_USTRINGPARAM( "slot:6381" ));

const ::rtl::OUString aSpecialFileMenu( RTL_CONSTASCII_USTRINGPARAM( "file" ));
const ::rtl::OUString aSpecialWindowMenu( RTL_CONSTASCII_USTRINGPARAM( "window" ));


MenuManager::MenuManager( REFERENCE< XFRAME >& rFrame, Menu* pMenu, sal_Bool bDelete, sal_Bool bDeleteChildren, sal_Bool bIsBookmarkMenu ) :
    OMutexMember(), OWeakObject()
{
    m_bActive           = sal_False;
    m_bDeleteMenu       = bDelete;
    m_bDeleteChildren   = bDeleteChildren;
    m_pVCLMenu          = pMenu;
    m_xFrame            = rFrame;
    m_bInitialized      = sal_False;
    m_bIsBookmarkMenu   = bIsBookmarkMenu;
    SAL_STATIC_CAST( ::com::sun::star::uno::XInterface*, (OWeakObject*)this )->acquire();

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
            if ( nItemId == SID_NEWDOCDIRECT ||
                 aItemCommand == aSlotNewDocDirect )
            {
                String sKey = SvtPathOptions().GetNewMenuPath();
                ::utl::LocalFileHelper::ConvertPhysicalNameToURL( sKey, sKey );
                PopupMenu* pSubMenu = CreateBookmarkMenu( sKey, sKey );
                pMenu->SetPopupMenu( nItemId, pSubMenu );
                MenuManager* pSubMenuManager = new MenuManager( rFrame, pSubMenu, sal_True, sal_False, sal_True );
                MenuItemHandler* pMenuItemHandler = new MenuItemHandler(
                                                            nItemId,
                                                            pSubMenuManager,
                                                            REFERENCE< XDISPATCH >() );
            }
            else if ( nItemId == SID_AUTOPILOTMENU ||
                      aItemCommand == aSlotAutoPilot )
            {
                String sKey = SvtPathOptions().GetAutoPilotPath();
                ::utl::LocalFileHelper::ConvertPhysicalNameToURL( sKey, sKey );
                PopupMenu* pSubMenu = CreateBookmarkMenu( sKey, sKey );
                pMenu->SetPopupMenu( nItemId, pSubMenu );
                MenuManager* pSubMenuManager = new MenuManager( rFrame, pSubMenu, sal_True, sal_False, sal_True );
                MenuItemHandler* pMenuItemHandler = new MenuItemHandler(
                                                            nItemId,
                                                            pSubMenuManager,
                                                            REFERENCE< XDISPATCH >() );
            }
            else if ( pMenu->GetItemType( i ) != MENUITEM_SEPARATOR )
                m_aMenuItemHandlerVector.push_back( new MenuItemHandler( nItemId, NULL, REFERENCE< XDISPATCH >() ));
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
    LOCK_MUTEX( aGuard, m_aMutex, "MenuManager::GetMenuItemHandler" )

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
        LOCK_MUTEX( aGuard, m_aMutex, "MenuManager::statusChanged" )

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
            LOCK_MUTEX( aGuard, m_aMutex, "MenuManager::statusChanged" )

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

            if ( bCheckmark )
                m_pVCLMenu->CheckItem( pStatusChangedMenu->nItemId, sal_True );
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
    MenuItemHandler* pMenuItemDisposing = NULL;

    {
        LOCK_MUTEX( aGuard, m_aMutex, "MenuManager::disposing" )

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


PopupMenu* MenuManager::CreateBookmarkMenu( const ::rtl::OUString aURL, const ::rtl::OUString aReferer )
{
    return new BmkMenu( aURL, aReferer );
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
            LOCK_MUTEX( aGuard, m_aMutex, "MenuManager::UpdateSpecialFileMenu" )

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
                aNewWindowListVector.push_back( pWin->GetText() );
                ++nItemId;
            }
        }
    }

    {
        LOCK_MUTEX( aGuard, m_aMutex, "MenuManager::UpdateSpecialWindowMenu" )

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

        if ( m_aMenuItemCommand == aSpecialFileMenu )
            UpdateSpecialFileMenu( pMenu );
        else if ( m_aMenuItemCommand == aSpecialWindowMenu )
            UpdateSpecialWindowMenu( pMenu );

        if ( m_bInitialized )
            return 0;
        else
        {
            URL aTargetURL;
            REFERENCE< XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))), UNO_QUERY );

            LOCK_MUTEX( aGuard, m_aMutex, "MenuManager::Activate" )

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
                            {
                                if ( aTargetURL.Protocol.compareToAscii("slot:") == COMPARE_EQUAL )
                                    xMenuItemDispatch = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );
                                else
                                    xMenuItemDispatch = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString::createFromAscii("_blank"), 0 );
                            }
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
    LOCK_MUTEX( aGuard, m_aMutex, "MenuManager::Select" )

    if ( pMenu == m_pVCLMenu )
    {
        USHORT nCurItemId = pMenu->GetCurItemId();
        if ( pMenu->GetItemType( nCurItemId ) != MENUITEM_SEPARATOR )
        {
            if ( m_aMenuItemCommand == aSpecialWindowMenu )
            {
                UNLOCK_MUTEX( aGuard, m_aMutex )

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
                            UNLOCK_MUTEX( aGuard, m_aMutex )
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
                    URL aTargetURL;
                    REFERENCE< XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance(
                        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))), UNO_QUERY );
                    aTargetURL.Complete = pMenuItemHandler->aMenuItemURL;
                    xTrans->parseStrict( aTargetURL );

                    if ( m_aMenuItemCommand == aSpecialFileMenu )
                    {
                        Sequence<PropertyValue> aArgs;
                        pMenuItemHandler->xMenuItemDispatch->dispatch( aTargetURL, aArgs );
                    }
                    else
                    {
                        Sequence<PropertyValue> aArgs;
                        pMenuItemHandler->xMenuItemDispatch->dispatch( aTargetURL, aArgs );
                    }
                }
            }
        }
    }

    return 1;
}


IMPL_LINK( MenuManager, Highlight, Menu *, pMenu )
{
    return 0;
}

}

