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

#pragma once

#include <memory>
#include <map>
#include <vector>
#include <rtl/ustring.hxx>
#include "root.hxx"
#include "xiroot.hxx"
#include <refdata.hxx>

#include <unordered_map>

class ScTokenArray;

class StringHashEntry
{
private:
    OUString          aString;
    sal_uInt32        nHash;

    static sal_uInt32   MakeHashCode( const OUString& );
public:
    inline          StringHashEntry( const OUString& );
    inline bool     operator ==( const StringHashEntry& ) const;
};

inline StringHashEntry::StringHashEntry( const OUString& r )
    : aString( r )
    , nHash( MakeHashCode(r) )
{
}

inline bool StringHashEntry::operator ==( const StringHashEntry& r ) const
{
    return ( nHash == r.nHash && aString ==  r.aString );
}

/**
 * Store and manage shared formula tokens.
 */
class SharedFormulaBuffer : public ExcRoot
{
    typedef std::unordered_map<ScAddress, std::unique_ptr<ScTokenArray>, ScAddressHashFunctor> TokenArraysType;
    TokenArraysType maTokenArrays;

public:
    SharedFormulaBuffer( RootData* pRD );
    virtual ~SharedFormulaBuffer();
    void Clear();
    void Store( const ScAddress& rPos, const ScTokenArray& rArray );
    const ScTokenArray* Find( const ScAddress& rRefPos ) const;
};

class RangeNameBufferWK3 final
{
private:
    struct Entry
    {
        StringHashEntry     aStrHashEntry;
        ScComplexRefData    aScComplexRefDataRel;
        sal_uInt16          nAbsInd;        // == 0 -> no Abs-Name yet!
        sal_uInt16          nRelInd;
        bool                bSingleRef;
                            Entry( const OUString& rName, const ScComplexRefData& rCRD )
                                : aStrHashEntry( rName )
                                , aScComplexRefDataRel( rCRD )
                                , nAbsInd(0)
                                , nRelInd(0)
                                , bSingleRef(false)
                            {
                            }
    };

    std::unique_ptr<ScTokenArray>
                       pScTokenArray;
    sal_uInt16         nIntCount;
    std::vector<Entry> maEntries;

public:
    RangeNameBufferWK3(const ScDocument& rDoc);
    ~RangeNameBufferWK3();
    void                    Add( const ScDocument& rDoc, const OUString& rName, const ScComplexRefData& rCRD );
    inline void             Add( const ScDocument& rDoc, const OUString& rName, const ScRange& aScRange );
    bool                    FindRel( const OUString& rRef, sal_uInt16& rIndex );
    bool                    FindAbs( const OUString& rRef, sal_uInt16& rIndex );
};

inline void RangeNameBufferWK3::Add( const ScDocument& rDoc, const OUString& rName, const ScRange& aScRange )
{
    ScComplexRefData        aCRD;
    ScSingleRefData*        pSRD;

    pSRD = &aCRD.Ref1;
    pSRD->InitAddress(aScRange.aStart);
    pSRD->SetFlag3D(true);

    pSRD = &aCRD.Ref2;
    pSRD->InitAddress(aScRange.aEnd);
    pSRD->SetFlag3D(true);

    Add( rDoc, rName, aCRD );
}

class ExtSheetBuffer : public ExcRoot
{
private:
    struct Cont
    {
        OUString      aFile;
        OUString      aTab;
        sal_uInt16    nTabNum;    // 0xFFFF -> not set yet
                                // 0xFFFE -> tried to set, but failed
                                // 0xFFFD -> should be in the same workbook, but not found
        bool          bSWB;
                    Cont( const OUString& rFilePathAndName, const OUString& rTabName,
                        const bool bSameWB ) :
                        aFile( rFilePathAndName ),
                        aTab( rTabName )
                    {
                        nTabNum = 0xFFFF;   // -> table not created yet
                        bSWB = bSameWB;
                    }
    };

    std::vector<Cont> maEntries;

public:
    inline          ExtSheetBuffer( RootData* );

    sal_Int16       Add( const OUString& rFilePathAndName,
                        const OUString& rTabName, const bool bSameWorkbook );

    bool            GetScTabIndex( sal_uInt16 nExcSheetIndex, sal_uInt16& rIn_LastTab_Out_ScIndex );

    void            Reset();
};

inline ExtSheetBuffer::ExtSheetBuffer( RootData* p ) : ExcRoot( p )
{
}

struct ExtName
{
    sal_uInt32        nStorageId;
    sal_uInt16        nFlags;

    ExtName( sal_uInt16 n ) : nStorageId( 0 ), nFlags( n ) {}

    bool            IsOLE() const;
};

class ExtNameBuff : protected XclImpRoot
{
public:
    explicit        ExtNameBuff( const XclImpRoot& rRoot );

    void            AddDDE( sal_Int16 nRefIdx );
    void            AddOLE( sal_Int16 nRefIdx, sal_uInt32 nStorageId );
    void            AddName( sal_Int16 nRefIdx );

    const ExtName*  GetNameByIndex( sal_Int16 nRefIdx, sal_uInt16 nNameIdx ) const;

    void            Reset();

private:
    typedef ::std::vector< ExtName >            ExtNameVec;
    typedef ::std::map< sal_Int16, ExtNameVec > ExtNameMap;

    ExtNameMap      maExtNames;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
