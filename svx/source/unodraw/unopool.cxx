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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/PropertyState.hpp>

#include <comphelper/propertysetinfo.hxx>
#include <rtl/uuid.h>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include "svx/unopool.hxx"
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

#include "svx/unoapi.hxx"
#include <memory>

using namespace ::com::sun::star;
using namespace ::rtl;
using namespace ::cppu;

SvxUnoDrawPool::SvxUnoDrawPool( SdrModel* pModel, sal_Int32 nServiceId ) throw()
: PropertySetHelper( SvxPropertySetInfoPool::getOrCreate( nServiceId ) ), mpModel( pModel )
{
    init();
}

/* deprecated */
SvxUnoDrawPool::SvxUnoDrawPool( SdrModel* pModel ) throw()
: PropertySetHelper( SvxPropertySetInfoPool::getOrCreate( SVXUNO_SERVICEID_COM_SUN_STAR_DRAWING_DEFAULTS ) ), mpModel( pModel )
{
    init();
}

SvxUnoDrawPool::~SvxUnoDrawPool() throw()
{
    SfxItemPool::Free(mpDefaultsPool);
}

void SvxUnoDrawPool::init()
{
    mpDefaultsPool = new SdrItemPool();
    SfxItemPool* pOutlPool=EditEngine::CreatePool();
    mpDefaultsPool->SetSecondaryPool(pOutlPool);

    SdrModel::SetTextDefaults( mpDefaultsPool, SdrEngineDefaults::GetFontHeight() );
    mpDefaultsPool->SetDefaultMetric((SfxMapUnit)SdrEngineDefaults::GetMapUnit());
    mpDefaultsPool->FreezeIdRanges();
}

SfxItemPool* SvxUnoDrawPool::getModelPool( sal_Bool bReadOnly ) throw()
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
            return NULL;
    }
}

void SvxUnoDrawPool::getAny( SfxItemPool* pPool, const comphelper::PropertyMapEntry* pEntry, uno::Any& rValue )
    throw(beans::UnknownPropertyException)
{
    switch( pEntry->mnHandle )
    {
    case OWN_ATTR_FILLBMP_MODE:
        {
            XFillBmpStretchItem* pStretchItem = (XFillBmpStretchItem*)&pPool->GetDefaultItem(XATTR_FILLBMP_STRETCH);
            XFillBmpTileItem* pTileItem = (XFillBmpTileItem*)&pPool->GetDefaultItem(XATTR_FILLBMP_TILE);
            if( pTileItem && pTileItem->GetValue() )
            {
                rValue <<= drawing::BitmapMode_REPEAT;
            }
            else if( pStretchItem && pStretchItem->GetValue() )
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
            const SfxMapUnit eMapUnit = pPool ? pPool->GetMetric((sal_uInt16)pEntry->mnHandle) : SFX_MAPUNIT_100TH_MM;

            sal_uInt8 nMemberId = pEntry->mnMemberId & (~SFX_METRIC_ITEM);
            if( eMapUnit == SFX_MAPUNIT_100TH_MM )
                nMemberId &= (~CONVERT_TWIPS);

            // DVO, OD 10.10.2003 #i18732#
            // Assure, that ID is a Which-ID (it could be a Slot-ID.)
            // Thus, convert handle to Which-ID.
            pPool->GetDefaultItem( pPool->GetWhich( (sal_uInt16)pEntry->mnHandle ) ).QueryValue( rValue, nMemberId );
        }
    }


    // check for needed metric translation
    const SfxMapUnit eMapUnit = pPool->GetMetric((sal_uInt16)pEntry->mnHandle);
    if(pEntry->mnMemberId & SFX_METRIC_ITEM && eMapUnit != SFX_MAPUNIT_100TH_MM)
    {
        SvxUnoConvertToMM( eMapUnit, rValue );
    }
    // convert int32 to correct enum type if needed
    else if ( pEntry->mpType->getTypeClass() == uno::TypeClass_ENUM && rValue.getValueType() == ::getCppuType((const sal_Int32*)0) )
    {
        sal_Int32 nEnum;
        rValue >>= nEnum;

        rValue.setValue( &nEnum, *pEntry->mpType );
    }
}

void SvxUnoDrawPool::putAny( SfxItemPool* pPool, const comphelper::PropertyMapEntry* pEntry, const uno::Any& rValue )
    throw(beans::UnknownPropertyException, lang::IllegalArgumentException)
{
    uno::Any aValue( rValue );

    const SfxMapUnit eMapUnit = pPool->GetMetric((sal_uInt16)pEntry->mnHandle);
    if(pEntry->mnMemberId & SFX_METRIC_ITEM && eMapUnit != SFX_MAPUNIT_100TH_MM)
    {
        SvxUnoConvertFromMM( eMapUnit, aValue );
    }

    // DVO, OD 10.10.2003 #i18732#
    // Assure, that ID is a Which-ID (it could be a Slot-ID.)
    // Thus, convert handle to Which-ID.
    const sal_uInt16 nWhich = pPool->GetWhich( (sal_uInt16)pEntry->mnHandle );
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

                    eMode = (drawing::BitmapMode)nMode;
                }

                pPool->SetPoolDefaultItem( XFillBmpStretchItem( eMode == drawing::BitmapMode_STRETCH ) );
                pPool->SetPoolDefaultItem( XFillBmpTileItem( eMode == drawing::BitmapMode_REPEAT ) );
                return;
            }
            while(0);

    default:
        {
            ::std::auto_ptr<SfxPoolItem> pNewItem( pPool->GetDefaultItem( nWhich ).Clone() );
            sal_uInt8 nMemberId = pEntry->mnMemberId & (~SFX_METRIC_ITEM);
            if( !pPool || (pPool->GetMetric(nWhich) == SFX_MAPUNIT_100TH_MM) )
                nMemberId &= (~CONVERT_TWIPS);

            if( !pNewItem->PutValue( aValue, nMemberId ) )
                throw lang::IllegalArgumentException();

            pPool->SetPoolDefaultItem( *pNewItem );
        }
    }
}

void SvxUnoDrawPool::_setPropertyValues( const comphelper::PropertyMapEntry** ppEntries, const uno::Any* pValues )
    throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException )
{
    SolarMutexGuard aGuard;

    SfxItemPool* pPool = getModelPool( sal_False );

    DBG_ASSERT( pPool, "I need a SfxItemPool!" );
    if( NULL == pPool )
        throw beans::UnknownPropertyException();

    while( *ppEntries )
        putAny( pPool, *ppEntries++, *pValues++ );
}

void SvxUnoDrawPool::_getPropertyValues( const comphelper::PropertyMapEntry** ppEntries, uno::Any* pValue )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException )
{
    SolarMutexGuard aGuard;

    SfxItemPool* pPool = getModelPool( sal_True );

    DBG_ASSERT( pPool, "I need a SfxItemPool!" );
    if( NULL == pPool )
        throw beans::UnknownPropertyException();

    while( *ppEntries )
        getAny( pPool, *ppEntries++, *pValue++ );
}

void SvxUnoDrawPool::_getPropertyStates( const comphelper::PropertyMapEntry** ppEntries, beans::PropertyState* pStates )
    throw(beans::UnknownPropertyException )
{
    SolarMutexGuard aGuard;

    SfxItemPool* pPool = getModelPool( sal_True );

    if( pPool && pPool != mpDefaultsPool )
    {
        while( *ppEntries )
        {
            // OD 13.10.2003 #i18732#
            // Assure, that ID is a Which-ID (it could be a Slot-ID.)
            // Thus, convert handle to Which-ID.
            const sal_uInt16 nWhich = pPool->GetWhich( ((sal_uInt16)(*ppEntries)->mnHandle) );

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
    throw(beans::UnknownPropertyException )
{
    SolarMutexGuard aGuard;

    SfxItemPool* pPool = getModelPool( sal_True );

    // OD 10.10.2003 #i18732#
    // Assure, that ID is a Which-ID (it could be a Slot-ID.)
    // Thus, convert handle to Which-ID.
    const sal_uInt16 nWhich = pPool->GetWhich( (sal_uInt16)pEntry->mnHandle );
    if ( pPool && pPool != mpDefaultsPool )
    {
        // OD 13.10.2003 #i18732# - use method <ResetPoolDefaultItem(..)>
        // instead of using probably incompatible item pool <mpDefaultsPool>.
        pPool->ResetPoolDefaultItem( nWhich );
    }
}

uno::Any SvxUnoDrawPool::_getPropertyDefault( const comphelper::PropertyMapEntry* pEntry )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException )
{
    SolarMutexGuard aGuard;

    // OD 13.10.2003 #i18732# - use method <GetPoolDefaultItem(..)> instead of
    // using probably incompatible item pool <mpDefaultsPool>
    uno::Any aAny;
    SfxItemPool* pPool = getModelPool( sal_True );
    const sal_uInt16 nWhich = pPool->GetWhich( (sal_uInt16)pEntry->mnHandle );
    const SfxPoolItem *pItem = pPool->GetPoolDefaultItem ( nWhich );
    pItem->QueryValue( aAny, pEntry->mnMemberId );

    return aAny;
}

// XInterface

uno::Any SAL_CALL SvxUnoDrawPool::queryInterface( const uno::Type & rType )
    throw( uno::RuntimeException )
{
    return OWeakAggObject::queryInterface( rType );
}

uno::Any SAL_CALL SvxUnoDrawPool::queryAggregation( const uno::Type & rType )
    throw(uno::RuntimeException)
{
    uno::Any aAny;

    if( rType == ::getCppuType((const uno::Reference< lang::XServiceInfo >*)0) )
        aAny <<= uno::Reference< lang::XServiceInfo >(this);
    else if( rType == ::getCppuType((const uno::Reference< lang::XTypeProvider >*)0) )
        aAny <<= uno::Reference< lang::XTypeProvider >(this);
    else if( rType == ::getCppuType((const uno::Reference< beans::XPropertySet >*)0) )
        aAny <<= uno::Reference< beans::XPropertySet >(this);
    else if( rType == ::getCppuType((const uno::Reference< beans::XPropertyState >*)0) )
        aAny <<= uno::Reference< beans::XPropertyState >(this);
    else if( rType == ::getCppuType((const uno::Reference< beans::XMultiPropertySet >*)0) )
        aAny <<= uno::Reference< beans::XMultiPropertySet >(this);
    else
        aAny <<= OWeakAggObject::queryAggregation( rType );

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
    throw (uno::RuntimeException)
{
    uno::Sequence< uno::Type > aTypes( 6 );
    uno::Type* pTypes = aTypes.getArray();

    *pTypes++ = ::getCppuType((const uno::Reference< uno::XAggregation>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< lang::XServiceInfo>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< lang::XTypeProvider>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< beans::XPropertySet>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< beans::XPropertyState>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< beans::XMultiPropertySet>*)0);

    return aTypes;
}

uno::Sequence< sal_Int8 > SAL_CALL SvxUnoDrawPool::getImplementationId()
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}

// XServiceInfo

sal_Bool SAL_CALL SvxUnoDrawPool::supportsService( const  OUString& ServiceName ) throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSNL( getSupportedServiceNames() );
    const OUString * pArray = aSNL.getConstArray();

    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;

    return sal_False;
}

OUString SAL_CALL SvxUnoDrawPool::getImplementationName() throw( uno::RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("SvxUnoDrawPool") );
}

uno::Sequence< OUString > SAL_CALL SvxUnoDrawPool::getSupportedServiceNames(  )
    throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.Defaults" ));
    return aSNS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
