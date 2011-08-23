/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove


#include <com/sun/star/uno/Any.hxx>

#include <tools/stream.hxx>

#include <bf_svtools/cintitem.hxx>

namespace binfilter
{

//============================================================================
//
//  class CntByteItem
//
//============================================================================

DBG_NAME(CntByteItem)

//============================================================================
TYPEINIT1_AUTOFACTORY(CntByteItem, SfxPoolItem);

//============================================================================
// virtual
int CntByteItem::operator ==(const SfxPoolItem & rItem) const
{
    DBG_CHKTHIS(CntByteItem, 0);
    DBG_ASSERT(rItem.ISA(CntByteItem),
               "CntByteItem::operator ==(): Bad type");
    return m_nValue == SAL_STATIC_CAST(const CntByteItem *, &rItem)->m_nValue;
}

//============================================================================
// virtual
int CntByteItem::Compare(const SfxPoolItem & rWith) const
{
    DBG_CHKTHIS(CntByteItem, 0);
    DBG_ASSERT(rWith.ISA(CntByteItem), "CntByteItem::Compare(): Bad type");
    return SAL_STATIC_CAST(const CntByteItem *, &rWith)->m_nValue < m_nValue ?
            -1 :
           SAL_STATIC_CAST(const CntByteItem *, &rWith)->m_nValue
             == m_nValue ?
            0 : 1;
}

//============================================================================
// virtual
SfxItemPresentation CntByteItem::GetPresentation(SfxItemPresentation,
                                                 SfxMapUnit, SfxMapUnit,
                                                 XubString & rText,
                                                 const ::IntlWrapper *) const
{
    DBG_CHKTHIS(CntByteItem, 0);
    rText = XubString::CreateFromInt32(m_nValue);
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

//============================================================================
// virtual
bool CntByteItem::QueryValue(com::sun::star::uno::Any& rVal,BYTE) const
{
    sal_Int8 nValue = m_nValue;
    rVal <<= nValue;
    return true;
}

//============================================================================
// virtual
bool CntByteItem::PutValue(const com::sun::star::uno::Any& rVal,BYTE)
{
    sal_Int8 nValue = sal_Int8();
    if (rVal >>= nValue)
    {
        m_nValue = nValue;
        return true;
    }

    DBG_ERROR( "CntByteItem::PutValue - Wrong type!" );
    return false;
}

//============================================================================
// virtual
SfxPoolItem * CntByteItem::Create(SvStream & rStream, USHORT) const
{
    DBG_CHKTHIS(CntByteItem, 0);
    short nTheValue = 0;
    rStream >> nTheValue;
    return new CntByteItem(Which(), BYTE(nTheValue));
}

//============================================================================
// virtual
SvStream & CntByteItem::Store(SvStream & rStream, USHORT) const
{
    DBG_CHKTHIS(CntByteItem, 0);
    rStream << short(m_nValue);
    return rStream;
}

//============================================================================
// virtual
SfxPoolItem * CntByteItem::Clone(SfxItemPool *) const
{
    DBG_CHKTHIS(CntByteItem, 0);
    return new CntByteItem(*this);
}

//============================================================================
// virtual
BYTE CntByteItem::GetMin() const
{
    DBG_CHKTHIS(CntByteItem, 0);
    return 0;
}

//============================================================================
// virtual
BYTE CntByteItem::GetMax() const
{
    DBG_CHKTHIS(CntByteItem, 0);
    return 255;
}

//============================================================================
// virtual
SfxFieldUnit CntByteItem::GetUnit() const
{
    DBG_CHKTHIS(CntByteItem, 0);
    return SFX_FUNIT_NONE;
}

//============================================================================
//
//  class CntUInt16Item
//
//============================================================================

DBG_NAME(CntUInt16Item);

//============================================================================
TYPEINIT1_AUTOFACTORY(CntUInt16Item, SfxPoolItem);

//============================================================================
CntUInt16Item::CntUInt16Item(USHORT which, SvStream & rStream) :
    SfxPoolItem(which)
{
    DBG_CTOR(CntUInt16Item, 0);
    USHORT nTheValue = 0;
    rStream >> nTheValue;
    m_nValue = nTheValue;
}

//============================================================================
// virtual
int CntUInt16Item::operator ==(const SfxPoolItem & rItem) const
{
    DBG_CHKTHIS(CntUInt16Item, 0);
    DBG_ASSERT(rItem.ISA(CntUInt16Item),
               "CntUInt16Item::operator ==(): Bad type");
    return m_nValue == SAL_STATIC_CAST(const CntUInt16Item *, &rItem)->
                        m_nValue;
}

//============================================================================
// virtual
int CntUInt16Item::Compare(const SfxPoolItem & rWith) const
{
    DBG_CHKTHIS(CntUInt16Item, 0);
    DBG_ASSERT(rWith.ISA(CntUInt16Item),
               "CntUInt16Item::Compare(): Bad type");
    return SAL_STATIC_CAST(const CntUInt16Item *, &rWith)->m_nValue
             < m_nValue ?
            -1 :
           SAL_STATIC_CAST(const CntUInt16Item *, &rWith)->m_nValue
             == m_nValue ?
            0 : 1;
}

//============================================================================
// virtual
SfxItemPresentation CntUInt16Item::GetPresentation(SfxItemPresentation,
                                                   SfxMapUnit, SfxMapUnit,
                                                   XubString & rText,
                                                   const ::IntlWrapper *)
    const
{
    DBG_CHKTHIS(CntUInt16Item, 0);
    rText = XubString::CreateFromInt32(m_nValue);
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

//============================================================================
// virtual
bool CntUInt16Item::QueryValue(com::sun::star::uno::Any& rVal,BYTE) const
{
    sal_Int32 nValue = m_nValue;
    rVal <<= nValue;
    return true;
}

//============================================================================
// virtual
bool CntUInt16Item::PutValue(const com::sun::star::uno::Any& rVal,BYTE)
{
    sal_Int32 nValue = 0;
    if (rVal >>= nValue)
    {
        DBG_ASSERT( nValue <= USHRT_MAX, "Overflow in UInt16 value!");
        m_nValue = (sal_uInt16)nValue;
        return true;
    }

    DBG_ERROR( "CntUInt16Item::PutValue - Wrong type!" );
    return false;
}

//============================================================================
// virtual
SfxPoolItem * CntUInt16Item::Create(SvStream & rStream, USHORT) const
{
    DBG_CHKTHIS(CntUInt16Item, 0);
    return new CntUInt16Item(Which(), rStream);
}

//============================================================================
// virtual
SvStream & CntUInt16Item::Store(SvStream &rStream, USHORT) const
{
    DBG_CHKTHIS(CntUInt16Item, 0);
    rStream << USHORT(m_nValue);
    return rStream;
}

//============================================================================
// virtual
SfxPoolItem * CntUInt16Item::Clone(SfxItemPool *) const
{
    DBG_CHKTHIS(CntUInt16Item, 0);
    return new CntUInt16Item(*this);
}

//============================================================================
// virtual
UINT16 CntUInt16Item::GetMin() const
{
    DBG_CHKTHIS(CntUInt16Item, 0);
    return 0;
}

//============================================================================
// virtual
UINT16 CntUInt16Item::GetMax() const
{
    DBG_CHKTHIS(CntUInt16Item, 0);
    return 65535;
}

//============================================================================
// virtual
SfxFieldUnit CntUInt16Item::GetUnit() const
{
    DBG_CHKTHIS(CntUInt16Item, 0);
    return SFX_FUNIT_NONE;
}

//============================================================================
//
//  class CntInt32Item
//
//============================================================================

DBG_NAME(CntInt32Item);

//============================================================================
TYPEINIT1_AUTOFACTORY(CntInt32Item, SfxPoolItem);

//============================================================================
CntInt32Item::CntInt32Item(USHORT which, SvStream & rStream) :
    SfxPoolItem(which)
{
    DBG_CTOR(CntInt32Item, 0);
    long nTheValue = 0;
    rStream >> nTheValue;
    m_nValue = nTheValue;
}

//============================================================================
// virtual
int CntInt32Item::operator ==(const SfxPoolItem & rItem) const
{
    DBG_CHKTHIS(CntInt32Item, 0);
    DBG_ASSERT(rItem.ISA(CntInt32Item),
               "CntInt32Item::operator ==(): Bad type");
    return m_nValue == SAL_STATIC_CAST(const CntInt32Item *, &rItem)->
                        m_nValue;
}

//============================================================================
// virtual
int CntInt32Item::Compare(const SfxPoolItem & rWith) const
{
    DBG_CHKTHIS(CntInt32Item, 0);
    DBG_ASSERT(rWith.ISA(CntInt32Item), "CntInt32Item::Compare(): Bad type");
    return SAL_STATIC_CAST(const CntInt32Item *, &rWith)->m_nValue
             < m_nValue ?
            -1 :
           SAL_STATIC_CAST(const CntInt32Item *, &rWith)->m_nValue
             == m_nValue ?
            0 : 1;
}

//============================================================================
// virtual
SfxItemPresentation CntInt32Item::GetPresentation(SfxItemPresentation,
                                                  SfxMapUnit, SfxMapUnit,
                                                  XubString & rText,
                                                  const ::IntlWrapper *) const
{
    DBG_CHKTHIS(CntInt32Item, 0);
    rText = XubString::CreateFromInt32(m_nValue);
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

//============================================================================
// virtual
bool CntInt32Item::QueryValue(com::sun::star::uno::Any& rVal,BYTE) const
{
    sal_Int32 nValue = m_nValue;
    rVal <<= nValue;
    return true;
}

//============================================================================
// virtual
bool CntInt32Item::PutValue(const com::sun::star::uno::Any& rVal,BYTE)
{
    sal_Int32 nValue = 0;
    if (rVal >>= nValue)
    {
        m_nValue = nValue;
        return true;
    }

    DBG_ERROR( "CntInt32Item::PutValue - Wrong type!" );
    return false;
}

//============================================================================
// virtual
SfxPoolItem * CntInt32Item::Create(SvStream & rStream, USHORT) const
{
    DBG_CHKTHIS(CntInt32Item, 0);
    return new CntInt32Item(Which(), rStream);
}

//============================================================================
// virtual
SvStream & CntInt32Item::Store(SvStream &rStream, USHORT) const
{
    DBG_CHKTHIS(CntInt32Item, 0);
    rStream << long(m_nValue);
    return rStream;
}

//============================================================================
// virtual
SfxPoolItem * CntInt32Item::Clone(SfxItemPool *) const
{
    DBG_CHKTHIS(CntInt32Item, 0);
    return new CntInt32Item(*this);
}

//============================================================================
// virtual
INT32 CntInt32Item::GetMin() const
{
    DBG_CHKTHIS(CntInt32Item, 0);
    return INT32(0x80000000);
}

//============================================================================
// virtual
INT32 CntInt32Item::GetMax() const
{
    DBG_CHKTHIS(CntInt32Item, 0);
    return 0x7FFFFFFF;
}

//============================================================================
// virtual
SfxFieldUnit CntInt32Item::GetUnit() const
{
    DBG_CHKTHIS(CntInt32Item, 0);
    return SFX_FUNIT_NONE;
}

//============================================================================
//
//  class CntUInt32Item
//
//============================================================================

DBG_NAME(CntUInt32Item);

//============================================================================
TYPEINIT1_AUTOFACTORY(CntUInt32Item, SfxPoolItem);

//============================================================================
CntUInt32Item::CntUInt32Item(USHORT which, SvStream & rStream) :
    SfxPoolItem(which)
{
    DBG_CTOR(CntUInt32Item, 0);
    sal_uInt32 nTheValue = 0;
    rStream >> nTheValue;
    m_nValue = nTheValue;
}

//============================================================================
// virtual
int CntUInt32Item::operator ==(const SfxPoolItem & rItem) const
{
    DBG_CHKTHIS(CntUInt32Item, 0);
    DBG_ASSERT(rItem.ISA(CntUInt32Item),
               "CntUInt32Item::operator ==(): Bad type");
    return m_nValue == SAL_STATIC_CAST(const CntUInt32Item *, &rItem)->
                        m_nValue;
}

//============================================================================
// virtual
int CntUInt32Item::Compare(const SfxPoolItem & rWith) const
{
    DBG_CHKTHIS(CntUInt32Item, 0);
    DBG_ASSERT(rWith.ISA(CntUInt32Item),
               "CntUInt32Item::operator ==(): Bad type");
    return SAL_STATIC_CAST(const CntUInt32Item *, &rWith)->m_nValue
             < m_nValue ?
            -1 :
           SAL_STATIC_CAST(const CntUInt32Item *, &rWith)->m_nValue
             == m_nValue ?
            0 : 1;
}

//============================================================================
// virtual
SfxItemPresentation CntUInt32Item::GetPresentation(SfxItemPresentation,
                                                   SfxMapUnit, SfxMapUnit,
                                                   XubString & rText,
                                                   const ::IntlWrapper *)
    const
{
    DBG_CHKTHIS(CntUInt32Item, 0);
    rText = XubString::CreateFromInt64(m_nValue);
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

//============================================================================
// virtual
bool CntUInt32Item::QueryValue(com::sun::star::uno::Any& rVal,BYTE) const
{
    sal_Int32 nValue = m_nValue;
    DBG_ASSERT( nValue>=0, "Overflow in UInt32 value!");
    rVal <<= nValue;
    return true;
}

//============================================================================
// virtual
bool CntUInt32Item::PutValue(const com::sun::star::uno::Any& rVal,BYTE)
{
    sal_Int32 nValue = 0;
    if (rVal >>= nValue)
    {
        DBG_ASSERT( nValue>=0, "Overflow in UInt32 value!");
        m_nValue = nValue;
        return true;
    }

    DBG_ERROR( "CntUInt32Item::PutValue - Wrong type!" );
    return false;
}

//============================================================================
// virtual
SfxPoolItem * CntUInt32Item::Create(SvStream & rStream, USHORT) const
{
    DBG_CHKTHIS(CntUInt32Item, 0);
    return new CntUInt32Item(Which(), rStream);
}

//============================================================================
// virtual
SvStream & CntUInt32Item::Store(SvStream &rStream, USHORT) const
{
    DBG_CHKTHIS(CntUInt32Item, 0);
    rStream << static_cast<sal_uInt32>(m_nValue);
    return rStream;
}

//============================================================================
// virtual
SfxPoolItem * CntUInt32Item::Clone(SfxItemPool *) const
{
    DBG_CHKTHIS(CntUInt32Item, 0);
    return new CntUInt32Item(*this);
}

//============================================================================
// virtual
UINT32 CntUInt32Item::GetMin() const
{
    DBG_CHKTHIS(CntUInt32Item, 0);
    return 0;
}

//============================================================================
// virtual
UINT32 CntUInt32Item::GetMax() const
{
    DBG_CHKTHIS(CntUInt32Item, 0);
    return 0xFFFFFFFF;
}

//============================================================================
// virtual
SfxFieldUnit CntUInt32Item::GetUnit() const
{
    DBG_CHKTHIS(CntUInt32Item, 0);
    return SFX_FUNIT_NONE;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
