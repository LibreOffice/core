/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
