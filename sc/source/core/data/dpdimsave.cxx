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

#include <dpcache.hxx>
#include <dpdimsave.hxx>
#include <dpgroup.hxx>
#include <dpobject.hxx>
#include <dputil.hxx>
#include <document.hxx>

#include <com/sun/star/sheet/DataPilotFieldGroupBy.hpp>

#include <svl/zforlist.hxx>
#include <osl/diagnose.h>
#include <rtl/math.hxx>
#include <algorithm>

#include <global.hxx>
#include <globstr.hrc>
#include <scresid.hxx>

using namespace com::sun::star;

ScDPSaveGroupItem::ScDPSaveGroupItem( const OUString& rName ) :
    aGroupName(rName) {}

ScDPSaveGroupItem::~ScDPSaveGroupItem() {}

void ScDPSaveGroupItem::AddElement( const OUString& rName )
{
    aElements.push_back(rName);
}

void ScDPSaveGroupItem::AddElementsFromGroup( const ScDPSaveGroupItem& rGroup )
{
    // add all elements of the other group (used for nested grouping)

    for ( std::vector<OUString>::const_iterator aIter(rGroup.aElements.begin());
                                aIter != rGroup.aElements.end(); ++aIter )
        aElements.push_back( *aIter );
}

bool ScDPSaveGroupItem::RemoveElement( const OUString& rName )
{
    for (std::vector<OUString>::iterator aIter = aElements.begin(); aIter != aElements.end(); ++aIter)
        if (*aIter == rName)          //TODO: ignore case
        {
            aElements.erase(aIter);   // found -> remove
            return true;                // don't have to look further
        }

    return false;   // not found
}

bool ScDPSaveGroupItem::IsEmpty() const
{
    return aElements.empty();
}

size_t ScDPSaveGroupItem::GetElementCount() const
{
    return aElements.size();
}

const OUString* ScDPSaveGroupItem::GetElementByIndex(size_t nIndex) const
{
    return (nIndex < aElements.size()) ? &aElements[ nIndex ] : nullptr;
}

void ScDPSaveGroupItem::Rename( const OUString& rNewName )
{
    aGroupName = rNewName;
}

void ScDPSaveGroupItem::RemoveElementsFromGroups( ScDPSaveGroupDimension& rDimension ) const
{
    // remove this group's elements from their groups in rDimension
    // (rDimension must be a different dimension from the one which contains this)

    for ( std::vector<OUString>::const_iterator aIter(aElements.begin()); aIter != aElements.end(); ++aIter )
        rDimension.RemoveFromGroups( *aIter );
}

void ScDPSaveGroupItem::ConvertElementsToItems(SvNumberFormatter* pFormatter) const
{
    maItems.reserve(aElements.size());
    std::vector<OUString>::const_iterator it = aElements.begin(), itEnd = aElements.end();
    for (; it != itEnd; ++it)
    {
        sal_uInt32 nFormat = 0;
        double fValue;
        ScDPItemData aData;
        if (pFormatter->IsNumberFormat(*it, nFormat, fValue))
            aData.SetValue(fValue);
        else
            aData.SetString(*it);

        maItems.push_back(aData);
    }
}

bool ScDPSaveGroupItem::HasInGroup(const ScDPItemData& rItem) const
{
    return std::find(maItems.begin(), maItems.end(), rItem) != maItems.end();
}

void ScDPSaveGroupItem::AddToData(ScDPGroupDimension& rDataDim) const
{
    ScDPGroupItem aGroup(aGroupName);
    std::vector<ScDPItemData>::const_iterator it = maItems.begin(), itEnd = maItems.end();
    for (; it != itEnd; ++it)
        aGroup.AddElement(*it);

    rDataDim.AddItem(aGroup);
}

ScDPSaveGroupDimension::ScDPSaveGroupDimension( const OUString& rSource, const OUString& rName ) :
    aSourceDim( rSource ),
    aGroupDimName( rName ),
    nDatePart( 0 )
{
}

ScDPSaveGroupDimension::ScDPSaveGroupDimension( const OUString& rSource, const OUString& rName, const ScDPNumGroupInfo& rDateInfo, sal_Int32 nPart ) :
    aSourceDim( rSource ),
    aGroupDimName( rName ),
    aDateInfo( rDateInfo ),
    nDatePart( nPart )
{
}

void ScDPSaveGroupDimension::SetDateInfo( const ScDPNumGroupInfo& rInfo, sal_Int32 nPart )
{
    aDateInfo = rInfo;
    nDatePart = nPart;
}

void ScDPSaveGroupDimension::AddGroupItem( const ScDPSaveGroupItem& rItem )
{
    aGroups.push_back( rItem );
}

OUString ScDPSaveGroupDimension::CreateGroupName(const OUString& rPrefix)
{
    // create a name for a new group, using "Group1", "Group2" etc. (translated prefix in rPrefix)

    //TODO: look in all dimensions, to avoid clashes with automatic groups (=name of base element)?
    //TODO: (only dimensions for the same base)

    sal_Int32 nAdd = 1;                                 // first try is "Group1"
    const sal_Int32 nMaxAdd = nAdd + aGroups.size();    // limit the loop
    while ( nAdd <= nMaxAdd )
    {
        OUString aGroupName = rPrefix + OUString::number( nAdd );
        bool bExists = false;

        // look for existing groups
        for ( ScDPSaveGroupItemVec::const_iterator aIter(aGroups.begin());
                                    aIter != aGroups.end() && !bExists; ++aIter )
            if (aIter->GetGroupName() == aGroupName)         //TODO: ignore case
                bExists = true;

        if ( !bExists )
            return aGroupName;          // found a new name

        ++nAdd;                         // continue with higher number
    }

    OSL_FAIL("CreateGroupName: no valid name found");
    return OUString();
}

const ScDPSaveGroupItem* ScDPSaveGroupDimension::GetNamedGroup( const OUString& rGroupName ) const
{
    return const_cast< ScDPSaveGroupDimension* >( this )->GetNamedGroupAcc( rGroupName );
}

ScDPSaveGroupItem* ScDPSaveGroupDimension::GetNamedGroupAcc( const OUString& rGroupName )
{
    for (ScDPSaveGroupItemVec::iterator aIter = aGroups.begin(); aIter != aGroups.end(); ++aIter)
        if (aIter->GetGroupName() == rGroupName)         //TODO: ignore case
            return &*aIter;

    return nullptr;        // none found
}

long ScDPSaveGroupDimension::GetGroupCount() const
{
    return aGroups.size();
}

const ScDPSaveGroupItem& ScDPSaveGroupDimension::GetGroupByIndex( long nIndex ) const
{
    return aGroups[nIndex];
}


void ScDPSaveGroupDimension::RemoveFromGroups( const OUString& rItemName )
{
    //  if the item is in any group, remove it from the group,
    //  also remove the group if it is empty afterwards

    for ( ScDPSaveGroupItemVec::iterator aIter(aGroups.begin()); aIter != aGroups.end(); ++aIter )
        if ( aIter->RemoveElement( rItemName ) )
        {
            if ( aIter->IsEmpty() )         // removed last item from the group?
                aGroups.erase( aIter );     // then remove the group

            return;     // don't have to look further
        }
}

void ScDPSaveGroupDimension::RemoveGroup(const OUString& rGroupName)
{
    for (ScDPSaveGroupItemVec::iterator aIter = aGroups.begin(); aIter != aGroups.end(); ++aIter)
        if (aIter->GetGroupName() == rGroupName)          //TODO: ignore case
        {
            aGroups.erase( aIter );
            return;                     // don't have to look further
        }
}

bool ScDPSaveGroupDimension::IsEmpty() const
{
    return aGroups.empty();
}

bool ScDPSaveGroupDimension::HasOnlyHidden(const ScDPUniqueStringSet& rVisible)
{
    // check if there are only groups that don't appear in the list of visible names

    bool bAllHidden = true;
    for (ScDPSaveGroupItemVec::const_iterator aIter = aGroups.begin(); aIter != aGroups.end() && bAllHidden; ++aIter)
    {
        if (rVisible.count(aIter->GetGroupName()) > 0)
            bAllHidden = false;
    }
    return bAllHidden;
}

void ScDPSaveGroupDimension::Rename( const OUString& rNewName )
{
    aGroupDimName = rNewName;
}

bool ScDPSaveGroupDimension::IsInGroup(const ScDPItemData& rItem) const
{
    ScDPSaveGroupItemVec::const_iterator it = aGroups.begin(), itEnd = aGroups.end();
    for (; it != itEnd; ++it)
    {
        if (it->HasInGroup(rItem))
            return true;
    }
    return false;
}

namespace {

inline bool isInteger(double fValue)
{
    return rtl::math::approxEqual(fValue, rtl::math::approxFloor(fValue));
}

void fillDateGroupDimension(
    ScDPCache& rCache, ScDPNumGroupInfo& rDateInfo, long nSourceDim, long nGroupDim,
    sal_Int32 nDatePart, const SvNumberFormatter* pFormatter)
{
    // Auto min/max is only used for "Years" part, but the loop is always
    // needed.
    double fSourceMin = 0.0;
    double fSourceMax = 0.0;
    bool bFirst = true;

    const ScDPCache::ScDPItemDataVec& rItems = rCache.GetDimMemberValues(nSourceDim);
    ScDPCache::ScDPItemDataVec::const_iterator it = rItems.begin(), itEnd = rItems.end();
    for (; it != itEnd; ++it)
    {
        const ScDPItemData& rItem = *it;
        if (rItem.GetType() != ScDPItemData::Value)
            continue;

        double fVal = rItem.GetValue();
        if (bFirst)
        {
            fSourceMin = fSourceMax = fVal;
            bFirst = false;
        }
        else
        {
            if (fVal < fSourceMin)
                fSourceMin = fVal;
            if ( fVal > fSourceMax )
                fSourceMax = fVal;
        }
    }

    // For the start/end values, use the same date rounding as in
    // ScDPNumGroupDimension::GetNumEntries (but not for the list of
    // available years).
    if (rDateInfo.mbAutoStart)
        rDateInfo.mfStart = rtl::math::approxFloor(fSourceMin);
    if (rDateInfo.mbAutoEnd)
        rDateInfo.mfEnd = rtl::math::approxFloor(fSourceMax) + 1;

    //TODO: if not automatic, limit fSourceMin/fSourceMax for list of year values?

    long nStart = 0, nEnd = 0; // end is inclusive

    switch (nDatePart)
    {
        case sheet::DataPilotFieldGroupBy::YEARS:
            nStart = ScDPUtil::getDatePartValue(
                fSourceMin, nullptr, sheet::DataPilotFieldGroupBy::YEARS, pFormatter);
            nEnd = ScDPUtil::getDatePartValue(fSourceMax, nullptr, sheet::DataPilotFieldGroupBy::YEARS, pFormatter);
            break;
        case sheet::DataPilotFieldGroupBy::QUARTERS: nStart = 1; nEnd = 4;   break;
        case sheet::DataPilotFieldGroupBy::MONTHS:   nStart = 1; nEnd = 12;  break;
        case sheet::DataPilotFieldGroupBy::DAYS:     nStart = 1; nEnd = 366; break;
        case sheet::DataPilotFieldGroupBy::HOURS:    nStart = 0; nEnd = 23;  break;
        case sheet::DataPilotFieldGroupBy::MINUTES:  nStart = 0; nEnd = 59;  break;
        case sheet::DataPilotFieldGroupBy::SECONDS:  nStart = 0; nEnd = 59;  break;
        default:
            OSL_FAIL("invalid date part");
    }

    // Now, populate the group items in the cache.
    rCache.ResetGroupItems(nGroupDim, rDateInfo, nDatePart);

    for (long nValue = nStart; nValue <= nEnd; ++nValue)
        rCache.SetGroupItem(nGroupDim, ScDPItemData(nDatePart, nValue));

    // add first/last entry (min/max)
    rCache.SetGroupItem(nGroupDim, ScDPItemData(nDatePart, ScDPItemData::DateFirst));
    rCache.SetGroupItem(nGroupDim, ScDPItemData(nDatePart, ScDPItemData::DateLast));
}

}

void ScDPSaveGroupDimension::AddToData( ScDPGroupTableData& rData ) const
{
    long nSourceIndex = rData.GetDimensionIndex( aSourceDim );
    if ( nSourceIndex >= 0 )
    {
        ScDPGroupDimension aDim( nSourceIndex, aGroupDimName );
        if ( nDatePart )
        {
            // date grouping

            aDim.SetDateDimension();
        }
        else
        {
            // normal (manual) grouping

            for (ScDPSaveGroupItemVec::const_iterator aIter(aGroups.begin()); aIter != aGroups.end(); ++aIter)
                aIter->AddToData(aDim);
        }

        rData.AddGroupDimension( aDim );
    }
}

void ScDPSaveGroupDimension::AddToCache(ScDPCache& rCache) const
{
    long nSourceDim = rCache.GetDimensionIndex(aSourceDim);
    if (nSourceDim < 0)
        return;

    long nDim = rCache.AppendGroupField();
    SvNumberFormatter* pFormatter = rCache.GetDoc()->GetFormatTable();

    if (nDatePart)
    {
        fillDateGroupDimension(rCache, aDateInfo, nSourceDim, nDim, nDatePart, pFormatter);
        return;
    }

    rCache.ResetGroupItems(nDim, aDateInfo, 0);
    {
        ScDPSaveGroupItemVec::const_iterator it = aGroups.begin(), itEnd = aGroups.end();
        for (; it != itEnd; ++it)
        {
            const ScDPSaveGroupItem& rGI = *it;
            rGI.ConvertElementsToItems(pFormatter);
            rCache.SetGroupItem(nDim, ScDPItemData(rGI.GetGroupName()));
        }
    }

    const ScDPCache::ScDPItemDataVec& rItems = rCache.GetDimMemberValues(nSourceDim);
    {
        ScDPCache::ScDPItemDataVec::const_iterator it = rItems.begin(), itEnd = rItems.end();
        for (; it != itEnd; ++it)
        {
            const ScDPItemData& rItem = *it;
            if (!IsInGroup(rItem))
                // Not in any group.  Add as its own group.
                rCache.SetGroupItem(nDim, rItem);
        }
    }
}

ScDPSaveNumGroupDimension::ScDPSaveNumGroupDimension( const OUString& rName, const ScDPNumGroupInfo& rInfo ) :
    aDimensionName( rName ),
    aGroupInfo( rInfo ),
    nDatePart( 0 )
{
}

ScDPSaveNumGroupDimension::ScDPSaveNumGroupDimension( const OUString& rName, const ScDPNumGroupInfo& rDateInfo, sal_Int32 nPart ) :
    aDimensionName( rName ),
    aDateInfo( rDateInfo ),
    nDatePart( nPart )
{
}

void ScDPSaveNumGroupDimension::AddToData( ScDPGroupTableData& rData ) const
{
    long nSource = rData.GetDimensionIndex( aDimensionName );
    if ( nSource >= 0 )
    {
        ScDPNumGroupDimension aDim( aGroupInfo );           // aGroupInfo: value grouping
        if ( nDatePart )
            aDim.SetDateDimension();

        rData.SetNumGroupDimension( nSource, aDim );
    }
}

void ScDPSaveNumGroupDimension::AddToCache(ScDPCache& rCache) const
{
    long nDim = rCache.GetDimensionIndex(aDimensionName);
    if (nDim < 0)
        return;

    if (aDateInfo.mbEnable)
    {
        // Date grouping
        SvNumberFormatter* pFormatter = rCache.GetDoc()->GetFormatTable();
        fillDateGroupDimension(rCache, aDateInfo, nDim, nDim, nDatePart, pFormatter);
    }
    else if (aGroupInfo.mbEnable)
    {
        // Number-range grouping

        // Look through the source entries for non-integer numbers, minimum
        // and maximum.

        // non-integer GroupInfo values count, too
        aGroupInfo.mbIntegerOnly =
            (aGroupInfo.mbAutoStart || isInteger(aGroupInfo.mfStart)) &&
            (aGroupInfo.mbAutoEnd || isInteger(aGroupInfo.mfEnd)) &&
            isInteger(aGroupInfo.mfStep);

        double fSourceMin = 0.0;
        double fSourceMax = 0.0;
        bool bFirst = true;

        const ScDPCache::ScDPItemDataVec& rItems = rCache.GetDimMemberValues(nDim);
        ScDPCache::ScDPItemDataVec::const_iterator it = rItems.begin(), itEnd = rItems.end();
        for (; it != itEnd; ++it)
        {
            const ScDPItemData& rItem = *it;
            if (rItem.GetType() != ScDPItemData::Value)
                continue;

            double fValue = rItem.GetValue();
            if (bFirst)
            {
                fSourceMin = fSourceMax = fValue;
                bFirst = false;
                continue;
            }

            if (fValue < fSourceMin)
                fSourceMin = fValue;
            if (fValue > fSourceMax)
                fSourceMax = fValue;

            if (aGroupInfo.mbIntegerOnly && !isInteger(fValue))
            {
                // If any non-integer numbers are involved, the group labels
                // are shown including their upper limit.
                aGroupInfo.mbIntegerOnly = false;
            }
        }

        if (aGroupInfo.mbDateValues)
        {
            // special handling for dates: always integer, round down limits
            aGroupInfo.mbIntegerOnly = true;
            fSourceMin = rtl::math::approxFloor(fSourceMin);
            fSourceMax = rtl::math::approxFloor(fSourceMax) + 1;
        }

        if (aGroupInfo.mbAutoStart)
            aGroupInfo.mfStart = fSourceMin;
        if (aGroupInfo.mbAutoEnd)
            aGroupInfo.mfEnd = fSourceMax;

        //TODO: limit number of entries?

        long nLoopCount = 0;
        double fLoop = aGroupInfo.mfStart;

        rCache.ResetGroupItems(nDim, aGroupInfo, 0);

        // Use "less than" instead of "less or equal" for the loop - don't
        // create a group that consists only of the end value. Instead, the
        // end value is then included in the last group (last group is bigger
        // than the others). The first group has to be created nonetheless.
        // GetNumGroupForValue has corresponding logic.

        bool bFirstGroup = true;
        while (bFirstGroup || (fLoop < aGroupInfo.mfEnd && !rtl::math::approxEqual(fLoop, aGroupInfo.mfEnd)))
        {
            ScDPItemData aItem;
            aItem.SetRangeStart(fLoop);
            rCache.SetGroupItem(nDim, aItem);
            ++nLoopCount;
            fLoop = aGroupInfo.mfStart + nLoopCount * aGroupInfo.mfStep;
            bFirstGroup = false;

            // ScDPItemData values are compared with approxEqual
        }

        ScDPItemData aItem;
        aItem.SetRangeFirst();
        rCache.SetGroupItem(nDim, aItem);

        aItem.SetRangeLast();
        rCache.SetGroupItem(nDim, aItem);
    }
}

void ScDPSaveNumGroupDimension::SetGroupInfo( const ScDPNumGroupInfo& rNew )
{
    aGroupInfo = rNew;
}

void ScDPSaveNumGroupDimension::SetDateInfo( const ScDPNumGroupInfo& rInfo, sal_Int32 nPart )
{
    aDateInfo = rInfo;
    nDatePart = nPart;
}

namespace {

struct ScDPSaveGroupDimNameFunc
{
    OUString       maDimName;
    explicit     ScDPSaveGroupDimNameFunc( const OUString& rDimName ) : maDimName( rDimName ) {}
    bool         operator()( const ScDPSaveGroupDimension& rGroupDim ) const { return rGroupDim.GetGroupDimName() == maDimName; }
};

struct ScDPSaveGroupSourceNameFunc
{
    OUString       maSrcDimName;
    explicit     ScDPSaveGroupSourceNameFunc( const OUString& rSrcDimName ) : maSrcDimName( rSrcDimName ) {}
    bool         operator()( const ScDPSaveGroupDimension& rGroupDim ) const { return rGroupDim.GetSourceDimName() == maSrcDimName; }
};

} // namespace

ScDPDimensionSaveData::ScDPDimensionSaveData()
{
}

bool ScDPDimensionSaveData::operator==( const ScDPDimensionSaveData& ) const
{
    return false;
}

void ScDPDimensionSaveData::AddGroupDimension( const ScDPSaveGroupDimension& rGroupDim )
{
    OSL_ENSURE( ::std::none_of( maGroupDims.begin(), maGroupDims.end(), ScDPSaveGroupDimNameFunc( rGroupDim.GetGroupDimName() ) ),
        "ScDPDimensionSaveData::AddGroupDimension - group dimension exists already" );
    // ReplaceGroupDimension() adds new or replaces existing
    ReplaceGroupDimension( rGroupDim );
}

void ScDPDimensionSaveData::ReplaceGroupDimension( const ScDPSaveGroupDimension& rGroupDim )
{
    ScDPSaveGroupDimVec::iterator aIt = ::std::find_if(
        maGroupDims.begin(), maGroupDims.end(), ScDPSaveGroupDimNameFunc( rGroupDim.GetGroupDimName() ) );
    if( aIt == maGroupDims.end() )
        maGroupDims.push_back( rGroupDim );
    else
        *aIt = rGroupDim;
}

void ScDPDimensionSaveData::RemoveGroupDimension( const OUString& rGroupDimName )
{
    ScDPSaveGroupDimVec::iterator aIt = ::std::find_if(
        maGroupDims.begin(), maGroupDims.end(), ScDPSaveGroupDimNameFunc( rGroupDimName ) );
    if( aIt != maGroupDims.end() )
        maGroupDims.erase( aIt );
}

void ScDPDimensionSaveData::AddNumGroupDimension( const ScDPSaveNumGroupDimension& rGroupDim )
{
    OSL_ENSURE( maNumGroupDims.count( rGroupDim.GetDimensionName() ) == 0,
        "ScDPDimensionSaveData::AddNumGroupDimension - numeric group dimension exists already" );
    // ReplaceNumGroupDimension() adds new or replaces existing
    ReplaceNumGroupDimension( rGroupDim );
}

void ScDPDimensionSaveData::ReplaceNumGroupDimension( const ScDPSaveNumGroupDimension& rGroupDim )
{
    ScDPSaveNumGroupDimMap::iterator aIt = maNumGroupDims.find( rGroupDim.GetDimensionName() );
    if( aIt == maNumGroupDims.end() )
        maNumGroupDims.emplace( rGroupDim.GetDimensionName(), rGroupDim );
    else
        aIt->second = rGroupDim;
}

void ScDPDimensionSaveData::RemoveNumGroupDimension( const OUString& rGroupDimName )
{
    maNumGroupDims.erase( rGroupDimName );
}

void ScDPDimensionSaveData::WriteToData( ScDPGroupTableData& rData ) const
{
    //  rData is assumed to be empty
    //  AddToData also handles date grouping

    for( ScDPSaveGroupDimVec::const_iterator aIt = maGroupDims.begin(), aEnd = maGroupDims.end(); aIt != aEnd; ++aIt )
        aIt->AddToData( rData );

    for( ScDPSaveNumGroupDimMap::const_iterator aIt = maNumGroupDims.begin(), aEnd = maNumGroupDims.end(); aIt != aEnd; ++aIt )
        aIt->second.AddToData( rData );
}

namespace {

class AddGroupDimToCache
{
    ScDPCache& mrCache;
public:
    explicit AddGroupDimToCache(ScDPCache& rCache) : mrCache(rCache) {}
    void operator() (const ScDPSaveGroupDimension& rDim)
    {
        rDim.AddToCache(mrCache);
    }
};

}

void ScDPDimensionSaveData::WriteToCache(ScDPCache& rCache) const
{
    std::for_each(maGroupDims.begin(), maGroupDims.end(), AddGroupDimToCache(rCache));
    ScDPSaveNumGroupDimMap::const_iterator it = maNumGroupDims.begin(), itEnd = maNumGroupDims.end();
    for (; it != itEnd; ++it)
        it->second.AddToCache(rCache);
}

const ScDPSaveGroupDimension* ScDPDimensionSaveData::GetGroupDimForBase( const OUString& rBaseDimName ) const
{
    return const_cast< ScDPDimensionSaveData* >( this )->GetGroupDimAccForBase( rBaseDimName );
}

const ScDPSaveGroupDimension* ScDPDimensionSaveData::GetNamedGroupDim( const OUString& rGroupDimName ) const
{
    return const_cast< ScDPDimensionSaveData* >( this )->GetNamedGroupDimAcc( rGroupDimName );
}

const ScDPSaveGroupDimension* ScDPDimensionSaveData::GetFirstNamedGroupDim( const OUString& rBaseDimName ) const
{
    return const_cast< ScDPDimensionSaveData* >( this )->GetFirstNamedGroupDimAcc( rBaseDimName );
}

const ScDPSaveGroupDimension* ScDPDimensionSaveData::GetNextNamedGroupDim( const OUString& rGroupDimName ) const
{
    return const_cast< ScDPDimensionSaveData* >( this )->GetNextNamedGroupDimAcc( rGroupDimName );
}

const ScDPSaveNumGroupDimension* ScDPDimensionSaveData::GetNumGroupDim( const OUString& rGroupDimName ) const
{
    return const_cast< ScDPDimensionSaveData* >( this )->GetNumGroupDimAcc( rGroupDimName );
}

ScDPSaveGroupDimension* ScDPDimensionSaveData::GetGroupDimAccForBase( const OUString& rBaseDimName )
{
    ScDPSaveGroupDimension* pGroupDim = GetFirstNamedGroupDimAcc( rBaseDimName );
    return pGroupDim ? pGroupDim : GetNextNamedGroupDimAcc( rBaseDimName );
}

ScDPSaveGroupDimension* ScDPDimensionSaveData::GetNamedGroupDimAcc( const OUString& rGroupDimName )
{
    ScDPSaveGroupDimVec::iterator aIt = ::std::find_if(
        maGroupDims.begin(), maGroupDims.end(), ScDPSaveGroupDimNameFunc( rGroupDimName ) );
    return (aIt == maGroupDims.end()) ? nullptr : &*aIt;
}

ScDPSaveGroupDimension* ScDPDimensionSaveData::GetFirstNamedGroupDimAcc( const OUString& rBaseDimName )
{
    ScDPSaveGroupDimVec::iterator aIt = ::std::find_if(
        maGroupDims.begin(), maGroupDims.end(), ScDPSaveGroupSourceNameFunc( rBaseDimName ) );
    return (aIt == maGroupDims.end()) ? nullptr : &*aIt;
}

ScDPSaveGroupDimension* ScDPDimensionSaveData::GetNextNamedGroupDimAcc( const OUString& rGroupDimName )
{
    // find the group dimension with the passed name
    ScDPSaveGroupDimVec::iterator aIt = ::std::find_if(
        maGroupDims.begin(), maGroupDims.end(), ScDPSaveGroupDimNameFunc( rGroupDimName ) );
    // find next group dimension based on the same source dimension name
    if( aIt != maGroupDims.end() )
        aIt = ::std::find_if( aIt + 1, maGroupDims.end(), ScDPSaveGroupSourceNameFunc( aIt->GetSourceDimName() ) );
    return (aIt == maGroupDims.end()) ? nullptr : &*aIt;
}

ScDPSaveNumGroupDimension* ScDPDimensionSaveData::GetNumGroupDimAcc( const OUString& rGroupDimName )
{
    ScDPSaveNumGroupDimMap::iterator aIt = maNumGroupDims.find( rGroupDimName );
    return (aIt == maNumGroupDims.end()) ? nullptr : &aIt->second;
}

bool ScDPDimensionSaveData::HasGroupDimensions() const
{
    return !maGroupDims.empty() || !maNumGroupDims.empty();
}

sal_Int32 ScDPDimensionSaveData::CollectDateParts( const OUString& rBaseDimName ) const
{
    sal_Int32 nParts = 0;
    // start with part of numeric group
    if( const ScDPSaveNumGroupDimension* pNumDim = GetNumGroupDim( rBaseDimName ) )
        nParts |= pNumDim->GetDatePart();
    // collect parts from all matching group dimensions
    for( const ScDPSaveGroupDimension* pGroupDim = GetFirstNamedGroupDim( rBaseDimName ); pGroupDim; pGroupDim = GetNextNamedGroupDim( pGroupDim->GetGroupDimName() ) )
        nParts |= pGroupDim->GetDatePart();

    return nParts;
}

OUString ScDPDimensionSaveData::CreateGroupDimName(
    const OUString& rSourceName, const ScDPObject& rObject, bool bAllowSource,
    const std::vector<OUString>* pDeletedNames )
{
    // create a name for the new dimension by appending a number to the original
    // dimension's name

    bool bUseSource = bAllowSource;     // if set, try the unchanged original name first

    sal_Int32 nAdd = 2;                 // first try is "Name2"
    const sal_Int32 nMaxAdd = 1000;     // limit the loop
    while ( nAdd <= nMaxAdd )
    {
        OUString aDimName( rSourceName );
        if ( !bUseSource )
            aDimName += OUString::number(nAdd);
        bool bExists = false;

        // look for existing group dimensions
        for( ScDPSaveGroupDimVec::const_iterator aIt = maGroupDims.begin(), aEnd = maGroupDims.end(); (aIt != aEnd) && !bExists; ++aIt )
            if( aIt->GetGroupDimName() == aDimName )         //TODO: ignore case
                bExists = true;

        // look for base dimensions that happen to have that name
        if ( !bExists && rObject.IsDimNameInUse( aDimName ) )
        {
            if ( pDeletedNames &&
                 std::find( pDeletedNames->begin(), pDeletedNames->end(), aDimName ) != pDeletedNames->end() )
            {
                // allow the name anyway if the name is in pDeletedNames
            }
            else
                bExists = true;
        }

        if ( !bExists )
            return aDimName;            // found a new name

        if ( bUseSource )
            bUseSource = false;
        else
            ++nAdd;                     // continue with higher number
    }
    OSL_FAIL("CreateGroupDimName: no valid name found");
    return OUString();
}

namespace
{
    static const char* aDatePartIds[] =
    {
        STR_DPFIELD_GROUP_BY_SECONDS,
        STR_DPFIELD_GROUP_BY_MINUTES,
        STR_DPFIELD_GROUP_BY_HOURS,
        STR_DPFIELD_GROUP_BY_DAYS,
        STR_DPFIELD_GROUP_BY_MONTHS,
        STR_DPFIELD_GROUP_BY_QUARTERS,
        STR_DPFIELD_GROUP_BY_YEARS
    };
}

OUString ScDPDimensionSaveData::CreateDateGroupDimName(
    sal_Int32 nDatePart, const ScDPObject& rObject, bool bAllowSource,
    const std::vector<OUString>* pDeletedNames )
{
    using namespace css::sheet::DataPilotFieldGroupBy;
    OUString aPartName;
    switch( nDatePart )
    {
        case SECONDS:  aPartName = ScResId(aDatePartIds[0]); break;
        case MINUTES:  aPartName = ScResId(aDatePartIds[1]); break;
        case HOURS:    aPartName = ScResId(aDatePartIds[2]); break;
        case DAYS:     aPartName = ScResId(aDatePartIds[3]); break;
        case MONTHS:   aPartName = ScResId(aDatePartIds[4]); break;
        case QUARTERS: aPartName = ScResId(aDatePartIds[5]); break;
        case YEARS:    aPartName = ScResId(aDatePartIds[6]); break;
    }
    OSL_ENSURE(!aPartName.isEmpty(), "ScDPDimensionSaveData::CreateDateGroupDimName - invalid date part");
    return CreateGroupDimName( aPartName, rObject, bAllowSource, pDeletedNames );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
