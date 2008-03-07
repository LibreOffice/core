/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: zoomslideritem.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 15:18:21 $
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

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

#include <svx/zoomslideritem.hxx>

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

// -----------------------------------------------------------------------

TYPEINIT1_FACTORY(SvxZoomSliderItem,SfxUInt16Item, new SvxZoomSliderItem);

#define ZOOMSLIDER_PARAM_CURRENTZOOM    "Columns"
#define ZOOMSLIDER_PARAM_SNAPPINGPOINTS "SnappingPoints"
#define ZOOMSLIDER_PARAMS           2

// -----------------------------------------------------------------------

SvxZoomSliderItem::SvxZoomSliderItem( USHORT nCurrentZoom, USHORT nMinZoom, USHORT nMaxZoom, USHORT _nWhich )
:   SfxUInt16Item( _nWhich, nCurrentZoom ), mnMinZoom( nMinZoom ), mnMaxZoom( nMaxZoom )
{
}

// -----------------------------------------------------------------------

SvxZoomSliderItem::SvxZoomSliderItem( const SvxZoomSliderItem& rOrig )
:   SfxUInt16Item( rOrig.Which(), rOrig.GetValue() ),
    maValues( rOrig.maValues )
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

    return ( GetValue() == rItem.GetValue()     &&
             maValues == rItem.maValues );
}

sal_Bool SvxZoomSliderItem::QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
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
            rVal <<= aSeq;
        }
        break;

        case MID_ZOOMSLIDER_CURRENTZOOM : rVal <<= (sal_Int32) GetValue(); break;
        case MID_ZOOMSLIDER_SNAPPINGPOINTS:
            {
                 rVal <<= maValues; break;
            }
        default:
            DBG_ERROR("svx::SvxZoomSliderItem::QueryValue(), Wrong MemberId!");
            return sal_False;
    }

    return sal_True;
}

sal_Bool SvxZoomSliderItem::PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId )
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
                }

                if ( bAllConverted && nConvertedCount == ZOOMSLIDER_PARAMS )
                {
                    SetValue( (UINT16)nCurrentZoom );
                    maValues = aValues;
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
                SetValue( (UINT16)nVal );
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

