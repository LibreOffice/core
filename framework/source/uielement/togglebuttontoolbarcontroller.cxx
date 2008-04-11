/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: togglebuttontoolbarcontroller.cxx,v $
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

#ifndef __FRAMEWORK_UIELEMENT_TOGGLEBUTTONTOOLBARCONTROLLER_HXX
#include "uielement/togglebuttontoolbarcontroller.hxx"
#endif

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <classes/addonsoptions.hxx>
#ifndef __FRAMEWORK_TOOLBAR_HXX_
#include "uielement/toolbar.hxx"
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/frame/XControlNotificationListener.hpp>
#include "com/sun/star/util/XMacroExpander.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#include <rtl/uri.hxx>
#include <vos/mutex.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <tools/urlobj.hxx>
#include <vcl/svapp.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/window.hxx>
#include <vcl/graph.hxx>
#include <vcl/bitmap.hxx>
#include <svtools/filter.hxx>
#include <svtools/miscopt.hxx>

using namespace ::rtl;
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
    const Reference< XMultiServiceFactory >& rServiceManager,
    const Reference< XFrame >&               rFrame,
    ToolBox*                                 pToolbar,
    USHORT                                   nID,
    Style                                    eStyle,
    const OUString&                          aCommand ) :
    ComplexToolbarController( rServiceManager, rFrame, pToolbar, nID, aCommand ),
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
    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
    ComplexToolbarController::dispose();
}

// ------------------------------------------------------------------

void SAL_CALL ToggleButtonToolbarController::execute( sal_Int16 KeyModifier )
throw ( RuntimeException )
{
    Reference< XDispatch >       xDispatch;
    Reference< XURLTransformer > xURLTransformer;
    OUString                     aCommandURL;
    OUString                     aSelectedText;
    ::com::sun::star::util::URL  aTargetURL;

    {
        vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

        if ( m_bDisposed )
            throw DisposedException();

        if ( m_bInitialized &&
             m_xFrame.is() &&
             m_xServiceManager.is() &&
             m_aCommandURL.getLength() )
        {
            xURLTransformer = m_xURLTransformer;
            xDispatch = getDispatchFromCommand( m_aCommandURL );
            aCommandURL = m_aCommandURL;
            aTargetURL = getInitializedURL();
            aSelectedText = m_aCurrentSelection;
        }
    }

    if ( xDispatch.is() && aTargetURL.Complete.getLength() > 0 )
    {
        Sequence<PropertyValue>   aArgs( 2 );

        // Add key modifier to argument list
        aArgs[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "KeyModifier" ));
        aArgs[0].Value <<= KeyModifier;
        aArgs[1].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Text" ));
        aArgs[1].Value <<= aSelectedText;

        // Execute dispatch asynchronously
        ExecuteInfo* pExecuteInfo = new ExecuteInfo;
        pExecuteInfo->xDispatch     = xDispatch;
        pExecuteInfo->aTargetURL    = aTargetURL;
        pExecuteInfo->aArgs         = aArgs;
        Application::PostUserEvent( STATIC_LINK(0, ComplexToolbarController , ExecuteHdl_Impl), pExecuteInfo );
    }
}

// ------------------------------------------------------------------

uno::Reference< awt::XWindow > SAL_CALL ToggleButtonToolbarController::createPopupWindow()
throw (::com::sun::star::uno::RuntimeException)
{
    uno::Reference< awt::XWindow > xWindow;

    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
    if (( m_eStyle == STYLE_DROPDOWNBUTTON ) ||
        ( m_eStyle == STYLE_TOGGLE_DROPDOWNBUTTON ))
    {
        // create popup menu
        PopupMenu aPopup;

        for ( sal_uInt32 i = 0; i < m_aDropdownMenuList.size(); i++ )
        {
            rtl::OUString aLabel( m_aDropdownMenuList[i] );
            aPopup.InsertItem( sal_uInt16( i+1 ), aLabel );
            if ( aLabel == m_aCurrentSelection )
                aPopup.CheckItem( sal_uInt16( i+1 ), sal_True );
            else
                aPopup.CheckItem( sal_uInt16( i+1 ), sal_False );
        }

        m_pToolbar->SetItemDown( m_nID, TRUE );
        aPopup.SetSelectHdl( LINK( this, ToggleButtonToolbarController, MenuSelectHdl ));
        aPopup.Execute( m_pToolbar, m_pToolbar->GetItemRect( m_nID ));
        m_pToolbar->SetItemDown( m_nID, FALSE );
    }

    return xWindow;
}

// ------------------------------------------------------------------

void ToggleButtonToolbarController::executeControlCommand( const ::com::sun::star::frame::ControlCommand& rControlCommand )
{
    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

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
                    aInfo[0].Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "List" ));
                    aInfo[0].Value <<= aList;
                    addNotifyInfo( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ListChanged" )),
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
                        m_pToolbar->SetItemText( m_nID, m_aCurrentSelection );

                        // send notification
                        uno::Sequence< beans::NamedValue > aInfo( 1 );
                        aInfo[0].Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ItemChecked" ));
                        aInfo[0].Value <<= nPos;
                        addNotifyInfo( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Pos" )),
                                    getDispatchFromCommand( m_aCommandURL ),
                                    aInfo );
                    }
                    break;
                }
            }
        }
        else if ( rControlCommand.Command.equalsAsciiL( "AddEntry", 8 ))
        {
            rtl::OUString   aText;
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
            rtl::OUString  aText;
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

            std::vector< ::rtl::OUString >::iterator aIter = m_aDropdownMenuList.begin();
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
                            std::vector< ::rtl::OUString >::iterator aIter = m_aDropdownMenuList.begin();
                            aIter += nPos;
                            m_aDropdownMenuList.erase( aIter );
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
                    rtl::OUString aText;
                    if ( rControlCommand.Arguments[i].Value >>= aText )
                    {
                        sal_Int32 nSize = sal_Int32( m_aDropdownMenuList.size() );
                        for ( sal_Int32 j = 0; j < nSize; j++ )
                        {
                            if ( m_aDropdownMenuList[j] == aText )
                            {
                                std::vector< ::rtl::OUString >::iterator aIter = m_aDropdownMenuList.begin();
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
    vos::OGuard aGuard( Application::GetSolarMutex() );

    sal_uInt16 nItemId = pMenu->GetCurItemId();
    if ( nItemId > 0 && nItemId <= m_aDropdownMenuList.size() )
    {
        m_aCurrentSelection = m_aDropdownMenuList[nItemId-1];
        m_pToolbar->SetItemText( m_nID, m_aCurrentSelection );

        execute( 0 );
    }
    return 0;
}

} // namespace
