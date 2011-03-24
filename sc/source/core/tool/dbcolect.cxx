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


#include <tools/debug.hxx>
#include <unotools/transliterationwrapper.hxx>

#include "dbcolect.hxx"
#include "global.hxx"
#include "refupdat.hxx"
#include "rechead.hxx"
#include "document.hxx"
#include "queryparam.hxx"
#include "globstr.hrc"
#include "subtotalparam.hxx"

//---------------------------------------------------------------------------------------

ScDBData::ScDBData( const String& rName,
                    SCTAB nTab,
                    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                    sal_Bool bByR, sal_Bool bHasH) :
    aName       (rName),
    nTable      (nTab),
    nStartCol   (nCol1),
    nStartRow   (nRow1),
    nEndCol     (nCol2),
    nEndRow     (nRow2),
    bByRow      (bByR),
    bHasHeader  (bHasH),
    bDoSize     (false),
    bKeepFmt    (false),
    bStripData  (false),
    bIsAdvanced (false),
    bDBSelection(false),
    nIndex      (0),
    bAutoFilter (false),
    bModified   (false)
{
    sal_uInt16 i;

    ScSortParam aSortParam;
    ScQueryParam aQueryParam;
    ScSubTotalParam aSubTotalParam;
    ScImportParam aImportParam;

    for (i=0; i<MAXQUERY; i++)
        pQueryStr[i] = new String;

    for (i=0; i<MAXSUBTOTAL; i++)
    {
        nSubTotals[i] = 0;
        pSubTotals[i] = NULL;
        pFunctions[i] = NULL;
    }

    SetSortParam( aSortParam );
    SetQueryParam( aQueryParam );
    SetSubTotalParam( aSubTotalParam );
    SetImportParam( aImportParam );
}

ScDBData::ScDBData( const ScDBData& rData ) :
    ScDataObject(),
    ScRefreshTimer      ( rData ),
    aName               (rData.aName),
    nTable              (rData.nTable),
    nStartCol           (rData.nStartCol),
    nStartRow           (rData.nStartRow),
    nEndCol             (rData.nEndCol),
    nEndRow             (rData.nEndRow),
    bByRow              (rData.bByRow),
    bHasHeader          (rData.bHasHeader),
    bDoSize             (rData.bDoSize),
    bKeepFmt            (rData.bKeepFmt),
    bStripData          (rData.bStripData),
    bSortCaseSens       (rData.bSortCaseSens),
    bSortNaturalSort    (rData.bSortNaturalSort),
    bIncludePattern     (rData.bIncludePattern),
    bSortInplace        (rData.bSortInplace),
    bSortUserDef        (rData.bSortUserDef),
    nSortUserIndex      (rData.nSortUserIndex),
    nSortDestTab        (rData.nSortDestTab),
    nSortDestCol        (rData.nSortDestCol),
    nSortDestRow        (rData.nSortDestRow),
    aSortLocale         (rData.aSortLocale),
    aSortAlgorithm      (rData.aSortAlgorithm),
    bQueryInplace       (rData.bQueryInplace),
    bQueryCaseSens      (rData.bQueryCaseSens),
    bQueryRegExp        (rData.bQueryRegExp),
    bQueryDuplicate     (rData.bQueryDuplicate),
    nQueryDestTab       (rData.nQueryDestTab),
    nQueryDestCol       (rData.nQueryDestCol),
    nQueryDestRow       (rData.nQueryDestRow),
    bIsAdvanced         (rData.bIsAdvanced),
    aAdvSource          (rData.aAdvSource),
    bSubRemoveOnly      (rData.bSubRemoveOnly),
    bSubReplace         (rData.bSubReplace),
    bSubPagebreak       (rData.bSubPagebreak),
    bSubCaseSens        (rData.bSubCaseSens),
    bSubDoSort          (rData.bSubDoSort),
    bSubAscending       (rData.bSubAscending),
    bSubIncludePattern  (rData.bSubIncludePattern),
    bSubUserDef         (rData.bSubUserDef),
    nSubUserIndex       (rData.nSubUserIndex),
    bDBImport           (rData.bDBImport),
    aDBName             (rData.aDBName),
    aDBStatement        (rData.aDBStatement),
    bDBNative           (rData.bDBNative),
    bDBSelection        (rData.bDBSelection),
    bDBSql              (rData.bDBSql),
    nDBType             (rData.nDBType),
    nIndex              (rData.nIndex),
    bAutoFilter         (rData.bAutoFilter),
    bModified           (rData.bModified)
{
    sal_uInt16 i;
    sal_uInt16 j;

    for (i=0; i<MAXSORT; i++)
    {
        bDoSort[i]      = rData.bDoSort[i];
        nSortField[i]   = rData.nSortField[i];
        bAscending[i]   = rData.bAscending[i];
    }
    for (i=0; i<MAXQUERY; i++)
    {
        bDoQuery[i]         = rData.bDoQuery[i];
        nQueryField[i]      = rData.nQueryField[i];
        eQueryOp[i]         = rData.eQueryOp[i];
        bQueryByString[i]   = rData.bQueryByString[i];
        bQueryByDate[i]     = rData.bQueryByDate[i];
        pQueryStr[i]        = new String( *(rData.pQueryStr[i]) );
        nQueryVal[i]        = rData.nQueryVal[i];
        eQueryConnect[i]    = rData.eQueryConnect[i];
    }
    for (i=0; i<MAXSUBTOTAL; i++)
    {
        bDoSubTotal[i]      = rData.bDoSubTotal[i];
        nSubField[i]        = rData.nSubField[i];

        SCCOL nCount    = rData.nSubTotals[i];
        nSubTotals[i]   = nCount;
        pFunctions[i]   = nCount > 0 ? new ScSubTotalFunc [nCount] : NULL;
        pSubTotals[i]   = nCount > 0 ? new SCCOL          [nCount] : NULL;

        for (j=0; j<nCount; j++)
        {
            pSubTotals[i][j] = rData.pSubTotals[i][j];
            pFunctions[i][j] = rData.pFunctions[i][j];
        }
    }
}

ScDBData& ScDBData::operator= (const ScDBData& rData)
{
    sal_uInt16 i;
    sal_uInt16 j;

    ScRefreshTimer::operator=( rData );
    aName               = rData.aName;
    nTable              = rData.nTable;
    nStartCol           = rData.nStartCol;
    nStartRow           = rData.nStartRow;
    nEndCol             = rData.nEndCol;
    nEndRow             = rData.nEndRow;
    bByRow              = rData.bByRow;
    bHasHeader          = rData.bHasHeader;
    bDoSize             = rData.bDoSize;
    bKeepFmt            = rData.bKeepFmt;
    bStripData          = rData.bStripData;
    bSortCaseSens       = rData.bSortCaseSens;
    bSortNaturalSort    = rData.bSortNaturalSort;
    bIncludePattern     = rData.bIncludePattern;
    bSortInplace        = rData.bSortInplace;
    nSortDestTab        = rData.nSortDestTab;
    nSortDestCol        = rData.nSortDestCol;
    nSortDestRow        = rData.nSortDestRow;
    bSortUserDef        = rData.bSortUserDef;
    nSortUserIndex      = rData.nSortUserIndex;
    aSortLocale         = rData.aSortLocale;
    aSortAlgorithm      = rData.aSortAlgorithm;
    bQueryInplace       = rData.bQueryInplace;
    bQueryCaseSens      = rData.bQueryCaseSens;
    bQueryRegExp        = rData.bQueryRegExp;
    bQueryDuplicate     = rData.bQueryDuplicate;
    nQueryDestTab       = rData.nQueryDestTab;
    nQueryDestCol       = rData.nQueryDestCol;
    nQueryDestRow       = rData.nQueryDestRow;
    bIsAdvanced         = rData.bIsAdvanced;
    aAdvSource          = rData.aAdvSource;
    bSubRemoveOnly      = rData.bSubRemoveOnly;
    bSubReplace         = rData.bSubReplace;
    bSubPagebreak       = rData.bSubPagebreak;
    bSubCaseSens        = rData.bSubCaseSens;
    bSubDoSort          = rData.bSubDoSort;
    bSubAscending       = rData.bSubAscending;
    bSubIncludePattern  = rData.bSubIncludePattern;
    bSubUserDef         = rData.bSubUserDef;
    nSubUserIndex       = rData.nSubUserIndex;
    bDBImport           = rData.bDBImport;
    aDBName             = rData.aDBName;
    aDBStatement        = rData.aDBStatement;
    bDBNative           = rData.bDBNative;
    bDBSelection        = rData.bDBSelection;
    bDBSql              = rData.bDBSql;
    nDBType             = rData.nDBType;
    nIndex              = rData.nIndex;
    bAutoFilter         = rData.bAutoFilter;

    for (i=0; i<MAXSORT; i++)
    {
        bDoSort[i]      = rData.bDoSort[i];
        nSortField[i]   = rData.nSortField[i];
        bAscending[i]   = rData.bAscending[i];
    }
    for (i=0; i<MAXQUERY; i++)
    {
        bDoQuery[i]         = rData.bDoQuery[i];
        nQueryField[i]      = rData.nQueryField[i];
        eQueryOp[i]         = rData.eQueryOp[i];
        bQueryByString[i]   = rData.bQueryByString[i];
        bQueryByDate[i]     = rData.bQueryByDate[i];
        *pQueryStr[i]       = *rData.pQueryStr[i];
        nQueryVal[i]        = rData.nQueryVal[i];
        eQueryConnect[i]    = rData.eQueryConnect[i];
    }
    for (i=0; i<MAXSUBTOTAL; i++)
    {
        bDoSubTotal[i]      = rData.bDoSubTotal[i];
        nSubField[i]        = rData.nSubField[i];
        SCCOL nCount    = rData.nSubTotals[i];
        nSubTotals[i]   = nCount;

        delete[] pSubTotals[i];
        delete[] pFunctions[i];

        pSubTotals[i] = nCount > 0 ? new SCCOL          [nCount] : NULL;
        pFunctions[i] = nCount > 0 ? new ScSubTotalFunc [nCount] : NULL;
        for (j=0; j<nCount; j++)
        {
            pSubTotals[i][j] = rData.pSubTotals[i][j];
            pFunctions[i][j] = rData.pFunctions[i][j];
        }
    }

    return *this;
}

sal_Bool ScDBData::operator== (const ScDBData& rData) const
{
    //  Daten, die nicht in den Params sind

    if ( nTable     != rData.nTable     ||
         bDoSize    != rData.bDoSize    ||
         bKeepFmt   != rData.bKeepFmt   ||
         bIsAdvanced!= rData.bIsAdvanced||
         bStripData != rData.bStripData ||
//       SAB: I think this should be here, but I don't want to break something
//         bAutoFilter!= rData.bAutoFilter||
         ScRefreshTimer::operator!=( rData )
        )
        return false;

    if ( bIsAdvanced && aAdvSource != rData.aAdvSource )
        return false;

    ScSortParam aSort1, aSort2;
    GetSortParam(aSort1);
    rData.GetSortParam(aSort2);
    if (!(aSort1 == aSort2))
        return false;

    ScQueryParam aQuery1, aQuery2;
    GetQueryParam(aQuery1);
    rData.GetQueryParam(aQuery2);
    if (!(aQuery1 == aQuery2))
        return false;

    ScSubTotalParam aSubTotal1, aSubTotal2;
    GetSubTotalParam(aSubTotal1);
    rData.GetSubTotalParam(aSubTotal2);
    if (!(aSubTotal1 == aSubTotal2))
        return false;

    ScImportParam aImport1, aImport2;
    GetImportParam(aImport1);
    rData.GetImportParam(aImport2);
    if (!(aImport1 == aImport2))
        return false;

    return sal_True;
}

ScDBData::~ScDBData()
{
    StopRefreshTimer();
    sal_uInt16 i;

    for (i=0; i<MAXQUERY; i++)
        delete pQueryStr[i];
    for (i=0; i<MAXSUBTOTAL; i++)
    {
        delete[] pSubTotals[i];
        delete[] pFunctions[i];
    }
}


String ScDBData::GetSourceString() const
{
    String aVal;
    if (bDBImport)
    {
        aVal = aDBName;
        aVal += '/';
        aVal += aDBStatement;
    }
    return aVal;
}

String ScDBData::GetOperations() const
{
    String aVal;
    if (bDoQuery[0])
        aVal = ScGlobal::GetRscString(STR_OPERATION_FILTER);

    if (bDoSort[0])
    {
        if (aVal.Len())
            aVal.AppendAscii( RTL_CONSTASCII_STRINGPARAM(", ") );
        aVal += ScGlobal::GetRscString(STR_OPERATION_SORT);
    }

    if (bDoSubTotal[0] && !bSubRemoveOnly)
    {
        if (aVal.Len())
            aVal.AppendAscii( RTL_CONSTASCII_STRINGPARAM(", ") );
        aVal += ScGlobal::GetRscString(STR_OPERATION_SUBTOTAL);
    }

    if (!aVal.Len())
        aVal = ScGlobal::GetRscString(STR_OPERATION_NONE);

    return aVal;
}

void ScDBData::GetArea(SCTAB& rTab, SCCOL& rCol1, SCROW& rRow1, SCCOL& rCol2, SCROW& rRow2,
                       bool bUseDynamicRange) const
{
    rTab  = nTable;
    rCol1 = nStartCol;
    rRow1 = nStartRow;
    rCol2 = nEndCol;
    rRow2 = bUseDynamicRange ? nDynamicEndRow : nEndRow;
}

void ScDBData::GetArea(ScRange& rRange, bool bUseDynamicRange) const
{
    SCROW nNewEndRow = bUseDynamicRange ? nDynamicEndRow : nEndRow;
    rRange = ScRange( nStartCol, nStartRow, nTable, nEndCol, nNewEndRow, nTable );
}

void ScDBData::SetArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2)
{
    nTable  = nTab;
    nStartCol = nCol1;
    nStartRow = nRow1;
    nEndCol   = nCol2;
    nEndRow   = nRow2;
}

void ScDBData::SetDynamicEndRow(SCROW nRow)
{
    nDynamicEndRow = nRow;
}

void ScDBData::MoveTo(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2)
{
    sal_uInt16 i;
    long nDifX = ((long) nCol1) - ((long) nStartCol);
    long nDifY = ((long) nRow1) - ((long) nStartRow);

    long nSortDif = bByRow ? nDifX : nDifY;
    long nSortEnd = bByRow ? static_cast<long>(nCol2) : static_cast<long>(nRow2);

    for (i=0; i<MAXSORT; i++)
    {
        nSortField[i] += nSortDif;
        if (nSortField[i] > nSortEnd)
        {
            nSortField[i] = 0;
            bDoSort[i]    = false;
        }
    }
    for (i=0; i<MAXQUERY; i++)
    {
        nQueryField[i] += nDifX;
        if (nQueryField[i] > nCol2)
        {
            nQueryField[i] = 0;
            bDoQuery[i]    = false;
        }
    }
    for (i=0; i<MAXSUBTOTAL; i++)
    {
        nSubField[i] = sal::static_int_cast<SCCOL>( nSubField[i] + nDifX );
        if (nSubField[i] > nCol2)
        {
            nSubField[i]   = 0;
            bDoSubTotal[i] = false;
        }
    }

    SetArea( nTab, nCol1, nRow1, nCol2, nRow2 );
}

void ScDBData::GetSortParam( ScSortParam& rSortParam ) const
{
    rSortParam.nCol1 = nStartCol;
    rSortParam.nRow1 = nStartRow;
    rSortParam.nCol2 = nEndCol;
    rSortParam.nRow2 = nEndRow;
    rSortParam.bByRow = bByRow;
    rSortParam.bHasHeader = bHasHeader;
    rSortParam.bCaseSens = bSortCaseSens;
    rSortParam.bNaturalSort = bSortNaturalSort;
    rSortParam.bInplace = bSortInplace;
    rSortParam.nDestTab = nSortDestTab;
    rSortParam.nDestCol = nSortDestCol;
    rSortParam.nDestRow = nSortDestRow;
    rSortParam.bIncludePattern = bIncludePattern;
    rSortParam.bUserDef = bSortUserDef;
    rSortParam.nUserIndex = nSortUserIndex;
    for (sal_uInt16 i=0; i<MAXSORT; i++)
    {
        rSortParam.bDoSort[i]    = bDoSort[i];
        rSortParam.nField[i]     = nSortField[i];
        rSortParam.bAscending[i] = bAscending[i];
    }
    rSortParam.aCollatorLocale = aSortLocale;
    rSortParam.aCollatorAlgorithm = aSortAlgorithm;
}

void ScDBData::SetSortParam( const ScSortParam& rSortParam )
{
    bSortCaseSens = rSortParam.bCaseSens;
    bSortNaturalSort = rSortParam.bNaturalSort;
    bIncludePattern = rSortParam.bIncludePattern;
    bSortInplace = rSortParam.bInplace;
    nSortDestTab = rSortParam.nDestTab;
    nSortDestCol = rSortParam.nDestCol;
    nSortDestRow = rSortParam.nDestRow;
    bSortUserDef = rSortParam.bUserDef;
    nSortUserIndex = rSortParam.nUserIndex;
    for (sal_uInt16 i=0; i<MAXSORT; i++)
    {
        bDoSort[i]    = rSortParam.bDoSort[i];
        nSortField[i] = rSortParam.nField[i];
        bAscending[i] = rSortParam.bAscending[i];
    }
    aSortLocale = rSortParam.aCollatorLocale;
    aSortAlgorithm = rSortParam.aCollatorAlgorithm;

    //#98317#; set the orientation
    bByRow = rSortParam.bByRow;
}

void ScDBData::GetQueryParam( ScQueryParam& rQueryParam ) const
{
    rQueryParam.nCol1 = nStartCol;
    rQueryParam.nRow1 = nStartRow;
    rQueryParam.nCol2 = nEndCol;
    rQueryParam.nRow2 = nEndRow;
    rQueryParam.nTab  = nTable;
    rQueryParam.bByRow = bByRow;
    rQueryParam.bHasHeader = bHasHeader;
    rQueryParam.bInplace = bQueryInplace;
    rQueryParam.bCaseSens = bQueryCaseSens;
    rQueryParam.bRegExp = bQueryRegExp;
    rQueryParam.bDuplicate = bQueryDuplicate;
    rQueryParam.nDestTab = nQueryDestTab;
    rQueryParam.nDestCol = nQueryDestCol;
    rQueryParam.nDestRow = nQueryDestRow;
    rQueryParam.nDynamicEndRow = nDynamicEndRow;

    rQueryParam.Resize( MAXQUERY );
    for (SCSIZE i=0; i<MAXQUERY; i++)
    {
        ScQueryEntry& rEntry = rQueryParam.GetEntry(i);

        rEntry.bDoQuery = bDoQuery[i];
        rEntry.nField = nQueryField[i];
        rEntry.eOp = eQueryOp[i];
        rEntry.bQueryByString = bQueryByString[i];
        rEntry.bQueryByDate = bQueryByDate[i];
        *rEntry.pStr = *pQueryStr[i];
        rEntry.nVal = nQueryVal[i];
        rEntry.eConnect = eQueryConnect[i];
    }
}

void ScDBData::SetQueryParam(const ScQueryParam& rQueryParam)
{
    DBG_ASSERT( rQueryParam.GetEntryCount() <= MAXQUERY ||
                !rQueryParam.GetEntry(MAXQUERY).bDoQuery,
                "zuviele Eintraege bei ScDBData::SetQueryParam" );

    //  set bIsAdvanced to sal_False for everything that is not from the
    //  advanced filter dialog
    bIsAdvanced = false;

    bQueryInplace = rQueryParam.bInplace;
    bQueryCaseSens = rQueryParam.bCaseSens;
    bQueryRegExp = rQueryParam.bRegExp;
    bQueryDuplicate = rQueryParam.bDuplicate;
    nQueryDestTab = rQueryParam.nDestTab;
    nQueryDestCol = rQueryParam.nDestCol;
    nQueryDestRow = rQueryParam.nDestRow;
    for (SCSIZE i=0; i<MAXQUERY; i++)
    {
        ScQueryEntry& rEntry = rQueryParam.GetEntry(i);

        bDoQuery[i] = rEntry.bDoQuery;
        nQueryField[i] = rEntry.nField;
        eQueryOp[i] = rEntry.eOp;
        bQueryByString[i] = rEntry.bQueryByString;
        bQueryByDate[i] = rEntry.bQueryByDate;
        *pQueryStr[i] = *rEntry.pStr;
        nQueryVal[i] = rEntry.nVal;
        eQueryConnect[i] = rEntry.eConnect;
    }
}

void ScDBData::SetAdvancedQuerySource(const ScRange* pSource)
{
    if (pSource)
    {
        aAdvSource = *pSource;
        bIsAdvanced = sal_True;
    }
    else
        bIsAdvanced = false;
}

sal_Bool ScDBData::GetAdvancedQuerySource(ScRange& rSource) const
{
    rSource = aAdvSource;
    return bIsAdvanced;
}

void ScDBData::GetSubTotalParam(ScSubTotalParam& rSubTotalParam) const
{
    sal_uInt16 i;
    sal_uInt16 j;

    rSubTotalParam.nCol1 = nStartCol;
    rSubTotalParam.nRow1 = nStartRow;
    rSubTotalParam.nCol2 = nEndCol;
    rSubTotalParam.nRow2 = nEndRow;

    rSubTotalParam.bRemoveOnly      = bSubRemoveOnly;
    rSubTotalParam.bReplace         = bSubReplace;
    rSubTotalParam.bPagebreak       = bSubPagebreak;
    rSubTotalParam.bCaseSens        = bSubCaseSens;
    rSubTotalParam.bDoSort          = bSubDoSort;
    rSubTotalParam.bAscending       = bSubAscending;
    rSubTotalParam.bIncludePattern  = bSubIncludePattern;
    rSubTotalParam.bUserDef         = bSubUserDef;
    rSubTotalParam.nUserIndex       = nSubUserIndex;

    for (i=0; i<MAXSUBTOTAL; i++)
    {
        rSubTotalParam.bGroupActive[i]  = bDoSubTotal[i];
        rSubTotalParam.nField[i]        = nSubField[i];
        SCCOL nCount = nSubTotals[i];

        rSubTotalParam.nSubTotals[i] = nCount;
        delete[] rSubTotalParam.pSubTotals[i];
        delete[] rSubTotalParam.pFunctions[i];
        rSubTotalParam.pSubTotals[i] = nCount > 0 ? new SCCOL[nCount] : NULL;
        rSubTotalParam.pFunctions[i] = nCount > 0 ? new ScSubTotalFunc[nCount]
                                              : NULL;
        for (j=0; j<nCount; j++)
        {
            rSubTotalParam.pSubTotals[i][j] = pSubTotals[i][j];
            rSubTotalParam.pFunctions[i][j] = pFunctions[i][j];
        }
    }
}

void ScDBData::SetSubTotalParam(const ScSubTotalParam& rSubTotalParam)
{
    sal_uInt16 i;
    sal_uInt16 j;

    bSubRemoveOnly      = rSubTotalParam.bRemoveOnly;
    bSubReplace         = rSubTotalParam.bReplace;
    bSubPagebreak       = rSubTotalParam.bPagebreak;
    bSubCaseSens        = rSubTotalParam.bCaseSens;
    bSubDoSort          = rSubTotalParam.bDoSort;
    bSubAscending       = rSubTotalParam.bAscending;
    bSubIncludePattern  = rSubTotalParam.bIncludePattern;
    bSubUserDef         = rSubTotalParam.bUserDef;
    nSubUserIndex       = rSubTotalParam.nUserIndex;

    for (i=0; i<MAXSUBTOTAL; i++)
    {
        bDoSubTotal[i]  = rSubTotalParam.bGroupActive[i];
        nSubField[i]    = rSubTotalParam.nField[i];
        SCCOL nCount = rSubTotalParam.nSubTotals[i];

        nSubTotals[i] = nCount;
        delete[] pSubTotals[i];
        delete[] pFunctions[i];
        pSubTotals[i] = nCount > 0 ? new SCCOL          [nCount] : NULL;
        pFunctions[i] = nCount > 0 ? new ScSubTotalFunc [nCount] : NULL;
        for (j=0; j<nCount; j++)
        {
            pSubTotals[i][j] = rSubTotalParam.pSubTotals[i][j];
            pFunctions[i][j] = rSubTotalParam.pFunctions[i][j];
        }
    }
}

void ScDBData::GetImportParam(ScImportParam& rImportParam) const
{
    rImportParam.nCol1 = nStartCol;
    rImportParam.nRow1 = nStartRow;
    rImportParam.nCol2 = nEndCol;
    rImportParam.nRow2 = nEndRow;

    rImportParam.bImport    = bDBImport;
    rImportParam.aDBName    = aDBName;
    rImportParam.aStatement = aDBStatement;
    rImportParam.bNative    = bDBNative;
    rImportParam.bSql       = bDBSql;
    rImportParam.nType      = nDBType;
}

void ScDBData::SetImportParam(const ScImportParam& rImportParam)
{
    bDBImport       = rImportParam.bImport;
    aDBName         = rImportParam.aDBName;
    aDBStatement    = rImportParam.aStatement;
    bDBNative       = rImportParam.bNative;
    bDBSql          = rImportParam.bSql;
    nDBType         = rImportParam.nType;
}

sal_Bool ScDBData::IsDBAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, sal_Bool bStartOnly) const
{
    if (nTab == nTable)
    {
        if ( bStartOnly )
            return ( nCol == nStartCol && nRow == nStartRow );
        else
            return ( nCol >= nStartCol && nCol <= nEndCol &&
                     nRow >= nStartRow && nRow <= nEndRow );
    }

    return false;
}

sal_Bool ScDBData::IsDBAtArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2) const
{
    return (sal_Bool)((nTab == nTable)
                    && (nCol1 == nStartCol) && (nRow1 == nStartRow)
                    && (nCol2 == nEndCol) && (nRow2 == nEndRow));
}

ScDataObject*   ScDBData::Clone() const
{
    return new ScDBData(*this);
}


//---------------------------------------------------------------------------------------
//  Compare zum Sortieren

short ScDBCollection::Compare(ScDataObject* pKey1, ScDataObject* pKey2) const
{
    const String& rStr1 = ((ScDBData*)pKey1)->GetName();
    const String& rStr2 = ((ScDBData*)pKey2)->GetName();
    return (short) ScGlobal::GetpTransliteration()->compareString( rStr1, rStr2 );
}

//  IsEqual - alles gleich

sal_Bool ScDBCollection::IsEqual(ScDataObject* pKey1, ScDataObject* pKey2) const
{
    return *(ScDBData*)pKey1 == *(ScDBData*)pKey2;
}

ScDBData* ScDBCollection::GetDBAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, sal_Bool bStartOnly) const
{
    ScDBData* pNoNameData = NULL;
    if (pItems)
    {
        const String& rNoName = ScGlobal::GetRscString( STR_DB_NONAME );

        for (sal_uInt16 i = 0; i < nCount; i++)
            if (((ScDBData*)pItems[i])->IsDBAtCursor(nCol, nRow, nTab, bStartOnly))
            {
                ScDBData* pDB = (ScDBData*)pItems[i];
                if ( pDB->GetName() == rNoName )
                    pNoNameData = pDB;
                else
                    return pDB;
            }
    }
    return pNoNameData;             // "unbenannt" nur zurueck, wenn sonst nichts gefunden
}

ScDBData* ScDBCollection::GetDBAtArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2) const
{
    ScDBData* pNoNameData = NULL;
    if (pItems)
    {
        const String& rNoName = ScGlobal::GetRscString( STR_DB_NONAME );

        for (sal_uInt16 i = 0; i < nCount; i++)
            if (((ScDBData*)pItems[i])->IsDBAtArea(nTab, nCol1, nRow1, nCol2, nRow2))
            {
                ScDBData* pDB = (ScDBData*)pItems[i];
                if ( pDB->GetName() == rNoName )
                    pNoNameData = pDB;
                else
                    return pDB;
            }
    }
    return pNoNameData;             // "unbenannt" nur zurueck, wenn sonst nichts gefunden
}

sal_Bool ScDBCollection::SearchName( const String& rName, sal_uInt16& rIndex ) const
{
    ScDBData aDataObj( rName, 0,0,0,0,0 );
    return Search( &aDataObj, rIndex );
}

void ScDBCollection::DeleteOnTab( SCTAB nTab )
{
    sal_uInt16 nPos = 0;
    while ( nPos < nCount )
    {
        // look for output positions on the deleted sheet

        SCCOL nEntryCol1, nEntryCol2;
        SCROW nEntryRow1, nEntryRow2;
        SCTAB nEntryTab;
        static_cast<const ScDBData*>(At(nPos))->GetArea( nEntryTab, nEntryCol1, nEntryRow1, nEntryCol2, nEntryRow2 );
        if ( nEntryTab == nTab )
            AtFree(nPos);
        else
            ++nPos;
    }
}

void ScDBCollection::UpdateReference(UpdateRefMode eUpdateRefMode,
                                SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
{
    for (sal_uInt16 i=0; i<nCount; i++)
    {
        SCCOL theCol1;
        SCROW theRow1;
        SCTAB theTab1;
        SCCOL theCol2;
        SCROW theRow2;
        SCTAB theTab2;
        ((ScDBData*)pItems[i])->GetArea( theTab1, theCol1, theRow1, theCol2, theRow2 );
        theTab2 = theTab1;

        sal_Bool bDoUpdate = ScRefUpdate::Update( pDoc, eUpdateRefMode,
                                                nCol1,nRow1,nTab1, nCol2,nRow2,nTab2, nDx,nDy,nDz,
                                                theCol1,theRow1,theTab1, theCol2,theRow2,theTab2 ) != UR_NOTHING;
        if (bDoUpdate)
            ((ScDBData*)pItems[i])->MoveTo( theTab1, theCol1, theRow1, theCol2, theRow2 );

        ScRange aAdvSource;
        if ( ((ScDBData*)pItems[i])->GetAdvancedQuerySource(aAdvSource) )
        {
            aAdvSource.GetVars( theCol1,theRow1,theTab1, theCol2,theRow2,theTab2 );
            if ( ScRefUpdate::Update( pDoc, eUpdateRefMode,
                                        nCol1,nRow1,nTab1, nCol2,nRow2,nTab2, nDx,nDy,nDz,
                                        theCol1,theRow1,theTab1, theCol2,theRow2,theTab2 ) )
            {
                aAdvSource.aStart.Set( theCol1,theRow1,theTab1 );
                aAdvSource.aEnd.Set( theCol2,theRow2,theTab2 );
                ((ScDBData*)pItems[i])->SetAdvancedQuerySource( &aAdvSource );

                bDoUpdate = sal_True;       // DBData is modified
            }
        }

        ((ScDBData*)pItems[i])->SetModified(bDoUpdate);

        //!     Testen, ob mitten aus dem Bereich geloescht/eingefuegt wurde !!!
    }
}


void ScDBCollection::UpdateMoveTab( SCTAB nOldPos, SCTAB nNewPos )
{
    //  wenn nOldPos vor nNewPos liegt, ist nNewPos schon angepasst

    for (sal_uInt16 i=0; i<nCount; i++)
    {
        ScRange aRange;
        ScDBData* pData = (ScDBData*)pItems[i];
        pData->GetArea( aRange );
        SCTAB nTab = aRange.aStart.Tab();               // hat nur eine Tabelle

        //  anpassen wie die aktuelle Tabelle bei ScTablesHint (tabvwsh5.cxx)

        if ( nTab == nOldPos )                          // verschobene Tabelle
            nTab = nNewPos;
        else if ( nOldPos < nNewPos )                   // nach hinten verschoben
        {
            if ( nTab > nOldPos && nTab <= nNewPos )    // nachrueckender Bereich
                --nTab;
        }
        else                                            // nach vorne verschoben
        {
            if ( nTab >= nNewPos && nTab < nOldPos )    // nachrueckender Bereich
                ++nTab;
        }

        sal_Bool bChanged = ( nTab != aRange.aStart.Tab() );
        if (bChanged)
            pData->SetArea( nTab, aRange.aStart.Col(), aRange.aStart.Row(),
                                    aRange.aEnd.Col(),aRange.aEnd .Row() );

        //  MoveTo ist nicht noetig, wenn nur die Tabelle geaendert ist

        pData->SetModified(bChanged);
    }
}


ScDBData* ScDBCollection::FindIndex(sal_uInt16 nIndex)
{
    sal_uInt16 i = 0;
    while (i < nCount)
    {
        if ((*this)[i]->GetIndex() == nIndex)
            return (*this)[i];
        i++;
    }
    return NULL;
}

sal_Bool ScDBCollection::Insert(ScDataObject* pScDataObject)
{
    ScDBData* pData = (ScDBData*) pScDataObject;
    if (!pData->GetIndex())     // schon gesetzt?
        pData->SetIndex(nEntryIndex++);
    sal_Bool bInserted = ScSortedCollection::Insert(pScDataObject);
    if ( bInserted && pData->HasImportParam() && !pData->HasImportSelection() )
    {
        pData->SetRefreshHandler( GetRefreshHandler() );
        pData->SetRefreshControl( pDoc->GetRefreshTimerControlAddress() );
    }
    return bInserted;
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
