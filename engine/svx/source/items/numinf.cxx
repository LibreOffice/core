/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
    m_pFormatter      ( pNumFormatter ),
    m_eValueType      ( SvxNumberValueType::Undefined ),
    m_aStringVal      ( u""_ustr ),
    m_nDoubleVal      ( 0 )
{
}


SvxNumberInfoItem::SvxNumberInfoItem( SvNumberFormatter* pNumFormatter,
                                      OUString aVal, const TypedWhichId<SvxNumberInfoItem> nId ) :
    SfxPoolItem     ( nId ),
    m_pFormatter      ( pNumFormatter ),
    m_eValueType      ( SvxNumberValueType::String ),
    m_aStringVal      (std::move( aVal )),
    m_nDoubleVal      ( 0 )
{
}


SvxNumberInfoItem::SvxNumberInfoItem( SvNumberFormatter* pNumFormatter,
                                      const double& rVal, const TypedWhichId<SvxNumberInfoItem> nId ) :
    SfxPoolItem     ( nId ),
    m_pFormatter      ( pNumFormatter ),
    m_eValueType      ( SvxNumberValueType::Number ),
    m_aStringVal      ( u""_ustr ),
    m_nDoubleVal      ( rVal )
{
}


SvxNumberInfoItem::SvxNumberInfoItem( SvNumberFormatter* pNumFormatter,
                                      const double& rVal, OUString aValueStr,
                                      const TypedWhichId<SvxNumberInfoItem> nId ) :
    SfxPoolItem     ( nId ),
    m_pFormatter      ( pNumFormatter ),
    m_eValueType      ( SvxNumberValueType::Number ),
    m_aStringVal      (std::move( aValueStr )),
    m_nDoubleVal      ( rVal )
{
}


SvxNumberInfoItem::SvxNumberInfoItem( const SvxNumberInfoItem& rItem ) :
    SfxPoolItem  ( rItem ),
    m_pFormatter   ( rItem.m_pFormatter ),
    m_eValueType   ( rItem.m_eValueType ),
    m_aStringVal   ( rItem.m_aStringVal ),
    m_nDoubleVal   ( rItem.m_nDoubleVal ),
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
           m_pFormatter == rOther.m_pFormatter &&
           m_eValueType == rOther.m_eValueType &&
           m_nDoubleVal == rOther.m_nDoubleVal &&
           m_aStringVal == rOther.m_aStringVal;
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
