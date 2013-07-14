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
#include <svl/cintitem.hxx>

//
//  class CntByteItem
//

DBG_NAME(CntByteItem)

TYPEINIT1_AUTOFACTORY(CntByteItem, SfxPoolItem);

// virtual
int CntByteItem::operator ==(const SfxPoolItem & rItem) const
{
    DBG_CHKTHIS(CntByteItem, 0);
    DBG_ASSERT(rItem.ISA(CntByteItem),
               "CntByteItem::operator ==(): Bad type");
    return m_nValue == (static_cast< const CntByteItem * >(&rItem))->m_nValue;
}

// virtual
int CntByteItem::Compare(const SfxPoolItem & rWith) const
{
    DBG_CHKTHIS(CntByteItem, 0);
    DBG_ASSERT(rWith.ISA(CntByteItem), "CntByteItem::Compare(): Bad type");
    return (static_cast< const CntByteItem * >(&rWith))->m_nValue < m_nValue ?
            -1 :
           (static_cast< const CntByteItem * >(&rWith))->m_nValue
             == m_nValue ?
            0 : 1;
}

// virtual
SfxItemPresentation CntByteItem::GetPresentation(SfxItemPresentation,
                                                 SfxMapUnit, SfxMapUnit,
                                                 OUString & rText,
                                                 const IntlWrapper *) const
{
    DBG_CHKTHIS(CntByteItem, 0);
    rText = OUString::number( m_nValue );
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

// virtual
bool CntByteItem::QueryValue(com::sun::star::uno::Any& rVal, sal_uInt8) const
{
    sal_Int8 nValue = m_nValue;
    rVal <<= nValue;
    return true;
}

// virtual
bool CntByteItem::PutValue(const com::sun::star::uno::Any& rVal, sal_uInt8)
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
    DBG_CHKTHIS(CntByteItem, 0);
    short nTheValue = 0;
    rStream >> nTheValue;
    return new CntByteItem(Which(), sal_uInt8(nTheValue));
}

// virtual
SvStream & CntByteItem::Store(SvStream & rStream, sal_uInt16) const
{
    DBG_CHKTHIS(CntByteItem, 0);
    rStream << short(m_nValue);
    return rStream;
}

// virtual
SfxPoolItem * CntByteItem::Clone(SfxItemPool *) const
{
    DBG_CHKTHIS(CntByteItem, 0);
    return new CntByteItem(*this);
}

// virtual
sal_uInt8 CntByteItem::GetMin() const
{
    DBG_CHKTHIS(CntByteItem, 0);
    return 0;
}

// virtual
sal_uInt8 CntByteItem::GetMax() const
{
    DBG_CHKTHIS(CntByteItem, 0);
    return 255;
}

// virtual
SfxFieldUnit CntByteItem::GetUnit() const
{
    DBG_CHKTHIS(CntByteItem, 0);
    return SFX_FUNIT_NONE;
}

//
//  class CntUInt16Item
//

DBG_NAME(CntUInt16Item);

TYPEINIT1_AUTOFACTORY(CntUInt16Item, SfxPoolItem);

CntUInt16Item::CntUInt16Item(sal_uInt16 which, SvStream & rStream) :
    SfxPoolItem(which)
{
    DBG_CTOR(CntUInt16Item, 0);
    sal_uInt16 nTheValue = 0;
    rStream >> nTheValue;
    m_nValue = nTheValue;
}

// virtual
int CntUInt16Item::operator ==(const SfxPoolItem & rItem) const
{
    DBG_CHKTHIS(CntUInt16Item, 0);
    DBG_ASSERT(rItem.ISA(CntUInt16Item),
               "CntUInt16Item::operator ==(): Bad type");
    return m_nValue == (static_cast< const CntUInt16Item * >(&rItem))->
                        m_nValue;
}

// virtual
int CntUInt16Item::Compare(const SfxPoolItem & rWith) const
{
    DBG_CHKTHIS(CntUInt16Item, 0);
    DBG_ASSERT(rWith.ISA(CntUInt16Item),
               "CntUInt16Item::Compare(): Bad type");
    return (static_cast< const CntUInt16Item * >(&rWith))->m_nValue
             < m_nValue ?
            -1 :
           (static_cast< const CntUInt16Item * >(&rWith))->m_nValue
             == m_nValue ?
            0 : 1;
}

// virtual
SfxItemPresentation CntUInt16Item::GetPresentation(SfxItemPresentation,
                                                   SfxMapUnit, SfxMapUnit,
                                                   OUString & rText,
                                                   const IntlWrapper *)
    const
{
    DBG_CHKTHIS(CntUInt16Item, 0);
    rText = OUString::number( m_nValue );
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

// virtual
bool CntUInt16Item::QueryValue(com::sun::star::uno::Any& rVal, sal_uInt8) const
{
    sal_Int32 nValue = m_nValue;
    rVal <<= nValue;
    return true;
}

// virtual
bool CntUInt16Item::PutValue(const com::sun::star::uno::Any& rVal, sal_uInt8)
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
    DBG_CHKTHIS(CntUInt16Item, 0);
    return new CntUInt16Item(Which(), rStream);
}

// virtual
SvStream & CntUInt16Item::Store(SvStream &rStream, sal_uInt16) const
{
    DBG_CHKTHIS(CntUInt16Item, 0);
    rStream << sal_uInt16(m_nValue);
    return rStream;
}

// virtual
SfxPoolItem * CntUInt16Item::Clone(SfxItemPool *) const
{
    DBG_CHKTHIS(CntUInt16Item, 0);
    return new CntUInt16Item(*this);
}

// virtual
sal_uInt16 CntUInt16Item::GetMin() const
{
    DBG_CHKTHIS(CntUInt16Item, 0);
    return 0;
}

// virtual
sal_uInt16 CntUInt16Item::GetMax() const
{
    DBG_CHKTHIS(CntUInt16Item, 0);
    return 65535;
}

// virtual
SfxFieldUnit CntUInt16Item::GetUnit() const
{
    DBG_CHKTHIS(CntUInt16Item, 0);
    return SFX_FUNIT_NONE;
}

//
//  class CntInt32Item
//

DBG_NAME(CntInt32Item);

TYPEINIT1_AUTOFACTORY(CntInt32Item, SfxPoolItem);

CntInt32Item::CntInt32Item(sal_uInt16 which, SvStream & rStream) :
    SfxPoolItem(which)
{
    DBG_CTOR(CntInt32Item, 0);
    //fdo#39428 SvStream no longer supports operator>>(long&)
    rStream >> m_nValue;
}

// virtual
int CntInt32Item::operator ==(const SfxPoolItem & rItem) const
{
    DBG_CHKTHIS(CntInt32Item, 0);
    DBG_ASSERT(rItem.ISA(CntInt32Item),
               "CntInt32Item::operator ==(): Bad type");
    return m_nValue == (static_cast< const CntInt32Item * >(&rItem))->
                        m_nValue;
}

// virtual
int CntInt32Item::Compare(const SfxPoolItem & rWith) const
{
    DBG_CHKTHIS(CntInt32Item, 0);
    DBG_ASSERT(rWith.ISA(CntInt32Item), "CntInt32Item::Compare(): Bad type");
    return (static_cast< const CntInt32Item * >(&rWith))->m_nValue
             < m_nValue ?
            -1 :
           (static_cast< const CntInt32Item * >(&rWith))->m_nValue
             == m_nValue ?
            0 : 1;
}

// virtual
SfxItemPresentation CntInt32Item::GetPresentation(SfxItemPresentation,
                                                  SfxMapUnit, SfxMapUnit,
                                                  OUString & rText,
                                                  const IntlWrapper *) const
{
    DBG_CHKTHIS(CntInt32Item, 0);
    rText = OUString::number( m_nValue );
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

// virtual
bool CntInt32Item::QueryValue(com::sun::star::uno::Any& rVal, sal_uInt8) const
{
    sal_Int32 nValue = m_nValue;
    rVal <<= nValue;
    return true;
}

// virtual
bool CntInt32Item::PutValue(const com::sun::star::uno::Any& rVal, sal_uInt8)
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
    DBG_CHKTHIS(CntInt32Item, 0);
    return new CntInt32Item(Which(), rStream);
}

// virtual
SvStream & CntInt32Item::Store(SvStream &rStream, sal_uInt16) const
{
    DBG_CHKTHIS(CntInt32Item, 0);
    //fdo#39428 SvStream no longer supports operator<<(long)
    rStream << m_nValue;
    return rStream;
}

// virtual
SfxPoolItem * CntInt32Item::Clone(SfxItemPool *) const
{
    DBG_CHKTHIS(CntInt32Item, 0);
    return new CntInt32Item(*this);
}

// virtual
sal_Int32 CntInt32Item::GetMin() const
{
    DBG_CHKTHIS(CntInt32Item, 0);
    return sal_Int32(0x80000000);
}

// virtual
sal_Int32 CntInt32Item::GetMax() const
{
    DBG_CHKTHIS(CntInt32Item, 0);
    return 0x7FFFFFFF;
}

// virtual
SfxFieldUnit CntInt32Item::GetUnit() const
{
    DBG_CHKTHIS(CntInt32Item, 0);
    return SFX_FUNIT_NONE;
}

//
//  class CntUInt32Item
//

DBG_NAME(CntUInt32Item);

TYPEINIT1_AUTOFACTORY(CntUInt32Item, SfxPoolItem);

CntUInt32Item::CntUInt32Item(sal_uInt16 which, SvStream & rStream) :
    SfxPoolItem(which)
{
    DBG_CTOR(CntUInt32Item, 0);
    sal_uInt32 nTheValue = 0;
    rStream >> nTheValue;
    m_nValue = nTheValue;
}

// virtual
int CntUInt32Item::operator ==(const SfxPoolItem & rItem) const
{
    DBG_CHKTHIS(CntUInt32Item, 0);
    DBG_ASSERT(rItem.ISA(CntUInt32Item),
               "CntUInt32Item::operator ==(): Bad type");
    return m_nValue == (static_cast< const CntUInt32Item * >(&rItem))->
                        m_nValue;
}

// virtual
int CntUInt32Item::Compare(const SfxPoolItem & rWith) const
{
    DBG_CHKTHIS(CntUInt32Item, 0);
    DBG_ASSERT(rWith.ISA(CntUInt32Item),
               "CntUInt32Item::operator ==(): Bad type");
    return (static_cast< const CntUInt32Item * >(&rWith))->m_nValue
             < m_nValue ?
            -1 :
           (static_cast< const CntUInt32Item * >(&rWith))->m_nValue
             == m_nValue ?
            0 : 1;
}

// virtual
SfxItemPresentation CntUInt32Item::GetPresentation(SfxItemPresentation,
                                                   SfxMapUnit, SfxMapUnit,
                                                   OUString & rText,
                                                   const IntlWrapper *)
    const
{
    DBG_CHKTHIS(CntUInt32Item, 0);
    rText = OUString::number(m_nValue);
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

// virtual
bool CntUInt32Item::QueryValue(com::sun::star::uno::Any& rVal, sal_uInt8) const
{
    sal_Int32 nValue = m_nValue;
    DBG_ASSERT( nValue>=0, "Overflow in UInt32 value!");
    rVal <<= nValue;
    return true;
}

// virtual
bool CntUInt32Item::PutValue(const com::sun::star::uno::Any& rVal, sal_uInt8)
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
    DBG_CHKTHIS(CntUInt32Item, 0);
    return new CntUInt32Item(Which(), rStream);
}

// virtual
SvStream & CntUInt32Item::Store(SvStream &rStream, sal_uInt16) const
{
    DBG_CHKTHIS(CntUInt32Item, 0);
    rStream << static_cast<sal_uInt32>(m_nValue);
    return rStream;
}

// virtual
SfxPoolItem * CntUInt32Item::Clone(SfxItemPool *) const
{
    DBG_CHKTHIS(CntUInt32Item, 0);
    return new CntUInt32Item(*this);
}

// virtual
sal_uInt32 CntUInt32Item::GetMin() const
{
    DBG_CHKTHIS(CntUInt32Item, 0);
    return 0;
}

// virtual
sal_uInt32 CntUInt32Item::GetMax() const
{
    DBG_CHKTHIS(CntUInt32Item, 0);
    return 0xFFFFFFFF;
}

// virtual
SfxFieldUnit CntUInt32Item::GetUnit() const
{
    DBG_CHKTHIS(CntUInt32Item, 0);
    return SFX_FUNIT_NONE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
