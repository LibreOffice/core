/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: interpr4.cxx,v $
 * $Revision: 1.57.92.5 $
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

// INCLUDE ---------------------------------------------------------------

#include "queryparam.hxx"

// ============================================================================

ScQueryParamBase::ScQueryParamBase() :
    nEntryCount(0)
{
    Resize( MAXQUERY );
    for (USHORT i=0; i<MAXQUERY; i++)
        pEntries[i].Clear();
}

ScQueryParamBase::ScQueryParamBase(const ScQueryParamBase& r) :
    bHasHeader(r.bHasHeader), bByRow(r.bByRow), bInplace(r.bInplace), bCaseSens(r.bCaseSens),
    bRegExp(r.bRegExp), bDuplicate(r.bDuplicate), bMixedComparison(r.bMixedComparison)
{
    nEntryCount = 0;

    Resize( r.nEntryCount );
    for (USHORT i=0; i<nEntryCount; i++)
        pEntries[i] = r.pEntries[i];
}

ScQueryParamBase::~ScQueryParamBase()
{
    delete[] pEntries;
}

void ScQueryParamBase::Resize(SCSIZE nNew)
{
    if ( nNew < MAXQUERY )
        nNew = MAXQUERY;                // nie weniger als MAXQUERY

    ScQueryEntry* pNewEntries = NULL;
    if ( nNew )
        pNewEntries = new ScQueryEntry[nNew];

    SCSIZE nCopy = Min( nEntryCount, nNew );
    for (SCSIZE i=0; i<nCopy; i++)
        pNewEntries[i] = pEntries[i];

    if ( nEntryCount )
        delete[] pEntries;
    nEntryCount = nNew;
    pEntries = pNewEntries;
}

void ScQueryParamBase::DeleteQuery( SCSIZE nPos )
{
    if (nPos<nEntryCount)
    {
        for (SCSIZE i=nPos; i+1<nEntryCount; i++)
            pEntries[i] = pEntries[i+1];

        pEntries[nEntryCount-1].Clear();
    }
    else
    {
        DBG_ERROR("Falscher Parameter bei ScQueryParam2::DeleteQuery");
    }
}

void ScQueryParamBase::FillInExcelSyntax(String& aCellStr, SCSIZE nIndex)
{
    if (aCellStr.Len() > 0)
    {
        if ( nIndex >= nEntryCount )
            Resize( nIndex+1 );

        ScQueryEntry& rEntry = pEntries[nIndex];

        rEntry.bDoQuery = TRUE;
        // Operatoren herausfiltern
        if (aCellStr.GetChar(0) == '<')
        {
            if (aCellStr.GetChar(1) == '>')
            {
                *rEntry.pStr = aCellStr.Copy(2);
                rEntry.eOp   = SC_NOT_EQUAL;
            }
            else if (aCellStr.GetChar(1) == '=')
            {
                *rEntry.pStr = aCellStr.Copy(2);
                rEntry.eOp   = SC_LESS_EQUAL;
            }
            else
            {
                *rEntry.pStr = aCellStr.Copy(1);
                rEntry.eOp   = SC_LESS;
            }
        }
        else if (aCellStr.GetChar(0) == '>')
        {
            if (aCellStr.GetChar(1) == '=')
            {
                *rEntry.pStr = aCellStr.Copy(2);
                rEntry.eOp   = SC_GREATER_EQUAL;
            }
            else
            {
                *rEntry.pStr = aCellStr.Copy(1);
                rEntry.eOp   = SC_GREATER;
            }
        }
        else
        {
            if (aCellStr.GetChar(0) == '=')
                *rEntry.pStr = aCellStr.Copy(1);
            else
                *rEntry.pStr = aCellStr;
            rEntry.eOp = SC_EQUAL;
        }
    }
}

// ============================================================================

ScQueryParamTable::ScQueryParamTable()
{
}

ScQueryParamTable::ScQueryParamTable(const ScQueryParamTable& r) :
    nCol1(r.nCol1),nRow1(r.nRow1),nCol2(r.nCol2),nRow2(r.nRow2),nTab(r.nTab)
{
}

ScQueryParamTable::~ScQueryParamTable()
{
}

// ============================================================================

ScQueryParam::ScQueryParam() :
    ScQueryParamBase(),
    ScQueryParamTable()
{
    Clear();
}

//------------------------------------------------------------------------

ScQueryParam::ScQueryParam( const ScQueryParam& r ) :
    ScQueryParamBase(r),
    ScQueryParamTable(r),
    bDestPers(r.bDestPers), nDestTab(r.nDestTab), nDestCol(r.nDestCol), nDestRow(r.nDestRow)
{
}

//------------------------------------------------------------------------

ScQueryParam::~ScQueryParam()
{
}

//------------------------------------------------------------------------

void ScQueryParam::Clear()
{
    nCol1=nCol2=nDestCol = 0;
    nRow1=nRow2=nDestRow = 0;
    nDestTab = 0;
    nTab = SCTAB_MAX;
    bHasHeader = bCaseSens = bRegExp = bMixedComparison = FALSE;
    bInplace = bByRow = bDuplicate = bDestPers = TRUE;

    Resize( MAXQUERY );
    for (USHORT i=0; i<MAXQUERY; i++)
        pEntries[i].Clear();
}

//------------------------------------------------------------------------

ScQueryParam& ScQueryParam::operator=( const ScQueryParam& r )
{
    nCol1       = r.nCol1;
    nRow1       = r.nRow1;
    nCol2       = r.nCol2;
    nRow2       = r.nRow2;
    nTab        = r.nTab;
    nDestTab    = r.nDestTab;
    nDestCol    = r.nDestCol;
    nDestRow    = r.nDestRow;
    bHasHeader  = r.bHasHeader;
    bInplace    = r.bInplace;
    bCaseSens   = r.bCaseSens;
    bRegExp     = r.bRegExp;
    bMixedComparison = r.bMixedComparison;
    bDuplicate  = r.bDuplicate;
    bByRow      = r.bByRow;
    bDestPers   = r.bDestPers;

    Resize( r.nEntryCount );
    for (USHORT i=0; i<nEntryCount; i++)
        pEntries[i] = r.pEntries[i];

    return *this;
}

//------------------------------------------------------------------------

BOOL ScQueryParam::operator==( const ScQueryParam& rOther ) const
{
    BOOL bEqual = FALSE;

    // Anzahl der Queries gleich?
    USHORT nUsed      = 0;
    USHORT nOtherUsed = 0;
    while ( nUsed<nEntryCount && pEntries[nUsed].bDoQuery ) ++nUsed;
    while ( nOtherUsed<rOther.nEntryCount && rOther.pEntries[nOtherUsed].bDoQuery )
        ++nOtherUsed;

    if (   (nUsed       == nOtherUsed)
        && (nCol1       == rOther.nCol1)
        && (nRow1       == rOther.nRow1)
        && (nCol2       == rOther.nCol2)
        && (nRow2       == rOther.nRow2)
        && (nTab        == rOther.nTab)
        && (bHasHeader  == rOther.bHasHeader)
        && (bByRow      == rOther.bByRow)
        && (bInplace    == rOther.bInplace)
        && (bCaseSens   == rOther.bCaseSens)
        && (bRegExp     == rOther.bRegExp)
        && (bMixedComparison == rOther.bMixedComparison)
        && (bDuplicate  == rOther.bDuplicate)
        && (bDestPers   == rOther.bDestPers)
        && (nDestTab    == rOther.nDestTab)
        && (nDestCol    == rOther.nDestCol)
        && (nDestRow    == rOther.nDestRow) )
    {
        bEqual = TRUE;
        for ( USHORT i=0; i<nUsed && bEqual; i++ )
            bEqual = pEntries[i] == rOther.pEntries[i];
    }
    return bEqual;
}

//------------------------------------------------------------------------

void ScQueryParam::MoveToDest()
{
    if (!bInplace)
    {
        SCsCOL nDifX = ((SCsCOL) nDestCol) - ((SCsCOL) nCol1);
        SCsROW nDifY = ((SCsROW) nDestRow) - ((SCsROW) nRow1);
        SCsTAB nDifZ = ((SCsTAB) nDestTab) - ((SCsTAB) nTab);

        nCol1 = sal::static_int_cast<SCCOL>( nCol1 + nDifX );
        nRow1 = sal::static_int_cast<SCROW>( nRow1 + nDifY );
        nCol2 = sal::static_int_cast<SCCOL>( nCol2 + nDifX );
        nRow2 = sal::static_int_cast<SCROW>( nRow2 + nDifY );
        nTab  = sal::static_int_cast<SCTAB>( nTab  + nDifZ );
        for (USHORT i=0; i<nEntryCount; i++)
            pEntries[i].nField += nDifX;

        bInplace = TRUE;
    }
    else
    {
        DBG_ERROR("MoveToDest, bInplace == TRUE");
    }
}

// ============================================================================

ScDBQueryParamBase::ScDBQueryParamBase(DataType eType) :
    ScQueryParamBase(),
    mnField(-1),
    meType(eType)
{
}

ScDBQueryParamBase::~ScDBQueryParamBase()
{
}

ScDBQueryParamBase::DataType ScDBQueryParamBase::GetType() const
{
    return meType;
}

// ============================================================================

ScDBQueryParamInternal::ScDBQueryParamInternal() :
    ScDBQueryParamBase(ScDBQueryParamBase::INTERNAL),
    ScQueryParamTable()
{
}

ScDBQueryParamInternal::~ScDBQueryParamInternal()
{
}

// ============================================================================

ScDBQueryParamMatrix::ScDBQueryParamMatrix() :
    ScDBQueryParamBase(ScDBQueryParamBase::MATRIX)
{
}

ScDBQueryParamMatrix::~ScDBQueryParamMatrix()
{
}

