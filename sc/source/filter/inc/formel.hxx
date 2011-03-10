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

#include <boost/ptr_container/ptr_vector.hpp>

#include <tools/solar.h>
#include <tools/string.hxx>

#include <compiler.hxx>
#include <global.hxx>

#include "root.hxx"
#include "tokstack.hxx"

class XclImpStream;
class ScTokenArray;
class ScFormulaCell;
struct ScSingleRefData;
struct ScComplexRefData;

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

class _ScRangeListTabs
{
    struct _ScRangeList : public boost::ptr_vector<ScRange>
    {
        iterator iterCur;
    };

    sal_Bool                        bHasRanges;
    _ScRangeList**              ppTabLists;
    _ScRangeList*               pAct;
    UINT16                      nAct;

public:

    _ScRangeListTabs ();

    ~_ScRangeListTabs();

    void Append( ScSingleRefData aSRD, SCsTAB nTab, const sal_Bool bLimit = true );
    void Append( ScComplexRefData aCRD, SCsTAB nTab, const sal_Bool bLimit = true );

    const ScRange* First ( const UINT16 nTab = 0 );
    const ScRange* Next ();

    inline bool HasRanges () const { return bHasRanges; }

    inline bool HasActList () const { return pAct != NULL; }
};

class ConverterBase
{
protected:
    TokenPool           aPool;          // User Token + Predefined Token
    TokenStack          aStack;
    ScAddress           aEingPos;
    ConvErr             eStatus;
    sal_Char*           pBuffer;        // Universal-Puffer
    sal_uInt16              nBufferSize;    // ...und seine Groesse

                        ConverterBase( sal_uInt16 nNewBuffer );
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
                        ExcelConverterBase( sal_uInt16 nNewBuffer );
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
    sal_Int32               nBytesLeft;

    inline void         Ignore( const long nSeekRel );
    inline void         Read( sal_Char& nByte );
    inline void         Read( sal_uInt8& nByte );
    inline void         Read( sal_uInt16& nUINT16 );
    inline void         Read( sal_Int16& nINT16 );
    inline void         Read( double& fDouble );
        inline void                     Read( sal_uInt32& nUINT32 );

                        LotusConverterBase( SvStream& rStr, sal_uInt16 nNewBuffer );
    virtual             ~LotusConverterBase();

public:
    void                Reset( const ScAddress& rEingPos );

    virtual ConvErr     Convert( const ScTokenArray*& rpErg, sal_Int32& nRest,
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

inline void LotusConverterBase::Read( sal_uInt8& nByte )
{
    aIn >> nByte;
    nBytesLeft--;
}

inline void LotusConverterBase::Read( sal_uInt16& nUINT16 )
{
    aIn >> nUINT16;
    nBytesLeft -= 2;
}

inline void LotusConverterBase::Read( sal_Int16& nINT16 )
{
    aIn >> nINT16;
    nBytesLeft -= 2;
}

inline void LotusConverterBase::Read( double& fDouble )
{
    aIn >> fDouble;
    nBytesLeft -= 8;
}

inline void LotusConverterBase::Read( sal_uInt32& nUINT32 )
{
    aIn >> nUINT32;
    nBytesLeft -= 4;
}

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
