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

#ifndef SC_RECHEAD_HXX
#define SC_RECHEAD_HXX

#include <tools/stream.hxx>

//      ID's for files:

#define SCID_SIZES          0x4200

//-------------------------------- since 4.0
#define SC_FONTCHARSET      0x0101          // Font-CharSets have to be right
//-------------------------------- since 5.0
#define SC_MATRIX_DOUBLEREF 0x0202          // DoubleRef implicit intersection
//--------------------------------
#define SC_CURRENT_VERSION  0x0205


// all above SC_31_EXPORT_VER has to be queried during saving,
// because 3.1-export writes this version number.

// btw: 10 following 09 is not a counting error but an intentional gap,
// because it was not clear, how long the RelRefs development would take. :)

class SvStream;

// -----------------------------------------------------------------------

        //  Header with size specification for multiple objects

class ScMultipleReadHeader
{
private:
    SvStream&       rStream;
    sal_uInt8*          pBuf;
    SvMemoryStream* pMemStream;
    sal_uLong           nEndPos;
    sal_uLong           nEntryEnd;
    sal_uLong           nTotalEnd;

public:
    ScMultipleReadHeader(SvStream& rNewStream);
    ~ScMultipleReadHeader();

    void    StartEntry();
    void    EndEntry();
    sal_uLong   BytesLeft() const;
};

class ScMultipleWriteHeader
{
private:
    SvStream&       rStream;
    SvMemoryStream  aMemStream;
    sal_uLong           nDataPos;
    sal_uInt32      nDataSize;
    sal_uLong           nEntryStart;

public:
    ScMultipleWriteHeader(SvStream& rNewStream, sal_uInt32 nDefault = 0);
    ~ScMultipleWriteHeader();

    void    StartEntry();
    void    EndEntry();
};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
