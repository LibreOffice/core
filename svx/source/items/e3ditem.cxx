/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: e3ditem.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 13:28:25 $
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

DBG_NAMEEX(SvxB3DVectorItem)
DBG_NAME(SvxB3DVectorItem)

// -----------------------------------------------------------------------

TYPEINIT1_AUTOFACTORY(SvxB3DVectorItem, SfxPoolItem);

// -----------------------------------------------------------------------

SvxB3DVectorItem::SvxB3DVectorItem()
{
    DBG_CTOR(SvxB3DVectorItem, 0);
}

SvxB3DVectorItem::~SvxB3DVectorItem()
{
    DBG_DTOR(SvxB3DVectorItem, 0);
}

// -----------------------------------------------------------------------

SvxB3DVectorItem::SvxB3DVectorItem( USHORT _nWhich, const basegfx::B3DVector& rVal ) :
    SfxPoolItem( _nWhich ),
    aVal( rVal )
{
    DBG_CTOR(SvxB3DVectorItem, 0);
}

// -----------------------------------------------------------------------

SvxB3DVectorItem::SvxB3DVectorItem( USHORT _nWhich, SvStream& rStream ) :
    SfxPoolItem( _nWhich )
{
    DBG_CTOR(SvxB3DVectorItem, 0);
    double fValue;
    rStream >> fValue; aVal.setX(fValue);
    rStream >> fValue; aVal.setY(fValue);
    rStream >> fValue; aVal.setZ(fValue);
}

// -----------------------------------------------------------------------

SvxB3DVectorItem::SvxB3DVectorItem( const SvxB3DVectorItem& rItem ) :
    SfxPoolItem( rItem ),
    aVal( rItem.aVal )
{
    DBG_CTOR(SvxB3DVectorItem, 0);
}

// -----------------------------------------------------------------------

int SvxB3DVectorItem::operator==( const SfxPoolItem &rItem ) const
{
    DBG_CHKTHIS(SvxB3DVectorItem, 0);
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
    return ((SvxB3DVectorItem&)rItem).aVal == aVal;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxB3DVectorItem::Clone( SfxItemPool* /*pPool*/ ) const
{
    DBG_CHKTHIS(SvxB3DVectorItem, 0);
    return new SvxB3DVectorItem( *this );
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxB3DVectorItem::Create(SvStream &rStream, USHORT /*nVersion*/) const
{
    DBG_CHKTHIS(SvxB3DVectorItem, 0);
    basegfx::B3DVector aStr;
    double fValue;
    rStream >> fValue; aStr.setX(fValue);
    rStream >> fValue; aStr.setY(fValue);
    rStream >> fValue; aStr.setZ(fValue);
    return new SvxB3DVectorItem(Which(), aStr);
}

// -----------------------------------------------------------------------

SvStream& SvxB3DVectorItem::Store(SvStream &rStream, USHORT /*nItemVersion*/) const
{
    DBG_CHKTHIS(SvxB3DVectorItem, 0);

    // ## if (nItemVersion)
    double fValue;
    fValue = aVal.getX(); rStream << fValue;
    fValue = aVal.getY(); rStream << fValue;
    fValue = aVal.getZ(); rStream << fValue;

    return rStream;
}

// -----------------------------------------------------------------------

sal_Bool SvxB3DVectorItem::QueryValue( uno::Any& rVal, BYTE /*nMemberId*/ ) const
{
    drawing::Direction3D aDirection;

    // Werte eintragen
    aDirection.DirectionX = aVal.getX();
    aDirection.DirectionY = aVal.getY();
    aDirection.DirectionZ = aVal.getZ();

    rVal <<= aDirection;
    return( sal_True );
}

// -----------------------------------------------------------------------

sal_Bool SvxB3DVectorItem::PutValue( const uno::Any& rVal, BYTE /*nMemberId*/ )
{
    drawing::Direction3D aDirection;
    if(!(rVal >>= aDirection))
        return sal_False;

    aVal.setX(aDirection.DirectionX);
    aVal.setY(aDirection.DirectionY);
    aVal.setZ(aDirection.DirectionZ);
    return sal_True;
}

// -----------------------------------------------------------------------

USHORT SvxB3DVectorItem::GetVersion (USHORT nFileFormatVersion) const
{
    return (nFileFormatVersion == SOFFICE_FILEFORMAT_31) ? USHRT_MAX : 0;
}

// eof
