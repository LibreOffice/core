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

#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <svtools/toolboxcontroller.hxx>
#include <vcl/svapp.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/toolbox.hxx>

using namespace ::com::sun::star;
using namespace css::awt;
using namespace css::uno;
using namespace css::beans;
using namespace css::lang;
using namespace css::frame;
using namespace css::util;

namespace framework
{

// Wrapper class to notify controller about events from ListBox.
// Unfortunaltly the events are notified through virtual methods instead
// of Listeners.

class ListBoxControl : public ListBox
{
    public:
        ListBoxControl( vcl::Window* pParent, WinBits nStyle, DropdownToolbarController* pListBoxListener );
        virtual ~ListBoxControl() override;
        virtual void dispose() override;

        virtual void Select() override;
        virtual void GetFocus() override;
        virtual void LoseFocus() override;
        virtual bool PreNotify( NotifyEvent& rNEvt ) override;

    private:
        DropdownToolbarController* m_pListBoxListener;
};

ListBoxControl::ListBoxControl( vcl::Window* pParent, WinBits nStyle, DropdownToolbarController* pListBoxListener ) :
    ListBox( pParent, nStyle )
    , m_pListBoxListener( pListBoxListener )
{
}

ListBoxControl::~ListBoxControl()
{
    disposeOnce();
}

void ListBoxControl::dispose()
{
    m_pListBoxListener = nullptr;
    ListBox::dispose();
}

void ListBoxControl::Select()
{
    ListBox::Select();
    if ( m_pListBoxListener )
        m_pListBoxListener->Select();
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

bool ListBoxControl::PreNotify( NotifyEvent& rNEvt )
{
    bool bRet = false;
    if ( m_pListBoxListener )
        bRet = false;
    if ( !bRet )
        bRet = ListBox::PreNotify( rNEvt );

    return bRet;
}

DropdownToolbarController::DropdownToolbarController(
    const Reference< XComponentContext >&    rxContext,
    const Reference< XFrame >&               rFrame,
    ToolBox*                                 pToolbar,
    sal_uInt16                                   nID,
    sal_Int32                                nWidth,
    const OUString&                          aCommand ) :
    ComplexToolbarController( rxContext, rFrame, pToolbar, nID, aCommand )
    ,   m_pListBoxControl( nullptr )
{
    m_pListBoxControl = VclPtr<ListBoxControl>::Create( m_pToolbar, WB_DROPDOWN|WB_AUTOHSCROLL|WB_BORDER, this );
    if ( nWidth == 0 )
        nWidth = 100;

    // default dropdown size
    ::Size aLogicalSize( 0, 160 );
    ::Size aPixelSize = m_pListBoxControl->LogicToPixel( aLogicalSize, MapUnit::MapAppFont );

    m_pListBoxControl->SetSizePixel( ::Size( nWidth, aPixelSize.Height() ));
    m_pToolbar->SetItemWindow( m_nID, m_pListBoxControl );
    m_pListBoxControl->SetDropDownLineCount( 5 );
}

DropdownToolbarController::~DropdownToolbarController()
{
}

void SAL_CALL DropdownToolbarController::dispose()
{
    SolarMutexGuard aSolarMutexGuard;

    m_pToolbar->SetItemWindow( m_nID, nullptr );
    m_pListBoxControl.disposeAndClear();

    ComplexToolbarController::dispose();
}

Sequence<PropertyValue> DropdownToolbarController::getExecuteArgs(sal_Int16 KeyModifier) const
{
    Sequence<PropertyValue> aArgs( 2 );
    OUString aSelectedText = m_pListBoxControl->GetSelectEntry();

    // Add key modifier to argument list
    aArgs[0].Name = "KeyModifier";
    aArgs[0].Value <<= KeyModifier;
    aArgs[1].Name = "Text";
    aArgs[1].Value <<= aSelectedText;
    return aArgs;
}

void DropdownToolbarController::Select()
{
    if ( m_pListBoxControl->GetEntryCount() > 0 )
    {
        vcl::Window::PointerState aState = m_pListBoxControl->GetPointerState();

        sal_uInt16 nKeyModifier = sal_uInt16( aState.mnState & KEY_MODIFIERS_MASK );
        execute( nKeyModifier );
    }
}

void DropdownToolbarController::GetFocus()
{
    notifyFocusGet();
}

void DropdownToolbarController::LoseFocus()
{
    notifyFocusLost();
}

void DropdownToolbarController::executeControlCommand( const css::frame::ControlCommand& rControlCommand )
{
    if ( rControlCommand.Command == "SetList" )
    {
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            if ( rControlCommand.Arguments[i].Name == "List" )
            {
                Sequence< OUString > aList;
                m_pListBoxControl->Clear();

                rControlCommand.Arguments[i].Value >>= aList;
                for ( sal_Int32 j = 0; j < aList.getLength(); j++ )
                    m_pListBoxControl->InsertEntry( aList[j] );

                m_pListBoxControl->SelectEntryPos( 0 );

                // send notification
                uno::Sequence< beans::NamedValue > aInfo { { "List", css::uno::makeAny(aList) } };
                addNotifyInfo( "ListChanged",
                               getDispatchFromCommand( m_aCommandURL ),
                               aInfo );

                break;
            }
        }
    }
    else if ( rControlCommand.Command == "AddEntry" )
    {
        OUString   aText;
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            if ( rControlCommand.Arguments[i].Name == "Text" )
            {
                if ( rControlCommand.Arguments[i].Value >>= aText )
                    m_pListBoxControl->InsertEntry( aText, LISTBOX_APPEND );
                break;
            }
        }
    }
    else if ( rControlCommand.Command == "InsertEntry" )
    {
        sal_Int32      nPos( LISTBOX_APPEND );
        OUString   aText;
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            if ( rControlCommand.Arguments[i].Name == "Pos" )
            {
                sal_Int32 nTmpPos = 0;
                if ( rControlCommand.Arguments[i].Value >>= nTmpPos )
                {
                    if (( nTmpPos >= 0 ) &&
                        ( nTmpPos < m_pListBoxControl->GetEntryCount() ))
                        nPos = nTmpPos;
                }
            }
            else if ( rControlCommand.Arguments[i].Name == "Text" )
                rControlCommand.Arguments[i].Value >>= aText;
        }

        m_pListBoxControl->InsertEntry( aText, nPos );
    }
    else if ( rControlCommand.Command == "RemoveEntryPos" )
    {
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            if ( rControlCommand.Arguments[i].Name == "Pos" )
            {
                sal_Int32 nPos( -1 );
                if ( rControlCommand.Arguments[i].Value >>= nPos )
                {
                    if ( 0 <= nPos && nPos < m_pListBoxControl->GetEntryCount() )
                        m_pListBoxControl->RemoveEntry( nPos );
                }
                break;
            }
        }
    }
    else if ( rControlCommand.Command == "RemoveEntryText" )
    {
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            if ( rControlCommand.Arguments[i].Name == "Text" )
            {
                OUString aText;
                if ( rControlCommand.Arguments[i].Value >>= aText )
                    m_pListBoxControl->RemoveEntry( aText );
                break;
            }
        }
    }
    else if ( rControlCommand.Command == "SetDropDownLines" )
    {
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            if ( rControlCommand.Arguments[i].Name == "Lines" )
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
