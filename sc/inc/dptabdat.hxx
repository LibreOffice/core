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

#ifndef SC_DPTABDAT_HXX
#define SC_DPTABDAT_HXX

#include "address.hxx"
#include "dpoutput.hxx"
#include "dpfilteredcache.hxx"
#include "dpcache.hxx"
#include "dpmacros.hxx"

#include "svl/zforlist.hxx"

#include <vector>
#include <set>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/noncopyable.hpp>

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


//
//  base class ScDPTableData to allow implementation with tabular data
//  by deriving only of this
//
#define SC_VALTYPE_EMPTY    0
#define SC_VALTYPE_VALUE    1
#define SC_VALTYPE_STRING   2
#define SC_VALTYPE_ERROR    3

struct ScDPValueData
{
    double  fValue;
    sal_uInt8   nType;

    void    Set( double fV, sal_uInt8 nT ) { fValue = fV; nType = nT; }
};

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

        bool                            bRepeatIfEmpty;

        CalcInfo();
    };

    ScDPTableData(ScDocument* pDoc);
    virtual     ~ScDPTableData();

    rtl::OUString GetFormattedString(long nDim, const ScDPItemData& rItem) const;

    long        GetDatePart( long nDateVal, long nHierarchy, long nLevel );

                //! use (new) typed collection instead of ScStrCollection
                //! or separate Str and ValueCollection

    virtual long                    GetColumnCount() = 0;
    virtual   const std::vector< SCROW >& GetColumnEntries( long nColumn ) ;
    virtual rtl::OUString           getDimensionName(long nColumn) = 0;
    virtual sal_Bool                    getIsDataLayoutDimension(long nColumn) = 0;
    virtual sal_Bool                    IsDateDimension(long nDim) = 0;
    virtual sal_uLong                   GetNumberFormat(long nDim);
    virtual sal_uInt32                  GetNumberFormatByIdx( NfIndexTableOffset );
    virtual void                    DisposeData() = 0;
    virtual void                    SetEmptyFlags( sal_Bool bIgnoreEmptyRows, sal_Bool bRepeatIfEmpty ) = 0;

    virtual bool                    IsRepeatIfEmpty();

    virtual void                    CreateCacheTable() = 0;
    virtual void                    FilterCacheTable(const ::std::vector<ScDPFilteredCache::Criterion>& rCriteria, const ::boost::unordered_set<sal_Int32>& rDataDims) = 0;
    virtual void                    GetDrillDownData(const ::std::vector<ScDPFilteredCache::Criterion>& rCriteria,
                                                     const ::boost::unordered_set<sal_Int32>& rCatDims,
                                                     ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >& rData) = 0;
    virtual void                    CalcResults(CalcInfo& rInfo, bool bAutoShow) = 0;
    virtual const ScDPFilteredCache&   GetCacheTable() const = 0;
    virtual void ReloadCacheTable() = 0;

                                    // overloaded in ScDPGroupTableData:
    virtual sal_Bool                    IsBaseForGroup(long nDim) const;
    virtual long                      GetGroupBase(long nGroupDim) const;
    virtual sal_Bool                    IsNumOrDateGroup(long nDim) const;
    virtual sal_Bool                    IsInGroup( const ScDPItemData& rGroupData, long nGroupIndex,
                                               const ScDPItemData& rBaseData, long nBaseIndex ) const;
    virtual sal_Bool                    HasCommonElement( const ScDPItemData& rFirstData, long nFirstIndex,
                                                      const ScDPItemData& rSecondData, long nSecondIndex ) const;

    virtual long                            GetMembersCount( long nDim );
    virtual const ScDPItemData*   GetMemberByIndex( long nDim, long nIndex );
    virtual const ScDPItemData*   GetMemberById( long nDim, long nId);
    virtual SCROW                        GetIdOfItemData( long  nDim, const ScDPItemData& rData );
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
        ::std::vector<ScDPValueData> aValues;
    };

    void            FillRowDataFromCacheTable(sal_Int32 nRow, const ScDPFilteredCache& rCacheTable, const CalcInfo& rInfo, CalcRowData& rData);
    void            ProcessRowData(CalcInfo& rInfo, CalcRowData& rData, bool bAutoShow);
    void            CalcResultsFromCacheTable(const ScDPFilteredCache& rCacheTable, CalcInfo& rInfo, bool bAutoShow);

private:
    void            GetItemData(const ScDPFilteredCache& rCacheTable, sal_Int32 nRow,
                                          const ::std::vector<long>& rDims, ::std::vector< SCROW >& rItemData);
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
