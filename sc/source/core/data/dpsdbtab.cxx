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

#include "dpsdbtab.hxx"
#include "global.hxx"
#include "globstr.hrc"
#include "dpcachetable.hxx"
#include "dptabres.hxx"
#include "document.hxx"
#include "dpobject.hxx"

#include <com/sun/star/sheet/DataImportMode.hpp>
#include <com/sun/star/sdb/CommandType.hpp>

using namespace com::sun::star;

using ::std::vector;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;

sal_Int32 ScImportSourceDesc::GetCommandType() const
{
    sal_Int32 nSdbType = -1;

    switch ( nType )
    {
        case sheet::DataImportMode_SQL:   nSdbType = sdb::CommandType::COMMAND; break;
        case sheet::DataImportMode_TABLE: nSdbType = sdb::CommandType::TABLE;   break;
        case sheet::DataImportMode_QUERY: nSdbType = sdb::CommandType::QUERY;   break;
        default:
            ;
    }
    return nSdbType;
}

const ScDPCache* ScImportSourceDesc::CreateCache(const ScDPDimensionSaveData* pDimData) const
{
    if (!mpDoc)
        return NULL;

    sal_Int32 nSdbType = GetCommandType();
    if (nSdbType < 0)
        return NULL;

    ScDPCollection::DBCaches& rCaches = mpDoc->GetDPCollection()->GetDBCaches();
    return rCaches.getCache(nSdbType, aDBName, aObject, pDimData);
}

ScDatabaseDPData::ScDatabaseDPData(
    ScDocument* pDoc, const ScDPCache* pCache) :
    ScDPTableData(pDoc),
    aCacheTable(pCache)
{
}

ScDatabaseDPData::~ScDatabaseDPData()
{
}

void ScDatabaseDPData::DisposeData()
{
    //! use OpenDatabase here?
    aCacheTable.clear();
}

long ScDatabaseDPData::GetColumnCount()
{
    CreateCacheTable();
    return GetCacheTable().getColSize();
}


rtl::OUString ScDatabaseDPData::getDimensionName(long nColumn)
{
    if (getIsDataLayoutDimension(nColumn))
    {
        //! different internal and display names?
        //return "Data";
        return ScGlobal::GetRscString(STR_PIVOT_DATA);
    }

    CreateCacheTable();
    return aCacheTable.getFieldName(static_cast<SCCOL>(nColumn));
}

sal_Bool ScDatabaseDPData::getIsDataLayoutDimension(long nColumn)
{
    return ( nColumn == GetCacheTable().getColSize());
}

sal_Bool ScDatabaseDPData::IsDateDimension(long /* nDim */)
{
    //! later...
    return false;
}

void ScDatabaseDPData::SetEmptyFlags( sal_Bool /* bIgnoreEmptyRows */, sal_Bool /* bRepeatIfEmpty */ )
{
    //  not used for database data
    //! disable flags
}

void ScDatabaseDPData::CreateCacheTable()
{
    if (!aCacheTable.empty())
        // cache table already created.
        return;

    if (!aCacheTable.hasCache())
    {
        OSL_FAIL("Cache table should be created with a live data cache instance at all times.");
        // This better not happen!!  Cache table should be created with a live
        // data cache instance at all times.
        return;
    }

    aCacheTable.fillTable();
}

void ScDatabaseDPData::FilterCacheTable(const vector<ScDPCacheTable::Criterion>& rCriteria, const boost::unordered_set<sal_Int32>& rCatDims)
{
    CreateCacheTable();
    aCacheTable.filterByPageDimension(
        rCriteria, (IsRepeatIfEmpty() ? rCatDims : boost::unordered_set<sal_Int32>()));
}

void ScDatabaseDPData::GetDrillDownData(const vector<ScDPCacheTable::Criterion>& rCriteria, const boost::unordered_set<sal_Int32>& rCatDims, Sequence< Sequence<Any> >& rData)
{
    CreateCacheTable();
    sal_Int32 nRowSize = aCacheTable.getRowSize();
    if (!nRowSize)
        return;

    aCacheTable.filterTable(
        rCriteria, rData, IsRepeatIfEmpty() ? rCatDims : boost::unordered_set<sal_Int32>());
}

void ScDatabaseDPData::CalcResults(CalcInfo& rInfo, bool bAutoShow)
{
    CreateCacheTable();
    CalcResultsFromCacheTable( aCacheTable, rInfo, bAutoShow);
}

const ScDPCacheTable& ScDatabaseDPData::GetCacheTable() const
{
    return aCacheTable;
}

void ScDatabaseDPData::ReloadCacheTable()
{
    aCacheTable.clear();
    CreateCacheTable();
}

// -----------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
