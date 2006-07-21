/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rechead.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 11:42:09 $
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
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include <tools/debug.hxx>

#include "rechead.hxx"
#include "scerrors.hxx"

// STATIC DATA -----------------------------------------------------------

// =======================================================================

ScReadHeader::ScReadHeader(SvStream& rNewStream) :
    rStream( rNewStream )
{
    sal_uInt32 nDataSize;
    rStream >> nDataSize;
    nDataEnd = rStream.Tell() + nDataSize;
}

ScReadHeader::~ScReadHeader()
{
    ULONG nReadEnd = rStream.Tell();
    DBG_ASSERT( nReadEnd <= nDataEnd, "zuviele Bytes gelesen" );
    if ( nReadEnd != nDataEnd )
    {
        if ( rStream.GetError() == SVSTREAM_OK )
            rStream.SetError( SCWARN_IMPORT_INFOLOST );
        rStream.Seek(nDataEnd);                     // Rest ueberspringen
    }
}

ULONG ScReadHeader::BytesLeft() const
{
    ULONG nReadEnd = rStream.Tell();
    if (nReadEnd <= nDataEnd)
        return nDataEnd-nReadEnd;

    DBG_ERROR("Fehler bei ScReadHeader::BytesLeft");
    return 0;
}

// -----------------------------------------------------------------------

ScWriteHeader::ScWriteHeader(SvStream& rNewStream, sal_uInt32 nDefault) :
    rStream( rNewStream )
{
    nDataSize = nDefault;
    rStream << nDataSize;

    nDataPos = rStream.Tell();
}

ScWriteHeader::~ScWriteHeader()
{
    ULONG nPos = rStream.Tell();

    if ( nPos - nDataPos != nDataSize )             // Default getroffen?
    {
        nDataSize = nPos - nDataPos;
        rStream.Seek(nDataPos - sizeof(sal_uInt32));
        rStream << nDataSize;                       // Groesse am Anfang eintragen
        rStream.Seek(nPos);
    }
}

// =======================================================================

ScMultipleReadHeader::ScMultipleReadHeader(SvStream& rNewStream) :
    rStream( rNewStream )
{
    sal_uInt32 nDataSize;
    rStream >> nDataSize;
    ULONG nDataPos = rStream.Tell();
    nTotalEnd = nDataPos + nDataSize;
    nEntryEnd = nTotalEnd;

    rStream.SeekRel(nDataSize);
    USHORT nID;
    rStream >> nID;
    if (nID != SCID_SIZES)
    {
        DBG_ERROR("SCID_SIZES nicht gefunden");
        if ( rStream.GetError() == SVSTREAM_OK )
            rStream.SetError( SVSTREAM_FILEFORMAT_ERROR );

        //  alles auf 0, damit BytesLeft() wenigstens abbricht
        pBuf = NULL; pMemStream = NULL;
        nEntryEnd = nDataPos;
    }
    else
    {
        sal_uInt32 nSizeTableLen;
        rStream >> nSizeTableLen;
        pBuf = new BYTE[nSizeTableLen];
        rStream.Read( pBuf, nSizeTableLen );
        pMemStream = new SvMemoryStream( (char*)pBuf, nSizeTableLen, STREAM_READ );
    }

    nEndPos = rStream.Tell();
    rStream.Seek( nDataPos );
}

ScMultipleReadHeader::~ScMultipleReadHeader()
{
    if ( pMemStream && pMemStream->Tell() != pMemStream->GetSize() )
    {
        DBG_ERRORFILE( "Sizes nicht vollstaendig gelesen" );
        if ( rStream.GetError() == SVSTREAM_OK )
            rStream.SetError( SCWARN_IMPORT_INFOLOST );
    }
    delete pMemStream;
    delete[] pBuf;

    rStream.Seek(nEndPos);
}

void ScMultipleReadHeader::EndEntry()
{
    ULONG nPos = rStream.Tell();
    DBG_ASSERT( nPos <= nEntryEnd, "zuviel gelesen" );
    if ( nPos != nEntryEnd )
    {
        if ( rStream.GetError() == SVSTREAM_OK )
            rStream.SetError( SCWARN_IMPORT_INFOLOST );
        rStream.Seek( nEntryEnd );          // Rest ueberspringen
    }

    nEntryEnd = nTotalEnd;          // den ganzen Rest, wenn kein StartEntry kommt
}

void ScMultipleReadHeader::StartEntry()
{
    ULONG nPos = rStream.Tell();
    sal_uInt32 nEntrySize;
    (*pMemStream) >> nEntrySize;

    nEntryEnd = nPos + nEntrySize;
    DBG_ASSERT( nEntryEnd <= nTotalEnd, "zuviele Eintraege gelesen" );
}

ULONG ScMultipleReadHeader::BytesLeft() const
{
    ULONG nReadEnd = rStream.Tell();
    if (nReadEnd <= nEntryEnd)
        return nEntryEnd-nReadEnd;

    DBG_ERROR("Fehler bei ScMultipleReadHeader::BytesLeft");
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
    ULONG nDataEnd = rStream.Tell();

    rStream << (USHORT) SCID_SIZES;
    rStream << static_cast<sal_uInt32>(aMemStream.Tell());
    rStream.Write( aMemStream.GetData(), aMemStream.Tell() );

    if ( nDataEnd - nDataPos != nDataSize )                 // Default getroffen?
    {
        nDataSize = nDataEnd - nDataPos;
        ULONG nPos = rStream.Tell();
        rStream.Seek(nDataPos-sizeof(sal_uInt32));
        rStream << nDataSize;                               // Groesse am Anfang eintragen
        rStream.Seek(nPos);
    }
}

void ScMultipleWriteHeader::EndEntry()
{
    ULONG nPos = rStream.Tell();
    aMemStream << static_cast<sal_uInt32>(nPos - nEntryStart);
}

void ScMultipleWriteHeader::StartEntry()
{
    ULONG nPos = rStream.Tell();
    nEntryStart = nPos;
}





