/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: descedit.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-22 15:24:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2006 by Sun Microsystems, Inc.
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
#include "precompiled_desktop.hxx"

#include <vcl/scrbar.hxx>
#include <svtools/txtattr.hxx>
#include <svtools/xtextedt.hxx>

#include "descedit.hxx"

#include "dp_gui.hrc"

using dp_gui::DescriptionEdit;

// DescriptionEdit -------------------------------------------------------

DescriptionEdit::DescriptionEdit( Window* pParent, const ResId& rResId ) :

    ExtMultiLineEdit( pParent, rResId ),

    m_bIsVerticalScrollBarHidden( true )

{
    Init();
}

// -----------------------------------------------------------------------

void DescriptionEdit::Init()
{
    Clear();
    // no tabstop
    SetStyle( ( GetStyle() & ~WB_TABSTOP ) | WB_NOTABSTOP );
    // read-only
    SetReadOnly();
    // no cursor
    EnableCursor( FALSE );
}

// -----------------------------------------------------------------------

void DescriptionEdit::UpdateScrollBar()
{
    if ( m_bIsVerticalScrollBarHidden )
    {
        ScrollBar*  pVScrBar = GetVScrollBar();
        if ( pVScrBar && pVScrBar->GetVisibleSize() < pVScrBar->GetRangeMax() )
        {
            pVScrBar->Show();
            m_bIsVerticalScrollBarHidden = false;
        }
    }
}

// -----------------------------------------------------------------------

void DescriptionEdit::Clear()
{
    SetText( String() );

    m_bIsVerticalScrollBarHidden = true;
    ScrollBar*  pVScrBar = GetVScrollBar();
    if ( pVScrBar )
        pVScrBar->Hide();
}

// -----------------------------------------------------------------------

void DescriptionEdit::SetDescription( const String& rDescription )
{
    SetText( rDescription );
    UpdateScrollBar();
}

