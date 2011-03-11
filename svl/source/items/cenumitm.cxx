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
#include "precompiled_svl.hxx"
#include <com/sun/star/uno/Any.hxx>
#include <tools/stream.hxx>
#include <svl/cenumitm.hxx>
#include <whassert.hxx>

#include <comphelper/extract.hxx>

//============================================================================
//
//  class SfxEnumItemInterface
//
//============================================================================

DBG_NAME(SfxEnumItemInterface)

//============================================================================
TYPEINIT1(SfxEnumItemInterface, SfxPoolItem)

//============================================================================
// virtual
int SfxEnumItemInterface::operator ==(const SfxPoolItem & rItem) const
{
    SFX_ASSERT(SfxPoolItem::operator ==(rItem), Which(), "unequal type");
    return GetEnumValue()
               == static_cast< const SfxEnumItemInterface * >(&rItem)->
                      GetEnumValue();
}

//============================================================================
// virtual
SfxItemPresentation
SfxEnumItemInterface::GetPresentation(SfxItemPresentation, SfxMapUnit,
                                      SfxMapUnit, XubString & rText,
                                      const IntlWrapper *) const
{
    rText = XubString::CreateFromInt32(GetEnumValue());
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

//============================================================================
// virtual
bool SfxEnumItemInterface::QueryValue(com::sun::star::uno::Any& rVal, sal_uInt8)
    const
{
    rVal <<= sal_Int32(GetEnumValue());
    return true;
}

//============================================================================
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

//============================================================================
XubString SfxEnumItemInterface::GetValueTextByPos(sal_uInt16) const
{
    DBG_WARNING("SfxEnumItemInterface::GetValueTextByPos(): Pure virtual");
    return XubString();
}

//============================================================================
// virtual
sal_uInt16 SfxEnumItemInterface::GetValueByPos(sal_uInt16 nPos) const
{
    return nPos;
}

//============================================================================
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

//============================================================================
// virtual
int SfxEnumItemInterface::HasBoolValue() const
{
    return false;
}

//============================================================================
// virtual
sal_Bool SfxEnumItemInterface::GetBoolValue() const
{
    return false;
}

//============================================================================
// virtual
void SfxEnumItemInterface::SetBoolValue(sal_Bool)
{}

//============================================================================
//
//  class CntEnumItem
//
//============================================================================

DBG_NAME(CntEnumItem)

//============================================================================
CntEnumItem::CntEnumItem(sal_uInt16 which, SvStream & rStream):
    SfxEnumItemInterface(which)
{
    m_nValue = 0;
    rStream >> m_nValue;
}

//============================================================================
TYPEINIT1(CntEnumItem, SfxEnumItemInterface)

//============================================================================
// virtual
SvStream & CntEnumItem::Store(SvStream & rStream, sal_uInt16) const
{
    rStream << m_nValue;
    return rStream;
}

//============================================================================
// virtual
sal_uInt16 CntEnumItem::GetEnumValue() const
{
    return GetValue();
}

//============================================================================
// virtual
void CntEnumItem::SetEnumValue(sal_uInt16 nTheValue)
{
    SetValue(nTheValue);
}

//============================================================================
//
//  class CntBoolItem
//
//============================================================================

DBG_NAME(CntBoolItem)

//============================================================================
TYPEINIT1_AUTOFACTORY(CntBoolItem, SfxPoolItem)

//============================================================================
CntBoolItem::CntBoolItem(sal_uInt16 which, SvStream & rStream):
    SfxPoolItem(which)
{
    m_bValue = false;
    rStream >> m_bValue;
}

//============================================================================
// virtual
int CntBoolItem::operator ==(const SfxPoolItem & rItem) const
{
    DBG_ASSERT(rItem.ISA(CntBoolItem),
               "CntBoolItem::operator ==(): Bad type");
    return m_bValue == static_cast< CntBoolItem const * >(&rItem)->m_bValue;
}

//============================================================================
// virtual
int CntBoolItem::Compare(const SfxPoolItem & rWith) const
{
    DBG_ASSERT(rWith.ISA(CntBoolItem), "CntBoolItem::Compare(): Bad type");
    return m_bValue == static_cast< CntBoolItem const * >(&rWith)->m_bValue ?
               0 : m_bValue ? -1 : 1;
}

//============================================================================
// virtual
SfxItemPresentation CntBoolItem::GetPresentation(SfxItemPresentation,
                                                 SfxMapUnit, SfxMapUnit,
                                                 UniString & rText,
                                                 const IntlWrapper *) const
{
    rText = GetValueTextByVal(m_bValue);
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

//============================================================================
// virtual
bool CntBoolItem::QueryValue(com::sun::star::uno::Any& rVal, sal_uInt8) const
{
    rVal <<= sal_Bool(m_bValue);
    return true;
}

//============================================================================
// virtual
bool CntBoolItem::PutValue(const com::sun::star::uno::Any& rVal, sal_uInt8)
{
    sal_Bool bTheValue = sal_Bool();
    if (rVal >>= bTheValue)
    {
        m_bValue = bTheValue;
        return true;
    }
    OSL_FAIL("CntBoolItem::PutValue(): Wrong type");
    return false;
}

//============================================================================
// virtual
SfxPoolItem * CntBoolItem::Create(SvStream & rStream, sal_uInt16) const
{
    return new CntBoolItem(Which(), rStream);
}

//============================================================================
// virtual
SvStream & CntBoolItem::Store(SvStream & rStream, sal_uInt16) const
{
    rStream << m_bValue;
    return rStream;
}

//============================================================================
// virtual
SfxPoolItem * CntBoolItem::Clone(SfxItemPool *) const
{
    return new CntBoolItem(*this);
}

//============================================================================
// virtual
sal_uInt16 CntBoolItem::GetValueCount() const
{
    return 2;
}

//============================================================================
// virtual
UniString CntBoolItem::GetValueTextByVal(sal_Bool bTheValue) const
{
    return
        bTheValue ?
            UniString::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("sal_True")) :
            UniString::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("sal_False"));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
