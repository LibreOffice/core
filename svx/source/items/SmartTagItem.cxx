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

#include <comphelper/propertysequence.hxx>
#include <svx/SmartTagItem.hxx>
#include <sal/log.hxx>

#include <com/sun/star/container/XStringKeyMap.hpp>
#include <utility>


using namespace ::com::sun::star;


SfxPoolItem* SvxSmartTagItem::CreateDefault() { SAL_WARN( "svx", "No SvxSmartTagItem factory available"); return nullptr; }

SvxSmartTagItem::SvxSmartTagItem( const TypedWhichId<SvxSmartTagItem> nId,
                                  const css::uno::Sequence < css::uno::Sequence< css::uno::Reference< css::smarttags::XSmartTagAction > > >& rActionComponentsSequence,
                                  const css::uno::Sequence < css::uno::Sequence< sal_Int32 > >& rActionIndicesSequence,
                                  const css::uno::Sequence< css::uno::Reference< css::container::XStringKeyMap > >& rStringKeyMaps,
                                  css::uno::Reference<css::text::XTextRange> xRange,
                                  css::uno::Reference<css::frame::XController> xController,
                                  css::lang::Locale aLocale,
                                  OUString aApplicationName,
                                  OUString aRangeText ) :
    SfxPoolItem( nId ),
    maActionComponentsSequence( rActionComponentsSequence ),
    maActionIndicesSequence( rActionIndicesSequence ),
    maStringKeyMaps( rStringKeyMaps ),
    mxRange(std::move( xRange )),
    mxController(std::move( xController )),
    maLocale(std::move( aLocale )),
    maApplicationName(std::move( aApplicationName )),
    maRangeText(std::move( aRangeText ))
{
}


bool SvxSmartTagItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    rVal <<= comphelper::InitPropertySequence( {
        { "ActionComponents", css::uno::Any( maActionComponentsSequence ) },
        { "ActionIndices", css::uno::Any( maActionIndicesSequence ) },
        { "StringKeyMaps", css::uno::Any( maStringKeyMaps ) },
        { "TextRange", css::uno::Any( mxRange ) },
        { "Controller", css::uno::Any( mxController ) },
        { "Locale", css::uno::Any( maLocale ) },
        { "ApplicationName", css::uno::Any( maApplicationName ) },
        { "RangeText", css::uno::Any( maRangeText ) },
    } );
    return true;
}

bool SvxSmartTagItem::PutValue( const uno::Any& /*rVal*/, sal_uInt8 /* nMemberId */)
{
    return false;
}


bool SvxSmartTagItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));

    const SvxSmartTagItem& rItem = static_cast<const SvxSmartTagItem&>(rAttr);

    return maActionComponentsSequence == rItem.maActionComponentsSequence &&
               maActionIndicesSequence == rItem.maActionIndicesSequence &&
               maStringKeyMaps == rItem.maStringKeyMaps &&
               mxRange == rItem.mxRange &&
               mxController == rItem.mxController &&
               maApplicationName == rItem.maApplicationName &&
               maRangeText == rItem.maRangeText;
}


SvxSmartTagItem* SvxSmartTagItem::Clone( SfxItemPool * ) const
{
    return new SvxSmartTagItem( *this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
