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
#include <svx/sdasitm.hxx>
#include <svx/svdattr.hxx>

using namespace ::std;
using namespace com::sun::star;


SdrCustomShapeGeometryItem::SdrCustomShapeGeometryItem()
:   SfxPoolItem( SDRATTR_CUSTOMSHAPE_GEOMETRY )
{}

SdrCustomShapeGeometryItem::SdrCustomShapeGeometryItem( const uno::Sequence< beans::PropertyValue >& rVal )
:   SfxPoolItem( SDRATTR_CUSTOMSHAPE_GEOMETRY )
{
    sal_Int32 i, j;
    aPropSeq = rVal;

    for ( i = 0; i < aPropSeq.getLength(); i++ )
    {
        beans::PropertyValue& rPropVal = aPropSeq[ i ];
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
            for ( j = 0; j < rPropSeq->getLength(); j++ )
            {
                beans::PropertyValue const & rPropVal2 = (*rPropSeq)[ j ];
                aPropPairHashMap[ PropertyPair( rPropVal.Name, rPropVal2.Name ) ] = j;
            }
        }
    }
}

css::uno::Any* SdrCustomShapeGeometryItem::GetPropertyValueByName( const OUString& rPropName )
{
    css::uno::Any* pRet = nullptr;
    PropertyHashMap::iterator aHashIter( aPropHashMap.find( rPropName ) );
    if ( aHashIter != aPropHashMap.end() )
        pRet = &aPropSeq[ (*aHashIter).second ].Value;
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
                pRet = const_cast<css::uno::Any *>(&(*rSecSequence)[ (*aHashIter).second ].Value);
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
        sal_Int32 i;
        if ( auto rSecSequence = o3tl::tryAccess<css::uno::Sequence<beans::PropertyValue>>(*pAny) )
        {   // old property is a sequence->each entry has to be removed from the HashPairMap
            for ( i = 0; i < rSecSequence->getLength(); i++ )
            {
                PropertyPairHashMap::iterator aHashIter( aPropPairHashMap.find( PropertyPair( rPropVal.Name, (*rSecSequence)[ i ].Name ) ) );
                if ( aHashIter != aPropPairHashMap.end() )
                    aPropPairHashMap.erase( aHashIter );
            }
        }
        *pAny = rPropVal.Value;
        if ( auto rSecSequence = o3tl::tryAccess<css::uno::Sequence<beans::PropertyValue>>(*pAny) )
        {   // the new property is a sequence->each entry has to be inserted into the HashPairMap
            for ( i = 0; i < rSecSequence->getLength(); i++ )
            {
                beans::PropertyValue const & rPropVal2 = (*rSecSequence)[ i ];
                aPropPairHashMap[ PropertyPair( rPropVal.Name, rPropVal2.Name ) ] = i;
            }
        }
    }
    else
    {   // it's a new property
        assert(aPropSeq.end() == std::find_if(aPropSeq.begin(), aPropSeq.end(),
            [&rPropVal](beans::PropertyValue const& rVal)
                { return rVal.Name == rPropVal.Name; } ));
        sal_uInt32 nIndex = aPropSeq.getLength();
        aPropSeq.realloc( nIndex + 1 );
        aPropSeq[ nIndex ] = rPropVal ;

        aPropHashMap[ rPropVal.Name ] = nIndex;
    }
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

            assert(aPropSeq.end() == std::find_if(aPropSeq.begin(), aPropSeq.end(),
                [&rSequenceName](beans::PropertyValue const& rV)
                    { return rV.Name == rSequenceName; } ));
            sal_uInt32 nIndex = aPropSeq.getLength();
            aPropSeq.realloc( nIndex + 1 );
            aPropSeq[ nIndex ] = aValue;
            aPropHashMap[ rSequenceName ] = nIndex;

            pSeqAny = &aPropSeq[ nIndex ].Value;
        }

        DBG_ASSERT( pSeqAny, "SdrCustomShapeGeometryItem::SetPropertyValue() - No Value??" );

        if( pSeqAny )
        {
            if ( auto rSecSequence = o3tl::tryAccess<css::uno::Sequence<beans::PropertyValue>>(*pSeqAny) )
            {
                PropertyPairHashMap::iterator aHashIter( aPropPairHashMap.find( PropertyPair( rSequenceName, rPropVal.Name ) ) );
                if ( aHashIter != aPropPairHashMap.end() )
                {
                    const_cast<css::uno::Sequence<css::beans::PropertyValue> &>(*rSecSequence)[ (*aHashIter).second ].Value = rPropVal.Value;
                }
                else
                {
                    sal_Int32 nCount = rSecSequence->getLength();
                    const_cast<css::uno::Sequence<css::beans::PropertyValue> &>(*rSecSequence).realloc( nCount + 1 );
                    const_cast<css::uno::Sequence<css::beans::PropertyValue> &>(*rSecSequence)[ nCount ] = rPropVal;

                    aPropPairHashMap[ PropertyPair( rSequenceName, rPropVal.Name ) ] = nCount;
                }
            }
        }
    }
}

void SdrCustomShapeGeometryItem::ClearPropertyValue( const OUString& rPropName )
{
    if ( aPropSeq.getLength() )
    {
        PropertyHashMap::iterator aHashIter( aPropHashMap.find( rPropName ) );
        if ( aHashIter != aPropHashMap.end() )
        {
             css::uno::Any* pSeqAny = &aPropSeq[ (*aHashIter).second ].Value;
            if ( pSeqAny )
            {
                if ( auto rSecSequence = o3tl::tryAccess<css::uno::Sequence<beans::PropertyValue>>(*pSeqAny) )
                {
                    sal_Int32 i;
                    for ( i = 0; i < rSecSequence->getLength(); i++ )
                    {
                        PropertyPairHashMap::iterator _aHashIter( aPropPairHashMap.find( PropertyPair( rPropName, (*rSecSequence)[ i ].Name ) ) );
                        if ( _aHashIter != aPropPairHashMap.end() )
                            aPropPairHashMap.erase( _aHashIter );       // removing property from pair hashmap
                    }
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
                    aPropSeq[ (*aHashIter).second ] = aPropSeq[ aPropSeq.getLength() - 1 ];
                }
                aPropSeq.realloc( aPropSeq.getLength() - 1 );
            }
            aPropHashMap.erase( aHashIter );                            // removing property from hashmap
        }
    }
}

SdrCustomShapeGeometryItem::~SdrCustomShapeGeometryItem()
{
}
bool SdrCustomShapeGeometryItem::operator==( const SfxPoolItem& rCmp ) const
{
    bool bRet = SfxPoolItem::operator==( rCmp );
    if ( bRet )
        bRet = static_cast<const SdrCustomShapeGeometryItem&>(rCmp).aPropSeq == aPropSeq;
    return bRet;
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

SfxPoolItem* SdrCustomShapeGeometryItem::Clone( SfxItemPool * /*pPool*/ ) const
{
    return new SdrCustomShapeGeometryItem( aPropSeq );
}

sal_uInt16 SdrCustomShapeGeometryItem::GetVersion( sal_uInt16 /*nFileFormatVersion*/ ) const
{
    return 1;
}
bool SdrCustomShapeGeometryItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    rVal <<= aPropSeq;
    return true;
}
bool SdrCustomShapeGeometryItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    if ( ! ( rVal >>= aPropSeq ) )
        return false;
    else
    {
        for (sal_Int32 i = 0; i < aPropSeq.getLength(); ++i)
        {
            for (sal_Int32 j = i+1; j < aPropSeq.getLength(); ++j)
            {
                if (aPropSeq[i].Name == aPropSeq[j].Name)
                {
                    assert(false); // serious bug: duplicate xml attribute exported
                    OUString const name(aPropSeq[i].Name);
                    aPropSeq.realloc(0);
                    throw uno::RuntimeException(
                        "CustomShapeGeometry has duplicate property " + name);
                }
            }
        }
        return true;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
