/*************************************************************************
 *
 *  $RCSfile: unopback.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: cl $ $Date: 2000-11-08 11:21:55 $
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

#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

#include <cppuhelper/extract.hxx>
#include <rtl/uuid.h>
#include <rtl/memory.h>

#ifndef _SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif
#include <svx/unoprov.hxx>

#include "unopback.hxx"
#include "unohelp.hxx"
#include "drawdoc.hxx"
#include "unokywds.hxx"

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

    *pStrings++ = OUString::createFromAscii( sUNO_Service_PageBackground );
    *pStrings   = OUString::createFromAscii( sUNO_Service_AreaShapeDescriptor );

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
            SfxItemPool& rPool = *mpSet->GetPool();
            SfxItemSet aSet( rPool, pMap->nWID, pMap->nWID);
            aSet.Put( *mpSet );

            if( !aSet.Count() )
                aSet.Put( rPool.GetDefaultItem( pMap->nWID ) );

            maPropSet.setPropertyValue( pMap, aValue, aSet );
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
            SfxItemPool& rPool = *mpSet->GetPool();
            SfxItemSet aSet( rPool, pMap->nWID, pMap->nWID);
            aSet.Put( *mpSet );

            if( !aSet.Count() )
                aSet.Put( rPool.GetDefaultItem( pMap->nWID ) );

            // Hole Wert aus ItemSet
            aAny = maPropSet.getPropertyValue( pMap, aSet );
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
        switch( mpSet->GetItemState( pMap->nWID, sal_False ) )
        {
        case SFX_ITEM_READONLY:
        case SFX_ITEM_SET:
            return beans::PropertyState_DIRECT_VALUE;
        case SFX_ITEM_DEFAULT:
            return beans::PropertyState_DEFAULT_VALUE;
        default:
//      case SFX_ITEM_DONTCARE:
//      case SFX_ITEM_DISABLED:
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
        mpSet->ClearItem( pMap->nWID );
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
        SfxItemPool& rPool = *mpSet->GetPool();
        SfxItemSet aSet( rPool, pMap->nWID, pMap->nWID);
        aSet.Put( rPool.GetDefaultItem( pMap->nWID ) );

        aAny = maPropSet.getPropertyValue( pMap, aSet );
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

