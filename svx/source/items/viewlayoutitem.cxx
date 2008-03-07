/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewlayoutitem.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 15:17:58 $
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

#include <svx/viewlayoutitem.hxx>

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

// -----------------------------------------------------------------------

TYPEINIT1_FACTORY(SvxViewLayoutItem,SfxUInt16Item, new SvxViewLayoutItem);

#define VIEWLAYOUT_PARAM_COLUMNS    "Columns"
#define VIEWLAYOUT_PARAM_BOOKMODE   "BookMode"
#define VIEWLAYOUT_PARAMS           2

// -----------------------------------------------------------------------

SvxViewLayoutItem::SvxViewLayoutItem
(
    USHORT      nColumns,
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

sal_Bool SvxViewLayoutItem::QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
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

sal_Bool SvxViewLayoutItem::PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId )
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
                    SetValue( (UINT16)nColumns );
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
                SetValue( (UINT16)nVal );
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

