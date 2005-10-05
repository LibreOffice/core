/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: frmdirlbox.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2005-10-05 14:34:58 $
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

#pragma hdrstop

#ifndef _SVX_FRMDIRLBOX_HXX
#include "frmdirlbox.hxx"
#endif

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

