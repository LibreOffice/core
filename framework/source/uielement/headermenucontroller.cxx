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

#include <uielement/headermenucontroller.hxx>

#include <threadhelp/resetableguard.hxx>
#include "services.h"

#include <classes/resource.hrc>
#include <classes/fwlresid.hxx>

#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/awt/MenuItemStyle.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <vcl/menu.hxx>
#include <vcl/svapp.hxx>
#include <vcl/i18nhelp.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/mutex.hxx>

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace com::sun::star::style;
using namespace com::sun::star::container;

const sal_uInt16 ALL_MENUITEM_ID = 1;

namespace framework
{

DEFINE_XSERVICEINFO_MULTISERVICE_2      (   HeaderMenuController                    ,
                                            OWeakObject                             ,
                                            SERVICENAME_POPUPMENUCONTROLLER         ,
                                            IMPLEMENTATIONNAME_HEADERMENUCONTROLLER
                                        )

DEFINE_INIT_SERVICE                     (   HeaderMenuController, {} )

HeaderMenuController::HeaderMenuController( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext, bool _bFooter ) :
    svt::PopupMenuControllerBase( xContext )
    ,m_bFooter(_bFooter)
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

    SolarMutexGuard aSolarMutexGuard;

    resetPopupMenu( rPopupMenu );
    if ( pPopupMenu )
        pVCLPopupMenu = (PopupMenu *)pPopupMenu->GetMenu();

    Reference< XStyleFamiliesSupplier > xStyleFamiliesSupplier( rModel, UNO_QUERY );
    if ( pVCLPopupMenu && xStyleFamiliesSupplier.is())
    {
        Reference< XNameAccess > xStyleFamilies = xStyleFamiliesSupplier->getStyleFamilies();

        OUString aCmd( ".uno:InsertPageHeader" );
        OUString aHeaderFooterIsOnStr( "HeaderIsOn" );
        if ( m_bFooter )
        {
            aCmd = OUString( ".uno:InsertPageFooter" );
            aHeaderFooterIsOnStr = OUString( "FooterIsOn" );
        }
        const OUString aIsPhysicalStr( "IsPhysical" );
        const OUString aDisplayNameStr( "DisplayName" );

        try
        {
            Reference< XNameContainer > xNameContainer;
            if ( xStyleFamilies->getByName("PageStyles") >>= xNameContainer )
            {
                Sequence< OUString > aSeqNames = xNameContainer->getElementNames();

                sal_uInt16   nId = 2;
                sal_uInt16  nCount = 0;
                sal_Bool bAllOneState( sal_True );
                sal_Bool bLastCheck( sal_True );
                sal_Bool bFirstChecked( sal_False );
                sal_Bool bFirstItemInserted( sal_False );
                for ( sal_Int32 n = 0; n < aSeqNames.getLength(); n++ )
                {
                    OUString aName = aSeqNames[n];
                    Reference< XPropertySet > xPropSet( xNameContainer->getByName( aName ), UNO_QUERY );
                    if ( xPropSet.is() )
                    {
                        sal_Bool bIsPhysical( sal_False );
                        if (( xPropSet->getPropertyValue( aIsPhysicalStr ) >>= bIsPhysical ) && bIsPhysical )
                        {
                            OUString aDisplayName;
                            sal_Bool      bHeaderIsOn( sal_False );
                            xPropSet->getPropertyValue( aDisplayNameStr ) >>= aDisplayName;
                            xPropSet->getPropertyValue( aHeaderFooterIsOnStr ) >>= bHeaderIsOn;

                            OUStringBuffer aStrBuf( aCmd );
                            aStrBuf.appendAscii( "?PageStyle:string=");
                            aStrBuf.append( aDisplayName );
                            aStrBuf.appendAscii( "&On:bool=" );
                            if ( !bHeaderIsOn )
                                aStrBuf.appendAscii( "true" );
                            else
                                aStrBuf.appendAscii( "false" );
                            OUString aCommand( aStrBuf.makeStringAndClear() );
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
                                bAllOneState = sal_False;
                            bLastCheck = bHeaderIsOn;
                            ++nCount;
                        }
                    }
                }

                if ( bAllOneState && ( nCount > 1 ))
                {
                    // Insert special item for all command
                    pVCLPopupMenu->InsertItem( ALL_MENUITEM_ID, FwlResId(STR_MENU_HEADFOOTALL).toString(), 0, OString(), 0 );

                    OUStringBuffer aStrBuf( aCmd );
                    aStrBuf.appendAscii( "?On:bool=" );

                    // Command depends on check state of first menu item entry
                    if ( !bFirstChecked )
                        aStrBuf.appendAscii( "true" );
                    else
                        aStrBuf.appendAscii( "false" );

                    pVCLPopupMenu->SetItemCommand( 1, aStrBuf.makeStringAndClear() );
                    pVCLPopupMenu->InsertSeparator(OString(), 1);
                }
            }
        }
        catch ( const com::sun::star::container::NoSuchElementException& )
        {
        }
    }
}

// XEventListener
void SAL_CALL HeaderMenuController::disposing( const EventObject& ) throw ( RuntimeException )
{
    Reference< css::awt::XMenuListener > xHolder(( OWeakObject *)this, UNO_QUERY );

    osl::MutexGuard aLock( m_aMutex );
    m_xFrame.clear();
    m_xDispatch.clear();

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
        osl::MutexGuard aLock( m_aMutex );
        m_xModel = xModel;
        if ( m_xPopupMenu.is() )
            fillPopupMenu( xModel, m_xPopupMenu );
    }
}

// XMenuListener
void HeaderMenuController::impl_select(const Reference< XDispatch >& _xDispatch,const ::com::sun::star::util::URL& aTargetURL)
{
    Sequence<PropertyValue>      aArgs;
    OSL_ENSURE(_xDispatch.is(),"HeaderMenuController::impl_select: No dispatch");
    if ( _xDispatch.is() )
        _xDispatch->dispatch( aTargetURL, aArgs );
}

void SAL_CALL HeaderMenuController::updatePopupMenu() throw (::com::sun::star::uno::RuntimeException)
{
    osl::ResettableMutexGuard aLock( m_aMutex );

    throwIfDisposed();

    Reference< com::sun::star::frame::XModel > xModel( m_xModel );
    aLock.clear();

    if ( !xModel.is() )
        svt::PopupMenuControllerBase::updatePopupMenu();

    aLock.reset();
    if ( m_xPopupMenu.is() && m_xModel.is() )
        fillPopupMenu( m_xModel, m_xPopupMenu );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
