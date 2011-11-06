/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include <svtools/fixedhyper.hxx>

//.........................................................................
namespace svt
{
//.........................................................................

// class FixedHyperlink --------------------------------------------------

FixedHyperlink::FixedHyperlink( Window* pParent, const ResId& rResId ) :
    ::toolkit::FixedHyperlinkBase( pParent, rResId ),
    m_nTextLen(0)
{
    Initialize();
}

FixedHyperlink::FixedHyperlink( Window* pParent, WinBits nWinStyle  ) :
    ::toolkit::FixedHyperlinkBase( pParent, nWinStyle ),
    m_nTextLen(0)
{
    Initialize();
}

FixedHyperlink::~FixedHyperlink()
{
}

void FixedHyperlink::Initialize()
{
    // saves the old pointer
    m_aOldPointer = GetPointer();
    // changes the font
    Font aFont = GetControlFont( );
    // to underline
    aFont.SetUnderline( UNDERLINE_SINGLE );
    SetControlFont( aFont );
    // changes the color to light blue
    SetTextColor( Color( COL_LIGHTBLUE ) );
    // calculates text len
    m_nTextLen = GetCtrlTextWidth( GetText() );
}

void FixedHyperlink::MouseMove( const MouseEvent& rMEvt )
{
    // changes the pointer if the control is enabled and the mouse is over the text.
    if ( !rMEvt.IsLeaveWindow() && IsEnabled() && GetPointerPosPixel().X() < m_nTextLen )
        SetPointer( POINTER_REFHAND );
    else
        SetPointer( m_aOldPointer );
}

void FixedHyperlink::MouseButtonUp( const MouseEvent& )
{
    // calls the link if the control is enabled and the mouse is over the text.
    if ( IsEnabled() && GetPointerPosPixel().X() < m_nTextLen )
        ImplCallEventListenersAndHandler( VCLEVENT_BUTTON_CLICK, m_aClickHdl, this );
}

void FixedHyperlink::RequestHelp( const HelpEvent& rHEvt )
{
    if ( IsEnabled() && GetPointerPosPixel().X() < m_nTextLen )
        FixedText::RequestHelp( rHEvt );
}

void FixedHyperlink::GetFocus()
{
    SetTextColor( Color( COL_LIGHTRED ) );
    Paint( Rectangle( Point(), GetSizePixel() ) );
    ShowFocus( Rectangle( Point( 1, 1 ), Size( m_nTextLen + 4, GetSizePixel().Height() - 2 ) ) );
}

void FixedHyperlink::LoseFocus()
{
    SetTextColor( Color( COL_LIGHTBLUE ) );
    Paint( Rectangle( Point(), GetSizePixel() ) );
    HideFocus();
}

void FixedHyperlink::KeyInput( const KeyEvent& rKEvt )
{
    switch ( rKEvt.GetKeyCode().GetCode() )
    {
        case KEY_SPACE:
        case KEY_RETURN:
            m_aClickHdl.Call( this );
            break;

        default:
            FixedText::KeyInput( rKEvt );
    }
}

void FixedHyperlink::SetURL( const String& rNewURL )
{
    m_sURL = rNewURL;
    SetQuickHelpText( m_sURL );
}

String  FixedHyperlink::GetURL() const
{
    return m_sURL;
}

void FixedHyperlink::SetDescription( const String& rNewDescription )
{
    SetText( rNewDescription );
    m_nTextLen = GetCtrlTextWidth( GetText() );
}

// class FixedHyperlinkImage ---------------------------------------------

FixedHyperlinkImage::FixedHyperlinkImage( Window* pParent, const ResId& rResId ) :
    FixedImage( pParent, rResId )
{
    Initialize();
}

FixedHyperlinkImage::FixedHyperlinkImage( Window* pParent, WinBits nWinStyle  ) :
    FixedImage( pParent, nWinStyle )
{
    Initialize();
}

FixedHyperlinkImage::~FixedHyperlinkImage()
{
}

void FixedHyperlinkImage::Initialize()
{
    // saves the old pointer
    m_aOldPointer = GetPointer();
}

void FixedHyperlinkImage::MouseMove( const MouseEvent& rMEvt )
{
    // changes the pointer if the control is enabled and the mouse is over the text.
    if ( !rMEvt.IsLeaveWindow() && IsEnabled() )
        SetPointer( POINTER_REFHAND );
    else
        SetPointer( m_aOldPointer );
}

void FixedHyperlinkImage::MouseButtonUp( const MouseEvent& )
{
    // calls the link if the control is enabled and the mouse is over the text.
    if ( IsEnabled() )
        ImplCallEventListenersAndHandler( VCLEVENT_BUTTON_CLICK, m_aClickHdl, this );

    Size aSize = GetSizePixel();
    Size aImgSz = GetImage().GetSizePixel();
    if ( aSize.Width() < aImgSz.Width() )
    {
        DBG_ERRORFILE("xxx");
    }
}

void FixedHyperlinkImage::RequestHelp( const HelpEvent& rHEvt )
{
    if ( IsEnabled() )
        FixedImage::RequestHelp( rHEvt );
}

void FixedHyperlinkImage::GetFocus()
{
    Paint( Rectangle( Point(), GetSizePixel() ) );
    ShowFocus( Rectangle( Point( 1, 1 ), Size( GetSizePixel().Width() - 2, GetSizePixel().Height() - 2 ) ) );
}

void FixedHyperlinkImage::LoseFocus()
{
    Paint( Rectangle( Point(), GetSizePixel() ) );
    HideFocus();
}

void FixedHyperlinkImage::KeyInput( const KeyEvent& rKEvt )
{
    switch ( rKEvt.GetKeyCode().GetCode() )
    {
        case KEY_SPACE:
        case KEY_RETURN:
            m_aClickHdl.Call( this );
            break;

        default:
            FixedImage::KeyInput( rKEvt );
    }
}

void FixedHyperlinkImage::SetURL( const String& rNewURL )
{
    m_sURL = rNewURL;
    SetQuickHelpText( m_sURL );
}

String  FixedHyperlinkImage::GetURL() const
{
    return m_sURL;
}

//.........................................................................
} // namespace svt
//.........................................................................

