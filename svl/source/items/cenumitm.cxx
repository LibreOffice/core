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
#include <tools/stream.hxx>
#include <svl/cenumitm.hxx>
#include <svl/eitem.hxx>
#include <whassert.hxx>

#include <comphelper/extract.hxx>

//
//  class SfxEnumItemInterface
//

DBG_NAME(SfxEnumItemInterface)

TYPEINIT1(SfxEnumItemInterface, SfxPoolItem)

// virtual
int SfxEnumItemInterface::operator ==(const SfxPoolItem & rItem) const
{
    SFX_ASSERT(SfxPoolItem::operator ==(rItem), Which(), "unequal type");
    return GetEnumValue()
               == static_cast< const SfxEnumItemInterface * >(&rItem)->
                      GetEnumValue();
}

// virtual
SfxItemPresentation
SfxEnumItemInterface::GetPresentation(SfxItemPresentation, SfxMapUnit,
                                      SfxMapUnit, OUString & rText,
                                      const IntlWrapper *) const
{
    rText = OUString::number( GetEnumValue() );
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

// virtual
bool SfxEnumItemInterface::QueryValue(com::sun::star::uno::Any& rVal, sal_uInt8)
    const
{
    rVal <<= sal_Int32(GetEnumValue());
    return true;
}

// virtual
bool SfxEnumItemInterface::PutValue(const com::sun::star::uno::Any& rVal,
                                    sal_uInt8)
{
    sal_Int32 nTheValue = 0;

    if ( ::cppu::enum2int( nTheValue, rVal ) )
    {
        SetEnumValue(sal_uInt16(nTheValue));
        return true;
    }
    OSL_FAIL("SfxEnumItemInterface::PutValue(): Wrong type");
    return false;
}

OUString SfxEnumItemInterface::GetValueTextByPos(sal_uInt16) const
{
    DBG_WARNING("SfxEnumItemInterface::GetValueTextByPos(): Pure virtual");
    return OUString();
}

// virtual
sal_uInt16 SfxEnumItemInterface::GetValueByPos(sal_uInt16 nPos) const
{
    return nPos;
}

// virtual
sal_uInt16 SfxEnumItemInterface::GetPosByValue(sal_uInt16 nValue) const
{
    sal_uInt16 nCount = GetValueCount();
    for (sal_uInt16 i = 0; i < nCount; ++i)
        if (GetValueByPos(i) == nValue)
            return i;
    return USHRT_MAX;
}

sal_Bool SfxEnumItemInterface::IsEnabled(sal_uInt16) const
{
    return sal_True;
}

// virtual
int SfxEnumItemInterface::HasBoolValue() const
{
    return false;
}

// virtual
sal_Bool SfxEnumItemInterface::GetBoolValue() const
{
    return false;
}

// virtual
void SfxEnumItemInterface::SetBoolValue(sal_Bool)
{}

//
//  class SfxEnumItem
//

DBG_NAME(SfxEnumItem)

SfxEnumItem::SfxEnumItem(sal_uInt16 const nWhich, SvStream & rStream)
    : SfxEnumItemInterface(nWhich)
{
    m_nValue = 0;
    rStream >> m_nValue;
}

TYPEINIT1(SfxEnumItem, SfxEnumItemInterface)

// virtual
SvStream & SfxEnumItem::Store(SvStream & rStream, sal_uInt16) const
{
    rStream << m_nValue;
    return rStream;
}

// virtual
sal_uInt16 SfxEnumItem::GetEnumValue() const
{
    return GetValue();
}

// virtual
void SfxEnumItem::SetEnumValue(sal_uInt16 const nTheValue)
{
    SetValue(nTheValue);
}

void SfxEnumItem::SetValue(sal_uInt16 const nTheValue)
{
    DBG_ASSERT(GetRefCount() == 0, "SfxEnumItem::SetValue(): Pooled item");
    m_nValue = nTheValue;
}

//
//  class SfxBoolItem
//

DBG_NAME(SfxBoolItem)

TYPEINIT1_AUTOFACTORY(SfxBoolItem, SfxPoolItem);

SfxBoolItem::SfxBoolItem(sal_uInt16 const nWhich, SvStream & rStream)
    : SfxPoolItem(nWhich)
{
    sal_Bool tmp = false;
    rStream >> tmp;
    m_bValue = tmp;
}

// virtual
int SfxBoolItem::operator ==(const SfxPoolItem & rItem) const
{
    DBG_ASSERT(rItem.ISA(SfxBoolItem),
               "SfxBoolItem::operator ==(): Bad type");
    return m_bValue == static_cast< SfxBoolItem const * >(&rItem)->m_bValue;
}

// virtual
int SfxBoolItem::Compare(const SfxPoolItem & rWith) const
{
    DBG_ASSERT(rWith.ISA(SfxBoolItem), "SfxBoolItem::Compare(): Bad type");
    return (m_bValue == static_cast<SfxBoolItem const*>(&rWith)->m_bValue) ?
               0 : m_bValue ? -1 : 1;
}

// virtual
SfxItemPresentation SfxBoolItem::GetPresentation(SfxItemPresentation,
                                                 SfxMapUnit, SfxMapUnit,
                                                 OUString & rText,
                                                 const IntlWrapper *) const
{
    rText = GetValueTextByVal(m_bValue);
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

// virtual
bool SfxBoolItem::QueryValue(com::sun::star::uno::Any& rVal, sal_uInt8) const
{
    rVal <<= m_bValue;
    return true;
}

// virtual
bool SfxBoolItem::PutValue(const com::sun::star::uno::Any& rVal, sal_uInt8)
{
    sal_Bool bTheValue = sal_Bool();
    if (rVal >>= bTheValue)
    {
        m_bValue = bTheValue;
        return true;
    }
    OSL_FAIL("SfxBoolItem::PutValue(): Wrong type");
    return false;
}

// virtual
SfxPoolItem * SfxBoolItem::Create(SvStream & rStream, sal_uInt16) const
{
    return new SfxBoolItem(Which(), rStream);
}

// virtual
SvStream & SfxBoolItem::Store(SvStream & rStream, sal_uInt16) const
{
    rStream << static_cast<sal_Bool>(m_bValue); // not bool for serialization!
    return rStream;
}

// virtual
SfxPoolItem * SfxBoolItem::Clone(SfxItemPool *) const
{
    return new SfxBoolItem(*this);
}

// virtual
sal_uInt16 SfxBoolItem::GetValueCount() const
{
    return 2;
}

// virtual
OUString SfxBoolItem::GetValueTextByVal(sal_Bool bTheValue) const
{
    return bTheValue ?  OUString("TRUE") : OUString("FALSE");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
