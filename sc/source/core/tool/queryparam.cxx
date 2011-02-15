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

using ::std::vector;

// ============================================================================

ScQueryParamBase::ScQueryParamBase()
{
    Resize( MAXQUERY );
    for (sal_uInt16 i=0; i<MAXQUERY; i++)
        maEntries[i].Clear();
}

ScQueryParamBase::ScQueryParamBase(const ScQueryParamBase& r) :
    bHasHeader(r.bHasHeader), bByRow(r.bByRow), bInplace(r.bInplace), bCaseSens(r.bCaseSens),
    bRegExp(r.bRegExp), bDuplicate(r.bDuplicate), bMixedComparison(r.bMixedComparison),
    maEntries(r.maEntries)
{
}

ScQueryParamBase::~ScQueryParamBase()
{
}

SCSIZE ScQueryParamBase::GetEntryCount() const
{
    return maEntries.size();
}

ScQueryEntry& ScQueryParamBase::GetEntry(SCSIZE n) const
{
    return maEntries[n];
}

void ScQueryParamBase::Resize(SCSIZE nNew)
{
    if ( nNew < MAXQUERY )
        nNew = MAXQUERY;                // nie weniger als MAXQUERY

    vector<ScQueryEntry> aNewEntries(nNew);
    SCSIZE nCopy = ::std::min(maEntries.size(), nNew);
    for (SCSIZE i=0; i<nCopy; i++)
        aNewEntries[i] = maEntries[i];

    maEntries.swap(aNewEntries);
}

void ScQueryParamBase::DeleteQuery( SCSIZE nPos )
{
    if (nPos >= maEntries.size())
        return;

    size_t n = maEntries.size();
    vector<ScQueryEntry> aNewEntries;
    aNewEntries.reserve(n);
    for (size_t i = 0; i < n; ++i)
        if (i != nPos)
            aNewEntries.push_back(maEntries[i]);

    // Don't forget to append an empty entry to make up for the removed one.
    // The size of the entries is not supposed to change.
    aNewEntries.push_back(ScQueryEntry());

    maEntries.swap(aNewEntries);
}

void ScQueryParamBase::FillInExcelSyntax(String& aCellStr, SCSIZE nIndex)
{
    if (aCellStr.Len() > 0)
    {
        if ( nIndex >= maEntries.size() )
            Resize( nIndex+1 );

        ScQueryEntry& rEntry = GetEntry(nIndex);

        rEntry.bDoQuery = sal_True;
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
    ScQueryParamTable(),
    bDestPers(true),
    nDestTab(0),
    nDestCol(0),
    nDestRow(0)
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

ScQueryParam::ScQueryParam( const ScDBQueryParamInternal& r ) :
    ScQueryParamBase(r),
    ScQueryParamTable(r),
    bDestPers(true),
    nDestTab(0),
    nDestCol(0),
    nDestRow(0)
{
}


//------------------------------------------------------------------------

ScQueryParam::~ScQueryParam()
{
}

//------------------------------------------------------------------------

void ScQueryParam::Clear()
{
    nCol1=nCol2 = 0;
    nRow1=nRow2 = 0;
    nTab = SCTAB_MAX;
    bHasHeader = bCaseSens = bRegExp = bMixedComparison = sal_False;
    bInplace = bByRow = bDuplicate = sal_True;

    Resize( MAXQUERY );
    for (sal_uInt16 i=0; i<MAXQUERY; i++)
        maEntries[i].Clear();

    ClearDestParams();
}

void ScQueryParam::ClearDestParams()
{
    bDestPers = true;
    nDestTab = 0;
    nDestCol = 0;
    nDestRow = 0;
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

    maEntries = r.maEntries;

    return *this;
}

//------------------------------------------------------------------------

sal_Bool ScQueryParam::operator==( const ScQueryParam& rOther ) const
{
    sal_Bool bEqual = sal_False;

    // Anzahl der Queries gleich?
    SCSIZE nUsed      = 0;
    SCSIZE nOtherUsed = 0;
    SCSIZE nEntryCount = GetEntryCount();
    SCSIZE nOtherEntryCount = rOther.GetEntryCount();

    while ( nUsed<nEntryCount && maEntries[nUsed].bDoQuery ) ++nUsed;
    while ( nOtherUsed<nOtherEntryCount && rOther.maEntries[nOtherUsed].bDoQuery )
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
        bEqual = sal_True;
        for ( SCSIZE i=0; i<nUsed && bEqual; i++ )
            bEqual = maEntries[i] == rOther.maEntries[i];
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
        size_t n = maEntries.size();
        for (size_t i=0; i<n; i++)
            maEntries[i].nField += nDifX;

        bInplace = sal_True;
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
    mbSkipString(true),
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

