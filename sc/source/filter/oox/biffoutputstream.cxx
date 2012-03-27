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

#include "biffoutputstream.hxx"

namespace oox {
namespace xls {

// ============================================================================

namespace prv {

BiffOutputRecordBuffer::BiffOutputRecordBuffer( BinaryOutputStream& rOutStrm, sal_uInt16 nMaxRecSize ) :
    mrOutStrm( rOutStrm ),
    mnMaxRecSize( nMaxRecSize ),
    mnRecId( BIFF_ID_UNKNOWN ),
    mbInRec( false )
{
    OSL_ENSURE( mrOutStrm.isSeekable(), "BiffOutputRecordBuffer::BiffOutputRecordBuffer - stream must be seekable" );
    maData.reserve( SAL_MAX_UINT16 );
}

void BiffOutputRecordBuffer::startRecord( sal_uInt16 nRecId )
{
    OSL_ENSURE( !mbInRec, "BiffOutputRecordBuffer::startRecord - another record still open" );
    mnRecId = nRecId;
    maData.clear();
    mbInRec = true;
}

void BiffOutputRecordBuffer::endRecord()
{
    OSL_ENSURE( mbInRec, "BiffOutputRecordBuffer::endRecord - no record open" );
    sal_uInt16 nRecSize = getLimitedValue< sal_uInt16, size_t >( maData.size(), 0, SAL_MAX_UINT16 );
    mrOutStrm.seekToEnd();
    mrOutStrm << mnRecId << nRecSize;
    if( nRecSize > 0 )
        mrOutStrm.writeMemory( &maData.front(), nRecSize );
    mbInRec = false;
}

void BiffOutputRecordBuffer::write( const void* pData, sal_uInt16 nBytes )
{
    OSL_ENSURE( mbInRec, "BiffOutputRecordBuffer::write - no record open" );
    OSL_ENSURE( nBytes > 0, "BiffOutputRecordBuffer::write - nothing to write" );
    OSL_ENSURE( nBytes <= getRecLeft(), "BiffOutputRecordBuffer::write - buffer overflow" );
    maData.resize( maData.size() + nBytes );
    memcpy( &*(maData.end() - nBytes), pData, nBytes );
}

} // namespace prv

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
