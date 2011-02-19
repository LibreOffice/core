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

#ifndef SC_FORMEL_HXX
#define SC_FORMEL_HXX

#include <tools/solar.h>
#include <tools/string.hxx>
#include "tokstack.hxx"
#include "root.hxx"
#include <global.hxx>
#include <compiler.hxx>


// ----- forwards --------------------------------------------------------

class XclImpStream;
class ScTokenArray;
class ScFormulaCell;
struct ScSingleRefData;
struct ScComplexRefData;




//------------------------------------------------------------------------

enum ConvErr
{
    ConvOK = 0,
    ConvErrNi,      // nicht implemntierter/unbekannter Opcode aufgetreten
    ConvErrNoMem,   // Fehler beim Speicheranfordern
    ConvErrExternal,// Add-Ins aus Excel werden nicht umgesetzt
    ConvErrCount    // Nicht alle Bytes der Formel 'erwischt'
};


enum FORMULA_TYPE
{
    FT_CellFormula,
    FT_RangeName,
    FT_SharedFormula
};




//--------------------------------------------------------- class ScRangeList -

class _ScRangeList : protected List
{
private:
protected:
public:
    virtual                 ~_ScRangeList();
    inline void             Append( const ScRange& rRange );
    inline void             Append( ScRange* pRange );
    inline void             Append( const ScSingleRefData& rSRD );
    inline void             Append( const ScComplexRefData& rCRD );

    using                   List::Count;
    inline BOOL             HasRanges( void ) const;

    inline const ScRange*   First( void );
    inline const ScRange*   Next( void );
};


inline void _ScRangeList::Append( const ScRange& r )
{
    List::Insert( new ScRange( r ), LIST_APPEND );
}


inline void _ScRangeList::Append( ScRange* p )
{
    List::Insert( p, LIST_APPEND );
}


inline BOOL _ScRangeList::HasRanges( void ) const
{
    return Count() > 0;
}


inline const ScRange* _ScRangeList::First( void )
{
    return ( const ScRange* ) List::First();
}


inline const ScRange* _ScRangeList::Next( void )
{
    return ( const ScRange* ) List::Next();
}


inline void _ScRangeList::Append( const ScSingleRefData& r )
{
    List::Insert( new ScRange( r.nCol, r.nRow, r.nTab ), LIST_APPEND );
}


inline void _ScRangeList::Append( const ScComplexRefData& r )
{
    List::Insert(   new ScRange(    r.Ref1.nCol, r.Ref1.nRow, r.Ref1.nTab,
                                    r.Ref2.nCol, r.Ref2.nRow, r.Ref2.nTab ),
                    LIST_APPEND );
}




//----------------------------------------------------- class ScRangeListTabs -

class _ScRangeListTabs
{
private:
protected:
    BOOL                        bHasRanges;
    _ScRangeList**              ppTabLists;
    _ScRangeList*               pAct;
    UINT16                      nAct;
public:
                                _ScRangeListTabs( void );
    virtual                     ~_ScRangeListTabs();

    void                        Append( ScSingleRefData aSRD, SCsTAB nTab, const BOOL bLimit = TRUE );
    void                        Append( ScComplexRefData aCRD, SCsTAB nTab, const BOOL bLimit = TRUE );

    inline BOOL                 HasRanges( void ) const;

    const ScRange*              First( const UINT16 nTab = 0 );
    const ScRange*              Next( void );
//      const ScRange*              NextContinue( void );
    inline const _ScRangeList*  GetActList( void ) const;
};


inline BOOL _ScRangeListTabs::HasRanges( void ) const
{
    return bHasRanges;
}


inline const _ScRangeList* _ScRangeListTabs::GetActList( void ) const
{
    return pAct;
}




class ConverterBase
{
protected:
    TokenPool           aPool;          // User Token + Predefined Token
    TokenStack          aStack;
    ScAddress           aEingPos;
    ConvErr             eStatus;
    sal_Char*           pBuffer;        // Universal-Puffer
    UINT16              nBufferSize;    // ...und seine Groesse

                        ConverterBase( UINT16 nNewBuffer );
    virtual             ~ConverterBase();

    void                Reset();

public:
    inline SCCOL        GetEingabeCol( void ) const { return aEingPos.Col(); }
    inline SCROW        GetEingabeRow( void ) const { return aEingPos.Row(); }
    inline SCTAB        GetEingabeTab( void ) const { return aEingPos.Tab(); }
    inline ScAddress    GetEingPos( void ) const    { return aEingPos; }
};



class ExcelConverterBase : public ConverterBase
{
protected:
                        ExcelConverterBase( UINT16 nNewBuffer );
    virtual             ~ExcelConverterBase();

public:
    void                Reset();
    void                Reset( const ScAddress& rEingPos );

    virtual ConvErr     Convert( const ScTokenArray*& rpErg, XclImpStream& rStrm, sal_Size nFormulaLen,
                                 bool bAllowArrays, const FORMULA_TYPE eFT = FT_CellFormula ) = 0;
    virtual ConvErr     Convert( _ScRangeListTabs&, XclImpStream& rStrm, sal_Size nFormulaLen, SCsTAB nTab,
                                    const FORMULA_TYPE eFT = FT_CellFormula ) = 0;
};



class LotusConverterBase : public ConverterBase
{
protected:
    SvStream&           aIn;
    INT32               nBytesLeft;

    inline void         Ignore( const long nSeekRel );
    inline void         Read( sal_Char& nByte );
    inline void         Read( BYTE& nByte );
    inline void         Read( UINT16& nUINT16 );
    inline void         Read( INT16& nINT16 );
    inline void         Read( double& fDouble );
        inline void                     Read( UINT32& nUINT32 );

                        LotusConverterBase( SvStream& rStr, UINT16 nNewBuffer );
    virtual             ~LotusConverterBase();

public:
    void                Reset( const ScAddress& rEingPos );

    virtual ConvErr     Convert( const ScTokenArray*& rpErg, INT32& nRest,
                                    const FORMULA_TYPE eFT = FT_CellFormula ) = 0;

protected:
    using               ConverterBase::Reset;
};


inline void LotusConverterBase::Ignore( const long nSeekRel )
{
    aIn.SeekRel( nSeekRel );
    nBytesLeft -= nSeekRel;
}

inline void LotusConverterBase::Read( sal_Char& nByte )
{
    aIn >> nByte;
    nBytesLeft--;
}

inline void LotusConverterBase::Read( BYTE& nByte )
{
    aIn >> nByte;
    nBytesLeft--;
}

inline void LotusConverterBase::Read( UINT16& nUINT16 )
{
    aIn >> nUINT16;
    nBytesLeft -= 2;
}

inline void LotusConverterBase::Read( INT16& nINT16 )
{
    aIn >> nINT16;
    nBytesLeft -= 2;
}

inline void LotusConverterBase::Read( double& fDouble )
{
    aIn >> fDouble;
    nBytesLeft -= 8;
}

inline void LotusConverterBase::Read( UINT32& nUINT32 )
{
    aIn >> nUINT32;
    nBytesLeft -= 4;
}

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
