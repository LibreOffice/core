/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SC_DPTABDAT_HXX
#define SC_DPTABDAT_HXX

#include "address.hxx"
#include "dpoutput.hxx"
#include "dpcachetable.hxx"
// Wang Xu Ming -- 2009-8-17
// DataPilot Migration - Cache&&Performance
#include "dptablecache.hxx"
// End Comments
#include <tools/string.hxx>

#include <vector>
#include <set>
#include <hash_map>
#include <hash_set>

namespace com { namespace sun { namespace star { namespace sheet {
    struct DataPilotFieldFilter;
}}}}


// -----------------------------------------------------------------------

#define SC_DAPI_DATE_HIERARCHIES    3

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
#define SC_DAPI_LEVEL_WEEK      1
#define SC_DAPI_LEVEL_WEEKDAY   2


// --------------------------------------------------------------------
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

 class SC_DLLPUBLIC ScDPTableData
{
    //  cached data for GetDatePart
    long    nLastDateVal;
    long    nLastHier;
    long    nLastLevel;
    long    nLastRet;
    // Wang Xu Ming -- 2009-8-17
    // DataPilot Migration - Cache&&Performance
    long                          mnCacheId;
    const ScDocument*   mpDoc;
    // End Comments
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

    // Wang Xu Ming -- 2009-8-17
    // DataPilot Migration - Cache&&Performance
    ScDPTableData(ScDocument* pDoc, long nCacheId );
    // End Comments
    virtual     ~ScDPTableData();

    long        GetDatePart( long nDateVal, long nHierarchy, long nLevel );

                //! use (new) typed collection instead of ScStrCollection
                //! or separate Str and ValueCollection

    virtual long                    GetColumnCount() = 0;
    // Wang Xu Ming -- 2009-8-17
    // DataPilot Migration - Cache&&Performance
    virtual   const std::vector< SCROW >& GetColumnEntries( long nColumn ) ;
    long                                                     GetCacheId() const;
    // End Comments
    virtual String                  getDimensionName(long nColumn) = 0;
    virtual sal_Bool                    getIsDataLayoutDimension(long nColumn) = 0;
    virtual sal_Bool                    IsDateDimension(long nDim) = 0;
    virtual sal_uLong                   GetNumberFormat(long nDim);
    virtual sal_uInt32                  GetNumberFormatByIdx( NfIndexTableOffset );
    virtual void                    DisposeData() = 0;
    virtual void                    SetEmptyFlags( sal_Bool bIgnoreEmptyRows, sal_Bool bRepeatIfEmpty ) = 0;

    virtual bool                    IsRepeatIfEmpty();

    virtual void                    CreateCacheTable() = 0;
    virtual void                    FilterCacheTable(const ::std::vector<ScDPCacheTable::Criterion>& rCriteria, const ::std::hash_set<sal_Int32>& rDataDims) = 0;
    virtual void                    GetDrillDownData(const ::std::vector<ScDPCacheTable::Criterion>& rCriteria,
                                                     const ::std::hash_set<sal_Int32>& rCatDims,
                                                     ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >& rData) = 0;
    virtual void                    CalcResults(CalcInfo& rInfo, bool bAutoShow) = 0;
    virtual const ScDPCacheTable&   GetCacheTable() const = 0;

                                    // overloaded in ScDPGroupTableData:
    virtual sal_Bool                    IsBaseForGroup(long nDim) const;
    virtual long                      GetGroupBase(long nGroupDim) const;
    virtual sal_Bool                    IsNumOrDateGroup(long nDim) const;
    virtual sal_Bool                    IsInGroup( const ScDPItemData& rGroupData, long nGroupIndex,
                                               const ScDPItemData& rBaseData, long nBaseIndex ) const;
    virtual sal_Bool                    HasCommonElement( const ScDPItemData& rFirstData, long nFirstIndex,
                                                      const ScDPItemData& rSecondData, long nSecondIndex ) const;

    // Wang Xu Ming -- 2009-8-17
    // DataPilot Migration - Cache&&Performance
    virtual long                            GetMembersCount( long nDim );
    virtual const ScDPItemData*   GetMemberByIndex( long nDim, long nIndex );
    virtual const ScDPItemData*   GetMemberById( long nDim, long nId);
    virtual SCROW                        GetIdOfItemData( long  nDim, const ScDPItemData& rData );
    virtual long                GetSourceDim( long nDim );
    virtual long                Compare( long nDim, long nDataId1, long nDataId2);
    // End Comments
protected:
    /** This structure stores vector arrays that hold intermediate data for
        each row during cache table iteration. */
    struct CalcRowData
    {
        // Wang Xu Ming -- 2009-8-17
        // DataPilot Migration - Cache&&Performance
        ::std::vector< SCROW >  aColData;
        ::std::vector< SCROW >  aRowData;
        ::std::vector< SCROW >  aPageData;
        // End Comments
        ::std::vector<ScDPValueData> aValues;
    };

    void            FillRowDataFromCacheTable(sal_Int32 nRow, const ScDPCacheTable& rCacheTable, const CalcInfo& rInfo, CalcRowData& rData);
    void            ProcessRowData(CalcInfo& rInfo, CalcRowData& rData, bool bAutoShow);
    void            CalcResultsFromCacheTable(const ScDPCacheTable& rCacheTable, CalcInfo& rInfo, bool bAutoShow);

private:
    // Wang Xu Ming -- 2009-8-17
    // DataPilot Migration - Cache&&Performance
    void            GetItemData(const ScDPCacheTable& rCacheTable, sal_Int32 nRow,
                                          const ::std::vector<long>& rDims, ::std::vector< SCROW >& rItemData);
    // End Comments
};
#endif

