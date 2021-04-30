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

#include <com/sun/star/beans/XPropertySet.hpp>
#include <svl/itemprop.hxx>
#include <tools/UnitConversion.hxx>
#include <editeng/unoipset.hxx>
#include <svl/itempool.hxx>
#include <svl/solar.hrc>
#include <o3tl/any.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <algorithm>

using namespace ::com::sun::star;


struct SvxIDPropertyCombine
{
    sal_uInt16  nWID;
    sal_uInt8   memberId;
    uno::Any    aAny;
};


SvxItemPropertySet::SvxItemPropertySet( const SfxItemPropertyMapEntry* pMap, SfxItemPool& rItemPool )
:   m_aPropertyMap( pMap ),
    mrItemPool( rItemPool )
{
}


SvxItemPropertySet::~SvxItemPropertySet()
{
    ClearAllUsrAny();
}


uno::Any* SvxItemPropertySet::GetUsrAnyForID(SfxItemPropertyMapEntry const & entry) const
{
    for (auto const & pActual : aCombineList)
    {
        if( pActual->nWID == entry.nWID && pActual->memberId == entry.nMemberId )
            return &pActual->aAny;
    }
    return nullptr;
}


void SvxItemPropertySet::AddUsrAnyForID(
    const uno::Any& rAny, SfxItemPropertyMapEntry const & entry)
{
    std::unique_ptr<SvxIDPropertyCombine> pNew(new SvxIDPropertyCombine);
    pNew->nWID = entry.nWID;
    pNew->memberId = entry.nMemberId;
    pNew->aAny = rAny;
    aCombineList.push_back( std::move(pNew) );
}


void SvxItemPropertySet::ClearAllUsrAny()
{
    aCombineList.clear();
}


static bool SvxUnoCheckForPositiveValue( const uno::Any& rVal )
{
    bool bConvert = true; // the default is that all metric items must be converted
    sal_Int32 nValue = 0;
    if( rVal >>= nValue )
        bConvert = (nValue > 0);
    return bConvert;
}


uno::Any SvxItemPropertySet::getPropertyValue( const SfxItemPropertyMapEntry* pMap, const SfxItemSet& rSet, bool bSearchInParent, bool bDontConvertNegativeValues )
{
    uno::Any aVal;
    if(!pMap || !pMap->nWID)
        return aVal;

    const SfxPoolItem* pItem = nullptr;
    SfxItemPool* pPool = rSet.GetPool();
    (void)rSet.GetItemState( pMap->nWID, bSearchInParent, &pItem );
    if( nullptr == pItem && pPool )
        pItem = &(pPool->GetDefaultItem( pMap->nWID ));

    const MapUnit eMapUnit = pPool ? pPool->GetMetric(pMap->nWID) : MapUnit::Map100thMM;
    sal_uInt8 nMemberId = pMap->nMemberId;
    if( eMapUnit == MapUnit::Map100thMM )
        nMemberId &= (~CONVERT_TWIPS);

    if(pItem)
    {
        pItem->QueryValue( aVal, nMemberId );
        if( pMap->nMoreFlags & PropertyMoreFlags::METRIC_ITEM )
        {
            if( eMapUnit != MapUnit::Map100thMM )
            {
                if ( !bDontConvertNegativeValues || SvxUnoCheckForPositiveValue( aVal ) )
                    SvxUnoConvertToMM( eMapUnit, aVal );
            }
        }
        else if ( pMap->aType.getTypeClass() == uno::TypeClass_ENUM &&
              aVal.getValueType() == ::cppu::UnoType<sal_Int32>::get() )
        {
            // convert typeless SfxEnumItem to enum type
            sal_Int32 nEnum;
            aVal >>= nEnum;
            aVal.setValue( &nEnum, pMap->aType );
        }
    }
    else
    {
        OSL_FAIL( "No SfxPoolItem found for property!" );
    }

    return aVal;
}


void SvxItemPropertySet::setPropertyValue( const SfxItemPropertyMapEntry* pMap, const uno::Any& rVal, SfxItemSet& rSet, bool bDontConvertNegativeValues )
{
    if(!pMap || !pMap->nWID)
        return;

    // Get item
    const SfxPoolItem* pItem = nullptr;
    SfxItemState eState = rSet.GetItemState( pMap->nWID, true, &pItem );
    SfxItemPool* pPool = rSet.GetPool();

    // Put UnoAny in the item value
    if(eState < SfxItemState::DEFAULT || pItem == nullptr)
    {
        if( pPool == nullptr )
        {
            OSL_FAIL( "No default item and no pool?" );
            return;
        }

        pItem = &pPool->GetDefaultItem( pMap->nWID );
    }

    uno::Any aValue(rVal);

    const MapUnit eMapUnit = pPool ? pPool->GetMetric(pMap->nWID) : MapUnit::Map100thMM;

    // check for needed metric translation
    if ((pMap->nMoreFlags & PropertyMoreFlags::METRIC_ITEM) && eMapUnit != MapUnit::Map100thMM)
    {
        if (!bDontConvertNegativeValues || SvxUnoCheckForPositiveValue(aValue))
            SvxUnoConvertFromMM(eMapUnit, aValue);
    }

    std::unique_ptr<SfxPoolItem> pNewItem(pItem->Clone());

    sal_uInt8 nMemberId = pMap->nMemberId;
    if (eMapUnit == MapUnit::Map100thMM)
        nMemberId &= (~CONVERT_TWIPS);

    if (pNewItem->PutValue(aValue, nMemberId))
    {
        // Set new item in item set
        pNewItem->SetWhich(pMap->nWID);
        rSet.Put(*pNewItem);
    }
}


uno::Any SvxItemPropertySet::getPropertyValue( const SfxItemPropertyMapEntry* pMap ) const
{
    // Already entered a value? Then finish quickly
    uno::Any* pUsrAny = GetUsrAnyForID(*pMap);
    if(pUsrAny)
        return *pUsrAny;

    // No UsrAny detected yet, generate Default entry and return this
    const MapUnit eMapUnit = mrItemPool.GetMetric(pMap->nWID);
    sal_uInt8 nMemberId = pMap->nMemberId;
    if( eMapUnit == MapUnit::Map100thMM )
        nMemberId &= (~CONVERT_TWIPS);
    uno::Any aVal;
    SfxItemSet aSet( mrItemPool, {{pMap->nWID, pMap->nWID}});

    if( (pMap->nWID < OWN_ATTR_VALUE_START) || (pMap->nWID > OWN_ATTR_VALUE_END ) )
    {
        // Get Default from ItemPool
        if(SfxItemPool::IsWhich(pMap->nWID))
            aSet.Put(mrItemPool.GetDefaultItem(pMap->nWID));
    }

    if(aSet.Count())
    {
        const SfxPoolItem* pItem = nullptr;
        SfxItemState eState = aSet.GetItemState( pMap->nWID, true, &pItem );
        if(eState >= SfxItemState::DEFAULT && pItem)
        {
            pItem->QueryValue( aVal, nMemberId );
            const_cast<SvxItemPropertySet*>(this)->AddUsrAnyForID(aVal, *pMap);
        }
    }

    // check for needed metric translation
    if(pMap->nMoreFlags & PropertyMoreFlags::METRIC_ITEM && eMapUnit != MapUnit::Map100thMM)
    {
        SvxUnoConvertToMM( eMapUnit, aVal );
    }

    if ( pMap->aType.getTypeClass() == uno::TypeClass_ENUM &&
          aVal.getValueType() == ::cppu::UnoType<sal_Int32>::get() )
    {
        sal_Int32 nEnum;
        aVal >>= nEnum;

        aVal.setValue( &nEnum, pMap->aType );
    }

    return aVal;
}


void SvxItemPropertySet::setPropertyValue( const SfxItemPropertyMapEntry* pMap, const uno::Any& rVal ) const
{
    uno::Any* pUsrAny = GetUsrAnyForID(*pMap);
    if(!pUsrAny)
        const_cast<SvxItemPropertySet*>(this)->AddUsrAnyForID(rVal, *pMap);
    else
        *pUsrAny = rVal;
}


const SfxItemPropertyMapEntry* SvxItemPropertySet::getPropertyMapEntry(std::u16string_view rName) const
{
    return m_aPropertyMap.getByName( rName );
 }


uno::Reference< beans::XPropertySetInfo > const &  SvxItemPropertySet::getPropertySetInfo() const
{
    if( !m_xInfo.is() )
        m_xInfo = new SfxItemPropertySetInfo( m_aPropertyMap );
    return m_xInfo;
}


/** converts the given any with a metric to 100th/mm if needed */
void SvxUnoConvertToMM( const MapUnit eSourceMapUnit, uno::Any & rMetric ) noexcept
{
    // map the metric of the itempool to 100th mm
    switch(eSourceMapUnit)
    {
        case MapUnit::MapTwip :
        {
            switch( rMetric.getValueTypeClass() )
            {
            case uno::TypeClass_BYTE:
                rMetric <<= static_cast<sal_Int8>(convertTwipToMm100(*o3tl::forceAccess<sal_Int8>(rMetric)));
                break;
            case uno::TypeClass_SHORT:
                rMetric <<= static_cast<sal_Int16>(convertTwipToMm100(*o3tl::forceAccess<sal_Int16>(rMetric)));
                break;
            case uno::TypeClass_UNSIGNED_SHORT:
                rMetric <<= static_cast<sal_uInt16>(convertTwipToMm100(*o3tl::forceAccess<sal_uInt16>(rMetric)));
                break;
            case uno::TypeClass_LONG:
                rMetric <<= static_cast<sal_Int32>(convertTwipToMm100(*o3tl::forceAccess<sal_Int32>(rMetric)));
                break;
            case uno::TypeClass_UNSIGNED_LONG:
                rMetric <<= static_cast<sal_uInt32>(convertTwipToMm100(*o3tl::forceAccess<sal_uInt32>(rMetric)));
                break;
            default:
                SAL_WARN("editeng", "AW: Missing unit translation to 100th mm, " << OString::number(static_cast<sal_Int32>(rMetric.getValueTypeClass())));
                assert(false);
            }
            break;
        }
        default:
        {
            OSL_FAIL("AW: Missing unit translation to 100th mm!");
        }
    }
}


/** converts the given any with a metric from 100th/mm to the given metric if needed */
void SvxUnoConvertFromMM( const MapUnit eDestinationMapUnit, uno::Any & rMetric ) noexcept
{
    switch(eDestinationMapUnit)
    {
        case MapUnit::MapTwip :
        {
            switch( rMetric.getValueTypeClass() )
            {
                case uno::TypeClass_BYTE:
                    rMetric <<= static_cast<sal_Int8>(sanitiseMm100ToTwip(*o3tl::forceAccess<sal_Int8>(rMetric)));
                    break;
                case uno::TypeClass_SHORT:
                    rMetric <<= static_cast<sal_Int16>(sanitiseMm100ToTwip(*o3tl::forceAccess<sal_Int16>(rMetric)));
                    break;
                case uno::TypeClass_UNSIGNED_SHORT:
                    rMetric <<= static_cast<sal_uInt16>(sanitiseMm100ToTwip(*o3tl::forceAccess<sal_uInt16>(rMetric)));
                    break;
                case uno::TypeClass_LONG:
                    rMetric <<= static_cast<sal_Int32>(sanitiseMm100ToTwip(*o3tl::forceAccess<sal_Int32>(rMetric)));
                    break;
                case uno::TypeClass_UNSIGNED_LONG:
                    rMetric <<= static_cast<sal_uInt32>(sanitiseMm100ToTwip(*o3tl::forceAccess<sal_uInt32>(rMetric)));
                    break;
                default:
                    OSL_FAIL("AW: Missing unit translation to 100th mm!");
            }
            break;
        }
        default:
        {
            OSL_FAIL("AW: Missing unit translation to PoolMetrics!");
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
