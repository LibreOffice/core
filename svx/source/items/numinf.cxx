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

#include <svx/numinf.hxx>
#include <utility>


SvxNumberInfoItem::SvxNumberInfoItem( SvNumberFormatter* pNumFormatter,
                                      const TypedWhichId<SvxNumberInfoItem> nId ) :
    SfxPoolItem     ( nId ),
    pFormatter      ( pNumFormatter ),
    eValueType      ( SvxNumberValueType::Undefined ),
    aStringVal      ( u""_ustr ),
    nDoubleVal      ( 0 )
{
}


SvxNumberInfoItem::SvxNumberInfoItem( SvNumberFormatter* pNumFormatter,
                                      OUString aVal, const TypedWhichId<SvxNumberInfoItem> nId ) :
    SfxPoolItem     ( nId ),
    pFormatter      ( pNumFormatter ),
    eValueType      ( SvxNumberValueType::String ),
    aStringVal      (std::move( aVal )),
    nDoubleVal      ( 0 )
{
}


SvxNumberInfoItem::SvxNumberInfoItem( SvNumberFormatter* pNumFormatter,
                                      const double& rVal, const TypedWhichId<SvxNumberInfoItem> nId ) :
    SfxPoolItem     ( nId ),
    pFormatter      ( pNumFormatter ),
    eValueType      ( SvxNumberValueType::Number ),
    aStringVal      ( u""_ustr ),
    nDoubleVal      ( rVal )
{
}


SvxNumberInfoItem::SvxNumberInfoItem( SvNumberFormatter* pNumFormatter,
                                      const double& rVal, OUString aValueStr,
                                      const TypedWhichId<SvxNumberInfoItem> nId ) :
    SfxPoolItem     ( nId ),
    pFormatter      ( pNumFormatter ),
    eValueType      ( SvxNumberValueType::Number ),
    aStringVal      (std::move( aValueStr )),
    nDoubleVal      ( rVal )
{
}


SvxNumberInfoItem::SvxNumberInfoItem( const SvxNumberInfoItem& rItem ) :
    SfxPoolItem  ( rItem ),
    pFormatter   ( rItem.pFormatter ),
    eValueType   ( rItem.eValueType ),
    aStringVal   ( rItem.aStringVal ),
    nDoubleVal   ( rItem.nDoubleVal ),
    mvDelFormats( rItem.mvDelFormats )
{
}


SvxNumberInfoItem::~SvxNumberInfoItem()
{
}


bool SvxNumberInfoItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper&
)   const
{
    rText.clear();
    return false;
}


bool SvxNumberInfoItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));

    const SvxNumberInfoItem& rOther = static_cast<const SvxNumberInfoItem&>(rItem);

    return mvDelFormats == rOther.mvDelFormats &&
           pFormatter == rOther.pFormatter &&
           eValueType == rOther.eValueType &&
           nDoubleVal == rOther.nDoubleVal &&
           aStringVal == rOther.aStringVal;
}

SvxNumberInfoItem* SvxNumberInfoItem::Clone( SfxItemPool * ) const
{
    return new SvxNumberInfoItem( *this );
}

void SvxNumberInfoItem::SetDelFormats( std::vector<sal_uInt32> && aData )
{
    mvDelFormats = std::move(aData);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
