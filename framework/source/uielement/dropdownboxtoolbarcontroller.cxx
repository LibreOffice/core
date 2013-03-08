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


#include "uielement/dropdownboxtoolbarcontroller.hxx"

#include "uielement/toolbar.hxx"

#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/status/ItemStatus.hpp>
#include <com/sun/star/frame/status/ItemState.hpp>
#include <com/sun/star/frame/status/Visibility.hpp>
#include <com/sun/star/frame/XControlNotificationListener.hpp>

#include <svtools/toolboxcontroller.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/mnemonic.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::frame::status;
using namespace ::com::sun::star::util;

namespace framework
{

// ------------------------------------------------------------------

// Wrapper class to notify controller about events from ListBox.
// Unfortunaltly the events are notifed through virtual methods instead
// of Listeners.

class ListBoxControl : public ListBox
{
    public:
        ListBoxControl( Window* pParent, WinBits nStyle, IListBoxListener* pListBoxListener );
        virtual ~ListBoxControl();

        virtual void Select();
        virtual void DoubleClick();
        virtual void GetFocus();
        virtual void LoseFocus();
        virtual long PreNotify( NotifyEvent& rNEvt );

    private:
        IListBoxListener* m_pListBoxListener;
};

ListBoxControl::ListBoxControl( Window* pParent, WinBits nStyle, IListBoxListener* pListBoxListener ) :
    ListBox( pParent, nStyle )
    , m_pListBoxListener( pListBoxListener )
{
}

ListBoxControl::~ListBoxControl()
{
    m_pListBoxListener = 0;
}

void ListBoxControl::Select()
{
    ListBox::Select();
    if ( m_pListBoxListener )
        m_pListBoxListener->Select();
}

void ListBoxControl::DoubleClick()
{
    ListBox::DoubleClick();
    if ( m_pListBoxListener )
        m_pListBoxListener->DoubleClick();
}

void ListBoxControl::GetFocus()
{
    ListBox::GetFocus();
    if ( m_pListBoxListener )
        m_pListBoxListener->GetFocus();
}

void ListBoxControl::LoseFocus()
{
    ListBox::LoseFocus();
    if ( m_pListBoxListener )
        m_pListBoxListener->LoseFocus();
}

long ListBoxControl::PreNotify( NotifyEvent& rNEvt )
{
    long nRet( 0 );
    if ( m_pListBoxListener )
        nRet = m_pListBoxListener->PreNotify( rNEvt );
    if ( nRet == 0 )
        nRet = ListBox::PreNotify( rNEvt );

    return nRet;
}

// ------------------------------------------------------------------

DropdownToolbarController::DropdownToolbarController(
    const Reference< XMultiServiceFactory >& rServiceManager,
    const Reference< XFrame >&               rFrame,
    ToolBox*                                 pToolbar,
    sal_uInt16                                   nID,
    sal_Int32                                nWidth,
    const OUString&                          aCommand ) :
    ComplexToolbarController( rServiceManager, rFrame, pToolbar, nID, aCommand )
    ,   m_pListBoxControl( 0 )
{
    m_pListBoxControl = new ListBoxControl( m_pToolbar, WB_DROPDOWN|WB_AUTOHSCROLL|WB_BORDER, this );
    if ( nWidth == 0 )
        nWidth = 100;

    // default dropdown size
    ::Size aLogicalSize( 0, 160 );
    ::Size aPixelSize = m_pListBoxControl->LogicToPixel( aLogicalSize, MAP_APPFONT );

    m_pListBoxControl->SetSizePixel( ::Size( nWidth, aPixelSize.Height() ));
    m_pToolbar->SetItemWindow( m_nID, m_pListBoxControl );
    m_pListBoxControl->SetDropDownLineCount( 5 );
}

// ------------------------------------------------------------------

DropdownToolbarController::~DropdownToolbarController()
{
}

// ------------------------------------------------------------------

void SAL_CALL DropdownToolbarController::dispose()
throw ( RuntimeException )
{
    SolarMutexGuard aSolarMutexGuard;

    m_pToolbar->SetItemWindow( m_nID, 0 );
    delete m_pListBoxControl;

    ComplexToolbarController::dispose();

    m_pListBoxControl = 0;
}

// ------------------------------------------------------------------
Sequence<PropertyValue> DropdownToolbarController::getExecuteArgs(sal_Int16 KeyModifier) const
{
    Sequence<PropertyValue> aArgs( 2 );
    OUString aSelectedText = m_pListBoxControl->GetSelectEntry();

    // Add key modifier to argument list
    aArgs[0].Name = OUString( "KeyModifier" );
    aArgs[0].Value <<= KeyModifier;
    aArgs[1].Name = OUString( "Text" );
    aArgs[1].Value <<= aSelectedText;
    return aArgs;
}

// ------------------------------------------------------------------

void DropdownToolbarController::Select()
{
    if ( m_pListBoxControl->GetEntryCount() > 0 )
    {
        Window::PointerState aState = m_pListBoxControl->GetPointerState();

        sal_uInt16 nKeyModifier = sal_uInt16( aState.mnState & KEY_MODTYPE );
        execute( nKeyModifier );
    }
}

void DropdownToolbarController::DoubleClick()
{
}

void DropdownToolbarController::GetFocus()
{
    notifyFocusGet();
}

void DropdownToolbarController::LoseFocus()
{
    notifyFocusLost();
}

long DropdownToolbarController::PreNotify( NotifyEvent& /*rNEvt*/ )
{
    return 0;
}

// --------------------------------------------------------

void DropdownToolbarController::executeControlCommand( const ::com::sun::star::frame::ControlCommand& rControlCommand )
{
    if ( rControlCommand.Command.equalsAsciiL( "SetList", 7 ))
    {
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            if ( rControlCommand.Arguments[i].Name.equalsAsciiL( "List", 4 ))
            {
                Sequence< OUString > aList;
                m_pListBoxControl->Clear();

                rControlCommand.Arguments[i].Value >>= aList;
                for ( sal_Int32 j = 0; j < aList.getLength(); j++ )
                    m_pListBoxControl->InsertEntry( aList[j] );

                m_pListBoxControl->SelectEntryPos( 0 );

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
    else if ( rControlCommand.Command.equalsAsciiL( "AddEntry", 8 ))
    {
        sal_uInt16      nPos( LISTBOX_APPEND );
        OUString   aText;
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            if ( rControlCommand.Arguments[i].Name.equalsAsciiL( "Text", 4 ))
            {
                if ( rControlCommand.Arguments[i].Value >>= aText )
                    m_pListBoxControl->InsertEntry( aText, nPos );
                break;
            }
        }
    }
    else if ( rControlCommand.Command.equalsAsciiL( "InsertEntry", 11 ))
    {
        sal_uInt16      nPos( LISTBOX_APPEND );
        OUString   aText;
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            if ( rControlCommand.Arguments[i].Name.equalsAsciiL( "Pos", 3 ))
            {
                sal_Int32 nTmpPos = 0;
                if ( rControlCommand.Arguments[i].Value >>= nTmpPos )
                {
                    if (( nTmpPos >= 0 ) &&
                        ( nTmpPos < sal_Int32( m_pListBoxControl->GetEntryCount() )))
                        nPos = sal_uInt16( nTmpPos );
                }
            }
            else if ( rControlCommand.Arguments[i].Name.equalsAsciiL( "Text", 4 ))
                rControlCommand.Arguments[i].Value >>= aText;
        }

        m_pListBoxControl->InsertEntry( aText, nPos );
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
                    if ( nPos < sal_Int32( m_pListBoxControl->GetEntryCount() ))
                        m_pListBoxControl->RemoveEntry( sal_uInt16( nPos ));
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
                    m_pListBoxControl->RemoveEntry( aText );
                break;
            }
        }
    }
    else if ( rControlCommand.Command.equalsAsciiL( "SetDropDownLines", 16 ))
    {
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            if ( rControlCommand.Arguments[i].Name.equalsAsciiL( "Lines", 5 ))
            {
                sal_Int32 nValue( 5 );
                rControlCommand.Arguments[i].Value >>= nValue;
                m_pListBoxControl->SetDropDownLineCount( sal_uInt16( nValue ));
                break;
            }
        }
    }
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
