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


#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif


#include <rtl/uuid.h>
#include <rtl/memory.h>

#ifndef _SVX_XFLBSTIT_HXX 
#include <bf_svx/xflbstit.hxx>
#endif
#ifndef _SVX_XFLBMTIT_HXX 
#include <bf_svx/xflbmtit.hxx>
#endif

#ifndef _SVDOBJ_HXX
#include <bf_svx/svdobj.hxx>
#endif
#ifndef _SVX_UNOSHAPE_HXX 
#include <bf_svx/unoshape.hxx>
#endif

#include "unopback.hxx"
#include "drawdoc.hxx"
#include "unokywds.hxx"
namespace binfilter {

using namespace ::rtl;
using namespace ::vos;
using namespace ::com::sun::star;

const SfxItemPropertyMap* ImplGetPageBackgroundPropertyMap()
{
    static const SfxItemPropertyMap aPageBackgroundPropertyMap_Impl[] =
    {
        FILL_PROPERTIES
        {0,0,0,0,0}
    };

    return aPageBackgroundPropertyMap_Impl;
}

UNO3_GETIMPLEMENTATION_IMPL( SdUnoPageBackground );

SdUnoPageBackground::SdUnoPageBackground( SdDrawDocument* pDoc /* = NULL */, SdrObject* pObj /* = NULL */ ) throw()
: maPropSet( ImplGetPageBackgroundPropertyMap() ), mpSet( NULL ), mpDoc( pDoc )
{
    if( pDoc )
    {
        StartListening( *pDoc );
        mpSet = new SfxItemSet( pDoc->GetPool(), XATTR_FILL_FIRST, XATTR_FILLRESERVED_LAST );

        if( pObj )
            mpSet->Put(pObj->GetItemSet());
    }
}

SdUnoPageBackground::SdUnoPageBackground( SdDrawDocument* pDoc, const SfxItemSet* pSet ) throw()
: maPropSet( ImplGetPageBackgroundPropertyMap() ), mpSet( NULL ), mpDoc( pDoc )
{
    if( pDoc )
    {
        StartListening( *pDoc );
        mpSet = new SfxItemSet( pDoc->GetPool(), XATTR_FILL_FIRST, XATTR_FILLRESERVED_LAST );

        if( pSet )
            mpSet->Put(*pSet);
    }
}

SdUnoPageBackground::~SdUnoPageBackground() throw()
{
    if( mpDoc )
        EndListening( *mpDoc );

    if( mpSet )
        delete mpSet;
}

void SdUnoPageBackground::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    const SdrHint* pSdrHint = PTR_CAST( SdrHint, &rHint );

    if( pSdrHint )
    {
        // delete item set if document is dying because then the pool
        // will also die
        if( pSdrHint->GetKind() == HINT_MODELCLEARED )
        {
            delete mpSet;
            mpSet = NULL;
            mpDoc = NULL;
        }
    }

}

void SdUnoPageBackground::fillItemSet( SdDrawDocument* pDoc, SfxItemSet& rSet ) throw()
{
    rSet.ClearItem();

    if( mpSet == NULL )
    {
        StartListening( *pDoc );
        mpDoc = pDoc;

        mpSet = new SfxItemSet( *rSet.GetPool(), XATTR_FILL_FIRST, XATTR_FILL_LAST );
        
        if( maPropSet.AreThereOwnUsrAnys() )
        {
            uno::Any* pAny;
            const SfxItemPropertyMap* pMap = maPropSet.getPropertyMap();
            while( pMap->pName )
            {
                pAny = maPropSet.GetUsrAnyForID( pMap->nWID );
                if( pAny )
                {
                    OUString aPropertyName( OUString::createFromAscii(pMap->pName));
                    setPropertyValue( aPropertyName, *pAny );
                }
                pMap++;
            }
        }
    }

    rSet.Put( *mpSet );
}

// XServiceInfo
OUString SAL_CALL SdUnoPageBackground::getImplementationName()
    throw(uno::RuntimeException)
{
    return OUString::createFromAscii( sUNO_SdUnoPageBackground );
}

sal_Bool SAL_CALL SdUnoPageBackground::supportsService( const OUString& ServiceName )
    throw(uno::RuntimeException)
{
    return SvxServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames() );
}

uno::Sequence< OUString > SAL_CALL SdUnoPageBackground::getSupportedServiceNames()
    throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aNameSequence( 2 );
    OUString* pStrings = aNameSequence.getArray();

    *pStrings++ = OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_Service_PageBackground ) );
    *pStrings   = OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_Service_FillProperties ) );

    return aNameSequence;
}

// XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL SdUnoPageBackground::getPropertySetInfo()
    throw(uno::RuntimeException)
{
    return maPropSet.getPropertySetInfo();
}

void SAL_CALL SdUnoPageBackground::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
    throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    const SfxItemPropertyMap* pMap = getPropertyMapEntry( aPropertyName );

    if( pMap == NULL )
    {
        throw beans::UnknownPropertyException();
    }
    else
    {
        if( mpSet )
        {
            if( pMap->nWID == OWN_ATTR_FILLBMP_MODE )
            {
                drawing::BitmapMode eMode;
                if( aValue >>= eMode )
                {
                    mpSet->Put( XFillBmpStretchItem( eMode == drawing::BitmapMode_STRETCH ) );
                    mpSet->Put( XFillBmpTileItem( eMode == drawing::BitmapMode_REPEAT ) );
                    return;
                }
                throw lang::IllegalArgumentException();
            }

            SfxItemPool& rPool = *mpSet->GetPool();
            SfxItemSet aSet( rPool,	pMap->nWID, pMap->nWID);
            aSet.Put( *mpSet );

            if( !aSet.Count() )
                aSet.Put( rPool.GetDefaultItem( pMap->nWID ) );

            if( pMap->nMemberId == MID_NAME && ( pMap->nWID == XATTR_FILLBITMAP || pMap->nWID == XATTR_FILLGRADIENT || pMap->nWID == XATTR_FILLHATCH || pMap->nWID == XATTR_FILLFLOATTRANSPARENCE ) )
            {
                OUString aName;
                if(!(aValue >>= aName ))
                    throw lang::IllegalArgumentException();

                SvxShape::SetFillAttribute( pMap->nWID, aName, aSet );
            }
            else
            {
                maPropSet.setPropertyValue( pMap, aValue, aSet );
            }

            mpSet->Put( aSet );
        }
        else
        {
            if(pMap && pMap->nWID)
                maPropSet.setPropertyValue( pMap, aValue );
        }
    }
}

uno::Any SAL_CALL SdUnoPageBackground::getPropertyValue( const OUString& PropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    uno::Any aAny;
    const SfxItemPropertyMap* pMap = getPropertyMapEntry(PropertyName);

    if( pMap == NULL )
    {
        throw beans::UnknownPropertyException();
    }
    else
    {
        if( mpSet )
        {
            if( pMap->nWID == OWN_ATTR_FILLBMP_MODE )
            {
                XFillBmpStretchItem* pStretchItem = (XFillBmpStretchItem*)mpSet->GetItem(XATTR_FILLBMP_STRETCH);
                XFillBmpTileItem* pTileItem = (XFillBmpTileItem*)mpSet->GetItem(XATTR_FILLBMP_TILE);

                if( pStretchItem && pTileItem )
                {
                    if( pTileItem->GetValue() )
                        aAny <<= drawing::BitmapMode_REPEAT;
                    else if( pStretchItem->GetValue() )
                        aAny <<= drawing::BitmapMode_STRETCH;
                    else
                        aAny <<= drawing::BitmapMode_NO_REPEAT;
                }
            }
            else
            {
                SfxItemPool& rPool = *mpSet->GetPool();
                SfxItemSet aSet( rPool,	pMap->nWID, pMap->nWID);
                aSet.Put( *mpSet );

                if( !aSet.Count() )
                    aSet.Put( rPool.GetDefaultItem( pMap->nWID ) );

                // Hole Wert aus ItemSet
                aAny = maPropSet.getPropertyValue( pMap, aSet );	
            }
        }
        else
        {
            if(pMap && pMap->nWID)
                aAny = maPropSet.getPropertyValue( pMap );
        }
    }
    return aAny;
}

void SAL_CALL SdUnoPageBackground::addPropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL SdUnoPageBackground::removePropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL SdUnoPageBackground::addVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL SdUnoPageBackground::removeVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}

// XPropertyState
beans::PropertyState SAL_CALL SdUnoPageBackground::getPropertyState( const OUString& PropertyName )
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    const SfxItemPropertyMap* pMap = getPropertyMapEntry(PropertyName);

    if( pMap == NULL )
        throw beans::UnknownPropertyException();

    if( mpSet )
    {
        if( pMap->nWID == OWN_ATTR_FILLBMP_MODE )
        {
            if( mpSet->GetItemState( XATTR_FILLBMP_STRETCH, false ) == SFX_ITEM_SET ||
                mpSet->GetItemState( XATTR_FILLBMP_TILE, false ) == SFX_ITEM_SET )
            {
                return beans::PropertyState_DIRECT_VALUE;
            }
            else
            {
                return beans::PropertyState_AMBIGUOUS_VALUE;
            }
        }

        switch( mpSet->GetItemState( pMap->nWID, sal_False ) )
        {
        case SFX_ITEM_READONLY:
        case SFX_ITEM_SET:
            return beans::PropertyState_DIRECT_VALUE;
        case SFX_ITEM_DEFAULT:
            return beans::PropertyState_DEFAULT_VALUE;
        default:
//		case SFX_ITEM_DONTCARE:
//		case SFX_ITEM_DISABLED:
            return beans::PropertyState_AMBIGUOUS_VALUE;
        }
    }
    else
    {
        if( NULL == maPropSet.GetUsrAnyForID(pMap->nWID) )
            return beans::PropertyState_DEFAULT_VALUE;
        else
            return beans::PropertyState_DIRECT_VALUE;
    }
}

uno::Sequence< beans::PropertyState > SAL_CALL SdUnoPageBackground::getPropertyStates( const uno::Sequence< OUString >& aPropertyName )
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    sal_Int32 nCount = aPropertyName.getLength();
    const OUString* pNames = aPropertyName.getConstArray();

    uno::Sequence< beans::PropertyState > aPropertyStateSequence( nCount );
    beans::PropertyState* pState = aPropertyStateSequence.getArray();

    while( nCount-- )
        *pState++ = getPropertyState( *pNames++ );

    return aPropertyStateSequence;
}

void SAL_CALL SdUnoPageBackground::setPropertyToDefault( const OUString& PropertyName )
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    const SfxItemPropertyMap* pMap = getPropertyMapEntry(PropertyName);

    if( pMap == NULL )
        throw beans::UnknownPropertyException();

    if( mpSet )
    {
        if( pMap->nWID == OWN_ATTR_FILLBMP_MODE )
        {
            mpSet->ClearItem( XATTR_FILLBMP_STRETCH );
            mpSet->ClearItem( XATTR_FILLBMP_TILE );
        }
        else
        {
            mpSet->ClearItem( pMap->nWID );
        }
    }
}

uno::Any SAL_CALL SdUnoPageBackground::getPropertyDefault( const OUString& aPropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    const SfxItemPropertyMap* pMap = getPropertyMapEntry(aPropertyName);
    if( pMap == NULL || mpSet == NULL )
        throw beans::UnknownPropertyException();

    uno::Any aAny;
    if( mpSet )
    {
        if( pMap->nWID == OWN_ATTR_FILLBMP_MODE )
        {
            aAny <<= drawing::BitmapMode_REPEAT;
        }
        else
        {
            SfxItemPool& rPool = *mpSet->GetPool();
            SfxItemSet aSet( rPool,	pMap->nWID, pMap->nWID);
            aSet.Put( rPool.GetDefaultItem( pMap->nWID ) );
    
            aAny = maPropSet.getPropertyValue( pMap, aSet );	
        }
    }
    return aAny;
}

/** this is used because our property map is not sorted yet */
const SfxItemPropertyMap* SdUnoPageBackground::getPropertyMapEntry( const OUString& rPropertyName ) const throw()
{
    const SfxItemPropertyMap *pMap = maPropSet.getPropertyMap();
    while( pMap->pName )
    {
        if( rPropertyName.compareToAscii( pMap->pName ) == 0 )
            return pMap;
        ++pMap;
    }

    return NULL;
}

}
