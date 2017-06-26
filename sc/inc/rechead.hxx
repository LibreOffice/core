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

#ifndef INCLUDED_SC_INC_RECHEAD_HXX
#define INCLUDED_SC_INC_RECHEAD_HXX

#include <tools/stream.hxx>
#include <memory>

//      ID's for files:

#define SCID_SIZES          0x4200


#define SC_31_EXPORT_VER    0x0012          // version for 3.1-export
// since 4.0
#define SC_FONTCHARSET      0x0101          // Font-CharSets have to be right

// since 5.0
#define SC_MATRIX_DOUBLEREF 0x0202          // DoubleRef implicit intersection
// since 5.2
#define SC_CURRENT_VERSION  0x0205

// all above SC_31_EXPORT_VER has to be queried during saving,
// because 3.1-export writes this version number.

// btw: 10 following 09 is not a counting error but an intentional gap,
// because it was not clear, how long the RelRefs development would take. :)

class SvStream;

        //  Header with size specification for multiple objects

class ScMultipleReadHeader
{
private:
    SvStream&           rStream;
    std::unique_ptr<sal_uInt8[]>
                        pBuf;
    SvMemoryStream*     pMemStream;
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
    sal_uLong       nDataPos;
    sal_uInt32      nDataSize;
    sal_uLong       nEntryStart;

public:
    ScMultipleWriteHeader(SvStream& rNewStream);
    ~ScMultipleWriteHeader();

    void    StartEntry();
    void    EndEntry();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
