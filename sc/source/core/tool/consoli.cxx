/*************************************************************************
 *
 *  $RCSfile: consoli.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:17 $
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

// INCLUDE ---------------------------------------------------------------

#include <tools/debug.hxx>
#include <math.h>
#include <string.h>
#include "consoli.hxx"
#include "document.hxx"
#include "olinetab.hxx"
#include "globstr.hrc"
#include "subtotal.hxx"
#include "compiler.hxx"                 // fuer errNoValue
#include "cell.hxx"

#define SC_CONS_NOTFOUND    0xFFFF

// STATIC DATA -----------------------------------------------------------

/*  Strings bei Gelegenheit ganz raus...
static USHORT nFuncRes[] = {                //  Reihenfolge wie bei enum ScSubTotalFunc
        0,                                  //  none
        STR_PIVOTFUNC_AVG,
        STR_PIVOTFUNC_COUNT,
        STR_PIVOTFUNC_COUNT2,
        STR_PIVOTFUNC_MAX,
        STR_PIVOTFUNC_MIN,
        STR_PIVOTFUNC_PROD,
        STR_PIVOTFUNC_STDDEV,
        STR_PIVOTFUNC_STDDEV2,
        STR_PIVOTFUNC_SUM,
        STR_PIVOTFUNC_VAR,
        STR_PIVOTFUNC_VAR2 };
*/

static OpCode eOpCodeTable[] = {            //  Reihenfolge wie bei enum ScSubTotalFunc
        ocBad,                              //  none
        ocAverage,
        ocCount,
        ocCount2,
        ocMax,
        ocMin,
        ocProduct,
        ocStDev,
        ocStDevP,
        ocSum,
        ocVar,
        ocVarP };

// -----------------------------------------------------------------------

void ScReferenceList::AddEntry( USHORT nCol, USHORT nRow, USHORT nTab )
{
    ScReferenceEntry* pOldData = pData;
    pData = new ScReferenceEntry[ nFullSize+1 ];
    if (pOldData)
    {
        memmove( pData, pOldData, nCount * sizeof(ScReferenceEntry) );
        delete[] pOldData;
    }
    while (nCount < nFullSize)
    {
        pData[nCount].nCol = SC_CONS_NOTFOUND;
        pData[nCount].nRow = SC_CONS_NOTFOUND;
        pData[nCount].nTab = SC_CONS_NOTFOUND;
        ++nCount;
    }
    pData[nCount].nCol = nCol;
    pData[nCount].nRow = nRow;
    pData[nCount].nTab = nTab;
    ++nCount;
    nFullSize = nCount;
}

void lcl_AddString( String**& pData, USHORT& nCount, const String& rInsert )
{
    String** pOldData = pData;
    pData = new String*[ nCount+1 ];
    if (pOldData)
    {
        memmove( pData, pOldData, nCount * sizeof(String*) );
        delete[] pOldData;
    }
    pData[nCount] = new String(rInsert);
    ++nCount;
}

// -----------------------------------------------------------------------

ScConsData::ScConsData() :
    eFunction(SUBTOTAL_FUNC_SUM),
    bReference(FALSE),
    bColByName(FALSE),
    bRowByName(FALSE),
    bSubTitles(FALSE),
    nColCount(0),
    ppColHeaders(NULL),
    nRowCount(0),
    ppRowHeaders(NULL),
    ppCount(NULL),
    ppSum(NULL),
    ppSumSqr(NULL),
    ppRefs(NULL),
    ppUsed(NULL),
    nDataCount(0),
    nTitleCount(0),
    ppTitles(NULL),
    ppTitlePos(NULL),
    bCornerUsed(FALSE)
{
}

ScConsData::~ScConsData()
{
    DeleteData();
}


#define DELETEARR(ppArray,nCount)   \
{                                   \
    USHORT i;                       \
    if (ppArray)                    \
        for(i=0; i<nCount; i++)     \
            delete[] ppArray[i];    \
    delete[] ppArray;               \
    ppArray = NULL;                 \
}

#define DELETESTR(ppArray,nCount)   \
{                                   \
    USHORT i;                       \
    if (ppArray)                    \
        for(i=0; i<nCount; i++)     \
            delete ppArray[i];      \
    delete[] ppArray;               \
    ppArray = NULL;                 \
}

void ScConsData::DeleteData()
{
    USHORT i;

    if (ppRefs)
        for (i=0; i<nColCount; i++)
        {
            for (USHORT j=0; j<nRowCount; j++)
                if (ppUsed[i][j])
                    ppRefs[i][j].Clear();
            delete[] ppRefs[i];
        }
    delete[] ppRefs;
    ppRefs = NULL;

//  DELETEARR( ppData1, nColCount );
//  DELETEARR( ppData2, nColCount );
    DELETEARR( ppCount, nColCount );
    DELETEARR( ppSum,   nColCount );
    DELETEARR( ppSumSqr,nColCount );
    DELETEARR( ppUsed,  nColCount );                // erst nach ppRefs !!!
    DELETEARR( ppTitlePos, nRowCount );
    DELETESTR( ppColHeaders, nColCount );
    DELETESTR( ppRowHeaders, nRowCount );
    DELETESTR( ppTitles, nTitleCount );
    nTitleCount = 0;
    nDataCount = 0;

    if (bColByName) nColCount = 0;                  // sonst stimmt ppColHeaders nicht
    if (bRowByName) nRowCount = 0;

    bCornerUsed = FALSE;
    aCornerText.Erase();
}

#undef DELETEARR
#undef DELETESTR

void ScConsData::InitData( BOOL bDelete )
{
    USHORT i;
    if (bDelete)
        DeleteData();

    if (bReference && nColCount && !ppRefs)
    {
        ppRefs = new ScReferenceList*[nColCount];
        for (i=0; i<nColCount; i++)
            ppRefs[i] = new ScReferenceList[nRowCount];
    }
    else if (nColCount && !ppCount)
    {
        ppCount  = new double*[nColCount];
        ppSum    = new double*[nColCount];
        ppSumSqr = new double*[nColCount];
        for (i=0; i<nColCount; i++)
        {
            ppCount[i]  = new double[nRowCount];
            ppSum[i]    = new double[nRowCount];
            ppSumSqr[i] = new double[nRowCount];
        }
    }

    if (nColCount && !ppUsed)
    {
        ppUsed = new BOOL*[nColCount];
        for (i=0; i<nColCount; i++)
        {
            ppUsed[i] = new BOOL[nRowCount];
            memset( ppUsed[i], 0, nRowCount * sizeof(BOOL) );
        }
    }

    if (nRowCount && nDataCount && !ppTitlePos)
    {
        ppTitlePos = new USHORT*[nRowCount];
        for (i=0; i<nRowCount; i++)
        {
            ppTitlePos[i] = new USHORT[nDataCount];
            memset( ppTitlePos[i], 0, nDataCount * sizeof(USHORT) );    //! unnoetig ?
        }
    }

    //  CornerText: einzelner String
}

void ScConsData::DoneFields()
{
    InitData(FALSE);
}

void ScConsData::SetSize( USHORT nCols, USHORT nRows )
{
    DeleteData();
    nColCount = nCols;
    nRowCount = nRows;
}

void ScConsData::GetSize( USHORT& rCols, USHORT& rRows ) const
{
    rCols = nColCount;
    rRows = nRowCount;
}

void ScConsData::SetFlags( ScSubTotalFunc eFunc, BOOL bColName, BOOL bRowName, BOOL bRef )
{
    DeleteData();
    bReference = bRef;
    bColByName = bColName;
    if (bColName) nColCount = 0;
    bRowByName = bRowName;
    if (bRowName) nRowCount = 0;
    eFunction = eFunc;
}

void ScConsData::AddFields( ScDocument* pSrcDoc, USHORT nTab,
                            USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2 )
{
    ++nDataCount;

    String aTitle;

    USHORT nStartCol = nCol1;
    USHORT nStartRow = nRow1;
    if (bColByName) ++nStartRow;
    if (bRowByName) ++nStartCol;

    if (bColByName)
    {
        for (USHORT nCol=nStartCol; nCol<=nCol2; nCol++)
        {
            pSrcDoc->GetString( nCol, nRow1, nTab, aTitle );
            if (aTitle.Len())
            {
                BOOL bFound = FALSE;
                for (USHORT i=0; i<nColCount && !bFound; i++)
                    if ( *ppColHeaders[i] == aTitle )
                        bFound = TRUE;
                if (!bFound)
                    lcl_AddString( ppColHeaders, nColCount, aTitle );
            }
        }
    }

    if (bRowByName)
    {
        for (USHORT nRow=nStartRow; nRow<=nRow2; nRow++)
        {
            pSrcDoc->GetString( nCol1, nRow, nTab, aTitle );
            if (aTitle.Len())
            {
                BOOL bFound = FALSE;
                for (USHORT i=0; i<nRowCount && !bFound; i++)
                    if ( *ppRowHeaders[i] == aTitle )
                        bFound = TRUE;
                if (!bFound)
                    lcl_AddString( ppRowHeaders, nRowCount, aTitle );
            }
        }
    }
}

void ScConsData::AddName( const String& rName )
{
    USHORT nArrX;
    USHORT nArrY;

    if (bReference)
    {
        lcl_AddString( ppTitles, nTitleCount, rName );

        for (nArrY=0; nArrY<nRowCount; nArrY++)
        {
            //  Daten auf gleiche Laenge bringen

            USHORT nMax = 0;
            for (nArrX=0; nArrX<nColCount; nArrX++)
                if (ppUsed[nArrX][nArrY])
                    nMax = Max( nMax, ppRefs[nArrX][nArrY].GetCount() );

            for (nArrX=0; nArrX<nColCount; nArrX++)
            {
                if (!ppUsed[nArrX][nArrY])
                {
                    ppUsed[nArrX][nArrY] = TRUE;
                    ppRefs[nArrX][nArrY].Init();
                }
                ppRefs[nArrX][nArrY].SetFullSize(nMax);
            }

            //  Positionen eintragen

            if (ppTitlePos)
                if (nTitleCount < nDataCount)
                    ppTitlePos[nArrY][nTitleCount] = nMax;
        }
    }
}

                                // rCount < 0 <=> Fehler aufgetreten

void lcl_UpdateArray( ScSubTotalFunc eFunc,
                         double& rCount, double& rSum, double& rSumSqr, double nVal )
{
    if (rCount < 0.0)
        return;
    switch (eFunc)
    {
        case SUBTOTAL_FUNC_SUM:
            if (!SubTotal::SafePlus(rSum, nVal))
                rCount = -MAXDOUBLE;
            break;
        case SUBTOTAL_FUNC_PROD:
            if (!SubTotal::SafeMult(rSum, nVal))
                rCount = -MAXDOUBLE;
            break;
        case SUBTOTAL_FUNC_CNT:
        case SUBTOTAL_FUNC_CNT2:
            rCount += 1.0;
            break;
        case SUBTOTAL_FUNC_AVE:
            if (!SubTotal::SafePlus(rSum, nVal))
                rCount = -MAXDOUBLE;
            else
                rCount += 1.0;
            break;
        case SUBTOTAL_FUNC_MAX:
            if (nVal > rSum)
                rSum = nVal;
            break;
        case SUBTOTAL_FUNC_MIN:
            if (nVal < rSum)
                rSum = nVal;
            break;
        case SUBTOTAL_FUNC_STD:
        case SUBTOTAL_FUNC_STDP:
        case SUBTOTAL_FUNC_VAR:
        case SUBTOTAL_FUNC_VARP:
        {
            BOOL bOk = SubTotal::SafePlus(rSum, nVal);
            bOk = bOk && SubTotal::SafeMult(nVal, nVal);
            bOk = bOk && SubTotal::SafePlus(rSumSqr, nVal);
            if (!bOk)
                rCount = -MAXDOUBLE;
            else
                rCount += 1.0;
            break;
        }
    }
}

void lcl_InitArray( ScSubTotalFunc eFunc,
                       double& rCount, double& rSum, double& rSumSqr, double nVal )
{
    rCount = 1.0;
    switch (eFunc)
    {
        case SUBTOTAL_FUNC_SUM:
        case SUBTOTAL_FUNC_MAX:
        case SUBTOTAL_FUNC_MIN:
        case SUBTOTAL_FUNC_PROD:
        case SUBTOTAL_FUNC_AVE:
            rSum = nVal;
            break;
        case SUBTOTAL_FUNC_STD:
        case SUBTOTAL_FUNC_STDP:
        case SUBTOTAL_FUNC_VAR:
        case SUBTOTAL_FUNC_VARP:
        {
            rSum = nVal;
            BOOL bOk = SubTotal::SafeMult(nVal, nVal);
            if (bOk)
                rSumSqr = nVal;
            else
                rCount = -MAXDOUBLE;
        }
            break;
        default:
            break;
    }
}

double lcl_CalcData( ScSubTotalFunc eFunc,
                        double fCount, double fSum, double fSumSqr)
{
    if (fCount < 0.0)
        return 0.0;
    double fVal = 0.0;
    switch (eFunc)
    {
        case SUBTOTAL_FUNC_CNT:
        case SUBTOTAL_FUNC_CNT2:
            fVal = fCount;
            break;
        case SUBTOTAL_FUNC_SUM:
        case SUBTOTAL_FUNC_MAX:
        case SUBTOTAL_FUNC_MIN:
        case SUBTOTAL_FUNC_PROD:
            fVal = fSum;
            break;
        case SUBTOTAL_FUNC_AVE:
            if (fCount > 0.0)
                fVal = fSum / fCount;
            else
                fCount = -MAXDOUBLE;
            break;
        case SUBTOTAL_FUNC_STD:
        {
            if (fCount > 1 && SubTotal::SafeMult(fSum, fSum))
                fVal = sqrt((fSumSqr - fSum/fCount)/(fCount-1.0));
            else
                fCount = -MAXDOUBLE;
        }
            break;
        case SUBTOTAL_FUNC_STDP:
        {
            if (fCount > 0 && SubTotal::SafeMult(fSum, fSum))
                fVal = sqrt((fSumSqr - fSum/fCount)/fCount);
            else
                fCount = -MAXDOUBLE;
        }
            break;
        case SUBTOTAL_FUNC_VAR:
        {
            if (fCount > 1 && SubTotal::SafeMult(fSum, fSum))
                fVal = (fSumSqr - fSum/fCount)/(fCount-1.0);
            else
                fCount = -MAXDOUBLE;
        }
            break;
        case SUBTOTAL_FUNC_VARP:
        {
            if (fCount > 0 && SubTotal::SafeMult(fSum, fSum))
                fVal = (fSumSqr - fSum/fCount)/fCount;
            else
                fCount = -MAXDOUBLE;
        }
            break;
        default:
        {
            DBG_ERROR("unbekannte Funktion bei Consoli::CalcData");
            fCount = -MAXDOUBLE;
        }
            break;
    }
    return fVal;
}

void ScConsData::AddData( ScDocument* pSrcDoc, USHORT nTab,
                            USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2 )
{
    PutInOrder(nCol1,nCol2);
    PutInOrder(nRow1,nRow2);
    if ( nCol2 >= nCol1 + nColCount && !bColByName )
    {
        DBG_ASSERT(0,"Bereich zu gross");
        nCol2 = nCol1 + nColCount - 1;
    }
    if ( nRow2 >= nRow1 + nRowCount && !bRowByName )
    {
        DBG_ASSERT(0,"Bereich zu gross");
        nRow2 = nRow1 + nRowCount - 1;
    }

    USHORT nCol;
    USHORT nRow;

    //      Ecke links oben

    if ( bColByName && bRowByName )
    {
        String aThisCorner;
        pSrcDoc->GetString(nCol1,nRow1,nTab,aThisCorner);
        if (bCornerUsed)
        {
            if (aCornerText != aThisCorner)
                aCornerText.Erase();
        }
        else
        {
            aCornerText = aThisCorner;
            bCornerUsed = TRUE;
        }
    }

    //      Titel suchen

    USHORT nStartCol = nCol1;
    USHORT nStartRow = nRow1;
    if (bColByName) ++nStartRow;
    if (bRowByName) ++nStartCol;
    String aTitle;
    USHORT* pDestCols = NULL;
    USHORT* pDestRows = NULL;
    if (bColByName)
    {
        pDestCols = new USHORT[nCol2-nStartCol+1];
        for (nCol=nStartCol; nCol<=nCol2; nCol++)
        {
            pSrcDoc->GetString(nCol,nRow1,nTab,aTitle);
            USHORT nPos = SC_CONS_NOTFOUND;
            if (aTitle.Len())
            {
                BOOL bFound = FALSE;
                for (USHORT i=0; i<nColCount && !bFound; i++)
                    if ( *ppColHeaders[i] == aTitle )
                    {
                        nPos = i;
                        bFound = TRUE;
                    }
                DBG_ASSERT(bFound, "Spalte nicht gefunden");
            }
            pDestCols[nCol-nStartCol] = nPos;
        }
    }
    if (bRowByName)
    {
        pDestRows = new USHORT[nRow2-nStartRow+1];
        for (nRow=nStartRow; nRow<=nRow2; nRow++)
        {
            pSrcDoc->GetString(nCol1,nRow,nTab,aTitle);
            USHORT nPos = SC_CONS_NOTFOUND;
            if (aTitle.Len())
            {
                BOOL bFound = FALSE;
                for (USHORT i=0; i<nRowCount && !bFound; i++)
                    if ( *ppRowHeaders[i] == aTitle )
                    {
                        nPos = i;
                        bFound = TRUE;
                    }
                DBG_ASSERT(bFound, "Zeile nicht gefunden");
            }
            pDestRows[nRow-nStartRow] = nPos;
        }
    }
    nCol1 = nStartCol;
    nRow1 = nStartRow;

    //      Daten

    BOOL bAnyCell = ( eFunction == SUBTOTAL_FUNC_CNT2 );
    for (nCol=nCol1; nCol<=nCol2; nCol++)
    {
        USHORT nArrX = nCol-nCol1;
        if (bColByName) nArrX = pDestCols[nArrX];
        if (nArrX != SC_CONS_NOTFOUND)
        {
            for (nRow=nRow1; nRow<=nRow2; nRow++)
            {
                USHORT nArrY = nRow-nRow1;
                if (bRowByName) nArrY = pDestRows[nArrY];
                if ( nArrY != SC_CONS_NOTFOUND && (
                        bAnyCell ? pSrcDoc->HasData( nCol, nRow, nTab )
                                 : pSrcDoc->HasValueData( nCol, nRow, nTab ) ) )
                {
                    if (bReference)
                    {
                        if (ppUsed[nArrX][nArrY])
                            ppRefs[nArrX][nArrY].AddEntry( nCol, nRow, nTab );
                        else
                        {
                            ppUsed[nArrX][nArrY] = TRUE;
                            ppRefs[nArrX][nArrY].Init();
                            ppRefs[nArrX][nArrY].AddEntry( nCol, nRow, nTab );
                        }
                    }
                    else
                    {
                        double nVal;
                        pSrcDoc->GetValue( nCol, nRow, nTab, nVal );
                        if (ppUsed[nArrX][nArrY])
                            lcl_UpdateArray( eFunction, ppCount[nArrX][nArrY],
                                         ppSum[nArrX][nArrY], ppSumSqr[nArrX][nArrY],
                                         nVal);
                        else
                        {
                            ppUsed[nArrX][nArrY] = TRUE;
                            lcl_InitArray( eFunction, ppCount[nArrX][nArrY],
                                                  ppSum[nArrX][nArrY],
                                                  ppSumSqr[nArrX][nArrY], nVal );
                        }
                    }
                }
            }
        }
    }

    delete[] pDestCols;
    delete[] pDestRows;
}

//  vorher testen, wieviele Zeilen eingefuegt werden (fuer Undo)

USHORT ScConsData::GetInsertCount() const
{
    USHORT nInsert = 0;
    USHORT nArrX;
    USHORT nArrY;
    if ( ppRefs && ppUsed )
    {
        for (nArrY=0; nArrY<nRowCount; nArrY++)
        {
            USHORT nNeeded = 0;
            for (nArrX=0; nArrX<nColCount; nArrX++)
                if (ppUsed[nArrX][nArrY])
                    nNeeded = Max( nNeeded, ppRefs[nArrX][nArrY].GetCount() );

            nInsert += nNeeded;
        }
    }
    return nInsert;
}

//  fertige Daten ins Dokument schreiben
//! optimieren nach Spalten?

void ScConsData::OutputToDocument( ScDocument* pDestDoc, USHORT nCol, USHORT nRow, USHORT nTab )
{
    OpCode eOpCode = eOpCodeTable[eFunction];

    USHORT nArrX;
    USHORT nArrY;
    USHORT nCount;
    USHORT nPos;
    USHORT i;

    //  Ecke links oben

    if ( bColByName && bRowByName && aCornerText.Len() )
        pDestDoc->SetString( nCol, nRow, nTab, aCornerText );

    //  Titel

    USHORT nStartCol = nCol;
    USHORT nStartRow = nRow;
    if (bColByName) ++nStartRow;
    if (bRowByName) ++nStartCol;

    if (bColByName)
        for (i=0; i<nColCount; i++)
            pDestDoc->SetString( nStartCol+i, nRow, nTab, *ppColHeaders[i] );
    if (bRowByName)
        for (i=0; i<nRowCount; i++)
            pDestDoc->SetString( nCol, nStartRow+i, nTab, *ppRowHeaders[i] );

    nCol = nStartCol;
    nRow = nStartRow;

    //  Daten

    if ( ppCount && ppUsed )                            // Werte direkt einfuegen
    {
        for (nArrX=0; nArrX<nColCount; nArrX++)
            for (nArrY=0; nArrY<nRowCount; nArrY++)
                if (ppUsed[nArrX][nArrY])
                {
                    double fVal = lcl_CalcData( eFunction, ppCount[nArrX][nArrY],
                                                ppSum[nArrX][nArrY],
                                                ppSumSqr[nArrX][nArrY]);
                    if (ppCount[nArrX][nArrY] < 0.0)
                        pDestDoc->SetError( nCol+nArrX, nRow+nArrY, nTab, errNoValue );
                    else
                        pDestDoc->SetValue( nCol+nArrX, nRow+nArrY, nTab, fVal );
                }
    }

    if ( ppRefs && ppUsed )                             // Referenzen einfuegen
    {
                                //! unterscheiden, ob nach Kategorien aufgeteilt
        String aString;

        SingleRefData aSRef;        // Daten fuer Referenz-Formelzellen
        aSRef.InitFlags();
        aSRef.SetFlag3D(TRUE);

        ComplRefData aCRef;         // Daten fuer Summen-Zellen
        aCRef.InitFlags();
        aCRef.Ref1.SetColRel(TRUE); aCRef.Ref1.SetRowRel(TRUE); aCRef.Ref1.SetTabRel(TRUE);
        aCRef.Ref2.SetColRel(TRUE); aCRef.Ref2.SetRowRel(TRUE); aCRef.Ref2.SetTabRel(TRUE);

        for (nArrY=0; nArrY<nRowCount; nArrY++)
        {
            USHORT nNeeded = 0;
            for (nArrX=0; nArrX<nColCount; nArrX++)
                if (ppUsed[nArrX][nArrY])
                    nNeeded = Max( nNeeded, ppRefs[nArrX][nArrY].GetCount() );

            if (nNeeded)
            {
                pDestDoc->InsertRow( 0,nTab, MAXCOL,nTab, nRow+nArrY, nNeeded );

                for (nArrX=0; nArrX<nColCount; nArrX++)
                    if (ppUsed[nArrX][nArrY])
                    {
                        ScReferenceList& rList = ppRefs[nArrX][nArrY];
                        nCount = rList.GetCount();
                        if (nCount)
                        {
                            for (nPos=0; nPos<nCount; nPos++)
                            {
                                ScReferenceEntry aRef = rList.GetEntry(nPos);
                                if (aRef.nTab != SC_CONS_NOTFOUND)
                                {
                                    //  Referenz einfuegen (absolut, 3d)

                                    aSRef.nCol = aRef.nCol;
                                    aSRef.nRow = aRef.nRow;
                                    aSRef.nTab = aRef.nTab;

                                    ScTokenArray aRefArr;
                                    aRefArr.AddSingleReference(aSRef);
                                    aRefArr.AddOpCode(ocStop);
                                    ScAddress aDest( nCol+nArrX, nRow+nArrY+nPos, nTab );
                                    ScBaseCell* pCell = new ScFormulaCell( pDestDoc, aDest, &aRefArr );
                                    pDestDoc->PutCell( aDest.Col(), aDest.Row(), aDest.Tab(), pCell );
                                }
                            }

                            //  Summe einfuegen (relativ, nicht 3d)

                            ScAddress aDest( nCol+nArrX, nRow+nArrY+nNeeded, nTab );

                            aCRef.Ref1.nTab = aCRef.Ref2.nTab = nTab;
                            aCRef.Ref1.nCol = aCRef.Ref2.nCol = nCol+nArrX;
                            aCRef.Ref1.nRow = nRow+nArrY;
                            aCRef.Ref2.nRow = nRow+nArrY+nNeeded-1;
                            aCRef.CalcRelFromAbs( aDest );

                            ScTokenArray aArr;
                            aArr.AddOpCode(eOpCode);            // ausgewaehlte Funktion
                            aArr.AddOpCode(ocOpen);
                            aArr.AddDoubleReference(aCRef);
                            aArr.AddOpCode(ocClose);
                            aArr.AddOpCode(ocStop);
                            ScBaseCell* pCell = new ScFormulaCell( pDestDoc, aDest, &aArr );
                            pDestDoc->PutCell( aDest.Col(), aDest.Row(), aDest.Tab(), pCell );
                        }
                    }

                //  Gliederung einfuegen

                ScOutlineArray* pOutArr = pDestDoc->GetOutlineTable( nTab, TRUE )->GetRowArray();
                USHORT nOutStart = nRow+nArrY;
                USHORT nOutEnd = nRow+nArrY+nNeeded-1;
                BOOL bSize = FALSE;
                pOutArr->Insert( nOutStart, nOutEnd, bSize );
                for (USHORT nOutRow=nOutStart; nOutRow<=nOutEnd; nOutRow++)
                    pDestDoc->ShowRow( nOutRow, nTab, FALSE );
                pDestDoc->UpdateOutlineRow( nOutStart, nOutEnd, nTab, FALSE );

                //  Zwischentitel

                if (ppTitlePos && ppTitles && ppRowHeaders)
                {
                    String aDelim( RTL_CONSTASCII_USTRINGPARAM(" / ") );
                    for (nPos=0; nPos<nDataCount; nPos++)
                    {
                        USHORT nTPos = ppTitlePos[nArrY][nPos];
                        BOOL bDo = TRUE;
                        if (nPos+1<nDataCount)
                            if (ppTitlePos[nArrY][nPos+1] == nTPos)
                                bDo = FALSE;                                    // leer
                        if ( bDo && nTPos < nNeeded )
                        {
                            aString =  *ppRowHeaders[nArrY];
                            aString += aDelim;
                            aString += *ppTitles[nPos];
                            pDestDoc->SetString( nCol-1, nRow+nArrY+nTPos, nTab, aString );
                        }
                    }
                }

                nRow += nNeeded;
            }
        }
    }
}





