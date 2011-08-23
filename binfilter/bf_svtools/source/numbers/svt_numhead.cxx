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

// MARKER(update_precomp.py): autogen include statement, do not remove

#include <tools/debug.hxx>

#include "numhead.hxx"

//		ID's fuer Dateien:
#define SV_NUMID_SIZES						0x4200

namespace binfilter
{

// STATIC DATA -----------------------------------------------------------

//SEG_EOFGLOBALS()

// =======================================================================
/*								wird fuer SvNumberformatter nicht gebraucht
//#pragma SEG_FUNCDEF(numhead_01)

SvNumReadHeader::SvNumReadHeader(SvStream& rNewStream) :
    rStream( rNewStream )
{
    ULONG nDataSize;
    rStream >> nDataSize;
    nDataEnd = rStream.Tell() + nDataSize;
}

//#pragma SEG_FUNCDEF(numhead_02)

SvNumReadHeader::~SvNumReadHeader()
{
    ULONG nReadEnd = rStream.Tell();
    DBG_ASSERT( nReadEnd <= nDataEnd, "zuviele Bytes gelesen" );
    if ( nReadEnd != nDataEnd )
        rStream.Seek(nDataEnd); 					// Rest ueberspringen
}

//#pragma SEG_FUNCDEF(numhead_03)

ULONG SvNumReadHeader::BytesLeft() const
{
    ULONG nReadEnd = rStream.Tell();
    if (nReadEnd <= nDataEnd)
        return nDataEnd-nReadEnd;

    DBG_ERROR("Fehler bei SvNumReadHeader::BytesLeft");
    return 0;
}

// -----------------------------------------------------------------------

//#pragma SEG_FUNCDEF(numhead_04)

SvNumWriteHeader::SvNumWriteHeader(SvStream& rNewStream, ULONG nDefault) :
    rStream( rNewStream )
{
    nDataSize = nDefault;
    rStream << nDataSize;
    nDataPos = rStream.Tell();
}

//#pragma SEG_FUNCDEF(numhead_05)

SvNumWriteHeader::~SvNumWriteHeader()
{
    ULONG nPos = rStream.Tell();

    if ( nPos - nDataPos != nDataSize )				// Default getroffen?
    {
        nDataSize = nPos - nDataPos;
        rStream.Seek(nDataPos - sizeof(sal_uInt32));
        rStream << nDataSize;						// Groesse am Anfang eintragen
        rStream.Seek(nPos);
    }
}
*/

// =======================================================================

//#pragma SEG_FUNCDEF(numhead_06)

//! mit Skip() synchron
ImpSvNumMultipleReadHeader::ImpSvNumMultipleReadHeader(SvStream& rNewStream) :
    rStream( rNewStream )
{
    sal_uInt32 nDataSize;
    rStream >> nDataSize;
    ULONG nDataPos = rStream.Tell();
    nEntryEnd = nDataPos;

    rStream.SeekRel(nDataSize);
    USHORT nID;
    rStream >> nID;
    if (nID != SV_NUMID_SIZES)
    {
        DBG_ERROR("SV_NUMID_SIZES nicht gefunden");
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

//! mit ctor synchron
// static
void ImpSvNumMultipleReadHeader::Skip( SvStream& rStream )
{
    sal_uInt32 nDataSize;
    rStream >> nDataSize;
    rStream.SeekRel( nDataSize );
    USHORT nID;
    rStream >> nID;
    if ( nID != SV_NUMID_SIZES )
    {
        DBG_ERROR("SV_NUMID_SIZES nicht gefunden");
    }
    sal_uInt32 nSizeTableLen;
    rStream >> nSizeTableLen;
    rStream.SeekRel( nSizeTableLen );
}

//#pragma SEG_FUNCDEF(numhead_08)

void ImpSvNumMultipleReadHeader::EndEntry()
{
    ULONG nPos = rStream.Tell();
    DBG_ASSERT( nPos <= nEntryEnd, "zuviel gelesen" );
    if ( nPos != nEntryEnd )
        rStream.Seek( nEntryEnd );			// Rest ueberspringen
}

//#pragma SEG_FUNCDEF(numhead_0d)

void ImpSvNumMultipleReadHeader::StartEntry()
{
    ULONG nPos = rStream.Tell();
    sal_uInt32 nEntrySize;
    (*pMemStream) >> nEntrySize;

    nEntryEnd = nPos + nEntrySize;
}

//#pragma SEG_FUNCDEF(numhead_09)

ULONG ImpSvNumMultipleReadHeader::BytesLeft() const
{
    ULONG nReadEnd = rStream.Tell();
    if (nReadEnd <= nEntryEnd)
        return nEntryEnd-nReadEnd;

    DBG_ERROR("Fehler bei ImpSvNumMultipleReadHeader::BytesLeft");
    return 0;
}

// -----------------------------------------------------------------------

//#pragma SEG_FUNCDEF(numhead_0a)

ImpSvNumMultipleWriteHeader::ImpSvNumMultipleWriteHeader(SvStream& rNewStream,
                                                   ULONG nDefault) :
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
    ULONG nDataEnd = rStream.Tell();

    rStream << (USHORT) SV_NUMID_SIZES;
    rStream << static_cast<sal_uInt32>(aMemStream.Tell());
    rStream.Write( aMemStream.GetData(), aMemStream.Tell() );

    if ( nDataEnd - nDataPos != nDataSize )					// Default getroffen?
    {
        nDataSize = nDataEnd - nDataPos;
        ULONG nPos = rStream.Tell();
        rStream.Seek(nDataPos-sizeof(sal_uInt32));
        rStream << nDataSize;								// Groesse am Anfang eintragen
        rStream.Seek(nPos);
    }
}

//#pragma SEG_FUNCDEF(numhead_0c)

void ImpSvNumMultipleWriteHeader::EndEntry()
{
    ULONG nPos = rStream.Tell();
    aMemStream << static_cast<sal_uInt32>(nPos - nEntryStart);
}

//#pragma SEG_FUNCDEF(numhead_0e)

void ImpSvNumMultipleWriteHeader::StartEntry()
{
    ULONG nPos = rStream.Tell();
    nEntryStart = nPos;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
