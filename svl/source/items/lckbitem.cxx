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
#include "precompiled_svl.hxx"

#define _LCKBITEM_CXX
#include <svl/lckbitem.hxx>
#include <svl/poolitem.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <tools/cachestr.hxx>

// STATIC DATA -----------------------------------------------------------


// -----------------------------------------------------------------------

TYPEINIT1_AUTOFACTORY(SfxLockBytesItem, SfxPoolItem);

// -----------------------------------------------------------------------

SfxLockBytesItem::SfxLockBytesItem()
{
}

// -----------------------------------------------------------------------

SfxLockBytesItem::SfxLockBytesItem( sal_uInt16 nW, SvLockBytes *pLockBytes )
:   SfxPoolItem( nW ),
    _xVal( pLockBytes )
{
}

// -----------------------------------------------------------------------

SfxLockBytesItem::SfxLockBytesItem( sal_uInt16 nW, SvStream &rStream )
:   SfxPoolItem( nW )
{
    rStream.Seek( 0L );
    _xVal = new SvLockBytes( new SvCacheStream(), sal_True );

    SvStream aLockBytesStream( _xVal );
    rStream >> aLockBytesStream;
}

// -----------------------------------------------------------------------

SfxLockBytesItem::SfxLockBytesItem( const SfxLockBytesItem& rItem )
:   SfxPoolItem( rItem ),
    _xVal( rItem._xVal )
{
}

// -----------------------------------------------------------------------

SfxLockBytesItem::~SfxLockBytesItem()
{
}

// -----------------------------------------------------------------------

int SfxLockBytesItem::operator==( const SfxPoolItem& rItem ) const
{
    return ((SfxLockBytesItem&)rItem)._xVal == _xVal;
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxLockBytesItem::Clone(SfxItemPool *) const
{
    return new SfxLockBytesItem( *this );
}

// -----------------------------------------------------------------------

#define MAX_BUF 32000

SfxPoolItem* SfxLockBytesItem::Create( SvStream &rStream, sal_uInt16 ) const
{
    sal_uInt32 nSize = 0;
    sal_uLong nActRead = 0;
    sal_Char cTmpBuf[MAX_BUF];
    SvMemoryStream aNewStream;
    rStream >> nSize;

    do {
        sal_uLong nToRead;
        if( (nSize - nActRead) > MAX_BUF )
            nToRead = MAX_BUF;
        else
            nToRead = nSize - nActRead;
        nActRead += rStream.Read( cTmpBuf, nToRead );
        aNewStream.Write( cTmpBuf, nToRead );
    } while( nSize > nActRead );

    return new SfxLockBytesItem( Which(), aNewStream );
}

// -----------------------------------------------------------------------

SvStream& SfxLockBytesItem::Store(SvStream &rStream, sal_uInt16 ) const
{
    SvStream aLockBytesStream( _xVal );
    sal_uInt32 nSize = aLockBytesStream.Seek( STREAM_SEEK_TO_END );
    aLockBytesStream.Seek( 0L );

    rStream << nSize;
    rStream << aLockBytesStream;

    return rStream;
}

//----------------------------------------------------------------------------
// virtual
sal_Bool SfxLockBytesItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 )
{
    com::sun::star::uno::Sequence< sal_Int8 > aSeq;
    if ( rVal >>= aSeq )
    {
        if ( aSeq.getLength() )
        {
            SvCacheStream* pStream = new SvCacheStream;
            pStream->Write( (void*)aSeq.getConstArray(), aSeq.getLength() );
            pStream->Seek(0);

            _xVal = new SvLockBytes( pStream, sal_True );
        }
        else
            _xVal = NULL;

        return sal_True;
    }

    DBG_ERROR( "SfxLockBytesItem::PutValue - Wrong type!" );
    return sal_False;
}

//----------------------------------------------------------------------------
// virtual
sal_Bool SfxLockBytesItem::QueryValue( com::sun::star::uno::Any& rVal,sal_uInt8 ) const
{
    if ( _xVal.Is() )
    {
        sal_uInt32 nLen;
        SvLockBytesStat aStat;

        if ( _xVal->Stat( &aStat, SVSTATFLAG_DEFAULT ) == ERRCODE_NONE )
            nLen = aStat.nSize;
        else
            return sal_False;

        sal_uLong nRead = 0;
        com::sun::star::uno::Sequence< sal_Int8 > aSeq( nLen );

        _xVal->ReadAt( 0, aSeq.getArray(), nLen, &nRead );
        rVal <<= aSeq;
    }
    else
    {
        com::sun::star::uno::Sequence< sal_Int8 > aSeq( 0 );
        rVal <<= aSeq;
    }

    return sal_True;
}

