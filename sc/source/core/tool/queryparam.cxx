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

#include "queryparam.hxx"
#include "queryentry.hxx"
#include "scmatrix.hxx"

#include <svl/sharedstringpool.hxx>
#include <svl/zforlist.hxx>
#include <o3tl/make_unique.hxx>
#include <osl/diagnose.h>

#include <algorithm>

namespace {

const size_t MAXQUERY = 8;

class FindByField : public std::unary_function<ScQueryEntry, bool>
{
    SCCOLROW mnField;
public:
    explicit FindByField(SCCOLROW nField) : mnField(nField) {}
    bool operator() (const std::unique_ptr<ScQueryEntry>& rpEntry) const
    {
        return rpEntry->bDoQuery && rpEntry->nField == mnField;
    }
};

struct FindUnused : public std::unary_function<ScQueryEntry, bool>
{
    bool operator() (const std::unique_ptr<ScQueryEntry>& rpEntry) const
    {
        return !rpEntry->bDoQuery;
    }
};

}

ScQueryParamBase::const_iterator ScQueryParamBase::begin() const
{
    return m_Entries.begin();
}

ScQueryParamBase::const_iterator ScQueryParamBase::end() const
{
    return m_Entries.end();
}

ScQueryParamBase::ScQueryParamBase() :
    eSearchType(utl::SearchParam::SearchType::Normal),
    bHasHeader(true),
    bByRow(true),
    bInplace(true),
    bCaseSens(false),
    bDuplicate(false),
    mbRangeLookup(false)
{
    for (size_t i = 0; i < MAXQUERY; ++i)
        m_Entries.push_back(o3tl::make_unique<ScQueryEntry>());
}

ScQueryParamBase::ScQueryParamBase(const ScQueryParamBase& r) :
    eSearchType(r.eSearchType), bHasHeader(r.bHasHeader), bByRow(r.bByRow), bInplace(r.bInplace),
    bCaseSens(r.bCaseSens), bDuplicate(r.bDuplicate), mbRangeLookup(r.mbRangeLookup)
{
    for (auto const& it : r.m_Entries)
    {
        m_Entries.push_back(o3tl::make_unique<ScQueryEntry>(*it));
    }
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
    return m_Entries.size();
}

const ScQueryEntry& ScQueryParamBase::GetEntry(SCSIZE n) const
{
    return *m_Entries[n];
}

ScQueryEntry& ScQueryParamBase::GetEntry(SCSIZE n)
{
    return *m_Entries[n];
}

ScQueryEntry& ScQueryParamBase::AppendEntry()
{
    // Find the first unused entry.
    EntriesType::iterator itr = std::find_if(
        m_Entries.begin(), m_Entries.end(), FindUnused());

    if (itr != m_Entries.end())
        // Found!
        return **itr;

    // Add a new entry to the end.
    m_Entries.push_back(o3tl::make_unique<ScQueryEntry>());
    return *m_Entries.back();
}

ScQueryEntry* ScQueryParamBase::FindEntryByField(SCCOLROW nField, bool bNew)
{
    EntriesType::iterator itr = std::find_if(
        m_Entries.begin(), m_Entries.end(), FindByField(nField));

    if (itr != m_Entries.end())
    {
        // existing entry found!
        return (*itr).get();
    }

    if (!bNew)
        // no existing entry found, and we are not creating a new one.
        return nullptr;

    return &AppendEntry();
}

void ScQueryParamBase::RemoveEntryByField(SCCOLROW nField)
{
    EntriesType::iterator itr = std::find_if(
        m_Entries.begin(), m_Entries.end(), FindByField(nField));

    if (itr != m_Entries.end())
    {
        m_Entries.erase(itr);
        if (m_Entries.size() < MAXQUERY)
            // Make sure that we have at least MAXQUERY number of entries at
            // all times.
            m_Entries.push_back(o3tl::make_unique<ScQueryEntry>());
    }
}

void ScQueryParamBase::Resize(size_t nNew)
{
    if (nNew < MAXQUERY)
        nNew = MAXQUERY;                // never less than MAXQUERY

    if (nNew < m_Entries.size())
    {
        size_t n = m_Entries.size() - nNew;
        for (size_t i = 0; i < n; ++i)
            m_Entries.pop_back();
    }
    else if (nNew > m_Entries.size())
    {
        size_t n = nNew - m_Entries.size();
        for (size_t i = 0; i < n; ++i)
            m_Entries.push_back(o3tl::make_unique<ScQueryEntry>());
    }
}

void ScQueryParamBase::FillInExcelSyntax(
    svl::SharedStringPool& rPool, const OUString& rCellStr, SCSIZE nIndex, SvNumberFormatter* pFormatter )
{
    if (nIndex >= m_Entries.size())
        Resize(nIndex+1);

    ScQueryEntry& rEntry = GetEntry(nIndex);
    ScQueryEntry::Item& rItem = rEntry.GetQueryItem();

    if (rCellStr.isEmpty())
        rItem.maString = svl::SharedString::getEmptyString();
    else
    {
        rEntry.bDoQuery = true;
        // Operatoren herausfiltern
        if (rCellStr[0] == '<')
        {
            if (rCellStr[1] == '>')
            {
                rItem.maString = rPool.intern(rCellStr.copy(2));
                rEntry.eOp   = SC_NOT_EQUAL;
            }
            else if (rCellStr[1] == '=')
            {
                rItem.maString = rPool.intern(rCellStr.copy(2));
                rEntry.eOp   = SC_LESS_EQUAL;
            }
            else
            {
                rItem.maString = rPool.intern(rCellStr.copy(1));
                rEntry.eOp   = SC_LESS;
            }
        }
        else if (rCellStr[0]== '>')
        {
            if (rCellStr[1] == '=')
            {
                rItem.maString = rPool.intern(rCellStr.copy(2));
                rEntry.eOp   = SC_GREATER_EQUAL;
            }
            else
            {
                rItem.maString = rPool.intern(rCellStr.copy(1));
                rEntry.eOp   = SC_GREATER;
            }
        }
        else
        {
            if (rCellStr[0] == '=')
                rItem.maString = rPool.intern(rCellStr.copy(1));
            else
                rItem.maString = rPool.intern(rCellStr);
            rEntry.eOp = SC_EQUAL;
        }
    }

    if (pFormatter)
    {
        sal_uInt32 nFormat = 0;
        bool bNumber = pFormatter->IsNumberFormat( rItem.maString.getString(), nFormat, rItem.mfVal);
        rItem.meType = bNumber ? ScQueryEntry::ByValue : ScQueryEntry::ByString;

        /* TODO: pFormatter currently is also used as a flag whether matching
         * empty cells with an empty string is triggered from the interpreter.
         * This could be handled independently if all queries should support
         * it, needs to be evaluated if that actually is desired. */
        if (rItem.meType == ScQueryEntry::ByString)
            rItem.mbMatchEmpty = (rEntry.eOp == SC_EQUAL && rItem.maString.isEmpty());
    }
}

ScQueryParamTable::ScQueryParamTable() :
    nCol1(0),nRow1(0),nCol2(0),nRow2(0),nTab(0)
{
}

ScQueryParamTable::ScQueryParamTable(const ScQueryParamTable& r) :
    nCol1(r.nCol1),nRow1(r.nRow1),nCol2(r.nCol2),nRow2(r.nRow2),nTab(r.nTab)
{
}

ScQueryParamTable::~ScQueryParamTable()
{
}

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

ScQueryParam::~ScQueryParam()
{
}

void ScQueryParam::Clear()
{
    nCol1=nCol2 = 0;
    nRow1=nRow2 = 0;
    nTab = SCTAB_MAX;
    eSearchType = utl::SearchParam::SearchType::Normal;
    bHasHeader = bCaseSens = false;
    bInplace = bByRow = bDuplicate = true;

    for (auto & itr : m_Entries)
    {
        itr->Clear();
    }

    ClearDestParams();
}

void ScQueryParam::ClearDestParams()
{
    bDestPers = true;
    nDestTab = 0;
    nDestCol = 0;
    nDestRow = 0;
}

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
    eSearchType = r.eSearchType;
    bDuplicate  = r.bDuplicate;
    bByRow      = r.bByRow;
    bDestPers   = r.bDestPers;

    m_Entries.clear();
    for (auto const& it : r.m_Entries)
    {
        m_Entries.push_back(o3tl::make_unique<ScQueryEntry>(*it));
    }

    return *this;
}

bool ScQueryParam::operator==( const ScQueryParam& rOther ) const
{
    bool bEqual = false;

    // Anzahl der Queries gleich?
    SCSIZE nUsed      = 0;
    SCSIZE nOtherUsed = 0;
    SCSIZE nEntryCount = GetEntryCount();
    SCSIZE nOtherEntryCount = rOther.GetEntryCount();

    while (nUsed<nEntryCount && m_Entries[nUsed]->bDoQuery) ++nUsed;
    while (nOtherUsed<nOtherEntryCount && rOther.m_Entries[nOtherUsed]->bDoQuery)
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
        && (eSearchType == rOther.eSearchType)
        && (bDuplicate  == rOther.bDuplicate)
        && (bDestPers   == rOther.bDestPers)
        && (nDestTab    == rOther.nDestTab)
        && (nDestCol    == rOther.nDestCol)
        && (nDestRow    == rOther.nDestRow) )
    {
        bEqual = true;
        for ( SCSIZE i=0; i<nUsed && bEqual; i++ )
            bEqual = *m_Entries[i] == *rOther.m_Entries[i];
    }
    return bEqual;
}

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
        size_t n = m_Entries.size();
        for (size_t i=0; i<n; i++)
            m_Entries[i]->nField += nDifX;

        bInplace = true;
    }
    else
    {
        OSL_FAIL("MoveToDest, bInplace == TRUE");
    }
}

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
