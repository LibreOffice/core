/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stdctrl.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-26 10:31:43 $
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

#include <stdctrl.hxx>

// =======================================================================

FixedInfo::FixedInfo( Window* pParent, WinBits nWinStyle ) :
    FixedText( pParent, nWinStyle | WB_INFO )
{
}

// -----------------------------------------------------------------------

FixedInfo::FixedInfo( Window* pParent, const ResId& rResId ) :
    FixedText( pParent, rResId )
{
    SetStyle( GetStyle() | WB_INFO );
}

namespace svt
{
    // class svt::SelectableFixedText ----------------------------------------

    SelectableFixedText::SelectableFixedText( Window* pParent, WinBits nWinStyle ) :
        Edit( pParent, nWinStyle )
    {
        Init();
    }

    // -----------------------------------------------------------------------

    SelectableFixedText::SelectableFixedText( Window* pParent, const ResId& rResId ) :
        Edit( pParent, rResId )
    {
        Init();
    }

    // -----------------------------------------------------------------------

    SelectableFixedText::~SelectableFixedText()
    {
    }

    // -----------------------------------------------------------------------

    void SelectableFixedText::Init()
    {
        // no tabstop
        SetStyle( ( GetStyle() & ~WB_TABSTOP ) | WB_NOTABSTOP );
        // no border
        SetBorderStyle( WINDOW_BORDER_NOBORDER );
        // read-only
        SetReadOnly();
        // make it transparent
        SetControlBackground();
        SetBackground();
        SetPaintTransparent( TRUE );
    }

    // -----------------------------------------------------------------------

    void SelectableFixedText::LoseFocus()
    {
        Edit::LoseFocus();
        // clear cursor
        Invalidate();
    }

} // namespace svt

