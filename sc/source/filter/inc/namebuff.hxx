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

#include <tools/string.hxx>
#include "compiler.hxx"
#include "root.hxx"
#include "xiroot.hxx"

#include "rangenam.hxx"
#include <boost/unordered_map.hpp>
#include <list>

class ScTokenArray;
class NameBuffer;




class StringHashEntry
{
private:
    friend class NameBuffer;
    String          aString;
    sal_uInt32          nHash;

    static sal_uInt32   MakeHashCode( const String& );
public:
    inline          StringHashEntry( const String& );
    inline          StringHashEntry( void );
    inline void     operator =( const sal_Char* );
    inline void     operator =( const String& );
    inline void     operator =( const StringHashEntry& );
    inline sal_Bool     operator ==( const StringHashEntry& ) const;
};


inline StringHashEntry::StringHashEntry( void )
{
}


inline StringHashEntry::StringHashEntry( const String& r ) : aString( r )
{
    nHash = MakeHashCode( r );
}


inline void StringHashEntry::operator =( const sal_Char* p )
{
    aString.AssignAscii( p );
    nHash = MakeHashCode( aString );
}


inline void StringHashEntry::operator =( const String& r )
{
    aString = r;
    nHash = MakeHashCode( r );
}


inline void StringHashEntry::operator =( const StringHashEntry& r )
{
    nHash = r.nHash;
    aString = r.aString;
}


inline sal_Bool StringHashEntry::operator ==( const StringHashEntry& r ) const
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
    inline const String*    Get( sal_uInt16 nIndex ) const;
    inline sal_uInt16       GetLastIndex() const;
    inline void             SetBase( sal_uInt16 nNewBase = 0 );
    void                    operator <<( const String& rNewString );
};

inline NameBuffer::NameBuffer( RootData* p ) : ExcRoot( p )
{
    nBase = 0;
}


inline NameBuffer::NameBuffer( RootData* p, sal_uInt16 nNewBase ) : ExcRoot( p )
{
    nBase = nNewBase;
}


inline const String* NameBuffer::Get ( sal_uInt16 n ) const
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




class ShrfmlaBuffer : public ExcRoot
{
    struct ScAddressHashFunc : public std::unary_function< const ScAddress &, size_t >
    {
        size_t operator() (const ScAddress &addr) const;
    };
    typedef boost::unordered_map <ScAddress, sal_uInt16, ScAddressHashFunc> ShrfmlaHash;
    typedef std::list <ScRange>                                  ShrfmlaList;

    ShrfmlaHash  index_hash;
    ShrfmlaList  index_list;
    size_t                  mnCurrIdx;

public:
                            ShrfmlaBuffer( RootData* pRD );
    virtual                 ~ShrfmlaBuffer();
    void                    Clear();
    void                    Store( const ScRange& rRange, const ScTokenArray& );
    sal_uInt16                  Find (const ScAddress & rAddress ) const;

    static String           CreateName( const ScRange& );
    };




class RangeNameBufferWK3
{
private:
    struct Entry
        {
        StringHashEntry     aStrHashEntry;
        ScComplexRefData        aScComplexRefDataRel;
        String              aScAbsName;
        sal_uInt16              nAbsInd;        // == 0 -> noch keine Abs-Name!
        sal_uInt16              nRelInd;
        sal_Bool                bSingleRef;
                            Entry( const String& rName, const String& rScName, const ScComplexRefData& rCRD ) :
                                aStrHashEntry( rName ),
                                aScComplexRefDataRel( rCRD ),
                                aScAbsName( rScName )
                            {
                                nAbsInd = 0;
                                aScAbsName.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "_ABS" ) );
                            }
        };

    ScTokenArray*           pScTokenArray;
    sal_uInt16                  nIntCount;
    std::vector<Entry> maEntries;

public:
                            RangeNameBufferWK3( void );
    virtual                 ~RangeNameBufferWK3();
    void                    Add( const String& rName, const ScComplexRefData& rCRD );
    inline void             Add( const String& rName, const ScRange& aScRange );
    sal_Bool                    FindRel( const String& rRef, sal_uInt16& rIndex );
    sal_Bool                    FindAbs( const String& rRef, sal_uInt16& rIndex );
};


inline void RangeNameBufferWK3::Add( const String& rName, const ScRange& aScRange )
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
        String      aFile;
        String      aTab;
        sal_uInt16      nTabNum;    // 0xFFFF -> noch nicht angelegt
                                // 0xFFFE -> versucht anzulegen, ging aber schief
                                // 0xFFFD -> soll im selben Workbook sein, findet's aber nicht
        sal_Bool        bSWB;
        sal_Bool        bLink;
                    Cont( const String& rFilePathAndName, const String& rTabName ) :
                        aFile( rFilePathAndName ),
                        aTab( rTabName )
                    {
                        nTabNum = 0xFFFF;   // -> Tabelle noch nicht erzeugt
                        bSWB = bLink = false;
                    }
                    Cont( const String& rFilePathAndName, const String& rTabName,
                        const sal_Bool bSameWB ) :
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

    sal_Int16       Add( const String& rFilePathAndName,
                        const String& rTabName, const sal_Bool bSameWorkbook = false );

    sal_Bool            GetScTabIndex( sal_uInt16 nExcSheetIndex, sal_uInt16& rIn_LastTab_Out_ScIndex );
    sal_Bool            IsLink( const sal_uInt16 nExcSheetIndex ) const;
    sal_Bool            GetLink( const sal_uInt16 nExcSheetIndex, String &rAppl, String &rDoc ) const;

    void            Reset( void );
};


inline ExtSheetBuffer::ExtSheetBuffer( RootData* p ) : ExcRoot( p )
{
}




struct ExtName
{
    String          aName;
    sal_uInt32          nStorageId;
    sal_uInt16          nFlags;

    inline          ExtName( const String& r, sal_uInt16 n ) : aName( r ), nStorageId( 0 ), nFlags( n ) {}

    sal_Bool            IsDDE( void ) const;
    sal_Bool            IsOLE( void ) const;
};




class ExtNameBuff : protected XclImpRoot
{
public:
    explicit        ExtNameBuff( const XclImpRoot& rRoot );

    void            AddDDE( const String& rName, sal_Int16 nRefIdx );
    void            AddOLE( const String& rName, sal_Int16 nRefIdx, sal_uInt32 nStorageId );
    void            AddName( const String& rName, sal_Int16 nRefIdx );

    const ExtName*  GetNameByIndex( sal_Int16 nRefIdx, sal_uInt16 nNameIdx ) const;

    void            Reset();

private:
    typedef ::std::vector< ExtName >            ExtNameVec;
    typedef ::std::map< sal_Int16, ExtNameVec > ExtNameMap;

    ExtNameMap      maExtNames;
};


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
