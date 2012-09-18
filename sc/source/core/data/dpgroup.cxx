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

#include "dpgroup.hxx"

#include "global.hxx"
#include "document.hxx"
#include "dpcachetable.hxx"
#include "dptabsrc.hxx"
#include "dptabres.hxx"
#include "dpobject.hxx"
#include "dpglobal.hxx"
#include "dputil.hxx"

#include <rtl/math.hxx>

#include <com/sun/star/sheet/DataPilotFieldGroupBy.hpp>
#include <com/sun/star/sheet/DataPilotFieldFilter.hpp>
#include <com/sun/star/i18n/CalendarDisplayIndex.hpp>

#include <vector>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::rtl::OUString;
using ::rtl::OUStringHash;

using ::std::vector;
using ::boost::shared_ptr;

#define D_TIMEFACTOR              86400.0

const sal_uInt16 SC_DP_LEAPYEAR = 1648;     // arbitrary leap year for date calculations

class ScDPGroupNumFilter : public ScDPCacheTable::FilterBase
{
public:
    ScDPGroupNumFilter(const ScDPItemData& rValue, const ScDPNumGroupInfo& rInfo);
    virtual ~ScDPGroupNumFilter() {}
    virtual bool match(const ScDPItemData &rCellData) const;
private:
    ScDPItemData maValue;
    ScDPNumGroupInfo maNumInfo;
};

ScDPGroupNumFilter::ScDPGroupNumFilter(const ScDPItemData& rValue, const ScDPNumGroupInfo& rInfo) :
    maValue(rValue), maNumInfo(rInfo) {}

bool ScDPGroupNumFilter::match(const ScDPItemData& rCellData) const
{
    if (rCellData.GetType() != ScDPItemData::Value)
        return false;

    double fVal = maValue.GetValue();
    if (rtl::math::isInf(fVal))
    {
        if (rtl::math::isSignBitSet(fVal))
        {
            // Less than the min value.
            return rCellData.GetValue() < maNumInfo.mfStart;
        }

        // Greater than the max value.
        return maNumInfo.mfEnd < rCellData.GetValue();
    }

    double low = fVal;
    double high = low + maNumInfo.mfStep;
    if (maNumInfo.mbIntegerOnly)
        high += 1.0;

    return low <= rCellData.GetValue() && rCellData.GetValue() < high;
}

class ScDPGroupDateFilter : public ScDPCacheTable::FilterBase
{
public:
    virtual ~ScDPGroupDateFilter() {}
    ScDPGroupDateFilter(
        const ScDPItemData& rValue, const Date& rNullDate, const ScDPNumGroupInfo& rNumInfo);

    virtual bool match(const ScDPItemData & rCellData) const;

private:
    ScDPGroupDateFilter(); // disabled

    ScDPItemData     maValue;
    Date             maNullDate;
    ScDPNumGroupInfo maNumInfo;
};

// ----------------------------------------------------------------------------

ScDPGroupDateFilter::ScDPGroupDateFilter(
    const ScDPItemData& rItem, const Date& rNullDate, const ScDPNumGroupInfo& rNumInfo) :
    maValue(rItem),
    maNullDate(rNullDate),
    maNumInfo(rNumInfo)
{
}

bool ScDPGroupDateFilter::match( const ScDPItemData & rCellData ) const
{
    using namespace ::com::sun::star::sheet;
    using ::rtl::math::approxFloor;
    using ::rtl::math::approxEqual;

    if ( !rCellData.IsValue() )
        return false;

    if (maValue.GetType() != ScDPItemData::GroupValue)
        return false;

    sal_Int32 nGroupType = maValue.GetGroupValue().mnGroupType;
    sal_Int32 nValue = maValue.GetGroupValue().mnValue;

    // Start and end dates are inclusive.  (An end date without a time value
    // is included, while an end date with a time value is not.)

    if ( rCellData.GetValue() < maNumInfo.mfStart && !approxEqual(rCellData.GetValue(), maNumInfo.mfStart) )
    {
        return nValue == ScDPItemData::DateFirst;
    }

    if ( rCellData.GetValue() > maNumInfo.mfEnd && !approxEqual(rCellData.GetValue(), maNumInfo.mfEnd) )
    {
        return nValue == ScDPItemData::DateLast;
    }


    if (nGroupType == DataPilotFieldGroupBy::HOURS || nGroupType == DataPilotFieldGroupBy::MINUTES ||
        nGroupType == DataPilotFieldGroupBy::SECONDS)
    {
        // handle time
        // (as in the cell functions, ScInterpreter::ScGetHour etc.: seconds are rounded)

        double time = rCellData.GetValue() - approxFloor(rCellData.GetValue());
        long seconds = static_cast<long>(approxFloor(time*D_TIMEFACTOR + 0.5));

        switch (nGroupType)
        {
            case DataPilotFieldGroupBy::HOURS:
            {
                sal_Int32 hrs = seconds / 3600;
                return hrs == nValue;
            }
            case DataPilotFieldGroupBy::MINUTES:
            {
                sal_Int32 minutes = (seconds % 3600) / 60;
                return minutes == nValue;
            }
            case DataPilotFieldGroupBy::SECONDS:
            {
                sal_Int32 sec = seconds % 60;
                return sec == nValue;
            }
            default:
                OSL_FAIL("invalid time part");
        }
        return false;
    }

    Date date = maNullDate + static_cast<long>(approxFloor(rCellData.GetValue()));
    switch (nGroupType)
    {
        case DataPilotFieldGroupBy::YEARS:
        {
            sal_Int32 year = static_cast<sal_Int32>(date.GetYear());
            return year == nValue;
        }
        case DataPilotFieldGroupBy::QUARTERS:
        {
            sal_Int32 qtr =  1 + (static_cast<sal_Int32>(date.GetMonth()) - 1) / 3;
            return qtr == nValue;
        }
        case DataPilotFieldGroupBy::MONTHS:
        {
            sal_Int32 month = static_cast<sal_Int32>(date.GetMonth());
            return month == nValue;
        }
        case DataPilotFieldGroupBy::DAYS:
        {
            Date yearStart(1, 1, date.GetYear());
            sal_Int32 days = (date - yearStart) + 1;       // Jan 01 has value 1
            if (days >= 60 && !date.IsLeapYear())
            {
                // This is not a leap year.  Adjust the value accordingly.
                ++days;
            }
            return days == nValue;
        }
        default:
            OSL_FAIL("invalid date part");
    }

    return false;
}

namespace {

bool isDateInGroup(const ScDPItemData& rGroupItem, const ScDPItemData& rChildItem)
{
    if (rGroupItem.GetType() != ScDPItemData::GroupValue || rChildItem.GetType() != ScDPItemData::GroupValue)
        return false;

    sal_Int32 nGroupPart = rGroupItem.GetGroupValue().mnGroupType;
    sal_Int32 nGroupValue = rGroupItem.GetGroupValue().mnValue;
    sal_Int32 nChildPart = rChildItem.GetGroupValue().mnGroupType;
    sal_Int32 nChildValue = rChildItem.GetGroupValue().mnValue;

    if (nGroupValue == ScDPItemData::DateFirst || nGroupValue == ScDPItemData::DateLast ||
        nChildValue == ScDPItemData::DateFirst || nChildValue == ScDPItemData::DateLast)
    {
        // first/last entry matches only itself
        return nGroupValue == nChildValue;
    }

    switch (nChildPart)        // inner part
    {
        case com::sun::star::sheet::DataPilotFieldGroupBy::MONTHS:
            // a month is only contained in its quarter
            if (nGroupPart == com::sun::star::sheet::DataPilotFieldGroupBy::QUARTERS)
                // months and quarters are both 1-based
                return (nGroupValue - 1 == (nChildValue - 1) / 3);

        case com::sun::star::sheet::DataPilotFieldGroupBy::DAYS:
            // a day is only contained in its quarter or month
            if (nGroupPart == com::sun::star::sheet::DataPilotFieldGroupBy::MONTHS ||
                nGroupPart == com::sun::star::sheet::DataPilotFieldGroupBy::QUARTERS)
            {
                Date aDate(1, 1, SC_DP_LEAPYEAR);
                aDate += (nChildValue - 1);            // days are 1-based
                sal_Int32 nCompare = aDate.GetMonth();
                if (nGroupPart == com::sun::star::sheet::DataPilotFieldGroupBy::QUARTERS)
                    nCompare = ( ( nCompare - 1 ) / 3 ) + 1;    // get quarter from date

                return nGroupValue == nCompare;
            }
            break;
        default:
            ;
    }

    return true;
}

}

ScDPGroupItem::ScDPGroupItem( const ScDPItemData& rName ) :
    aGroupName( rName )
{
}

ScDPGroupItem::~ScDPGroupItem()
{
}

void ScDPGroupItem::AddElement( const ScDPItemData& rName )
{
    aElements.push_back( rName );
}

bool ScDPGroupItem::HasElement( const ScDPItemData& rData ) const
{
    for ( ScDPItemDataVec::const_iterator aIter(aElements.begin()); aIter != aElements.end(); ++aIter )
        if ( aIter->IsCaseInsEqual( rData ) )
            return true;

    return false;
}

bool ScDPGroupItem::HasCommonElement( const ScDPGroupItem& rOther ) const
{
    for ( ScDPItemDataVec::const_iterator aIter(aElements.begin()); aIter != aElements.end(); ++aIter )
        if ( rOther.HasElement( *aIter ) )
            return true;

    return false;
}

void ScDPGroupItem::FillGroupFilter( ScDPCacheTable::GroupFilter& rFilter ) const
{
    ScDPItemDataVec::const_iterator itrEnd = aElements.end();
    for (ScDPItemDataVec::const_iterator itr = aElements.begin(); itr != itrEnd; ++itr)
        rFilter.addMatchItem(*itr);
}

// -----------------------------------------------------------------------

ScDPGroupDimension::ScDPGroupDimension( long nSource, const String& rNewName ) :
    nSourceDim( nSource ),
    nGroupDim( -1 ),
    aGroupName( rNewName ),
    mbDateDimension(false)
{
}

ScDPGroupDimension::~ScDPGroupDimension()
{
    maMemberEntries.clear();
}

ScDPGroupDimension::ScDPGroupDimension( const ScDPGroupDimension& rOther ) :
    nSourceDim( rOther.nSourceDim ),
    nGroupDim( rOther.nGroupDim ),
    aGroupName( rOther.aGroupName ),
    aItems( rOther.aItems ),
    mbDateDimension(rOther.mbDateDimension)
{
}

ScDPGroupDimension& ScDPGroupDimension::operator=( const ScDPGroupDimension& rOther )
{
    nSourceDim = rOther.nSourceDim;
    nGroupDim  = rOther.nGroupDim;
    aGroupName = rOther.aGroupName;
    aItems     = rOther.aItems;
    mbDateDimension = rOther.mbDateDimension;
    return *this;
}

void ScDPGroupDimension::AddItem( const ScDPGroupItem& rItem )
{
    aItems.push_back( rItem );
}

void ScDPGroupDimension::SetGroupDim( long nDim )
{
    nGroupDim = nDim;
}

const std::vector<SCROW>& ScDPGroupDimension::GetColumnEntries(
    const ScDPCacheTable& rCacheTable) const
{
    if (!maMemberEntries.empty())
        return maMemberEntries;

    rCacheTable.getCache()->GetGroupDimMemberIds(nGroupDim, maMemberEntries);
    return maMemberEntries;
}



const ScDPGroupItem* ScDPGroupDimension::GetGroupForData( const ScDPItemData& rData ) const
{
    for (ScDPGroupItemVec::const_iterator aIter = aItems.begin(); aIter != aItems.end(); ++aIter)
        if (aIter->HasElement(rData))
            return &*aIter;

    return NULL;
}

const ScDPGroupItem* ScDPGroupDimension::GetGroupForName( const ScDPItemData& rName ) const
{
    for ( ScDPGroupItemVec::const_iterator aIter(aItems.begin()); aIter != aItems.end(); ++aIter )
        if ( aIter->GetName().IsCaseInsEqual( rName ) )
            return &*aIter;

    return NULL;
}

const ScDPGroupItem* ScDPGroupDimension::GetGroupByIndex( size_t nIndex ) const
{
    if (nIndex >= aItems.size())
        return NULL;

    return &aItems[nIndex];
}

void ScDPGroupDimension::DisposeData()
{
    maMemberEntries.clear();
}

void ScDPGroupDimension::SetDateDimension()
{
    mbDateDimension = true;
}

bool ScDPGroupDimension::IsDateDimension() const
{
    return mbDateDimension;
}

// -----------------------------------------------------------------------

ScDPNumGroupDimension::ScDPNumGroupDimension() : mbDateDimension(false) {}

ScDPNumGroupDimension::ScDPNumGroupDimension( const ScDPNumGroupInfo& rInfo ) :
    aGroupInfo(rInfo), mbDateDimension(false) {}

ScDPNumGroupDimension::ScDPNumGroupDimension( const ScDPNumGroupDimension& rOther ) :
    aGroupInfo(rOther.aGroupInfo), mbDateDimension(rOther.mbDateDimension) {}

ScDPNumGroupDimension& ScDPNumGroupDimension::operator=( const ScDPNumGroupDimension& rOther )
{
    aGroupInfo = rOther.aGroupInfo;
    mbDateDimension = rOther.mbDateDimension;
    return *this;
}

void ScDPNumGroupDimension::DisposeData()
{
    aGroupInfo = ScDPNumGroupInfo();
    maMemberEntries.clear();
}

bool ScDPNumGroupDimension::IsDateDimension() const
{
    return mbDateDimension;
}

ScDPNumGroupDimension::~ScDPNumGroupDimension()
{
}

void ScDPNumGroupDimension::SetDateDimension()
{
    aGroupInfo.mbEnable = true;   //! or query both?
    mbDateDimension = true;
}

const std::vector<SCROW>& ScDPNumGroupDimension::GetNumEntries(
    SCCOL nSourceDim, const ScDPCache* pCache) const
{
    if (!maMemberEntries.empty())
        return maMemberEntries;

    pCache->GetGroupDimMemberIds(nSourceDim, maMemberEntries);
    return maMemberEntries;
}

ScDPGroupTableData::ScDPGroupTableData( const shared_ptr<ScDPTableData>& pSource, ScDocument* pDocument ) :
    ScDPTableData(pDocument),
    pSourceData( pSource ),
    pDoc( pDocument )
{
    OSL_ENSURE( pSource, "ScDPGroupTableData: pSource can't be NULL" );

    CreateCacheTable();
    nSourceCount = pSource->GetColumnCount();               // real columns, excluding data layout
    pNumGroups = new ScDPNumGroupDimension[nSourceCount];
}

ScDPGroupTableData::~ScDPGroupTableData()
{
    delete[] pNumGroups;
}

boost::shared_ptr<ScDPTableData> ScDPGroupTableData::GetSourceTableData()
{
    return pSourceData;
}

void ScDPGroupTableData::AddGroupDimension( const ScDPGroupDimension& rGroup )
{
    ScDPGroupDimension aNewGroup( rGroup );
    aNewGroup.SetGroupDim( GetColumnCount() );      // new dimension will be at the end
    aGroups.push_back( aNewGroup );
    aGroupNames.insert(aNewGroup.GetName());
}

void ScDPGroupTableData::SetNumGroupDimension( long nIndex, const ScDPNumGroupDimension& rGroup )
{
    if ( nIndex < nSourceCount )
    {
        pNumGroups[nIndex] = rGroup;

        // automatic minimum / maximum is handled in GetNumEntries
    }
}

long ScDPGroupTableData::GetDimensionIndex( const rtl::OUString& rName )
{
    for (long i = 0; i < nSourceCount; ++i)                         // nSourceCount excludes data layout
        if (pSourceData->getDimensionName(i).equals(rName))        //! ignore case?
            return i;
    return -1;  // none
}

long ScDPGroupTableData::GetColumnCount()
{
    return nSourceCount + aGroups.size();
}

bool ScDPGroupTableData::IsNumGroupDimension( long nDimension ) const
{
    return ( nDimension < nSourceCount && pNumGroups[nDimension].GetInfo().mbEnable );
}

void ScDPGroupTableData::GetNumGroupInfo(long nDimension, ScDPNumGroupInfo& rInfo)
{
    if ( nDimension < nSourceCount )
        rInfo = pNumGroups[nDimension].GetInfo();
}
long  ScDPGroupTableData::GetMembersCount( long nDim )
{
    const std::vector< SCROW >&  members = GetColumnEntries( nDim );
    return members.size();
}
const std::vector< SCROW >& ScDPGroupTableData::GetColumnEntries( long  nColumn )
{
    if ( nColumn >= nSourceCount )
    {
        if ( getIsDataLayoutDimension( nColumn) )     // data layout dimension?
            nColumn = nSourceCount;                         // index of data layout in source data
        else
        {
            const ScDPGroupDimension& rGroupDim = aGroups[nColumn - nSourceCount];
            return rGroupDim.GetColumnEntries( GetCacheTable() );
        }
    }

    if ( IsNumGroupDimension( nColumn ) )
    {
        // dimension number is unchanged for numerical groups
        return pNumGroups[nColumn].GetNumEntries(
            static_cast<SCCOL>(nColumn), GetCacheTable().getCache());
    }

    return pSourceData->GetColumnEntries( nColumn );
}

const ScDPItemData* ScDPGroupTableData::GetMemberById( long nDim, long nId )
{
    return pSourceData->GetMemberById( nDim, nId );
}

rtl::OUString ScDPGroupTableData::getDimensionName(long nColumn)
{
    if ( nColumn >= nSourceCount )
    {
        if ( nColumn == sal::static_int_cast<long>( nSourceCount + aGroups.size() ) )     // data layout dimension?
            nColumn = nSourceCount;                         // index of data layout in source data
        else
            return aGroups[nColumn - nSourceCount].GetName();
    }

    return pSourceData->getDimensionName( nColumn );
}

sal_Bool ScDPGroupTableData::getIsDataLayoutDimension(long nColumn)
{
    // position of data layout dimension is moved from source data
    return ( nColumn == sal::static_int_cast<long>( nSourceCount + aGroups.size() ) );    // data layout dimension?
}

sal_Bool ScDPGroupTableData::IsDateDimension(long nDim)
{
    if ( nDim >= nSourceCount )
    {
        if ( nDim == sal::static_int_cast<long>( nSourceCount + aGroups.size() ) )        // data layout dimension?
            nDim = nSourceCount;                            // index of data layout in source data
        else
            nDim = aGroups[nDim - nSourceCount].GetSourceDim();  // look at original dimension
    }

    return pSourceData->IsDateDimension( nDim );
}

sal_uLong ScDPGroupTableData::GetNumberFormat(long nDim)
{
    if ( nDim >= nSourceCount )
    {
        if ( nDim == sal::static_int_cast<long>( nSourceCount + aGroups.size() ) )        // data layout dimension?
            nDim = nSourceCount;                            // index of data layout in source data
        else
            nDim = aGroups[nDim - nSourceCount].GetSourceDim();  // look at original dimension
    }

    return pSourceData->GetNumberFormat( nDim );
}

void ScDPGroupTableData::DisposeData()
{
    for ( ScDPGroupDimensionVec::iterator aIter(aGroups.begin()); aIter != aGroups.end(); ++aIter )
        aIter->DisposeData();

    for ( long i=0; i<nSourceCount; i++ )
        pNumGroups[i].DisposeData();

    pSourceData->DisposeData();
}

void ScDPGroupTableData::SetEmptyFlags( sal_Bool bIgnoreEmptyRows, sal_Bool bRepeatIfEmpty )
{
    pSourceData->SetEmptyFlags( bIgnoreEmptyRows, bRepeatIfEmpty );
}

bool ScDPGroupTableData::IsRepeatIfEmpty()
{
    return pSourceData->IsRepeatIfEmpty();
}

void ScDPGroupTableData::CreateCacheTable()
{
    pSourceData->CreateCacheTable();
}

void ScDPGroupTableData::ModifyFilterCriteria(vector<ScDPCacheTable::Criterion>& rCriteria)
{
    typedef boost::unordered_map<long, const ScDPGroupDimension*> GroupFieldMapType;
    GroupFieldMapType aGroupFieldIds;
    {
        ScDPGroupDimensionVec::const_iterator itr = aGroups.begin(), itrEnd = aGroups.end();
        for (; itr != itrEnd; ++itr)
            aGroupFieldIds.insert( boost::unordered_map<long, const ScDPGroupDimension*>::value_type(itr->GetGroupDim(), &(*itr)) );
    }

    vector<ScDPCacheTable::Criterion> aNewCriteria;
    aNewCriteria.reserve(rCriteria.size() + aGroups.size());

    // Go through all the filtered field names and process them appropriately.

    const ScDPCache* pCache = GetCacheTable().getCache();
    vector<ScDPCacheTable::Criterion>::const_iterator itrEnd = rCriteria.end();
    GroupFieldMapType::const_iterator itrGrpEnd = aGroupFieldIds.end();
    for (vector<ScDPCacheTable::Criterion>::const_iterator itr = rCriteria.begin(); itr != itrEnd; ++itr)
    {
        ScDPCacheTable::SingleFilter* pFilter = dynamic_cast<ScDPCacheTable::SingleFilter*>(itr->mpFilter.get());
        if (!pFilter)
            // We expect this to be a single filter.
            continue;

        GroupFieldMapType::const_iterator itrGrp = aGroupFieldIds.find(itr->mnFieldIndex);
        if (itrGrp == itrGrpEnd)
        {
            if (IsNumGroupDimension(itr->mnFieldIndex))
            {
                // internal number group field
                ScDPCacheTable::Criterion aCri;
                aCri.mnFieldIndex = itr->mnFieldIndex;
                const ScDPNumGroupDimension& rNumGrpDim = pNumGroups[itr->mnFieldIndex];
                const ScDPNumGroupInfo* pNumInfo = pCache->GetNumGroupInfo(itr->mnFieldIndex);

                if (pNumInfo)
                {
                    if (rNumGrpDim.IsDateDimension())
                    {
                        // grouped by dates.
                        aCri.mpFilter.reset(
                            new ScDPGroupDateFilter(
                                pFilter->getMatchValue(), *pDoc->GetFormatTable()->GetNullDate(), *pNumInfo));
                    }
                    else
                    {
                        // This dimension is grouped by numeric ranges.
                        aCri.mpFilter.reset(
                            new ScDPGroupNumFilter(pFilter->getMatchValue(), *pNumInfo));
                    }
                }

                aNewCriteria.push_back(aCri);
            }
            else
            {
                // This is a regular source field.
                aNewCriteria.push_back(*itr);
            }
        }
        else
        {
            // This is an ordinary group field or external number group field.

            const ScDPGroupDimension* pGrpDim = itrGrp->second;
            long nSrcDim = pGrpDim->GetSourceDim();
            long nGrpDim = pGrpDim->GetGroupDim();
            const ScDPNumGroupInfo* pNumInfo = pCache->GetNumGroupInfo(nGrpDim);

            if (pGrpDim->IsDateDimension() && pNumInfo)
            {
                // external number group
                ScDPCacheTable::Criterion aCri;
                aCri.mnFieldIndex = nSrcDim;  // use the source dimension, not the group dimension.
                aCri.mpFilter.reset(
                    new ScDPGroupDateFilter(
                        pFilter->getMatchValue(), *pDoc->GetFormatTable()->GetNullDate(), *pNumInfo));

                aNewCriteria.push_back(aCri);
            }
            else
            {
                // normal group

                // Note that each group dimension may have multiple group names!
                size_t nGroupItemCount = pGrpDim->GetItemCount();
                for (size_t i = 0; i < nGroupItemCount; ++i)
                {
                    const ScDPGroupItem* pGrpItem = pGrpDim->GetGroupByIndex(i);
                    ScDPItemData aName = pFilter->getMatchValue();

                    if (!pGrpItem || !pGrpItem->GetName().IsCaseInsEqual(aName))
                        continue;

                    ScDPCacheTable::Criterion aCri;
                    aCri.mnFieldIndex = nSrcDim;
                    aCri.mpFilter.reset(new ScDPCacheTable::GroupFilter());
                    ScDPCacheTable::GroupFilter* pGrpFilter =
                        static_cast<ScDPCacheTable::GroupFilter*>(aCri.mpFilter.get());

                    pGrpItem->FillGroupFilter(*pGrpFilter);
                    aNewCriteria.push_back(aCri);
                }
            }
        }
    }
    rCriteria.swap(aNewCriteria);
}

void ScDPGroupTableData::FilterCacheTable(const vector<ScDPCacheTable::Criterion>& rCriteria, const boost::unordered_set<sal_Int32>& rCatDims)
{
    vector<ScDPCacheTable::Criterion> aNewCriteria(rCriteria);
    ModifyFilterCriteria(aNewCriteria);
    pSourceData->FilterCacheTable(aNewCriteria, rCatDims);
}

void ScDPGroupTableData::GetDrillDownData(const vector<ScDPCacheTable::Criterion>& rCriteria, const boost::unordered_set<sal_Int32>& rCatDims, Sequence< Sequence<Any> >& rData)
{
    vector<ScDPCacheTable::Criterion> aNewCriteria(rCriteria);
    ModifyFilterCriteria(aNewCriteria);
    pSourceData->GetDrillDownData(aNewCriteria, rCatDims, rData);
}

void ScDPGroupTableData::CalcResults(CalcInfo& rInfo, bool bAutoShow)
{
    // #i111435# Inside FillRowDataFromCacheTable/GetItemData, virtual methods
    // getIsDataLayoutDimension and GetSourceDim are used, so it has to be called
    // with original rInfo, containing dimension indexes of the grouped data.

    const ScDPCacheTable& rCacheTable = pSourceData->GetCacheTable();
    sal_Int32 nRowSize = rCacheTable.getRowSize();
    for (sal_Int32 nRow = 0; nRow < nRowSize; ++nRow)
    {
        sal_Int32 nLastRow;
        if (!rCacheTable.isRowActive(nRow, &nLastRow))
        {
            nRow = nLastRow;
            continue;
        }

        CalcRowData aData;
        FillRowDataFromCacheTable(nRow, rCacheTable, rInfo, aData);

        if ( !rInfo.aColLevelDims.empty() )
            FillGroupValues(aData.aColData, rInfo.aColLevelDims);
        if ( !rInfo.aRowLevelDims.empty() )
            FillGroupValues(aData.aRowData, rInfo.aRowLevelDims);
        if ( !rInfo.aPageDims.empty() )
            FillGroupValues(aData.aPageData, rInfo.aPageDims);

        ProcessRowData(rInfo, aData, bAutoShow);
    }
}

const ScDPCacheTable& ScDPGroupTableData::GetCacheTable() const
{
    return pSourceData->GetCacheTable();
}

void ScDPGroupTableData::ReloadCacheTable()
{
    pSourceData->ReloadCacheTable();
}

void ScDPGroupTableData::FillGroupValues(vector<SCROW>& rItems, const vector<long>& rDims)
{
    long nGroupedColumns = aGroups.size();

    const ScDPCache* pCache = GetCacheTable().getCache();
    vector<long>::const_iterator it = rDims.begin(), itEnd = rDims.end();
    for (size_t i = 0; it != itEnd; ++it, ++i)
    {
        long nColumn = *it;
        bool bDateDim = false;

        long nSourceDim = nColumn;
        if ( nColumn >= nSourceCount && nColumn < nSourceCount + nGroupedColumns )
        {
            const ScDPGroupDimension& rGroupDim = aGroups[nColumn - nSourceCount];
            nSourceDim= rGroupDim.GetSourceDim();
            bDateDim = rGroupDim.IsDateDimension();
            if (!bDateDim)                         // date is handled below
            {
                const ScDPItemData& rItem = *GetMemberById(nSourceDim, rItems[i]);
                const ScDPGroupItem* pGroupItem = rGroupDim.GetGroupForData(rItem);
                if (pGroupItem)
                {
                    rItems[i] =
                        pCache->GetIdByItemData(nColumn, pGroupItem->GetName());
                }
                else
                    rItems[i] = pCache->GetIdByItemData(nColumn, rItem);
            }
        }
        else if ( IsNumGroupDimension( nColumn ) )
        {
            bDateDim = pNumGroups[nColumn].IsDateDimension();
            if (!bDateDim)                         // date is handled below
            {
                const ScDPItemData* pData = pCache->GetItemDataById(nSourceDim, rItems[i]);
                if (pData->GetType() == ScDPItemData::Value)
                {
                    ScDPNumGroupInfo aNumInfo;
                    GetNumGroupInfo(nColumn, aNumInfo);
                    double fGroupValue = ScDPUtil::getNumGroupStartValue(pData->GetValue(), aNumInfo);
                    ScDPItemData aItemData;
                    aItemData.SetRangeStart(fGroupValue);
                    rItems[i] = pCache->GetIdByItemData(nSourceDim, aItemData);
                }
                // else (textual) keep original value
            }
        }

        const ScDPNumGroupInfo* pNumInfo = pCache->GetNumGroupInfo(nColumn);

        if (bDateDim && pNumInfo)
        {
            // This is a date group dimension.
            sal_Int32 nDatePart = pCache->GetGroupType(nColumn);
            const ScDPItemData* pData = pCache->GetItemDataById(nSourceDim, rItems[i]);
            if (pData->GetType() == ScDPItemData::Value)
            {
                SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
                sal_Int32 nPartValue = ScDPUtil::getDatePartValue(
                    pData->GetValue(), *pNumInfo, nDatePart, pFormatter);
                rtl::OUString aName = ScDPUtil::getDateGroupName(
                    nDatePart, nPartValue, pFormatter, pNumInfo->mfStart, pNumInfo->mfEnd);

                ScDPItemData aItem(nDatePart, nPartValue);
                rItems[i] = pCache->GetIdByItemData(nColumn, aItem);
            }
        }
    }
}

sal_Bool ScDPGroupTableData::IsBaseForGroup(long nDim) const
{
    for ( ScDPGroupDimensionVec::const_iterator aIter(aGroups.begin()); aIter != aGroups.end(); ++aIter )
    {
        const ScDPGroupDimension& rDim = *aIter;
        if ( rDim.GetSourceDim() == nDim )
            return sal_True;
    }

    return false;
}

long ScDPGroupTableData::GetGroupBase(long nGroupDim) const
{
    for ( ScDPGroupDimensionVec::const_iterator aIter(aGroups.begin()); aIter != aGroups.end(); ++aIter )
    {
        const ScDPGroupDimension& rDim = *aIter;
        if ( rDim.GetGroupDim() == nGroupDim )
            return rDim.GetSourceDim();
    }

    return -1;      // none
}

sal_Bool ScDPGroupTableData::IsNumOrDateGroup(long nDimension) const
{
    // Virtual method from ScDPTableData, used in result data to force text labels.

    if ( nDimension < nSourceCount )
    {
        return pNumGroups[nDimension].GetInfo().mbEnable ||
               pNumGroups[nDimension].IsDateDimension();
    }

    for ( ScDPGroupDimensionVec::const_iterator aIter(aGroups.begin()); aIter != aGroups.end(); ++aIter )
    {
        const ScDPGroupDimension& rDim = *aIter;
        if ( rDim.GetGroupDim() == nDimension )
            return rDim.IsDateDimension();
    }

    return false;
}

sal_Bool ScDPGroupTableData::IsInGroup( const ScDPItemData& rGroupData, long nGroupIndex,
                                    const ScDPItemData& rBaseData, long nBaseIndex ) const
{
    for ( ScDPGroupDimensionVec::const_iterator aIter(aGroups.begin()); aIter != aGroups.end(); ++aIter )
    {
        const ScDPGroupDimension& rDim = *aIter;
        if ( rDim.GetGroupDim() == nGroupIndex && rDim.GetSourceDim() == nBaseIndex )
        {
            if (rDim.IsDateDimension())
            {
                return isDateInGroup(rGroupData, rBaseData);
            }
            else
            {
                // If the item is in a group, only that group is valid.
                // If the item is not in any group, its own name is valid.

                const ScDPGroupItem* pGroup = rDim.GetGroupForData( rBaseData );
                return pGroup ? pGroup->GetName().IsCaseInsEqual( rGroupData ) :
                                rGroupData.IsCaseInsEqual( rBaseData );
            }
        }
    }

    OSL_FAIL("IsInGroup: no group dimension found");
    return true;
}

sal_Bool ScDPGroupTableData::HasCommonElement( const ScDPItemData& rFirstData, long nFirstIndex,
                                         const ScDPItemData& rSecondData, long nSecondIndex ) const
{
    const ScDPGroupDimension* pFirstDim = NULL;
    const ScDPGroupDimension* pSecondDim = NULL;
    for ( ScDPGroupDimensionVec::const_iterator aIter(aGroups.begin()); aIter != aGroups.end(); ++aIter )
    {
        const ScDPGroupDimension* pDim = &(*aIter);
        if ( pDim->GetGroupDim() == nFirstIndex )
            pFirstDim = pDim;
        else if ( pDim->GetGroupDim() == nSecondIndex )
            pSecondDim = pDim;
    }
    if ( pFirstDim && pSecondDim )
    {
        bool bFirstDate = pFirstDim->IsDateDimension();
        bool bSecondDate = pSecondDim->IsDateDimension();
        if (bFirstDate || bSecondDate)
        {
            // If one is a date group dimension, the other one must be, too.
            if (!bFirstDate || !bSecondDate)
            {
                OSL_FAIL( "mix of date and non-date groups" );
                return true;
            }

            return isDateInGroup(rFirstData, rSecondData);
        }

        const ScDPGroupItem* pFirstItem = pFirstDim->GetGroupForName( rFirstData );
        const ScDPGroupItem* pSecondItem = pSecondDim->GetGroupForName( rSecondData );
        if ( pFirstItem && pSecondItem )
        {
            // two existing groups -> sal_True if they have a common element
            return pFirstItem->HasCommonElement( *pSecondItem );
        }
        else if ( pFirstItem )
        {
            // "automatic" group contains only its own name
            return pFirstItem->HasElement( rSecondData );
        }
        else if ( pSecondItem )
        {
            // "automatic" group contains only its own name
            return pSecondItem->HasElement( rFirstData );
        }
        else
        {
            // no groups -> sal_True if equal
            return rFirstData.IsCaseInsEqual( rSecondData );
        }
    }

    OSL_FAIL("HasCommonElement: no group dimension found");
    return true;
}

long ScDPGroupTableData::GetSourceDim( long nDim )
{
    if ( getIsDataLayoutDimension( nDim ) )
        return nSourceCount;
    if (  nDim >= nSourceCount && nDim < nSourceCount +(long) aGroups.size()  )
    {
         const ScDPGroupDimension& rGroupDim = aGroups[nDim - nSourceCount];
            return  rGroupDim.GetSourceDim();
    }
    return nDim;
}

long ScDPGroupTableData::Compare(long nDim, long nDataId1, long nDataId2)
{
    if ( getIsDataLayoutDimension(nDim) )
        return 0;
    return ScDPItemData::Compare( *GetMemberById(nDim,  nDataId1),*GetMemberById(nDim,  nDataId2) );
}

#if DEBUG_PIVOT_TABLE
#include <iostream>
using std::cout;
using std::endl;

void ScDPGroupTableData::Dump() const
{
    cout << "--- ScDPGroupTableData" << endl;
    for (long i = 0; i < nSourceCount; ++i)
    {
        cout << "* dimension: " << i << endl;
        const ScDPNumGroupDimension& rGrp = pNumGroups[i];
        rGrp.GetInfo().Dump();
    }
    cout << "---" << endl;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
