/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <svx/SmartTagItem.hxx>

#include <com/sun/star/container/XStringKeyMap.hpp>


using namespace ::com::sun::star;


SfxPoolItem* SvxSmartTagItem::CreateDefault() { DBG_ASSERT(false, "No SvxSmartTagItem factory available"); return nullptr; }

SvxSmartTagItem::SvxSmartTagItem( const sal_uInt16 nId,
                                  const css::uno::Sequence < css::uno::Sequence< css::uno::Reference< css::smarttags::XSmartTagAction > > >& rActionComponentsSequence,
                                  const css::uno::Sequence < css::uno::Sequence< sal_Int32 > >& rActionIndicesSequence,
                                  const css::uno::Sequence< css::uno::Reference< css::container::XStringKeyMap > >& rStringKeyMaps,
                                  const css::uno::Reference<css::text::XTextRange>& rRange,
                                  const css::uno::Reference<css::frame::XController>& rController,
                                  const css::lang::Locale& rLocale,
                                  const OUString& rApplicationName,
                                  const OUString& rRangeText ) :
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





bool SvxSmartTagItem::QueryValue( uno::Any& /* rVal */, sal_uInt8 /* nMemberId */ ) const
{
    return false;
}

bool SvxSmartTagItem::PutValue( const uno::Any& /*rVal*/, sal_uInt8 /* nMemberId */)
{
    return false;
}


bool SvxSmartTagItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    const SvxSmartTagItem& rItem = static_cast<const SvxSmartTagItem&>(rAttr);

    return maActionComponentsSequence == rItem.maActionComponentsSequence &&
               maActionIndicesSequence == rItem.maActionIndicesSequence &&
               maStringKeyMaps == rItem.maStringKeyMaps &&
               mxRange == rItem.mxRange &&
               mxController == rItem.mxController &&
               maApplicationName == rItem.maApplicationName &&
               maRangeText == rItem.maRangeText;
}


SfxPoolItem* SvxSmartTagItem::Clone( SfxItemPool * ) const
{
    return new SvxSmartTagItem( *this );
}


SvStream& SvxSmartTagItem::Store( SvStream& rStream, sal_uInt16 /*nItemVersion*/ ) const
{
    return rStream;
}


SfxPoolItem* SvxSmartTagItem::Create(SvStream& , sal_uInt16) const
{
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
