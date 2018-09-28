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

void* DummyWindowWrapper::GetControlValue() const
{
    return nullptr;
}

void DummyWindowWrapper::SetControlValue( void* )
{
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


} // namespace sfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
