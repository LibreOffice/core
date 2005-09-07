/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: itemconnect.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:20:07 $
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

#ifndef SFX_ITEMCONNECT_HXX
#include "itemconnect.hxx"
#endif

#include <boost/shared_ptr.hpp>
#include <list>

#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif

// ============================================================================

namespace sfx {

// ============================================================================
// Helpers
// ============================================================================

namespace {

TriState lclConvertToTriState( bool bKnown, bool bIsKnownFlag, bool bIsUnknownFlag )
{
    return (bKnown && bIsKnownFlag) ? STATE_CHECK : ((!bKnown && bIsUnknownFlag) ? STATE_NOCHECK : STATE_DONTKNOW);
}

} // namespace

// ----------------------------------------------------------------------------

USHORT ItemWrapperHelper::GetWhichId( const SfxItemSet& rItemSet, USHORT nSlot )
{
    return rItemSet.GetPool()->GetWhich( nSlot );
}

bool ItemWrapperHelper::IsKnownItem( const SfxItemSet& rItemSet, USHORT nSlot )
{
    return rItemSet.GetItemState( GetWhichId( rItemSet, nSlot ), TRUE ) != SFX_ITEM_UNKNOWN;
}

const SfxPoolItem* ItemWrapperHelper::GetUniqueItem( const SfxItemSet& rItemSet, USHORT nSlot )
{
    USHORT nWhich = GetWhichId( rItemSet, nSlot );
    return (rItemSet.GetItemState( nWhich, TRUE ) >= SFX_ITEM_DEFAULT) ? rItemSet.GetItem( nWhich, TRUE ) : 0;
}

const SfxPoolItem& ItemWrapperHelper::GetDefaultItem( const SfxItemSet& rItemSet, USHORT nSlot )
{
    return rItemSet.GetPool()->GetDefaultItem( GetWhichId( rItemSet, nSlot ) );
}

void ItemWrapperHelper::RemoveDefaultItem( SfxItemSet& rDestSet, const SfxItemSet& rOldSet, USHORT nSlot )
{
    USHORT nWhich = GetWhichId( rDestSet, nSlot );
    if( rOldSet.GetItemState( nWhich, FALSE ) == SFX_ITEM_DEFAULT )
        rDestSet.ClearItem( nWhich );
}

// ============================================================================
// Base control wrapper classes
// ============================================================================

ControlWrapperBase::~ControlWrapperBase()
{
}

// ============================================================================
// Single control wrappers
// ============================================================================

DummyWindowWrapper::DummyWindowWrapper( Window& rWindow ) :
    SingleControlWrapperType( rWindow )
{
}

bool DummyWindowWrapper::IsControlDontKnow() const
{
    return false;
}

void DummyWindowWrapper::SetControlDontKnow( bool )
{
}

void* DummyWindowWrapper::GetControlValue() const
{
    return 0;
}

void DummyWindowWrapper::SetControlValue( void* )
{
}

// ----------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------

ColorListBoxWrapper::ColorListBoxWrapper(ColorListBox & rListBox):
    SingleControlWrapper< ColorListBox, Color >(rListBox)
{}

ColorListBoxWrapper::~ColorListBoxWrapper()
{}

bool ColorListBoxWrapper::IsControlDontKnow() const
{
    return GetControl().GetSelectEntryCount() == 0;
}

void ColorListBoxWrapper::SetControlDontKnow( bool bSet )
{
    if( bSet ) GetControl().SetNoSelection();
}

Color ColorListBoxWrapper::GetControlValue() const
{
    return GetControl().GetSelectEntryColor();
}

void ColorListBoxWrapper::SetControlValue( Color aColor )
{
    GetControl().SelectEntry( aColor );
}

// ============================================================================
// Multi control wrappers
// ============================================================================

typedef std::vector< ControlWrapperBase* >  ControlWrpVec;
typedef ControlWrpVec::iterator             ControlWrpVecI;
typedef ControlWrpVec::const_iterator       ControlWrpVecCI;

struct MultiControlWrapperHelper_Impl
{
    ControlWrpVec       maVec;
};

MultiControlWrapperHelper::MultiControlWrapperHelper() :
    mxImpl( new MultiControlWrapperHelper_Impl )
{
}

MultiControlWrapperHelper::~MultiControlWrapperHelper()
{
}

void MultiControlWrapperHelper::RegisterControlWrapper( ControlWrapperBase& rWrapper )
{
    mxImpl->maVec.push_back( &rWrapper );
}

void MultiControlWrapperHelper::ModifyControl( TriState eEnable, TriState eShow )
{
    for( ControlWrpVecI aIt = mxImpl->maVec.begin(), aEnd = mxImpl->maVec.end(); aIt != aEnd; ++aIt )
        (*aIt)->ModifyControl( eEnable, eShow );
}

bool MultiControlWrapperHelper::IsControlDontKnow() const
{
    bool bIs = !mxImpl->maVec.empty();
    for( ControlWrpVecCI aIt = mxImpl->maVec.begin(), aEnd = mxImpl->maVec.end(); bIs && (aIt != aEnd); ++aIt )
        bIs &= (*aIt)->IsControlDontKnow();
    return bIs;
}

void MultiControlWrapperHelper::SetControlDontKnow( bool bSet )
{
    for( ControlWrpVecI aIt = mxImpl->maVec.begin(), aEnd = mxImpl->maVec.end(); aIt != aEnd; ++aIt )
        (*aIt)->SetControlDontKnow( bSet );
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

TriState ItemConnectionBase::GetEnableState( bool bKnown ) const
{
    return lclConvertToTriState( bKnown, mnFlags & ITEMCONN_ENABLE_KNOWN, mnFlags & ITEMCONN_DISABLE_UNKNOWN );
}

TriState ItemConnectionBase::GetShowState( bool bKnown ) const
{
    return lclConvertToTriState( bKnown, mnFlags & ITEMCONN_SHOW_KNOWN, mnFlags & ITEMCONN_HIDE_UNKNOWN );
}

// ============================================================================
// Standard connections
// ============================================================================

DummyItemConnection::DummyItemConnection( USHORT nSlot, Window& rWindow, ItemConnFlags nFlags ) :
    ItemConnectionBase( nFlags ),
    DummyWindowWrapper( rWindow ),
    mnSlot( nSlot )
{
}

void DummyItemConnection::ApplyFlags( const SfxItemSet& rItemSet )
{
    bool bKnown = ItemWrapperHelper::IsKnownItem( rItemSet, mnSlot );
    ModifyControl( GetEnableState( bKnown ), GetShowState( bKnown ) );
}

void DummyItemConnection::Reset( const SfxItemSet& rItemSet )
{
}

bool DummyItemConnection::FillItemSet( SfxItemSet& rDestSet, const SfxItemSet& rOldSet )
{
    return false;   // item set not changed
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
    mxImpl( new ItemConnectionArrayImpl )
{
}

ItemConnectionArray::~ItemConnectionArray()
{
}

void ItemConnectionArray::AddConnection( ItemConnectionBase* pConnection )
{
    mxImpl->Append( pConnection );
}

void ItemConnectionArray::ApplyFlags( const SfxItemSet& rItemSet )
{
    mxImpl->ApplyFlags( rItemSet );
}

void ItemConnectionArray::Reset( const SfxItemSet& rItemSet )
{
    mxImpl->Reset( rItemSet );
}

bool ItemConnectionArray::FillItemSet( SfxItemSet& rDestSet, const SfxItemSet& rOldSet )
{
    return mxImpl->FillItemSet( rDestSet, rOldSet );
}

// ============================================================================

} // namespace sfx

