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

// STATIC DATA

ScMultipleReadHeader::ScMultipleReadHeader(SvStream& rNewStream) :
    rStream( rNewStream )
{
    sal_uInt32 nDataSize;
    rStream >> nDataSize;
    sal_uLong nDataPos = rStream.Tell();
    nTotalEnd = nDataPos + nDataSize;
    nEntryEnd = nTotalEnd;

    rStream.SeekRel(nDataSize);
    sal_uInt16 nID;
    rStream >> nID;
    if (nID != SCID_SIZES)
    {
        OSL_FAIL("SCID_SIZES not found");
        if ( rStream.GetError() == SVSTREAM_OK )
            rStream.SetError( SVSTREAM_FILEFORMAT_ERROR );

        //  everything to 0, so  BytesLeft() aborts at least
        pBuf = NULL; pMemStream = NULL;
        nEntryEnd = nDataPos;
    }
    else
    {
        sal_uInt32 nSizeTableLen;
        rStream >> nSizeTableLen;
        pBuf = new sal_uInt8[nSizeTableLen];
        rStream.Read( pBuf, nSizeTableLen );
        pMemStream = new SvMemoryStream( (char*)pBuf, nSizeTableLen, STREAM_READ );
    }

    nEndPos = rStream.Tell();
    rStream.Seek( nDataPos );
}

ScMultipleReadHeader::~ScMultipleReadHeader()
{
    if ( pMemStream && pMemStream->Tell() != pMemStream->GetEndOfData() )
    {
        OSL_FAIL( "Sizes not fully read" );
        if ( rStream.GetError() == SVSTREAM_OK )
            rStream.SetError( SCWARN_IMPORT_INFOLOST );
    }
    delete pMemStream;
    delete[] pBuf;

    rStream.Seek(nEndPos);
}

void ScMultipleReadHeader::EndEntry()
{
    sal_uLong nPos = rStream.Tell();
    OSL_ENSURE( nPos <= nEntryEnd, "read too much" );
    if ( nPos != nEntryEnd )
    {
        if ( rStream.GetError() == SVSTREAM_OK )
            rStream.SetError( SCWARN_IMPORT_INFOLOST );
        rStream.Seek( nEntryEnd );          // ignore the rest
    }

    nEntryEnd = nTotalEnd;          // all remaining, if no StartEntry follows
}

void ScMultipleReadHeader::StartEntry()
{
    sal_uLong nPos = rStream.Tell();
    sal_uInt32 nEntrySize;
    (*pMemStream) >> nEntrySize;

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

ScMultipleWriteHeader::ScMultipleWriteHeader(SvStream& rNewStream, sal_uInt32 nDefault) :
    rStream( rNewStream ),
    aMemStream( 4096, 4096 )
{
    nDataSize = nDefault;
    rStream << nDataSize;

    nDataPos = rStream.Tell();
    nEntryStart = nDataPos;
}

ScMultipleWriteHeader::~ScMultipleWriteHeader()
{
    sal_uLong nDataEnd = rStream.Tell();

    rStream << (sal_uInt16) SCID_SIZES;
    rStream << static_cast<sal_uInt32>(aMemStream.Tell());
    rStream.Write( aMemStream.GetData(), aMemStream.Tell() );

    if ( nDataEnd - nDataPos != nDataSize )                 // matched default ?
    {
        nDataSize = nDataEnd - nDataPos;
        sal_uLong nPos = rStream.Tell();
        rStream.Seek(nDataPos-sizeof(sal_uInt32));
        rStream << nDataSize;                               // record size at the beginning
        rStream.Seek(nPos);
    }
}

void ScMultipleWriteHeader::EndEntry()
{
    sal_uLong nPos = rStream.Tell();
    aMemStream << static_cast<sal_uInt32>(nPos - nEntryStart);
}

void ScMultipleWriteHeader::StartEntry()
{
    sal_uLong nPos = rStream.Tell();
    nEntryStart = nPos;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
