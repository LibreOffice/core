/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svl.hxx"
#include <com/sun/star/uno/Any.hxx>
#include <tools/stream.hxx>
#include <svl/cenumitm.hxx>
#include <whassert.hxx>

#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif

//============================================================================
//
//  class SfxEnumItemInterface
//
//============================================================================

DBG_NAME(SfxEnumItemInterface)

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
sal_Bool SfxEnumItemInterface::QueryValue(com::sun::star::uno::Any& rVal, sal_uInt8)
    const
{
    rVal <<= sal_Int32(GetEnumValue());
    return true;
}

//============================================================================
// virtual
sal_Bool SfxEnumItemInterface::PutValue(const com::sun::star::uno::Any& rVal,
                                    sal_uInt8)
{
    sal_Int32 nTheValue = 0;

    if ( ::cppu::enum2int( nTheValue, rVal ) )
    {
        SetEnumValue(sal_uInt16(nTheValue));
        return true;
    }
    DBG_ERROR("SfxEnumItemInterface::PutValue(): Wrong type");
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
    DBG_ASSERT(dynamic_cast< const CntBoolItem* >(&rItem), "CntBoolItem::operator ==(): Bad type");
    return m_bValue == static_cast< CntBoolItem const * >(&rItem)->m_bValue;
}

//============================================================================
// virtual
int CntBoolItem::Compare(const SfxPoolItem & rWith) const
{
    DBG_ASSERT(dynamic_cast< const CntBoolItem* >(&rWith), "CntBoolItem::Compare(): Bad type");
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
sal_Bool CntBoolItem::QueryValue(com::sun::star::uno::Any& rVal, sal_uInt8) const
{
    rVal <<= sal_Bool(m_bValue);
    return true;
}

//============================================================================
// virtual
sal_Bool CntBoolItem::PutValue(const com::sun::star::uno::Any& rVal, sal_uInt8)
{
    sal_Bool bTheValue = sal_Bool();
    if (rVal >>= bTheValue)
    {
        m_bValue = bTheValue;
        return true;
    }
    DBG_ERROR("CntBoolItem::PutValue(): Wrong type");
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

