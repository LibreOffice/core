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

#ifndef SC_RECHEAD_HXX
#define SC_RECHEAD_HXX

#include <tools/stream.hxx>

//      ID's for files:

#define SCID_SIZES          0x4200

#define SCID_POOLS          0x4210
#define SCID_DOCPOOL        0x4211
#define SCID_STYLEPOOL      0x4212
#define SCID_NEWPOOLS       0x4213      // ID for SC 3.0a
#define SCID_EDITPOOL       0x4214

#define SCID_DOCUMENT       0x4220
#define SCID_DOCFLAGS       0x4221
#define SCID_TABLE          0x4222
#define SCID_DRAWING        0x4223
#define SCID_RANGENAME      0x4224
#define SCID_DBAREAS        0x4225
#define SCID_PIVOT          0x4226
#define SCID_CHARTS         0x4227
#define SCID_NUMFORMAT      0x4228
#define SCID_DOCOPTIONS     0x4229
#define SCID_VIEWOPTIONS    0x422a
#define SCID_PRINTSETUP     0x422b
#define SCID_CHARSET        0x422c
#define SCID_NEWDOCUMENT    0x422d      // ID for SC 3.0a
#define SCID_DDELINKS       0x422e
#define SCID_AREALINKS      0x422f
#define SCID_CONDFORMATS    0x4230
#define SCID_VALIDATION     0x4231
#define SCID_COLNAMERANGES  0x4232
#define SCID_ROWNAMERANGES  0x4233
#define SCID_DETOPLIST      0x4234
#define SCID_CONSOLIDATA    0x4235
#define SCID_CHANGETRACK    0x4236
#define SCID_CHGVIEWSET     0x4237
#define SCID_LINKUPMODE     0x4238
#define SCID_DATAPILOT      0x4239

#define SCID_COLUMNS        0x4240
#define SCID_COLROWFLAGS    0x4241
#define SCID_TABOPTIONS     0x4242
#define SCID_TABLINK        0x4243

#define SCID_COLDATA        0x4250
#define SCID_COLNOTES       0x4251
#define SCID_COLATTRIB      0x4252

#define SCID_DRAWPOOL       0x4260
#define SCID_DRAWMODEL      0x4261

//  file version
//  if the high-byte is incremented, the document won't be opened by older SCs
#define SC_INITIAL_VERSION  0x0001
#define SC_FORMULA_LCLVER   0x0002          // formula with local version no.

#define SC_NEW_TOKEN_ARRAYS 0x0003          // new TokenArray format
#define SC_FORMULA_VALUES   0x0004          // values in formula cells
#define SC_FORMULA_VALUES2  0x0005          // values in formula cells
#define SC_DATABYTES        0x0006          // data bytes, small tables
#define SC_DATABYTES2       0x0007          // data bytes, small tables
#define SC_NUMFMT           0x0008          // number format of formula cell
#define SC_NEWIF            0x0009          // new coding of ocIf (komp.)

#define SC_RELATIVE_REFS    0x0010          // relative references
#define SC_SUBTOTAL_FLAG    0x0011          // bSubTotal of formula cell
#define SC_COLROWNAME_RANGEPAIR 0x0012      // ColRowNameRanges as ScRangePair

#define SC_31_EXPORT_VER    0x0012          // version for 3.1-export
// since 4.0
#define SC_32K_ROWS         0x0100          // 32000 rows - incompatibel
#define SC_FONTCHARSET      0x0101          // Font-CharSets have to be right

#define SC_40_EXPORT_VER    0x0101          // version for 4.0-Export
// since 5.0
#define SC_RECALC_MODE_BITS 0x0201          // TokenArray RecalcMode
#define SC_MATRIX_DOUBLEREF 0x0202          // DoubleRef implicit intersection
#define SC_VERSION_EDITPOOL 0x0203          // EditCells with EditPool
#define SC_SUBTOTAL_BUGFIX  0x0204          // bSubTotal of formula cell (true)
// since 5.2
#define SC_CONVERT_RECALC_ON_LOAD 0x0205    // CONVERT function recalculated on each load
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
