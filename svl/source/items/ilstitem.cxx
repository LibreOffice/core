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

#include <com/sun/star/script/Converter.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>

#include <svl/ilstitem.hxx>
#include <o3tl/hash_combine.hxx>


SfxPoolItem* SfxIntegerListItem::CreateDefault() { return new SfxIntegerListItem; }

SfxIntegerListItem::SfxIntegerListItem()
    : SfxPoolItem(0, SfxItemType::SfxIntegerListItemType)
{
}

SfxIntegerListItem::SfxIntegerListItem( sal_uInt16 which, ::std::vector < sal_Int32 >&& rList )
    : SfxPoolItem( which, SfxItemType::SfxIntegerListItemType )
    , m_aList( std::move(rList) )
{
}

SfxIntegerListItem::SfxIntegerListItem( sal_uInt16 which, const css::uno::Sequence < sal_Int32 >& rList )
    : SfxPoolItem( which, SfxItemType::SfxIntegerListItemType )
{
    comphelper::sequenceToContainer(m_aList, rList);
}

SfxIntegerListItem::~SfxIntegerListItem()
{
}

bool SfxIntegerListItem::isHashable() const
{
    return true;
}

size_t SfxIntegerListItem::hashCode() const
{
    std::size_t seed = 0;
    for (auto const & i : m_aList)
        o3tl::hash_combine(seed, i);
    return seed;
}

bool SfxIntegerListItem::operator==( const SfxPoolItem& rPoolItem ) const
{
    if ( !SfxPoolItem::operator==(rPoolItem) )
        return false;

    const SfxIntegerListItem & rItem = static_cast<const SfxIntegerListItem&>(rPoolItem);
    return rItem.m_aList == m_aList;
}

SfxIntegerListItem* SfxIntegerListItem::Clone( SfxItemPool * ) const
{
    return new SfxIntegerListItem( *this );
}

bool SfxIntegerListItem::PutValue  ( const css::uno::Any& rVal, sal_uInt8 )
{
    ASSERT_CHANGE_REFCOUNTED_ITEM;
    css::uno::Reference < css::script::XTypeConverter > xConverter
            ( css::script::Converter::create(::comphelper::getProcessComponentContext()) );
    css::uno::Any aNew;
    try { aNew = xConverter->convertTo( rVal, cppu::UnoType<css::uno::Sequence < sal_Int32 >>::get() ); }
    catch (css::uno::Exception&)
    {
        return true;
    }

    css::uno::Sequence<sal_Int32> aTempSeq;
    bool bRet = aNew >>= aTempSeq;
    if (bRet)
        m_aList = comphelper::sequenceToContainer<std::vector<sal_Int32>>(aTempSeq);
    return bRet;
}

bool SfxIntegerListItem::QueryValue( css::uno::Any& rVal, sal_uInt8 ) const
{
    rVal <<= comphelper::containerToSequence(m_aList);
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
