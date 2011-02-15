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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

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
#include "collect.hxx"
#include "attrib.hxx"
#include "cell.hxx"
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
#include "segmenttree.hxx"

#include <vector>

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
bool SplitString( const rtl::OUString &sWhole,
    rtl::OUString &sPrefix, rtl::OUString &sSuffix, double &fNum )
{
    i18n::LocaleDataItem aLocaleItem = ScGlobal::pLocaleData->getLocaleItem();

    // Get prefix element
    rtl::OUString sEmpty, sUser = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "-" ));
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
               const BOOL bCaseSens, const ScUserListData* pData, const CollatorWrapper *pCW )
{
    rtl::OUString sStr1( sInput1 ), sStr2( sInput2 ), sPre1, sSuf1, sPre2, sSuf2;

    do
    {
        double nNum1, nNum2;
        BOOL bNumFound1 = SplitString( sStr1, sPre1, sSuf1, nNum1 );
        BOOL bNumFound2 = SplitString( sStr2, sPre2, sSuf2, nNum2 );

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

const USHORT nMaxSorts = 3;     // maximale Anzahl Sortierkriterien in aSortParam

struct ScSortInfo
{
    ScBaseCell*     pCell;
    SCCOLROW        nOrg;
    DECL_FIXEDMEMPOOL_NEWDEL( ScSortInfo );
};
const USHORT nMemPoolSortInfo = (0x8000 - 64) / sizeof(ScSortInfo);
IMPL_FIXEDMEMPOOL_NEWDEL( ScSortInfo, nMemPoolSortInfo, nMemPoolSortInfo )

// END OF STATIC DATA -----------------------------------------------------


class ScSortInfoArray
{
private:
    ScSortInfo**    pppInfo[nMaxSorts];
    SCSIZE          nCount;
    SCCOLROW        nStart;
    USHORT          nUsedSorts;

public:
                ScSortInfoArray( USHORT nSorts, SCCOLROW nInd1, SCCOLROW nInd2 ) :
                        nCount( nInd2 - nInd1 + 1 ), nStart( nInd1 ),
                        nUsedSorts( Min( nSorts, nMaxSorts ) )
                    {
                        for ( USHORT nSort = 0; nSort < nUsedSorts; nSort++ )
                        {
                            ScSortInfo** ppInfo = new ScSortInfo* [nCount];
                            for ( SCSIZE j = 0; j < nCount; j++ )
                                ppInfo[j] = new ScSortInfo;
                            pppInfo[nSort] = ppInfo;
                        }
                    }
                ~ScSortInfoArray()
                    {
                        for ( USHORT nSort = 0; nSort < nUsedSorts; nSort++ )
                        {
                            ScSortInfo** ppInfo = pppInfo[nSort];
                            for ( SCSIZE j = 0; j < nCount; j++ )
                                delete ppInfo[j];
                            delete [] ppInfo;
                        }
                    }
    ScSortInfo* Get( USHORT nSort, SCCOLROW nInd )
                    { return (pppInfo[nSort])[ nInd - nStart ]; }
    void        Swap( SCCOLROW nInd1, SCCOLROW nInd2 )
                    {
                        SCSIZE n1 = static_cast<SCSIZE>(nInd1 - nStart);
                        SCSIZE n2 = static_cast<SCSIZE>(nInd2 - nStart);
                        for ( USHORT nSort = 0; nSort < nUsedSorts; nSort++ )
                        {
                            ScSortInfo** ppInfo = pppInfo[nSort];
                            ScSortInfo* pTmp = ppInfo[n1];
                            ppInfo[n1] = ppInfo[n2];
                            ppInfo[n2] = pTmp;
                        }
                    }
    USHORT      GetUsedSorts() { return nUsedSorts; }
    ScSortInfo**    GetFirstArray() { return pppInfo[0]; }
    SCCOLROW    GetStart() { return nStart; }
    SCSIZE      GetCount() { return nCount; }
};

ScSortInfoArray* ScTable::CreateSortInfoArray( SCCOLROW nInd1, SCCOLROW nInd2 )
{
    USHORT nUsedSorts = 1;
    while ( nUsedSorts < nMaxSorts && aSortParam.bDoSort[nUsedSorts] )
        nUsedSorts++;
    ScSortInfoArray* pArray = new ScSortInfoArray( nUsedSorts, nInd1, nInd2 );
    if ( aSortParam.bByRow )
    {
        for ( USHORT nSort = 0; nSort < nUsedSorts; nSort++ )
        {
            SCCOL nCol = static_cast<SCCOL>(aSortParam.nField[nSort]);
            ScColumn* pCol = &aCol[nCol];
            for ( SCROW nRow = nInd1; nRow <= nInd2; nRow++ )
            {
//2do: FillSortInfo an ScColumn und Array abklappern statt Search in GetCell
                ScSortInfo* pInfo = pArray->Get( nSort, nRow );
                pInfo->pCell = pCol->GetCell( nRow );
                pInfo->nOrg = nRow;
            }
        }
    }
    else
    {
        for ( USHORT nSort = 0; nSort < nUsedSorts; nSort++ )
        {
            SCROW nRow = aSortParam.nField[nSort];
            for ( SCCOL nCol = static_cast<SCCOL>(nInd1);
                    nCol <= static_cast<SCCOL>(nInd2); nCol++ )
            {
                ScSortInfo* pInfo = pArray->Get( nSort, nCol );
                pInfo->pCell = GetCell( nCol, nRow );
                pInfo->nOrg = nCol;
            }
        }
    }
    return pArray;
}


BOOL ScTable::IsSortCollatorGlobal() const
{
    return  pSortCollator == ScGlobal::GetCollator() ||
            pSortCollator == ScGlobal::GetCaseCollator();
}


void ScTable::InitSortCollator( const ScSortParam& rPar )
{
    if ( rPar.aCollatorLocale.Language.getLength() )
    {
        if ( !pSortCollator || IsSortCollatorGlobal() )
            pSortCollator = new CollatorWrapper( pDocument->GetServiceManager() );
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


void ScTable::SortReorder( ScSortInfoArray* pArray, ScProgress& rProgress )
{
    BOOL bByRow = aSortParam.bByRow;
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
            DBG_ASSERT( p == ppInfo[nPos], "SortReorder: nOrg MisMatch" );
        }
        rProgress.SetStateOnPercent( nPos );
    }
}

short ScTable::CompareCell( USHORT nSort,
            ScBaseCell* pCell1, SCCOL nCell1Col, SCROW nCell1Row,
            ScBaseCell* pCell2, SCCOL nCell2Col, SCROW nCell2Row )
{
    short nRes = 0;

    CellType eType1 = CELLTYPE_NONE, eType2 = CELLTYPE_NONE;
    if (pCell1)
    {
        eType1 = pCell1->GetCellType();
        if (eType1 == CELLTYPE_NOTE)
            pCell1 = NULL;
    }
    if (pCell2)
    {
        eType2 = pCell2->GetCellType();
        if (eType2 == CELLTYPE_NOTE)
            pCell2 = NULL;
    }

    if (pCell1)
    {
        if (pCell2)
        {
            BOOL bStr1 = ( eType1 != CELLTYPE_VALUE );
            if ( eType1 == CELLTYPE_FORMULA && ((ScFormulaCell*)pCell1)->IsValue() )
                bStr1 = FALSE;
            BOOL bStr2 = ( eType2 != CELLTYPE_VALUE );
            if ( eType2 == CELLTYPE_FORMULA && ((ScFormulaCell*)pCell2)->IsValue() )
                bStr2 = FALSE;

            if ( bStr1 && bStr2 )           // nur Strings untereinander als String vergleichen!
            {
                String aStr1;
                String aStr2;
                if (eType1 == CELLTYPE_STRING)
                    ((ScStringCell*)pCell1)->GetString(aStr1);
                else
                    GetString(nCell1Col, nCell1Row, aStr1);
                if (eType2 == CELLTYPE_STRING)
                    ((ScStringCell*)pCell2)->GetString(aStr2);
                else
                    GetString(nCell2Col, nCell2Row, aStr2);

                BOOL bUserDef     = aSortParam.bUserDef;        // custom sort order
                BOOL bNaturalSort = aSortParam.bNaturalSort;    // natural sort
                BOOL bCaseSens    = aSortParam.bCaseSens;       // case sensitivity

                if (bUserDef)
                {
                    ScUserListData* pData =
                        static_cast<ScUserListData*>( (ScGlobal::GetUserList()->At(
                        aSortParam.nUserIndex)) );

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
                        bUserDef = FALSE;

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
                double nVal1;
                double nVal2;
                if (eType1 == CELLTYPE_VALUE)
                    nVal1 = ((ScValueCell*)pCell1)->GetValue();
                else if (eType1 == CELLTYPE_FORMULA)
                    nVal1 = ((ScFormulaCell*)pCell1)->GetValue();
                else
                    nVal1 = 0;
                if (eType2 == CELLTYPE_VALUE)
                    nVal2 = ((ScValueCell*)pCell2)->GetValue();
                else if (eType2 == CELLTYPE_FORMULA)
                    nVal2 = ((ScFormulaCell*)pCell2)->GetValue();
                else
                    nVal2 = 0;
                if (nVal1 < nVal2)
                    nRes = -1;
                else if (nVal1 > nVal2)
                    nRes = 1;
            }
            if ( !aSortParam.bAscending[nSort] )
                nRes = -nRes;
        }
        else
            nRes = -1;
    }
    else
    {
        if ( pCell2 )
            nRes = 1;
        else
            nRes = 0;                   // beide leer
    }
    return nRes;
}

short ScTable::Compare( ScSortInfoArray* pArray, SCCOLROW nIndex1, SCCOLROW nIndex2 )
{
    short nRes;
    USHORT nSort = 0;
    do
    {
        ScSortInfo* pInfo1 = pArray->Get( nSort, nIndex1 );
        ScSortInfo* pInfo2 = pArray->Get( nSort, nIndex2 );
        if ( aSortParam.bByRow )
            nRes = CompareCell( nSort,
                pInfo1->pCell, static_cast<SCCOL>(aSortParam.nField[nSort]), pInfo1->nOrg,
                pInfo2->pCell, static_cast<SCCOL>(aSortParam.nField[nSort]), pInfo2->nOrg );
        else
            nRes = CompareCell( nSort,
                pInfo1->pCell, static_cast<SCCOL>(pInfo1->nOrg), aSortParam.nField[nSort],
                pInfo2->pCell, static_cast<SCCOL>(pInfo2->nOrg), aSortParam.nField[nSort] );
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
    for (SCROW nRow = aSortParam.nRow1; nRow <= aSortParam.nRow2; nRow++)
    {
        aCol[nCol1].SwapCell(nRow, aCol[nCol2]);
        if (aSortParam.bIncludePattern)
        {
            const ScPatternAttr* pPat1 = GetPattern(nCol1, nRow);
            const ScPatternAttr* pPat2 = GetPattern(nCol2, nRow);
            if (pPat1 != pPat2)
            {
                SetPattern(nCol1, nRow, *pPat2, TRUE);
                SetPattern(nCol2, nRow, *pPat1, TRUE);
            }
        }
    }
}

void ScTable::SwapRow(SCROW nRow1, SCROW nRow2)
{
    for (SCCOL nCol = aSortParam.nCol1; nCol <= aSortParam.nCol2; nCol++)
    {
        aCol[nCol].SwapRow(nRow1, nRow2);
        if (aSortParam.bIncludePattern)
        {
            const ScPatternAttr* pPat1 = GetPattern(nCol, nRow1);
            const ScPatternAttr* pPat2 = GetPattern(nCol, nRow2);
            if (pPat1 != pPat2)
            {
                SetPattern(nCol, nRow1, *pPat2, TRUE);
                SetPattern(nCol, nRow2, *pPat1, TRUE);
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
}

short ScTable::Compare(SCCOLROW nIndex1, SCCOLROW nIndex2)
{
    short nRes;
    USHORT nSort = 0;
    if (aSortParam.bByRow)
    {
        do
        {
            SCCOL nCol = static_cast<SCCOL>(aSortParam.nField[nSort]);
            ScBaseCell* pCell1 = aCol[nCol].GetCell( nIndex1 );
            ScBaseCell* pCell2 = aCol[nCol].GetCell( nIndex2 );
            nRes = CompareCell( nSort, pCell1, nCol, nIndex1, pCell2, nCol, nIndex2 );
        } while ( nRes == 0 && ++nSort < nMaxSorts && aSortParam.bDoSort[nSort] );
    }
    else
    {
        do
        {
            SCROW nRow = aSortParam.nField[nSort];
            ScBaseCell* pCell1 = aCol[nIndex1].GetCell( nRow );
            ScBaseCell* pCell2 = aCol[nIndex2].GetCell( nRow );
            nRes = CompareCell( nSort, pCell1, static_cast<SCCOL>(nIndex1),
                    nRow, pCell2, static_cast<SCCOL>(nIndex2), nRow );
        } while ( nRes == 0 && ++nSort < nMaxSorts && aSortParam.bDoSort[nSort] );
    }
    return nRes;
}

BOOL ScTable::IsSorted( SCCOLROW nStart, SCCOLROW nEnd )    // ueber aSortParam
{
    for (SCCOLROW i=nStart; i<nEnd; i++)
    {
        if (Compare( i, i+1 ) > 0)
            return FALSE;
    }
    return TRUE;
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

void ScTable::Sort(const ScSortParam& rSortParam, BOOL bKeepQuery)
{
    aSortParam = rSortParam;
    InitSortCollator( rSortParam );
    bGlobalKeepQuery = bKeepQuery;
    if (rSortParam.bByRow)
    {
        SCROW nLastRow = 0;
        for (SCCOL nCol = aSortParam.nCol1; nCol <= aSortParam.nCol2; nCol++)
            nLastRow = Max(nLastRow, aCol[nCol].GetLastDataPos());
        nLastRow = Min(nLastRow, aSortParam.nRow2);
        SCROW nRow1 = (rSortParam.bHasHeader ?
            aSortParam.nRow1 + 1 : aSortParam.nRow1);
        if (!IsSorted(nRow1, nLastRow))
        {
            ScProgress aProgress( pDocument->GetDocumentShell(),
                                    ScGlobal::GetRscString(STR_PROGRESS_SORTING), nLastRow - nRow1 );
            ScSortInfoArray* pArray = CreateSortInfoArray( nRow1, nLastRow );
            if ( nLastRow - nRow1 > 255 )
                DecoladeRow( pArray, nRow1, nLastRow );
            QuickSort( pArray, nRow1, nLastRow );
            SortReorder( pArray, aProgress );
            delete pArray;
            // #158377# #i59745# update position of caption objects of cell notes
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
            ScProgress aProgress( pDocument->GetDocumentShell(),
                                    ScGlobal::GetRscString(STR_PROGRESS_SORTING), nLastCol - nCol1 );
            ScSortInfoArray* pArray = CreateSortInfoArray( nCol1, nLastCol );
            QuickSort( pArray, nCol1, nLastCol );
            SortReorder( pArray, aProgress );
            delete pArray;
            // #158377# #i59745# update position of caption objects of cell notes
            ScNoteUtil::UpdateCaptionPositions( *pDocument, ScRange( nCol1, aSortParam.nRow1, nTab, nLastCol, aSortParam.nRow2, nTab ) );
        }
    }
    DestroySortCollator();
}


//      Testen, ob beim Loeschen von Zwischenergebnissen andere Daten mit geloescht werden
//      (fuer Hinweis-Box)

BOOL ScTable::TestRemoveSubTotals( const ScSubTotalParam& rParam )
{
    SCCOL nStartCol = rParam.nCol1;
    SCROW nStartRow = rParam.nRow1 + 1;     // Header
    SCCOL nEndCol   = rParam.nCol2;
    SCROW nEndRow    = rParam.nRow2;

    SCCOL nCol;
    SCROW nRow;
    ScBaseCell* pCell;

    BOOL bWillDelete = FALSE;
    for ( nCol=nStartCol; nCol<=nEndCol && !bWillDelete; nCol++ )
    {
        ScColumnIterator aIter( &aCol[nCol],nStartRow,nEndRow );
        while ( aIter.Next( nRow, pCell ) && !bWillDelete )
        {
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                if (((ScFormulaCell*)pCell)->IsSubTotal())
                {
                    for (SCCOL nTestCol=0; nTestCol<=MAXCOL; nTestCol++)
                        if (nTestCol<nStartCol || nTestCol>nEndCol)
                            if (aCol[nTestCol].HasDataAt(nRow))
                                bWillDelete = TRUE;
                }
        }
    }
    return bWillDelete;
}

//      alte Ergebnisse loeschen
//      rParam.nRow2 wird veraendert !

void ScTable::RemoveSubTotals( ScSubTotalParam& rParam )
{
    SCCOL nStartCol = rParam.nCol1;
    SCROW nStartRow = rParam.nRow1 + 1;     // Header
    SCCOL nEndCol   = rParam.nCol2;
    SCROW nEndRow    = rParam.nRow2;            // wird veraendert

    SCCOL nCol;
    SCROW nRow;
    ScBaseCell* pCell;

    for ( nCol=nStartCol; nCol<=nEndCol; nCol++ )
    {
        ScColumnIterator aIter( &aCol[nCol],nStartRow,nEndRow );
        while ( aIter.Next( nRow, pCell ) )
        {
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                if (((ScFormulaCell*)pCell)->IsSubTotal())
                {
                    RemoveRowBreak(nRow+1, false, true);
                    pDocument->DeleteRow( 0,nTab, MAXCOL,nTab, nRow, 1 );
                    --nEndRow;
                    aIter = ScColumnIterator( &aCol[nCol],nRow,nEndRow );
                }
        }
    }

    rParam.nRow2 = nEndRow;                 // neues Ende
}

//  harte Zahlenformate loeschen (fuer Ergebnisformeln)

void lcl_RemoveNumberFormat( ScTable* pTab, SCCOL nCol, SCROW nRow )
{
    const ScPatternAttr* pPattern = pTab->GetPattern( nCol, nRow );
    if ( pPattern->GetItemSet().GetItemState( ATTR_VALUE_FORMAT, FALSE )
            == SFX_ITEM_SET )
    {
        ScPatternAttr aNewPattern( *pPattern );
        SfxItemSet& rSet = aNewPattern.GetItemSet();
        rSet.ClearItem( ATTR_VALUE_FORMAT );
        rSet.ClearItem( ATTR_LANGUAGE_FORMAT );
        pTab->SetPattern( nCol, nRow, aNewPattern, TRUE );
    }
}


// at least MSC needs this at linkage level to be able to use it in a template
typedef struct lcl_ScTable_DoSubTotals_RowEntry
{
    USHORT  nGroupNo;
    SCROW   nSubStartRow;
    SCROW   nDestRow;
    SCROW   nFuncStart;
    SCROW   nFuncEnd;
} RowEntry;

//      neue Zwischenergebnisse
//      rParam.nRow2 wird veraendert !

BOOL ScTable::DoSubTotals( ScSubTotalParam& rParam )
{
    SCCOL nStartCol = rParam.nCol1;
    SCROW nStartRow = rParam.nRow1 + 1;     // Header
    SCCOL nEndCol   = rParam.nCol2;
    SCROW nEndRow    = rParam.nRow2;            // wird veraendert
    USHORT i;

    //  Leerzeilen am Ende weglassen,
    //  damit alle Ueberlaeufe (MAXROW) bei InsertRow gefunden werden (#35180#)
    //  Wenn sortiert wurde, sind alle Leerzeilen am Ende.
    SCSIZE nEmpty = GetEmptyLinesInBlock( nStartCol, nStartRow, nEndCol, nEndRow, DIR_BOTTOM );
    nEndRow -= nEmpty;

    USHORT nLevelCount = 0;             // Anzahl Gruppierungen
    BOOL bDoThis = TRUE;
    for (i=0; i<MAXSUBTOTAL && bDoThis; i++)
        if (rParam.bGroupActive[i])
            nLevelCount = i+1;
        else
            bDoThis = FALSE;

    if (nLevelCount==0)                 // nichts tun
        return TRUE;

    SCCOL*          nGroupCol = rParam.nField;  // Spalten nach denen
                                                // gruppiert wird

    //  #44444# Durch (leer) als eigene Kategorie muss immer auf
    //  Teilergebniszeilen aus den anderen Spalten getestet werden
    //  (frueher nur, wenn eine Spalte mehrfach vorkam)
    BOOL bTestPrevSub = ( nLevelCount > 1 );

    String  aSubString;
    String  aOutString;

    BOOL bIgnoreCase = !rParam.bCaseSens;

    String *pCompString[MAXSUBTOTAL];               // Pointer wegen Compiler-Problemen
    for (i=0; i<MAXSUBTOTAL; i++)
        pCompString[i] = new String;

                                //! sortieren?

    ScStyleSheet* pStyle = (ScStyleSheet*) pDocument->GetStyleSheetPool()->Find(
                                ScGlobal::GetRscString(STR_STYLENAME_RESULT), SFX_STYLE_FAMILY_PARA );

    BOOL bSpaceLeft = TRUE;                                         // Erfolg beim Einfuegen?

    // #90279# For performance reasons collect formula entries so their
    // references don't have to be tested for updates each time a new row is
    // inserted
    RowEntry aRowEntry;
    ::std::vector< RowEntry > aRowVector;

    for (USHORT nLevel=0; nLevel<=nLevelCount && bSpaceLeft; nLevel++)      // incl. Gesamtergebnis
    {
        BOOL bTotal = ( nLevel == nLevelCount );
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
                    *pCompString[i] = ScGlobal::pCharClass->upper( aSubString );
                else
                    *pCompString[i] = aSubString;
            }                                                   // aSubString bleibt auf dem letzten stehen

            BOOL bBlockVis = FALSE;             // Gruppe eingeblendet?
            aRowEntry.nSubStartRow = nStartRow;
            for (SCROW nRow=nStartRow; nRow<=nEndRow+1 && bSpaceLeft; nRow++)
            {
                BOOL bChanged;
                if (nRow>nEndRow)
                    bChanged = TRUE;
                else
                {
                    bChanged = FALSE;
                    if (!bTotal)
                    {
                        String aString;
                        for (i=0; i<=aRowEntry.nGroupNo && !bChanged; i++)
                        {
                            GetString( nGroupCol[i], nRow, aString );
                            if (bIgnoreCase)
                                ScGlobal::pCharClass->toUpper( aString );
                            //  #41427# wenn sortiert, ist "leer" eine eigene Gruppe
                            //  sonst sind leere Zellen unten erlaubt
                            bChanged = ( ( aString.Len() || rParam.bDoSort ) &&
                                            aString != *pCompString[i] );
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
                                    bChanged = FALSE;
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
                    bBlockVis = FALSE;
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
                            USHORT nStrId = STR_TABLE_ERGEBNIS;
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
                                *pCompString[i] = ScGlobal::pCharClass->upper( aSubString );
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
            ScBaseCell* pCell = new ScFormulaCell( pDocument, ScAddress(
                        nResCols[nResult], iEntry->nDestRow, nTab), &aArr );
            PutCell( nResCols[nResult], iEntry->nDestRow, pCell );

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


BOOL ScTable::ValidQuery(SCROW nRow, const ScQueryParam& rParam,
        BOOL* pSpecial /* =NULL */ , ScBaseCell* pCell /* =NULL */ ,
        BOOL* pbTestEqualCondition /* = NULL */ )
{
    if (!rParam.GetEntry(0).bDoQuery)
        return TRUE;

    //---------------------------------------------------------------

    const SCSIZE nFixedBools = 32;
    BOOL aBool[nFixedBools];
    BOOL aTest[nFixedBools];
    SCSIZE nEntryCount = rParam.GetEntryCount();
    BOOL* pPasst = ( nEntryCount <= nFixedBools ? &aBool[0] : new BOOL[nEntryCount] );
    BOOL* pTest = ( nEntryCount <= nFixedBools ? &aTest[0] : new BOOL[nEntryCount] );

    long    nPos = -1;
    SCSIZE  i    = 0;
    BOOL    bMatchWholeCell = pDocument->GetDocOptions().IsMatchWholeCell();
    CollatorWrapper* pCollator = (rParam.bCaseSens ? ScGlobal::GetCaseCollator() :
        ScGlobal::GetCollator());
    ::utl::TransliterationWrapper* pTransliteration = (rParam.bCaseSens ?
        ScGlobal::GetCaseTransliteration() : ScGlobal::GetpTransliteration());

    while ( (i < nEntryCount) && rParam.GetEntry(i).bDoQuery )
    {
        ScQueryEntry& rEntry = rParam.GetEntry(i);
        // we can only handle one single direct query
        if ( !pCell || i > 0 )
            pCell = GetCell( static_cast<SCCOL>(rEntry.nField), nRow );

        BOOL bOk = FALSE;
        BOOL bTestEqual = FALSE;

        if ( pSpecial && pSpecial[i] )
        {
            if (rEntry.nVal == SC_EMPTYFIELDS)
                bOk = !( aCol[rEntry.nField].HasDataAt( nRow ) );
            else // if (rEntry.nVal == SC_NONEMPTYFIELDS)
                bOk = aCol[rEntry.nField].HasDataAt( nRow );
        }
        else if ( !rEntry.bQueryByString && (pCell ? pCell->HasValueData() :
                    HasValueData( static_cast<SCCOL>(rEntry.nField), nRow)))
        {   // by Value
            double nCellVal;
            if ( pCell )
            {
                switch ( pCell->GetCellType() )
                {
                    case CELLTYPE_VALUE :
                        nCellVal = ((ScValueCell*)pCell)->GetValue();
                    break;
                    case CELLTYPE_FORMULA :
                        nCellVal = ((ScFormulaCell*)pCell)->GetValue();
                    break;
                    default:
                        nCellVal = 0.0;
                }

            }
            else
                nCellVal = GetValue( static_cast<SCCOL>(rEntry.nField), nRow );

            /* NOTE: lcl_PrepareQuery() prepares a filter query such that if a
             * date+time format was queried rEntry.bQueryByDate is not set. In
             * case other queries wanted to use this mechanism they should do
             * the same, in other words only if rEntry.nVal is an integer value
             * rEntry.bQueryByDate should be true and the time fraction be
             * stripped here. */
            if (rEntry.bQueryByDate)
            {
                sal_uInt32 nNumFmt = GetNumberFormat(static_cast<SCCOL>(rEntry.nField), nRow);
                const SvNumberformat* pEntry = pDocument->GetFormatTable()->GetEntry(nNumFmt);
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
                    bOk = ::rtl::math::approxEqual( nCellVal, rEntry.nVal );
                    break;
                case SC_LESS :
                    bOk = (nCellVal < rEntry.nVal) && !::rtl::math::approxEqual( nCellVal, rEntry.nVal );
                    break;
                case SC_GREATER :
                    bOk = (nCellVal > rEntry.nVal) && !::rtl::math::approxEqual( nCellVal, rEntry.nVal );
                    break;
                case SC_LESS_EQUAL :
                    bOk = (nCellVal < rEntry.nVal) || ::rtl::math::approxEqual( nCellVal, rEntry.nVal );
                    if ( bOk && pbTestEqualCondition )
                        bTestEqual = ::rtl::math::approxEqual( nCellVal, rEntry.nVal );
                    break;
                case SC_GREATER_EQUAL :
                    bOk = (nCellVal > rEntry.nVal) || ::rtl::math::approxEqual( nCellVal, rEntry.nVal );
                    if ( bOk && pbTestEqualCondition )
                        bTestEqual = ::rtl::math::approxEqual( nCellVal, rEntry.nVal );
                    break;
                case SC_NOT_EQUAL :
                    bOk = !::rtl::math::approxEqual( nCellVal, rEntry.nVal );
                    break;
                default:
                {
                    // added to avoid warnings
                }
            }
        }
        else if ( (rEntry.eOp == SC_EQUAL || rEntry.eOp == SC_NOT_EQUAL) ||
                  (rEntry.eOp == SC_CONTAINS || rEntry.eOp == SC_DOES_NOT_CONTAIN ||
                   rEntry.eOp == SC_BEGINS_WITH || rEntry.eOp == SC_ENDS_WITH ||
                   rEntry.eOp == SC_DOES_NOT_BEGIN_WITH || rEntry.eOp == SC_DOES_NOT_END_WITH) ||
                (rEntry.bQueryByString && (pCell ? pCell->HasStringData() :
                                           HasStringData(
                                               static_cast<SCCOL>(rEntry.nField),
                                               nRow))))
        {   // by String
            String  aCellStr;
            if( rEntry.eOp == SC_CONTAINS || rEntry.eOp == SC_DOES_NOT_CONTAIN
                || rEntry.eOp == SC_BEGINS_WITH || rEntry.eOp == SC_ENDS_WITH
                || rEntry.eOp == SC_DOES_NOT_BEGIN_WITH || rEntry.eOp == SC_DOES_NOT_END_WITH )
                bMatchWholeCell = FALSE;
            if ( pCell )
            {
                if (pCell->GetCellType() != CELLTYPE_NOTE)
                {
                    ULONG nFormat = GetNumberFormat( static_cast<SCCOL>(rEntry.nField), nRow );
                    ScCellFormat::GetInputString( pCell, nFormat, aCellStr, *(pDocument->GetFormatTable()) );
                }
            }
            else
                GetInputString( static_cast<SCCOL>(rEntry.nField), nRow, aCellStr );

            BOOL bRealRegExp = (rParam.bRegExp && ((rEntry.eOp == SC_EQUAL)
                || (rEntry.eOp == SC_NOT_EQUAL) || (rEntry.eOp == SC_CONTAINS)
                || (rEntry.eOp == SC_DOES_NOT_CONTAIN) || (rEntry.eOp == SC_BEGINS_WITH)
                || (rEntry.eOp == SC_ENDS_WITH) || (rEntry.eOp == SC_DOES_NOT_BEGIN_WITH)
                || (rEntry.eOp == SC_DOES_NOT_END_WITH)));
            BOOL bTestRegExp = (pbTestEqualCondition && rParam.bRegExp
                && ((rEntry.eOp == SC_LESS_EQUAL)
                    || (rEntry.eOp == SC_GREATER_EQUAL)));
            if ( bRealRegExp || bTestRegExp )
            {
                xub_StrLen nStart = 0;
                xub_StrLen nEnd   = aCellStr.Len();

                // from 614 on, nEnd is behind the found text
                BOOL bMatch = FALSE;
                if ( rEntry.eOp == SC_ENDS_WITH || rEntry.eOp == SC_DOES_NOT_END_WITH )
                {
                    nEnd = 0;
                    nStart = aCellStr.Len();
                    bMatch = (BOOL) rEntry.GetSearchTextPtr( rParam.bCaseSens )
                        ->SearchBkwrd( aCellStr, &nStart, &nEnd );
                }
                else
                {
                    bMatch = (BOOL) rEntry.GetSearchTextPtr( rParam.bCaseSens )
                        ->SearchFrwrd( aCellStr, &nStart, &nEnd );
                }
                if ( bMatch && bMatchWholeCell
                        && (nStart != 0 || nEnd != aCellStr.Len()) )
                    bMatch = FALSE;    // RegExp must match entire cell string
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
                        bOk = ( bMatch && (nEnd == aCellStr.Len()) );
                        break;
                    case SC_DOES_NOT_END_WITH:
                        bOk = !( bMatch && (nEnd == aCellStr.Len()) );
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
                if ( rEntry.eOp == SC_EQUAL || rEntry.eOp == SC_NOT_EQUAL
                    || rEntry.eOp == SC_CONTAINS || rEntry.eOp == SC_DOES_NOT_CONTAIN
                    || rEntry.eOp == SC_BEGINS_WITH || rEntry.eOp == SC_ENDS_WITH
                    || rEntry.eOp == SC_DOES_NOT_BEGIN_WITH || rEntry.eOp == SC_DOES_NOT_END_WITH )
                {
                    if ( !rEntry.bQueryByString && rEntry.pStr->Len() == 0 )
                    {
                        // #i18374# When used from functions (match, countif, sumif, vlookup, hlookup, lookup),
                        // the query value is assigned directly, and the string is empty. In that case,
                        // don't find any string (isEqual would find empty string results in formula cells).
                        bOk = FALSE;
                        if ( rEntry.eOp == SC_NOT_EQUAL )
                            bOk = !bOk;
                    }
                    else if ( bMatchWholeCell )
                    {
                        bOk = pTransliteration->isEqual( aCellStr, *rEntry.pStr );
                        if ( rEntry.eOp == SC_NOT_EQUAL )
                            bOk = !bOk;
                    }
                    else
                    {
                        String aCell( pTransliteration->transliterate(
                            aCellStr, ScGlobal::eLnge, 0, aCellStr.Len(),
                            NULL ) );
                        String aQuer( pTransliteration->transliterate(
                            *rEntry.pStr, ScGlobal::eLnge, 0, rEntry.pStr->Len(),
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
                    sal_Int32 nCompare = pCollator->compareString(
                        aCellStr, *rEntry.pStr );
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
                            if ( bOk && pbTestEqualCondition && !bTestEqual )
                                bTestEqual = (nCompare == 0);
                            break;
                        case SC_GREATER_EQUAL :
                            bOk = (nCompare >= 0);
                            if ( bOk && pbTestEqualCondition && !bTestEqual )
                                bTestEqual = (nCompare == 0);
                            break;
                        default:
                        {
                            // added to avoid warnings
                        }
                    }
                }
            }
        }
        else if (rParam.bMixedComparison)
        {
            if (rEntry.bQueryByString &&
                    (rEntry.eOp == SC_LESS || rEntry.eOp == SC_LESS_EQUAL) &&
                    (pCell ? pCell->HasValueData() :
                     HasValueData( static_cast<SCCOL>(rEntry.nField), nRow)))
            {
                bOk = TRUE;
            }
            else if (!rEntry.bQueryByString &&
                    (rEntry.eOp == SC_GREATER || rEntry.eOp == SC_GREATER_EQUAL) &&
                    (pCell ? pCell->HasStringData() :
                     HasStringData( static_cast<SCCOL>(rEntry.nField), nRow)))
            {
                bOk = TRUE;
            }
        }

        if (nPos == -1)
        {
            nPos++;
            pPasst[nPos] = bOk;
            pTest[nPos] = bTestEqual;
        }
        else
        {
            if (rEntry.eConnect == SC_AND)
            {
                pPasst[nPos] = pPasst[nPos] && bOk;
                pTest[nPos] = pTest[nPos] && bTestEqual;
            }
            else
            {
                nPos++;
                pPasst[nPos] = bOk;
                pTest[nPos] = bTestEqual;
            }
        }
        i++;
    }

    for ( long j=1; j <= nPos; j++ )
    {
        pPasst[0] = pPasst[0] || pPasst[j];
        pTest[0] = pTest[0] || pTest[j];
    }

    BOOL bRet = pPasst[0];
    if ( pPasst != &aBool[0] )
        delete [] pPasst;
    if ( pbTestEqualCondition )
        *pbTestEqualCondition = pTest[0];
    if ( pTest != &aTest[0] )
        delete [] pTest;

    return bRet;
}

void ScTable::TopTenQuery( ScQueryParam& rParam )
{
    BOOL bSortCollatorInitialized = FALSE;
    SCSIZE nEntryCount = rParam.GetEntryCount();
    SCROW nRow1 = (rParam.bHasHeader ? rParam.nRow1 + 1 : rParam.nRow1);
    SCSIZE nCount = static_cast<SCSIZE>(rParam.nRow2 - nRow1 + 1);
    for ( SCSIZE i=0; (i<nEntryCount) && (rParam.GetEntry(i).bDoQuery); i++ )
    {
        ScQueryEntry& rEntry = rParam.GetEntry(i);
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
                    bSortCollatorInitialized = TRUE;
                    InitSortCollator( aLocalSortParam );
                }
                ScSortInfoArray* pArray = CreateSortInfoArray( nRow1, rParam.nRow2 );
                DecoladeRow( pArray, nRow1, rParam.nRow2 );
                QuickSort( pArray, nRow1, rParam.nRow2 );
                ScSortInfo** ppInfo = pArray->GetFirstArray();
                SCSIZE nValidCount = nCount;
                // keine Note-/Leerzellen zaehlen, sind ans Ende sortiert
                while ( nValidCount > 0 && ( ppInfo[nValidCount-1]->pCell == NULL ||
                                             ppInfo[nValidCount-1]->pCell->GetCellType() == CELLTYPE_NOTE ) )
                    nValidCount--;
                // keine Strings zaehlen, sind zwischen Value und Leer
                while ( nValidCount > 0
                  && ppInfo[nValidCount-1]->pCell->HasStringData() )
                    nValidCount--;
                if ( nValidCount > 0 )
                {
                    if ( rEntry.bQueryByString )
                    {   // dat wird nix
                        rEntry.bQueryByString = FALSE;
                        rEntry.nVal = 10;   // 10 bzw. 10%
                    }
                    SCSIZE nVal = (rEntry.nVal >= 1 ? static_cast<SCSIZE>(rEntry.nVal) : 1);
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
                    ScBaseCell* pCell = ppInfo[nOffset]->pCell;
                    if ( pCell->HasValueData() )
                    {
                        if ( pCell->GetCellType() == CELLTYPE_VALUE )
                            rEntry.nVal = ((ScValueCell*)pCell)->GetValue();
                        else
                            rEntry.nVal = ((ScFormulaCell*)pCell)->GetValue();
                    }
                    else
                    {
                        DBG_ERRORFILE( "TopTenQuery: pCell kein ValueData" );
                        rEntry.eOp = SC_GREATER_EQUAL;
                        rEntry.nVal = 0;
                    }
                }
                else
                {
                    rEntry.eOp = SC_GREATER_EQUAL;
                    rEntry.bQueryByString = FALSE;
                    rEntry.nVal = 0;
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

static void lcl_PrepareQuery( ScDocument* pDoc, ScTable* pTab, ScQueryParam& rParam, BOOL* pSpecial )
{
    bool bTopTen = false;
    SCSIZE nEntryCount = rParam.GetEntryCount();

    for ( SCSIZE i = 0; i < nEntryCount; ++i )
    {
        pSpecial[i] = FALSE;
        ScQueryEntry& rEntry = rParam.GetEntry(i);
        if ( rEntry.bDoQuery )
        {
            if ( rEntry.bQueryByString )
            {
                sal_uInt32 nIndex = 0;
                rEntry.bQueryByString = !( pDoc->GetFormatTable()->
                    IsNumberFormat( *rEntry.pStr, nIndex, rEntry.nVal ) );
                if (rEntry.bQueryByDate)
                {
                    if (!rEntry.bQueryByString && ((nIndex % SV_COUNTRY_LANGUAGE_OFFSET) != 0))
                    {
                        const SvNumberformat* pEntry = pDoc->GetFormatTable()->GetEntry(nIndex);
                        if (pEntry)
                        {
                            short nNumFmtType = pEntry->GetType();
                            if (!((nNumFmtType & NUMBERFORMAT_DATE) && !(nNumFmtType & NUMBERFORMAT_TIME)))
                                rEntry.bQueryByDate = false;    // not a date only
                        }
                        else
                            rEntry.bQueryByDate = false;    // what the ... not a date
                    }
                    else
                        rEntry.bQueryByDate = false;    // not a date
                }
            }
            else
            {
                // #58736# call from UNO or second call from autofilter
                if ( rEntry.nVal == SC_EMPTYFIELDS || rEntry.nVal == SC_NONEMPTYFIELDS )
                {
                    pSpecial[i] = TRUE;
                }
            }
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
    }

    if ( bTopTen )
    {
        pTab->TopTenQuery( rParam );
    }
}

SCSIZE ScTable::Query(ScQueryParam& rParamOrg, BOOL bKeepSub)
{
    ScQueryParam    aParam( rParamOrg );
    ScStrCollection aScStrCollection;
    StrData*        pStrData = NULL;

    BOOL    bStarted = FALSE;
    BOOL    bOldResult = TRUE;
    SCROW   nOldStart = 0;
    SCROW   nOldEnd = 0;

    SCSIZE nCount   = 0;
    SCROW nOutRow   = 0;
    SCROW nHeader   = aParam.bHasHeader ? 1 : 0;

    SCSIZE nEntryCount = aParam.GetEntryCount();
    BOOL* pSpecial = new BOOL[nEntryCount];
    lcl_PrepareQuery( pDocument, this, aParam, pSpecial );

    if (!aParam.bInplace)
    {
        nOutRow = aParam.nDestRow + nHeader;
        if (nHeader > 0)
            CopyData( aParam.nCol1, aParam.nRow1, aParam.nCol2, aParam.nRow1,
                            aParam.nDestCol, aParam.nDestRow, aParam.nDestTab );
    }

    InitializeNoteCaptions();

    if (aParam.bInplace)
        IncRecalcLevel();       // #i116164# once for all entries

    SCROW nRealRow2 = aParam.bUseDynamicRange ? aParam.nDynamicEndRow : aParam.nRow2;
    for (SCROW j = aParam.nRow1 + nHeader; j <= nRealRow2; ++j)
    {
        BOOL bResult;                                   // Filterergebnis
        BOOL bValid = ValidQuery(j, aParam, pSpecial);
        if (!bValid && bKeepSub)                        // Subtotals stehenlassen
        {
            for (SCCOL nCol=aParam.nCol1; nCol<=aParam.nCol2 && !bValid; nCol++)
            {
                ScBaseCell* pCell;
                pCell = GetCell( nCol, j );
                if ( pCell )
                    if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                        if (((ScFormulaCell*)pCell)->IsSubTotal())
                            if (RefVisible((ScFormulaCell*)pCell))
                                bValid = TRUE;
            }
        }
        if (bValid)
        {
            if (aParam.bDuplicate)
                bResult = TRUE;
            else
            {
                String aStr;
                for (SCCOL k=aParam.nCol1; k <= aParam.nCol2; k++)
                {
                    String aCellStr;
                    GetString(k, j, aCellStr);
                    aStr += aCellStr;
                    aStr += (sal_Unicode)1;
                }
                pStrData = new StrData(aStr);

                BOOL bIsUnique = TRUE;
                if (pStrData)
                    bIsUnique = aScStrCollection.Insert(pStrData);
                if (bIsUnique)
                    bResult = TRUE;
                else
                {
                    delete pStrData;
                    bResult = FALSE;
                }
            }
        }
        else
            bResult = FALSE;

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
            bStarted = TRUE;
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

    delete[] pSpecial;
    SetDrawPageSize();
    return nCount;
}

BOOL ScTable::CreateExcelQuery(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ScQueryParam& rQueryParam)
{
    BOOL    bValid = TRUE;
    SCCOL* pFields = new SCCOL[nCol2-nCol1+1];
    String  aCellStr;
    SCCOL   nCol = nCol1;
    DBG_ASSERT( rQueryParam.nTab != SCTAB_MAX, "rQueryParam.nTab no value, not bad but no good" );
    SCTAB   nDBTab = (rQueryParam.nTab == SCTAB_MAX ? nTab : rQueryParam.nTab);
    SCROW   nDBRow1 = rQueryParam.nRow1;
    SCCOL   nDBCol2 = rQueryParam.nCol2;
    // Erste Zeile muessen Spaltenkoepfe sein
    while (bValid && (nCol <= nCol2))
    {
        String aQueryStr;
        GetUpperCellString(nCol, nRow1, aQueryStr);
        BOOL bFound = FALSE;
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
            bValid = FALSE;
        nCol++;
    }
    if (bValid)
    {
        ULONG nVisible = 0;
        for ( nCol=nCol1; nCol<=nCol2; nCol++ )
            nVisible += aCol[nCol].VisibleCount( nRow1+1, nRow2 );

        if ( nVisible > SCSIZE_MAX / sizeof(void*) )
        {
            DBG_ERROR("zu viele Filterkritierien");
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
                ScGlobal::pCharClass->toUpper( aCellStr );
                if (aCellStr.Len() > 0)
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
                        bValid = FALSE;
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

BOOL ScTable::CreateStarQuery(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ScQueryParam& rQueryParam)
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
        return FALSE;

    BOOL bValid;
    BOOL bFound;
    String aCellStr;
    SCSIZE nIndex = 0;
    SCROW nRow = nRow1;
    DBG_ASSERT( rQueryParam.nTab != SCTAB_MAX, "rQueryParam.nTab no value, not bad but no good" );
    SCTAB   nDBTab = (rQueryParam.nTab == SCTAB_MAX ? nTab : rQueryParam.nTab);
    SCROW   nDBRow1 = rQueryParam.nRow1;
    SCCOL   nDBCol2 = rQueryParam.nCol2;

    SCSIZE nNewEntries = static_cast<SCSIZE>(nRow2-nRow1+1);
    rQueryParam.Resize( nNewEntries );

    do
    {
        ScQueryEntry& rEntry = rQueryParam.GetEntry(nIndex);

        bValid = FALSE;
        // Erste Spalte UND/ODER
        if (nIndex > 0)
        {
            GetUpperCellString(nCol1, nRow, aCellStr);
            if ( aCellStr == ScGlobal::GetRscString(STR_TABLE_UND) )
            {
                rEntry.eConnect = SC_AND;
                bValid = TRUE;
            }
            else if ( aCellStr == ScGlobal::GetRscString(STR_TABLE_ODER) )
            {
                rEntry.eConnect = SC_OR;
                bValid = TRUE;
            }
        }
        // Zweite Spalte FeldName
        if ((nIndex < 1) || bValid)
        {
            bFound = FALSE;
            GetUpperCellString(nCol1 + 1, nRow, aCellStr);
            for (SCCOL i=rQueryParam.nCol1; (i <= nDBCol2) && (!bFound); i++)
            {
                String aFieldStr;
                if ( nTab == nDBTab )
                    GetUpperCellString(i, nDBRow1, aFieldStr);
                else
                    pDocument->GetUpperCellString(i, nDBRow1, nDBTab, aFieldStr);
                bFound = (aCellStr == aFieldStr);
                if (bFound)
                {
                    rEntry.nField = i;
                    bValid = TRUE;
                }
                else
                    bValid = FALSE;
            }
        }
        // Dritte Spalte Operator =<>...
        if (bValid)
        {
            bFound = FALSE;
            GetUpperCellString(nCol1 + 2, nRow, aCellStr);
            if (aCellStr.GetChar(0) == '<')
            {
                if (aCellStr.GetChar(1) == '>')
                    rEntry.eOp = SC_NOT_EQUAL;
                else if (aCellStr.GetChar(1) == '=')
                    rEntry.eOp = SC_LESS_EQUAL;
                else
                    rEntry.eOp = SC_LESS;
            }
            else if (aCellStr.GetChar(0) == '>')
            {
                if (aCellStr.GetChar(1) == '=')
                    rEntry.eOp = SC_GREATER_EQUAL;
                else
                    rEntry.eOp = SC_GREATER;
            }
            else if (aCellStr.GetChar(0) == '=')
                rEntry.eOp = SC_EQUAL;

        }
        // Vierte Spalte Wert
        if (bValid)
        {
            GetString(nCol1 + 3, nRow, *rEntry.pStr);
            rEntry.bDoQuery = TRUE;
        }
        nIndex++;
        nRow++;
    }
    while (bValid && (nRow <= nRow2) /* && (nIndex < MAXQUERY) */ );
    return bValid;
}

BOOL ScTable::CreateQueryParam(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ScQueryParam& rQueryParam)
{
    SCSIZE i, nCount;
    PutInOrder(nCol1, nCol2);
    PutInOrder(nRow1, nRow2);

    nCount = rQueryParam.GetEntryCount();
    for (i=0; i < nCount; i++)
        rQueryParam.GetEntry(i).Clear();

    // Standard QueryTabelle
    BOOL bValid = CreateStarQuery(nCol1, nRow1, nCol2, nRow2, rQueryParam);
    // Excel QueryTabelle
    if (!bValid)
        bValid = CreateExcelQuery(nCol1, nRow1, nCol2, nRow2, rQueryParam);

    nCount = rQueryParam.GetEntryCount();
    if (bValid)
    {
        //  bQueryByString muss gesetzt sein
        for (i=0; i < nCount; i++)
            rQueryParam.GetEntry(i).bQueryByString = TRUE;
    }
    else
    {
        //  nix
        for (i=0; i < nCount; i++)
            rQueryParam.GetEntry(i).Clear();
    }
    return bValid;
}

BOOL ScTable::HasColHeader( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW /* nEndRow */ )
{
    for (SCCOL nCol=nStartCol; nCol<=nEndCol; nCol++)
    {
        CellType eType = GetCellType( nCol, nStartRow );
        if (eType != CELLTYPE_STRING && eType != CELLTYPE_EDIT)
            return FALSE;
    }
    return TRUE;
}

BOOL ScTable::HasRowHeader( SCCOL nStartCol, SCROW nStartRow, SCCOL /* nEndCol */, SCROW nEndRow )
{
    for (SCROW nRow=nStartRow; nRow<=nEndRow; nRow++)
    {
        CellType eType = GetCellType( nStartCol, nRow );
        if (eType != CELLTYPE_STRING && eType != CELLTYPE_EDIT)
            return FALSE;
    }
    return TRUE;
}

void ScTable::GetFilterEntries(SCCOL nCol, SCROW nRow1, SCROW nRow2, TypedScStrCollection& rStrings, bool& rHasDates)
{
    aCol[nCol].GetFilterEntries( nRow1, nRow2, rStrings, rHasDates );
}

void ScTable::GetFilteredFilterEntries(
    SCCOL nCol, SCROW nRow1, SCROW nRow2, const ScQueryParam& rParam, TypedScStrCollection& rStrings, bool& rHasDates )
{
    // remove the entry for this column from the query parameter
    ScQueryParam aParam( rParam );
    SCSIZE nEntryCount = aParam.GetEntryCount();
    for ( SCSIZE i = 0; i < nEntryCount && aParam.GetEntry(i).bDoQuery; ++i )
    {
        ScQueryEntry& rEntry = aParam.GetEntry(i);
        if ( rEntry.nField == nCol )
        {
            aParam.DeleteQuery(i);
            break;
        }
    }
    nEntryCount = aParam.GetEntryCount();

    BOOL* pSpecial = new BOOL[nEntryCount];
    lcl_PrepareQuery( pDocument, this, aParam, pSpecial );
    bool bHasDates = false;
    for ( SCROW j = nRow1; j <= nRow2; ++j )
    {
        if ( ValidQuery( j, aParam, pSpecial ) )
        {
            bool bThisHasDates = false;
            aCol[nCol].GetFilterEntries( j, j, rStrings, bThisHasDates );
            bHasDates |= bThisHasDates;
        }
    }

    rHasDates = bHasDates;
    delete[] pSpecial;
}

BOOL ScTable::GetDataEntries(SCCOL nCol, SCROW nRow, TypedScStrCollection& rStrings, BOOL bLimit)
{
    return aCol[nCol].GetDataEntries( nRow, rStrings, bLimit );
}

SCSIZE ScTable::GetCellCount(SCCOL nCol) const
{
    return aCol[nCol].GetCellCount();
}

ULONG ScTable::GetCellCount() const
{
    ULONG nCellCount = 0;

    for ( SCCOL nCol=0; nCol<=MAXCOL; nCol++ )
        nCellCount += aCol[nCol].GetCellCount();

    return nCellCount;
}

ULONG ScTable::GetWeightedCount() const
{
    ULONG nCellCount = 0;

    for ( SCCOL nCol=0; nCol<=MAXCOL; nCol++ )
        if ( aCol[nCol].GetCellCount() )                    // GetCellCount ist inline
            nCellCount += aCol[nCol].GetWeightedCount();

    return nCellCount;
}

ULONG ScTable::GetCodeCount() const
{
    ULONG nCodeCount = 0;

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
    BOOL bSingle = ( rMark.IsMarked() || !rMark.IsMultiMarked() );

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

void ScTable::FindConditionalFormat( ULONG nKey, ScRangeList& rList )
{
    SCROW nStartRow = 0, nEndRow = 0;
    for (SCCOL nCol=0; nCol<=MAXCOL; nCol++)
    {
        ScAttrIterator* pIter = aCol[nCol].CreateAttrIterator( 0, MAXROW );
        const ScPatternAttr* pPattern = pIter->Next( nStartRow, nEndRow );
        while (pPattern)
        {
            if (((SfxUInt32Item&)pPattern->GetItem(ATTR_CONDITIONAL)).GetValue() == nKey)
                rList.Join( ScRange(nCol,nStartRow,nTab, nCol,nEndRow,nTab) );
            pPattern = pIter->Next( nStartRow, nEndRow );
        }
        delete pIter;
    }
}

void ScTable::IncRecalcLevel()
{
    ++nRecalcLvl;
}

void ScTable::DecRecalcLevel(bool bUpdateNoteCaptionPos)
{
     if (!--nRecalcLvl)
         SetDrawPageSize(true, bUpdateNoteCaptionPos);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
