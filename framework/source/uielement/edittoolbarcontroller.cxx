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

#include <uielement/edittoolbarcontroller.hxx>

#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <svtools/toolboxcontroller.hxx>
#include <vcl/svapp.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/event.hxx>

using namespace ::com::sun::star;
using namespace css::uno;
using namespace css::beans;
using namespace css::lang;
using namespace css::frame;
using namespace css::util;

namespace framework
{

// Wrapper class to notify controller about events from edit.
// Unfortunaltly the events are notified through virtual methods instead
// of Listeners.

class EditControl : public Edit
{
    public:
        EditControl( vcl::Window* pParent, WinBits nStyle, EditToolbarController* pEditToolbarController );
        virtual ~EditControl() override;
        virtual void dispose() override;

        virtual void Modify() override;
        virtual void GetFocus() override;
        virtual void LoseFocus() override;
        virtual bool PreNotify( NotifyEvent& rNEvt ) override;

    private:
        EditToolbarController* m_pEditToolbarController;
};

EditControl::EditControl( vcl::Window* pParent, WinBits nStyle, EditToolbarController* pEditToolbarController ) :
    Edit( pParent, nStyle )
    , m_pEditToolbarController( pEditToolbarController )
{
}

EditControl::~EditControl()
{
    disposeOnce();
}

void EditControl::dispose()
{
    m_pEditToolbarController = nullptr;
    Edit::dispose();
}

void EditControl::Modify()
{
    Edit::Modify();
    if ( m_pEditToolbarController )
        m_pEditToolbarController->Modify();
}

void EditControl::GetFocus()
{
    Edit::GetFocus();
    if ( m_pEditToolbarController )
        m_pEditToolbarController->GetFocus();
}

void EditControl::LoseFocus()
{
    Edit::LoseFocus();
    if ( m_pEditToolbarController )
        m_pEditToolbarController->LoseFocus();
}

bool EditControl::PreNotify( NotifyEvent& rNEvt )
{
    bool bRet = false;
    if ( m_pEditToolbarController )
        bRet = m_pEditToolbarController->PreNotify( rNEvt );
    if ( !bRet )
        bRet = Edit::PreNotify( rNEvt );

    return bRet;
}

EditToolbarController::EditToolbarController(
    const Reference< XComponentContext >&    rxContext,
    const Reference< XFrame >&               rFrame,
    ToolBox*                                 pToolbar,
    sal_uInt16                                   nID,
    sal_Int32                                nWidth,
    const OUString&                          aCommand ) :
    ComplexToolbarController( rxContext, rFrame, pToolbar, nID, aCommand )
    ,   m_pEditControl( nullptr )
{
    m_pEditControl = VclPtr<EditControl>::Create( m_pToolbar, WB_BORDER, this );
    if ( nWidth == 0 )
        nWidth = 100;

    // Calculate height of the edit field according to the application font height
    sal_Int32 nHeight = getFontSizePixel( m_pEditControl ) + 6 + 1;

    m_pEditControl->SetSizePixel( ::Size( nWidth, nHeight ));
    m_pToolbar->SetItemWindow( m_nID, m_pEditControl );
}

EditToolbarController::~EditToolbarController()
{
}

void SAL_CALL EditToolbarController::dispose()
{
    SolarMutexGuard aSolarMutexGuard;

    m_pToolbar->SetItemWindow( m_nID, nullptr );
    m_pEditControl.disposeAndClear();

    ComplexToolbarController::dispose();
}

Sequence<PropertyValue> EditToolbarController::getExecuteArgs(sal_Int16 KeyModifier) const
{
    Sequence<PropertyValue> aArgs( 2 );
    OUString aSelectedText = m_pEditControl->GetText();

    // Add key modifier to argument list
    aArgs[0].Name = "KeyModifier";
    aArgs[0].Value <<= KeyModifier;
    aArgs[1].Name = "Text";
    aArgs[1].Value <<= aSelectedText;
    return aArgs;
}

void EditToolbarController::Modify()
{
    notifyTextChanged( m_pEditControl->GetText() );
}

void EditToolbarController::GetFocus()
{
    notifyFocusGet();
}

void EditToolbarController::LoseFocus()
{
    notifyFocusLost();
}

bool EditToolbarController::PreNotify( NotifyEvent const & rNEvt )
{
    if( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        const ::KeyEvent* pKeyEvent = rNEvt.GetKeyEvent();
        const vcl::KeyCode& rKeyCode = pKeyEvent->GetKeyCode();
        if(( rKeyCode.GetModifier() | rKeyCode.GetCode()) == KEY_RETURN )
        {
            // Call execute only with non-empty text
            if ( !m_pEditControl->GetText().isEmpty() )
                execute( rKeyCode.GetModifier() );
            return true;
        }
    }

    return false;
}

void EditToolbarController::executeControlCommand( const css::frame::ControlCommand& rControlCommand )
{
    if ( rControlCommand.Command.startsWith( "SetText" ))
    {
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            if ( rControlCommand.Arguments[i].Name.startsWith( "Text" ))
            {
                OUString aText;
                rControlCommand.Arguments[i].Value >>= aText;
                m_pEditControl->SetText( aText );

                // send notification
                notifyTextChanged( aText );
                break;
            }
        }
    }
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
