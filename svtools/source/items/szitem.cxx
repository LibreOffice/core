/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: szitem.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 15:01:59 $
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
#include "precompiled_svtools.hxx"

#include "szitem.hxx"

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif

#include "poolitem.hxx"
#include "memberid.hrc"

// STATIC DATA -----------------------------------------------------------

DBG_NAME(SfxSizeItem)

// -----------------------------------------------------------------------

TYPEINIT1_AUTOFACTORY(SfxSizeItem, SfxPoolItem);

// -----------------------------------------------------------------------

SfxSizeItem::SfxSizeItem()
{
    DBG_CTOR(SfxSizeItem, 0);
}

// -----------------------------------------------------------------------

SfxSizeItem::SfxSizeItem( USHORT nW, const Size& rVal ) :
    SfxPoolItem( nW ),
    aVal( rVal )
{
    DBG_CTOR(SfxSizeItem, 0);
}

// -----------------------------------------------------------------------

SfxSizeItem::SfxSizeItem( USHORT nW, SvStream &rStream ) :
    SfxPoolItem( nW )
{
    DBG_CTOR(SfxSizeItem, 0);
    rStream >> aVal;
}

// -----------------------------------------------------------------------

SfxSizeItem::SfxSizeItem( const SfxSizeItem& rItem ) :
    SfxPoolItem( rItem ),
    aVal( rItem.aVal )
{
    DBG_CTOR(SfxSizeItem, 0);
}

// -----------------------------------------------------------------------

SfxItemPresentation SfxSizeItem::GetPresentation
(
    SfxItemPresentation     /*ePresentation*/,
    SfxMapUnit              /*eCoreMetric*/,
    SfxMapUnit              /*ePresentationMetric*/,
    XubString&              rText,
    const IntlWrapper *
)   const
{
    DBG_CHKTHIS(SfxSizeItem, 0);
    rText = UniString::CreateFromInt32(aVal.Width());
    rText.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
    rText += UniString::CreateFromInt32(aVal.Height());
    rText.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

// -----------------------------------------------------------------------

int SfxSizeItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_CHKTHIS(SfxSizeItem, 0);
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
    return ((SfxSizeItem&)rItem).aVal == aVal;
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxSizeItem::Clone(SfxItemPool *) const
{
    DBG_CHKTHIS(SfxSizeItem, 0);
    return new SfxSizeItem( *this );
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxSizeItem::Create(SvStream &rStream, USHORT ) const
{
    DBG_CHKTHIS(SfxSizeItem, 0);
    Size aStr;
    rStream >> aStr;
    return new SfxSizeItem(Which(), aStr);
}

// -----------------------------------------------------------------------

SvStream& SfxSizeItem::Store(SvStream &rStream, USHORT ) const
{
    DBG_CHKTHIS(SfxSizeItem, 0);
    rStream << aVal;
    return rStream;
}

// -----------------------------------------------------------------------
BOOL  SfxSizeItem::QueryValue( com::sun::star::uno::Any& rVal,
                               BYTE nMemberId ) const
{
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;

    Size aTmp(aVal);
    if( bConvert )
    {
        aTmp.Height() = ( aTmp.Height() * 127 + 36) / 72;
        aTmp.Width() = ( aTmp.Width() * 127 + 36) / 72;
    }

    switch ( nMemberId )
    {
        case 0:
        {
            rVal <<= com::sun::star::awt::Size( aTmp.getWidth(), aTmp.getHeight() );
            break;
        }
        case MID_WIDTH:
            rVal <<= aTmp.getWidth(); break;
        case MID_HEIGHT:
            rVal <<= aTmp.getHeight(); break;
        default: DBG_ERROR("Wrong MemberId!"); return FALSE;
    }

    return TRUE;
}

// -----------------------------------------------------------------------
BOOL SfxSizeItem::PutValue( const com::sun::star::uno::Any& rVal,
                            BYTE nMemberId )
{
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;

    BOOL bRet = FALSE;
    com::sun::star::awt::Size aValue;
    sal_Int32 nVal;
    if ( !nMemberId )
        bRet = ( rVal >>= aValue );
    else
    {
        bRet = ( rVal >>= nVal );
        if ( nMemberId == MID_WIDTH )
        {
            aValue.Width = nVal;
            aValue.Height = aVal.Height();
        }
        else
        {
            aValue.Height = nVal;
            aValue.Width = aVal.Width();
        }
    }

    if ( bRet )
    {
        Size aTmp( aValue.Width, aValue.Height );
        if( bConvert )
        {
            aTmp.Height() = ( aTmp.Height() * 72 + 63) / 127;
            aTmp.Width() = ( aTmp.Width() * 72 + 63) / 127;
        }

        aVal = aTmp;
    }

    return bRet;
}



