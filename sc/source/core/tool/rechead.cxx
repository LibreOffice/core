/*************************************************************************
 *
 *  $RCSfile: rechead.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include <tools/debug.hxx>

#include "rechead.hxx"
#include "scerrors.hxx"

// STATIC DATA -----------------------------------------------------------

// =======================================================================

ScReadHeader::ScReadHeader(SvStream& rNewStream) :
    rStream( rNewStream )
{
    ULONG nDataSize;
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

ScWriteHeader::ScWriteHeader(SvStream& rNewStream, ULONG nDefault) :
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
        rStream.Seek(nDataPos - sizeof(ULONG));
        rStream << nDataSize;                       // Groesse am Anfang eintragen
        rStream.Seek(nPos);
    }
}

// =======================================================================

ScMultipleReadHeader::ScMultipleReadHeader(SvStream& rNewStream) :
    rStream( rNewStream )
{
    ULONG nDataSize;
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
        ULONG nSizeTableLen;
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
    delete pBuf;

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
    ULONG nEntrySize;
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

ScMultipleWriteHeader::ScMultipleWriteHeader(SvStream& rNewStream, ULONG nDefault) :
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
    rStream << aMemStream.Tell();
    rStream.Write( aMemStream.GetData(), aMemStream.Tell() );

    if ( nDataEnd - nDataPos != nDataSize )                 // Default getroffen?
    {
        nDataSize = nDataEnd - nDataPos;
        ULONG nPos = rStream.Tell();
        rStream.Seek(nDataPos-sizeof(ULONG));
        rStream << nDataSize;                               // Groesse am Anfang eintragen
        rStream.Seek(nPos);
    }
}

void ScMultipleWriteHeader::EndEntry()
{
    ULONG nPos = rStream.Tell();
    aMemStream << nPos - nEntryStart;
}

void ScMultipleWriteHeader::StartEntry()
{
    ULONG nPos = rStream.Tell();
    nEntryStart = nPos;
}





