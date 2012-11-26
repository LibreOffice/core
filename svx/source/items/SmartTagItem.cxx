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

// include ---------------------------------------------------------------

#include <svx/SmartTagItem.hxx>

#ifndef _COM_SUN_STAR_SMARTTAGS_XSMARTTAGPROPERTIES_HPP_
#include <com/sun/star/container/XStringKeyMap.hpp>
#endif


//#include <svl/memberid.hrc>
//#include "svxids.hrc"
//#include "svxitems.hrc"

using namespace ::com::sun::star;

// class SvxFontItem -----------------------------------------------------
IMPL_POOLITEM_FACTORY(SvxSmartTagItem)

SvxSmartTagItem::SvxSmartTagItem() :
    SfxPoolItem( 0 ),
    maActionComponentsSequence( ),
    maActionIndicesSequence( ),
    maStringKeyMaps( ),
    mxRange( ),
    mxController( ),
    maLocale( ),
    maApplicationName( ),
    maRangeText( )
{
}

// -----------------------------------------------------------------------

SvxSmartTagItem::SvxSmartTagItem( const sal_uInt16 nId,
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

sal_Bool SvxSmartTagItem::QueryValue( uno::Any& /* rVal */, sal_uInt8 /* nMemberId */ ) const
{
    return sal_False;
}
// -----------------------------------------------------------------------
sal_Bool SvxSmartTagItem::PutValue( const uno::Any& /*rVal*/, sal_uInt8 /* nMemberId */)
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

SvStream& SvxSmartTagItem::Store( SvStream& rStream, sal_uInt16 /*nItemVersion*/ ) const
{
    return rStream;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxSmartTagItem::Create(SvStream& , sal_uInt16) const
{
    return 0;
}
