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
#include <comphelper/anycompare.hxx>
#include <comphelper/anytohash.hxx>
#include <svx/sdasitm.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>

using namespace ::std;
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
    css::uno::Any* pRet = nullptr;
    PropertyHashMap::iterator aHashIter( aPropHashMap.find( rPropName ) );
    if ( aHashIter != aPropHashMap.end() )
        pRet = &aPropSeq.getArray()[ (*aHashIter).second ].Value;
    return pRet;
}

const css::uno::Any* SdrCustomShapeGeometryItem::GetPropertyValueByName( const OUString& rPropName ) const
{
    const css::uno::Any* pRet = nullptr;
    PropertyHashMap::const_iterator aHashIter( aPropHashMap.find( rPropName ) );
    if ( aHashIter != aPropHashMap.end() )
        pRet = &aPropSeq[ (*aHashIter).second ].Value;
    return pRet;
}

css::uno::Any* SdrCustomShapeGeometryItem::GetPropertyValueByName( const OUString& rSequenceName, const OUString& rPropName )
{
    css::uno::Any* pRet = nullptr;
    css::uno::Any* pSeqAny = GetPropertyValueByName( rSequenceName );
    if ( pSeqAny )
    {
        if ( auto rSecSequence = o3tl::tryAccess<css::uno::Sequence<beans::PropertyValue>>(*pSeqAny) )
        {
            PropertyPairHashMap::iterator aHashIter( aPropPairHashMap.find( PropertyPair( rSequenceName, rPropName ) ) );
            if ( aHashIter != aPropPairHashMap.end() )
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
            PropertyPairHashMap::const_iterator aHashIter( aPropPairHashMap.find( PropertyPair( rSequenceName, rPropName ) ) );
            if ( aHashIter != aPropPairHashMap.end() )
            {
                pRet = &(*rSecSequence)[ (*aHashIter).second ].Value;
            }
        }
    }
    return pRet;
}

void SdrCustomShapeGeometryItem::SetPropertyValue( const css::beans::PropertyValue& rPropVal )
{
    css::uno::Any* pAny = GetPropertyValueByName( rPropVal.Name );
    if ( pAny )
    {   // property is already available
        if ( auto rSecSequence = o3tl::tryAccess<css::uno::Sequence<beans::PropertyValue>>(*pAny) )
        {   // old property is a sequence->each entry has to be removed from the HashPairMap
            for ( auto const & i : *rSecSequence )
            {
                PropertyPairHashMap::iterator aHashIter( aPropPairHashMap.find( PropertyPair( rPropVal.Name, i.Name ) ) );
                if ( aHashIter != aPropPairHashMap.end() )
                    aPropPairHashMap.erase( aHashIter );
            }
        }
        *pAny = rPropVal.Value;
        if ( auto rSecSequence = o3tl::tryAccess<css::uno::Sequence<beans::PropertyValue>>(*pAny) )
        {   // the new property is a sequence->each entry has to be inserted into the HashPairMap
            for ( sal_Int32 i = 0; i < rSecSequence->getLength(); i++ )
            {
                beans::PropertyValue const & rPropVal2 = (*rSecSequence)[ i ];
                aPropPairHashMap[ PropertyPair( rPropVal.Name, rPropVal2.Name ) ] = i;
            }
        }
    }
    else
    {   // it's a new property
        assert(std::none_of(std::cbegin(aPropSeq), std::cend(aPropSeq),
            [&rPropVal](beans::PropertyValue const& rVal)
                { return rVal.Name == rPropVal.Name; } ));
        sal_uInt32 nIndex = aPropSeq.getLength();
        aPropSeq.realloc( nIndex + 1 );
        aPropSeq.getArray()[ nIndex ] = rPropVal ;

        aPropHashMap[ rPropVal.Name ] = nIndex;
    }
    InvalidateHash();
}

void SdrCustomShapeGeometryItem::SetPropertyValue( const OUString& rSequenceName, const css::beans::PropertyValue& rPropVal )
{
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

            assert(std::none_of(std::cbegin(aPropSeq), std::cend(aPropSeq),
                [&rSequenceName](beans::PropertyValue const& rV)
                    { return rV.Name == rSequenceName; } ));
            sal_uInt32 nIndex = aPropSeq.getLength();
            aPropSeq.realloc( nIndex + 1 );
            auto pPropSeq = aPropSeq.getArray();
            pPropSeq[ nIndex ] = aValue;
            aPropHashMap[ rSequenceName ] = nIndex;

            pSeqAny = &pPropSeq[ nIndex ].Value;
        }

        if (auto pSecSequence = o3tl::tryAccess<css::uno::Sequence<beans::PropertyValue>>(*pSeqAny))
        {
            PropertyPairHashMap::iterator aHashIter(
                aPropPairHashMap.find(PropertyPair(rSequenceName, rPropVal.Name)));
            auto& rSeq = const_cast<css::uno::Sequence<css::beans::PropertyValue>&>(*pSecSequence);
            if (aHashIter != aPropPairHashMap.end())
            {
                rSeq.getArray()[(*aHashIter).second].Value = rPropVal.Value;
            }
            else
            {
                const sal_Int32 nCount = pSecSequence->getLength();
                rSeq.realloc(nCount + 1);
                rSeq.getArray()[nCount] = rPropVal;

                aPropPairHashMap[PropertyPair(rSequenceName, rPropVal.Name)] = nCount;
            }
        }
    }
    InvalidateHash();
}

void SdrCustomShapeGeometryItem::ClearPropertyValue( const OUString& rPropName )
{
    if ( !aPropSeq.hasElements() )
        return;

    PropertyHashMap::iterator aHashIter( aPropHashMap.find( rPropName ) );
    if ( aHashIter == aPropHashMap.end() )
        return;

    auto pPropSeq = aPropSeq.getArray();
    css::uno::Any& rSeqAny = pPropSeq[(*aHashIter).second].Value;
    if (auto pSecSequence
        = o3tl::tryAccess<css::uno::Sequence<beans::PropertyValue>>(rSeqAny))
    {
        for (const auto& rPropVal : *pSecSequence)
        {
            auto _aHashIter(aPropPairHashMap.find(PropertyPair(rPropName, rPropVal.Name)));
            if (_aHashIter != aPropPairHashMap.end())
                aPropPairHashMap.erase(_aHashIter); // removing property from pair hashmap
        }
    }
    sal_Int32 nLength = aPropSeq.getLength();
    if ( nLength )
    {
        sal_Int32 nIndex  = (*aHashIter).second;
        if ( nIndex != ( nLength - 1 ) )                        // resizing sequence
        {
            PropertyHashMap::iterator aHashIter2( aPropHashMap.find( aPropSeq[ nLength - 1 ].Name ) );
            (*aHashIter2).second = nIndex;
            pPropSeq[ nIndex ] = aPropSeq[ nLength - 1 ];
        }
        aPropSeq.realloc( nLength - 1 );
    }
    aPropHashMap.erase( aHashIter );                            // removing property from hashmap
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
    if( aHashState != other.aHashState )
        return false;
    if( aHashState == HashState::Valid && aHash != other.aHash )
        return false;

    return aPropSeq == other.aPropSeq;
}

bool SdrCustomShapeGeometryItem::operator<( const SfxPoolItem& rCmp ) const
{
    assert(dynamic_cast<const SdrCustomShapeGeometryItem*>( &rCmp ));
    const SdrCustomShapeGeometryItem& other = static_cast<const SdrCustomShapeGeometryItem&>(rCmp);
    // Again, try to optimize by checking hashes first (this is operator< for sorting purposes,
    // so the ordering can be somewhat arbitrary).
    UpdateHash();
    other.UpdateHash();
    if( aHashState != other.aHashState )
        return aHashState < other.aHashState;
    if( aHashState == HashState::Valid )
        return aHash < other.aHash;

    return comphelper::anyLess( css::uno::Any( aPropSeq ),
        css::uno::Any( other.aPropSeq ));
}

void SdrCustomShapeGeometryItem::UpdateHash() const
{
    if( aHashState != HashState::Unknown )
        return;
    std::optional< size_t > hash = comphelper::anyToHash( css::uno::Any( aPropSeq ));
    if( hash.has_value())
    {
        aHash = *hash;
        aHashState = HashState::Valid;
    }
    else
        aHashState = HashState::Unusable;
}

void SdrCustomShapeGeometryItem::InvalidateHash()
{
    aHashState = HashState::Unknown;
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
    return new SdrCustomShapeGeometryItem( aPropSeq );
}

bool SdrCustomShapeGeometryItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    rVal <<= aPropSeq;
    return true;
}

bool SdrCustomShapeGeometryItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    css::uno::Sequence< css::beans::PropertyValue > propSeq;
    if ( ! ( rVal >>= propSeq ) )
        return false;

    SetPropSeq( propSeq );
    return true;
}

void SdrCustomShapeGeometryItem::SetPropSeq( const css::uno::Sequence< css::beans::PropertyValue >& rVal )
{
    if( aPropSeq == rVal )
        return;

    aPropSeq = rVal;
    aPropHashMap.clear();
    aPropPairHashMap.clear();
    for ( sal_Int32 i = 0; i < aPropSeq.getLength(); i++ )
    {
        const beans::PropertyValue& rPropVal = aPropSeq[ i ];
        std::pair<PropertyHashMap::iterator, bool> const ret(
                aPropHashMap.insert(std::make_pair(rPropVal.Name, i)));
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
                aPropPairHashMap[ PropertyPair( rPropVal.Name, rPropVal2.Name ) ] = j;
            }
        }
    }
    InvalidateHash();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
