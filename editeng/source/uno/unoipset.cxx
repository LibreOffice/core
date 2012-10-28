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

#include <com/sun/star/beans/XPropertySet.hpp>
#include <svl/eitem.hxx>

#include <boost/unordered_map.hpp>
#include <svl/itemprop.hxx>

#include <editeng/unoipset.hxx>
#include <editeng/editids.hrc>
#include <editeng/editeng.hxx>
#include <svl/itempool.hxx>
#include <algorithm>

using namespace ::com::sun::star;
using namespace ::rtl;

using ::std::vector;

//----------------------------------------------------------------------

struct SfxItemPropertyMapEntryHash
{
    size_t operator()(const SfxItemPropertyMapEntry* pMap) const { return (size_t)pMap; }
};

//----------------------------------------------------------------------

struct SvxIDPropertyCombine
{
    sal_uInt16  nWID;
    uno::Any    aAny;
};


SvxItemPropertySet::SvxItemPropertySet( const SfxItemPropertyMapEntry* pMap, SfxItemPool& rItemPool, sal_Bool bConvertTwips )
:   m_aPropertyMap( pMap ),
    _pMap(pMap), mbConvertTwips(bConvertTwips), mrItemPool( rItemPool )
{
}

//----------------------------------------------------------------------
SvxItemPropertySet::~SvxItemPropertySet()
{
    ClearAllUsrAny();
}

//----------------------------------------------------------------------
uno::Any* SvxItemPropertySet::GetUsrAnyForID(sal_uInt16 nWID) const
{
    for ( size_t i = 0, n = aCombineList.size(); i < n; ++i )
    {
        SvxIDPropertyCombine* pActual = aCombineList[ i ];
        if( pActual->nWID == nWID )
            return &pActual->aAny;
    }
    return NULL;
}

//----------------------------------------------------------------------
void SvxItemPropertySet::AddUsrAnyForID(const uno::Any& rAny, sal_uInt16 nWID)
{
    SvxIDPropertyCombine* pNew = new SvxIDPropertyCombine;
    pNew->nWID = nWID;
    pNew->aAny = rAny;
    aCombineList.push_back( pNew );
}

//----------------------------------------------------------------------

void SvxItemPropertySet::ClearAllUsrAny()
{
    for ( size_t i = 0, n = aCombineList.size(); i < n; ++i )
        delete aCombineList[ i ];
    aCombineList.clear();
}

//----------------------------------------------------------------------

sal_Bool SvxUnoCheckForPositiveValue( const uno::Any& rVal )
{
    sal_Bool bConvert = sal_True; // the default is that all metric items must be converted
    sal_Int32 nValue = 0;
    if( rVal >>= nValue )
        bConvert = (nValue > 0);
    return bConvert;
}


//----------------------------------------------------------------------
uno::Any SvxItemPropertySet::getPropertyValue( const SfxItemPropertySimpleEntry* pMap, const SfxItemSet& rSet, bool bSearchInParent, bool bDontConvertNegativeValues ) const
{
    uno::Any aVal;
    if(!pMap || !pMap->nWID)
        return aVal;

    const SfxPoolItem* pItem = 0;
    SfxItemPool* pPool = rSet.GetPool();
    rSet.GetItemState( pMap->nWID, bSearchInParent, &pItem );
    if( NULL == pItem && pPool )
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
        else if ( pMap->pType->getTypeClass() == uno::TypeClass_ENUM &&
              aVal.getValueType() == ::getCppuType((const sal_Int32*)0) )
        {
            // convert typeless SfxEnumItem to enum type
            sal_Int32 nEnum;
            aVal >>= nEnum;
            aVal.setValue( &nEnum, *pMap->pType );
        }
    }
    else
    {
        OSL_FAIL( "No SfxPoolItem found for property!" );
    }

    return aVal;
}

//----------------------------------------------------------------------
void SvxItemPropertySet::setPropertyValue( const SfxItemPropertySimpleEntry* pMap, const uno::Any& rVal, SfxItemSet& rSet, bool bDontConvertNegativeValues ) const
{
    if(!pMap || !pMap->nWID)
        return;

    // Get item
    const SfxPoolItem* pItem = 0;
    SfxItemState eState = rSet.GetItemState( pMap->nWID, sal_True, &pItem );
    SfxItemPool* pPool = rSet.GetPool();

    // Put UnoAny in the item value
    if(eState < SFX_ITEM_DEFAULT || pItem == NULL)
    {
        if( pPool == NULL )
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
            rSet.Put( *pNewItem, pMap->nWID );
        }
        delete pNewItem;
    }
}

//----------------------------------------------------------------------
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

    if( (pMap->nWID < OWN_ATTR_VALUE_START) && (pMap->nWID > OWN_ATTR_VALUE_END ) )
    {
        // Get Default from ItemPool
        if(mrItemPool.IsWhich(pMap->nWID))
            aSet.Put(mrItemPool.GetDefaultItem(pMap->nWID));
    }

    if(aSet.Count())
    {
        const SfxPoolItem* pItem = NULL;
        SfxItemState eState = aSet.GetItemState( pMap->nWID, sal_True, &pItem );
        if(eState >= SFX_ITEM_DEFAULT && pItem)
        {
            pItem->QueryValue( aVal, nMemberId );
            ((SvxItemPropertySet*)this)->AddUsrAnyForID(aVal, pMap->nWID);
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

    if ( pMap->pType->getTypeClass() == uno::TypeClass_ENUM &&
          aVal.getValueType() == ::getCppuType((const sal_Int32*)0) )
    {
        sal_Int32 nEnum;
        aVal >>= nEnum;

        aVal.setValue( &nEnum, *pMap->pType );
    }

    return aVal;
}

//----------------------------------------------------------------------

void SvxItemPropertySet::setPropertyValue( const SfxItemPropertySimpleEntry* pMap, const uno::Any& rVal ) const
{
    uno::Any* pUsrAny = GetUsrAnyForID(pMap->nWID);
    if(!pUsrAny)
        ((SvxItemPropertySet*)this)->AddUsrAnyForID(rVal, pMap->nWID);
    else
        *pUsrAny = rVal;
}

//----------------------------------------------------------------------

const SfxItemPropertySimpleEntry* SvxItemPropertySet::getPropertyMapEntry(const OUString &rName) const
{
    return m_aPropertyMap.getByName( rName );
 }

//----------------------------------------------------------------------

uno::Reference< beans::XPropertySetInfo >  SvxItemPropertySet::getPropertySetInfo() const
{
    if( !m_xInfo.is() )
        m_xInfo = new SfxItemPropertySetInfo( m_aPropertyMap );
    return m_xInfo;
}

//----------------------------------------------------------------------

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
                rMetric <<= (sal_Int8)(TWIPS_TO_MM(*(sal_Int8*)rMetric.getValue()));
                break;
            case uno::TypeClass_SHORT:
                rMetric <<= (sal_Int16)(TWIPS_TO_MM(*(sal_Int16*)rMetric.getValue()));
                break;
            case uno::TypeClass_UNSIGNED_SHORT:
                rMetric <<= (sal_uInt16)(TWIPS_TO_MM(*(sal_uInt16*)rMetric.getValue()));
                break;
            case uno::TypeClass_LONG:
                rMetric <<= (sal_Int32)(TWIPS_TO_MM(*(sal_Int32*)rMetric.getValue()));
                break;
            case uno::TypeClass_UNSIGNED_LONG:
                rMetric <<= (sal_uInt32)(TWIPS_TO_MM(*(sal_uInt32*)rMetric.getValue()));
                break;
            default:
                OSL_FAIL("AW: Missing unit translation to 100th mm!");
            }
            break;
        }
        default:
        {
            OSL_FAIL("AW: Missing unit translation to 100th mm!");
        }
    }
}

//----------------------------------------------------------------------

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
                    rMetric <<= (sal_Int8)(MM_TO_TWIPS(*(sal_Int8*)rMetric.getValue()));
                    break;
                case uno::TypeClass_SHORT:
                    rMetric <<= (sal_Int16)(MM_TO_TWIPS(*(sal_Int16*)rMetric.getValue()));
                    break;
                case uno::TypeClass_UNSIGNED_SHORT:
                    rMetric <<= (sal_uInt16)(MM_TO_TWIPS(*(sal_uInt16*)rMetric.getValue()));
                    break;
                case uno::TypeClass_LONG:
                    rMetric <<= (sal_Int32)(MM_TO_TWIPS(*(sal_Int32*)rMetric.getValue()));
                    break;
                case uno::TypeClass_UNSIGNED_LONG:
                    rMetric <<= (sal_uInt32)(MM_TO_TWIPS(*(sal_uInt32*)rMetric.getValue()));
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
