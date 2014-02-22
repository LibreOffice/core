/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "dpdimsave.hxx"
#include "dpgroup.hxx"
#include "dpobject.hxx"
#include "dputil.hxx"
#include "document.hxx"

#include <com/sun/star/sheet/DataPilotFieldGroupBy.hpp>

#include <svl/zforlist.hxx>
#include <rtl/math.hxx>
#include <algorithm>

#include "global.hxx"
#include "scresid.hxx"
#include "globstr.hrc"

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
    

    for ( std::vector<OUString>::const_iterator aIter(rGroup.aElements.begin());
                                aIter != rGroup.aElements.end(); ++aIter )
        aElements.push_back( *aIter );
}

bool ScDPSaveGroupItem::RemoveElement( const OUString& rName )
{
    for (std::vector<OUString>::iterator aIter = aElements.begin(); aIter != aElements.end(); ++aIter)
        if (*aIter == rName)          
        {
            aElements.erase(aIter);   
            return true;                
        }

    return false;   
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
    return (nIndex < aElements.size()) ? &aElements[ nIndex ] : 0;
}

void ScDPSaveGroupItem::Rename( const OUString& rNewName )
{
    aGroupName = rNewName;
}

void ScDPSaveGroupItem::RemoveElementsFromGroups( ScDPSaveGroupDimension& rDimension ) const
{
    
    

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

ScDPSaveGroupDimension::~ScDPSaveGroupDimension()
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
    

    
    

    sal_Int32 nAdd = 1;                                 
    const sal_Int32 nMaxAdd = nAdd + aGroups.size();    
    while ( nAdd <= nMaxAdd )
    {
        OUString aGroupName = rPrefix + OUString::number( nAdd );
        bool bExists = false;

        
        for ( ScDPSaveGroupItemVec::const_iterator aIter(aGroups.begin());
                                    aIter != aGroups.end() && !bExists; ++aIter )
            if (aIter->GetGroupName().equals(aGroupName))         
                bExists = true;

        if ( !bExists )
            return aGroupName;          

        ++nAdd;                         
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
        if (aIter->GetGroupName().equals(rGroupName))         
            return &*aIter;

    return NULL;        
}

long ScDPSaveGroupDimension::GetGroupCount() const
{
    return aGroups.size();
}

const ScDPSaveGroupItem* ScDPSaveGroupDimension::GetGroupByIndex( long nIndex ) const
{
    return const_cast< ScDPSaveGroupDimension* >( this )->GetGroupAccByIndex( nIndex );
}

ScDPSaveGroupItem* ScDPSaveGroupDimension::GetGroupAccByIndex( long nIndex )
{
    return &aGroups[nIndex];
}

void ScDPSaveGroupDimension::RemoveFromGroups( const OUString& rItemName )
{
    
    

    for ( ScDPSaveGroupItemVec::iterator aIter(aGroups.begin()); aIter != aGroups.end(); ++aIter )
        if ( aIter->RemoveElement( rItemName ) )
        {
            if ( aIter->IsEmpty() )         
                aGroups.erase( aIter );     

            return;     
        }
}

void ScDPSaveGroupDimension::RemoveGroup(const OUString& rGroupName)
{
    for (ScDPSaveGroupItemVec::iterator aIter = aGroups.begin(); aIter != aGroups.end(); ++aIter)
        if (aIter->GetGroupName().equals(rGroupName))          
        {
            aGroups.erase( aIter );
            return;                     
        }
}

bool ScDPSaveGroupDimension::IsEmpty() const
{
    return aGroups.empty();
}

bool ScDPSaveGroupDimension::HasOnlyHidden(const ScDPUniqueStringSet& rVisible)
{
    

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
    sal_Int32 nDatePart, SvNumberFormatter* pFormatter)
{
    
    
    double fSourceMin = 0.0;
    double fSourceMax = 0.0;
    bool bFirst = true;

    const ScDPCache::ItemsType& rItems = rCache.GetDimMemberValues(nSourceDim);
    ScDPCache::ItemsType::const_iterator it = rItems.begin(), itEnd = rItems.end();
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

    
    
    
    if (rDateInfo.mbAutoStart)
        rDateInfo.mfStart = rtl::math::approxFloor(fSourceMin);
    if (rDateInfo.mbAutoEnd)
        rDateInfo.mfEnd = rtl::math::approxFloor(fSourceMax) + 1;

    

    long nStart = 0, nEnd = 0; 

    switch (nDatePart)
    {
        case sheet::DataPilotFieldGroupBy::YEARS:
            nStart = ScDPUtil::getDatePartValue(
                fSourceMin, NULL, sheet::DataPilotFieldGroupBy::YEARS, pFormatter);
            nEnd = ScDPUtil::getDatePartValue(fSourceMax, NULL, sheet::DataPilotFieldGroupBy::YEARS, pFormatter);
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

    
    rCache.ResetGroupItems(nGroupDim, rDateInfo, nDatePart);

    for (sal_Int32 nValue = nStart; nValue <= nEnd; ++nValue)
        rCache.SetGroupItem(nGroupDim, ScDPItemData(nDatePart, nValue));

    
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
            

            aDim.SetDateDimension();
        }
        else
        {
            

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

    const ScDPCache::ItemsType& rItems = rCache.GetDimMemberValues(nSourceDim);
    {
        ScDPCache::ItemsType::const_iterator it = rItems.begin(), itEnd = rItems.end();
        for (; it != itEnd; ++it)
        {
            const ScDPItemData& rItem = *it;
            if (!IsInGroup(rItem))
                
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

ScDPSaveNumGroupDimension::~ScDPSaveNumGroupDimension()
{
}

void ScDPSaveNumGroupDimension::AddToData( ScDPGroupTableData& rData ) const
{
    long nSource = rData.GetDimensionIndex( aDimensionName );
    if ( nSource >= 0 )
    {
        ScDPNumGroupDimension aDim( aGroupInfo );           
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
        
        SvNumberFormatter* pFormatter = rCache.GetDoc()->GetFormatTable();
        fillDateGroupDimension(rCache, aDateInfo, nDim, nDim, nDatePart, pFormatter);
    }
    else if (aGroupInfo.mbEnable)
    {
        

        
        

        
        aGroupInfo.mbIntegerOnly =
            (aGroupInfo.mbAutoStart || isInteger(aGroupInfo.mfStart)) &&
            (aGroupInfo.mbAutoEnd || isInteger(aGroupInfo.mfEnd)) &&
            isInteger(aGroupInfo.mfStep);

        double fSourceMin = 0.0;
        double fSourceMax = 0.0;
        bool bFirst = true;

        const ScDPCache::ItemsType& rItems = rCache.GetDimMemberValues(nDim);
        ScDPCache::ItemsType::const_iterator it = rItems.begin(), itEnd = rItems.end();
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
                
                
                aGroupInfo.mbIntegerOnly = false;
            }
        }

        if (aGroupInfo.mbDateValues)
        {
            
            aGroupInfo.mbIntegerOnly = true;
            fSourceMin = rtl::math::approxFloor(fSourceMin);
            fSourceMax = rtl::math::approxFloor(fSourceMax) + 1;
        }

        if (aGroupInfo.mbAutoStart)
            aGroupInfo.mfStart = fSourceMin;
        if (aGroupInfo.mbAutoEnd)
            aGroupInfo.mfEnd = fSourceMax;

        

        long nLoopCount = 0;
        double fLoop = aGroupInfo.mfStart;

        rCache.ResetGroupItems(nDim, aGroupInfo, 0);

        
        
        
        
        

        bool bFirstGroup = true;
        while (bFirstGroup || (fLoop < aGroupInfo.mfEnd && !rtl::math::approxEqual(fLoop, aGroupInfo.mfEnd)))
        {
            ScDPItemData aItem;
            aItem.SetRangeStart(fLoop);
            rCache.SetGroupItem(nDim, aItem);
            ++nLoopCount;
            fLoop = aGroupInfo.mfStart + nLoopCount * aGroupInfo.mfStep;
            bFirstGroup = false;

            
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
    inline explicit     ScDPSaveGroupDimNameFunc( const OUString& rDimName ) : maDimName( rDimName ) {}
    inline bool         operator()( const ScDPSaveGroupDimension& rGroupDim ) const { return rGroupDim.GetGroupDimName() == maDimName; }
};

struct ScDPSaveGroupSourceNameFunc
{
    OUString       maSrcDimName;
    inline explicit     ScDPSaveGroupSourceNameFunc( const OUString& rSrcDimName ) : maSrcDimName( rSrcDimName ) {}
    inline bool         operator()( const ScDPSaveGroupDimension& rGroupDim ) const { return rGroupDim.GetSourceDimName() == maSrcDimName; }
};

} 



ScDPDimensionSaveData::ScDPDimensionSaveData()
{
}

ScDPDimensionSaveData::~ScDPDimensionSaveData()
{
}

bool ScDPDimensionSaveData::operator==( const ScDPDimensionSaveData& ) const
{
    return false;
}

void ScDPDimensionSaveData::AddGroupDimension( const ScDPSaveGroupDimension& rGroupDim )
{
    OSL_ENSURE( ::std::find_if( maGroupDims.begin(), maGroupDims.end(), ScDPSaveGroupDimNameFunc( rGroupDim.GetGroupDimName() ) ) == maGroupDims.end(),
        "ScDPDimensionSaveData::AddGroupDimension - group dimension exists already" );
    
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
    
    ReplaceNumGroupDimension( rGroupDim );
}

void ScDPDimensionSaveData::ReplaceNumGroupDimension( const ScDPSaveNumGroupDimension& rGroupDim )
{
    ScDPSaveNumGroupDimMap::iterator aIt = maNumGroupDims.find( rGroupDim.GetDimensionName() );
    if( aIt == maNumGroupDims.end() )
        maNumGroupDims.insert( ScDPSaveNumGroupDimMap::value_type( rGroupDim.GetDimensionName(), rGroupDim ) );
    else
        aIt->second = rGroupDim;
}

void ScDPDimensionSaveData::RemoveNumGroupDimension( const OUString& rGroupDimName )
{
    maNumGroupDims.erase( rGroupDimName );
}

void ScDPDimensionSaveData::WriteToData( ScDPGroupTableData& rData ) const
{
    
    

    for( ScDPSaveGroupDimVec::const_iterator aIt = maGroupDims.begin(), aEnd = maGroupDims.end(); aIt != aEnd; ++aIt )
        aIt->AddToData( rData );

    for( ScDPSaveNumGroupDimMap::const_iterator aIt = maNumGroupDims.begin(), aEnd = maNumGroupDims.end(); aIt != aEnd; ++aIt )
        aIt->second.AddToData( rData );
}

namespace {

class AddGroupDimToCache : std::unary_function<ScDPSaveGroupDimension, void>
{
    ScDPCache& mrCache;
public:
    AddGroupDimToCache(ScDPCache& rCache) : mrCache(rCache) {}
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
    return (aIt == maGroupDims.end()) ? 0 : &*aIt;
}

ScDPSaveGroupDimension* ScDPDimensionSaveData::GetFirstNamedGroupDimAcc( const OUString& rBaseDimName )
{
    ScDPSaveGroupDimVec::iterator aIt = ::std::find_if(
        maGroupDims.begin(), maGroupDims.end(), ScDPSaveGroupSourceNameFunc( rBaseDimName ) );
    return (aIt == maGroupDims.end()) ? 0 : &*aIt;
}

ScDPSaveGroupDimension* ScDPDimensionSaveData::GetNextNamedGroupDimAcc( const OUString& rGroupDimName )
{
    
    ScDPSaveGroupDimVec::iterator aIt = ::std::find_if(
        maGroupDims.begin(), maGroupDims.end(), ScDPSaveGroupDimNameFunc( rGroupDimName ) );
    
    if( aIt != maGroupDims.end() )
        aIt = ::std::find_if( aIt + 1, maGroupDims.end(), ScDPSaveGroupSourceNameFunc( aIt->GetSourceDimName() ) );
    return (aIt == maGroupDims.end()) ? 0 : &*aIt;
}

ScDPSaveNumGroupDimension* ScDPDimensionSaveData::GetNumGroupDimAcc( const OUString& rGroupDimName )
{
    ScDPSaveNumGroupDimMap::iterator aIt = maNumGroupDims.find( rGroupDimName );
    return (aIt == maNumGroupDims.end()) ? 0 : &aIt->second;
}

bool ScDPDimensionSaveData::HasGroupDimensions() const
{
    return !maGroupDims.empty() || !maNumGroupDims.empty();
}

sal_Int32 ScDPDimensionSaveData::CollectDateParts( const OUString& rBaseDimName ) const
{
    sal_Int32 nParts = 0;
    
    if( const ScDPSaveNumGroupDimension* pNumDim = GetNumGroupDim( rBaseDimName ) )
        nParts |= pNumDim->GetDatePart();
    
    for( const ScDPSaveGroupDimension* pGroupDim = GetFirstNamedGroupDim( rBaseDimName ); pGroupDim; pGroupDim = GetNextNamedGroupDim( pGroupDim->GetGroupDimName() ) )
        nParts |= pGroupDim->GetDatePart();

    return nParts;
}

OUString ScDPDimensionSaveData::CreateGroupDimName(
    const OUString& rSourceName, const ScDPObject& rObject, bool bAllowSource,
    const std::vector<OUString>* pDeletedNames )
{
    
    

    bool bUseSource = bAllowSource;     

    sal_Int32 nAdd = 2;                 
    const sal_Int32 nMaxAdd = 1000;     
    while ( nAdd <= nMaxAdd )
    {
        OUString aDimName( rSourceName );
        if ( !bUseSource )
            aDimName += OUString::number(nAdd);
        bool bExists = false;

        
        for( ScDPSaveGroupDimVec::const_iterator aIt = maGroupDims.begin(), aEnd = maGroupDims.end(); (aIt != aEnd) && !bExists; ++aIt )
            if( aIt->GetGroupDimName() == aDimName )         
                bExists = true;

        
        if ( !bExists && rObject.IsDimNameInUse( aDimName ) )
        {
            if ( pDeletedNames &&
                 std::find( pDeletedNames->begin(), pDeletedNames->end(), aDimName ) != pDeletedNames->end() )
            {
                
            }
            else
                bExists = true;
        }

        if ( !bExists )
            return aDimName;            

        if ( bUseSource )
            bUseSource = false;
        else
            ++nAdd;                     
    }
    OSL_FAIL("CreateGroupDimName: no valid name found");
    return OUString();
}

namespace
{
    static const sal_uInt16 nDatePartIds[] =
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
    using namespace ::com::sun::star::sheet::DataPilotFieldGroupBy;
    OUString aPartName;
    switch( nDatePart )
    {
        case SECONDS:  aPartName = ScGlobal::GetRscString( nDatePartIds[0] ); break;
        case MINUTES:  aPartName = ScGlobal::GetRscString( nDatePartIds[1] ); break;
        case HOURS:    aPartName = ScGlobal::GetRscString( nDatePartIds[2] ); break;
        case DAYS:     aPartName = ScGlobal::GetRscString( nDatePartIds[3] ); break;
        case MONTHS:   aPartName = ScGlobal::GetRscString( nDatePartIds[4] ); break;
        case QUARTERS: aPartName = ScGlobal::GetRscString( nDatePartIds[5] ); break;
        case YEARS:    aPartName = ScGlobal::GetRscString( nDatePartIds[6] ); break;
    }
    OSL_ENSURE(!aPartName.isEmpty(), "ScDPDimensionSaveData::CreateDateGroupDimName - invalid date part");
    return CreateGroupDimName( aPartName, rObject, bAllowSource, pDeletedNames );
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
