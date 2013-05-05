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

#include <tools/debug.hxx>

#include "numhead.hxx"

//      ID's fuer Dateien:
#define SV_NUMID_SIZES                      0x4200

//#pragma SEG_FUNCDEF(numhead_06)

//! mit Skip() synchron
ImpSvNumMultipleReadHeader::ImpSvNumMultipleReadHeader(SvStream& rNewStream) :
    rStream( rNewStream )
{
    sal_uInt32 nDataSize;
    rStream >> nDataSize;
    sal_uLong nDataPos = rStream.Tell();
    nEntryEnd = nDataPos;

    rStream.SeekRel(nDataSize);
    sal_uInt16 nID;
    rStream >> nID;
    if (nID != SV_NUMID_SIZES)
    {
        OSL_FAIL("SV_NUMID_SIZES nicht gefunden");
    }
    sal_uInt32 nSizeTableLen;
    rStream >> nSizeTableLen;
    pBuf = new char[nSizeTableLen];
    rStream.Read( pBuf, nSizeTableLen );
    pMemStream = new SvMemoryStream( pBuf, nSizeTableLen, STREAM_READ );

    nEndPos = rStream.Tell();
    rStream.Seek( nDataPos );
}

//#pragma SEG_FUNCDEF(numhead_07)

ImpSvNumMultipleReadHeader::~ImpSvNumMultipleReadHeader()
{
    DBG_ASSERT( pMemStream->Tell() == pMemStream->GetEndOfData(),
                "Sizes nicht vollstaendig gelesen" );
    delete pMemStream;
    delete [] pBuf;

    rStream.Seek(nEndPos);
}

//#pragma SEG_FUNCDEF(numhead_08)

void ImpSvNumMultipleReadHeader::EndEntry()
{
    sal_uLong nPos = rStream.Tell();
    DBG_ASSERT( nPos <= nEntryEnd, "zuviel gelesen" );
    if ( nPos != nEntryEnd )
        rStream.Seek( nEntryEnd );          // Rest ueberspringen
}

//#pragma SEG_FUNCDEF(numhead_0d)

void ImpSvNumMultipleReadHeader::StartEntry()
{
    sal_uLong nPos = rStream.Tell();
    sal_uInt32 nEntrySize;
    (*pMemStream) >> nEntrySize;

    nEntryEnd = nPos + nEntrySize;
}

//#pragma SEG_FUNCDEF(numhead_09)

sal_uLong ImpSvNumMultipleReadHeader::BytesLeft() const
{
    sal_uLong nReadEnd = rStream.Tell();
    if (nReadEnd <= nEntryEnd)
        return nEntryEnd-nReadEnd;

    OSL_FAIL("Fehler bei ImpSvNumMultipleReadHeader::BytesLeft");
    return 0;
}

// -----------------------------------------------------------------------

//#pragma SEG_FUNCDEF(numhead_0a)

ImpSvNumMultipleWriteHeader::ImpSvNumMultipleWriteHeader(SvStream& rNewStream,
                                                   sal_uLong nDefault) :
    rStream( rNewStream ),
    aMemStream( 4096, 4096 )
{
    nDataSize = nDefault;
    rStream << nDataSize;

    nDataPos = rStream.Tell();
    nEntryStart = nDataPos;
}

//#pragma SEG_FUNCDEF(numhead_0b)

ImpSvNumMultipleWriteHeader::~ImpSvNumMultipleWriteHeader()
{
    sal_uLong nDataEnd = rStream.Tell();

    rStream << (sal_uInt16) SV_NUMID_SIZES;
    rStream << static_cast<sal_uInt32>(aMemStream.Tell());
    rStream.Write( aMemStream.GetData(), aMemStream.Tell() );

    if ( nDataEnd - nDataPos != nDataSize )                 // Default getroffen?
    {
        nDataSize = nDataEnd - nDataPos;
        sal_uLong nPos = rStream.Tell();
        rStream.Seek(nDataPos-sizeof(sal_uInt32));
        rStream << nDataSize;                               // Groesse am Anfang eintragen
        rStream.Seek(nPos);
    }
}

//#pragma SEG_FUNCDEF(numhead_0c)

void ImpSvNumMultipleWriteHeader::EndEntry()
{
    sal_uLong nPos = rStream.Tell();
    aMemStream << static_cast<sal_uInt32>(nPos - nEntryStart);
}

//#pragma SEG_FUNCDEF(numhead_0e)

void ImpSvNumMultipleWriteHeader::StartEntry()
{
    sal_uLong nPos = rStream.Tell();
    nEntryStart = nPos;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
