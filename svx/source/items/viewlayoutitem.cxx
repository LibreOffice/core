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

#include <svx/viewlayoutitem.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

// -----------------------------------------------------------------------

#define VIEWLAYOUT_PARAM_COLUMNS    "Columns"
#define VIEWLAYOUT_PARAM_BOOKMODE   "BookMode"
#define VIEWLAYOUT_PARAMS           2

// -----------------------------------------------------------------------
IMPL_POOLITEM_FACTORY(SvxViewLayoutItem)

SvxViewLayoutItem::SvxViewLayoutItem
(
    sal_uInt16      nColumns,
    bool        bBookMode,
    sal_uInt16  _nWhich
)
:   SfxUInt16Item( _nWhich, nColumns ),
    mbBookMode( bBookMode )
{
}

// -----------------------------------------------------------------------

SvxViewLayoutItem::SvxViewLayoutItem( const SvxViewLayoutItem& rOrig )
:   SfxUInt16Item( rOrig.Which(), rOrig.GetValue() ),
    mbBookMode( rOrig.IsBookMode() )
{
}

// -----------------------------------------------------------------------

SvxViewLayoutItem::~SvxViewLayoutItem()
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxViewLayoutItem::Clone( SfxItemPool * /*pPool*/ ) const
{
    return new SvxViewLayoutItem( *this );
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxViewLayoutItem::Create( SvStream& /*rStrm*/, sal_uInt16 /*nVersion*/ ) const
{
/*   sal_uInt16 nValue;
    sal_uInt16 nValSet;
    sal_Int8 nType;
    rStrm >> nValue >> nValSet >> nType;
    SvxZoomItem* pNew = new SvxZoomItem( (SvxZoomType)nType, nValue, Which() );
    pNew->SetValueSet( nValSet );
    return pNew;*/
    return 0;
}

// -----------------------------------------------------------------------

SvStream& SvxViewLayoutItem::Store( SvStream& rStrm, sal_uInt16 /*nItemVersion*/ ) const
{
/*   rStrm << (sal_uInt16)GetValue()
          << nValueSet
          << (sal_Int8)eType;*/
    return rStrm;
}

// -----------------------------------------------------------------------

int SvxViewLayoutItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    SvxViewLayoutItem& rItem = (SvxViewLayoutItem&)rAttr;

    return ( GetValue() == rItem.GetValue()     &&
             mbBookMode == rItem.IsBookMode() );
}

sal_Bool SvxViewLayoutItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case 0 :
        {
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aSeq( VIEWLAYOUT_PARAMS );
            aSeq[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( VIEWLAYOUT_PARAM_COLUMNS ));
            aSeq[0].Value <<= sal_Int32( GetValue() );
            aSeq[1].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( VIEWLAYOUT_PARAM_BOOKMODE ));
            aSeq[1].Value <<= sal_Bool( mbBookMode );
            rVal <<= aSeq;
        }
        break;

        case MID_VIEWLAYOUT_COLUMNS : rVal <<= (sal_Int32) GetValue(); break;
        case MID_VIEWLAYOUT_BOOKMODE: rVal <<= (sal_Bool) mbBookMode; break;
        default:
            DBG_ERROR("svx::SvxViewLayoutItem::QueryValue(), Wrong MemberId!");
            return sal_False;
    }

    return sal_True;
}

sal_Bool SvxViewLayoutItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId )
{
//  sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case 0 :
        {
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aSeq;
            if (( rVal >>= aSeq ) && ( aSeq.getLength() == VIEWLAYOUT_PARAMS ))
            {
                sal_Int32 nColumns( 0 );
                sal_Bool  bBookMode = sal_False;
                sal_Bool  bAllConverted( sal_True );
                sal_Int16 nConvertedCount( 0 );
                for ( sal_Int32 i = 0; i < aSeq.getLength(); i++ )
                {
                    if ( aSeq[i].Name.equalsAscii( VIEWLAYOUT_PARAM_COLUMNS ))
                    {
                        bAllConverted &= ( aSeq[i].Value >>= nColumns );
                        ++nConvertedCount;
                    }
                    else if ( aSeq[i].Name.equalsAscii( VIEWLAYOUT_PARAM_BOOKMODE ))
                    {
                        bAllConverted &= ( aSeq[i].Value >>= bBookMode );
                        ++nConvertedCount;
                    }
                }

                if ( bAllConverted && nConvertedCount == VIEWLAYOUT_PARAMS )
                {
                    SetValue( (sal_uInt16)nColumns );
                    mbBookMode = bBookMode;
                    return sal_True;
                }
            }

            return sal_False;
        }

        case MID_VIEWLAYOUT_COLUMNS:
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

        case MID_VIEWLAYOUT_BOOKMODE:
        {
            sal_Bool bBookMode = sal_False;
            if ( rVal >>= bBookMode )
            {
                 mbBookMode = bBookMode;
                return sal_True;
            }
            else
                return sal_False;
        }

        default:
            DBG_ERROR("svx::SvxViewLayoutItem::PutValue(), Wrong MemberId!");
            return sal_False;
    }

    return sal_True;
}

