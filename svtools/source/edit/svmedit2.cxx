/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svmedit2.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 14:47:18 $
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

#include "svmedit2.hxx"
#include "xtextedt.hxx"

ExtMultiLineEdit::ExtMultiLineEdit( Window* pParent, WinBits nWinStyle ) :

    MultiLineEdit( pParent, nWinStyle )

{
}

ExtMultiLineEdit::ExtMultiLineEdit( Window* pParent, const ResId& rResId ) :

    MultiLineEdit( pParent, rResId )

{
}

ExtMultiLineEdit::~ExtMultiLineEdit()
{
}

void ExtMultiLineEdit::InsertText( const String& rNew, BOOL )
{
    GetTextView()->InsertText( rNew, FALSE );
}

void ExtMultiLineEdit::SetAutoScroll( BOOL bAutoScroll )
{
    GetTextView()->SetAutoScroll( bAutoScroll );
}

void ExtMultiLineEdit::EnableCursor( BOOL bEnable )
{
    GetTextView()->EnableCursor( bEnable );
}

void ExtMultiLineEdit::SetAttrib( const TextAttrib& rAttr, ULONG nPara, USHORT nStart, USHORT nEnd )
{
    GetTextEngine()->SetAttrib( rAttr, nPara, nStart, nEnd );
}

void ExtMultiLineEdit::SetLeftMargin( USHORT nLeftMargin )
{
    GetTextEngine()->SetLeftMargin( nLeftMargin );
}

ULONG ExtMultiLineEdit::GetParagraphCount() const
{
    return GetTextEngine()->GetParagraphCount();
}

