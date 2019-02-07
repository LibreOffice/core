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

#include <com/sun/star/drawing/BitmapMode.hpp>
#include <vcl/svapp.hxx>
#include <svl/itemset.hxx>
#include <svx/svdpool.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <svx/xflbstit.hxx>
#include <svx/xflbmtit.hxx>
#include <svx/svdobj.hxx>
#include <svx/unoprov.hxx>
#include <svx/unoshape.hxx>
#include <svx/unoshprp.hxx>

#include "unopback.hxx"
#include <drawdoc.hxx>
#include <unokywds.hxx>

using namespace ::com::sun::star;

const SvxItemPropertySet* ImplGetPageBackgroundPropertySet()
{
    static const SfxItemPropertyMapEntry aPageBackgroundPropertyMap_Impl[] =
    {
        FILL_PROPERTIES
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    static SvxItemPropertySet aPageBackgroundPropertySet_Impl( aPageBackgroundPropertyMap_Impl, SdrObject::GetGlobalDrawObjectItemPool() );
    return &aPageBackgroundPropertySet_Impl;
}

UNO3_GETIMPLEMENTATION_IMPL( SdUnoPageBackground );

SdUnoPageBackground::SdUnoPageBackground(
    SdDrawDocument* pDoc /* = NULL */,
    const SfxItemSet* pSet /* = NULL */)
:   mpPropSet(ImplGetPageBackgroundPropertySet()),
    mpDoc(pDoc)
{
    if( pDoc )
    {
        StartListening( *pDoc );
        mpSet = std::make_unique<SfxItemSet>( pDoc->GetPool(), svl::Items<XATTR_FILL_FIRST, XATTR_FILL_LAST>{} );

        if( pSet )
            mpSet->Put(*pSet);
    }
}

SdUnoPageBackground::~SdUnoPageBackground() throw()
{
    SolarMutexGuard g;

    if( mpDoc )
        EndListening( *mpDoc );
}

void SdUnoPageBackground::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    const SdrHint* pSdrHint = dynamic_cast<const SdrHint*>( &rHint );

    if( pSdrHint )
    {
        // delete item set if document is dying because then the pool
        // will also die
        if( pSdrHint->GetKind() == SdrHintKind::ModelCleared )
        {
            mpSet.reset();
            mpDoc = nullptr;
        }
    }

}

void SdUnoPageBackground::fillItemSet( SdDrawDocument* pDoc, SfxItemSet& rSet ) throw()
{
    rSet.ClearItem();

    if( mpSet == nullptr )
    {
        StartListening( *pDoc );
        mpDoc = pDoc;

        mpSet = std::make_unique<SfxItemSet>( *rSet.GetPool(), svl::Items<XATTR_FILL_FIRST, XATTR_FILL_LAST>{} );

        if( mpPropSet->AreThereOwnUsrAnys() )
        {
            PropertyEntryVector_t aProperties = mpPropSet->getPropertyMap().getPropertyEntries();

            for( const auto& rProp : aProperties )
            {
                uno::Any* pAny = mpPropSet->GetUsrAnyForID( rProp.nWID );
                if( pAny )
                {
                    OUString aPropertyName( rProp.sName );
                    switch( rProp.nWID )
                    {
                        case XATTR_FILLFLOATTRANSPARENCE :
                        case XATTR_FILLGRADIENT :
                        {
                            if ( ( pAny->getValueType() == ::cppu::UnoType< css::awt::Gradient>::get() )
                                && ( rProp.nMemberId == MID_FILLGRADIENT ) )
                            {
                                setPropertyValue( aPropertyName, *pAny );
                            }
                            else if ( ( pAny->getValueType() == ::cppu::UnoType<OUString>::get() ) &&
                                        ( rProp.nMemberId == MID_NAME ) )
                            {
                                setPropertyValue( aPropertyName, *pAny );
                            }
                        }
                        break;
                        case XATTR_FILLHATCH :
                        {
                            if ( ( pAny->getValueType() == ::cppu::UnoType< css::drawing::Hatch>::get() )
                                && ( rProp.nMemberId == MID_FILLHATCH ) )
                            {
                                setPropertyValue( aPropertyName, *pAny );
                            }
                            else if ( ( pAny->getValueType() == ::cppu::UnoType<OUString>::get() ) &&
                                        ( rProp.nMemberId == MID_NAME ) )
                            {
                                setPropertyValue( aPropertyName, *pAny );
                            }
                        }
                        break;
                        case XATTR_FILLBITMAP :
                        {
                            if (rProp.nMemberId == MID_BITMAP &&
                                (pAny->getValueType() == cppu::UnoType<css::awt::XBitmap>::get() ||
                                 pAny->getValueType() == cppu::UnoType<css::graphic::XGraphic>::get()))
                            {
                                setPropertyValue( aPropertyName, *pAny );
                            }
                            else if (pAny->getValueType() == ::cppu::UnoType<OUString>::get() && rProp.nMemberId == MID_NAME)
                            {
                                setPropertyValue( aPropertyName, *pAny );
                            }
                        }
                        break;

                        default:
                            setPropertyValue( aPropertyName, *pAny );
                    }
                }
            }
        }
    }

    rSet.Put( *mpSet );
}

// XServiceInfo
OUString SAL_CALL SdUnoPageBackground::getImplementationName()
{
    return OUString("SdUnoPageBackground");
}

sal_Bool SAL_CALL SdUnoPageBackground::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

uno::Sequence< OUString > SAL_CALL SdUnoPageBackground::getSupportedServiceNames()
{
    uno::Sequence< OUString > aNameSequence( 2 );
    OUString* pStrings = aNameSequence.getArray();

    *pStrings++ = sUNO_Service_PageBackground;
    *pStrings   = sUNO_Service_FillProperties;

    return aNameSequence;
}

// XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL SdUnoPageBackground::getPropertySetInfo()
{
    return mpPropSet->getPropertySetInfo();
}

void SAL_CALL SdUnoPageBackground::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
{
    SolarMutexGuard aGuard;

    const SfxItemPropertySimpleEntry* pEntry = getPropertyMapEntry( aPropertyName );

    if( pEntry == nullptr )
    {
        throw beans::UnknownPropertyException( aPropertyName, static_cast<cppu::OWeakObject*>(this));
    }

    if( mpSet )
    {
        if( pEntry->nWID == OWN_ATTR_FILLBMP_MODE )
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
        SfxItemSet aSet( rPool, {{pEntry->nWID, pEntry->nWID}});
        aSet.Put( *mpSet );

        if( !aSet.Count() )
            aSet.Put( rPool.GetDefaultItem( pEntry->nWID ) );

        if( pEntry->nMemberId == MID_NAME && ( pEntry->nWID == XATTR_FILLBITMAP || pEntry->nWID == XATTR_FILLGRADIENT || pEntry->nWID == XATTR_FILLHATCH || pEntry->nWID == XATTR_FILLFLOATTRANSPARENCE ) )
        {
            OUString aName;
            if(!(aValue >>= aName ))
                throw lang::IllegalArgumentException();

            SvxShape::SetFillAttribute( pEntry->nWID, aName, aSet );
        }
        else
        {
            SvxItemPropertySet_setPropertyValue( pEntry, aValue, aSet );
        }

        mpSet->Put( aSet );
    }
    else
    {
        if(pEntry->nWID)
            mpPropSet->setPropertyValue( pEntry, aValue );
    }
}

uno::Any SAL_CALL SdUnoPageBackground::getPropertyValue( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;

    uno::Any aAny;
    const SfxItemPropertySimpleEntry* pEntry = getPropertyMapEntry(PropertyName);

    if( pEntry == nullptr )
    {
        throw beans::UnknownPropertyException( PropertyName, static_cast<cppu::OWeakObject*>(this));
    }

    if( mpSet )
    {
        if( pEntry->nWID == OWN_ATTR_FILLBMP_MODE )
        {
            const XFillBmpStretchItem* pStretchItem = mpSet->GetItem<XFillBmpStretchItem>(XATTR_FILLBMP_STRETCH);
            const XFillBmpTileItem* pTileItem = mpSet->GetItem<XFillBmpTileItem>(XATTR_FILLBMP_TILE);

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
            SfxItemSet aSet( rPool, {{pEntry->nWID, pEntry->nWID}});
            aSet.Put( *mpSet );

            if( !aSet.Count() )
                aSet.Put( rPool.GetDefaultItem( pEntry->nWID ) );

            // get value from ItemSet
            aAny = SvxItemPropertySet_getPropertyValue( pEntry, aSet );
        }
    }
    else
    {
        if(pEntry->nWID)
            aAny = mpPropSet->getPropertyValue( pEntry );
    }
    return aAny;
}

void SAL_CALL SdUnoPageBackground::addPropertyChangeListener( const OUString& , const uno::Reference< beans::XPropertyChangeListener >&  ) {}
void SAL_CALL SdUnoPageBackground::removePropertyChangeListener( const OUString& , const uno::Reference< beans::XPropertyChangeListener >&  ) {}
void SAL_CALL SdUnoPageBackground::addVetoableChangeListener( const OUString& , const uno::Reference< beans::XVetoableChangeListener >&  ) {}
void SAL_CALL SdUnoPageBackground::removeVetoableChangeListener( const OUString& , const uno::Reference< beans::XVetoableChangeListener >&  ) {}

// XPropertyState
beans::PropertyState SAL_CALL SdUnoPageBackground::getPropertyState( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;

    const SfxItemPropertySimpleEntry* pEntry = getPropertyMapEntry(PropertyName);

    if( pEntry == nullptr )
        throw beans::UnknownPropertyException( PropertyName, static_cast<cppu::OWeakObject*>(this));

    if( mpSet )
    {
        if( pEntry->nWID == OWN_ATTR_FILLBMP_MODE )
        {
            if( mpSet->GetItemState( XATTR_FILLBMP_STRETCH, false ) == SfxItemState::SET ||
                mpSet->GetItemState( XATTR_FILLBMP_TILE, false ) == SfxItemState::SET )
            {
                return beans::PropertyState_DIRECT_VALUE;
            }
            else
            {
                return beans::PropertyState_AMBIGUOUS_VALUE;
            }
        }

        switch( mpSet->GetItemState( pEntry->nWID, false ) )
        {
        case SfxItemState::READONLY:
        case SfxItemState::SET:
            return beans::PropertyState_DIRECT_VALUE;
        case SfxItemState::DEFAULT:
            return beans::PropertyState_DEFAULT_VALUE;
        default:
//      case SfxItemState::DONTCARE:
//      case SfxItemState::DISABLED:
            return beans::PropertyState_AMBIGUOUS_VALUE;
        }
    }
    else
    {
        if( nullptr == mpPropSet->GetUsrAnyForID(pEntry->nWID) )
            return beans::PropertyState_DEFAULT_VALUE;
        else
            return beans::PropertyState_DIRECT_VALUE;
    }
}

uno::Sequence< beans::PropertyState > SAL_CALL SdUnoPageBackground::getPropertyStates( const uno::Sequence< OUString >& aPropertyName )
{
    SolarMutexGuard aGuard;

    sal_Int32 nCount = aPropertyName.getLength();
    const OUString* pNames = aPropertyName.getConstArray();

    uno::Sequence< beans::PropertyState > aPropertyStateSequence( nCount );
    beans::PropertyState* pState = aPropertyStateSequence.getArray();

    while( nCount-- )
        *pState++ = getPropertyState( *pNames++ );

    return aPropertyStateSequence;
}

void SAL_CALL SdUnoPageBackground::setPropertyToDefault( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;

    const SfxItemPropertySimpleEntry* pEntry = getPropertyMapEntry(PropertyName);

    if( pEntry == nullptr )
        throw beans::UnknownPropertyException( PropertyName, static_cast<cppu::OWeakObject*>(this));

    if( mpSet )
    {
        if( pEntry->nWID == OWN_ATTR_FILLBMP_MODE )
        {
            mpSet->ClearItem( XATTR_FILLBMP_STRETCH );
            mpSet->ClearItem( XATTR_FILLBMP_TILE );
        }
        else
        {
            mpSet->ClearItem( pEntry->nWID );
        }
    }
}

uno::Any SAL_CALL SdUnoPageBackground::getPropertyDefault( const OUString& aPropertyName )
{
    SolarMutexGuard aGuard;

    const SfxItemPropertySimpleEntry* pEntry = getPropertyMapEntry(aPropertyName);
    if( pEntry == nullptr || mpSet == nullptr )
        throw beans::UnknownPropertyException( aPropertyName, static_cast<cppu::OWeakObject*>(this));

    uno::Any aAny;
    if (pEntry->nWID == OWN_ATTR_FILLBMP_MODE)
    {
        aAny <<= drawing::BitmapMode_REPEAT;
    }
    else
    {
        SfxItemPool& rPool = *mpSet->GetPool();
        SfxItemSet aSet(rPool, { { pEntry->nWID, pEntry->nWID } });
        aSet.Put(rPool.GetDefaultItem(pEntry->nWID));

        aAny = SvxItemPropertySet_getPropertyValue(pEntry, aSet);
    }
    return aAny;
}

/** this is used because our property map is not sorted yet */
const SfxItemPropertySimpleEntry* SdUnoPageBackground::getPropertyMapEntry( const OUString& rPropertyName ) const throw()
{
    return mpPropSet->getPropertyMap().getByName(rPropertyName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
