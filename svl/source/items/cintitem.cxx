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
#include <svl/cintitem.hxx>
#include <sal/log.hxx>


// virtual
bool CntByteItem::operator ==(const SfxPoolItem & rItem) const
{
    assert(SfxPoolItem::operator==(rItem));
    return m_nValue == static_cast< const CntByteItem * >(&rItem)->m_nValue;
}

// virtual
bool CntByteItem::GetPresentation(SfxItemPresentation, MapUnit, MapUnit,
                                  OUString & rText,
                                  const IntlWrapper&) const
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

    SAL_WARN("svl.items", "CntByteItem::PutValue - Wrong type!");
    return false;
}

// virtual
CntByteItem* CntByteItem::Clone(SfxItemPool *) const
{
    return new CntByteItem(*this);
}

// virtual
bool CntUInt16Item::operator ==(const SfxPoolItem & rItem) const
{
    assert(SfxPoolItem::operator==(rItem));
    return m_nValue == static_cast<const CntUInt16Item *>(&rItem)->m_nValue;
}

// virtual
bool CntUInt16Item::GetPresentation(SfxItemPresentation,
                                    MapUnit, MapUnit,
                                    OUString & rText,
                                    const IntlWrapper&)
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
        SAL_WARN_IF(nValue < 0 || nValue > SAL_MAX_UINT16, "svl.items", "Overflow in UInt16 value!");
        m_nValue = static_cast<sal_uInt16>(nValue);
        return true;
    }

    SAL_WARN("svl.items", "CntUInt16Item::PutValue - Wrong type!");
    return false;
}

// virtual
CntUInt16Item* CntUInt16Item::Clone(SfxItemPool *) const
{
    return new CntUInt16Item(*this);
}

// virtual
bool CntInt32Item::operator ==(const SfxPoolItem & rItem) const
{
    assert(SfxPoolItem::operator==(rItem));
    return m_nValue == static_cast<const CntInt32Item *>(&rItem)->m_nValue;
}

// virtual
bool CntInt32Item::GetPresentation(SfxItemPresentation,
                                   MapUnit, MapUnit,
                                   OUString & rText,
                                   const IntlWrapper&) const
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

    SAL_WARN("svl.items", "CntInt32Item::PutValue - Wrong type!");
    return false;
}

// virtual
CntInt32Item* CntInt32Item::Clone(SfxItemPool *) const
{
    return new CntInt32Item(*this);
}

// virtual
bool CntUInt32Item::operator ==(const SfxPoolItem & rItem) const
{
    assert(SfxPoolItem::operator==(rItem));
    return m_nValue == static_cast<const CntUInt32Item *>(&rItem)->m_nValue;
}

// virtual
bool CntUInt32Item::GetPresentation(SfxItemPresentation,
                                    MapUnit, MapUnit,
                                    OUString & rText,
                                    const IntlWrapper&)
    const
{
    rText = OUString::number(m_nValue);
    return true;
}

// virtual
bool CntUInt32Item::QueryValue(css::uno::Any& rVal, sal_uInt8) const
{
    sal_Int32 nValue = m_nValue;
    SAL_WARN_IF(nValue < 0, "svl.items", "Overflow in UInt32 value!");
    rVal <<= nValue;
    return true;
}

// virtual
bool CntUInt32Item::PutValue(const css::uno::Any& rVal, sal_uInt8)
{
    sal_Int32 nValue = 0;
    if (rVal >>= nValue)
    {
        SAL_WARN_IF(nValue < 0, "svl.items", "Overflow in UInt32 value!");
        m_nValue = nValue;
        return true;
    }

    SAL_WARN("svl.items", "CntUInt32Item::PutValue - Wrong type!");
    return false;
}

// virtual
CntUInt32Item* CntUInt32Item::Clone(SfxItemPool *) const
{
    return new CntUInt32Item(*this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
