/*************************************************************************
 *
 *  $RCSfile: menumanager.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: cd $ $Date: 2001-04-02 14:07:32 $
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

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_MENUOPTIONS_HXX
#include <svtools/menuoptions.hxx>
#endif

#include <vcl/svapp.hxx>

#include <vos/mutex.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

using namespace ::cppu;
using namespace ::vos;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;

namespace framework
{

MenuManager::MenuManager( REFERENCE< XFRAME >& rFrame, Menu* pMenu, sal_Bool bDelete, sal_Bool bDeleteChildren ) :
    OMutexMember(), OWeakObject()
{
    m_bActive           = sal_False;
    m_bDeleteMenu       = bDelete;
    m_bDeleteChildren   = bDeleteChildren;
    m_pVCLMenu          = pMenu;
    m_xFrame            = rFrame;
    m_bInitialized      = sal_False;

    SAL_STATIC_CAST( ::com::sun::star::uno::XInterface*, (OWeakObject*)this )->acquire();

    int nItemCount = pMenu->GetItemCount();
    for ( int i = 0; i < nItemCount; i++ )
    {
        USHORT nItemId = pMenu->GetItemId( i );

        PopupMenu* pPopupMenu = pMenu->GetPopupMenu( nItemId );
        if ( pPopupMenu )
        {
            MenuManager* pSubMenuManager = new MenuManager( rFrame, pPopupMenu, bDeleteChildren, sal_False );
            MenuItemHandler* pMenuItemHandler = new MenuItemHandler(
                                                        nItemId,
                                                        pSubMenuManager,
                                                        REFERENCE< XDISPATCH >() );
            m_aMenuItemHandlerVector.push_back( pMenuItemHandler );
        }
        else
        {
            if ( pMenu->GetItemType( i ) != MENUITEM_SEPARATOR )
                m_aMenuItemHandlerVector.push_back( new MenuItemHandler( nItemId, NULL, REFERENCE< XDISPATCH >() ));
        }
    }

    m_pVCLMenu->SetHighlightHdl( LINK( this, MenuManager, Highlight ));
    m_pVCLMenu->SetActivateHdl( LINK( this, MenuManager, Activate ));
    m_pVCLMenu->SetDeactivateHdl( LINK( this, MenuManager, Deactivate ));
    m_pVCLMenu->SetSelectHdl( LINK( this, MenuManager, Select ));
}


Any SAL_CALL MenuManager::queryInterface( const ::com::sun::star::uno::Type & rType )
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
    LOCK_MUTEX( aGuard, m_aMutex, "MenuManager::~MenuManager" )

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
        sal_Bool bMenuItemEnabled = m_pVCLMenu->IsItemEnabled( pStatusChangedMenu->nItemId );

        if ( Event.IsEnabled != bMenuItemEnabled )
        {
            OGuard  aGuard( Application::GetSolarMutex() );
            m_pVCLMenu->EnableItem( pStatusChangedMenu->nItemId, Event.IsEnabled );
        }


        sal_Bool bCheckmark = sal_False;

        Event.State >>= bCheckmark;
        if ( bCheckmark )
        {
            OGuard  aGuard( Application::GetSolarMutex() );
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

        if ( m_bActive || m_bInitialized )
            return 0;

        m_bActive = TRUE;

        LOCK_MUTEX( aGuard, m_aMutex, "MenuManager::Activate" )
        std::vector< MenuItemHandler* >::iterator p;
        for ( p = m_aMenuItemHandlerVector.begin(); p != m_aMenuItemHandlerVector.end(); p++ )
        {
            MenuItemHandler* pMenuItemHandler = *p;
            if ( pMenuItemHandler &&
                 pMenuItemHandler->pSubMenuManager == 0 &&
                 !pMenuItemHandler->xMenuItemDispatch.is() )
            {
                ::rtl::OUString aItemCommand = pMenu->GetItemCommand( pMenuItemHandler->nItemId );
                if ( !aItemCommand.getLength() )
                {
//                  aItemCommand = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "commandId:" ));
                    aItemCommand = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "slot:" ));
                    aItemCommand += ::rtl::OUString::valueOf( (sal_Int32)pMenuItemHandler->nItemId );
                }

                REFERENCE< XDISPATCHPROVIDER > xDispatchProvider( m_xFrame, UNO_QUERY );
                if ( xDispatchProvider.is() )
                {
                    URL aTargetURL;
                    aTargetURL.Complete = aItemCommand;

                    REFERENCE< XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance(
                            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))), UNO_QUERY );
                    xTrans->parseStrict( aTargetURL );

                    REFERENCE< XDISPATCH > xMenuItemDispatch = xDispatchProvider->queryDispatch(
                                                                    aTargetURL, ::rtl::OUString(), 0 );

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

        m_bInitialized = sal_True;
    }

    return 0;
}


IMPL_LINK( MenuManager, Deactivate, Menu *, pMenu )
{
    if ( pMenu == m_pVCLMenu )
        m_bActive = sal_False;

    return 0;
}


IMPL_LINK( MenuManager, Select, Menu *, pMenu )
{
    if ( pMenu == m_pVCLMenu )
    {
        LOCK_MUTEX( aGuard, m_aMutex, "MenuManager::Select" )
        USHORT nCurItemId = pMenu->GetCurItemId();
        if ( pMenu->GetItemType( nCurItemId ) != MENUITEM_SEPARATOR )
        {
            MenuItemHandler* pMenuItemHandler = GetMenuItemHandler( nCurItemId );
            if ( pMenuItemHandler && pMenuItemHandler->xMenuItemDispatch.is() )
            {
                URL aTargetURL;
                aTargetURL.Complete = pMenuItemHandler->aMenuItemURL;

                REFERENCE< XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))), UNO_QUERY );
                xTrans->parseStrict( aTargetURL );

                Sequence<PropertyValue> aArgs;
                pMenuItemHandler->xMenuItemDispatch->dispatch( aTargetURL, aArgs );
            }
        }
    }

    return 0;
}


IMPL_LINK( MenuManager, Highlight, Menu *, pMenu )
{
    return 0;
}

}