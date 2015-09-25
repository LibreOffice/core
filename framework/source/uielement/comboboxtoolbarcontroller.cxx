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

#include "uielement/comboboxtoolbarcontroller.hxx"

#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/status/ItemStatus.hpp>
#include <com/sun/star/frame/status/ItemState.hpp>
#include <com/sun/star/frame/status/Visibility.hpp>
#include <com/sun/star/frame/XControlNotificationListener.hpp>
#include <com/sun/star/util/Color.hpp>

#include <svtools/toolboxcontroller.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/combobox.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::frame::status;
using namespace ::com::sun::star::util;

namespace framework
{

// Wrapper class to notify controller about events from combobox.
// Unfortunaltly the events are notifed through virtual methods instead
// of Listeners.

class ComboBoxControl : public ComboBox
{
    public:
        ComboBoxControl( vcl::Window* pParent, WinBits nStyle, IComboBoxListener* pComboBoxListener );
        virtual ~ComboBoxControl();
        virtual void dispose() SAL_OVERRIDE;

        virtual void Select() SAL_OVERRIDE;
        virtual void DoubleClick() SAL_OVERRIDE;
        virtual void Modify() SAL_OVERRIDE;
        virtual void KeyInput( const ::KeyEvent& rKEvt ) SAL_OVERRIDE;
        virtual void GetFocus() SAL_OVERRIDE;
        virtual void LoseFocus() SAL_OVERRIDE;
        virtual bool PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;

    private:
        IComboBoxListener* m_pComboBoxListener;
};

ComboBoxControl::ComboBoxControl( vcl::Window* pParent, WinBits nStyle, IComboBoxListener* pComboBoxListener ) :
    ComboBox( pParent, nStyle )
    , m_pComboBoxListener( pComboBoxListener )
{
}

ComboBoxControl::~ComboBoxControl()
{
    disposeOnce();
}

void ComboBoxControl::dispose()
{
    m_pComboBoxListener = 0;
    ComboBox::dispose();
}

void ComboBoxControl::Select()
{
    ComboBox::Select();
    if ( m_pComboBoxListener )
        m_pComboBoxListener->Select();
}

void ComboBoxControl::DoubleClick()
{
    ComboBox::DoubleClick();
    if ( m_pComboBoxListener )
        m_pComboBoxListener->DoubleClick();
}

void ComboBoxControl::Modify()
{
    ComboBox::Modify();
    if ( m_pComboBoxListener )
        m_pComboBoxListener->Modify();
}

void ComboBoxControl::KeyInput( const ::KeyEvent& rKEvt )
{
    ComboBox::KeyInput( rKEvt );
    if ( m_pComboBoxListener )
        m_pComboBoxListener->KeyInput( rKEvt );
}

void ComboBoxControl::GetFocus()
{
    ComboBox::GetFocus();
    if ( m_pComboBoxListener )
        m_pComboBoxListener->GetFocus();
}

void ComboBoxControl::LoseFocus()
{
    ComboBox::LoseFocus();
    if ( m_pComboBoxListener )
        m_pComboBoxListener->LoseFocus();
}

bool ComboBoxControl::PreNotify( NotifyEvent& rNEvt )
{
    bool bRet = false;
    if ( m_pComboBoxListener )
        bRet = m_pComboBoxListener->PreNotify( rNEvt );
    if ( !bRet )
        bRet = ComboBox::PreNotify( rNEvt );

    return bRet;
}

ComboboxToolbarController::ComboboxToolbarController(
    const Reference< XComponentContext >& rxContext,
    const Reference< XFrame >&            rFrame,
    ToolBox*                              pToolbar,
    sal_uInt16                            nID,
    sal_Int32                             nWidth,
    const OUString&                       aCommand ) :
    ComplexToolbarController( rxContext, rFrame, pToolbar, nID, aCommand )
    ,   m_pComboBox( 0 )
{
    m_pComboBox = VclPtr<ComboBoxControl>::Create( m_pToolbar, WB_DROPDOWN, this );
    if ( nWidth == 0 )
        nWidth = 100;

    // default dropdown size
    ::Size aLogicalSize( 8, 160 );
    ::Size aPixelSize = m_pComboBox->LogicToPixel( aLogicalSize, MAP_APPFONT );

    m_pComboBox->SetSizePixel( ::Size( nWidth, aPixelSize.Height() ));
    m_pToolbar->SetItemWindow( m_nID, m_pComboBox );
}

ComboboxToolbarController::~ComboboxToolbarController()
{
}

void SAL_CALL ComboboxToolbarController::dispose()
throw ( RuntimeException, std::exception )
{
    SolarMutexGuard aSolarMutexGuard;

    m_pToolbar->SetItemWindow( m_nID, 0 );
    m_pComboBox.disposeAndClear();

    ComplexToolbarController::dispose();
}

Sequence<PropertyValue> ComboboxToolbarController::getExecuteArgs(sal_Int16 KeyModifier) const
{
    Sequence<PropertyValue> aArgs( 2 );
    OUString aSelectedText = m_pComboBox->GetText();

    // Add key modifier to argument list
    aArgs[0].Name = "KeyModifier";
    aArgs[0].Value <<= KeyModifier;
    aArgs[1].Name = "Text";
    aArgs[1].Value <<= aSelectedText;
    return aArgs;
}

void ComboboxToolbarController::Select()
{
    if ( m_pComboBox->GetEntryCount() > 0 )
    {
        vcl::Window::PointerState aState = m_pComboBox->GetPointerState();

        sal_uInt16 nKeyModifier = sal_uInt16( aState.mnState & KEY_MODIFIERS_MASK );
        execute( nKeyModifier );
    }
}

void ComboboxToolbarController::DoubleClick()
{
}

void ComboboxToolbarController::Modify()
{
    notifyTextChanged( m_pComboBox->GetText() );
}

void ComboboxToolbarController::KeyInput( const ::KeyEvent& )
{
}

void ComboboxToolbarController::GetFocus()
{
    notifyFocusGet();
}

void ComboboxToolbarController::LoseFocus()
{
    notifyFocusLost();
}

bool ComboboxToolbarController::PreNotify( NotifyEvent& rNEvt )
{
    switch ( rNEvt.GetType() )
    {
        case MouseNotifyEvent::KEYINPUT :
            {
                const ::KeyEvent* pKeyEvent = rNEvt.GetKeyEvent();
                const vcl::KeyCode& rKeyCode = pKeyEvent->GetKeyCode();
                if(( rKeyCode.GetModifier() | rKeyCode.GetCode()) == KEY_RETURN )
                {
                    // Call execute only with non-empty text
                    if ( !m_pComboBox->GetText().isEmpty() )
                        execute( rKeyCode.GetModifier() );
                    return true;
                }
            }
            break;
        case MouseNotifyEvent::GETFOCUS :
            notifyFocusGet();
            break;
        case MouseNotifyEvent::LOSEFOCUS :
            notifyFocusLost();
            break;
        default :
            break;
    }
    return false;
}

void ComboboxToolbarController::executeControlCommand( const ::com::sun::star::frame::ControlCommand& rControlCommand )
{
    if ( rControlCommand.Command == "SetText" )
    {
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            if ( rControlCommand.Arguments[i].Name == "Text" )
            {
                OUString aText;
                rControlCommand.Arguments[i].Value >>= aText;
                m_pComboBox->SetText( aText );

                // send notification
                notifyTextChanged( aText );
                break;
            }
        }
    }
    else if ( rControlCommand.Command == "SetList" )
    {
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            if ( rControlCommand.Arguments[i].Name == "List" )
            {
                Sequence< OUString > aList;
                m_pComboBox->Clear();

                rControlCommand.Arguments[i].Value >>= aList;
                for ( sal_Int32 j = 0; j < aList.getLength(); j++ )
                    m_pComboBox->InsertEntry( aList[j] );

                // send notification
                uno::Sequence< beans::NamedValue > aInfo( 1 );
                aInfo[0].Name  = "List";
                aInfo[0].Value <<= aList;
                addNotifyInfo( OUString( "ListChanged" ),
                               getDispatchFromCommand( m_aCommandURL ),
                               aInfo );

                break;
            }
        }
    }
    else if ( rControlCommand.Command == "AddEntry" )
    {
        sal_Int32      nPos( COMBOBOX_APPEND );
        OUString   aText;
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            if ( rControlCommand.Arguments[i].Name == "Text" )
            {
                if ( rControlCommand.Arguments[i].Value >>= aText )
                    m_pComboBox->InsertEntry( aText, nPos );
                break;
            }
        }
    }
    else if ( rControlCommand.Command == "InsertEntry" )
    {
        sal_Int32      nPos( COMBOBOX_APPEND );
        OUString   aText;
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            if ( rControlCommand.Arguments[i].Name == "Pos" )
            {
                sal_Int32 nTmpPos = 0;
                if ( rControlCommand.Arguments[i].Value >>= nTmpPos )
                {
                    if (( nTmpPos >= 0 ) &&
                        ( nTmpPos < sal_Int32( m_pComboBox->GetEntryCount() )))
                        nPos = nTmpPos;
                }
            }
            else if ( rControlCommand.Arguments[i].Name == "Text" )
                rControlCommand.Arguments[i].Value >>= aText;
        }

        m_pComboBox->InsertEntry( aText, nPos );
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
                    if ( 0 <= nPos && nPos < sal_Int32( m_pComboBox->GetEntryCount() ))
                        m_pComboBox->RemoveEntryAt(nPos);
                }
                break;
            }
        }
    }
    else if ( rControlCommand.Command == "RemoveEntryText" )
    {
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            if ( rControlCommand.Arguments[i].Name == "Text")
            {
                OUString aText;
                if ( rControlCommand.Arguments[i].Value >>= aText )
                    m_pComboBox->RemoveEntry( aText );
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
                m_pComboBox->SetDropDownLineCount( sal_uInt16( nValue ));
                break;
            }
        }
    }
    else if ( rControlCommand.Command == "SetBackgroundColor" )
    {
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            if ( rControlCommand.Arguments[i].Name == "Color" )
            {
                com::sun::star::util::Color aColor(0);
                if ( rControlCommand.Arguments[i].Value >>= aColor )
                {
                    ::Color aBackColor( static_cast< sal_uInt32 >( aColor ));
                    m_pComboBox->SetControlBackground( aBackColor );
                }
                break;
            }
        }
    }
    else if ( rControlCommand.Command == "SetTextColor" )
    {
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            if ( rControlCommand.Arguments[i].Name == "Color" )
            {
                com::sun::star::util::Color aColor(0);
                if ( rControlCommand.Arguments[i].Value >>= aColor )
                {
                    ::Color aForeColor( static_cast< sal_uInt32 >( aColor ));
                    m_pComboBox->SetControlForeground( aForeColor );
                }
                break;
            }
        }
    }
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
