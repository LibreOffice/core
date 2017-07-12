/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "rechead.hxx"
#include "scerrors.hxx"

#include <osl/diagnose.h>

ScMultipleReadHeader::ScMultipleReadHeader(SvStream& rNewStream) :
    rStream( rNewStream )
{
    sal_uInt32 nDataSize;
    rStream.ReadUInt32( nDataSize );
    sal_uLong nDataPos = rStream.Tell();
    nTotalEnd = nDataPos + nDataSize;
    nEntryEnd = nTotalEnd;

    rStream.SeekRel(nDataSize);
    sal_uInt16 nID;
    rStream.ReadUInt16( nID );
    if (nID != SCID_SIZES)
    {
        OSL_FAIL("SCID_SIZES not found");
        if ( rStream.GetError() == ERRCODE_NONE )
            rStream.SetError( SVSTREAM_FILEFORMAT_ERROR );

        //  everything to 0, so  BytesLeft() aborts at least
        pBuf = nullptr; pMemStream = nullptr;
        nEntryEnd = nDataPos;
    }
    else
    {
        sal_uInt32 nSizeTableLen;
        rStream.ReadUInt32( nSizeTableLen );
        pBuf.reset( new sal_uInt8[nSizeTableLen] );
        rStream.ReadBytes( pBuf.get(), nSizeTableLen );
        pMemStream = new SvMemoryStream( pBuf.get(), nSizeTableLen, StreamMode::READ );
    }

    nEndPos = rStream.Tell();
    rStream.Seek( nDataPos );
}

ScMultipleReadHeader::~ScMultipleReadHeader()
{
    if ( pMemStream && pMemStream->Tell() != pMemStream->GetEndOfData() )
    {
        OSL_FAIL( "Sizes not fully read" );
        if ( rStream.GetError() == ERRCODE_NONE )
            rStream.SetError( SCWARN_IMPORT_INFOLOST );
    }
    delete pMemStream;

    rStream.Seek(nEndPos);
}

void ScMultipleReadHeader::EndEntry()
{
    sal_uLong nPos = rStream.Tell();
    OSL_ENSURE( nPos <= nEntryEnd, "read too much" );
    if ( nPos != nEntryEnd )
    {
        if ( rStream.GetError() == ERRCODE_NONE )
            rStream.SetError( SCWARN_IMPORT_INFOLOST );
        rStream.Seek( nEntryEnd );          // ignore the rest
    }

    nEntryEnd = nTotalEnd;          // all remaining, if no StartEntry follows
}

void ScMultipleReadHeader::StartEntry()
{
    sal_uLong nPos = rStream.Tell();
    sal_uInt32 nEntrySize;
    (*pMemStream).ReadUInt32( nEntrySize );

    nEntryEnd = nPos + nEntrySize;
    OSL_ENSURE( nEntryEnd <= nTotalEnd, "read too many entries" );
}

sal_uLong ScMultipleReadHeader::BytesLeft() const
{
    sal_uLong nReadEnd = rStream.Tell();
    if (nReadEnd <= nEntryEnd)
        return nEntryEnd-nReadEnd;

    OSL_FAIL("ScMultipleReadHeader::BytesLeft: Error");
    return 0;
}

ScMultipleWriteHeader::ScMultipleWriteHeader(SvStream& rNewStream) :
    rStream( rNewStream ),
    aMemStream( 4096, 4096 )
{
    nDataSize = 0;
    rStream.WriteUInt32( nDataSize );

    nDataPos = rStream.Tell();
    nEntryStart = nDataPos;
}

ScMultipleWriteHeader::~ScMultipleWriteHeader()
{
    sal_uLong nDataEnd = rStream.Tell();

    rStream.WriteUInt16( SCID_SIZES );
    rStream.WriteUInt32( aMemStream.Tell() );
    rStream.WriteBytes( aMemStream.GetData(), aMemStream.Tell() );

    if ( nDataEnd - nDataPos != nDataSize )                 // matched default ?
    {
        nDataSize = nDataEnd - nDataPos;
        sal_uLong nPos = rStream.Tell();
        rStream.Seek(nDataPos-sizeof(sal_uInt32));
        rStream.WriteUInt32( nDataSize );                               // record size at the beginning
        rStream.Seek(nPos);
    }
}

void ScMultipleWriteHeader::EndEntry()
{
    sal_uLong nPos = rStream.Tell();
    aMemStream.WriteUInt32( nPos - nEntryStart );
}

void ScMultipleWriteHeader::StartEntry()
{
    sal_uLong nPos = rStream.Tell();
    nEntryStart = nPos;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
