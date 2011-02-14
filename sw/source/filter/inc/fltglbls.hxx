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
    sal_uInt16          nColStart;
    sal_uInt16          nColEnd;
    sal_uInt16          nRowStart;
    sal_uInt16          nRowEnd;
    sal_uInt16          nAnzCols;
    sal_uInt16          nAnzRows;

public:
    FilterGlobals( SwDoc& rDoc, const SwPaM& rPam );
    ~FilterGlobals();

    SwDoc           *pD;
    SwPaM           *pPam;
    const SwTable   *pTable;

    SvNumberFormatter *pNumFormatter;
    LanguageType    eDefLanguage;
    sal_uLong           nStandard;
    sal_uLong           nDefFormat;     // = 0xFFFFFFFF

    void SetRange( sal_uInt16 nCS, sal_uInt16 nCE, sal_uInt16 nRS, sal_uInt16 nRE );

    sal_Bool IsInColRange( sal_uInt16 nCol )
                {   return ( nCol >= nColStart && nCol <= nColEnd ); }
    sal_Bool IsInRowRange( sal_uInt16 nRow )
                {   return ( nRow >= nRowStart && nRow <= nRowEnd ); }
    sal_Bool IsInRange( sal_uInt16 nCol, sal_uInt16 nRow )
                {   return IsInRowRange(nRow) && IsInColRange(nCol); }

    void NormalizeCol( sal_uInt16 &rCol )   { rCol -= nColStart; }
    void NormalizeRow( sal_uInt16 &rRow )   { rRow -= nRowStart; }
    void Normalize( sal_uInt16 &rCol, sal_uInt16 &rRow )
                        {   NormalizeCol( rCol ); NormalizeRow( rRow ); }

    sal_uInt16 AnzCols() const  { return nAnzCols; }
    sal_uInt16 AnzRows() const  { return nAnzRows; }

    sal_Bool ColRangeLimitter( sal_uInt16 &rCS, sal_uInt16 &rCE );

    void InsertText( sal_uInt16 nCol, sal_uInt16 nRow, const String& rStr );
    void CreateTable();
    void InsertAttr( const SfxPoolItem& rItem );

    inline void     ColLimitter( sal_uInt16 &rCol );
    inline void     RowLimitter( sal_uInt16 &rRow );
};





inline void FilterGlobals::ColLimitter( sal_uInt16 &rCol )
{
    if( rCol < nColStart )
        rCol = nColStart;
    else if( rCol > nColEnd )
        rCol = nColEnd;
}

inline void FilterGlobals::RowLimitter( sal_uInt16 &rRow )
{
    if( rRow < nRowStart )
        rRow = nRowStart;
    else if( rRow > nRowEnd )
        rRow = nRowEnd;
}

#endif
