/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: numhead.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2006-03-16 13:06:19 $
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

#ifndef NF_NUMHEAD_HXX
#define NF_NUMHEAD_HXX

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

// -----------------------------------------------------------------------

        //  "Automatischer" Record-Header mit Groessenangabe

/*                      wird fuer SvNumberFormatter nicht gebraucht
class SvNumReadHeader
{
private:
    SvStream&   rStream;
    ULONG       nDataEnd;

public:
    SvNumReadHeader(SvStream& rNewStream);
    ~SvNumReadHeader();

    ULONG   BytesLeft() const;
};

class SvNumWriteHeader
{
private:
    SvStream&   rStream;
    ULONG       nDataPos;
    ULONG       nDataSize;

public:
    SvNumWriteHeader(SvStream& rNewStream, ULONG nDefault = 0);
    ~SvNumWriteHeader();
};

*/

        //  Header mit Groessenangaben fuer mehrere Objekte

class ImpSvNumMultipleReadHeader
{
private:
    SvStream&       rStream;
    char*           pBuf;
    SvMemoryStream* pMemStream;
    ULONG           nEndPos;
    ULONG           nEntryEnd;

public:
    ImpSvNumMultipleReadHeader(SvStream& rNewStream);
    ~ImpSvNumMultipleReadHeader();

    void    StartEntry();
    void    EndEntry();
    ULONG   BytesLeft() const;

    static void Skip( SvStream& );      // komplett ueberspringen
};

class ImpSvNumMultipleWriteHeader
{
private:
    SvStream&       rStream;
    SvMemoryStream  aMemStream;
    ULONG           nDataPos;
    sal_uInt32      nDataSize;
    ULONG           nEntryStart;

public:
    ImpSvNumMultipleWriteHeader(SvStream& rNewStream, ULONG nDefault = 0);
    ~ImpSvNumMultipleWriteHeader();

    void    StartEntry();
    void    EndEntry();
};

#endif


