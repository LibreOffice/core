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

#include "services.h"

#include <strings.hrc>
#include <classes/fwkresid.hxx>

#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/awt/MenuItemStyle.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <vcl/menu.hxx>
#include <vcl/svapp.hxx>
#include <vcl/i18nhelp.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/mutex.hxx>

//  Defines

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

HeaderMenuController::HeaderMenuController( const css::uno::Reference< css::uno::XComponentContext >& xContext, bool _bFooter ) :
    svt::PopupMenuControllerBase( xContext )
    ,m_bFooter(_bFooter)
{
}

HeaderMenuController::~HeaderMenuController()
{
}

// private function
void HeaderMenuController::fillPopupMenu( const Reference< css::frame::XModel >& rModel, Reference< css::awt::XPopupMenu >& rPopupMenu )
{
    VCLXPopupMenu*       pPopupMenu        = static_cast<VCLXPopupMenu *>(VCLXMenu::GetImplementation( rPopupMenu ));
    PopupMenu*           pVCLPopupMenu     = nullptr;

    SolarMutexGuard aSolarMutexGuard;

    resetPopupMenu( rPopupMenu );
    if ( pPopupMenu )
        pVCLPopupMenu = static_cast<PopupMenu *>(pPopupMenu->GetMenu());

    Reference< XStyleFamiliesSupplier > xStyleFamiliesSupplier( rModel, UNO_QUERY );
    if ( pVCLPopupMenu && xStyleFamiliesSupplier.is())
    {
        Reference< XNameAccess > xStyleFamilies = xStyleFamiliesSupplier->getStyleFamilies();

        OUString aCmd( ".uno:InsertPageHeader" );
        OUString aHeaderFooterIsOnStr( "HeaderIsOn" );
        if ( m_bFooter )
        {
            aCmd = ".uno:InsertPageFooter";
            aHeaderFooterIsOnStr = "FooterIsOn";
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
                bool bAllOneState( true );
                bool bLastCheck( true );
                bool bFirstChecked( false );
                bool bFirstItemInserted( false );
                for ( sal_Int32 n = 0; n < aSeqNames.getLength(); n++ )
                {
                    OUString aName = aSeqNames[n];
                    Reference< XPropertySet > xPropSet( xNameContainer->getByName( aName ), UNO_QUERY );
                    if ( xPropSet.is() )
                    {
                        bool bIsPhysical( false );
                        if (( xPropSet->getPropertyValue( aIsPhysicalStr ) >>= bIsPhysical ) && bIsPhysical )
                        {
                            OUString aDisplayName;
                            bool      bHeaderIsOn( false );
                            xPropSet->getPropertyValue( aDisplayNameStr ) >>= aDisplayName;
                            xPropSet->getPropertyValue( aHeaderFooterIsOnStr ) >>= bHeaderIsOn;

                            OUStringBuffer aStrBuf( aCmd );
                            aStrBuf.append( "?PageStyle:string=");
                            aStrBuf.append( aDisplayName );
                            aStrBuf.append( "&On:bool=" );
                            if ( !bHeaderIsOn )
                                aStrBuf.append( "true" );
                            else
                                aStrBuf.append( "false" );
                            OUString aCommand( aStrBuf.makeStringAndClear() );
                            pVCLPopupMenu->InsertItem( nId, aDisplayName, MenuItemBits::CHECKABLE );
                            if ( !bFirstItemInserted )
                            {
                                bFirstItemInserted = true;
                                bFirstChecked      = bHeaderIsOn;
                            }

                            pVCLPopupMenu->SetItemCommand( nId, aCommand );

                            if ( bHeaderIsOn )
                                pVCLPopupMenu->CheckItem( nId );
                            ++nId;

                            // Check if all entries have the same state
                            if( bAllOneState && n && bHeaderIsOn != bLastCheck )
                                bAllOneState = false;
                            bLastCheck = bHeaderIsOn;
                            ++nCount;
                        }
                    }
                }

                if ( bAllOneState && ( nCount > 1 ))
                {
                    // Insert special item for all command
                    pVCLPopupMenu->InsertItem( ALL_MENUITEM_ID, FwkResId(STR_MENU_HEADFOOTALL), MenuItemBits::NONE, OString(), 0 );

                    OUStringBuffer aStrBuf( aCmd );
                    aStrBuf.append( "?On:bool=" );

                    // Command depends on check state of first menu item entry
                    if ( !bFirstChecked )
                        aStrBuf.append( "true" );
                    else
                        aStrBuf.append( "false" );

                    pVCLPopupMenu->SetItemCommand( 1, aStrBuf.makeStringAndClear() );
                    pVCLPopupMenu->InsertSeparator(OString(), 1);
                }
            }
        }
        catch ( const css::container::NoSuchElementException& )
        {
        }
    }
}

// XEventListener
void SAL_CALL HeaderMenuController::disposing( const EventObject& )
{
    Reference< css::awt::XMenuListener > xHolder(static_cast<OWeakObject *>(this), UNO_QUERY );

    osl::MutexGuard aLock( m_aMutex );
    m_xFrame.clear();
    m_xDispatch.clear();

    if ( m_xPopupMenu.is() )
        m_xPopupMenu->removeMenuListener( Reference< css::awt::XMenuListener >(static_cast<OWeakObject *>(this), UNO_QUERY ));
    m_xPopupMenu.clear();
}

// XStatusListener
void SAL_CALL HeaderMenuController::statusChanged( const FeatureStateEvent& Event )
{
    Reference< css::frame::XModel > xModel;

    if ( Event.State >>= xModel )
    {
        osl::MutexGuard aLock( m_aMutex );
        m_xModel = xModel;
        if ( m_xPopupMenu.is() )
            fillPopupMenu( xModel, m_xPopupMenu );
    }
}

// XMenuListener
void SAL_CALL HeaderMenuController::updatePopupMenu()
{
    osl::ResettableMutexGuard aLock( m_aMutex );

    throwIfDisposed();

    Reference< css::frame::XModel > xModel( m_xModel );
    aLock.clear();

    if ( !xModel.is() )
        svt::PopupMenuControllerBase::updatePopupMenu();

    aLock.reset();
    if ( m_xPopupMenu.is() && m_xModel.is() )
        fillPopupMenu( m_xModel, m_xPopupMenu );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
