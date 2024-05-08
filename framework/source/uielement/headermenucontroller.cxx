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

#include <services.h>

#include <strings.hrc>
#include <classes/fwkresid.hxx>

#include <com/sun/star/awt/MenuItemStyle.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <vcl/svapp.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/mutex.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <toolkit/awt/vclxmenu.hxx>

//  Defines

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace com::sun::star::style;
using namespace com::sun::star::container;

const sal_uInt16 ALL_MENUITEM_ID = 1;

namespace framework
{

// XInterface, XTypeProvider, XServiceInfo

OUString SAL_CALL HeaderMenuController::getImplementationName()
{
    return u"com.sun.star.comp.framework.HeaderMenuController"_ustr;
}

sal_Bool SAL_CALL HeaderMenuController::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

css::uno::Sequence< OUString > SAL_CALL HeaderMenuController::getSupportedServiceNames()
{
    return { SERVICENAME_POPUPMENUCONTROLLER };
}

HeaderMenuController::HeaderMenuController( const css::uno::Reference< css::uno::XComponentContext >& xContext, bool _bFooter ) :
    svt::PopupMenuControllerBase( xContext )
    ,m_bFooter(_bFooter)
{
}

HeaderMenuController::~HeaderMenuController()
{
}

// private function
void HeaderMenuController::fillPopupMenu( const Reference< css::frame::XModel >& rModel, Reference< css::awt::XPopupMenu > const & rPopupMenu )
{
    SolarMutexGuard aSolarMutexGuard;

    resetPopupMenu( rPopupMenu );

    Reference< XStyleFamiliesSupplier > xStyleFamiliesSupplier( rModel, UNO_QUERY );
    if (!xStyleFamiliesSupplier.is())
        return;

    Reference< XNameAccess > xStyleFamilies = xStyleFamiliesSupplier->getStyleFamilies();

    OUString aCmd( u".uno:InsertPageHeader"_ustr );
    OUString aHeaderFooterIsOnStr( u"HeaderIsOn"_ustr );
    if ( m_bFooter )
    {
        aCmd = ".uno:InsertPageFooter";
        aHeaderFooterIsOnStr = "FooterIsOn";
    }
    static constexpr OUStringLiteral aIsPhysicalStr( u"IsPhysical" );
    static constexpr OUStringLiteral aDisplayNameStr( u"DisplayName" );

    try
    {
        Reference< XNameContainer > xNameContainer;
        if ( xStyleFamilies->getByName(u"PageStyles"_ustr) >>= xNameContainer )
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

                        OUStringBuffer aStrBuf( aCmd
                            + "?PageStyle:string="
                            + aDisplayName
                            + "&On:bool=" );
                        if ( !bHeaderIsOn )
                            aStrBuf.append( "true" );
                        else
                            aStrBuf.append( "false" );
                        OUString aCommand( aStrBuf.makeStringAndClear() );
                        rPopupMenu->insertItem(nId, aDisplayName, css::awt::MenuItemStyle::CHECKABLE, nCount);
                        if ( !bFirstItemInserted )
                        {
                            bFirstItemInserted = true;
                            bFirstChecked      = bHeaderIsOn;
                        }

                        rPopupMenu->setCommand(nId, aCommand);
                        rPopupMenu->checkItem(nId, bHeaderIsOn);
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
                rPopupMenu->insertItem(ALL_MENUITEM_ID, FwkResId(STR_MENU_HEADFOOTALL), 0, 0);

                OUStringBuffer aStrBuf( aCmd + "?On:bool=" );

                // Command depends on check state of first menu item entry
                if ( !bFirstChecked )
                    aStrBuf.append( "true" );
                else
                    aStrBuf.append( "false" );

                rPopupMenu->setCommand(1, aStrBuf.makeStringAndClear());
                rPopupMenu->insertSeparator(1);
            }
        }
    }
    catch ( const css::container::NoSuchElementException& )
    {
    }
}

// XEventListener
void SAL_CALL HeaderMenuController::disposing( const EventObject& )
{
    Reference< css::awt::XMenuListener > xHolder(this);

    std::unique_lock aLock( m_aMutex );
    m_xFrame.clear();
    m_xDispatch.clear();

    if ( m_xPopupMenu.is() )
        m_xPopupMenu->removeMenuListener( Reference< css::awt::XMenuListener >(this) );
    m_xPopupMenu.clear();
}

// XStatusListener
void SAL_CALL HeaderMenuController::statusChanged( const FeatureStateEvent& Event )
{
    Reference< css::frame::XModel > xModel;

    if ( Event.State >>= xModel )
    {
        std::unique_lock aLock( m_aMutex );
        m_xModel = xModel;
        if ( m_xPopupMenu.is() )
            fillPopupMenu( xModel, m_xPopupMenu );
    }
}

// XMenuListener
void SAL_CALL HeaderMenuController::updatePopupMenu()
{
    std::unique_lock aLock( m_aMutex );

    throwIfDisposed(aLock);

    Reference< css::frame::XModel > xModel( m_xModel );
    aLock.unlock();

    if ( !xModel.is() )
        svt::PopupMenuControllerBase::updatePopupMenu();

    aLock.lock();
    if ( m_xPopupMenu.is() && m_xModel.is() )
        fillPopupMenu( m_xModel, m_xPopupMenu );
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
framework_HeaderMenuController_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(new framework::HeaderMenuController(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
