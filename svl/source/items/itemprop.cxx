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
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <memory>

/*
 * UNO III Implementation
 */
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;

SfxItemPropertyMap::SfxItemPropertyMap( const SfxItemPropertyMapEntry* pEntries )
{
    m_aMap.reserve(128);
    while( !pEntries->aName.isEmpty() )
    {
        m_aMap.insert( pEntries );
        ++pEntries;
    }
}

SfxItemPropertyMap::SfxItemPropertyMap( const SfxItemPropertyMap& rSource ) = default;

SfxItemPropertyMap::~SfxItemPropertyMap()
{
}

const SfxItemPropertyMapEntry* SfxItemPropertyMap::getByName( std::u16string_view rName ) const
{
    struct Compare
    {
        bool operator() ( const SfxItemPropertyMapEntry* lhs, std::u16string_view rhs ) const
        {
            return lhs->aName < rhs;
        }
        bool operator() ( std::u16string_view lhs, const SfxItemPropertyMapEntry* rhs ) const
        {
            return lhs < rhs->aName;
        }
    };
    auto it = std::lower_bound(m_aMap.begin(), m_aMap.end(), rName, Compare());
    if (it == m_aMap.end() || Compare()(rName, *it))
        return nullptr;
    return *it;
}

uno::Sequence<beans::Property> const & SfxItemPropertyMap::getProperties() const
{
    if( !m_aPropSeq.hasElements() )
    {
        m_aPropSeq.realloc( m_aMap.size() );
        beans::Property* pPropArray = m_aPropSeq.getArray();
        sal_uInt32 n = 0;
        for( const SfxItemPropertyMapEntry* pEntry : m_aMap )
        {
            pPropArray[n].Name = pEntry->aName;
            pPropArray[n].Handle = pEntry->nWID;
            pPropArray[n].Type = pEntry->aType;
            pPropArray[n].Attributes =
                sal::static_int_cast< sal_Int16 >(pEntry->nFlags);
            n++;
        }
    }

    return m_aPropSeq;
}

beans::Property SfxItemPropertyMap::getPropertyByName( const OUString& rName ) const
{
    const SfxItemPropertyMapEntry* pEntry = getByName(rName);
    if( !pEntry )
        throw UnknownPropertyException(rName);
    beans::Property aProp;
    aProp.Name = rName;
    aProp.Handle = pEntry->nWID;
    aProp.Type = pEntry->aType;
    aProp.Attributes = sal::static_int_cast< sal_Int16 >(pEntry->nFlags);
    return aProp;
}

bool SfxItemPropertyMap::hasPropertyByName( std::u16string_view rName ) const
{
    return getByName(rName) != nullptr;
}

SfxItemPropertySet::~SfxItemPropertySet()
{
}

void SfxItemPropertySet::getPropertyValue( const SfxItemPropertyMapEntry& rEntry,
            const SfxItemSet& rSet, Any& rAny ) const
{
    // get the SfxPoolItem
    const SfxPoolItem* pItem = nullptr;
    SfxItemState eState = rSet.GetItemState( rEntry.nWID, true, &pItem );
    if (SfxItemState::SET != eState && SfxItemPool::IsWhich(rEntry.nWID) )
        pItem = &rSet.GetPool()->GetDefaultItem(rEntry.nWID);
    // return item values as uno::Any
    if(eState >= SfxItemState::DEFAULT && pItem)
    {
        pItem->QueryValue( rAny, rEntry.nMemberId );
    }
    else if(0 == (rEntry.nFlags & PropertyAttribute::MAYBEVOID))
    {
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
{
    // detect which-id
    const SfxItemPropertyMapEntry* pEntry = m_aMap.getByName( rName );
    if ( !pEntry )
        throw UnknownPropertyException(rName);
    getPropertyValue( *pEntry,rSet, rAny );
}

Any SfxItemPropertySet::getPropertyValue( const OUString &rName,
                                          const SfxItemSet& rSet ) const
{
    Any aVal;
    getPropertyValue( rName,rSet, aVal );
    return aVal;
}

void SfxItemPropertySet::setPropertyValue( const SfxItemPropertyMapEntry& rEntry,
                                           const Any& aVal,
                                           SfxItemSet& rSet ) const
{
    // get the SfxPoolItem
    const SfxPoolItem* pItem = nullptr;
    std::unique_ptr<SfxPoolItem> pNewItem;
    SfxItemState eState = rSet.GetItemState( rEntry.nWID, true, &pItem );
    if (SfxItemState::SET != eState && SfxItemPool::IsWhich(rEntry.nWID))
        pItem = &rSet.GetPool()->GetDefaultItem(rEntry.nWID);
    if (pItem)
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
{
    const SfxItemPropertyMapEntry* pEntry = m_aMap.getByName( rName );
    if ( !pEntry )
    {
        throw UnknownPropertyException(rName);
    }
    setPropertyValue(*pEntry, aVal, rSet);
}

PropertyState SfxItemPropertySet::getPropertyState(const SfxItemPropertyMapEntry& rEntry, const SfxItemSet& rSet) const
    noexcept
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
{
    PropertyState eRet = PropertyState_DIRECT_VALUE;

    // Get WhichId
    const SfxItemPropertyMapEntry* pEntry = m_aMap.getByName( rName );
    if( !pEntry || !pEntry->nWID )
    {
        throw UnknownPropertyException(rName);
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

SfxItemPropertySetInfo::SfxItemPropertySetInfo(const SfxItemPropertyMap &rMap )
    : m_aOwnMap( rMap )
{
}

SfxItemPropertySetInfo::SfxItemPropertySetInfo(const SfxItemPropertyMapEntry *pEntries )
    : m_aOwnMap( pEntries )
{
}

Sequence< Property > SAL_CALL SfxItemPropertySetInfo::getProperties(  )
{
    return m_aOwnMap.getProperties();
}

SfxItemPropertySetInfo::~SfxItemPropertySetInfo()
{
}

Property SAL_CALL SfxItemPropertySetInfo::getPropertyByName( const OUString& rName )
{
    return m_aOwnMap.getPropertyByName( rName );
}

sal_Bool SAL_CALL SfxItemPropertySetInfo::hasPropertyByName( const OUString& rName )
{
    return m_aOwnMap.hasPropertyByName( rName );
}

SfxExtItemPropertySetInfo::SfxExtItemPropertySetInfo( const SfxItemPropertyMapEntry *pEntries,
                                                      const Sequence<Property>& rPropSeq )
{
    while( !pEntries->aName.isEmpty() )
    {
        maMap.emplace( pEntries->aName, *pEntries );
        ++pEntries;
    }
    for( const auto & rProp : rPropSeq )
    {
        SfxItemPropertySimpleEntry aTemp(
            sal::static_int_cast< sal_Int16 >( rProp.Handle ), //nWID
            rProp.Type, //aType
            rProp.Attributes); //nFlags
        maMap[rProp.Name] = aTemp;
    }
}

SfxExtItemPropertySetInfo::~SfxExtItemPropertySetInfo()
{
}

Sequence< Property > SAL_CALL SfxExtItemPropertySetInfo::getProperties(  )
{
    if( !m_aPropSeq.hasElements() )
    {
        m_aPropSeq.realloc( maMap.size() );
        beans::Property* pPropArray = m_aPropSeq.getArray();
        sal_uInt32 n = 0;
        for( const auto& rPair : maMap )
        {
            const SfxItemPropertySimpleEntry& rEntry = rPair.second;
            pPropArray[n].Name = rPair.first;
            pPropArray[n].Handle = rEntry.nWID;
            pPropArray[n].Type = rEntry.aType;
            pPropArray[n].Attributes =
                sal::static_int_cast< sal_Int16 >(rEntry.nFlags);
            n++;
        }
    }

    return m_aPropSeq;
}

Property SAL_CALL SfxExtItemPropertySetInfo::getPropertyByName( const OUString& rPropertyName )
{
    auto aIter = maMap.find(rPropertyName);
    if( aIter == maMap.end() )
        throw UnknownPropertyException(rPropertyName);
    const SfxItemPropertySimpleEntry& rEntry = aIter->second;
    beans::Property aProp;
    aProp.Name = rPropertyName;
    aProp.Handle = rEntry.nWID;
    aProp.Type = rEntry.aType;
    aProp.Attributes = sal::static_int_cast< sal_Int16 >(rEntry.nFlags);
    return aProp;
}

sal_Bool SAL_CALL SfxExtItemPropertySetInfo::hasPropertyByName( const OUString& rPropertyName )
{
    return maMap.find(rPropertyName) != maMap.end();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
