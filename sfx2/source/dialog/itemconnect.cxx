/*************************************************************************
 *
 *  $RCSfile: itemconnect.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-12-16 11:00:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SFX_ITEMCONNECT_HXX
#include "itemconnect.hxx"
#endif

#include <list>
#include <boost/shared_ptr.hpp>

#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif

// ============================================================================

namespace sfx {

// ============================================================================
// Helpers
// ============================================================================

USHORT ItemConnHelper::GetWhichId( const SfxItemSet& rItemSet, USHORT nSlot )
{
    return rItemSet.GetPool()->GetWhich( nSlot );
}

const SfxPoolItem* ItemConnHelper::GetUniqueItem( const SfxItemSet& rItemSet, USHORT nSlot )
{
    USHORT nWhich = GetWhichId( rItemSet, nSlot );
    return (rItemSet.GetItemState( nWhich, TRUE ) >= SFX_ITEM_DEFAULT) ? rItemSet.GetItem( nWhich, TRUE ) : 0;
}

void ItemConnHelper::RemoveDefaultItem( SfxItemSet& rDestSet, const SfxItemSet& rOldSet, USHORT nSlot )
{
    USHORT nWhich = GetWhichId( rDestSet, nSlot );
    if( rOldSet.GetItemState( nWhich, FALSE ) == SFX_ITEM_DEFAULT )
        rDestSet.ClearItem( nWhich );
}

void ItemConnHelper::HandleFlags( const SfxItemSet& rItemSet, USHORT nSlot, Window& rWindow, ItemConnFlags nFlags )
{
    if( rItemSet.GetItemState( GetWhichId( rItemSet, nSlot ), TRUE ) == SFX_ITEM_UNKNOWN )
    {
        if( nFlags & ITEMCONN_DISABLE_UNKNOWN )
            rWindow.Enable( false );
        if( nFlags & ITEMCONN_HIDE_UNKNOWN )
            rWindow.Show( false );
    }
    else
    {
        if( nFlags & ITEMCONN_ENABLE_KNOWN )
            rWindow.Enable( true );
        if( nFlags & ITEMCONN_SHOW_KNOWN )
            rWindow.Show( true );
    }
}

// ============================================================================
// Control wrappers
// ============================================================================

CheckBoxWrapper::CheckBoxWrapper( CheckBox& rCheckBox ) :
        SingleControlWrapperType( rCheckBox )
{
}

bool CheckBoxWrapper::IsControlDontKnow() const
{
    return GetControl().GetState() == STATE_DONTKNOW;
}

void CheckBoxWrapper::SetControlDontKnow( bool bSet )
{
    GetControl().EnableTriState( bSet );
    GetControl().SetState( bSet ? STATE_DONTKNOW : STATE_NOCHECK );
}

BOOL CheckBoxWrapper::GetControlValue() const
{
    return GetControl().IsChecked();
}

void CheckBoxWrapper::SetControlValue( BOOL bValue )
{
    GetControl().Check( bValue );
}

// ----------------------------------------------------------------------------

EditWrapper::EditWrapper( Edit& rEdit ) :
        SingleControlWrapperType( rEdit )
{
}

bool EditWrapper::IsControlDontKnow() const
{
    // no "don't know" state - empty string is a valid value of an Edit
    return false;
}

void EditWrapper::SetControlDontKnow( bool bSet )
{
    if( bSet )
        GetControl().SetText( String() );
}

String EditWrapper::GetControlValue() const
{
    return GetControl().GetText();
}

void EditWrapper::SetControlValue( String aValue )
{
    GetControl().SetText( aValue );
}

// ============================================================================
// Base connection classes
// ============================================================================

ItemConnectionBase::ItemConnectionBase( ItemConnFlags nFlags ) :
    mnFlags( nFlags )
{
}

ItemConnectionBase::~ItemConnectionBase()
{
}

void ItemConnectionBase::Activate( bool bActive )
{
    if( bActive ) mnFlags &= ~ITEMCONN_INACTIVE; else mnFlags |= ITEMCONN_INACTIVE;
}

bool ItemConnectionBase::IsActive() const
{
    return !(mnFlags & ITEMCONN_INACTIVE);
}

void ItemConnectionBase::DoApplyFlags( const SfxItemSet& rItemSet )
{
    if( IsActive() )
        ApplyFlags( rItemSet );
}

void ItemConnectionBase::DoReset( const SfxItemSet& rItemSet )
{
    if( IsActive() )
        Reset( rItemSet );
}

bool ItemConnectionBase::DoFillItemSet( SfxItemSet& rDestSet, const SfxItemSet& rOldSet )
{
    return IsActive() && FillItemSet( rDestSet, rOldSet );
}

// ============================================================================
// Standard connections
// ============================================================================

DummyItemConnection::DummyItemConnection( USHORT nSlot, Window& rWindow, ItemConnFlags nFlags ) :
    ItemConnectionBase( nFlags ),
    mrWindow( rWindow ),
    mnSlot( nSlot )
{
}

void DummyItemConnection::ApplyFlags( const SfxItemSet& rItemSet )
{
    ItemConnHelper::HandleFlags( rItemSet, mnSlot, mrWindow, GetFlags() );
}

void DummyItemConnection::Reset( const SfxItemSet& rItemSet )
{
    // nothing to do
}

bool DummyItemConnection::FillItemSet( SfxItemSet& rDestSet, const SfxItemSet& rOldSet )
{
    // nothing to do
    return false;
}

// ----------------------------------------------------------------------------

CheckBoxConnection::CheckBoxConnection( USHORT nSlot, CheckBox& rCheckBox, ItemConnFlags nFlags ) :
    SingleItemConnectionType( nSlot, CheckBoxWrapper( rCheckBox ), nFlags )
{
}

// ----------------------------------------------------------------------------

EditConnection::EditConnection( USHORT nSlot, Edit& rEdit, ItemConnFlags nFlags ) :
    SingleItemConnectionType( nSlot, EditWrapper( rEdit ), nFlags )
{
}

// ============================================================================
// Array of connections
// ============================================================================

class ItemConnectionArrayImpl
{
public:
    void                        Append( ItemConnectionBase* pConnection );

    void                        ApplyFlags( const SfxItemSet& rItemSet );
    void                        Reset( const SfxItemSet& rItemSet );
    bool                        FillItemSet( SfxItemSet& rDestSet, const SfxItemSet& rOldSet );

private:
    typedef boost::shared_ptr< ItemConnectionBase > ItemConnectionRef;
    typedef std::list< ItemConnectionRef >          ItemConnectionList;
    typedef ItemConnectionList::iterator            ItemConnectionListIt;

    ItemConnectionList          maList;
};

void ItemConnectionArrayImpl::Append( ItemConnectionBase* pConnection )
{
    if( pConnection )
        maList.push_back( ItemConnectionRef( pConnection ) );
}

void ItemConnectionArrayImpl::ApplyFlags( const SfxItemSet& rItemSet )
{
    for( ItemConnectionListIt aIt = maList.begin(), aEnd = maList.end(); aIt != aEnd; ++aIt )
        (*aIt)->DoApplyFlags( rItemSet );
}

void ItemConnectionArrayImpl::Reset( const SfxItemSet& rItemSet )
{
    for( ItemConnectionListIt aIt = maList.begin(), aEnd = maList.end(); aIt != aEnd; ++aIt )
        (*aIt)->DoReset( rItemSet );
}

bool ItemConnectionArrayImpl::FillItemSet( SfxItemSet& rDestSet, const SfxItemSet& rOldSet )
{
    bool bChanged = false;
    for( ItemConnectionListIt aIt = maList.begin(), aEnd = maList.end(); aIt != aEnd; ++aIt )
        bChanged |= (*aIt)->DoFillItemSet( rDestSet, rOldSet );
    return bChanged;
}

// ----------------------------------------------------------------------------

ItemConnectionArray::ItemConnectionArray() :
    mpImpl( new ItemConnectionArrayImpl )
{
}

ItemConnectionArray::~ItemConnectionArray()
{
}

void ItemConnectionArray::AddConnection( ItemConnectionBase* pConnection )
{
    mpImpl->Append( pConnection );
}

void ItemConnectionArray::ApplyFlags( const SfxItemSet& rItemSet )
{
    mpImpl->ApplyFlags( rItemSet );
}

void ItemConnectionArray::Reset( const SfxItemSet& rItemSet )
{
    mpImpl->Reset( rItemSet );
}

bool ItemConnectionArray::FillItemSet( SfxItemSet& rDestSet, const SfxItemSet& rOldSet )
{
    return mpImpl->FillItemSet( rDestSet, rOldSet );
}

// ============================================================================

} // namespace sfx

