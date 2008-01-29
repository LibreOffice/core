/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pivot.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:19:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// -----------------------------------------------------------------------

#ifdef _MSC_VER
#pragma optimize("",off)
#endif

// INCLUDE ---------------------------------------------------------------

#include <svtools/zforlist.hxx>
#include <tools/solar.h>
#include <string.h>
#include <math.h>


#include "globstr.hrc"
#include "global.hxx"
#include "subtotal.hxx"
#include "scitems.hxx"
#include "attrib.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "document.hxx"
#include "userlist.hxx"
#include "pivot.hxx"
#include "cell.hxx"
#include "rechead.hxx"
#include "compiler.hxx"                         // fuer errNoValue
#include "progress.hxx"


// STATIC DATA -----------------------------------------------------------

//! bei Gelegenheit...

static short    nStaticStrRefCount = 0;
static String*  pLabel[PIVOT_MAXFUNC+1];            // incl. "auto"
static String*  pLabelTotal;
static String*  pLabelData;

static SCSIZE nDataMult = 1;

#define nFirstLine  2

static const USHORT nFuncMaskArr[PIVOT_MAXFUNC+1] =
    {   PIVOT_FUNC_SUM,
        PIVOT_FUNC_COUNT,
        PIVOT_FUNC_AVERAGE,
        PIVOT_FUNC_MAX,
        PIVOT_FUNC_MIN,
        PIVOT_FUNC_PRODUCT,
        PIVOT_FUNC_COUNT_NUM,
        PIVOT_FUNC_STD_DEV,
        PIVOT_FUNC_STD_DEVP,
        PIVOT_FUNC_STD_VAR,
        PIVOT_FUNC_STD_VARP,
        PIVOT_FUNC_AUTO };              // automatisch

// -----------------------------------------------------------------------

//  1   Filter-Knopf
//  2   Feldnamen links
//  3   "Daten" links
//  4   Feldnamen oben
//  5   "Daten" oben
//  6   einzelne "Gesamt" oben rechts
//  7   "Gesamt" oben rechts
//  8   einzelne "Gesamt" unten links
//  9   "Gesamt" unten links
//  10  innere Kategorie links
//  11  Teilergebnis Label einzeln links
//  12  Teilergebnis Label gesamt links
//  13  letzte Kategorie links
//  14  innere Kategorie oben
//  15  Teilergebnis Label einzeln oben
//  16  Teilergebnis Label gesamt oben
//  17  letzte Kategorie oben
//  18  Werte innen
//  19  Werte in Teilergebnisspalte
//  20  Werte in Gesamt-Spalte
//  21  Werte in einzelnen Gesamt-Spalten
//  22  Werte in Ergebnis-Zeile Teilergebnis oder Gesamt
//  23  Kreuzung von Spalte/Zeile (Teilergebnis-Spalte)
//  24  Kreuzung von Spalte/Zeile (Gesamt-Spalte)
//  25  wie 24 bei einzelnen "Gesamt"

SCSIZE lcl_MaskToIndex( USHORT nFuncMask )
{
    SCSIZE i;
    for (i=0; i<=PIVOT_MAXFUNC; i++)
        if (nFuncMask == nFuncMaskArr[i])
            return i;

    DBG_ERROR("Falsche Maske in MaskToIndex");
    return 0;
}

BOOL lcl_IsEmptyLine( ScDocument* pDoc, const ScAddress& rPos, SCCOL nCol2 )
{
            //! ans Document verschieben !!!

    ScAddress aAdr( rPos );
    for (SCCOL nCol=aAdr.Col(); nCol<=nCol2; nCol++)
    {
        aAdr.SetCol( nCol );
        if ( pDoc->GetCell( aAdr ) )
            return FALSE;
    }
    return TRUE;
}

ScPivot::ScPivot(ScDocument* pDocument) :
    pDoc            (pDocument),
    aQuery          (),
    bHasHeader      (FALSE),
    bIgnoreEmpty    (FALSE),
    bDetectCat      (FALSE),
    bMakeTotalCol   (TRUE),
    bMakeTotalRow   (TRUE),
    nColNameCount   (0),
    pColNames       (NULL),
    nSrcCol1        (0),
    nSrcRow1        (0),
    nSrcCol2        (0),
    nSrcRow2        (0),
    nSrcTab         (0),
    nDestCol1       (0),
    nDestRow1       (0),
    nDestCol2       (0),
    nDestRow2       (0),
    nDestTab        (0),
    nDataStartCol   (0),
    nDataStartRow   (0),
    nColCount       (0),
    nRowCount       (0),
    nDataCount      (0),
    bValidArea      (FALSE),
    bDataAtCol      (FALSE)
{
    for (SCSIZE i=0; i<PIVOT_MAXFIELD; i++)
    {
        pColList[i] = new PivotStrCollection();
        pRowList[i] = new PivotStrCollection();
    }
    pDataList = pColList[0];
    ppDataArr = NULL;
    nDataColCount = 0;
    nDataRowCount = 0;
    nRecCount = 0;
    pColRef = NULL;

    // Initialisierung der statischen Strings, wenn noetig
    nStaticStrRefCount += 1;
    if ( nStaticStrRefCount < 2 )
    {
        pLabelTotal = new String( ScGlobal::GetRscString(STR_PIVOT_TOTAL) );
        pLabelData  = new String( ScGlobal::GetRscString(STR_PIVOT_DATA) );

        for (SCSIZE i=0; i<=PIVOT_MAXFUNC; i++ )    //  incl. "auto"
            pLabel[i] = new String;                 //  kein Leerzeichen

        *pLabel[ 0] = ScGlobal::GetRscString(STR_FUN_TEXT_SUM);
        *pLabel[ 1] = ScGlobal::GetRscString(STR_FUN_TEXT_COUNT);
        *pLabel[ 2] = ScGlobal::GetRscString(STR_FUN_TEXT_AVG);
        *pLabel[ 3] = ScGlobal::GetRscString(STR_FUN_TEXT_MAX);
        *pLabel[ 4] = ScGlobal::GetRscString(STR_FUN_TEXT_MIN);
        *pLabel[ 5] = ScGlobal::GetRscString(STR_FUN_TEXT_PRODUCT);
        *pLabel[ 6] = ScGlobal::GetRscString(STR_FUN_TEXT_COUNT);           // Count2
        *pLabel[ 7] = ScGlobal::GetRscString(STR_FUN_TEXT_STDDEV);
        *pLabel[ 8] = ScGlobal::GetRscString(STR_FUN_TEXT_STDDEV);          // Stddev2
        *pLabel[ 9] = ScGlobal::GetRscString(STR_FUN_TEXT_VAR);
        *pLabel[10] = ScGlobal::GetRscString(STR_FUN_TEXT_VAR);             // Var2
        *pLabel[11] = ScGlobal::GetRscString(STR_TABLE_ERGEBNIS);
    }
}

ScPivot::ScPivot(const ScPivot& rPivot):
    DataObject(),
    pDoc            (rPivot.pDoc),
    aQuery          (rPivot.aQuery),
    bHasHeader      (rPivot.bHasHeader),
    bIgnoreEmpty    (rPivot.bIgnoreEmpty),
    bDetectCat      (rPivot.bDetectCat),
    bMakeTotalCol   (rPivot.bMakeTotalCol),
    bMakeTotalRow   (rPivot.bMakeTotalRow),
    aName           (rPivot.aName),
    aTag            (rPivot.aTag),
    nColNameCount   (0),
    pColNames       (NULL),
    nSrcCol1        (rPivot.nSrcCol1),
    nSrcRow1        (rPivot.nSrcRow1),
    nSrcCol2        (rPivot.nSrcCol2),
    nSrcRow2        (rPivot.nSrcRow2),
    nSrcTab         (rPivot.nSrcTab),
    nDestCol1       (rPivot.nDestCol1),
    nDestRow1       (rPivot.nDestRow1),
    nDestCol2       (rPivot.nDestCol2),
    nDestRow2       (rPivot.nDestRow2),
    nDestTab        (rPivot.nDestTab),
    nDataStartCol   (0),
    nDataStartRow   (0),
    nColCount       (0),
    nRowCount       (0),
    nDataCount      (0),
    bValidArea      (FALSE),
    bDataAtCol      (FALSE)
{
    if (rPivot.nColNameCount>0 && rPivot.pColNames)
    {
        nColNameCount = rPivot.nColNameCount;
        pColNames = new String[nColNameCount];
        for (SCSIZE nCol=0; nCol<nColNameCount; nCol++)
            pColNames[nCol] = rPivot.pColNames[nCol];
    }

    for (SCSIZE i=0; i<PIVOT_MAXFIELD; i++)
    {
        pColList[i] = new PivotStrCollection();
        pRowList[i] = new PivotStrCollection();
    }
    pDataList = pColList[0];
    ppDataArr = NULL;
    nRecCount = 0;
    pColRef = NULL;

    SetColFields( rPivot.aColArr, rPivot.nColCount );
    SetRowFields( rPivot.aRowArr, rPivot.nRowCount );
    SetDataFields( rPivot.aDataArr, rPivot.nDataCount );

    nStaticStrRefCount += 1;
}

ScPivot::~ScPivot()
{
    for (SCSIZE i=0; i<PIVOT_MAXFIELD; i++)
    {
        delete pColList[i];
        delete pRowList[i];
    }
    if (ppDataArr)
    {
        for (SCSIZE j=0; j<nDataRowCount; j++)
            delete[] ppDataArr[j];
        delete[] ppDataArr;
        ppDataArr = NULL;
    }
    delete[] pColRef;

    delete[] pColNames;

    // statische Strings ggF. wieder abraeumen
    nStaticStrRefCount -= 1;
    if ( nStaticStrRefCount == 0 )
    {
        delete pLabelTotal;
        delete pLabelData;

        for ( SCSIZE k=0; k<=PIVOT_MAXFUNC; k++ )           // incl. "auto"
            delete pLabel[k];
    }
}

ScPivot* ScPivot::CreateNew() const
{
    ScPivot* pNewPivot = new ScPivot( pDoc );

    pNewPivot->SetQuery(aQuery);
    pNewPivot->SetHeader(bHasHeader);
    pNewPivot->SetIgnoreEmpty(bIgnoreEmpty);
    pNewPivot->SetDetectCat(bDetectCat);
    pNewPivot->SetMakeTotalCol(bMakeTotalCol);
    pNewPivot->SetMakeTotalRow(bMakeTotalRow);

    pNewPivot->SetSrcArea( nSrcCol1, nSrcRow1, nSrcCol2, nSrcRow2, nSrcTab );
    pNewPivot->SetDestPos( nDestCol1, nDestRow1, nDestTab );

    return pNewPivot;
}

void lcl_LoadFieldArr30( SvStream& /* rStream */, PivotField* /* pField */, USHORT /* nCount */ )
{
#if SC_ROWLIMIT_STREAM_ACCESS
#error address types changed!
    USHORT i;

    for (i=0; i<nCount; i++)
    {
        rStream >> pField[i].nCol
                >> pField[i].nFuncMask
                >> pField[i].nFuncCount;
    }
#endif
}

void lcl_LoadFieldArr( SvStream& /* rStream */, PivotField* /* pField */, USHORT /* nCount */ )
{
#if SC_ROWLIMIT_STREAM_ACCESS
#error address types changed!
    USHORT i;

    for (i=0; i<nCount; i++)
    {
        BYTE cData;
        rStream >> cData;
        if( cData & 0x0F )
            rStream.SeekRel( cData & 0x0F );
        rStream >> pField[i].nCol
                >> pField[i].nFuncMask
                >> pField[i].nFuncCount;
    }
#endif
}

void lcl_SaveFieldArr( SvStream& /* rStream */, const PivotField* /* pField */, USHORT /* nCount */ )
{
#if SC_ROWLIMIT_STREAM_ACCESS
#error address types changed!
    USHORT i;

    for (i=0; i<nCount; i++)
    {
        rStream << (BYTE) 0x00
                << pField[i].nCol
                << pField[i].nFuncMask
                << pField[i].nFuncCount;
    }
#endif
}

//  nach Load muessen Daten neu berechnet werden !

BOOL ScPivot::Load( SvStream& /* rStream */, ScMultipleReadHeader& rHdr )
{
    rHdr.StartEntry();
#if SC_ROWLIMIT_STREAM_ACCESS
#error address types changed!

    rStream >> bHasHeader

            >> nSrcCol1
            >> nSrcRow1
            >> nSrcCol2
            >> nSrcRow2
            >> nSrcTab

            >> nDestCol1
            >> nDestRow1
            >> nDestCol2
            >> nDestRow2
            >> nDestTab;

                        //  Arrays immer ueber Set...Fields initalisieren!

    short nCount;
    PivotFieldArr aFieldArr;

    if( pDoc->GetSrcVersion() >= SC_DATABYTES2 )
    {
        rStream >> nCount;
        lcl_LoadFieldArr( rStream, aFieldArr, nCount );
        SetColFields(aFieldArr, nCount);

        rStream >> nCount;
        lcl_LoadFieldArr( rStream, aFieldArr, nCount );
        SetRowFields(aFieldArr, nCount);

        rStream >> nCount;
        lcl_LoadFieldArr( rStream, aFieldArr, nCount );
        SetDataFields(aFieldArr, nCount);
    }
    else
    {
        rStream >> nCount;
        lcl_LoadFieldArr30( rStream, aFieldArr, nCount );
        SetColFields(aFieldArr, nCount);

        rStream >> nCount;
        lcl_LoadFieldArr30( rStream, aFieldArr, nCount );
        SetRowFields(aFieldArr, nCount);

        rStream >> nCount;
        lcl_LoadFieldArr30( rStream, aFieldArr, nCount );
        SetDataFields(aFieldArr, nCount);
    }

    aQuery.Load( rStream );

    rStream >> bIgnoreEmpty;
    rStream >> bDetectCat;

    if (rHdr.BytesLeft())
    {
        rStream >> bMakeTotalCol;       // ab 355i
        rStream >> bMakeTotalRow;
    }

    if (rHdr.BytesLeft())               // ab 500a
    {
        rStream.ReadByteString( aName, rStream.GetStreamCharSet() );
        rStream.ReadByteString( aTag,  rStream.GetStreamCharSet() );

        DBG_ASSERT(!pColNames, "Spaltennamen schon gesetzt?")
        rStream >> nColNameCount;
        if (nColNameCount)
        {
            pColNames = new String[nColNameCount];
            for (SCCOL nCol=0; nCol<nColNameCount; nCol++)
                rStream.ReadByteString( pColNames[nCol], rStream.GetStreamCharSet() );
        }
    }
    // sonst wird hinterher aus ScPivotCollection::Load ein Name vergeben

    rHdr.EndEntry();
#endif
    return TRUE;
}

BOOL ScPivot::Store( SvStream& /* rStream */, ScMultipleWriteHeader& rHdr ) const
{
    rHdr.StartEntry();
#if SC_ROWLIMIT_STREAM_ACCESS
#error address types changed!

    rStream << bHasHeader

            << nSrcCol1
            << nSrcRow1
            << nSrcCol2
            << nSrcRow2
            << nSrcTab

            << nDestCol1
            << nDestRow1
            << nDestCol2
            << nDestRow2
            << nDestTab

            << nColCount;
    lcl_SaveFieldArr( rStream, aColArr, nColCount );
    rStream << nRowCount;
    lcl_SaveFieldArr( rStream, aRowArr, nRowCount );
    rStream << nDataCount;
    lcl_SaveFieldArr( rStream, aDataArr, nDataCount );

    aQuery.Store( rStream );

    rStream << bIgnoreEmpty;
    rStream << bDetectCat;

    rStream << bMakeTotalCol;       // ab 355i
    rStream << bMakeTotalRow;

    if( rStream.GetVersion() > SOFFICE_FILEFORMAT_40 )  // Name/Tag/Spalten ab 5.0
    {
        rStream.WriteByteString( aName, rStream.GetStreamCharSet() );
        rStream.WriteByteString( aTag,  rStream.GetStreamCharSet() );

        if (!pColNames) ((ScPivot*)this)->nColNameCount = 0;        // soll nicht sein
        rStream << nColNameCount;
        for (SCCOL nCol=0; nCol<nColNameCount; nCol++)
            rStream.WriteByteString( pColNames[nCol], rStream.GetStreamCharSet() );
    }

    rHdr.EndEntry();
#endif
    return TRUE;
}

void ScPivot::SetQuery(const ScQueryParam& rQuery)
{
    aQuery = rQuery;

    bValidArea = FALSE;
}

void ScPivot::GetQuery(ScQueryParam& rQuery) const
{
    rQuery = aQuery;
}

void ScPivot::SetHeader(BOOL bHeader)
{
    bHasHeader = bHeader;
    bValidArea = FALSE;
}

BOOL ScPivot::GetHeader() const
{
    return bHasHeader;
}

void ScPivot::SetIgnoreEmpty(BOOL bIgnore)
{
    bIgnoreEmpty = bIgnore;
    bValidArea   = FALSE;
}

BOOL ScPivot::GetIgnoreEmpty() const
{
    return bIgnoreEmpty;
}

void ScPivot::SetDetectCat(BOOL bDetect)
{
    bDetectCat = bDetect;
    bValidArea = FALSE;
}

BOOL ScPivot::GetDetectCat() const
{
    return bDetectCat;
}

void ScPivot::SetMakeTotalCol(BOOL bSet)
{
    bMakeTotalCol = bSet;
    bValidArea    = FALSE;
}

BOOL ScPivot::GetMakeTotalCol() const
{
    return bMakeTotalCol;
}

void ScPivot::SetMakeTotalRow(BOOL bSet)
{
    bMakeTotalRow = bSet;
    bValidArea    = FALSE;
}

BOOL ScPivot::GetMakeTotalRow() const
{
    return bMakeTotalRow;
}

void ScPivot::SetName(const String& rNew)
{
    aName = rNew;
}

const String& ScPivot::GetName() const
{
    return aName;
}

void ScPivot::SetTag(const String& rNew)
{
    aTag = rNew;
}

const String& ScPivot::GetTag() const
{
    return aTag;
}

void ScPivot::SetSrcArea(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, SCTAB nTab)
{
    nSrcCol1 = Min(nCol1, (SCCOL)MAXCOL);
    nSrcRow1 = Min(nRow1, (SCROW)MAXROW);
    nSrcCol2 = Min(nCol2, (SCCOL)MAXCOL);
    nSrcRow2 = Min(nRow2, (SCROW)MAXROW);
    nSrcTab = nTab;
    bValidArea = FALSE;
}

void ScPivot::GetSrcArea(SCCOL& rCol1, SCROW& rRow1, SCCOL& rCol2, SCROW& rRow2, SCTAB& rTab) const
{
    rCol1 = nSrcCol1;
    rRow1 = nSrcRow1;
    rCol2 = nSrcCol2;
    rRow2 = nSrcRow2;
    rTab = nSrcTab;
}

ScRange ScPivot::GetSrcArea() const
{
    return ScRange( nSrcCol1,nSrcRow1,nSrcTab, nSrcCol2,nSrcRow2,nSrcTab );
}

void ScPivot::SetDestPos(SCCOL nCol, SCROW nRow, SCTAB nTab)
{
    nDestCol1 = nCol;
    nDestRow1 = nRow;
    nDestTab = nTab;
    bValidArea = FALSE;
}

void ScPivot::GetDestArea(SCCOL& rCol1, SCROW& rRow1, SCCOL& rCol2, SCROW& rRow2, SCTAB& rTab) const
{
    rCol1 = nDestCol1;
    rRow1 = nDestRow1;
    rTab = nDestTab;
    if (bValidArea)
    {
        rCol2 = nDestCol2;
        rRow2 = nDestRow2;
    }
    else
    {
        rCol2 = nDestCol1;
        rRow2 = nDestRow1;
    }
}

ScRange ScPivot::GetDestArea() const
{
    ScAddress aStart( nDestCol1, nDestRow1, nDestTab );
    ScAddress aEnd = aStart;
    if ( bValidArea )
        aEnd = ScAddress( nDestCol2, nDestRow2, nDestTab );
    return ScRange( aStart, aEnd );
}

void ScPivot::MoveSrcArea( SCCOL nNewCol, SCROW nNewRow, SCTAB nNewTab )
{
    if ( nNewCol != nSrcCol1 || nNewRow != nSrcRow1 || nNewTab != nSrcTab )
    {
        SCsCOL nDiffX = nNewCol - (SCsCOL) nSrcCol1;
        SCsROW nDiffY = nNewRow - (SCsROW) nSrcRow1;

        nSrcTab = nNewTab;
        nSrcCol1 = sal::static_int_cast<SCCOL>( nSrcCol1 + nDiffX );
        nSrcCol2 = sal::static_int_cast<SCCOL>( nSrcCol2 + nDiffX );
        nSrcRow1 = sal::static_int_cast<SCROW>( nSrcRow1 + nDiffY );
        nSrcRow2 = sal::static_int_cast<SCROW>( nSrcRow2 + nDiffY );

        aQuery.nCol1 = sal::static_int_cast<SCCOL>( aQuery.nCol1 + nDiffX );
        aQuery.nCol2 = sal::static_int_cast<SCCOL>( aQuery.nCol2 + nDiffX );
        aQuery.nRow1 = sal::static_int_cast<SCROW>( aQuery.nRow1 + nDiffY );
        aQuery.nRow2 = sal::static_int_cast<SCROW>( aQuery.nRow2 + nDiffY );

        SCSIZE nEC = aQuery.GetEntryCount();
        for (SCSIZE i=0; i<nEC; i++)
            if (aQuery.GetEntry(i).bDoQuery)
                aQuery.GetEntry(i).nField += nDiffX;

        if (bValidArea)
        {
            SCSIZE nC;
            SCSIZE nR;
            for (nC=0; nC<nColCount; nC++)
                if (aColArr[nC].nCol != PIVOT_DATA_FIELD)
                    aColArr[nC].nCol = sal::static_int_cast<SCsCOL>( aColArr[nC].nCol + nDiffX );
            for (nR=0; nR<nRowCount; nR++)
                if (aRowArr[nR].nCol != PIVOT_DATA_FIELD)
                    aRowArr[nR].nCol = sal::static_int_cast<SCsCOL>( aRowArr[nR].nCol + nDiffX );
            for (nC=0; nC<nDataCount; nC++)
                if (aDataArr[nC].nCol != PIVOT_DATA_FIELD)
                    aDataArr[nC].nCol = sal::static_int_cast<SCsCOL>( aDataArr[nC].nCol + nDiffX );
        }
    }
}

void ScPivot::ExtendSrcArea( SCCOL nNewEndCol, SCROW nNewEndRow )
{
    DBG_ASSERT( nNewEndCol >= nSrcCol2 && nNewEndRow >= nSrcRow2, "ExtendSrcArea: zu klein" );

    nSrcCol2 = nNewEndCol;
    nSrcRow2 = nNewEndRow;

    //  alles andere bleibt erhalten
}

void ScPivot::MoveDestArea( SCCOL nNewCol, SCROW nNewRow, SCTAB nNewTab )
{
    if ( nNewCol != nDestCol1 || nNewRow != nDestRow1 || nNewTab != nDestTab )
    {
        SCsCOL nDiffX = nNewCol - (SCsCOL) nDestCol1;
        SCsROW nDiffY = nNewRow - (SCsROW) nDestRow1;

        nDestTab = nNewTab;
        nDestCol1 = sal::static_int_cast<SCCOL>( nDestCol1 + nDiffX );
        nDestRow1 = sal::static_int_cast<SCROW>( nDestRow1 + nDiffY );

        if (bValidArea)
        {
            nDestCol2 = sal::static_int_cast<SCCOL>( nDestCol2 + nDiffX );
            nDestRow2 = sal::static_int_cast<SCROW>( nDestRow2 + nDiffY );

            nDataStartCol = sal::static_int_cast<SCCOL>( nDataStartCol + nDiffX );
            nDataStartRow = sal::static_int_cast<SCROW>( nDataStartRow + nDiffY );
        }
    }
}

void ScPivot::SetColFields(const PivotField* pFieldArr, SCSIZE nCount)
{
    nColCount = Max(static_cast<SCSIZE>(0), Min(nCount, PIVOT_MAXFIELD));
    for (SCSIZE i = 0; i < nColCount; i++)
    {
        aColArr[i] = pFieldArr[i];
        aColArr[i].nFuncCount = 0;
        if (aColArr[i].nCol == PIVOT_DATA_FIELD)
        {
            aColArr[i].nFuncMask = PIVOT_FUNC_NONE;
            pDataList = pColList[i];
            bDataAtCol = TRUE;
        }
        else
        {
            for (SCsCOL j=0; j<=PIVOT_MAXFUNC; j++)             // incl. "auto"
                if (aColArr[i].nFuncMask & nFuncMaskArr[j])
                    aColArr[i].nFuncCount++;
        }
    }
    bValidArea = FALSE;
}

void ScPivot::GetColFields(PivotField* pFieldArr, SCSIZE& rCount) const
{
    for (SCSIZE i=0; i<nColCount; i++)
        pFieldArr[i] = aColArr[i];
    rCount = nColCount;
}

void ScPivot::SetRowFields(const PivotField* pFieldArr, SCSIZE nCount)
{
    nRowCount = Max(static_cast<SCSIZE>(0), Min(nCount, PIVOT_MAXFIELD));
    for (SCSIZE i = 0; i < nRowCount; i++)
    {
        aRowArr[i] = pFieldArr[i];
        aRowArr[i].nFuncCount = 0;
        if (aRowArr[i].nCol == PIVOT_DATA_FIELD)
        {
            aRowArr[i].nFuncMask = PIVOT_FUNC_NONE;
            pDataList = pRowList[i];
            bDataAtCol = FALSE;
        }
        else
        {
            for (SCSIZE j=0; j<=PIVOT_MAXFUNC; j++)             // incl. "auto"
                if (aRowArr[i].nFuncMask & nFuncMaskArr[j])
                    aRowArr[i].nFuncCount++;
        }
    }
    bValidArea = FALSE;
}

void ScPivot::GetRowFields(PivotField* pFieldArr, SCSIZE& rCount) const
{
    for (SCSIZE i=0; i<nRowCount; i++)
        pFieldArr[i] = aRowArr[i];
    rCount = nRowCount;
}

void ScPivot::SetDataFields(const PivotField* pFieldArr, SCSIZE nCount)
{
    USHORT nFuncNo;
    SCSIZE i;

    //
    //      nDataCount vorausberechnen (wie unten)
    //

    nDataCount = 0;
    for (i = 0; i < nCount; i++)
        for (nFuncNo=0; nFuncNo<PIVOT_MAXFUNC; nFuncNo++)
            if (pFieldArr[i].nFuncMask & nFuncMaskArr[nFuncNo])
                if (nDataCount+1 < PIVOT_MAXFIELD)
                    ++nDataCount;

    //
    //      Eintraege anpassen
    //

    if ((nRowCount == 1) && (aRowArr[0].nCol == PIVOT_DATA_FIELD) && (nDataCount == 1))
    {
        aColArr[nColCount] = aRowArr[0];
        pDataList = pColList[nColCount];
        nColCount++;
        nRowCount--;
        bDataAtCol = TRUE;
    }
    if ((nColCount == 1) && (aColArr[0].nCol == PIVOT_DATA_FIELD) && (nDataCount == 1))
    {
        aRowArr[nRowCount] = aColArr[0];
        pDataList = pRowList[nRowCount];
        nRowCount++;
        nColCount--;
        bDataAtCol = FALSE;
    }

    if ((nDataCount == 1)
        && (aColArr[nColCount-1].nCol != PIVOT_DATA_FIELD)
        && (aColArr[nRowCount-1].nCol != PIVOT_DATA_FIELD))
    {
        if (bDataAtCol)
        {
            PivotField aField;
            SCSIZE nIndex = PIVOT_MAXFIELD;
            for (i=0; i<nColCount; i++)
            {
                if (aColArr[i].nCol == PIVOT_DATA_FIELD)
                {
                    aField = aColArr[i];
                    nIndex = i;
                }
            }
            DBG_ASSERT(nIndex < PIVOT_MAXFIELD, "no data field (GPF in old versions!)");
            if ( nIndex < PIVOT_MAXFIELD )
            {
                memcpy(&aColArr[nIndex], &aColArr[nIndex+1], (PIVOT_MAXFIELD - nIndex - 1) * sizeof(PivotField));
                aColArr[nColCount-1] = aField;
                pDataList = pColList[nColCount-1];
            }
        }
        else
        {
            PivotField aField;
            SCSIZE nIndex = PIVOT_MAXFIELD;
            for (i=0; i<nRowCount; i++)
            {
                if (aRowArr[i].nCol == PIVOT_DATA_FIELD)
                {
                    aField = aRowArr[i];
                    nIndex = i;
                }
            }
            DBG_ASSERT(nIndex < PIVOT_MAXFIELD, "no data field (GPF in old versions!)");
            if ( nIndex < PIVOT_MAXFIELD )
            {
                memcpy(&aRowArr[nIndex], &aRowArr[nIndex+1], (PIVOT_MAXFIELD - nIndex - 1) * sizeof(PivotField));
                aRowArr[nRowCount-1] = aField;
                pDataList = pRowList[nRowCount-1];
            }
        }
    }

    //
    //      Datenfelder in Eintraege mit nur einer Funktion aufteilen
    //

    pDataList->FreeAll();
    nDataCount = 0;
    for (i = 0; i < nCount; i++)
    {
        for (nFuncNo=0; nFuncNo<PIVOT_MAXFUNC; nFuncNo++)
            if (pFieldArr[i].nFuncMask & nFuncMaskArr[nFuncNo])
                if (nDataCount+1 < PIVOT_MAXFIELD)
                {
                    aDataArr[nDataCount] = pFieldArr[i];
                    aDataArr[nDataCount].nFuncCount = 0;
                    aDataArr[nDataCount].nFuncMask  = nFuncMaskArr[nFuncNo];

                    String aStr;
                    pDoc->GetString(aDataArr[nDataCount].nCol, nSrcRow1, nSrcTab, aStr);
                    if (aStr.Len() == 0)
                        aStr = ColToAlpha( aDataArr[nDataCount].nCol );
                    TypedStrData* pStrData = new TypedStrData(aStr);
                    if (!(pDataList->AtInsert(pDataList->GetCount(), pStrData)))
                    {
                        delete pStrData;
                        DBG_ERROR("Fehler bei pDataList->AtInsert");
                    }

                    ++nDataCount;
                }
    }

    //
    //
    //

    bValidArea = FALSE;
}

void ScPivot::GetDataFields(PivotField* pFieldArr, SCSIZE& rCount) const
{
/*  for (SCSIZE i=0; i<nDataCount; i++)
        pFieldArr[i] = aDataArr[i];
    rCount = nDataCount;
*/

    rCount = 0;
    for (SCSIZE i=0; i<nDataCount; i++)
    {
        BOOL bFound = FALSE;
        for (SCSIZE j=0; j<rCount && !bFound; j++)
            if (pFieldArr[j].nCol == aDataArr[i].nCol)
            {
                //  add to previous column only if new bits aren't already set there
                if ( ( pFieldArr[j].nFuncMask & aDataArr[i].nFuncMask ) == 0 )
                {
                    pFieldArr[j].nFuncMask |= aDataArr[i].nFuncMask;
                    pFieldArr[j].nFuncCount++;
                    bFound = TRUE;
                }
            }
        if (!bFound)
        {
            pFieldArr[rCount] = aDataArr[i];
            ++rCount;
        }
    }
}

BOOL ScPivot::CreateData(BOOL bKeepDest)
{
    //
    //
    //

    SCCOL nOldCol2 = nDestCol2;
    SCROW nOldRow2 = nDestRow2;

    pColRef = new PivotColRef[MAXCOL];
    aQuery.nCol1 = nSrcCol1;
    aQuery.nRow1 = nSrcRow1;
    aQuery.nCol2 = nSrcCol2;
    aQuery.nRow2 = nSrcRow2;
    aQuery.bHasHeader = bHasHeader;
    BOOL bRet = CreateFields();
    if (bRet)
    {
        SCSIZE i=0;                 // nDataMult berechnen - nach CreateFields, vor CreateFieldData !!!
        nDataMult = 1;
        if (nDataCount > 1)
        {
            if (bDataAtCol)
            {
                while (i<nColCount && aColArr[i].nCol != PIVOT_DATA_FIELD) i++;
                i++;
                while (i<nColCount)
                    nDataMult *= pColList[i++]->GetCount();
            }
            else
            {
                while (i<nRowCount && aRowArr[i].nCol != PIVOT_DATA_FIELD) i++;
                i++;
                while (i<nRowCount)
                    nDataMult *= pRowList[i++]->GetCount();
            }
        }
        DBG_ASSERT(nDataMult,"nDataMult==0");

        CalcArea();
        if ((ValidCol(nDestCol2)) && (ValidRow(nDestRow2)))
        {
            CreateFieldData();
            bValidArea = TRUE;
        }
        else
            bRet = FALSE;
    }

    if ( bKeepDest )
    {
        bValidArea = TRUE;          //! ???
        nDestCol2 = nOldCol2;
        nDestRow2 = nOldRow2;
    }

    return bRet;
}

void ScPivot::DrawData()
{
    ScProgress aProgress( pDoc->GetDocumentShell(), ScGlobal::GetRscString(STR_PIVOT_PROGRESS), nDestRow2-nDestRow1 );

    SCSIZE i;

    SCCOL nCol;
    SCROW nRow;
    String aStr;
    pDoc->pTab[nDestTab]->DeleteArea(nDestCol1, nDestRow1, nDestCol2, nDestRow2, IDF_ALL);

    if ( nDataStartRow > nDestRow1+nFirstLine )
        SetStyle(nDestCol1, nDestRow1+nFirstLine, nDestCol2, nDataStartRow-1, PIVOT_STYLE_TOP);
    SetStyle(nDataStartCol, nDataStartRow, nDestCol2, nDestRow2, PIVOT_STYLE_INNER);

    pDoc->SetString(nDestCol1, nDestRow1, nDestTab, ScGlobal::GetRscString(STR_CELL_FILTER));
    //  Kategorie 1
    SetButton(nDestCol1, nDestRow1, nDestCol1, nDestRow1);

    if (bHasHeader)                     // Spalten / Zeilennamen ausgeben
    {
        if (nColCount != 0)
        {
            nCol = nDestCol1;
            nRow = nDataStartRow - 1;
            for (i=0; i<nColCount; i++)
            {
                if (aColArr[i].nCol != PIVOT_DATA_FIELD)
                {
                    pDoc->GetString(aColArr[i].nCol, nSrcRow1, nSrcTab, aStr);
                    if ( !aStr.Len() )
                        aStr = ColToAlpha( aColArr[i].nCol );
                    pDoc->SetString(nCol, nRow, nDestTab, aStr);
                    //  Kategorie 2
                    nCol++;
                }
                else if (nDataCount > 1)
                {
                    pDoc->SetString(nCol, nRow, nDestTab, *pLabelData);
                    //  Kategorie 3
                    nCol++;
                }
            }
            SetButton(nDestCol1, nRow, nCol-1, nRow);
            SetStyle(nDestCol1, nRow, nCol-1, nRow, PIVOT_STYLE_FIELDNAME);
        }
        if (nRowCount != 0)
        {
            nCol = nDataStartCol;
            nRow = nDestRow1 + nFirstLine;
            for (i=0; i<nRowCount; i++)
            {
                if (aRowArr[i].nCol != PIVOT_DATA_FIELD)
                {
                    pDoc->GetString(aRowArr[i].nCol, nSrcRow1, nSrcTab, aStr);
                    if ( !aStr.Len() )
                        aStr = ColToAlpha( aRowArr[i].nCol );
                    pDoc->SetString(nCol, nRow, nDestTab, aStr);
                    //  Kategorie 4
                    nCol++;
                }
                else if (nDataCount > 1)
                {
                    pDoc->SetString(nCol, nRow, nDestTab, *pLabelData);
                    //  Kategorie 5
                    nCol++;
                }
            }
            SetButton(nDataStartCol, nRow, nCol-1, nRow);
            SetStyle(nDataStartCol, nRow, nCol-1, nRow, PIVOT_STYLE_FIELDNAME);
        }
    }

    BOOL bNoRows = (nRowCount == 0) || ( nRowCount == 1 && aRowArr[0].nCol == PIVOT_DATA_FIELD );
    BOOL bNoCols = (nColCount == 0) || ( nColCount == 1 && aColArr[0].nCol == PIVOT_DATA_FIELD );
    if (!bMakeTotalCol) bNoRows = TRUE;
    if (!bMakeTotalRow) bNoCols = TRUE;

    SCCOL nTotalCol = nDestCol2;
    SCROW nTotalRow = nDestRow2;
    if (bDataAtCol)
        nTotalRow = sal::static_int_cast<SCROW>( nTotalRow - ( nDataCount - 1 ) );
    else
        nTotalCol = sal::static_int_cast<SCCOL>( nTotalCol - ( nDataCount - 1 ) );

                            // Spaltenkoepfe ausgeben und ColRef initialisieren
                            // (String-Collections sind initialisiert)
    nDataIndex = 0;
    nColIndex = 0;
    nCol = nDataStartCol;
    nRecCount = 0;
    RowToTable(0, nCol);

                            // Zeilenkoepfe und Daten ausgeben
                            // (ruft SetDataLine/SetFuncLine auf)
    nRowIndex = 0;
    nRow = nDataStartRow;
    ColToTable(0, nRow, aProgress);

                            // Gesamtergebnis-Zeilen

    if (!bNoCols)
    {
        if (bDataAtCol)
            for (SCSIZE nTotCnt = 0; nTotCnt<nDataCount; nTotCnt++)
                SetFuncLine(nDataStartCol, nRow+nTotCnt, nDestTab,
                            aDataArr[nTotCnt].nFuncMask, nTotCnt, 0, nDataRowCount);
        else
            SetFuncLine(nDataStartCol, nRow, nDestTab, PIVOT_FUNC_AUTO, SCSIZE_MAX, 0, nDataRowCount);
    }


    // Rahmen Spaltenergebnis

    if (!bNoRows)
    {
        if (!bDataAtCol)
        {
            for (i=0; i<nDataCount; i++)
            {
                String aLab = *pLabelTotal;
                aLab += ' ';
                aLab += *pLabel[lcl_MaskToIndex( aDataArr[i].nFuncMask )];
                aLab += ' ';
                aLab += pDataList->GetString(sal::static_int_cast<USHORT>(i));
                pDoc->SetString(sal::static_int_cast<SCCOL>(nTotalCol+i),
                                sal::static_int_cast<SCROW>(nDestRow1 + nFirstLine), nDestTab, aLab);
                //  Kategorie 6
            }
        }
        else
        {
            pDoc->SetString(nTotalCol, nDestRow1 + nFirstLine, nDestTab, *pLabelTotal);
            //  Kategorie 7
        }

        if ( nDataStartRow > 0 )
            SetStyle(nTotalCol, nDestRow1+nFirstLine, nDestCol2, nDataStartRow-1, PIVOT_STYLE_TITLE);
        SetStyle(nTotalCol, nDataStartRow, nDestCol2, nDestRow2, PIVOT_STYLE_RESULT);
        SetFrame(nTotalCol, nDestRow1 + nFirstLine, nDestCol2, nDestRow2);
    }

    // Rahmen Zeilenergebnis

    if (!bNoCols)
    {
        if (bDataAtCol)
        {
            for (i=0; i<nDataCount; i++)
            {
                String aLab = *pLabelTotal;
                aLab += ' ';
                aLab += *pLabel[lcl_MaskToIndex( aDataArr[i].nFuncMask )];
                aLab += ' ';
                aLab += pDataList->GetString(sal::static_int_cast<USHORT>(i));
                pDoc->SetString(nDestCol1, nTotalRow+i, nDestTab, aLab);
                //  Kategorie 8
            }
        }
        else
        {
            pDoc->SetString(nDestCol1, nTotalRow, nDestTab, *pLabelTotal);
            //  Kategorie 9
        }

        if ( nDataStartCol > 0 )
            SetStyle(nDestCol1, nTotalRow, nDataStartCol-1, nDestRow2, PIVOT_STYLE_TITLE);
        SetStyle(nDataStartCol, nTotalRow, nDestCol2, nDestRow2, PIVOT_STYLE_RESULT);
        SetFrame(nDestCol1, nTotalRow, nDestCol2, nDestRow2);
    }

    // Rahmen gesamt
    SetFrame(nDestCol1, nDestRow1 + nFirstLine, nDestCol2, nDestRow2, 40);
}

void ScPivot::ReleaseData()
{
    for (SCSIZE i = 0; i < PIVOT_MAXFIELD; i++)
    {
        pColList[i]->FreeAll();
        pRowList[i]->FreeAll();
    }
    if (ppDataArr)
    {
        for (SCSIZE i=0; i<nDataRowCount; i++)
            delete[] ppDataArr[i];
        delete[] ppDataArr;
        ppDataArr = NULL;
    }
    nDataColCount = 0;
    nDataRowCount = 0;
    delete[] pColRef;
    pColRef = NULL;
}

BOOL ScPivot::IsPivotAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab) const
{
    if (bValidArea)
        return ( nTab == nDestTab
                    && nCol >= nDestCol1 && nCol <= nDestCol2
                    && nRow >= nDestRow1 && nRow <= nDestRow2 );
    else
        return FALSE;
}

BOOL ScPivot::IsFilterAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab) const
{
    if (bValidArea)
        return (nCol == nDestCol1 && nRow == nDestRow1 && nTab == nDestTab);
    else
        return FALSE;
}

BOOL ScPivot::GetColFieldAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, SCCOL& rField) const
{
    rField = 0;
    BOOL bRet = FALSE;
    if (bValidArea)
    {
        bRet = ( nCol >= nDestCol1 && nCol < nDataStartCol
                && nRow == nDataStartRow - 1
                && nTab == nDestTab );
        if (bRet)
        {
            rField = aColArr[nCol - nDestCol1].nCol;
            if (rField == PIVOT_DATA_FIELD)
                bRet = (nDataCount > 1);
        }
    }
    return bRet;
}

BOOL ScPivot::GetRowFieldAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, SCCOL& rField) const
{
    rField = 0;
    BOOL bRet = FALSE;
    if (bValidArea)
    {
        bRet = ( nCol >= nDataStartCol && nCol < sal::static_int_cast<SCCOL>(nDataStartCol + nRowCount)
                && nRow == nDestRow1 + nFirstLine
                && nTab == nDestTab );
        if (bRet)
        {
            rField = aRowArr[nCol - nDataStartCol].nCol;
            if (rField == PIVOT_DATA_FIELD)
                bRet = (nDataCount > 1);
        }
    }
    return bRet;
}


//--------------------------------------------------------------------------------------------------
// Private Methoden
//--------------------------------------------------------------------------------------------------

BOOL ScPivot::CreateFields()
{
    SCSIZE  i;
    SCROW   nRow;
    SCROW   nHeader;
    String  aStr;
    TypedStrData* pStrData;
    if (bHasHeader)
        nHeader = 1;
    else
        nHeader = 0;

    // Sortieren nach Benutzerdefinierte Listen ??
    for (i = 0; i < nColCount; i++)
    {
        if (aColArr[i].nCol != PIVOT_DATA_FIELD)
        {
            pDoc->GetString(aColArr[i].nCol, nSrcRow1 + nHeader, nSrcTab, aStr);
            pColList[i]->SetUserData(ScGlobal::GetUserList()->GetData(aStr));
        }
        else
            pColList[i]->SetUserData(NULL);
    }
    for (i = 0; i < nRowCount; i++)
    {
        if (aRowArr[i].nCol != PIVOT_DATA_FIELD)
        {
            pDoc->GetString(aRowArr[i].nCol, nSrcRow1 + nHeader, nSrcTab, aStr);
            pRowList[i]->SetUserData(ScGlobal::GetUserList()->GetData(aStr));
        }
        else
            pRowList[i]->SetUserData(NULL);
    }

    ScAddress aSrcAdr( nSrcCol1, 0, nSrcTab );
    for (nRow = nSrcRow1 + nHeader; nRow <= nSrcRow2; nRow++)
    {
        BOOL bValidLine = TRUE;
        if (bIgnoreEmpty)
        {
            aSrcAdr.SetRow( nRow );
            bValidLine = !lcl_IsEmptyLine( pDoc, aSrcAdr, nSrcCol2 );
        }
        if (bValidLine)
            bValidLine = pDoc->pTab[nSrcTab]->ValidQuery(nRow, aQuery);
        if (bValidLine)
        {
            //  Sortierte Liste der Felder erzeugen
            //! statt GetCategoryString leere weglassen !

            for (i = 0; i < nColCount; i++)
            {
                if (aColArr[i].nCol != PIVOT_DATA_FIELD)
                {
                    SCROW nCatRow = bDetectCat ? GetCategoryRow( aColArr[i].nCol, nRow ) : nRow;
                    pStrData = new TypedStrData( pDoc, aColArr[i].nCol, nCatRow, nSrcTab, TRUE );
                    if (!(pColList[i]->Insert(pStrData)))
                        delete pStrData;
                }
            }
            for (i = 0; i < nRowCount; i++)
            {
                if (aRowArr[i].nCol != PIVOT_DATA_FIELD)
                {
                    SCROW nCatRow = bDetectCat ? GetCategoryRow( aRowArr[i].nCol, nRow ) : nRow;
                    pStrData = new TypedStrData( pDoc, aRowArr[i].nCol, nCatRow, nSrcTab, TRUE );
                    if (!(pRowList[i]->Insert(pStrData)))
                        delete pStrData;
                }
            }
        }
    }
    return TRUE;
}

void ScPivot::CreateFieldData()
{
    SCSIZE* pRowListIndex = nRowCount ? new SCSIZE[nRowCount] : NULL;
    SCSIZE* pColListIndex = nColCount ? new SCSIZE[nColCount] : NULL;

    SCSIZE i,j,k;

    ppDataArr = new SubTotal*[nDataRowCount];
    for (i=0; i<nDataRowCount; i++)
        ppDataArr[i] = new SubTotal[nDataColCount];

    if (bDataAtCol)
        for (j=0; j<nDataRowCount; j++)
            for (i=0; i<nDataColCount; i++)
                ppDataArr[j][i].nIndex = j/nDataMult%nDataCount;
    else
        for (j=0; j<nDataRowCount; j++)
            for (i=0; i<nDataColCount; i++)
                ppDataArr[j][i].nIndex = i/nDataMult%nDataCount;

    SCROW nHeader;
    if (bHasHeader)
        nHeader = 1;
    else
        nHeader = 0;
    ScAddress aSrcAdr( nSrcCol1, 0, nSrcTab );
    for (SCROW nRow = nSrcRow1 + nHeader; nRow <= nSrcRow2; nRow++)
    {
        BOOL bValidLine = TRUE;
        if (bIgnoreEmpty)
        {
            aSrcAdr.SetRow( nRow );
            bValidLine = !lcl_IsEmptyLine( pDoc, aSrcAdr, nSrcCol2 );
        }
        if (bValidLine)
            bValidLine = pDoc->pTab[nSrcTab]->ValidQuery(nRow, aQuery);
        if (bValidLine)
        {
            //      Indizes der Kategorien nur einmal ausserhalb nDataCount
            for (j=0; j<nRowCount; j++)
                if (aRowArr[j].nCol != PIVOT_DATA_FIELD)
                {
                    SCROW nCatRow = bDetectCat ? GetCategoryRow( aRowArr[j].nCol, nRow ) : nRow;
                    TypedStrData aStrData( pDoc, aRowArr[j].nCol, nCatRow, nSrcTab, TRUE );
                    pRowListIndex[j] = pRowList[j]->GetIndex(&aStrData);
                }
            for (j=0; j<nColCount; j++)
                if (aColArr[j].nCol != PIVOT_DATA_FIELD)
                {
                    SCROW nCatRow = bDetectCat ? GetCategoryRow( aColArr[j].nCol, nRow ) : nRow;
                    TypedStrData aStrData( pDoc, aColArr[j].nCol, nCatRow, nSrcTab, TRUE );
                        pColListIndex[j] = pColList[j]->GetIndex(&aStrData);
                }

            String aStr;
            SCSIZE nCIndex;
            SCSIZE nRIndex;
            SCSIZE nIndex;
            ScAddress aAdr( 0, nRow, nSrcTab );

            for (i=0; i<nDataCount; i++)
            {
                // ColIndex Berechnen
                nCIndex = 0;
                for (j=0; j<nRowCount; j++)
                {
                    if (aRowArr[j].nCol == PIVOT_DATA_FIELD)
                        nIndex = i;
                    else
                        nIndex = pRowListIndex[j];
                    if (nIndex)
                    {
                        for (k=j+1; k<nRowCount; k++)
                            nIndex *= pRowList[k]->GetCount();
                        nCIndex += nIndex;
                    }
                }
                // RowIndex Berechnen
                nRIndex = 0;
                for (j=0; j<nColCount; j++)
                {
                    if (aColArr[j].nCol == PIVOT_DATA_FIELD)
                        nIndex = i;
                    else
                        nIndex = pColListIndex[j];
                    if (nIndex)
                    {
                        for (k=j+1; k<nColCount; k++)
                            nIndex *= pColList[k]->GetCount();
                        nRIndex += nIndex;
                    }
                }
                //  Daten eintragen
                if ((nCIndex < nDataColCount) && (nRIndex < nDataRowCount))
                {
                    DBG_ASSERT(ppDataArr[nRIndex][nCIndex].nIndex == i, "falsch init.")

                    ppDataArr[nRIndex][nCIndex].nIndex = i;
                    aAdr.SetCol( aDataArr[i].nCol );
                    CellType eCellType = pDoc->GetCellType( aAdr );
                    if ((eCellType != CELLTYPE_NONE) && (eCellType != CELLTYPE_NOTE))
                    {
                        BOOL bValue = (eCellType == CELLTYPE_VALUE);
                        if (eCellType == CELLTYPE_FORMULA)
                        {
                            ScBaseCell* pCell = pDoc->GetCell( aAdr );
                            bValue = ((ScFormulaCell*)pCell)->IsValue();
                        }

                        if (bValue)
                        {
                            double nVal = pDoc->GetValue( aAdr );
                            ppDataArr[nRIndex][nCIndex].Update(nVal);
                        }
                        else
                            ppDataArr[nRIndex][nCIndex].UpdateNoVal();      // nur nCount
                    }
                }
            }
        }
    }

    delete pColListIndex;
    delete pRowListIndex;
}

void ScPivot::CalcArea()
{
    BOOL bNoRows = (nRowCount == 0) || ( nRowCount == 1 && aRowArr[0].nCol == PIVOT_DATA_FIELD );
    BOOL bNoCols = (nColCount == 0) || ( nColCount == 1 && aColArr[0].nCol == PIVOT_DATA_FIELD );
    if (!bMakeTotalCol) bNoRows = TRUE;
    if (!bMakeTotalRow) bNoCols = TRUE;

    // StartSpalte/StartZeile des Datenbereichs berechnen
    if (bDataAtCol)
    {
        if (nDataCount > 1)
            nDataStartCol = sal::static_int_cast<SCCOL>(nDestCol1 + nColCount);
        else
            nDataStartCol = sal::static_int_cast<SCCOL>(nDestCol1 + Max(static_cast<SCSIZE>(0), nColCount - 1));
    }
    else
        nDataStartCol = sal::static_int_cast<SCCOL>(nDestCol1 + nColCount);
    if (!bDataAtCol)
    {
        if (nDataCount > 1)
            nDataStartRow = nDestRow1 + nRowCount + nFirstLine + 1;
        else
            nDataStartRow = nDestRow1 + Max(static_cast<SCSIZE>(0), nRowCount - 1) + nFirstLine + 1;
    }
    else
        nDataStartRow = nDestRow1 + nRowCount + nFirstLine + 1;

    //
    //      Groesse der PivotTabelle berechnen
    //

    if (nRowCount == 0 || (nRowCount==1 && aRowArr[0].nCol==PIVOT_DATA_FIELD && nDataCount==1))
    {
        nDataColCount = 1;
        if (nDataCount == 1)
            nDestCol2 = sal::static_int_cast<SCCOL>(nDestCol1 + nColCount - 1);
        else
            nDestCol2 = sal::static_int_cast<SCCOL>(nDestCol1 + nColCount);
    }
    else
    {
        SCSIZE nDx;
        // Anzahl Spalten
        if ((aRowArr[nRowCount-1].nCol == PIVOT_DATA_FIELD) && (nDataCount == 1))
            nDx = 2;
        else
            nDx = 1;
        SCSIZE nColLines = pRowList[nRowCount-nDx]->GetCount(); // SCSIZE to recognize overflow
        nDataColCount = pRowList[nRowCount-nDx]->GetCount();
        for (SCSIZE i=nRowCount-nDx; i-- > 0; )
        {
            nColLines *= pRowList[i]->GetCount();
            nDataColCount *= pRowList[i]->GetCount();
            if (!bDataAtCol)
                nColLines += (pRowList[i]->GetCount() * aRowArr[i].nFuncCount * nDataCount);
            else
                nColLines += (pRowList[i]->GetCount() * aRowArr[i].nFuncCount);
        }
        /*
        // Ergebnisspalten des letzten Elements
        if (aRowArr[nRowCount-1].nCol != PIVOT_DATA_FIELD)
            nColLines += (pRowList[nRowCount-1]->GetCount() * aRowArr[nRowCount-1].nFuncCount);
        */
        if (nColLines > static_cast<SCSIZE>(MAXCOL))
            nDestCol2 = MAXCOL+2;   // ungueltig, 1 wird unten abgezogen
        else if (bDataAtCol)
        {
            if (nDataCount > 1)
                nDestCol2 = sal::static_int_cast<SCCOL>(nDestCol1 + nColCount + nColLines);
            else
                nDestCol2 = sal::static_int_cast<SCCOL>(nDestCol1 + (nColCount - 1) + nColLines);
            if (!bMakeTotalCol)
                --nDestCol2;
        }
        else
            nDestCol2 = sal::static_int_cast<SCCOL>(nDestCol1 + nColCount + nColLines);
    }

    if (nColCount == 0 || (nColCount==1 && aColArr[0].nCol==PIVOT_DATA_FIELD && nDataCount==1))
    {
        nDataRowCount = 1;
        if (nDataCount == 1)
            nDestRow2 = nDestRow1 + (nRowCount - 1) + nFirstLine + 1;
        else
            nDestRow2 = nDestRow1 + nRowCount + nFirstLine + 1;
    }
    else
    {
        SCSIZE nDx;
        // Anzahl Zeilen
        if ((aColArr[nColCount-1].nCol == PIVOT_DATA_FIELD) && (nDataCount == 1))
            nDx = 2;
        else
            nDx = 1;
        SCSIZE nRowLines = pColList[nColCount-nDx]->GetCount(); // SCSIZE to recognize overflow
        nDataRowCount = pColList[nColCount-nDx]->GetCount();
        for (SCSIZE i=nColCount-nDx; i-- > 0; )
        {
            nRowLines *= pColList[i]->GetCount();
            nDataRowCount *= pColList[i]->GetCount();
            if (bDataAtCol)
                nRowLines += (pColList[i]->GetCount() * aColArr[i].nFuncCount * nDataCount);
            else
                nRowLines += (pColList[i]->GetCount() * aColArr[i].nFuncCount);
        }
        /*
        // Ergebniszeilen des letzten Elements
        if (aColArr[nColCount-1].nCol != PIVOT_DATA_FIELD)
            nRowLines += (pColList[nColCount-1]->GetCount() * aColArr[nColCount-1].nFuncCount);
        */
        if (nRowLines > static_cast<SCSIZE>(MAXROW))
            nDestRow2 = MAXROW+2;   // ungueltig, 1 wird unten abgezogen
        else if (!bDataAtCol)
        {
            if (nDataCount > 1)
                nDestRow2 = nDestRow1 + nRowCount + nRowLines + nFirstLine + 1;
            else
                nDestRow2 = nDestRow1 + (nRowCount - 1) + nRowLines + nFirstLine + 1;
            if (!bMakeTotalRow)
                --nDestRow2;
        }
        else
            nDestRow2 = nDestRow1 + nRowCount + nRowLines + nFirstLine + 1;
    }

    if (bDataAtCol)
    {
        if (!bNoCols)
            nDestRow2 += nDataCount;
        nDestRow2 --;
    }
    else
    {
        if (!bNoRows)
            nDestCol2 = sal::static_int_cast<SCCOL>(nDestCol2 + nDataCount);
        nDestCol2 --;
    }
}

void ScPivot::SetDataLine(SCCOL nCol, SCROW nRow, SCTAB /* nTab */, SCSIZE nRIndex)
{
    SCSIZE nCIndex2;

    SubTotal aGrandTotal[PIVOT_MAXFIELD];           // pro Daten-Feld

    for (SCSIZE i=0; i < nColIndex; i++)
    {
        SCSIZE nCIndex = pColRef[i].nDataIndex;
        if (nCIndex != sal::static_int_cast<SCSIZE>(PIVOT_FUNC_REF))
        {
//          if ( ppDataArr[nRIndex][nCIndex].GetCount() )
            {
                SCSIZE nDIndex = ppDataArr[nRIndex][nCIndex].nIndex;
                SetValue( sal::static_int_cast<SCCOL>(nCol+i), nRow, ppDataArr[nRIndex][nCIndex], aDataArr[nDIndex].nFuncMask );
                //  Kategorie 18

                if (bDataAtCol)
                    aGrandTotal[0].Update(ppDataArr[nRIndex][nCIndex]);
                else
                    aGrandTotal[nDIndex].Update(ppDataArr[nRIndex][nCIndex]);
            }
        }
        else
        {
            SubTotal aTotal;
            SCSIZE k = i-1;
            while ((pColRef[k].nDataIndex == sal::static_int_cast<SCSIZE>(PIVOT_FUNC_REF)) && (k > 0))
                k--;
            for (SCSIZE j=k+1; (j-- > 0) && (pColRef[j].nRecCount > pColRef[i].nRecCount); )
            {
                nCIndex2 = pColRef[j].nDataIndex;
                if (nCIndex2 != sal::static_int_cast<SCSIZE>(PIVOT_FUNC_REF))
                {
                    if ((pColRef[i].nIndex == ppDataArr[nRIndex][nCIndex2].nIndex) ||
                        (pColRef[i].nIndex == SCSIZE_MAX))
                    {
                        aTotal.Update( ppDataArr[nRIndex][nCIndex2] );
                    }
                }
            }

            USHORT nFunc = pColRef[i].nFuncMask;
            if (nFunc == PIVOT_FUNC_AUTO)
                nFunc = aDataArr[nRIndex/nDataMult%nDataCount].nFuncMask;
            SetValue( sal::static_int_cast<SCCOL>(nCol+i), nRow, aTotal, nFunc );
            //  Kategorie 19
        }
    }

    BOOL bNoRows = (nRowCount == 0) || ( nRowCount == 1 && aRowArr[0].nCol == PIVOT_DATA_FIELD );
    if (!bMakeTotalCol) bNoRows = TRUE;

    if (!bNoRows)
    {
        if (bDataAtCol)
        {
            SetValue( nDestCol2, nRow, aGrandTotal[0], aDataArr[nRIndex/nDataMult%nDataCount].nFuncMask );
            //  Kategorie 20
        }
        else
        {
            SCCOL nTotalCol = sal::static_int_cast<SCCOL>(nDestCol2 - nDataCount + 1);
            for (SCSIZE nTotCnt = 0; nTotCnt<nDataCount; nTotCnt++)
            {
                SetValue( sal::static_int_cast<SCCOL>(nTotalCol+nTotCnt), nRow, aGrandTotal[nTotCnt], aDataArr[nTotCnt].nFuncMask );
                //  Kategorie 21
            }
        }
    }
}

void ScPivot::SetFuncLine(SCCOL nCol, SCROW nRow, SCTAB /* nTab */, USHORT nFunc, SCSIZE nIndex, SCSIZE nStartRIndex, SCSIZE nEndRIndex)
{
    SCSIZE nSubtCount = 0;
    SubTotal aGrandTotal[PIVOT_MAXFIELD];
    USHORT nThisFunc = nFunc;

    for (SCSIZE i=0; i<nColIndex; i++)
    {
        SCSIZE nCIndex = pColRef[i].nDataIndex;
        if (nCIndex != sal::static_int_cast<SCSIZE>(PIVOT_FUNC_REF))
        {
            SubTotal aTotal;
            for (SCSIZE j = nStartRIndex; j < nEndRIndex; j++)
            {
                SCSIZE nDIndex = ppDataArr[j][nCIndex].nIndex;
                if ((nIndex == nDIndex) || (nIndex == SCSIZE_MAX))
                {
                    aTotal.Update( ppDataArr[j][nCIndex] );
                }
            }

            if (bDataAtCol)
                aGrandTotal[0].Update( aTotal );
            else
                aGrandTotal[nCIndex/nDataMult%nDataCount].Update( aTotal );     //! immer ?

            if (nFunc == PIVOT_FUNC_AUTO)
            {
                if (bDataAtCol)
                {
                    if (nIndex<nDataCount)
                        nThisFunc = aDataArr[nIndex].nFuncMask;
                    else
                    {
                        DBG_ERROR("wat fuer'n Index ???");
                    }
                }
                else
                    nThisFunc = aDataArr[nCIndex/nDataMult%nDataCount].nFuncMask;
            }
            SetValue( sal::static_int_cast<SCCOL>(nCol+i), nRow, aTotal, nThisFunc );
            //  Kategorie 22
        }
        else
        {                       //  Kreuzungspunkte kompatibel ?

            if ( nFunc == pColRef[i].nFuncMask )
            {
                SCSIZE nEffIndex = nIndex;
                if (nEffIndex == SCSIZE_MAX)
                {
                    nEffIndex = nSubtCount % nDataCount;
                    ++nSubtCount;
                }
                SubTotal aTotal;

                SCSIZE k = i-1;
                while ((pColRef[k].nDataIndex == sal::static_int_cast<SCSIZE>(PIVOT_FUNC_REF)) && (k > 0))
                    k--;
                for (SCSIZE j=k+1; (j-- > 0) && (pColRef[j].nRecCount > pColRef[i].nRecCount); )
                {
                    nCIndex = pColRef[j].nDataIndex;
                    if (nCIndex != sal::static_int_cast<SCSIZE>(PIVOT_FUNC_REF))
                    {
                        for (SCSIZE nRIndex = nStartRIndex; nRIndex < nEndRIndex; nRIndex++)
                        {
                            SCSIZE nDIndex = ppDataArr[nRIndex][nCIndex].nIndex;
                            if (nEffIndex == nDIndex)
                            {
                                aTotal.Update( ppDataArr[nRIndex][nCIndex] );
                            }
                        }
                    }
                }

                if (nFunc == PIVOT_FUNC_AUTO)
                {
                    if (nEffIndex<nDataCount)
                        nThisFunc = aDataArr[nEffIndex].nFuncMask;
                    else
                    {
                        DBG_ERROR("wat fuer'n Index ???");
                    }
                }
                SetValue( sal::static_int_cast<SCCOL>(nCol+i), nRow, aTotal, nThisFunc );
                //  Kategorie 23
            }
        }
    }

    BOOL bNoRows = (nRowCount == 0) || ( nRowCount == 1 && aRowArr[0].nCol == PIVOT_DATA_FIELD );
    if (!bMakeTotalCol) bNoRows = TRUE;

    if (!bNoRows)
    {
        if (bDataAtCol)
        {
            if (nFunc == PIVOT_FUNC_AUTO)
            {
                if (nIndex<nDataCount)
                    nThisFunc = aDataArr[nIndex].nFuncMask;
                else
                {
                    DBG_ERROR("wat fuer'n Index ???");
                }
            }
            SetValue( nDestCol2, nRow, aGrandTotal[0], nThisFunc );
            //  Kategorie 24
        }
        else
        {
            SCCOL nTotalCol = sal::static_int_cast<SCCOL>(nDestCol2 - nDataCount + 1);
            for (SCSIZE nTotCnt = 0; nTotCnt<nDataCount; nTotCnt++)
            {
                if (nFunc == PIVOT_FUNC_AUTO)
                    nThisFunc = aDataArr[nTotCnt%nDataCount].nFuncMask;
                SetValue( sal::static_int_cast<SCCOL>(nTotalCol+nTotCnt), nRow, aGrandTotal[nTotCnt], nThisFunc );
                //  Kategorie 25
            }
        }
    }
}

void ScPivot::ColToTable(SCSIZE nField, SCROW& nRow, ScProgress& rProgress)
{
    SCCOL nCol = sal::static_int_cast<SCCOL>(nDestCol1 + nField);
    if (nColCount == 0)
    {
//      SetDataLine(nCol + 1, nRow, nDestTab, nRowIndex);
        SetDataLine(nCol, nRow, nDestTab, nRowIndex);
        nRowIndex++;
        return;
    }

    SCSIZE nDx;
    if ((aColArr[nColCount -1].nCol == PIVOT_DATA_FIELD) && (nDataCount == 1))
        nDx = 2;
    else
        nDx = 1;
    if (nField < nColCount - nDx)
    {
        for (USHORT i = 0; i < pColList[nField]->GetCount(); i++)
        {
            SCSIZE nSaveIndex = nRowIndex;
            String aStr = pColList[nField]->GetString(i);
            if (!aStr.Len()) aStr = ScGlobal::GetRscString(STR_EMPTYDATA);
            pDoc->SetString(nCol, nRow, nDestTab, aStr);
            //  Kategorie 10
            SCROW nSaveRow = nRow;
            ColToTable(nField + 1, nRow, rProgress);
            SetStyle(nCol, nSaveRow, nCol, nRow - 1, PIVOT_STYLE_CATEGORY);
            SetFrame(nCol, nSaveRow, nCol, nRow - 1);
            if (aColArr[nField].nFuncCount > 0)                 // Zwischenergebnisse eingestellt?
            {
                nSaveRow = nRow;
                for (SCSIZE j=0; j<=PIVOT_MAXFUNC; j++)                 // incl. "auto"
                {
                    if (aColArr[nField].nFuncMask & nFuncMaskArr[j])
                    {
                        String aLab;
                        if (bDataAtCol)
                        {
                            for (SCSIZE k=0; k < nDataCount; k++)
                            {
                                String aDataStr = pDataList->GetString(sal::static_int_cast<USHORT>(k));    // always String
                                aLab = aStr;
                                SCSIZE nFuncType;
                                if ( j==PIVOT_MAXFUNC )
                                    nFuncType = lcl_MaskToIndex( aDataArr[k].nFuncMask );
                                else
                                    nFuncType = j;
                                aLab += ' ';
                                aLab += *pLabel[nFuncType];
                                aLab += ' ';
                                aLab += aDataStr;
                                pDoc->SetString(nCol, nRow, nDestTab, aLab);
                                //  Kategorie 11
                                SetFuncLine(nDataStartCol, nRow, nDestTab, nFuncMaskArr[j], k, nSaveIndex, nRowIndex);
                                nRow++;
                            }
                        }
                        else
                        {
                            aLab = aStr;
                            aLab += ' ';
                            aLab += *pLabel[j];
                            pDoc->SetString(nCol, nRow, nDestTab, aLab);
                            //  Kategorie 12
                            SetFuncLine(nDataStartCol, nRow, nDestTab, nFuncMaskArr[j], SCSIZE_MAX, nSaveIndex, nRowIndex);
                            nRow++;
                        }
                    }
                }
                if ( nDataStartCol > 0 )
                    SetStyle(nCol, nSaveRow, nDataStartCol-1, nRow-1, PIVOT_STYLE_TITLE);
                SetStyle(nDataStartCol, nSaveRow, nDestCol2, nRow-1, PIVOT_STYLE_RESULT);
                SetFrameHor(nCol, nSaveRow, nDestCol2, nRow-1);
            }
            nSaveIndex = nRowIndex;
        }
    }
    else if (nField < nColCount)
    {
        SCSIZE nCatCount = pColList[nField]->GetCount();
        SetStyle(nCol, nRow, nCol, nRow+nCatCount-1, PIVOT_STYLE_CATEGORY);
        SetFrame(nCol, nRow, nDestCol2, nRow+nCatCount-1);
        for (SCSIZE i = 0; i < nCatCount; i++)
        {
            String aTmpStr = pColList[nField]->GetString(sal::static_int_cast<USHORT>(i));
            if (!aTmpStr.Len()) aTmpStr = ScGlobal::GetRscString(STR_EMPTYDATA);

            String aPutStr;
            if (pColList[nField] == pDataList)
            {
                SCSIZE nFuncType = lcl_MaskToIndex( aDataArr[i].nFuncMask );
                aPutStr  = *pLabel[nFuncType];
                aPutStr += ' ';
                aPutStr += aTmpStr;
            }
            else
                aPutStr += aTmpStr;

            pDoc->SetString(nCol, nRow, nDestTab, aPutStr);
            //  Kategorie 13
            SetDataLine(nCol + 1, nRow, nDestTab, nRowIndex);
            nRowIndex++;
            nRow++;

            rProgress.SetState( nRow - nDestRow1 );
        }
    }
}

void ScPivot::RowToTable(SCSIZE nField, SCCOL& nCol)
{
    nRecCount++;
    SCROW nRow = nDestRow1 + nFirstLine + nField + 1;
    if (nRowCount == 0)
    {
        pColRef[nColIndex].nDataIndex = nDataIndex;
        nColIndex++;
        nDataIndex++;
        return;
    }

    SCSIZE nDx;
    if ((aRowArr[nRowCount -1].nCol == PIVOT_DATA_FIELD) && (nDataCount == 1))
        nDx = 2;
    else
        nDx = 1;

    if (nField < nRowCount - nDx)
    {
        for (USHORT i = 0; i < pRowList[nField]->GetCount(); i++)
        {
            String aStr = pRowList[nField]->GetString(i);
            if (!aStr.Len()) aStr = ScGlobal::GetRscString(STR_EMPTYDATA);
            pDoc->SetString(nCol, nRow, nDestTab, aStr);
            //  Kategorie 14
            SCCOL nSaveCol = nCol;
            RowToTable(nField + 1, nCol);
            SetStyle(nSaveCol, nRow, nCol - 1, nRow, PIVOT_STYLE_CATEGORY);
            SetFrame(nSaveCol, nRow, nCol - 1, nRow);
            if (aRowArr[nField].nFuncCount > 0)
            {
                nSaveCol = nCol;
                for (SCSIZE j=0; j<=PIVOT_MAXFUNC; j++)                 // incl. "auto"
                {
                    if (aRowArr[nField].nFuncMask & nFuncMaskArr[j])
                    {
                        String aLab;
                        if (!bDataAtCol)
                        {
                            for (SCSIZE k=0; k < nDataCount; k++)
                            {
                                aLab = aStr;
                                SCSIZE nFuncType;
                                if ( j==PIVOT_MAXFUNC )
                                    nFuncType = lcl_MaskToIndex( aDataArr[k].nFuncMask );
                                else
                                    nFuncType = j;
                                aLab += ' ';
                                aLab += *pLabel[nFuncType];
                                aLab += ' ';
                                aLab += pDataList->GetString(sal::static_int_cast<USHORT>(k));
                                pDoc->SetString(nCol, nRow, nDestTab, aLab);
                                //  Kategorie 15
                                pColRef[nColIndex].nDataIndex = PIVOT_FUNC_REF;
                                pColRef[nColIndex].nRecCount = nRecCount;
                                pColRef[nColIndex].nIndex = k;
                                pColRef[nColIndex].nFuncMask = nFuncMaskArr[j];
                                nColIndex++;
                                nCol++;
                            }
                        }
                        else
                        {
                            aLab = aStr;
                            aLab += ' ';
                            aLab += *pLabel[j];
                            pDoc->SetString(nCol, nRow, nDestTab, aLab);
                            //  Kategorie 16
                            pColRef[nColIndex].nDataIndex = PIVOT_FUNC_REF;
                            pColRef[nColIndex].nRecCount = nRecCount;
                            pColRef[nColIndex].nIndex = SCSIZE_MAX;
                            pColRef[nColIndex].nFuncMask = nFuncMaskArr[j];
                            nColIndex++;
                            nCol++;
                        }
                    }
                }
                if ( nDataStartRow > 0 )
                    SetStyle(nSaveCol, nRow,
                                nCol-1, nDataStartRow-1, PIVOT_STYLE_TITLE);
                SetStyle(nSaveCol, nDataStartRow, nCol-1, nDestRow2, PIVOT_STYLE_RESULT);
                SetFrameVer(nSaveCol, nRow, nCol-1, nDestRow2);
            }
        }
    }
    else if (nField < nRowCount)
    {
        SCSIZE nCatCount = pRowList[nField]->GetCount();
        SetStyle(nCol, nRow, sal::static_int_cast<SCCOL>(nCol+nCatCount-1), nRow, PIVOT_STYLE_CATEGORY);
        SetFrame(nCol, nRow, sal::static_int_cast<SCCOL>(nCol+nCatCount-1), nDestRow2);
        for (SCSIZE i = 0; i < nCatCount; i++)
        {
            String aTmpStr = pRowList[nField]->GetString(sal::static_int_cast<USHORT>(i));
            if (!aTmpStr.Len()) aTmpStr = ScGlobal::GetRscString(STR_EMPTYDATA);

            String aPutStr;
            if (pRowList[nField] == pDataList)
            {
                SCSIZE nFuncType = lcl_MaskToIndex( aDataArr[i].nFuncMask );
                aPutStr  = *pLabel[nFuncType];
                aPutStr += ' ';
                aPutStr += aTmpStr;
            }
            else
                aPutStr = aTmpStr;

            pDoc->SetString(nCol, nRow, nDestTab, aPutStr);
            //  Kategorie 17
            pColRef[nColIndex].nDataIndex = nDataIndex;
            pColRef[nColIndex].nRecCount = nRecCount;
            pColRef[nColIndex].nIndex = SCSIZE_MAX;
            pColRef[nColIndex].nFuncMask = PIVOT_FUNC_NONE;
            nColIndex++;
            nDataIndex++;
            nCol++;
        }
    }
    nRecCount--;
}

SCROW ScPivot::GetCategoryRow( SCCOL nCol, SCROW nRow )
{
    SCROW nMinRow = nSrcRow1;
    if (bHasHeader) ++nMinRow;
    BOOL bFound = FALSE;
    do
    {
        if ( !pDoc->HasData( nCol, nRow, nSrcTab ) && nRow>nMinRow )
            --nRow;
        else
            bFound = TRUE;
    }
    while (!bFound);
    return nRow;
}



