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
#include <comphelper/anytohash.hxx>
#include <svx/sdasitm.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>

using namespace com::sun::star;


SdrCustomShapeGeometryItem::SdrCustomShapeGeometryItem()
:   SfxPoolItem( SDRATTR_CUSTOMSHAPE_GEOMETRY )
{}

SdrCustomShapeGeometryItem::SdrCustomShapeGeometryItem( const uno::Sequence< beans::PropertyValue >& rVal )
:   SfxPoolItem( SDRATTR_CUSTOMSHAPE_GEOMETRY )
{
    SetPropSeq( rVal );
}

css::uno::Any* SdrCustomShapeGeometryItem::GetPropertyValueByName( const OUString& rPropName )
{
    ASSERT_CHANGE_REFCOUNTED_ITEM;
    css::uno::Any* pRet = nullptr;
    PropertyHashMap::iterator aHashIter( m_aPropHashMap.find( rPropName ) );
    if ( aHashIter != m_aPropHashMap.end() )
        pRet = &m_aPropSeq.getArray()[ (*aHashIter).second ].Value;
    return pRet;
}

const css::uno::Any* SdrCustomShapeGeometryItem::GetPropertyValueByName( const OUString& rPropName ) const
{
    const css::uno::Any* pRet = nullptr;
    PropertyHashMap::const_iterator aHashIter( m_aPropHashMap.find( rPropName ) );
    if ( aHashIter != m_aPropHashMap.end() )
        pRet = &m_aPropSeq[ (*aHashIter).second ].Value;
    return pRet;
}

css::uno::Any* SdrCustomShapeGeometryItem::GetPropertyValueByName( const OUString& rSequenceName, const OUString& rPropName )
{
    ASSERT_CHANGE_REFCOUNTED_ITEM;
    css::uno::Any* pRet = nullptr;
    css::uno::Any* pSeqAny = GetPropertyValueByName( rSequenceName );
    if ( pSeqAny )
    {
        if ( auto rSecSequence = o3tl::tryAccess<css::uno::Sequence<beans::PropertyValue>>(*pSeqAny) )
        {
            PropertyPairHashMap::iterator aHashIter( m_aPropPairHashMap.find( PropertyPair( rSequenceName, rPropName ) ) );
            if ( aHashIter != m_aPropPairHashMap.end() )
            {
                pRet = &const_cast<css::uno::Sequence<css::beans::PropertyValue> &>(*rSecSequence).getArray()[ (*aHashIter).second ].Value;
            }
        }
    }
    return pRet;
}

const css::uno::Any* SdrCustomShapeGeometryItem::GetPropertyValueByName( const OUString& rSequenceName, const OUString& rPropName ) const
{
    const css::uno::Any* pRet = nullptr;
    const css::uno::Any* pSeqAny = GetPropertyValueByName( rSequenceName );
    if ( pSeqAny )
    {
        if ( auto rSecSequence = o3tl::tryAccess<css::uno::Sequence<beans::PropertyValue>>(*pSeqAny) )
        {
            PropertyPairHashMap::const_iterator aHashIter( m_aPropPairHashMap.find( PropertyPair( rSequenceName, rPropName ) ) );
            if ( aHashIter != m_aPropPairHashMap.end() )
            {
                pRet = &(*rSecSequence)[ (*aHashIter).second ].Value;
            }
        }
    }
    return pRet;
}

void SdrCustomShapeGeometryItem::SetPropertyValue( const css::beans::PropertyValue& rPropVal )
{
    ASSERT_CHANGE_REFCOUNTED_ITEM;
    css::uno::Any* pAny = GetPropertyValueByName( rPropVal.Name );
    if ( pAny )
    {   // property is already available
        if ( auto rSecSequence = o3tl::tryAccess<css::uno::Sequence<beans::PropertyValue>>(*pAny) )
        {   // old property is a sequence->each entry has to be removed from the HashPairMap
            for ( auto const & i : *rSecSequence )
            {
                PropertyPairHashMap::iterator aHashIter( m_aPropPairHashMap.find( PropertyPair( rPropVal.Name, i.Name ) ) );
                if ( aHashIter != m_aPropPairHashMap.end() )
                    m_aPropPairHashMap.erase( aHashIter );
            }
        }
        *pAny = rPropVal.Value;
        if ( auto rSecSequence = o3tl::tryAccess<css::uno::Sequence<beans::PropertyValue>>(*pAny) )
        {   // the new property is a sequence->each entry has to be inserted into the HashPairMap
            for ( sal_Int32 i = 0; i < rSecSequence->getLength(); i++ )
            {
                beans::PropertyValue const & rPropVal2 = (*rSecSequence)[ i ];
                m_aPropPairHashMap[ PropertyPair( rPropVal.Name, rPropVal2.Name ) ] = i;
            }
        }
    }
    else
    {   // it's a new property
        assert(std::none_of(std::cbegin(m_aPropSeq), std::cend(m_aPropSeq),
            [&rPropVal](beans::PropertyValue const& rVal)
                { return rVal.Name == rPropVal.Name; } ));
        sal_uInt32 nIndex = m_aPropSeq.getLength();
        m_aPropSeq.realloc( nIndex + 1 );
        m_aPropSeq.getArray()[ nIndex ] = rPropVal ;

        m_aPropHashMap[ rPropVal.Name ] = nIndex;
    }
    InvalidateHash();
}

void SdrCustomShapeGeometryItem::SetPropertyValue( const OUString& rSequenceName, const css::beans::PropertyValue& rPropVal )
{
    ASSERT_CHANGE_REFCOUNTED_ITEM;
    css::uno::Any* pAny = GetPropertyValueByName( rSequenceName, rPropVal.Name );
    if ( pAny ) // just replacing
        *pAny = rPropVal.Value;
    else
    {
        css::uno::Any* pSeqAny = GetPropertyValueByName( rSequenceName );
        if( pSeqAny == nullptr )
        {
            css::uno::Sequence < beans::PropertyValue > aSeq;
            beans::PropertyValue aValue;
            aValue.Name = rSequenceName;
            aValue.Value <<= aSeq;

            assert(std::none_of(std::cbegin(m_aPropSeq), std::cend(m_aPropSeq),
                [&rSequenceName](beans::PropertyValue const& rV)
                    { return rV.Name == rSequenceName; } ));
            sal_uInt32 nIndex = m_aPropSeq.getLength();
            m_aPropSeq.realloc( nIndex + 1 );
            auto pPropSeq = m_aPropSeq.getArray();
            pPropSeq[ nIndex ] = std::move(aValue);
            m_aPropHashMap[ rSequenceName ] = nIndex;

            pSeqAny = &pPropSeq[ nIndex ].Value;
        }

        if (auto pSecSequence = o3tl::tryAccess<css::uno::Sequence<beans::PropertyValue>>(*pSeqAny))
        {
            PropertyPairHashMap::iterator aHashIter(
                m_aPropPairHashMap.find(PropertyPair(rSequenceName, rPropVal.Name)));
            auto& rSeq = const_cast<css::uno::Sequence<css::beans::PropertyValue>&>(*pSecSequence);
            if (aHashIter != m_aPropPairHashMap.end())
            {
                rSeq.getArray()[(*aHashIter).second].Value = rPropVal.Value;
            }
            else
            {
                const sal_Int32 nCount = pSecSequence->getLength();
                rSeq.realloc(nCount + 1);
                rSeq.getArray()[nCount] = rPropVal;

                m_aPropPairHashMap[PropertyPair(rSequenceName, rPropVal.Name)] = nCount;
            }
        }
    }
    InvalidateHash();
}

void SdrCustomShapeGeometryItem::ClearPropertyValue( const OUString& rPropName )
{
    ASSERT_CHANGE_REFCOUNTED_ITEM;
    if ( !m_aPropSeq.hasElements() )
        return;

    PropertyHashMap::iterator aHashIter( m_aPropHashMap.find( rPropName ) );
    if ( aHashIter == m_aPropHashMap.end() )
        return;

    auto pPropSeq = m_aPropSeq.getArray();
    css::uno::Any& rSeqAny = pPropSeq[(*aHashIter).second].Value;
    if (auto pSecSequence
        = o3tl::tryAccess<css::uno::Sequence<beans::PropertyValue>>(rSeqAny))
    {
        for (const auto& rPropVal : *pSecSequence)
        {
            auto _aHashIter(m_aPropPairHashMap.find(PropertyPair(rPropName, rPropVal.Name)));
            if (_aHashIter != m_aPropPairHashMap.end())
                m_aPropPairHashMap.erase(_aHashIter); // removing property from pair hashmap
        }
    }
    sal_Int32 nLength = m_aPropSeq.getLength();
    if ( nLength )
    {
        sal_Int32 nIndex  = (*aHashIter).second;
        if ( nIndex != ( nLength - 1 ) )                        // resizing sequence
        {
            PropertyHashMap::iterator aHashIter2( m_aPropHashMap.find( m_aPropSeq[ nLength - 1 ].Name ) );
            assert(aHashIter2 != m_aPropHashMap.end());
            (*aHashIter2).second = nIndex;
            pPropSeq[ nIndex ] = m_aPropSeq[ nLength - 1 ];
        }
        m_aPropSeq.realloc( nLength - 1 );
    }
    m_aPropHashMap.erase( aHashIter );                            // removing property from hashmap
    InvalidateHash();
}

SdrCustomShapeGeometryItem::~SdrCustomShapeGeometryItem()
{
}

bool SdrCustomShapeGeometryItem::operator==( const SfxPoolItem& rCmp ) const
{
    if( !SfxPoolItem::operator==( rCmp ))
        return false;
    const SdrCustomShapeGeometryItem& other = static_cast<const SdrCustomShapeGeometryItem&>(rCmp);
    // This is called often by SfxItemPool, and comparing uno sequences is relatively slow.
    // So keep a hash of the sequence and if either of the sequences has a usable hash,
    // compare using that.
    UpdateHash();
    other.UpdateHash();
    if( m_aHashState != other.m_aHashState )
        return false;
    if( m_aHashState == HashState::Valid && m_aHash != other.m_aHash )
        return false;

    return m_aPropSeq == other.m_aPropSeq;
}

void SdrCustomShapeGeometryItem::UpdateHash() const
{
    if( m_aHashState != HashState::Unknown )
        return;
    std::optional< size_t > hash = comphelper::anyToHash( css::uno::Any( m_aPropSeq ));
    if( hash.has_value())
    {
        m_aHash = *hash;
        m_aHashState = HashState::Valid;
    }
    else
        m_aHashState = HashState::Unusable;
}

void SdrCustomShapeGeometryItem::InvalidateHash()
{
    m_aHashState = HashState::Unknown;
}

bool SdrCustomShapeGeometryItem::GetPresentation(
    SfxItemPresentation ePresentation, MapUnit /*eCoreMetric*/,
    MapUnit /*ePresentationMetric*/, OUString &rText, const IntlWrapper&) const
{
    rText += " ";
    if ( ePresentation == SfxItemPresentation::Complete )
    {
        rText = " " + rText;
        return true;
    }
    else if ( ePresentation == SfxItemPresentation::Nameless )
        return true;
    return false;
}

SdrCustomShapeGeometryItem* SdrCustomShapeGeometryItem::Clone( SfxItemPool * /*pPool*/ ) const
{
    return new SdrCustomShapeGeometryItem( m_aPropSeq );
}

bool SdrCustomShapeGeometryItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    rVal <<= m_aPropSeq;
    return true;
}

bool SdrCustomShapeGeometryItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    ASSERT_CHANGE_REFCOUNTED_ITEM;
    css::uno::Sequence< css::beans::PropertyValue > propSeq;
    if ( ! ( rVal >>= propSeq ) )
        return false;

    SetPropSeq( propSeq );
    return true;
}

void SdrCustomShapeGeometryItem::SetPropSeq( const css::uno::Sequence< css::beans::PropertyValue >& rVal )
{
    ASSERT_CHANGE_REFCOUNTED_ITEM;
    if( m_aPropSeq == rVal )
        return;

    m_aPropSeq = rVal;
    m_aPropHashMap.clear();
    m_aPropPairHashMap.clear();
    for ( sal_Int32 i = 0; i < m_aPropSeq.getLength(); i++ )
    {
        const beans::PropertyValue& rPropVal = m_aPropSeq[ i ];
        std::pair<PropertyHashMap::iterator, bool> const ret(
                m_aPropHashMap.insert(std::make_pair(rPropVal.Name, i)));
        assert(ret.second); // serious bug: duplicate xml attribute exported
        if (!ret.second)
        {
            throw uno::RuntimeException(
                "CustomShapeGeometry has duplicate property " + rPropVal.Name);
        }
        if (auto rPropSeq = o3tl::tryAccess<uno::Sequence<beans::PropertyValue>>(
                rPropVal.Value))
        {
            for ( sal_Int32 j = 0; j < rPropSeq->getLength(); j++ )
            {
                beans::PropertyValue const & rPropVal2 = (*rPropSeq)[ j ];
                m_aPropPairHashMap[ PropertyPair( rPropVal.Name, rPropVal2.Name ) ] = j;
            }
        }
    }
    InvalidateHash();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
