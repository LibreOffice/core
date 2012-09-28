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

#include "rechead.hxx"
#include "scerrors.hxx"

// STATIC DATA -----------------------------------------------------------

// =======================================================================

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
        OSL_FAIL("SCID_SIZES nicht gefunden");
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
        OSL_FAIL( "Sizes nicht vollstaendig gelesen" );
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
    OSL_ENSURE( nPos <= nEntryEnd, "zuviel gelesen" );
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
    OSL_ENSURE( nEntryEnd <= nTotalEnd, "zuviele Eintraege gelesen" );
}

sal_uLong ScMultipleReadHeader::BytesLeft() const
{
    sal_uLong nReadEnd = rStream.Tell();
    if (nReadEnd <= nEntryEnd)
        return nEntryEnd-nReadEnd;

    OSL_FAIL("Fehler bei ScMultipleReadHeader::BytesLeft");
    return 0;
}

// -----------------------------------------------------------------------

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
