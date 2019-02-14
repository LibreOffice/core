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

#include <vcl/event.hxx>
#include <vcl/fixedhyper.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <vcl/ptrstyle.hxx>
#include <comphelper/anytostring.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <com/sun/star/system/XSystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>

using namespace css;

FixedHyperlink::FixedHyperlink(vcl::Window* pParent, WinBits nWinStyle)
    : FixedText(pParent, nWinStyle)
    , m_nTextLen(0)
    , m_aOldPointer(PointerStyle::Arrow)
{
    Initialize();
}

void FixedHyperlink::Initialize()
{
    // saves the old pointer
    m_aOldPointer = GetPointer();
    // changes the font
    vcl::Font aFont = GetControlFont( );
    // to underline
    aFont.SetUnderline( LINESTYLE_SINGLE );
    SetControlFont( aFont );
    // changes the color to link color
    SetControlForeground( Application::GetSettings().GetStyleSettings().GetLinkColor() );
    // calculates text len
    m_nTextLen = GetCtrlTextWidth( GetText() );

    SetClickHdl(LINK(this, FixedHyperlink, HandleClick));
}

bool FixedHyperlink::ImplIsOverText(Point aPosition)
{
    Size aSize = GetOutputSizePixel();

    bool bIsOver = false;

    if (GetStyle() & WB_RIGHT)
    {
        return aPosition.X() > (aSize.Width() - m_nTextLen);
    }
    else if (GetStyle() & WB_CENTER)
    {
        bIsOver = aPosition.X() > (aSize.Width() / 2 - m_nTextLen / 2) &&
                  aPosition.X() < (aSize.Width() / 2 + m_nTextLen / 2);
    }
    else
    {
        bIsOver = aPosition.X() < m_nTextLen;
    }

    return bIsOver;
}

void FixedHyperlink::MouseMove( const MouseEvent& rMEvt )
{
    // changes the pointer if the control is enabled and the mouse is over the text.
    if ( !rMEvt.IsLeaveWindow() && IsEnabled() && ImplIsOverText(GetPointerPosPixel()) )
        SetPointer( PointerStyle::RefHand );
    else
        SetPointer( m_aOldPointer );
}

void FixedHyperlink::MouseButtonUp( const MouseEvent& )
{
    // calls the link if the control is enabled and the mouse is over the text.
    if ( IsEnabled() && ImplIsOverText(GetPointerPosPixel()) )
        ImplCallEventListenersAndHandler( VclEventId::ButtonClick, [this] () { m_aClickHdl.Call(*this); } );
}

void FixedHyperlink::RequestHelp( const HelpEvent& rHEvt )
{
    if ( IsEnabled() && ImplIsOverText(GetPointerPosPixel()) )
        FixedText::RequestHelp( rHEvt );
}

void FixedHyperlink::GetFocus()
{
    SetTextColor( COL_LIGHTRED );
    Invalidate(tools::Rectangle(Point(), GetSizePixel()));
    ShowFocus( tools::Rectangle( Point( 1, 1 ), Size( m_nTextLen + 4, GetSizePixel().Height() - 2 ) ) );
}

void FixedHyperlink::LoseFocus()
{
    SetTextColor( GetControlForeground() );
    Invalidate(tools::Rectangle(Point(), GetSizePixel()));
    HideFocus();
}

void FixedHyperlink::KeyInput( const KeyEvent& rKEvt )
{
    switch ( rKEvt.GetKeyCode().GetCode() )
    {
        case KEY_SPACE:
        case KEY_RETURN:
            m_aClickHdl.Call( *this );
            break;

        default:
            FixedText::KeyInput( rKEvt );
    }
}

void FixedHyperlink::SetURL( const OUString& rNewURL )
{
    m_sURL = rNewURL;
    SetQuickHelpText( m_sURL );
}


void FixedHyperlink::SetText(const OUString& rNewDescription)
{
    FixedText::SetText(rNewDescription);
    m_nTextLen = GetCtrlTextWidth(GetText());
}

bool FixedHyperlink::set_property(const OString &rKey, const OUString &rValue)
{
    if (rKey == "uri")
        SetURL(rValue);
    else
        return FixedText::set_property(rKey, rValue);
    return true;
}

IMPL_LINK(FixedHyperlink, HandleClick, FixedHyperlink&, rHyperlink, void)
{
    if ( rHyperlink.m_sURL.isEmpty() ) // Nothing to do, when the URL is empty
        return;

    try
    {
        uno::Reference< system::XSystemShellExecute > xSystemShellExecute(
            system::SystemShellExecute::create(comphelper::getProcessComponentContext()));
        //throws css::lang::IllegalArgumentException, css::system::SystemShellExecuteException
        xSystemShellExecute->execute( rHyperlink.m_sURL, OUString(), system::SystemShellExecuteFlags::URIS_ONLY );
    }
    catch ( const uno::Exception& )
    {
        uno::Any exc(cppu::getCaughtException());
        OUString msg(comphelper::anyToString(exc));
        SolarMutexGuard g;
        std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(GetFrameWeld(), VclMessageType::Error, VclButtonsType::Ok, msg));
        xErrorBox->set_title(rHyperlink.GetText());
        xErrorBox->run();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
