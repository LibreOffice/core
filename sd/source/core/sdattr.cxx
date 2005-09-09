/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdattr.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 03:13:09 $
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

#include "sdattr.hxx"

using namespace ::com::sun::star;

/*************************************************************************
|*
|*  DiaEffectItem
|*
*************************************************************************/
TYPEINIT1_AUTOFACTORY( DiaEffectItem, SfxEnumItem );


DiaEffectItem::DiaEffectItem( presentation::FadeEffect eFE ) :
    SfxEnumItem( ATTR_DIA_EFFECT, eFE )
{
}


DiaEffectItem::DiaEffectItem( SvStream& rIn ) :
    SfxEnumItem( ATTR_DIA_EFFECT, rIn )
{
}


SfxPoolItem* DiaEffectItem::Clone( SfxItemPool* pPool ) const
{
    return new DiaEffectItem( *this );
}


SfxPoolItem* DiaEffectItem::Create( SvStream& rIn, USHORT nVer ) const
{
    return new DiaEffectItem( rIn );
}

/*************************************************************************
|*
|*  DiaSpeedItem
|*
*************************************************************************/
TYPEINIT1_AUTOFACTORY( DiaSpeedItem, SfxEnumItem );


DiaSpeedItem::DiaSpeedItem( FadeSpeed eFS ) :
    SfxEnumItem( ATTR_DIA_SPEED, eFS )
{
}


DiaSpeedItem::DiaSpeedItem( SvStream& rIn ) :
    SfxEnumItem( ATTR_DIA_SPEED, rIn )
{
}


SfxPoolItem* DiaSpeedItem::Clone( SfxItemPool* pPool ) const
{
    return new DiaSpeedItem( *this );
}


SfxPoolItem* DiaSpeedItem::Create( SvStream& rIn, USHORT nVer ) const
{
    return new DiaSpeedItem( rIn );
}

/*************************************************************************
|*
|*  DiaAutoItem
|*
*************************************************************************/
TYPEINIT1_AUTOFACTORY( DiaAutoItem, SfxEnumItem );

DiaAutoItem::DiaAutoItem( PresChange eChange ) :
    SfxEnumItem( ATTR_DIA_AUTO, eChange )
{
}


DiaAutoItem::DiaAutoItem( SvStream& rIn ) :
    SfxEnumItem( ATTR_DIA_AUTO, rIn )
{
}


SfxPoolItem* DiaAutoItem::Clone( SfxItemPool* pPool ) const
{
    return new DiaAutoItem( *this );
}


SfxPoolItem* DiaAutoItem::Create( SvStream& rIn, USHORT nVer ) const
{
    return new DiaAutoItem( rIn );
}

/*************************************************************************
|*
|*  DiaTimeItem
|*
*************************************************************************/
TYPEINIT1_AUTOFACTORY( DiaTimeItem, SfxUInt32Item );


DiaTimeItem::DiaTimeItem( UINT32 nValue ) :
        SfxUInt32Item( ATTR_DIA_TIME, nValue )
{
}


SfxPoolItem* DiaTimeItem::Clone( SfxItemPool* pPool ) const
{
    return new DiaTimeItem( *this );
}


int DiaTimeItem::operator==( const SfxPoolItem& rItem ) const
{
    return( ( (DiaTimeItem&) rItem ).GetValue() == GetValue() );
}



