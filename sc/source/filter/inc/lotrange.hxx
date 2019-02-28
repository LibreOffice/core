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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_LOTRANGE_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_LOTRANGE_HXX

#include <refdata.hxx>
#include <types.hxx>

struct LOTUS_ROOT;

typedef sal_uInt16  LR_ID;

#define ID_FAIL 0xFFFF

class LotusRange
{
    friend class LotusRangeList;
private:
    sal_uInt32              nHash;
    SCCOL               nColStart;
    SCROW               nRowStart;
    SCCOL               nColEnd;
    SCROW               nRowEnd;
    LR_ID               nId;
    void                MakeHash();
    inline void         Copy( const LotusRange& );
    inline void         SetId( LR_ID nId );
public:
                        LotusRange( SCCOL nCol, SCROW nRow );
                        LotusRange( SCCOL nColS, SCROW nRowS, SCCOL nColE, SCROW nRowE );
                        LotusRange( const LotusRange& );
    inline LotusRange   &operator =( const LotusRange& );
    inline bool         operator ==( const LotusRange& ) const;
    inline bool         IsSingle() const;
};

inline void LotusRange::Copy( const LotusRange& rCpy )
{
    nHash = rCpy.nHash;
    nColStart = rCpy.nColStart;
    nRowStart = rCpy.nRowStart;
    nColEnd = rCpy.nColEnd;
    nRowEnd = rCpy.nRowEnd;
    nId = rCpy.nId;
}

inline void LotusRange::SetId( LR_ID nNewId )
{
    nId = nNewId;
}

inline LotusRange &LotusRange::operator =( const LotusRange& rCpy )
{
    Copy( rCpy );
    return *this;
}

inline bool LotusRange::operator ==( const LotusRange& rRef ) const
{
    return ( nHash == rRef.nHash && nColStart == rRef.nColStart &&
        nRowStart == rRef.nRowStart && nColEnd == rRef.nColEnd &&
        nRowEnd == rRef.nRowEnd );
}

inline bool LotusRange::IsSingle() const
{
    return ( nColStart == nColEnd && nRowStart == nRowEnd );
}

class LotusRangeList
{
private:
    LR_ID               nIdCnt;
    ScComplexRefData    aComplRef;
    std::vector<std::unique_ptr<LotusRange>> maRanges;

public:
    LotusRangeList();
    ~LotusRangeList();
    inline LR_ID        GetIndex( SCCOL nCol, SCROW nRow );
    inline LR_ID        GetIndex( SCCOL nColS, SCROW nRowS, SCCOL nColE, SCROW nRowE );
    LR_ID               GetIndex( const LotusRange& );
    void                Append( std::unique_ptr<LotusRange> pLR );
};

inline LR_ID LotusRangeList::GetIndex( SCCOL nCol, SCROW nRow )
{
    LotusRange aRef( nCol, nRow );
    return GetIndex( aRef );
}

inline LR_ID LotusRangeList::GetIndex( SCCOL nColS, SCROW nRowS, SCCOL nColE, SCROW nRowE )
{
    LotusRange aRef( nColS, nRowS, nColE, nRowE );
    return GetIndex( aRef );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
