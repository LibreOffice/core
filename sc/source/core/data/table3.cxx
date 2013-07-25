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


#include <rtl/math.hxx>
#include <unotools/textsearch.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <unotools/charclass.hxx>
#include <unotools/collatorwrapper.hxx>
#include <com/sun/star/i18n/CollatorOptions.hpp>
#include <stdlib.h>
#include <unotools/transliterationwrapper.hxx>

#include "table.hxx"
#include "scitems.hxx"
#include "attrib.hxx"
#include "formulacell.hxx"
#include "document.hxx"
#include "globstr.hrc"
#include "global.hxx"
#include "stlpool.hxx"
#include "compiler.hxx"
#include "patattr.hxx"
#include "subtotal.hxx"
#include "docoptio.hxx"
#include "markdata.hxx"
#include "rangelst.hxx"
#include "attarray.hxx"
#include "userlist.hxx"
#include "progress.hxx"
#include "cellform.hxx"
#include "postit.hxx"
#include "queryparam.hxx"
#include "queryentry.hxx"
#include "segmenttree.hxx"
#include "subtotalparam.hxx"
#include "docpool.hxx"
#include "cellvalue.hxx"
#include "tokenarray.hxx"
#include "mtvcellfunc.hxx"
#include "columnspanset.hxx"

#include <vector>
#include <boost/unordered_set.hpp>

using namespace ::com::sun::star;

namespace naturalsort {

using namespace ::com::sun::star::i18n;

/** Splits a given string into three parts: the prefix, number string, and
    the suffix.

    @param sWhole
    Original string to be split into pieces

    @param sPrefix
    Prefix string that consists of the part before the first number token

    @param sSuffix
    String after the last number token.  This may still contain number strings.

    @param fNum
    Number converted from the middle number string

    @return Returns TRUE if a numeral element is found in a given string, or
    FALSE if no numeral element is found.
*/
bool SplitString( const OUString &sWhole,
    OUString &sPrefix, OUString &sSuffix, double &fNum )
{
    i18n::LocaleDataItem aLocaleItem = ScGlobal::pLocaleData->getLocaleItem();

    // Get prefix element
    OUString sEmpty, sUser = OUString( "-" );
    ParseResult aPRPre = ScGlobal::pCharClass->parsePredefinedToken(
        KParseType::IDENTNAME, sWhole, 0,
        KParseTokens::ANY_LETTER, sUser, KParseTokens::ANY_LETTER, sUser );
    sPrefix = sWhole.copy( 0, aPRPre.EndPos );

    // Return FALSE if no numeral element is found
    if ( aPRPre.EndPos == sWhole.getLength() )
        return false;

    // Get numeral element
    sUser = aLocaleItem.decimalSeparator;
    ParseResult aPRNum = ScGlobal::pCharClass->parsePredefinedToken(
        KParseType::ANY_NUMBER, sWhole, aPRPre.EndPos,
        KParseTokens::ANY_NUMBER, sEmpty, KParseTokens::ANY_NUMBER, sUser );

    if ( aPRNum.EndPos == aPRPre.EndPos )
        return false;

    fNum = aPRNum.Value;
    sSuffix = sWhole.copy( aPRNum.EndPos );

    return true;
}

/** Naturally compares two given strings.

    This is the main function that should be called externally.  It returns
    either 1, 0, or -1 depending on the comparison result of given two strings.

    @param sInput1
    Input string 1

    @param sInput2
    Input string 2

    @param bCaseSens
    Boolean value for case sensitivity

    @param pData
    Pointer to user defined sort list

    @param pCW
    Pointer to collator wrapper for normal string comparison

    @return Returnes 1 if sInput1 is greater, 0 if sInput1 == sInput2, and -1 if
    sInput2 is greater.
*/
short Compare( const String &sInput1, const String &sInput2,
               const bool bCaseSens, const ScUserListData* pData, const CollatorWrapper *pCW )
{
    OUString sStr1( sInput1 ), sStr2( sInput2 ), sPre1, sSuf1, sPre2, sSuf2;

    do
    {
        double nNum1, nNum2;
        bool bNumFound1 = SplitString( sStr1, sPre1, sSuf1, nNum1 );
        bool bNumFound2 = SplitString( sStr2, sPre2, sSuf2, nNum2 );

        short nPreRes; // Prefix comparison result
        if ( pData )
        {
            if ( bCaseSens )
            {
                if ( !bNumFound1 || !bNumFound2 )
                    return static_cast<short>(pData->Compare( sStr1, sStr2 ));
                else
                    nPreRes = pData->Compare( sPre1, sPre2 );
            }
            else
            {
                if ( !bNumFound1 || !bNumFound2 )
                    return static_cast<short>(pData->ICompare( sStr1, sStr2 ));
                else
                    nPreRes = pData->ICompare( sPre1, sPre2 );
            }
        }
        else
        {
            if ( !bNumFound1 || !bNumFound2 )
                return static_cast<short>(pCW->compareString( sStr1, sStr2 ));
            else
                nPreRes = static_cast<short>(pCW->compareString( sPre1, sPre2 ));
        }

        // Prefix strings differ.  Return immediately.
        if ( nPreRes != 0 ) return nPreRes;

        if ( nNum1 != nNum2 )
        {
            if ( nNum1 < nNum2 ) return -1;
            return static_cast<short>( nNum1 > nNum2 );
        }

        // The prefix and the first numerical elements are equal, but the suffix
        // strings may still differ.  Stay in the loop.

        sStr1 = sSuf1;
        sStr2 = sSuf2;

    } while (true);

    return 0;
}

}

// STATIC DATA -----------------------------------------------------------

struct ScSortInfo
{
    ScRefCellValue maCell;
    SCCOLROW        nOrg;
    DECL_FIXEDMEMPOOL_NEWDEL( ScSortInfo );
};
IMPL_FIXEDMEMPOOL_NEWDEL( ScSortInfo )

// END OF STATIC DATA -----------------------------------------------------


class ScSortInfoArray
{
private:
    ScSortInfo***   pppInfo;
    SCSIZE          nCount;
    SCCOLROW        nStart;
    sal_uInt16      nUsedSorts;

public:
                ScSortInfoArray( sal_uInt16 nSorts, SCCOLROW nInd1, SCCOLROW nInd2 ) :
                        pppInfo( new ScSortInfo**[nSorts]),
                        nCount( nInd2 - nInd1 + 1 ), nStart( nInd1 ),
                        nUsedSorts( nSorts )
                    {
                        for ( sal_uInt16 nSort = 0; nSort < nUsedSorts; nSort++ )
                        {
                            ScSortInfo** ppInfo = new ScSortInfo* [nCount];
                            for ( SCSIZE j = 0; j < nCount; j++ )
                                ppInfo[j] = new ScSortInfo;
                            pppInfo[nSort] = ppInfo;
                        }
                    }
                ~ScSortInfoArray()
                    {
                        for ( sal_uInt16 nSort = 0; nSort < nUsedSorts; nSort++ )
                        {
                            ScSortInfo** ppInfo = pppInfo[nSort];
                            for ( SCSIZE j = 0; j < nCount; j++ )
                                delete ppInfo[j];
                            delete [] ppInfo;
                        }
                        delete[] pppInfo;
                    }
    ScSortInfo* Get( sal_uInt16 nSort, SCCOLROW nInd )
                    { return (pppInfo[nSort])[ nInd - nStart ]; }
    void        Swap( SCCOLROW nInd1, SCCOLROW nInd2 )
                    {
                        SCSIZE n1 = static_cast<SCSIZE>(nInd1 - nStart);
                        SCSIZE n2 = static_cast<SCSIZE>(nInd2 - nStart);
                        for ( sal_uInt16 nSort = 0; nSort < nUsedSorts; nSort++ )
                        {
                            ScSortInfo** ppInfo = pppInfo[nSort];
                            ScSortInfo* pTmp = ppInfo[n1];
                            ppInfo[n1] = ppInfo[n2];
                            ppInfo[n2] = pTmp;
                        }
                    }
    sal_uInt16      GetUsedSorts() const { return nUsedSorts; }
    ScSortInfo**    GetFirstArray() const { return pppInfo[0]; }
    SCCOLROW    GetStart() const { return nStart; }
    SCSIZE      GetCount() const { return nCount; }
};

ScSortInfoArray* ScTable::CreateSortInfoArray( SCCOLROW nInd1, SCCOLROW nInd2 )
{
    sal_uInt16 nUsedSorts = 1;
    while ( nUsedSorts < aSortParam.GetSortKeyCount() && aSortParam.maKeyState[nUsedSorts].bDoSort )
        nUsedSorts++;
    ScSortInfoArray* pArray = new ScSortInfoArray( nUsedSorts, nInd1, nInd2 );
    if ( aSortParam.bByRow )
    {
        for ( sal_uInt16 nSort = 0; nSort < nUsedSorts; nSort++ )
        {
            SCCOL nCol = static_cast<SCCOL>(aSortParam.maKeyState[nSort].nField);
            ScColumn* pCol = &aCol[nCol];
            for ( SCROW nRow = nInd1; nRow <= nInd2; nRow++ )
            {
                ScSortInfo* pInfo = pArray->Get( nSort, nRow );
                pInfo->maCell = pCol->GetCellValue(nRow);
                pInfo->nOrg = nRow;
            }
        }
    }
    else
    {
        for ( sal_uInt16 nSort = 0; nSort < nUsedSorts; nSort++ )
        {
            SCROW nRow = aSortParam.maKeyState[nSort].nField;
            for ( SCCOL nCol = static_cast<SCCOL>(nInd1);
                    nCol <= static_cast<SCCOL>(nInd2); nCol++ )
            {
                ScSortInfo* pInfo = pArray->Get( nSort, nCol );
                pInfo->maCell = GetCellValue(nCol, nRow);
                pInfo->nOrg = nCol;
            }
        }
    }
    return pArray;
}


bool ScTable::IsSortCollatorGlobal() const
{
    return  pSortCollator == ScGlobal::GetCollator() ||
            pSortCollator == ScGlobal::GetCaseCollator();
}


void ScTable::InitSortCollator( const ScSortParam& rPar )
{
    if ( !rPar.aCollatorLocale.Language.isEmpty() )
    {
        if ( !pSortCollator || IsSortCollatorGlobal() )
            pSortCollator = new CollatorWrapper( comphelper::getProcessComponentContext() );
        pSortCollator->loadCollatorAlgorithm( rPar.aCollatorAlgorithm,
            rPar.aCollatorLocale, (rPar.bCaseSens ? 0 : SC_COLLATOR_IGNORES) );
    }
    else
    {   // SYSTEM
        DestroySortCollator();
        pSortCollator = (rPar.bCaseSens ? ScGlobal::GetCaseCollator() :
            ScGlobal::GetCollator());
    }
}


void ScTable::DestroySortCollator()
{
    if ( pSortCollator )
    {
        if ( !IsSortCollatorGlobal() )
            delete pSortCollator;
        pSortCollator = NULL;
    }
}


void ScTable::SortReorder( ScSortInfoArray* pArray, ScProgress* pProgress )
{
    bool bByRow = aSortParam.bByRow;
    SCSIZE nCount = pArray->GetCount();
    SCCOLROW nStart = pArray->GetStart();
    ScSortInfo** ppInfo = pArray->GetFirstArray();
    ::std::vector<ScSortInfo*> aTable(nCount);
    SCSIZE nPos;
    for ( nPos = 0; nPos < nCount; nPos++ )
        aTable[ppInfo[nPos]->nOrg - nStart] = ppInfo[nPos];

    SCCOLROW nDest = nStart;
    for ( nPos = 0; nPos < nCount; nPos++, nDest++ )
    {
        SCCOLROW nOrg = ppInfo[nPos]->nOrg;
        if ( nDest != nOrg )
        {
            if ( bByRow )
                SwapRow( nDest, nOrg );
            else
                SwapCol( static_cast<SCCOL>(nDest), static_cast<SCCOL>(nOrg) );
            // neue Position des weggeswapten eintragen
            ScSortInfo* p = ppInfo[nPos];
            p->nOrg = nDest;
            ::std::swap(p, aTable[nDest-nStart]);
            p->nOrg = nOrg;
            ::std::swap(p, aTable[nOrg-nStart]);
            OSL_ENSURE( p == ppInfo[nPos], "SortReorder: nOrg MisMatch" );
        }
        if(pProgress)
            pProgress->SetStateOnPercent( nPos );
    }
}

short ScTable::CompareCell(
    sal_uInt16 nSort,
    ScRefCellValue& rCell1, SCCOL nCell1Col, SCROW nCell1Row,
    ScRefCellValue& rCell2, SCCOL nCell2Col, SCROW nCell2Row ) const
{
    short nRes = 0;

    CellType eType1 = rCell1.meType, eType2 = rCell2.meType;

    if (!rCell1.isEmpty())
    {
        if (!rCell2.isEmpty())
        {
            bool bStr1 = ( eType1 != CELLTYPE_VALUE );
            if (eType1 == CELLTYPE_FORMULA && rCell1.mpFormula->IsValue())
                bStr1 = false;
            bool bStr2 = ( eType2 != CELLTYPE_VALUE );
            if (eType2 == CELLTYPE_FORMULA && rCell2.mpFormula->IsValue())
                bStr2 = false;

            if ( bStr1 && bStr2 )           // nur Strings untereinander als String vergleichen!
            {
                OUString aStr1;
                OUString aStr2;
                if (eType1 == CELLTYPE_STRING)
                    aStr1 = *rCell1.mpString;
                else
                    GetString(nCell1Col, nCell1Row, aStr1);
                if (eType2 == CELLTYPE_STRING)
                    aStr2 = *rCell2.mpString;
                else
                    GetString(nCell2Col, nCell2Row, aStr2);

                bool bUserDef     = aSortParam.bUserDef;        // custom sort order
                bool bNaturalSort = aSortParam.bNaturalSort;    // natural sort
                bool bCaseSens    = aSortParam.bCaseSens;       // case sensitivity

                if (bUserDef)
                {
                    ScUserList* pList = ScGlobal::GetUserList();
                    const ScUserListData* pData = (*pList)[aSortParam.nUserIndex];

                    if (pData)
                    {
                        if ( bNaturalSort )
                            nRes = naturalsort::Compare( aStr1, aStr2, bCaseSens, pData, pSortCollator );
                        else
                        {
                            if ( bCaseSens )
                                nRes = sal::static_int_cast<short>( pData->Compare(aStr1, aStr2) );
                            else
                                nRes = sal::static_int_cast<short>( pData->ICompare(aStr1, aStr2) );
                        }
                    }
                    else
                        bUserDef = false;

                }
                if (!bUserDef)
                {
                    if ( bNaturalSort )
                        nRes = naturalsort::Compare( aStr1, aStr2, bCaseSens, NULL, pSortCollator );
                    else
                        nRes = static_cast<short>( pSortCollator->compareString( aStr1, aStr2 ) );
                }
            }
            else if ( bStr1 )               // String <-> Zahl
                nRes = 1;                   // Zahl vorne
            else if ( bStr2 )               // Zahl <-> String
                nRes = -1;                  // Zahl vorne
            else                            // Zahlen untereinander
            {
                double nVal1 = rCell1.getValue();
                double nVal2 = rCell2.getValue();
                if (nVal1 < nVal2)
                    nRes = -1;
                else if (nVal1 > nVal2)
                    nRes = 1;
            }
            if ( !aSortParam.maKeyState[nSort].bAscending )
                nRes = -nRes;
        }
        else
            nRes = -1;
    }
    else
    {
        if (!rCell2.isEmpty())
            nRes = 1;
        else
            nRes = 0;                   // beide leer
    }
    return nRes;
}

short ScTable::Compare( ScSortInfoArray* pArray, SCCOLROW nIndex1, SCCOLROW nIndex2 ) const
{
    short nRes;
    sal_uInt16 nSort = 0;
    do
    {
        ScSortInfo* pInfo1 = pArray->Get( nSort, nIndex1 );
        ScSortInfo* pInfo2 = pArray->Get( nSort, nIndex2 );
        if ( aSortParam.bByRow )
            nRes = CompareCell( nSort,
                pInfo1->maCell, static_cast<SCCOL>(aSortParam.maKeyState[nSort].nField), pInfo1->nOrg,
                pInfo2->maCell, static_cast<SCCOL>(aSortParam.maKeyState[nSort].nField), pInfo2->nOrg );
        else
            nRes = CompareCell( nSort,
                pInfo1->maCell, static_cast<SCCOL>(pInfo1->nOrg), aSortParam.maKeyState[nSort].nField,
                pInfo2->maCell, static_cast<SCCOL>(pInfo2->nOrg), aSortParam.maKeyState[nSort].nField );
    } while ( nRes == 0 && ++nSort < pArray->GetUsedSorts() );
    if( nRes == 0 )
    {
        ScSortInfo* pInfo1 = pArray->Get( 0, nIndex1 );
        ScSortInfo* pInfo2 = pArray->Get( 0, nIndex2 );
        if( pInfo1->nOrg < pInfo2->nOrg )
            nRes = -1;
        else if( pInfo1->nOrg > pInfo2->nOrg )
            nRes = 1;
    }
    return nRes;
}

void ScTable::QuickSort( ScSortInfoArray* pArray, SCsCOLROW nLo, SCsCOLROW nHi )
{
    if ((nHi - nLo) == 1)
    {
        if (Compare(pArray, nLo, nHi) > 0)
            pArray->Swap( nLo, nHi );
    }
    else
    {
        SCsCOLROW ni = nLo;
        SCsCOLROW nj = nHi;
        do
        {
            while ((ni <= nHi) && (Compare(pArray, ni, nLo)) < 0)
                ni++;
            while ((nj >= nLo) && (Compare(pArray, nLo, nj)) < 0)
                nj--;
            if (ni <= nj)
            {
                if (ni != nj)
                    pArray->Swap( ni, nj );
                ni++;
                nj--;
            }
        } while (ni < nj);
        if ((nj - nLo) < (nHi - ni))
        {
            if (nLo < nj)
                QuickSort(pArray, nLo, nj);
            if (ni < nHi)
                QuickSort(pArray, ni, nHi);
        }
        else
        {
            if (ni < nHi)
                QuickSort(pArray, ni, nHi);
            if (nLo < nj)
                QuickSort(pArray, nLo, nj);
        }
    }
}

void ScTable::SwapCol(SCCOL nCol1, SCCOL nCol2)
{
    SCROW nRowStart = aSortParam.nRow1;
    SCROW nRowEnd = aSortParam.nRow2;
    for (SCROW nRow = nRowStart; nRow <= nRowEnd; nRow++)
    {
        aCol[nCol1].SwapCell(nRow, aCol[nCol2]);
        if (aSortParam.bIncludePattern)
        {
            const ScPatternAttr* pPat1 = GetPattern(nCol1, nRow);
            const ScPatternAttr* pPat2 = GetPattern(nCol2, nRow);
            if (pPat1 != pPat2)
            {
                pDocument->GetPool()->Put(*pPat1);
                SetPattern(nCol1, nRow, *pPat2, true);
                SetPattern(nCol2, nRow, *pPat1, true);
                pDocument->GetPool()->Remove(*pPat1);
            }
        }
    }

    ScNotes aNoteMap(pDocument);
    ScNotes::iterator itr = maNotes.begin();
    while(itr != maNotes.end())
    {
        SCCOL nCol = itr->first.first;
        SCROW nRow = itr->first.second;
        ScPostIt* pPostIt = itr->second;
        ++itr;

        if(nRow >= nRowStart && nRow <= nRowEnd)
        {
            if (nCol == nCol1)
            {
                aNoteMap.insert(nCol2, nRow, pPostIt);
                maNotes.ReleaseNote(nCol, nRow);
            }
            else if (nCol == nCol2)
            {
                aNoteMap.insert(nCol1, nRow, pPostIt);
                maNotes.ReleaseNote(nCol, nRow);

            }
        }
    }

    itr = aNoteMap.begin();
    while(itr != aNoteMap.end())
    {
        //we can here assume that there is no note in the target location
        SCCOL nCol = itr->first.first;
        SCROW nRow = itr->first.second;
        ScPostIt* pPostIt = itr->second;
        ++itr;

        maNotes.insert(nCol, nRow, pPostIt);
        aNoteMap.ReleaseNote(nCol, nRow);
    }
}

void ScTable::SwapRow(SCROW nRow1, SCROW nRow2)
{
    SCCOL nColStart = aSortParam.nCol1;
    SCCOL nColEnd = aSortParam.nCol2;
    for (SCCOL nCol = nColStart; nCol <= nColEnd; nCol++)
    {
        aCol[nCol].SwapRow(nRow1, nRow2);
        if (aSortParam.bIncludePattern)
        {
            const ScPatternAttr* pPat1 = GetPattern(nCol, nRow1);
            const ScPatternAttr* pPat2 = GetPattern(nCol, nRow2);
            if (pPat1 != pPat2)
            {
                pDocument->GetPool()->Put(*pPat1);
                SetPattern(nCol, nRow1, *pPat2, true);
                SetPattern(nCol, nRow2, *pPat1, true);
                pDocument->GetPool()->Remove(*pPat1);
            }
        }
    }
    if (bGlobalKeepQuery)
    {
        bool bRow1Hidden = RowHidden(nRow1);
        bool bRow2Hidden = RowHidden(nRow2);
        SetRowHidden(nRow1, nRow1, bRow2Hidden);
        SetRowHidden(nRow2, nRow2, bRow1Hidden);

        bool bRow1Filtered = RowFiltered(nRow1);
        bool bRow2Filtered = RowFiltered(nRow2);
        SetRowFiltered(nRow1, nRow1, bRow2Filtered);
        SetRowFiltered(nRow2, nRow2, bRow1Filtered);
    }

    ScNotes aNoteMap(pDocument);
    ScNotes::iterator itr = maNotes.begin();
    while(itr != maNotes.end())
    {
        SCCOL nCol = itr->first.first;
        SCROW nRow = itr->first.second;
        ScPostIt* pPostIt = itr->second;
        ++itr;

        if( nCol >= nColStart && nCol <= nColEnd )
        {
            if (nRow == nRow1)
            {
                aNoteMap.insert(nCol, nRow2, pPostIt);
                maNotes.ReleaseNote(nCol, nRow);
            }
            else if (nRow == nRow2)
            {
                aNoteMap.insert(nCol, nRow1, pPostIt);
                maNotes.ReleaseNote(nCol, nRow);
            }
        }
    }

    itr = aNoteMap.begin();
    while(itr != aNoteMap.end())
    {
        //we can here assume that there is no note in the target location
        SCCOL nCol = itr->first.first;
        SCROW nRow = itr->first.second;
        ScPostIt* pPostIt = itr->second;
        ++itr;

        maNotes.insert(nCol, nRow, pPostIt);
        aNoteMap.ReleaseNote(nCol, nRow);
    }
}

short ScTable::Compare(SCCOLROW nIndex1, SCCOLROW nIndex2) const
{
    short nRes;
    sal_uInt16 nSort = 0;
    const sal_uInt32 nMaxSorts = aSortParam.GetSortKeyCount();
    if (aSortParam.bByRow)
    {
        do
        {
            SCCOL nCol = static_cast<SCCOL>(aSortParam.maKeyState[nSort].nField);
            ScRefCellValue aCell1 = aCol[nCol].GetCellValue(nIndex1);
            ScRefCellValue aCell2 = aCol[nCol].GetCellValue(nIndex2);
            nRes = CompareCell(nSort, aCell1, nCol, nIndex1, aCell2, nCol, nIndex2);
        } while ( nRes == 0 && ++nSort < nMaxSorts && aSortParam.maKeyState[nSort].bDoSort );
    }
    else
    {
        do
        {
            SCROW nRow = aSortParam.maKeyState[nSort].nField;
            ScRefCellValue aCell1 = aCol[nIndex1].GetCellValue(nRow);
            ScRefCellValue aCell2 = aCol[nIndex2].GetCellValue(nRow);
            nRes = CompareCell( nSort, aCell1, static_cast<SCCOL>(nIndex1),
                    nRow, aCell2, static_cast<SCCOL>(nIndex2), nRow );
        } while ( nRes == 0 && ++nSort < nMaxSorts && aSortParam.maKeyState[nSort].bDoSort );
    }
    return nRes;
}

bool ScTable::IsSorted( SCCOLROW nStart, SCCOLROW nEnd ) const   // ueber aSortParam
{
    for (SCCOLROW i=nStart; i<nEnd; i++)
    {
        if (Compare( i, i+1 ) > 0)
            return false;
    }
    return true;
}

void ScTable::DecoladeRow( ScSortInfoArray* pArray, SCROW nRow1, SCROW nRow2 )
{
    SCROW nRow;
    SCROW nMax = nRow2 - nRow1;
    for (SCROW i = nRow1; (i + 4) <= nRow2; i += 4)
    {
        nRow = rand() % nMax;
        pArray->Swap(i, nRow1 + nRow);
    }
}

void ScTable::Sort(const ScSortParam& rSortParam, bool bKeepQuery, ScProgress* pProgress)
{
    aSortParam = rSortParam;
    InitSortCollator( rSortParam );
    bGlobalKeepQuery = bKeepQuery;
    if (rSortParam.bByRow)
    {
        SCROW nLastRow = 0;
        for (SCCOL nCol = aSortParam.nCol1; nCol <= aSortParam.nCol2; nCol++)
            nLastRow = std::max(nLastRow, aCol[nCol].GetLastDataPos());
        nLastRow = std::min(nLastRow, aSortParam.nRow2);
        SCROW nRow1 = (rSortParam.bHasHeader ?
            aSortParam.nRow1 + 1 : aSortParam.nRow1);
        if (!IsSorted(nRow1, nLastRow))
        {
            if(pProgress)
                pProgress->SetState( 0, nLastRow-nRow1 );
            ScSortInfoArray* pArray = CreateSortInfoArray( nRow1, nLastRow );
            if ( nLastRow - nRow1 > 255 )
                DecoladeRow( pArray, nRow1, nLastRow );
            QuickSort( pArray, nRow1, nLastRow );
            SortReorder( pArray, pProgress );
            delete pArray;
            // #i59745# update position of caption objects of cell notes
            ScNoteUtil::UpdateCaptionPositions( *pDocument, ScRange( aSortParam.nCol1, nRow1, nTab, aSortParam.nCol2, nLastRow, nTab ) );
        }
    }
    else
    {
        SCCOL nLastCol;
        for (nLastCol = aSortParam.nCol2;
             (nLastCol > aSortParam.nCol1) && aCol[nLastCol].IsEmptyBlock(aSortParam.nRow1, aSortParam.nRow2); nLastCol--)
        {
        }
        SCCOL nCol1 = (rSortParam.bHasHeader ?
            aSortParam.nCol1 + 1 : aSortParam.nCol1);
        if (!IsSorted(nCol1, nLastCol))
        {
            if(pProgress)
                pProgress->SetState( 0, nLastCol-nCol1 );
            ScSortInfoArray* pArray = CreateSortInfoArray( nCol1, nLastCol );
            QuickSort( pArray, nCol1, nLastCol );
            SortReorder( pArray, pProgress );
            delete pArray;
            // #i59745# update position of caption objects of cell notes
            ScNoteUtil::UpdateCaptionPositions( *pDocument, ScRange( nCol1, aSortParam.nRow1, nTab, nLastCol, aSortParam.nRow2, nTab ) );
        }
    }
    DestroySortCollator();
}

namespace {

class SubTotalRowFinder
{
    const ScTable& mrTab;
    const ScSubTotalParam& mrParam;

public:
    SubTotalRowFinder(const ScTable& rTab, const ScSubTotalParam& rParam) :
        mrTab(rTab), mrParam(rParam) {}

    bool operator() (size_t nRow, const ScFormulaCell* pCell)
    {
        if (!pCell->IsSubTotal())
            return false;

        SCCOL nStartCol = mrParam.nCol1;
        SCCOL nEndCol = mrParam.nCol2;

        for (SCCOL i = 0; i <= MAXCOL; ++i)
        {
            if (nStartCol <= i && i <= nEndCol)
                continue;

            if (mrTab.HasData(i, nRow))
                return true;
        }

        return false;
    }
};

}

bool ScTable::TestRemoveSubTotals( const ScSubTotalParam& rParam )
{
    SCCOL nStartCol = rParam.nCol1;
    SCROW nStartRow = rParam.nRow1 + 1;     // Header
    SCCOL nEndCol   = rParam.nCol2;
    SCROW nEndRow    = rParam.nRow2;

    for (SCCOL nCol = nStartCol; nCol <= nEndCol; ++nCol)
    {
        const sc::CellStoreType& rCells = aCol[nCol].maCells;
        SubTotalRowFinder aFunc(*this, rParam);
        std::pair<sc::CellStoreType::const_iterator,size_t> aPos =
            sc::FindFormula(rCells, nStartRow, nEndRow, aFunc);
        if (aPos.first != rCells.end())
            return true;
    }
    return false;
}

namespace {

class RemoveSubTotalsHandler
{
    std::vector<SCROW> maRemoved;
public:

    void operator() (size_t nRow, const ScFormulaCell* p)
    {
        if (p->IsSubTotal())
            maRemoved.push_back(nRow);
    }

    void getRows(std::vector<SCROW>& rRows)
    {
        // Sort and remove duplicates.
        std::sort(maRemoved.begin(), maRemoved.end());
        std::vector<SCROW>::iterator it = std::unique(maRemoved.begin(), maRemoved.end());
        maRemoved.erase(it, maRemoved.end());

        maRemoved.swap(rRows);
    }
};

}

void ScTable::RemoveSubTotals( ScSubTotalParam& rParam )
{
    SCCOL nStartCol = rParam.nCol1;
    SCROW nStartRow = rParam.nRow1 + 1;     // Header
    SCCOL nEndCol   = rParam.nCol2;
    SCROW nEndRow    = rParam.nRow2;            // wird veraendert

    RemoveSubTotalsHandler aFunc;
    for (SCCOL nCol = nStartCol; nCol <= nEndCol; ++nCol)
    {
        const sc::CellStoreType& rCells = aCol[nCol].maCells;
        sc::ParseFormula(rCells.begin(), rCells, nStartRow, nEndRow, aFunc);
    }

    std::vector<SCROW> aRows;
    aFunc.getRows(aRows);

    std::vector<SCROW>::reverse_iterator it = aRows.rbegin(), itEnd = aRows.rend();
    for (; it != itEnd; ++it)
    {
        SCROW nRow = *it;
        RemoveRowBreak(nRow+1, false, true);
        pDocument->DeleteRow(0, nTab, MAXCOL, nTab, nRow, 1);
    }

    rParam.nRow2 -= aRows.size();
}

//  harte Zahlenformate loeschen (fuer Ergebnisformeln)

static void lcl_RemoveNumberFormat( ScTable* pTab, SCCOL nCol, SCROW nRow )
{
    const ScPatternAttr* pPattern = pTab->GetPattern( nCol, nRow );
    if ( pPattern->GetItemSet().GetItemState( ATTR_VALUE_FORMAT, false )
            == SFX_ITEM_SET )
    {
        ScPatternAttr aNewPattern( *pPattern );
        SfxItemSet& rSet = aNewPattern.GetItemSet();
        rSet.ClearItem( ATTR_VALUE_FORMAT );
        rSet.ClearItem( ATTR_LANGUAGE_FORMAT );
        pTab->SetPattern( nCol, nRow, aNewPattern, true );
    }
}


// at least MSC needs this at linkage level to be able to use it in a template
typedef struct lcl_ScTable_DoSubTotals_RowEntry
{
    sal_uInt16  nGroupNo;
    SCROW   nSubStartRow;
    SCROW   nDestRow;
    SCROW   nFuncStart;
    SCROW   nFuncEnd;
} RowEntry;

//      neue Zwischenergebnisse
//      rParam.nRow2 wird veraendert !

bool ScTable::DoSubTotals( ScSubTotalParam& rParam )
{
    SCCOL nStartCol = rParam.nCol1;
    SCROW nStartRow = rParam.nRow1 + 1;     // Header
    SCCOL nEndCol   = rParam.nCol2;
    SCROW nEndRow    = rParam.nRow2;            // wird veraendert
    sal_uInt16 i;

    //  Leerzeilen am Ende weglassen,
    //  damit alle Ueberlaeufe (MAXROW) bei InsertRow gefunden werden (#35180#)
    //  Wenn sortiert wurde, sind alle Leerzeilen am Ende.
    SCSIZE nEmpty = GetEmptyLinesInBlock( nStartCol, nStartRow, nEndCol, nEndRow, DIR_BOTTOM );
    nEndRow -= nEmpty;

    sal_uInt16 nLevelCount = 0;             // Anzahl Gruppierungen
    bool bDoThis = true;
    for (i=0; i<MAXSUBTOTAL && bDoThis; i++)
        if (rParam.bGroupActive[i])
            nLevelCount = i+1;
        else
            bDoThis = false;

    if (nLevelCount==0)                 // nichts tun
        return true;

    SCCOL*          nGroupCol = rParam.nField;  // Spalten nach denen
                                                // gruppiert wird

    //  Durch (leer) als eigene Kategorie muss immer auf
    //  Teilergebniszeilen aus den anderen Spalten getestet werden
    //  (frueher nur, wenn eine Spalte mehrfach vorkam)
    bool bTestPrevSub = ( nLevelCount > 1 );

    OUString  aSubString;
    String  aOutString;

    bool bIgnoreCase = !rParam.bCaseSens;

    String *pCompString[MAXSUBTOTAL];               // Pointer wegen Compiler-Problemen
    for (i=0; i<MAXSUBTOTAL; i++)
        pCompString[i] = new String;

                                //! sortieren?

    ScStyleSheet* pStyle = (ScStyleSheet*) pDocument->GetStyleSheetPool()->Find(
                                ScGlobal::GetRscString(STR_STYLENAME_RESULT), SFX_STYLE_FAMILY_PARA );

    bool bSpaceLeft = true;                                         // Erfolg beim Einfuegen?

    // For performance reasons collect formula entries so their
    // references don't have to be tested for updates each time a new row is
    // inserted
    RowEntry aRowEntry;
    ::std::vector< RowEntry > aRowVector;

    for (sal_uInt16 nLevel=0; nLevel<=nLevelCount && bSpaceLeft; nLevel++)      // incl. Gesamtergebnis
    {
        bool bTotal = ( nLevel == nLevelCount );
        aRowEntry.nGroupNo = bTotal ? 0 : (nLevelCount-nLevel-1);

        // how many results per level
        SCCOL nResCount         = rParam.nSubTotals[aRowEntry.nGroupNo];
        // result functions
        ScSubTotalFunc* eResFunc = rParam.pFunctions[aRowEntry.nGroupNo];

        if (nResCount > 0)                                      // sonst nur sortieren
        {
            for (i=0; i<=aRowEntry.nGroupNo; i++)
            {
                GetString( nGroupCol[i], nStartRow, aSubString );
                if ( bIgnoreCase )
                    *pCompString[i] = ScGlobal::pCharClass->uppercase( aSubString );
                else
                    *pCompString[i] = aSubString;
            }                                                   // aSubString bleibt auf dem letzten stehen

            bool bBlockVis = false;             // Gruppe eingeblendet?
            aRowEntry.nSubStartRow = nStartRow;
            for (SCROW nRow=nStartRow; nRow<=nEndRow+1 && bSpaceLeft; nRow++)
            {
                bool bChanged;
                if (nRow>nEndRow)
                    bChanged = true;
                else
                {
                    bChanged = false;
                    if (!bTotal)
                    {
                        OUString aString;
                        for (i=0; i<=aRowEntry.nGroupNo && !bChanged; i++)
                        {
                            GetString( nGroupCol[i], nRow, aString );
                            if (bIgnoreCase)
                                aString = ScGlobal::pCharClass->uppercase(aString);
                            //  wenn sortiert, ist "leer" eine eigene Gruppe
                            //  sonst sind leere Zellen unten erlaubt
                            bChanged = ( ( !aString.isEmpty() || rParam.bDoSort ) &&
                                            aString != OUString(*pCompString[i]) );
                        }
                        if ( bChanged && bTestPrevSub )
                        {
                            // No group change on rows that will contain subtotal formulas
                            for ( ::std::vector< RowEntry >::const_iterator
                                    iEntry( aRowVector.begin());
                                    iEntry != aRowVector.end(); ++iEntry)
                            {
                                if ( iEntry->nDestRow == nRow )
                                {
                                    bChanged = false;
                                    break;
                                }
                            }
                        }
                    }
                }
                if ( bChanged )
                {
                    aRowEntry.nDestRow   = nRow;
                    aRowEntry.nFuncStart = aRowEntry.nSubStartRow;
                    aRowEntry.nFuncEnd   = nRow-1;

                    bSpaceLeft = pDocument->InsertRow( 0, nTab, MAXCOL, nTab,
                            aRowEntry.nDestRow, 1 );
                    DBShowRow( aRowEntry.nDestRow, bBlockVis );
                    bBlockVis = false;
                    if ( rParam.bPagebreak && nRow < MAXROW &&
                            aRowEntry.nSubStartRow != nStartRow && nLevel == 0)
                        SetRowBreak(aRowEntry.nSubStartRow, false, true);

                    if (bSpaceLeft)
                    {
                        for ( ::std::vector< RowEntry >::iterator iMove(
                                    aRowVector.begin() );
                                iMove != aRowVector.end(); ++iMove)
                        {
                            if ( aRowEntry.nDestRow <= iMove->nSubStartRow )
                                ++iMove->nSubStartRow;
                            if ( aRowEntry.nDestRow <= iMove->nDestRow )
                                ++iMove->nDestRow;
                            if ( aRowEntry.nDestRow <= iMove->nFuncStart )
                                ++iMove->nFuncStart;
                            if ( aRowEntry.nDestRow <= iMove->nFuncEnd )
                                ++iMove->nFuncEnd;
                        }
                        // collect formula positions
                        aRowVector.push_back( aRowEntry );

                        if (bTotal)     // "Gesamtergebnis"
                            aOutString = ScGlobal::GetRscString( STR_TABLE_GESAMTERGEBNIS );
                        else
                        {               // " Ergebnis"
                            aOutString = aSubString;
                            if (!aOutString.Len())
                                aOutString = ScGlobal::GetRscString( STR_EMPTYDATA );
                            aOutString += ' ';
                            sal_uInt16 nStrId = STR_TABLE_ERGEBNIS;
                            if ( nResCount == 1 )
                                switch ( eResFunc[0] )
                                {
                                    case SUBTOTAL_FUNC_AVE:     nStrId = STR_FUN_TEXT_AVG;      break;
                                    case SUBTOTAL_FUNC_CNT:
                                    case SUBTOTAL_FUNC_CNT2:    nStrId = STR_FUN_TEXT_COUNT;    break;
                                    case SUBTOTAL_FUNC_MAX:     nStrId = STR_FUN_TEXT_MAX;      break;
                                    case SUBTOTAL_FUNC_MIN:     nStrId = STR_FUN_TEXT_MIN;      break;
                                    case SUBTOTAL_FUNC_PROD:    nStrId = STR_FUN_TEXT_PRODUCT;  break;
                                    case SUBTOTAL_FUNC_STD:
                                    case SUBTOTAL_FUNC_STDP:    nStrId = STR_FUN_TEXT_STDDEV;   break;
                                    case SUBTOTAL_FUNC_SUM:     nStrId = STR_FUN_TEXT_SUM;      break;
                                    case SUBTOTAL_FUNC_VAR:
                                    case SUBTOTAL_FUNC_VARP:    nStrId = STR_FUN_TEXT_VAR;      break;
                                    default:
                                    {
                                        // added to avoid warnings
                                    }
                                }
                            aOutString += ScGlobal::GetRscString( nStrId );
                        }
                        SetString( nGroupCol[aRowEntry.nGroupNo], aRowEntry.nDestRow, nTab, aOutString );
                        ApplyStyle( nGroupCol[aRowEntry.nGroupNo], aRowEntry.nDestRow, *pStyle );

                        ++nRow;
                        ++nEndRow;
                        aRowEntry.nSubStartRow = nRow;
                        for (i=0; i<=aRowEntry.nGroupNo; i++)
                        {
                            GetString( nGroupCol[i], nRow, aSubString );
                            if ( bIgnoreCase )
                                *pCompString[i] = ScGlobal::pCharClass->uppercase( aSubString );
                            else
                                *pCompString[i] = aSubString;
                        }
                    }
                }
                bBlockVis = !RowFiltered(nRow);
            }
        }
    }

    // now insert the formulas
    ScComplexRefData aRef;
    aRef.InitFlags();
    aRef.Ref1.nTab = nTab;
    aRef.Ref2.nTab = nTab;
    for ( ::std::vector< RowEntry >::const_iterator iEntry( aRowVector.begin());
            iEntry != aRowVector.end(); ++iEntry)
    {
        SCCOL nResCount         = rParam.nSubTotals[iEntry->nGroupNo];
        SCCOL* nResCols         = rParam.pSubTotals[iEntry->nGroupNo];
        ScSubTotalFunc* eResFunc = rParam.pFunctions[iEntry->nGroupNo];
        for ( SCCOL nResult=0; nResult < nResCount; ++nResult )
        {
            aRef.Ref1.nCol = nResCols[nResult];
            aRef.Ref1.nRow = iEntry->nFuncStart;
            aRef.Ref2.nCol = nResCols[nResult];
            aRef.Ref2.nRow = iEntry->nFuncEnd;

            ScTokenArray aArr;
            aArr.AddOpCode( ocSubTotal );
            aArr.AddOpCode( ocOpen );
            aArr.AddDouble( (double) eResFunc[nResult] );
            aArr.AddOpCode( ocSep );
            aArr.AddDoubleReference( aRef );
            aArr.AddOpCode( ocClose );
            aArr.AddOpCode( ocStop );
            ScFormulaCell* pCell = new ScFormulaCell(
                pDocument, ScAddress(nResCols[nResult], iEntry->nDestRow, nTab), &aArr);

            SetFormulaCell(nResCols[nResult], iEntry->nDestRow, pCell);

            if ( nResCols[nResult] != nGroupCol[iEntry->nGroupNo] )
            {
                ApplyStyle( nResCols[nResult], iEntry->nDestRow, *pStyle );

                //  Zahlformat loeschen
                lcl_RemoveNumberFormat( this, nResCols[nResult], iEntry->nDestRow );
            }
        }

    }

    //!     je nach Einstellung Zwischensummen-Zeilen nach oben verschieben ?

    //!     Outlines direkt erzeugen?

    if (bSpaceLeft)
        DoAutoOutline( nStartCol, nStartRow, nEndCol, nEndRow );

    for (i=0; i<MAXSUBTOTAL; i++)
        delete pCompString[i];

    rParam.nRow2 = nEndRow;                 // neues Ende
    return bSpaceLeft;
}

void ScTable::MarkSubTotalCells(
    sc::ColumnSpanSet& rSet, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, bool bVal ) const
{
    if (!ValidCol(nCol1) || !ValidCol(nCol2))
        return;

    // Pick up all subtotal formula cells.
    for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
        aCol[nCol].MarkSubTotalCells(rSet, nRow1, nRow2, bVal);

    // Pick up all filtered rows.
    ScFlatBoolRowSegments::RangeData aFilteredSpan;
    SCROW nRow = nRow1;
    while (nRow <= nRow2)
    {
        if (!mpFilteredRows->getRangeData(nRow, aFilteredSpan))
            // Failed for whatever reason.
            return;

        if (aFilteredSpan.mbValue)
        {
            // Filtered span found.
            for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
                rSet.set(nTab, nCol, nRow, aFilteredSpan.mnRow2, bVal);
        }

        nRow = aFilteredSpan.mnRow2 + 1;
    }
}

namespace {

class QueryEvaluator
{
    const ScDocument& mrDoc;
    const ScTable& mrTab;
    const ScQueryParam& mrParam;
    const bool* mpTestEqualCondition;
    utl::TransliterationWrapper* mpTransliteration;
    CollatorWrapper* mpCollator;
    const bool mbMatchWholeCell;

    bool isPartialTextMatchOp(const ScQueryEntry& rEntry) const
    {
        switch (rEntry.eOp)
        {
            // these operators can only be used with textural comparisons.
            case SC_CONTAINS:
            case SC_DOES_NOT_CONTAIN:
            case SC_BEGINS_WITH:
            case SC_ENDS_WITH:
            case SC_DOES_NOT_BEGIN_WITH:
            case SC_DOES_NOT_END_WITH:
                return true;
            default:
                ;
        }
        return false;
    }

    bool isTextMatchOp(const ScQueryEntry& rEntry) const
    {
        if (isPartialTextMatchOp(rEntry))
            return true;

        switch (rEntry.eOp)
        {
            // these operators can be used for either textural or value comparison.
            case SC_EQUAL:
            case SC_NOT_EQUAL:
                return true;
            default:
                ;
        }
        return false;
    }

    bool isRealRegExp(const ScQueryEntry& rEntry) const
    {
        if (!mrParam.bRegExp)
            return false;

        return isTextMatchOp(rEntry);
    }

    bool isTestRegExp(const ScQueryEntry& rEntry) const
    {
        if (!mpTestEqualCondition)
            return false;

        if (!mrParam.bRegExp)
            return false;

        return (rEntry.eOp == SC_LESS_EQUAL || rEntry.eOp == SC_GREATER_EQUAL);
    }

public:
    QueryEvaluator(const ScDocument& rDoc, const ScTable& rTab, const ScQueryParam& rParam,
                   const bool* pTestEqualCondition) :
        mrDoc(rDoc),
        mrTab(rTab),
        mrParam(rParam),
        mpTestEqualCondition(pTestEqualCondition),
        mbMatchWholeCell(rDoc.GetDocOptions().IsMatchWholeCell())
    {
        if (rParam.bCaseSens)
        {
            mpTransliteration = ScGlobal::GetCaseTransliteration();
            mpCollator = ScGlobal::GetCaseCollator();
        }
        else
        {
            mpTransliteration = ScGlobal::GetpTransliteration();
            mpCollator = ScGlobal::GetCollator();
        }
    }

    bool isQueryByValue(
        const ScQueryEntry::Item& rItem, SCCOL nCol, SCROW nRow, ScRefCellValue& rCell)
    {
        if (rItem.meType == ScQueryEntry::ByString)
            return false;

        if (!rCell.isEmpty())
        {
            if (rCell.meType == CELLTYPE_FORMULA && rCell.mpFormula->GetErrCode())
                // Error values are compared as string.
                return false;

            return rCell.hasNumeric();
        }

        return mrTab.HasValueData(nCol, nRow);
    }

    bool isQueryByString(
        const ScQueryEntry& rEntry, const ScQueryEntry::Item& rItem,
        SCCOL nCol, SCROW nRow, ScRefCellValue& rCell)
    {
        if (isTextMatchOp(rEntry))
            return true;

        if (rItem.meType != ScQueryEntry::ByString)
            return false;

        if (!rCell.isEmpty())
            return rCell.hasString();

        return mrTab.HasStringData(nCol, nRow);
    }

    std::pair<bool,bool> compareByValue(
        const ScRefCellValue& rCell, SCCOL nCol, SCROW nRow,
        const ScQueryEntry& rEntry, const ScQueryEntry::Item& rItem)
    {
        bool bOk = false;
        bool bTestEqual = false;
        double nCellVal;
        if (!rCell.isEmpty())
        {
            switch (rCell.meType)
            {
                case CELLTYPE_VALUE :
                    nCellVal = rCell.mfValue;
                break;
                case CELLTYPE_FORMULA :
                    nCellVal = rCell.mpFormula->GetValue();
                break;
                default:
                    nCellVal = 0.0;
            }

        }
        else
            nCellVal = mrTab.GetValue(nCol, nRow);

        /* NOTE: lcl_PrepareQuery() prepares a filter query such that if a
         * date+time format was queried rEntry.bQueryByDate is not set. In
         * case other queries wanted to use this mechanism they should do
         * the same, in other words only if rEntry.nVal is an integer value
         * rEntry.bQueryByDate should be true and the time fraction be
         * stripped here. */
        if (rItem.meType == ScQueryEntry::ByDate)
        {
            sal_uInt32 nNumFmt = mrTab.GetNumberFormat(nCol, nRow);
            const SvNumberformat* pEntry = mrDoc.GetFormatTable()->GetEntry(nNumFmt);
            if (pEntry)
            {
                short nNumFmtType = pEntry->GetType();
                /* NOTE: Omitting the check for absence of
                 * NUMBERFORMAT_TIME would include also date+time formatted
                 * values of the same day. That may be desired in some
                 * cases, querying all time values of a day, but confusing
                 * in other cases. A user can always setup a standard
                 * filter query for x >= date AND x < date+1 */
                if ((nNumFmtType & NUMBERFORMAT_DATE) && !(nNumFmtType & NUMBERFORMAT_TIME))
                {
                    // The format is of date type.  Strip off the time
                    // element.
                    nCellVal = ::rtl::math::approxFloor(nCellVal);
                }
            }
        }

        switch (rEntry.eOp)
        {
            case SC_EQUAL :
                bOk = ::rtl::math::approxEqual(nCellVal, rItem.mfVal);
                break;
            case SC_LESS :
                bOk = (nCellVal < rItem.mfVal) && !::rtl::math::approxEqual(nCellVal, rItem.mfVal);
                break;
            case SC_GREATER :
                bOk = (nCellVal > rItem.mfVal) && !::rtl::math::approxEqual(nCellVal, rItem.mfVal);
                break;
            case SC_LESS_EQUAL :
                bOk = (nCellVal < rItem.mfVal) || ::rtl::math::approxEqual(nCellVal, rItem.mfVal);
                if ( bOk && mpTestEqualCondition )
                    bTestEqual = ::rtl::math::approxEqual(nCellVal, rItem.mfVal);
                break;
            case SC_GREATER_EQUAL :
                bOk = (nCellVal > rItem.mfVal) || ::rtl::math::approxEqual( nCellVal, rItem.mfVal);
                if ( bOk && mpTestEqualCondition )
                    bTestEqual = ::rtl::math::approxEqual(nCellVal, rItem.mfVal);
                break;
            case SC_NOT_EQUAL :
                bOk = !::rtl::math::approxEqual(nCellVal, rItem.mfVal);
                break;
            default:
            {
                // added to avoid warnings
            }
        }

        return std::pair<bool,bool>(bOk, bTestEqual);
    }

    std::pair<bool,bool> compareByString(
        ScRefCellValue& rCell, SCROW nRow, const ScQueryEntry& rEntry, const ScQueryEntry::Item& rItem)
    {
        bool bOk = false;
        bool bTestEqual = false;
        bool bMatchWholeCell = mbMatchWholeCell;
        OUString  aCellStr;
        if (isPartialTextMatchOp(rEntry))
            // may have to do partial textural comparison.
            bMatchWholeCell = false;

        if (!rCell.isEmpty())
        {
            if (rCell.meType == CELLTYPE_FORMULA && rCell.mpFormula->GetErrCode())
            {
                // Error cell is evaluated as string (for now).
                aCellStr = ScGlobal::GetErrorString(rCell.mpFormula->GetErrCode());
            }
            else
            {
                sal_uLong nFormat = mrTab.GetNumberFormat( static_cast<SCCOL>(rEntry.nField), nRow );
                ScCellFormat::GetInputString(rCell, nFormat, aCellStr, *mrDoc.GetFormatTable(), &mrDoc);
            }
        }
        else
            mrTab.GetInputString( static_cast<SCCOL>(rEntry.nField), nRow, aCellStr );

        bool bRealRegExp = isRealRegExp(rEntry);
        bool bTestRegExp = isTestRegExp(rEntry);

        if ( bRealRegExp || bTestRegExp )
        {
            sal_Int32 nStart = 0;
            sal_Int32 nEnd   = aCellStr.getLength();

            // from 614 on, nEnd is behind the found text
            bool bMatch = false;
            if ( rEntry.eOp == SC_ENDS_WITH || rEntry.eOp == SC_DOES_NOT_END_WITH )
            {
                nEnd = 0;
                nStart = aCellStr.getLength();
                bMatch = rEntry.GetSearchTextPtr( mrParam.bCaseSens )
                    ->SearchBackward( aCellStr, &nStart, &nEnd );
            }
            else
            {
                bMatch = rEntry.GetSearchTextPtr( mrParam.bCaseSens )
                    ->SearchForward( aCellStr, &nStart, &nEnd );
            }
            if ( bMatch && bMatchWholeCell
                    && (nStart != 0 || nEnd != aCellStr.getLength()) )
                bMatch = false;    // RegExp must match entire cell string
            if ( bRealRegExp )
                switch (rEntry.eOp)
            {
                case SC_EQUAL:
                case SC_CONTAINS:
                    bOk = bMatch;
                    break;
                case SC_NOT_EQUAL:
                case SC_DOES_NOT_CONTAIN:
                    bOk = !bMatch;
                    break;
                case SC_BEGINS_WITH:
                    bOk = ( bMatch && (nStart == 0) );
                    break;
                case SC_DOES_NOT_BEGIN_WITH:
                    bOk = !( bMatch && (nStart == 0) );
                    break;
                case SC_ENDS_WITH:
                    bOk = ( bMatch && (nEnd == aCellStr.getLength()) );
                    break;
                case SC_DOES_NOT_END_WITH:
                    bOk = !( bMatch && (nEnd == aCellStr.getLength()) );
                    break;
                default:
                    {
                        // added to avoid warnings
                    }
            }
            else
                bTestEqual = bMatch;
        }
        if ( !bRealRegExp )
        {
            // Simple string matching i.e. no regexp match.
            if (isTextMatchOp(rEntry))
            {
                if (rItem.meType != ScQueryEntry::ByString && rItem.maString.isEmpty())
                {
                    // #i18374# When used from functions (match, countif, sumif, vlookup, hlookup, lookup),
                    // the query value is assigned directly, and the string is empty. In that case,
                    // don't find any string (isEqual would find empty string results in formula cells).
                    bOk = false;
                    if ( rEntry.eOp == SC_NOT_EQUAL )
                        bOk = !bOk;
                }
                else if ( bMatchWholeCell )
                {
                    bOk = mpTransliteration->isEqual(aCellStr, rItem.maString);
                    if ( rEntry.eOp == SC_NOT_EQUAL )
                        bOk = !bOk;
                }
                else
                {
                    const OUString& rQueryStr = rItem.maString;
                    String aCell( mpTransliteration->transliterate(
                        aCellStr, ScGlobal::eLnge, 0, aCellStr.getLength(),
                        NULL ) );
                    String aQuer( mpTransliteration->transliterate(
                        rQueryStr, ScGlobal::eLnge, 0, rQueryStr.getLength(),
                        NULL ) );
                    xub_StrLen nIndex = (rEntry.eOp == SC_ENDS_WITH
                        || rEntry.eOp == SC_DOES_NOT_END_WITH)? (aCell.Len()-aQuer.Len()):0;
                    xub_StrLen nStrPos = aCell.Search( aQuer, nIndex );
                    switch (rEntry.eOp)
                    {
                    case SC_EQUAL:
                    case SC_CONTAINS:
                        bOk = ( nStrPos != STRING_NOTFOUND );
                        break;
                    case SC_NOT_EQUAL:
                    case SC_DOES_NOT_CONTAIN:
                        bOk = ( nStrPos == STRING_NOTFOUND );
                        break;
                    case SC_BEGINS_WITH:
                        bOk = ( nStrPos == 0 );
                        break;
                    case SC_DOES_NOT_BEGIN_WITH:
                        bOk = ( nStrPos != 0 );
                        break;
                    case SC_ENDS_WITH:
                        bOk = ( nStrPos + aQuer.Len() == aCell.Len() );
                        break;
                    case SC_DOES_NOT_END_WITH:
                        bOk = ( nStrPos + aQuer.Len() != aCell.Len() );
                        break;
                    default:
                        {
                            // added to avoid warnings
                        }
                    }
                }
            }
            else
            {   // use collator here because data was probably sorted
                sal_Int32 nCompare = mpCollator->compareString(
                    aCellStr, rItem.maString);
                switch (rEntry.eOp)
                {
                    case SC_LESS :
                        bOk = (nCompare < 0);
                        break;
                    case SC_GREATER :
                        bOk = (nCompare > 0);
                        break;
                    case SC_LESS_EQUAL :
                        bOk = (nCompare <= 0);
                        if ( bOk && mpTestEqualCondition && !bTestEqual )
                            bTestEqual = (nCompare == 0);
                        break;
                    case SC_GREATER_EQUAL :
                        bOk = (nCompare >= 0);
                        if ( bOk && mpTestEqualCondition && !bTestEqual )
                            bTestEqual = (nCompare == 0);
                        break;
                    default:
                    {
                        // added to avoid warnings
                    }
                }
            }
        }

        return std::pair<bool,bool>(bOk, bTestEqual);
    }
};

}

bool ScTable::ValidQuery(
    SCROW nRow, const ScQueryParam& rParam, ScRefCellValue* pCell, bool* pbTestEqualCondition)
{
    if (!rParam.GetEntry(0).bDoQuery)
        return true;

    SCSIZE nEntryCount = rParam.GetEntryCount();

    typedef std::pair<bool,bool> ResultType;
    static std::vector<ResultType> aResults;
    if (aResults.size() < nEntryCount)
        aResults.resize(nEntryCount);

    long    nPos = -1;
    QueryEvaluator aEval(*pDocument, *this, rParam, pbTestEqualCondition);
    ScQueryParam::const_iterator it, itBeg = rParam.begin(), itEnd = rParam.end();
    for (it = itBeg; it != itEnd && it->bDoQuery; ++it)
    {
        const ScQueryEntry& rEntry = *it;
        SCCOL nCol = static_cast<SCCOL>(rEntry.nField);

        // we can only handle one single direct query
        ScRefCellValue aCell;
        if (pCell && it == itBeg)
            aCell = *pCell;
        else
            aCell = GetCellValue(nCol, nRow);

        std::pair<bool,bool> aRes(false, false);

        const ScQueryEntry::QueryItemsType& rItems = rEntry.GetQueryItems();
        if (rItems.size() == 1 && rItems.front().meType == ScQueryEntry::ByEmpty)
        {
            if (rEntry.IsQueryByEmpty())
                aRes.first = !aCol[rEntry.nField].HasDataAt(nRow);
            else
            {
                OSL_ASSERT(rEntry.IsQueryByNonEmpty());
                aRes.first = aCol[rEntry.nField].HasDataAt(nRow);
            }
        }
        else
        {
            ScQueryEntry::QueryItemsType::const_iterator itr = rItems.begin(), itrEnd = rItems.end();

            for (; itr != itrEnd; ++itr)
            {
                if (aEval.isQueryByValue(*itr, nCol, nRow, aCell))
                {
                    std::pair<bool,bool> aThisRes =
                        aEval.compareByValue(aCell, nCol, nRow, rEntry, *itr);
                    aRes.first |= aThisRes.first;
                    aRes.second |= aThisRes.second;
                }
                else if (aEval.isQueryByString(rEntry, *itr, nCol, nRow, aCell))
                {
                    std::pair<bool,bool> aThisRes =
                        aEval.compareByString(aCell, nRow, rEntry, *itr);
                    aRes.first |= aThisRes.first;
                    aRes.second |= aThisRes.second;
                }

                if (aRes.first && aRes.second)
                    break;
            }
        }

        if (nPos == -1)
        {
            nPos++;
            aResults[nPos] = aRes;
        }
        else
        {
            if (rEntry.eConnect == SC_AND)
            {
                aResults[nPos].first = aResults[nPos].first && aRes.first;
                aResults[nPos].second = aResults[nPos].second && aRes.second;
            }
            else
            {
                nPos++;
                aResults[nPos] = aRes;
            }
        }
    }

    for ( long j=1; j <= nPos; j++ )
    {
        aResults[0].first = aResults[0].first || aResults[j].first;
        aResults[0].second = aResults[0].second || aResults[j].second;
    }

    bool bRet = aResults[0].first;
    if ( pbTestEqualCondition )
        *pbTestEqualCondition = aResults[0].second;

    return bRet;
}

void ScTable::TopTenQuery( ScQueryParam& rParam )
{
    bool bSortCollatorInitialized = false;
    SCSIZE nEntryCount = rParam.GetEntryCount();
    SCROW nRow1 = (rParam.bHasHeader ? rParam.nRow1 + 1 : rParam.nRow1);
    SCSIZE nCount = static_cast<SCSIZE>(rParam.nRow2 - nRow1 + 1);
    for ( SCSIZE i=0; (i<nEntryCount) && (rParam.GetEntry(i).bDoQuery); i++ )
    {
        ScQueryEntry& rEntry = rParam.GetEntry(i);
        ScQueryEntry::Item& rItem = rEntry.GetQueryItem();

        switch ( rEntry.eOp )
        {
            case SC_TOPVAL:
            case SC_BOTVAL:
            case SC_TOPPERC:
            case SC_BOTPERC:
            {
                ScSortParam aLocalSortParam( rParam, static_cast<SCCOL>(rEntry.nField) );
                aSortParam = aLocalSortParam;       // used in CreateSortInfoArray, Compare
                if ( !bSortCollatorInitialized )
                {
                    bSortCollatorInitialized = true;
                    InitSortCollator( aLocalSortParam );
                }
                ScSortInfoArray* pArray = CreateSortInfoArray( nRow1, rParam.nRow2 );
                DecoladeRow( pArray, nRow1, rParam.nRow2 );
                QuickSort( pArray, nRow1, rParam.nRow2 );
                ScSortInfo** ppInfo = pArray->GetFirstArray();
                SCSIZE nValidCount = nCount;
                // keine Note-/Leerzellen zaehlen, sind ans Ende sortiert
                while (nValidCount > 0 && ppInfo[nValidCount-1]->maCell.isEmpty())
                    nValidCount--;
                // keine Strings zaehlen, sind zwischen Value und Leer
                while (nValidCount > 0 && ppInfo[nValidCount-1]->maCell.hasString())
                    nValidCount--;
                if ( nValidCount > 0 )
                {
                    if ( rItem.meType == ScQueryEntry::ByString )
                    {   // dat wird nix
                        rItem.meType = ScQueryEntry::ByValue;
                        rItem.mfVal = 10;   // 10 bzw. 10%
                    }
                    SCSIZE nVal = (rItem.mfVal >= 1 ? static_cast<SCSIZE>(rItem.mfVal) : 1);
                    SCSIZE nOffset = 0;
                    switch ( rEntry.eOp )
                    {
                        case SC_TOPVAL:
                        {
                            rEntry.eOp = SC_GREATER_EQUAL;
                            if ( nVal > nValidCount )
                                nVal = nValidCount;
                            nOffset = nValidCount - nVal;   // 1 <= nVal <= nValidCount
                        }
                        break;
                        case SC_BOTVAL:
                        {
                            rEntry.eOp = SC_LESS_EQUAL;
                            if ( nVal > nValidCount )
                                nVal = nValidCount;
                            nOffset = nVal - 1;     // 1 <= nVal <= nValidCount
                        }
                        break;
                        case SC_TOPPERC:
                        {
                            rEntry.eOp = SC_GREATER_EQUAL;
                            if ( nVal > 100 )
                                nVal = 100;
                            nOffset = nValidCount - (nValidCount * nVal / 100);
                            if ( nOffset >= nValidCount )
                                nOffset = nValidCount - 1;
                        }
                        break;
                        case SC_BOTPERC:
                        {
                            rEntry.eOp = SC_LESS_EQUAL;
                            if ( nVal > 100 )
                                nVal = 100;
                            nOffset = (nValidCount * nVal / 100);
                            if ( nOffset >= nValidCount )
                                nOffset = nValidCount - 1;
                        }
                        break;
                        default:
                        {
                            // added to avoid warnings
                        }
                    }
                    ScRefCellValue aCell = ppInfo[nOffset]->maCell;
                    if (aCell.hasNumeric())
                        rItem.mfVal = aCell.getValue();
                    else
                    {
                        OSL_FAIL( "TopTenQuery: pCell no ValueData" );
                        rEntry.eOp = SC_GREATER_EQUAL;
                        rItem.mfVal = 0;
                    }
                }
                else
                {
                    rEntry.eOp = SC_GREATER_EQUAL;
                    rItem.meType = ScQueryEntry::ByValue;
                    rItem.mfVal = 0;
                }
                delete pArray;
            }
            break;
            default:
            {
                // added to avoid warnings
            }
        }
    }
    if ( bSortCollatorInitialized )
        DestroySortCollator();
}

namespace {

class PrepareQueryItem : public std::unary_function<ScQueryEntry::Item, void>
{
    const ScDocument& mrDoc;
public:
    PrepareQueryItem(const ScDocument& rDoc) : mrDoc(rDoc) {}

    void operator() (ScQueryEntry::Item& rItem)
    {
        if (rItem.meType != ScQueryEntry::ByString && rItem.meType != ScQueryEntry::ByDate)
            return;

        sal_uInt32 nIndex = 0;
        bool bNumber = mrDoc.GetFormatTable()->
            IsNumberFormat(rItem.maString, nIndex, rItem.mfVal);

        // Advanced Filter creates only ByString queries that need to be
        // converted to ByValue if appropriate. rItem.mfVal now holds the value
        // if bNumber==true.

        if (rItem.meType == ScQueryEntry::ByString)
        {
            if (bNumber)
                rItem.meType = ScQueryEntry::ByValue;
            return;
        }

        // Double-check if the query by date is really appropriate.

        if (bNumber && ((nIndex % SV_COUNTRY_LANGUAGE_OFFSET) != 0))
        {
            const SvNumberformat* pEntry = mrDoc.GetFormatTable()->GetEntry(nIndex);
            if (pEntry)
            {
                short nNumFmtType = pEntry->GetType();
                if (!((nNumFmtType & NUMBERFORMAT_DATE) && !(nNumFmtType & NUMBERFORMAT_TIME)))
                    rItem.meType = ScQueryEntry::ByValue;    // not a date only
            }
            else
                rItem.meType = ScQueryEntry::ByValue;    // what the ... not a date
        }
        else
            rItem.meType = ScQueryEntry::ByValue;    // not a date
    }
};

void lcl_PrepareQuery( const ScDocument* pDoc, ScTable* pTab, ScQueryParam& rParam )
{
    bool bTopTen = false;
    SCSIZE nEntryCount = rParam.GetEntryCount();

    for ( SCSIZE i = 0; i < nEntryCount; ++i )
    {
        ScQueryEntry& rEntry = rParam.GetEntry(i);
        if (!rEntry.bDoQuery)
            continue;

        ScQueryEntry::QueryItemsType& rItems = rEntry.GetQueryItems();
        std::for_each(rItems.begin(), rItems.end(), PrepareQueryItem(*pDoc));

        if ( !bTopTen )
        {
            switch ( rEntry.eOp )
            {
                case SC_TOPVAL:
                case SC_BOTVAL:
                case SC_TOPPERC:
                case SC_BOTPERC:
                {
                    bTopTen = true;
                }
                break;
                default:
                {
                }
            }
        }
    }

    if ( bTopTen )
    {
        pTab->TopTenQuery( rParam );
    }
}

}

SCSIZE ScTable::Query(ScQueryParam& rParamOrg, bool bKeepSub)
{
    ScQueryParam    aParam( rParamOrg );
    typedef boost::unordered_set<OUString, OUStringHash> StrSetType;
    StrSetType aStrSet;

    bool    bStarted = false;
    bool    bOldResult = true;
    SCROW   nOldStart = 0;
    SCROW   nOldEnd = 0;

    SCSIZE nCount   = 0;
    SCROW nOutRow   = 0;
    SCROW nHeader   = aParam.bHasHeader ? 1 : 0;

    lcl_PrepareQuery(pDocument, this, aParam);

    if (!aParam.bInplace)
    {
        nOutRow = aParam.nDestRow + nHeader;
        if (nHeader > 0)
            CopyData( aParam.nCol1, aParam.nRow1, aParam.nCol2, aParam.nRow1,
                            aParam.nDestCol, aParam.nDestRow, aParam.nDestTab );
    }


    if (aParam.bInplace)
        InitializeNoteCaptions();

    SCROW nRealRow2 = aParam.nRow2;
    for (SCROW j = aParam.nRow1 + nHeader; j <= nRealRow2; ++j)
    {
        bool bResult;                                   // Filterergebnis
        bool bValid = ValidQuery(j, aParam);
        if (!bValid && bKeepSub)                        // Subtotals stehenlassen
        {
            for (SCCOL nCol=aParam.nCol1; nCol<=aParam.nCol2 && !bValid; nCol++)
            {
                ScRefCellValue aCell = GetCellValue(nCol, j);
                if (aCell.meType != CELLTYPE_FORMULA)
                    continue;

                if (!aCell.mpFormula->IsSubTotal())
                    continue;

                if (RefVisible(aCell.mpFormula))
                    bValid = true;
            }
        }
        if (bValid)
        {
            if (aParam.bDuplicate)
                bResult = true;
            else
            {
                OUString aStr;
                for (SCCOL k=aParam.nCol1; k <= aParam.nCol2; k++)
                {
                    OUString aCellStr;
                    GetString(k, j, aCellStr);
                    OUStringBuffer aBuf(aStr);
                    aBuf.append(aCellStr);
                    aBuf.append(static_cast<sal_Unicode>(1));
                    aStr = aBuf.makeStringAndClear();
                }

                std::pair<StrSetType::iterator, bool> r = aStrSet.insert(aStr);
                bool bIsUnique = r.second; // unique if inserted.
                bResult = bIsUnique;
            }
        }
        else
            bResult = false;

        if (aParam.bInplace)
        {
            if (bResult == bOldResult && bStarted)
                nOldEnd = j;
            else
            {
                if (bStarted)
                    DBShowRows(nOldStart,nOldEnd, bOldResult);
                nOldStart = nOldEnd = j;
                bOldResult = bResult;
            }
            bStarted = true;
        }
        else
        {
            if (bResult)
            {
                CopyData( aParam.nCol1,j, aParam.nCol2,j, aParam.nDestCol,nOutRow,aParam.nDestTab );
                ++nOutRow;
            }
        }
        if (bResult)
            ++nCount;
    }

    if (aParam.bInplace && bStarted)
        DBShowRows(nOldStart,nOldEnd, bOldResult);

    if (aParam.bInplace)
        SetDrawPageSize();

    return nCount;
}

bool ScTable::CreateExcelQuery(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ScQueryParam& rQueryParam)
{
    bool    bValid = true;
    SCCOL* pFields = new SCCOL[nCol2-nCol1+1];
    OUString  aCellStr;
    SCCOL   nCol = nCol1;
    OSL_ENSURE( rQueryParam.nTab != SCTAB_MAX, "rQueryParam.nTab no value, not bad but no good" );
    SCTAB   nDBTab = (rQueryParam.nTab == SCTAB_MAX ? nTab : rQueryParam.nTab);
    SCROW   nDBRow1 = rQueryParam.nRow1;
    SCCOL   nDBCol2 = rQueryParam.nCol2;
    // Erste Zeile muessen Spaltenkoepfe sein
    while (bValid && (nCol <= nCol2))
    {
        OUString aQueryStr;
        GetUpperCellString(nCol, nRow1, aQueryStr);
        bool bFound = false;
        SCCOL i = rQueryParam.nCol1;
        while (!bFound && (i <= nDBCol2))
        {
            if ( nTab == nDBTab )
                GetUpperCellString(i, nDBRow1, aCellStr);
            else
                pDocument->GetUpperCellString(i, nDBRow1, nDBTab, aCellStr);
            bFound = (aCellStr == aQueryStr);
            if (!bFound) i++;
        }
        if (bFound)
            pFields[nCol - nCol1] = i;
        else
            bValid = false;
        nCol++;
    }
    if (bValid)
    {
        sal_uLong nVisible = 0;
        for ( nCol=nCol1; nCol<=nCol2; nCol++ )
            nVisible += aCol[nCol].VisibleCount( nRow1+1, nRow2 );

        if ( nVisible > SCSIZE_MAX / sizeof(void*) )
        {
            OSL_FAIL("too many filter criteria");
            nVisible = 0;
        }

        SCSIZE nNewEntries = nVisible;
        rQueryParam.Resize( nNewEntries );

        SCSIZE nIndex = 0;
        SCROW nRow = nRow1 + 1;
        while (nRow <= nRow2)
        {
            nCol = nCol1;
            while (nCol <= nCol2)
            {
                GetInputString( nCol, nRow, aCellStr );
                if (!aCellStr.isEmpty())
                {
                    if (nIndex < nNewEntries)
                    {
                        rQueryParam.GetEntry(nIndex).nField = pFields[nCol - nCol1];
                        rQueryParam.FillInExcelSyntax(aCellStr, nIndex);
                        nIndex++;
                        if (nIndex < nNewEntries)
                            rQueryParam.GetEntry(nIndex).eConnect = SC_AND;
                    }
                    else
                        bValid = false;
                }
                nCol++;
            }
            nRow++;
            if (nIndex < nNewEntries)
                rQueryParam.GetEntry(nIndex).eConnect = SC_OR;
        }
    }
    delete [] pFields;
    return bValid;
}

bool ScTable::CreateStarQuery(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ScQueryParam& rQueryParam)
{
    // A valid StarQuery must be at least 4 columns wide. To be precise it
    // should be exactly 4 columns ...
    // Additionally, if this wasn't checked, a formula pointing to a valid 1-3
    // column Excel style query range immediately left to itself would result
    // in a circular reference when the field name or operator or value (first
    // to third query range column) is obtained (#i58354#). Furthermore, if the
    // range wasn't sufficiently specified data changes wouldn't flag formula
    // cells for recalculation.
    if (nCol2 - nCol1 < 3)
        return false;

    bool bValid;
    bool bFound;
    OUString aCellStr;
    SCSIZE nIndex = 0;
    SCROW nRow = nRow1;
    OSL_ENSURE( rQueryParam.nTab != SCTAB_MAX, "rQueryParam.nTab no value, not bad but no good" );
    SCTAB   nDBTab = (rQueryParam.nTab == SCTAB_MAX ? nTab : rQueryParam.nTab);
    SCROW   nDBRow1 = rQueryParam.nRow1;
    SCCOL   nDBCol2 = rQueryParam.nCol2;

    SCSIZE nNewEntries = static_cast<SCSIZE>(nRow2-nRow1+1);
    rQueryParam.Resize( nNewEntries );

    do
    {
        ScQueryEntry& rEntry = rQueryParam.GetEntry(nIndex);

        bValid = false;
        // Erste Spalte UND/ODER
        if (nIndex > 0)
        {
            GetUpperCellString(nCol1, nRow, aCellStr);
            if ( aCellStr == ScGlobal::GetRscString(STR_TABLE_UND) )
            {
                rEntry.eConnect = SC_AND;
                bValid = true;
            }
            else if ( aCellStr == ScGlobal::GetRscString(STR_TABLE_ODER) )
            {
                rEntry.eConnect = SC_OR;
                bValid = true;
            }
        }
        // Zweite Spalte FeldName
        if ((nIndex < 1) || bValid)
        {
            bFound = false;
            GetUpperCellString(nCol1 + 1, nRow, aCellStr);
            for (SCCOL i=rQueryParam.nCol1; (i <= nDBCol2) && (!bFound); i++)
            {
                OUString aFieldStr;
                if ( nTab == nDBTab )
                    GetUpperCellString(i, nDBRow1, aFieldStr);
                else
                    pDocument->GetUpperCellString(i, nDBRow1, nDBTab, aFieldStr);
                bFound = (aCellStr == aFieldStr);
                if (bFound)
                {
                    rEntry.nField = i;
                    bValid = true;
                }
                else
                    bValid = false;
            }
        }
        // Dritte Spalte Operator =<>...
        if (bValid)
        {
            bFound = false;
            GetUpperCellString(nCol1 + 2, nRow, aCellStr);
            if (aCellStr[0] == '<')
            {
                if (aCellStr[1] == '>')
                    rEntry.eOp = SC_NOT_EQUAL;
                else if (aCellStr[1] == '=')
                    rEntry.eOp = SC_LESS_EQUAL;
                else
                    rEntry.eOp = SC_LESS;
            }
            else if (aCellStr[0] == '>')
            {
                if (aCellStr[1] == '=')
                    rEntry.eOp = SC_GREATER_EQUAL;
                else
                    rEntry.eOp = SC_GREATER;
            }
            else if (aCellStr[0] == '=')
                rEntry.eOp = SC_EQUAL;

        }
        // Vierte Spalte Wert
        if (bValid)
        {
            OUString aStr;
            GetString(nCol1 + 3, nRow, aStr);
            rEntry.GetQueryItem().maString = aStr;
            rEntry.bDoQuery = true;
        }
        nIndex++;
        nRow++;
    }
    while (bValid && (nRow <= nRow2) /* && (nIndex < MAXQUERY) */ );
    return bValid;
}

bool ScTable::CreateQueryParam(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ScQueryParam& rQueryParam)
{
    SCSIZE i, nCount;
    PutInOrder(nCol1, nCol2);
    PutInOrder(nRow1, nRow2);

    nCount = rQueryParam.GetEntryCount();
    for (i=0; i < nCount; i++)
        rQueryParam.GetEntry(i).Clear();

    // Standard QueryTabelle
    bool bValid = CreateStarQuery(nCol1, nRow1, nCol2, nRow2, rQueryParam);
    // Excel QueryTabelle
    if (!bValid)
        bValid = CreateExcelQuery(nCol1, nRow1, nCol2, nRow2, rQueryParam);

    nCount = rQueryParam.GetEntryCount();
    if (bValid)
    {
        //  bQueryByString muss gesetzt sein
        for (i=0; i < nCount; i++)
            rQueryParam.GetEntry(i).GetQueryItem().meType = ScQueryEntry::ByString;
    }
    else
    {
        //  nix
        for (i=0; i < nCount; i++)
            rQueryParam.GetEntry(i).Clear();
    }
    return bValid;
}

bool ScTable::HasColHeader( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW /* nEndRow */ ) const
{
    for (SCCOL nCol=nStartCol; nCol<=nEndCol; nCol++)
    {
        CellType eType = GetCellType( nCol, nStartRow );
        if (eType != CELLTYPE_STRING && eType != CELLTYPE_EDIT)
            return false;
    }
    return true;
}

bool ScTable::HasRowHeader( SCCOL nStartCol, SCROW nStartRow, SCCOL /* nEndCol */, SCROW nEndRow ) const
{
    for (SCROW nRow=nStartRow; nRow<=nEndRow; nRow++)
    {
        CellType eType = GetCellType( nStartCol, nRow );
        if (eType != CELLTYPE_STRING && eType != CELLTYPE_EDIT)
            return false;
    }
    return true;
}

void ScTable::GetFilterEntries(SCCOL nCol, SCROW nRow1, SCROW nRow2, std::vector<ScTypedStrData>& rStrings, bool& rHasDates)
{
    aCol[nCol].GetFilterEntries( nRow1, nRow2, rStrings, rHasDates );
}

void ScTable::GetFilteredFilterEntries(
    SCCOL nCol, SCROW nRow1, SCROW nRow2, const ScQueryParam& rParam, std::vector<ScTypedStrData>& rStrings, bool& rHasDates)
{
    // remove the entry for this column from the query parameter
    ScQueryParam aParam( rParam );
    aParam.RemoveEntryByField(nCol);

    lcl_PrepareQuery(pDocument, this, aParam);
    bool bHasDates = false;
    for ( SCROW j = nRow1; j <= nRow2; ++j )
    {
        if (ValidQuery(j, aParam))
        {
            bool bThisHasDates = false;
            aCol[nCol].GetFilterEntries( j, j, rStrings, bThisHasDates );
            bHasDates |= bThisHasDates;
        }
    }

    rHasDates = bHasDates;
}

bool ScTable::GetDataEntries(SCCOL nCol, SCROW nRow, std::set<ScTypedStrData>& rStrings, bool bLimit)
{
    return aCol[nCol].GetDataEntries( nRow, rStrings, bLimit );
}

ScDocument& ScTable::GetDoc()
{
    return *pDocument;
}

const ScDocument& ScTable::GetDoc() const
{
    return *pDocument;
}

SCSIZE ScTable::GetCellCount(SCCOL nCol) const
{
    return aCol[nCol].GetCellCount();
}

sal_uLong ScTable::GetCellCount() const
{
    sal_uLong nCellCount = 0;

    for ( SCCOL nCol=0; nCol<=MAXCOL; nCol++ )
        nCellCount += aCol[nCol].GetCellCount();

    return nCellCount;
}

sal_uLong ScTable::GetWeightedCount() const
{
    sal_uLong nCellCount = 0;

    for ( SCCOL nCol=0; nCol<=MAXCOL; nCol++ )
        if ( aCol[nCol].GetCellCount() )                    // GetCellCount ist inline
            nCellCount += aCol[nCol].GetWeightedCount();

    return nCellCount;
}

sal_uLong ScTable::GetCodeCount() const
{
    sal_uLong nCodeCount = 0;

    for ( SCCOL nCol=0; nCol<=MAXCOL; nCol++ )
        if ( aCol[nCol].GetCellCount() )                    // GetCellCount ist inline
            nCodeCount += aCol[nCol].GetCodeCount();

    return nCodeCount;
}

sal_Int32 ScTable::GetMaxStringLen( SCCOL nCol, SCROW nRowStart,
        SCROW nRowEnd, CharSet eCharSet ) const
{
    if ( ValidCol(nCol) )
        return aCol[nCol].GetMaxStringLen( nRowStart, nRowEnd, eCharSet );
    else
        return 0;
}

xub_StrLen ScTable::GetMaxNumberStringLen(
    sal_uInt16& nPrecision, SCCOL nCol, SCROW nRowStart, SCROW nRowEnd ) const
{
    if ( ValidCol(nCol) )
        return aCol[nCol].GetMaxNumberStringLen( nPrecision, nRowStart, nRowEnd );
    else
        return 0;
}

void ScTable::UpdateSelectionFunction( ScFunctionData& rData,
                        SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                        const ScMarkData& rMark )
{
    //  Cursor neben einer Markierung nicht beruecksichtigen:
    //! nur noch MarkData uebergeben, Cursorposition ggf. hineinselektieren!!!
    bool bSingle = ( rMark.IsMarked() || !rMark.IsMultiMarked() );

    // Mehrfachselektion:

    SCCOL nCol;
    if ( rMark.IsMultiMarked() )
        for (nCol=0; nCol<=MAXCOL && !rData.bError; nCol++)
            if ( !pColFlags || !ColHidden(nCol) )
                aCol[nCol].UpdateSelectionFunction( rMark, rData, *mpHiddenRows,
                                                    bSingle && ( nCol >= nStartCol && nCol <= nEndCol ),
                                                    nStartRow, nEndRow );

    //  Einfachselektion (oder Cursor) nur wenn nicht negativ (und s.o.):

    if ( bSingle && !rMark.IsMarkNegative() )
        for (nCol=nStartCol; nCol<=nEndCol && !rData.bError; nCol++)
            if ( !pColFlags || !ColHidden(nCol) )
                aCol[nCol].UpdateAreaFunction( rData, *mpHiddenRows, nStartRow, nEndRow );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
