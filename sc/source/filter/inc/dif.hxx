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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_DIF_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_DIF_HXX

#include <map>
#include <memory>
#include <vector>

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

#include <types.hxx>

class SvStream;
class SvNumberFormatter;
class ScDocument;

extern const sal_Unicode pKeyTABLE[];
extern const sal_Unicode pKeyVECTORS[];
extern const sal_Unicode pKeyTUPLES[];
extern const sal_Unicode pKeyDATA[];
extern const sal_Unicode pKeyBOT[];
extern const sal_Unicode pKeyEOD[];
extern const sal_Unicode pKeyTRUE[];
extern const sal_Unicode pKeyFALSE[];
extern const sal_Unicode pKeyNA[];
extern const sal_Unicode pKeyV[];
extern const sal_Unicode pKey1_0[];

enum TOPIC
{
    T_UNKNOWN,
    T_TABLE, T_VECTORS, T_TUPLES, T_DATA, T_LABEL, T_COMMENT, T_SIZE,
    T_PERIODICITY, T_MAJORSTART, T_MINORSTART, T_TRUELENGTH, T_UINITS,
    T_DISPLAYUNITS,
    T_END
};

enum DATASET { D_BOT, D_EOD, D_NUMERIC, D_STRING, D_UNKNOWN, D_SYNT_ERROR };

class DifParser
{
public:
    OUStringBuffer      m_aData;
    double              fVal;
    sal_uInt32          nVector;
    sal_uInt32          nVal;
    sal_uInt32          nNumFormat;
private:
    SvNumberFormatter*  pNumFormatter;
    SvStream&           rIn;
    OUString       aLookAheadLine;

    bool                ReadNextLine( OUString& rStr );
    bool                LookAhead();
    DATASET             GetNumberDataset( const sal_Unicode* pPossibleNumericData );
    static inline bool  IsBOT( const sal_Unicode* pRef );
    static inline bool  IsEOD( const sal_Unicode* pRef );
    static inline bool  Is1_0( const sal_Unicode* pRef );
public:
                        DifParser( SvStream&, const ScDocument&, rtl_TextEncoding );

    TOPIC               GetNextTopic();

    DATASET             GetNextDataset();

    static const sal_Unicode* ScanIntVal( const sal_Unicode* pStart, sal_uInt32& rRet );

    static inline bool  IsNumber( const sal_Unicode cChar );

    static inline bool  IsV( const sal_Unicode* pRef );
};

inline bool DifParser::IsBOT( const sal_Unicode* pRef )
{
    return  (   pRef[ 0 ] == pKeyBOT[0] &&
                pRef[ 1 ] == pKeyBOT[1] &&
                pRef[ 2 ] == pKeyBOT[2] &&
                pRef[ 3 ] == pKeyBOT[3] );
}

inline bool DifParser::IsEOD( const sal_Unicode* pRef )
{
    return  (   pRef[ 0 ] == pKeyEOD[0] &&
                pRef[ 1 ] == pKeyEOD[1] &&
                pRef[ 2 ] == pKeyEOD[2] &&
                pRef[ 3 ] == pKeyEOD[3] );
}

inline bool DifParser::Is1_0( const sal_Unicode* pRef )
{
    return  (   pRef[ 0 ] == pKey1_0[0] &&
                pRef[ 1 ] == pKey1_0[1] &&
                pRef[ 2 ] == pKey1_0[2] &&
                pRef[ 3 ] == pKey1_0[3] );
}

inline bool DifParser::IsV( const sal_Unicode* pRef )
{
    return  (   pRef[ 0 ] == pKeyV[0] &&
                pRef[ 1 ] == pKeyV[1]   );
}

inline bool DifParser::IsNumber( const sal_Unicode cChar )
{
    return ( cChar >= '0' && cChar <= '9' );
}

class DifColumn
{
    friend class DifAttrCache;

    struct ENTRY
    {
        sal_uInt32 nNumFormat;
        SCROW nStart;
        SCROW nEnd;
    };

    ENTRY *mpCurrent;
    std::vector<ENTRY> maEntries;

    DifColumn();

    void SetNumFormat( SCROW nRow, const sal_uInt32 nNumFormat );

    void NewEntry( const SCROW nPos, const sal_uInt32 nNumFormat );

    void Apply( ScDocument &rDoc, const SCCOL nCol, const SCTAB nTab );
};

class DifAttrCache
{
public:

    DifAttrCache();

    ~DifAttrCache();

    void SetNumFormat( const SCCOL nCol, const SCROW nRow, const sal_uInt32 nNumFormat );

    void Apply( ScDocument&, SCTAB nTab );

private:

    std::map<SCCOL, std::unique_ptr<DifColumn>> maColMap;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
