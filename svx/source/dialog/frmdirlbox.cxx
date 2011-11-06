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

