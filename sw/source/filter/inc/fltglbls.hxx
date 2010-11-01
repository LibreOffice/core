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
#ifndef _FLTGLBS_HXX
#define _FLTGLBS_HXX

#include <tools/string.hxx>
#include <i18npool/lang.h>
#include <svl/svarray.hxx>


class SfxPoolItem;
class SwDoc;
class SwPaM;
class SwTable;

class XF_Buffer;
class ColorBuffer;
class FontBuffer;
class SvNumberFormatter;


class ExcGlob;
extern ExcGlob *pExcGlob;

class LotGlob;
extern LotGlob *pLotGlob;

// ----- Basis-Klasse ----------------------------------------------------
class FilterGlobals
{
protected:
    SvPtrarr        aTblFmts;
    USHORT          nColStart;
    USHORT          nColEnd;
    USHORT          nRowStart;
    USHORT          nRowEnd;
    USHORT          nAnzCols;
    USHORT          nAnzRows;

public:
    FilterGlobals( SwDoc& rDoc, const SwPaM& rPam );
    ~FilterGlobals();

    SwDoc           *pD;
    SwPaM           *pPam;
    const SwTable   *pTable;

    SvNumberFormatter *pNumFormatter;
    LanguageType    eDefLanguage;
    ULONG           nStandard;
    ULONG           nDefFormat;     // = 0xFFFFFFFF

    void SetRange( USHORT nCS, USHORT nCE, USHORT nRS, USHORT nRE );

    BOOL IsInColRange( USHORT nCol )
                {   return ( nCol >= nColStart && nCol <= nColEnd ); }
    BOOL IsInRowRange( USHORT nRow )
                {   return ( nRow >= nRowStart && nRow <= nRowEnd ); }
    BOOL IsInRange( USHORT nCol, USHORT nRow )
                {   return IsInRowRange(nRow) && IsInColRange(nCol); }

    void NormalizeCol( USHORT &rCol )   { rCol -= nColStart; }
    void NormalizeRow( USHORT &rRow )   { rRow -= nRowStart; }
    void Normalize( USHORT &rCol, USHORT &rRow )
                        {   NormalizeCol( rCol ); NormalizeRow( rRow ); }

    USHORT AnzCols() const  { return nAnzCols; }
    USHORT AnzRows() const  { return nAnzRows; }

    BOOL ColRangeLimitter( USHORT &rCS, USHORT &rCE );

    void InsertText( USHORT nCol, USHORT nRow, const String& rStr );
    void CreateTable();
    void InsertAttr( const SfxPoolItem& rItem );

    inline void     ColLimitter( USHORT &rCol );
    inline void     RowLimitter( USHORT &rRow );
};





inline void FilterGlobals::ColLimitter( USHORT &rCol )
{
    if( rCol < nColStart )
        rCol = nColStart;
    else if( rCol > nColEnd )
        rCol = nColEnd;
}

inline void FilterGlobals::RowLimitter( USHORT &rRow )
{
    if( rRow < nRowStart )
        rRow = nRowStart;
    else if( rRow > nRowEnd )
        rRow = nRowEnd;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
