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
#include "precompiled_svx.hxx"
#include <tools/stream.hxx>

#include <svx/zoomslideritem.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

// -----------------------------------------------------------------------

TYPEINIT1_FACTORY(SvxZoomSliderItem,SfxUInt16Item, new SvxZoomSliderItem);

#define ZOOMSLIDER_PARAM_CURRENTZOOM    "Columns"
#define ZOOMSLIDER_PARAM_SNAPPINGPOINTS "SnappingPoints"
#define ZOOMSLIDER_PARAM_MINZOOM        "MinValue"
#define ZOOMSLIDER_PARAM_MAXZOOM        "MaxValue"
#define ZOOMSLIDER_PARAMS           4

// -----------------------------------------------------------------------

SvxZoomSliderItem::SvxZoomSliderItem( sal_uInt16 nCurrentZoom, sal_uInt16 nMinZoom, sal_uInt16 nMaxZoom, sal_uInt16 _nWhich )
:   SfxUInt16Item( _nWhich, nCurrentZoom ), mnMinZoom( nMinZoom ), mnMaxZoom( nMaxZoom )
{
}

// -----------------------------------------------------------------------

SvxZoomSliderItem::SvxZoomSliderItem( const SvxZoomSliderItem& rOrig )
: SfxUInt16Item( rOrig.Which(), rOrig.GetValue() )
, maValues( rOrig.maValues )
, mnMinZoom( rOrig.mnMinZoom )
, mnMaxZoom( rOrig.mnMaxZoom )
{
}

// -----------------------------------------------------------------------

SvxZoomSliderItem::~SvxZoomSliderItem()
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxZoomSliderItem::Clone( SfxItemPool * /*pPool*/ ) const
{
    return new SvxZoomSliderItem( *this );
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxZoomSliderItem::Create( SvStream& /*rStrm*/, sal_uInt16 /*nVersion*/ ) const
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

SvStream& SvxZoomSliderItem::Store( SvStream& rStrm, sal_uInt16 /*nItemVersion*/ ) const
{
/*   rStrm << (sal_uInt16)GetValue()
          << nValueSet
          << (sal_Int8)eType;*/
    return rStrm;
}

// -----------------------------------------------------------------------

int SvxZoomSliderItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    SvxZoomSliderItem& rItem = (SvxZoomSliderItem&)rAttr;

    return ( GetValue() == rItem.GetValue() && maValues == rItem.maValues &&
             mnMinZoom == rItem.mnMinZoom && mnMaxZoom == rItem.mnMaxZoom );
}

sal_Bool SvxZoomSliderItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case 0 :
            {
                ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aSeq( ZOOMSLIDER_PARAMS );
                aSeq[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ZOOMSLIDER_PARAM_CURRENTZOOM ));
                aSeq[0].Value <<= sal_Int32( GetValue() );
                aSeq[1].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ZOOMSLIDER_PARAM_SNAPPINGPOINTS ));
                aSeq[1].Value <<= maValues;
                aSeq[2].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ZOOMSLIDER_PARAM_MINZOOM ) );
                aSeq[2].Value <<= mnMinZoom;
                aSeq[3].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ZOOMSLIDER_PARAM_MAXZOOM ) );
                aSeq[3].Value <<= mnMaxZoom;
                rVal <<= aSeq;
            }
            break;

        case MID_ZOOMSLIDER_CURRENTZOOM :
            {
                rVal <<= (sal_Int32) GetValue();
            }
            break;
        case MID_ZOOMSLIDER_SNAPPINGPOINTS:
            {
                 rVal <<= maValues;
            }
            break;
        case MID_ZOOMSLIDER_MINZOOM:
            {
                rVal <<= mnMinZoom;
            }
            break;
        case MID_ZOOMSLIDER_MAXZOOM:
            {
                rVal <<= mnMaxZoom;
            }
            break;
        default:
            DBG_ERROR("svx::SvxZoomSliderItem::QueryValue(), Wrong MemberId!");
            return sal_False;
    }

    return sal_True;
}

sal_Bool SvxZoomSliderItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId )
{
//  sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case 0 :
            {
                ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aSeq;
                if (( rVal >>= aSeq ) && ( aSeq.getLength() == ZOOMSLIDER_PARAMS ))
                {
                    sal_Int32 nCurrentZoom( 0 );
                    com::sun::star::uno::Sequence < sal_Int32 > aValues;

                    sal_Bool  bAllConverted( sal_True );
                    sal_Int16 nConvertedCount( 0 );
                    sal_Int32 nMinZoom( 0 ), nMaxZoom( 0 );

                    for ( sal_Int32 i = 0; i < aSeq.getLength(); i++ )
                    {
                        if ( aSeq[i].Name.equalsAscii( ZOOMSLIDER_PARAM_CURRENTZOOM ))
                        {
                            bAllConverted &= ( aSeq[i].Value >>= nCurrentZoom );
                            ++nConvertedCount;
                        }
                        else if ( aSeq[i].Name.equalsAscii( ZOOMSLIDER_PARAM_SNAPPINGPOINTS ))
                        {
                            bAllConverted &= ( aSeq[i].Value >>= aValues );
                            ++nConvertedCount;
                        }
                        else if( aSeq[i].Name.equalsAscii( ZOOMSLIDER_PARAM_MINZOOM ) )
                        {
                            bAllConverted &= ( aSeq[i].Value >>= nMinZoom );
                            ++nConvertedCount;
                        }
                        else if( aSeq[i].Name.equalsAscii( ZOOMSLIDER_PARAM_MAXZOOM ) )
                        {
                            bAllConverted &= ( aSeq[i].Value >>= nMaxZoom );
                            ++nConvertedCount;
                        }
                    }

                    if ( bAllConverted && nConvertedCount == ZOOMSLIDER_PARAMS )
                    {
                        SetValue( (sal_uInt16)nCurrentZoom );
                        maValues = aValues;
                        mnMinZoom = sal::static_int_cast< sal_uInt16 >( nMinZoom );
                        mnMaxZoom = sal::static_int_cast< sal_uInt16 >( nMaxZoom );

                        return sal_True;
                    }
                }

                return sal_False;
            }

        case MID_ZOOMSLIDER_CURRENTZOOM:
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

        case MID_ZOOMSLIDER_SNAPPINGPOINTS:
            {
                com::sun::star::uno::Sequence < sal_Int32 > aValues;
                if ( rVal >>= aValues )
                {
                    maValues = aValues;
                    return sal_True;
                }
                else
                    return sal_False;
            }
        case MID_ZOOMSLIDER_MINZOOM:
            {
                sal_Int32 nVal = 0;
                if( rVal >>= nVal )
                {
                    mnMinZoom = (sal_uInt16)nVal;
                    return sal_True;
                }
                else
                    return sal_False;
            }
        case MID_ZOOMSLIDER_MAXZOOM:
            {
                sal_Int32 nVal = 0;
                if( rVal >>= nVal )
                {
                    mnMaxZoom = (sal_uInt16)nVal;
                    return sal_True;
                }
                else
                    return sal_False;
            }
        default:
            DBG_ERROR("svx::SvxZoomSliderItem::PutValue(), Wrong MemberId!");
            return sal_False;
    }

    return sal_True;
}

void SvxZoomSliderItem::AddSnappingPoint( sal_Int32 nNew )
{
    const sal_Int32 nValues = maValues.getLength();
    maValues.realloc(  nValues + 1 );
    sal_Int32* pValues = maValues.getArray();
    pValues[ nValues ] = nNew;
}

const com::sun::star::uno::Sequence < sal_Int32 >& SvxZoomSliderItem::GetSnappingPoints() const
{
    return maValues;
}

