/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
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


// INCLUDE ---------------------------------------------------------------

#include "queryparam.hxx"
#include "queryentry.hxx"

namespace {

const size_t MAXQUERY = 8;

class FindByField : public std::unary_function<ScQueryEntry, bool>
{
    SCCOLROW mnField;
public:
    FindByField(SCCOLROW nField) : mnField(nField) {}
    bool operator() (const ScQueryEntry& rEntry) const
    {
        return rEntry.bDoQuery && rEntry.nField == mnField;
    }
};

struct FindUnused : public std::unary_function<ScQueryEntry, bool>
{
    bool operator() (const ScQueryEntry& rEntry) const
    {
        return !rEntry.bDoQuery;
    }
};

}

ScQueryParamBase::const_iterator ScQueryParamBase::begin() const
{
    return maEntries.begin();
}

ScQueryParamBase::const_iterator ScQueryParamBase::end() const
{
    return maEntries.end();
}

ScQueryParamBase::ScQueryParamBase() :
    bHasHeader(true),
    bByRow(true),
    bInplace(true),
    bCaseSens(false),
    bRegExp(false),
    bDuplicate(false)
{
    for (size_t i = 0; i < MAXQUERY; ++i)
        maEntries.push_back(new ScQueryEntry);
}

ScQueryParamBase::ScQueryParamBase(const ScQueryParamBase& r) :
    bHasHeader(r.bHasHeader), bByRow(r.bByRow), bInplace(r.bInplace), bCaseSens(r.bCaseSens),
    bRegExp(r.bRegExp), bDuplicate(r.bDuplicate),
    maEntries(r.maEntries)
{
}

ScQueryParamBase::~ScQueryParamBase()
{
}

bool ScQueryParamBase::IsValidFieldIndex() const
{
    return true;
}

SCSIZE ScQueryParamBase::GetEntryCount() const
{
    return maEntries.size();
}

const ScQueryEntry& ScQueryParamBase::GetEntry(SCSIZE n) const
{
    return maEntries[n];
}

ScQueryEntry& ScQueryParamBase::GetEntry(SCSIZE n)
{
    return maEntries[n];
}

ScQueryEntry& ScQueryParamBase::AppendEntry()
{
    // Find the first unused entry.
    EntriesType::iterator itr = std::find_if(
        maEntries.begin(), maEntries.end(), FindUnused());

    if (itr != maEntries.end())
        // Found!
        return *itr;

    // Add a new entry to the end.
    maEntries.push_back(new ScQueryEntry);
    return maEntries.back();
}

ScQueryEntry* ScQueryParamBase::FindEntryByField(SCCOLROW nField, bool bNew)
{
    EntriesType::iterator itr = std::find_if(
        maEntries.begin(), maEntries.end(), FindByField(nField));

    if (itr != maEntries.end())
    {
        // existing entry found!
        return &(*itr);
    }

    if (!bNew)
        // no existing entry found, and we are not creating a new one.
        return NULL;

    return &AppendEntry();
}

void ScQueryParamBase::RemoveEntryByField(SCCOLROW nField)
{
    EntriesType::iterator itr = std::find_if(
        maEntries.begin(), maEntries.end(), FindByField(nField));

    if (itr != maEntries.end())
    {
        maEntries.erase(itr);
        if (maEntries.size() < MAXQUERY)
            // Make sure that we have at least MAXQUERY number of entries at
            // all times.
            maEntries.push_back(new ScQueryEntry);
    }
}

void ScQueryParamBase::Resize(size_t nNew)
{
    if (nNew < MAXQUERY)
        nNew = MAXQUERY;                // never less than MAXQUERY

    if (nNew < maEntries.size())
    {
        size_t n = maEntries.size() - nNew;
        for (size_t i = 0; i < n; ++i)
            maEntries.pop_back();
    }
    else if (nNew > maEntries.size())
    {
        size_t n = nNew - maEntries.size();
        for (size_t i = 0; i < n; ++i)
            maEntries.push_back(new ScQueryEntry);
    }
}

void ScQueryParamBase::FillInExcelSyntax(const rtl::OUString& rStr, SCSIZE nIndex)
{
    const String aCellStr = rStr;
    if (aCellStr.Len() > 0)
    {
        if ( nIndex >= maEntries.size() )
            Resize( nIndex+1 );

        ScQueryEntry& rEntry = GetEntry(nIndex);
        ScQueryEntry::Item& rItem = rEntry.GetQueryItem();

        rEntry.bDoQuery = sal_True;
        // Operatoren herausfiltern
        if (aCellStr.GetChar(0) == '<')
        {
            if (aCellStr.GetChar(1) == '>')
            {
                rItem.maString = aCellStr.Copy(2);
                rEntry.eOp   = SC_NOT_EQUAL;
            }
            else if (aCellStr.GetChar(1) == '=')
            {
                rItem.maString = aCellStr.Copy(2);
                rEntry.eOp   = SC_LESS_EQUAL;
            }
            else
            {
                rItem.maString = aCellStr.Copy(1);
                rEntry.eOp   = SC_LESS;
            }
        }
        else if (aCellStr.GetChar(0) == '>')
        {
            if (aCellStr.GetChar(1) == '=')
            {
                rItem.maString = aCellStr.Copy(2);
                rEntry.eOp   = SC_GREATER_EQUAL;
            }
            else
            {
                rItem.maString = aCellStr.Copy(1);
                rEntry.eOp   = SC_GREATER;
            }
        }
        else
        {
            if (aCellStr.GetChar(0) == '=')
                rItem.maString = aCellStr.Copy(1);
            else
                rItem.maString = aCellStr;
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
    bHasHeader = bCaseSens = bRegExp = false;
    bInplace = bByRow = bDuplicate = sal_True;

    boost::ptr_vector<ScQueryEntry>::iterator itr = maEntries.begin(), itrEnd = maEntries.end();
    for (; itr != itrEnd; ++itr)
        itr->Clear();

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
    bDuplicate  = r.bDuplicate;
    bByRow      = r.bByRow;
    bDestPers   = r.bDestPers;

    maEntries = r.maEntries.clone();

    return *this;
}

//------------------------------------------------------------------------

bool ScQueryParam::operator==( const ScQueryParam& rOther ) const
{
    bool bEqual = false;

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
        && (bDuplicate  == rOther.bDuplicate)
        && (bDestPers   == rOther.bDestPers)
        && (nDestTab    == rOther.nDestTab)
        && (nDestCol    == rOther.nDestCol)
        && (nDestRow    == rOther.nDestRow) )
    {
        bEqual = true;
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
        OSL_FAIL("MoveToDest, bInplace == TRUE");
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

bool ScDBQueryParamInternal::IsValidFieldIndex() const
{
    return nCol1 <= mnField && mnField <= nCol2;
}

// ============================================================================

ScDBQueryParamMatrix::ScDBQueryParamMatrix() :
    ScDBQueryParamBase(ScDBQueryParamBase::MATRIX)
{
}

bool ScDBQueryParamMatrix::IsValidFieldIndex() const
{
    SCSIZE nC, nR;
    mpMatrix->GetDimensions(nC, nR);
    return 0 <= mnField && mnField <= static_cast<SCCOL>(nC);
}

ScDBQueryParamMatrix::~ScDBQueryParamMatrix()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
