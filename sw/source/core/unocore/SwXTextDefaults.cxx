/*************************************************************************
 *
 *  $RCSfile: SwXTextDefaults.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mtg $ $Date: 2001-03-21 14:51:13 $
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
 *  Contributor(s): Martin Gallwey (gallwey@sun.com)
 *
 *
 ************************************************************************/
#ifndef _SW_XTEXT_DEFAULTS_HXX
#include <SwXTextDefaults.hxx>
#endif


#ifndef _UNOMAP_HXX
#include <unomap.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _UNOPRNMS_HXX
#include <unoprnms.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif


using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;

SwXTextDefaults::SwXTextDefaults ( SwDoc * pNewDoc )
: pDoc (pNewDoc)
, aPropSet (aSwMapProvider.GetPropertyMap ( PROPERTY_MAP_TEXT_DEFAULT ) )
{
}
SwXTextDefaults::~SwXTextDefaults ()
{
}
Reference< XPropertySetInfo > SAL_CALL SwXTextDefaults::getPropertySetInfo(  )
        throw(RuntimeException)
{
    static uno::Reference < XPropertySetInfo > xRef = aPropSet.getPropertySetInfo();
    return xRef;
}
void SAL_CALL SwXTextDefaults::setPropertyValue( const OUString& rPropertyName, const Any& aValue )
        throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex());
    if (!pDoc)
        throw RuntimeException();
    const SfxItemPropertyMap *pMap = SfxItemPropertyMap::GetByName( aPropSet.getPropertyMap(), rPropertyName);
    if (!pMap)
        throw UnknownPropertyException();
    if ( pMap->nFlags & PropertyAttribute::READONLY)
        throw IllegalArgumentException();
    switch (pMap->nWID)
    {
        case RES_CHRATR_FONT:
        case RES_PARATR_TABSTOP:
        {
            const SfxPoolItem& rItem = pDoc->GetDefault(pMap->nWID);
            SfxPoolItem * pNewItem = rItem.Clone();
            pNewItem->PutValue( aValue, pMap->nMemberId);
            pDoc->SetDefault(*pNewItem);
            delete pNewItem;
        }
        break;
    }
}
Any SAL_CALL SwXTextDefaults::getPropertyValue( const OUString& rPropertyName )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex());
    if (!pDoc)
        throw RuntimeException();
    const SfxItemPropertyMap *pMap = SfxItemPropertyMap::GetByName( aPropSet.getPropertyMap(), rPropertyName);
    if (!pMap)
        throw UnknownPropertyException();
    if ( pMap->nFlags & PropertyAttribute::READONLY)
        throw IllegalArgumentException();
    Any aRet;
    switch (pMap->nWID)
    {
        case RES_CHRATR_FONT:
        case RES_PARATR_TABSTOP:
        {
            const SfxPoolItem& rItem = pDoc->GetDefault(pMap->nWID);
            rItem.QueryValue( aRet, pMap->nMemberId );
        }
        break;
    }

    return aRet;
}
void SAL_CALL SwXTextDefaults::addPropertyChangeListener( const OUString& rPropertyName, const Reference< XPropertyChangeListener >& xListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    DBG_WARNING ( "not implemented" );
}
void SAL_CALL SwXTextDefaults::removePropertyChangeListener( const OUString& rPropertyName, const Reference< XPropertyChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    DBG_WARNING ( "not implemented" );
}
void SAL_CALL SwXTextDefaults::addVetoableChangeListener( const OUString& rPropertyName, const Reference< XVetoableChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    DBG_WARNING ( "not implemented" );
}
void SAL_CALL SwXTextDefaults::removeVetoableChangeListener( const OUString& rPropertyName, const Reference< XVetoableChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    DBG_WARNING ( "not implemented" );
}
    // XPropertyState
PropertyState SAL_CALL SwXTextDefaults::getPropertyState( const OUString& rPropertyName )
        throw(UnknownPropertyException, RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex());
    PropertyState eRet = PropertyState_DIRECT_VALUE;
    if (!pDoc)
        throw RuntimeException();
    const SfxItemPropertyMap *pMap = SfxItemPropertyMap::GetByName( aPropSet.getPropertyMap(), rPropertyName);
    if (!pMap)
        throw UnknownPropertyException();
    if ( pMap->nFlags & PropertyAttribute::READONLY)
        throw IllegalArgumentException();
    switch (pMap->nWID)
    {
        case RES_CHRATR_FONT:
        case RES_PARATR_TABSTOP:
        {
            const SfxPoolItem& rItem = pDoc->GetDefault(pMap->nWID);
            if (IsStaticDefaultItem ( &rItem ) )
                eRet = PropertyState_DEFAULT_VALUE;
        }
        break;
    }
    return eRet;
}
Sequence< PropertyState > SAL_CALL SwXTextDefaults::getPropertyStates( const Sequence< OUString >& rPropertyNames )
        throw(UnknownPropertyException, RuntimeException)
{
    const sal_Int32 nCount = rPropertyNames.getLength();
    const OUString * pNames = rPropertyNames.getConstArray();
    Sequence < PropertyState > aRet ( nCount );
    PropertyState *pState = aRet.getArray();

    for ( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++)
        pState[nIndex] = getPropertyState( pNames[nIndex] );

    return aRet;
}
void SAL_CALL SwXTextDefaults::setPropertyToDefault( const OUString& rPropertyName )
        throw(UnknownPropertyException, RuntimeException)
{
    if (!pDoc)
        throw RuntimeException();
    const SfxItemPropertyMap *pMap = SfxItemPropertyMap::GetByName( aPropSet.getPropertyMap(), rPropertyName);
    if (!pMap)
        throw UnknownPropertyException();
    if ( pMap->nFlags & PropertyAttribute::READONLY)
        throw IllegalArgumentException();
   SfxItemPool rSet (pDoc->GetAttrPool());
   rSet.ResetPoolDefaultItem ( pMap->nWID );
}
Any SAL_CALL SwXTextDefaults::getPropertyDefault( const OUString& rPropertyName )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    if (!pDoc)
        throw RuntimeException();
    const SfxItemPropertyMap *pMap = SfxItemPropertyMap::GetByName( aPropSet.getPropertyMap(), rPropertyName);
    if (!pMap)
        throw UnknownPropertyException();
    if ( pMap->nFlags & PropertyAttribute::READONLY)
        throw IllegalArgumentException();
    Any aRet;
    SfxItemPool rSet (pDoc->GetAttrPool());
    const SfxPoolItem *pItem = rSet.GetPoolDefaultItem ( pMap->nWID );
    pItem->QueryValue( aRet, pMap->nMemberId );
    return aRet;
}
