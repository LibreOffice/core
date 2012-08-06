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

#include <com/sun/star/drawing/Direction3D.hpp>
#include <tools/stream.hxx>

#include <svx/e3ditem.hxx>

using namespace ::rtl;
using namespace ::com::sun::star;

// STATIC DATA -----------------------------------------------------------

DBG_NAMEEX(SvxB3DVectorItem)
DBG_NAME(SvxB3DVectorItem)

// -----------------------------------------------------------------------

TYPEINIT1_FACTORY(SvxB3DVectorItem, SfxPoolItem, new SvxB3DVectorItem);

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

SvxB3DVectorItem::SvxB3DVectorItem( sal_uInt16 _nWhich, const basegfx::B3DVector& rVal ) :
    SfxPoolItem( _nWhich ),
    aVal( rVal )
{
    DBG_CTOR(SvxB3DVectorItem, 0);
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

SfxPoolItem* SvxB3DVectorItem::Create(SvStream &rStream, sal_uInt16 /*nVersion*/) const
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

SvStream& SvxB3DVectorItem::Store(SvStream &rStream, sal_uInt16 /*nItemVersion*/) const
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

bool SvxB3DVectorItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    drawing::Direction3D aDirection;

    // Werte eintragen
    aDirection.DirectionX = aVal.getX();
    aDirection.DirectionY = aVal.getY();
    aDirection.DirectionZ = aVal.getZ();

    rVal <<= aDirection;
    return true;
}

// -----------------------------------------------------------------------

bool SvxB3DVectorItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    drawing::Direction3D aDirection;
    if(!(rVal >>= aDirection))
        return false;

    aVal.setX(aDirection.DirectionX);
    aVal.setY(aDirection.DirectionY);
    aVal.setZ(aDirection.DirectionZ);
    return true;
}

// -----------------------------------------------------------------------

sal_uInt16 SvxB3DVectorItem::GetVersion (sal_uInt16 nFileFormatVersion) const
{
    return (nFileFormatVersion == SOFFICE_FILEFORMAT_31) ? USHRT_MAX : 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
