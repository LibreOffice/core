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

#include "svx/ofaitem.hxx"

OfaPtrItem::OfaPtrItem( sal_uInt16 _nWhich, void *_pPtr )
    : SfxPoolItem( _nWhich ), pPtr( _pPtr )
{

}
OfaPtrItem::OfaPtrItem( const OfaPtrItem& rItem)
    : SfxPoolItem( rItem.Which() ), pPtr( rItem.pPtr )
{
}

int OfaPtrItem::operator==( const SfxPoolItem& rItem) const
{
    return ((OfaPtrItem&)rItem).pPtr == pPtr;
}

SfxPoolItem* OfaPtrItem::Clone( SfxItemPool * ) const
{
    return new OfaPtrItem( *this );
}

//---------------------------------------------------------------------------
/*
TYPEINIT1_FACTORY(DashListPtrItem, SvxDashListPtrItem, new DashListPtrItem(0));

DashListPtrItem::DashListPtrItem( sal_uInt16 nWhich, SvxDashListItem* pPtr )
    : OfaPtrItem( nWhich ), pPtr( pPtr )
{
}

DashListPtrItem::DashListPtrItem( const DashListPtrItem& )
    : OfaPtrItem( rItem.Which() ), pPtr( rItem.pPtr )
{
}

int DashListPtrItem::operator==( const SfxPoolItem& rItem ) const
{
    return ((DashListPtrItem&)rItem).GetValue() == GetValue();
}

SfxPoolItem* DashListPtrItem::Clone( SfxItemPool *pPool ) const
{
    return new DashListPtrItem( *this );
}

sal_Bool DashListPtrItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const
{
}

sal_Bool DashListPtrItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 )
{
    sal_Int64 nHyper;
    if ( rVal >>= nHyper )
    {
        SetValue( (SvxDash
    }

    return sal_False;
}
*/

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
