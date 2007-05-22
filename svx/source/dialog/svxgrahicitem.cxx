/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svxgrahicitem.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 15:17:45 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include "svxgrahicitem.hxx"
TYPEINIT1(SvxGraphicItem,SfxPoolItem);
SvxGraphicItem::SvxGraphicItem( USHORT _nWhich, const Graphic& rGraphic )
    : SfxPoolItem( _nWhich ), aGraphic( rGraphic )
{

}
SvxGraphicItem::SvxGraphicItem( const SvxGraphicItem& rItem)
    : SfxPoolItem( rItem.Which() ), aGraphic( rItem.aGraphic )
{
}

int SvxGraphicItem::operator==( const SfxPoolItem& rItem) const
{
    return ((SvxGraphicItem&)rItem).aGraphic == aGraphic;
}

SfxPoolItem* SvxGraphicItem::Clone( SfxItemPool * ) const
{
    return new SvxGraphicItem( *this );
}
