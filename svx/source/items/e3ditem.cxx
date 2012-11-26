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
#include <com/sun/star/drawing/Direction3D.hpp>
#include <tools/stream.hxx>

#include <svx/e3ditem.hxx>

using namespace ::rtl;
using namespace ::com::sun::star;

// STATIC DATA -----------------------------------------------------------

SvxB3DVectorItem::SvxB3DVectorItem()
{
}

SvxB3DVectorItem::~SvxB3DVectorItem()
{
}

// -----------------------------------------------------------------------

SvxB3DVectorItem::SvxB3DVectorItem( sal_uInt16 _nWhich, const basegfx::B3DVector& rVal ) :
    SfxPoolItem( _nWhich ),
    aVal( rVal )
{
}

// -----------------------------------------------------------------------

SvxB3DVectorItem::SvxB3DVectorItem( sal_uInt16 _nWhich, SvStream& rStream ) :
    SfxPoolItem( _nWhich )
{
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
}

// -----------------------------------------------------------------------

int SvxB3DVectorItem::operator==( const SfxPoolItem &rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
    return ((SvxB3DVectorItem&)rItem).aVal == aVal;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxB3DVectorItem::Clone( SfxItemPool* /*pPool*/ ) const
{
    return new SvxB3DVectorItem( *this );
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxB3DVectorItem::Create(SvStream &rStream, sal_uInt16 /*nVersion*/) const
{
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
    // ## if (nItemVersion)
    double fValue;
    fValue = aVal.getX(); rStream << fValue;
    fValue = aVal.getY(); rStream << fValue;
    fValue = aVal.getZ(); rStream << fValue;

    return rStream;
}

// -----------------------------------------------------------------------

sal_Bool SvxB3DVectorItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
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

sal_Bool SvxB3DVectorItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
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

sal_uInt16 SvxB3DVectorItem::GetVersion (sal_uInt16 nFileFormatVersion) const
{
    return (nFileFormatVersion == SOFFICE_FILEFORMAT_31) ? USHRT_MAX : 0;
}

// eof
