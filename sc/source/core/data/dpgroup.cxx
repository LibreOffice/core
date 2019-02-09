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

#include <dpgroup.hxx>

#include <dpcache.hxx>
#include <document.hxx>
#include <dpfilteredcache.hxx>
#include <dputil.hxx>

#include <osl/diagnose.h>
#include <rtl/math.hxx>

#include <com/sun/star/sheet/DataPilotFieldGroupBy.hpp>

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Sequence;

using ::std::vector;
using ::std::shared_ptr;

const sal_uInt16 SC_DP_LEAPYEAR = 1648;     // arbitrary leap year for date calculations

class ScDPGroupNumFilter : public ScDPFilteredCache::FilterBase
{
public:
    ScDPGroupNumFilter(const std::vector<ScDPItemData>& rValues, const ScDPNumGroupInfo& rInfo);

    virtual bool match(const ScDPItemData &rCellData) const override;
    virtual std::vector<ScDPItemData> getMatchValues() const override;
private:
    std::vector<ScDPItemData> const maValues;
    ScDPNumGroupInfo const maNumInfo;
};

ScDPGroupNumFilter::ScDPGroupNumFilter(const std::vector<ScDPItemData>& rValues, const ScDPNumGroupInfo& rInfo) :
    maValues(rValues), maNumInfo(rInfo) {}

bool ScDPGroupNumFilter::match(const ScDPItemData& rCellData) const
{
    if (rCellData.GetType() != ScDPItemData::Value)
        return false;

    for (const auto& rValue : maValues)
    {
        double fVal = rValue.GetValue();
        if (rtl::math::isInf(fVal))
        {
            if (rtl::math::isSignBitSet(fVal))
            {
                // Less than the min value.
                if (rCellData.GetValue() < maNumInfo.mfStart)
                    return true;
            }

            // Greater than the max value.
            if (maNumInfo.mfEnd < rCellData.GetValue())
                return true;

            continue;
        }

        double low = fVal;
        double high = low + maNumInfo.mfStep;
        if (maNumInfo.mbIntegerOnly)
            high += 1.0;

        if (low <= rCellData.GetValue() && rCellData.GetValue() < high)
            return true;
    }

    return false;
}

std::vector<ScDPItemData> ScDPGroupNumFilter::getMatchValues() const
{
    return std::vector<ScDPItemData>();
}

class ScDPGroupDateFilter : public ScDPFilteredCache::FilterBase
{
public:
    ScDPGroupDateFilter(
        const std::vector<ScDPItemData>& rValues, const Date& rNullDate, const ScDPNumGroupInfo& rNumInfo);

    virtual bool match(const ScDPItemData & rCellData) const override;
    virtual std::vector<ScDPItemData> getMatchValues() const override;

private:
    std::vector<ScDPItemData> const maValues;
    Date const             maNullDate;
    ScDPNumGroupInfo const maNumInfo;
};

ScDPGroupDateFilter::ScDPGroupDateFilter(
    const std::vector<ScDPItemData>& rValues, const Date& rNullDate, const ScDPNumGroupInfo& rNumInfo) :
    maValues(rValues),
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

    for (const ScDPItemData& rValue : maValues)
    {
        if (rValue.GetType() != ScDPItemData::GroupValue)
            continue;

        sal_Int32 nGroupType = rValue.GetGroupValue().mnGroupType;
        sal_Int32 nValue = rValue.GetGroupValue().mnValue;

        // Start and end dates are inclusive.  (An end date without a time value
        // is included, while an end date with a time value is not.)

        if (rCellData.GetValue() < maNumInfo.mfStart && !approxEqual(rCellData.GetValue(), maNumInfo.mfStart))
        {
            if (nValue == ScDPItemData::DateFirst)
                return true;
            continue;
        }

        if (rCellData.GetValue() > maNumInfo.mfEnd && !approxEqual(rCellData.GetValue(), maNumInfo.mfEnd))
        {
            if (nValue == ScDPItemData::DateLast)
                return true;
            continue;
        }

        if (nGroupType == DataPilotFieldGroupBy::HOURS || nGroupType == DataPilotFieldGroupBy::MINUTES ||
            nGroupType == DataPilotFieldGroupBy::SECONDS)
        {
            // handle time
            // (do as in the cell functions, ScInterpreter::ScGetHour() etc.)

            sal_uInt16 nHour, nMinute, nSecond;
            double fFractionOfSecond;
            tools::Time::GetClock( rCellData.GetValue(), nHour, nMinute, nSecond, fFractionOfSecond, 0);

            switch (nGroupType)
            {
                case DataPilotFieldGroupBy::HOURS:
                {
                    if (nHour == nValue)
                        return true;
                }
                break;
                case DataPilotFieldGroupBy::MINUTES:
                {
                    if (nMinute == nValue)
                        return true;
                }
                break;
                case DataPilotFieldGroupBy::SECONDS:
                {
                    if (nSecond == nValue)
                        return true;
                }
                break;
                default:
                    OSL_FAIL("invalid time part");
            }

            continue;
        }

        Date date = maNullDate + static_cast<sal_Int32>(approxFloor(rCellData.GetValue()));
        switch (nGroupType)
        {
            case DataPilotFieldGroupBy::YEARS:
            {
                sal_Int32 year = static_cast<sal_Int32>(date.GetYear());
                if (year == nValue)
                    return true;
            }
            break;
            case DataPilotFieldGroupBy::QUARTERS:
            {
                sal_Int32 qtr =  1 + (static_cast<sal_Int32>(date.GetMonth()) - 1) / 3;
                if (qtr == nValue)
                    return true;
            }
            break;
            case DataPilotFieldGroupBy::MONTHS:
            {
                sal_Int32 month = static_cast<sal_Int32>(date.GetMonth());
                if (month == nValue)
                    return true;
            }
            break;
            case DataPilotFieldGroupBy::DAYS:
            {
                Date yearStart(1, 1, date.GetYear());
                sal_Int32 days = (date - yearStart) + 1;       // Jan 01 has value 1
                if (days >= 60 && !date.IsLeapYear())
                {
                    // This is not a leap year.  Adjust the value accordingly.
                    ++days;
                }
                if (days == nValue)
                    return true;
            }
            break;
            default:
                OSL_FAIL("invalid date part");
        }
    }

    return false;
}

std::vector<ScDPItemData> ScDPGroupDateFilter::getMatchValues() const
{
    return std::vector<ScDPItemData>();
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
        case css::sheet::DataPilotFieldGroupBy::MONTHS:
            // a month is only contained in its quarter
            if (nGroupPart == css::sheet::DataPilotFieldGroupBy::QUARTERS)
                // months and quarters are both 1-based
                return (nGroupValue - 1 == (nChildValue - 1) / 3);
            break;
        case css::sheet::DataPilotFieldGroupBy::DAYS:
            // a day is only contained in its quarter or month
            if (nGroupPart == css::sheet::DataPilotFieldGroupBy::MONTHS ||
                nGroupPart == css::sheet::DataPilotFieldGroupBy::QUARTERS)
            {
                Date aDate(1, 1, SC_DP_LEAPYEAR);
                aDate.AddDays(nChildValue - 1);            // days are 1-based
                sal_Int32 nCompare = aDate.GetMonth();
                if (nGroupPart == css::sheet::DataPilotFieldGroupBy::QUARTERS)
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
    return std::any_of(aElements.begin(), aElements.end(),
        [&rData](const ScDPItemData& rElement) { return rElement.IsCaseInsEqual(rData); });
}

bool ScDPGroupItem::HasCommonElement( const ScDPGroupItem& rOther ) const
{
    return std::any_of(aElements.begin(), aElements.end(),
        [&rOther](const ScDPItemData& rElement) { return rOther.HasElement(rElement); });
}

void ScDPGroupItem::FillGroupFilter( ScDPFilteredCache::GroupFilter& rFilter ) const
{
    for (const auto& rElement : aElements)
        rFilter.addMatchItem(rElement);
}

ScDPGroupDimension::ScDPGroupDimension( long nSource, const OUString& rNewName ) :
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
    const ScDPFilteredCache& rCacheTable) const
{
    if (!maMemberEntries.empty())
        return maMemberEntries;

    rCacheTable.getCache().GetGroupDimMemberIds(nGroupDim, maMemberEntries);
    return maMemberEntries;
}

const ScDPGroupItem* ScDPGroupDimension::GetGroupForData( const ScDPItemData& rData ) const
{
    auto aIter = std::find_if(aItems.begin(), aItems.end(),
        [&rData](const ScDPGroupItem& rItem) { return rItem.HasElement(rData); });
    if (aIter != aItems.end())
        return &*aIter;

    return nullptr;
}

const ScDPGroupItem* ScDPGroupDimension::GetGroupForName( const ScDPItemData& rName ) const
{
    auto aIter = std::find_if(aItems.begin(), aItems.end(),
        [&rName](const ScDPGroupItem& rItem) { return rItem.GetName().IsCaseInsEqual(rName); });
    if (aIter != aItems.end())
        return &*aIter;

    return nullptr;
}

const ScDPGroupItem* ScDPGroupDimension::GetGroupByIndex( size_t nIndex ) const
{
    if (nIndex >= aItems.size())
        return nullptr;

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

ScDPNumGroupDimension::~ScDPNumGroupDimension()
{
}

void ScDPNumGroupDimension::SetDateDimension()
{
    aGroupInfo.mbEnable = true;   //TODO: or query both?
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
    pNumGroups.reset( new ScDPNumGroupDimension[nSourceCount] );
}

ScDPGroupTableData::~ScDPGroupTableData()
{
}

void ScDPGroupTableData::AddGroupDimension( const ScDPGroupDimension& rGroup )
{
    ScDPGroupDimension aNewGroup( rGroup );
    aNewGroup.SetGroupDim( GetColumnCount() );      // new dimension will be at the end
    aGroups.push_back( aNewGroup );
}

void ScDPGroupTableData::SetNumGroupDimension( long nIndex, const ScDPNumGroupDimension& rGroup )
{
    if ( nIndex < nSourceCount )
    {
        pNumGroups[nIndex] = rGroup;

        // automatic minimum / maximum is handled in GetNumEntries
    }
}

long ScDPGroupTableData::GetDimensionIndex( const OUString& rName )
{
    for (long i = 0; i < nSourceCount; ++i)                         // nSourceCount excludes data layout
        if (pSourceData->getDimensionName(i) == rName)        //TODO: ignore case?
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
            static_cast<SCCOL>(nColumn), &GetCacheTable().getCache());
    }

    return pSourceData->GetColumnEntries( nColumn );
}

const ScDPItemData* ScDPGroupTableData::GetMemberById( long nDim, long nId )
{
    return pSourceData->GetMemberById( nDim, nId );
}

OUString ScDPGroupTableData::getDimensionName(long nColumn)
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

bool ScDPGroupTableData::getIsDataLayoutDimension(long nColumn)
{
    // position of data layout dimension is moved from source data
    return ( nColumn == sal::static_int_cast<long>( nSourceCount + aGroups.size() ) );    // data layout dimension?
}

bool ScDPGroupTableData::IsDateDimension(long nDim)
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

sal_uInt32 ScDPGroupTableData::GetNumberFormat(long nDim)
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
    for ( auto& rGroup : aGroups )
        rGroup.DisposeData();

    for ( long i=0; i<nSourceCount; i++ )
        pNumGroups[i].DisposeData();

    pSourceData->DisposeData();
}

void ScDPGroupTableData::SetEmptyFlags( bool bIgnoreEmptyRows, bool bRepeatIfEmpty )
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

namespace {

class FindCaseInsensitive
{
    ScDPItemData const maValue;
public:
    explicit FindCaseInsensitive(const ScDPItemData& rVal) : maValue(rVal) {}

    bool operator() (const ScDPItemData& rItem) const
    {
        return maValue.IsCaseInsEqual(rItem);
    }
};

}

void ScDPGroupTableData::ModifyFilterCriteria(vector<ScDPFilteredCache::Criterion>& rCriteria)
{
    // Build dimension ID to object map for group dimensions.
    typedef std::unordered_map<long, const ScDPGroupDimension*> GroupFieldMapType;
    GroupFieldMapType aGroupFieldIds;

    for (const auto& rGroup : aGroups)
    {
        aGroupFieldIds.emplace(rGroup.GetGroupDim(), &rGroup);
    }

    vector<ScDPFilteredCache::Criterion> aNewCriteria;
    aNewCriteria.reserve(rCriteria.size() + aGroups.size());

    // Go through all the filtered field names and process them appropriately.

    const ScDPCache& rCache = GetCacheTable().getCache();
    GroupFieldMapType::const_iterator itrGrpEnd = aGroupFieldIds.end();
    for (const auto& rCriterion : rCriteria)
    {
        std::vector<ScDPItemData> aMatchValues = rCriterion.mpFilter->getMatchValues();

        GroupFieldMapType::const_iterator itrGrp = aGroupFieldIds.find(rCriterion.mnFieldIndex);
        if (itrGrp == itrGrpEnd)
        {
            if (IsNumGroupDimension(rCriterion.mnFieldIndex))
            {
                // internal number group field
                const ScDPNumGroupInfo* pNumInfo = rCache.GetNumGroupInfo(rCriterion.mnFieldIndex);
                if (!pNumInfo)
                    // Number group dimension without num info?  Something is wrong...
                    continue;

                ScDPFilteredCache::Criterion aCri;
                aCri.mnFieldIndex = rCriterion.mnFieldIndex;
                const ScDPNumGroupDimension& rNumGrpDim = pNumGroups[rCriterion.mnFieldIndex];

                if (rNumGrpDim.IsDateDimension())
                {
                    // grouped by dates.
                    aCri.mpFilter.reset(
                        new ScDPGroupDateFilter(
                            aMatchValues, pDoc->GetFormatTable()->GetNullDate(), *pNumInfo));
                }
                else
                {
                    // This dimension is grouped by numeric ranges.
                    aCri.mpFilter.reset(
                        new ScDPGroupNumFilter(aMatchValues, *pNumInfo));
                }

                aNewCriteria.push_back(aCri);
            }
            else
            {
                // This is a regular source field.
                aNewCriteria.push_back(rCriterion);
            }
        }
        else
        {
            // This is an ordinary group field or external number group field.

            const ScDPGroupDimension* pGrpDim = itrGrp->second;
            long nSrcDim = pGrpDim->GetSourceDim();
            long nGrpDim = pGrpDim->GetGroupDim();
            const ScDPNumGroupInfo* pNumInfo = rCache.GetNumGroupInfo(nGrpDim);

            if (pGrpDim->IsDateDimension() && pNumInfo)
            {
                // external number group
                ScDPFilteredCache::Criterion aCri;
                aCri.mnFieldIndex = nSrcDim;  // use the source dimension, not the group dimension.
                aCri.mpFilter.reset(
                    new ScDPGroupDateFilter(
                        aMatchValues, pDoc->GetFormatTable()->GetNullDate(), *pNumInfo));

                aNewCriteria.push_back(aCri);
            }
            else
            {
                // normal group

                ScDPFilteredCache::Criterion aCri;
                aCri.mnFieldIndex = nSrcDim;
                aCri.mpFilter.reset(new ScDPFilteredCache::GroupFilter());
                ScDPFilteredCache::GroupFilter* pGrpFilter =
                    static_cast<ScDPFilteredCache::GroupFilter*>(aCri.mpFilter.get());

                size_t nGroupItemCount = pGrpDim->GetItemCount();
                for (size_t i = 0; i < nGroupItemCount; ++i)
                {
                    const ScDPGroupItem* pGrpItem = pGrpDim->GetGroupByIndex(i);
                    if (!pGrpItem)
                        continue;

                    // Make sure this group name equals one of the match values.
                    if (std::none_of(aMatchValues.begin(), aMatchValues.end(), FindCaseInsensitive(pGrpItem->GetName())))
                        continue;

                    pGrpItem->FillGroupFilter(*pGrpFilter);
                }

                aNewCriteria.push_back(aCri);
            }
        }
    }
    rCriteria.swap(aNewCriteria);
}

void ScDPGroupTableData::FilterCacheTable(const vector<ScDPFilteredCache::Criterion>& rCriteria, const std::unordered_set<sal_Int32>& rCatDims)
{
    vector<ScDPFilteredCache::Criterion> aNewCriteria(rCriteria);
    ModifyFilterCriteria(aNewCriteria);
    pSourceData->FilterCacheTable(aNewCriteria, rCatDims);
}

void ScDPGroupTableData::GetDrillDownData(const vector<ScDPFilteredCache::Criterion>& rCriteria, const std::unordered_set<sal_Int32>& rCatDims, Sequence< Sequence<Any> >& rData)
{
    vector<ScDPFilteredCache::Criterion> aNewCriteria(rCriteria);
    ModifyFilterCriteria(aNewCriteria);
    pSourceData->GetDrillDownData(aNewCriteria, rCatDims, rData);
}

void ScDPGroupTableData::CalcResults(CalcInfo& rInfo, bool bAutoShow)
{
    // #i111435# Inside FillRowDataFromCacheTable/GetItemData, virtual methods
    // getIsDataLayoutDimension and GetSourceDim are used, so it has to be called
    // with original rInfo, containing dimension indexes of the grouped data.

    const ScDPFilteredCache& rCacheTable = pSourceData->GetCacheTable();
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

const ScDPFilteredCache& ScDPGroupTableData::GetCacheTable() const
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

    const ScDPCache& rCache = GetCacheTable().getCache();
    size_t i = 0;
    for (long nColumn : rDims)
    {
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
                        rCache.GetIdByItemData(nColumn, pGroupItem->GetName());
                }
                else
                    rItems[i] = rCache.GetIdByItemData(nColumn, rItem);
            }
        }
        else if ( IsNumGroupDimension( nColumn ) )
        {
            bDateDim = pNumGroups[nColumn].IsDateDimension();
            if (!bDateDim)                         // date is handled below
            {
                const ScDPItemData* pData = rCache.GetItemDataById(nSourceDim, rItems[i]);
                if (pData->GetType() == ScDPItemData::Value)
                {
                    ScDPNumGroupInfo aNumInfo;
                    GetNumGroupInfo(nColumn, aNumInfo);
                    double fGroupValue = ScDPUtil::getNumGroupStartValue(pData->GetValue(), aNumInfo);
                    ScDPItemData aItemData;
                    aItemData.SetRangeStart(fGroupValue);
                    rItems[i] = rCache.GetIdByItemData(nSourceDim, aItemData);
                }
                // else (textual) keep original value
            }
        }

        const ScDPNumGroupInfo* pNumInfo = rCache.GetNumGroupInfo(nColumn);

        if (bDateDim && pNumInfo)
        {
            // This is a date group dimension.
            sal_Int32 nDatePart = rCache.GetGroupType(nColumn);
            const ScDPItemData* pData = rCache.GetItemDataById(nSourceDim, rItems[i]);
            if (pData->GetType() == ScDPItemData::Value)
            {
                SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
                sal_Int32 nPartValue = ScDPUtil::getDatePartValue(
                    pData->GetValue(), pNumInfo, nDatePart, pFormatter);

                ScDPItemData aItem(nDatePart, nPartValue);
                rItems[i] = rCache.GetIdByItemData(nColumn, aItem);
            }
        }

        ++i;
    }
}

bool ScDPGroupTableData::IsBaseForGroup(long nDim) const
{
    return std::any_of(aGroups.begin(), aGroups.end(),
        [&nDim](const ScDPGroupDimension& rDim) { return rDim.GetSourceDim() == nDim; });
}

long ScDPGroupTableData::GetGroupBase(long nGroupDim) const
{
    auto aIter = std::find_if(aGroups.begin(), aGroups.end(),
        [&nGroupDim](const ScDPGroupDimension& rDim) { return rDim.GetGroupDim() == nGroupDim; });
    if (aIter != aGroups.end())
        return aIter->GetSourceDim();

    return -1;      // none
}

bool ScDPGroupTableData::IsNumOrDateGroup(long nDimension) const
{
    // Virtual method from ScDPTableData, used in result data to force text labels.

    if ( nDimension < nSourceCount )
    {
        return pNumGroups[nDimension].GetInfo().mbEnable ||
               pNumGroups[nDimension].IsDateDimension();
    }

    auto aIter = std::find_if(aGroups.begin(), aGroups.end(),
        [&nDimension](const ScDPGroupDimension& rDim) { return rDim.GetGroupDim() == nDimension; });
    if (aIter != aGroups.end())
        return aIter->IsDateDimension();

    return false;
}

bool ScDPGroupTableData::IsInGroup( const ScDPItemData& rGroupData, long nGroupIndex,
                                    const ScDPItemData& rBaseData, long nBaseIndex ) const
{
    auto aIter = std::find_if(aGroups.begin(), aGroups.end(),
        [&nGroupIndex, &nBaseIndex](const ScDPGroupDimension& rDim) {
            return rDim.GetGroupDim() == nGroupIndex && rDim.GetSourceDim() == nBaseIndex; });
    if (aIter != aGroups.end())
    {
        const ScDPGroupDimension& rDim = *aIter;
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

    OSL_FAIL("IsInGroup: no group dimension found");
    return true;
}

bool ScDPGroupTableData::HasCommonElement( const ScDPItemData& rFirstData, long nFirstIndex,
                                         const ScDPItemData& rSecondData, long nSecondIndex ) const
{
    const ScDPGroupDimension* pFirstDim = nullptr;
    const ScDPGroupDimension* pSecondDim = nullptr;
    for ( const auto& rDim : aGroups )
    {
        const ScDPGroupDimension* pDim = &rDim;
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
    if (  nDim >= nSourceCount && nDim < nSourceCount +static_cast<long>(aGroups.size())  )
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
    const ScDPItemData* rItem1 = GetMemberById(nDim, nDataId1);
    const ScDPItemData* rItem2 = GetMemberById(nDim, nDataId2);
    if (rItem1 == nullptr || rItem2 == nullptr)
        return 0;
    return ScDPItemData::Compare( *rItem1,*rItem2);
}

#if DUMP_PIVOT_TABLE

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
