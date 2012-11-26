/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <svx/sdasitm.hxx>
#include <svx/svdattr.hxx>

using namespace ::std;
using namespace com::sun::star;

bool SdrCustomShapeGeometryItem::PropertyEq::operator()( const rtl::OUString& r1, const rtl::OUString& r2 ) const
{
    return r1.equals( r2 );
}
bool SdrCustomShapeGeometryItem::PropertyPairEq::operator()( const SdrCustomShapeGeometryItem::PropertyPair& r1, const SdrCustomShapeGeometryItem::PropertyPair& r2 ) const
{
    return ( r1.first.equals( r2.first ) ) && ( r1.second.equals( r2.second ) );
}
size_t SdrCustomShapeGeometryItem::PropertyPairHash::operator()( const SdrCustomShapeGeometryItem::PropertyPair &r1 ) const
{
    return (size_t)r1.first.hashCode() + r1.second.hashCode();
};

SdrCustomShapeGeometryItem::SdrCustomShapeGeometryItem()
:   SfxPoolItem( SDRATTR_CUSTOMSHAPE_GEOMETRY )
{}
SdrCustomShapeGeometryItem::SdrCustomShapeGeometryItem( const uno::Sequence< beans::PropertyValue >& rVal )
:   SfxPoolItem( SDRATTR_CUSTOMSHAPE_GEOMETRY )
{
    sal_Int32 i, j;
    aPropSeq = rVal;

    // hashing property values
//  beans::PropertyValue* pPropValues = aPropSeq.getArray();
//  const rtl::OUString* pPtr = NULL;
    for ( i = 0; i < aPropSeq.getLength(); i++ )
    {
        beans::PropertyValue& rPropVal = aPropSeq[ i ];
        aPropHashMap[ rPropVal.Name ] = i;
        if ( rPropVal.Value.getValueType() == ::getCppuType((const ::com::sun::star::uno::Sequence < beans::PropertyValue >*)0) )
        {
            uno::Sequence< beans::PropertyValue >& rPropSeq = *( uno::Sequence< beans::PropertyValue >*)rPropVal.Value.getValue();
            for ( j = 0; j < rPropSeq.getLength(); j++ )
            {
                beans::PropertyValue& rPropVal2 = rPropSeq[ j ];
                aPropPairHashMap[ PropertyPair( rPropVal.Name, rPropVal2.Name ) ] = j;
            }
        }
    }
}

com::sun::star::uno::Any* SdrCustomShapeGeometryItem::GetPropertyValueByName( const rtl::OUString& rPropName )
{
    com::sun::star::uno::Any* pRet = NULL;
    PropertyHashMap::iterator aHashIter( aPropHashMap.find( rPropName ) );
    if ( aHashIter != aPropHashMap.end() )
        pRet = &aPropSeq[ (*aHashIter).second ].Value;
    return pRet;
}

com::sun::star::uno::Any* SdrCustomShapeGeometryItem::GetPropertyValueByName( const rtl::OUString& rSequenceName, const rtl::OUString& rPropName )
{
    com::sun::star::uno::Any* pRet = NULL;
    com::sun::star::uno::Any* pSeqAny = GetPropertyValueByName( rSequenceName );
    if ( pSeqAny )
    {
        if ( pSeqAny->getValueType() == ::getCppuType((const ::com::sun::star::uno::Sequence < beans::PropertyValue >*)0) )
        {
            PropertyPairHashMap::iterator aHashIter( aPropPairHashMap.find( PropertyPair( rSequenceName, rPropName ) ) );
            if ( aHashIter != aPropPairHashMap.end() )
            {
                ::com::sun::star::uno::Sequence < beans::PropertyValue >& rSecSequence =
                    *((::com::sun::star::uno::Sequence < beans::PropertyValue >*)pSeqAny->getValue());
                pRet = &rSecSequence[ (*aHashIter).second ].Value;
            }
        }
    }
    return pRet;
}

void SdrCustomShapeGeometryItem::SetPropertyValue( const com::sun::star::beans::PropertyValue& rPropVal )
{
    com::sun::star::uno::Any* pAny = GetPropertyValueByName( rPropVal.Name );
    if ( pAny )
    {   // property is already available
        sal_Int32 i;
        if ( pAny->getValueType() == ::getCppuType((const ::com::sun::star::uno::Sequence < beans::PropertyValue >*)0) )
        {   // old property is a sequence->each entry has to be removed from the HashPairMap
            ::com::sun::star::uno::Sequence < beans::PropertyValue >& rSecSequence =
                *((::com::sun::star::uno::Sequence < beans::PropertyValue >*)pAny->getValue());
            for ( i = 0; i < rSecSequence.getLength(); i++ )
            {
                PropertyPairHashMap::iterator aHashIter( aPropPairHashMap.find( PropertyPair( rPropVal.Name, rSecSequence[ i ].Name ) ) );
                if ( aHashIter != aPropPairHashMap.end() )
                    aPropPairHashMap.erase( aHashIter );
            }
        }
        *pAny = rPropVal.Value;
        if ( rPropVal.Value.getValueType() == ::getCppuType((const ::com::sun::star::uno::Sequence < beans::PropertyValue >*)0) )
        {   // the new property is a sequence->each entry has to be inserted into the HashPairMap
            ::com::sun::star::uno::Sequence < beans::PropertyValue >& rSecSequence =
                *((::com::sun::star::uno::Sequence < beans::PropertyValue >*)pAny->getValue());
            for ( i = 0; i < rSecSequence.getLength(); i++ )
            {
                beans::PropertyValue& rPropVal2 = rSecSequence[ i ];
                aPropPairHashMap[ PropertyPair( rPropVal.Name, rPropVal2.Name ) ] = i;
            }
        }
    }
    else
    {   // its a new property
        sal_uInt32 nIndex = aPropSeq.getLength();
        aPropSeq.realloc( nIndex + 1 );
        aPropSeq[ nIndex ] = rPropVal ;

        aPropHashMap[ rPropVal.Name ] = nIndex;
    }
}

void SdrCustomShapeGeometryItem::SetPropertyValue( const rtl::OUString& rSequenceName, const com::sun::star::beans::PropertyValue& rPropVal )
{
    com::sun::star::uno::Any* pAny = GetPropertyValueByName( rSequenceName, rPropVal.Name );
    if ( pAny ) // just replacing
        *pAny = rPropVal.Value;
    else
    {
        com::sun::star::uno::Any* pSeqAny = GetPropertyValueByName( rSequenceName );
        if( pSeqAny == NULL )
        {
            ::com::sun::star::uno::Sequence < beans::PropertyValue > aSeq;
            beans::PropertyValue aValue;
            aValue.Name = rSequenceName;
            aValue.Value = ::com::sun::star::uno::makeAny( aSeq );

            sal_uInt32 nIndex = aPropSeq.getLength();
            aPropSeq.realloc( nIndex + 1 );
            aPropSeq[ nIndex ] = aValue;
            aPropHashMap[ rSequenceName ] = nIndex;

            pSeqAny = &aPropSeq[ nIndex ].Value;
        }

        DBG_ASSERT( pSeqAny, "SdrCustomShapeGeometryItem::SetPropertyValue() - No Value??" );

        if( pSeqAny )
        {
            if ( pSeqAny->getValueType() == ::getCppuType((const ::com::sun::star::uno::Sequence < beans::PropertyValue >*)0) )
            {
                PropertyPairHashMap::iterator aHashIter( aPropPairHashMap.find( PropertyPair( rSequenceName, rPropVal.Name ) ) );
                if ( aHashIter != aPropPairHashMap.end() )
                {
                    ::com::sun::star::uno::Sequence < beans::PropertyValue >& rSecSequence =
                        *((::com::sun::star::uno::Sequence < beans::PropertyValue >*)pSeqAny->getValue());
                    rSecSequence[ (*aHashIter).second ].Value = rPropVal.Value;
                }
                else
                {
                    ::com::sun::star::uno::Sequence < beans::PropertyValue >& rSecSequence =
                        *((::com::sun::star::uno::Sequence < beans::PropertyValue >*)pSeqAny->getValue());

                    sal_Int32 nCount = rSecSequence.getLength();
                    rSecSequence.realloc( nCount + 1 );
                    rSecSequence[ nCount ] = rPropVal;

                    aPropPairHashMap[ PropertyPair( rSequenceName, rPropVal.Name ) ] = nCount;
                }
            }
        }
    }
}

void SdrCustomShapeGeometryItem::ClearPropertyValue( const rtl::OUString& rPropName )
{
    if ( aPropSeq.getLength() )
    {
        PropertyHashMap::iterator aHashIter( aPropHashMap.find( rPropName ) );
        if ( aHashIter != aPropHashMap.end() )
        {
             com::sun::star::uno::Any* pSeqAny = &aPropSeq[ (*aHashIter).second ].Value;
            if ( pSeqAny )
            {
                if ( pSeqAny->getValueType() == ::getCppuType((const ::com::sun::star::uno::Sequence < beans::PropertyValue >*)0) )
                {
                    ::com::sun::star::uno::Sequence < beans::PropertyValue >& rSecSequence =
                        *((::com::sun::star::uno::Sequence < beans::PropertyValue >*)pSeqAny->getValue());

                    sal_Int32 i;
                    for ( i = 0; i < rSecSequence.getLength(); i++ )
                    {
                        PropertyPairHashMap::iterator _aHashIter( aPropPairHashMap.find( PropertyPair( rPropName, rSecSequence[ i ].Name ) ) );
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

void SdrCustomShapeGeometryItem::ClearPropertyValue( const rtl::OUString& rSequenceName, const rtl::OUString& rPropName )
{
    com::sun::star::uno::Any* pSeqAny = GetPropertyValueByName( rSequenceName );
    if ( pSeqAny )
    {
        if ( pSeqAny->getValueType() == ::getCppuType((const ::com::sun::star::uno::Sequence < beans::PropertyValue >*)0) )
        {
            PropertyPairHashMap::iterator aHashIter( aPropPairHashMap.find( PropertyPair( rSequenceName, rPropName ) ) );
            if ( aHashIter != aPropPairHashMap.end() )
            {
                ::com::sun::star::uno::Sequence < beans::PropertyValue >& rSecSequence =
                    *((::com::sun::star::uno::Sequence < beans::PropertyValue >*)pSeqAny->getValue());

                sal_Int32 nLength = rSecSequence.getLength();
                if ( nLength )
                {
                    sal_Int32 nIndex  = (*aHashIter).second;
                    if ( nIndex != ( nLength - 1 ) )                            // resizing sequence
                    {
                        PropertyPairHashMap::iterator aHashIter2( aPropPairHashMap.find( PropertyPair( rSequenceName, rSecSequence[ nLength - 1 ].Name ) ) );
                        (*aHashIter2).second = nIndex;
                        rSecSequence[ nIndex ] = rSecSequence[ nLength - 1 ];
                    }
                    rSecSequence.realloc( aPropSeq.getLength() - 1 );
                }
                aPropPairHashMap.erase( aHashIter );
            }
        }
    }
}

SdrCustomShapeGeometryItem::~SdrCustomShapeGeometryItem()
{
}
SdrCustomShapeGeometryItem::SdrCustomShapeGeometryItem( SvStream& /*rIn*/, sal_uInt16 nVersion ):
    SfxPoolItem( SDRATTR_CUSTOMSHAPE_GEOMETRY )
{
    if ( nVersion )
    {

    }
}
int __EXPORT SdrCustomShapeGeometryItem::operator==( const SfxPoolItem& rCmp ) const
{
    int bRet = SfxPoolItem::operator==( rCmp );
    if ( bRet )
        bRet = ((SdrCustomShapeGeometryItem&)rCmp).aPropSeq == aPropSeq;
    return bRet;
}

SfxItemPresentation __EXPORT SdrCustomShapeGeometryItem::GetPresentation(
    SfxItemPresentation ePresentation, SfxMapUnit /*eCoreMetric*/,
    SfxMapUnit /*ePresentationMetric*/, XubString &rText, const IntlWrapper *) const
{
    rText += sal_Unicode( ' ' );
    if ( ePresentation == SFX_ITEM_PRESENTATION_COMPLETE )
    {
        XubString aStr;
//      SdrItemPool::TakeItemName( Which(), aStr );
        aStr += sal_Unicode( ' ' );
        rText.Insert( aStr, 0 );
    }
    return ePresentation;
}

SfxPoolItem* __EXPORT SdrCustomShapeGeometryItem::Create( SvStream& rIn, sal_uInt16 nItemVersion ) const
{
    return new SdrCustomShapeGeometryItem( rIn, nItemVersion );
}

SvStream& __EXPORT SdrCustomShapeGeometryItem::Store( SvStream& rOut, sal_uInt16 nItemVersion ) const
{
    if ( nItemVersion )
    {

    }
    return rOut;
}

SfxPoolItem* __EXPORT SdrCustomShapeGeometryItem::Clone( SfxItemPool * /*pPool*/ ) const
{
    SdrCustomShapeGeometryItem* pItem = new SdrCustomShapeGeometryItem( GetGeometry() );
//  SdrCustomShapeGeometryItem* pItem = new SdrCustomShapeGeometryItem( *this );

/*
    for ( i = 0; i < GetCount(); i++ )
    {
        const SdrCustomShapeAdjustmentValue& rVal = GetValue( i );
        pItem->SetValue( i, rVal );
    }
*/
    return pItem;
}

#ifdef SDR_ISPOOLABLE
int __EXPORT SdrCustomShapeGeometryItem::IsPoolable() const
{
    sal_uInt16 nId=Which();
    return nId < SDRATTR_NOTPERSIST_FIRST || nId > SDRATTR_NOTPERSIST_LAST;
}
#endif
sal_uInt16 SdrCustomShapeGeometryItem::GetVersion( sal_uInt16 /*nFileFormatVersion*/ ) const
{
    return 1;
}
sal_Bool SdrCustomShapeGeometryItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    rVal <<= aPropSeq;
    return sal_True;
}
sal_Bool SdrCustomShapeGeometryItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    if ( ! ( rVal >>= aPropSeq ) )
        return sal_False;
    else
        return sal_True;
}
const uno::Sequence< beans::PropertyValue >& SdrCustomShapeGeometryItem::GetGeometry() const
{
    return aPropSeq;
}

// eof
