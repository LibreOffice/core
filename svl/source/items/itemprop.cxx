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

#include <sal/config.h>

#include <o3tl/any.hxx>
#include <svl/itemprop.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <memory>
#include <unordered_map>
/*
 * UNO III Implementation
 */
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;

struct equalOUString
{
  bool operator()(const OUString& r1, const OUString&  r2) const
  {
    return r1.equals( r2 );
  }
};

typedef std::unordered_map< OUString,
                            SfxItemPropertySimpleEntry,
                            OUStringHash,
                            equalOUString > SfxItemPropertyHashMap_t;

class SfxItemPropertyMap_Impl : public SfxItemPropertyHashMap_t
{
public:
    mutable uno::Sequence< beans::Property > m_aPropSeq;

    SfxItemPropertyMap_Impl(){}
    explicit SfxItemPropertyMap_Impl( const SfxItemPropertyMap_Impl* pSource );
};

SfxItemPropertyMap_Impl::SfxItemPropertyMap_Impl( const SfxItemPropertyMap_Impl* pSource )
{
    this->SfxItemPropertyHashMap_t::operator=( *pSource );
    m_aPropSeq = pSource->m_aPropSeq;
}

SfxItemPropertyMap::SfxItemPropertyMap( const SfxItemPropertyMapEntry* pEntries ) :
    m_pImpl( new SfxItemPropertyMap_Impl )
{
    while( !pEntries->aName.isEmpty() )
    {
        (*m_pImpl) [ pEntries->aName ] = pEntries;
        ++pEntries;
    }
}

SfxItemPropertyMap::SfxItemPropertyMap( const SfxItemPropertyMap& rSource ) :
    m_pImpl( new SfxItemPropertyMap_Impl( rSource.m_pImpl.get() ) )
{
}

SfxItemPropertyMap::~SfxItemPropertyMap()
{
}

const SfxItemPropertySimpleEntry* SfxItemPropertyMap::getByName( const OUString &rName ) const
{
    SfxItemPropertyHashMap_t::const_iterator aIter = m_pImpl->find(rName);
    if( aIter == m_pImpl->end() )
        return nullptr;
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
            pPropArray[n].Type = pEntry->aType;
            pPropArray[n].Attributes =
                sal::static_int_cast< sal_Int16 >(pEntry->nFlags);
            n++;
            ++aIt;
        }
    }

    return m_pImpl->m_aPropSeq;
}

beans::Property SfxItemPropertyMap::getPropertyByName( const OUString & rName ) const
    throw( beans::UnknownPropertyException )
{
    SfxItemPropertyHashMap_t::const_iterator aIter = m_pImpl->find(rName);
    if( aIter == m_pImpl->end() )
        throw UnknownPropertyException();
    const SfxItemPropertySimpleEntry* pEntry = &aIter->second;
    beans::Property aProp;
    aProp.Name = rName;
    aProp.Handle = pEntry->nWID;
    aProp.Type = pEntry->aType;
    aProp.Attributes = sal::static_int_cast< sal_Int16 >(pEntry->nFlags);
    return aProp;
}

bool SfxItemPropertyMap::hasPropertyByName( const OUString& rName ) const
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
            pPropArray[nElement].Type, //aType
            pPropArray[nElement].Attributes); //nFlags
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

void SfxItemPropertySet::getPropertyValue( const SfxItemPropertySimpleEntry& rEntry,
            const SfxItemSet& rSet, Any& rAny ) const
                        throw(RuntimeException)
{
    // get the SfxPoolItem
    const SfxPoolItem* pItem = nullptr;
    SfxItemState eState = rSet.GetItemState( rEntry.nWID, true, &pItem );
    if(SfxItemState::SET != eState && SFX_WHICH_MAX > rEntry.nWID )
        pItem = &rSet.GetPool()->GetDefaultItem(rEntry.nWID);
    // return item values as uno::Any
    if(eState >= SfxItemState::DEFAULT && pItem)
    {
        pItem->QueryValue( rAny, rEntry.nMemberId );
    }
    else
    {
        SfxItemSet aSet(*rSet.GetPool(), rEntry.nWID, rEntry.nWID);
        if(0 == (rEntry.nFlags & PropertyAttribute::MAYBEVOID))
            throw RuntimeException(
                    "Property not found in ItemSet but not MAYBEVOID?", nullptr);
    }


    // convert general SfxEnumItem values to specific values
    if( rEntry.aType.getTypeClass() == TypeClass_ENUM &&
         rAny.getValueTypeClass() == TypeClass_LONG )
    {
        sal_Int32 nTmp = *o3tl::forceAccess<sal_Int32>(rAny);
        rAny.setValue( &nTmp, rEntry.aType );
    }
}

void SfxItemPropertySet::getPropertyValue( const OUString &rName,
                                           const SfxItemSet& rSet, Any& rAny ) const
    throw(RuntimeException, UnknownPropertyException)
{
    // detect which-id
    const SfxItemPropertySimpleEntry* pEntry = m_aMap.getByName( rName );
    if ( !pEntry )
        throw UnknownPropertyException();
    getPropertyValue( *pEntry,rSet, rAny );
}

Any SfxItemPropertySet::getPropertyValue( const OUString &rName,
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
    const SfxPoolItem* pItem = nullptr;
    std::unique_ptr<SfxPoolItem> pNewItem;
    SfxItemState eState = rSet.GetItemState( rEntry.nWID, true, &pItem );
    if(SfxItemState::SET != eState && SFX_WHICH_MAX > rEntry.nWID )
        pItem = &rSet.GetPool()->GetDefaultItem(rEntry.nWID);
    //maybe there's another way to find an Item
    if(eState < SfxItemState::DEFAULT)
    {
        SfxItemSet aSet(*rSet.GetPool(), rEntry.nWID, rEntry.nWID);
    }
    if(!pNewItem && pItem)
    {
        pNewItem.reset(pItem->Clone());
    }
    if(pNewItem)
    {
        if( !pNewItem->PutValue( aVal, rEntry.nMemberId ) )
        {
            throw IllegalArgumentException();
        }
        // apply new item
        rSet.Put( *pNewItem );
    }
}

void SfxItemPropertySet::setPropertyValue( const OUString &rName,
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

    // Get item state
    SfxItemState eState = rSet.GetItemState( nWhich, false );
    // Return item value as UnoAny
    if(eState == SfxItemState::DEFAULT)
        eRet = PropertyState_DEFAULT_VALUE;
    else if(eState < SfxItemState::DEFAULT)
        eRet = PropertyState_AMBIGUOUS_VALUE;
    return eRet;
}

PropertyState   SfxItemPropertySet::getPropertyState(const OUString& rName, const SfxItemSet& rSet) const
    throw(UnknownPropertyException)
{
    PropertyState eRet = PropertyState_DIRECT_VALUE;

    // Get WhichId
    const SfxItemPropertySimpleEntry* pEntry = m_aMap.getByName( rName );
    if( !pEntry || !pEntry->nWID )
    {
        throw UnknownPropertyException();
    }
    sal_uInt16 nWhich = pEntry->nWID;

    // Get item state
    SfxItemState eState = rSet.GetItemState(nWhich, false);
    // Return item value as UnoAny
    if(eState == SfxItemState::DEFAULT)
        eRet = PropertyState_DEFAULT_VALUE;
    else if(eState < SfxItemState::DEFAULT)
        eRet = PropertyState_AMBIGUOUS_VALUE;
    return eRet;
}

Reference<XPropertySetInfo> const & SfxItemPropertySet::getPropertySetInfo() const
{
    if( !m_xInfo.is() )
        m_xInfo = new SfxItemPropertySetInfo( m_aMap );
    return m_xInfo;
}

struct SfxItemPropertySetInfo_Impl
{
    SfxItemPropertyMap*         m_pOwnMap;
};

SfxItemPropertySetInfo::SfxItemPropertySetInfo(const SfxItemPropertyMap &rMap )
    :  m_pImpl( new SfxItemPropertySetInfo_Impl )
{
    m_pImpl->m_pOwnMap = new SfxItemPropertyMap( rMap );
}

SfxItemPropertySetInfo::SfxItemPropertySetInfo(const SfxItemPropertyMapEntry *pEntries )
    : m_pImpl( new SfxItemPropertySetInfo_Impl )
{
    m_pImpl->m_pOwnMap = new SfxItemPropertyMap( pEntries );
}

Sequence< Property > SAL_CALL SfxItemPropertySetInfo::getProperties(  )
    throw(RuntimeException, std::exception)
{
    return m_pImpl->m_pOwnMap->getProperties();
}

SfxItemPropertySetInfo::~SfxItemPropertySetInfo()
{
    delete m_pImpl->m_pOwnMap;
}

Property SAL_CALL SfxItemPropertySetInfo::getPropertyByName( const OUString& rName )
    throw(UnknownPropertyException, RuntimeException, std::exception)
{
    return m_pImpl->m_pOwnMap->getPropertyByName( rName );
}

sal_Bool SAL_CALL SfxItemPropertySetInfo::hasPropertyByName( const OUString& rName )
    throw(RuntimeException, std::exception)
{
    return m_pImpl->m_pOwnMap->hasPropertyByName( rName );
}

SfxExtItemPropertySetInfo::SfxExtItemPropertySetInfo( const SfxItemPropertyMapEntry *pMap,
                                                      const Sequence<Property>& rPropSeq )
    : aExtMap( pMap )
{
    aExtMap.mergeProperties( rPropSeq );
}

SfxExtItemPropertySetInfo::~SfxExtItemPropertySetInfo()
{
}

Sequence< Property > SAL_CALL SfxExtItemPropertySetInfo::getProperties(  ) throw(RuntimeException, std::exception)
{
    return aExtMap.getProperties();
}

Property SAL_CALL SfxExtItemPropertySetInfo::getPropertyByName( const OUString& rPropertyName )
    throw(UnknownPropertyException, RuntimeException, std::exception)
{
    return aExtMap.getPropertyByName( rPropertyName );
}

sal_Bool SAL_CALL SfxExtItemPropertySetInfo::hasPropertyByName( const OUString& rPropertyName )
    throw(RuntimeException, std::exception)
{
    return aExtMap.hasPropertyByName( rPropertyName );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
