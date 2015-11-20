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

#ifndef INCLUDED_SC_INC_DPTABDAT_HXX
#define INCLUDED_SC_INC_DPTABDAT_HXX

#include "address.hxx"
#include "dpoutput.hxx"
#include "dpfilteredcache.hxx"
#include "dpcache.hxx"
#include "calcmacros.hxx"

#include <svl/zforlist.hxx>

#include <boost/noncopyable.hpp>

#include <set>
#include <unordered_set>
#include <unordered_map>
#include <vector>

namespace com { namespace sun { namespace star { namespace sheet {
    struct DataPilotFieldFilter;
}}}}

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
class ScDPResultMember;
class ScDocument;

/**
 * Base class that abstracts different data source types of a datapilot
 * table.
 */
class SC_DLLPUBLIC ScDPTableData : public ::boost::noncopyable
{
    //  cached data for GetDatePart
    long    nLastDateVal;
    long    nLastHier;
    long    nLastLevel;
    long    nLastRet;
    const ScDocument* mpDoc;
public:

    /** This structure stores dimension information used when calculating
        results.  These data are read only during result calculation, so it
        should be passed as a const instance. */
    struct CalcInfo
    {
        ::std::vector<long>             aColLevelDims;
        ::std::vector<ScDPDimension*>   aColDims;
        ::std::vector<ScDPLevel*>       aColLevels;
        ::std::vector<long>             aRowLevelDims;
        ::std::vector<ScDPDimension*>   aRowDims;
        ::std::vector<ScDPLevel*>       aRowLevels;
        ::std::vector<long>             aPageDims;
        ::std::vector<long>             aDataSrcCols;

        ScDPInitState*                  pInitState;
        ScDPResultMember*               pColRoot;
        ScDPResultMember*               pRowRoot;

        CalcInfo();
    };

    ScDPTableData(ScDocument* pDoc);
    virtual     ~ScDPTableData();

    OUString GetFormattedString(long nDim, const ScDPItemData& rItem) const;

    long        GetDatePart( long nDateVal, long nHierarchy, long nLevel );

                //! use (new) typed collection instead of ScStrCollection
                //! or separate Str and ValueCollection

    virtual long                    GetColumnCount() = 0;
    virtual const std::vector< SCROW >& GetColumnEntries( long nColumn ) ;
    virtual OUString                getDimensionName(long nColumn) = 0;
    virtual bool                    getIsDataLayoutDimension(long nColumn) = 0;
    virtual bool                    IsDateDimension(long nDim) = 0;
    virtual sal_uLong               GetNumberFormat(long nDim);
    sal_uInt32                      GetNumberFormatByIdx( NfIndexTableOffset );
    virtual void                    DisposeData() = 0;
    virtual void                    SetEmptyFlags( bool bIgnoreEmptyRows, bool bRepeatIfEmpty ) = 0;

    virtual bool                    IsRepeatIfEmpty();

    virtual void                    CreateCacheTable() = 0;
    virtual void                    FilterCacheTable(const std::vector<ScDPFilteredCache::Criterion>& rCriteria, const std::unordered_set<sal_Int32>& rDataDims) = 0;
    virtual void                    GetDrillDownData(const std::vector<ScDPFilteredCache::Criterion>& rCriteria,
                                                     const std::unordered_set<sal_Int32>& rCatDims,
                                                     css::uno::Sequence< css::uno::Sequence< css::uno::Any > >& rData) = 0;
    virtual void                    CalcResults(CalcInfo& rInfo, bool bAutoShow) = 0;
    virtual const ScDPFilteredCache&   GetCacheTable() const = 0;
    virtual void ReloadCacheTable() = 0;

                                    // override in ScDPGroupTableData:
    virtual bool                    IsBaseForGroup(long nDim) const;
    virtual long                    GetGroupBase(long nGroupDim) const;
    virtual bool                    IsNumOrDateGroup(long nDim) const;
    virtual bool                    IsInGroup( const ScDPItemData& rGroupData, long nGroupIndex,
                                               const ScDPItemData& rBaseData, long nBaseIndex ) const;
    virtual bool                    HasCommonElement( const ScDPItemData& rFirstData, long nFirstIndex,
                                                      const ScDPItemData& rSecondData, long nSecondIndex ) const;

    virtual long                            GetMembersCount( long nDim );
    const ScDPItemData*   GetMemberByIndex( long nDim, long nIndex );
    virtual const ScDPItemData*   GetMemberById( long nDim, long nId);
    virtual long                GetSourceDim( long nDim );
    virtual long                Compare( long nDim, long nDataId1, long nDataId2);

#if DEBUG_PIVOT_TABLE
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
                                          const ::std::vector<long>& rDims, ::std::vector< SCROW >& rItemData);
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
