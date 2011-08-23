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

#include <string.h>
#ifndef GCC
#endif

#include <bf_svtools/itemprop.hxx>
#include <bf_svtools/itempool.hxx>
#include <bf_svtools/itemset.hxx>
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

/*************************************************************************
    UNO III Implementation
*************************************************************************/
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace ::rtl;

namespace binfilter
{

/* -----------------------------21.02.00 11:26--------------------------------

 ---------------------------------------------------------------------------*/
const SfxItemPropertyMap*	SfxItemPropertyMap::GetByName(
    const SfxItemPropertyMap *pMap, const OUString &rName )
{
    while ( pMap->pName )
    {
        if( rName.equalsAsciiL( pMap->pName, pMap->nNameLen ) )
            return pMap;
        ++pMap;
    }
    return 0;
}
/* -----------------------------21.02.00 11:26--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SfxItemPropertySet::FillItem(SfxItemSet&, USHORT, BOOL) const
{
    return FALSE;
}
/* -----------------------------06.06.01 12:32--------------------------------

 ---------------------------------------------------------------------------*/
void SfxItemPropertySet::getPropertyValue( const SfxItemPropertyMap& rMap,
            const SfxItemSet& rSet, Any& rAny ) const
                        throw(RuntimeException)
{
    // item holen
    const SfxPoolItem* pItem = 0;
    SfxItemState eState = rSet.GetItemState( rMap.nWID, TRUE, &pItem );
    if(SFX_ITEM_SET != eState && SFX_WHICH_MAX > rMap.nWID )
        pItem = &rSet.GetPool()->GetDefaultItem(rMap.nWID);
    // item-Wert als UnoAny zurueckgeben
    if(eState >= SFX_ITEM_DEFAULT && pItem)
    {
        pItem->QueryValue( rAny, rMap.nMemberId );
    }
    else
    {
        SfxItemSet aSet(*rSet.GetPool(), rMap.nWID, rMap.nWID);
        if(FillItem(aSet, rMap.nWID, TRUE))
        {
            const SfxPoolItem& rItem = aSet.Get(rMap.nWID);
            rItem.QueryValue( rAny, rMap.nMemberId );
        }
        else if(0 == (rMap.nFlags & PropertyAttribute::MAYBEVOID))
            throw RuntimeException();
    }


    // allgemeine SfxEnumItem Values in konkrete wandeln
    if( rMap.pType && TypeClass_ENUM == rMap.pType->getTypeClass() &&
         rAny.getValueTypeClass() == TypeClass_LONG )
    {
        INT32 nTmp = *(INT32*)rAny.getValue();
        rAny.setValue( &nTmp, *rMap.pType );
    }
}


/* -----------------------------15.11.00 12:32--------------------------------

 ---------------------------------------------------------------------------*/
Any SfxItemPropertySet::getPropertyValue( const SfxItemPropertyMap& rMap,
            const SfxItemSet& rSet ) const
                        throw(RuntimeException)
{
    Any aAny;
    getPropertyValue(rMap, rSet, aAny);
    return aAny;
}

/* -----------------------------06.06.01 12:32--------------------------------

 ---------------------------------------------------------------------------*/
void SfxItemPropertySet::getPropertyValue( const OUString &rName,
            const SfxItemSet& rSet, Any& rAny ) const
                        throw(RuntimeException, UnknownPropertyException)
{
    // which-id ermitteln
    const SfxItemPropertyMap* pMap = SfxItemPropertyMap::GetByName( _pMap, rName );
    if ( !pMap )
        throw UnknownPropertyException();
    getPropertyValue( *pMap,rSet, rAny );
}
/* -----------------------------21.02.00 11:26--------------------------------

 ---------------------------------------------------------------------------*/
Any SfxItemPropertySet::getPropertyValue( const OUString &rName,
            const SfxItemSet& rSet ) const
                        throw(RuntimeException, UnknownPropertyException)
{
    Any aVal;
    getPropertyValue( rName,rSet, aVal );
    return aVal;
}
/* -----------------------------15.11.00 14:46--------------------------------

 ---------------------------------------------------------------------------*/
void SfxItemPropertySet::setPropertyValue( const SfxItemPropertyMap& rMap,
                                            const Any& aVal,
                                            SfxItemSet& rSet ) const
                                            throw(RuntimeException,
                                                    IllegalArgumentException)
{
    // item holen
    const SfxPoolItem* pItem = 0;
    SfxPoolItem *pNewItem = 0;
    SfxItemState eState = rSet.GetItemState( rMap.nWID, TRUE, &pItem );
    if(SFX_ITEM_SET != eState && SFX_WHICH_MAX > rMap.nWID )
        pItem = &rSet.GetPool()->GetDefaultItem(rMap.nWID);
    //maybe there's another way to find an Item
    if(eState < SFX_ITEM_DEFAULT)
    {
        SfxItemSet aSet(*rSet.GetPool(), rMap.nWID, rMap.nWID);
        if(FillItem(aSet, rMap.nWID, FALSE))
        {
            const SfxPoolItem &rItem = aSet.Get(rMap.nWID);
            pNewItem = rItem.Clone();
        }
    }
    if(!pNewItem && pItem)
    {
        pNewItem = pItem->Clone();
    }
    if(pNewItem)
    {
        if( !pNewItem->PutValue( aVal, rMap.nMemberId ) )
        {
            DELETEZ(pNewItem);
            throw IllegalArgumentException();
        }
        // neues item in itemset setzen
        rSet.Put( *pNewItem, rMap.nWID );
        delete pNewItem;
    }
}
/* -----------------------------21.02.00 11:26--------------------------------

 ---------------------------------------------------------------------------*/
void SfxItemPropertySet::setPropertyValue( const OUString &rName,
                                            const Any& aVal,
                                            SfxItemSet& rSet ) const
                                            throw(RuntimeException,
                                                    IllegalArgumentException,
                                                    UnknownPropertyException)
{
    const SfxItemPropertyMap* pMap = SfxItemPropertyMap::GetByName( _pMap, rName );
    if ( !pMap )
    {
        throw UnknownPropertyException();
    }
    setPropertyValue(*pMap, aVal, rSet);
}
/* -----------------------------21.02.00 11:26--------------------------------

 ---------------------------------------------------------------------------*/
PropertyState SfxItemPropertySet::getPropertyState(const SfxItemPropertyMap& rMap, const SfxItemSet& rSet)
                                    throw()
{
    PropertyState eRet = PropertyState_DIRECT_VALUE;
    USHORT nWhich = rMap.nWID;

    // item state holen
    SfxItemState eState = rSet.GetItemState( nWhich, FALSE );
    // item-Wert als UnoAny zurueckgeben
    if(eState == SFX_ITEM_DEFAULT)
        eRet = PropertyState_DEFAULT_VALUE;
    else if(eState < SFX_ITEM_DEFAULT)
        eRet = PropertyState_AMBIGUOUS_VALUE;
    return eRet;
}

PropertyState   SfxItemPropertySet::getPropertyState(
                                const OUString& rName, const SfxItemSet& rSet)
                                    throw(UnknownPropertyException)
{
    PropertyState eRet = PropertyState_DIRECT_VALUE;

    // which-id ermitteln
    const SfxItemPropertyMap* pMap = SfxItemPropertyMap::GetByName( _pMap, rName );
    USHORT nWhich = pMap ? pMap->nWID : 0;
    if ( !nWhich )
    {
        throw UnknownPropertyException();
    }

    // item holen
    const SfxPoolItem* pItem = 0;
    SfxItemState eState = rSet.GetItemState( nWhich, FALSE, &pItem );
    if(!pItem && nWhich != rSet.GetPool()->GetSlotId(nWhich))
        pItem = &rSet.GetPool()->GetDefaultItem(nWhich);
    // item-Wert als UnoAny zurueckgeben
    if(eState == SFX_ITEM_DEFAULT)
        eRet = PropertyState_DEFAULT_VALUE;
    else if(eState < SFX_ITEM_DEFAULT)
        eRet = PropertyState_AMBIGUOUS_VALUE;
    return eRet;
}
/* -----------------------------21.02.00 11:26--------------------------------

 ---------------------------------------------------------------------------*/
Reference<XPropertySetInfo>
    SfxItemPropertySet::getPropertySetInfo() const
{
    Reference<XPropertySetInfo> aRef(new SfxItemPropertySetInfo( _pMap ));
    return aRef;
}

/* -----------------------------21.02.00 11:09--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< Property > SAL_CALL
        SfxItemPropertySetInfo::getProperties(  )
            throw(RuntimeException)
{
    USHORT n = 0;
    {
        for ( const SfxItemPropertyMap *pMap = _pMap; pMap->pName; ++pMap )
            ++n;
    }

    Sequence<Property> aPropSeq( n );
    n = 0;
    for ( const SfxItemPropertyMap *pMap = _pMap; pMap->pName; ++pMap )
    {
        aPropSeq.getArray()[n].Name = OUString::createFromAscii( pMap->pName );
        aPropSeq.getArray()[n].Handle = pMap->nWID;
        if(pMap->pType)
            aPropSeq.getArray()[n].Type = *pMap->pType;
        aPropSeq.getArray()[n].Attributes =
            sal::static_int_cast< sal_Int16 >(pMap->nFlags);
        n++;
    }

    return aPropSeq;
}
/* -----------------------------21.02.00 11:27--------------------------------

 ---------------------------------------------------------------------------*/
Property SAL_CALL
        SfxItemPropertySetInfo::getPropertyByName( const ::rtl::OUString& rName )
            throw(UnknownPropertyException, RuntimeException)
{
    Property aProp;
    for( const SfxItemPropertyMap *pMap = _pMap; pMap->pName; ++pMap )
    {
        if( rName.equalsAsciiL( pMap->pName, pMap->nNameLen ))
        {
            aProp.Name = rName;
            aProp.Handle = pMap->nWID;
            if(pMap->pType)
                aProp.Type = *pMap->pType;
            aProp.Attributes = sal::static_int_cast< sal_Int16 >(pMap->nFlags);
            break;
        }
    }
    if(!aProp.Name.getLength())
        throw UnknownPropertyException();
    return aProp;
}
/* -----------------------------21.02.00 11:28--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SAL_CALL
        SfxItemPropertySetInfo::hasPropertyByName( const ::rtl::OUString& rName )
            throw(RuntimeException)
{
    for ( const SfxItemPropertyMap *pMap = _pMap; pMap->pName; ++pMap )
    {
        if( rName.equalsAsciiL( pMap->pName, pMap->nNameLen ))
            return TRUE;
    }
    return FALSE;
}
/* -----------------------------21.02.00 12:03--------------------------------

 ---------------------------------------------------------------------------*/
SfxExtItemPropertySetInfo::SfxExtItemPropertySetInfo(
                                const SfxItemPropertyMap *pMap,
                                const Sequence<Property>& rPropSeq ) :
                _pExtMap(pMap)
{
    long nLen = rPropSeq.getLength();
    long nExtLen = 0;
    while(pMap[nExtLen].pName)
        nExtLen++;
    aPropertySeq.realloc(nExtLen + nLen);


    long nNewLen = aPropertySeq.getLength();
    Property* pNewArr = aPropertySeq.getArray();
    long nCount1 = 0;//map
    long nCount2 = 0;//sequence
    long nDouble = 0;//Anzahl gleicher Property-Namen
    BOOL bFromMap, bZero = FALSE;

    const Property* pPropSeqArr = rPropSeq.getConstArray();
    for( long i = 0; i < nNewLen; i++)
    {
        bFromMap = FALSE;
        if(nCount1 < nExtLen && nCount2 < nLen)
        {
//			int nDiff = strcmp(pMap[nCount1].pName, OUStringToString(rPropSeq.getConstArray()[nCount2].Name, CHARSET_SYSTEM ));
            sal_Int32 nDiff = pPropSeqArr[nCount2].Name.compareToAscii(pMap[nCount1].pName, pMap[nCount1].nNameLen );
            if(nDiff > 0)
            {
                bFromMap = TRUE;
            }
            else if(0 == nDiff)
            {
                nDouble++;
                bFromMap = TRUE;
                nCount2++;
            }
        }
        else
        {
            if(nCount1 < nExtLen)
                bFromMap = TRUE;
            else if(nCount2>= nLen)
                bZero = TRUE;
        }
        if(bFromMap)
        {
            pNewArr[i].Name = OUString::createFromAscii( pMap[nCount1].pName );
            pNewArr[i].Handle = pMap[nCount1].nWID;
            if(pMap[nCount1].pType)
                pNewArr[i].Type = *pMap[nCount1].pType;
            pNewArr[i].Attributes = sal::static_int_cast< sal_Int16 >(
                pMap[nCount1].nFlags);
            nCount1++;
        }
        else if(!bZero)
        {
            pNewArr[i] = pPropSeqArr[nCount2];
            nCount2++;
        }
    }
    if(nDouble)
        aPropertySeq.realloc(nExtLen + nLen - nDouble);
}
/* -----------------------------21.02.00 12:03--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< Property > SAL_CALL
        SfxExtItemPropertySetInfo::getProperties(  ) throw(RuntimeException)
{
    return aPropertySeq;
}
/* -----------------------------21.02.00 12:03--------------------------------

 ---------------------------------------------------------------------------*/
Property SAL_CALL
        SfxExtItemPropertySetInfo::getPropertyByName( const OUString& aPropertyName )
            throw(UnknownPropertyException, RuntimeException)
{
    const Property* pPropArr = aPropertySeq.getConstArray();
    long nLen = aPropertySeq.getLength();
    for( long i = 0; i < nLen; i++)
    {
        if(aPropertyName == pPropArr[i].Name)
            return pPropArr[i];
    }
    return Property();
}
/* -----------------------------21.02.00 12:03--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SAL_CALL
        SfxExtItemPropertySetInfo::hasPropertyByName( const OUString& aPropertyName )
            throw(RuntimeException)
{
    const Property* pPropArr = aPropertySeq.getConstArray();
    long nLen = aPropertySeq.getLength();
    for( long i = 0; i < nLen; i++)
    {
        if(aPropertyName == pPropArr[i].Name)
            return TRUE;
    }
    return FALSE;
}

// --------------------------------------------------------------------
// SfxItemPropertySetInfo
// --------------------------------------------------------------------

::com::sun::star::uno::Any SAL_CALL SfxItemPropertySetInfo::queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException)
{
    return cppu::WeakImplHelper1<com::sun::star::beans::XPropertySetInfo>::queryInterface( aType );
}

// --------------------------------------------------------------------

void SAL_CALL SfxItemPropertySetInfo::acquire(  ) throw ()
{
    cppu::WeakImplHelper1<com::sun::star::beans::XPropertySetInfo>::release();
}

// --------------------------------------------------------------------

void SAL_CALL SfxItemPropertySetInfo::release(  ) throw ()
{
    cppu::WeakImplHelper1<com::sun::star::beans::XPropertySetInfo>::release();
}

// --------------------------------------------------------------------
// SfxExtItemPropertySetInfo
// --------------------------------------------------------------------

::com::sun::star::uno::Any SAL_CALL SfxExtItemPropertySetInfo::queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException)
{
    return cppu::WeakImplHelper1<com::sun::star::beans::XPropertySetInfo>::queryInterface( aType );
}

// --------------------------------------------------------------------

void SAL_CALL SfxExtItemPropertySetInfo::acquire(  ) throw ()
{
    cppu::WeakImplHelper1<com::sun::star::beans::XPropertySetInfo>::release();
}

// --------------------------------------------------------------------

void SAL_CALL SfxExtItemPropertySetInfo::release(  ) throw ()
{
    cppu::WeakImplHelper1<com::sun::star::beans::XPropertySetInfo>::release();
}

// --------------------------------------------------------------------
}
