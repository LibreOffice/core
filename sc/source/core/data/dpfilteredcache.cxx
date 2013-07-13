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

#include "dpfilteredcache.hxx"
#include "document.hxx"
#include "address.hxx"
#include "formulacell.hxx"
#include "dptabdat.hxx"
#include "dptabsrc.hxx"
#include "dpobject.hxx"
#include "queryparam.hxx"
#include "queryentry.hxx"
#include "dpitemdata.hxx"

#include <com/sun/star/i18n/LocaleDataItem.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/sheet/DataPilotFieldFilter.hpp>
#include <com/sun/star/sheet/DataPilotFieldGroupBy.hpp>

#include <memory>

using namespace ::com::sun::star;

using ::std::vector;
using ::std::pair;
using ::std::auto_ptr;
using ::com::sun::star::i18n::LocaleDataItem;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::sheet::DataPilotFieldFilter;

ScDPFilteredCache::SingleFilter::SingleFilter(const ScDPItemData& rItem) :
    maItem(rItem) {}

bool ScDPFilteredCache::SingleFilter::match(const ScDPItemData& rCellData) const
{
    return maItem == rCellData;
}

std::vector<ScDPItemData> ScDPFilteredCache::SingleFilter::getMatchValues() const
{
    std::vector<ScDPItemData> aValues;
    aValues.push_back(maItem);
    return aValues;
}

const ScDPItemData& ScDPFilteredCache::SingleFilter::getMatchValue() const
{
    return maItem;
}

ScDPFilteredCache::GroupFilter::GroupFilter()
{
}

bool ScDPFilteredCache::GroupFilter::match(const ScDPItemData& rCellData) const
{
    vector<ScDPItemData>::const_iterator it = maItems.begin(), itEnd = maItems.end();
    for (; it != itEnd; ++it)
    {
        bool bMatch = *it == rCellData;
        if (bMatch)
            return true;
    }
    return false;
}

std::vector<ScDPItemData> ScDPFilteredCache::GroupFilter::getMatchValues() const
{
    return maItems;
}

void ScDPFilteredCache::GroupFilter::addMatchItem(const ScDPItemData& rItem)
{
    maItems.push_back(rItem);
}

size_t ScDPFilteredCache::GroupFilter::getMatchItemCount() const
{
    return maItems.size();
}

// ----------------------------------------------------------------------------

ScDPFilteredCache::Criterion::Criterion() :
    mnFieldIndex(-1),
    mpFilter(static_cast<FilterBase*>(NULL))
{
}

// ----------------------------------------------------------------------------

ScDPFilteredCache::ScDPFilteredCache(const ScDPCache& rCache) :
    maShowByFilter(0, MAXROW+1, false), maShowByPage(0, MAXROW+1, true), mrCache(rCache)
{
}

ScDPFilteredCache::~ScDPFilteredCache()
{
}

sal_Int32 ScDPFilteredCache::getRowSize() const
{
    return mrCache.GetRowCount();
}

sal_Int32 ScDPFilteredCache::getColSize() const
{
    return mrCache.GetColumnCount();
}

void ScDPFilteredCache::fillTable(
    const ScQueryParam& rQuery, bool bIgnoreEmptyRows, bool bRepeatIfEmpty)
{
    SCROW nRowCount = getRowSize();
    SCROW nDataSize = mrCache.GetDataSize();
    SCCOL nColCount = getColSize();
    if (nRowCount <= 0 || nColCount <= 0)
        return;

    maShowByFilter.clear();
    maShowByPage.clear();
    maShowByPage.build_tree();

    // Process the non-empty data rows.
    for (SCROW nRow = 0; nRow < nDataSize; ++nRow)
    {
        if (!getCache()->ValidQuery(nRow, rQuery))
            continue;

        if (bIgnoreEmptyRows && getCache()->IsRowEmpty(nRow))
            continue;

        maShowByFilter.insert_back(nRow, nRow+1, true);
    }

    // Process the trailing empty rows.
    if (!bIgnoreEmptyRows)
        maShowByFilter.insert_back(nDataSize, nRowCount, true);

    maShowByFilter.build_tree();

    // Initialize field entries container.
    maFieldEntries.clear();
    maFieldEntries.reserve(nColCount);

    // Build unique field entries.
    for (SCCOL nCol = 0; nCol < nColCount; ++nCol)
    {
        maFieldEntries.push_back( vector<SCROW>() );
        SCROW nMemCount = getCache()->GetDimMemberCount( nCol );
        if (!nMemCount)
            continue;

        std::vector<SCROW> aAdded(nMemCount, -1);
        bool bShow = false;
        SCROW nEndSegment = -1;
        for (SCROW nRow = 0; nRow < nRowCount; ++nRow)
        {
            if (nRow > nEndSegment)
            {
                if (!maShowByFilter.search_tree(nRow, bShow, NULL, &nEndSegment).second)
                {
                    OSL_FAIL("Tree search failed!");
                    continue;
                }
                --nEndSegment; // End position is not inclusive. Move back one.
            }

            if (!bShow)
            {
                nRow = nEndSegment;
                continue;
            }

            SCROW nIndex = getCache()->GetItemDataId(nCol, nRow, bRepeatIfEmpty);
            SCROW nOrder = getOrder(nCol, nIndex);
            aAdded[nOrder] = nIndex;
        }
        for (SCROW nRow = 0; nRow < nMemCount; ++nRow)
        {
            if (aAdded[nRow] != -1)
                maFieldEntries.back().push_back(aAdded[nRow]);
        }
    }
}

void ScDPFilteredCache::fillTable()
{
    SCROW nRowCount = getRowSize();
    SCCOL nColCount = getColSize();
    if (nRowCount <= 0 || nColCount <= 0)
        return;

    maShowByPage.clear();
    maShowByPage.build_tree();

    maShowByFilter.clear();
    maShowByFilter.insert_front(0, nRowCount, true);
    maShowByFilter.build_tree();

    // Initialize field entries container.
    maFieldEntries.clear();
    maFieldEntries.reserve(nColCount);

    // Data rows
    for (SCCOL nCol = 0; nCol < nColCount; ++nCol)
    {
        maFieldEntries.push_back( vector<SCROW>() );
        SCROW nMemCount = getCache()->GetDimMemberCount( nCol );
        if (!nMemCount)
            continue;

        std::vector<SCROW> aAdded(nMemCount, -1);

        for (SCROW nRow = 0; nRow < nRowCount; ++nRow)
        {
            SCROW nIndex = getCache()->GetItemDataId(nCol, nRow, false);
            SCROW nOrder = getOrder(nCol, nIndex);
            aAdded[nOrder] = nIndex;
        }
        for (SCROW nRow = 0; nRow < nMemCount; ++nRow)
        {
            if (aAdded[nRow] != -1)
                maFieldEntries.back().push_back(aAdded[nRow]);
        }
    }
}

bool ScDPFilteredCache::isRowActive(sal_Int32 nRow, sal_Int32* pLastRow) const
{
    bool bFilter = false, bPage = true;
    SCROW nLastRowFilter = MAXROW, nLastRowPage = MAXROW;
    maShowByFilter.search_tree(nRow, bFilter, NULL, &nLastRowFilter);
    maShowByPage.search_tree(nRow, bPage, NULL, &nLastRowPage);
    if (pLastRow)
    {
        // Return the last row of current segment.
        *pLastRow = nLastRowFilter < nLastRowPage ? nLastRowFilter : nLastRowPage;
        *pLastRow -= 1; // End position is not inclusive. Move back one.
    }

    return bFilter && bPage;
}

void ScDPFilteredCache::filterByPageDimension(const vector<Criterion>& rCriteria, const boost::unordered_set<sal_Int32>& rRepeatIfEmptyDims)
{
    SCROW nRowSize = getRowSize();

    maShowByPage.clear();

    for (SCROW nRow = 0; nRow < nRowSize; ++nRow)
    {
        bool bShow = isRowQualified(nRow, rCriteria, rRepeatIfEmptyDims);
        maShowByPage.insert_back(nRow, nRow+1, bShow);
    }

    maShowByPage.build_tree();
}

const ScDPItemData* ScDPFilteredCache::getCell(SCCOL nCol, SCROW nRow, bool bRepeatIfEmpty) const
{
   SCROW nId= mrCache.GetItemDataId(nCol, nRow, bRepeatIfEmpty);
   return mrCache.GetItemDataById( nCol, nId );
}

void  ScDPFilteredCache::getValue( ScDPValue& rVal, SCCOL nCol, SCROW nRow, bool bRepeatIfEmpty) const
{
    const ScDPItemData* pData = getCell( nCol, nRow, bRepeatIfEmpty );

    if (pData)
    {
        rVal.mfValue = pData->IsValue() ? pData->GetValue() : 0.0;
        rVal.meType = pData->GetCellType();
    }
    else
        rVal.Set(0.0, ScDPValue::Empty);
}

OUString ScDPFilteredCache::getFieldName(SCCOL nIndex) const
{
    return mrCache.GetDimensionName(nIndex);
}

const ::std::vector<SCROW>&  ScDPFilteredCache::getFieldEntries( sal_Int32 nColumn ) const
{
    if (nColumn < 0 || static_cast<size_t>(nColumn) >= maFieldEntries.size())
    {
        // index out of bound.  Hopefully this code will never be reached.
        static const ::std::vector<SCROW> emptyEntries;
        return emptyEntries;
    }
    return maFieldEntries[nColumn];
}

void ScDPFilteredCache::filterTable(const vector<Criterion>& rCriteria, Sequence< Sequence<Any> >& rTabData,
                                 const boost::unordered_set<sal_Int32>& rRepeatIfEmptyDims)
{
    sal_Int32 nRowSize = getRowSize();
    sal_Int32 nColSize = getColSize();

    if (!nRowSize)
        // no data to filter.
        return;

    // Row first, then column.
    vector< Sequence<Any> > tableData;
    tableData.reserve(nRowSize+1);

    // Header first.
    Sequence<Any> headerRow(nColSize);
    for (SCCOL  nCol = 0; nCol < nColSize; ++nCol)
    {
        OUString str;
        str = getFieldName( nCol);
        Any any;
        any <<= str;
        headerRow[nCol] = any;
    }
    tableData.push_back(headerRow);

    for (sal_Int32 nRow = 0; nRow < nRowSize; ++nRow)
    {
        sal_Int32 nLastRow;
        if (!isRowActive(nRow, &nLastRow))
        {
            // This row is filtered out.
            nRow = nLastRow;
            continue;
        }

        if (!isRowQualified(nRow, rCriteria, rRepeatIfEmptyDims))
            continue;

        // Insert this row into table.

        Sequence<Any> row(nColSize);
        for (SCCOL nCol = 0; nCol < nColSize; ++nCol)
        {
            Any any;
            bool bRepeatIfEmpty = rRepeatIfEmptyDims.count(nCol) > 0;
            const ScDPItemData* pData= getCell(nCol, nRow, bRepeatIfEmpty);
            if ( pData->IsValue() )
                any <<= pData->GetValue();
            else
            {
                  OUString string (pData->GetString() );
                  any <<= string;
            }
            row[nCol] = any;
        }
        tableData.push_back(row);
    }

    // convert vector to Seqeunce
    sal_Int32 nTabSize = static_cast<sal_Int32>(tableData.size());
    rTabData.realloc(nTabSize);
    for (sal_Int32 i = 0; i < nTabSize; ++i)
        rTabData[i] = tableData[i];
}

SCROW ScDPFilteredCache::getOrder(long nDim, SCROW nIndex) const
{
    return mrCache.GetOrder(nDim, nIndex);
}

void ScDPFilteredCache::clear()
{
    maFieldEntries.clear();
    maShowByFilter.clear();
    maShowByPage.clear();
}

bool ScDPFilteredCache::empty() const
{
    return maFieldEntries.empty();
}

bool ScDPFilteredCache::isRowQualified(sal_Int32 nRow, const vector<Criterion>& rCriteria,
                                    const boost::unordered_set<sal_Int32>& rRepeatIfEmptyDims) const
{
    sal_Int32 nColSize = getColSize();
    vector<Criterion>::const_iterator itrEnd = rCriteria.end();
    for (vector<Criterion>::const_iterator itr = rCriteria.begin(); itr != itrEnd; ++itr)
    {
        if (itr->mnFieldIndex >= nColSize)
            // specified field is outside the source data columns.  Don't
            // use this criterion.
            continue;

        // Check if the 'repeat if empty' flag is set for this field.
        bool bRepeatIfEmpty = rRepeatIfEmptyDims.count(itr->mnFieldIndex) > 0;
        const ScDPItemData* pCellData = getCell(static_cast<SCCOL>(itr->mnFieldIndex), nRow, bRepeatIfEmpty);
        if (!itr->mpFilter->match(*pCellData))
            return false;
    }
    return true;
}

const ScDPCache* ScDPFilteredCache::getCache() const
{
    return &mrCache;
}

#if DEBUG_PIVOT_TABLE
using std::cout;
using std::endl;

void ScDPFilteredCache::dumpRowFlag(const RowFlagType& rFlag) const
{
    RowFlagType::const_iterator it = rFlag.begin(), itEnd = rFlag.end();
    bool bShow = it->second;
    SCROW nRow1 = it->first;
    for (++it; it != itEnd; ++it)
    {
        SCROW nRow2 = it->first;
        cout << "  * range " << nRow1 << "-" << nRow2 << ": " << (bShow ? "on" : "off") << endl;
        bShow = it->second;
        nRow1 = nRow2;
    }
}

void ScDPFilteredCache::dump() const
{
    cout << "--- pivot filtered cache dump" << endl;

    cout << endl;
    cout << "* show by filter" << endl;
    dumpRowFlag(maShowByFilter);

    cout << endl;
    cout << "* show by page dimensions" << endl;
    dumpRowFlag(maShowByPage);

    cout << endl;
    cout << "* field entries" << endl;
    size_t nFieldCount = maFieldEntries.size();
    for (size_t i = 0; i < nFieldCount; ++i)
    {
        const vector<SCROW>& rField = maFieldEntries[i];
        cout << "  * field " << i << endl;
        for (size_t j = 0, n = rField.size(); j < n; ++j)
            cout << "    ID: " << rField[j] << endl;
    }
    cout << "---" << endl;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
