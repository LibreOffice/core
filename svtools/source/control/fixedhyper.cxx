/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fixedhyper.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:24:57 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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

//.........................................................................
} // namespace svt
//.........................................................................

