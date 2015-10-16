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

#include <vcl/fixedhyper.hxx>

FixedHyperlink::FixedHyperlink(vcl::Window* pParent, WinBits nWinStyle)
    : FixedText(pParent, nWinStyle)
    , m_nTextLen(0)
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
    aFont.SetUnderline( UNDERLINE_SINGLE );
    SetControlFont( aFont );
    // changes the color to light blue
    SetControlForeground( Color( COL_LIGHTBLUE ) );
    // calculates text len
    m_nTextLen = GetCtrlTextWidth( GetText() );
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
        ImplCallEventListenersAndHandler( VCLEVENT_BUTTON_CLICK, [this] () { m_aClickHdl.Call(*this); } );
}

void FixedHyperlink::RequestHelp( const HelpEvent& rHEvt )
{
    if ( IsEnabled() && ImplIsOverText(GetPointerPosPixel()) )
        FixedText::RequestHelp( rHEvt );
}

void FixedHyperlink::GetFocus()
{
    SetTextColor( Color( COL_LIGHTRED ) );
    Invalidate(Rectangle(Point(), GetSizePixel()));
    ShowFocus( Rectangle( Point( 1, 1 ), Size( m_nTextLen + 4, GetSizePixel().Height() - 2 ) ) );
}

void FixedHyperlink::LoseFocus()
{
    SetTextColor( GetControlForeground() );
    Invalidate(Rectangle(Point(), GetSizePixel()));
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

bool FixedHyperlink::set_property(const OString &rKey, const OString &rValue)
{
    if (rKey == "uri")
        SetURL(OStringToOUString(rValue, RTL_TEXTENCODING_UTF8));
    else
        return FixedText::set_property(rKey, rValue);
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
