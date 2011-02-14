/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include <svtools/svmedit2.hxx>
#include <svtools/xtextedt.hxx>

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

void ExtMultiLineEdit::InsertText( const String& rNew, sal_Bool )
{
    GetTextView()->InsertText( rNew, sal_False );
}

void ExtMultiLineEdit::SetAutoScroll( sal_Bool bAutoScroll )
{
    GetTextView()->SetAutoScroll( bAutoScroll );
}

void ExtMultiLineEdit::EnableCursor( sal_Bool bEnable )
{
    GetTextView()->EnableCursor( bEnable );
}

void ExtMultiLineEdit::SetAttrib( const TextAttrib& rAttr, sal_uLong nPara, sal_uInt16 nStart, sal_uInt16 nEnd )
{
    GetTextEngine()->SetAttrib( rAttr, nPara, nStart, nEnd );
}

void ExtMultiLineEdit::SetLeftMargin( sal_uInt16 nLeftMargin )
{
    GetTextEngine()->SetLeftMargin( nLeftMargin );
}

sal_uLong ExtMultiLineEdit::GetParagraphCount() const
{
    return GetTextEngine()->GetParagraphCount();
}

