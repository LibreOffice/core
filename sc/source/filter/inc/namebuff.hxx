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

#ifndef SC_NAMEBUFF_HXX
#define SC_NAMEBUFF_HXX

#include <tools/debug.hxx>
#include <tools/string.hxx>
#include "compiler.hxx"
#include "root.hxx"
#include "xiroot.hxx"

#include "rangenam.hxx"
#include <boost/unordered_map.hpp>
#include <list>

class ScDocument;
class ScTokenArray;
class NameBuffer;




class StringHashEntry
{
private:
    friend class NameBuffer;
    String          aString;
    UINT32          nHash;

    static UINT32   MakeHashCode( const String& );
public:
    inline          StringHashEntry( const String& );
    inline          StringHashEntry( void );
    inline void     operator =( const sal_Char* );
    inline void     operator =( const String& );
    inline void     operator =( const StringHashEntry& );
    inline BOOL     operator ==( const StringHashEntry& ) const;
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


inline BOOL StringHashEntry::operator ==( const StringHashEntry& r ) const
{
    return ( nHash == r.nHash && aString ==  r.aString );
}



class NameBuffer : private List, public ExcRoot
{
private:
    UINT16                  nBase;      // Index-Basis
public:
//    inline                  NameBuffer( void );   //prevent empty rootdata
    inline                  NameBuffer( RootData* );
    inline                  NameBuffer( RootData*, UINT16 nNewBase );

    virtual                 ~NameBuffer();
    inline const String*    Get( UINT16 nIndex );
    inline UINT16           GetLastIndex( void );
    inline void             SetBase( UINT16 nNewBase = 0 );
    void                    operator <<( const String& rNewString );
};

//prevent empty rootdata
//inline NameBuffer::NameBuffer( void )
//{
//    nBase = 0;
//}


inline NameBuffer::NameBuffer( RootData* p ) : ExcRoot( p )
{
    nBase = 0;
}


inline NameBuffer::NameBuffer( RootData* p, UINT16 nNewBase ) : ExcRoot( p )
{
    nBase = nNewBase;
}


inline const String* NameBuffer::Get( UINT16 n )
{
    if( n < nBase )
        return NULL;
    else
    {
        StringHashEntry* pObj = ( StringHashEntry* ) List::GetObject( n );

        if( pObj )
            return &pObj->aString;
        else
            return NULL;
    }
}


inline UINT16 NameBuffer::GetLastIndex( void )
{
    DBG_ASSERT( Count() + nBase <= 0xFFFF, "*NameBuffer::GetLastIndex(): Ich hab' die Nase voll!" );

    return ( UINT16 ) ( Count() + nBase );
}


inline void NameBuffer::SetBase( UINT16 nNewBase )
{
    nBase = nNewBase;
}




class ShrfmlaBuffer : public ExcRoot
{
    struct ScAddressHashFunc : public std::unary_function< const ScAddress &, size_t >
    {
        size_t operator() (const ScAddress &addr) const;
    };
    typedef boost::unordered_map <ScAddress, USHORT, ScAddressHashFunc> ShrfmlaHash;
    typedef std::list <ScRange>                                  ShrfmlaList;

    ShrfmlaHash  index_hash;
    ShrfmlaList  index_list;
    size_t                  mnCurrIdx;

public:
                            ShrfmlaBuffer( RootData* pRD );
    virtual                 ~ShrfmlaBuffer();
    void                    Clear();
    void                    Store( const ScRange& rRange, const ScTokenArray& );
    USHORT                  Find (const ScAddress & rAddress ) const;

    static String           CreateName( const ScRange& );
    };




class RangeNameBufferWK3 : private List
{
private:
    struct ENTRY
        {
        StringHashEntry     aStrHashEntry;
        ScComplexRefData        aScComplexRefDataRel;
        String              aScAbsName;
        UINT16              nAbsInd;        // == 0 -> noch keine Abs-Name!
        UINT16              nRelInd;
        BOOL                bSingleRef;
                            ENTRY( const String& rName, const String& rScName, const ScComplexRefData& rCRD ) :
                                aStrHashEntry( rName ),
                                aScComplexRefDataRel( rCRD ),
                                aScAbsName( rScName )
                            {
                                nAbsInd = 0;
                                aScAbsName.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "_ABS" ) );
                            }
        };

    ScTokenArray*           pScTokenArray;
    UINT16                  nIntCount;
public:
                            RangeNameBufferWK3( void );
    virtual                 ~RangeNameBufferWK3();
    void                    Add( const String& rName, const ScComplexRefData& rCRD );
    inline void             Add( const String& rName, const ScRange& aScRange );
    BOOL                    FindRel( const String& rRef, UINT16& rIndex );
    BOOL                    FindAbs( const String& rRef, UINT16& rIndex );
};


inline void RangeNameBufferWK3::Add( const String& rName, const ScRange& aScRange )
{
    ScComplexRefData        aCRD;
    ScSingleRefData*        pSRD;
    const ScAddress*    pScAddr;

    pSRD = &aCRD.Ref1;
    pScAddr = &aScRange.aStart;
    pSRD->SetFlag3D( TRUE );
    pSRD->nCol = pScAddr->Col();
    pSRD->nRow = pScAddr->Row();
    pSRD->nTab = pScAddr->Tab();

    // zunaechst ALLE Refs nur absolut
    pSRD->SetColRel( FALSE );
    pSRD->SetRowRel( FALSE );
    pSRD->SetTabRel( FALSE );

    pSRD = &aCRD.Ref2;
    pScAddr = &aScRange.aEnd;
    pSRD->SetFlag3D( TRUE );
    pSRD->nCol = pScAddr->Col();
    pSRD->nRow = pScAddr->Row();
    pSRD->nTab = pScAddr->Tab();

    // zunaechst ALLE Refs nur absolut
    pSRD->SetColRel( FALSE );
    pSRD->SetRowRel( FALSE );
    pSRD->SetTabRel( FALSE );

    Add( rName, aCRD );
}




class ExtSheetBuffer : private List, public ExcRoot
{
private:
    struct Cont
        {
        String      aFile;
        String      aTab;
        UINT16      nTabNum;    // 0xFFFF -> noch nicht angelegt
                                // 0xFFFE -> versucht anzulegen, ging aber schief
                                // 0xFFFD -> soll im selben Workbook sein, findet's aber nicht
        BOOL        bSWB;
        BOOL        bLink;
                    Cont( const String& rFilePathAndName, const String& rTabName ) :
                        aFile( rFilePathAndName ),
                        aTab( rTabName )
                    {
                        nTabNum = 0xFFFF;   // -> Tabelle noch nicht erzeugt
                        bSWB = bLink = FALSE;
                    }
                    Cont( const String& rFilePathAndName, const String& rTabName,
                        const BOOL bSameWB ) :
                        aFile( rFilePathAndName ),
                        aTab( rTabName )
                    {
                        nTabNum = 0xFFFF;   // -> Tabelle noch nicht erzeugt
                        bSWB = bSameWB;
                        bLink = FALSE;
                    }
        };
public:
    inline          ExtSheetBuffer( RootData* );
    virtual         ~ExtSheetBuffer();

    sal_Int16       Add( const String& rFilePathAndName,
                        const String& rTabName, const BOOL bSameWorkbook = FALSE );

    BOOL            GetScTabIndex( UINT16 nExcSheetIndex, UINT16& rIn_LastTab_Out_ScIndex );
    BOOL            IsLink( const UINT16 nExcSheetIndex ) const;
    BOOL            GetLink( const UINT16 nExcSheetIndex, String &rAppl, String &rDoc ) const;

    void            Reset( void );
};


inline ExtSheetBuffer::ExtSheetBuffer( RootData* p ) : ExcRoot( p )
{
}




struct ExtName
{
    String          aName;
    UINT32          nStorageId;
    UINT16          nFlags;

    inline          ExtName( const String& r, sal_uInt16 n ) : aName( r ), nStorageId( 0 ), nFlags( n ) {}

    BOOL            IsDDE( void ) const;
    BOOL            IsOLE( void ) const;
};




class ExtNameBuff : protected XclImpRoot
{
public:
    explicit        ExtNameBuff( const XclImpRoot& rRoot );

    void            AddDDE( const String& rName, sal_Int16 nRefIdx );
    void            AddOLE( const String& rName, sal_Int16 nRefIdx, UINT32 nStorageId );
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
