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

#include <svl/zforlist.hxx>

#include "dpshttab.hxx"
#include "dptabres.hxx"
#include "document.hxx"
#include "formulacell.hxx"
#include "dpfilteredcache.hxx"
#include "dpobject.hxx"
#include "globstr.hrc"
#include "rangenam.hxx"
#include "queryentry.hxx"

#include <osl/diagnose.h>

#include <vector>
#include <set>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Sequence;
using ::std::vector;

ScSheetDPData::ScSheetDPData(ScDocument* pD, const ScSheetSourceDesc& rDesc, const ScDPCache& rCache) :
    ScDPTableData(pD),
    aQuery ( rDesc.GetQueryParam() ),
    bIgnoreEmptyRows( false ),
    bRepeatIfEmpty(false),
    aCacheTable(rCache)
{
    SCSIZE nEntryCount( aQuery.GetEntryCount());
    for (SCSIZE j = 0; j < nEntryCount; ++j)
    {
        ScQueryEntry& rEntry = aQuery.GetEntry(j);
        if (rEntry.bDoQuery)
        {
            ScQueryEntry::Item& rItem = rEntry.GetQueryItem();
            if (rItem.meType == ScQueryEntry::ByString)
            {
                sal_uInt32 nIndex = 0;
                bool bNumber = pD->GetFormatTable()->IsNumberFormat(
                    rItem.maString.getString(), nIndex, rItem.mfVal);
                rItem.meType = bNumber ? ScQueryEntry::ByValue : ScQueryEntry::ByString;
            }
        }
    }
}

ScSheetDPData::~ScSheetDPData()
{
}

void ScSheetDPData::DisposeData()
{
    aCacheTable.clear();
}

long ScSheetDPData::GetColumnCount()
{
    CreateCacheTable();
    return aCacheTable.getColSize();
}

OUString ScSheetDPData::getDimensionName(long nColumn)
{
    CreateCacheTable();
    if (getIsDataLayoutDimension(nColumn))
    {
        //TODO: different internal and display names?
        //return "Data";
        return ScGlobal::GetRscString(STR_PIVOT_DATA);
    }
    else if (nColumn >= aCacheTable.getColSize())
    {
        OSL_FAIL("getDimensionName: invalid dimension");
        return OUString();
    }
    else
    {
        return aCacheTable.getFieldName(static_cast<SCCOL>(nColumn));
    }
}

bool ScSheetDPData::IsDateDimension(long nDim)
{
    CreateCacheTable();
    long nColCount = aCacheTable.getColSize();
    if (getIsDataLayoutDimension(nDim))
    {
        return false;
    }
    else if (nDim >= nColCount)
    {
        OSL_FAIL("IsDateDimension: invalid dimension");
        return false;
    }
    else
    {
        return GetCacheTable().getCache().IsDateDimension( nDim);
    }
}

sal_uLong ScSheetDPData::GetNumberFormat(long nDim)
{
    CreateCacheTable();
    if (getIsDataLayoutDimension(nDim))
    {
        return 0;
    }
    else if (nDim >= GetCacheTable().getColSize())
    {
        OSL_FAIL("GetNumberFormat: invalid dimension");
        return 0;
    }
    else
    {
        return GetCacheTable().getCache().GetNumberFormat( nDim );
    }
}
sal_uInt32  ScDPTableData::GetNumberFormatByIdx( NfIndexTableOffset eIdx )
{
    if( !mpDoc )
        return 0;

    if ( SvNumberFormatter* pFormatter = mpDoc->GetFormatTable() )
        return pFormatter->GetFormatIndex( eIdx, LANGUAGE_SYSTEM );

    return 0;
}

bool ScSheetDPData::getIsDataLayoutDimension(long nColumn)
{
    CreateCacheTable();
    return (nColumn ==(long)( aCacheTable.getColSize()));
}

void ScSheetDPData::SetEmptyFlags( bool bIgnoreEmptyRowsP, bool bRepeatIfEmptyP )
{
    bIgnoreEmptyRows = bIgnoreEmptyRowsP;
    bRepeatIfEmpty   = bRepeatIfEmptyP;
}

bool ScSheetDPData::IsRepeatIfEmpty()
{
    return bRepeatIfEmpty;
}

void ScSheetDPData::CreateCacheTable()
{
    // Scan and store the data from the source range.
    if (!aCacheTable.empty())
        // already cached.
        return;

    aCacheTable.fillTable(aQuery, bIgnoreEmptyRows, bRepeatIfEmpty);
}

void ScSheetDPData::FilterCacheTable(const vector<ScDPFilteredCache::Criterion>& rCriteria, const std::unordered_set<sal_Int32>& rCatDims)
{
    CreateCacheTable();
    aCacheTable.filterByPageDimension(
        rCriteria, (IsRepeatIfEmpty() ? rCatDims : std::unordered_set<sal_Int32>()));
}

void ScSheetDPData::GetDrillDownData(const vector<ScDPFilteredCache::Criterion>& rCriteria, const std::unordered_set<sal_Int32>& rCatDims, Sequence< Sequence<Any> >& rData)
{
    CreateCacheTable();
    sal_Int32 nRowSize = aCacheTable.getRowSize();
    if (!nRowSize)
        return;

    aCacheTable.filterTable(
        rCriteria, rData, IsRepeatIfEmpty() ? rCatDims : std::unordered_set<sal_Int32>());
}

void ScSheetDPData::CalcResults(CalcInfo& rInfo, bool bAutoShow)
{
    CreateCacheTable();
    CalcResultsFromCacheTable(aCacheTable, rInfo, bAutoShow);
}

const ScDPFilteredCache& ScSheetDPData::GetCacheTable() const
{
    return aCacheTable;
}

void ScSheetDPData::ReloadCacheTable()
{
    aCacheTable.clear();
    CreateCacheTable();
}

#if DUMP_PIVOT_TABLE

void ScSheetDPData::Dump() const
{
    // TODO : Implement this.
}

#endif

ScSheetSourceDesc::ScSheetSourceDesc(ScDocument* pDoc) :
    mpDoc(pDoc) {}

void ScSheetSourceDesc::SetSourceRange(const ScRange& rRange)
{
    maSourceRange = rRange;
    maRangeName.clear(); // overwrite existing range name if any.
}

const ScRange& ScSheetSourceDesc::GetSourceRange() const
{
    if (!maRangeName.isEmpty())
    {
        // Obtain the source range from the range name first.
        maSourceRange = ScRange();
        ScRangeName* pRangeName = mpDoc->GetRangeName();
        do
        {
            if (!pRangeName)
                break;

            OUString aUpper = ScGlobal::pCharClass->uppercase(maRangeName);
            const ScRangeData* pData = pRangeName->findByUpperName(aUpper);
            if (!pData)
                break;

            // range name found.  Fow now, we only use the first token and
            // ignore the rest.
            ScRange aRange;
            if (!pData->IsReference(aRange))
                break;

            maSourceRange = aRange;
        }
        while (false);
    }
    return maSourceRange;
}

void ScSheetSourceDesc::SetRangeName(const OUString& rName)
{
    maRangeName = rName;
}

bool ScSheetSourceDesc::HasRangeName() const
{
    return !maRangeName.isEmpty();
}

void ScSheetSourceDesc::SetQueryParam(const ScQueryParam& rParam)
{
    maQueryParam = rParam;
}

bool ScSheetSourceDesc::operator== (const ScSheetSourceDesc& rOther) const
{
    return maSourceRange == rOther.maSourceRange &&
        maRangeName == rOther.maRangeName &&
        maQueryParam  == rOther.maQueryParam;
}

const ScDPCache* ScSheetSourceDesc::CreateCache(const ScDPDimensionSaveData* pDimData) const
{
    if (!mpDoc)
        return nullptr;

    const char* pErrId = CheckSourceRange();
    if (pErrId)
    {
        OSL_FAIL( "Error Create Cache" );
        return nullptr;
    }

    // All cache instances are managed centrally by ScDPCollection.
    ScDPCollection* pDPs = mpDoc->GetDPCollection();
    if (HasRangeName())
    {
        // Name-based data source.
        ScDPCollection::NameCaches& rCaches = pDPs->GetNameCaches();
        return rCaches.getCache(GetRangeName(), GetSourceRange(), pDimData);
    }

    ScDPCollection::SheetCaches& rCaches = pDPs->GetSheetCaches();
    return rCaches.getCache(GetSourceRange(), pDimData);
}

const char* ScSheetSourceDesc::CheckSourceRange() const
{
    if (!mpDoc)
        return STR_ERR_DATAPILOTSOURCE;

    // Make sure the range is valid and sane.
    const ScRange& rSrcRange = GetSourceRange();
    if (!rSrcRange.IsValid())
        return STR_ERR_DATAPILOTSOURCE;

    if (rSrcRange.aStart.Col() > rSrcRange.aEnd.Col() || rSrcRange.aStart.Row() > rSrcRange.aEnd.Row())
        return STR_ERR_DATAPILOTSOURCE;

    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
