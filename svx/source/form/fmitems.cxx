/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmitems.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 22:53:24 $
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
#ifndef _SVX_FMITEMS_HXX
#include "fmitems.hxx"
#endif

#include <svxids.hrc>

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

//========================================================================
// class FmInterfaceItem
//========================================================================
TYPEINIT1(FmInterfaceItem, SfxPoolItem);

//------------------------------------------------------------------------------
int FmInterfaceItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );
    return( xInterface == ((FmInterfaceItem&)rAttr).GetInterface() );
}

//------------------------------------------------------------------------------
SfxPoolItem* FmInterfaceItem::Clone( SfxItemPool* ) const
{
    return new FmInterfaceItem( *this );
}

//------------------------------------------------------------------------------
SvStream& FmInterfaceItem::Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const
{
    return rStrm;
}

//------------------------------------------------------------------------------
SfxPoolItem* FmInterfaceItem::Create( SvStream& rStrm, sal_uInt16 ) const
{
    return new FmInterfaceItem( *this );
}




