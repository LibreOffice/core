/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: headermenucontroller.cxx,v $
 * $Revision: 1.9 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"
#include <uielement/headermenucontroller.hxx>

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <threadhelp/resetableguard.hxx>
#include "services.h"

#ifndef __FRAMEWORK_CLASSES_RESOURCE_HRC_
#include <classes/resource.hrc>
#endif
#include <classes/fwkresid.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/awt/MenuItemStyle.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _VCL_MENU_HXX_
#include <vcl/menu.hxx>
#endif
#include <vcl/svapp.hxx>
#include <vcl/i18nhelp.hxx>
#include <tools/urlobj.hxx>
#include <rtl/ustrbuf.hxx>
//#include <tools/solar.hrc>

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________
//

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace com::sun::star::style;
using namespace com::sun::star::container;

// Copied from Writer module
//#define RID_SW_SHELLRES (RID_SW_START + 1250 + 1)
//#define STR_ALLPAGE_HEADFOOT 14

const USHORT ALL_MENUITEM_ID = 1;

namespace framework
{

DEFINE_XSERVICEINFO_MULTISERVICE        (   HeaderMenuController                    ,
                                            OWeakObject                             ,
                                            SERVICENAME_POPUPMENUCONTROLLER         ,
                                            IMPLEMENTATIONNAME_HEADERMENUCONTROLLER
                                        )

DEFINE_INIT_SERVICE                     (   HeaderMenuController, {} )

HeaderMenuController::HeaderMenuController( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager ) :
    PopupMenuControllerBase( xServiceManager )
{
}

HeaderMenuController::~HeaderMenuController()
{
}

// private function
void HeaderMenuController::fillPopupMenu( const Reference< ::com::sun::star::frame::XModel >& rModel, Reference< css::awt::XPopupMenu >& rPopupMenu )
{
    VCLXPopupMenu*                                     pPopupMenu        = (VCLXPopupMenu *)VCLXMenu::GetImplementation( rPopupMenu );
    PopupMenu*                                         pVCLPopupMenu     = 0;

    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

    resetPopupMenu( rPopupMenu );
    if ( pPopupMenu )
        pVCLPopupMenu = (PopupMenu *)pPopupMenu->GetMenu();

    Reference< XStyleFamiliesSupplier > xStyleFamiliesSupplier( rModel, UNO_QUERY );
    if ( pVCLPopupMenu && xStyleFamiliesSupplier.is())
    {
        Reference< XNameAccess > xStyleFamilies = xStyleFamiliesSupplier->getStyleFamilies();

        try
        {
            const rtl::OUString aCmd( RTL_CONSTASCII_USTRINGPARAM( ".uno:InsertPageHeader" ));
            const rtl::OUString aIsPhysicalStr( RTL_CONSTASCII_USTRINGPARAM( "IsPhysical" ));
            const rtl::OUString aDisplayNameStr( RTL_CONSTASCII_USTRINGPARAM( "DisplayName" ));
            const rtl::OUString aHeaderIsOnStr( RTL_CONSTASCII_USTRINGPARAM( "HeaderIsOn" ));

            Reference< XNameContainer > xNameContainer;
            Any a = xStyleFamilies->getByName( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PageStyles" )));
            if ( a >>= xNameContainer )
            {
                Sequence< rtl::OUString > aSeqNames = xNameContainer->getElementNames();

                USHORT   nId = 2;
                USHORT  nCount = 0;
                sal_Bool bAllOneState( sal_True );
                sal_Bool bLastCheck( sal_True );
                sal_Bool bFirstChecked( sal_False );
                sal_Bool bFirstItemInserted( sal_False );
                for ( sal_Int32 n = 0; n < aSeqNames.getLength(); n++ )
                {
                    rtl::OUString aName = aSeqNames[n];
                    Reference< XPropertySet > xPropSet( xNameContainer->getByName( aName ), UNO_QUERY );
                    if ( xPropSet.is() )
                    {
                        sal_Bool bIsPhysical( sal_False );
                        a = xPropSet->getPropertyValue( aIsPhysicalStr );
                        if (( a >>= bIsPhysical ) && bIsPhysical )
                        {
                            rtl::OUString aDisplayName;
                            sal_Bool      bHeaderIsOn( sal_False );
                            a = xPropSet->getPropertyValue( aDisplayNameStr );
                            a >>= aDisplayName;
                            a = xPropSet->getPropertyValue( aHeaderIsOnStr );
                            a >>= bHeaderIsOn;

                            rtl::OUStringBuffer aStrBuf( aCmd );
                            aStrBuf.appendAscii( "?PageStyle:string=");
                            aStrBuf.append( aDisplayName );
                            aStrBuf.appendAscii( "&On:bool=" );
                            if ( !bHeaderIsOn )
                                aStrBuf.appendAscii( "true" );
                            else
                                aStrBuf.appendAscii( "false" );
                            rtl::OUString aCommand( aStrBuf.makeStringAndClear() );
                            pVCLPopupMenu->InsertItem( nId, aDisplayName, MIB_CHECKABLE );
                            if ( !bFirstItemInserted )
                            {
                                bFirstItemInserted = sal_True;
                                bFirstChecked      = bHeaderIsOn;
                            }

                            pVCLPopupMenu->SetItemCommand( nId, aCommand );

                            if ( bHeaderIsOn )
                                pVCLPopupMenu->CheckItem( nId, sal_True );
                            ++nId;

                            // Check if all entries have the same state
                            if( bAllOneState && n && bHeaderIsOn != bLastCheck )
                                bAllOneState = FALSE;
                            bLastCheck = bHeaderIsOn;
                            ++nCount;
                        }
                    }
                }

                if ( bAllOneState && ( nCount > 1 ))
                {
                    // Insert special item for all command
                    pVCLPopupMenu->InsertItem( ALL_MENUITEM_ID, String( FwkResId( STR_MENU_HEADFOOTALL )), 0, 0 );

                    rtl::OUStringBuffer aStrBuf( aCmd );
                    aStrBuf.appendAscii( "?On:bool=" );

                    // Command depends on check state of first menu item entry
                    if ( !bFirstChecked )
                        aStrBuf.appendAscii( "true" );
                    else
                        aStrBuf.appendAscii( "false" );

                    pVCLPopupMenu->SetItemCommand( 1, aStrBuf.makeStringAndClear() );
                    pVCLPopupMenu->InsertSeparator( 1 );
                }
            }
        }
        catch ( com::sun::star::container::NoSuchElementException& )
        {
        }
    }
}

// XEventListener
void SAL_CALL HeaderMenuController::disposing( const EventObject& ) throw ( RuntimeException )
{
    Reference< css::awt::XMenuListener > xHolder(( OWeakObject *)this, UNO_QUERY );

    ResetableGuard aLock( m_aLock );
    m_xFrame.clear();
    m_xDispatch.clear();
    m_xServiceManager.clear();

    if ( m_xPopupMenu.is() )
        m_xPopupMenu->removeMenuListener( Reference< css::awt::XMenuListener >(( OWeakObject *)this, UNO_QUERY ));
    m_xPopupMenu.clear();
}

// XStatusListener
void SAL_CALL HeaderMenuController::statusChanged( const FeatureStateEvent& Event ) throw ( RuntimeException )
{
    Reference< com::sun::star::frame::XModel > xModel;

    if ( Event.State >>= xModel )
    {
        ResetableGuard aLock( m_aLock );
        m_xModel = xModel;
        if ( m_xPopupMenu.is() )
            fillPopupMenu( xModel, m_xPopupMenu );
    }
}

// XMenuListener
void SAL_CALL HeaderMenuController::highlight( const css::awt::MenuEvent& ) throw (RuntimeException)
{
}

void SAL_CALL HeaderMenuController::select( const css::awt::MenuEvent& rEvent ) throw (RuntimeException)
{
    Reference< css::awt::XPopupMenu >   xPopupMenu;
    Reference< XDispatch >              xDispatch;
    Reference< XMultiServiceFactory >   xServiceManager;

    ResetableGuard aLock( m_aLock );
    xPopupMenu      = m_xPopupMenu;
    xDispatch       = m_xDispatch;
    xServiceManager = m_xServiceManager;
    aLock.unlock();

    if ( xPopupMenu.is() && xDispatch.is() )
    {
        VCLXPopupMenu* pPopupMenu = (VCLXPopupMenu *)VCLXPopupMenu::GetImplementation( xPopupMenu );
        if ( pPopupMenu )
        {
            css::util::URL               aTargetURL;
            Sequence<PropertyValue>      aArgs( 1 );
            Reference< XURLTransformer > xURLTransformer( xServiceManager->createInstance(
                                                            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))),
                                                        UNO_QUERY );

            {
                vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
                PopupMenu* pVCLPopupMenu = (PopupMenu *)pPopupMenu->GetMenu();

                aTargetURL.Complete = pVCLPopupMenu->GetItemCommand( rEvent.MenuId );
            }

            xURLTransformer->parseStrict( aTargetURL );
            xDispatch->dispatch( aTargetURL, aArgs );
        }
    }
}

void SAL_CALL HeaderMenuController::activate( const css::awt::MenuEvent& ) throw (RuntimeException)
{
}

void SAL_CALL HeaderMenuController::deactivate( const css::awt::MenuEvent& ) throw (RuntimeException)
{
}

// XPopupMenuController
void SAL_CALL HeaderMenuController::setPopupMenu( const Reference< css::awt::XPopupMenu >& xPopupMenu ) throw ( RuntimeException )
{
    ResetableGuard aLock( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    if ( m_xFrame.is() && !m_xPopupMenu.is() )
    {
        // Create popup menu on demand
        vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

        m_xPopupMenu = xPopupMenu;
        m_xPopupMenu->addMenuListener( Reference< css::awt::XMenuListener >( (OWeakObject*)this, UNO_QUERY ));

        Reference< XURLTransformer > xURLTransformer( m_xServiceManager->createInstance(
                                                        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))),
                                                    UNO_QUERY );
        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );

        com::sun::star::util::URL aTargetURL;
        aTargetURL.Complete = m_aCommandURL;
        xURLTransformer->parseStrict( aTargetURL );
        m_xDispatch = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );

        updatePopupMenu();
    }
}

void SAL_CALL HeaderMenuController::updatePopupMenu() throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    Reference< com::sun::star::frame::XModel > xModel( m_xModel );
    aLock.unlock();

    if ( !xModel.is() )
        PopupMenuControllerBase::updatePopupMenu();

    aLock.lock();
    if ( m_xPopupMenu.is() && m_xModel.is() )
        fillPopupMenu( m_xModel, m_xPopupMenu );
}

// XInitialization
void SAL_CALL HeaderMenuController::initialize( const Sequence< Any >& aArguments ) throw ( Exception, RuntimeException )
{
    PopupMenuControllerBase::initialize( aArguments );
}

}
