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

#include <svl/zforlist.hxx>

#include "dpshttab.hxx"
#include "dptabres.hxx"
#include "document.hxx"
#include "cell.hxx"
#include "dpcachetable.hxx"
#include "dpobject.hxx"
#include "globstr.hrc"
#include "rangenam.hxx"
#include "queryentry.hxx"

#include <com/sun/star/sheet/DataPilotFieldFilter.hpp>

#include <vector>
#include <set>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;
using ::std::vector;

// -----------------------------------------------------------------------

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
                    rItem.maString, nIndex, rItem.mfVal);
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

rtl::OUString ScSheetDPData::getDimensionName(long nColumn)
{
    CreateCacheTable();
    if (getIsDataLayoutDimension(nColumn))
    {
        //! different internal and display names?
        //return "Data";
        return ScGlobal::GetRscString(STR_PIVOT_DATA);
    }
    else if (nColumn >= aCacheTable.getColSize())
    {
        OSL_FAIL("getDimensionName: invalid dimension");
        return rtl::OUString();
    }
    else
    {
        return aCacheTable.getFieldName(static_cast<SCCOL>(nColumn));
    }
}

sal_Bool ScSheetDPData::IsDateDimension(long nDim)
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
        return GetCacheTable().getCache()->IsDateDimension( nDim);
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
        return GetCacheTable().getCache()->GetNumberFormat( nDim );
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

sal_Bool ScSheetDPData::getIsDataLayoutDimension(long nColumn)
{
    CreateCacheTable();
    return (nColumn ==(long)( aCacheTable.getColSize()));
}

void ScSheetDPData::SetEmptyFlags( sal_Bool bIgnoreEmptyRowsP, sal_Bool bRepeatIfEmptyP )
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

void ScSheetDPData::FilterCacheTable(const vector<ScDPCacheTable::Criterion>& rCriteria, const boost::unordered_set<sal_Int32>& rCatDims)
{
    CreateCacheTable();
    aCacheTable.filterByPageDimension(
        rCriteria, (IsRepeatIfEmpty() ? rCatDims : boost::unordered_set<sal_Int32>()));
}

void ScSheetDPData::GetDrillDownData(const vector<ScDPCacheTable::Criterion>& rCriteria, const boost::unordered_set<sal_Int32>& rCatDims, Sequence< Sequence<Any> >& rData)
{
    CreateCacheTable();
    sal_Int32 nRowSize = aCacheTable.getRowSize();
    if (!nRowSize)
        return;

    aCacheTable.filterTable(
        rCriteria, rData, IsRepeatIfEmpty() ? rCatDims : boost::unordered_set<sal_Int32>());
}

void ScSheetDPData::CalcResults(CalcInfo& rInfo, bool bAutoShow)
{
    CreateCacheTable();
    CalcResultsFromCacheTable(aCacheTable, rInfo, bAutoShow);
}

const ScDPCacheTable& ScSheetDPData::GetCacheTable() const
{
    return aCacheTable;
}

void ScSheetDPData::ReloadCacheTable()
{
    aCacheTable.clear();
    CreateCacheTable();
}

ScSheetSourceDesc::ScSheetSourceDesc(ScDocument* pDoc) :
    mpDoc(pDoc) {}

void ScSheetSourceDesc::SetSourceRange(const ScRange& rRange)
{
    maSourceRange = rRange;
    maRangeName = OUString(); // overwrite existing range name if any.
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

const OUString& ScSheetSourceDesc::GetRangeName() const
{
    return maRangeName;
}

bool ScSheetSourceDesc::HasRangeName() const
{
    return !maRangeName.isEmpty();
}

void ScSheetSourceDesc::SetQueryParam(const ScQueryParam& rParam)
{
    maQueryParam = rParam;
}

const ScQueryParam& ScSheetSourceDesc::GetQueryParam() const
{
    return maQueryParam;
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
        return NULL;

    sal_uLong nErrId = CheckSourceRange();
    if (nErrId)
    {
        OSL_FAIL( "Error Create Cache\n" );
        return NULL;
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

sal_uLong ScSheetSourceDesc::CheckSourceRange() const
{
    if (!mpDoc)
        return STR_ERR_DATAPILOTSOURCE;

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
