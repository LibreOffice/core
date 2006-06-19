/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ofaitem.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 16:13:39 $
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

#include "ofaitem.hxx"

OfaPtrItem::OfaPtrItem( USHORT _nWhich, void *_pPtr )
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
TYPEINIT1_AUTOFACTORY(DashListPtrItem, SvxDashListPtrItem);

DashListPtrItem::DashListPtrItem( USHORT nWhich, SvxDashListItem* pPtr )
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

sal_Bool DashListPtrItem::QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const
{
}

sal_Bool DashListPtrItem::PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 )
{
    sal_Int64 nHyper;
    if ( rVal >>= nHyper )
    {
        SetValue( (SvxDash
    }

    return sal_False;
}
*/
