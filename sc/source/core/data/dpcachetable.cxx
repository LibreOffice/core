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
// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include "dpcachetable.hxx"
#include "document.hxx"
#include "address.hxx"
#include "cell.hxx"
#include "dptabdat.hxx"
#include "dptabsrc.hxx"
#include "dpobject.hxx"
#include "queryparam.hxx"

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

using ::rtl::OUString;
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


static BOOL lcl_HasQueryEntry( const ScQueryParam& rParam )
{
    return rParam.GetEntryCount() > 0 &&
            rParam.GetEntry(0).bDoQuery;
}

// ----------------------------------------------------------------------------

ScDPCacheTable::FilterItem::FilterItem() :
    mfValue(0.0),
    mbHasValue(false)
{
}

bool  ScDPCacheTable::FilterItem::match( const  ScDPItemData& rCellData ) const
{
    if (rCellData.GetString()!= maString &&
        (!rCellData.IsValue()|| rCellData.GetValue()!=  mfValue))
            return false;
    return true;
}

// ----------------------------------------------------------------------------

ScDPCacheTable::SingleFilter::SingleFilter(String aString, double fValue, bool bHasValue)
{
    maItem.maString = aString;
    maItem.mfValue      = fValue;
    maItem.mbHasValue   = bHasValue;
}

bool ScDPCacheTable::SingleFilter::match( const  ScDPItemData& rCellData ) const
{
      return maItem.match(rCellData);
}

const String& ScDPCacheTable::SingleFilter::getMatchString()
{
    return maItem.maString;
}

double ScDPCacheTable::SingleFilter::getMatchValue() const
{
    return maItem.mfValue;
}

bool ScDPCacheTable::SingleFilter::hasValue() const
{
    return maItem.mbHasValue;
}

// ----------------------------------------------------------------------------

ScDPCacheTable::GroupFilter::GroupFilter()
{
}

bool ScDPCacheTable::GroupFilter::match( const  ScDPItemData& rCellData ) const
{
    vector<FilterItem>::const_iterator itrEnd = maItems.end();
        for (vector<FilterItem>::const_iterator itr = maItems.begin(); itr != itrEnd; ++itr)
        {
            bool bMatch = itr->match( rCellData);
            if (bMatch)
                return  true;
        }
        return false;
}

void ScDPCacheTable::GroupFilter::addMatchItem(const String& rStr, double fVal, bool bHasValue)
{
    FilterItem aItem;
    aItem.maString = rStr;
    aItem.mfValue = fVal;
    aItem.mbHasValue = bHasValue;
    maItems.push_back(aItem);
}

size_t ScDPCacheTable::GroupFilter::getMatchItemCount() const
{
    return maItems.size();
}

// ----------------------------------------------------------------------------

ScDPCacheTable::Criterion::Criterion() :
    mnFieldIndex(-1),
    mpFilter(static_cast<FilterBase*>(NULL))
{
}

// ----------------------------------------------------------------------------

ScDPCacheTable::ScDPCacheTable( ScDocument* pDoc, long nId ) :
    mpCache( NULL ),
    mpNoneCache( NULL )
{
    if ( nId >= 0 )
        mpCache = pDoc->GetDPCollection()->GetDPObjectCache( nId );
    else
    { //create a temp cache object
        initNoneCache( NULL );
    }
}

ScDPCacheTable::~ScDPCacheTable()
{
}

sal_Int32 ScDPCacheTable::getRowSize() const
{
    return getCache()->GetRowCount();
}

sal_Int32 ScDPCacheTable::getColSize() const
{
    return getCache()->GetColumnCount();
}

void ScDPCacheTable::fillTable(
    const ScQueryParam& rQuery, bool* pSpecial, bool bIgnoreEmptyRows, bool bRepeatIfEmpty)
{
    // check cache
    if ( mpCache == NULL )
        initNoneCache( NULL );

   const SCROW  nRowCount = getRowSize();
   const SCCOL  nColCount = (SCCOL) getColSize();
   if ( nRowCount <= 0 || nColCount <= 0)
        return;

    maRowsVisible.clear();
    maRowsVisible.reserve(nRowCount);

    // Initialize field entries container.
    maFieldEntries.clear();
    maFieldEntries.reserve(nColCount);

    // Data rows
    for (SCCOL nCol = 0; nCol < nColCount; ++nCol)
    {
        SCROW nMemCount = getCache()->GetDimMemberCount( nCol );
        if ( nMemCount )
        {
            std::vector< SCROW > pAdded( nMemCount, -1 );

            for (SCROW nRow = 0; nRow < nRowCount; ++nRow )
            {
                SCROW nIndex = getCache()->GetItemDataId( nCol, nRow, bRepeatIfEmpty );
                SCROW nOrder = getCache()->GetOrder( nCol, nIndex );

                if ( nCol == 0 )
                         maRowsVisible.push_back(false);

                if ( lcl_HasQueryEntry(rQuery) &&
                    !getCache()->ValidQuery( nRow , rQuery, pSpecial ) )
                    continue;
                if ( bIgnoreEmptyRows &&  getCache()->IsRowEmpty( nRow ) )
                    continue;
                // Insert a new row into cache table.
                if ( nCol == 0 )
                     maRowsVisible.back() = true;

                pAdded[nOrder] = nIndex;
            }
            maFieldEntries.push_back( vector<SCROW>() );
            for ( SCROW nRow = 0; nRow < nMemCount; nRow++ )
            {
                if ( pAdded[nRow] != -1 )
                    maFieldEntries.back().push_back( pAdded[nRow] );
            }
        }
    }
}

void ScDPCacheTable::fillTable()
{
    // check cache
    if ( mpCache == NULL )
        initNoneCache( NULL );

   const SCROW  nRowCount = getRowSize();
   const SCCOL  nColCount = (SCCOL) getColSize();
   if ( nRowCount <= 0 || nColCount <= 0)
        return;

    maRowsVisible.clear();
    maRowsVisible.reserve(nRowCount);


    // Initialize field entries container.
    maFieldEntries.clear();
    maFieldEntries.reserve(nColCount);

    // Data rows
    for (SCCOL nCol = 0; nCol < nColCount; ++nCol)
    {
        SCROW nMemCount = getCache()->GetDimMemberCount( nCol );
        if ( nMemCount )
        {
            std::vector< SCROW > pAdded( nMemCount, -1 );

            for (SCROW nRow = 0; nRow < nRowCount; ++nRow )
            {
                SCROW nIndex = getCache()->GetItemDataId( nCol, nRow, false );
                SCROW nOrder = getCache()->GetOrder( nCol, nIndex );

                if ( nCol == 0 )
                     maRowsVisible.push_back(true);


                pAdded[nOrder] = nIndex;
            }
            maFieldEntries.push_back( vector<SCROW>() );
            for ( SCROW nRow = 0; nRow < nMemCount; nRow++ )
            {
                if ( pAdded[nRow] != -1 )
                    maFieldEntries.back().push_back( pAdded[nRow] );
            }
        }
    }
}

bool ScDPCacheTable::isRowActive(sal_Int32 nRow) const
{
    if (nRow < 0 || static_cast<size_t>(nRow) >= maRowsVisible.size())
        // row index out of bound
        return false;

    return maRowsVisible[nRow];
}

void ScDPCacheTable::filterByPageDimension(const vector<Criterion>& rCriteria, const boost::unordered_set<sal_Int32>& rRepeatIfEmptyDims)
{
    sal_Int32 nRowSize = getRowSize();
    if (nRowSize != static_cast<sal_Int32>(maRowsVisible.size()))
    {
        // sizes of the two tables differ!
        return;
    }

    for (sal_Int32 nRow = 0; nRow < nRowSize; ++nRow)
        maRowsVisible[nRow] = isRowQualified(nRow, rCriteria, rRepeatIfEmptyDims);
}

const ScDPItemData* ScDPCacheTable::getCell(SCCOL nCol, SCROW nRow, bool bRepeatIfEmpty) const
{
   SCROW nId= getCache()->GetItemDataId(nCol, nRow, bRepeatIfEmpty);
   return getCache()->GetItemDataById( nCol, nId );
}

void  ScDPCacheTable::getValue( ScDPValueData& rVal, SCCOL nCol, SCROW nRow, bool bRepeatIfEmpty) const
{
    const ScDPItemData* pData = getCell( nCol, nRow, bRepeatIfEmpty );

    if (pData)
    {
        rVal.fValue = pData->IsValue() ? pData->GetValue() : 0.0;
        rVal.nType = pData->GetType();
    }
    else
        rVal.Set(0.0, SC_VALTYPE_EMPTY);
}
String ScDPCacheTable::getFieldName(SCCOL  nIndex) const
{
    return getCache()->GetDimensionName( nIndex );
}

const ::std::vector<SCROW>&  ScDPCacheTable::getFieldEntries( sal_Int32 nColumn ) const
{
    if (nColumn < 0 || static_cast<size_t>(nColumn) >= maFieldEntries.size())
    {
        // index out of bound.  Hopefully this code will never be reached.
        static const ::std::vector<SCROW> emptyEntries;
        return emptyEntries;
    }
    return maFieldEntries[nColumn];
}

void ScDPCacheTable::filterTable(const vector<Criterion>& rCriteria, Sequence< Sequence<Any> >& rTabData,
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
        if (!maRowsVisible[nRow])
            // This row is filtered out.
            continue;

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

void ScDPCacheTable::clear()
{
    maFieldEntries.clear();
    maRowsVisible.clear();
}

bool ScDPCacheTable::empty() const
{
    return ( mpCache == NULL&& mpNoneCache == NULL ) || maFieldEntries.size()==0;
}

bool ScDPCacheTable::isRowQualified(sal_Int32 nRow, const vector<Criterion>& rCriteria,
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


void ScDPCacheTable::initNoneCache( ScDocument* pDoc )
{
    mpCache = NULL;
    delete mpNoneCache;
    mpNoneCache = new ScDPTableDataCache( pDoc );
}

ScDPTableDataCache* ScDPCacheTable::getCache() const
{
    if ( mpCache )
        return mpCache;
    return mpNoneCache;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
