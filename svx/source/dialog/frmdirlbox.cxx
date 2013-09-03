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

#include <svx/frmdirlbox.hxx>
#include <vcl/builder.hxx>

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

FrameDirectionListBox::FrameDirectionListBox( Window* pParent, const ResId& rResId ) :
    ListBox( pParent, rResId )
{
}

FrameDirectionListBox::FrameDirectionListBox( Window* pParent, WinBits nBits ) :
    ListBox( pParent, nBits )
{
}

FrameDirectionListBox::~FrameDirectionListBox()
{
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeFrameDirectionListBox(Window *pParent, VclBuilder::stringmap &)
{
    FrameDirectionListBox* pListBox = new FrameDirectionListBox(pParent, WB_LEFT|WB_DROPDOWN|WB_VCENTER|WB_3DLOOK|WB_TABSTOP);
    pListBox->EnableAutoSize(true);
    return pListBox;
}


void FrameDirectionListBox::InsertEntryValue( const OUString& rString, SvxFrameDirection eDirection, sal_uInt16 nPos )
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
