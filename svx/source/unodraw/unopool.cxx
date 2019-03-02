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

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/PropertyState.hpp>

#include <comphelper/propertysetinfo.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <vcl/svapp.hxx>
#include <svx/unopool.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpool.hxx>
#include <svx/unoprov.hxx>
#include <svx/svdobj.hxx>
#include <svx/unoshprp.hxx>
#include <svx/xflbstit.hxx>
#include <svx/xflbmtit.hxx>
#include <svx/unopage.hxx>
#include <svx/svdetc.hxx>
#include <editeng/editeng.hxx>
#include <tools/debug.hxx>

#include <svx/unoapi.hxx>
#include <memory>

using namespace ::com::sun::star;
using namespace ::cppu;

SvxUnoDrawPool::SvxUnoDrawPool(SdrModel* pModel, sal_Int32 nServiceId)
: PropertySetHelper( SvxPropertySetInfoPool::getOrCreate( nServiceId ) ), mpModel( pModel )
{
    init();
}

/* deprecated */
SvxUnoDrawPool::SvxUnoDrawPool(SdrModel* pModel)
: PropertySetHelper( SvxPropertySetInfoPool::getOrCreate( SVXUNO_SERVICEID_COM_SUN_STAR_DRAWING_DEFAULTS ) ), mpModel( pModel )
{
    init();
}

SvxUnoDrawPool::~SvxUnoDrawPool() throw()
{
    if (mpDefaultsPool)
    {
        SfxItemPool* pOutlPool = mpDefaultsPool->GetSecondaryPool();
        SfxItemPool::Free(mpDefaultsPool);
        SfxItemPool::Free(pOutlPool);
    }
}

void SvxUnoDrawPool::init()
{
    mpDefaultsPool = new SdrItemPool();
    SfxItemPool* pOutlPool=EditEngine::CreatePool();
    mpDefaultsPool->SetSecondaryPool(pOutlPool);

    SdrModel::SetTextDefaults( mpDefaultsPool, SdrEngineDefaults::GetFontHeight() );
    mpDefaultsPool->SetDefaultMetric(SdrEngineDefaults::GetMapUnit());
    mpDefaultsPool->FreezeIdRanges();
}

SfxItemPool* SvxUnoDrawPool::getModelPool( bool bReadOnly ) throw()
{
    if( mpModel )
    {
        return &mpModel->GetItemPool();
    }
    else
    {
        if( bReadOnly )
            return mpDefaultsPool;
        else
            return nullptr;
    }
}

void SvxUnoDrawPool::getAny( SfxItemPool const * pPool, const comphelper::PropertyMapEntry* pEntry, uno::Any& rValue )
{
    switch( pEntry->mnHandle )
    {
    case OWN_ATTR_FILLBMP_MODE:
        {
            if (pPool->GetDefaultItem(XATTR_FILLBMP_TILE).GetValue())
            {
                rValue <<= drawing::BitmapMode_REPEAT;
            }
            else if (pPool->GetDefaultItem(XATTR_FILLBMP_STRETCH).GetValue())
            {
                rValue <<= drawing::BitmapMode_STRETCH;
            }
            else
            {
                rValue <<= drawing::BitmapMode_NO_REPEAT;
            }
            break;
        }
    default:
        {
            const MapUnit eMapUnit = pPool->GetMetric(static_cast<sal_uInt16>(pEntry->mnHandle));

            sal_uInt8 nMemberId = pEntry->mnMemberId;
            if( eMapUnit == MapUnit::Map100thMM )
                nMemberId &= (~CONVERT_TWIPS);

            // DVO, OD 10.10.2003 #i18732#
            // Assure, that ID is a Which-ID (it could be a Slot-ID.)
            // Thus, convert handle to Which-ID.
            pPool->GetDefaultItem( pPool->GetWhich( static_cast<sal_uInt16>(pEntry->mnHandle) ) ).QueryValue( rValue, nMemberId );
        }
    }


    // check for needed metric translation
    const MapUnit eMapUnit = pPool->GetMetric(static_cast<sal_uInt16>(pEntry->mnHandle));
    if(pEntry->mnMoreFlags & PropertyMoreFlags::METRIC_ITEM && eMapUnit != MapUnit::Map100thMM)
    {
        SvxUnoConvertToMM( eMapUnit, rValue );
    }
    // convert int32 to correct enum type if needed
    else if ( pEntry->maType.getTypeClass() == uno::TypeClass_ENUM && rValue.getValueType() == ::cppu::UnoType<sal_Int32>::get() )
    {
        sal_Int32 nEnum;
        rValue >>= nEnum;

        rValue.setValue( &nEnum, pEntry->maType );
    }
}

void SvxUnoDrawPool::putAny( SfxItemPool* pPool, const comphelper::PropertyMapEntry* pEntry, const uno::Any& rValue )
{
    uno::Any aValue( rValue );

    const MapUnit eMapUnit = pPool->GetMetric(static_cast<sal_uInt16>(pEntry->mnHandle));
    if(pEntry->mnMoreFlags & PropertyMoreFlags::METRIC_ITEM && eMapUnit != MapUnit::Map100thMM)
    {
        SvxUnoConvertFromMM( eMapUnit, aValue );
    }

    // DVO, OD 10.10.2003 #i18732#
    // Assure, that ID is a Which-ID (it could be a Slot-ID.)
    // Thus, convert handle to Which-ID.
    const sal_uInt16 nWhich = pPool->GetWhich( static_cast<sal_uInt16>(pEntry->mnHandle) );
    switch( nWhich )
    {
        case OWN_ATTR_FILLBMP_MODE:
            do
            {
                drawing::BitmapMode eMode;
                if(!(aValue >>= eMode) )
                {
                    sal_Int32 nMode = 0;
                    if(!(aValue >>= nMode))
                        throw lang::IllegalArgumentException();

                    eMode = static_cast<drawing::BitmapMode>(nMode);
                }

                pPool->SetPoolDefaultItem( XFillBmpStretchItem( eMode == drawing::BitmapMode_STRETCH ) );
                pPool->SetPoolDefaultItem( XFillBmpTileItem( eMode == drawing::BitmapMode_REPEAT ) );
                return;
            }
            while(false);

    default:
        {
            std::unique_ptr<SfxPoolItem> pNewItem( pPool->GetDefaultItem( nWhich ).Clone() );
            sal_uInt8 nMemberId = pEntry->mnMemberId;
            if( pPool->GetMetric(nWhich) == MapUnit::Map100thMM )
                nMemberId &= (~CONVERT_TWIPS);

            if( !pNewItem->PutValue( aValue, nMemberId ) )
                throw lang::IllegalArgumentException();

            pPool->SetPoolDefaultItem( *pNewItem );
        }
    }
}

void SvxUnoDrawPool::_setPropertyValues( const comphelper::PropertyMapEntry** ppEntries, const uno::Any* pValues )
{
    SolarMutexGuard aGuard;

    SfxItemPool* pPool = getModelPool( false );

    DBG_ASSERT( pPool, "I need a SfxItemPool!" );
    if( nullptr == pPool )
        throw beans::UnknownPropertyException( "no pool, no properties..", static_cast<cppu::OWeakObject*>(this));

    while( *ppEntries )
        putAny( pPool, *ppEntries++, *pValues++ );
}

void SvxUnoDrawPool::_getPropertyValues( const comphelper::PropertyMapEntry** ppEntries, uno::Any* pValue )
{
    SolarMutexGuard aGuard;

    SfxItemPool* pPool = getModelPool( true );

    DBG_ASSERT( pPool, "I need a SfxItemPool!" );
    if( nullptr == pPool )
        throw beans::UnknownPropertyException( "no pool, no properties..", static_cast<cppu::OWeakObject*>(this));

    while( *ppEntries )
        getAny( pPool, *ppEntries++, *pValue++ );
}

void SvxUnoDrawPool::_getPropertyStates( const comphelper::PropertyMapEntry** ppEntries, beans::PropertyState* pStates )
{
    SolarMutexGuard aGuard;

    SfxItemPool* pPool = getModelPool( true );

    if( pPool && pPool != mpDefaultsPool )
    {
        while( *ppEntries )
        {
            // OD 13.10.2003 #i18732#
            // Assure, that ID is a Which-ID (it could be a Slot-ID.)
            // Thus, convert handle to Which-ID.
            const sal_uInt16 nWhich = pPool->GetWhich( static_cast<sal_uInt16>((*ppEntries)->mnHandle) );

            switch( nWhich )
            {
            case OWN_ATTR_FILLBMP_MODE:
                {
                    // use method <IsStaticDefaultItem(..)> instead of using
                    // probably incompatible item pool <mpDefaultPool>.
                    if ( IsStaticDefaultItem( &(pPool->GetDefaultItem( XATTR_FILLBMP_STRETCH )) ) ||
                         IsStaticDefaultItem( &(pPool->GetDefaultItem( XATTR_FILLBMP_TILE )) ) )
                    {
                        *pStates = beans::PropertyState_DEFAULT_VALUE;
                    }
                    else
                    {
                        *pStates = beans::PropertyState_DIRECT_VALUE;
                    }
                }
                break;
            default:
                // OD 13.10.2003 #i18732# - correction:
                // use method <IsStaticDefaultItem(..)> instead of using probably
                // incompatible item pool <mpDefaultPool>.
                const SfxPoolItem& r1 = pPool->GetDefaultItem( nWhich );
                //const SfxPoolItem& r2 = mpDefaultPool->GetDefaultItem( nWhich );

                if ( IsStaticDefaultItem( &r1 ) )
                {
                    *pStates = beans::PropertyState_DEFAULT_VALUE;
                }
                else
                {
                    *pStates = beans::PropertyState_DIRECT_VALUE;
                }
            }

            pStates++;
            ppEntries++;
        }
    }
    else
    {
        // as long as we have no model, all properties are default
        while( *ppEntries++ )
            *pStates++ = beans::PropertyState_DEFAULT_VALUE;
        return;
    }
}

void SvxUnoDrawPool::_setPropertyToDefault( const comphelper::PropertyMapEntry* pEntry )
{
    SolarMutexGuard aGuard;

    SfxItemPool* pPool = getModelPool( true );

    // OD 10.10.2003 #i18732#
    // Assure, that ID is a Which-ID (it could be a Slot-ID.)
    // Thus, convert handle to Which-ID.
    const sal_uInt16 nWhich = pPool->GetWhich( static_cast<sal_uInt16>(pEntry->mnHandle) );
    if ( pPool && pPool != mpDefaultsPool )
    {
        // OD 13.10.2003 #i18732# - use method <ResetPoolDefaultItem(..)>
        // instead of using probably incompatible item pool <mpDefaultsPool>.
        pPool->ResetPoolDefaultItem( nWhich );
    }
}

uno::Any SvxUnoDrawPool::_getPropertyDefault( const comphelper::PropertyMapEntry* pEntry )
{
    SolarMutexGuard aGuard;

    // OD 13.10.2003 #i18732# - use method <GetPoolDefaultItem(..)> instead of
    // using probably incompatible item pool <mpDefaultsPool>
    uno::Any aAny;
    SfxItemPool* pPool = getModelPool( true );
    const sal_uInt16 nWhich = pPool->GetWhich( static_cast<sal_uInt16>(pEntry->mnHandle) );
    const SfxPoolItem *pItem = pPool->GetPoolDefaultItem ( nWhich );
    if (pItem)
    {
        pItem->QueryValue( aAny, pEntry->mnMemberId );
    }

    return aAny;
}

// XInterface

uno::Any SAL_CALL SvxUnoDrawPool::queryInterface( const uno::Type & rType )
{
    return OWeakAggObject::queryInterface( rType );
}

uno::Any SAL_CALL SvxUnoDrawPool::queryAggregation( const uno::Type & rType )
{
    uno::Any aAny;

    if( rType == cppu::UnoType<lang::XServiceInfo>::get())
        aAny <<= uno::Reference< lang::XServiceInfo >(this);
    else if( rType == cppu::UnoType<lang::XTypeProvider>::get())
        aAny <<= uno::Reference< lang::XTypeProvider >(this);
    else if( rType == cppu::UnoType<beans::XPropertySet>::get())
        aAny <<= uno::Reference< beans::XPropertySet >(this);
    else if( rType == cppu::UnoType<beans::XPropertyState>::get())
        aAny <<= uno::Reference< beans::XPropertyState >(this);
    else if( rType == cppu::UnoType<beans::XMultiPropertySet>::get())
        aAny <<= uno::Reference< beans::XMultiPropertySet >(this);
    else
        aAny = OWeakAggObject::queryAggregation( rType );

    return aAny;
}

void SAL_CALL SvxUnoDrawPool::acquire() throw ( )
{
    OWeakAggObject::acquire();
}

void SAL_CALL SvxUnoDrawPool::release() throw ( )
{
    OWeakAggObject::release();
}

uno::Sequence< uno::Type > SAL_CALL SvxUnoDrawPool::getTypes()
{
    static const uno::Sequence aTypes {
        cppu::UnoType<uno::XAggregation>::get(),
        cppu::UnoType<lang::XServiceInfo>::get(),
        cppu::UnoType<lang::XTypeProvider>::get(),
        cppu::UnoType<beans::XPropertySet>::get(),
        cppu::UnoType<beans::XPropertyState>::get(),
        cppu::UnoType<beans::XMultiPropertySet>::get() };
    return aTypes;
}

uno::Sequence< sal_Int8 > SAL_CALL SvxUnoDrawPool::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// XServiceInfo
sal_Bool SAL_CALL SvxUnoDrawPool::supportsService( const  OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

OUString SAL_CALL SvxUnoDrawPool::getImplementationName()
{
    return OUString("SvxUnoDrawPool");
}

uno::Sequence< OUString > SAL_CALL SvxUnoDrawPool::getSupportedServiceNames(  )
{
    uno::Sequence<OUString> aSNS { "com.sun.star.drawing.Defaults" };
    return aSNS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
