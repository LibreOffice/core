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

#ifndef SC_NAMEBUFF_HXX
#define SC_NAMEBUFF_HXX

#include <rtl/ustring.hxx>
#include "compiler.hxx"
#include "root.hxx"
#include "xiroot.hxx"

#include "rangenam.hxx"
#include "formulacell.hxx"

#include <boost/unordered_map.hpp>
#include <list>

class ScTokenArray;
class NameBuffer;




class StringHashEntry
{
private:
    friend class NameBuffer;
    OUString          aString;
    sal_uInt32        nHash;

    static sal_uInt32   MakeHashCode( const OUString& );
public:
    inline          StringHashEntry( const OUString& );
    inline          StringHashEntry( void );
    inline void     operator =( const sal_Char* );
    inline void     operator =( const OUString& );
    inline void     operator =( const StringHashEntry& );
    inline bool     operator ==( const StringHashEntry& ) const;
};


inline StringHashEntry::StringHashEntry( void )
{
}


inline StringHashEntry::StringHashEntry( const OUString& r ) : aString( r )
{
    nHash = MakeHashCode( r );
}


inline void StringHashEntry::operator =( const sal_Char* p )
{
    aString = OUString(p, strlen(p), RTL_TEXTENCODING_ASCII_US);
    nHash = MakeHashCode( aString );
}


inline void StringHashEntry::operator =( const OUString& r )
{
    aString = r;
    nHash = MakeHashCode( r );
}


inline void StringHashEntry::operator =( const StringHashEntry& r )
{
    nHash = r.nHash;
    aString = r.aString;
}


inline bool StringHashEntry::operator ==( const StringHashEntry& r ) const
{
    return ( nHash == r.nHash && aString ==  r.aString );
}



class NameBuffer : public ExcRoot
{
private:
    sal_uInt16                  nBase;      // Index-Basis
    std::vector<StringHashEntry*> maHashes;

public:

    inline                  NameBuffer( RootData* );
    inline                  NameBuffer( RootData*, sal_uInt16 nNewBase );

    virtual                 ~NameBuffer();
    inline const OUString*  Get( sal_uInt16 nIndex ) const;
    inline sal_uInt16       GetLastIndex() const;
    inline void             SetBase( sal_uInt16 nNewBase = 0 );
    void                    operator <<( const OUString& rNewString );
};

inline NameBuffer::NameBuffer( RootData* p ) : ExcRoot( p )
{
    nBase = 0;
}


inline NameBuffer::NameBuffer( RootData* p, sal_uInt16 nNewBase ) : ExcRoot( p )
{
    nBase = nNewBase;
}


inline const OUString* NameBuffer::Get ( sal_uInt16 n ) const
{
    if( n < nBase || n >= maHashes.size() )
        return NULL;

    return &(maHashes[n]->aString);
}


inline sal_uInt16 NameBuffer::GetLastIndex () const
{
    int size = maHashes.size() + nBase;

    OSL_ENSURE( size <= 0xFFFF, "*NameBuffer::GetLastIndex(): I'm sick and tired!" );

    return static_cast<sal_uInt16>( size );
}


inline void NameBuffer::SetBase( sal_uInt16 nNewBase )
{
    nBase = nNewBase;
}

/**
 * Store and manage shared formula tokens.
 */
class SharedFormulaBuffer : public ExcRoot
{
    typedef boost::unordered_map<ScAddress, ScTokenArray*, ScAddressHashFunctor> TokenArraysType;
    TokenArraysType maTokenArrays;

public:
    SharedFormulaBuffer( RootData* pRD );
    virtual ~SharedFormulaBuffer();
    void Clear();
    void Store( const ScAddress& rPos, const ScTokenArray& rArray );
    const ScTokenArray* Find( const ScAddress& rRefPos ) const;
};

class RangeNameBufferWK3
{
private:
    struct Entry
    {
        StringHashEntry     aStrHashEntry;
        ScComplexRefData    aScComplexRefDataRel;
        OUString            aScAbsName;
        sal_uInt16          nAbsInd;        // == 0 -> noch keine Abs-Name!
        sal_uInt16          nRelInd;
        bool                bSingleRef;
                            Entry( const OUString& rName, const OUString& rScName, const ScComplexRefData& rCRD )
                                : aStrHashEntry( rName )
                                , aScComplexRefDataRel( rCRD )
                                , aScAbsName( rScName )
                                , nAbsInd(0)
                                , nRelInd(0)
                                , bSingleRef(false)
                            {
                                aScAbsName = "_ABS";
                            }
    };

    ScTokenArray*           pScTokenArray;
    sal_uInt16                  nIntCount;
    std::vector<Entry> maEntries;

public:
                            RangeNameBufferWK3( void );
    virtual                 ~RangeNameBufferWK3();
    void                    Add( const OUString& rName, const ScComplexRefData& rCRD );
    inline void             Add( const OUString& rName, const ScRange& aScRange );
    bool                    FindRel( const OUString& rRef, sal_uInt16& rIndex );
    bool                    FindAbs( const OUString& rRef, sal_uInt16& rIndex );
};


inline void RangeNameBufferWK3::Add( const OUString& rName, const ScRange& aScRange )
{
    ScComplexRefData        aCRD;
    ScSingleRefData*        pSRD;

    pSRD = &aCRD.Ref1;
    pSRD->InitAddress(aScRange.aStart);
    pSRD->SetFlag3D(true);

    pSRD = &aCRD.Ref2;
    pSRD->InitAddress(aScRange.aEnd);
    pSRD->SetFlag3D(true);

    Add( rName, aCRD );
}




class ExtSheetBuffer : public ExcRoot
{
private:
    struct Cont
        {
        OUString      aFile;
        OUString      aTab;
        sal_uInt16    nTabNum;    // 0xFFFF -> noch nicht angelegt
                                // 0xFFFE -> versucht anzulegen, ging aber schief
                                // 0xFFFD -> soll im selben Workbook sein, findet's aber nicht
        bool          bSWB;
        bool          bLink;
                    Cont( const OUString& rFilePathAndName, const OUString& rTabName ) :
                        aFile( rFilePathAndName ),
                        aTab( rTabName )
                    {
                        nTabNum = 0xFFFF;   // -> Tabelle noch nicht erzeugt
                        bSWB = bLink = false;
                    }
                    Cont( const OUString& rFilePathAndName, const OUString& rTabName,
                        const bool bSameWB ) :
                        aFile( rFilePathAndName ),
                        aTab( rTabName )
                    {
                        nTabNum = 0xFFFF;   // -> Tabelle noch nicht erzeugt
                        bSWB = bSameWB;
                        bLink = false;
                    }
        };

    std::vector<Cont> maEntries;

public:
    inline          ExtSheetBuffer( RootData* );

    sal_Int16       Add( const OUString& rFilePathAndName,
                        const OUString& rTabName, const bool bSameWorkbook = false );

    bool            GetScTabIndex( sal_uInt16 nExcSheetIndex, sal_uInt16& rIn_LastTab_Out_ScIndex );
    bool            IsLink( const sal_uInt16 nExcSheetIndex ) const;
    bool            GetLink( const sal_uInt16 nExcSheetIndex, OUString &rAppl, OUString &rDoc ) const;

    void            Reset( void );
};


inline ExtSheetBuffer::ExtSheetBuffer( RootData* p ) : ExcRoot( p )
{
}




struct ExtName
{
    OUString          aName;
    sal_uInt32        nStorageId;
    sal_uInt16        nFlags;

    inline          ExtName( const OUString& r, sal_uInt16 n ) : aName( r ), nStorageId( 0 ), nFlags( n ) {}

    bool            IsDDE( void ) const;
    bool            IsOLE( void ) const;
};




class ExtNameBuff : protected XclImpRoot
{
public:
    explicit        ExtNameBuff( const XclImpRoot& rRoot );

    void            AddDDE( const OUString& rName, sal_Int16 nRefIdx );
    void            AddOLE( const OUString& rName, sal_Int16 nRefIdx, sal_uInt32 nStorageId );
    void            AddName( const OUString& rName, sal_Int16 nRefIdx );

    const ExtName*  GetNameByIndex( sal_Int16 nRefIdx, sal_uInt16 nNameIdx ) const;

    void            Reset();

private:
    typedef ::std::vector< ExtName >            ExtNameVec;
    typedef ::std::map< sal_Int16, ExtNameVec > ExtNameMap;

    ExtNameMap      maExtNames;
};


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
