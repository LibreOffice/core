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
#include <osl/diagnose.h>
#include <tools/stream.hxx>
#include <svl/cintitem.hxx>


// virtual
bool CntByteItem::operator ==(const SfxPoolItem & rItem) const
{
    DBG_ASSERT(dynamic_cast<const CntByteItem*>( &rItem ) !=  nullptr,
               "CntByteItem::operator ==(): Bad type");
    return m_nValue == (static_cast< const CntByteItem * >(&rItem))->m_nValue;
}

// virtual
bool CntByteItem::GetPresentation(SfxItemPresentation,
                                                 SfxMapUnit, SfxMapUnit,
                                                 OUString & rText,
                                                 const IntlWrapper *) const
{
    rText = OUString::number( m_nValue );
    return true;
}

// virtual
bool CntByteItem::QueryValue(css::uno::Any& rVal, sal_uInt8) const
{
    sal_Int8 nValue = m_nValue;
    rVal <<= nValue;
    return true;
}

// virtual
bool CntByteItem::PutValue(const css::uno::Any& rVal, sal_uInt8)
{
    sal_Int8 nValue = sal_Int8();
    if (rVal >>= nValue)
    {
        m_nValue = nValue;
        return true;
    }

    OSL_FAIL( "CntByteItem::PutValue - Wrong type!" );
    return false;
}

// virtual
SfxPoolItem * CntByteItem::Create(SvStream & rStream, sal_uInt16) const
{
    short nTheValue = 0;
    rStream.ReadInt16( nTheValue );
    return new CntByteItem(Which(), sal_uInt8(nTheValue));
}

// virtual
SvStream & CntByteItem::Store(SvStream & rStream, sal_uInt16) const
{
    rStream.WriteInt16( short(m_nValue) );
    return rStream;
}

// virtual
SfxPoolItem * CntByteItem::Clone(SfxItemPool *) const
{
    return new CntByteItem(*this);
}


CntUInt16Item::CntUInt16Item(sal_uInt16 which, SvStream & rStream) :
    SfxPoolItem(which)
{
    sal_uInt16 nTheValue = 0;
    rStream.ReadUInt16( nTheValue );
    m_nValue = nTheValue;
}

// virtual
bool CntUInt16Item::operator ==(const SfxPoolItem & rItem) const
{
    DBG_ASSERT(dynamic_cast< const CntUInt16Item* >(&rItem) !=  nullptr,
               "CntUInt16Item::operator ==(): Bad type");
    return m_nValue == (static_cast< const CntUInt16Item * >(&rItem))->
                        m_nValue;
}

// virtual
bool CntUInt16Item::GetPresentation(SfxItemPresentation,
                                                   SfxMapUnit, SfxMapUnit,
                                                   OUString & rText,
                                                   const IntlWrapper *)
    const
{
    rText = OUString::number( m_nValue );
    return true;
}

// virtual
bool CntUInt16Item::QueryValue(css::uno::Any& rVal, sal_uInt8) const
{
    sal_Int32 nValue = m_nValue;
    rVal <<= nValue;
    return true;
}

// virtual
bool CntUInt16Item::PutValue(const css::uno::Any& rVal, sal_uInt8)
{
    sal_Int32 nValue = 0;
    if (rVal >>= nValue)
    {
        DBG_ASSERT( nValue <= USHRT_MAX, "Overflow in UInt16 value!");
        m_nValue = (sal_uInt16)nValue;
        return true;
    }

    OSL_FAIL( "CntUInt16Item::PutValue - Wrong type!" );
    return false;
}

// virtual
SfxPoolItem * CntUInt16Item::Create(SvStream & rStream, sal_uInt16) const
{
    return new CntUInt16Item(Which(), rStream);
}

// virtual
SvStream & CntUInt16Item::Store(SvStream &rStream, sal_uInt16) const
{
    rStream.WriteUInt16( m_nValue );
    return rStream;
}

// virtual
SfxPoolItem * CntUInt16Item::Clone(SfxItemPool *) const
{
    return new CntUInt16Item(*this);
}


CntInt32Item::CntInt32Item(sal_uInt16 which, SvStream & rStream)
    : SfxPoolItem(which)
    , m_nValue(0)
{
    rStream.ReadInt32( m_nValue );
}

// virtual
bool CntInt32Item::operator ==(const SfxPoolItem & rItem) const
{
    DBG_ASSERT(dynamic_cast< const CntInt32Item*>(&rItem) !=  nullptr,
               "CntInt32Item::operator ==(): Bad type");
    return m_nValue == (static_cast< const CntInt32Item * >(&rItem))->
                        m_nValue;
}

// virtual
bool CntInt32Item::GetPresentation(SfxItemPresentation,
                                                  SfxMapUnit, SfxMapUnit,
                                                  OUString & rText,
                                                  const IntlWrapper *) const
{
    rText = OUString::number( m_nValue );
    return true;
}

// virtual
bool CntInt32Item::QueryValue(css::uno::Any& rVal, sal_uInt8) const
{
    sal_Int32 nValue = m_nValue;
    rVal <<= nValue;
    return true;
}

// virtual
bool CntInt32Item::PutValue(const css::uno::Any& rVal, sal_uInt8)
{
    sal_Int32 nValue = 0;
    if (rVal >>= nValue)
    {
        m_nValue = nValue;
        return true;
    }

    OSL_FAIL( "CntInt32Item::PutValue - Wrong type!" );
    return false;
}

// virtual
SfxPoolItem * CntInt32Item::Create(SvStream & rStream, sal_uInt16) const
{
    return new CntInt32Item(Which(), rStream);
}

// virtual
SvStream & CntInt32Item::Store(SvStream &rStream, sal_uInt16) const
{
    rStream.WriteInt32( m_nValue );
    return rStream;
}

// virtual
SfxPoolItem * CntInt32Item::Clone(SfxItemPool *) const
{
    return new CntInt32Item(*this);
}


CntUInt32Item::CntUInt32Item(sal_uInt16 which, SvStream & rStream) :
    SfxPoolItem(which)
{
    sal_uInt32 nTheValue = 0;
    rStream.ReadUInt32( nTheValue );
    m_nValue = nTheValue;
}

// virtual
bool CntUInt32Item::operator ==(const SfxPoolItem & rItem) const
{
    DBG_ASSERT(dynamic_cast< const CntUInt32Item* >(&rItem) !=  nullptr,
               "CntUInt32Item::operator ==(): Bad type");
    return m_nValue == (static_cast< const CntUInt32Item * >(&rItem))->
                        m_nValue;
}

// virtual
bool CntUInt32Item::GetPresentation(SfxItemPresentation,
                                                   SfxMapUnit, SfxMapUnit,
                                                   OUString & rText,
                                                   const IntlWrapper *)
    const
{
    rText = OUString::number(m_nValue);
    return true;
}

// virtual
bool CntUInt32Item::QueryValue(css::uno::Any& rVal, sal_uInt8) const
{
    sal_Int32 nValue = m_nValue;
    DBG_ASSERT( nValue>=0, "Overflow in UInt32 value!");
    rVal <<= nValue;
    return true;
}

// virtual
bool CntUInt32Item::PutValue(const css::uno::Any& rVal, sal_uInt8)
{
    sal_Int32 nValue = 0;
    if (rVal >>= nValue)
    {
        DBG_ASSERT( nValue>=0, "Overflow in UInt32 value!");
        m_nValue = nValue;
        return true;
    }

    OSL_FAIL( "CntUInt32Item::PutValue - Wrong type!" );
    return false;
}

// virtual
SfxPoolItem * CntUInt32Item::Create(SvStream & rStream, sal_uInt16) const
{
    return new CntUInt32Item(Which(), rStream);
}

// virtual
SvStream & CntUInt32Item::Store(SvStream &rStream, sal_uInt16) const
{
    rStream.WriteUInt32( m_nValue );
    return rStream;
}

// virtual
SfxPoolItem * CntUInt32Item::Clone(SfxItemPool *) const
{
    return new CntUInt32Item(*this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
