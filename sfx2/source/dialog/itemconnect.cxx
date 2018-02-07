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

#include <svl/itempool.hxx>
#include <sfx2/itemconnect.hxx>
#include <vector>
#include <memory>

namespace sfx {


// Helpers


namespace {

TriState lclConvertToTriState( bool bKnown, bool bIsUnknownFlag )
{
    return (!bKnown && bIsUnknownFlag) ? TRISTATE_FALSE : TRISTATE_INDET;
}

} // namespace


sal_uInt16 ItemWrapperHelper::GetWhichId( const SfxItemSet& rItemSet, sal_uInt16 nSlot )
{
    return rItemSet.GetPool()->GetWhich( nSlot );
}

bool ItemWrapperHelper::IsKnownItem( const SfxItemSet& rItemSet, sal_uInt16 nSlot )
{
    return rItemSet.GetItemState( GetWhichId( rItemSet, nSlot ) ) != SfxItemState::UNKNOWN;
}

const SfxPoolItem* ItemWrapperHelper::GetUniqueItem( const SfxItemSet& rItemSet, sal_uInt16 nSlot )
{
    sal_uInt16 nWhich = GetWhichId( rItemSet, nSlot );
    return (rItemSet.GetItemState( nWhich ) >= SfxItemState::DEFAULT) ? rItemSet.GetItem( nWhich ) : nullptr;
}

const SfxPoolItem& ItemWrapperHelper::GetDefaultItem( const SfxItemSet& rItemSet, sal_uInt16 nSlot )
{
    return rItemSet.GetPool()->GetDefaultItem( GetWhichId( rItemSet, nSlot ) );
}

void ItemWrapperHelper::RemoveDefaultItem( SfxItemSet& rDestSet, const SfxItemSet& rOldSet, sal_uInt16 nSlot )
{
    sal_uInt16 nWhich = GetWhichId( rDestSet, nSlot );
    if( rOldSet.GetItemState( nWhich, false ) == SfxItemState::DEFAULT )
        rDestSet.ClearItem( nWhich );
}


// Base control wrapper classes


ControlWrapperBase::~ControlWrapperBase()
{
}


// Single control wrappers


DummyWindowWrapper::DummyWindowWrapper( vcl::Window& rWindow ) :
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
    return nullptr;
}

void DummyWindowWrapper::SetControlValue( void* )
{
}


CheckBoxWrapper::CheckBoxWrapper( CheckBox& rCheckBox ) :
        SingleControlWrapperType( rCheckBox )
{
}

bool CheckBoxWrapper::IsControlDontKnow() const
{
    return GetControl().GetState() == TRISTATE_INDET;
}

void CheckBoxWrapper::SetControlDontKnow( bool bSet )
{
    GetControl().EnableTriState( bSet );
    GetControl().SetState( bSet ? TRISTATE_INDET : TRISTATE_FALSE );
}

bool CheckBoxWrapper::GetControlValue() const
{
    return GetControl().IsChecked();
}

void CheckBoxWrapper::SetControlValue( bool bValue )
{
    GetControl().Check( bValue );
}


// Multi control wrappers


typedef std::vector< ControlWrapperBase* >  ControlWrpVec;

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

void MultiControlWrapperHelper::ModifyControl( TriState eShow )
{
    for (auto const& elem : mxImpl->maVec)
        elem->ModifyControl( eShow );
}

bool MultiControlWrapperHelper::IsControlDontKnow() const
{
    if (mxImpl->maVec.empty())
        return false;
    for (auto const& elem : mxImpl->maVec)
    {
        if (!elem->IsControlDontKnow())
            return false;
    }
    return true;
}

void MultiControlWrapperHelper::SetControlDontKnow( bool bSet )
{
    for (auto const& elem : mxImpl->maVec)
        elem->SetControlDontKnow(bSet);
}


// Base connection classes


ItemConnectionBase::ItemConnectionBase( ItemConnFlags nFlags ) :
    mnFlags( nFlags )
{
}

ItemConnectionBase::~ItemConnectionBase()
{
}

void ItemConnectionBase::DoApplyFlags( const SfxItemSet* pItemSet )
{
    ApplyFlags( pItemSet );
}

void ItemConnectionBase::DoReset( const SfxItemSet* pItemSet )
{
    Reset( pItemSet );
}

bool ItemConnectionBase::DoFillItemSet( SfxItemSet& rDestSet, const SfxItemSet& rOldSet )
{
    return FillItemSet( rDestSet, rOldSet );
}

TriState ItemConnectionBase::GetShowState( bool bKnown ) const
{
    return lclConvertToTriState( bKnown, bool(mnFlags & ItemConnFlags::HideUnknown) );
}


// Standard connections


DummyItemConnection::DummyItemConnection( sal_uInt16 nSlot, vcl::Window& rWindow, ItemConnFlags nFlags ) :
    ItemConnectionBase( nFlags ),
    DummyWindowWrapper( rWindow ),
    mnSlot( nSlot )
{
}

void DummyItemConnection::ApplyFlags( const SfxItemSet* pItemSet )
{
    bool bKnown = ItemWrapperHelper::IsKnownItem( *pItemSet, mnSlot );
    ModifyControl( GetShowState( bKnown ) );
}

void DummyItemConnection::Reset( const SfxItemSet* )
{
}

bool DummyItemConnection::FillItemSet( SfxItemSet& /*rDestSet*/, const SfxItemSet& /*rOldSet*/ )
{
    return false;   // item set not changed
}


// Array of connections


class ItemConnectionArrayImpl
{
public:
    void                        Append( ItemConnectionBase* pConnection );

    void                        ApplyFlags( const SfxItemSet* pItemSet );
    void                        Reset( const SfxItemSet* pItemSet );
    bool                        FillItemSet( SfxItemSet& rDestSet, const SfxItemSet& rOldSet );

private:
    typedef std::shared_ptr< ItemConnectionBase > ItemConnectionRef;
    typedef std::vector< ItemConnectionRef > ItemConnectionVector;

    ItemConnectionVector maVector;
};

void ItemConnectionArrayImpl::Append( ItemConnectionBase* pConnection )
{
    if( pConnection )
        maVector.push_back( ItemConnectionRef( pConnection ) );
}

void ItemConnectionArrayImpl::ApplyFlags( const SfxItemSet* pItemSet )
{
    for (auto const& itemConnection : maVector)
        itemConnection->DoApplyFlags( pItemSet );
}

void ItemConnectionArrayImpl::Reset( const SfxItemSet* pItemSet )
{
    for (auto const& itemConnection : maVector)
        itemConnection->DoReset( pItemSet );
}

bool ItemConnectionArrayImpl::FillItemSet( SfxItemSet& rDestSet, const SfxItemSet& rOldSet )
{
    bool bChanged = false;
    for (auto const& itemConnection : maVector)
        bChanged |= itemConnection->DoFillItemSet( rDestSet, rOldSet );
    return bChanged;
}


ItemConnectionArray::ItemConnectionArray() :
    ItemConnectionBase(ItemConnFlags::NONE),
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

void ItemConnectionArray::ApplyFlags( const SfxItemSet* pItemSet )
{
    mxImpl->ApplyFlags( pItemSet );
}

void ItemConnectionArray::Reset( const SfxItemSet* pItemSet )
{
    mxImpl->Reset( pItemSet );
}

bool ItemConnectionArray::FillItemSet( SfxItemSet& rDestSet, const SfxItemSet& rOldSet )
{
    return mxImpl->FillItemSet( rDestSet, rOldSet );
}


} // namespace sfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
