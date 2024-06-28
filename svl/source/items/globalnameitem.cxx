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

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/script/Converter.hpp>

#include <osl/diagnose.h>

#include <comphelper/processfactory.hxx>

#include <svl/globalnameitem.hxx>
#include <o3tl/hash_combine.hxx>

SfxPoolItem* SfxGlobalNameItem::CreateDefault() { return new SfxGlobalNameItem; }


SfxGlobalNameItem::SfxGlobalNameItem() :
    SfxPoolItem(0, SfxItemType::SfxGlobalNameItemType)
{
}


SfxGlobalNameItem::SfxGlobalNameItem( sal_uInt16 nW, const SvGlobalName& rName )
:   SfxPoolItem( nW, SfxItemType::SfxGlobalNameItemType ),
    m_aName( rName )
{
}

SfxGlobalNameItem::~SfxGlobalNameItem()
{
}

bool SfxGlobalNameItem::isHashable() const
{
    return true;
}

size_t SfxGlobalNameItem::hashCode() const
{
    std::size_t seed = 0;
    o3tl::hash_combine(seed, m_aName.GetCLSID().Data1);
    o3tl::hash_combine(seed, m_aName.GetCLSID().Data2);
    o3tl::hash_combine(seed, m_aName.GetCLSID().Data3);
    return seed;
}

bool SfxGlobalNameItem::operator==( const SfxPoolItem& rItem ) const
{
    return SfxPoolItem::operator==(rItem) &&
        static_cast<const SfxGlobalNameItem&>(rItem).m_aName == m_aName;
}

SfxGlobalNameItem* SfxGlobalNameItem::Clone(SfxItemPool *) const
{
    return new SfxGlobalNameItem( *this );
}

// virtual
bool SfxGlobalNameItem::PutValue( const css::uno::Any& rVal, sal_uInt8 )
{
    ASSERT_CHANGE_REFCOUNTED_ITEM;
    css::uno::Reference < css::script::XTypeConverter > xConverter
            ( css::script::Converter::create( ::comphelper::getProcessComponentContext() ));
    css::uno::Sequence< sal_Int8 > aSeq;
    css::uno::Any aNew;

    try { aNew = xConverter->convertTo( rVal, cppu::UnoType<css::uno::Sequence < sal_Int8 >>::get() ); }
    catch (css::uno::Exception&) {}
    aNew >>= aSeq;
    if ( aSeq.getLength() == 16 )
    {
        m_aName.MakeFromMemory( aSeq.getConstArray() );
        return true;
    }

    OSL_FAIL( "SfxGlobalNameItem::PutValue - Wrong type!" );
    return true;
}

// virtual
bool SfxGlobalNameItem::QueryValue( css::uno::Any& rVal, sal_uInt8 ) const
{
    css::uno::Sequence< sal_Int8 > aSeq( 16 );
    void const * pData = &m_aName.GetCLSID();
    memcpy( aSeq.getArray(), pData, 16 );
    rVal <<= aSeq;
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
