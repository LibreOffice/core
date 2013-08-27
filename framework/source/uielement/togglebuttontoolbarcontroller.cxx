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


#include "uielement/togglebuttontoolbarcontroller.hxx"

#include <framework/addonsoptions.hxx>

#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XControlNotificationListener.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <rtl/uri.hxx>
#include <osl/mutex.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/window.hxx>
#include <vcl/graph.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/toolbox.hxx>
#include <svtools/miscopt.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::util;

namespace framework
{

// ------------------------------------------------------------------

ToggleButtonToolbarController::ToggleButtonToolbarController(
    const Reference< XComponentContext >&    rxContext,
    const Reference< XFrame >&               rFrame,
    ToolBox*                                 pToolbar,
    sal_uInt16                                   nID,
    Style                                    eStyle,
    const OUString&                          aCommand ) :
    ComplexToolbarController( rxContext, rFrame, pToolbar, nID, aCommand ),
    m_eStyle( eStyle )
{
    if ( eStyle == STYLE_DROPDOWNBUTTON )
        m_pToolbar->SetItemBits( m_nID, TIB_DROPDOWNONLY | m_pToolbar->GetItemBits( m_nID ) );
    else if ( eStyle == STYLE_TOGGLE_DROPDOWNBUTTON )
        m_pToolbar->SetItemBits( m_nID, TIB_DROPDOWN | m_pToolbar->GetItemBits( m_nID ) );
}

// ------------------------------------------------------------------

ToggleButtonToolbarController::~ToggleButtonToolbarController()
{
}

// ------------------------------------------------------------------

void SAL_CALL ToggleButtonToolbarController::dispose()
throw ( RuntimeException )
{
    SolarMutexGuard aSolarMutexGuard;
    ComplexToolbarController::dispose();
}

// ------------------------------------------------------------------
Sequence<PropertyValue> ToggleButtonToolbarController::getExecuteArgs(sal_Int16 KeyModifier) const
{
    Sequence<PropertyValue> aArgs( 2 );

    // Add key modifier to argument list
    aArgs[0].Name = OUString( "KeyModifier" );
    aArgs[0].Value <<= KeyModifier;
    aArgs[1].Name = OUString( "Text" );
    aArgs[1].Value <<= m_aCurrentSelection;
    return aArgs;
}

// ------------------------------------------------------------------

uno::Reference< awt::XWindow > SAL_CALL ToggleButtonToolbarController::createPopupWindow()
throw (::com::sun::star::uno::RuntimeException)
{
    uno::Reference< awt::XWindow > xWindow;

    SolarMutexGuard aSolarMutexGuard;
    if (( m_eStyle == STYLE_DROPDOWNBUTTON ) ||
        ( m_eStyle == STYLE_TOGGLE_DROPDOWNBUTTON ))
    {
        // create popup menu
        ::PopupMenu aPopup;
        const sal_uInt32 nCount = m_aDropdownMenuList.size();
        for ( sal_uInt32 i = 0; i < nCount; i++ )
        {
            OUString aLabel( m_aDropdownMenuList[i] );
            aPopup.InsertItem( sal_uInt16( i+1 ), aLabel );
            if ( aLabel == m_aCurrentSelection )
                aPopup.CheckItem( sal_uInt16( i+1 ), sal_True );
            else
                aPopup.CheckItem( sal_uInt16( i+1 ), sal_False );
        }

        m_pToolbar->SetItemDown( m_nID, sal_True );
        aPopup.SetSelectHdl( LINK( this, ToggleButtonToolbarController, MenuSelectHdl ));
        aPopup.Execute( m_pToolbar, m_pToolbar->GetItemRect( m_nID ));
        m_pToolbar->SetItemDown( m_nID, sal_False );
    }

    return xWindow;
}

// ------------------------------------------------------------------

void ToggleButtonToolbarController::executeControlCommand( const ::com::sun::star::frame::ControlCommand& rControlCommand )
{
    SolarMutexGuard aSolarMutexGuard;

    if (( m_eStyle == STYLE_DROPDOWNBUTTON ) ||
        ( m_eStyle == STYLE_TOGGLE_DROPDOWNBUTTON ))
    {
        if ( rControlCommand.Command.equalsAsciiL( "SetList", 7 ))
        {
            for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
            {
                if ( rControlCommand.Arguments[i].Name.equalsAsciiL( "List", 4 ))
                {
                    Sequence< OUString > aList;
                    m_aDropdownMenuList.clear();

                    rControlCommand.Arguments[i].Value >>= aList;
                    for ( sal_Int32 j = 0; j < aList.getLength(); j++ )
                        m_aDropdownMenuList.push_back( aList[j] );

                    // send notification
                    uno::Sequence< beans::NamedValue > aInfo( 1 );
                    aInfo[0].Name  = OUString( "List" );
                    aInfo[0].Value <<= aList;
                    addNotifyInfo( OUString( "ListChanged" ),
                                getDispatchFromCommand( m_aCommandURL ),
                                aInfo );

                    break;
                }
            }
        }
        else if ( rControlCommand.Command.equalsAsciiL( "CheckItemPos", 12 ))
        {
            for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
            {
                if ( rControlCommand.Arguments[i].Name.equalsAsciiL( "Pos", 3 ))
                {
                    sal_Int32 nPos( -1 );

                    rControlCommand.Arguments[i].Value >>= nPos;
                    if ( nPos >= 0 &&
                         ( sal::static_int_cast< sal_uInt32 >(nPos)
                           < m_aDropdownMenuList.size() ) )
                    {
                        m_aCurrentSelection = m_aDropdownMenuList[nPos];

                        // send notification
                        uno::Sequence< beans::NamedValue > aInfo( 1 );
                        aInfo[0].Name  = OUString( "ItemChecked" );
                        aInfo[0].Value <<= nPos;
                        addNotifyInfo( OUString( "Pos" ),
                                    getDispatchFromCommand( m_aCommandURL ),
                                    aInfo );
                    }
                    break;
                }
            }
        }
        else if ( rControlCommand.Command.equalsAsciiL( "AddEntry", 8 ))
        {
            OUString   aText;
            for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
            {
                if ( rControlCommand.Arguments[i].Name.equalsAsciiL( "Text", 4 ))
                {
                    if ( rControlCommand.Arguments[i].Value >>= aText )
                        m_aDropdownMenuList.push_back( aText );
                    break;
                }
            }
        }
        else if ( rControlCommand.Command.equalsAsciiL( "InsertEntry", 11 ))
        {
            sal_Int32      nPos( COMBOBOX_APPEND );
            sal_Int32      nSize = sal_Int32( m_aDropdownMenuList.size() );
            OUString  aText;
            for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
            {
                if ( rControlCommand.Arguments[i].Name.equalsAsciiL( "Pos", 3 ))
                {
                    sal_Int32 nTmpPos = 0;
                    if ( rControlCommand.Arguments[i].Value >>= nTmpPos )
                    {
                        if (( nTmpPos >= 0 ) && ( nTmpPos < sal_Int32( nSize )))
                            nPos = nTmpPos;
                    }
                }
                else if ( rControlCommand.Arguments[i].Name.equalsAsciiL( "Text", 4 ))
                    rControlCommand.Arguments[i].Value >>= aText;
            }

            std::vector< OUString >::iterator aIter = m_aDropdownMenuList.begin();
            aIter += nPos;
            m_aDropdownMenuList.insert( aIter, aText );
        }
        else if ( rControlCommand.Command.equalsAsciiL( "RemoveEntryPos", 14 ))
        {
            for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
            {
                if ( rControlCommand.Arguments[i].Name.equalsAsciiL( "Pos", 3 ))
                {
                    sal_Int32 nPos( -1 );
                    if ( rControlCommand.Arguments[i].Value >>= nPos )
                    {
                        if ( nPos < sal_Int32( m_aDropdownMenuList.size() ))
                        {
                            m_aDropdownMenuList.erase(m_aDropdownMenuList.begin() + nPos);
                        }
                    }
                    break;
                }
            }
        }
        else if ( rControlCommand.Command.equalsAsciiL( "RemoveEntryText", 15 ))
        {
            for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
            {
                if ( rControlCommand.Arguments[i].Name.equalsAsciiL( "Text", 4 ))
                {
                    OUString aText;
                    if ( rControlCommand.Arguments[i].Value >>= aText )
                    {
                        sal_Int32 nSize = sal_Int32( m_aDropdownMenuList.size() );
                        for ( sal_Int32 j = 0; j < nSize; j++ )
                        {
                            if ( m_aDropdownMenuList[j] == aText )
                            {
                                std::vector< OUString >::iterator aIter = m_aDropdownMenuList.begin();
                                aIter += j;
                                m_aDropdownMenuList.erase( aIter );
                                break;
                            }
                        }
                    }
                    break;
                }
            }
        }
    }
}

IMPL_LINK( ToggleButtonToolbarController, MenuSelectHdl, Menu *, pMenu )
{
    SolarMutexGuard aGuard;

    sal_uInt16 nItemId = pMenu->GetCurItemId();
    if ( nItemId > 0 && nItemId <= m_aDropdownMenuList.size() )
    {
        m_aCurrentSelection = m_aDropdownMenuList[nItemId-1];

        execute( 0 );
    }
    return 0;
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
