/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <com/sun/star/uno/Any.hxx>
#include <tools/stream.hxx>
#include <svl/cenumitm.hxx>
#include <svl/eitem.hxx>
#include <whassert.hxx>

#include <comphelper/extract.hxx>

//

//

DBG_NAME(SfxEnumItemInterface)

TYPEINIT1(SfxEnumItemInterface, SfxPoolItem)


bool SfxEnumItemInterface::operator ==(const SfxPoolItem & rItem) const
{
    SFX_ASSERT(SfxPoolItem::operator ==(rItem), Which(), "unequal type");
    return GetEnumValue()
               == static_cast< const SfxEnumItemInterface * >(&rItem)->
                      GetEnumValue();
}


SfxItemPresentation
SfxEnumItemInterface::GetPresentation(SfxItemPresentation, SfxMapUnit,
                                      SfxMapUnit, OUString & rText,
                                      const IntlWrapper *) const
{
    rText = OUString::number( GetEnumValue() );
    return SFX_ITEM_PRESENTATION_NAMELESS;
}


bool SfxEnumItemInterface::QueryValue(com::sun::star::uno::Any& rVal, sal_uInt8)
    const
{
    rVal <<= sal_Int32(GetEnumValue());
    return true;
}


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


sal_uInt16 SfxEnumItemInterface::GetValueByPos(sal_uInt16 nPos) const
{
    return nPos;
}


sal_uInt16 SfxEnumItemInterface::GetPosByValue(sal_uInt16 nValue) const
{
    sal_uInt16 nCount = GetValueCount();
    for (sal_uInt16 i = 0; i < nCount; ++i)
        if (GetValueByPos(i) == nValue)
            return i;
    return USHRT_MAX;
}

bool SfxEnumItemInterface::IsEnabled(sal_uInt16) const
{
    return true;
}


bool SfxEnumItemInterface::HasBoolValue() const
{
    return false;
}


bool SfxEnumItemInterface::GetBoolValue() const
{
    return false;
}


void SfxEnumItemInterface::SetBoolValue(sal_Bool)
{}

//

//

DBG_NAME(SfxEnumItem)

SfxEnumItem::SfxEnumItem(sal_uInt16 const nWhich, SvStream & rStream)
    : SfxEnumItemInterface(nWhich)
{
    m_nValue = 0;
    rStream.ReadUInt16( m_nValue );
}

TYPEINIT1(SfxEnumItem, SfxEnumItemInterface)


SvStream & SfxEnumItem::Store(SvStream & rStream, sal_uInt16) const
{
    rStream.WriteUInt16( m_nValue );
    return rStream;
}


sal_uInt16 SfxEnumItem::GetEnumValue() const
{
    return GetValue();
}


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

//

DBG_NAME(SfxBoolItem)

TYPEINIT1_AUTOFACTORY(SfxBoolItem, SfxPoolItem);

SfxBoolItem::SfxBoolItem(sal_uInt16 const nWhich, SvStream & rStream)
    : SfxPoolItem(nWhich)
{
    bool tmp = false;
    rStream.ReadCharAsBool( tmp );
    m_bValue = tmp;
}


bool SfxBoolItem::operator ==(const SfxPoolItem & rItem) const
{
    DBG_ASSERT(rItem.ISA(SfxBoolItem),
               "SfxBoolItem::operator ==(): Bad type");
    return m_bValue == static_cast< SfxBoolItem const * >(&rItem)->m_bValue;
}


int SfxBoolItem::Compare(const SfxPoolItem & rWith) const
{
    DBG_ASSERT(rWith.ISA(SfxBoolItem), "SfxBoolItem::Compare(): Bad type");
    return (m_bValue == static_cast<SfxBoolItem const*>(&rWith)->m_bValue) ?
               0 : m_bValue ? -1 : 1;
}


SfxItemPresentation SfxBoolItem::GetPresentation(SfxItemPresentation,
                                                 SfxMapUnit, SfxMapUnit,
                                                 OUString & rText,
                                                 const IntlWrapper *) const
{
    rText = GetValueTextByVal(m_bValue);
    return SFX_ITEM_PRESENTATION_NAMELESS;
}


bool SfxBoolItem::QueryValue(com::sun::star::uno::Any& rVal, sal_uInt8) const
{
    rVal <<= m_bValue;
    return true;
}


bool SfxBoolItem::PutValue(const com::sun::star::uno::Any& rVal, sal_uInt8)
{
    bool bTheValue = bool();
    if (rVal >>= bTheValue)
    {
        m_bValue = bTheValue;
        return true;
    }
    OSL_FAIL("SfxBoolItem::PutValue(): Wrong type");
    return false;
}


SfxPoolItem * SfxBoolItem::Create(SvStream & rStream, sal_uInt16) const
{
    return new SfxBoolItem(Which(), rStream);
}


SvStream & SfxBoolItem::Store(SvStream & rStream, sal_uInt16) const
{
    rStream.WriteUChar( m_bValue ); 
    return rStream;
}


SfxPoolItem * SfxBoolItem::Clone(SfxItemPool *) const
{
    return new SfxBoolItem(*this);
}


sal_uInt16 SfxBoolItem::GetValueCount() const
{
    return 2;
}


OUString SfxBoolItem::GetValueTextByVal(sal_Bool bTheValue) const
{
    return bTheValue ?  OUString("TRUE") : OUString("FALSE");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
