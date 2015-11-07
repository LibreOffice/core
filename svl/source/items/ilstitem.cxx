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
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <comphelper/processfactory.hxx>

#include <svl/ilstitem.hxx>


SfxPoolItem* SfxIntegerListItem::CreateDefault() { return new SfxIntegerListItem; }

SfxIntegerListItem::SfxIntegerListItem()
{
}

SfxIntegerListItem::SfxIntegerListItem( sal_uInt16 which, const ::std::vector < sal_Int32 >& rList )
    : SfxPoolItem( which )
{
    m_aList.realloc( rList.size() );
    for ( size_t n=0; n<rList.size(); ++n )
        m_aList[n] = rList[n];
}

SfxIntegerListItem::SfxIntegerListItem( sal_uInt16 which, const css::uno::Sequence < sal_Int32 >& rList )
    : SfxPoolItem( which )
{
    m_aList.realloc( rList.getLength() );
    for ( sal_Int32 n=0; n<rList.getLength(); ++n )
        m_aList[n] = rList[n];
}

SfxIntegerListItem::SfxIntegerListItem( const SfxIntegerListItem& rItem )
    : SfxPoolItem( rItem )
{
    m_aList = rItem.m_aList;
}

SfxIntegerListItem::~SfxIntegerListItem()
{
}

bool SfxIntegerListItem::operator==( const SfxPoolItem& rPoolItem ) const
{
    if ( dynamic_cast< const SfxIntegerListItem* >( &rPoolItem) ==  nullptr )
        return false;

    const SfxIntegerListItem rItem = static_cast<const SfxIntegerListItem&>(rPoolItem);
    return rItem.m_aList == m_aList;
}

SfxPoolItem* SfxIntegerListItem::Clone( SfxItemPool * ) const
{
    return new SfxIntegerListItem( *this );
}

bool SfxIntegerListItem::PutValue  ( const css::uno::Any& rVal, sal_uInt8 )
{
    css::uno::Reference < css::script::XTypeConverter > xConverter
            ( css::script::Converter::create(::comphelper::getProcessComponentContext()) );
    css::uno::Any aNew;
    try { aNew = xConverter->convertTo( rVal, cppu::UnoType<css::uno::Sequence < sal_Int32 >>::get() ); }
    catch (css::uno::Exception&)
    {
        return true;
    }

    return ( aNew >>= m_aList );
}

bool SfxIntegerListItem::QueryValue( css::uno::Any& rVal, sal_uInt8 ) const
{
    rVal <<= m_aList;
    return true;
}

void SfxIntegerListItem::GetList( ::std::vector< sal_Int32 >& rList ) const
{
    rList.reserve( m_aList.getLength() );
    for ( sal_Int32 n=0; n<m_aList.getLength(); ++n )
        rList.push_back( m_aList[n] );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
