/*************************************************************************
 *
 *  $RCSfile: unopool.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: cl $ $Date: 2001-03-05 12:54:58 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HDL_
#include <com/sun/star/beans/PropertyState.hpp>
#endif

#include <unotools/propertysetinfo.hxx>

#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _SVX_UNOPOOL_HXX_
#include "unopool.hxx"
#endif
#ifndef _SVDMODEL_HXX
#include "svdmodel.hxx"
#endif
#ifndef _SVDPOOL_HXX
#include "svdpool.hxx"
#endif
#ifndef SVX_UNOPROV_HXX
#include "unoprov.hxx"
#endif
#ifndef _SVDOBJ_HXX
#include "svdobj.hxx"
#endif
#ifndef _SVX_UNOSHPRP_HXX
#include "unoshprp.hxx"
#endif
#ifndef _SVX_XFLBSTIT_HXX
#include "xflbstit.hxx"
#endif
#ifndef _SVX_XFLBMTIT_HXX
#include "xflbmtit.hxx"
#endif
#ifndef _SVX_UNOWPAGE_HXX
#include "unopage.hxx"
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::rtl;
using namespace ::cppu;

SvxUnoDrawPool::SvxUnoDrawPool( SdrModel* pModel ) throw()
: PropertySetHelper( SvxPropertySetInfoPool::getOrCreate( SVXUNO_SERVICEID_COM_SUN_STAR_DRAWING_DEFAULTS ) ), mpModel( pModel )
{
    mpDefaultsPool = SdrObject::GetGlobalDrawObjectItemPool();
}

SvxUnoDrawPool::~SvxUnoDrawPool() throw()
{
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

void SvxUnoDrawPool::getAny( SfxItemPool* pPool, const utl::PropertyMapEntry* pEntry, Any& rValue )
    throw(UnknownPropertyException)
{
    switch( pEntry->mnWhich )
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
        pPool->GetDefaultItem( pEntry->mnWhich ).QueryValue( rValue, pEntry->mnMemberId );
    }


    // check for needed metric translation
    const SfxMapUnit eMapUnit = pPool->GetMetric(pEntry->mnWhich);
    if(pEntry->mnMemberId & SFX_METRIC_ITEM && eMapUnit != SFX_MAPUNIT_100TH_MM)
    {
        // map the metric of the itempool to 100th mm
        switch(eMapUnit)
        {
        case SFX_MAPUNIT_TWIP :
        {
            switch( rValue.getValueTypeClass() )
            {
            case uno::TypeClass_BYTE:
                rValue <<= (sal_Int8)(TWIPS_TO_MM(*(sal_Int8*)rValue.getValue()));
                break;
            case uno::TypeClass_SHORT:
                rValue <<= (sal_Int16)(TWIPS_TO_MM(*(sal_Int16*)rValue.getValue()));
                break;
            case uno::TypeClass_UNSIGNED_SHORT:
                rValue <<= (sal_uInt16)(TWIPS_TO_MM(*(sal_uInt16*)rValue.getValue()));
                break;
            case uno::TypeClass_LONG:
                rValue <<= (sal_Int32)(TWIPS_TO_MM(*(sal_Int32*)rValue.getValue()));
                break;
            case uno::TypeClass_UNSIGNED_LONG:
                rValue <<= (sal_uInt32)(TWIPS_TO_MM(*(sal_uInt32*)rValue.getValue()));
                break;
            default:
                DBG_ERROR("AW: Missing unit translation to 100th mm!");
            }
            break;
        }
        default:
        {
            DBG_ERROR("AW: Missing unit translation to 100th mm!");
        }
        }
    }

    // convert int32 to correct enum type if needed
    if ( pEntry->mpType->getTypeClass() == TypeClass_ENUM && rValue.getValueType() == ::getCppuType((const sal_Int32*)0) )
    {
        sal_Int32 nEnum;
        rValue >>= nEnum;

        rValue.setValue( &nEnum, *pEntry->mpType );
    }
}

void SvxUnoDrawPool::putAny( SfxItemPool* pPool, const utl::PropertyMapEntry* pEntry, const Any& rValue )
    throw(UnknownPropertyException, IllegalArgumentException)
{
    Any aValue( rValue );

    const SfxMapUnit eMapUnit = pPool->GetMetric(pEntry->mnWhich);
    if(pEntry->mnMemberId & SFX_METRIC_ITEM && eMapUnit != SFX_MAPUNIT_100TH_MM)
    {
        switch(eMapUnit)
        {
            case SFX_MAPUNIT_TWIP :
            {
                if( aValue.getValueType() == ::getCppuType(( const sal_Int32 *)0))
                    aValue <<= (sal_Int32)(MM_TO_TWIPS(*(sal_Int32*)aValue.getValue()));
                else if( aValue.getValueType() == ::getCppuType(( const sal_uInt32*)0))
                    aValue <<= (sal_uInt32)(MM_TO_TWIPS(*(sal_uInt32*)aValue.getValue()));
                else if( aValue.getValueType() == ::getCppuType(( const sal_uInt16*)0))
                    aValue <<= (sal_uInt16)(MM_TO_TWIPS(*(sal_uInt16*)aValue.getValue()));
                else
                    DBG_ERROR("AW: Missing unit translation to PoolMetrics!");
                break;
            }
            default:
            {
                DBG_ERROR("AW: Missing unit translation to PoolMetrics!");
            }
        }
    }

    const sal_uInt16 nWhich = pEntry->mnWhich;
    switch( nWhich )
    {
        case OWN_ATTR_FILLBMP_MODE:
            do
            {
                drawing::BitmapMode eMode;
                if(!(aValue >>= eMode) )
                {
                    sal_Int32 nMode;
                    if(!(aValue >>= nMode))
                        throw IllegalArgumentException();

                    eMode = (drawing::BitmapMode)nMode;
                }

                pPool->SetPoolDefaultItem( XFillBmpStretchItem( eMode == drawing::BitmapMode_STRETCH ) );
                pPool->SetPoolDefaultItem( XFillBmpTileItem( eMode == drawing::BitmapMode_REPEAT ) );
                return;
            }
            while(0);
            break;

    default:
        SfxPoolItem* pNewItem = pPool->GetDefaultItem( nWhich ).Clone();
        if( !pNewItem->PutValue( aValue, pEntry->mnMemberId ) )
            throw IllegalArgumentException();

        pPool->SetPoolDefaultItem( *pNewItem );
    }
}

void SvxUnoDrawPool::_setPropertyValues( const utl::PropertyMapEntry** ppEntries, const Any* pValues )
    throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException )
{
    vos::OGuard aGuard( Application::GetSolarMutex() );

    SfxItemPool* pPool = getModelPool( sal_False );

    DBG_ASSERT( pPool, "I need a SfxItemPool!" );
    if( NULL == pPool )
        throw UnknownPropertyException();

    while( *ppEntries )
        putAny( pPool, *ppEntries++, *pValues++ );
}

void SvxUnoDrawPool::_getPropertyValues( const utl::PropertyMapEntry** ppEntries, Any* pValue )
    throw(UnknownPropertyException, WrappedTargetException )
{
    vos::OGuard aGuard( Application::GetSolarMutex() );

    SfxItemPool* pPool = getModelPool( sal_True );

    DBG_ASSERT( pPool, "I need a SfxItemPool!" );
    if( NULL == pPool )
        throw UnknownPropertyException();

    while( *ppEntries )
        getAny( pPool, *ppEntries++, *pValue++ );
}

void SvxUnoDrawPool::_getPropertyStates( const utl::PropertyMapEntry** ppEntries, PropertyState* pStates )
    throw(UnknownPropertyException )
{
    vos::OGuard aGuard( Application::GetSolarMutex() );

    SfxItemPool* pPool = getModelPool( sal_True );

    if( pPool && pPool != mpDefaultsPool )
    {
        while( *ppEntries )
        {
            const sal_uInt16 nWhich = (*ppEntries)->mnWhich;

            switch( nWhich )
            {
            case OWN_ATTR_FILLBMP_MODE:
                {
                    if( pPool->GetDefaultItem( XATTR_FILLBMP_STRETCH ) == mpDefaultsPool->GetDefaultItem( XATTR_FILLBMP_STRETCH ) ||
                        pPool->GetDefaultItem( XATTR_FILLBMP_TILE ) == mpDefaultsPool->GetDefaultItem( XATTR_FILLBMP_TILE ) )
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
                if( pPool->GetDefaultItem( nWhich ) == mpDefaultsPool->GetDefaultItem( nWhich ) )
                {
                    *pStates = PropertyState_DEFAULT_VALUE;
                }
                else
                {
                    *pStates = PropertyState_DIRECT_VALUE;
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
            *pStates++ = PropertyState_DEFAULT_VALUE;
        return;
    }
}

void SvxUnoDrawPool::_setPropertyToDefault( const utl::PropertyMapEntry* pEntry )
    throw(UnknownPropertyException )
{
    vos::OGuard aGuard( Application::GetSolarMutex() );

    SfxItemPool* pPool = getModelPool( sal_True );

    if( pPool && pPool != mpDefaultsPool )
        pPool->Put( mpDefaultsPool->GetDefaultItem( pEntry->mnWhich ), pEntry->mnWhich );
}

Any SvxUnoDrawPool::_getPropertyDefault( const utl::PropertyMapEntry* pEntry )
    throw(UnknownPropertyException, WrappedTargetException )
{
    vos::OGuard aGuard( Application::GetSolarMutex() );

    Any aAny;
    getAny( mpDefaultsPool, pEntry, aAny );
    return aAny;
}

// XInterface

Any SAL_CALL SvxUnoDrawPool::queryInterface( const Type & rType )
    throw( RuntimeException )
{
    return OWeakAggObject::queryInterface( rType );
}

Any SAL_CALL SvxUnoDrawPool::queryAggregation( const Type & rType )
    throw(RuntimeException)
{
    Any aAny;

    if( rType == ::getCppuType((const Reference< XServiceInfo >*)0) )
        aAny <<= Reference< XServiceInfo >(this);
    else if( rType == ::getCppuType((const Reference< XTypeProvider >*)0) )
        aAny <<= Reference< XTypeProvider >(this);
    else if( rType == ::getCppuType((const Reference< XPropertySet >*)0) )
        aAny <<= Reference< XPropertySet >(this);
    else if( rType == ::getCppuType((const Reference< XPropertyState >*)0) )
        aAny <<= Reference< XPropertyState >(this);
    else if( rType == ::getCppuType((const Reference< XMultiPropertySet >*)0) )
        aAny <<= Reference< XMultiPropertySet >(this);
    else
        aAny <<= OWeakAggObject::queryAggregation( rType );

    return aAny;
}

void SAL_CALL SvxUnoDrawPool::acquire() throw(uno::RuntimeException)
{
    OWeakAggObject::acquire();
}

void SAL_CALL SvxUnoDrawPool::release() throw(uno::RuntimeException)
{
    OWeakAggObject::release();
}

uno::Sequence< uno::Type > SAL_CALL SvxUnoDrawPool::getTypes()
    throw (uno::RuntimeException)
{
    uno::Sequence< uno::Type > aTypes( 6 );
    uno::Type* pTypes = aTypes.getArray();

    *pTypes++ = ::getCppuType((const uno::Reference< XAggregation>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< XServiceInfo>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< XTypeProvider>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< XPropertySet>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< XPropertyState>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< XMultiPropertySet>*)0);

    return aTypes;
}

uno::Sequence< sal_Int8 > SAL_CALL SvxUnoDrawPool::getImplementationId()
    throw (uno::RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );

    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}

// XServiceInfo

sal_Bool SAL_CALL SvxUnoDrawPool::supportsService( const  OUString& ServiceName ) throw(RuntimeException)
{
    Sequence< OUString > aSNL( getSupportedServiceNames() );
    const OUString * pArray = aSNL.getConstArray();

    for( INT32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return TRUE;

    return FALSE;
}

OUString SAL_CALL SvxUnoDrawPool::getImplementationName() throw( RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("SvxUnoDrawPool") );
}

Sequence< OUString > SAL_CALL SvxUnoDrawPool::getSupportedServiceNames(  )
    throw( RuntimeException )
{
    Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.Defaults" ));
    return aSNS;
}
