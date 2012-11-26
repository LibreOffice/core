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
#include <tools/stream.hxx>
#ifndef __SBX_SBXVARIABLE_HXX
#include <basic/sbxvar.hxx>
#endif

#include <svx/zoomitem.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

// -----------------------------------------------------------------------

#define ZOOM_PARAM_VALUE    "Value"
#define ZOOM_PARAM_VALUESET "ValueSet"
#define ZOOM_PARAM_TYPE     "Type"
#define ZOOM_PARAMS         3

// -----------------------------------------------------------------------

IMPL_POOLITEM_FACTORY(SvxZoomItem)

SvxZoomItem::SvxZoomItem
(
    SvxZoomType eZoomType,
    sal_uInt16      nVal,
    sal_uInt16      _nWhich
)
:   SfxUInt16Item( _nWhich, nVal ),
    nValueSet( SVX_ZOOM_ENABLE_ALL ),
    eType( eZoomType )
{
}

// -----------------------------------------------------------------------

SvxZoomItem::SvxZoomItem( const SvxZoomItem& rOrig )
:   SfxUInt16Item( rOrig.Which(), rOrig.GetValue() ),
    nValueSet( rOrig.GetValueSet() ),
    eType( rOrig.GetType() )
{
}

// -----------------------------------------------------------------------

SvxZoomItem::~SvxZoomItem()
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxZoomItem::Clone( SfxItemPool * /*pPool*/ ) const
{
    return new SvxZoomItem( *this );
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxZoomItem::Create( SvStream& rStrm, sal_uInt16 /*nVersion*/ ) const
{
    sal_uInt16 nValue;
    sal_uInt16 nValSet;
    sal_Int8 nType;
    rStrm >> nValue >> nValSet >> nType;
    SvxZoomItem* pNew = new SvxZoomItem( (SvxZoomType)nType, nValue, Which() );
    pNew->SetValueSet( nValSet );
    return pNew;
}

// -----------------------------------------------------------------------

SvStream& SvxZoomItem::Store( SvStream& rStrm, sal_uInt16 /*nItemVersion*/ ) const
{
    rStrm << (sal_uInt16)GetValue()
          << nValueSet
          << (sal_Int8)eType;
    return rStrm;
}

// -----------------------------------------------------------------------

int SvxZoomItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    SvxZoomItem& rItem = (SvxZoomItem&)rAttr;

    return ( GetValue() == rItem.GetValue()     &&
             nValueSet  == rItem.GetValueSet()  &&
             eType      == rItem.GetType()          );
}

sal_Bool SvxZoomItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
//  sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case 0 :
        {
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aSeq( ZOOM_PARAMS );
            aSeq[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ZOOM_PARAM_VALUE ));
            aSeq[0].Value <<= sal_Int32( GetValue() );
            aSeq[1].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ZOOM_PARAM_VALUESET ));
            aSeq[1].Value <<= sal_Int16( nValueSet );
            aSeq[2].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ZOOM_PARAM_TYPE ));
            aSeq[2].Value <<= sal_Int16( eType );
            rVal <<= aSeq;
        }
        break;

        case MID_VALUE: rVal <<= (sal_Int32) GetValue(); break;
        case MID_VALUESET: rVal <<= (sal_Int16) nValueSet; break;
        case MID_TYPE: rVal <<= (sal_Int16) eType; break;
        default:
            DBG_ERROR("svx::SvxZoomItem::QueryValue(), Wrong MemberId!");
            return sal_False;
    }

    return sal_True;
}

sal_Bool SvxZoomItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId )
{
//  sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case 0 :
        {
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aSeq;
            if (( rVal >>= aSeq ) && ( aSeq.getLength() == ZOOM_PARAMS ))
            {
                sal_Int32 nValueTmp( 0 );
                sal_Int16 nValueSetTmp( 0 );
                sal_Int16 nTypeTmp( 0 );
                sal_Bool  bAllConverted( sal_True );
                sal_Int16 nConvertedCount( 0 );
                for ( sal_Int32 i = 0; i < aSeq.getLength(); i++ )
                {
                    if ( aSeq[i].Name.equalsAscii( ZOOM_PARAM_VALUE ))
                    {
                        bAllConverted &= ( aSeq[i].Value >>= nValueTmp );
                        ++nConvertedCount;
                    }
                    else if ( aSeq[i].Name.equalsAscii( ZOOM_PARAM_VALUESET ))
                    {
                        bAllConverted &= ( aSeq[i].Value >>= nValueSetTmp );
                        ++nConvertedCount;
                    }
                    else if ( aSeq[i].Name.equalsAscii( ZOOM_PARAM_TYPE ))
                    {
                        bAllConverted &= ( aSeq[i].Value >>= nTypeTmp );
                        ++nConvertedCount;
                    }
                }

                if ( bAllConverted && nConvertedCount == ZOOM_PARAMS )
                {
                    SetValue( (sal_uInt16)nValueTmp );
                    nValueSet = nValueSetTmp;
                    eType = SvxZoomType( nTypeTmp );
                    return sal_True;
                }
            }

            return sal_False;
        }

        case MID_VALUE:
        {
            sal_Int32 nVal = 0;
            if ( rVal >>= nVal )
            {
                SetValue( (sal_uInt16)nVal );
                return sal_True;
            }
            else
                return sal_False;
        }

        case MID_VALUESET:
        case MID_TYPE:
        {
            sal_Int16 nVal = sal_Int16();
            if ( rVal >>= nVal )
            {
                if ( nMemberId == MID_VALUESET )
                    nValueSet = (sal_Int16) nVal;
                else if ( nMemberId == MID_TYPE )
                    eType = SvxZoomType( (sal_Int16) nVal );
                return sal_True;
            }
            else
                return sal_False;
        }

        default:
            DBG_ERROR("svx::SvxZoomItem::PutValue(), Wrong MemberId!");
            return sal_False;
    }

    return sal_True;
}
