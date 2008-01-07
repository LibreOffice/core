/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: itemprop.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-07 08:56:23 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"
#include <string.h>
#ifndef GCC
#endif

#include <svtools/itemprop.hxx>
#include <svtools/itempool.hxx>
#include <svtools/itemset.hxx>
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
/* -----------------------------21.02.00 11:26--------------------------------

 ---------------------------------------------------------------------------*/
const SfxItemPropertyMap*   SfxItemPropertyMap::GetByName(
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
/* -----------------------------12.12.03 14:04--------------------------------

 ---------------------------------------------------------------------------*/
const SfxItemPropertyMap*   SfxItemPropertyMap::GetTolerantByName(
    const SfxItemPropertyMap *pMap, const OUString &rName )
{
    sal_Int32 nLength(rName.getLength());
    while ( pMap->pName )
    {
        if( nLength == pMap->nNameLen )
        {
            sal_Int32 nResult(rName.compareToAscii(pMap->pName));
            if (nResult == 0)
                return pMap;
            else if (nResult < 0)
                return 0;
        }
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
//          int nDiff = strcmp(pMap[nCount1].pName, OUStringToString(rPropSeq.getConstArray()[nCount2].Name, CHARSET_SYSTEM ));
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

