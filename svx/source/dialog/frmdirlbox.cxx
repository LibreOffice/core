/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_svx.hxx"
#include <svx/frmdirlbox.hxx>

namespace svx {

// ============================================================================

namespace {

inline void* lclEnumToVoid( SvxFrameDirection eDirection )
{
    return reinterpret_cast< void* >( static_cast< sal_uInt32 >( eDirection ) );
}

inline SvxFrameDirection lclVoidToEnum( void* pDirection )
{
    return static_cast< SvxFrameDirection >( reinterpret_cast< sal_IntPtr >( pDirection ) );
}

} // namespace

// ============================================================================

FrameDirectionListBox::FrameDirectionListBox( Window* pParent, WinBits nStyle ) :
    ListBox( pParent, nStyle )
{
}

FrameDirectionListBox::FrameDirectionListBox( Window* pParent, const ResId& rResId ) :
    ListBox( pParent, rResId )
{
}

FrameDirectionListBox::~FrameDirectionListBox()
{
}

void FrameDirectionListBox::InsertEntryValue( const String& rString, SvxFrameDirection eDirection, sal_uInt16 nPos )
{
    sal_uInt16 nRealPos = InsertEntry( rString, nPos );
    SetEntryData( nRealPos, lclEnumToVoid( eDirection ) );
}

void FrameDirectionListBox::RemoveEntryValue( SvxFrameDirection eDirection )
{
    sal_uInt16 nPos = GetEntryPos( lclEnumToVoid( eDirection ) );
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
        RemoveEntry( nPos );
}

void FrameDirectionListBox::SelectEntryValue( SvxFrameDirection eDirection )
{
    sal_uInt16 nPos = GetEntryPos( lclEnumToVoid( eDirection ) );
    if( nPos == LISTBOX_ENTRY_NOTFOUND )
        SetNoSelection();
    else
        SelectEntryPos( nPos );
}

SvxFrameDirection FrameDirectionListBox::GetSelectEntryValue() const
{
    sal_uInt16 nPos = GetSelectEntryPos();
    if( nPos == LISTBOX_ENTRY_NOTFOUND )
        return static_cast< SvxFrameDirection >( 0xFFFF );
    return lclVoidToEnum( GetEntryData( nPos ) );
}

// ============================================================================

FrameDirListBoxWrapper::FrameDirListBoxWrapper( FrameDirListBox& rListBox ) :
    SingleControlWrapperType( rListBox )
{
}

bool FrameDirListBoxWrapper::IsControlDontKnow() const
{
    return GetControl().GetSelectEntryCount() == 0;
}

void FrameDirListBoxWrapper::SetControlDontKnow( bool bSet )
{
    if( bSet )
        GetControl().SetNoSelection();
}

SvxFrameDirection FrameDirListBoxWrapper::GetControlValue() const
{
    return GetControl().GetSelectEntryValue();
}

void FrameDirListBoxWrapper::SetControlValue( SvxFrameDirection eValue )
{
    GetControl().SelectEntryValue( eValue );
}

// ============================================================================

} // namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
