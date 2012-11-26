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
#include <svl/cintitem.hxx>

//============================================================================
//
//  class CntByteItem
//
//============================================================================

DBG_NAME(CntByteItem)

//============================================================================
CntByteItem::CntByteItem(sal_uInt16 which, SvStream & rStream):
    SfxPoolItem(which)
{
    DBG_CTOR(CntByteItem, 0);
    rStream >> m_nValue;
}

//============================================================================
// virtual
int CntByteItem::operator ==(const SfxPoolItem & rItem) const
{
    DBG_CHKTHIS(CntByteItem, 0);
    DBG_ASSERT(dynamic_cast< const CntByteItem* >(&rItem), "CntByteItem::operator ==(): Bad type");
    return m_nValue == SAL_STATIC_CAST(const CntByteItem *, &rItem)->m_nValue;
}

//============================================================================
// virtual
int CntByteItem::Compare(const SfxPoolItem & rWith) const
{
    DBG_CHKTHIS(CntByteItem, 0);
    DBG_ASSERT(dynamic_cast< const CntByteItem* >(&rWith), "CntByteItem::Compare(): Bad type");
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
                                                 const IntlWrapper *) const
{
    DBG_CHKTHIS(CntByteItem, 0);
    rText = XubString::CreateFromInt32(m_nValue);
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

//============================================================================
// virtual
sal_Bool CntByteItem::QueryValue(com::sun::star::uno::Any& rVal,sal_uInt8) const
{
    sal_Int8 nValue = m_nValue;
    rVal <<= nValue;
    return sal_True;
}

//============================================================================
// virtual
sal_Bool CntByteItem::PutValue(const com::sun::star::uno::Any& rVal,sal_uInt8)
{
    sal_Int8 nValue = sal_Int8();
    if (rVal >>= nValue)
    {
        m_nValue = nValue;
        return sal_True;
    }

    DBG_ERROR( "CntByteItem::PutValue - Wrong type!" );
    return sal_False;
}

//============================================================================
// virtual
SfxPoolItem * CntByteItem::Create(SvStream & rStream, sal_uInt16) const
{
    DBG_CHKTHIS(CntByteItem, 0);
    short nTheValue = 0;
    rStream >> nTheValue;
    return new CntByteItem(Which(), sal_uInt8(nTheValue));
}

//============================================================================
// virtual
SvStream & CntByteItem::Store(SvStream & rStream, sal_uInt16) const
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
sal_uInt8 CntByteItem::GetMin() const
{
    DBG_CHKTHIS(CntByteItem, 0);
    return 0;
}

//============================================================================
// virtual
sal_uInt8 CntByteItem::GetMax() const
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
CntUInt16Item::CntUInt16Item(sal_uInt16 which, SvStream & rStream) :
    SfxPoolItem(which)
{
    DBG_CTOR(CntUInt16Item, 0);
    sal_uInt16 nTheValue = 0;
    rStream >> nTheValue;
    m_nValue = nTheValue;
}

//============================================================================
// virtual
int CntUInt16Item::operator ==(const SfxPoolItem & rItem) const
{
    DBG_CHKTHIS(CntUInt16Item, 0);
    DBG_ASSERT(dynamic_cast< const CntUInt16Item* >(&rItem), "CntUInt16Item::operator ==(): Bad type");
    return m_nValue == SAL_STATIC_CAST(const CntUInt16Item *, &rItem)->
                        m_nValue;
}

//============================================================================
// virtual
int CntUInt16Item::Compare(const SfxPoolItem & rWith) const
{
    DBG_CHKTHIS(CntUInt16Item, 0);
    DBG_ASSERT(dynamic_cast< const CntUInt16Item* >(&rWith), "CntUInt16Item::Compare(): Bad type");
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
                                                   const IntlWrapper *)
    const
{
    DBG_CHKTHIS(CntUInt16Item, 0);
    rText = XubString::CreateFromInt32(m_nValue);
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

//============================================================================
// virtual
sal_Bool CntUInt16Item::QueryValue(com::sun::star::uno::Any& rVal,sal_uInt8) const
{
    sal_Int32 nValue = m_nValue;
    rVal <<= nValue;
    return sal_True;
}

//============================================================================
// virtual
sal_Bool CntUInt16Item::PutValue(const com::sun::star::uno::Any& rVal,sal_uInt8)
{
    sal_Int32 nValue = 0;
    if (rVal >>= nValue)
    {
        DBG_ASSERT( nValue <= USHRT_MAX, "Overflow in UInt16 value!");
        m_nValue = (sal_uInt16)nValue;
        return sal_True;
    }

    DBG_ERROR( "CntUInt16Item::PutValue - Wrong type!" );
    return sal_False;
}

//============================================================================
// virtual
SfxPoolItem * CntUInt16Item::Create(SvStream & rStream, sal_uInt16) const
{
    DBG_CHKTHIS(CntUInt16Item, 0);
    return new CntUInt16Item(Which(), rStream);
}

//============================================================================
// virtual
SvStream & CntUInt16Item::Store(SvStream &rStream, sal_uInt16) const
{
    DBG_CHKTHIS(CntUInt16Item, 0);
    rStream << sal_uInt16(m_nValue);
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
sal_uInt16 CntUInt16Item::GetMin() const
{
    DBG_CHKTHIS(CntUInt16Item, 0);
    return 0;
}

//============================================================================
// virtual
sal_uInt16 CntUInt16Item::GetMax() const
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
CntInt32Item::CntInt32Item(sal_uInt16 which, SvStream & rStream) :
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
    DBG_ASSERT(dynamic_cast< const CntInt32Item* >(&rItem), "CntInt32Item::operator ==(): Bad type");
    return m_nValue == SAL_STATIC_CAST(const CntInt32Item *, &rItem)->
                        m_nValue;
}

//============================================================================
// virtual
int CntInt32Item::Compare(const SfxPoolItem & rWith) const
{
    DBG_CHKTHIS(CntInt32Item, 0);
    DBG_ASSERT(dynamic_cast< const CntInt32Item* >(&rWith), "CntInt32Item::Compare(): Bad type");
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
                                                  const IntlWrapper *) const
{
    DBG_CHKTHIS(CntInt32Item, 0);
    rText = XubString::CreateFromInt32(m_nValue);
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

//============================================================================
// virtual
sal_Bool CntInt32Item::QueryValue(com::sun::star::uno::Any& rVal,sal_uInt8) const
{
    sal_Int32 nValue = m_nValue;
    rVal <<= nValue;
    return sal_True;
}

//============================================================================
// virtual
sal_Bool CntInt32Item::PutValue(const com::sun::star::uno::Any& rVal,sal_uInt8)
{
    sal_Int32 nValue = 0;
    if (rVal >>= nValue)
    {
        m_nValue = nValue;
        return sal_True;
    }

    DBG_ERROR( "CntInt32Item::PutValue - Wrong type!" );
    return sal_False;
}

//============================================================================
// virtual
SfxPoolItem * CntInt32Item::Create(SvStream & rStream, sal_uInt16) const
{
    DBG_CHKTHIS(CntInt32Item, 0);
    return new CntInt32Item(Which(), rStream);
}

//============================================================================
// virtual
SvStream & CntInt32Item::Store(SvStream &rStream, sal_uInt16) const
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
sal_Int32 CntInt32Item::GetMin() const
{
    DBG_CHKTHIS(CntInt32Item, 0);
    return sal_Int32(0x80000000);
}

//============================================================================
// virtual
sal_Int32 CntInt32Item::GetMax() const
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
CntUInt32Item::CntUInt32Item(sal_uInt16 which, SvStream & rStream) :
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
    DBG_ASSERT(dynamic_cast< const CntUInt32Item* >(&rItem), "CntUInt32Item::operator ==(): Bad type");
    return m_nValue == SAL_STATIC_CAST(const CntUInt32Item *, &rItem)->
                        m_nValue;
}

//============================================================================
// virtual
int CntUInt32Item::Compare(const SfxPoolItem & rWith) const
{
    DBG_CHKTHIS(CntUInt32Item, 0);
    DBG_ASSERT(dynamic_cast< const CntUInt32Item* >(&rWith), "CntUInt32Item::operator ==(): Bad type");
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
                                                   const IntlWrapper *)
    const
{
    DBG_CHKTHIS(CntUInt32Item, 0);
    rText = XubString::CreateFromInt64(m_nValue);
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

//============================================================================
// virtual
sal_Bool CntUInt32Item::QueryValue(com::sun::star::uno::Any& rVal,sal_uInt8) const
{
    sal_Int32 nValue = m_nValue;
    DBG_ASSERT( nValue>=0, "Overflow in UInt32 value!");
    rVal <<= nValue;
    return sal_True;
}

//============================================================================
// virtual
sal_Bool CntUInt32Item::PutValue(const com::sun::star::uno::Any& rVal,sal_uInt8)
{
    sal_Int32 nValue = 0;
    if (rVal >>= nValue)
    {
        DBG_ASSERT( nValue>=0, "Overflow in UInt32 value!");
        m_nValue = nValue;
        return sal_True;
    }

    DBG_ERROR( "CntUInt32Item::PutValue - Wrong type!" );
    return sal_False;
}

//============================================================================
// virtual
SfxPoolItem * CntUInt32Item::Create(SvStream & rStream, sal_uInt16) const
{
    DBG_CHKTHIS(CntUInt32Item, 0);
    return new CntUInt32Item(Which(), rStream);
}

//============================================================================
// virtual
SvStream & CntUInt32Item::Store(SvStream &rStream, sal_uInt16) const
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
sal_uInt32 CntUInt32Item::GetMin() const
{
    DBG_CHKTHIS(CntUInt32Item, 0);
    return 0;
}

//============================================================================
// virtual
sal_uInt32 CntUInt32Item::GetMax() const
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

