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
