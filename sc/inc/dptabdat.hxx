/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dptabdat.hxx,v $
 * $Revision: 1.9 $
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
#include "dpcachetable.hxx"

#include <tools/string.hxx>

#include <vector>
#include <set>
#include <hash_map>
#include <hash_set>

namespace com { namespace sun { namespace star { namespace sheet {
    struct DataPilotFieldFilter;
}}}}

class TypedScStrCollection;
class ScSimpleSharedString;

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

struct ScDPItemData
{
    String  aString;
    double  fValue;
    BOOL    bHasValue;

                ScDPItemData() : fValue(0.0), bHasValue(FALSE) {}
                ScDPItemData( const String& rS, double fV = 0.0, BOOL bHV = FALSE ) :
                    aString(rS), fValue(fV), bHasValue( bHV ) {}

    void        SetString( const String& rS ) { aString = rS; bHasValue = FALSE; }
    BOOL        IsCaseInsEqual( const ScDPItemData& r ) const;

    size_t      Hash() const;

    // exact equality
    BOOL        operator==( const ScDPItemData& r ) const;
    // case insensitive equality
    static sal_Int32    Compare( const ScDPItemData& rA, const ScDPItemData& rB );
};

#define SC_VALTYPE_EMPTY    0
#define SC_VALTYPE_VALUE    1
#define SC_VALTYPE_STRING   2
#define SC_VALTYPE_ERROR    3

struct ScDPValueData
{
    double  fValue;
    BYTE    nType;

    void    Set( double fV, BYTE nT ) { fValue = fV; nType = nT; }
};

class ScDPResultMember;
class ScDPDimension;
class ScDPLevel;
class ScDPInitState;
class ScDPResultMember;
class ScDocument;

class ScDPTableData
{
    //  cached data for GetDatePart
    long    nLastDateVal;
    long    nLastHier;
    long    nLastLevel;
    long    nLastRet;
    ScSimpleSharedString& mrSharedString;

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

    long        GetDatePart( long nDateVal, long nHierarchy, long nLevel );

                //! use (new) typed collection instead of ScStrCollection
                //! or separate Str and ValueCollection

    virtual long                    GetColumnCount() = 0;
    virtual const TypedScStrCollection& GetColumnEntries(long nColumn) = 0;
    virtual String                  getDimensionName(long nColumn) = 0;
    virtual BOOL                    getIsDataLayoutDimension(long nColumn) = 0;
    virtual BOOL                    IsDateDimension(long nDim) = 0;
    virtual UINT32                  GetNumberFormat(long nDim);
    virtual void                    DisposeData() = 0;
    virtual void                    SetEmptyFlags( BOOL bIgnoreEmptyRows, BOOL bRepeatIfEmpty ) = 0;

    virtual bool                    IsRepeatIfEmpty();

    virtual void                    CreateCacheTable() = 0;
    virtual void                    FilterCacheTable(const ::std::vector<ScDPCacheTable::Criterion>& rCriteria, const ::std::hash_set<sal_Int32>& rDataDims) = 0;
    virtual void                    GetDrillDownData(const ::std::vector<ScDPCacheTable::Criterion>& rCriteria,
                                                     const ::std::hash_set<sal_Int32>& rCatDims,
                                                     ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >& rData) = 0;
    virtual void                    CalcResults(CalcInfo& rInfo, bool bAutoShow) = 0;
    virtual const ScDPCacheTable&   GetCacheTable() const = 0;

                                    // overloaded in ScDPGroupTableData:
    virtual BOOL                    IsBaseForGroup(long nDim) const;
    virtual long                    GetGroupBase(long nGroupDim) const;
    virtual BOOL                    IsNumOrDateGroup(long nDim) const;
    virtual BOOL                    IsInGroup( const ScDPItemData& rGroupData, long nGroupIndex,
                                               const ScDPItemData& rBaseData, long nBaseIndex ) const;
    virtual BOOL                    HasCommonElement( const ScDPItemData& rFirstData, long nFirstIndex,
                                                      const ScDPItemData& rSecondData, long nSecondIndex ) const;

    ScSimpleSharedString&           GetSharedString();

protected:
    /** This structure stores vector arrays that hold intermediate data for
        each row during cache table iteration. */
    struct CalcRowData
    {
        ::std::vector<ScDPItemData>  aColData;
        ::std::vector<ScDPItemData>  aRowData;
        ::std::vector<ScDPItemData>  aPageData;
        ::std::vector<ScDPValueData> aValues;
    };

    void            FillRowDataFromCacheTable(sal_Int32 nRow, const ScDPCacheTable& rCacheTable, const CalcInfo& rInfo, CalcRowData& rData);
    void            ProcessRowData(CalcInfo& rInfo, CalcRowData& rData, bool bAutoShow);
    void            CalcResultsFromCacheTable(const ScDPCacheTable& rCacheTable, CalcInfo& rInfo, bool bAutoShow);

private:
    void            GetItemData(const ScDPCacheTable& rCacheTable, sal_Int32 nRow,
                                const ::std::vector<long>& rDims, ::std::vector<ScDPItemData>& rItemData);
};


#endif

