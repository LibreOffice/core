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
#include <svl/eitem.hxx>

#include <svl/itemprop.hxx>

#include <editeng/unoipset.hxx>
#include <editeng/editids.hrc>
#include <editeng/editeng.hxx>
#include <svl/itempool.hxx>
#include <o3tl/any.hxx>
#include <osl/diagnose.h>
#include <algorithm>

using namespace ::com::sun::star;


struct SvxIDPropertyCombine
{
    sal_uInt16  nWID;
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


uno::Any* SvxItemPropertySet::GetUsrAnyForID(sal_uInt16 nWID) const
{
    for (SvxIDPropertyCombine* pActual : aCombineList)
    {
        if( pActual->nWID == nWID )
            return &pActual->aAny;
    }
    return nullptr;
}


void SvxItemPropertySet::AddUsrAnyForID(const uno::Any& rAny, sal_uInt16 nWID)
{
    SvxIDPropertyCombine* pNew = new SvxIDPropertyCombine;
    pNew->nWID = nWID;
    pNew->aAny = rAny;
    aCombineList.push_back( pNew );
}


void SvxItemPropertySet::ClearAllUsrAny()
{
    for (SvxIDPropertyCombine* i : aCombineList)
        delete i;
    aCombineList.clear();
}


bool SvxUnoCheckForPositiveValue( const uno::Any& rVal )
{
    bool bConvert = true; // the default is that all metric items must be converted
    sal_Int32 nValue = 0;
    if( rVal >>= nValue )
        bConvert = (nValue > 0);
    return bConvert;
}


uno::Any SvxItemPropertySet::getPropertyValue( const SfxItemPropertySimpleEntry* pMap, const SfxItemSet& rSet, bool bSearchInParent, bool bDontConvertNegativeValues )
{
    uno::Any aVal;
    if(!pMap || !pMap->nWID)
        return aVal;

    const SfxPoolItem* pItem = nullptr;
    SfxItemPool* pPool = rSet.GetPool();
    rSet.GetItemState( pMap->nWID, bSearchInParent, &pItem );
    if( nullptr == pItem && pPool )
        pItem = &(pPool->GetDefaultItem( pMap->nWID ));

    const SfxMapUnit eMapUnit = pPool ? pPool->GetMetric((sal_uInt16)pMap->nWID) : SFX_MAPUNIT_100TH_MM;
    sal_uInt8 nMemberId = pMap->nMemberId & (~SFX_METRIC_ITEM);
    if( eMapUnit == SFX_MAPUNIT_100TH_MM )
        nMemberId &= (~CONVERT_TWIPS);

    if(pItem)
    {
        pItem->QueryValue( aVal, nMemberId );
        if( pMap->nMemberId & SFX_METRIC_ITEM )
        {
            if( eMapUnit != SFX_MAPUNIT_100TH_MM )
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


void SvxItemPropertySet::setPropertyValue( const SfxItemPropertySimpleEntry* pMap, const uno::Any& rVal, SfxItemSet& rSet, bool bDontConvertNegativeValues )
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

    DBG_ASSERT( pItem, "Got no default for item!" );
    if( pItem )
    {
        uno::Any aValue( rVal );

        const SfxMapUnit eMapUnit = pPool ? pPool->GetMetric((sal_uInt16)pMap->nWID) : SFX_MAPUNIT_100TH_MM;

        // check for needed metric translation
        if( (pMap->nMemberId & SFX_METRIC_ITEM) && eMapUnit != SFX_MAPUNIT_100TH_MM )
        {
            if ( !bDontConvertNegativeValues || SvxUnoCheckForPositiveValue( aValue ) )
                SvxUnoConvertFromMM( eMapUnit, aValue );
        }

        SfxPoolItem *pNewItem = pItem->Clone();

        sal_uInt8 nMemberId = pMap->nMemberId & (~SFX_METRIC_ITEM);
        if( eMapUnit == SFX_MAPUNIT_100TH_MM )
            nMemberId &= (~CONVERT_TWIPS);

        if( pNewItem->PutValue( aValue, nMemberId ) )
        {
            // Set new item in item set
            pNewItem->SetWhich( pMap->nWID );
            rSet.Put( *pNewItem );
        }
        delete pNewItem;
    }
}


uno::Any SvxItemPropertySet::getPropertyValue( const SfxItemPropertySimpleEntry* pMap ) const
{
    // Already entered a value? Then finish quickly
    uno::Any* pUsrAny = GetUsrAnyForID(pMap->nWID);
    if(pUsrAny)
        return *pUsrAny;

    // No UsrAny detected yet, generate Default entry and return this
    const SfxMapUnit eMapUnit = mrItemPool.GetMetric((sal_uInt16)pMap->nWID);
    sal_uInt8 nMemberId = pMap->nMemberId & (~SFX_METRIC_ITEM);
    if( eMapUnit == SFX_MAPUNIT_100TH_MM )
        nMemberId &= (~CONVERT_TWIPS);
    uno::Any aVal;
    SfxItemSet aSet( mrItemPool, pMap->nWID, pMap->nWID);

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
            const_cast<SvxItemPropertySet*>(this)->AddUsrAnyForID(aVal, pMap->nWID);
        }
    }

    if( pMap->nMemberId & SFX_METRIC_ITEM )
    {
        // check for needed metric translation
        if(pMap->nMemberId & SFX_METRIC_ITEM && eMapUnit != SFX_MAPUNIT_100TH_MM)
        {
            SvxUnoConvertToMM( eMapUnit, aVal );
        }
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


void SvxItemPropertySet::setPropertyValue( const SfxItemPropertySimpleEntry* pMap, const uno::Any& rVal ) const
{
    uno::Any* pUsrAny = GetUsrAnyForID(pMap->nWID);
    if(!pUsrAny)
        const_cast<SvxItemPropertySet*>(this)->AddUsrAnyForID(rVal, pMap->nWID);
    else
        *pUsrAny = rVal;
}


const SfxItemPropertySimpleEntry* SvxItemPropertySet::getPropertyMapEntry(const OUString &rName) const
{
    return m_aPropertyMap.getByName( rName );
 }


uno::Reference< beans::XPropertySetInfo > const &  SvxItemPropertySet::getPropertySetInfo() const
{
    if( !m_xInfo.is() )
        m_xInfo = new SfxItemPropertySetInfo( m_aPropertyMap );
    return m_xInfo;
}


#ifndef TWIPS_TO_MM
#define TWIPS_TO_MM(val) ((val * 127 + 36) / 72)
#endif
#ifndef MM_TO_TWIPS
#define MM_TO_TWIPS(val) ((val * 72 + 63) / 127)
#endif

/** converts the given any with a metric to 100th/mm if needed */
void SvxUnoConvertToMM( const SfxMapUnit eSourceMapUnit, uno::Any & rMetric ) throw()
{
    // map the metric of the itempool to 100th mm
    switch(eSourceMapUnit)
    {
        case SFX_MAPUNIT_TWIP :
        {
            switch( rMetric.getValueTypeClass() )
            {
            case uno::TypeClass_BYTE:
                rMetric <<= (sal_Int8)(TWIPS_TO_MM(*o3tl::forceAccess<sal_Int8>(rMetric)));
                break;
            case uno::TypeClass_SHORT:
                rMetric <<= (sal_Int16)(TWIPS_TO_MM(*o3tl::forceAccess<sal_Int16>(rMetric)));
                break;
            case uno::TypeClass_UNSIGNED_SHORT:
                rMetric <<= (sal_uInt16)(TWIPS_TO_MM(*o3tl::forceAccess<sal_uInt16>(rMetric)));
                break;
            case uno::TypeClass_LONG:
                rMetric <<= (sal_Int32)(TWIPS_TO_MM(*o3tl::forceAccess<sal_Int32>(rMetric)));
                break;
            case uno::TypeClass_UNSIGNED_LONG:
                rMetric <<= (sal_uInt32)(TWIPS_TO_MM(*o3tl::forceAccess<sal_uInt32>(rMetric)));
                break;
            default:
                OSL_FAIL(OString("AW: Missing unit translation to 100th mm, " + OString::number(rMetric.getValueTypeClass())).getStr());
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
void SvxUnoConvertFromMM( const SfxMapUnit eDestinationMapUnit, uno::Any & rMetric ) throw()
{
    switch(eDestinationMapUnit)
    {
        case SFX_MAPUNIT_TWIP :
        {
            switch( rMetric.getValueTypeClass() )
            {
                case uno::TypeClass_BYTE:
                    rMetric <<= (sal_Int8)(MM_TO_TWIPS(*o3tl::forceAccess<sal_Int8>(rMetric)));
                    break;
                case uno::TypeClass_SHORT:
                    rMetric <<= (sal_Int16)(MM_TO_TWIPS(*o3tl::forceAccess<sal_Int16>(rMetric)));
                    break;
                case uno::TypeClass_UNSIGNED_SHORT:
                    rMetric <<= (sal_uInt16)(MM_TO_TWIPS(*o3tl::forceAccess<sal_uInt16>(rMetric)));
                    break;
                case uno::TypeClass_LONG:
                    rMetric <<= (sal_Int32)(MM_TO_TWIPS(*o3tl::forceAccess<sal_Int32>(rMetric)));
                    break;
                case uno::TypeClass_UNSIGNED_LONG:
                    rMetric <<= (sal_uInt32)(MM_TO_TWIPS(*o3tl::forceAccess<sal_uInt32>(rMetric)));
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
