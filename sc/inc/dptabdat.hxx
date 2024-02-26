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

#pragma once

#include "dpfilteredcache.hxx"
#include "calcmacros.hxx"

#include <svl/zforlist.hxx>
#include <tools/long.hxx>

#include <unordered_set>
#include <vector>

#define SC_DAPI_HIERARCHY_FLAT      0
#define SC_DAPI_HIERARCHY_QUARTER   1
#define SC_DAPI_HIERARCHY_WEEK      2

#define SC_DAPI_FLAT_LEVELS     1       // single level for flat dates
#define SC_DAPI_QUARTER_LEVELS  4       // levels in year/quarter/month/day hierarchy
#define SC_DAPI_WEEK_LEVELS     3       // levels in year/week/day hierarchy

#define SC_DAPI_LEVEL_YEAR      0
#define SC_DAPI_LEVEL_QUARTER   1
#define SC_DAPI_LEVEL_MONTH     2
#define SC_DAPI_LEVEL_DAY       3
#define SC_DAPI_LEVEL_WEEK      4
#define SC_DAPI_LEVEL_WEEKDAY   5

class ScDPResultMember;
class ScDPDimension;
class ScDPLevel;
class ScDPInitState;
class ScDocument;

/**
 * Base class that abstracts different data source types of a datapilot
 * table.
 */
class SAL_DLLPUBLIC_RTTI ScDPTableData
{
    //  cached data for GetDatePart
    tools::Long    nLastDateVal;
    tools::Long    nLastHier;
    tools::Long    nLastLevel;
    tools::Long    nLastRet;
    const ScDocument* mpDoc;
public:

    /** This structure stores dimension information used when calculating
        results.  These data are read only during result calculation, so it
        should be passed as a const instance. */
    struct CalcInfo
    {
        ::std::vector<sal_Int32>        aColLevelDims;
        ::std::vector<ScDPDimension*>   aColDims;
        ::std::vector<ScDPLevel*>       aColLevels;
        ::std::vector<sal_Int32>        aRowLevelDims;
        ::std::vector<ScDPDimension*>   aRowDims;
        ::std::vector<ScDPLevel*>       aRowLevels;
        ::std::vector<sal_Int32>        aPageDims;
        ::std::vector<sal_Int32>        aDataSrcCols;

        ScDPInitState*                  pInitState;
        ScDPResultMember*               pColRoot;
        ScDPResultMember*               pRowRoot;

        CalcInfo();
    };

    ScDPTableData(const ScDPTableData&) = delete;
    const ScDPTableData& operator=(const ScDPTableData&) = delete;
    ScDPTableData(const ScDocument* pDoc);
    virtual     ~ScDPTableData();

    OUString GetFormattedString(sal_Int32 nDim, const ScDPItemData& rItem, bool bLocaleIndependent) const;

    tools::Long        GetDatePart( tools::Long nDateVal, tools::Long nHierarchy, tools::Long nLevel );

                //! use (new) typed collection instead of ScStrCollection
                //! or separate Str and ValueCollection

    virtual sal_Int32               GetColumnCount() = 0;
    virtual const std::vector< SCROW >& GetColumnEntries( sal_Int32 nColumn ) ;
    virtual OUString                getDimensionName(sal_Int32 nColumn) = 0;
    virtual bool                    getIsDataLayoutDimension(sal_Int32 nColumn) = 0;
    virtual bool                    IsDateDimension(sal_Int32 nDim) = 0;
    virtual sal_uInt32              GetNumberFormat(sal_Int32 nDim);
    sal_uInt32                      GetNumberFormatByIdx( NfIndexTableOffset );
    virtual void                    DisposeData() = 0;
    virtual void                    SetEmptyFlags( bool bIgnoreEmptyRows, bool bRepeatIfEmpty ) = 0;

    virtual bool                    IsRepeatIfEmpty();

    virtual void                    CreateCacheTable() = 0;
    virtual void                    FilterCacheTable(std::vector<ScDPFilteredCache::Criterion>&& rCriteria, std::unordered_set<sal_Int32>&& rDataDims) = 0;
    virtual void                    GetDrillDownData(std::vector<ScDPFilteredCache::Criterion>&& rCriteria,
                                                    std::unordered_set<sal_Int32>&& rCatDims,
                                                     css::uno::Sequence< css::uno::Sequence< css::uno::Any > >& rData) = 0;
    virtual void                    CalcResults(CalcInfo& rInfo, bool bAutoShow) = 0;
    virtual const ScDPFilteredCache&   GetCacheTable() const = 0;
    virtual void ReloadCacheTable() = 0;

                                    // override in ScDPGroupTableData:
    virtual bool                    IsBaseForGroup(sal_Int32 nDim) const;
    virtual sal_Int32               GetGroupBase(sal_Int32 nGroupDim) const;
    virtual bool                    IsNumOrDateGroup(sal_Int32 nDim) const;
    virtual bool                    IsInGroup( const ScDPItemData& rGroupData, sal_Int32 nGroupIndex,
                                               const ScDPItemData& rBaseData, sal_Int32 nBaseIndex ) const;
    virtual bool                    HasCommonElement( const ScDPItemData& rFirstData, sal_Int32 nFirstIndex,
                                                      const ScDPItemData& rSecondData, sal_Int32     nSecondIndex ) const;

    virtual sal_Int32               GetMembersCount( sal_Int32 nDim );
    const ScDPItemData*   GetMemberByIndex( sal_Int32 nDim, sal_Int32 nIndex );
    SC_DLLPUBLIC virtual const ScDPItemData*   GetMemberById( sal_Int32 nDim, sal_Int32 nId);
    virtual sal_Int32                GetSourceDim( sal_Int32 nDim );
    virtual sal_Int32                Compare( sal_Int32 nDim, sal_Int32 nDataId1, sal_Int32 nDataId2);

#if DUMP_PIVOT_TABLE
    virtual void Dump() const;
#endif

protected:
    /** This structure stores vector arrays that hold intermediate data for
        each row during cache table iteration. */
    struct CalcRowData
    {
        ::std::vector< SCROW >  aColData;
        ::std::vector< SCROW >  aRowData;
        ::std::vector< SCROW >  aPageData;
        ::std::vector<ScDPValue> aValues;
    };

    void            FillRowDataFromCacheTable(sal_Int32 nRow, const ScDPFilteredCache& rCacheTable, const CalcInfo& rInfo, CalcRowData& rData);
    static void     ProcessRowData(CalcInfo& rInfo, const CalcRowData& rData, bool bAutoShow);
    void            CalcResultsFromCacheTable(const ScDPFilteredCache& rCacheTable, CalcInfo& rInfo, bool bAutoShow);

private:
    void            GetItemData(const ScDPFilteredCache& rCacheTable, sal_Int32 nRow,
                                          const ::std::vector<sal_Int32>& rDims, ::std::vector< SCROW >& rItemData);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
