/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: e3ditem.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 05:19:29 $
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

#ifndef _COM_SUN_STAR_DRAWING_DIRECTION3D_HPP_
#include <com/sun/star/drawing/Direction3D.hpp>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

#include "e3ditem.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;

// STATIC DATA -----------------------------------------------------------

DBG_NAMEEX(SvxVector3DItem);
DBG_NAME(SvxVector3DItem);

// -----------------------------------------------------------------------

TYPEINIT1_AUTOFACTORY(SvxVector3DItem, SfxPoolItem);

// -----------------------------------------------------------------------

SvxVector3DItem::SvxVector3DItem()
{
    DBG_CTOR(SvxVector3DItem, 0);
}

SvxVector3DItem::~SvxVector3DItem()
{
    DBG_DTOR(SvxVector3DItem, 0);
}

// -----------------------------------------------------------------------

SvxVector3DItem::SvxVector3DItem( USHORT _nWhich, const Vector3D& rVal ) :
    SfxPoolItem( _nWhich ),
    aVal( rVal )
{
    DBG_CTOR(SvxVector3DItem, 0);
}

// -----------------------------------------------------------------------

SvxVector3DItem::SvxVector3DItem( USHORT _nWhich, SvStream& rStream ) :
    SfxPoolItem( _nWhich )
{
    DBG_CTOR(SvxVector3DItem, 0);
    rStream >> aVal;
}

// -----------------------------------------------------------------------

SvxVector3DItem::SvxVector3DItem( const SvxVector3DItem& rItem ) :
    SfxPoolItem( rItem ),
    aVal( rItem.aVal )
{
    DBG_CTOR(SvxVector3DItem, 0);
}

// -----------------------------------------------------------------------

int SvxVector3DItem::operator==( const SfxPoolItem &rItem ) const
{
    DBG_CHKTHIS(SvxVector3DItem, 0);
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
    return ((SvxVector3DItem&)rItem).aVal == aVal;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxVector3DItem::Clone( SfxItemPool * ) const
{
    DBG_CHKTHIS(SvxVector3DItem, 0);
    return new SvxVector3DItem( *this );
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxVector3DItem::Create(SvStream &rStream, USHORT /*nVersion*/) const
{
    DBG_CHKTHIS(SvxVector3DItem, 0);
    Vector3D aStr;
    rStream >> aStr;
    return new SvxVector3DItem(Which(), aStr);
}

// -----------------------------------------------------------------------

SvStream& SvxVector3DItem::Store(SvStream &rStream, USHORT /*nItemVersion*/) const
{
    DBG_CHKTHIS(SvxVector3DItem, 0);

    // ## if (nItemVersion)
    rStream << aVal;

    return rStream;
}

// -----------------------------------------------------------------------

sal_Bool SvxVector3DItem::QueryValue( uno::Any& rVal, BYTE /*nMemberId*/ ) const
{
    drawing::Direction3D aDirection;

    // Werte eintragen
    aDirection.DirectionX = aVal.X();
    aDirection.DirectionY = aVal.Y();
    aDirection.DirectionZ = aVal.Z();

    rVal <<= aDirection;
    return( sal_True );
}

// -----------------------------------------------------------------------

sal_Bool SvxVector3DItem::PutValue( const uno::Any& rVal, BYTE /*nMemberId*/ )
{
    drawing::Direction3D aDirection;
    if(!(rVal >>= aDirection))
        return sal_False;

    aVal.X() = aDirection.DirectionX;
    aVal.Y() = aDirection.DirectionY;
    aVal.Z() = aDirection.DirectionZ;
    return sal_True;
}

// -----------------------------------------------------------------------

USHORT SvxVector3DItem::GetVersion (USHORT nFileFormatVersion) const
{
    return (nFileFormatVersion == SOFFICE_FILEFORMAT_31) ? USHRT_MAX : 0;
}


