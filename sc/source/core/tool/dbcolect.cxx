/*************************************************************************
 *
 *  $RCSfile: dbcolect.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-10-26 18:59:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include <tools/debug.hxx>
#include <tools/intn.hxx>

#include "dbcolect.hxx"
#include "global.hxx"
#include "refupdat.hxx"
#include "rechead.hxx"
#include "document.hxx"
#include "globstr.hrc"


//---------------------------------------------------------------------------------------

ScDBData::ScDBData( const String& rName,
                    USHORT nTab,
                    USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                    BOOL bByR, BOOL bHasH) :
    aName       (rName),
    nTable      (nTab),
    nStartCol   (nCol1),
    nStartRow   (nRow1),
    nEndCol     (nCol2),
    nEndRow     (nRow2),
    bByRow      (bByR),
    bDoSize     (FALSE),
    bKeepFmt    (FALSE),
    bStripData  (FALSE),
    bHasHeader  (bHasH),
    bDBSelection(FALSE),
    nIndex      (0),
    nExportIndex(0),
    bIsAdvanced (FALSE),
    bAutoFilter (FALSE),
    bModified   (FALSE)
{
    USHORT i;

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

ScDBData::ScDBData( SvStream& rStream, ScMultipleReadHeader& rHdr ) :
                    // nicht in der Datei:
    bAutoFilter     (FALSE),
    bModified       (FALSE),
    nExportIndex    (0),
                    // nicht in alten Versionen:
    bDoSize         (FALSE),
    bKeepFmt        (FALSE),
    bStripData      (FALSE),
    nIndex          (0),
    bIsAdvanced     (FALSE),
    bDBSelection    (FALSE),
    bDBSql          (TRUE),
    nDBType         (ScDbTable),
    nSubUserIndex   (0),
    bSortUserDef    (FALSE),
    nSortUserIndex  (0)
{
    rHdr.StartEntry();

    USHORT i;
    USHORT j;
    BYTE nDummy;
    rtl_TextEncoding eCharSet = rStream.GetStreamCharSet();

    rStream.ReadByteString( aName, eCharSet );
    rStream >> nTable;
    rStream >> nStartCol;
    rStream >> nStartRow;
    rStream >> nEndCol;
    rStream >> nEndRow;
    rStream >> bByRow;
    rStream >> bHasHeader;
    rStream >> bSortCaseSens;
    rStream >> bIncludePattern;
    rStream >> bSortInplace;
    rStream >> nSortDestTab;
    rStream >> nSortDestCol;
    rStream >> nSortDestRow;
    rStream >> bQueryInplace;
    rStream >> bQueryCaseSens;
    rStream >> bQueryRegExp;
    rStream >> bQueryDuplicate;
    rStream >> nQueryDestTab;
    rStream >> nQueryDestCol;
    rStream >> nQueryDestRow;
    rStream >> bSubRemoveOnly;
    rStream >> bSubReplace;
    rStream >> bSubPagebreak;
    rStream >> bSubCaseSens;
    rStream >> bSubDoSort;
    rStream >> bSubAscending;
    rStream >> bSubIncludePattern;
    rStream >> bSubUserDef;
    rStream >> bDBImport;

    rStream.ReadByteString( aDBName, eCharSet );
    rStream.ReadByteString( aDBStatement, eCharSet );
    rStream >> bDBNative;

    for (i=0; i<MAXSORT; i++)
    {
        rStream >> bDoSort[i];
        rStream >> nSortField[i];
        rStream >> bAscending[i];
    }
    for (i=0; i<MAXQUERY; i++)
    {
        rStream >> bDoQuery[i];
        rStream >> nQueryField[i];
        rStream >> nDummy; eQueryOp[i] = (ScQueryOp) nDummy;
        rStream >> bQueryByString[i];
        pQueryStr[i] = new String;
        rStream.ReadByteString( *pQueryStr[i], eCharSet );
        rStream >> nQueryVal[i];
        rStream >> nDummy; eQueryConnect[i] = (ScQueryConnect) nDummy;
    }
    for (i=0; i<MAXSUBTOTAL; i++)
    {
        rStream >> bDoSubTotal[i];
        rStream >> nSubField[i];

        USHORT nCount;
        rStream >> nCount;
        nSubTotals[i] = nCount;

        pSubTotals[i] = nCount ? new USHORT         [nCount] : NULL;
        pFunctions[i] = nCount ? new ScSubTotalFunc [nCount] : NULL;

        for (j=0; j<nCount; j++)
        {
            rStream >> pSubTotals[i][j];
            rStream >> nDummy; pFunctions[i][j] = (ScSubTotalFunc)nDummy;
        }
    }

    if (rHdr.BytesLeft())
        rStream >> nIndex;

    if (rHdr.BytesLeft())
        rStream >> bDBSelection;

    if (rHdr.BytesLeft())
        rStream >> bDBSql;              // Default = TRUE

    if (rHdr.BytesLeft())
    {
        rStream >> nSubUserIndex;
        rStream >> bSortUserDef;
        rStream >> nSortUserIndex;
    }

    if (rHdr.BytesLeft())
    {
        rStream >> bDoSize;
        rStream >> bKeepFmt;
    }

    if (rHdr.BytesLeft())
        rStream >> bStripData;

    if (rHdr.BytesLeft())
        rStream >> nDBType;             // Default = ScDbTable

    if (rHdr.BytesLeft())
    {
        rStream >> bIsAdvanced;         // Default = FALSE
        if (bIsAdvanced)
            rStream >> aAdvSource;
    }

    rHdr.EndEntry();

    // #43070# rottes Dokument?!?
    // nEndCol war 258
    // und auch die CellInfo pPattern in ScOutputData FindRotated waren NULL
    if ( nStartCol > MAXCOL )
    {
        DBG_ERRORFILE( "nStartCol > MAXCOL" );
        nStartCol = MAXCOL;
    }
    if ( nStartRow > MAXROW )
    {
        DBG_ERRORFILE( "nStartRow > MAXROW" );
        nStartRow = MAXROW;
    }
    if ( nEndCol > MAXCOL )
    {
        DBG_ERRORFILE( "nEndCol > MAXCOL" );
        nEndCol = MAXCOL;
    }
    if ( nEndRow > MAXROW )
    {
        DBG_ERRORFILE( "nEndRow > MAXROW" );
        nEndRow = MAXROW;
    }
    if ( nQueryDestCol > MAXCOL )
    {
        DBG_ERRORFILE( "nQueryDestCol > MAXCOL" );
        nQueryDestCol = MAXCOL;
    }
    if ( nQueryDestRow > MAXROW )
    {
        DBG_ERRORFILE( "nQueryDestRow > MAXROW" );
        nQueryDestRow = MAXROW;
    }
}

BOOL ScDBData::Store( SvStream& rStream, ScMultipleWriteHeader& rHdr ) const
{
    rHdr.StartEntry();

    USHORT i;
    USHORT j;
    rtl_TextEncoding eCharSet = rStream.GetStreamCharSet();

    rStream.WriteByteString( aName, eCharSet );
    rStream << nTable;
    rStream << nStartCol;
    rStream << nStartRow;
    rStream << nEndCol;
    rStream << nEndRow;
    rStream << bByRow;
    rStream << bHasHeader;
    rStream << bSortCaseSens;
    rStream << bIncludePattern;
    rStream << bSortInplace;
    rStream << nSortDestTab;
    rStream << nSortDestCol;
    rStream << nSortDestRow;
    rStream << bQueryInplace;
    rStream << bQueryCaseSens;
    rStream << bQueryRegExp;
    rStream << bQueryDuplicate;
    rStream << nQueryDestTab;
    rStream << nQueryDestCol;
    rStream << nQueryDestRow;
    rStream << bSubRemoveOnly;
    rStream << bSubReplace;
    rStream << bSubPagebreak;
    rStream << bSubCaseSens;
    rStream << bSubDoSort;
    rStream << bSubAscending;
    rStream << bSubIncludePattern;
    rStream << bSubUserDef;
    rStream << bDBImport;

    rStream.WriteByteString( aDBName, eCharSet );
    rStream.WriteByteString( aDBStatement, eCharSet );
    rStream << bDBNative;

    for (i=0; i<MAXSORT; i++)
    {
        rStream << bDoSort[i];
        rStream << nSortField[i];
        rStream << bAscending[i];
    }
    for (i=0; i<MAXQUERY; i++)
    {
        rStream << bDoQuery[i];
        rStream << nQueryField[i];
        rStream << (BYTE) eQueryOp[i];
        rStream << bQueryByString[i];
        rStream.WriteByteString( *pQueryStr[i], eCharSet );
        rStream << nQueryVal[i];
        rStream << (BYTE) eQueryConnect[i];
    }
    for (i=0; i<MAXSUBTOTAL; i++)
    {
        rStream << bDoSubTotal[i];
        rStream << nSubField[i];

        USHORT nCount = nSubTotals[i];
        rStream << nCount;
        for (j=0; j<nCount; j++)
        {
            rStream << pSubTotals[i][j];
            rStream << (BYTE)pFunctions[i][j];
        }
    }
    rStream << nIndex;                  // seit 24.10.95

    rStream << bDBSelection;

    rStream << bDBSql;                  // seit 4.2.97

    rStream << nSubUserIndex;           // seit 5.2.97
    rStream << bSortUserDef;
    rStream << nSortUserIndex;

    rStream << bDoSize;                 // seit 13.2.97
    rStream << bKeepFmt;

    rStream << bStripData;              // seit 23.2.97

    if( rStream.GetVersion() > SOFFICE_FILEFORMAT_40 )
    {
        //  folgendes gab's in der 4.0 noch nicht

        //  alte Versionen suchen immer nach Tables und Queries
        rStream << nDBType;                 // seit 20.11.97

        //  starting from 591, store advanced filter source range
        //  only if set, to avoid unneccessary warnings
        if (bIsAdvanced)
        {
            rStream << (BOOL) TRUE;
            rStream << aAdvSource;
        }
    }

    rHdr.EndEntry();
    return TRUE;
}

ScDBData::ScDBData( const ScDBData& rData ) :
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
    bIncludePattern     (rData.bIncludePattern),
    bSortInplace        (rData.bSortInplace),
    nSortDestTab        (rData.nSortDestTab),
    nSortDestCol        (rData.nSortDestCol),
    nSortDestRow        (rData.nSortDestRow),
    bSortUserDef        (rData.bSortUserDef),
    nSortUserIndex      (rData.nSortUserIndex),
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
    nExportIndex        (rData.nExportIndex),
    bAutoFilter         (rData.bAutoFilter),
    bModified           (rData.bModified)
{
    USHORT i;
    USHORT j;

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
        pQueryStr[i]        = new String( *(rData.pQueryStr[i]) );
        nQueryVal[i]        = rData.nQueryVal[i];
        eQueryConnect[i]    = rData.eQueryConnect[i];
    }
    for (i=0; i<MAXSUBTOTAL; i++)
    {
        bDoSubTotal[i]      = rData.bDoSubTotal[i];
        nSubField[i]        = rData.nSubField[i];

        USHORT nCount   = rData.nSubTotals[i];
        nSubTotals[i]   = nCount;
        pFunctions[i]   = nCount ? new ScSubTotalFunc [nCount] : NULL;
        pSubTotals[i]   = nCount ? new USHORT         [nCount] : NULL;

        for (j=0; j<nCount; j++)
        {
            pSubTotals[i][j] = rData.pSubTotals[i][j];
            pFunctions[i][j] = rData.pFunctions[i][j];
        }
    }
}

ScDBData& ScDBData::operator= (const ScDBData& rData)
{
    USHORT i;
    USHORT j;

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
    bIncludePattern     = rData.bIncludePattern;
    bSortInplace        = rData.bSortInplace;
    nSortDestTab        = rData.nSortDestTab;
    nSortDestCol        = rData.nSortDestCol;
    nSortDestRow        = rData.nSortDestRow;
    bSortUserDef        = rData.bSortUserDef;
    nSortUserIndex      = rData.nSortUserIndex;
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
    nExportIndex        = rData.nExportIndex;

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
        *pQueryStr[i]       = *rData.pQueryStr[i];
        nQueryVal[i]        = rData.nQueryVal[i];
        eQueryConnect[i]    = rData.eQueryConnect[i];
    }
    for (i=0; i<MAXSUBTOTAL; i++)
    {
        bDoSubTotal[i]      = rData.bDoSubTotal[i];
        nSubField[i]        = rData.nSubField[i];
        USHORT nCount   = rData.nSubTotals[i];
        nSubTotals[i]   = nCount;

        delete[] pSubTotals[i];
        delete[] pFunctions[i];

        pSubTotals[i] = nCount ? new USHORT         [nCount] : NULL;
        pFunctions[i] = nCount ? new ScSubTotalFunc [nCount] : NULL;
        for (j=0; j<nCount; j++)
        {
            pSubTotals[i][j] = rData.pSubTotals[i][j];
            pFunctions[i][j] = rData.pFunctions[i][j];
        }
    }

    return *this;
}

BOOL ScDBData::operator== (const ScDBData& rData) const
{
    //  Daten, die nicht in den Params sind

    if ( nTable     != rData.nTable     ||
         bDoSize    != rData.bDoSize    ||
         bKeepFmt   != rData.bKeepFmt   ||
         bIsAdvanced!= rData.bIsAdvanced||
         bStripData != rData.bStripData )
        return FALSE;

    if ( bIsAdvanced && aAdvSource != rData.aAdvSource )
        return FALSE;

    ScSortParam aSort1, aSort2;
    GetSortParam(aSort1);
    rData.GetSortParam(aSort2);
    if (!(aSort1 == aSort2))
        return FALSE;

    ScQueryParam aQuery1, aQuery2;
    GetQueryParam(aQuery1);
    rData.GetQueryParam(aQuery2);
    if (!(aQuery1 == aQuery2))
        return FALSE;

    ScSubTotalParam aSubTotal1, aSubTotal2;
    GetSubTotalParam(aSubTotal1);
    rData.GetSubTotalParam(aSubTotal2);
    if (!(aSubTotal1 == aSubTotal2))
        return FALSE;

    ScImportParam aImport1, aImport2;
    GetImportParam(aImport1);
    rData.GetImportParam(aImport2);
    if (!(aImport1 == aImport2))
        return FALSE;

    return TRUE;
}

ScDBData::~ScDBData()
{
    USHORT i;

    for (i=0; i<MAXQUERY; i++)
        delete pQueryStr[i];
    for (i=0; i<MAXSUBTOTAL; i++)
    {
        delete[] pSubTotals[i];
        delete[] pFunctions[i];
    }
}

BOOL ScDBData::IsBeyond(USHORT nMaxRow) const
{
    return ( nStartRow > nMaxRow ||
             nEndRow > nMaxRow ||
             nQueryDestRow > nMaxRow );
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

void ScDBData::GetArea(USHORT& rTab, USHORT& rCol1, USHORT& rRow1, USHORT& rCol2, USHORT& rRow2) const
{
    rTab  = nTable;
    rCol1 = nStartCol;
    rRow1 = nStartRow;
    rCol2 = nEndCol;
    rRow2 = nEndRow;
}

void ScDBData::GetArea(ScRange& rRange) const
{
    rRange = ScRange( nStartCol,nStartRow,nTable, nEndCol,nEndRow,nTable );
}

void ScDBData::SetArea(USHORT nTab, USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2)
{
    nTable  = nTab;
    nStartCol = nCol1;
    nStartRow = nRow1;
    nEndCol   = nCol2;
    nEndRow   = nRow2;
}

void ScDBData::MoveTo(USHORT nTab, USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2)
{
    USHORT i;
    short nDifX = ((short) nCol1) - ((short) nStartCol);
    short nDifY = ((short) nRow1) - ((short) nStartRow);

    short nSortDif = bByRow ? nDifX : nDifY;
    USHORT nSortEnd = bByRow ? nCol2 : nRow2;

    for (i=0; i<MAXSORT; i++)
    {
        nSortField[i] += nSortDif;
        if (nSortField[i] > nSortEnd)
        {
            nSortField[i] = 0;
            bDoSort[i]    = FALSE;
        }
    }
    for (i=0; i<MAXQUERY; i++)
    {
        nQueryField[i] += nDifX;
        if (nQueryField[i] > nCol2)
        {
            nQueryField[i] = 0;
            bDoQuery[i]    = FALSE;
        }
    }
    for (i=0; i<MAXSUBTOTAL; i++)
    {
        nSubField[i] += nDifX;
        if (nSubField[i] > nCol2)
        {
            nSubField[i]   = 0;
            bDoSubTotal[i] = FALSE;
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
    rSortParam.bInplace = bSortInplace;
    rSortParam.nDestTab = nSortDestTab;
    rSortParam.nDestCol = nSortDestCol;
    rSortParam.nDestRow = nSortDestRow;
    rSortParam.bIncludePattern = bIncludePattern;
    rSortParam.bUserDef = bSortUserDef;
    rSortParam.nUserIndex = nSortUserIndex;
    for (USHORT i=0; i<MAXSORT; i++)
    {
        rSortParam.bDoSort[i]    = bDoSort[i];
        rSortParam.nField[i]     = nSortField[i];
        rSortParam.bAscending[i] = bAscending[i];
    }
}

void ScDBData::SetSortParam( const ScSortParam& rSortParam )
{
    bSortCaseSens = rSortParam.bCaseSens;
    bIncludePattern = rSortParam.bIncludePattern;
    bSortInplace = rSortParam.bInplace;
    nSortDestTab = rSortParam.nDestTab;
    nSortDestCol = rSortParam.nDestCol;
    nSortDestRow = rSortParam.nDestRow;
    bSortUserDef = rSortParam.bUserDef;
    nSortUserIndex = rSortParam.nUserIndex;
    for (USHORT i=0; i<MAXSORT; i++)
    {
        bDoSort[i]    = rSortParam.bDoSort[i];
        nSortField[i] = rSortParam.nField[i];
        bAscending[i] = rSortParam.bAscending[i];
    }
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

    rQueryParam.Resize( MAXQUERY );
    for (USHORT i=0; i<MAXQUERY; i++)
    {
        ScQueryEntry& rEntry = rQueryParam.GetEntry(i);

        rEntry.bDoQuery = bDoQuery[i];
        rEntry.nField = nQueryField[i];
        rEntry.eOp = eQueryOp[i];
        rEntry.bQueryByString = bQueryByString[i];
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

    //  set bIsAdvanced to FALSE for everything that is not from the
    //  advanced filter dialog
    bIsAdvanced = FALSE;

    bQueryInplace = rQueryParam.bInplace;
    bQueryCaseSens = rQueryParam.bCaseSens;
    bQueryRegExp = rQueryParam.bRegExp;
    bQueryDuplicate = rQueryParam.bDuplicate;
    nQueryDestTab = rQueryParam.nDestTab;
    nQueryDestCol = rQueryParam.nDestCol;
    nQueryDestRow = rQueryParam.nDestRow;
    for (USHORT i=0; i<MAXQUERY; i++)
    {
        ScQueryEntry& rEntry = rQueryParam.GetEntry(i);

        bDoQuery[i] = rEntry.bDoQuery;
        nQueryField[i] = rEntry.nField;
        eQueryOp[i] = rEntry.eOp;
        bQueryByString[i] = rEntry.bQueryByString;
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
        bIsAdvanced = TRUE;
    }
    else
        bIsAdvanced = FALSE;
}

BOOL ScDBData::GetAdvancedQuerySource(ScRange& rSource) const
{
    rSource = aAdvSource;
    return bIsAdvanced;
}

void ScDBData::GetSubTotalParam(ScSubTotalParam& rSubTotalParam) const
{
    USHORT i;
    USHORT j;

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
        USHORT nCount = nSubTotals[i];

        rSubTotalParam.nSubTotals[i] = nCount;
        delete[] rSubTotalParam.pSubTotals[i];
        delete[] rSubTotalParam.pFunctions[i];
        rSubTotalParam.pSubTotals[i] = nCount ? new USHORT[nCount] : NULL;
        rSubTotalParam.pFunctions[i] = nCount ? new ScSubTotalFunc[nCount]
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
    USHORT i;
    USHORT j;

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
        USHORT nCount = rSubTotalParam.nSubTotals[i];

        nSubTotals[i] = nCount;
        delete[] pSubTotals[i];
        delete[] pFunctions[i];
        pSubTotals[i] = nCount ? new USHORT         [nCount] : NULL;
        pFunctions[i] = nCount ? new ScSubTotalFunc [nCount] : NULL;
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

BOOL ScDBData::IsDBAtCursor(USHORT nCol, USHORT nRow, USHORT nTab, BOOL bStartOnly) const
{
    if (nTab == nTable)
    {
        if ( bStartOnly )
            return ( nCol == nStartCol && nRow == nStartRow );
        else
            return ( nCol >= nStartCol && nCol <= nEndCol &&
                     nRow >= nStartRow && nRow <= nEndRow );
    }

    return FALSE;
}

BOOL ScDBData::IsDBAtArea(USHORT nTab, USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2) const
{
    return (BOOL)((nTab == nTable)
                    && (nCol1 == nStartCol) && (nRow1 == nStartRow)
                    && (nCol2 == nEndCol) && (nRow2 == nEndRow));
}

String ScDBData::GetTargetName(const String& rDocName) const
{
    String aTargetName(rDocName);
    aTargetName += '.';
    aTargetName += aName;
    return aTargetName;
}

DataObject* ScDBData::Clone() const
{
    return new ScDBData(*this);
}

//---------------------------------------------------------------------------------------
//  Compare zum Sortieren

short ScDBCollection::Compare(DataObject* pKey1, DataObject* pKey2) const
{
    const String& rStr1 = ((ScDBData*)pKey1)->GetName();
    const String& rStr2 = ((ScDBData*)pKey2)->GetName();
    StringCompare eComp = ScGlobal::pScInternational->Compare(
                                rStr1, rStr2, INTN_COMPARE_IGNORECASE );
    if (eComp == COMPARE_EQUAL)
        return 0;
    else if (eComp == COMPARE_LESS)
        return -1;
    else
        return 1;
}

//  IsEqual - alles gleich

BOOL ScDBCollection::IsEqual(DataObject* pKey1, DataObject* pKey2) const
{
    return *(ScDBData*)pKey1 == *(ScDBData*)pKey2;
}

ScDBData* ScDBCollection::GetDBAtCursor(USHORT nCol, USHORT nRow, USHORT nTab, BOOL bStartOnly) const
{
    ScDBData* pNoNameData = NULL;
    if (pItems)
    {
        const String& rNoName = ScGlobal::GetRscString( STR_DB_NONAME );

        for (USHORT i = 0; i < nCount; i++)
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

ScDBData* ScDBCollection::GetDBAtArea(USHORT nTab, USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2) const
{
    ScDBData* pNoNameData = NULL;
    if (pItems)
    {
        const String& rNoName = ScGlobal::GetRscString( STR_DB_NONAME );

        for (USHORT i = 0; i < nCount; i++)
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

BOOL ScDBCollection::SearchName( const String& rName, USHORT& rIndex ) const
{
    ScDBData aDataObj( rName, 0,0,0,0,0 );
    return Search( &aDataObj, rIndex );
}

BOOL ScDBCollection::Load( SvStream& rStream )
{
    BOOL bSuccess = TRUE;
    USHORT nNewCount;

    while( nCount > 0 )
        AtFree(0);                  // alles loeschen

    ScMultipleReadHeader aHdr( rStream );

    rStream >> nNewCount;
    for (USHORT i=0; i<nNewCount && bSuccess; i++)
    {
        ScDBData* pData = new ScDBData( rStream, aHdr );
        Insert( pData );
    }
    if (aHdr.BytesLeft())   //  ... Erweiterungen
        rStream >> nEntryIndex;
    return bSuccess;
}

BOOL ScDBCollection::Store( SvStream& rStream ) const
{
    ScMultipleWriteHeader aHdr( rStream );

    USHORT i;
    USHORT nSaveCount = nCount;
    USHORT nSaveMaxRow = pDoc->GetSrcMaxRow();
    if ( nSaveMaxRow < MAXROW )
    {
        nSaveCount = 0;
        for (i=0; i<nCount; i++)
            if ( !((const ScDBData*)At(i))->IsBeyond(nSaveMaxRow) )
                ++nSaveCount;

        if ( nSaveCount < nCount )
            pDoc->SetLostData();            // Warnung ausgeben
    }

    rStream << nSaveCount;

    BOOL bSuccess = TRUE;
    for (i=0; i<nCount && bSuccess; i++)
    {
        const ScDBData* pDBData = (const ScDBData*)At(i);
        if ( nSaveMaxRow == MAXROW || !pDBData->IsBeyond(nSaveMaxRow) )
            bSuccess = pDBData->Store( rStream, aHdr );
    }

    rStream << nEntryIndex;             // seit 24.10.95

    return bSuccess;
}

void ScDBCollection::UpdateReference(UpdateRefMode eUpdateRefMode,
                                USHORT nCol1, USHORT nRow1, USHORT nTab1,
                                USHORT nCol2, USHORT nRow2, USHORT nTab2,
                                short nDx, short nDy, short nDz )
{
    for (USHORT i=0; i<nCount; i++)
    {
        USHORT theCol1;
        USHORT theRow1;
        USHORT theTab1;
        USHORT theCol2;
        USHORT theRow2;
        USHORT theTab2;
        ((ScDBData*)pItems[i])->GetArea( theTab1, theCol1, theRow1, theCol2, theRow2 );
        theTab2 = theTab1;

        BOOL bDoUpdate = ScRefUpdate::Update( pDoc, eUpdateRefMode,
                                                nCol1,nRow1,nTab1, nCol2,nRow2,nTab2, nDx,nDy,nDz,
                                                theCol1,theRow1,theTab1, theCol2,theRow2,theTab2 );
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

                bDoUpdate = TRUE;       // DBData is modified
            }
        }

        ((ScDBData*)pItems[i])->SetModified(bDoUpdate);

        //!     Testen, ob mitten aus dem Bereich geloescht/eingefuegt wurde !!!
    }
}


void ScDBCollection::UpdateMoveTab( USHORT nOldPos, USHORT nNewPos )
{
    //  wenn nOldPos vor nNewPos liegt, ist nNewPos schon angepasst

    for (USHORT i=0; i<nCount; i++)
    {
        ScRange aRange;
        ScDBData* pData = (ScDBData*)pItems[i];
        pData->GetArea( aRange );
        USHORT nTab = aRange.aStart.Tab();              // hat nur eine Tabelle

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

        BOOL bChanged = ( nTab != aRange.aStart.Tab() );
        if (bChanged)
            pData->SetArea( nTab, aRange.aStart.Col(), aRange.aStart.Row(),
                                    aRange.aEnd.Col(),aRange.aEnd .Row() );

        //  MoveTo ist nicht noetig, wenn nur die Tabelle geaendert ist

        pData->SetModified(bChanged);
    }
}


ScDBData* ScDBCollection::FindIndex(USHORT nIndex)
{
    USHORT i = 0;
    while (i < nCount)
    {
        if ((*this)[i]->GetIndex() == nIndex)
            return (*this)[i];
        i++;
    }
    return NULL;
}

BOOL ScDBCollection::Insert(DataObject* pDataObject)
{
    if (!((ScDBData*)pDataObject)->GetIndex())      // schon gesetzt?
        ((ScDBData*)pDataObject)->SetIndex(nEntryIndex++);
    return SortedCollection::Insert(pDataObject);
}




