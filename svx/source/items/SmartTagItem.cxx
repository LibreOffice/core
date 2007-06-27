/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SmartTagItem.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 13:40:45 $
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

// include ---------------------------------------------------------------

#include "SmartTagItem.hxx"

#ifndef _COM_SUN_STAR_SMARTTAGS_XSMARTTAGPROPERTIES_HPP_
#include <com/sun/star/container/XStringKeyMap.hpp>
#endif


//#include <svtools/memberid.hrc>
//#include "svxids.hrc"
//#include "svxitems.hrc"

using namespace ::com::sun::star;

TYPEINIT1(SvxSmartTagItem, SfxPoolItem);

// class SvxFontItem -----------------------------------------------------

SvxSmartTagItem::SvxSmartTagItem( const USHORT nId,
                                  const com::sun::star::uno::Sequence < com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::smarttags::XSmartTagAction > > >& rActionComponentsSequence,
                                  const com::sun::star::uno::Sequence < com::sun::star::uno::Sequence< sal_Int32 > >& rActionIndicesSequence,
                                  const com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::container::XStringKeyMap > >& rStringKeyMaps,
                                  const com::sun::star::uno::Reference<com::sun::star::text::XTextRange> rRange,
                                  const com::sun::star::uno::Reference<com::sun::star::frame::XController> rController,
                                  const com::sun::star::lang::Locale rLocale,
                                  const rtl::OUString& rApplicationName,
                                  const rtl::OUString& rRangeText ) :
    SfxPoolItem( nId ),
    maActionComponentsSequence( rActionComponentsSequence ),
    maActionIndicesSequence( rActionIndicesSequence ),
    maStringKeyMaps( rStringKeyMaps ),
    mxRange( rRange ),
    mxController( rController ),
    maLocale( rLocale ),
    maApplicationName( rApplicationName ),
    maRangeText( rRangeText )
{
}

// -----------------------------------------------------------------------

// -----------------------------------------------------------------------

sal_Bool SvxSmartTagItem::QueryValue( uno::Any& /* rVal */, BYTE /* nMemberId */ ) const
{
    return sal_False;
}
// -----------------------------------------------------------------------
sal_Bool SvxSmartTagItem::PutValue( const uno::Any& /*rVal*/, BYTE /* nMemberId */)
{
    return sal_False;
}

// -----------------------------------------------------------------------

int SvxSmartTagItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    const SvxSmartTagItem& rItem = static_cast<const SvxSmartTagItem&>(rAttr);

    int bRet = maActionComponentsSequence == rItem.maActionComponentsSequence &&
               maActionIndicesSequence == rItem.maActionIndicesSequence &&
               maStringKeyMaps == rItem.maStringKeyMaps &&
               mxRange == rItem.mxRange &&
               mxController == rItem.mxController &&
               maApplicationName == rItem.maApplicationName &&
               maRangeText == rItem.maRangeText;

    return bRet;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxSmartTagItem::Clone( SfxItemPool * ) const
{
    return new SvxSmartTagItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxSmartTagItem::Store( SvStream& rStream, USHORT /*nItemVersion*/ ) const
{
    return rStream;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxSmartTagItem::Create(SvStream& , USHORT) const
{
    return 0;
}
