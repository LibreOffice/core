/*************************************************************************
 *
 *  $RCSfile: numhead.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:03 $
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

#pragma hdrstop

#ifndef _DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif

#include "numhead.hxx"

//      ID's fuer Dateien:
#define SV_NUMID_SIZES                      0x4200

// STATIC DATA -----------------------------------------------------------

//SEG_EOFGLOBALS()

// =======================================================================
/*                              wird fuer SvNumberformatter nicht gebraucht
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
        rStream.Seek(nDataEnd);                     // Rest ueberspringen
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

    if ( nPos - nDataPos != nDataSize )             // Default getroffen?
    {
        nDataSize = nPos - nDataPos;
        rStream.Seek(nDataPos - sizeof(ULONG));
        rStream << nDataSize;                       // Groesse am Anfang eintragen
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
    ULONG nDataSize;
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
    ULONG nSizeTableLen;
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
    DBG_ASSERT( pMemStream->Tell() == pMemStream->GetSize(),
                "Sizes nicht vollstaendig gelesen" );
    delete pMemStream;
    delete pBuf;

    rStream.Seek(nEndPos);
}

//! mit ctor synchron
// static
void ImpSvNumMultipleReadHeader::Skip( SvStream& rStream )
{
    ULONG nDataSize;
    rStream >> nDataSize;
    rStream.SeekRel( nDataSize );
    USHORT nID;
    rStream >> nID;
    if ( nID != SV_NUMID_SIZES )
    {
        DBG_ERROR("SV_NUMID_SIZES nicht gefunden");
    }
    ULONG nSizeTableLen;
    rStream >> nSizeTableLen;
    rStream.SeekRel( nSizeTableLen );
}

//#pragma SEG_FUNCDEF(numhead_08)

void ImpSvNumMultipleReadHeader::EndEntry()
{
    ULONG nPos = rStream.Tell();
    DBG_ASSERT( nPos <= nEntryEnd, "zuviel gelesen" );
    if ( nPos != nEntryEnd )
        rStream.Seek( nEntryEnd );          // Rest ueberspringen
}

//#pragma SEG_FUNCDEF(numhead_0d)

void ImpSvNumMultipleReadHeader::StartEntry()
{
    ULONG nPos = rStream.Tell();
    ULONG nEntrySize;
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

//#pragma SEG_FUNCDEF(numhead_0c)

void ImpSvNumMultipleWriteHeader::EndEntry()
{
    ULONG nPos = rStream.Tell();
    aMemStream << nPos - nEntryStart;
}

//#pragma SEG_FUNCDEF(numhead_0e)

void ImpSvNumMultipleWriteHeader::StartEntry()
{
    ULONG nPos = rStream.Tell();
    nEntryStart = nPos;
}


/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.9  2000/09/18 14:13:40  willem.vandorp
    OpenOffice header added.

    Revision 1.8  2000/08/31 13:41:32  willem.vandorp
    Header and footer replaced

    Revision 1.7  1999/04/06 15:24:24  ER
    #64244# add: static ImpSvNumMultipleReadHeader Skip


      Rev 1.6   06 Apr 1999 17:24:24   ER
   #64244# add: static ImpSvNumMultipleReadHeader Skip

      Rev 1.5   17 Oct 1997 14:31:08   ER
   new: Keyword NNN, ,- ; Datumformate System/DIN, #.##0,-- ; Comments

      Rev 1.4   30 Jul 1997 12:54:20   ER
   onSave: StoreSYSTEM, onLoad: ConvertSystem; Header aufgeraeumt

      Rev 1.3   22 Feb 1996 11:36:26   STE
   imp imp imp

      Rev 1.2   26 Jul 1995 19:35:14   STE
   International

      Rev 1.1   05 Jul 1995 17:04:42   STE
   die ersten beiden Klassen auskommentiert

      Rev 1.0   05 Jul 1995 16:28:20   STE
   Initial revision.


------------------------------------------------------------------------*/



