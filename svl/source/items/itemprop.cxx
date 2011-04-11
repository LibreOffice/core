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
#include "precompiled_svl.hxx"

#include <svl/itemprop.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <boost/unordered_map.hpp>
/*************************************************************************
    UNO III Implementation
*************************************************************************/
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;

struct equalOUString
{
  bool operator()(const ::rtl::OUString& r1, const ::rtl::OUString&  r2) const
  {
    return r1.equals( r2 );
  }
};

typedef ::boost::unordered_map< ::rtl::OUString,
                                 SfxItemPropertySimpleEntry,
                                 ::rtl::OUStringHash,
                                 equalOUString > SfxItemPropertyHashMap_t;

class SfxItemPropertyMap_Impl : public SfxItemPropertyHashMap_t
{
public:
    mutable uno::Sequence< beans::Property > m_aPropSeq;

    SfxItemPropertyMap_Impl(){}
    SfxItemPropertyMap_Impl( const SfxItemPropertyMap_Impl* pSource );
};
SfxItemPropertyMap_Impl::SfxItemPropertyMap_Impl( const SfxItemPropertyMap_Impl* pSource )
{
    this->SfxItemPropertyHashMap_t::operator=( *pSource );
    m_aPropSeq = pSource->m_aPropSeq;
}

SfxItemPropertyMap::SfxItemPropertyMap( const SfxItemPropertyMapEntry* pEntries ) :
    m_pImpl( new SfxItemPropertyMap_Impl )
{
    while( pEntries->pName )
    {
        ::rtl::OUString sEntry(pEntries->pName, pEntries->nNameLen, RTL_TEXTENCODING_ASCII_US );
        (*m_pImpl) [ sEntry ] = pEntries;
        ++pEntries;
    }
}

SfxItemPropertyMap::SfxItemPropertyMap( const SfxItemPropertyMap* pSource ) :
    m_pImpl( new SfxItemPropertyMap_Impl( pSource->m_pImpl ) )
{
}

SfxItemPropertyMap::~SfxItemPropertyMap()
{
    delete m_pImpl;
}

const SfxItemPropertySimpleEntry* SfxItemPropertyMap::getByName( const ::rtl::OUString &rName ) const
{
    SfxItemPropertyHashMap_t::const_iterator aIter = m_pImpl->find(rName);
    if( aIter == m_pImpl->end() )
        return 0;
    return &aIter->second;
}

uno::Sequence<beans::Property> SfxItemPropertyMap::getProperties() const
{
    if( !m_pImpl->m_aPropSeq.getLength() )
    {
        m_pImpl->m_aPropSeq.realloc( m_pImpl->size() );
        beans::Property* pPropArray = m_pImpl->m_aPropSeq.getArray();
        sal_uInt32 n = 0;
        SfxItemPropertyHashMap_t::const_iterator aIt = m_pImpl->begin();
        while( aIt != m_pImpl->end() )
        //for ( const SfxItemPropertyMap *pMap = _pMap; pMap->pName; ++pMap )
        {
            const SfxItemPropertySimpleEntry* pEntry = &(*aIt).second;
            pPropArray[n].Name = (*aIt).first;
            pPropArray[n].Handle = pEntry->nWID;
            if(pEntry->pType)
                pPropArray[n].Type = *pEntry->pType;
            pPropArray[n].Attributes =
                sal::static_int_cast< sal_Int16 >(pEntry->nFlags);
            n++;
            ++aIt;
        }
    }

    return m_pImpl->m_aPropSeq;
}

beans::Property SfxItemPropertyMap::getPropertyByName( const ::rtl::OUString rName ) const
    throw( beans::UnknownPropertyException )
{
    SfxItemPropertyHashMap_t::const_iterator aIter = m_pImpl->find(rName);
    if( aIter == m_pImpl->end() )
        throw UnknownPropertyException();
    const SfxItemPropertySimpleEntry* pEntry = &aIter->second;
    beans::Property aProp;
    aProp.Name = rName;
    aProp.Handle = pEntry->nWID;
    if(pEntry->pType)
        aProp.Type = *pEntry->pType;
    aProp.Attributes = sal::static_int_cast< sal_Int16 >(pEntry->nFlags);
    return aProp;
}

sal_Bool SfxItemPropertyMap::hasPropertyByName( const ::rtl::OUString& rName ) const
{
    SfxItemPropertyHashMap_t::const_iterator aIter = m_pImpl->find(rName);
    return aIter != m_pImpl->end();
}

void SfxItemPropertyMap::mergeProperties( const uno::Sequence< beans::Property >& rPropSeq )
{
    const beans::Property* pPropArray = rPropSeq.getConstArray();
    sal_uInt32 nElements = rPropSeq.getLength();
    for( sal_uInt32 nElement = 0; nElement < nElements; ++nElement )
    {
        SfxItemPropertySimpleEntry aTemp(
            sal::static_int_cast< sal_Int16 >( pPropArray[nElement].Handle ), //nWID
            &pPropArray[nElement].Type, //pType
            pPropArray[nElement].Attributes, //nFlags
            0 ); //nMemberId
        (*m_pImpl)[pPropArray[nElement].Name] = aTemp;
    }
}

PropertyEntryVector_t SfxItemPropertyMap::getPropertyEntries() const
{
    PropertyEntryVector_t aRet;
    aRet.reserve(m_pImpl->size());

    SfxItemPropertyHashMap_t::const_iterator aIt = m_pImpl->begin();
    while( aIt != m_pImpl->end() )
    {
        const SfxItemPropertySimpleEntry* pEntry = &(*aIt).second;
        aRet.push_back( SfxItemPropertyNamedEntry( (*aIt).first, * pEntry ) );
        ++aIt;
    }
    return aRet;
}

sal_uInt32 SfxItemPropertyMap::getSize() const
{
    return m_pImpl->size();
}

SfxItemPropertySet::~SfxItemPropertySet()
{
}

sal_Bool SfxItemPropertySet::FillItem(SfxItemSet&, sal_uInt16, sal_Bool) const
{
    return sal_False;
}

void SfxItemPropertySet::getPropertyValue( const SfxItemPropertySimpleEntry& rEntry,
            const SfxItemSet& rSet, Any& rAny ) const
                        throw(RuntimeException)
{
    // get the SfxPoolItem
    const SfxPoolItem* pItem = 0;
    SfxItemState eState = rSet.GetItemState( rEntry.nWID, sal_True, &pItem );
    if(SFX_ITEM_SET != eState && SFX_WHICH_MAX > rEntry.nWID )
        pItem = &rSet.GetPool()->GetDefaultItem(rEntry.nWID);
    // return item values as uno::Any
    if(eState >= SFX_ITEM_DEFAULT && pItem)
    {
        pItem->QueryValue( rAny, rEntry.nMemberId );
    }
    else
    {
        SfxItemSet aSet(*rSet.GetPool(), rEntry.nWID, rEntry.nWID);
        if(FillItem(aSet, rEntry.nWID, sal_True))
        {
            const SfxPoolItem& rItem = aSet.Get(rEntry.nWID);
            rItem.QueryValue( rAny, rEntry.nMemberId );
        }
        else if(0 == (rEntry.nFlags & PropertyAttribute::MAYBEVOID))
            throw RuntimeException();
    }


    // convert general SfxEnumItem values to specific values
    if( rEntry.pType && TypeClass_ENUM == rEntry.pType->getTypeClass() &&
         rAny.getValueTypeClass() == TypeClass_LONG )
    {
        sal_Int32 nTmp = *(sal_Int32*)rAny.getValue();
        rAny.setValue( &nTmp, *rEntry.pType );
    }
}

void SfxItemPropertySet::getPropertyValue( const rtl::OUString &rName,
            const SfxItemSet& rSet, Any& rAny ) const
                        throw(RuntimeException, UnknownPropertyException)
{
    // detect which-id
    const SfxItemPropertySimpleEntry* pEntry = m_aMap.getByName( rName );
    if ( !pEntry )
        throw UnknownPropertyException();
    getPropertyValue( *pEntry,rSet, rAny );
}

Any SfxItemPropertySet::getPropertyValue( const rtl::OUString &rName,
            const SfxItemSet& rSet ) const
                        throw(RuntimeException, UnknownPropertyException)
{
    Any aVal;
    getPropertyValue( rName,rSet, aVal );
    return aVal;
}

void SfxItemPropertySet::setPropertyValue( const SfxItemPropertySimpleEntry& rEntry,
                                            const Any& aVal,
                                            SfxItemSet& rSet ) const
                                            throw(RuntimeException,
                                                    IllegalArgumentException)
{
    // get the SfxPoolItem
    const SfxPoolItem* pItem = 0;
    SfxPoolItem *pNewItem = 0;
    SfxItemState eState = rSet.GetItemState( rEntry.nWID, sal_True, &pItem );
    if(SFX_ITEM_SET != eState && SFX_WHICH_MAX > rEntry.nWID )
        pItem = &rSet.GetPool()->GetDefaultItem(rEntry.nWID);
    //maybe there's another way to find an Item
    if(eState < SFX_ITEM_DEFAULT)
    {
        SfxItemSet aSet(*rSet.GetPool(), rEntry.nWID, rEntry.nWID);
        if(FillItem(aSet, rEntry.nWID, sal_False))
        {
            const SfxPoolItem &rItem = aSet.Get(rEntry.nWID);
            pNewItem = rItem.Clone();
        }
    }
    if(!pNewItem && pItem)
    {
        pNewItem = pItem->Clone();
    }
    if(pNewItem)
    {
        if( !pNewItem->PutValue( aVal, rEntry.nMemberId ) )
        {
            DELETEZ(pNewItem);
            throw IllegalArgumentException();
        }
        // apply new item
        rSet.Put( *pNewItem, rEntry.nWID );
        delete pNewItem;
    }
}

void SfxItemPropertySet::setPropertyValue( const rtl::OUString &rName,
                                            const Any& aVal,
                                            SfxItemSet& rSet ) const
                                            throw(RuntimeException,
                                                    IllegalArgumentException,
                                                    UnknownPropertyException)
{
    const SfxItemPropertySimpleEntry* pEntry = m_aMap.getByName( rName );
    if ( !pEntry )
    {
        throw UnknownPropertyException();
    }
    setPropertyValue(*pEntry, aVal, rSet);
}

PropertyState SfxItemPropertySet::getPropertyState(const SfxItemPropertySimpleEntry& rEntry, const SfxItemSet& rSet) const
                                    throw()
{
    PropertyState eRet = PropertyState_DIRECT_VALUE;
    sal_uInt16 nWhich = rEntry.nWID;

    // item state holen
    SfxItemState eState = rSet.GetItemState( nWhich, sal_False );
    // item-Wert als UnoAny zurueckgeben
    if(eState == SFX_ITEM_DEFAULT)
        eRet = PropertyState_DEFAULT_VALUE;
    else if(eState < SFX_ITEM_DEFAULT)
        eRet = PropertyState_AMBIGUOUS_VALUE;
    return eRet;
}
PropertyState   SfxItemPropertySet::getPropertyState(
    const rtl::OUString& rName, const SfxItemSet& rSet) const
                                    throw(UnknownPropertyException)
{
    PropertyState eRet = PropertyState_DIRECT_VALUE;

    // which-id ermitteln
    const SfxItemPropertySimpleEntry* pEntry = m_aMap.getByName( rName );
    if( !pEntry || !pEntry->nWID )
    {
        throw UnknownPropertyException();
    }
    sal_uInt16 nWhich = pEntry->nWID;

    // item holen
    const SfxPoolItem* pItem = 0;
    SfxItemState eState = rSet.GetItemState( nWhich, sal_False, &pItem );
    if(!pItem && nWhich != rSet.GetPool()->GetSlotId(nWhich))
        pItem = &rSet.GetPool()->GetDefaultItem(nWhich);
    // item-Wert als UnoAny zurueckgeben
    if(eState == SFX_ITEM_DEFAULT)
        eRet = PropertyState_DEFAULT_VALUE;
    else if(eState < SFX_ITEM_DEFAULT)
        eRet = PropertyState_AMBIGUOUS_VALUE;
    return eRet;
}

Reference<XPropertySetInfo>
    SfxItemPropertySet::getPropertySetInfo() const
{
    if( !m_xInfo.is() )
        m_xInfo = new SfxItemPropertySetInfo( &m_aMap );
    return m_xInfo;
}

struct SfxItemPropertySetInfo_Impl
{
    SfxItemPropertyMap*         m_pOwnMap;
};

SfxItemPropertySetInfo::SfxItemPropertySetInfo(const SfxItemPropertyMap *pMap ) :
    m_pImpl( new SfxItemPropertySetInfo_Impl )
{
    m_pImpl->m_pOwnMap = new SfxItemPropertyMap( pMap );
}

SfxItemPropertySetInfo::SfxItemPropertySetInfo(const SfxItemPropertyMapEntry *pEntries ) :
    m_pImpl( new SfxItemPropertySetInfo_Impl )
{
    m_pImpl->m_pOwnMap = new SfxItemPropertyMap( pEntries );
}

Sequence< Property > SAL_CALL
        SfxItemPropertySetInfo::getProperties(  )
            throw(RuntimeException)
{
    return m_pImpl->m_pOwnMap->getProperties();
}

const SfxItemPropertyMap* SfxItemPropertySetInfo::getMap() const
{
    return m_pImpl->m_pOwnMap;
}


SfxItemPropertySetInfo::~SfxItemPropertySetInfo()
{
    delete m_pImpl->m_pOwnMap;
    delete m_pImpl;
}

Property SAL_CALL
        SfxItemPropertySetInfo::getPropertyByName( const ::rtl::OUString& rName )
            throw(UnknownPropertyException, RuntimeException)
{
    return m_pImpl->m_pOwnMap->getPropertyByName( rName );
}

sal_Bool SAL_CALL
        SfxItemPropertySetInfo::hasPropertyByName( const ::rtl::OUString& rName )
            throw(RuntimeException)
{
    return m_pImpl->m_pOwnMap->hasPropertyByName( rName );
}

SfxExtItemPropertySetInfo::SfxExtItemPropertySetInfo(
                                const SfxItemPropertyMapEntry *pMap,
                                const Sequence<Property>& rPropSeq ) :
                aExtMap( pMap )
{
    aExtMap.mergeProperties( rPropSeq );
}

SfxExtItemPropertySetInfo::~SfxExtItemPropertySetInfo()
{
}

Sequence< Property > SAL_CALL
        SfxExtItemPropertySetInfo::getProperties(  ) throw(RuntimeException)
{
    return aExtMap.getProperties();
}

Property SAL_CALL
SfxExtItemPropertySetInfo::getPropertyByName( const rtl::OUString& rPropertyName )
            throw(UnknownPropertyException, RuntimeException)
{
    return aExtMap.getPropertyByName( rPropertyName );
}

sal_Bool SAL_CALL
SfxExtItemPropertySetInfo::hasPropertyByName( const rtl::OUString& rPropertyName )
            throw(RuntimeException)
{
    return aExtMap.hasPropertyByName( rPropertyName );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
