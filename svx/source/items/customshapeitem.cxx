/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: customshapeitem.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 12:53:25 $
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
#include "precompiled_svx.hxx"

#ifndef _SDASITM_HXX
#include "sdasitm.hxx"
#endif
#include "svdattr.hxx"

using namespace ::std;
using namespace com::sun::star;

SdrCustomShapeEngineItem::SdrCustomShapeEngineItem()
:   SfxStringItem( SDRATTR_CUSTOMSHAPE_ENGINE, String() )
{}
SdrCustomShapeEngineItem::SdrCustomShapeEngineItem( const String& rVal )
:   SfxStringItem( SDRATTR_CUSTOMSHAPE_ENGINE, rVal )
{}

SdrCustomShapeDataItem::SdrCustomShapeDataItem()
:   SfxStringItem( SDRATTR_CUSTOMSHAPE_DATA, String() )
{}
SdrCustomShapeDataItem::SdrCustomShapeDataItem( const String& rVal )
:   SfxStringItem( SDRATTR_CUSTOMSHAPE_DATA, rVal )
{}

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

TYPEINIT1_AUTOFACTORY( SdrCustomShapeGeometryItem, SfxPoolItem );
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
    USHORT nId=Which();
    return nId < SDRATTR_NOTPERSIST_FIRST || nId > SDRATTR_NOTPERSIST_LAST;
}
#endif
sal_uInt16 SdrCustomShapeGeometryItem::GetVersion( sal_uInt16 /*nFileFormatVersion*/ ) const
{
    return 1;
}
sal_Bool SdrCustomShapeGeometryItem::QueryValue( uno::Any& rVal, BYTE /*nMemberId*/ ) const
{
    rVal <<= aPropSeq;
    return sal_True;
}
sal_Bool SdrCustomShapeGeometryItem::PutValue( const uno::Any& rVal, BYTE /*nMemberId*/ )
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
/*
const uno::Any* GetValueByName( const rtl::OUString& rProperty ) const
{

}
*/
SdrCustomShapeReplacementURLItem::SdrCustomShapeReplacementURLItem()
:   SfxStringItem( SDRATTR_CUSTOMSHAPE_REPLACEMENT_URL, String() )
{}
SdrCustomShapeReplacementURLItem::SdrCustomShapeReplacementURLItem( const String& rVal )
:   SfxStringItem( SDRATTR_CUSTOMSHAPE_REPLACEMENT_URL, rVal )
{}

