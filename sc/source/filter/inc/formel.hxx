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

#ifndef SC_FORMEL_HXX
#define SC_FORMEL_HXX

#include <tools/solar.h>

#include <compiler.hxx>
#include <global.hxx>

#include "root.hxx"
#include "tokstack.hxx"

#include <boost/ptr_container/ptr_map.hpp>
#include <vector>

class XclImpStream;
class ScTokenArray;
struct ScSingleRefData;
struct ScComplexRefData;

enum ConvErr
{
    ConvOK = 0,
    ConvErrNi,      // unimplemented/unknown opcode occurred
    ConvErrNoMem,   // alloc error
    ConvErrExternal,// excel add-ins are not converted
    ConvErrCount    // did not get all bytes of formula
};

enum FORMULA_TYPE
{
    FT_CellFormula,
    FT_RangeName,
    FT_SharedFormula,
    FT_CondFormat
};

class _ScRangeListTabs
{
    typedef ::std::vector<ScRange> RangeListType;
    typedef ::boost::ptr_map<SCTAB, RangeListType> TabRangeType;
    TabRangeType maTabRanges;
    RangeListType::const_iterator maItrCur;
    RangeListType::const_iterator maItrCurEnd;

public:
    _ScRangeListTabs ();
    ~_ScRangeListTabs();

    void Append( const ScAddress& aSRD, SCTAB nTab, bool bLimit = true );
    void Append( const ScRange& aCRD, SCTAB nTab, bool bLimit = true );

    const ScRange* First ( SCTAB nTab = 0 );
    const ScRange* Next ();

    bool HasRanges () const { return !maTabRanges.empty(); }
};

class ConverterBase
{
protected:
    TokenPool           aPool;          // user token + predefined token
    TokenStack          aStack;
    ScAddress           aEingPos;
    ConvErr             eStatus;
    sal_Char*           pBuffer;        // universal buffer
    sal_uInt16              nBufferSize;    // ...and its size

                        ConverterBase( sal_uInt16 nNewBuffer );
    virtual             ~ConverterBase();

    void                Reset();
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
