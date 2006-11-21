/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: zoomitem.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-21 17:11:50 $
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
#ifndef __SBX_SBXVARIABLE_HXX
#include <basic/sbxvar.hxx>
#endif

#include "zoomitem.hxx"

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

// -----------------------------------------------------------------------

TYPEINIT1_AUTOFACTORY(SvxZoomItem,SfxUInt16Item);

#define ZOOM_PARAM_VALUE    "Value"
#define ZOOM_PARAM_VALUESET "ValueSet"
#define ZOOM_PARAM_TYPE     "Type"
#define ZOOM_PARAMS         3

// -----------------------------------------------------------------------

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

sal_Bool SvxZoomItem::QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
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

sal_Bool SvxZoomItem::PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId )
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
                    SetValue( (UINT16)nValueTmp );
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
                SetValue( (UINT16)nVal );
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
