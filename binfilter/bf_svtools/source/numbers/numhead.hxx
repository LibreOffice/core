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

#ifndef NF_NUMHEAD_HXX
#define NF_NUMHEAD_HXX

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

namespace binfilter
{

// -----------------------------------------------------------------------

        //	"Automatischer" Record-Header mit Groessenangabe

/* 						wird fuer SvNumberFormatter nicht gebraucht
class SvNumReadHeader
{
private:
    SvStream&	rStream;
    ULONG		nDataEnd;

public:
    SvNumReadHeader(SvStream& rNewStream);
    ~SvNumReadHeader();

    ULONG	BytesLeft() const;
};

class SvNumWriteHeader
{
private:
    SvStream&	rStream;
    ULONG		nDataPos;
    ULONG		nDataSize;

public:
    SvNumWriteHeader(SvStream& rNewStream, ULONG nDefault = 0);
    ~SvNumWriteHeader();
};

*/

        //	Header mit Groessenangaben fuer mehrere Objekte

class ImpSvNumMultipleReadHeader
{
private:
    SvStream&		rStream;
    char*			pBuf;
    SvMemoryStream*	pMemStream;
    ULONG			nEndPos;
    ULONG			nEntryEnd;

public:
    ImpSvNumMultipleReadHeader(SvStream& rNewStream);
    ~ImpSvNumMultipleReadHeader();

    void	StartEntry();
    void	EndEntry();
    ULONG	BytesLeft() const;

    static void Skip( SvStream& );		// komplett ueberspringen
};

class ImpSvNumMultipleWriteHeader
{
private:
    SvStream&		rStream;
    SvMemoryStream	aMemStream;
    ULONG			nDataPos;
    sal_uInt32		nDataSize;
    ULONG			nEntryStart;

public:
    ImpSvNumMultipleWriteHeader(SvStream& rNewStream, ULONG nDefault = 0);
    ~ImpSvNumMultipleWriteHeader();

    void	StartEntry();
    void	EndEntry();
};

}

#endif


